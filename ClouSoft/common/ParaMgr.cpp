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

int GprsCommCfgParaToOob(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[128];
	DWORD dwTmp = 0;

//		//4500 公网通信模块1——通讯配置
//		DT_STRUCT, 0x0c,
//			DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
//			DT_ENUM, 0x00,	//永久在线（0），被动激活（1）
//			DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
//			DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
//			DT_ARRAY, 0x01,	//帧听端口列表
//				DT_LONG_U, 0x11, 0x22, 
//			DT_VIS_STR, 0x05,		//APN
//				'c', 'm', 'n',  'e', 't',
//			DT_VIS_STR, 0x04,		///用户名
//				'c', 'a', 'r', 'd',	
//			DT_VIS_STR, 0x04,		///密码
//				'c', 'a', 'r', 'd',	
//			DT_OCT_STR, 4,	//代理服务器地址
//				 0xC0, 0x00, 0x00, 0x01,
//			DT_LONG_U,	//代理端口
//				0x88, 0x88,
//			DT_UNSIGN,	//超时时间及重发次数
//				0x7B,	//超时时间30S，重试次数3次 	
//			DT_LONG_U,	//心跳周期
//				0x01, 0x2C,	//300s
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00,
	*p++ = DT_STRUCT;
	*p++ = 0x0c;
	iLen += 2;
	
	ReadItemEx(BN0, PN0, 0x008f, bBuf);
	//TraceBuf(DB_FAPROTO, "To OOB Read 0x008f==========>>>>>", bBuf, 3);
	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
	*p++ = DT_ENUM;
	*p++ = (bBuf[0]>>4) & 0x03;
	iLen += 2;
	//永久在线（0），被动激活（1）
	*p++ = DT_ENUM;
	if((bBuf[0] & 0x03) == 1)
	{
		*p++ = 0;
	}
	else
	{
		*p++ = 1;
	}
	iLen += 2;
	//连接方式 TCP（0），UDP（1）
	*p++ = DT_ENUM;
	*p++ = bBuf[0] >> 7;
	iLen += 2;
	//连接应用方式 主备模式（0），多连接模式（1）
	*p++ = DT_ENUM;
	*p++ = 0x00;
	iLen += 2;
	ReadItemEx(BN0, PN0, 0x007f, bBuf);
	//TraceBuf(DB_FAPROTO, "To OOB Read 0x007f==========>>>>>", bBuf, 64);
	//帧听端口列表
	*p++ = DT_ARRAY;
	*p++ = 0x01;
	*p++ = DT_LONG_U;
	*p++ = bBuf[23];//面向对象要翻转
	*p++ = bBuf[22];
	iLen += 5;
	ReadItemEx(BN0, PN0, 0x003f, bBuf);
	//APN
	*p++ = DT_VIS_STR;
	*p++ = 0x10;
	memcpy(p, &bBuf[12], 16);
	p += 16;
	iLen += 18;

	ReadItemEx(BN0, PN0, 0x010f, bBuf);
	///用户名
	*p++ = DT_VIS_STR;
	*p++ = 0x20;
	memcpy(p, &bBuf[0], 32);
	p += 32;
	iLen += 34;
	///密码
	*p++ = DT_VIS_STR;
	*p++ = 0x20;
	memcpy(p, &bBuf[32], 32);
	p += 32;
	iLen += 34;
	ReadItemEx(BN0, PN0, 0x007f, bBuf);
	//代理服务器地址
	*p++ = DT_OCT_STR;
	*p++ = 4;
	memcpy(p, &bBuf[13], 4);
	p += 4;
	iLen += 6;
	//代理端口
	*p++ = DT_LONG_U;
	memrcpy(p, &bBuf[17], 2);
	//TraceBuf(DB_CRITICAL,"\r\n GprsCommCfgParaToOob: 1->",&bBuf[17],2);
	p += 2;
	iLen += 3;
	ReadItemEx(BN0, PN0, 0x001f, bBuf);
	//超时时间及重发次数
	dwTmp = bBuf[3]&0x0f;
	dwTmp <<= 8;
	dwTmp += bBuf[2];
	if(dwTmp>=64)
	{
		dwTmp = 63;
	}
	dwTmp <<= 2;
	dwTmp += ((bBuf[3]&0x30)>>4);
	*p++ = DT_UNSIGN;
	*p++ = (BYTE)dwTmp;
	iLen += 2;
	//心跳周期
	dwTmp = bBuf[5];
	dwTmp *= 60;//转换为秒
	*p++ = DT_LONG_U;
	memrcpy(p, (BYTE*)&dwTmp, 2);
	p += 2;
	iLen += 3;
	//TraceBuf(DB_FAPROTO, "To OOB pBuf==========>>>>>", pBuf, iLen);
	
	return iLen;
}

int GprsMastCommParaToOob(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[128];

//	//4501 公网通信模块1——主站通信参数表
//	DT_ARRAY, 2,
//		DT_STRUCT, 0x02,
//			DT_OCT_STR, 4, 
//				0x3A, 0xFB, 0x4A, 0x65, //58.251.74.101
//			DT_LONG_U, 
//				0x19, 0x22, //6434
//		DT_STRUCT, 0x02,
//			DT_OCT_STR, 4, 
//				0x00, 0x00, 0x00, 0x00,
//			DT_LONG_U, 
//				0x00, 0x00,
	*p++ = DT_ARRAY;
	*p++ = 0x02;
	iLen += 2;
	
	ReadItemEx(BN0, PN0, 0x003f, bBuf);
	
	//TraceBuf(DB_CRITICAL,"\r\n GprsMastCommParaToOob:1->",bBuf, 12);
	*p++ = DT_STRUCT;
	*p++ = 0x02;
	iLen += 2;
	*p++ = DT_OCT_STR;
	*p++ = 4;
	//memcpy(p, &bBuf[0], 4);
	p += 4;
	iLen += 6;
	*p++ = DT_LONG_U;
	//memrcpy(p, &bBuf[4], 2);
	p += 2;
	iLen += 3;
	
	*p++ = DT_STRUCT;
	*p++ = 0x02;
	iLen += 2;
	*p++ = DT_OCT_STR;
	*p++ = 4;
	//memcpy(p, &bBuf[6], 4);
	p += 4;
	iLen += 6;
	*p++ = DT_LONG_U;
	//memrcpy(p, &bBuf[10], 2);
	p += 2;
	iLen += 3;	
	
	return iLen;
}

