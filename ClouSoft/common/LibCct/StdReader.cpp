/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CctRdCtrl.cpp
 * 摘    要：载波抄表控制管理
 * 当前版本：1.0
 * 作    者：CL
 * 完成日期：2016年8月
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaAPI.h"
#include "StdReader.h"
#include "ComAPI.h"
#include "LibDbConst.h"
#include "Trace.h"
#include "DbFmt.h"
#include "DbOIAPI.h"
#include "OIObjInfo.h"
#include "MtrHook.h"
#include "MtrCtrl.h"
#include "TaskManager.h"
#include "MeterAPI.h"
#include "DbCctAPI.h"
#include "DL69845.h"
#include "DL645V07.h"
#include "DL645Ext.h"

extern TMtrClkPrg g_MtrClkPrg;
extern WORD DL645MakeFrm(BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen);

typedef int (CStdReader::*pFun)(TMtrRdCtrl*, TRdItem*, TMtrPara*, BYTE*, WORD); 

CStdReader::CStdReader()
{
	m_semReader = NewSemaphore(1);
	m_pszName = "Unknow module";
	memset(&m_RtRunMdInfo, 0x00, sizeof(m_RtRunMdInfo));
	memset((BYTE*)&m_TRunStateInfo, 0, sizeof(m_TRunStateInfo));
	memset((BYTE*)&m_TRtStat, 0, sizeof(m_TRtStat));

    GetRooterTermAddr(m_TRtStat.bTermAddr,m_TRtStat.bTermLen);

	GetCurTime(&m_TRunStateInfo.tLastUdpTime);	
	
	m_fRxComlpete = false;

	m_iPn = -1;
	m_dwLastWaitSec = 0;
}

void CStdReader::Init()
{
	InitRcv();
	InitPhyPort();
}
void CStdReader::InitRcv()
{
	m_LoopBuf.ClrBuf();
	m_LoopBuf.Init(1024);
}

void CStdReader::InitPhyPort()
{
	bool fOpenOK = m_Comm.Open(COMM_CCT_PLC, CBR_9600, 8, ONESTOPBIT, EVENPARITY);
	if (!fOpenOK)
		DTRACE(DB_CCT, ("CStdReader::Init fail to open COMM%d for StdReader.\n", COMM_CCT_PLC));
	else
		DTRACE(DB_CCT, ("CStdReader::Init succ to open COMM%d for StdReader.\n", COMM_CCT_PLC));
}

void CStdReader::ClearRcv()
{
	m_LoopBuf.ClrBuf();
}

void CStdReader::LockReader() 
{ 
	WaitSemaphore(m_semReader); 
}

void CStdReader::UnLockReader() 
{ 
	SignalSemaphore(m_semReader); 
}

void CStdReader::LockDirRd()
{
	//获取端口权限
	m_TRunStateInfo.fDirRdFlg = true;
	LockReader();
	if (!m_TRunStateInfo.fRtPause)
	{
		m_TRunStateInfo.fRtPause = true;
		Afn12Fn02_RtPause();
	}
}

void CStdReader::UnLockDirRd()
{
	//释放端口权限
	m_TRunStateInfo.fDirRdFlg = false;
	//路由恢复，由CctRunStateMonitor来恢复
	// 	if (m_TRunStateInfo.fRtPause)
	// 	{
	// 		m_TRunStateInfo.fRtPause = false;	//恢复路由
	// 		Afn12Fn03_RtResume();
	// 	}
	UnLockReader();
}


//描述：初始化路由模块
//参数：@bFn F1-硬件初始化，F2-参数区初始化，F3-数据区初始化
//返回：是否初始化成功
bool CStdReader::InitRouter(BYTE bFn)
{
	WORD wMtrSn = 0;
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;

	if (bFn == FN(1))
		DTRACE(DB_CCT, ("AFN01-F1: hardware reset.\n"));
	else if (bFn == FN(2))
		DTRACE(DB_CCT, ("AFN01-F2: parameter reset.\n"));
	else if (bFn == FN(3))
		DTRACE(DB_CCT, ("AFN01-F3: data reset.\n"));
	else
	{
		DTRACE(DB_CCT, ("AFN01-F%d: unsupport!\n", bFn));
		return false;
	}
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_INIT, bFn, NULL, 0, bTxBuf);
	ClearRcv(); //清空缓冲区，避免收到错误的帧

	for (BYTE bTryCnt = 0; bTryCnt < 2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_CON && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					return true;
				}
			}
		}
	}

	DTRACE(DB_CCT, ("CStdReader::InitRouter: AFN=01H-F%d fail!\n", bFn));

	return false;
}

//描述：获取载波初始化状态
bool CStdReader::GetInitState()
{
	return m_TRtStat.fPlcInit;
}

//描述：硬件初始化
bool CStdReader::Afn01Fn01_HardwareInit()
{
	return InitRouter(FN(1));
}

//描述：参数初始化（恢复出厂设置）
bool CStdReader::Afn01Fn02_ParmInit()
{
	return InitRouter(FN(2));
}

//描述：数据区初始化
bool CStdReader::Afn01Fn03_DataInit()
{
	return InitRouter(FN(3));
}

//描述：对路由上报的模块信息处理
void  CStdReader::Afn03Fn10_RptRtRunInfo(BYTE* pbBuf)
{
	BYTE bBuf[32] = {0};
	if(pbBuf == NULL)
		return ;

	memset(&m_RtRunMdInfo, 0x00, sizeof(m_RtRunMdInfo));
	DTRACE(DB_CCT, ("/*************************************************************/\r\n"));
	DTRACE(DB_CCT, ("CStdReader :: RtRunModResolve Start.\r\n"));
	m_RtRunMdInfo.bModType = pbBuf[0] & 0x0f;  //通信方式
	m_RtRunMdInfo.bRtType = (pbBuf[0]&0x10)>>4;   //路由管理方式
	m_RtRunMdInfo.bNodeType = (pbBuf[0]&0x20)>>5; //从节点信息模式
	m_RtRunMdInfo.bRdType = (pbBuf[0]&0xc0)>>6;   //周期抄表模式

	DTRACE(DB_CCT, ("Router Module Type:%d-%s.\r\n",m_RtRunMdInfo.bModType, (m_RtRunMdInfo.bModType==CCT_RD_TYPE_PLC ? "PLC" : "Radio")));
	DTRACE(DB_CCT, ("Router Manage Type:%d-%s.\r\n",m_RtRunMdInfo.bRtType, (m_RtRunMdInfo.bRtType==0 ? "No Route" : "Route Manage Module-Self")));
	DTRACE(DB_CCT, ("Router Node Type:%d-%s.\r\n",m_RtRunMdInfo.bNodeType, (m_RtRunMdInfo.bNodeType==0 ? "not download Node" : "Need download Node to Module")));
	DTRACE(DB_CCT, ("Router Read Meter Type:%d-%s \r\n",m_RtRunMdInfo.bRdType, (m_RtRunMdInfo.bRdType==CCT_RD_BY_TERM ? "Term initiate Read Meter" : "Module initiate Read Meter")));

	if(m_RtRunMdInfo.bRdType == CCT_RD_TERM_ROUTE)  //如果两种都支持，暂只使用路由主导的吧
		m_RtRunMdInfo.bRdType = CCT_RD_BY_ROUTE; 
	m_RtRunMdInfo.bTansTmOut = (pbBuf[1] & 0x07);  //传输延时参数支持
	m_RtRunMdInfo.bRdFailType = ((pbBuf[1]&0x18)>>3); //失败节点切换发起方式
	m_RtRunMdInfo.bBdCfmType = (pbBuf[1]&0x20)>>5;  //广播命令确认方式
	m_RtRunMdInfo.bBdChnlType = (pbBuf[1]&0xc0)>>6; //广播命令信道执行方式
	m_RtRunMdInfo.bChnlNum = (pbBuf[2] & 0x1f); //信道数量，低5位  
	m_RtRunMdInfo.bVolMis = (pbBuf[2]&0xe0)>>3;   //低压电网掉电信息，高3位
	m_RtRunMdInfo.bVolMis = pbBuf[3] & 0x0f;   //速率数量，低4位
	m_RtRunMdInfo.bNodeTmOut = pbBuf[6];
	m_RtRunMdInfo.wBdTmOut  = ByteToWord(&pbBuf[7]);
	m_RtRunMdInfo.wFrmMaxLen  = ByteToWord(&pbBuf[9]);
	m_RtRunMdInfo.wTansMaxLen  = ByteToWord(&pbBuf[11]);
	m_RtRunMdInfo.bUpdateTmOut  = pbBuf[13];

	memcpy(m_RtRunMdInfo.bMainAdd, &pbBuf[14], 6);
	m_RtRunMdInfo.wMaxNodeNum = ByteToWord(&pbBuf[20]);
	m_RtRunMdInfo.wNodeNum = ByteToWord(&pbBuf[22]);
	memcpy(m_RtRunMdInfo.bProRelsDate, &pbBuf[24], 3);
	memcpy(m_RtRunMdInfo.bProRecdDate, &pbBuf[27], 3);
	memcpy(m_RtRunMdInfo.bFacCode, &pbBuf[30], 9);

	DTRACE(DB_CCT, ("Router Main Note Addr %02x%02x%02x%02x%02x%02x .\r\n", pbBuf[19],pbBuf[18],pbBuf[17],pbBuf[16],pbBuf[15],pbBuf[14]));
	DTRACE(DB_CCT, ("Router Storage Node Max Num:%d, Downloaded Node Num:%d.\r\n", m_RtRunMdInfo.wMaxNodeNum, m_RtRunMdInfo.wNodeNum));
	DTRACE(DB_CCT, ("Router Proto Release Date:%02x-%02x-%02x, Record Date:%02x-%02x-%02x.\r\n", pbBuf[24],pbBuf[25],pbBuf[26],pbBuf[27],pbBuf[28],pbBuf[29]));
	DTRACE(DB_CCT, ("Router Factory Code: %c%c .\r\n",pbBuf[30],pbBuf[31]));
	DTRACE(DB_CCT, ("Router Mode Version: %02x%02x, Date:%02x-%02x-%02x .\r\n",pbBuf[38], pbBuf[37],pbBuf[36],pbBuf[35],pbBuf[34]));
	DTRACE(DB_CCT, ("CStdReader :: RtRunModResolve End.\r\n"));
	DTRACE(DB_CCT, ("/*************************************************************/\r\n"));

	if(m_RtRunMdInfo.bChnlNum > 0)
		memcpy(m_RtRunMdInfo.wSpeedArr, &pbBuf[39], m_RtRunMdInfo.bChnlNum);

	m_RtRunMdInfo.bTrySendCnt = 3;
	if (pbBuf[31]=='T' && pbBuf[30]=='C')
	{
		m_RtRunMdInfo.bTrySendCnt = 1;
		m_RtRunMdInfo.bModule = AR_LNK_TC;
		m_pszName = "TC";
		m_TRtStat.fIsNeedRtReq = true;
	}
	else if (pbBuf[31]=='E' && pbBuf[30]=='S')
	{
		m_RtRunMdInfo.bTrySendCnt = 3;
		m_RtRunMdInfo.bModule = AR_LNK_ES;
		m_pszName = "ES";
		m_TRtStat.fIsNeedRtReq = true;
	}
	else if (pbBuf[31]=='0' && pbBuf[30]=='1')
	{
		m_RtRunMdInfo.bTrySendCnt = 1;
		m_RtRunMdInfo.bModule = AR_LNK_RSC;
		m_pszName = "RSC";
		m_TRtStat.fSyncAddr = true;
		m_TRtStat.fIsNeedRtReq = false;
	}
	else if (pbBuf[31]=='S' && pbBuf[30]=='L')
	{
		m_RtRunMdInfo.bTrySendCnt = 3;
		m_RtRunMdInfo.bModule = AR_LNK_LS;
		m_pszName = "LS";
		m_TRtStat.fIsNeedRtReq = true;
	}
	else if (pbBuf[31]=='D' && pbBuf[30]=='G')
	{
		m_RtRunMdInfo.bTrySendCnt = 3;
		m_RtRunMdInfo.bModule = AR_LNK_LS;
		m_pszName = "SGD";
		m_TRtStat.fIsNeedRtReq = true;
	}
	else if (pbBuf[31]=='Q' && pbBuf[30]=='J')
	{
		m_RtRunMdInfo.bTrySendCnt = 3;
		m_RtRunMdInfo.bModule = AR_LNK_GY;
		m_pszName = "GY";
		m_TRtStat.fIsNeedRtReq = true;
	}
	else 
	{
		m_RtRunMdInfo.bTrySendCnt = 1;
		m_RtRunMdInfo.bModule = AR_LNK_UNKNOW;
		m_pszName = "Unknow module";
		m_TRtStat.fSyncAddr = true;
		m_TRtStat.fIsNeedRtReq = true;
	}


	//m_TRtStat.fPlcInit = true;
	//m_fIsAutoReptVersion = true;
}

//描述：处理AFN=05 Fn=1上报从节点信息
bool CStdReader::Afn05Fn01_SetMainNodeAddr()
{
	BYTE bCtrl;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;
	BYTE bData[32];

	bCtrl = 0x41;

	bR[0] = 0; //D2通信模块标识：0表示对集中器的通信模块操作;1表示对载波表的通信模块操作。
	bR[1] = 0;		//信道
	bR[2] = 0xff;
	bR[3] = 0x0;
	bR[4] = 0;
	bR[5] = 0;

	DTRACE(DB_CCT, ("AFN05-F1: Set router addr.\n"));
	memset(bData, 0, sizeof(bData));
    if(IsAllAByte(m_TRtStat.bTermAddr, 0x00, 6))
    {
        GetRooterTermAddr(m_TRtStat.bTermAddr,m_TRtStat.bTermLen);
    }
	memcpy(bData, m_TRtStat.bTermAddr, 6);
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_CTRL, FN(1), bData, 6, bTxBuf);
	ClearRcv();
	for (BYTE bTryCnt = 0; bTryCnt < m_RtRunMdInfo.bTrySendCnt; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(m_RtRunMdInfo.bNodeTmOut/m_RtRunMdInfo.bTrySendCnt))
			{
				if (m_TRcv13762.bAfn==AFN_CON && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					return true;
				}
			}
		}
	}

	return false;
}

//描述：启动广播
bool CStdReader::Afn05Fn3_StartBoardCast(BYTE *pbReqBuf, WORD wLen)
{
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[128] = {0};
	BYTE wTxLen, wRxLen;
	BYTE *pbTxBuf = bTxBuf;

	bR[0] = 0x01;

	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_CTRL, FN(3), pbReqBuf, wLen, bTxBuf);

	for (BYTE bTryCnt=0; bTryCnt<2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_CON && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					WORD wWait = ByteToWord(m_TRcv13762.bDtBuf+4);
					if (wWait > 180)
						m_dwLastWaitSec = 180;
					else
						m_dwLastWaitSec = wWait;
					return true;
				}
			}
		}
	}

	DTRACE(DB_CCT, ("AFN05-F3: Start boardcast fail...\n"));
	return false;
}

//描述：处理AFN=6 Fn=1上报从节点信息
void CStdReader::Afn06Fn01_RptNodeInfo()
{

}

//描述：处理AFN=6 Fn=2上报抄读数据
bool CStdReader::Afn06Fn02_RptData()
{	
	TFrm69845 tRcvFrm;
	TMtrRdCtrl* pMtrRdCtrl;
	TRdItem tRptItem;
	WORD wDlyTime, wPn, w69845Len, wLen, wLen13762;
	WORD wDtLen = m_TRcv13762.wDtLen;
	BYTE *pbDt = m_TRcv13762.bDtBuf;
	BYTE *pb69845;
	BYTE bBuf[256];
	BYTE bFrm13762[64];
	char szTsa[16];
	BYTE bTsa[TSA_LEN] = {0};
	BYTE bR[6];

	pbDt++;	//从节点序号
	pbDt++;	//通信协议类型
	wDlyTime = ByteToWord(pbDt);	//当前报文本地通信上行时长
	pbDt += 2;
	pb69845 = pbDt;
	w69845Len =wDtLen - (pbDt - m_TRcv13762.bDtBuf);

	memset(szTsa, 0, sizeof(szTsa));
	memset((BYTE*)&tRcvFrm, 0, sizeof(tRcvFrm));
	if (DecodeFram69845(pb69845, w69845Len, &tRcvFrm))
	{
		memset(bBuf, 0, sizeof(bBuf));
		wPn = GetMeterPn(tRcvFrm.bSA, tRcvFrm.bSALen, true);	
		if (wPn <= 0)
		{
			DTRACE(DB_CCT, ("AFN06-F2: Report Invalid MTR=0x%s.\n", HexToStr(bTsa+1, 6, szTsa)));
		}
		else
		{
			bTsa[0] = 6;
			revcpy(bTsa+1, tRcvFrm.bSA, 6);
			pMtrRdCtrl = GetMtrRdCtrl(wPn, bTsa);
			if (pMtrRdCtrl == NULL)
			{
				DTRACE(DB_CCT, ("AFN06-F2: GetMtrRdCtrl() fail.\n"));
			}
			else
			{	memset((BYTE*)&tRptItem, 0, sizeof(tRptItem));
				wLen = DecodeReportApdu(tRcvFrm.bAPDUData, tRcvFrm.wAPDULen, &tRptItem, bBuf, true);
				if (wLen > 0)
					SaveMtrData(pMtrRdCtrl, tRptItem.bReqType, tRptItem.bCSD, bBuf, wLen);	//+2:跳过1字节DAR+记录条数
			}
			PutMtrRdCtrl(wPn, bTsa, true);
		}
	}

	memset(bR, 0, sizeof(bR));
	bR[0] = 0x04;	
	bR[1] |= m_TRcv13762.bR[1]&0x0f;
	bR[5] = m_TRcv13762.bR[5];
	memset(bFrm13762, 0, sizeof(bFrm13762));
	wLen13762 = Make1376_2Frm(bTsa+1, 6,  0x01, bR, AFN_CON, FN(1), NULL, 0, bFrm13762);
	Send(bFrm13762, wLen13762);
	memset((BYTE*)&m_TRcv13762, 0, sizeof(m_TRcv13762));

	return true;
}

//描述：处理AFN=6 Fn=3上报路由工况信息
void CStdReader::Afn06Fn03_RptRtInfo()
{
	char *pszState[2] = {"[Read meter complete.]", "[Search meter complete.]"};
	WORD wMtrSn = 0;
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bData[8] = {0};
	BYTE *pbData = bData;
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;

	bCtrl = 0x41;

	//bR[0] = 0x04;	
	bR[1] |= m_TRcv13762.bR[1]&0x0f;
	bR[5] = m_TRcv13762.bR[5];

	*pbData++ = 0xff;
	*pbData++ = 0xff;
	*pbData++ = 0xff;
	*pbData++ = 0xff;
	*pbData++ = 0x00;
	*pbData++ = 0x00;

	DTRACE(DB_CCT, ("AFN06-F3: Report router work state=%s.\n", pszState[m_TRcv13762.bDtBuf[0]-1]));

	if (m_TRcv13762.bDtBuf[0] == 2)
		m_fRptSchMtrEnd = true;	
	memset((BYTE*)&m_TRcv13762, 0, sizeof(m_TRcv13762));
}

//描述：处理AFN=6 Fn=4上报从节点信息及设备类型
void CStdReader::Afn06Fn04_RptMtrInfo()
{
	m_dwLastRptMtrClk = GetClick();
	SaveSchMtrResult(0xf2090201, m_TRcv13762.bDtBuf, m_TRcv13762.wDtLen);

	BYTE bR[6];
	BYTE bFrm13762[128];
	WORD wLen13762;

	memset(bR, 0, sizeof(bR));
	bR[0] = 0x04;	
	bR[1] |= m_TRcv13762.bR[1]&0x0f;
	bR[5] = m_TRcv13762.bR[5];

	memset(bFrm13762, 0, sizeof(bFrm13762));
	wLen13762 = Make1376_2Frm(m_TRcv13762.bSrcAddr, 6,  0x01, bR, AFN_CON, FN(1), NULL, 0, bFrm13762);

	Send(bFrm13762, wLen13762);

	memset((BYTE*)&m_TRcv13762, 0, sizeof(m_TRcv13762));
}

//描述：处理AFN=6 Fn=5上报从节点事件
//备注：浙江协要求跨台区搜表是通过AFN=06 F5上报
void CStdReader::Afn06Fn5_RptNodeEvt()
{
	m_dwLastRptMtrClk = GetClick();
	SaveCrossSchMtrResult(m_TRcv13762.bDtBuf, m_TRcv13762.wDtLen);

	BYTE bR[6];
	BYTE bFrm13762[128];
	WORD wLen13762;

	memset(bR, 0, sizeof(bR));
	bR[0] = 0x04;	
	bR[1] |= m_TRcv13762.bR[1]&0x0f;
	bR[5] = m_TRcv13762.bR[5];

	memset(bFrm13762, 0, sizeof(bFrm13762));
	wLen13762 = Make1376_2Frm(m_TRcv13762.bSrcAddr, 6,  0x01, bR, AFN_CON, FN(1), NULL, 0, bFrm13762);

	Send(bFrm13762, wLen13762);

	memset((BYTE*)&m_TRcv13762, 0, sizeof(m_TRcv13762));
}

//描述：读取路由模块载波节点数量
WORD CStdReader::Afn10Fn01_RdRtNodeNum()
{
	WORD wNodeNum, wSupportNum;
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[128] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;

	bR[0] = 0x01;
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_QRYRT, FN(1), NULL, 0, bTxBuf);
	ClearRcv(); //清空缓冲区，避免收到错误的帧

	for (BYTE bTryCnt=0; bTryCnt<2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_QRYRT && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					wNodeNum = ByteToWord(m_TRcv13762.bDtBuf);
					wSupportNum = ByteToWord(m_TRcv13762.bDtBuf+2);

					DTRACE(DB_CCT, ("CStdReader::ReadNodeNum:wNodeNum=%d, wSupportNum=%d\r\n", wNodeNum, wSupportNum));
					return wNodeNum;
				}
			}
		}
	}

	return -1;
}

//描述：读取从节点信息 AFN=10H-F2
//参数：@bRdMtrNum 一次性读取电表个数
//		@wStartSn 开始序号
//		@pbOutBuf 返回数据,【电表地址6字节+从节点信息2字节】
//返回：读取到的电表有效个数
int CStdReader::Afn10Fn02_RdNodeInfo(BYTE bRdMtrNum, WORD wStartSn, BYTE *pbOutBuf)
{
	WORD wSlaveMtrTotal = 0;
	BYTE bRespMtrCnt = 0;
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bData[16] = {0};
	BYTE bTxBuf[64] = {0};
	BYTE wTxLen, wRxLen;
	BYTE *pbTxBuf = bTxBuf;
	BYTE *pbData = bData;
	char szTsa[TSA_LEN];

	bR[0] = 0x01;

	*pbData++ = wStartSn%256;	
	*pbData++ = wStartSn/256;	
	*pbData++ = bRdMtrNum;	

	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_QRYRT, FN(2), bData, pbData-bData, bTxBuf);
	ClearRcv(); //清空缓冲区，避免收到错误的帧

	for (BYTE bTryCnt = 0; bTryCnt < 2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_QRYRT && DtToFn(m_TRcv13762.bDt)==FN(2))
				{
					BYTE *pbFmtPtr = m_TRcv13762.bDtBuf;

					wSlaveMtrTotal = ByteToWord(pbFmtPtr);	pbFmtPtr += 2;
					bRespMtrCnt = *pbFmtPtr;	pbFmtPtr++;
					for (BYTE i = 0; i < bRespMtrCnt; i++)
					{
						BYTE bMtrProType = 0;
						BYTE bMtr[6] = {0};

						memcpy(bMtr, pbFmtPtr, 6);	pbFmtPtr += 6;

						pbFmtPtr ++;
						bMtrProType = (*pbFmtPtr++>>3)&0x07;
						memset(szTsa, 0, sizeof(szTsa));
						DTRACE(DB_CCT, ("Read router Mtr addr:%s, MtrPro:%d.\n", HexToStr(bMtr, 6, szTsa, true), bMtrProType));
					}

					memcpy(pbOutBuf,  m_TRcv13762.bDtBuf,  m_TRcv13762.wDtLen);

					return bRespMtrCnt;
				}
			}
		}
	}

	return -1;
}

//描述：查询路由的运行状态
int CStdReader::Afn10Fn04_QueryRtRunInfo(BYTE *pbRespBuf)
{
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;
	BYTE bMtrNum, bMtrPro;

	bR[0] = 0x01;
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_QRYRT, FN(4), NULL, 0, bTxBuf);

	for (BYTE bTryCnt=0; bTryCnt<2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_QRYRT && DtToFn(m_TRcv13762.bDt)==FN(4))
				{
					memcpy(pbRespBuf, m_TRcv13762.bDtBuf, m_TRcv13762.wDtLen);

					return m_TRcv13762.wDataLen;
				}
			}
		}
	}

	DTRACE(DB_CCT, ("AFN10-F4: Query router run info fail.\n"));

	return -1;
}

