//////////////////////////////////////////////
//	File name:      TTSkinTool.cpp
//	Author:	        
//  Version:        V1.0
//	Date:           2014/1/17
//	Description:    皮肤库打包 
//	Function List:	BOOL GetSkinFilePath(const CString& strFilePath);
//                  BOOL GetSkinFileInfo(const CString& strFilePath);
//                  BOOL OnSkinToBin();
//                  BOOL OnSetSkinPackagePath(const CString& strPath);
//                  BOOL OnCovToBin();
//                  LPBYTE OnIndexStart(const CString& strIndexPath, DWORD* bufSize);
//                  BOOL IsFilter(LPCTSTR lpFilter, const CString& strSource);
//                  BOOL SetSkinFilePath(const CString& strFilePath);
//                  BOOL SetOutPath(const CString& strOutPath);
//                  BOOL Formatting();
//                  BOOL Search(const CString& strFilePath);
//                  BOOL SetTargetFile(const CString& strTargetFile);
//	History:
//	Date:
//	Author:
//	Modification:
//////////////////////////////////////////////////
//#include "stdafx.h"
#include "TTSkinTool.h"
#include <afx.h>
#include <atlstr.h>

#if _UNICODE
#define STR_EMPTY L""
#else
#define STR_EMPTY ""
#endif

//const TCHAR g_strFilter[] = _T(".jpg .bmp .png .BMP .JPG .PNG");

CTTSkinTool::CTTSkinTool(void)
{
    m_bIndexOfFirst = true;
    m_nSkinNum = 0;
    m_strTepPath = "";           
    m_strSkinPackName = "";
    m_strOutPutPath = "";
    m_strTargetFile = "";
    m_pData = NULL;
    m_nDataHeadSize = 0;
}

CTTSkinTool::~CTTSkinTool(void)
{
    if (NULL != m_pData)
    {
        delete[] m_pData;
        m_pData = NULL;
    }
}

///////////////////////////////////////////////////////
//	Function:			OnSetSkinPackagePath(CString path)
//	Description:		设置皮肤库路径，并遍历路径下皮肤文件获取信息
//                      并压缩成二进制文件
//	Calls:				GetSkinFilePath(CString path) 获取皮肤文件信息
//						OnCovToBin()将皮肤文件写到二进制文件中
//	Called By:          OnSetPath(CString path)
//	Table Accessed: 
//	Table Updated:
//	Input:				皮肤库路径参数 path
//	Output:				二进制皮肤文件 *.dat
//	Return:				void
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::OnSetSkinPackagePath(const CString& strPath)
{
    m_strSkinPackName = strPath.Mid(strPath.ReverseFind('\\') + 1); //获取库名
    m_strTepPath = strPath.Left(strPath.ReverseFind('\\') + 1);   //获取绝对路径
    swprintf_s(m_SkinInfo.szName, L"%s", m_strSkinPackName);
    wcscpy_s(m_SkinInfo.szVersion, L".0.1"); 
    GetSkinFilePath(strPath);   //遍历每个皮肤路径
    m_SkinInfo.nNum = m_nSkinNum;

    return TRUE;
}


///////////////////////////////////////////////////////
//	Function:				GetSkinFilePath(CString path)
//	Description:		    遍历皮肤库
//	Calls:					GetSkinFileInfo(strPath); //获取皮肤文件信息
//	Called By:              OnSetPath(CString path)
//	Table Accessed: 
//	Table Updated:
//	Input:					皮肤库路径参数 path
//	Output:					
//	Return:					void
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::GetSkinFilePath(const CString& strFilePath)
{
    CString strPath = strFilePath;
    CString filefile = _TEXT("");

    CFileFind filefind;
    if (strPath.Right(1) != "\\")
    {
        strPath += _TEXT("\\");
    }
    strPath += "*.*";
    BOOL bRet = filefind.FindFile(strPath);
    CString strTmpPath;

    while (bRet)
    {
        bRet = filefind.FindNextFile();
        filefile = filefind.GetFileName();
        if ( (!filefind.IsDots()) && (!filefind.IsDirectory()))
        {
            //查找到皮肤文件
            //判断文件是否是图片文件
            strTmpPath = filefind.GetFilePath();
            //去掉过滤，所有文件都打包
            //if(IsFilter(g_strFilter , strTmpPath))
            //{
                //m_vTTSkinPath.push_back(strTmpPath.GetBuffer(0));
                //strTmpPath.ReleaseBuffer();
                GetSkinFileInfo(strTmpPath); //获取皮肤文件信息
            //}
        }
        else if (filefind.IsDots())
        {
            continue;
        }
        else if (filefind.IsDirectory())
        {
            //找到文件夹
            strPath = filefind.GetFilePath();
            filefile = _TEXT("");
            GetSkinFilePath(strPath);
        }
    }

    return TRUE;
}