int SmsCommParaToOob(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[128];

//	//4502 公网通信模块1——短信通信参数
//		DT_STRUCT, 0x03,
//			DT_VIS_STR, 8,	//短信中心号码
//				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			DT_ARRAY, 0x01,
//				DT_VIS_STR, 8,	//主站号码
//					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			DT_ARRAY, 0x01, 
//				DT_VIS_STR, 8,	//短信通知目的号码
//					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	ReadItemEx(BN0, PN0, 0x004f, bBuf);
	
	*p++ = DT_STRUCT;
	*p++ = 0x03;
	iLen += 2;
	//短信中心号码
	*p++ = DT_VIS_STR;
	*p++ = 8;
	memcpy(p, &bBuf[8], 8);
	p += 8;
	iLen += 10;
	//主站号码
	*p++ = DT_ARRAY;
	*p++= 0x01;
	iLen += 2;
	*p++ = DT_VIS_STR;
	*p++ = 8;
	memcpy(p, &bBuf[0], 8);
	p += 8;
	iLen += 10;
	//短信通知目的号码
	//先用主站号码填充
	*p++ = DT_ARRAY;
	*p++= 0x01;
	iLen += 2;
	*p++ = DT_VIS_STR;
	*p++ = 8;
	memcpy(p, &bBuf[0], 8);
	p += 8;
	iLen += 10;
	
	return iLen;
}

//	//以太网通信参数
//	BYTE g_bEthCommCfg[] = {DT_STRUCT, 0x08, 
//								DT_ENUM, 		//工作模式
//								DT_ENUM, 	//连接方式
//								DT_ENUM, 	//连接应用方式
//								DT_ARRAY, 8, //帧听端口列表
//									DT_LONG_U,
//								DT_OCT_STR, 4,	RLF,//代理服务器地址
//								DT_LONG_U, 		//代理端口
//								DT_UNSIGN,  //超时时间及重发次数
//								DT_LONG_U, 		//心跳周期
//	};
//	
//	//以太网网络配置
//	BYTE g_bEthNetCfg[] = {DT_STRUCT, 0x06,
//								DT_ENUM,	//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
//								DT_OCT_STR, 0x08, RLV,	//IP地址
//								DT_OCT_STR, 0x08, RLV,	//子网掩码
//								DT_OCT_STR, 0x08, RLV,	//网关地址
//								DT_VIS_STR, 0x20, RLV,	//PPPoE用户名
//								DT_VIS_STR, 0x20, RLV,	//PPPoE密码
//	};
//	
//	//MAC地址
//	BYTE g_bEthMacCfg[] = {DT_MAC, 0x06, RLV};	

int EthCommParaToOob(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[128];
	DWORD dwTmp = 0;

//		//0x4510~0x4517 以太网模块属性2——通信配置
//		DT_STRUCT, 0x08,
//			DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
//			DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
//			DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
//			DT_ARRAY, 0x01,	//帧听端口列表
//				DT_LONG_U, 0x24, 0x54,	//9300
//			DT_OCT_STR, 4,	//代理服务器地址
//				0xC0, 0x00, 0x00, 0x01,
//			DT_LONG_U,	//代理端口
//				0x88, 0x88,
//			DT_UNSIGN,	//超时时间及重发次数
//				0x7B,	//超时时间30S，重试次数3次 	
//			DT_LONG_U,	//心跳周期
//				0x01, 0x2c,	//300s
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
//		0x00,


	*p++ = DT_STRUCT;
	*p++ = 0x08;
	iLen += 2;
	
	ReadItemEx(BN0, PN0, 0x008f, bBuf);
	//TraceBuf(DB_FAPROTO, "To OOB Read 0x008f==========>>>>>", bBuf, 3);
	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
	*p++ = DT_ENUM;
	*p++ = (bBuf[0]>>4) & 0x03;
	iLen += 2;
	//连接方式 TCP（0），UDP（1）
	*p++ = DT_ENUM;
	*p++ = bBuf[0] >> 7;
	iLen += 2;
	//连接应用方式 主备模式（0），多连接模式（1）
	*p++ = DT_ENUM;
	*p++ = 0x00;
	iLen += 2;
	
	ReadItemEx(BN0, PN0, 0x007f, bBuf);
	//TraceBuf(DB_FAPROTO, "To OOB Read 0x007f==========>>>>>", bBuf, 32);
	//帧听端口列表
	*p++ = DT_ARRAY;
	*p++ = 0x01;
	*p++ = DT_LONG_U;
	*p++ = bBuf[23];//面向对象要翻转
	*p++ = bBuf[22];
	iLen += 5;
	//代理服务器地址
	*p++ = DT_OCT_STR;
	*p++ = 4;
	memcpy(p, &bBuf[13], 4);
	p += 4;
	iLen += 6;
	//代理端口
	*p++ = DT_LONG_U;
	memrcpy(p, &bBuf[17], 2);
	p += 2;
	iLen += 3;

	ReadItemEx(BN0, PN0, 0x001f, bBuf);
	//超时时间及重发次数
	dwTmp = bBuf[3]&0x0f;
	dwTmp <<= 8;
	dwTmp += bBuf[2];
	if(dwTmp>=64)
	{
		dwTmp = 63;
	}
	dwTmp <<= 2;
	dwTmp += ((bBuf[3]&0x30)>>4);
	*p++ = DT_UNSIGN;
	*p++ = (BYTE)dwTmp;
	iLen += 2;
	//心跳周期
	dwTmp = bBuf[5];
	dwTmp *= 60;//转换为秒
	*p++ = DT_LONG_U;
	memrcpy(p, (BYTE*)&dwTmp, 2);
	p += 2;
	iLen += 3;
	//TraceBuf(DB_FAPROTO, "To OOB pBuf==========>>>>>", pBuf, iLen);
	return iLen;
}

int EthMastCommParaToOob(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[128];

//	//4501 公网通信模块1——主站通信参数表
//	DT_ARRAY, 2,
//		DT_STRUCT, 0x02,
//			DT_OCT_STR, 4, 
//				0x3A, 0xFB, 0x4A, 0x65, //58.251.74.101
//			DT_LONG_U, 
//				0x19, 0x22, //6434
//		DT_STRUCT, 0x02,
//			DT_OCT_STR, 4, 
//				0x00, 0x00, 0x00, 0x00,
//			DT_LONG_U, 
//				0x00, 0x00,
	*p++ = DT_ARRAY;
	*p++ = 0x02;
	iLen += 2;
	
	ReadItemEx(BN0, PN0, 0x003f, bBuf);
	
	*p++ = DT_STRUCT;
	*p++ = 0x02;
	iLen += 2;
	*p++ = DT_OCT_STR;
	*p++ = 4;
	//memcpy(p, &bBuf[0], 4);
	p += 4;
	iLen += 6;
	*p++ = DT_LONG_U;
	//memrcpy(p, &bBuf[4], 2);
	p += 2;
	iLen += 3;
	
	*p++ = DT_STRUCT;
	*p++ = 0x02;
	iLen += 2;
	*p++ = DT_OCT_STR;
	*p++ = 4;
	//memcpy(p, &bBuf[6], 4);
	p += 4;
	iLen += 6;
	*p++ = DT_LONG_U;
	//memrcpy(p, &bBuf[10], 2);
	p += 2;
	iLen += 3;	
	
	return iLen;
}