//描述：添加载波测量点到路由模块 AFN=11H-F1
//参数：@pbInBuf 电表档案信息【从节点数量+[地址6字节+协议类型1字节]+[...]+...】
//		@bInLen 数据长度
//返回：是否添加成功
bool CStdReader::Afn11Fn01_AddNode(BYTE *pbInBuf, BYTE bInLen)
{
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;
	BYTE bMtrNum, bMtrPro;
	BYTE *pbPtr = pbInBuf;
	char szTsa[TSA_LEN];

	bMtrNum = *pbPtr++;
	for (BYTE i=0; i<bMtrNum; i++)
	{
		memset(szTsa, 0, sizeof(szTsa));
		DTRACE(DB_CCT, ("Add Mtr addr: %s, MtrPro:%d.\n",  HexToStr(pbPtr, 6, szTsa, true), pbPtr[6]));
		pbPtr += 7;
	}

	bR[0] = 0x01;
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_SETRT, FN(1), pbInBuf, bInLen, bTxBuf);

	for (BYTE bTryCnt=0; bTryCnt<2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_CON && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					DTRACE(DB_CCT, ("CStdReader::Afn11Fn01_AddNode: add meter succ.\n"));
					return true;
				}
			}
		}
	}

	DTRACE(DB_CCT, ("CStdReader::Afn11Fn01_AddNode: add meter fail.\n"));
	return false;
}

//描述：删除载波测量点到路由模块 AFN=11H-F2
//参数：@pbInBuf 【从节点数量+[表地址6字节]+...】
//		@bInLen 数据长度
int CStdReader::Afn11Fn02_DelNode(BYTE *pbInBuf, BYTE bInLen)
{
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;
	BYTE *pbInBuf0 = pbInBuf;
	BYTE bNum;

	bNum = *pbInBuf++;
	for (BYTE i=0; i<bNum; i++)
	{
		DTRACE(DB_CCT, ("Delete Mtr addr: %02x%02x%02x%02x%02x%02x.\n", \
					pbInBuf[0],pbInBuf[1],pbInBuf[2],pbInBuf[3],pbInBuf[4],pbInBuf[5]));
		pbInBuf += 6;
	}

	bR[0] = 0x01;
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_SETRT, FN(2), pbInBuf0, bInLen, bTxBuf);

	for (BYTE bTryCnt=0; bTryCnt<2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_CON && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CStdReader::Afn11Fn05_ActSlaveNodeRpt(BYTE *pbBuf, BYTE bLen)
{
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[128] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;

	bR[0] = 0x01;
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_SETRT, FN(5), pbBuf, bLen, bTxBuf);

	for (BYTE bTryCnt=0; bTryCnt<2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_CON && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CStdReader::Afn11Fn06_StopSlaveNodeRpt()
{
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[128] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;

	bR[0] = 0x01;
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_SETRT, FN(6), NULL, 0, bTxBuf);

	for (BYTE bTryCnt=0; bTryCnt<2; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_CON && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CStdReader::RtCtrl(BYTE bFn)
{
	BYTE bCtrl = 0x41;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[64] = {0};
	BYTE bRxBuf[128] = {0};
	BYTE wTxLen, wRxLen;
	BYTE *pbTxBuf = bTxBuf;

	if (bFn == FN(1))
	{
		DTRACE(DB_CCT, ("AFN12-F1: Router restart.\n"));
	}
	else if (bFn == FN(2))
	{
		DTRACE(DB_CCT, ("AFN12-F2: Router pause.\n"));
	}
	else if (bFn == FN(3))
	{
		DTRACE(DB_CCT, ("AFN12-F3: Router resume.\n"));
	}
	else
	{
		DTRACE(DB_CCT, ("AFN12-F%d: unsupport!\n", bFn));
		return false;
	}


	bR[0] = 0x01;
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_CTRLRT, bFn, NULL, 0, bTxBuf);

	ClearRcv(); //清空缓冲区，避免收到错误的帧

	for (BYTE bTryCnt=0; bTryCnt<2; bTryCnt++)
	{
		DWORD dwLastClick = GetClick();

		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
GOTO_RXHANDLEFRM:
			if (RxHandleFrm(1, false))
			{
				if (m_TRcv13762.bAfn==AFN_CON && DtToFn(m_TRcv13762.bDt)==FN(1))
					return true;
				if (GetClick()-dwLastClick <= 2)	//2s
					goto GOTO_RXHANDLEFRM;
			}
			memset((BYTE*)&m_TRcv13762, 0, sizeof(m_TRcv13762));
		}
	}

	DTRACE(DB_CCT, ("CStdReader::InitRouter: AFN=01H-F%d fail!\n", bFn));

	return false;
}

//描述：控制路由重启
bool CStdReader::Afn12Fn01_RtRestart()
{
	m_TRunStateInfo.fRtPause = false;	//重启路由时，必须清掉路由暂停标识

	return RtCtrl(FN(1));
}

//描述：控制路由暂停
bool CStdReader::Afn12Fn02_RtPause()
{
	return RtCtrl(FN(2));
}

//描述：控制路由恢复
bool CStdReader::Afn12Fn03_RtResume()
{
	return RtCtrl(FN(3));
}

//描述：路由数据转发类 AFN=13H-F1
//参数：@pbTsa 表地址
//		@bTsaLen 表地址长度
//		@pbInbuf 为F1的数据类容
//		@wInLen 为F1的数据长度
//		@pbOutBuf 为F1的抄读内容
//		@bProId 为协议类型
//返回：返回数据长度
int CStdReader::Afn13Fn01_RtFwd(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbInBuf, WORD wInLen, TMtrRdCtrl *pMtrRdCtrl, TRdItem *pRdItem, BYTE *pbOutBuf, BYTE bProId, bool fAnaly645data, bool fIs645Proxy)
{
	BYTE bCtrl;
	//BYTE bRevTsa[TSA_LEN];
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[1024] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;
	DWORD dwLastSendClick;

	//revcpy(bRevTsa, pbTsa, bTsaLen);
	//if (memcmp(pbTsa, m_TRunStateInfo.bRdFailTsa, 6) != 0)
	{
		bCtrl = 0x41;
		bR[0] = (1<<2); //D2通信模块标识：0表示对集中器的通信模块操作;1表示对载波表的通信模块操作。
		bR[1] = 0;		//信道
		bR[2] = 0xff;
		bR[3] = 0x0;
		bR[4] = 0;
		bR[5] = 0;
		wTxLen = Make1376_2Frm(pbTsa, bTsaLen, bCtrl, bR, AFN_RTFWD, FN(1), pbInBuf, wInLen, bTxBuf);

		for (BYTE bTryCnt=0; bTryCnt<m_RtRunMdInfo.bTrySendCnt; bTryCnt++)
		{
			dwLastSendClick = GetClick();
			if (Send(bTxBuf, wTxLen) == wTxLen)
			{
GOTO_RxHandleFrm:
				//if (RxHandleFrm(m_RtRunMdInfo.bNodeTmOut))
				if (RxHandleFrm(1))
				{
					if (m_TRcv13762.bAfn==AFN_RTFWD && DtToFn(m_TRcv13762.bDt)==FN(1))
					{
						int iRet;
						WORD wRcvFrmLen;
						BYTE *pData = m_TRcv13762.bDtBuf;
						//BYTE bPreCnt;

						pData += 3;	//2字节当前报文本地通信上行时长 + 1字节通信协议类型
						wRcvFrmLen = *pData++;
						if (wRcvFrmLen > 0)
						{
							if (bProId == PROTOCOLNO_DLT69845)
							{
								TFrm69845 tRcvFmt;
								memset((BYTE*)&tRcvFmt, 0, sizeof(tRcvFmt));
								if (DecodeFram69845(pData, wRcvFrmLen, &tRcvFmt))
								{
									TRdItem tRptItem;
									memset((BYTE*)&tRptItem, 0, sizeof(tRptItem));
									DecodeReportApdu(tRcvFmt.bAPDUData, tRcvFmt.wAPDULen, &tRptItem, pbOutBuf);
									if (tRcvFmt.bSALen==bTsaLen && 
											memcmp(tRcvFmt.bSA, pbTsa, 6)==0)	
									{
										if (pRdItem->bReqType == GET_REQUEST_NORMAL)
											iRet = GetResponseNormal(pRdItem->dwOAD, tRcvFmt.bAPDUData, tRcvFmt.wAPDULen, pbOutBuf);
										else
											iRet = GetResponseRecord(pRdItem->dwOAD, tRcvFmt.bAPDUData, tRcvFmt.wAPDULen, pRdItem->bRCSD, pRdItem->wRcsdLen, pbOutBuf);

										if (iRet < 0)
										{
											if ((pRdItem->dwOAD&0xfff0ffff)==0x50000200)	//冻结数据
												return -1;
										}

										if (iRet <= 0)
											goto RET_RTFWD;
										return iRet;
									}
								}
							}
							else if (bProId == PROTOCOLNO_DLT645 || bProId == PROTOCOLNO_DLT645_V07)
							{
								if (!fAnaly645data)
								{
									DWORD dwOAD;

									if (fIs645Proxy)
										dwOAD = pRdItem->dwOAD;
									else
										dwOAD = OoOadToDWord(&pRdItem->bRCSD[1+pRdItem->wRcsdIdx*5+1]);
									iRet = GetDL645_9707DataVal(pData, wRcvFrmLen, bProId, 0, dwOAD, pbOutBuf, pRdItem);
									if (iRet < 0)
										goto RET_RTFWD;
								}
								else
								{
									for (BYTE i=0; i<wRcvFrmLen; i++)
									{
										if (pData[0]==0x68 && pData[7]==0x68 && pData[wRcvFrmLen-i-1]==0x16)
										{
											if (pData[9] > 4)
											{
												iRet = pData[9] - 4;
												for (BYTE j=0; j<iRet; j++)
													pbOutBuf[j] = pData[9+5+j]-0x33;
											}
											else
												iRet = -1;

											break;
										}
										pData++;
									}
								}

								return iRet;
							}
							else if (bProId == PROTOCOLNO_SBJC)
							{
								iRet = GetDL645_EXTDataVal(pData, wRcvFrmLen, bProId, 0, pRdItem->dwOAD, pbOutBuf);
								if (iRet <= 0)
									goto RET_RTFWD;
								return iRet;
							}
						}
						else if (wRcvFrmLen == 0)
							goto RET_RTFWD;
					}
				}

				if (GetClick()-dwLastSendClick < m_RtRunMdInfo.bNodeTmOut)
					goto GOTO_RxHandleFrm;
			}
		}
	}
RET_RTFWD:
	//到这里表示已经抄读失败，填充无效数据
	pbOutBuf[0] = DAR;
	pbOutBuf[1] = DAR_REQ_TIMEOUT;
	if (bProId == PROTOCOLNO_DLT645_V07)
		return -1;
	else
		return 2;
}

//描述：处理AFN=14 Fn=1路由请求抄读内容
int CStdReader::Afn14Fn1_RtReqRd()
{
	TMtrRdCtrl* pMtrRdCtrl;
	TMtrPara tMtrPara;
	TRtReqInfo tInfo;
	TRdItem tRdItem;
	int iRet;
	WORD wApduLen;
	BYTE bTxBuf[512];
	BYTE b3762Buf[512];
	BYTE bApdu[256];
	BYTE bTsa[TSA_LEN+1];
	BYTE *pbTx = bTxBuf;
	BYTE bCtrl;
	BYTE bR[6] = {0}; 
	BYTE bCn = 0;
	BYTE bCheckCnt = 0;
	char szTsa[64] = {0};
	pFun g_pFun = NULL;

	//从路由模块请求的信息中提取表地址
	memset((BYTE*)&tInfo, 0, sizeof(tInfo));
	tInfo.bPhase = m_TRcv13762.bDtBuf[0];
	tInfo.bRevTsa[0] = tInfo.bTsa[0] = 6;
	memcpy(tInfo.bTsa+1, &m_TRcv13762.bDtBuf[1], 6);

	//将路由模块请求的电表地址转测量点
	tInfo.wPn = RouterMtrAddrConvertPn(tInfo.bTsa+1);
	tInfo.bTsaLen = GetMeterAddrLen(tInfo.wPn);
	tInfo.bTsa[0] = tInfo.bTsaLen;

	if (tInfo.wPn <= 0)	//表示测量点不存在，直接置为抄读成功标识
	{
		pbTx += SetRouterRequestInfo(RT_RD_SUCC, pbTx);
		DTRACE(DB_CCT, ("AFN14-F1: Invalid Meter:0x%s.\n", HexToStr(tInfo.bTsa+1, tInfo.bTsa[0], szTsa)));
	}
	else
	{
		//将表地址取反
		revcpy(tInfo.bRevTsa+1, tInfo.bTsa+1, tInfo.bTsa[0]);
		tInfo.bRevTsa[0] = tInfo.bTsa[0];

		//获取测量点和表地址对应的抄表控制结构
		pMtrRdCtrl = GetMtrRdCtrl(tInfo.wPn, tInfo.bTsa);
		if (pMtrRdCtrl == NULL)
		{
			pbTx += SetRouterRequestInfo(RT_RD_SUCC, pbTx);
			DTRACE(DB_CCT, ("AFN14-F1: GetMtrRdCtrl fail, Meter:0x%s.\n", HexToStr(tInfo.bTsa+1, tInfo.bTsa[0], szTsa)));
		}
		else
		{
			DoTaskSwitch(pMtrRdCtrl);

			//获取该测量点对应档案信息
			GetMeterPara(tInfo.wPn, &tMtrPara);
TRY_LOOP_ID:
			memset((BYTE*)&tRdItem, 0, sizeof(tRdItem));
			iRet = SearchAnUnReadID(GetCurPrio(bCn), tInfo.wPn, pMtrRdCtrl, &tRdItem, true);
			DTRACE(DB_CCT, ("AFN14-F1: Ret=%d, Meter:0x%s.\n", iRet, HexToStr(tInfo.bTsa+1, tInfo.bTsa[0], szTsa)));
			switch (iRet)
			{
				case RD_ERR_UNFIN:
					if (tMtrPara.bProId==PROTOCOLNO_DLT645 || tMtrPara.bProId==PROTOCOLNO_DLT645_V07)
					{
						g_pFun = &CStdReader::ReadDLT_645;
						pbTx += SetRouterRequestInfo(RT_RD_FAIL, pbTx);
					}
					else if (tMtrPara.bProId == PROTOCOLNO_SBJC)
					{
						g_pFun = &CStdReader::ReadDLT_69845;
						pbTx += SetRouterRequestInfo(RT_RD_FAIL, pbTx);
					}
					else if (tMtrPara.bProId == PROTOCOLNO_DLT69845)
					{
						if (tRdItem.bReqType == 1)
							wApduLen = GetRequestNormal(tRdItem.dwOAD, bApdu);
						else
							wApduLen = GetRequestRecord(tRdItem.dwOAD, bApdu, tRdItem.bRSD, tRdItem.wRsdLen, tRdItem.bRCSD, tRdItem.wRcsdLen);
						*pbTx++ = RT_RD_GOTO;
						*pbTx++ = 0x00;	
						iRet = Make698_45Frm(tInfo.bTsa+1, GetMeterAddrLen(tInfo.wPn), 0x43, 0, 0, SER_ADDR_TYPE_SIG, bApdu, wApduLen, pbTx+1);
						*pbTx++ = iRet;
						pbTx += iRet;
						*pbTx++ = 0x00;	//从节点附属节点数量n
						PrintInfo(&tRdItem, &tMtrPara);
					}
					else 
					{
						DTRACE(DB_CCT, ("DoCommSch(): Nonsupport protocol, Mtr addr=%s, Proto=%d.\n", HexToStr(bTsa+1, bTsa[0], szTsa), tMtrPara.bProId));
					}
					break;
				case RD_ERR_UNTIME:
				case RD_ERR_HALT:
					SaveTask(pMtrRdCtrl);
					pbTx += SetRouterRequestInfo(RT_RD_SUCC, pbTx);
					break;
				case RD_ERR_CHKTSK:
					DTRACE(DB_CCT, ("AFN14-F1: Loop search no Read Mtr:0x%s, RealLoopCnt=%d, MaxLoopCnt=%d.\n", 
								HexToStr(tInfo.bTsa+1, tInfo.bTsa[0], szTsa),pMtrRdCtrl->schItem.bLoopCnt, LOOP_MAX_CNT));
					goto TRY_LOOP_ID;
				default:	//RD_ERR_OK:
					SaveTask(pMtrRdCtrl);
					pbTx += SetRouterRequestInfo(RT_RD_SUCC, pbTx);
					break;
			}
		}
	}

	//376.2控制参数、信息域
	bCtrl = GetCtrl(m_TRcv13762.bCtrl);
	bR[0] = 0x04;	
	bR[1] |= m_TRcv13762.bR[1]&0x0f;
	bR[5] = m_TRcv13762.bR[5];

	memset(b3762Buf, 0, sizeof(b3762Buf));
	iRet = Make1376_2Frm(tInfo.bTsa+1, 6,  bCtrl, bR, AFN_RTRD, FN(1), bTxBuf, pbTx-bTxBuf, b3762Buf);
	Send(b3762Buf, iRet);

	if (g_pFun != NULL)
	{
		BYTE bRxBuf[512] = {0};
		BYTE *pbRx = bRxBuf;

		(this->*g_pFun)(pMtrRdCtrl, &tRdItem, &tMtrPara, bRxBuf, sizeof(bRxBuf));
		RouterResume();
	}

	PutMtrRdCtrl(tInfo.wPn, tInfo.bTsa, true);
	memset((BYTE*)&m_TRcv13762, 0, sizeof(m_TRcv13762));

	return iRet;
}

//描述：处理AFN=14 Fn=2 路由请求终端时间
//参数：
bool CStdReader::Afn14Fn2_RtReqClk()
{
	TTime tNowTime;
	BYTE bCtrl;
	BYTE bR[6] = {0}; 
	BYTE bBuf[16] = {0};
	BYTE bTxBuf[128] = {0};
	BYTE wTxLen;
	BYTE *pbBuf = bBuf; 
	BYTE *pbTxBuf = bTxBuf;


	bCtrl = GetCtrl(m_TRcv13762.bCtrl);
	bR[0] = m_TRcv13762.bR[0]&0x01;
	bR[0] |= m_TRcv13762.bR[0]&0x04;
	bR[0] |= m_TRcv13762.bR[0]&0xf0;
	bR[1] |= m_TRcv13762.bR[1]&0x0f;
	bR[5] = m_TRcv13762.bR[5];

	GetCurTime((TTime*)&tNowTime);
	*pbBuf++ = ByteToBcd(tNowTime.nSecond);
	*pbBuf++ = ByteToBcd(tNowTime.nMinute);
	*pbBuf++ = ByteToBcd(tNowTime.nHour);
	*pbBuf++ = ByteToBcd(tNowTime.nDay);
	*pbBuf++ = ByteToBcd(tNowTime.nMonth);
	*pbBuf++ = ByteToBcd(tNowTime.nYear%100);	

	wTxLen = Make1376_2Frm(NULL, 0,  bCtrl, bR, AFN_RTRD, FN(2), bBuf, pbBuf-bBuf, bTxBuf);
	Send(bTxBuf, wTxLen);
	memset((BYTE*)&m_TRcv13762, 0, sizeof(m_TRcv13762));

	return false;
}

void CStdReader::RouterPause()
{
	if (!m_TRunStateInfo.fRtPause)
	{
		m_TRunStateInfo.fRtPause = true;	//暂停路由
		Afn12Fn02_RtPause();
	}
	m_TRunStateInfo.dwLstDirClk = GetClick();
}

void CStdReader::RouterResume()
{
	if (m_TRunStateInfo.fRtPause)
	{
		m_TRunStateInfo.fRtPause = false;
		Afn12Fn03_RtResume();
	}
}


bool CStdReader::ReadPlcModuleInfo()
{
	WORD wMtrSn = 0;
	BYTE bCtrl;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;

	bCtrl = 0x41;
	wTxLen = Make1376_2Frm(NULL, 0, bCtrl, bR, AFN_QRYDATA, FN(10), NULL, 0, bTxBuf);
	ClearRcv(); 

	for (BYTE bTryCnt=0; bTryCnt<1; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(2))
			{
				if (m_TRcv13762.bAfn==AFN_QRYDATA && DtToFn(m_TRcv13762.bDt)==FN(10))
				{
					//Afn03Fn10_RptRtRunInfo(m_TRcv13762.bDtBuf);
					return true;
				}
			}
		}
	}

	return false;
}

//描述：直接抄读数据
//参数：@wMtrSn
//		@pbBuf 为array CSD,array ROAD
int CStdReader::DirectReadMeterData(WORD wMtrSn, BYTE *pbBuf)
{
	return -1;
}



