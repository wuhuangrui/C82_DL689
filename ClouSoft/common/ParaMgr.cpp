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
#include "OoFmt.h"
#include "CctTaskMangerOob.h"
#include "CctAPI.h"
#include "Trace.h"
#include "TaskManager.h"
#include "FrzTask.h"
#include "bios.h"
#include "FaProto.h"
#include "FileTran.h"
#include "FaAPI.h"


extern int OIWrite_Spec(const ToaMap* pOI, BYTE* pbBuf);
extern bool IsNeedWrSpec(const ToaMap* pOI);

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

	f = open(szName, O_RDONLY|O_BINARY, S_IREAD);

    if (f < 0)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : Open %s failed.\r\n", szName));
        iRet = -1;//打开文件失败
        return iRet;
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
	/*
	wMycrc = PPPINITFCS16;	
	for(int i=0; i<m_iParaLen/512; i++)
	{
		wMycrc = pppfcs16(wMycrc,m_pbParaData+i*512, 512);
	}	
	if(m_iParaLen%512 > 0)
	{
		wMycrc = pppfcs16(wMycrc,m_pbParaData+512*(m_iParaLen/512), m_iParaLen%512);
	}
	wMycrc ^= 0xffff;
	*/

	wMycrc = CheckCrc16(m_pbParaData, m_iParaLen);

	if (dwCrc != (DWORD)wMycrc)
	{
	    iRet = -6;//配置文件CRC错
	    goto ERROROUT;
	}
	
    close(f);
    return iRet;
    
ERROROUT:
	Clean();
	if (f >= 0)
	    close(f);
//  unlink(szName);
    return iRet;
}


bool CParaMgr::ParseOAD(BYTE *pbData, DWORD dwLen) 
{
	WORD wOI, wLen;
	BYTE bAttr,bIndex;
	BYTE *pbTmp = pbData;

	wOI = OoOiToWord(pbTmp);
	pbTmp += 2;
	bAttr = *pbTmp++;
	bIndex = *pbTmp++;

	pbTmp += 4; //??4??????ID
	wLen = dwLen - (pbTmp-pbData);
	
	if (OoProWriteAttr_dft(wOI, bAttr, bIndex, pbTmp, wLen, true) < 0)
	{
		DTRACE(DB_CRITICAL, ("CParaMgr::ParseOAD : write Attr error. wOI = 0x%02x, bAttr = %d, bIndex = %d, \
				wLen = %d\r\n",wOI,bAttr,bIndex,wLen));
		return false;
	}

	return true;
}

bool CParaMgr::ParseOMD(BYTE *pbData, DWORD dwLen) 
{
	WORD wOI1;
	BYTE bMath, bMode;
	BYTE bRes[100];
	int iRet;

	WORD wOI2;
	BYTE bAttr, bIndex;
	BYTE *pbTmp = pbData;
	int iParaLen = 0;
	
	//??OMD ????
	wOI1 = OoOiToWord(pbTmp);
	pbTmp += 2;
	bMath = *pbTmp++;
	bMode = *pbTmp++;

	//??OAD ????
	wOI2 = OoOiToWord(pbTmp);
	pbTmp += 2;
	bAttr = *pbTmp++;
	bIndex = *pbTmp++;

	//DTRACE(DB_CRITICAL,("run here ParseOMD\r\n"));

	iRet = DoObjMethod(wOI1, bMath, bMode, pbTmp, &iParaLen, NULL, bRes);

	if ((iRet < 0) || (iRet > sizeof(bRes)))
	{
		DTRACE(DB_CRITICAL, \
			("CParaMgr::ParseOMD : DoObjMethod error. wOI1=%02x bMath=%x bMode=%x iRet = %d\r\n", wOI1, bMath, bMode, iRet));
		return false;
	}

	return true;
}