int EthNetParaToOob(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bNetConType = 0;
	BYTE bBuf[128];

//	//0x4510~0x4517 以太网模块属性4——网络配置
//	DT_STRUCT, 0x06,
//		DT_ENUM, 0x01,	//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
//		DT_OCT_STR, 0x04,	//IP地址	192.168.1.200
//			0xC0, 0xA8, 0x01, 0xC8, 
//		DT_OCT_STR, 0x04,	//子网掩码	255.255.255.0
//			0xFF, 0xFF, 0xFF, 0x00,
//		DT_OCT_STR, 0x04,	//网关地址	192.168.1.1
//			0xC0, 0xA8, 0x01, 0x01, 
//		DT_VIS_STR, 0x04,	//PPPoE用户名
//			'T',	'E',   'S', 'T',   
//		DT_VIS_STR, 0x04,	//PPPoE密码
//			'T',	'E',   'S', 'T',   

	*p++ = DT_STRUCT;
	*p++ = 0x06;
	iLen += 2;
	ReadItemEx(BN0, PN0, 0x007f, bBuf); 
	ReadItemEx(BN10, PN0, 0xa1b6, &bNetConType); //以太网连接方式0：自动获取IP，1：pppoe拨号
	//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
	*p++ = DT_ENUM;
	if (bNetConType == 1)
	{
		*p++ = 2;
	}
	else
	{
		if (!IsAllAByte(&bBuf[8], 0x00, 4))
			*p++ = 1;
		else
			*p++ = 0;
	}
	iLen += 2;
	//IP地址	192.168.1.200
	*p++ = DT_OCT_STR;
	*p++ = 4;
	memcpy(p, &bBuf[0], 4);
	p += 4;
	iLen += 6;
	//子网掩码	255.255.255.0
	*p++ = DT_OCT_STR;
	*p++ = 4;
	memcpy(p, &bBuf[4], 4);
	p += 4;
	iLen += 6;
	//网关地址	192.168.1.1
	*p++ = DT_OCT_STR;
	*p++ = 4;
	memcpy(p, &bBuf[8], 4);
	p += 4;
	iLen += 6;

	ReadItemEx(BN0, PN0, 0x010f, bBuf);
	///用户名
	*p++ = DT_VIS_STR;
	*p++ = 0x20;
	memcpy(p, &bBuf[0], 32);
	p += 32;
	iLen += 34;
	///密码
	*p++ = DT_VIS_STR;
	*p++ = 0x20;
	memcpy(p, &bBuf[32], 32);
	p += 32;
	iLen += 34;
	
	return iLen;
}

int EthMacParaToOob(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[128];

//	//0x4510~0x4517 以太网模块属性5——MAC地址
//	DT_MAC, 0x06,
//		0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
	ReadItemEx(BN10, PN0, 0xa150, bBuf);
	*p++ = DT_MAC;
	*p++ = 0x06;
	iLen += 2;
	memcpy(p, &bBuf[0], 6);
	p += 6;
	iLen += 6;	
	
	return iLen;
}

int TermAddrToOob(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[128], bBuf2[4];
	DWORD dwTmp = 0;

	//0x4001， 通讯地址，默认01--8byte
	//DT_OCT_STR,0x06,
	//	0x11,0x22,0x33,0x44,0x55,0x66,
	//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	memset(pBuf, 0, 8);
	ReadItemEx(BN0, PN0, 0x232f, bBuf);
	*p++ = DT_OCT_STR;
	*p++ = 0x06;
	iLen += 2;
	*p++ = bBuf[0];
	*p++ = bBuf[1];
	memcpy((BYTE *)&dwTmp, &bBuf[2], 2);
	DWORDToBCD(dwTmp, bBuf2, 4);
	memrcpy(p, bBuf2, 4);
	iLen += 6;
	
	return iLen;
}

TOobFmtMap g_OobParaTransTbl[] = 
{
	// OA----------格式描述串----格式描述串长度----格式转换函数
	//公网通信模块1
	{0x45000200,	0x4500,	BANK0,	PN0,	/*g_bGprsCommCfg,		sizeof(g_bGprsCommCfg),*/				GprsCommCfgParaToOob},	//通讯配置
	{0x45000300,	0x4501,	BANK0,	PN0,	/*g_bMastCommPara,	sizeof(g_bMastCommPara),*/			GprsMastCommParaToOob},	//主站通信参数表
	{0x45000400,	0x4502,	BANK0,	PN0,	/*g_bSmsCommPara,		sizeof(g_bSmsCommPara),*/				SmsCommParaToOob},	//短信通信参数
	{0x45000500,	0x4503,	BANK0,	PN0,	/*g_bEleVerInfo,		sizeof(g_bEleVerInfo),*/				NULL},	//版本信息 		不用转换
	{0x45000700,	0x4505,	BANK0,	PN0,	/*g_bSimCCID,			sizeof(g_bSimCCID),	*/				NULL},	//SIM卡ICCID 	不用转换
	{0x45000800,	0x4506,	BANK0,	PN0,	/*g_bIMSI,			sizeof(g_bIMSI),*/					NULL},	//IMSI			不用转换
	{0x45000900,	0x4507,	BANK0,	PN0,	/*g_bSigStrenth,		sizeof(g_bSigStrenth),*/				NULL},	//信号强度		不用转换
	{0x45000A00,	0x4508,	BANK0,	PN0,	/*g_bSimNo,			sizeof(g_bSimNo),*/					NULL},	//SIM卡号码		不用转换
	{0x45000B00,	0x4509,	BANK0,	PN0,	/*g_DialIp,			sizeof(g_DialIp),*/					NULL},	//拨号IP		不用转换
	//0x4510以太网通信模块1
	{0x45100200,	0x4510,	BANK0,	PN0,	/*g_bEthCommCfg,		sizeof(g_bEthCommCfg),*/				EthCommParaToOob},	//通讯配置
	{0x45100300,	0x4511,	BANK0,	PN0,	/*g_bMastCommPara,	sizeof(g_bMastCommPara),*/			EthMastCommParaToOob},	//主站通信参数表
	{0x45100400,	0x4512,	BANK0,	PN0,	/*g_bEthNetCfg,		sizeof(g_bEthNetCfg),*/				EthNetParaToOob},	//网络配置参数
	{0x45100500,	0x4513,	BANK0,	PN0,	/*g_bEthMacCfg,		sizeof(g_bEthMacCfg),*/				EthMacParaToOob},	//MAC地址
	{0x40010200,	0x4001, BANK0,	PN0,	/*g_bEthMacCfg, 	sizeof(g_bEthMacCfg),*/ 			TermAddrToOob},	//终端地址
};
#define OOB_CONVERT_NUM sizeof(g_OobParaTransTbl)/sizeof(TOobFmtMap)

int TermCommCfgParaTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256];
	BYTE *pDat = bBuf;
	DWORD dwTmp = 0;
	
//		//4500 公网通信模块1——通讯配置
//		DT_STRUCT, 0x0c,
//			DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
//			DT_ENUM, 0x00,	//永久在线（0），被动激活（1）
//			DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
//			DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
//			DT_ARRAY, 0x01,	//帧听端口列表
//				DT_LONG_U, 0x11, 0x22, 
//			DT_VIS_STR, 0x05,		//APN
//				'c', 'm', 'n',  'e', 't',
//			DT_VIS_STR, 0x04,		///用户名
//				'c', 'a', 'r', 'd',	
//			DT_VIS_STR, 0x04,		///密码
//				'c', 'a', 'r', 'd',	
//			DT_OCT_STR, 4,	//代理服务器地址
//				 0xC0, 0x00, 0x00, 0x01,
//			DT_LONG_U,	//代理端口
//				0x88, 0x88,
//			DT_UNSIGN,	//超时时间及重发次数
//				0x7B,	//超时时间30S，重试次数3次 	
//			DT_LONG_U,	//心跳周期
//				0x01, 0x2C,	//300s
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00,
	*p = 10;	//终端数传机延时时间RTS
	iLen += 1;
	*(p+1) = 3;	 //终端作为启动站允许发送传输延时时间
	iLen += 1;
	ReadItemEx(BN0, PN0, 0x4500, pDat);
	//TraceBuf(DB_FAPROTO, "To 13761 Read 0x4500==========>>>>>", pDat, 32);
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x0c;	
	pDat++;//= DT_ENUM;
	pDat++;//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
	pDat++;//= DT_ENUM;
	pDat++;//永久在线（0），被动激活（1）
	pDat++;//= DT_ENUM;
	pDat++;//连接方式 TCP（0），UDP（1）
	pDat++;//= DT_ENUM;
	pDat++;//连接应用方式 主备模式（0），多连接模式（1）
	pDat++;//DT_ARRAY, 
	dwTmp = *pDat++;//帧听端口列表
	dwTmp *= 3;
	pDat += dwTmp;
	pDat++;//DT_VIS_STR, 
	dwTmp = *pDat++;//APN
	pDat += dwTmp;
	pDat++;//DT_VIS_STR, 
	dwTmp = *pDat++;//用户名
	pDat += dwTmp;
	pDat++;//DT_VIS_STR, 
	dwTmp = *pDat++;//密码
	pDat += dwTmp;
	pDat += 6;//代理服务器地址
	pDat += 3;//代理端口
	pDat += 1;//DT_UNSIGN
	dwTmp = *pDat>>2;//超时时间
	dwTmp |= ((*pDat&0x03)<<12);//重发次数
	pDat++;
	memcpy(p+2, (BYTE*)&dwTmp, 2);
	iLen += 2;

	*(p+4) = 3;	 //需要主站确认的通信服务（CON=1）的标志
	iLen += 1;
	
	pDat += 1;//DT_UNSIGN
	dwTmp = 0;
	memrcpy((BYTE*)&dwTmp, pDat, 2);//心跳周期
	dwTmp /= 60;//转成分钟
	if(dwTmp>255)
	{
		dwTmp = 255;
	}
	*(p+5) = (BYTE)dwTmp;//
	iLen += 1;	
	//TraceBuf(DB_FAPROTO, "To 13761 pBuf==========>>>>>", pBuf, iLen);
	return iLen;
}

int GprsCommCfgParaTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256];
	BYTE *pDat = bBuf;

//		//4500 公网通信模块1——通讯配置
//		DT_STRUCT, 0x0c,
//			DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
//			DT_ENUM, 0x00,	//永久在线（0），被动激活（1）
//			DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
//			DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
//			DT_ARRAY, 0x01,	//帧听端口列表
//				DT_LONG_U, 0x11, 0x22, 
//			DT_VIS_STR, 0x05,		//APN
//				'c', 'm', 'n',  'e', 't',
//			DT_VIS_STR, 0x04,		///用户名
//				'c', 'a', 'r', 'd',	
//			DT_VIS_STR, 0x04,		///密码
//				'c', 'a', 'r', 'd',	
//			DT_OCT_STR, 4,	//代理服务器地址
//				 0xC0, 0x00, 0x00, 0x01,
//			DT_LONG_U,	//代理端口
//				0x88, 0x88,
//			DT_UNSIGN,	//超时时间及重发次数
//				0x7B,	//超时时间30S，重试次数3次 	
//			DT_LONG_U,	//心跳周期
//				0x01, 0x2C,	//300s
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00,
	ReadItemEx(BN0, PN0, 0x4500, pDat);
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x0c;
	
	/*//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
	pDat++;//= DT_ENUM;
	*(p+1) = *pDat++;
	iLen += 1;
	//永久在线（0），被动激活（1）
	pDat++;//= DT_ENUM;
	*(p+0) = *pDat++;
	iLen += 1;
	//连接方式 TCP（0），UDP（1）
	pDat++;// = DT_ENUM;
	*(p+2) = *pDat++;
	iLen += 1;
	//永久在线、时段在线模式重拨间隔
	*(p+3) = 0;
	*(p+4) = 0;
	iLen += 2;
	//被动激活模式重拨次数
	*(p+5) = 0;
	iLen += 1;
	//被动激活模式连续无通信自动断线时间
	*(p+6) = 0;
	iLen += 1;
	//时段在线模式允许在线时段标志
	*(p+7) = 0;
	*(p+8) = 0;
	*(p+9) = 0;
	iLen += 3;	*/
	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
	pDat++;//= DT_ENUM;
	*p = ((*pDat)++) << 4;
	//永久在线（0），被动激活（1）
	pDat++;//= DT_ENUM;
	*p |= (*pDat++) & 0x03;
	//连接方式 TCP（0），UDP（1）
	pDat++;// = DT_ENUM;
	*p = (*pDat++) << 7;
	iLen += 1;
	//永久在线、时段在线模式重拨间隔
	*(p+1) = 0;
	*(p+2) = 0;
	iLen += 2;
	//被动激活模式重拨次数
	*(p+3) = 0;
	iLen += 1;
	//被动激活模式连续无通信自动断线时间
	*(p+4) = 0;
	iLen += 1;
	//时段在线模式允许在线时段标志
	*(p+5) = 0;
	*(p+6) = 0;
	*(p+7) = 0;
	iLen += 3;	
	
	return iLen;
}

int GprsMastCommParaTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256];
	BYTE *pDat = bBuf;
	DWORD dwTmp = 0;

//	//4501 公网通信模块1——主站通信参数表
//	DT_ARRAY, 2,
//		DT_STRUCT, 0x02,
//			DT_OCT_STR, 4, 
//				0x3A, 0xFB, 0x4A, 0x65, //58.251.74.101
//			DT_LONG_U, 
//				0x19, 0x22, //6434
//		DT_STRUCT, 0x02,
//			DT_OCT_STR, 4, 
//				0x00, 0x00, 0x00, 0x00,
//			DT_LONG_U, 
//				0x00, 0x00,
	ReadItemEx(BN0, PN0, 0x4501, pDat);
	pDat++;// = DT_ARRAY;
	pDat++;// = 0x02;
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x02;
	pDat++;// = DT_OCT_STR;
	pDat++;// = 4;
	//memcpy(p, pDat, 4);
	pDat += 4;
	iLen += 4;
	*pDat++;// = DT_LONG_U;
	//memrcpy(p+4, pDat, 2);
	pDat += 2;
	iLen += 2;
	
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x02;
	pDat++;// = DT_OCT_STR;
	pDat++;// = 4;
	//memcpy(p+6, pDat, 4);
	pDat += 4;
	iLen += 4;
	pDat++;// = DT_LONG_U;
	//memrcpy(p+10, pDat, 2);
	pDat += 2;
	iLen += 2;

	ReadItemEx(BN0, PN0, 0x4500, pDat);
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x0c;	
	pDat++;//= DT_ENUM;
	pDat++;//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
	pDat++;//= DT_ENUM;
	pDat++;//永久在线（0），被动激活（1）
	pDat++;//= DT_ENUM;
	pDat++;//连接方式 TCP（0），UDP（1）
	pDat++;//= DT_ENUM;
	pDat++;//连接应用方式 主备模式（0），多连接模式（1）
	pDat++;//DT_ARRAY, 
	dwTmp = *pDat++;//帧听端口列表
	dwTmp *= 3;
	pDat += dwTmp;
	pDat++;//DT_VIS_STR, 
	dwTmp = *pDat++;//APN
	if(dwTmp>16)
	{
		dwTmp = 16;
	}
	memcpy(p+12, pDat, dwTmp);
	memset(p+12+dwTmp, 0, 16-dwTmp);
	pDat += dwTmp;
	iLen += 16;//固定16字节
	
	return iLen;
}

int SmsCommParaTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256];
	BYTE *pDat = bBuf;

//	//4502 公网通信模块1——短信通信参数
//		DT_STRUCT, 0x03,
//			DT_VIS_STR, 8,	//短信中心号码
//				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			DT_ARRAY, 0x01,
//				DT_VIS_STR, 8,	//主站号码
//					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//			DT_ARRAY, 0x01, 
//				DT_VIS_STR, 8,	//短信通知目的号码
//					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	ReadItemEx(BN0, PN0, 0x4502, bBuf);
	
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x03;
	//短信中心号码
	pDat++;// = DT_VIS_STR;
	pDat++;// = 8;
	memcpy(p+8, pDat, 8);
	pDat += 8;
	iLen += 8;
	//主站号码
	pDat++;// = DT_ARRAY;
	pDat++;// = 0x01;
	pDat++;// = DT_VIS_STR;
	pDat++;// = 8;
	memcpy(p, pDat, 8);
	iLen += 8;
	
	return iLen;
}

int TermIpParaTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256];
	BYTE *pDat = bBuf;
	DWORD dwTmp =0;
//	//0x4510~0x4517 以太网模块属性4——网络配置
//	DT_STRUCT, 0x06,
//		DT_ENUM, 0x01,	//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
//		DT_OCT_STR, 0x04,	//IP地址	192.168.1.200
//			0xC0, 0xA8, 0x01, 0xC8, 
//		DT_OCT_STR, 0x04,	//子网掩码	255.255.255.0
//			0xFF, 0xFF, 0xFF, 0x00,
//		DT_OCT_STR, 0x04,	//网关地址	192.168.1.1
//			0xC0, 0xA8, 0x01, 0x01, 
//		DT_VIS_STR, 0x04,	//PPPoE用户名
//			'T',	'E',   'S', 'T',   
//		DT_VIS_STR, 0x04,	//PPPoE密码
//			'T',	'E',   'S', 'T',   

	ReadItemEx(BN0, PN0, 0x4512, bBuf);
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x06;
	pDat++;// = DT_ENUM;
	pDat++;//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
	//IP地址	192.168.1.200
	pDat++;// = DT_OCT_STR;
	pDat++;// = 4;
	memcpy(p, pDat, 4);
	pDat += 4;
	iLen += 4;
	//子网掩码	255.255.255.0
	pDat++;// = DT_OCT_STR;
	pDat++;// = 4;
	memcpy(p+4, pDat, 4);
	pDat += 4;
	iLen += 4;
	//网关地址	192.168.1.1
	pDat++;// = DT_OCT_STR;
	pDat++;// = 4;
	memcpy(p+8, pDat, 4);
	pDat += 4;
	iLen += 4;
	
	*(p+12) = 0;//代理类型
	iLen += 1;