int CStdReader::Set_OAD_to_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData)
{
	TOobMtrInfo tTMtrInfo;
	TRdItem tRdItem;
	BYTE bTxBuf[256];
	BYTE bBuf[64] = {0};
	BYTE *pbBuf = bBuf;
	int iTxLen;
	int iRet;
	BYTE *pbData0 = pbData;
	BYTE bNum;

	*pbBuf++ = bType; 
	*pbBuf++ = bChoice; //Choice
	*pbBuf++ = 0; //PIID
	memcpy(pbBuf, pApdu, wApduLen);
	pbBuf += wApduLen;

	bTxBuf[0] = PRO_TYPE_TRANS;
	bTxBuf[1] = 0x00;	//通信延时相关标识
	bTxBuf[2] = 0x00;	//附属相关延时标识

	if (GetMeterInfo(bTsa, bTsaLen, &tTMtrInfo) < 0)
		return -1;

	BYTE bType1;
	WORD wLen;
	int iLen;

	bNum = *pApdu++;
	*pbData++ = bNum;
	if (bChoice==2)
	{
		LockDirRd();
		for (BYTE i=0; i<bNum; i++)
		{
			tRdItem.dwOAD = OoOadToDWord(pApdu);
			memcpy(pbData, pApdu, 4);
			pbData += 4;

			const ToaMap* pOI = GetOIMap(OoOadToDWord(pApdu));
			iLen = OoScanData(pApdu+4, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType1);
			if (iLen < 0)
			{
				DTRACE(DB_FAPROTO, ("CStdReader::Set_OAD_to_645_meter(): OoScanData error, bType=%d, bChoice=%d, dwOAD=%08x.\n", bType, bChoice, OoOadToDWord(pApdu)));
				break;
			}

			pApdu += 4;
			iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
			if (iTxLen > 0)
			{
				bTxBuf[3] = iTxLen;
				iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
				if (iRet > 2)
				{
					//pbData += iRet;
					*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
			}
			pApdu += iRet;
		}
		UnLockDirRd();

		iRet = pbData - pbData0;
		pbData = pbData0;
		return iRet;
	}
	else if (bChoice== 3)
	{
		LockDirRd();
		for (BYTE i=0; i<bNum; i++)
		{
			tRdItem.dwOAD = OoOadToDWord(pApdu);
			memcpy(pbData, pApdu, 4);
			pbData += 4;

			const ToaMap* pOI = GetOIMap(OoOadToDWord(pApdu));
			iLen = OoScanData(pApdu+4, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType1);
			if (iLen < 0)
			{
				DTRACE(DB_FAPROTO, ("CStdReader::Set_OAD_to_645_meter(): OoScanData error, bType=%d, bChoice=%d, dwOAD=%08x.\n", bType, bChoice, OoOadToDWord(pApdu)));
				break;
			}
			pApdu += 4;
			iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
			if (iTxLen > 0)
			{
				bTxBuf[3] = iTxLen;
				iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
				if (iRet > 2)
				{
					//pbData += iRet;
					*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
			}
			pApdu += iRet;

			tRdItem.dwOAD = OoOadToDWord(pApdu);
			memcpy(pbData, pApdu, 4);
			pbData += 4;

			pApdu += 4;
			iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
			if (iTxLen > 0)
			{
				bTxBuf[3] = iTxLen;
				iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
				if (iRet > 2)
				{
					pbData += iRet;
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
			}

			pApdu++;	//跳过“延时读取时间”
		}

		UnLockDirRd();

		iRet = pbData - pbData0;
		pbData = pbData0;
		return iRet;
	}

}

int CStdReader::Act_OAD_to_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData)
{
	TOobMtrInfo tTMtrInfo;
	TRdItem tRdItem;
	BYTE bTxBuf[256];
	BYTE bBuf[64] = {0};
	BYTE *pbBuf = bBuf;
	int iTxLen;
	int iRet;
	BYTE *pbData0 = pbData;
	BYTE bNum;

	*pbBuf++ = bType; 
	*pbBuf++ = bChoice; //Choice
	*pbBuf++ = 0; //PIID
	memcpy(pbBuf, pApdu, wApduLen);
	pbBuf += wApduLen;

	bTxBuf[0] = PRO_TYPE_TRANS;
	bTxBuf[1] = 0x00;	//通信延时相关标识
	bTxBuf[2] = 0x00;	//附属相关延时标识

	if (GetMeterInfo(bTsa, bTsaLen, &tTMtrInfo) < 0)
		return -1;

	BYTE bType1;
	WORD wLen;
	int iLen;

	bNum = *pApdu++;
	*pbData++ = bNum;
	if (bChoice==2)
	{
		LockDirRd();
		for (BYTE i=0; i<bNum; i++)
		{
			tRdItem.dwOAD = OoOadToDWord(pApdu);
			memcpy(pbData, pApdu, 4);
			pbData += 4;

			const ToaMap* pOI = GetOIMap(OoOadToDWord(pApdu));
			iLen = OoScanData(pApdu+4, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType1);
			if (iLen < 0)
			{
				DTRACE(DB_FAPROTO, ("CStdReader::Set_OAD_to_645_meter(): OoScanData error, bType=%d, bChoice=%d, dwOAD=%08x.\n", bType, bChoice, OoOadToDWord(pApdu)));
				break;
			}

			pApdu += 4;
			iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
			if (iTxLen > 0)
			{
				bTxBuf[3] = iTxLen;
				iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
				if (iRet > 2)
				{
					//pbData += iRet;
					*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
			}
			pApdu += iRet;
		}
		UnLockDirRd();

		iRet = pbData - pbData0;
		pbData = pbData0;
		return iRet;
	}
	else if (bChoice== 3)
	{
		LockDirRd();
		for (BYTE i=0; i<bNum; i++)
		{
			tRdItem.dwOAD = OoOadToDWord(pApdu);
			memcpy(pbData, pApdu, 4);
			pbData += 4;

			const ToaMap* pOI = GetOIMap(OoOadToDWord(pApdu));
			iLen = OoScanData(pApdu+4, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType1);
			if (iLen < 0)
			{
				DTRACE(DB_FAPROTO, ("CStdReader::Set_OAD_to_645_meter(): OoScanData error, bType=%d, bChoice=%d, dwOAD=%08x.\n", bType, bChoice, OoOadToDWord(pApdu)));
				break;
			}
			pApdu += 4;
			iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
			if (iTxLen > 0)
			{
				bTxBuf[3] = iTxLen;
				iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
				if (iRet > 2)
				{
					//pbData += iRet;
					*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
			}
			pApdu += iRet;

			tRdItem.dwOAD = OoOadToDWord(pApdu);
			memcpy(pbData, pApdu, 4);
			pbData += 4;

			pApdu += 4;
			iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
			if (iTxLen > 0)
			{
				bTxBuf[3] = iTxLen;
				iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
				if (iRet > 2)
				{
					pbData += iRet;
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
			}

			pApdu++;	//跳过“延时读取时间”
		}

		UnLockDirRd();

		iRet = pbData - pbData0;
		pbData = pbData0;
		return iRet;
	}

}

//OK
int CStdReader::Do_uplink_request_to_698_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData)
{
	BYTE bTxBuf[256];
	BYTE bBuf[64] = {0};
	BYTE *pbBuf = bBuf;
	int iTxLen;
	int iRet;
	BYTE *pbData0 = pbData;
	BYTE bNum;

	*pbBuf++ = bType; 
	*pbBuf++ = bChoice; //Choice
	*pbBuf++ = 0; //PIID
	memcpy(pbBuf, pApdu, wApduLen);
	pbBuf += wApduLen;

	bTxBuf[0] = PRO_TYPE_TRANS;
	bTxBuf[1] = 0x00;	//通信延时相关标识
	bTxBuf[2] = 0x00;	//附属相关延时标识


	iTxLen = Make698_45Frm(bTsa, bTsaLen, 0x43, 0, 0, SER_ADDR_TYPE_SIG, bBuf, pbBuf-bBuf, &bTxBuf[4]);
	if (iTxLen > 0)
	{
		bTxBuf[3] = iTxLen;
		iRet = DoFwdData(bTsa, bTsaLen, bTxBuf, iTxLen+4, wTimeOut, pbData);
		iRet -= 2;	//减去时间标签+上报信息域
	}

	return iRet;
}

struct MeterID {
	DWORD dwOAD;  // 协议层OAD，
	DWORD  dwInterID[5]; //程序内部OAD映射表 OAD
	int (*DataDealFunction)(void *pdata, int DataLen);
};

int post_deal_data_0x40000200(void *pbdata, int iDataLen)
{
	int iRet = 0;
	BYTE * pbTmp = (BYTE*)pbdata;
	memmove((BYTE*)pbTmp+6,pbTmp+10,3);
	iRet = 9+4;
	return iRet;
}

struct MeterID MeterIDMap[] = {
	{0x40000200, {0x40000200, 0x40000209, 0}, post_deal_data_0x40000200},
	{0, {0}, NULL},
};


struct MeterID* find_interal_meterId_map(DWORD dwOAD)
{
	struct MeterID *pMeterMap = NULL;
	for (int i=0; i<sizeof(MeterIDMap)/sizeof(MeterIDMap[0]); i++)
	{
		if (dwOAD == MeterIDMap[i].dwOAD)
		{
			pMeterMap = &MeterIDMap[i];
			break;	
		}
	}

	return pMeterMap;
}	

int find_interal_meterID_num(DWORD dwOAD)
{
	struct MeterID *pMeterMap = NULL;
	int iNum = 0;
	for (int i=0; i<sizeof(MeterIDMap)/sizeof(MeterIDMap[0]); i++)
	{
		if (dwOAD == MeterIDMap[i].dwOAD)
		{
			pMeterMap = &MeterIDMap[i];
			break;	
		}
	}

	if (pMeterMap != NULL)
	{
		for (int i=0; i<sizeof(MeterIDMap[0].dwInterID); i++)
		{
			if (pMeterMap->dwInterID[i] == 0)
			{
				iNum = i;
				break;
			}
		}
	}

	return iNum;

}


int CStdReader::Read_OneOAD_from_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData)
{
	BYTE bTxBuf[256];
	BYTE bRxBuf[256];
	TRdItem tRdItem;
	TOobMtrInfo tTMtrInfo;
	DWORD *pdwOADTmp = NULL;
	int iTxLen;
	int iRet = -1, iTotalLen=0;
	BYTE *pbData0 = pbData;
	BYTE *pbPostDealData = NULL;
	BYTE bNum;
	int iNum;
	struct MeterID *pMeterIDMap = NULL;
	DWORD dwOriginOAD;

	bTxBuf[0] = PRO_TYPE_TRANS;
	bTxBuf[1] = 0x00;	//通信延时相关标识
	bTxBuf[2] = 0x00;	//附属相关延时标识


	if (GetMeterInfo(bTsa, bTsaLen, &tTMtrInfo) < 0)
		return -1;

	dwOriginOAD = tRdItem.dwOAD = OoOadToDWord(pInApdu);
	pInApdu += 4;

	//search the internal map table OAD and number of OAD in the table.
	pMeterIDMap = find_interal_meterId_map(tRdItem.dwOAD);
	iNum = find_interal_meterID_num(tRdItem.dwOAD);

	// if we can't find the internal OAD int the map table, use the origin OAD to read the Item. 	
	if (pMeterIDMap == NULL)
	{
		pdwOADTmp = &tRdItem.dwOAD;
		iNum = 1;	
	}
	else
	{
		pdwOADTmp = &pMeterIDMap->dwInterID[0];
	}

	LockDirRd();
	for (int iCount=0; iCount<iNum; iCount++)
	{		
		tRdItem.dwOAD = pdwOADTmp[iCount];
		iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
		if (iTxLen > 0)
		{
			bTxBuf[3] = iTxLen;
			//fill the origin OAD
			if (iCount == 0)
			{
				pbData += OoDWordToOad(dwOriginOAD, pbData);
				pbPostDealData = pbData;
			}

			iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData+1, tTMtrInfo.bProType, false, true);
			
			if (iRet > 2)
			{
				pbData[0] = 0x01;	//1字节choice
				iRet += 1;
				pbData += iRet;
				iTotalLen += iRet;
			}
			else
			{
				pbData[0] = DAR;
				iTotalLen = 1;
			}

			DTRACE(DB_FAPROTO,("Read_OneOAD_from_645_meter: iRet=%d\r\n", iRet));
		}
	}
	UnLockDirRd();

	iRet = pbData - pbData0;
	pbData = pbData0;

	if (pMeterIDMap != NULL)
	{
		if (pMeterIDMap->DataDealFunction != NULL)
		{
			iRet = pMeterIDMap->DataDealFunction(pbPostDealData, iRet);
		}
	}

	DTRACE(DB_FAPROTO,("Read_OneOAD_from_645_meter: iRet=%d\r\n", iRet));

	return iRet;

}

int CStdReader::Read_RecordData_from_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData)
{
	TOobMtrInfo tTMtrInfo;
	TRdItem tRdItem;
	BYTE bTxBuf[256];
	BYTE bBuf[64] = {0};
	BYTE *pbBuf = bBuf;
	int iTxLen;
	int iRet;
	BYTE *pbData0 = pbData;
	BYTE bNum;

	*pbBuf++ = bType; 
	*pbBuf++ = bChoice; //Choice
	*pbBuf++ = 0; //PIID
	memcpy(pbBuf, pApdu, wApduLen);
	pbBuf += wApduLen;

	bTxBuf[0] = PRO_TYPE_TRANS;
	bTxBuf[1] = 0x00;	//通信延时相关标识
	bTxBuf[2] = 0x00;	//附属相关延时标识


	DWORD dwOAD;
	BYTE bRcsdNum, bRoadNum;

	if (GetMeterInfo(bTsa, bTsaLen, &tTMtrInfo) < 0)
		return -1;

	//OAD
	dwOAD = OoOadToDWord(pApdu);
	memcpy(pbData, pApdu, 4);
	pApdu += 4;
	pbData += 4;

	//RSD
	BYTE bRsdLen = ScanRSD(pApdu, false);
	pApdu += bRsdLen;

	//RCSD
	BYTE bRcsdLen = ScanRCSD(pApdu, false);
	memcpy(pbData, pApdu, bRcsdLen);
	pbData += bRcsdLen;
	bRcsdNum = *pApdu++;

	BYTE *pRespDataState = pbData;	//记录数据+M条记录 = 2 BYTE
	pbData += 2;

	LockDirRd();
	tRdItem.dwOAD = dwOAD;
	for (BYTE i=0; i<bRcsdNum; i++)
	{
		DWORD dwRealOAD;
		BYTE bTmpChoice;

		tRdItem.bRCSD[1+i*5] = *pApdu++;	//choice
		memcpy(&tRdItem.bRCSD[1+i*5+1], pApdu, 4);
		pApdu += 4;

		bTmpChoice = tRdItem.bRCSD[1+i*5];
		if (bTmpChoice == 0)
		{
			tRdItem.wRcsdIdx = i;
			memrcpy((BYTE*)&dwRealOAD, &tRdItem.bRCSD[1+i*5+1], 4);
			if ((tRdItem.dwOAD == 0x50040200) ||(tRdItem.dwOAD ==0x50050200)||(tRdItem.dwOAD ==0x50060200))//日、月、结算
			{
				if (dwRealOAD < 0x20000000) //电量和需量
				{
					if (tRdItem.dwOAD == 0x50040200)
						dwRealOAD += 4;
					else
						dwRealOAD += 5;
				}
			}

			iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, dwRealOAD, &bTxBuf[4]);
			if (iTxLen > 0)
			{
				bTxBuf[3] = iTxLen;
				iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType);
				if (iRet > 2)
					pbData += iRet;
				else
					*pbData++ = 0x00;
			}
		}
		else	//ROAD 暂不考虑该情况
		{
			*pbData++ = 0x00;
			DTRACE(DB_FAPROTO, ("CStdReader::Set_OAD_to_645_meter(): OoScanData error, bType=%d, bTmpChoice=%d, dwOAD=%08x.\n", bType, bTmpChoice, OoOadToDWord(pApdu)));
			break;
		}
	}
	UnLockDirRd();

	pRespDataState[0] = 0x01;	//记录数据
	pRespDataState[1] = 0x01;	//M条记录

	iRet = pbData - pbData0;
	pbData = pbData0;
	return iRet;

}



//描述：直接抄读代理数据
//参数：@bType 方法类型，如设置、读取、操作
//		@bChoice bType方法类型中子类型NORMAL\NORMAL_LIST
int CStdReader::DirAskProxy(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData)
{
	TOobMtrInfo tTMtrInfo;
	int iRet = -1;
	BYTE *pbData0 = pbData;
	BYTE bNum;

	if (GetMeterInfo(bTsa, bTsaLen, &tTMtrInfo) < 0)
		return -1;

	if (tTMtrInfo.bProType == 3)	//698.45
	{
		iRet = Do_uplink_request_to_698_meter(bType, bChoice, bTsa, bTsaLen, pApdu, wApduLen, wTimeOut, pbData);	
	}
	else //645-07
	{
		if(bType == 5)	//读取
		{
			if (bChoice == 1)	//读取单个
			{
				iRet =	Read_OneOAD_from_645_meter(bType, bChoice, bTsa, bTsaLen, pApdu, wApduLen, wTimeOut, pbData);	
			}
			else if (bChoice == 2)	//读取多个OAD 
			{
				bNum = *pApdu++;
				*pbData++ = bNum;
				for (BYTE i=0; i<bNum; i++)
				{
					iRet =	Read_OneOAD_from_645_meter(bType, bChoice, bTsa, bTsaLen, pApdu, wApduLen, wTimeOut, pbData);	
					if (iRet == -1)
						goto DirAskProxy_error1;

					pbData += iRet;
				}
				iRet = pbData - pbData0;
				pbData = pbData0;
			}
			else if (bChoice == 3)	//读取记录型
			{
				iRet = 	Read_RecordData_from_645_meter(bType, bChoice, bTsa, bTsaLen, pApdu, wApduLen, wTimeOut, pbData);	
			}
		}
		else if (bType==6 || bType==7)	//设置 || 操作
		{
			iRet = Set_OAD_to_645_meter(bType, bChoice, bTsa, bTsaLen, pApdu, wApduLen, wTimeOut, pbData);	
		}
	}

	return iRet;

DirAskProxy_error1:
	*pbData0 = 0;
	iRet = 1;
	return iRet;

}
//描述：直接抄读代理数据
//参数：@bType 方法类型，如设置、读取、操作
//		@bChoice bType方法类型中子类型NORMAL\NORMAL_LIST
/*
int CStdReader::DirAskProxy(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData)
{
	TOobMtrInfo tTMtrInfo;
	TRdItem tRdItem;
	BYTE bTxBuf[256];
	BYTE bBuf[64] = {0};
	BYTE *pbBuf = bBuf;
	int iTxLen;
	int iRet;
	BYTE *pbData0 = pbData;
	BYTE bNum;

	*pbBuf++ = bType; 
	*pbBuf++ = bChoice; //Choice
	*pbBuf++ = 0; //PIID
	memcpy(pbBuf, pApdu, wApduLen);
	pbBuf += wApduLen;

	bTxBuf[0] = PRO_TYPE_TRANS;
	bTxBuf[1] = 0x00;	//通信延时相关标识
	bTxBuf[2] = 0x00;	//附属相关延时标识

	if (GetMeterInfo(bTsa, bTsaLen, &tTMtrInfo) < 0)
		return -1;

	if (tTMtrInfo.bProType == 3)	//698.45
	{
		iTxLen = Make698_45Frm(bTsa, bTsaLen, 0x43, 0, 0, SER_ADDR_TYPE_SIG, bBuf, pbBuf-bBuf, &bTxBuf[4]);
		if (iTxLen > 0)
		{
			bTxBuf[3] = iTxLen;
			iRet = DoFwdData(bTsa, bTsaLen, bTxBuf, iTxLen+4, wTimeOut, pbData);
			iRet -= 2;	//减去时间标签+上报信息域
			return iRet;
		}
	}
	else //645-07
	{
		if(bType == 5)	//读取
		{
			if (bChoice == 1)	//读取单个
			{
				LockDirRd();
				tRdItem.dwOAD = OoOadToDWord(pApdu);
				pApdu += 4;
				iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
				if (iTxLen > 0)
				{
					bTxBuf[3] = iTxLen;
					pbData += OoDWordToOad(tRdItem.dwOAD, pbData);
					iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData+1, tTMtrInfo.bProType, false, true);
					if (iRet > 2)
					{
						pbData[0] = 0x01;	//1字节choice
						iRet += 1;
						pbData += iRet;
					}
					else
					{
						pbData[0] = DAR;
					}
				}
				UnLockDirRd();

				iRet = pbData - pbData0;
				pbData = pbData0;
				return iRet;
			}
			else if (bChoice == 2)	//读取多个OAD 
			{
				LockDirRd();
				bNum = *pApdu++;
				*pbData++ = bNum;
				for (BYTE i=0; i<bNum; i++)
				{
					tRdItem.dwOAD = OoOadToDWord(pApdu);
					pApdu += 4;
					iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
					if (iTxLen > 0)
					{
						bTxBuf[3] = iTxLen;
						pbData += OoDWordToOad(tRdItem.dwOAD, pbData);
						iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData+1, tTMtrInfo.bProType, false, true);
						if (iRet > 2)
						{
							pbData[0] = 0x01;	//1字节choice
							iRet += 1;
							pbData += iRet;
						}
						else
						{
							pbData[0] = DAR;
						}
					}
				}
				UnLockDirRd();

				iRet = pbData - pbData0;
				pbData = pbData0;
				return iRet;
			}
			else if (bChoice == 3)	//读取记录型
			{
				DWORD dwOAD;
				BYTE bRcsdNum, bRoadNum;
				BYTE bChoice;

				//OAD
				dwOAD = OoOadToDWord(pApdu);
				memcpy(pbData, pApdu, 4);
				pApdu += 4;
				pbData += 4;

				//RSD
				BYTE bRsdLen = ScanRSD(pApdu, false);
				pApdu += bRsdLen;

				//RCSD
				BYTE bRcsdLen = ScanRCSD(pApdu, false);
				memcpy(pbData, pApdu, bRcsdLen);
				pbData += bRcsdLen;
				bRcsdNum = *pApdu++;

				BYTE *pRespDataState = pbData;	//记录数据+M条记录 = 2 BYTE
				pbData += 2;

				LockDirRd();
				tRdItem.dwOAD = dwOAD;
				for (BYTE i=0; i<bRcsdNum; i++)
				{
					DWORD dwRealOAD;
					BYTE bChoice;

					tRdItem.bRCSD[1+i*5] = *pApdu++;	//choice
					memcpy(&tRdItem.bRCSD[1+i*5+1], pApdu, 4);
					pApdu += 4;

					bChoice = tRdItem.bRCSD[1+i*5];
					if (bChoice == 0)
					{
						tRdItem.wRcsdIdx = i;
						memrcpy((BYTE*)&dwRealOAD, &tRdItem.bRCSD[1+i*5+1], 4);
						if ((tRdItem.dwOAD == 0x50040200) ||(tRdItem.dwOAD ==0x50050200)||(tRdItem.dwOAD ==0x50060200))//日、月、结算
						{
							if (dwRealOAD < 0x20000000) //电量和需量
							{
								if (tRdItem.dwOAD == 0x50040200)
									dwRealOAD += 4;
								else
									dwRealOAD += 5;
							}
						}

						iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, dwRealOAD, &bTxBuf[4]);
						if (iTxLen > 0)
						{
							bTxBuf[3] = iTxLen;
							iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType);
							if (iRet > 2)
								pbData += iRet;
							else
								*pbData++ = 0x00;
						}
					}
					else	//ROAD 暂不考虑该情况
					{
						*pbData++ = 0x00;
						DTRACE(DB_FAPROTO, ("DirAskProxy(): OoScanData error, bType=%d, bChoice=%d, dwOAD=%08x.\n", bType, bChoice, OoOadToDWord(pApdu)));
						break;
					}
				}
				UnLockDirRd();

				pRespDataState[0] = 0x01;	//记录数据
				pRespDataState[1] = 0x01;	//M条记录

				iRet = pbData - pbData0;
				pbData = pbData0;
				return iRet;
			}
		}
		else if (bType==6 || bType==7)	//设置 || 操作
		{
			BYTE bNum, bType1;
			WORD wLen;
			int iLen;

			bNum = *pApdu++;
			*pbData++ = bNum;
			if (bChoice==2)
			{
				LockDirRd();
				for (BYTE i=0; i<bNum; i++)
				{
					tRdItem.dwOAD = OoOadToDWord(pApdu);
					memcpy(pbData, pApdu, 4);
					pbData += 4;

					const ToaMap* pOI = GetOIMap(OoOadToDWord(pApdu));
					iLen = OoScanData(pApdu+4, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType1);
					if (iLen < 0)
					{
						DTRACE(DB_FAPROTO, ("DirAskProxy(): OoScanData error, bType=%d, bChoice=%d, dwOAD=%08x.\n", bType, bChoice, OoOadToDWord(pApdu)));
						break;
					}

					pApdu += 4;
					iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
					if (iTxLen > 0)
					{
						bTxBuf[3] = iTxLen;
						iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
						if (iRet > 2)
						{
							//pbData += iRet;
							*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
						}
						else
						{
							*pbData++ = 0x00;
						}
					}
					else
					{
						*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
					}
					pApdu += iRet;
				}
				UnLockDirRd();

				iRet = pbData - pbData0;
				pbData = pbData0;
				return iRet;
			}
			else if (bChoice== 3)
			{
				LockDirRd();
				for (BYTE i=0; i<bNum; i++)
				{
					tRdItem.dwOAD = OoOadToDWord(pApdu);
					memcpy(pbData, pApdu, 4);
					pbData += 4;

					const ToaMap* pOI = GetOIMap(OoOadToDWord(pApdu));
					iLen = OoScanData(pApdu+4, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType1);
					if (iLen < 0)
					{
						DTRACE(DB_FAPROTO, ("DirAskProxy(): OoScanData error, bType=%d, bChoice=%d, dwOAD=%08x.\n", bType, bChoice, OoOadToDWord(pApdu)));
						break;
					}
					pApdu += 4;
					iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
					if (iTxLen > 0)
					{
						bTxBuf[3] = iTxLen;
						iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
						if (iRet > 2)
						{
							//pbData += iRet;
							*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
						}
						else
						{
							*pbData++ = 0x00;
						}
					}
					else
					{
						*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
					}
					pApdu += iRet;

					tRdItem.dwOAD = OoOadToDWord(pApdu);
					memcpy(pbData, pApdu, 4);
					pbData += 4;

					pApdu += 4;
					iTxLen = DL645_9707MakeFrm(bTsa, bTsaLen, tTMtrInfo.bProType, 0, tRdItem.dwOAD, &bTxBuf[4]);
					if (iTxLen > 0)
					{
						bTxBuf[3] = iTxLen;
						iRet = Afn13Fn01_RtFwd(bTsa, bTsaLen, bTxBuf, iTxLen+4, NULL, &tRdItem, pbData, tTMtrInfo.bProType, false, true);
						if (iRet > 2)
						{
							pbData += iRet;
						}
						else
						{
							*pbData++ = 0x00;
						}
					}
					else
					{
						*pbData++ = DAR_RES_RW;	//645表不支持，设置、操作，直接回“拒绝读写”
					}

					pApdu++;	//跳过“延时读取时间”
				}

				UnLockDirRd();

				iRet = pbData - pbData0;
				pbData = pbData0;
				return iRet;
			}
		}
	}

	return -1;
}
*/
//描述:组645帧,数据已经放到帧中
//返回:帧长度
WORD CStdReader::Make645Frm(BYTE* pbFrm, const BYTE* pbAddr, BYTE bCmd, BYTE bDataLen)
{
	pbFrm[0] = 0x68;
	memcpy(&pbFrm[1], pbAddr, 6);
	pbFrm[7] = 0x68;
	pbFrm[8] = bCmd;
	pbFrm[9] = bDataLen;

	//+0x33
	for (WORD i=10; i<(WORD)bDataLen+10; i++)
	{
		pbFrm[i] += 0x33;
	}	 

	pbFrm[10+(WORD)bDataLen] = CheckSum(pbFrm, (WORD)bDataLen+10);
	pbFrm[11+(WORD)bDataLen] = 0x16;

	return bDataLen+12;
}  

//描述：组698.45数据结构
//参数：@pbMtr	表地址
//		@bMtrLen 表地址长度	 地址的字节数，取值范围：0…15，对应表示1…16个字节长度
//		@bCtrl	控制参数	
//		@bAFType	地址域服务器地址的类别标志，采用编码方式，0表示单地址，1表示通配地址，2表示组地址，3表示广播地址
//		@bCA	客户机地址CA用1字节无符号整数表示，取值范围0…255，值为0表示不关注客户机地址
//		@wFramFmt 分帧格式
//		@pbAPDUData	应用协议控制单元数据
//		@wAPDULen	长度
//		@pbOutBuf	返回参数
//返回：pbOutBuf数据长度
int CStdReader::Make698_45Frm(BYTE *pbMtr, BYTE bMtrLen, BYTE bCtrl, BYTE bAFType, BYTE bCA, WORD wFramFmt, BYTE *pbAPDU, WORD wAPDULen, BYTE *pbRespBuf)
{
	WORD wLen;
	BYTE *p = pbRespBuf;
	BYTE *pHeadCs;	//帧头校验	

	//--------------帧头--------------
	*p++ = 0x68;
	p += PRO_69845_DATA_LEN;
	*p++ = bCtrl;
	*p++ = ((bMtrLen-1)&0x0f) | (bAFType<<6);	
	revcpy(p, pbMtr, bMtrLen);
	//memcpy(p, pbMtr, bMtrLen);
	p += bMtrLen;
	*p++ = bCA;
	pHeadCs = p;
	p += PRO_69845_HEAD_CS_LEN;

	//-----------链路用户数据-----------
	if (bCtrl & 0x20)	//启动标志位PRM
	{
		WordToByte(wFramFmt, p);
		p += 2;
	}
	memcpy(p, pbAPDU, wAPDULen);
	p += wAPDULen;
	*p++ = 0; //TimeTag  OPTIONAL

	//-----------计算数据长度------------
	wLen = (p - pbRespBuf) + PRO_69845_FRAM_CS_LEN - PRO_69845_LEN_OFFSET;
	WordToByte(wLen, &pbRespBuf[1]);

	//-----------帧头校验------------
	WordToByte(CheckCRC16(pbRespBuf+PRO_69845_LEN_OFFSET, pHeadCs-pbRespBuf-PRO_69845_LEN_OFFSET), pHeadCs);

	//-----------帧校验------------
	WordToByte(CheckCRC16(pbRespBuf+PRO_69845_LEN_OFFSET, p-pbRespBuf-PRO_69845_LEN_OFFSET), p);
	p += 2;
	*p++ = 0x16;

	return p - pbRespBuf;
}