///////////////////////////////////////////////////////
//	Function:	    GetSkinFileInfo(CString m_FilePath)
//	Description:	获取皮肤文件基本信息
//	Calls:
//	Called By:      GetSkinFileInfo(strPath); //获取皮肤文件信息
//	Table Accessed: 
//	Table Updated:
//	Input:	        CString strFilePath //文件路径
//	Output:	
//	Return:		    void
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::GetSkinFileInfo(const CString& strFilePath)
{
    CFile file;
    if (!file.Open(strFilePath, CFile::modeRead))
    {
        //文件损坏
        return FALSE;
    }

    //取得文件长度
    unsigned int nFileLen = (unsigned int)file.GetLength();
    //按文件长度申请空间
    LPBYTE pByte = new BYTE[nFileLen];
    if (NULL == pByte)
    {
        return FALSE;
    }
    ZeroMemory(pByte, (size_t)nFileLen);

    //读取文件所有数据
    UINT nBytesRead = file.Read(pByte, (UINT)nFileLen);
    if ((nBytesRead != nFileLen) || (nFileLen <= 0))
    {
        return FALSE;
    }

    m_nSkinNum++;

    //将皮肤文件数据保存到m_FileData 
    m_FileData.push_back(pByte);

    file.Close();

    //文件路径
    CString strTmp = strFilePath.Right(strFilePath.GetLength() - m_strTepPath.GetLength());

    TTSkinNode m_SkinNode;     //定义一个皮肤文件结构体，保存相关信息

    //m_SkinNode.szPath = strTmp.GetBuffer(0);
    //strTmp.ReleaseBuffer();
    swprintf_s(m_SkinNode.szPath, L"%s", strTmp.GetBuffer(0));
    m_SkinNode.nPos = 0;
    m_SkinNode.nSizeByte = nFileLen;

    m_vTTSkin.push_back(m_SkinNode);

    return TRUE;
}


///////////////////////////////////////////////////////
//	Function:		    OnCovToBin()()
//	Description:		将皮肤文件转存二进制文件中
//	Calls:
//	Called By:          OnSetPath(CString path)
//	Table Accessed: 
//	Table Updated:
//	Input:					
//	Output:	
//	Return:				void
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::OnCovToBin()
{
    int nSize = sizeof(TTSkinInfo);
    //输出文件及路径
    CString strOut = m_strOutPutPath +_T("\\") + m_strSkinPackName + _T(".dat");

    m_InFile.open(strOut, ios::out | ios::binary);

    //用fstream类定义输入输出二进制文件流对象iofile
    if(!m_InFile)
    {
        return FALSE;
    }

    //1.向磁盘文件输出基本信息
    m_InFile.write((char *)&m_SkinInfo, nSize);

    //2.文件头大小
    int nNodeSize = m_vTTSkin.size() * sizeof(TTSkinNode); //文件节点总大小
    unsigned long nDataSize = 0;

    unsigned int nPosTemp = nNodeSize + nSize + sizeof(unsigned long);//保存前i个皮肤文件数据大小总和
    for (unsigned int i = 0; i < m_vTTSkin.size(); i++)    //计算每个皮肤文件数据保存起始位置
    {
        m_vTTSkin[i].nPos = nPosTemp;
        nPosTemp += m_vTTSkin[i].nSizeByte;

        nDataSize += m_vTTSkin[i].nSizeByte;

        //向磁盘文件输出二进制皮肤信息数据
        m_InFile.write((char *)&m_vTTSkin[i], sizeof(TTSkinNode));
    }
    
    //向文件写入二进制数据的总大小
    m_InFile.write((char*)&nDataSize, sizeof(unsigned long));

    //3.获取皮肤文件数据并保存为二进制
    //OnSkinToBin();
    LPBYTE pByte = NULL;
    unsigned int bufSize = 0;
    for (unsigned int i = 0; i < m_FileData.size(); i++)
    {
        pByte = m_FileData[i];
        bufSize = m_vTTSkin[i].nSizeByte;
        m_InFile.write((char *)pByte, (streamsize)bufSize);

        delete[] pByte;
        pByte = NULL;
    }
    
    m_InFile.close();

    return TRUE;
}