//		//0x4510~0x4517 以太网模块属性2——通信配置
//		DT_STRUCT, 0x08,
//			DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
//			DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
//			DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
//			DT_ARRAY, 0x01, //帧听端口列表
//				DT_LONG_U, 0x24, 0x54,	//9300
//			DT_OCT_STR, 4,	//代理服务器地址
//				0xC0, 0x00, 0x00, 0x01,
//			DT_LONG_U,	//代理端口
//				0x88, 0x88,
//			DT_UNSIGN,	//超时时间及重发次数
//				0x7B,	//超时时间30S，重试次数3次	
//			DT_LONG_U,	//心跳周期
//				0x01, 0x2c, //300s
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//		0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
//		0x00,
	pDat = bBuf;
	ReadItemEx(BN0, PN0, 0x4512, pDat);
	pDat += 9;
	dwTmp = *pDat;
	dwTmp *= 3;//DT_ARRAY, 0x01, //帧听端口列表
	pDat += dwTmp;
	pDat += 2;//DT_OCT_STR, 4,	//代理服务器地址
	memcpy(p+13, pDat, 4);
	pDat += 4;
	iLen += 4;
	pDat++; //DT_LONG_U //代理端口
	memrcpy(p+17, pDat, 2);
	pDat += 2;
	iLen += 2;

	*(p+19) = 0;//代理服务器连接方式
	iLen += 1;
	*(p+20) = 0;//用户名长度m
	iLen += 1;
	memset(p+21, 0, 6);//用户名
	iLen += 6;
	*(p+27) = 0;//密码长度m
	iLen += 1;
	memset(p+28, 0, 3);//密码
	iLen += 3;
	//终端侦听端口
	pDat = bBuf;
	ReadItemEx(BN0, PN0, 0x4512, pDat);
	pDat += 10;//DT_ARRAY, 0x01, //帧听端口列表
	pDat++;//DT_LONG_U
	memrcpy(p+31, pDat, 2);
	iLen += 2;
	return iLen;
}

int TermUpComParaTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256];
	BYTE *pDat = bBuf;

//		//4500 公网通信模块1——通讯配置
//		DT_STRUCT, 0x0c,
//			DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
//			DT_ENUM, 0x00,	//永久在线（0），被动激活（1）
//			DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
//			DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
//			DT_ARRAY, 0x01, //帧听端口列表
//				DT_LONG_U, 0x11, 0x22, 
//			DT_VIS_STR, 0x05,		//APN
//				'c', 'm', 'n',	'e', 't',
//			DT_VIS_STR, 0x04,		///用户名
//				'c', 'a', 'r', 'd', 
//			DT_VIS_STR, 0x04,		///密码
//				'c', 'a', 'r', 'd', 
//			DT_OCT_STR, 4,	//代理服务器地址
//				 0xC0, 0x00, 0x00, 0x01,
//			DT_LONG_U,	//代理端口
//				0x88, 0x88,
//			DT_UNSIGN,	//超时时间及重发次数
//				0x7B,	//超时时间30S，重试次数3次	
//			DT_LONG_U,	//心跳周期
//				0x01, 0x2C, //300s
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00,
	ReadItemEx(BN0, PN0, 0x4500, pDat);
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x0c;
	
	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
	pDat++;//= DT_ENUM;
	*p = ((*pDat)++) << 4;
	//永久在线（0），被动激活（1）
	pDat++;//= DT_ENUM;
	*p |= (*pDat++) & 0x03;
	//连接方式 TCP（0），UDP（1）
	pDat++;// = DT_ENUM;
	*p = (*pDat++) << 7;
	iLen += 1;
	//永久在线、时段在线模式重拨间隔
	*(p+1) = 0;
	*(p+2) = 0;
	iLen += 2;
	//被动激活模式重拨次数
	*(p+3) = 0;
	iLen += 1;
	//被动激活模式连续无通信自动断线时间
	*(p+4) = 0;
	iLen += 1;
	//时段在线模式允许在线时段标志
	*(p+5) = 0;
	*(p+6) = 0;
	*(p+7) = 0;
	iLen += 3;	
	
	return iLen;
}

int VPNParaTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256];
	BYTE *pDat = bBuf;
	DWORD dwTmp = 0;
	
//		//4500 公网通信模块1——通讯配置
//		DT_STRUCT, 0x0c,
//			DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
//			DT_ENUM, 0x00,	//永久在线（0），被动激活（1）
//			DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
//			DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
//			DT_ARRAY, 0x01, //帧听端口列表
//				DT_LONG_U, 0x11, 0x22, 
//			DT_VIS_STR, 0x05,		//APN
//				'c', 'm', 'n',	'e', 't',
//			DT_VIS_STR, 0x04,		///用户名
//				'c', 'a', 'r', 'd', 
//			DT_VIS_STR, 0x04,		///密码
//				'c', 'a', 'r', 'd', 
//			DT_OCT_STR, 4,	//代理服务器地址
//				 0xC0, 0x00, 0x00, 0x01,
//			DT_LONG_U,	//代理端口
//				0x88, 0x88,
//			DT_UNSIGN,	//超时时间及重发次数
//				0x7B,	//超时时间30S，重试次数3次	
//			DT_LONG_U,	//心跳周期
//				0x01, 0x2C, //300s
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//			0x00,
	ReadItemEx(BN0, PN0, 0x4500, pDat);
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x0c;	
	pDat++;//= DT_ENUM;
	pDat++;//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
	pDat++;//= DT_ENUM;
	pDat++;//永久在线（0），被动激活（1）
	pDat++;//= DT_ENUM;
	pDat++;//连接方式 TCP（0），UDP（1）
	pDat++;//= DT_ENUM;
	pDat++;//连接应用方式 主备模式（0），多连接模式（1）
	pDat++;//DT_ARRAY, 
	dwTmp = *pDat++;//帧听端口列表
	dwTmp *= 3;
	pDat += dwTmp;
	pDat++;//DT_VIS_STR, 
	dwTmp = *pDat++;//APN
	pDat += dwTmp;
	pDat++;//DT_VIS_STR, 
	dwTmp = *pDat++;//用户名
	if(dwTmp>32)
	{
		dwTmp = 32;
	}
	memcpy(p, pDat, dwTmp);
	memset(p+dwTmp, 0, 32-dwTmp);
	pDat += dwTmp;
	iLen += 32;//固定32字节
	
	pDat++;//DT_VIS_STR, 
	dwTmp = *pDat++;//密码
	if(dwTmp>32)
	{
		dwTmp = 32;
	}
	memcpy(p+32, pDat, dwTmp);
	memset(p+32+dwTmp, 0, 32-dwTmp);
	pDat += dwTmp;
	iLen += 32;//固定32字节
	
	return iLen;
}

int TermEthIpModeTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256];
	BYTE *pDat = bBuf;
//	//0x4510~0x4517 以太网模块属性4——网络配置
//	DT_STRUCT, 0x06,
//		DT_ENUM, 0x01,	//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
//		DT_OCT_STR, 0x04,	//IP地址	192.168.1.200
//			0xC0, 0xA8, 0x01, 0xC8, 
//		DT_OCT_STR, 0x04,	//子网掩码	255.255.255.0
//			0xFF, 0xFF, 0xFF, 0x00,
//		DT_OCT_STR, 0x04,	//网关地址	192.168.1.1
//			0xC0, 0xA8, 0x01, 0x01, 
//		DT_VIS_STR, 0x04,	//PPPoE用户名
//			'T',	'E',   'S', 'T',   
//		DT_VIS_STR, 0x04,	//PPPoE密码
//			'T',	'E',   'S', 'T',   

	ReadItemEx(BN0, PN0, 0x4512, bBuf);
	pDat++;// = DT_STRUCT;
	pDat++;// = 0x06;
	pDat++;// = DT_ENUM;
	if (*pDat == 2)
		*p = 1;
	else
		*p = 0;//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
	iLen += 1;
	return iLen;
}

