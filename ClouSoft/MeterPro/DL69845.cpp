/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DL69845.c
 * 摘    要：本文件给出698.45抄表协议的功能实现
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年8月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "DL69845.h"
#include "DbAPI.h"
#include "Esam.h"


WORD DL69845MakeFrm(WORD wPn, BYTE* pbAddr, BYTE* pbTxBuf, BYTE* pbAPDU, WORD wAPDULen);
int DL69845TxRx(struct TMtrPro* pMtrPro, T698Tmp* pTmp698, WORD wLen, bool bSecurity);
bool DL69845RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize);
void DL69845GetProPrintType(BYTE* pbPrintPro, char* pszProName);


bool Mtr69845Init(struct TMtrPro* pMtrPro, BYTE bThrId)
{
	pMtrPro->bThrId = bThrId;

	pMtrPro->pfnAskItem = DL69845AskItem;	
	pMtrPro->pfnDirAskItem = DL69845DirAskItem;
	pMtrPro->pfnRcvBlock = DL69845RcvBlock;
	pMtrPro->pfnGetProPrintType = DL69845GetProPrintType;
	pMtrPro->pfnWriteItem = DL69845WriteItem;	

	pMtrPro->pbTxBuf = &m_MtrTxBuf[bThrId][0];
	pMtrPro->pbRxBuf = &m_MtrRxBuf[bThrId][0];
	memset(pMtrPro->pbTxBuf, 0, MTR_FRM_SIZE); 
	memset(pMtrPro->pbRxBuf, 0, MTR_FRM_SIZE); 	
	return true;
}

//描述：读取698.45表的ESAM序列号接口
//参数：@pMtrPro 电表协议指针
//返回：结果数据长度
int DL69845GetMtrEsamSN(struct TMtrPro* pMtrPro, BYTE* pbData)
{
	int iRet;
	BYTE bEsamSn[20];
	T698Tmp tTmp698;
	WORD wAPDULen, wFrmLen;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;
	BYTE bFrmHead = 8 + pMtrPara->bAddr[0];
	bool bSecurity = false;

	memset(bEsamSn, 0, sizeof(bEsamSn));
	iRet = ReadItemEx(BN0, pMtrPara->wPn, 0xF114, bEsamSn);
	if (IsAllAByte(bEsamSn, 0, sizeof(bEsamSn)))
	{
		wAPDULen = GetRequestNormal(0xF1000200, pbTxBuf+bFrmHead);

		wFrmLen = DL69845MakeFrm(pMtrPara->wPn, pMtrPara->bAddr, pbTxBuf, pbTxBuf+bFrmHead, wAPDULen, bSecurity);

		memset((BYTE* )&tTmp698, 0, sizeof(tTmp698));
		iRet = DL69845TxRx(pMtrPro, &tTmp698, wFrmLen, bSecurity);

		if (iRet > 0)
		{
			if (GetResponseNormal(0xF1000200, &pbRxBuf[tTmp698.wRxAPDUPos], tTmp698.wRxAPDULen, bEsamSn) > 0)
			{
				WriteItemEx(BN0, pMtrPara->wPn, 0xF114, bEsamSn);
				memcpy(pbData, &bEsamSn[2], bEsamSn[1]);
				iRet = bEsamSn[1];
			}
		}
	}
	else if (iRet > 0)
	{
		memcpy(pbData, &bEsamSn[2], bEsamSn[1]);
		iRet = bEsamSn[1];
	}

	return iRet;
}

