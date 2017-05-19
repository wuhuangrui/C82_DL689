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

//�ļ���ʽ:�ļ�����(4���ֽ�)+CRCУ��(4���ֽ�)+����1+����2+...+����n
//���ݸ�ʽ:�������(2���ֽ�,����������ID/������BANK/������Point/���������ݳ���)
//+������ID(2���ֽ�)+������BANK(2���ֽ�)+������Point(2���ֽ�)+����������
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
        iRet = -1;//���ļ�ʧ��
        return 0;
    }
    iFileLen = lseek(f, 0, SEEK_END);
    if (iFileLen < 8)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : %s is not a valid file.\r\n", szName));
        iRet = -2;//��Ч�����ļ�
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
        iRet = -3;//�����ļ����Ȳ���
        goto ERROROUT;
    }
    if (iFileLen > 50*1024)//�ļ���С���ܳ���50K
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : file %s is to long.\r\n", szName));
        iRet = -4;//�����ļ�̫��
		goto ERROROUT;        
    }
    
    iDataLen = iFileLen-sizeof(DWORD)-sizeof(DWORD);
    m_pbParaData = new BYTE[iDataLen];
    lseek(f, sizeof(DWORD)+sizeof(DWORD), SEEK_SET);
    if (read(f, m_pbParaData, iDataLen) != iDataLen)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : read %s error.\r\n", szName));
        iRet = -5;//��ȡ�����ļ�����
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
	    iRet = -6;//�����ļ�CRC��
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

//���ݸ�ʽ:�������(2���ֽ�,����������ID/������BANK/������Point/���������ݳ���)
//+������ID(2���ֽ�)+������BANK(2���ֽ�)+������Point(2���ֽ�)+����������
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