int TermAddrTo13761(BYTE *pBuf)
{
	int iLen = 0;
	BYTE *p = pBuf;
	BYTE bBuf[256], bBuf2[16], bLen;
	BYTE *pDat = bBuf;
	DWORD Addr;

	//0x4001， 通讯地址，默认01--8byte
	//DT_OCT_STR,0x06,
	//	0x11,0x22,0x33,0x44,0x55,0x66,
	//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	ReadItemEx(BN0, PN0, 0x4001, pDat);
	pDat++;// = DT_OCT_STR;
	bLen = *pDat++;// = 0x06;

	*p++ = *pDat++;	//行政区划
	*p++ = *pDat++;
	memrcpy(bBuf2, pDat, bLen-2);
	Addr = BcdToDWORD(bBuf2, 4);
	memcpy(p, (BYTE *)&Addr, 2);
	iLen += 4;
	
	return iLen;
}


T13761FmtMap g_13761ParaTransTbl[] = 
{
	// OA----------格式描述串----格式描述串长度----格式转换函数
	{0x001f,	BANK0,	PN0,	TermCommCfgParaTo13761},	//FN1
	{0x003f,	BANK0,	PN0,	GprsMastCommParaTo13761},	//FN3
	{0x004f,	BANK0,	PN0,	SmsCommParaTo13761},	//FN4
	{0x005f,	BANK0,	PN0,	NULL},	//FN5 没法转换
	{0x006f,	BANK0,	PN0,	NULL},	//FN6 没法转换
	{0x007f,	BANK0,	PN0,	TermIpParaTo13761},	//FN7
	{0x008f,	BANK0,	PN0,	TermUpComParaTo13761},	//FN8
	{0x010f,	BANK0,	PN0,	VPNParaTo13761},	//FN16
	{0xa1b6,	BANK10,	PN0,	TermEthIpModeTo13761},
	{0x232f,	BANK0, 	PN0,	TermAddrTo13761},
};
#define Proto13761_CONVERT_NUM sizeof(g_13761ParaTransTbl)/sizeof(T13761FmtMap)

