/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterPro.h
 * 摘    要：本文件主要包含抄表协议的基本API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaAPI.h"
#include "MtrProAPI.h"
#include "FaConst.h"


struct TMtrPro g_MtrPro[3];
extern TMtrClkPrg g_MtrClkPrg;
//描述:通过测量点号取出对应测量点的参数和保存变量
//参数:	@wPn:测量点号
//		@pMtrPara:用于返回该电表参数的指针
bool LoadMtrPara(WORD wPn, TMtrPara* pMtrPara)
{
	if (pMtrPara == NULL)
		return false;

	///////////////////////
	return true;
}

//描述:通过测量点号写入对应测量点保存变量
//参数:	@wPn:测量点号
//		@pSaveInf:用于传入该电表保存变量的指针
//		@pbUnsupIdFlg:用于传入该电表保存的ID是否支持标志的缓存的指针
bool SaveMtrInfo(WORD wPn, TMtrSaveInf* pSaveInf)
{
	if (pSaveInf == NULL)
		return false;

	///////////////////////
	return true;
}

//描述:创建各电表协议的类变量
//参数:	@wPn:测量点号
//		@pMtrPara:用于输入电表参数的指针
//		@pSaveInf:用于输入电表保存变量的指针
//返回值:成功则返回具体电表协议结构的指针,失败则返回空指针
struct TMtrPro*  CreateMtrPro(WORD wPn, TMtrPara* pMtrPara, BYTE bThrId)
{
	g_MtrPro[bThrId].pMtrPara = pMtrPara;	

	switch (pMtrPara->bProId)
	{
	case PROTOCOLNO_DLT645:			
		g_MtrPro[bThrId].pMtrPara->pComm = &g_commRs485[bThrId];
		if ( Mtr645Init(&g_MtrPro[bThrId], bThrId) )
			return &g_MtrPro[bThrId];
		break;
	case PROTOCOLNO_DLT645_V07:
		g_MtrPro[bThrId].pMtrPara->pComm = &g_commRs485[bThrId];
		if ( Mtr645V07Init(&g_MtrPro[bThrId], bThrId) )
			return &g_MtrPro[bThrId];	
		break;		

	case PROTOCOLNO_DLT69845:
		g_MtrPro[bThrId].pMtrPara->pComm = &g_commRs485[bThrId];
		if ( Mtr69845Init(&g_MtrPro[bThrId], bThrId) )
			return &g_MtrPro[bThrId];	

#ifdef EN_SBJC_V2
    case PROTOCOLNO_SBJC:
		g_MtrPro[bThrId].pMtrPara->pComm = &g_commRs485[bThrId];
        if ( Mtr645ExtInit(&g_MtrPro[bThrId], bThrId) )
            return &g_MtrPro[bThrId];	
        break;
#endif
	default:
		break;
	}

	return NULL;
}


//描述:抄读各电表协议数据接口
//参数:	@pMtrPro: 用于输入具体电表协议结构的指针
//		@wPn:测量点号
//		@wID:抄读ID
//		@pbBuf:用于返回抄读数据的指针

int AskMtrItem(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	BYTE bPrintPro = pMtrPro->pMtrPara->bProId;
	char szProName[20];
	int iRet=0;

	pMtrPro->pfnGetProPrintType(&bPrintPro, szProName);	
	//DTRACE(bPrintPro, ("MtrPro=%s::Point=%d,read OAD=0x%08x.\r\n", szProName,pMtrPro->pMtrPara->wPn, dwOAD)); 

	iRet = pMtrPro->pfnAskItem(pMtrPro, bRespType, dwOAD, pbData, pbRSD, bLenRSD, pbRCSD, bLenRCSD);

	return  iRet;
}