//描述：将buf数据转换成698.45数据结构
//参数：@pbInBuf 数据
//		@wInLen	 数据长度
//		@ptTFrmFmt 返回值
bool CStdReader::DecodeFram69845(BYTE *pbInBuf, WORD wInLen, TFrm69845 *ptTFrmFmt)
{
	BYTE bAddrMtrLen;

	for (WORD i=0; i<wInLen; i++)
	{
		if (*pbInBuf==0x68 && *(pbInBuf+(wInLen-i-1))==0x16)
		{
			BYTE *pbInBuf0 = pbInBuf;
			WORD wFramLen = wInLen - i;

			ptTFrmFmt->bHead = *pbInBuf++;
			ptTFrmFmt->wDataLen = ByteToWord(pbInBuf);	pbInBuf += 2;
			ptTFrmFmt->bCtrl = *pbInBuf++;
			ptTFrmFmt->bSAFlg = *pbInBuf++;
			bAddrMtrLen = (ptTFrmFmt->bSAFlg&0x0f) + 1;
			ptTFrmFmt->bSALen = bAddrMtrLen;
			revcpy(ptTFrmFmt->bSA, pbInBuf, bAddrMtrLen);	 pbInBuf += bAddrMtrLen;
			ptTFrmFmt->bCA = *pbInBuf++;
			if (CheckCRC16(pbInBuf0+1, pbInBuf-pbInBuf0-1) == (ptTFrmFmt->wHCS=ByteToWord(pbInBuf)))
			{
				pbInBuf += 2;
				ptTFrmFmt->wAPDULen = wFramLen-(pbInBuf-pbInBuf0)-3;	//wInLen-(pbPtr-pbInBuf)-3: -3 = HCS+0x16
				memcpy(ptTFrmFmt->bAPDUData, pbInBuf, ptTFrmFmt->wAPDULen);	
				pbInBuf += ptTFrmFmt->wAPDULen;
				if (CheckCRC16(pbInBuf0+1, wFramLen-4) == (ptTFrmFmt->wFCS=ByteToWord(pbInBuf)))
				{
					pbInBuf += 2;
					ptTFrmFmt->bEnd = 0x16;

					return true;
				}
			}
		}
		pbInBuf++;
	}

	return false;
}


//描述：从APDU中提取OAD或关联OAD, 同时将相应的数据提取出来
//参数：@pApdu 路由模块上报的APDU帧
//		@wApduLen APDU数据长度
//		@pRptItem 返回的OAD及关联OAD
//		@pbBuf APDU中的OAD数据内容
//		@fIsRptFlg 是否是AFN06-F2请求帧
//返回：数据长度
int CStdReader::DecodeReportApdu(BYTE *pApdu, WORD wApduLen, TRdItem *pRptItem, BYTE *pbBuf,  bool fIsRptFlg)
{
	WORD wLen = 0;
	BYTE bLnkNum;
	BYTE *pApdu0 = pApdu;

	if (*pApdu++ == GET_RESPONSE)
	{
		if (*pApdu++ == GET_RESPONCE_NORMAL)
		{
			pRptItem->bReqType = GET_RESPONCE_NORMAL;
			pApdu++;	//PIID-ACD
			pRptItem->dwOAD = OoOadToDWord(pApdu);
			pRptItem->bCSD[0] = 0;	//OAD
			memcpy(pRptItem->bCSD+1, pApdu, 4);
			pApdu += 4;
			pApdu++;	//读取一个属性，去掉一个字节的结果DAR或Data
			wLen = wApduLen - (pApdu - pApdu0);
			memcpy(pbBuf, pApdu, wLen);
		}
		else
		{
			pRptItem->bReqType = GET_RESPONCE_RECORD;
			pApdu++;	//PIID-ACD
			pRptItem->dwOAD = OoOadToDWord(pApdu);
			BYTE *pbCSD = pRptItem->bCSD;

			if (fIsRptFlg && ((pRptItem->dwOAD&0xff000000)==0x30000000))
			{
				DWORD dwEvtSnOAD;
				BYTE *pbBuf0 = pbBuf;

				//*pbCSD++ = 0x01;	//CSD中为ROAD
				memcpy(pbCSD, pApdu, 4);
				pbCSD += 4;
				pApdu += 4;
				bLnkNum = *pApdu++;

				dwEvtSnOAD = OoOadToDWord(pApdu+1);
				if (dwEvtSnOAD == 0x20220200)
				{
					pApdu += 5;
					*pbCSD++ = bLnkNum - 1;	//+1:全事件抄读时多了一个“事件记录序号”，这里去掉，不然不能匹配事件方案中的RCSD
				}
				for (BYTE i=0; i<bLnkNum; i++)
				{
					pApdu++;
					memcpy(pbCSD, pApdu, 4);
					pbCSD += 4;
					pApdu += 4;
				}

				pApdu += 2;	//记录数据+M条记录
				wLen = wApduLen - (pApdu - pApdu0);
				pbBuf += WordToByte(wLen, pbBuf);
				memcpy(pbBuf, pApdu, wLen);
				pbBuf += wLen;

				wLen = pbBuf - pbBuf0;
				pbBuf = pbBuf0;
			}
			else
			{
				*pbCSD++ = 0x01;	//CSD中为ROAD
				memcpy(pbCSD, pApdu, 4);
				pbCSD += 4;
				pApdu += 4;
				bLnkNum = *pApdu++;

				*pbCSD++ = bLnkNum;
				for (BYTE i=0; i<bLnkNum; i++)
				{
					pApdu++;
					memcpy(pbCSD, pApdu, 4);
					pbCSD += 4;
					pApdu += 4;
				}

				pApdu += 2;	//记录数据+M条记录
				wLen = wApduLen - (pApdu - pApdu0);
				memcpy(pbBuf, pApdu, wLen);
			}
		}
	}

	return wLen;
}


//描述：组1376.2帧，该函数实现所有功能码对应功能；
//参数：@pbTsa	表地址
//		@bTsaLen	表地址长度
//		@bCtrl	控制码
//		@pbR	信息域
//		@bAfn	功能码
//		@wFn	数据单元
//		@pbInbuf	传入数据，这个参数很灵活，传入的数据可能为698.45、645、T188、376.2数据单元帧，通过前两个字节判断协议类型
//		@wInLen	传入数据长度
//		@pbOutBuf 返回数据
//		@wOutLen	返回数据长度
int CStdReader::Make1376_2Frm(BYTE *pbTsa, BYTE bTsaLen, BYTE bCtrl, BYTE *pbR,  BYTE bAfn, BYTE bFn, const void * pbInbuf, WORD wInLen, BYTE *pbRespBuf)
{
	BYTE *p = pbRespBuf;
	BYTE *pbCs;
	BYTE *pbCtrl;
	BYTE bTsa[TSA_LEN] = {0};

	*p++ = 0x68;
	p += PRO_3762_DATA_LEN;
	pbCtrl = p;
	*p++ = bCtrl;
	memcpy(p, pbR, 6);
	p += 6;
	if (pbR[0] & 0x04)	//通信模块标识
	{
		memcpy(p, m_TRtStat.bTermAddr, m_TRtStat.bTermLen);
		p += 6;
		if (pbTsa != NULL)
		{
			memcpy(bTsa, pbTsa, bTsaLen);
			EncodeRouterMtrAddr(bTsa, bTsaLen);
			memcpy(p, bTsa, 6);
			p += 6;
		}
	}
	*p++ = bAfn;
	FnToDt(bFn, p);
	p += 2;
	memcpy(p, pbInbuf, wInLen);
	p += wInLen;
	*p++ = CheckCS(pbCtrl, p-pbCtrl);
	*p++ = 0x16;
	WordToByte(p-pbRespBuf, pbRespBuf+1);

	return p - pbRespBuf;
}

//描述：1376.2数据结构转数组
//参数：@ptTFrmFmt	1376.2数据结构帧
//		@pbOutBuf	返回数据
//返回：数据长度
int CStdReader::Pro1376_2ToBuf(TFrm13762 *ptTFrmFmt, BYTE *pbOutBuf)
{
	WORD wOutLen;
	BYTE *p = pbOutBuf;

	*p++ = ptTFrmFmt->bHead;
	*p++ = ptTFrmFmt->wDataLen%256;
	*p++ = ptTFrmFmt->wDataLen/256;
	*p++ = ptTFrmFmt->bCtrl;
	memcpy(p, ptTFrmFmt->bR, 6);	
	p += 6;
	if (ptTFrmFmt->bR[0] & 0x04)	//通信模块标识
	{
		memcpy(p, ptTFrmFmt->bSrcAddr, 6);	
		p += 6;
		for (BYTE i=0; i<((ptTFrmFmt->bR[0]>>4)&0x0f); i++)
		{
			memcpy(p, ptTFrmFmt->bRelayAddr, 6);	
			p += 6;
		}
		memcpy(p, ptTFrmFmt->bDesAddr, 6);	
		p += 6;
	}
	*p++ = ptTFrmFmt->bAfn;
	*p++ = ptTFrmFmt->bDt[0];
	*p++ = ptTFrmFmt->bDt[1];
	memcpy(p, ptTFrmFmt->bDtBuf, ptTFrmFmt->wDtLen);	
	p += ptTFrmFmt->wDtLen;
	*p++ = ptTFrmFmt->bCs;
	*p++ = ptTFrmFmt->bEnd;
	wOutLen = p - pbOutBuf;

	return wOutLen;
}

//描述：获取控制域信息
BYTE CStdReader::GetCtrl(BYTE bRcvCtrl)
{
	BYTE bCtrl = 0;

	//时钟请求由路由模块发起，即路由模块是启动站，集中器从动站
	bCtrl = m_RtRunMdInfo.bModType;
	bCtrl |= (bRcvCtrl & 0x40)? 0x00 : 0x40;

	return bCtrl;
}

//描述:帧格式中的DT到FN转换
//返回:正确则返回FN,否则返回0
WORD CStdReader::DtToFn(BYTE* pbDt)
{
	const static BYTE bDtToFn[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	for (WORD i=0; i<8; i++)
	{
		if (pbDt[0] == bDtToFn[i])
		{
			return (WORD )pbDt[1]*8 + i + 1;
		}
	}

	return 0;
}

//描述:FN到帧格式中DT的转换
void CStdReader::FnToDt(WORD wFn, BYTE* pbDt)
{
	pbDt[0] = 0x1<<((wFn-1)%8);
	pbDt[1] = (BYTE)((wFn-1)/8);
}

// WORD CStdReader::GetRequestNormal(DWORD dwOAD, BYTE* pbTxBuf)
// {
// 	BYTE *pbTxBuf0 = pbTxBuf;
// 	WORD wLen;
// 
// 	*pbTxBuf++ = DL69845_APPSVR_GETREQUEST; //GET-Request
// 	*pbTxBuf++ = DL69845_APPSVR_GETREQUEST_NORMAL;	//GET-Request-NORMAL
// 	*pbTxBuf++ = 0x05; //PIID
// 	memcpy(pbTxBuf, (BYTE *)&dwOAD, sizeof(dwOAD));	
// 	pbTxBuf += 4;
// 	*pbTxBuf++ = 0x00;
// 
// 	wLen = pbTxBuf - pbTxBuf0;
// 	pbTxBuf = pbTxBuf0;
// 	return wLen;
// }

// WORD CStdReader::GetRequestRecord(DWORD dwOAD, BYTE* pbTxBuf, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
// {
// 	BYTE *pbTxBuf0 = pbTxBuf;	
// 	WORD wLen;
// 
// 	*pbTxBuf++ = DL69845_APPSVR_GETREQUEST; //GET-Request
// 	*pbTxBuf++ = DL69845_APPSVR_GETREQUEST_RECORD;	//GET-Request-RECORD
// 	*pbTxBuf++ = 0x05; //PIID
// 	memcpy(pbTxBuf, (BYTE *)&dwOAD, sizeof(dwOAD));
// 	pbTxBuf += 4;
// 	memcpy(pbTxBuf, pbRSD, bLenRSD);
// 	pbTxBuf += bLenRSD;
// 	memcpy(pbTxBuf, pbRCSD, bLenRCSD);
// 	pbTxBuf += bLenRCSD;
// 
// 	wLen = pbTxBuf - pbTxBuf0;
// 	pbTxBuf = pbTxBuf0;
// 	return wLen;
// }

// int CStdReader::GetResponseNormal(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbDstBuf)
// {
// 	int iRet = -1;
// 	DWORD dwRxOAD;
// 
// 	memcpy((BYTE* )&dwRxOAD, &pbSrcBuf[3], sizeof(dwRxOAD));
// 	if (dwOAD == dwRxOAD)
// 	{
// 		iRet = wSrcLen - 9;
// 		memcpy(pbDstBuf, &pbSrcBuf[7], iRet);
// 	}
// 
// 	return iRet;
// }
// 
// int CStdReader::GetResponseRecord(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbRCSD, BYTE bLenRCSD, BYTE* pbDstBuf)
// {
// 	int iRet = -1;
// 	DWORD dwRxOAD;
// 
// 	memcpy((BYTE* )&dwRxOAD, &pbSrcBuf[3], sizeof(dwRxOAD));
// 	if (dwOAD == dwRxOAD)
// 	{
// 		iRet = wSrcLen - 9 - bLenRCSD;
// 		if (iRet > 0)
// 			memcpy(pbDstBuf, &pbSrcBuf[7+bLenRCSD], iRet);
// 	}
// 
// 	return iRet;
// }

//描述：组RSD和RCSD内容
//		@pbRSD 接收RSD内存
//		@wRSDLen 返回RSDLen
//		@pbRCSD 接收RCSD内存
//		@wRCSDLen 返回RCSDLen
//		@bType 采集方式
//		@pbData 采集方式内容
//		@pbCSD 组RCSD的内容
void CStdReader::GetRSDAndRCSD(DWORD *pdwOAD, BYTE* pbRSD, WORD* wRSDLen, BYTE* pbRCSD, WORD* wRCSDLen, BYTE bMethod, BYTE* pbData, BYTE* pbCSD)
{
	TTime tmNow;
	DWORD dwOAD;
	BYTE bNum;
	BYTE *pbRSD0 = pbRSD;
	BYTE *pbRCSD0 = pbRCSD;
	BYTE *pbCSD0 = pbCSD;

	GetCurTime(&tmNow);		
	switch(bMethod)
	{
		case 0: //采集当前数据
			break;

		case 1:	//采集上第N次
			*pbRSD++ = 9;
			*pbRSD++ = DT_UNSIGN; //unsigned
			*pbRSD++ = *pbData;
			*pdwOAD = ByteToDWord(pbCSD);
			dwOAD = OoOadToDWord(pbCSD);	
			pbCSD += 4;
			break;

		case 2:	//按冻结时标采集	对应冻结时标：0x2021
			*pbRSD++ = 1; //RSD 方法1
			OoDWordToOad(0x20210200, pbRSD);	//OAD=0x20210200	记录选择描述符
			pbRSD += 4;
			*pbRSD++ = 0x1C; //date_time_s

			*pdwOAD = ByteToDWord(pbCSD);
			dwOAD = OoOadToDWord(pbCSD);	
			pbCSD += 4;
			switch (dwOAD)
			{
				case 0x50030200:	//小时冻结
					*pbRSD++ = tmNow.nYear/256;
					*pbRSD++ = tmNow.nYear%256;
					*pbRSD++ = tmNow.nMonth;
					*pbRSD++ = tmNow.nDay;
					*pbRSD++ = tmNow.nHour;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					break;

				case 0x50040200:	//日冻结
				case 0x50050200:	//结算日冻结
					*pbRSD++ = tmNow.nYear/256;
					*pbRSD++ = tmNow.nYear%256;
					*pbRSD++ = tmNow.nMonth;
					*pbRSD++ = tmNow.nDay;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					break;

				case 0x50060200:	//月冻结
					*pbRSD++ = tmNow.nYear/256;
					*pbRSD++ = tmNow.nYear%256;
					*pbRSD++ = tmNow.nMonth;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					break;

				case 0x50070200:	//年冻结
					*pbRSD++ = tmNow.nYear/256;
					*pbRSD++ = tmNow.nYear%256;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					break;
			}
		case 3:	//按时标间隔采集TI
			break;
	}
	*wRSDLen = (WORD)(pbRSD - pbRSD0);

	bNum = *pbCSD;	//关联OAD
	*wRCSDLen = 1 + bNum*4;
	memcpy(pbRCSD, pbCSD, *wRCSDLen);
	pbRCSD = pbRCSD0;
}

//描述：直接执行透传方案报文
int CStdReader::DirectTransSchMsg(BYTE bSchNo, TransFilePara *pTransFilePara, TTransMsg *pTTransMsg)
{
	TTime tTime;
	BYTE bTxBuf[512], bRxBuf[512];
	BYTE *pbRxBuf;
	BYTE bCtrl, bR[6];
	WORD wTxLen, wRxLen;

	//组1376.2帧发送帧
	memset(bRxBuf, 0, sizeof(bRxBuf));
	bRxBuf[0] = 0;	//0:透明传输, 1:DL/T645-1997, 2:DL/T645-2007
	bRxBuf[1] = 0;	//通信延时相关标识
	bRxBuf[2] = 0;	//从节点附属节点数量
	bRxBuf[3] = pTTransMsg->bMsgLen;
	memcpy(bRxBuf+4, pTTransMsg->bMsgBuf, pTTransMsg->bMsgLen);
	wRxLen = pTTransMsg->bMsgLen+4;
	bCtrl = 0x41;
	bR[0] = (1<<2); //D2通信模块标识：0表示对集中器的通信模块操作;1表示对载波表的通信模块操作。
	bR[1] = 0;		//信道
	bR[2] = 0xff;
	bR[3] = 0x0;
	bR[4] = 0;
	bR[5] = 0;
	wTxLen = Make1376_2Frm(pTransFilePara->bTsa, pTransFilePara->bTsaLen,  bCtrl, bR, AFN_RTFWD, FN(1), bRxBuf, wRxLen, bTxBuf);
	memset(bRxBuf, 0, sizeof(bRxBuf));
	//填充固定字段数据[方案编号_1BYTE + 报文执行时间_7byte + 通信地址TSA + 报文序号_1Byte + 报文响应时间_7Byte + data_255Byte]
	pbRxBuf = bRxBuf;
	*pbRxBuf++ = bSchNo;
	GetCurTime(&tTime);
	pbRxBuf += OoTimeToDateTimeS(&tTime, pbRxBuf);
	DWORD dwOAD = 0x40010200;	
	dwOAD = OoOadToDWord((BYTE*)&dwOAD);
	BYTE bTsaLen = OoGetDataLen(81, (BYTE*)&dwOAD);	
	memcpy(pbRxBuf, pTransFilePara->bTsa, pTransFilePara->bTsaLen);
	pbRxBuf += bTsaLen;
	*pbRxBuf++ = pTTransMsg->bSn;
	BYTE *pbRespMsgTime = pbRxBuf;	//先保存报文响应时间
	pbRxBuf += 7;

	for (BYTE bTryCnt=0; bTryCnt<m_RtRunMdInfo.bTrySendCnt; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(m_RtRunMdInfo.bNodeTmOut/m_RtRunMdInfo.bTrySendCnt))
			{
				if (m_TRcv13762.bAfn==AFN_RTFWD && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					BYTE *pbFmtPtr = m_TRcv13762.bDtBuf;
					BYTE bPreCnt;
					WORD wRcvFrmLen;

					pbFmtPtr += 3;	//跳过 2字节“当前报文本地通信上行时长”+ 1字节“通信协议类型”
					wRxLen = *pbFmtPtr++;
					//过滤0xFE
					if (wRxLen > 10)	
					{
						BYTE i;
						for (i = 0; i < 6; i++)
						{
							if (*pbFmtPtr != 0xfe)
								break;
							pbFmtPtr++;
						}
						wRxLen -= i;
					}
					memcpy(pbRxBuf, pbFmtPtr, wRxLen);
					pbRxBuf += wRxLen;
					GetCurTime(&tTime);
					pbRxBuf += OoTimeToDateTimeS(&tTime, pbRespMsgTime);

					SaveTransSchMsg(bSchNo, bRxBuf, pbRxBuf-bRxBuf);
					return wRxLen;
				}
			}
		}
	}

	return -1;
}


int CStdReader::SaveTransSchMsg(BYTE bSchNo, BYTE *pbMsgBuf, WORD wMsgLen)
{
	char pszTableName[32] = {0};

	sprintf(pszTableName, "TranAcqSch_%03d.para", bSchNo);

	return SaveRecord(pszTableName, pbMsgBuf);
}

int CStdReader::DoFwdData(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE *pbRespBuf, bool fGetApdu)
{
	BYTE bCtrl;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE bData[256];
	BYTE bTsaRev[TSA_LEN] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;
	int iRcvLen = -1;

	bCtrl = 0x41;
	bR[0] = (1<<2); //D2通信模块标识：0表示对集中器的通信模块操作;1表示对载波表的通信模块操作。
	bR[1] = 0;		//信道
	bR[2] = 0xff;
	bR[3] = 0x0;
	bR[4] = 0;
	bR[5] = 0;
	wTxLen = Make1376_2Frm(pbTsa, bTsaLen, bCtrl, bR, AFN_RTFWD, FN(1), pbReqBuf, wReqLen, bTxBuf);

	LockDirRd();

	for (BYTE bTryCnt=0; bTryCnt<m_RtRunMdInfo.bTrySendCnt; bTryCnt++)
	{
		if (Send(bTxBuf, wTxLen) == wTxLen)
		{
			if (RxHandleFrm(wTimeOut/m_RtRunMdInfo.bTrySendCnt))
			{
				if (m_TRcv13762.bAfn==AFN_RTFWD && DtToFn(m_TRcv13762.bDt)==FN(1))
				{
					revcpy(bTsaRev, pbTsa, bTsaLen);
					if ((memcmp(bTsaRev, m_TRcv13762.bSrcAddr, 6)==0) || (m_RtRunMdInfo.bModule==AR_LNK_SGD))
					{
						BYTE *pData = m_TRcv13762.bDtBuf + 3;	//跳过 2字节“当前报文本地通信上行时长”+ 1字节“通信协议类型”

						iRcvLen = *pData++;
						for (WORD i=0; i<iRcvLen; i++, iRcvLen--)
						{
							if (*pData==0x68 && *(pData+iRcvLen-1)==0x16)
							{
								TFrm69845 tFrm;

								if (DecodeFram69845(pData, iRcvLen, &tFrm))
								{
									if (m_RtRunMdInfo.bModule == AR_LNK_SGD)
									{
										if (memcmp(tFrm.bSA, pbTsa, 6) != 0)
											break;
									}

									if (!fGetApdu)
									{								
										memcpy(pbRespBuf, tFrm.bAPDUData+3, tFrm.wAPDULen-3);	//-3:调试 类型+类型中的选择+PIID-ACD
										iRcvLen = tFrm.wAPDULen-3;
									}
									else
									{
										memcpy(pbRespBuf, pData, iRcvLen);
									}
								}
								else	//DLT_645
								{
									memcpy(pbRespBuf, pData, iRcvLen);
								}
								UnLockDirRd();
								return iRcvLen;
							}
							pData++;
						}
					}
					else
					{
						char szSendAddr[16] = {0};
						char szRecvAddr[16] = {0};

						DTRACE(DB_CCT, ("DoFwdData(): Cct meter mismatch, Send meter addr=%s, Recv meter addr=%s!\n", \
									HexToStr(bTsaRev, 6, szSendAddr), HexToStr(m_TRcv13762.bSrcAddr, 6, szRecvAddr)));
					}
				}
			}
		}
	}

RET_DOFWD:
	pbRespBuf[0] = 0;	//DAR
	pbRespBuf[1] = 33;	//请求超时
	pbRespBuf[2] = 0;	//上报
	pbRespBuf[3] = 0;	//时间标签
	UnLockDirRd();

	return 4;
}

//描述：初始化任务ID调度，初始化一个有效的任务配置单元
void CStdReader::InitTaskIdSched()
{
	memset((BYTE*)&m_TRunStateInfo, 0, sizeof(m_TRunStateInfo));
}

//描述：透明任务控制抄读
bool CStdReader::DoTaskIdSched()
{
	TTaskCfg tTaskCfg;
	DWORD dwCurSec, dwStartSec, dwEndSec;
	bool fRet = false;

	for (WORD wIndex=1; wIndex<=TASK_ID_NUM; wIndex++)
	{
		memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
		if (GetTaskCfg(wIndex, (TTaskCfg*)&tTaskCfg) && 
				tTaskCfg.bSchType==SCH_TYPE_TRANS)
		{
			if (GetTaskCurExeTime(&tTaskCfg, &dwCurSec, &dwStartSec, &dwEndSec) != 0)
				continue;

			if (GetTaskUpdateTime(tTaskCfg.bTaskId) == dwCurSec)
				continue;

			//初始化透明方案开始执行参数
			if (!m_TRunStateInfo.tTransSchSate.fStart)
			{
				m_TRunStateInfo.tTransSchSate.fStart = true;
				m_TRunStateInfo.tTransSchSate.fFinsh = false;
				m_TRunStateInfo.tTransSchSate.iStart = -1;
				m_TRunStateInfo.tTransSchSate.iMsgSn = 0;
				m_TRunStateInfo.tCurExeTaskCfg = tTaskCfg;
			}

			//当前透明任务已经执行完成
			if (m_TRunStateInfo.tTransSchSate.fFinsh)
				SetTaskUpdateTime(tTaskCfg.bTaskId, dwCurSec);
			fRet = true;
		}
	}

	return fRet;
}