bool IsNeedRdBySecurityMode(struct TMtrPro* pMtrPro, DWORD dwOAD)
{
	int iRet;
	T698Tmp tTmp698;
	BYTE bSecurity[2] = {0};
	WORD wAPDULen, wFrmLen;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
	BYTE bFrmHead = 8 + pMtrPara->bAddr[0];
	BYTE bData[10] = {0};
	WORD wOi;
	
	ReadItemEx(BN0, pMtrPara->wPn, 0xF116, bSecurity);

	if(bSecurity[1] == 0)
	{
		wAPDULen = GetRequestNormal(0xF1010200, pbTxBuf+bFrmHead);	
		wFrmLen = DL69845MakeFrm(pMtrPara->wPn, pMtrPara->bAddr, pbTxBuf, pbTxBuf+bFrmHead, wAPDULen, false);

		memset((BYTE* )&tTmp698, 0, sizeof(tTmp698));
		iRet = DL69845TxRx(pMtrPro, &tTmp698, wFrmLen, false);
		memset(pbTxBuf, 0, sizeof(pbTxBuf));
		if (iRet > 0)
		{
			GetResponseNormal(0xF1010200, &pbRxBuf[tTmp698.wRxAPDUPos], tTmp698.wRxAPDULen, bData);
			memcpy(bSecurity, bData, 2);
		}
		else
			return false;

		if(bSecurity[1] != 1)
		{
			bSecurity[0] = DT_ENUM;
			bSecurity[1] = 2;//未启用安全模式
			WriteItemEx(BN0, pMtrPara->wPn, 0xF116, bSecurity);
			return false;
		}
		else
		{
			WriteItemEx(BN0, pMtrPara->wPn, 0xF116, bSecurity);
		}
	}
	else if(bSecurity[1] == 2)
		return false;

	wOi = dwOAD >> 16;
	if(((wOi&0xf000)==0x1000) || ((wOi&0xf000)==0x2000&&(wOi!=0x202c)) || 
		((wOi&0xf000)==0x4000&&(wOi!=0x4000)&&(wOi!=0x4001)&&(wOi!=0x4002)&&(wOi!=0x4111)) ||
		((wOi&0xf000)==0x3000) || ((wOi&0xff00)==0x5000) || ((wOi&0xff00)==0x8000))
	{
		DTRACE(DB_DL69845, ("IsNeedRdBySecurityMode is need, dwOAD=%x.\r\n",dwOAD));
		return true;
	}

	return false;
}
	
//描述：读取698.45数据标识的数据的接口
//参数：@pMtrPro 电表协议指针
//	    @bRespType电表帧的返回类型1:GetResponseNormal; 3:GetResponseRecord。
//                   如果是LIST方式，保存的时候转换成单个再保存
//      @pbData表返回的数据
//		@dwOAD
//      @pbRSD选择方法
//      @bLenRSD选择方法长度
//      @pbRCSD选择属性列
//      @pbRCSD选择属性列长度
//返回：结果数据长度
int DL69845AskItem(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	int iRet;
	T698Tmp tTmp698;
	WORD wAPDULen, wFrmLen;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;
	BYTE bFrmHead = 8 + pMtrPara->bAddr[0];
	bool bSecurity = false;

	if ( !MtrProOpenComm(pMtrPro->pMtrPara->pComm, &pMtrPro->pMtrPara->CommPara) )
		return 0;

	if (dwOAD == 0x202A0200)	//目的服务器地址，电科院检测台体不能读取，直接用当前抄读的表地址填充
	{
		iRet = pMtrPara->bAddr[0] + 1;
		memcpy(pbData, pMtrPara->bAddr, iRet);
		return iRet;
	}

	if (dwOAD==0x60400200 || dwOAD==0x60410200 || dwOAD==0x60420200)
    {
        TTime time;
        GetCurTime(&time);        
        return OoTimeToDateTimeS(&time, pbData);
    }
	
	bSecurity = IsNeedRdBySecurityMode(pMtrPro, dwOAD);
	DTRACE(DB_DL69845, ("DL69845AskItem bSecurity=%d.\r\n",bSecurity));

	if (bRespType == 1)
		wAPDULen = GetRequestNormal(dwOAD, pbTxBuf+bFrmHead);	
	else
		wAPDULen = GetRequestRecord(dwOAD, pbTxBuf+bFrmHead, pbRSD, bLenRSD, pbRCSD, bLenRCSD);

	wFrmLen = DL69845MakeFrm(pMtrPara->wPn, pMtrPara->bAddr, pbTxBuf, pbTxBuf+bFrmHead, wAPDULen, bSecurity);

	memset((BYTE* )&tTmp698, 0, sizeof(tTmp698));
	iRet = DL69845TxRx(pMtrPro, &tTmp698, wFrmLen, bSecurity);

	if (iRet > 0)
	{
		if (bRespType == 1)
			return GetResponseNormal(dwOAD, &pbRxBuf[tTmp698.wRxAPDUPos], tTmp698.wRxAPDULen, pbData);
		else
			return GetResponseRecord(dwOAD, &pbRxBuf[tTmp698.wRxAPDUPos], tTmp698.wRxAPDULen, pbRCSD, bLenRCSD, pbData);
	}
	else
		return iRet;
}