//单地址广播校时处理
int OneAddrBroadcast_485(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	BYTE bBuf1[100];
	BYTE bBuf2[50];
	int iRet = 0;
	int iStep = -1;
	TTime tmMtr,tmTm;
	BYTE bProId = pMtrPro->pMtrPara->bProId;
	BYTE btProId;
	char szProName[20];
	
	//BYTE bBuf3[50];
	memset(bBuf1, 0, sizeof(bBuf1));
	if (OoProReadAttr(0x4204, 3, 0, bBuf1, sizeof(bBuf1), &iStep) > 0)
	{
		if (bBuf1[9] == 1)//启用
		{
			memset(bBuf2, 0, sizeof(bBuf2));
			GetCurTime(&tmTm);
			tmMtr.nYear = OoLongUnsignedToWord(&pbData[1]);
			tmMtr.nMonth = pbData[3];
			tmMtr.nDay= pbData[4];
			tmMtr.nHour= pbData[5];
			tmMtr.nMinute= pbData[6];
			tmMtr.nSecond= pbData[7];//暂时先固定加1秒的通信延时
			if (IsInvalidTime(tmMtr))
				return -1;

			if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[5]*3600+bBuf1[6]*60+bBuf1[7]))
				return -1;
			
			DWORD dwSecM = TimeToSeconds(tmMtr);
			DWORD dwSecT = TimeToSeconds(tmTm);
			//if ((dwSecM > (dwSecT+300)) || (dwSecT > (dwSecM+300)))//时间差大于5分钟的不理会
			//	return -1;
			if (dwSecM > (dwSecT+bBuf1[3]) ||dwSecT > (dwSecM+bBuf1[3]))//电表时间超差
			{
				TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
				g_MtrClkPrg.bEvtSrcTSA[0] = DT_TSA;
				g_MtrClkPrg.bEvtSrcTSA[1] = pMtrPara->bAddr[0]+1;
				g_MtrClkPrg.bEvtSrcTSA[2] = pMtrPara->bAddr[0]-1;
				//g_MtrClkPrg.bEvtSrcTSA[1] = bTsaLen;
				memcpy(&g_MtrClkPrg.bEvtSrcTSA[3], &pMtrPara->bAddr[1], pMtrPara->bAddr[0]);
				memcpy(&g_MtrClkPrg.bClock[0], pbData, 8);
				g_MtrClkPrg.bClkErr[0] = DT_INT;
				pMtrPro->pfnGetProPrintType(&btProId, szProName);

				
				if (bProId == PROTOCOLNO_DLT645_V07 || bProId == PROTOCOLNO_DLT645)
				{
					BYTE bBufY[18]={0x68, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x08, 0x06, 0x01, 0x02, 0x03, 0x07, 0x03, 0x17, 0x5E, 0x16};

					revcpy(&bBufY[1], &pMtrPara->bAddr[1],6);
					bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
					bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
					bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
					bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
					bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
					bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
					bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);
					memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
					DTRACE(DB_METER, ("OneAddrBroadcast_485 MtrPro=%s::Point=%d\r\n", szProName,pMtrPara->wPn)); 
					iRet = pMtrPro->pfnWriteItem(pMtrPro, dwOAD, 0x04000101, sizeof(bBufY));
					//if (iRet < 0)
					//	return -1;
				}
				else if (bProId == PROTOCOLNO_DLT69845)
				{
					bBuf1[0] = 7;//ACTION_REQ
					bBuf1[1] = DL69845_APPSVR_GETREQUEST_NORMAL;	//GET-Request-NORMAL
					bBuf1[2] = 0; //PIID
					bBuf1[3] = 0x40;
					bBuf1[4] = 0x00;
					bBuf1[5] = 0x7f;
					bBuf1[6] = 0x00;
					bBuf1[7] = 0x1c;
					bBuf1[8] = tmTm.nYear>>8;
					bBuf1[9] = tmTm.nYear & 0x00ff;
					bBuf1[10] = tmTm.nMonth;
					bBuf1[11] = tmTm.nDay;
					bBuf1[12] = tmTm.nHour;
					bBuf1[13] = tmTm.nMinute;
					bBuf1[14] = tmTm.nSecond;
					bBuf1[15] = 0x00;	//时间标签
					DTRACE(DB_METER, ("OneAddrBroadcast_485 MtrPro=%s::Point=%d,write id=0x40000200.\r\n", szProName,pMtrPara->wPn)); 
			
					iStep = Make69845Frm_485(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], 0x43, 0, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);
					iRet = pMtrPro->pfnWriteItem(pMtrPro, dwOAD, 0x40000200, iStep);
					//if (iRet < 0)
					//	return -1;
					
				}
				memset(bBuf1, 0, sizeof(bBuf1));
				iRet = AskMtrItem(pMtrPro, bRespType, dwOAD, bBuf1, pbRSD, bLenRSD, pbRCSD, bLenRCSD);
				if (iRet > 0)
				{
					tmMtr.nYear = OoLongUnsignedToWord(&bBuf1[1]);
					tmMtr.nMonth = bBuf1[3];
					tmMtr.nDay= bBuf1[4];
					tmMtr.nHour= bBuf1[5];
					tmMtr.nMinute= bBuf1[6];
					tmMtr.nSecond= bBuf1[7];

					GetCurTime(&tmTm);
					dwSecM = TimeToSeconds(tmMtr);
					dwSecT = TimeToSeconds(tmTm);
					DWORD dwDel;
					BYTE bDel;
					if (dwSecT > dwSecM)
					{
						dwDel = dwSecT - dwSecM;
						if (dwDel > 127)
							bDel = 127;
						else
							bDel = dwDel;
						g_MtrClkPrg.bClkErr[1] =(bDel^0xff) + 1;//正数的负数是该正数按位取反再加1
					}
					else
					{
						dwDel = dwSecM - dwSecT;
						if (dwDel > 127)
							bDel = 127;
						else
							bDel = dwDel;
						g_MtrClkPrg.bClkErr[1] = bDel;//电表快为正
					}
				}
				else
					g_MtrClkPrg.bClkErr[1] = 1;//如果读不回时间就给个1～2秒左右的误差值

				SetInfo(INFO_TERM_MTRCLKPRG);

				return 1;
			}
				
		}
	}
	return -1;
}
extern WORD CheckCRC16(BYTE *pInBuf, int iInLen);