//描述：通过时间间隔(秒、分、时)判断是否是直抄的条件
bool CStdReader::IsDirReadTi(TTimeInterv *pTi)
{
	if (pTi->bUnit>=TIME_UNIT_SECONDS && pTi->bUnit<=TIME_UNIT_HOUR)
		return true;
	return false;
}


#define MINNUM_MTR_NUM		10//200//10
#define MAXNUM_PERCENTAGE	(100)	//日冻结任务完成百分比
#define MAXNUM_HOUR			4		//日冻结任务抄读截至小时
#define MAXNUM_MIN			00		//日冻结任务抄读截至分钟

//描述：查询日冻结任务的抄读状态
//返回：返回true表示日冻结任务抄读完成，或日冻结任务超出时间控制
bool CStdReader::QueryFrzTaskReadState()
{
	TTime tMaxRdFrzTime;
	TMtrRdCtrl *pMtrRdCtrl;
	TTaskCfg tTaskCfg;
	int iSchCfgLen;
	DWORD dwNowSec, dwRdMaxFrzSec;
	WORD wFmtLen, wLen, wFieldLen, wTolCsdNum, wSucCsdNum, wPn, wTolPnNum, wSucPnNum;
	BYTE *pbSch, *pFmt, *pbFieldFmt, *pbTaskCSD;
	const BYTE *pbMtrMask;
	BYTE bType, bTsa[TSA_LEN];
	BYTE bTaskIdx;
	WORD wPnToAllTaskNum, wPnToSucTaskNum;

	if (!m_TRtStat.fIsNeedRtReq)
		return true;

	GetCurTime(&tMaxRdFrzTime);
	tMaxRdFrzTime.nHour = MAXNUM_HOUR;
	tMaxRdFrzTime.nMinute = MAXNUM_MIN;
	dwRdMaxFrzSec = TimeToSeconds(tMaxRdFrzTime);
	dwNowSec = GetCurTime();
	if (dwNowSec > dwRdMaxFrzSec)
		return true;

	pbMtrMask = GetPlcPnMask();
	if (IsAllAByte(pbMtrMask, 0, PN_MASK_SIZE))
		return true;

	wSucPnNum = 0;
	wTolPnNum = CalcuBitNum(pbMtrMask, PN_MASK_SIZE);
	if (wTolPnNum >= MINNUM_MTR_NUM)	//超过MINNUM_MTR_NUM块表，进行抄表优先级时间控制
	{
		for (WORD wMaskIdx=0; wMaskIdx<PN_MASK_SIZE; wMaskIdx++)
		{
			if (pbMtrMask[wMaskIdx] != 0)
			{
				for (BYTE bIdx=0; bIdx<8; bIdx++)
				{
					if (!(pbMtrMask[wMaskIdx] & (1<<bIdx)))
						continue;

					wPn = wMaskIdx*8 + bIdx;

					memset(bTsa, 0, sizeof(bTsa));
					bTsa[0] = GetMeterTsa(wPn, bTsa+1);
					pMtrRdCtrl = GetMtrRdCtrl(wPn, bTsa);
					if (pMtrRdCtrl == NULL)
						return false;

					wPnToAllTaskNum=0;	//表地址对应的所有有效任务个数
					wPnToSucTaskNum=0;	//表地址对应的所有抄读成功的有效任务个数
					bTaskIdx = 0;
					while (bTaskIdx < MTR_TASK_NUM)
					{
						if (pMtrRdCtrl->taskSucFlg[bTaskIdx].bValid == 1)
						{	
							wPnToAllTaskNum++;	

							if (GetTaskCfg(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, &tTaskCfg) /*&& !IsDirReadTi(&tTaskCfg.tiExe)*/)	//是日、月冻结间隔才进行抄表成功率判断
							{
								pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
								if (pbSch!=NULL)
								{
									BYTE bArryCsdIdx;
									switch (tTaskCfg.bSchType)
									{
										case SCH_TYPE_COMM:
											pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
											bArryCsdIdx = 3;
											break;
										case SCH_TYPE_EVENT:
											pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
											bArryCsdIdx = 1;
											break;
										default:
											continue;
									}

									if (pFmt != NULL)
									{
										if ((pbTaskCSD=OoGetField(pbSch, pFmt, wFmtLen, bArryCsdIdx, &wLen, &bType, &pbFieldFmt, &wFieldLen)) != NULL)
										{
#if 0	//遍历方案的RCSD判断成功率
											wTolCsdNum = pbTaskCSD[1];
											wSucCsdNum = CalcuBitNum(pMtrRdCtrl->taskSucFlg[bTaskIdx].bSucFlg, TASK_SUC_FLG_LEN);
											if (wTolCsdNum == wSucCsdNum)	//表地址对应的采集方案数据项是否采集成功
												wPnToSucTaskNum++;
#else	//通过保存的成功标识判断成功率
											if (pMtrRdCtrl->taskSucFlg[bTaskIdx].fRecSaved)
												wPnToSucTaskNum++;
#endif
										}
										else
										{
											wPnToSucTaskNum++;
										}
									}
									else
									{
										wPnToSucTaskNum++;
									}
								}
							}
							else 
							{
								wPnToSucTaskNum++;
							}
						}
						bTaskIdx++;
					}

					//相等，表示该表地址对应的所有日冻结任务全部抄读完了
					if (wPnToAllTaskNum==0 || wPnToSucTaskNum==wPnToAllTaskNum)	
						wSucPnNum++;

					PutMtrRdCtrl(wPn, bTsa, true);
				}
			}
		}

		if ((float)wSucPnNum/wTolPnNum*100  < MAXNUM_PERCENTAGE)
			return false;
	}

	return true;
}

//描述：执行采集方案
bool CStdReader::DoAllAcqSch()
{
	bool fRet = false;

	if (DoTaskIdSched())
		DoTransSch();

	MtrBroadcast();
	BroadcastAdjustTime();

	if (QueryFrzTaskReadState())
		fRet = DoCommSch();
	return fRet;
}

bool CStdReader::DoCommSch()
{
	BYTE bPerRdMtrNum = 1;
	bool fIsRet = false;

	while (bPerRdMtrNum++ <= 5)
	{
		TMtrRdCtrl* pMtrRdCtrl;
		TMtrPara tMtrPara;
		TRdItem tRdItem;
		int iRet;
		BYTE bRxBuf[1024];
		BYTE bTsa[TSA_LEN+1];	
		BYTE bCn;
		char szTsa[32];

		m_iPn = SearchNextPnFromMask((BYTE*)GetPlcPnMask(), m_iPn);
		if (m_iPn < 0)
		{
			m_iPn = -1;
			return false;
		}
		memset(bTsa, 0, sizeof(bTsa));
		bTsa[0] = GetMeterTsa(m_iPn, bTsa+1);
		pMtrRdCtrl = GetMtrRdCtrl(m_iPn, bTsa);
		if (pMtrRdCtrl == NULL)
			return false;
		DoTaskSwitch(pMtrRdCtrl);
		GetMeterPara(m_iPn, &tMtrPara);

		while (1)
		{
			if (m_TRunStateInfo.fDirRdFlg /*|| !GetTaskCyleUnit(pMtrRdCtrl)*/)
			{
				fIsRet = true;
				break;
			}

			memset((BYTE*)&tRdItem, 0, sizeof(tRdItem));
			iRet = SearchAnUnReadID(GetCurPrio(bCn=0), m_iPn, pMtrRdCtrl, &tRdItem, true);
			switch(iRet)
			{
				case RD_ERR_OK:
					SaveTask(pMtrRdCtrl);
					goto RET_DoCommSch;
				case RD_ERR_UNFIN:
					if (tMtrPara.bProId==PROTOCOLNO_DLT645 || tMtrPara.bProId==PROTOCOLNO_DLT645_V07)
					{
						memset(bRxBuf, 0, sizeof(bRxBuf));
						if (ReadDLT_645(pMtrRdCtrl, &tRdItem, &tMtrPara, bRxBuf, sizeof(bRxBuf)) < 0)
							goto RET_DoCommSch;
					}
					else if (tMtrPara.bProId == PROTOCOLNO_DLT69845)
					{
						memset(bRxBuf, 0, sizeof(bRxBuf));
						if (ReadDLT_69845(pMtrRdCtrl, &tRdItem, &tMtrPara, bRxBuf, sizeof(bRxBuf)) < 0)
							goto RET_DoCommSch;
					}
					else if (tMtrPara.bProId == PROTOCOLNO_SBJC)
					{
						memset(bRxBuf, 0, sizeof(bRxBuf));
						if (ReadDLT_SBJC(pMtrRdCtrl, &tRdItem, &tMtrPara, bRxBuf, sizeof(bRxBuf)) < 0)
							goto RET_DoCommSch;
					}
					else 
					{
						memset(szTsa, 0, sizeof(szTsa));
						DTRACE(DB_CCT, ("DoCommSch(): Nonsupport protocol, Mtr addr=%s, Proto=%d.\n", HexToStr(bTsa+1, bTsa[0], szTsa), tMtrPara.bProId));
						goto RET_DoCommSch;
					}
					break;
				case RD_ERR_UNTIME:
					SaveTask(pMtrRdCtrl);
					goto RET_DoCommSch;
				case RD_ERR_HALT:
					SaveTask(pMtrRdCtrl);
					goto RET_DoCommSch;
				case RD_ERR_CHKTSK:
					memset(szTsa, 0, sizeof(szTsa));
					DTRACE(DB_CCT, ("Afn13-F1: Loop search no Read Mtr:0x%s, RealLoopCnt=%d, MaxLoopCnt=%d.\n", 
								HexToStr(bTsa+1, bTsa[0], szTsa),pMtrRdCtrl->schItem.bLoopCnt, LOOP_MAX_CNT));
					break;
				default:
					SaveTask(pMtrRdCtrl); 
					goto RET_DoCommSch;
			}
		}

RET_DoCommSch:
		PutMtrRdCtrl(m_iPn, bTsa, true);
		if (fIsRet)
			break;
	}

	return false;
}


bool CStdReader::DoTransSch()
{
	TransFilePara tTransFilePara;
	TTransMsg tTransMsg;

	memset((BYTE*)&tTransFilePara, 0, sizeof(tTransFilePara));
	memset((BYTE*)&tTransMsg, 0, sizeof(tTransMsg));

	m_TRunStateInfo.tTransSchSate.fFinsh = GetTransSchParam(m_TRunStateInfo.tCurExeTaskCfg.bSchNo, 
			&m_TRunStateInfo.tTransSchSate.iStart, 
			&m_TRunStateInfo.tTransSchSate.iMsgSn, 
			&tTransFilePara, 
			&tTransMsg);
	if (!m_TRunStateInfo.tTransSchSate.fFinsh)
	{
		RouterPause();
		DirectTransSchMsg(m_TRunStateInfo.tCurExeTaskCfg.bSchNo, &tTransFilePara, &tTransMsg);
		return false;
	}

	return true;
}


#define PER_RD_MTR_CNT	10
//描述：更新电表档案到路由模块
bool CStdReader::SyncMeterAddr()
{
	TOobMtrInfo tTMtrInfo;
	WORD wRtMtrNum = 0;
	WORD wStartMtrSn;
	BYTE bTsa[TSA_LEN], bTsaRev[TSA_LEN], bMtrPro, bAddNum;
	BYTE bBuf[512] = {0};
	BYTE *pbPtr;
	bool fSameMtrFlg = false;

	TMtrInfo tDbMtrInfo[POINT_NUM];
	WORD wDbMtrNum;

	memset((BYTE*)&tDbMtrInfo, 0, sizeof(tDbMtrInfo));
	wDbMtrNum = GetPlcNodeAddr(tDbMtrInfo, POINT_NUM);
	if (wDbMtrNum == 0)
	{
		Afn01Fn02_ParmInit();
		return true;
	}

	wRtMtrNum = Afn10Fn01_RdRtNodeNum();
	if (wRtMtrNum==0 || abs(wDbMtrNum-wRtMtrNum)>5)	//档案个数超过5个直接参数区初始化,再将系统库的档案添加到路由模块
	{
		if (Afn01Fn02_ParmInit())
		{
			bAddNum = 0;
			pbPtr = bBuf + 1;

			for (WORD k=0; k<wDbMtrNum; k++)
			{
				if (tDbMtrInfo[k].bMtrPro == PRO_TYPE_69845)	//面向对象协议698.45转化为透明传输协议
				{
					if (m_RtRunMdInfo.bModule==AR_LNK_ES || m_RtRunMdInfo.bModule==AR_LNK_LS)
						bMtrPro = PRO_TYPE_69845;
					else
						bMtrPro = 0;
				}
				else
					bMtrPro = tDbMtrInfo[k].bMtrPro;

				//revcpy(pbPtr,tDbMtrInfo[k].bTsa, 6);	
				EncodeRouterMtrAddr(tDbMtrInfo[k].bTsa, tDbMtrInfo[k].bTsaLen);
				memcpy(pbPtr,tDbMtrInfo[k].bTsa, 6);	

				pbPtr += 6;
				*pbPtr++ = bMtrPro;
				bAddNum++;

				if (bAddNum >= 10)
				{
					bBuf[0] = bAddNum;
					Afn11Fn01_AddNode(bBuf, pbPtr - bBuf);
					memset(bBuf, 0, sizeof(bBuf));
					pbPtr = bBuf + 1;
					bAddNum = 0;
				}
			}

			if (bAddNum > 0)
			{
				bBuf[0] = bAddNum;
				Afn11Fn01_AddNode(bBuf, pbPtr - bBuf);
			}
		}	
	}
	else //比对档案
	{
		WORD wValidRtMtrNum = 0;
		BYTE bRdRoutAddr[POINT_NUM][8]={0};
		BYTE bNoExitMtrMask[PN_MASK_SIZE] = {0};
		BYTE bRdRtMtrMask[PN_MASK_SIZE] = {0};
		WORD wRtTotalMtrNum;
		BYTE bRdMtrNum;

		//读取路由模块里档案，比对在系统库里是否存在
		wStartMtrSn = 1;
		while (wStartMtrSn<=POINT_NUM && wValidRtMtrNum<wRtMtrNum)
		{
			memset(bBuf, 0, sizeof(bBuf));
			pbPtr = bBuf;
			if (Afn10Fn02_RdNodeInfo(PER_RD_MTR_CNT, wStartMtrSn, bBuf))
			{
				wRtTotalMtrNum = ByteToWord(pbPtr);	
				pbPtr += 2;
				bRdMtrNum = *pbPtr++;
				for (BYTE i=0; i<bRdMtrNum; i++)
				{
					//revcpy(bRdRoutAddr[wValidRtMtrNum], pbPtr, 6);
					memcpy(bRdRoutAddr[wValidRtMtrNum], pbPtr, 6);

					pbPtr += 6;
					pbPtr += 2;	//跳过从节点信息
					fSameMtrFlg = false;
					for (WORD m=0; m<wDbMtrNum; m++)
					{
						BYTE bTmpTsa[TSA_LEN] = {0};
						memcpy(bTmpTsa, tDbMtrInfo[m].bTsa, TSA_LEN);
						EncodeRouterMtrAddr(bTmpTsa, tDbMtrInfo[m].bTsaLen);
						if (memcmp(bRdRoutAddr[wValidRtMtrNum], bTmpTsa, 6) == 0)
						{
							fSameMtrFlg = true;
							break;
						}
					}

					if (fSameMtrFlg)	//将系统库中不存在的电表记录下来，方便后面统一删除
						bRdRtMtrMask[wValidRtMtrNum/8] |= (1<<(wValidRtMtrNum%8));
					else
						bNoExitMtrMask[wValidRtMtrNum/8] |= (1<<(wValidRtMtrNum%8));
					wValidRtMtrNum ++;	
				}
			}

			wStartMtrSn += PER_RD_MTR_CNT;
		}

		//终端里的电表地址与路由模块电表地址不一致，删除不一致的电表
		if (!IsAllAByte(bNoExitMtrMask, 0, PN_MASK_SIZE))	
		{
			BYTE bDelMtrAddr[PER_RD_MTR_CNT*6+1] = {0};
			BYTE *pbPtr = bDelMtrAddr+1;
			BYTE bDelCnt = 0;
			for (WORD i=0; i<PN_MASK_SIZE; i++)
			{
				if (bNoExitMtrMask[i])
				{
					for (BYTE bBit = 0; bBit < 8; bBit++)
					{
						if (bNoExitMtrMask[i] & (1<<bBit))
						{
							WORD wNoExitSn = i*8 + bBit;

							//revcpy(pbPtr, bRdRoutAddr[wNoExitSn], 6);	
							memcpy(pbPtr, bRdRoutAddr[wNoExitSn], 6);	
							pbPtr += 6;
							bDelCnt++;
						}

						if (bDelCnt >= 10)
						{
							bDelMtrAddr[0] = bDelCnt;
							Afn11Fn02_DelNode(bDelMtrAddr, pbPtr-bDelMtrAddr);
							bDelCnt = 0;
							memset(bDelMtrAddr, 0, sizeof(bDelMtrAddr));
							pbPtr = bDelMtrAddr+1;
						}
					}
				}
			}
			if (bDelCnt > 0)
			{
				bDelMtrAddr[0] = bDelCnt;
				Afn11Fn02_DelNode(bDelMtrAddr, pbPtr-bDelMtrAddr);
			}
		}
		//比对档案
		BYTE bAddMtr[1+PER_RD_MTR_CNT*7] = {0};
		BYTE bAddMtrCnt = 0;
		BYTE *pbAddMtr = bAddMtr + 1;

		for (WORD wDbMtrIdx=0; wDbMtrIdx<wDbMtrNum; wDbMtrIdx++)
		{
			fSameMtrFlg = false;
			for (WORD wRtMtrIdx=0; wRtMtrIdx<wValidRtMtrNum; wRtMtrIdx++)
			{
				BYTE bTmpTsa[TSA_LEN] = {0};
				memcpy(bTmpTsa, tDbMtrInfo[wDbMtrIdx].bTsa, TSA_LEN);
				EncodeRouterMtrAddr(bTmpTsa, tDbMtrInfo[wDbMtrIdx].bTsaLen);
				if (memcmp(bTmpTsa, bRdRoutAddr[wRtMtrIdx], 6) == 0)
				{
					fSameMtrFlg = true;
					break;
				}
			}

			if (!fSameMtrFlg)
			{
				//revcpy(pbAddMtr, tDbMtrInfo[wDbMtrIdx].bTsa, 6);	

				EncodeRouterMtrAddr(tDbMtrInfo[wDbMtrIdx].bTsa, tDbMtrInfo[wDbMtrIdx].bTsaLen);
				memcpy(pbAddMtr, tDbMtrInfo[wDbMtrIdx].bTsa, 6);	
				pbAddMtr += 6;

				if (tDbMtrInfo[wDbMtrIdx].bMtrPro == PRO_TYPE_69845)	//面向对象协议698.45转化为透明传输协议
				{
					if (m_RtRunMdInfo.bModule==AR_LNK_ES || m_RtRunMdInfo.bModule==AR_LNK_LS)
						bMtrPro = PRO_TYPE_69845;
					else
						bMtrPro = 0;
				}
				else
					bMtrPro = tDbMtrInfo[wDbMtrIdx].bMtrPro;

				*pbAddMtr++ = bMtrPro;

				bAddMtrCnt++;

				if (bAddMtrCnt >= 10)
				{
					bAddMtr[0] = bAddMtrCnt;
					Afn11Fn01_AddNode(bAddMtr, pbAddMtr - bAddMtr);
					memset(bAddMtr, 0, sizeof(bAddMtr));
					pbAddMtr = bAddMtr + 1;
					bAddMtrCnt = 0;
				}
			}
		}

		if (bAddMtrCnt > 0)
		{
			bAddMtr[0] = bAddMtrCnt;
			Afn11Fn01_AddNode(bAddMtr, pbAddMtr - bAddMtr);
		}
		return true;
	}

	return false;
}



void CStdReader::CctRunStateMonitor()
{
	TTime tNowTime;
	const BYTE bDlyMin = 1;

	if (m_TRtStat.fIsNeedRtReq)
	{
		if (GetClick()-m_TRunStateInfo.dwLstDirClk > 120)
		{
			m_TRunStateInfo.dwLstDirClk = GetClick();
			GetCurTime(&tNowTime);
			if (!IsSameDay(m_TRunStateInfo.tLastUdpTime, tNowTime) && 
					(tNowTime.nMinute>bDlyMin))	//是否跨日
			{
				m_TRunStateInfo.tLastUdpTime = tNowTime;
				m_TRunStateInfo.fRtPause = false;	//重启路由
				DTRACE(DB_CCT, ("Interval day restart router.\n"));
				CheckMeterSearchTime();
				Afn12Fn01_RtRestart();
			}
			else if (m_TRunStateInfo.fRtPause && (m_bCctExeState!=CCT_SCH_MTR))
			{
				m_TRunStateInfo.fRtPause = false;	//恢复路由
				Afn12Fn03_RtResume();
			}

			//间隔3小时重启一次路由
			BYTE bRtRstHour = 3;
			if ((tNowTime.nHour%bRtRstHour==0 && m_bCctExeState!=CCT_SCH_MTR) && 
					(tNowTime.nHour != m_TRunStateInfo.tLastUdpTime.nHour) &&
					(tNowTime.nMinute>bDlyMin))
			{
				m_TRunStateInfo.tLastUdpTime = tNowTime;
				DTRACE(DB_CCT, ("Interval %d hour restart router.\n", bRtRstHour));
				Afn12Fn01_RtRestart();
			}
		}
	}
}