//描述：代理读取698.45协议表数据的接口
//参数：@pMtrPro 电表协议指针
//	    @bRespType电表帧的返回类型1:GetResponseNormal; 3:GetResponseRecord。
//                   如果是LIST方式，保存的时候转换成单个再保存
//      @pbData表返回的数据
//		@dwOAD
//      @pbRSD选择方法
//      @bLenRSD选择方法长度
//      @pbRCSD选择属性列
//      @pbRCSD选择属性列长度
//返回：结果数据长度
int DL69845DirAskItem(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData)
{
	int iRet;
	T698Tmp tTmp698;
	WORD wAPDULen, wFrmLen;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;
	BYTE bFrmHead = 7 + pMtrPara->bAddr[0] + 1;
	bool bSecurity = false;

	if ( !MtrProOpenComm(pMtrPro->pMtrPara->pComm, &pMtrPro->pMtrPara->CommPara) )
		return 0;

	pbTxBuf[bFrmHead] = bRespType; //AFN
	pbTxBuf[bFrmHead+1] = bChoice; //Choice
	pbTxBuf[bFrmHead+2] = 0; //PIID
	memcpy(pbTxBuf+bFrmHead+3, pbTx, wTxLen);
	wAPDULen = wTxLen + 3;

	wFrmLen = DL69845MakeFrm(pMtrPara->wPn, pMtrPara->bAddr, pbTxBuf, pbTxBuf+bFrmHead, wAPDULen, bSecurity);

    for (BYTE bCnt=0; bCnt<2; bCnt++)
    {
    	memset((BYTE* )&tTmp698, 0, sizeof(tTmp698));
    	iRet = DL69845TxRx(pMtrPro, &tTmp698, wFrmLen, bSecurity);

    	if (iRet > 0)
    	{
    		memcpy(pbData, &pbRxBuf[tTmp698.wRxAPDUPos+3], tTmp698.wRxAPDULen-3);
    		iRet = tTmp698.wRxAPDULen - 3;
            break;
    	}        
    }
    return iRet;
}

WORD GetRequestNormal(DWORD dwOAD, BYTE* pbTxBuf)
{
	pbTxBuf[0] = DL69845_APPSVR_GETREQUEST; //GET-Request
	pbTxBuf[1] = DL69845_APPSVR_GETREQUEST_NORMAL;	//GET-Request-NORMAL
	pbTxBuf[2] = 0; //PIID
	OoDWordToOad(dwOAD, pbTxBuf+3);

	return 7;
}

WORD GetRequestRecord(DWORD dwOAD, BYTE* pbTxBuf, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	pbTxBuf[0] = DL69845_APPSVR_GETREQUEST; //GET-Request
	pbTxBuf[1] = DL69845_APPSVR_GETREQUEST_RECORD;	//GET-Request-RECORD
	pbTxBuf[2] = 0; //PIID
	OoDWordToOad(dwOAD, pbTxBuf+3);
	memcpy(pbTxBuf+3+sizeof(dwOAD), pbRSD, bLenRSD);
	memcpy(pbTxBuf+3+sizeof(dwOAD)+bLenRSD, pbRCSD, bLenRCSD);

	return 7+bLenRSD+bLenRCSD;
}

int GetResponseNormal(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbDstBuf, WORD wPn)
{
	int iDataLen;
	int iRet = -1;
	DWORD dwRxOAD;
	BYTE *pFollowReport;

	dwRxOAD = OoOadToDWord(&pbSrcBuf[3]);
	if (dwOAD == dwRxOAD)
	{
		if (pbSrcBuf[7]==1 && wSrcLen>10)
		{
			if ((iDataLen=OoGetDataLen(DT_OAD, &pbSrcBuf[3])) > 0)
			{
				pFollowReport = &pbSrcBuf[8+iDataLen];
				if (*pFollowReport++ == 1) //上报若干个对象属性及其数据
				{
					pFollowReport++; //个数，不考虑多个
					DWORD dwRePortOAD = OoOadToDWord(pFollowReport);
					pFollowReport += 4;
					if (dwRePortOAD == 0x33200201) //事件上报
					{
						if (*pFollowReport++ == 1) //data
						{
							BYTE bPnRdStatWordFlg[PN_MASK_SIZE];
							ReadItemEx(BN0, PN0, 0x3B19, bPnRdStatWordFlg);//抄读主动上报状态字电表标志位
							bPnRdStatWordFlg[wPn/8] |= (1<<(wPn%8));
							WriteItemEx(BN0, PN0, 0x3B19, bPnRdStatWordFlg);

							//保存需要抄读的事件OAD
							int iLen = ReadItemEx(BN0, PN0, 0x3B20, bPnRdStatWordFlg);//抄读主动上报状态字电表标志位
							if (iLen > 0)
							{
								int i = 0;
								for (; i<bPnRdStatWordFlg[0]; i++)
								{
									if (memcmp(pFollowReport, &bPnRdStatWordFlg[1+i*4], 4) == 0)
									{
										break;
									}
								}
								if (i == bPnRdStatWordFlg[0])
								{
									if ((iLen-1)/4 > bPnRdStatWordFlg[0])
									{										
										bPnRdStatWordFlg[0] += 1;
									}
									else
									{
										memmove(&bPnRdStatWordFlg[1], &bPnRdStatWordFlg[5], (bPnRdStatWordFlg[0]-1)*4);
									}
									memcpy(&bPnRdStatWordFlg[1+i*4], pFollowReport, 4);
									WriteItemEx(BN0, wPn, 0x3B20, bPnRdStatWordFlg);
								}
							}
						}
					}
				}
			}
			iRet = wSrcLen - 8 - 2;
			memcpy(pbDstBuf, &pbSrcBuf[8], iRet);
		}
		else
		{
			iRet = -2; //不支持
		}
	}

	return iRet;
}