//文件格式:文件长度(4个字节)+CRC校验(4个字节)+数据1+数据2+...+数据n
//数据格式:数据项长度(2个字节,包含数据项ID/数据项BANK/数据项Point/数据项内容长度)
//+数据项ID(2个字节)+数据项BANK(2个字节)+数据项Point(2个字节)+数据项内容
//opt:0 to oob; 1:to1376.1
int CParaMgr::OobParaTrans(char* pUserDir, BYTE opt, BYTE* pbPara)
{	
	int iRet = 0;
#ifdef SYS_LINUX
	int i, f;
    char szName[255];
	char command[255] = {0};;
	BYTE bBuf[512];
    int iFileLen = 0;
    int iDataLen = 0;
    int iLen = 0;
    WORD wMycrc;
	DWORD dwOff = 0;
	DIR* dir;
	char bUserPath[128] = {0};
	//先建立目录
	memset(bUserPath, 0, sizeof(bUserPath));
	sprintf(bUserPath, pUserDir);
	dir = opendir(bUserPath);
	if (dir == NULL)
	{
		if (mkdir(bUserPath, 0) == -1)
			return false;
	}
	else
	{
		closedir(dir);
	}
	
	memset(bUserPath, 0, sizeof(bUserPath));
	sprintf(bUserPath, "%spara",pUserDir);
	dir = opendir(bUserPath);
	if (dir == NULL)
	{
		if (mkdir(bUserPath, 0) == -1)
			return false;
	}
	else
	{
		closedir(dir);
	}
	memset(bUserPath, 0, sizeof(bUserPath));
	sprintf(bUserPath, "%sdata",pUserDir);
	dir = opendir(bUserPath);
	if (dir == NULL)
	{
		if (mkdir(bUserPath, 0) == -1)
			return false;
	}
	else 
	{
		closedir(dir);
	}

	memset(bUserPath, 0, sizeof(bUserPath));
	sprintf(bUserPath, "%scfg",pUserDir);
	dir = opendir(bUserPath);
	if (dir == NULL)
	{
		if (mkdir(bUserPath, 0) == -1)
			return false;
	}
	else 
	{
		closedir(dir);
	}
	//创建目标DFT文件
	sprintf(szName, "%s/ParaTrans.dft", bUserPath);

    f = open(szName, O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE);    //O_RDONLY, S_IREAD|S_IWRITE
    if (f < 0)
    {
        DTRACE(DB_CRITICAL, ("CParaMgr::OobParaTrans : Open %s failed.\r\n", szName));
        iRet = -1;//打开文件失败
        goto ERROROUT;
    }
//	    lseek(f, 8, SEEK_SET);//文件长度(4个字节)+CRC校验(4个字节)
    dwOff = 8;
	if(opt == 0)
	{
		for(i=0; i<OOB_CONVERT_NUM; i++)
		{
			if(g_OobParaTransTbl[i].pfnFmtTrans != NULL)
			{
				iLen = g_OobParaTransTbl[i].pfnFmtTrans(bBuf+8);
				if(iLen > 0)
				{
					if (i == 1 || i == 10)	//主站参数
					{
						//TraceBuf(DB_CRITICAL, "CParaMgr::OobParaTrans to oob pbPara:", pbPara, 12);
						memcpy(bBuf+8+6, pbPara, 4);
						memrcpy(bBuf+8+11, pbPara+4, 2);
						memcpy(bBuf+8+17, pbPara+6, 4);
						memrcpy(bBuf+8+22, pbPara+10, 2);
						//TraceBuf(DB_CRITICAL, "CParaMgr::OobParaTrans to oob bBuf+8:===============>>>>>>>", bBuf+8, 24);
					}
					memcpy(bBuf+2, &g_OobParaTransTbl[i].wID, 6);
					iLen += 6;//包含wID wBn wPn
					memcpy(bBuf, (BYTE*)&iLen, 2);
					iLen += 2;//包含数据项目长度					
					lseek(f, dwOff, SEEK_SET);
					if (write(f, bBuf, iLen) != (int )iLen)
					{			
						DTRACE(DB_GLOBAL, ("CParaMgr::OobParaTrans :  fail to write %s .\r\n", szName));
						iRet = -1;//写配置文件出错
						goto ERROROUT;
					}
					dwOff += iLen;
					if (dwOff > 50*1024)//文件大小不能超过50K
					{
						DTRACE(DB_CRITICAL, ("CParaMgr::OobParaTrans : file %s is to long.\r\n", szName));
						iRet = -2;//配置文件太大
						goto ERROROUT;		  
					}
				}
			}
		}
	}
	else
	{
		for(i=0; i<Proto13761_CONVERT_NUM; i++)
		{
			if(g_13761ParaTransTbl[i].pfnFmtTrans != NULL)
			{
				iLen = g_13761ParaTransTbl[i].pfnFmtTrans(bBuf+8);
				if(iLen > 0)
				{
					if (i == 1)	//主站参数
					{
						//TraceBuf(DB_CRITICAL, "CParaMgr::OobParaTrans to 13761 pbPara+6:======>>>>>>>>", pbPara+6, 24);
						memcpy(bBuf+8, pbPara+6, 4);
						memrcpy(bBuf+8+4, pbPara+11, 2);
						memcpy(bBuf+8+6, pbPara+17, 4);
						memrcpy(bBuf+8+10, pbPara+22, 2);
						//TraceBuf(DB_CRITICAL, "CParaMgr::OobParaTrans to 13761 bBuf+8:=======>>>>>>>", bBuf+8, 12);
					}
					memcpy(bBuf+2, &g_13761ParaTransTbl[i].wID, 6);
					iLen += 6;//包含wID wBn wPn
					memcpy(bBuf, (BYTE*)&iLen, 2);
					iLen += 2;//包含数据项目长度					
					lseek(f, dwOff, SEEK_SET);
					if (write(f, bBuf, iLen) != (int )iLen)
					{			
						DTRACE(DB_GLOBAL, ("CParaMgr::OobParaTrans :  fail to write %s .\r\n", szName));
						iRet = -1;//写配置文件出错
						goto ERROROUT;
					}
					dwOff += iLen;
					if (dwOff > 50*1024)//文件大小不能超过50K
					{
						DTRACE(DB_CRITICAL, ("CParaMgr::OobParaTrans : file %s is to long.\r\n", szName));
						iRet = -2;//配置文件太大
						goto ERROROUT;		  
					}
				}
			}
		}
	}	
	
	
	iFileLen = dwOff;
	iDataLen = dwOff-8;
    dwOff = 8;

    ///////////////////////////////////////////////////////////////////////
	wMycrc = 0;	
	for(int i=0; i<iDataLen/512; i++)
	{
		lseek(f, dwOff, SEEK_SET);
		if (read(f, bBuf, 512) != 512)
		{
			DTRACE(DB_CRITICAL, ("CParaMgr::OobParaTrans : read %s error.\r\n", szName));
			iRet = -3;//读取配置文件出错
			goto ERROROUT;
		}
		wMycrc = get_crc_16(wMycrc, bBuf, 512);
		dwOff += 512;
	}	

	iLen = iDataLen%512;
	if(iLen > 0)
	{
		lseek(f, dwOff, SEEK_SET);
		if (read(f, bBuf, iLen) != iLen)
		{
			DTRACE(DB_CRITICAL, ("CParaMgr::OobParaTrans : read %s error.\r\n", szName));
			iRet = -3;//读取配置文件出错
			goto ERROROUT;
		}
		wMycrc = get_crc_16(wMycrc, bBuf, iLen);
		dwOff += iLen;
	}
	
	DTRACE(DB_CRITICAL, ("CParaMgr::OobParaTrans : wMycrc = %d.\r\n", wMycrc));
	memcpy(bBuf, (BYTE*)&iFileLen, 4);
	memcpy(bBuf+4, (BYTE*)&wMycrc, 2);
	memset(bBuf+6, 0, 2);
	lseek(f, 0, SEEK_SET);
	if (write(f, bBuf, 8) != 8)
	{			
		DTRACE(DB_GLOBAL, ("CParaMgr::OobParaTrans :	fail to write %s .\r\n", szName));
		iRet = -4;//写配置文件 CRC校验出错
		goto ERROROUT;
	}
	
    close(f);
	//创建自动应用DFT文件
	memset(bUserPath, 0, sizeof(bUserPath));
	sprintf(bUserPath, "%scfg",pUserDir);
	sprintf(command, "rm -rf %s/cfgpermit.cfg",bUserPath);
	system(command);
	sprintf(command, "echo ParaTrans.dft>%s/cfgpermit.cfg",bUserPath);
	system(command);	
    return iRet;
    
ERROROUT:
	if (f >= 0)
	    close(f);

#endif
    return iRet;
}


//opt:0只切换程序，不切换参数
//	  1只切换程序及参数
bool CParaMgr::SwitchToOob(BYTE* pbPara)
{
	char command[255] = {0};
//		//先保存参数
//	#ifdef SYS_VDK			
//			FaSavePara();	//DSP平台比较慢,只保存参数
//	#else			
//			FaSave();		//其它平台比较快,全部保存
//	#endif
	//if(opt == 1)
	{
		if(OobParaTrans("/mnt/data/oob/",0,pbPara)!=0)
		{
			printf("CParaMgr::SwitchToOob OobParaTrans fail!\r\n");
			//参数文件创建OK
			return false;
		}	
		printf("CParaMgr::SwitchToOob OobParaTrans ok!\r\n");
	}
	//创建自动应用DFT文件
	sprintf(command, "echo oob>/mnt/app/app_ver");
	system(command);	
	SetInfo(INFO_APP_RST);//复位CPU
	return true;
}

//opt:0只切换程序，不切换参数
//	  1只切换程序及参数
bool CParaMgr::SwitchTo13761(BYTE* pbPara)
{
	char command[255] = {0};
//		//先保存参数
//	#ifdef SYS_VDK			
//			FaSavePara();	//DSP平台比较慢,只保存参数
//	#else			
//			FaSave();		//其它平台比较快,全部保存
//	#endif
	//if(opt == 1)
	{
		if(OobParaTrans("/mnt/data/",1,pbPara)!=0)
		{
			printf("CParaMgr::SwitchTo13761 OobParaTrans fail!\r\n");
			//参数文件创建OK
			return false;
		}
		printf("CParaMgr::SwitchTo13761 OobParaTrans ok!\r\n");
	}
	//删除自动应用DFT文件
	sprintf(command, "rm -rf /mnt/app/app_ver");
	system(command);	
//		SetInfo(INFO_APP_RST);//复位CPU
	SetInfo(INFO_CLASS19_METHOD_RST);//复位CPU
	return true;
}

