#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sysfs.h"
#include "FaCfg.h"
#include "sysdebug.h"
#include "ComAPI.h"
#include "LibDbAPI.h"
#include "ParaMgr.h"
#include "ParaMgrHook.h"

CParaMgr g_pmParaMgr;
CParaMgr::CParaMgr()
{
    m_pbParaData = NULL;
    m_iParaLen = 0;
}

CParaMgr::~CParaMgr()
{
	Clean();
}

//文件格式:文件长度(4个字节)+CRC校验(4个字节)+数据1+数据2+...+数据n
//数据格式:数据项长度(2个字节,包含数据项ID/数据项BANK/数据项Point/数据项内容长度)
//+数据项ID(2个字节)+数据项BANK(2个字节)+数据项Point(2个字节)+数据项内容
int CParaMgr::LoadPara(char* szFileName)
{
    int f;
    char szName[255];
    int iFileLen;
    int iDataLen;
    int iLen;
    int iRet = 0;
    BYTE bBuf[32];
    DWORD dwCrc;
    WORD wMycrc;
   
	
	
	sprintf(szName, USER_CFG_PATH"%s", szFileName);

    f = open(szName, O_RDONLY, S_IREAD);
    if (f < 0)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : Open %s failed.\r\n", szName));
        iRet = -1;//打开文件失败
        return 0;
    }
    iFileLen = lseek(f, 0, SEEK_END);
    if (iFileLen < 8)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : %s is not a valid file.\r\n", szName));
        iRet = -2;//无效配置文件
        goto ERROROUT;
    }
    lseek(f, 0, SEEK_SET);
    if (read(f, bBuf, sizeof(DWORD)+sizeof(DWORD)) != sizeof(DWORD)+sizeof(DWORD))
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : read %s error.\r\n", szName));
        goto ERROROUT;
    }
    memcpy(&iLen, bBuf, sizeof(DWORD));
    memcpy(&dwCrc, &bBuf[4], sizeof(DWORD));
    if (iLen != iFileLen)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : file %s length error.\r\n", szName));
        iRet = -3;//配置文件长度不对
        goto ERROROUT;
    }
    if (iFileLen > 50*1024)//文件大小不能超过50K
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : file %s is to long.\r\n", szName));
        iRet = -4;//配置文件太大
		goto ERROROUT;        
    }
    
    iDataLen = iFileLen-sizeof(DWORD)-sizeof(DWORD);
    m_pbParaData = new BYTE[iDataLen];
    lseek(f, sizeof(DWORD)+sizeof(DWORD), SEEK_SET);
    if (read(f, m_pbParaData, iDataLen) != iDataLen)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : read %s error.\r\n", szName));
        iRet = -5;//读取配置文件出错
		goto ERROROUT;
    }
    m_iParaLen = iDataLen;
    ///////////////////////////////////////////////////////////////////////
	wMycrc = 0;	
	for(int i=0; i<m_iParaLen/512; i++)
	{
		wMycrc = get_crc_16(wMycrc, m_pbParaData+i*512, 512);
	}	
	if(m_iParaLen%512 > 0)
	{
		wMycrc = get_crc_16(wMycrc, m_pbParaData+512*(m_iParaLen/512), m_iParaLen%512);
	}
	if (dwCrc != (DWORD)wMycrc)
	{
	    iRet = -6;//配置文件CRC错
	    goto ERROROUT;
	}
	
    close(f);
    return iRet;
    
ERROROUT:
	if (m_pbParaData != NULL)
	{
	    delete []m_pbParaData;
	    m_iParaLen = 0;
	}
	Clean();
	if (f >= 0)
	    close(f);
//  unlink(szName);
    return iRet;
}

//数据格式:数据项长度(2个字节,包含数据项ID/数据项BANK/数据项Point/数据项内容长度)
//+数据项ID(2个字节)+数据项BANK(2个字节)+数据项Point(2个字节)+数据项内容
bool CParaMgr::Parse(void)
{
    BYTE* p = m_pbParaData;
    WORD wID;
    WORD wBank;
    WORD wPoint;
    WORD wLen;
	BYTE* p1;
    
    if (m_pbParaData == NULL)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : Parse para error, data not exist.\r\n"));
        goto ERROROUT;    
    }
    while (m_iParaLen > 0)
    {
		p1 = p;
        memcpy(&wLen, p, sizeof(WORD));
        p += sizeof(WORD);
        memcpy(&wID, p, sizeof(WORD));
        p += sizeof(WORD);
        memcpy(&wBank, p, sizeof(WORD));
        p += sizeof(WORD);
        memcpy(&wPoint, p, sizeof(WORD));
        p += sizeof(WORD);

		if (!OnWriteSpecialPara(wBank, wPoint, wID, p1, wLen))
        {
            if (WriteItemEx(wBank, wPoint, wID, p) != (wLen-sizeof(WORD)*3))
            {
                DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : Parse para error, BANK=%d, POINT=%d, ID=%d\r\n", wBank, wPoint, wID));
                goto ERROROUT;
            }
        }
        m_iParaLen -= (wLen+sizeof(WORD));
        p += (wLen-sizeof(WORD)*3);
    }
    Clean();
    return true;
ERROROUT:
	Clean();
	return false;    
}

void CParaMgr::Clean(void)
{
    if (m_pbParaData != NULL)
	{
	    delete []m_pbParaData;
	    m_pbParaData = NULL;
	    m_iParaLen = 0;
	}
}