///////////////////////////////////////////////////////
//	Function:				OnSkinToBin(CString m_FilePath)
//	Description:		    将路径参数对应皮肤文件转存二进制文件
//	Calls:					
//	Called By:              OnCovToBin()
//	Table Accessed: 
//	Table Updated:
//	Input:					皮肤库路径参数 path
//	Output:					
//	Return:					void
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::OnSkinToBin()
{
    LPBYTE pByte = NULL;
    unsigned int bufSize = 0;
    for (unsigned int i = 0; i < m_FileData.size(); i++)
    {
        pByte = m_FileData[i];
        bufSize = m_vTTSkin[i].nSizeByte;
        m_InFile.write((char *)pByte, (streamsize)bufSize);

        delete[] pByte;
        pByte = NULL;
    }

    return TRUE;
}


///////////////////////////////////////////////////////
//	Function:				OnIndexStart(CString strIndexPath)
//	Description:		    根据传入路径参数，在打包文件中查找对应皮肤数据
//	Calls:
//	Called By:
//	Table Accessed: 
//	Table Updated:
//	Input:					CString strIndexPath
//	Output:	
//	Return:					void
//	Others:
////////////////////////////////////////////
LPBYTE CTTSkinTool::OnIndexStart(const CString& strIndexPath, DWORD* bufSize)
{
    if (strIndexPath.IsEmpty() || m_strTargetFile.IsEmpty())
    {
        return NULL;
    }

    CString strFileName = m_strTargetFile.Mid(m_strTargetFile.ReverseFind('\\') +1);
    CString strFile = strFileName.Left(strFileName.ReverseFind('.'));

    TTSkinNode sSkinNode;   //定义一个皮肤文件结构体，保存相关信息

    if (m_bIndexOfFirst) //第一次查询需要建立查找索引
    {
        m_bIndexOfFirst = false;

        m_OutFile.open(m_strTargetFile, ios::in | ios::binary);
        if(!m_OutFile)
        {
            return NULL;
        }

        int nNum = 0;
        m_OutFile.seekg(80, ios::beg); 
        m_OutFile.read((char *)&nNum, sizeof(int));

        //按文件头长度申请空间
        LPBYTE pByte = new BYTE[nNum * sizeof(TTSkinNode)];
        if (NULL == pByte)
        {
            return NULL;
        }
        //ZeroMemory(pByte, m_num * sizeof(TTSkinNode));

        m_OutFile.read((char *)pByte, nNum*sizeof(TTSkinNode));

        for (int i = 0; i < nNum; i++)
        {
            CopyMemory((void *)&sSkinNode, (void *)(pByte + i*sizeof(TTSkinNode)), sizeof(TTSkinNode));
            m_mapTTSkinNodeHM[sSkinNode.szPath] = sSkinNode;
        }

        //获取二进制数据的总大小
        unsigned long nDataSize = 0;
        unsigned int nPos = sizeof(_TTSkinInfo) + nNum * sizeof(TTSkinNode);
        m_OutFile.seekg(nPos, ios::beg); 
        m_OutFile.read((char *)&nDataSize, sizeof(unsigned long));

        //将二进制数据读入到buf中
        if (0 != nDataSize)
        {
            m_pData = new BYTE[nDataSize];
            if (NULL == m_pData)
            {
                return NULL;
            }
            nPos += sizeof(unsigned long);
            m_OutFile.seekg(nPos, ios::beg);
            m_OutFile.read((char*)m_pData, nDataSize);
            m_nDataHeadSize = sizeof(_TTSkinInfo) + nNum * sizeof(TTSkinNode) + sizeof(unsigned long);
        }

        m_OutFile.close();

        delete[] pByte;
        pByte = NULL;
    }
    
    CString strTmp = strIndexPath.Mid(strIndexPath.Find(strFile.GetBuffer(0)));
    hash_map<wstring, TTSkinNode>::iterator iterEnd = m_mapTTSkinNodeHM.end();
    hash_map<wstring, TTSkinNode>::iterator iter = m_mapTTSkinNodeHM.find(strTmp.GetBuffer(0));
    if (iterEnd != iter)
    {
        *bufSize = (*iter).second.nSizeByte;
        unsigned int pos = (*iter).second.nPos;
        //按文件长度申请空间
        LPBYTE pByte = NULL;

        if (NULL != m_pData)
        {
            pos = pos - m_nDataHeadSize; 
            pByte = m_pData + pos;
            return pByte;
        }
    }
   
    return NULL;
}