int GetResponseRecord(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbRCSD, BYTE bLenRCSD, BYTE* pbDstBuf)
{
	int iRet = -1;
	DWORD dwRxOAD;

	dwRxOAD = OoOadToDWord(&pbSrcBuf[3]);
	if (dwOAD == dwRxOAD)
	{
		if (wSrcLen > 11+bLenRCSD)
		{
			iRet = wSrcLen - 9 - bLenRCSD - 2;
			if ((dwOAD&0xff000000) == 0x30000000)
			{
				memcpy(pbDstBuf, &iRet, 2);
				memcpy(pbDstBuf+2, &pbSrcBuf[7+bLenRCSD+2], iRet);
				iRet += 2;
			}
			else
				memcpy(pbDstBuf, &pbSrcBuf[7+bLenRCSD+2], iRet);
		}
		else
		{
			iRet = -2; //不支持
		}
	}

	return iRet;
}

//描述：组发送帧
WORD DL69845MakeFrm(WORD wPn, BYTE* pbAddr, BYTE* pbTxBuf, BYTE* pbAPDU, WORD wAPDULen, bool bSecurity)
{
	int iLen;
	//BYTE bSecurity[2];
	BYTE bEsamBuf[32], bBuf[256];
	WORD wLen, wCrc, wPtr = 0;
	BYTE bAddrLen = pbAddr[0];

	wLen = 7 + bAddrLen + wAPDULen + 3;

#if 1//modibf by yuanjw at 20170802 
//	ReadItemEx(BN0, PN0, 0xF112, bSecurity);
	//if (bSecurity[1] == 1)
	if (bSecurity == true)
	{
		iLen = EsamGetRandom(bEsamBuf);
		if (iLen > 0)
		{
			WriteItemEx(BN0, wPn, 0xF115, bEsamBuf);

			memset(bBuf, 0, sizeof(bBuf));
			bBuf[0] = 16; //安全请求
			bBuf[1] = 0; //明文应用数据单元
#if 0
			bBuf[2] = DT_OCT_STR;
			bBuf[3] = wAPDULen;
			memcpy(&bBuf[4], pbAPDU, wAPDULen);
			bBuf[4+wAPDULen] = 1; //随机数RN
			bBuf[5+wAPDULen] = DT_OCT_STR;
			bBuf[6+wAPDULen] = iLen;
			memcpy(&bBuf[7+wAPDULen], bEsamBuf, iLen);
			
			wLen += (7+iLen);
#else
			bBuf[2] = wAPDULen;
			memcpy(&bBuf[3], pbAPDU, wAPDULen);
			bBuf[3+wAPDULen] = 1; //随机数RN
			bBuf[4+wAPDULen] = DT_OCT_STR;
			bBuf[5+wAPDULen] = iLen;
			memcpy(&bBuf[6+wAPDULen], bEsamBuf, iLen);
			
			wLen += (6+iLen);
#endif

		}
		else
		{
			memset(bEsamBuf, 0, sizeof(bEsamBuf));
			WriteItemEx(BN0, wPn, 0xF115, bEsamBuf);
		}
	}
#endif
	pbTxBuf[wPtr++] = 0x68;
	memcpy(&pbTxBuf[wPtr], (BYTE *)&wLen, 2);
	wPtr += 2;
	pbTxBuf[wPtr++] = DL698_CTL_DIR_CLI | DL698_CTL_PRM_CLI | DL698_CTL_AFN_USERDATA;
	if (IsExistHalfAByte(&pbAddr[1], 0x0A, bAddrLen))//存在A就是通配地址
		pbTxBuf[wPtr++] = ((bAddrLen - 1) & 0x0f) | 0x40;
	else
		pbTxBuf[wPtr++] = (bAddrLen - 1) & 0x0f;
	revcpy(&pbTxBuf[wPtr], &pbAddr[1], bAddrLen); //TA
	wPtr += bAddrLen;
	pbTxBuf[wPtr++] = 0; //CA
	wCrc = CheckCrc16(pbTxBuf+1, wPtr-1);
	pbTxBuf[wPtr++] = (wCrc&0xff);
	pbTxBuf[wPtr++] = (wCrc>>8);

#if 1
	//if (bSecurity[1] == 1)
	if (bSecurity == true)
	{
		if (iLen > 0)
		{
			memcpy(&pbTxBuf[wPtr], bBuf, wAPDULen+iLen+6);
			wPtr += (wAPDULen + iLen + 6);
		}
	}
	else
#endif
		wPtr += wAPDULen;

	pbTxBuf[wPtr++] = 0; //时间标签
	wCrc = CheckCrc16(pbTxBuf+1, wPtr-1);
	pbTxBuf[wPtr++] = (wCrc&0xff);
	pbTxBuf[wPtr++] = (wCrc>>8);

	pbTxBuf[wPtr++] = 0x16;

	return wPtr;
}