int Make69845Frm_485(BYTE *pbMtr, BYTE bMtrLen, BYTE bCtrl, BYTE bAFType, BYTE bCA, WORD wFramFmt, BYTE *pbAPDU, WORD wAPDULen, BYTE *pbRespBuf)
{
	WORD wLen;
	BYTE *p = pbRespBuf;
	BYTE *pHeadCs;	//帧头校验	

	//--------------帧头--------------
	*p++ = 0x68;
	p += 0x02;
	*p++ = bCtrl;
	*p++ = ((bMtrLen-1)&0x0f) | (bAFType<<6);	
	revcpy(p, pbMtr, bMtrLen);
	//memcpy(p, pbMtr, bMtrLen);
	p += bMtrLen;
	*p++ = bCA;
	pHeadCs = p;
	p += 0x02;

	//-----------链路用户数据-----------
	if (bCtrl & 0x20)	//启动标志位PRM
	{
		WordToByte(wFramFmt, p);
		p += 2;
	}
	memcpy(p, pbAPDU, wAPDULen);
	p += wAPDULen;

	//-----------计算数据长度------------
	wLen = (p - pbRespBuf) + 0x02 - 0x01;
	WordToByte(wLen, &pbRespBuf[1]);

	//-----------帧头校验------------
	WordToByte(CheckCRC16(pbRespBuf+0x01, pHeadCs-pbRespBuf-0x01), pHeadCs);

	//-----------帧校验------------
	WordToByte(CheckCRC16(pbRespBuf+0x01, p-pbRespBuf-0x01), p);
	p += 2;
	*p++ = 0x16;

	return p - pbRespBuf;
}

TTime tLastTime[31];//用于记录各端口广播校时和单地址校时的执行时间
//tLastTime[30]-记录载波单地址
//tLastTime[15]-记录载波广播
//tLastTime[port1],tLastTime[port[port2],485 I和485-II的单地址校时
//tLastTime[16+port1],tLastTime[port[16+port2],485 I和485-II的广播校时