bool CStdReader::StartBoardCast(int iMin)
{
	int fRet;
	BYTE bBdAddr[6] = {0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
	BYTE bData[16];

	memset(bData, 0, sizeof(bData));
	bData[0] = 0x02;	//00H＝透明传输；01H＝DL/T645—1997；02H＝DL/T645—2007；03H＝相位识别功能；04H～FFH保留。

	bData[12] = iMin%256;
	bData[13] = iMin/256;
	bData[1] = Make645Frm(bData+2, bBdAddr, 0x13, 2);

	fRet = Afn05Fn3_StartBoardCast(bData, bData[1]+2);
	if (fRet)
	{
		DTRACE(DB_CCT, ("AFN=05 Fn=03 Start broadcast successful, Need waiting %d minutes...\n", iMin));
	}

	return fRet;
}

//描述：启动搜表
//返回：-1-搜表失败，0-广播启动结束，1-正在广播
int CStdReader::StartSchMtr()
{
	int iBroadCastMin;

	//获取参数失败直接退出，结束当前搜表
	if (!GetSchMtrParam(&m_TSchMtrParm))
		return -1;

	//时段搜表 
	if (m_fPeriodSchMtr)
	{
		//未设置为每天周期搜表
		if (!m_TSchMtrParm.fAutoSchMtr)
			return -1;
		//不在搜表时段
		if (!IsSchMtrPeriod())
			return -1;
	}

	//备注：1.每天周期搜表前清空	每天周期搜表是0x60020900
	//		2.每次搜表前清空		每次搜表包括立即启动搜表，以及每天周期搜表
	if (m_TSchMtrParm.bClrMtrChoice==1 && m_fPeriodSchMtr && !m_fClrFile)	//每天周期搜表前清空,不包括立即启动搜表
	{
		m_fClrFile = true;
		DeleteSearchMtrFile();
		DeleteCrossSearchMtrFile();
		DTRACE(DB_CCT, ("Daily cycle search meter before clear File.\n"));
	}	
	else if (m_TSchMtrParm.bClrMtrChoice==2 && !m_fClrFile)	//每次搜表前清空
	{
		m_fClrFile = true;
		DeleteSearchMtrFile();
		DeleteCrossSearchMtrFile();
		DTRACE(DB_CCT, ("Each times search meter before clear File.\n"));
	}

	RouterPause();

	SetSchMtrState(true);

	if (m_RtRunMdInfo.bModule == AR_LNK_TC)
	{
		if (m_fRightNowSchMtr)
			iBroadCastMin = 4;
		else
			iBroadCastMin = 60;

		if (!m_fStartBoardCast)
		{
			StartBoardCast(iBroadCastMin);
			m_fStartBoardCast = true;
			m_dwStartBoardCastClk = GetClick();
		}

		if (m_fStartBoardCast)
		{
			int iCount = GetClick() - m_dwStartBoardCastClk;
#ifndef SYS_WIN
			int iBroadCastSec = iBroadCastMin*60;
#else
			int iBroadCastSec = 20;
#endif
			if (iCount < iBroadCastSec)	//启动广播搜表时间
			{
				DTRACE(DB_CCT, ("Start search meter count down %ds.\n", iBroadCastSec - iCount));
				return 1;
			}
		}
	}
	else
		m_dwStartBoardCastClk = GetClick();

	m_dwLastRptMtrClk = GetClick();
	m_fRptSchMtrEnd = false;

	return 0;
}

bool CStdReader::StartNodeActive()
{
	TTime tNowTime;
	int iActWaitMin;
	BYTE bBuf[32];
	BYTE *p = bBuf;

	iActWaitMin = GetNodeActWaitMin();

	GetCurTime(&tNowTime);
	*p++ = 0x00;
	*p++ = ByteToBcd(tNowTime.nMinute);
	*p++ = ByteToBcd(tNowTime.nHour);
	*p++ = ByteToBcd(tNowTime.nDay);
	*p++ = ByteToBcd(tNowTime.nMonth);
	*p++ = ByteToBcd(tNowTime.nYear%100);
	*p++ = iActWaitMin%256;	
	*p++ = iActWaitMin/256;	
	*p++ = 0x03;
	*p++ = 0x01;
	if (!Afn11Fn05_ActSlaveNodeRpt(bBuf, p - bBuf))
	{
		DTRACE(DB_CCT, ("Active slave node report fail!\n"));
		return false;
	}

	DTRACE(DB_CCT, ("Active slave node report successful, Need waiting iActWaitMin=%ds.\n", iActWaitMin*60));

	return true;
}

bool CStdReader::WaitMtrReport()
{
	//非立即启动搜表 && （时段搜表 且 不在搜表时段）
	if (!m_fRightNowSchMtr && (m_fPeriodSchMtr && !IsSchMtrPeriod()))
		return true;

	if (m_fRightNowSchMtr)
	{
		int iKeptTime = GetRightNowSchMtrKeepTime();

		if(iKeptTime <= 0)	//0表示不限时间直至搜表结束
			iKeptTime = 120;	//给个120min

		if (GetClick() - m_dwStartBoardCastClk > iKeptTime*60)	//立即启动搜表结束
			return true;
	}

	if (GetClick() - m_dwLastRptMtrClk >= 10*60)
	{
		BYTE bBuf[128];

		memset(bBuf, 0, sizeof(bBuf));
		if (Afn10Fn04_QueryRtRunInfo(bBuf)>0)
        {
		    if ((bBuf[0] & 0x03)==0x01)	//Bit0=1,为路由完成标志
            {      
			    m_fRptSchMtrEnd = true;
            }
                /*
            if (bBuf[13]==0x08 && bBuf[14]==0x08 && bBuf[15]==0x08)	//青岛鼎信要求AFN=10 F4返回的数据中，三相工作步骤均为08时载波从节点主动注册认为结束
            {// 这种判断是否通用，需要确认
                m_fRptSchMtrEnd = true;            
            }
            */
        }			

		if (m_fRptSchMtrEnd)
        {      
			return true;
        }

		m_dwLastRptMtrClk = GetClick();

        //是否再次激活从节点主动注册有待商榷，对于中电华瑞、深国电宽带载波，相当于重启一次搜表
		//StartNodeActive();	//超过10分钟，再次启动从节点注册
	}

	if (m_fRptSchMtrEnd)
		return true;

	return false;
}

int CStdReader::FinishSchMtr()
{
	Afn11Fn06_StopSlaveNodeRpt();
	DTRACE(DB_CCT, ("Finish search meter.\n"));

	SetSchMtrState(false);
	if (GetUdpMtrFlg())
		SetInfo(INFO_MTR_UPDATE);

	return true;
}

//描述：执行其他任务，包括搜表、透明方案、直抄数据
void CStdReader::DoOtherTask()
{
	if (!m_TRtStat.fPlcInit)
		return;

	if (DoSchMtrAddr())	//不在搜表
	{
		m_bCctExeState = CCT_SCH_MTR;
	}
	else
	{
		DoAllAcqSch();
		m_bCctExeState = CCT_RD_MTR;
	}

	CctRunStateMonitor();
}

//描述：通过串口向载波路由发数据
//返回：已发送长度
DWORD CStdReader::Send(BYTE *pSendBuf, DWORD dwLen)
{
	if (!m_Comm.IsOpen())
	{
		DTRACE(DB_CCT, ("CStdReader :: Send : com is closed.\n"));
		return 0;
	}

	if (dwLen>0 && IsDebugOn(DB_CCTTXFRM))
	{	
		char szBuf[48];
		sprintf(szBuf, "%s --> ", m_pszName);
		TraceBuf(DB_CCTTXFRM, szBuf, pSendBuf, dwLen);
	}

	return m_Comm.Write(pSendBuf, dwLen);
}

//描述：载波路由的数据
//返回：接收到的长度
DWORD CStdReader::Receive(BYTE *pRecvBuf, DWORD wLen)
{
	if (!m_Comm.IsOpen())
	{
		DTRACE(DB_CCT, ("CStdReader :: Receive : com is closed.\n"));
		return 0;
	}
	DWORD dwRet = 0;
	dwRet = m_Comm.Read(pRecvBuf, wLen);

	if (dwRet>0 && IsDebugOn(DB_CCTRXFRM))
	{	
		char szBuf[48];
		sprintf(szBuf, "%s <-- ", m_pszName);
		TraceBuf(DB_CCTRXFRM, szBuf, pRecvBuf, dwRet);
	}

	return dwRet;
}

//描述:	接收一个数据块,判断是否接收到一个完整的通信帧
//返回:	返回已经扫描过的字节数,如果收到一个完整的通信帧则返回正数,否则返回负数
int CStdReader::RcvFrame(BYTE* pbBlock, int nLen)
{
	BYTE *pbPtr = pbBlock;
	bool fFramIsOk = false;

	m_fRxComlpete = false;

	memset((BYTE*)&m_TRcv13762, 0, sizeof(m_TRcv13762));
	if (nLen > 0)
	{
		BYTE bStep = 0;
		for (int i = 0; i < nLen; i++)
		{
			BYTE b = *pbPtr;
			switch(bStep)
			{
				case 0:	//0x68 
					if (b == 0x68)
					{
						m_TRcv13762.bHead = 0x68;
						bStep = 1;
					}
					pbPtr++;
					break;
				case 1:	//Len
					m_TRcv13762.wDataLen = ByteToWord(pbPtr);	pbPtr += 2;
					bStep = 2;
					break;
				case 2:	//Ctrl
					m_TRcv13762.bCtrl = b;	pbPtr++;
					bStep = 3;
					break;
				case 3:	//R Addr
					memcpy(m_TRcv13762.bR, pbPtr, 6);	pbPtr += 6;	i += 6;
					if (m_TRcv13762.bR[0] & 0x04)	//通信模块标识,为0时，无地址域A
					{
						BYTE bRelayNum = (m_TRcv13762.bR[0]>>4);

						memcpy(m_TRcv13762.bSrcAddr, pbPtr, 6); 
						pbPtr += 6; i += 6;

						for (BYTE j = 0; j < bRelayNum; j++)
						{
							memcpy(&m_TRcv13762.bRelayAddr[j*6], pbPtr, 6); 
							pbPtr += 6; i += 6;
						}
						memcpy(m_TRcv13762.bDesAddr, pbPtr, 6); 
						pbPtr += 6; i += 6;
					}
					bStep = 4;
				case 4:	//AFN	Dt
					b=*pbPtr++; 
					m_TRcv13762.bAfn = b;	i++;
					b=*pbPtr++;
					m_TRcv13762.bDt[0] = b;	i++;
					b=*pbPtr++;
					m_TRcv13762.bDt[1] = b;	i++;
					bStep = 5;
					break;
				case 5:	//user data
					if (m_TRcv13762.wDataLen > i+2)
					{
						m_TRcv13762.wDtLen = m_TRcv13762.wDataLen-i-2;	//-2 : CS 0x16
						memcpy(m_TRcv13762.bDtBuf, pbPtr, m_TRcv13762.wDtLen);
					}
					else
					{
						m_TRcv13762.wDtLen  = 0;	
					}

					pbPtr += m_TRcv13762.wDtLen;	
					i += m_TRcv13762.wDtLen;
					b = *pbPtr++;
					bStep = 6;
				case 6:
					m_TRcv13762.bCs = b;	
					bStep = 7;
					break;
				default:
					if (b == 0x16)
					{
						BYTE bBuf[512] = {0};
						BYTE bCs;
						WORD wLen;

						m_TRcv13762.bEnd = 0x16;
						wLen = Pro1376_2ToBuf((TFrm13762*)&m_TRcv13762, bBuf);
						if (CheckCS(bBuf+3, wLen-5) == m_TRcv13762.bCs)
						{
							m_fRxComlpete = true;
							return i+1;
						}
					}
			}
		}
	}

	return -nLen;
}

//描述：载波协议层解析函数
//返回：针对不能解析的直接返回false
bool CStdReader::DefHanleFrm()
{
	bool fRet = false;
	BYTE bFn = DtToFn(m_TRcv13762.bDt);
	BYTE bAfn = m_TRcv13762.bAfn;

	switch(bAfn)
	{
		case AFN_CON:
			fRet = false;
			break;
		case AFN_QRYDATA:
			if (bFn == FN(10))
				Afn03Fn10_RptRtRunInfo(m_TRcv13762.bDtBuf);
			break;
		case AFN_REP:
			if (bFn == FN(1))	//上报从节点信息
				Afn06Fn01_RptNodeInfo();
			else if (bFn == FN(2))	//上报抄读数据
				Afn06Fn02_RptData();
			else if (bFn == FN(3))	//上报路由工况变动信息
				Afn06Fn03_RptRtInfo();
			else if (bFn == FN(4))	//上报从节点信息及设备类型
				Afn06Fn04_RptMtrInfo();
			else if (bFn == FN(5))	//上报从节点事件
				Afn06Fn5_RptNodeEvt();
			else
				fRet = true;
			break;
		case AFN_RTRD:
			if (bFn == FN(1))	//路由请求抄读内容
				Afn14Fn1_RtReqRd();
			else if (bFn == FN(2))	//路由请求集中器时钟
				Afn14Fn2_RtReqClk();
			else
				fRet = true;
			break;
		default:
			fRet = true;
	}

	if (fRet)
	{
		//DTRACE(DB_CCT, ("CStdReader::DefHanleFrm(): AFN=0x%02x Fn=%02d, unsupport!\n", bAfn, bFn));
		return false;
	}

	return false;
}

//描述：数据处理
bool CStdReader::RxHandleFrm(DWORD dwSeconds, bool fIsDefHanleFrm)
{
	DWORD dwOldTick = GetTick();
	int len = 0;
	BYTE bBuf[1024];

	if (dwSeconds == 0)
		dwOldTick = GetTick()-1;
	do
	{	
		memset(bBuf, 0, sizeof(bBuf));
		len = m_LoopBuf.GetBufLen();

		if (len <= 0 || !m_fRxComlpete)	//优先处理循环缓冲区的数据，或者缓冲区有数据，但接收不完整，所以应去把剩下的接收回来一起处理
		{				//如果循环缓冲区还有数据，先处理完缓冲区的数据在接收，否则有可能会溢出	
			len = Receive(bBuf, sizeof(bBuf));
			if (len>0)
				m_LoopBuf.PutToBuf(bBuf, len);
		}
		len = m_LoopBuf.RxFromBuf(bBuf, sizeof(bBuf)-10);

		if (len > 0)
		{
			int nScanLen = RcvFrame(bBuf, len);
			if (nScanLen > 0)
			{   
				m_LoopBuf.DeleteFromBuf(nScanLen); //删除已经扫描的数据

				if (fIsDefHanleFrm)
				{
					if (DefHanleFrm() == false) //帧组成功了,但是没有被默认帧处理函数所处理,要出到外面去处理
						return true;
				}
				else
					return true;
			}
			else if(len>=sizeof(bBuf)-10) //缓冲区满了，但是还没有一个完整的帧
			{
				m_LoopBuf.DeleteFromBuf(len); //删除已经扫描的数据
			}
		}

		if (dwSeconds != 0)
			Sleep(100);	//防止程序在这里陷入死循环

	} while (GetTick()-dwOldTick <= dwSeconds*1000);

	return false;
}

void CStdReader::DoAutoRead()
{
	if (!m_TRtStat.fPlcInit)
		return;

	if (RxHandleFrm(1))
		DefHanleFrm();
}

void CStdReader::CctRunStateCheck()
{
	if (GetInfo(INFO_PLC_MOD_CHANGED))
    {   
		memset((BYTE*)&m_TRtStat, 0, sizeof(m_TRtStat));
        GetRooterTermAddr(m_TRtStat.bTermAddr,m_TRtStat.bTermLen);
    }

	if (GetInfo(INFO_SYNC_MTR))	
		m_TRtStat.fSyncAddr = false;

	if (!m_TRtStat.fGetPlcInfo)
	{
		Afn12Fn02_RtPause();
		if (ReadPlcModuleInfo())
			m_TRtStat.fGetPlcInfo = true;
		Afn12Fn03_RtResume();
	}
	if (m_TRtStat.fGetPlcInfo && !m_TRtStat.fSetMainNode)
	{
		Afn12Fn02_RtPause();
		if (Afn05Fn01_SetMainNodeAddr())
			m_TRtStat.fSetMainNode = true;
		Afn12Fn03_RtResume();
	}
	if (m_TRtStat.fSetMainNode && !m_TRtStat.fSyncAddr)
	{
		Afn12Fn02_RtPause();
		SyncMeterAddr();
		Afn12Fn01_RtRestart();
		m_TRtStat.fSyncAddr = true;
		m_TRtStat.fPlcInit = true;
	}
}

void CStdReader::RunThread()
{
	int iMonitorID = ReqThreadMonitorID("CStdReader-thrd", 4*60*60);	//申请线程监控ID,

	

	DTRACE(DB_CCT, ("CStdReader::Cct thread started.\n"));

	Init();

	while(1)
	{
		LockReader();
		CctRunStateCheck();
		if (GetInitState())
		{
			DoAutoRead();
			DoOtherTask();
		}
		else
			Sleep(100);

		UnLockReader();
		Sleep(1000);    // don't lock fast. add some delay time. add by whr 20170523
		UpdThreadRunClick(iMonitorID);
	}

	ReleaseThreadMonitorID(iMonitorID);
}


TThreadRet OobStdReaderPlcThread(void* pvArg)
{
	g_CStdReader = new CStdReader;

	g_CStdReader->RunThread();

	delete g_CStdReader;

	return THREAD_RET_OK;
}

void NewCctThread()
{
	NewThread(OobStdReaderPlcThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
}


int GetSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen, DWORD dwStartSec, DWORD dwEndSec)
{
	if (g_CStdReader != NULL)
	{
		return g_CStdReader->GetSchMtrResult(piStart, pbBuf, wMaxLen, dwStartSec, dwEndSec);
	}

	return -1;
}

int GetCrossSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen)
{
	if (g_CStdReader != NULL)
	{
		return g_CStdReader->GetCrossSchMtrResult(piStart, pbBuf, wMaxLen);
	}

	return -1;
}

void SaveSearchPnToDb(BYTE* pbMtrAddr, BYTE bAddrLen, BYTE bPro, TTime tmNow, BYTE bBaud)
{
	DWORD dwOAD;
	if (g_CStdReader != NULL)
	{
		BYTE bBuf[64] = {0};
		BYTE *p = bBuf;

		//376.2 AFN=06 F4
		*p++ = 0x01;	//上报从节点的数量n
		memcpy(p, pbMtrAddr, 6);	//从节点1通信地址
		p += 6;
		*p++ = bPro;	//从节点1通信协议类型
		*p++ = 0;	//从节点1序号	
		*p++ = 0;
		*p++ = 0;	//从节点1设备类型

		dwOAD = 0xF2010200 + (bBaud&0x1f);
		g_CStdReader->SaveSchMtrResult(dwOAD, bBuf, p-bBuf, bAddrLen);
	}
}

CStdReader *g_CStdReader;

//645表的载波组帧接口
int CStdReader::DL645_9707MakeFrm(BYTE *pbMtr, BYTE bMtrLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *bpBuf)
{
	BYTE bAddr[6];
	TItemList tItem;
	int wFrmLen = 0;
	TV07Tmp TmpV07;
	BYTE bIDLen = 2;
	char szTsa[32];
	DWORD dwTmpID;

	if (bMtrLen > 6)
		bMtrLen = 6;
	memset(bAddr, 0, 6-bMtrLen);
	memcpy(&bAddr[6-bMtrLen], pbMtr, bMtrLen);
	Toad645Map* pOad645Map = GetOad645Map(dwOAD);
	if (pOad645Map == NULL)
	{
		DTRACE(DB_CCT, ("DL645_9707MakeFrm(): Nonsupport OAD=%08x, Meter:%s, MtrPro:%d.\n", 
					dwOAD, HexToStr(pbMtr, bMtrLen, szTsa), bProId));
		return -1;
	}
	if (bProId == PROTOCOLNO_DLT645_V07)
	{
		bIDLen = 4;
		DL645toDL645V07(0, pOad645Map->wID, &tItem);
		memcpy(bpBuf+DL645V07_DATA, (BYTE*)&tItem.dwProId, sizeof(DWORD));
		dwTmpID = tItem.dwProId;
		wFrmLen = DL645V07MakeFrm(&TmpV07, bpBuf, bAddr, DL645V07_CMD_ASK_DATA, bIDLen);	
	}
	else
	{
		memcpy(bpBuf+DL645V07_DATA, (BYTE*)&pOad645Map->wID, sizeof(WORD));
		dwTmpID = pOad645Map->wID;
		wFrmLen = DL645MakeFrm(bpBuf, bAddr, 0x01, 2);
	}

	DTRACE(DB_CCT, ("DL645_9707MakeFrm(): Meter:%s  MtrPro;%d, OAD:0x%08x, 645_ID:0x%08x.\n",
				HexToStr(pbMtr, bMtrLen, szTsa), bProId, dwOAD, dwTmpID));
	return wFrmLen;
}

//四表的载波组帧接口
int CStdReader::DL645_EXTMakeFrm(BYTE *pbMtr, BYTE bMtrLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *bpBuf)
{
	BYTE bAddr[7];
	int wFrmLen = 0;
	BYTE bData[200];
	WORD wTmpID;
	BYTE eLen;
	char szTsa[32];

	if (bMtrLen > 7)
		bMtrLen = 7;
	memset(bAddr, 0, 7-bMtrLen);
	memcpy(&bAddr[7-bMtrLen], pbMtr, bMtrLen);
	Toad645Map* pOad645Map = GetOad645ExtMap(dwOAD);

	if (pOad645Map == NULL)
	{
		DTRACE(DB_CCT, ("Cct DL645_EXTMakeFrm err\n"));
		return -1;
	}
	switch (bSubProId)
	{
		case PROTOCOLNO_HUAXU_T188_MBUS:
		case PROTOCOLNO_HUAXU_T188_RS485:
		case PROTOCOLNO_DENENG_T188:
		case PROTOCOLNO_BEILIN_T188:
			eLen = Make188AskItemFrm(bSubProId, bAddr, pOad645Map->wID, 0, bData);
			wFrmLen = MakeExt645Frm(bAddr, DL645EXT_CMD_T188, bData, eLen, bpBuf);//先不管采集器地址 
			break;

		case PROTOCOLNO_JSJD_MBUS:
			eLen = MakeJsJdAskItemFrm(bSubProId, bAddr, pOad645Map->wID, 0, bData);
			wFrmLen = MakeExt645Frm(bAddr, DL645EXT_CMD_OTHER, bData, eLen, bpBuf);
			break;

		case PROTOCOLNO_JSLX_RS485:
			eLen = MakeJsLxAskItemFrm(bSubProId, bAddr, pOad645Map->wID, 0, bData);
			wFrmLen = MakeExt645Frm(bAddr, DL645EXT_CMD_OTHER, bData, eLen, bpBuf);
			break;

		default:
			wTmpID = ((pOad645Map->wID>>8) | (pOad645Map->wID<<8)); 
			eLen = Make188AskItemFrm(bSubProId, bAddr, wTmpID, 0, bData);
			wFrmLen = MakeExt645Frm(bAddr, DL645EXT_CMD_T188, bData, eLen, bpBuf);
			break;
	}
	DTRACE(DB_CCT, ("CctRead SBJC Mtr:%s OAD:0x%x  ID:0x%x.   bSubProId:0x%x\n",HexToStr(pbMtr, bMtrLen, szTsa),dwOAD, pOad645Map->wID,bSubProId));

	return wFrmLen;

}

//注意，对返回帧的地址和ID都还没有做进一步的正确性比较的
int CStdReader::GetDL645_9707DataVal(BYTE *psData, BYTE bsLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *pbData, TRdItem *pRdItem, bool fAnaly645data)
{
	//BYTE bAfnPos = FRM_AFN_A;
	//Toad645Map* pOad645Map = GetOad645Map(dwOAD);
	int iLen = 0;
	int i=0, iDataLen, iIdLen,j;
	BYTE bCtrl;
	BYTE buf[250];

	memset(buf, 0, sizeof(buf));
	for ( i=0;i<bsLen;i++)  //返回的645报文可能有前导符
	{
		if (psData[i]==0x68 && psData[i+7]==0x68)
			break;				
	}

	if(i+1 == bsLen)
		return -1;

	bsLen -=i;
	bCtrl = psData[i+8];//取得控制码
	if (bCtrl == 0x91 ||bCtrl == 0x81 ||bCtrl == 0x0b ||bCtrl == 0xb1)
	{
		int iDataLen = psData[i+9];
		for (j=0; j<iDataLen; j++)
		{
			psData[i+10+j] -= 0x33;
		}
		DWORD dwMtrID = 0;
		if (bCtrl == 0x81)
		{
			dwMtrID = ByteToWord(&psData[i+10]);
			if (iDataLen <= 2)
			{
				goto MTRRET_ERR;
			}
			if (dwMtrID == 0xc010)
			{
				//预留时分秒的空间,去掉周
				buf[3] = BcdToByte(psData[i+13]);
				buf[4] = BcdToByte(psData[i+14]);
				buf[6] = (BcdToByte(psData[i+15])+2000)>>8;
				buf[5] = (BcdToByte(psData[i+15])+2000)&0x00ff;
			}
			else
				memcpy(buf, &psData[i+12], iDataLen-2);
			if (dwOAD == 0x40000209)
			{
				pbData[0] = BcdToByte(buf[2]);
				pbData[1] = BcdToByte(buf[1]);
				pbData[2] = BcdToByte(buf[0]);
				return iDataLen-4;
			}
			else if (dwOAD == 0x20210200)	//日冻结时标
			{
				BYTE bDataTime[8];
				WORD wYear;

				memcpy(bDataTime, buf, 5);
				buf[0] = 0x00;
				buf[1] = 0x00;
				buf[2] = 0x00;
				buf[3] = BcdToByte(bDataTime[2]);
				buf[4] = BcdToByte(bDataTime[3]);
				wYear = 2000 + BcdToByte(bDataTime[4]);
				buf[5] = wYear%256;
				buf[6] = wYear/256;
			}
		}
		else if (bCtrl== 0x91 || bCtrl==0xb1) 
		{
			dwMtrID = ByteToDWord(&psData[i+10]);
			if (iDataLen <= 4)
			{
				goto MTRRET_ERR;
			}
			//memcpy(buf, &psData[i+14], iDataLen-4);
			if (dwMtrID == 0x4000101)
			{
				//预留时分秒的空间，去掉周
				buf[3] = BcdToByte(psData[i+15]);
				buf[4] = BcdToByte(psData[i+16]);
				buf[6] = (BcdToByte(psData[i+17])+2000)>>8;
				buf[5] = (BcdToByte(psData[i+17])+2000)&0x00ff;
			}
			else
				memcpy(buf, &psData[i+14], iDataLen-4);
			if (dwOAD == 0x40000209)
			{
				pbData[0] = BcdToByte(buf[2]);
				pbData[1] = BcdToByte(buf[1]);
				pbData[2] = BcdToByte(buf[0]);
				return iDataLen-4;
			}
			else if (dwOAD == 0x20210200)	//日冻结时标
			{
				BYTE bDataTime[8];
				WORD wYear;

				memcpy(bDataTime, buf, 5);
				buf[0] = 0x00;
				buf[1] = 0x00;
				buf[2] = 0x00;
				buf[3] = BcdToByte(bDataTime[2]);
				buf[4] = BcdToByte(bDataTime[3]);
				wYear = 2000 + BcdToByte(bDataTime[4]);
				buf[5] = wYear%256;
				buf[6] = wYear/256;
			}
		}
		//const ToaMap* pOI = GetOIMap(dwOAD);
		Toad645Map* pOad645Map = GetOad645Map(dwOAD);
		if (pOad645Map == NULL)
		{
			return -1;
		}

		//iDataLen = OIFmtDataExt(buf, iDataLen-4, pbData, pOad645Map->pFmt, pOad645Map->wFmtLen, dwOAD);
		iDataLen = OIFmtDataExt(buf, pOad645Map->w645Len, pbData, pOad645Map->pFmt, pOad645Map->wFmtLen, dwOAD);
		if (pOad645Map->dwOAD == 0x20210200)
		{
			TTime tNowTime;

			GetCurTime(&tNowTime);
			if (tNowTime.nYear!=OoOiToWord(&pbData[1]) || 
					tNowTime.nMonth!=pbData[3] || 
					tNowTime.nDay!=pbData[4])
			{
				DTRACE(DB_CCT, ("GetDL645_9707DataVal: Day frozen Time mismatch, Meter time=%04d-%02d-%02d %02d-%02d-%02d.\n",
							OoOiToWord(&pbData[1]), pbData[3], pbData[4], pbData[5], pbData[6], pbData[7]));
				return -1;
			}
		}
		return iDataLen;
	}
	else if (bCtrl== 0x94)//对电表校时回复为0x94
	{
		return 0;
	}
	else
	{
		goto MTRRET_ERR;
	}


MTRRET_ERR:
	//到这里表示已经抄读失败，填充无效数据
	DTRACE(DB_CCT, ("Cct DL645_9707 return err\n"));
	return -1;

	//return 0;
}