//描述：帧解析
int DL69845TxRx(struct TMtrPro* pMtrPro, T698Tmp* pTmp698, WORD wLen, bool bSecurity)
{
	int iLen;
	bool fReadSuccess;
	BYTE /*bSecurity[2],*/ bDataLen, bMacLen, bMAC[20], bEsamBuf[20];
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;

	if (MtrProSend(pMtrPro->pMtrPara->pComm, pbTxBuf, wLen) != wLen)
	{
		DTRACE(DB_DL69845, ("TxRx : fail to write comm.\r\n"));
		return 0;
	}

	pTmp698->nRxStep = 0;	

	fReadSuccess = ReadCommFrm(pMtrPro, (void*)pTmp698, 0, 4, 3, 200, MTR_FRM_SIZE, 0, NULL, 0);

	if (fReadSuccess)	//接收到一个完整的帧
	{	
#if 1//modibf by yuanjw at 20170802 
		//ReadItemEx(BN0, PN0, 0xF112, bSecurity);
		//if (bSecurity[1] == 1)
		if (bSecurity == true)
		{
			if (/*iLen>0 && */pbRxBuf[pTmp698->wRxAPDUPos]==144)
			{
				if (pbRxBuf[pTmp698->wRxAPDUPos+1] == 0) //明文
				{
					bDataLen = pbRxBuf[pTmp698->wRxAPDUPos+2];
					bMacLen = pbRxBuf[pTmp698->wRxAPDUPos+2+bDataLen+2+1];
					if (bMacLen < sizeof(bMAC))
						memcpy(bMAC, &pbRxBuf[pTmp698->wRxAPDUPos+2+bDataLen+3], bMacLen);
					ReadItemEx(BN0, pMtrPara->wPn, 0xF115, bEsamBuf);
					//if (Esam_ReadMtrDataVerify(0, &pMtrPara->bAddr[1], pMtrPara->bAddr[0], bEsamBuf, 12, &pbRxBuf[pTmp698->wRxAPDUPos+2], bDataLen, bMAC, NULL) == 0)
					{
						DTRACE(DB_DL69845, ("DL69845TxRx : Security Rx date len=%x\r\n",bDataLen));
						memmove(&pbRxBuf[pTmp698->wRxAPDUPos], &pbRxBuf[pTmp698->wRxAPDUPos+3], bDataLen);
						pTmp698->wRxAPDULen -= (3+2+bMacLen+1);
						DTRACE(DB_DL69845, ("DL69845TxRx : Security Rx date bMacLen=%x, wRxAPDULen=%x\r\n",bMacLen,pTmp698->wRxAPDULen));
					}
				}
			}
		}
#endif
		return 1;
	}
	DTRACE(DB_DL69845, ("TxRx : fail to rx frame.\r\n")); 

	return 0;
}