extern TMtrPara g_MtrPara[LOGIC_PORT_NUM];
extern const BYTE* Get485PnMask( );
extern bool GetMeterPara(WORD wPn, TMtrPara* pMtrPara);
extern WORD SearchPnFromMask(const BYTE* pbPnMask, WORD wStartPn);
//485单地址广播校时处理总接口
int MtrBroadcast_485(BYTE bThrId)
{
	TRdItem tRdItem;
	int m_iPn = 0;
	int iRet;
	int iApduLen;
	int iLen69845;
	int iRespLen;;
	BYTE bRespData[512];
	BYTE bCn = 0;
	BYTE bCheckCnt = 0;
	WORD wPlcNum;
	TTime tmTm;
	BYTE bBuf1[100];
	int iStep = -1;
	BYTE bPort;
	
	if (GetInfo(INFO_ONE_BRAODCAST_ARG_485))
	{
		DTRACE(DB_METER, ("INFO_ONE_BRAODCAST_ARG_485 \r\n")); 
		memset((BYTE *)&tLastTime[bThrId], 0, sizeof(TTime));
		SetInfo(INFO_ONE_BRAODCAST_ARG_CCT);
	}
	bPort = LOGIC_PORT_MIN + bThrId;
	if (OoProReadAttr(0x4204, 3, 0, bBuf1, sizeof(bBuf1), &iStep) > 0)
	{
		if (bBuf1[9] == 1)//启用
		{
			GetCurTime(&tmTm);
			if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[5]*3600+bBuf1[6]*60+bBuf1[7]))
				return -1;

			if (tmTm.nYear == tLastTime[bThrId].nYear && tmTm.nMonth== tLastTime[bThrId].nMonth && tmTm.nDay== tLastTime[bThrId].nDay)//一天只执行一次
				return -1;
			DTRACE(DB_METER, ("star 485 mtrBroadcast \r\n")); 
			tLastTime[bThrId] = tmTm;
			memset((BYTE*)&tRdItem, 0, sizeof(tRdItem));
			tRdItem.bReqType = 1;

			const BYTE* pbPnMask = Get485PnMask();	//取得非载波的485屏蔽位.
			while (1)
			{
				if (pbPnMask == NULL)
				{
					Sleep(500);
					break;
				}
				m_iPn = SearchPnFromMask(pbPnMask, m_iPn);	//这里搜出的测量点都是485的
				if (m_iPn >= POINT_NUM)
				{
					//Sleep(500);
					break;
				}
				if (GetPnPort(m_iPn) != bPort) //不是本端口的电能表
				{
					m_iPn++;
					continue;
				}

				GetMeterPara(m_iPn, &g_MtrPara[bThrId]);
				TMtrPro * pMtrPro = CreateMtrPro(m_iPn, &g_MtrPara[bThrId], bThrId);

				tRdItem.dwOAD = 0x40000200;
				tRdItem.wRsdLen = 0;
				tRdItem.wRcsdLen = 0;
				tRdItem.bReqType = 1;
				memset(tRdItem.bRSD, 0, 50);
				memset(tRdItem.bRCSD, 0, 50);
				iRet = AskMtrItem(pMtrPro, tRdItem.bReqType, tRdItem.dwOAD, bRespData, tRdItem.bRSD, tRdItem.wRsdLen, tRdItem.bRCSD, tRdItem.wRcsdLen);
				if (iRet > 0)
				{
					OneAddrBroadcast_485(pMtrPro, tRdItem.bReqType, tRdItem.dwOAD, bRespData, tRdItem.bRSD, tRdItem.wRsdLen, tRdItem.bRCSD, tRdItem.wRcsdLen);
				}
				m_iPn++;
				if (m_iPn >= POINT_NUM)
				{
					Sleep(500);
				}

			}
		}
	}

	return 0;
}
extern DWORD GbValToBaudrate(BYTE val);
int BroadcastAdjustTime_485(BYTE bThrId)
{
	TRdItem tRdItem;
	int m_iPn = 0;
	int iRet;
	int iApduLen;
	int iLen69845;
	int iRespLen;;
	BYTE bCn = 0;
	BYTE bCheckCnt = 0;
	WORD wPlcNum;
	TTime tmTm;
	BYTE bBuf1[100];
	int iStep = -1;
	BYTE bPort;
	
	if (GetInfo(INFO_MTR_BRAODCAST_ARG_485))
	{
		DTRACE(DB_METER, ("INFO_MTR_BRAODCAST_ARG_485 \r\n")); 
		memset((BYTE *)&tLastTime[16], 0, sizeof(TTime));//随便哪个485线程收到了就把485的时间都清了
		memset((BYTE *)&tLastTime[16+bThrId], 0, sizeof(TTime));
		//memset((BYTE *)&tLastTime[16+bThrId+1], 0, sizeof(TTime));
		//memset((BYTE *)&tLastTime[16+bThrId+2], 0, sizeof(TTime));
	}
	
	const BYTE* pbPnMask = Get485PnMask();	//取得非载波的485屏蔽位.
	if (pbPnMask == NULL)
	{
		return -1;
	}
	m_iPn = SearchPnFromMask(pbPnMask, m_iPn);	//这里搜出的测量点都是485的
	if (m_iPn >= POINT_NUM)
	{
		return -1;
	}
	
	bPort = LOGIC_PORT_MIN + bThrId;
	//对于广播命令只能是按645表的发一次再按对象表的发一次
	memset(bBuf1, 0, sizeof(bBuf1));
	tRdItem.bReqType = 1;
	tRdItem.dwOAD = 0x40000200;
	BYTE bBufY[18]={0x68, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x68, 0x08, 0x06, 0x01, 0x02, 0x03, 0x07, 0x03, 0x17, 0x5E, 0x16};
	if (OoProReadAttr(0x4204, 2, 0, bBuf1, sizeof(bBuf1), &iStep) > 0)
	{
		if (bBuf1[7] == 1)//启用
		{
			GetCurTime(&tmTm);
			if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[3]*3600+bBuf1[4]*60+bBuf1[5]))
				return -1;

			if (tmTm.nYear == tLastTime[16+bThrId].nYear && tmTm.nMonth== tLastTime[16+bThrId].nMonth && tmTm.nDay== tLastTime[16+bThrId].nDay)//一天只执行一次
				return -1;
			
			memcpy( (BYTE *)&tLastTime[16+bThrId].nYear,(BYTE *)&tmTm, sizeof(TTime));
			TMtrPara tMtrPara;
			tMtrPara.CommPara.wPort = MeterPortToPhy(bPort);
			tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(2);//1200;
			tMtrPara.CommPara.bByteSize = 8;
			tMtrPara.CommPara.bStopBits = ONESTOPBIT;
			tMtrPara.CommPara.bParity = EVENPARITY;
			tMtrPara.bProId = PROTOCOLNO_DLT645;//广播校时97和07一样的,但波特率不一样，
			memset(tMtrPara.bAddr, 0x99, 6);
			TMtrPro * pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
			//if (bProId == PROTOCOLNO_DLT645_V07 || bProId == PROTOCOLNO_DLT645)// 07表校时是分两次操作的
			{
				bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
				bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
				bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
				bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
				bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
				bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
				bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);
				memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
				DTRACE(DB_CCT, ("bPort %d Broadcast 645-1200 \r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x04000101, sizeof(bBufY));
				//按1200波特率再发一次
				GetCurTime(&tmTm);
				tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(3);//2400;
				bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
				bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
				bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
				bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
				bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
				bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
				bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);
				pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
				memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
				DTRACE(DB_CCT, ("bPort %d Broadcast 645-1200 \r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x04000101, sizeof(bBufY));
			}
			//if (bProId == PROTOCOLNO_DLT69845)//对象表的广播校时，地址按多少来做???????
			{
					bBuf1[0] = 7;//ACTION_REQ
					bBuf1[1] = DL69845_APPSVR_GETREQUEST_NORMAL;	//GET-Request-NORMAL
					bBuf1[2] = 0; //PIID
					bBuf1[3] = 0x40;
					bBuf1[4] = 0x00;
					bBuf1[5] = 0x7f;// 方法127
					bBuf1[6] = 0x00;
					bBuf1[7] = 0x1c;
					bBuf1[8] = tmTm.nYear>>8;
					bBuf1[9] = tmTm.nYear & 0x00ff;
					bBuf1[10] = tmTm.nMonth;
					bBuf1[11] = tmTm.nDay;
					bBuf1[12] = tmTm.nHour;
					bBuf1[13] = tmTm.nMinute;
					bBuf1[14] = tmTm.nSecond;
					bBuf1[15] = 0x00;	//时间标签
			
					bBufY[0] = 0xC1;
					bBufY[1] = 0xAA;
					tMtrPara.bProId = PROTOCOLNO_DLT69845;
					GetCurTime(&tmTm);
					bBuf1[14] = tmTm.nSecond;
					tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(2);
					pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
					iStep = Make69845Frm_485(bBufY+1, 1, 0x43, 3, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);//意思是广播时的地址域为：  C1 AA
					DTRACE(DB_CCT, ("bPort %d Broadcast 69845-1200\r\n",bPort)); 
					iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);

					GetCurTime(&tmTm);
					bBuf1[14] = tmTm.nSecond;
					tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(3);
					pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
					iStep = Make69845Frm_485(bBufY+1, 1, 0x43, 3, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);
					DTRACE(DB_CCT, ("bPort %d Broadcast 69845-2400\r\n",bPort)); 
					iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);

					GetCurTime(&tmTm);
					bBuf1[14] = tmTm.nSecond;
					tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(4);
					pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
					iStep = Make69845Frm_485(bBufY+1, 1, 0x43, 3, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);
					DTRACE(DB_CCT, ("bPort %d Broadcast 69845-4800\r\n",bPort)); 
					iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);

					GetCurTime(&tmTm);
					bBuf1[14] = tmTm.nSecond;
					tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(6);
					pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
					iStep = Make69845Frm_485(bBufY+1, 1, 0x43, 3, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);
					DTRACE(DB_CCT, ("bPort %d Broadcast 69845-9600\r\n",bPort)); 
					iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);

			}
		}
	}
	return 0;
}