//注意，对返回帧的地址和ID都还没有做进一步的正确性比较的
int CStdReader::GetDL645_EXTDataVal(BYTE *psData, BYTE bsLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *pbData)
{
	//BYTE bAfnPos = FRM_AFN_A;
	//Toad645Map* pOad645Map = GetOad645ExtMap(dwOAD);
	int iLen = 0;
	int i=0, iDataLen, iIdLen,j;
	BYTE bCtrl;
	BYTE buf[250];

	memset(buf, 0, sizeof(buf));
	for ( i=0;i<bsLen;i++)  //返回的645报文可能有前导符
	{
		if (psData[i]==0x68 && psData[i+7]==0x68)
			break;				
	}

	if(i+1 == bsLen)
		return -1;

	bsLen -=i;
	bCtrl = psData[i+8];//取得645帧的控制码
	if (bCtrl == 0x9f)
	{
		bCtrl = psData[i+10];////取得扩展控制码
		if (bCtrl == 0xc8)
			goto MTRRET_ERR;
		bCtrl = psData[i+20];////取得T188帧控制码
		if (bCtrl != 0x81)
			goto MTRRET_ERR;

		int iDataLen = psData[i+21];
		DWORD dwMtrID = 0;
		dwMtrID = ByteToWord(&psData[i+22]);
		if (iDataLen <= 2)
		{
			goto MTRRET_ERR;
		}
		memcpy(buf, &psData[i+24+1], iDataLen-2-1);//ID后面有一个字节的序列号SER
		const ToaMap* pOI = GetOIMap(dwOAD);
		if (pOI == NULL)
			return -1;

		iDataLen = OIFmtDataExt(buf, iDataLen-4, pbData, pOI->pFmt, pOI->wFmtLen, dwOAD);
		return iDataLen;
	}
	else
	{
		goto MTRRET_ERR;
	}


MTRRET_ERR:
	//到这里表示已经抄读失败，填充无效数据
	DTRACE(DB_CCT, ("Cct DL645_EXT return err\n"));
	return -1;

	//return 0;
}

//单地址广播校时处理
int CStdReader::OneAddrBroadcast(BYTE *pbTsa, BYTE *pbInBuf, WORD wInLen, TMtrPara tTMtrPara, TRdItem *pRdItem, BYTE *pbData, BYTE bProId)
{
	BYTE bBuf1[100];
	BYTE bApdu[256];
	int iApduLen;
	int iStep = -1;
	TTime tmMtr,tmTm;
	int iRet;
	DWORD dwSecM;
	DWORD dwSecT;
	int iLen69845;
	int iRespLen;;
	BYTE bFrm69845[100];
	BYTE bTsaLen;

	//BYTE bBuf3[50];
	memset(bBuf1, 0, sizeof(bBuf1));
	if (OoProReadAttr(0x4204, 3, 0, bBuf1, sizeof(bBuf1), &iStep) > 0)
	{
		//if (bBuf1[9] == 1)//启用
		{
			//memset(bBuf2, 0, sizeof(bBuf2));
			GetCurTime(&tmTm);
			{
				tmMtr.nYear = OoLongUnsignedToWord(&pbData[1]);
				tmMtr.nMonth = pbData[3];
				tmMtr.nDay= pbData[4];
				tmMtr.nHour= pbData[5];
				tmMtr.nMinute= pbData[6];
				tmMtr.nSecond= pbData[7]+2;//暂时先固定加1秒的通信延时
				if (IsInvalidTime(tmMtr))
					return -1;
			}

			//if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[5]*3600+bBuf1[6]*60+bBuf1[7]))
			//	return -1;

			dwSecM = TimeToSeconds(tmMtr);
			dwSecT = TimeToSeconds(tmTm);
			//if ((dwSecM > (dwSecT+300)) || (dwSecT > (dwSecM+300)))//时间差大于5分钟的不理会
			//	return -1;

			if (dwSecM > (dwSecT+bBuf1[3]) ||dwSecT > (dwSecM+bBuf1[3]))//电表时间超差
			{
				bTsaLen = pbTsa[0];
				g_MtrClkPrg.bEvtSrcTSA[0] = DT_TSA;
				g_MtrClkPrg.bEvtSrcTSA[1] = bTsaLen+1;
				g_MtrClkPrg.bEvtSrcTSA[2] = bTsaLen-1;
				memcpy(&g_MtrClkPrg.bEvtSrcTSA[3], &pbTsa[1], bTsaLen);
				memcpy(&g_MtrClkPrg.bClock[0], pbData, 8);
				g_MtrClkPrg.bClkErr[0] = DT_INT;
				if (bProId == PROTOCOLNO_DLT645_V07 || bProId == PROTOCOLNO_DLT645)// 07表校时是分两次操作的
				{
					BYTE bBufY[18]={0x68, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x08, 0x06, 0x01, 0x02, 0x03, 0x07, 0x03, 0x17, 0x5E, 0x16};

					//memcpy(&bBufY[1], &pbInBuf[5], 6);
					memrcpy(&bBufY[1], &pbTsa[1], bTsaLen);
					bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
					bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
					bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
					bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
					bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
					bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
					bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);
					memcpy(&pbInBuf[4], bBufY, sizeof(bBufY));
					pbInBuf[3] = sizeof(bBufY);
					DTRACE(DB_CCT, ("cct OneAddrBroadcast 07-645 \r\n")); 
					iRet = Afn13Fn01_Broadcast(pbTsa+1, bTsaLen, pbInBuf, sizeof(bBufY)+4, pRdItem, pbData, bProId, 0);
					memset(bBuf1, 0, sizeof(bBuf1));
					iRet = ReadDL645_9707Time(pbTsa, tTMtrPara, pRdItem, bBuf1, bProId);
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

					iStep = Make698_45Frm(pbTsa+1, bTsaLen, 0x43, 0, 0, SER_ADDR_TYPE_SIG, bBuf1, 16, pbInBuf+4);
					pbInBuf[3] = iStep;
					DTRACE(DB_CCT, ("OneAddrBroadcast 69845 write id=0x40000200.\r\n")); 
					Afn13Fn01_Broadcast(pbTsa+1, bTsaLen, pbInBuf, iStep+4, pRdItem, pbData, bProId, 0);
					//校时后再读表时间回来
					iApduLen = GetRequestNormal(pRdItem->dwOAD, bApdu);
					//DTRACE(DB_CCT, ("Send RequestNormal Mtr:%s OAD:0x%s.\n",HexToStr(bDbTsa+1, bDbTsa[0], szTsa), HexToStr((BYTE*)&tRdItem.dwOAD, 4, szOAD, true)));
					bFrm69845[0] = PRO_TYPE_TRANS;
					bFrm69845[1] = 0x00;	//通信延时相关标识
					bFrm69845[2] = 0x00;	//附属相关延时标识

					bApdu[iApduLen++] = 0x00;	//时间标签
					iLen69845 = Make698_45Frm(pbTsa+1, pbTsa[0], 0x43, 0, 0, SER_ADDR_TYPE_SIG, bApdu, iApduLen, &bFrm69845[4]);
					bFrm69845[3] = iLen69845;
					memset(bBuf1, 0, sizeof(bBuf1));
					iRet = Afn13Fn01_RtFwd(pbTsa+1, pbTsa[0], bFrm69845, iLen69845+4, NULL, pRdItem, bBuf1, tTMtrPara.bProId);
					if (iRet == 2)//即读不回时间
						iRet = -1;

				}
				if (iRet > 0)
				{
					tmMtr.nYear = OoLongUnsignedToWord(&bBuf1[1]);
					tmMtr.nMonth = bBuf1[3];
					tmMtr.nDay= bBuf1[4];
					tmMtr.nHour= bBuf1[5];
					tmMtr.nMinute= bBuf1[6];
					tmMtr.nSecond= bBuf1[7]+1;//暂时先固定加1秒的通信延时

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
					g_MtrClkPrg.bClkErr[1] = 2;//如果读不回时间就给个1～2秒左右的误差值

				SetInfo(INFO_TERM_MTRCLKPRG);
				DTRACE(DB_CCT, ("send INFO_TERM_MTRCLKPRG \r\n")); 
				return 1;
			}
		}
	}
	return -1;
}

extern TTime tLastTime[31];
//单地址广播校时处理总接口
int CStdReader::MtrBroadcast()
{
	TTime tNowTime, tStartCheckMtrTime;
	TMtrPara tMtrPara;
	TRdItem tRdItem;
	int iPn = 0;
	int iRet;
	DWORD dwCurSec, dwStartCheckMtrSec;
	BYTE bTsa[TSA_LEN+1];
	BYTE bCheckTimePara[16] = {0};
	BYTE bPerExeMtrCnt;

	if (GetInfo(INFO_ONE_BRAODCAST_ARG_CCT))
	{
		DTRACE(DB_CCT, ("INFO_ONE_BRAODCAST_ARG_CHG_CCT \r\n")); 
		memset((BYTE *)&tLastTime[30], 0, sizeof(TTime));
	}

	memset(bCheckTimePara, 0, sizeof(bCheckTimePara));
	if (ReadItemEx(BANK0, PN0, 0x4205, bCheckTimePara) > 0)
	{
		if (bCheckTimePara[9] == 1)	//启用
		{
			dwCurSec = GetCurTime();
			GetCurTime(&tNowTime);
			GetCurTime(&tStartCheckMtrTime);
			tStartCheckMtrTime.nHour = bCheckTimePara[5];
			tStartCheckMtrTime.nMinute = bCheckTimePara[6];
			tStartCheckMtrTime.nSecond = bCheckTimePara[7];
			dwStartCheckMtrSec = TimeToSeconds(tStartCheckMtrTime);
			if (dwCurSec > dwStartCheckMtrSec)
			{
				if (IsSameDay(tNowTime, tLastTime[30]))	//一天只执行一次
					return -1;

				DTRACE(DB_CCT, ("Start PLC broaddcast...\n")); 
				memset((BYTE*)&tRdItem, 0, sizeof(tRdItem));
				tRdItem.bReqType = 1;

				bPerExeMtrCnt = 1;
				while (bPerExeMtrCnt++ < 10)
				{
					BYTE bTxBuf[256] = {0};
					BYTE bRxBuf[256] = {0};
					BYTE bApdu[64] = {0};
					BYTE *pbTx = bTxBuf;
					BYTE *pApdu = bApdu;

					iPn = SearchNextPnFromMask((BYTE*)GetPlcPnMask(), iPn);
					if (iPn < 0)
						break;

					memset(bTsa, 0, sizeof(bTsa));
					bTsa[0] = GetMeterTsa(iPn, bTsa+1);
					GetMeterPara(iPn, &tMtrPara);
					tRdItem.dwOAD = 0x40000200;

					if (tMtrPara.bProId == PROTOCOLNO_DLT645)	//645-97
						*pbTx++ = PROTOCOLNO_DLT645;
					else if (tMtrPara.bProId == PROTOCOLNO_DLT645_V07)
						*pbTx++ = PROTOCOLNO_DLT645_V07;
					else 
						*pbTx++ = PRO_TYPE_TRANS;
					*pbTx++ = 0x00;	//通信延时相关标识
					*pbTx++ = 0x00;	//附属相关延时标识

					RouterPause();
					if (tMtrPara.bProId == PROTOCOLNO_DLT645 || tMtrPara.bProId == PROTOCOLNO_DLT645_V07)
					{
						iRet = ReadDL645_9707Time(bTsa, tMtrPara, &tRdItem, bRxBuf, tMtrPara.bProId);
						if (iRet > 0)
							OneAddrBroadcast(bTsa, bTxBuf, pbTx-bTxBuf, tMtrPara, &tRdItem, bRxBuf, tMtrPara.bProId);
					}
					else	//PROTOCOLNO_DLT69845
					{
						pApdu += GetRequestNormal(tRdItem.dwOAD, pApdu);
						*pApdu++ = 0x00;	//时间标签
						iRet = Make698_45Frm(bTsa+1, bTsa[0], 0x43, 0, 0, SER_ADDR_TYPE_SIG, bApdu, pApdu-bApdu, pbTx+1);
						*pbTx++ = iRet;
						pbTx += iRet;
						memset(bRxBuf, 0, sizeof(bRxBuf));
						iRet = Afn13Fn01_RtFwd(bTsa+1, bTsa[0], bTxBuf, pbTx-bTxBuf, NULL, &tRdItem, bRxBuf, tMtrPara.bProId);
						if (iRet > 0)
							OneAddrBroadcast(bTsa, bTxBuf, pbTx-bTxBuf, tMtrPara, &tRdItem, bRxBuf, tMtrPara.bProId);
					}
				}

				if (iPn < 0)
					tLastTime[30] = tNowTime;
			}
		}
	}
}

//广播校时
int CStdReader::BroadcastAdjustTime()
{
	TRdItem tRdItem;
	int m_iPn = 0;
	int iRet;
	int iApduLen;
	int iLen69845;
	int iRespLen;;
	BYTE bFrm69845[512];
	BYTE bRespData[512];
	BYTE bDbTsa[TSA_LEN] = {0};	//系统库中实际的电表地址信息，bDbTsa[0]表地址长度，bDbTsa[1...TSA_LEN-1]为具体地址
	BYTE bCn = 0;
	BYTE bCheckCnt = 0;
	WORD wPlcNum;
	char szTsa[32];
	TTime tmTm;
	BYTE bBuf1[100];
	int iStep = -1;

	if (GetInfo(INFO_MTR_BRAODCAST_ARG_CCT))
	{
		DTRACE(DB_CCT, ("INFO_MTR_BRAODCAST_ARG_CCT \r\n")); 
		memset((BYTE *)&tLastTime[15], 0, sizeof(TTime));
	}
	m_iPn = SearchNextPnFromMask((BYTE*)GetPlcPnMask(), m_iPn);
	if (m_iPn < 0)
		return false;
	if(m_bCctExeState==CCT_SCH_MTR)//搜表状态不进行广播
		return -1;

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

			if (tmTm.nYear == tLastTime[15].nYear && tmTm.nMonth== tLastTime[15].nMonth && tmTm.nDay== tLastTime[15].nDay)//一天只执行一次
				return -1;

			Afn12Fn02_RtPause();
			memcpy( (BYTE *)&tLastTime[15].nYear,(BYTE *)&tmTm, sizeof(TTime));

			bFrm69845[0] = 0x00;//00H＝透明传输；01H＝DL/T645—1997；02H＝DL/T645—2007；03H＝相位识别功能；04H～FFH保留。

			//if (bProId == PROTOCOLNO_DLT645_V07 || bProId == PROTOCOLNO_DLT645)// 07表校时是分两次操作的
			{

				bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
				bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
				bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
				bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
				bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
				bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
				bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);
				memcpy(&bFrm69845[2], bBufY, sizeof(bBufY));
				bFrm69845[1] = sizeof(bBufY);
				DTRACE(DB_CCT, ("BroadcastAdjustTime: Start...\n")); 
				Afn05Fn3_StartBoardCast(bFrm69845, sizeof(bBufY)+2);
				//直接在这里延时等待，完了再发对象帧的,这样在等待期间就响应不了别的命令了
				//这得讨论一下怎么做
				if (m_dwLastWaitSec > 180)
					m_dwLastWaitSec = 180;
				Sleep(m_dwLastWaitSec*1000);

				//载波的发了广播帧后好象是要等待一几分钟吧，这个再确认
			}
			//if (bProId == PROTOCOLNO_DLT69845)
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

				bBufY[0] = 0xAA;
				iStep = Make698_45Frm(bBufY, 1, 0x43, 3, 0, SER_ADDR_TYPE_SIG, bBuf1, 16, bFrm69845+2);//对象表的地址及长度待定!!!
				bFrm69845[1] = iStep;
				DTRACE(DB_CCT, ("OneAddrBroadcast 69845 write id=0x40000200.\r\n")); 
				memset(bRespData, 0, sizeof(bRespData));
				Afn05Fn3_StartBoardCast(bFrm69845, iStep+2);
				//直接在这里延时等待，,这样在等待期间就响应不了别的命令了
				//这得讨论一下怎么做
				if (m_dwLastWaitSec > 180)
					m_dwLastWaitSec = 180;
				Sleep(m_dwLastWaitSec*1000);
			}
			Afn12Fn03_RtResume();
		}
	}
	return 0;
}

//描述：路由数据转发类 AFN=14H-F1
//参数：@pbTsa 表地址
//		@bTsaLen 表地址长度
//		@pbInbuf 为F1的数据类容
//		@wInLen 为F1的数据长度
//		@pbOutBuf 为F1的抄读内容
//		@bProId 为协议类型
//返回：返回数据长度
int CStdReader::Afn13Fn01_Broadcast(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbInBuf, WORD wInLen, TRdItem *pRdItem, BYTE *pbOutBuf, BYTE bProId, BYTE bWaitTm)
{
	BYTE bCtrl;
	//BYTE bRevTsa[TSA_LEN];
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;
	BYTE *pbTxBuf = bTxBuf;
	DWORD dwLastSendClick;

	//revcpy(bRevTsa, pbTsa, bTsaLen);
	//if (memcmp(pbTsa, m_TRunStateInfo.bRdFailTsa, 6) != 0)
	{
		bCtrl = 0x41;
		bR[0] = (1<<2); //D2通信模块标识：0表示对集中器的通信模块操作;1表示对载波表的通信模块操作。
		bR[1] = 0;		//信道
		bR[2] = 0xff;
		bR[3] = 0x0;
		bR[4] = 0;
		bR[5] = 0;
		wTxLen = Make1376_2Frm(pbTsa, bTsaLen, bCtrl, bR, AFN_RTFWD, FN(1), pbInBuf, wInLen, bTxBuf);

		for (BYTE bTryCnt=0; bTryCnt<m_RtRunMdInfo.bTrySendCnt; bTryCnt++)
		{
			dwLastSendClick = GetClick();
			if (Send(bTxBuf, wTxLen) == wTxLen)
			{
GOTO_RxHandleFrm:
				//if (RxHandleFrm(m_RtRunMdInfo.bNodeTmOut))
				if (RxHandleFrm(1))
				{
					if (m_TRcv13762.bAfn==AFN_RTFWD && DtToFn(m_TRcv13762.bDt)==FN(1))
					{
						int iRet;
						WORD wRcvFrmLen;
						BYTE *pData = m_TRcv13762.bDtBuf;
						//BYTE bPreCnt;

						pData += 3;	//2字节当前报文本地通信上行时长 + 1字节通信协议类型
						wRcvFrmLen = *pData++;
						if (wRcvFrmLen > 0)
						{
							if (bProId == PROTOCOLNO_DLT69845)
							{
								TFrm69845 tRcvFmt;
								memset((BYTE*)&tRcvFmt, 0, sizeof(tRcvFmt));
								if (DecodeFram69845(pData, wRcvFrmLen, &tRcvFmt))
								{
									TRdItem tRptItem;
									memset((BYTE*)&tRptItem, 0, sizeof(tRptItem));
									DecodeReportApdu(tRcvFmt.bAPDUData, tRcvFmt.wAPDULen, &tRptItem, pbOutBuf);
									if (tRcvFmt.bSALen==bTsaLen && 
											memcmp(tRcvFmt.bSA, pbTsa, 6)==0)	
									{
										if (pRdItem->bReqType == GET_REQUEST_NORMAL)
											iRet = GetResponseNormal(pRdItem->dwOAD, tRcvFmt.bAPDUData, tRcvFmt.wAPDULen, pbOutBuf);
										else
											iRet = GetResponseRecord(pRdItem->dwOAD, tRcvFmt.bAPDUData, tRcvFmt.wAPDULen, pRdItem->bRCSD, pRdItem->wRcsdLen, pbOutBuf);

										if (iRet <= 0)
											goto RET_RTFWD;
										return iRet;
									}
								}
							}
							else if (bProId == PROTOCOLNO_DLT645 || bProId == PROTOCOLNO_DLT645_V07)
							{
								iRet = GetDL645_9707DataVal(pData, wRcvFrmLen, bProId, 0, pRdItem->dwOAD, pbOutBuf, pRdItem);
								if (iRet < 0)
									goto RET_RTFWD;
								return iRet;
							}
							else if (bProId == PROTOCOLNO_SBJC)
							{
								iRet = GetDL645_EXTDataVal(pData, wRcvFrmLen, bProId, 0, pRdItem->dwOAD, pbOutBuf);
								if (iRet <= 0)
									goto RET_RTFWD;
								return iRet;
							}
						}
						else if (wRcvFrmLen == 0)
							goto RET_RTFWD;
					}
				}

				if (GetClick()-dwLastSendClick < m_RtRunMdInfo.bNodeTmOut || bWaitTm==0)
					goto RET_RTFWD;
			}
		}
	}
RET_RTFWD:
	//到这里表示已经抄读失败，填充无效数据
	pbOutBuf[0] = DAR;
	pbOutBuf[1] = DAR_REQ_TIMEOUT;
	if (bProId == PROTOCOLNO_DLT645_V07)
		return -1;
	else
		return 2;
}

int CStdReader::ReadDL645_9707Time(BYTE * pDbTsa, TMtrPara tTMtrPara, TRdItem *pRdItem, BYTE *pbData, BYTE bProId)
{
	int iLen69845;
	int iRespLen;;
	BYTE bFrm69845[100];
	BYTE bRespData[100];
	int iRet;

	//memset((BYTE*)tRdItem, 0, sizeof(tRdItem));
	bFrm69845[0] = PRO_TYPE_TRANS;
	bFrm69845[1] = 0x00;	//通信延时相关标识
	bFrm69845[2] = 0x00;	//附属相关延时标识
	pRdItem->dwOAD = 0x40000200;
	iLen69845 = DL645_9707MakeFrm(&tTMtrPara.bAddr[1], tTMtrPara.bAddr[0], tTMtrPara.bProId, tTMtrPara.bSubProId, pRdItem->dwOAD, &bFrm69845[4]);//注意，浙江的才这么改！tTMtrPara.bAddr[0]
	if (iLen69845 <= 0)
	{
		return -1;
	}
	bFrm69845[3] = iLen69845;

	memset(bRespData, 0, sizeof(bRespData));
	iRet = Afn13Fn01_RtFwd(pDbTsa+1, pDbTsa[0], bFrm69845, iLen69845+4, NULL, pRdItem, bRespData, tTMtrPara.bProId);
	if (iRet > 0)
		memcpy(pbData, bRespData, 8);
	else
		return -1;

	pRdItem->dwOAD = 0x40000209;
	iLen69845 = DL645_9707MakeFrm(&tTMtrPara.bAddr[1], tTMtrPara.bAddr[0], tTMtrPara.bProId, tTMtrPara.bSubProId, pRdItem->dwOAD, &bFrm69845[4]);//注意，浙江的才这么改！tTMtrPara.bAddr[0]
	if (iLen69845 <= 0)
	{
		return -1;
	}

	bFrm69845[3] = iLen69845;
	memset(bRespData, 0, sizeof(bRespData));
	iRet = Afn13Fn01_RtFwd(pDbTsa+1, pDbTsa[0], bFrm69845, iLen69845+4, NULL, pRdItem, bRespData, tTMtrPara.bProId);
	if (iRet > 0)
	{
		memcpy(pbData+5, bRespData, 3);
		return 8;
	}
	return -1;
}

static BYTE g_bEngTatolFmt[1] = {DT_DB_LONG_U};
static BYTE g_bPowerTatolFmt[1] = {DT_DB_LONG};