///////////////////////////////////////////////////////
//	Function:				SetSkinFilePath(const CString& strFilePath)
//	Description:		    设置皮肤库路径
//	Calls:
//	Called By:
//	Table Accessed: 
//	Table Updated:
//	Input:					CString strFilePath
//	Output:	
//	Return:					BOOL
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::SetSkinFilePath(const TTSTRING& strFilePath)
{
    if(strFilePath != STR_EMPTY)
    {
        m_strSkinPath = strFilePath.c_str();
        return TRUE;
    }

    return FALSE;    
}


///////////////////////////////////////////////////////
//	Function:				SetOutPath(const CString& strOutPath)
//	Description:		    设置输出路径
//	Calls:
//	Called By:
//	Table Accessed: 
//	Table Updated:
//	Input:					CString strOutPath
//	Output:	
//	Return:					BOOL
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::SetOutPath(const TTSTRING& strOutPath)
{
    if (strOutPath != STR_EMPTY)
    {
        m_strOutPutPath = strOutPath.c_str();
        return TRUE;
    }

    return FALSE;
}


///////////////////////////////////////////////////////
//	Function:				Formatting()
//	Description:		    转换
//	Calls:
//	Called By:
//	Table Accessed: 
//	Table Updated:
//	Input:					
//	Output:	
//	Return:					BOOL
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::Formatting()
{
    if (!m_strSkinPath.IsEmpty())
    {
        OnSetSkinPackagePath(m_strSkinPath);
    }

    if(OnCovToBin())
    {
        return TRUE;
    }

    return FALSE;
}


///////////////////////////////////////////////////////
//	Function:				Search(const CString& strFilePath)
//	Description:		    查找
//	Calls:
//	Called By:
//	Table Accessed: 
//	Table Updated:
//	Input:					CString strFilePath
//	Output:	                DWORD* bufSize
//	Return:					BOOL
//	Others:
////////////////////////////////////////////
LPBYTE CTTSkinTool::Search(const TTSTRING& strFilePath, DWORD* bufSize)
{
    LPBYTE pData = NULL;

    CString strTmpFilePath = strFilePath.c_str();
    pData = OnIndexStart(strTmpFilePath, bufSize);

    return pData;
}


///////////////////////////////////////////////////////
//	Function:				IsFilter(LPCTSTR lpFilter, const CString& strSource)
//	Description:		    文件过滤，判断后缀(.bmp .png .jpg)
//	Calls:
//	Called By:
//	Table Accessed: 
//	Table Updated:
//	Input:					CString strSource, LPCTSTR lpFilter
//	Output:	
//	Return:					BOOL
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::IsFilter(LPCTSTR lpFilter, const CString& strSource)
{
    if (strSource.IsEmpty() || NULL == lpFilter)
    {
        return FALSE;
    }

    CString strFileName = strSource.Mid(strSource.ReverseFind('\\') + 1);

    if(StrStr(lpFilter, StrStr(strFileName, TEXT(".")))!=NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



///////////////////////////////////////////////////////
//	Function:				SetTargetFile(const CString& strTargetFile)
//	Description:		    设置目标文件路径
//	Calls:
//	Called By:
//	Table Accessed: 
//	Table Updated:
//	Input:					CString strTargetFile
//	Output:	
//	Return:					BOOL
//	Others:
////////////////////////////////////////////
BOOL CTTSkinTool::SetTargetFile(const TTSTRING& strTargetFile)
{
    if (strTargetFile != STR_EMPTY)
    {   
        CString strTmp = strTargetFile.c_str();
        //两个字符串不同
        if(strTmp != m_strTargetFile)
        {
            m_bIndexOfFirst = true;
            m_strTargetFile = strTmp;
        }
        return TRUE;
    }

    return FALSE;
}