//描述：接收验证
bool DL69845RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize)
{
	WORD wCrc;
	BYTE b; 

	T698Tmp* pTmp698 = (T698Tmp* )pTmpInf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf; 
	BYTE* pbTxBuf = pMtrPro->pbTxBuf; 


#ifdef FRM_SEG_FLG
	memset((BYTE*)pTmp698, 0, sizeof(T698Tmp));
#endif

	for ( ; dwLen; dwLen--)
	{
		b = *pbBlock++;

		switch (pTmp698->nRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				memset(pbRxBuf, 0, MTR_FRM_SIZE);
				pbRxBuf[0] = 0x68;
				pTmp698->wRxPtr = 1;
				pTmp698->wRxCnt = 4;
				pTmp698->nRxStep = 1;
			}
			break;
		case 1:    //地址域前的数据
			pbRxBuf[pTmp698->wRxPtr++] = b;
			pTmp698->wRxCnt --;
			if (pTmp698->wRxCnt == 0)   //接收完，进行校验
			{
				if ((pbRxBuf[DL69845_CTL_POS]&0x07) == (pbTxBuf[DL69845_CTL_POS]&0x07))
				{
					memcpy(&pTmp698->wRxDataLen, &pbRxBuf[DL69845_LEN_POS], 2);
					pTmp698->wRxCnt = (pbRxBuf[DL69845_TSA_POS]&0x0f) + 4;
					pTmp698->nRxStep = 2;
				}
				else
				{
					pTmp698->nRxStep = 0;
				}
			}
			break;
		case 2:    //数据域前的数据
			pbRxBuf[pTmp698->wRxPtr++] = b;
			pTmp698->wRxCnt --;
			if (pTmp698->wRxCnt == 0)   //接收完，进行校验
			{
				wCrc = CheckCrc16(pbRxBuf+1, pTmp698->wRxPtr - 3);
				if (memcmp(&pbTxBuf[DL69845_TSA_POS], &pbRxBuf[DL69845_TSA_POS], pTmp698->wRxPtr - 8)==0 && (pbRxBuf[pTmp698->wRxPtr-2]==(wCrc&0xff)) && (pbRxBuf[pTmp698->wRxPtr-1]==((wCrc>>8)&0xff)))
				{
					pTmp698->wRxCnt = pTmp698->wRxDataLen + 2 - pTmp698->wRxPtr;
					pTmp698->wRxAPDUPos = pTmp698->wRxPtr;
					pTmp698->nRxStep = 3;
				}
				else
				{
					pTmp698->nRxStep = 0;
				}
			}
			break;
		case 3:     //数据 + 检验码 + 结束码
			pbRxBuf[pTmp698->wRxPtr++] = b;
			pTmp698->wRxCnt -- ;
			if (pTmp698->wRxCnt == 0)   //接收完，进行校验
			{
				pTmp698->nRxStep = 0;

				wCrc = CheckCrc16(pbRxBuf+1, pTmp698->wRxPtr - 4);
				if (pbRxBuf[pTmp698->wRxPtr-1]==0x16 && (pbRxBuf[pTmp698->wRxPtr-3]==(wCrc&0xff)) && (pbRxBuf[pTmp698->wRxPtr-2]==((wCrc>>8)&0xff)) && (pbRxBuf[pTmp698->wRxAPDUPos]&0x7f)==pbTxBuf[pTmp698->wRxAPDUPos])
				{
					pTmp698->wRxAPDULen = pTmp698->wRxPtr - pTmp698->wRxAPDUPos - 3;
					return true;    //接收到完整的一帧
				}
			}
			break;
		default:
			pTmp698->nRxStep = 0;
			break;
		} //switch (pTmp698->nRxStep)
	}

	return false;
}

void DL69845GetProPrintType(BYTE* pbPrintPro, char* pszProName)
{
	*pbPrintPro = DB_DL69845;
	memcpy(pszProName, "DL69845", sizeof("DL69845"));
}

int DL69845WriteItem(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen)
{
	int iRet;
	T698Tmp tTmp698;
	if ( !MtrProOpenComm(pMtrPro->pMtrPara->pComm, &pMtrPro->pMtrPara->CommPara) )
		return 0;

	iRet = DL69845TxRx(pMtrPro, &tTmp698, wLen, false);
	if (iRet > 0)
		return 0;
	return -1;

}