//描述：读取2007版645支持的事件数据
int CStdReader::DL645V07AskItemErc(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId, DWORD dwOAD, BYTE* pbRxBuf)
{
	int iRet;
	TTime tmHap, tmEnd;
	WORD wOI;
	BYTE bRcsdNum, bChnNum;
	DWORD dwRelaOAD;
	BYTE i, *pbTmp = pbRxBuf;
	DWORD dwVal=0, dwCurVal=0;
	BYTE *pbRCSD = pRdItem->bRCSD;
	BYTE mBuf[MTR_FRM_SIZE], bTmpBuf[60];


	TErcRdCtrl* pErcRdCtrl = GetOad07645ErcMap(dwOAD);

	if (pErcRdCtrl == NULL) //不支持的数据项
		return -2;

	dwVal = ByteToDWORD(pbRxBuf, 4);

	memset(mBuf, 0, sizeof(mBuf));
	iRet = DL645V07ProIdTxRx(pRdItem, pbAddr, bAddrLen, bProId, pErcRdCtrl->dwErcNumID, mBuf, sizeof(mBuf));
	if (iRet > 0)
		dwCurVal = BcdToDWORD(mBuf, iRet);
	DTRACE(DB_CCT, ("DL645V07AskItemErc: dwOAD=0x%08x, dwLastCnt=%d, dwCurCnt=%d.\n", dwOAD, dwVal, dwCurVal));
	if (dwVal == dwCurVal)
		return -1;

	bRcsdNum = *pbRCSD++;
	pbRCSD += 5;
	pbTmp += 2; //预留存实际长度
	*pbTmp++ = DT_DB_LONG_U;
	pbTmp += OoDWordToDoubleLongUnsigned(dwCurVal, pbTmp);

	memset(&tmHap, 0, sizeof(tmHap));
	memset(&tmEnd, 0, sizeof(tmEnd));
	for (i=0; i<pErcRdCtrl->bNum; i++)
	{
		memset(mBuf, 0, sizeof(mBuf));
		iRet = DL645V07ProIdTxRx(pRdItem, pbAddr, bAddrLen, bProId, pErcRdCtrl->dwRdID[i], mBuf, sizeof(mBuf));
		if (iRet > 0)
		{
			if (pErcRdCtrl->dwRdID[i] == 0x03050001)
			{
				Fmt1ToTime(mBuf, tmHap);
				Fmt1ToTime(mBuf+9, tmEnd);
			}
			else if (pErcRdCtrl->dwRdID[i] == 0x03300401)
			{
				Fmt1ToTime(mBuf+4, tmHap);
				Fmt1ToTime(mBuf+10, tmEnd);
			}
			else if (IsOneIDHapEndTime(pErcRdCtrl->dwRdID[i]))
			{
				Fmt1ToTime(mBuf, tmHap);
				Fmt1ToTime(mBuf+6, tmEnd);
			}
			else if (IsOnlyHapTime(pErcRdCtrl->dwRdID[i]))
			{
				Fmt1ToTime(mBuf, tmHap);
				memset(&tmEnd, 0, sizeof(tmEnd));
			}
			else
			{
				if (i == 0)
					Fmt1ToTime(mBuf, tmHap);
				else
					Fmt1ToTime(mBuf, tmEnd);
			}
		}
		else
		{
			if (pErcRdCtrl->dwRdID[i]==0x03050001 || pErcRdCtrl->dwRdID[i]==0x03300401 || IsOneIDHapEndTime(pErcRdCtrl->dwRdID[i]) || IsOnlyHapTime(pErcRdCtrl->dwRdID[i]))
			{
				memset(&tmHap, 0, sizeof(tmHap));
				memset(&tmEnd, 0, sizeof(tmEnd));
			}
			else
			{
				if (i == 0)
					memset(&tmHap, 0, sizeof(tmHap));
				else
					memset(&tmEnd, 0, sizeof(tmEnd));
			}
		}
	}

	//wOI = dwOAD>>16;
	//if (wOI>0x3008 && wOI!=0x300B) //事件发生源
	//{
	//	*pbTmp++ = DT_TSA;
	//	*pbTmp++ = 7;
	//	*pbTmp++ = 5;
	//	memcpy(pbTmp, &pMtrPro->pMtrPara->bAddr[1], 6);
	//	pbTmp += 6;
	//}
	//*pbTmp++ = DT_ARRAY;
	//*pbTmp++ = 3;
	//memset(bTmpBuf, 0, sizeof(bTmpBuf));
	//iRet = OoReadAttr(0x4300, ATTR10, bTmpBuf, NULL, NULL);	//读取配置参数
	//if (iRet<=0 || bTmpBuf[0]!=DT_ARRAY)
	//{
	//	for (i=0; i<3; i++)
	//	{
	//		*pbTmp++ = DT_STRUCT;
	//		*pbTmp++ = 2;
	//		*pbTmp++ = DT_OAD;
	//		if (i == 0)
	//			pbTmp += OoDWordToOad(0x45000000, pbTmp);
	//		else
	//			pbTmp += OoDWordToOad(0x45100000, pbTmp);
	//		*pbTmp++ = DT_UNSIGN;
	//		*pbTmp++ = 0;
	//	}
	//}
	//else
	//{
	//	bChnNum = (bTmpBuf[1]>CN_RPT_NUM) ? CN_RPT_NUM : bTmpBuf[1];
	//	for (i=0; i<3; i++)
	//	{
	//		*pbTmp++ = DT_STRUCT;
	//		*pbTmp++ = 2;
	//		*pbTmp++ = DT_OAD;
	//		if (i<bChnNum)
	//			memcpy(pbTmp, &bTmpBuf[5*i+3], 4);	//通道OAD
	//		else
	//			OoDWordToOad(0x45100000, pbTmp);
	//		pbTmp += 4;
	//		*pbTmp++ = DT_UNSIGN;
	//		*pbTmp++ = 0;
	//	}
	//}

	for (i=1; i<bRcsdNum; i++)
	{
		pbRCSD++;
		memrcpy((BYTE* )&dwRelaOAD, pbRCSD, 4);
		if (dwRelaOAD==0x201E0200 || dwRelaOAD==0x20200200 || dwRelaOAD==0x20220200) //跳过必抄项
		{
			if (dwRelaOAD == 0x20220200)
			{
				*pbTmp++ = DT_DB_LONG_U;
				pbTmp += OoDWordToDoubleLongUnsigned(dwCurVal, pbTmp);
			}
			else if (dwRelaOAD == 0x201E0200)
			{
				if ( !IsTimeEmpty(tmHap) )
				{
					*pbTmp++ = DT_DATE_TIME_S;
					pbTmp += OoTimeToDateTimeS(&tmHap, pbTmp);
				}
				else
					*pbTmp++ = DT_NULL;
			}
			else if (dwRelaOAD == 0x20200200)
			{
				if ( !IsTimeEmpty(tmEnd) )
				{
					*pbTmp++ = DT_DATE_TIME_S;
					pbTmp += OoTimeToDateTimeS(&tmEnd, pbTmp);
				}
				else
					*pbTmp++ = DT_NULL;
			}
			pbRCSD += 4;
			continue;
		}
		else if (IsHapEngOAD(dwRelaOAD) || IsEndEngOAD(dwRelaOAD))
			iRet = DL645V07AskItemErcHapEndEng(pRdItem, pbAddr, bAddrLen, bProId, dwRelaOAD, pErcRdCtrl, pbTmp);
		else
			iRet = DL645V07AskItemErc(pRdItem, pbAddr, bAddrLen, bProId, dwRelaOAD, pbTmp);

		pbRCSD += 4;
		if (iRet > 0)
			pbTmp += iRet;
		else
			*pbTmp++ = DT_NULL;
	}

	iRet = pbTmp - pbRxBuf - 2;
	memcpy(pbRxBuf, &iRet, 2);
	return iRet+2;
}

//描述：读取2007版645支持的事件数据
int CStdReader::DL645V07ProIdTxRx(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId, DWORD dwProId, BYTE* pbRxBuf, WORD wMaxRxLen)
{		
	BYTE bTxBuf[32] = {0};
	BYTE bRxBuf[512] = {0};
	char szTsa[TSA_LEN+1] = {0};
	int iTxLen, iRxLen;
	TV07Tmp tTmpV07; 

	memset(&tTmpV07, 0, sizeof(tTmpV07));
	memcpy(&bTxBuf[4+DL645V07_DATA], (BYTE*)&dwProId, sizeof(DWORD));
	iTxLen = DL645V07MakeFrm(&tTmpV07, &bTxBuf[4], pbAddr, DL645V07_CMD_ASK_DATA, 4);	
	bTxBuf[0] = PRO_TYPE_645_07;
	bTxBuf[1] = 0x00;	//通信延时相关标识
	bTxBuf[2] = 0x00;	//附属相关延时标识
	bTxBuf[3] = iTxLen;
	RouterPause();
	iRxLen = Afn13Fn01_RtFwd(pbAddr, bAddrLen, bTxBuf, iTxLen+4, NULL, pRdItem, bRxBuf, bProId, true);
	if (iRxLen>0 && iRxLen<=wMaxRxLen)
	{
		memcpy(pbRxBuf, bRxBuf, iRxLen);
	}
	else
	{
		iRxLen = -1;
		DTRACE(DB_CCT, ("DL645V07ProIdTxRx(): Nonsupport ID=0x%08x, Meter:%s.\n", dwProId,HexToStr(pbAddr, 6, szTsa)));
		iRxLen = 1;
		pbRxBuf[0] = 0;
	}

	return iRxLen;
}

int CStdReader::DL645V07AskItemErcHapEndEng(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId,  DWORD dwOAD, TErcRdCtrl* pErcRdCtrl, BYTE* pbDstBuf)
{
	int iRet = -1;
	DWORD dwID;
	BYTE *pbData = pbDstBuf;
	BYTE bBuf[MTR_FRM_SIZE];

	memset(bBuf, 0, sizeof(bBuf));
	if (pErcRdCtrl->dwRdID[0]==0x1d000101 || pErcRdCtrl->dwRdID[0]==0x1e000101)
	{
		dwID = 0;
		if (IsHapEngOAD(dwOAD))
		{
			switch(dwOAD)
			{
				case 0x00102201:
					dwID = pErcRdCtrl->dwRdID[0] + 0x0200;
					break;
				case 0x00202201:
					dwID = pErcRdCtrl->dwRdID[0] + 0x0300;
					break;
				case 0x00502201:
					dwID = pErcRdCtrl->dwRdID[0] + 0x0400;
					break;
				case 0x00602201:
					dwID = pErcRdCtrl->dwRdID[0] + 0x0500;
					break;
				case 0x00702201:
					dwID = pErcRdCtrl->dwRdID[0] + 0x0600;
					break;
				case 0x00802201:
					dwID = pErcRdCtrl->dwRdID[0] + 0x0700;
					break;
				default:
					dwID = 0;
					break;
			}
		}

		if (dwID > 0)
		{
			iRet = DL645V07ProIdTxRx(pRdItem, pbAddr, bAddrLen, bProId, dwID, bBuf, sizeof(bBuf));
			if (iRet > 0)
			{
				if ((iRet=OIFmtDataExt(bBuf, 4, pbData, g_bEngTatolFmt, sizeof(g_bEngTatolFmt)/sizeof(BYTE), dwOAD)) > 0)
				{
					pbData += iRet;
				}
			}
		}

		if (iRet <= 0)
			*pbData++ = DT_NULL;
	}
	else if (IsHapEndEngSameID(pErcRdCtrl->dwRdID[0]))
	{
		dwID = 0;
		WORD dwBaseOff = 0;
		if (IsHapEngOAD(dwOAD))
		{
			switch(pErcRdCtrl->dwRdID[0])
			{
				case 0x03300101:
					dwBaseOff = 10;
					break;
				case 0x03300d01:
				case 0x03300e01:
				case 0x03350001:
				case 0x03370001:
					dwBaseOff = 12;	//发生时间（6）、结束时间（6）
					break;
				case 0x03360001:
					dwBaseOff = 13;
					break;
				case 0x03301301:
					dwBaseOff = 28;
					break;
				default:
					dwBaseOff = 0;
					break;
			}

			switch(dwOAD)
			{
				case 0x00102201:
					dwID = dwBaseOff;	
					break;
				case 0x00202201:
					dwID = dwBaseOff + 4;
					break;
				case 0x00502201:
					if (pErcRdCtrl->dwRdID[0]==0x03300101 || pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01)
						dwID = dwBaseOff + 8;
					else
						dwID = 0;
					break;
				case 0x00602201:
					if (pErcRdCtrl->dwRdID[0]==0x03300101 || pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01)
						dwID = dwBaseOff + 12;
					else
						dwID = 0;
					break;
				case 0x00702201:
					if (pErcRdCtrl->dwRdID[0]==0x03300101 || pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01)
						dwID = dwBaseOff + 16;
					else
						dwID = 0;
					break;
				case 0x00802201:
					if (pErcRdCtrl->dwRdID[0]==0x03300101 || pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01)
						dwID = dwBaseOff + 20;
					else
						dwID = 0;
					break;
				default:
					dwID = 0;
					break;
			}
		}
		else if (IsEndEngOAD(dwOAD))
		{
			switch(pErcRdCtrl->dwRdID[0])
			{
				case 0x03300d01:
				case 0x03300e01:
					dwBaseOff = 36;	//发生时间（6）+ 结束时间（6） + 发生前的相应数据（24）	
					break;
				case 0x03350001:
					dwBaseOff = 20;
					break;
				case 0x03360001:
					dwBaseOff = 21;
					break;
				default:
					dwBaseOff = 0;
					break;
			}

			switch(dwOAD)
			{
				case 0x00108201:
					if (pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01 || pErcRdCtrl->dwRdID[0]==0x03350001 || pErcRdCtrl->dwRdID[0]==0x03360001)
						dwID = dwBaseOff;
					else
						dwID = 0;
					break;
				case 0x00208201:
					if (pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01 || pErcRdCtrl->dwRdID[0]==0x03350001 || pErcRdCtrl->dwRdID[0]==0x03360001)
						dwID = dwBaseOff + 4;
					else
						dwID = 0;
					break;
				case 0x00508201:
					if (pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01)
						dwID = dwBaseOff + 8;
					else
						dwID = 0;
					break;
				case 0x00608201:
					if (pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01)
						dwID = dwBaseOff + 12;
					else
						dwID = 0;
					break;
				case 0x00708201:
					if (pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01)
						dwID = dwBaseOff + 16;
					else
						dwID = 0;
					break;
				case 0x00808201:
					if (pErcRdCtrl->dwRdID[0]==0x03300d01 || pErcRdCtrl->dwRdID[0]==0x03300e01)
						dwID = dwBaseOff + 20;
					else
						dwID = 0;
					break;
				default:
					dwID = 0;
					break;
			}
		}

		if (dwID > 0)
		{
			iRet = DL645V07ProIdTxRx(pRdItem, pbAddr, bAddrLen, bProId, pErcRdCtrl->dwRdID[0], bBuf, sizeof(bBuf));
			if (iRet > 0)
			{
				if ((iRet=OIFmtDataExt(bBuf+dwID, 4, pbData, g_bEngTatolFmt, sizeof(g_bEngTatolFmt)/sizeof(BYTE), dwOAD)) > 0)
				{
					pbData += iRet;
				}
			}
		}

		if (iRet <= 0)
			*pbData++ = DT_NULL;
	}
	else
	{
		dwID = 0;
		if (IsHapEngOAD(dwOAD))
		{
			dwID = pErcRdCtrl->dwRdID[0];
		}
		else if (IsEndEngOAD(dwOAD))
		{
			dwID = pErcRdCtrl->dwRdID[1];
		}

		switch(dwOAD)
		{
			case 0x00102201:
			case 0x00108201:
				dwID += 0x0100;
				break;
			case 0x00202201:
			case 0x00208201:
				dwID += 0x0200;
				break;
			case 0x00302201:
			case 0x00308201:
				dwID += 0x0300;
				break;
			case 0x00402201:
			case 0x00408201:
				dwID += 0x0400;
				break;
			default:
				dwID = 0;
				break;
		}

		if (dwID > 0)
		{
			iRet = DL645V07ProIdTxRx(pRdItem, pbAddr, bAddrLen, bProId, dwID, bBuf, sizeof(bBuf));
			if (iRet > 0)
			{
				if ((iRet=OIFmtDataExt(bBuf, 4, pbData, g_bEngTatolFmt, sizeof(g_bEngTatolFmt)/sizeof(BYTE), dwOAD)) > 0)
				{
					pbData += iRet;
				}
			}
		}

		if (iRet <= 0)
			*pbData++ = DT_NULL;
	}

	return pbData - pbDstBuf;
}

//描述：抄读645表协议数据
int CStdReader::ReadDLT_645(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen)
{
	int iLen;
	BYTE *pbRxBuf0 = pbRxBuf;
	BYTE bTxBuf[1024];
	BYTE *pbTx = bTxBuf;
	DWORD dwRealOAD;

	if ((pRdItem->dwOAD&0xff000000) == 0x30000000)	//07-645表全事件抄读
	{
		PrintInfo(pRdItem, pMtrPara);
		memcpy(pbRxBuf, &pRdItem->dwEvtCnt, sizeof(DWORD));
		iLen = DL645V07AskItemErc(pRdItem, &pMtrPara->bAddr[1], pMtrPara->bAddr[0], pMtrPara->bProId, pRdItem->dwOAD, pbRxBuf);
		if (iLen <= 0)
		{
			pMtrRdCtrl->taskSucFlg[pMtrRdCtrl->schItem.bTaskIdx].fRecSaved = true;	//特殊处理，直接置成功标识！
			return -1;
		}
		UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[pMtrRdCtrl->schItem.bTaskIdx].bTaskId, TASK_MONIINDEX_RCVNUM);
		SaveMtrData(pMtrRdCtrl, pRdItem->bReqType, pRdItem->bCSD, pbRxBuf, iLen);	
		//SaveTask(pMtrRdCtrl);
		return iLen;
	}
	else
	{
		if (pRdItem->bReqType == 1)	
		{
			pbTx = bTxBuf;
			*pbTx++ = (pMtrPara->bProId==PRO_TYPE_645_97)? PRO_TYPE_645_97: PRO_TYPE_645_07;
			*pbTx++ = 0x00;	//通信延时相关标识
			*pbTx++ = 0x00;	//附属相关延时标识
			iLen = DL645_9707MakeFrm(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], pMtrPara->bProId, pMtrPara->bSubProId, pRdItem->dwOAD, pbTx+1);//注意，浙江的才这么改！tTMtrPara.bAddr[0]
			if (iLen <= 0)
				return -1;
			*pbTx++ = iLen;	//数据长度
			pbTx += iLen;
			RouterPause();
			PrintInfo(pRdItem, pMtrPara);
			iLen = Afn13Fn01_RtFwd(&pMtrPara->bAddr[1],  pMtrPara->bAddr[0], bTxBuf, pbTx-bTxBuf, pMtrRdCtrl, pRdItem, pbRxBuf, pMtrPara->bProId, false, true);
			if (iLen < 0)
			{
				*pbRxBuf++ = DAR;
				*pbRxBuf++ = DAR_REQ_TIMEOUT;
			}
			else
				pbRxBuf += iLen;
		}
		else	
		{
			BYTE bTmpBuf[512] = {0};
			BYTE bRecNum = pRdItem->bRCSD[0];

			for (BYTE i=0; i<bRecNum; i++)
			{
				pbTx = bTxBuf;
				*pbTx++ = (pMtrPara->bProId==PRO_TYPE_645_97)? PRO_TYPE_645_97: PRO_TYPE_645_07;
				*pbTx++ = 0x00;	//通信延时相关标识
				*pbTx++ = 0x00;	//附属相关延时标识

				pRdItem->wRcsdIdx = i;
				memrcpy((BYTE* )&dwRealOAD, &pRdItem->bRCSD[1+i*5+1], 4);
				if ((pRdItem->dwOAD == 0x50040200) ||(pRdItem->dwOAD ==0x50050200)||(pRdItem->dwOAD ==0x50060200))//日、月、结算
				{
					if (dwRealOAD < 0x20000000) //电量和需量
					{
						if (pRdItem->dwOAD == 0x50040200)
							dwRealOAD += 4;
						else
							dwRealOAD += 5;
					}
				}

				iLen = DL645_9707MakeFrm(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], pMtrPara->bProId, pMtrPara->bSubProId, dwRealOAD, pbTx+1);//注意，浙江的才这么改！tTMtrPara.bAddr[0]
				if (iLen <= 0)
					return -1;

				*pbTx++ = iLen;	//数据长度
				pbTx += iLen;
				RouterPause();
				iLen = Afn13Fn01_RtFwd(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], bTxBuf, pbTx-bTxBuf, pMtrRdCtrl, pRdItem, bTmpBuf, pMtrPara->bProId);
				if (iLen < 0)
				{
					if (dwRealOAD == 0x20210200)	//返回-1，表示电表日冻结时间与终端时间不匹配
						return -1;
				}
				if (iLen > 0)
				{
					if (iLen > (wMaxRxLen-(pbRxBuf-pbRxBuf0)))
					{
						DTRACE(DB_CCT, ("ReadDLT_645(): Receive buffer overflow, wMaxRxLen=%d, Current recv Len(%d) > Surplus len(%d).\n", 
									wMaxRxLen, iLen, wMaxRxLen-(pbRxBuf-pbRxBuf0)));
						return -1;
					}
					memcpy(pbRxBuf, bTmpBuf, iLen);
					pbRxBuf += iLen;
				}
				else
				{
					*pbRxBuf++ = DAR;
					*pbRxBuf++ = DAR_REQ_TIMEOUT;
				}
			}
		}
	}

	iLen = pbRxBuf - pbRxBuf0;
	pbRxBuf = pbRxBuf0;

	SaveMtrData(pMtrRdCtrl, pRdItem->bReqType, pRdItem->bCSD, pbRxBuf, iLen);
	return iLen;
}

//描述：抄读698.45表协议接口
int CStdReader::ReadDLT_69845(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen)
{
	int iLen, iApduLen;
	BYTE *pbRxBuf0 = pbRxBuf;
	BYTE bApdu[256];
	BYTE bTxBuf[1024];
	BYTE *pbTx = bTxBuf;

	if (pRdItem->bReqType == 1)
		iApduLen = GetRequestNormal(pRdItem->dwOAD, bApdu);
	else
		iApduLen = GetRequestRecord(pRdItem->dwOAD, bApdu, pRdItem->bRSD, pRdItem->wRsdLen, pRdItem->bRCSD, pRdItem->wRcsdLen);

	*pbTx++ = PRO_TYPE_TRANS;
	*pbTx++ = 0x00;	//通信延时相关标识
	*pbTx++ = 0x00;	//附属相关延时标识

	iLen = Make698_45Frm(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], 0x43, 0, 0, SER_ADDR_TYPE_SIG, bApdu, iApduLen, pbTx+1);
	if (iLen < 0)
		return -1;

	*pbTx++ = iLen;	//数据长度
	pbTx += iLen;

	RouterPause();
	PrintInfo(pRdItem, pMtrPara);
	iLen = Afn13Fn01_RtFwd(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], bTxBuf, pbTx-bTxBuf, pMtrRdCtrl, pRdItem, pbRxBuf, pMtrPara->bProId);
	if (iLen < 0)
	{
		if ((pRdItem->dwOAD&0xfff0ffff)==0x50000200)	//冻结数据
			return -1;
	}
	if (iLen > 0)
	{
		pbRxBuf += iLen;
	}
	else
	{
		*pbRxBuf++ = DAR;
		*pbRxBuf++ = DAR_REQ_TIMEOUT;
	}

	iLen = pbRxBuf - pbRxBuf0;
	pbRxBuf = pbRxBuf0;
	SaveMtrData(pMtrRdCtrl, pRdItem->bReqType, pRdItem->bCSD, pbRxBuf, iLen);

	return iLen;
}

//描述：抄读水汽热表协议数据
int CStdReader::ReadDLT_SBJC(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen)
{
	int iLen;
	BYTE *pbRxBuf0 = pbRxBuf;
	BYTE bTxBuf[1024];
	BYTE *pbTx = bTxBuf;
	DWORD dwRealOAD;

	*pbTx++ = PRO_TYPE_TRANS;
	*pbTx++ = 0x00;	//通信延时相关标识
	*pbTx++ = 0x00;	//附属相关延时标识

	iLen = DL645_EXTMakeFrm(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], pMtrPara->bProId, pMtrPara->bSubProId, pRdItem->dwOAD, pbTx+1);
	if (iLen <= 0)
		return -1;

	*pbTx++ = iLen;
	pbTx += iLen;
	RouterPause();
	iLen = Afn13Fn01_RtFwd(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], bTxBuf, pbTx-bTxBuf, pMtrRdCtrl, pRdItem, pbRxBuf, pMtrPara->bProId);
	if (iLen > 0)
	{
		pbRxBuf += iLen;
	}
	else
	{
		*pbRxBuf++ = DAR;
		*pbRxBuf++ = DAR_REQ_TIMEOUT;
	}

	iLen = pbRxBuf - pbRxBuf0;
	pbRxBuf = pbRxBuf0;
	SaveMtrData(pMtrRdCtrl, pRdItem->bReqType, pRdItem->bCSD, pbRxBuf, iLen);

	return iLen;
}

//描述：设置14-01路由请求抄读的信息
int CStdReader::SetRouterRequestInfo(BYTE bState, BYTE *pBuf)
{
	BYTE *pBuf0 = pBuf;

	*pBuf++ = bState;
	*pBuf++ = 0x00;	//通信延时相关性标志
	*pBuf++ = 0x00;	//路由请求数据长度L
	*pBuf++ = 0x00;	//从节点附属节点数量n

	pBuf = pBuf0;

	return 4;
}

//描述：更新告警事件信号屏蔽字
//参数：@wIndex 发现未知电表索引
//		@fState false:清除索引wIndex对应的标识，反之
bool SetSchMtrEvtMask(WORD wIndex, bool fState)
{
	if (g_CStdReader)
		return g_CStdReader->SetSchMtrEvtMask(wIndex, fState);

	return false;
}


//描述：更新告警事件信号屏蔽字
bool UpdataSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen)
{
	if (g_CStdReader)
		return g_CStdReader->UpdataSchMtrEvtMask(pbMask, wMaskLen);

	return false;
}

//描述：获取告警事件信号屏蔽字
bool GetSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen)
{
	if (g_CStdReader)
		return g_CStdReader->GetSchMtrEvtMask(pbMask, wMaskLen);

	return false;
}

//描述：清除告警事件屏蔽字
void ClearSchMtrEvtMask()
{
	if (g_CStdReader)
		g_CStdReader->ClearSchMtrEvtMask();
}

//描述：通过索引获取告警数据
//参数：@iIndex 告警索引
//		@pbBuf 返回的告警数据内容
//返回：-1获取数据失败，>0告警数据的长度
int GetSchMtrEvtData(int iIndex, BYTE *pbBuf)
{
	if (g_CStdReader)
		return g_CStdReader->GetSchMtrEvtData(iIndex, pbBuf);

	return -1;
}