bool CParaMgr::ParseExtID(BYTE *pbData, DWORD dwLen)
{
	WORD wBn, wPn, wID;
	int iRet;
	bool fRet;
	BYTE *pbTmp = pbData;

	memcpy((BYTE*)&wID, (BYTE*)pbTmp, 2);
	pbTmp += 2;
	memcpy((BYTE*)&wBn, (BYTE*)pbTmp, 2);
	pbTmp += 2;
	memcpy((BYTE*)&wPn, (BYTE*)pbTmp, 2);
	pbTmp += 2;

	iRet = WriteItemEx(wBn, wPn, wID, pbTmp);

	if (iRet < 0)
	{
		DTRACE(DB_CRITICAL, ("CParaMgr::ParseExtID : WriteItemEx error. iRet = %d\r\n",iRet));
		return false;
	}

	return true;
}



//????:?????(2???,?????ID/???BANK/???Point/???????)
//+???ID(2???)+???BANK(2???)+???Point(2???)+?????
bool CParaMgr::Parse(void)
{
    BYTE* pbTmp = m_pbParaData;
    WORD wLen, wLen1;
    BYTE bType;
    BYTE* pbTmp1;
    
    //DTRACE(DB_CRITICAL,("run here Parse\r\n"));
    if (m_pbParaData == NULL)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::LoadPara : Parse para error, data not exist.\r\n"));
        goto ERROROUT;    
    }
    while (m_iParaLen > 0)
    {
        memcpy(&wLen, pbTmp, sizeof(WORD));
        pbTmp  += sizeof(WORD);
		bType = *pbTmp++ ;
		pbTmp1 = pbTmp;
		wLen1 = wLen - 1;

		if (bType == DATA_TYPE_OAD)
		{
			if (ParseOAD(pbTmp1, wLen1) == false)
			{
				DTRACE(DB_CRITICAL, ("CParaMgr::ParseOAD :error\r\n"));
	            goto ERROROUT;
			}

		}
		else if (bType == DATA_TYPE_OMD)
		{
			if (ParseOMD(pbTmp1, wLen1) == false)
			{
				DTRACE(DB_CRITICAL, ("CParaMgr::ParseOMD :error\r\n"));
	            goto ERROROUT;
			}
		}
		else if (bType == DATA_TYPE_EXT_ID)
		{
			if (ParseExtID(pbTmp1, wLen1) == false)
			{
				DTRACE(DB_CRITICAL, ("CParaMgr::ParseExtID :error\r\n"));
	            goto ERROROUT;
			}
		}
		else 
		{
			DTRACE(DB_CRITICAL, ("CParaMgr::Parse : no define data type. bType = %d\r\n",bType));
		}

        m_iParaLen -= (wLen + sizeof(WORD));

		
		pbTmp += (wLen - sizeof(BYTE));
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



static DWORD dwDftOam2OmdMap[][2] = {
	//???
	{0x23010200, 0x23010400},
	{0x23020200, 0x23020400},
	{0x23030200, 0x23030400},
	{0x23040200, 0x23040400},
	{0x23050200, 0x23050400},
	{0x23060200, 0x23060400},
	{0x23070200, 0x23070400},
	{0x23080200, 0x23080400},
	//??
	{0x24010400, 0x24017f00},
	{0x24020400, 0x24027f00},
	{0x24030400, 0x24037f00},
	{0x24040400, 0x24047f00},
	{0x24050400, 0x24057f00},
	{0x24060400, 0x24067f00},
	{0x24070400, 0x24077f00},
	{0x24080400, 0x24087f00},
	//???
	{0x50000300, 0x50000700},
	{0x50010300, 0x50010700},
	{0x50020300, 0x50020700},
	{0x50030300, 0x50030700},
	{0x50040300, 0x50040700},
	{0x50050300, 0x50050700},
	{0x50060300, 0x50060700},
	{0x50070300, 0x50070700},
	{0x50080300, 0x50080700},
	{0x50090300, 0x50090700},
	{0x500A0300, 0x500A0700},
	{0x500B0300, 0x500B0700},
	//?????
	{0x60120200, 0x60127f00},
	{0x60140200, 0x60147f00},
	{0x60160200, 0x60167f00},
	{0x60180200, 0x60187f00},
	{0x601c0200, 0x601c7f00},

};

DWORD CParaMgr::Search_OMD_from_OAD(DWORD dwOAD)
{
	int iIndex = 0;
	DWORD dwOMD = 0;

	for (iIndex=0; iIndex<sizeof(dwDftOam2OmdMap)/sizeof(dwDftOam2OmdMap[0]); iIndex++)
	{
		if (dwOAD == dwDftOam2OmdMap[iIndex][0])
		{
			dwOMD = dwDftOam2OmdMap[iIndex][1];
			break;
		}
	}
	return dwOMD;
}


//??:??OI??,?????????,???,??????????
int CParaMgr::OIWrite_Spec_dft(const ToaMap* pOI, BYTE* pbBuf)
{
	BYTE *pbBuf0 = pbBuf;

	BYTE bMaxPortNum, bPortNum, bPortSn;
	BYTE bType;
	WORD wTotalLen = 0;
	int iRet = -1;
	DWORD dwOMD;

	dwOMD = Search_OMD_from_OAD(pOI->dwOA);
	if (dwOMD != 0)
		iRet = special_write_math(dwOMD, pbBuf);

	return iRet;
}


int CParaMgr::special_write_math(DWORD dwOMD, BYTE *pbData)
{
	WORD wOI1;
	BYTE bMath, bMode;
	BYTE bRes[100];
	int iRet;

	BYTE *pbTmp = pbData;
	int iParaLen = 0;


	wOI1 = dwOMD>>16;
	bMath = dwOMD>>8;
	bMode = dwOMD;

	iRet = DoObjMethod(wOI1, bMath, bMode, pbTmp, &iParaLen, NULL, bRes);

	if ((iRet < 0) || (iRet > sizeof(bRes)))
	{
		DTRACE(DB_CRITICAL, ("CParaMgr::special_write_math : DoObjMethod error. iRet=%d OMD=%04x\r\n", iRet, dwOMD));
		return iRet;
	}

	return iParaLen;
}



//??:?????????????,??????OoPro2AppScan()????????????
//      ????dft?????,dft??OAD????,??????OAD?,???????,
//		???  whr -- 20170419
//??:@wOI	????
//	   @bAtrr	???????? bit-string(SIZE(8))
//	@bIndex???????
//	   @pbBuf	??????????,??????????????
int CParaMgr::OoProWriteAttr_dft(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wLen, bool fIsSecurityLayer)
{
	BYTE bTmpBuf[3000], bType, bPassWord[16];	//??????,?????????????,?????????????.
	BYTE bDbBuf[3000];
	BYTE *pbTmpBuf = bTmpBuf;
	BYTE bPerm = 0x11;
	ToaMap* pOI = NULL;
	WORD wID, wDataLen;
	int iLen = -1;
	int iLen0;
	//int iDataLen = 0;
	//BYTE bOADBuf[4];

	memset(bTmpBuf, 0, sizeof(bTmpBuf));//??????????0?(?????????)???????
	if ((bAttr == 0) || ((bAttr == 1)))	//0??,?????????
	{
		return -1;	//????
	}
	else //????
	{
		DWORD dwOIAtt = ((DWORD )wOI<<16) + ((DWORD )bAttr<<8);
		const ToaMap* pOI = GetOIMap(dwOIAtt);
		if ((wOI>=0xF000) && (wOI<=0xF002))
		{
			iLen = SetFileTransAttr(wOI, bAttr, bIndex, pbBuf);
			return iLen;
		}
		if (pOI == NULL)
			return -1;

		if (pOI->wMode == MAP_VAL || pOI->wMode == MAP_BYTE)	//?????????,?????
			return -1;

		if (bIndex == 0)	//???
		{
			pbTmpBuf = pbBuf;
			wID = pOI->wID;
			int nRet = OoScanData(pbTmpBuf, pOI->pFmt, pOI->wFmtLen, false, -1, &wDataLen, &bType);
			if (nRet > 0)
			{
				if (IsNeedWrSpec(pOI))
				{
					iLen = OIWrite_Spec(pOI, pbTmpBuf);
					goto RET_ATTR;
				}
				else if (IsNeedWrSpec_dft(pOI))
				{
					iLen = OIWrite_Spec_dft(pOI, pbTmpBuf);
					goto RET_ATTR;
				}
				else 
				{
					//????????,?????????????????,????????
					memset(bDbBuf, 0, sizeof(bDbBuf));
					iLen0 = ReadItemEx(BN0, pOI->wPn, pOI->wID, bDbBuf);
					if (iLen0 > 0)	
					{
						int nRdRet = OoScanData(bDbBuf, pOI->pFmt, pOI->wFmtLen, false, -1, &wDataLen, &bType);
						if (nRdRet>0 && nRet==nRdRet && (memcmp(bDbBuf, pbTmpBuf, nRdRet) == 0))
						{
							iLen = nRdRet;
							goto RET_ATTR;
						}
						else
						{
							iLen = nRet;
						}
					}

					if ((dwOIAtt&0xf0000000) == 0x30000000) // ??
					{						
						ReInitEvtPara(dwOIAtt);
						if (dwOIAtt==0x31060900 && pbTmpBuf[1]==1)	//??????????????
						{
							UpdateTermPowerOffTime();
						}
					}
					else if ((dwOIAtt&0xf0000000) == 0x21000300)	//??????,??????
					{
						OnStatParaChg();
					}

					if ((iLen0 = WriteItemEx(BN0, pOI->wPn, pOI->wID, pbTmpBuf, bPerm, bPassWord)) <= 0)
					{
						DTRACE(DB_FAPROTO, ("DlmsWriteAttrToDB: There is something wrong when call WriteItemEx()\n"));
						return -1;	//??????
					}
				}
			}
			else
			{
				DTRACE(DB_FAPROTO, ("OoProWriteAttr: OoDataScan Data failed, nRet=%d !\n", nRet));
				return -1;
			}

			if (iLen == -1)//????????????
				iLen = iLen0;
		}
		else	//???
		{
			int iSrcLen;
			WORD wFmtLen;
			BYTE bSrc[1024];
			BYTE *pbFmt;

			memset(bSrc, 0, sizeof(bSrc));
			iSrcLen = OoReadAttr(wOI, bAttr, bSrc, &pbFmt, &wFmtLen);
			if (iSrcLen > 0)
			{
				//	OoDWordToOad(GetOAD(wOI, bAttr, bIndex), bOADBuf);
				//	iDataLen = OoGetDataLen(DT_OAD, bOADBuf);	//+1:??????
				//	if (!(iDataLen >= wLen))
				//		wLen = iDataLen;	
				//wLen??????????,???4??OAD,??-4,-5??0,-1???????? change by whr 20170328
				if (fIsSecurityLayer)
					iSrcLen = OoWriteField(bSrc, iSrcLen, pbFmt, wFmtLen, bIndex-1, pbBuf, wLen); //?sam????????
				else
				{
					if (wLen>0)
						iSrcLen = OoWriteField(bSrc, iSrcLen, pbFmt, wFmtLen, bIndex-1, pbBuf, wLen-1); //1??????
					else
						iSrcLen = 0;
				}
				if (iSrcLen > 0)
					return OoProWriteAttr(wOI, bAttr, 0, bSrc, iSrcLen, fIsSecurityLayer);
			}
		}

	
RET_ATTR:
		//OutBeepMs(50);
		TrigerSave();
		return iLen;
	}

	return -1;
}


bool CParaMgr::IsNeedWrSpec_dft(const ToaMap* pOI)
{

	DWORD dwOMD;
	bool fRet;

	dwOMD = Search_OMD_from_OAD(pOI->dwOA);
	if (dwOMD != 0)
		fRet = true;
	else
		fRet = false;

	return fRet;
}
