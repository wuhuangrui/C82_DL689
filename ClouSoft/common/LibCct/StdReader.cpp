/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CctRdCtrl.cpp
 * ժ    Ҫ���ز�������ƹ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�CL
 * ������ڣ�2016��8��
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
	//��ȡ�˿�Ȩ��
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
	//�ͷŶ˿�Ȩ��
	m_TRunStateInfo.fDirRdFlg = false;
	//·�ɻָ�����CctRunStateMonitor���ָ�
	// 	if (m_TRunStateInfo.fRtPause)
	// 	{
	// 		m_TRunStateInfo.fRtPause = false;	//�ָ�·��
	// 		Afn12Fn03_RtResume();
	// 	}
	UnLockReader();
}


//��������ʼ��·��ģ��
//������@bFn F1-Ӳ����ʼ����F2-��������ʼ����F3-��������ʼ��
//���أ��Ƿ��ʼ���ɹ�
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
	ClearRcv(); //��ջ������������յ������֡

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

//��������ȡ�ز���ʼ��״̬
bool CStdReader::GetInitState()
{
	return m_TRtStat.fPlcInit;
}

//������Ӳ����ʼ��
bool CStdReader::Afn01Fn01_HardwareInit()
{
	return InitRouter(FN(1));
}

//������������ʼ�����ָ��������ã�
bool CStdReader::Afn01Fn02_ParmInit()
{
	return InitRouter(FN(2));
}

//��������������ʼ��
bool CStdReader::Afn01Fn03_DataInit()
{
	return InitRouter(FN(3));
}

//��������·���ϱ���ģ����Ϣ����
void  CStdReader::Afn03Fn10_RptRtRunInfo(BYTE* pbBuf)
{
	BYTE bBuf[32] = {0};
	if(pbBuf == NULL)
		return ;

	memset(&m_RtRunMdInfo, 0x00, sizeof(m_RtRunMdInfo));
	DTRACE(DB_CCT, ("/*************************************************************/\r\n"));
	DTRACE(DB_CCT, ("CStdReader :: RtRunModResolve Start.\r\n"));
	m_RtRunMdInfo.bModType = pbBuf[0] & 0x0f;  //ͨ�ŷ�ʽ
	m_RtRunMdInfo.bRtType = (pbBuf[0]&0x10)>>4;   //·�ɹ���ʽ
	m_RtRunMdInfo.bNodeType = (pbBuf[0]&0x20)>>5; //�ӽڵ���Ϣģʽ
	m_RtRunMdInfo.bRdType = (pbBuf[0]&0xc0)>>6;   //���ڳ���ģʽ

	DTRACE(DB_CCT, ("Router Module Type:%d-%s.\r\n",m_RtRunMdInfo.bModType, (m_RtRunMdInfo.bModType==CCT_RD_TYPE_PLC ? "PLC" : "Radio")));
	DTRACE(DB_CCT, ("Router Manage Type:%d-%s.\r\n",m_RtRunMdInfo.bRtType, (m_RtRunMdInfo.bRtType==0 ? "No Route" : "Route Manage Module-Self")));
	DTRACE(DB_CCT, ("Router Node Type:%d-%s.\r\n",m_RtRunMdInfo.bNodeType, (m_RtRunMdInfo.bNodeType==0 ? "not download Node" : "Need download Node to Module")));
	DTRACE(DB_CCT, ("Router Read Meter Type:%d-%s \r\n",m_RtRunMdInfo.bRdType, (m_RtRunMdInfo.bRdType==CCT_RD_BY_TERM ? "Term initiate Read Meter" : "Module initiate Read Meter")));

	if(m_RtRunMdInfo.bRdType == CCT_RD_TERM_ROUTE)  //������ֶ�֧�֣���ֻʹ��·�������İ�
		m_RtRunMdInfo.bRdType = CCT_RD_BY_ROUTE; 
	m_RtRunMdInfo.bTansTmOut = (pbBuf[1] & 0x07);  //������ʱ����֧��
	m_RtRunMdInfo.bRdFailType = ((pbBuf[1]&0x18)>>3); //ʧ�ܽڵ��л�����ʽ
	m_RtRunMdInfo.bBdCfmType = (pbBuf[1]&0x20)>>5;  //�㲥����ȷ�Ϸ�ʽ
	m_RtRunMdInfo.bBdChnlType = (pbBuf[1]&0xc0)>>6; //�㲥�����ŵ�ִ�з�ʽ
	m_RtRunMdInfo.bChnlNum = (pbBuf[2] & 0x1f); //�ŵ���������5λ  
	m_RtRunMdInfo.bVolMis = (pbBuf[2]&0xe0)>>3;   //��ѹ����������Ϣ����3λ
	m_RtRunMdInfo.bVolMis = pbBuf[3] & 0x0f;   //������������4λ
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

//����������AFN=05 Fn=1�ϱ��ӽڵ���Ϣ
bool CStdReader::Afn05Fn01_SetMainNodeAddr()
{
	BYTE bCtrl;
	BYTE bR[6] = {0}; 
	BYTE bTxBuf[256] = {0};
	BYTE wTxLen;
	BYTE bData[32];

	bCtrl = 0x41;

	bR[0] = 0; //D2ͨ��ģ���ʶ��0��ʾ�Լ�������ͨ��ģ�����;1��ʾ���ز����ͨ��ģ�������
	bR[1] = 0;		//�ŵ�
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

//�����������㲥
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

//����������AFN=6 Fn=1�ϱ��ӽڵ���Ϣ
void CStdReader::Afn06Fn01_RptNodeInfo()
{

}

//����������AFN=6 Fn=2�ϱ���������
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

	pbDt++;	//�ӽڵ����
	pbDt++;	//ͨ��Э������
	wDlyTime = ByteToWord(pbDt);	//��ǰ���ı���ͨ������ʱ��
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
					SaveMtrData(pMtrRdCtrl, tRptItem.bReqType, tRptItem.bCSD, bBuf, wLen);	//+2:����1�ֽ�DAR+��¼����
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

//����������AFN=6 Fn=3�ϱ�·�ɹ�����Ϣ
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

//����������AFN=6 Fn=4�ϱ��ӽڵ���Ϣ���豸����
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

//����������AFN=6 Fn=5�ϱ��ӽڵ��¼�
//��ע���㽭ЭҪ���̨���ѱ���ͨ��AFN=06 F5�ϱ�
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

//��������ȡ·��ģ���ز��ڵ�����
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
	ClearRcv(); //��ջ������������յ������֡

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

//��������ȡ�ӽڵ���Ϣ AFN=10H-F2
//������@bRdMtrNum һ���Զ�ȡ������
//		@wStartSn ��ʼ���
//		@pbOutBuf ��������,������ַ6�ֽ�+�ӽڵ���Ϣ2�ֽڡ�
//���أ���ȡ���ĵ����Ч����
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
	ClearRcv(); //��ջ������������յ������֡

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

//��������ѯ·�ɵ�����״̬
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

//����������ز������㵽·��ģ�� AFN=11H-F1
//������@pbInBuf �������Ϣ���ӽڵ�����+[��ַ6�ֽ�+Э������1�ֽ�]+[...]+...��
//		@bInLen ���ݳ���
//���أ��Ƿ���ӳɹ�
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

//������ɾ���ز������㵽·��ģ�� AFN=11H-F2
//������@pbInBuf ���ӽڵ�����+[���ַ6�ֽ�]+...��
//		@bInLen ���ݳ���
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

	ClearRcv(); //��ջ������������յ������֡

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

//����������·������
bool CStdReader::Afn12Fn01_RtRestart()
{
	m_TRunStateInfo.fRtPause = false;	//����·��ʱ���������·����ͣ��ʶ

	return RtCtrl(FN(1));
}

//����������·����ͣ
bool CStdReader::Afn12Fn02_RtPause()
{
	return RtCtrl(FN(2));
}

//����������·�ɻָ�
bool CStdReader::Afn12Fn03_RtResume()
{
	return RtCtrl(FN(3));
}

//������·������ת���� AFN=13H-F1
//������@pbTsa ���ַ
//		@bTsaLen ���ַ����
//		@pbInbuf ΪF1����������
//		@wInLen ΪF1�����ݳ���
//		@pbOutBuf ΪF1�ĳ�������
//		@bProId ΪЭ������
//���أ��������ݳ���
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
		bR[0] = (1<<2); //D2ͨ��ģ���ʶ��0��ʾ�Լ�������ͨ��ģ�����;1��ʾ���ز����ͨ��ģ�������
		bR[1] = 0;		//�ŵ�
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

						pData += 3;	//2�ֽڵ�ǰ���ı���ͨ������ʱ�� + 1�ֽ�ͨ��Э������
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
											if ((pRdItem->dwOAD&0xfff0ffff)==0x50000200)	//��������
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
	//�������ʾ�Ѿ�����ʧ�ܣ������Ч����
	pbOutBuf[0] = DAR;
	pbOutBuf[1] = DAR_REQ_TIMEOUT;
	if (bProId == PROTOCOLNO_DLT645_V07)
		return -1;
	else
		return 2;
}

//����������AFN=14 Fn=1·�����󳭶�����
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

	//��·��ģ���������Ϣ����ȡ���ַ
	memset((BYTE*)&tInfo, 0, sizeof(tInfo));
	tInfo.bPhase = m_TRcv13762.bDtBuf[0];
	tInfo.bRevTsa[0] = tInfo.bTsa[0] = 6;
	memcpy(tInfo.bTsa+1, &m_TRcv13762.bDtBuf[1], 6);

	//��·��ģ������ĵ���ַת������
	tInfo.wPn = RouterMtrAddrConvertPn(tInfo.bTsa+1);
	tInfo.bTsaLen = GetMeterAddrLen(tInfo.wPn);
	tInfo.bTsa[0] = tInfo.bTsaLen;

	if (tInfo.wPn <= 0)	//��ʾ�����㲻���ڣ�ֱ����Ϊ�����ɹ���ʶ
	{
		pbTx += SetRouterRequestInfo(RT_RD_SUCC, pbTx);
		DTRACE(DB_CCT, ("AFN14-F1: Invalid Meter:0x%s.\n", HexToStr(tInfo.bTsa+1, tInfo.bTsa[0], szTsa)));
	}
	else
	{
		//�����ַȡ��
		revcpy(tInfo.bRevTsa+1, tInfo.bTsa+1, tInfo.bTsa[0]);
		tInfo.bRevTsa[0] = tInfo.bTsa[0];

		//��ȡ������ͱ��ַ��Ӧ�ĳ�����ƽṹ
		pMtrRdCtrl = GetMtrRdCtrl(tInfo.wPn, tInfo.bTsa);
		if (pMtrRdCtrl == NULL)
		{
			pbTx += SetRouterRequestInfo(RT_RD_SUCC, pbTx);
			DTRACE(DB_CCT, ("AFN14-F1: GetMtrRdCtrl fail, Meter:0x%s.\n", HexToStr(tInfo.bTsa+1, tInfo.bTsa[0], szTsa)));
		}
		else
		{
			DoTaskSwitch(pMtrRdCtrl);

			//��ȡ�ò������Ӧ������Ϣ
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
						*pbTx++ = 0x00;	//�ӽڵ㸽���ڵ�����n
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

	//376.2���Ʋ�������Ϣ��
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

//����������AFN=14 Fn=2 ·�������ն�ʱ��
//������
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
		m_TRunStateInfo.fRtPause = true;	//��ͣ·��
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

//������ֱ�ӳ�������
//������@wMtrSn
//		@pbBuf Ϊarray CSD,array ROAD
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
	bTxBuf[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bTxBuf[2] = 0x00;	//���������ʱ��ʶ

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
					*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
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
					*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
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
				*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
			}

			pApdu++;	//��������ʱ��ȡʱ�䡱
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
	bTxBuf[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bTxBuf[2] = 0x00;	//���������ʱ��ʶ

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
					*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
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
					*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
				}
				else
				{
					*pbData++ = 0x00;
				}
			}
			else
			{
				*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
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
				*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
			}

			pApdu++;	//��������ʱ��ȡʱ�䡱
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
	bTxBuf[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bTxBuf[2] = 0x00;	//���������ʱ��ʶ


	iTxLen = Make698_45Frm(bTsa, bTsaLen, 0x43, 0, 0, SER_ADDR_TYPE_SIG, bBuf, pbBuf-bBuf, &bTxBuf[4]);
	if (iTxLen > 0)
	{
		bTxBuf[3] = iTxLen;
		iRet = DoFwdData(bTsa, bTsaLen, bTxBuf, iTxLen+4, wTimeOut, pbData);
		iRet -= 2;	//��ȥʱ���ǩ+�ϱ���Ϣ��
	}

	return iRet;
}

struct MeterID {
	DWORD dwOAD;  // Э���OAD��
	DWORD  dwInterID[5]; //�����ڲ�OADӳ��� OAD
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
	bTxBuf[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bTxBuf[2] = 0x00;	//���������ʱ��ʶ


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
				pbData[0] = 0x01;	//1�ֽ�choice
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
	bTxBuf[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bTxBuf[2] = 0x00;	//���������ʱ��ʶ


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

	BYTE *pRespDataState = pbData;	//��¼����+M����¼ = 2 BYTE
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
			if ((tRdItem.dwOAD == 0x50040200) ||(tRdItem.dwOAD ==0x50050200)||(tRdItem.dwOAD ==0x50060200))//�ա��¡�����
			{
				if (dwRealOAD < 0x20000000) //����������
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
		else	//ROAD �ݲ����Ǹ����
		{
			*pbData++ = 0x00;
			DTRACE(DB_FAPROTO, ("CStdReader::Set_OAD_to_645_meter(): OoScanData error, bType=%d, bTmpChoice=%d, dwOAD=%08x.\n", bType, bTmpChoice, OoOadToDWord(pApdu)));
			break;
		}
	}
	UnLockDirRd();

	pRespDataState[0] = 0x01;	//��¼����
	pRespDataState[1] = 0x01;	//M����¼

	iRet = pbData - pbData0;
	pbData = pbData0;
	return iRet;

}



//������ֱ�ӳ�����������
//������@bType �������ͣ������á���ȡ������
//		@bChoice bType����������������NORMAL\NORMAL_LIST
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
		if(bType == 5)	//��ȡ
		{
			if (bChoice == 1)	//��ȡ����
			{
				iRet =	Read_OneOAD_from_645_meter(bType, bChoice, bTsa, bTsaLen, pApdu, wApduLen, wTimeOut, pbData);	
			}
			else if (bChoice == 2)	//��ȡ���OAD 
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
			else if (bChoice == 3)	//��ȡ��¼��
			{
				iRet = 	Read_RecordData_from_645_meter(bType, bChoice, bTsa, bTsaLen, pApdu, wApduLen, wTimeOut, pbData);	
			}
		}
		else if (bType==6 || bType==7)	//���� || ����
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
//������ֱ�ӳ�����������
//������@bType �������ͣ������á���ȡ������
//		@bChoice bType����������������NORMAL\NORMAL_LIST
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
	bTxBuf[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bTxBuf[2] = 0x00;	//���������ʱ��ʶ

	if (GetMeterInfo(bTsa, bTsaLen, &tTMtrInfo) < 0)
		return -1;

	if (tTMtrInfo.bProType == 3)	//698.45
	{
		iTxLen = Make698_45Frm(bTsa, bTsaLen, 0x43, 0, 0, SER_ADDR_TYPE_SIG, bBuf, pbBuf-bBuf, &bTxBuf[4]);
		if (iTxLen > 0)
		{
			bTxBuf[3] = iTxLen;
			iRet = DoFwdData(bTsa, bTsaLen, bTxBuf, iTxLen+4, wTimeOut, pbData);
			iRet -= 2;	//��ȥʱ���ǩ+�ϱ���Ϣ��
			return iRet;
		}
	}
	else //645-07
	{
		if(bType == 5)	//��ȡ
		{
			if (bChoice == 1)	//��ȡ����
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
						pbData[0] = 0x01;	//1�ֽ�choice
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
			else if (bChoice == 2)	//��ȡ���OAD 
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
							pbData[0] = 0x01;	//1�ֽ�choice
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
			else if (bChoice == 3)	//��ȡ��¼��
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

				BYTE *pRespDataState = pbData;	//��¼����+M����¼ = 2 BYTE
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
						if ((tRdItem.dwOAD == 0x50040200) ||(tRdItem.dwOAD ==0x50050200)||(tRdItem.dwOAD ==0x50060200))//�ա��¡�����
						{
							if (dwRealOAD < 0x20000000) //����������
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
					else	//ROAD �ݲ����Ǹ����
					{
						*pbData++ = 0x00;
						DTRACE(DB_FAPROTO, ("DirAskProxy(): OoScanData error, bType=%d, bChoice=%d, dwOAD=%08x.\n", bType, bChoice, OoOadToDWord(pApdu)));
						break;
					}
				}
				UnLockDirRd();

				pRespDataState[0] = 0x01;	//��¼����
				pRespDataState[1] = 0x01;	//M����¼

				iRet = pbData - pbData0;
				pbData = pbData0;
				return iRet;
			}
		}
		else if (bType==6 || bType==7)	//���� || ����
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
							*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
						}
						else
						{
							*pbData++ = 0x00;
						}
					}
					else
					{
						*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
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
							*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
						}
						else
						{
							*pbData++ = 0x00;
						}
					}
					else
					{
						*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
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
						*pbData++ = DAR_RES_RW;	//645��֧�֣����á�������ֱ�ӻء��ܾ���д��
					}

					pApdu++;	//��������ʱ��ȡʱ�䡱
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
//����:��645֡,�����Ѿ��ŵ�֡��
//����:֡����
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

//��������698.45���ݽṹ
//������@pbMtr	���ַ
//		@bMtrLen ���ַ����	 ��ַ���ֽ�����ȡֵ��Χ��0��15����Ӧ��ʾ1��16���ֽڳ���
//		@bCtrl	���Ʋ���	
//		@bAFType	��ַ���������ַ������־�����ñ��뷽ʽ��0��ʾ����ַ��1��ʾͨ���ַ��2��ʾ���ַ��3��ʾ�㲥��ַ
//		@bCA	�ͻ�����ַCA��1�ֽ��޷���������ʾ��ȡֵ��Χ0��255��ֵΪ0��ʾ����ע�ͻ�����ַ
//		@wFramFmt ��֡��ʽ
//		@pbAPDUData	Ӧ��Э����Ƶ�Ԫ����
//		@wAPDULen	����
//		@pbOutBuf	���ز���
//���أ�pbOutBuf���ݳ���
int CStdReader::Make698_45Frm(BYTE *pbMtr, BYTE bMtrLen, BYTE bCtrl, BYTE bAFType, BYTE bCA, WORD wFramFmt, BYTE *pbAPDU, WORD wAPDULen, BYTE *pbRespBuf)
{
	WORD wLen;
	BYTE *p = pbRespBuf;
	BYTE *pHeadCs;	//֡ͷУ��	

	//--------------֡ͷ--------------
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

	//-----------��·�û�����-----------
	if (bCtrl & 0x20)	//������־λPRM
	{
		WordToByte(wFramFmt, p);
		p += 2;
	}
	memcpy(p, pbAPDU, wAPDULen);
	p += wAPDULen;
	*p++ = 0; //TimeTag  OPTIONAL

	//-----------�������ݳ���------------
	wLen = (p - pbRespBuf) + PRO_69845_FRAM_CS_LEN - PRO_69845_LEN_OFFSET;
	WordToByte(wLen, &pbRespBuf[1]);

	//-----------֡ͷУ��------------
	WordToByte(CheckCRC16(pbRespBuf+PRO_69845_LEN_OFFSET, pHeadCs-pbRespBuf-PRO_69845_LEN_OFFSET), pHeadCs);

	//-----------֡У��------------
	WordToByte(CheckCRC16(pbRespBuf+PRO_69845_LEN_OFFSET, p-pbRespBuf-PRO_69845_LEN_OFFSET), p);
	p += 2;
	*p++ = 0x16;

	return p - pbRespBuf;
}


//��������buf����ת����698.45���ݽṹ
//������@pbInBuf ����
//		@wInLen	 ���ݳ���
//		@ptTFrmFmt ����ֵ
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


//��������APDU����ȡOAD�����OAD, ͬʱ����Ӧ��������ȡ����
//������@pApdu ·��ģ���ϱ���APDU֡
//		@wApduLen APDU���ݳ���
//		@pRptItem ���ص�OAD������OAD
//		@pbBuf APDU�е�OAD��������
//		@fIsRptFlg �Ƿ���AFN06-F2����֡
//���أ����ݳ���
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
			pApdu++;	//��ȡһ�����ԣ�ȥ��һ���ֽڵĽ��DAR��Data
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

				//*pbCSD++ = 0x01;	//CSD��ΪROAD
				memcpy(pbCSD, pApdu, 4);
				pbCSD += 4;
				pApdu += 4;
				bLnkNum = *pApdu++;

				dwEvtSnOAD = OoOadToDWord(pApdu+1);
				if (dwEvtSnOAD == 0x20220200)
				{
					pApdu += 5;
					*pbCSD++ = bLnkNum - 1;	//+1:ȫ�¼�����ʱ����һ�����¼���¼��š�������ȥ������Ȼ����ƥ���¼������е�RCSD
				}
				for (BYTE i=0; i<bLnkNum; i++)
				{
					pApdu++;
					memcpy(pbCSD, pApdu, 4);
					pbCSD += 4;
					pApdu += 4;
				}

				pApdu += 2;	//��¼����+M����¼
				wLen = wApduLen - (pApdu - pApdu0);
				pbBuf += WordToByte(wLen, pbBuf);
				memcpy(pbBuf, pApdu, wLen);
				pbBuf += wLen;

				wLen = pbBuf - pbBuf0;
				pbBuf = pbBuf0;
			}
			else
			{
				*pbCSD++ = 0x01;	//CSD��ΪROAD
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

				pApdu += 2;	//��¼����+M����¼
				wLen = wApduLen - (pApdu - pApdu0);
				memcpy(pbBuf, pApdu, wLen);
			}
		}
	}

	return wLen;
}


//��������1376.2֡���ú���ʵ�����й������Ӧ���ܣ�
//������@pbTsa	���ַ
//		@bTsaLen	���ַ����
//		@bCtrl	������
//		@pbR	��Ϣ��
//		@bAfn	������
//		@wFn	���ݵ�Ԫ
//		@pbInbuf	�������ݣ��������������������ݿ���Ϊ698.45��645��T188��376.2���ݵ�Ԫ֡��ͨ��ǰ�����ֽ��ж�Э������
//		@wInLen	�������ݳ���
//		@pbOutBuf ��������
//		@wOutLen	�������ݳ���
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
	if (pbR[0] & 0x04)	//ͨ��ģ���ʶ
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

//������1376.2���ݽṹת����
//������@ptTFrmFmt	1376.2���ݽṹ֡
//		@pbOutBuf	��������
//���أ����ݳ���
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
	if (ptTFrmFmt->bR[0] & 0x04)	//ͨ��ģ���ʶ
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

//��������ȡ��������Ϣ
BYTE CStdReader::GetCtrl(BYTE bRcvCtrl)
{
	BYTE bCtrl = 0;

	//ʱ��������·��ģ�鷢�𣬼�·��ģ��������վ���������Ӷ�վ
	bCtrl = m_RtRunMdInfo.bModType;
	bCtrl |= (bRcvCtrl & 0x40)? 0x00 : 0x40;

	return bCtrl;
}

//����:֡��ʽ�е�DT��FNת��
//����:��ȷ�򷵻�FN,���򷵻�0
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

//����:FN��֡��ʽ��DT��ת��
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

//��������RSD��RCSD����
//		@pbRSD ����RSD�ڴ�
//		@wRSDLen ����RSDLen
//		@pbRCSD ����RCSD�ڴ�
//		@wRCSDLen ����RCSDLen
//		@bType �ɼ���ʽ
//		@pbData �ɼ���ʽ����
//		@pbCSD ��RCSD������
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
		case 0: //�ɼ���ǰ����
			break;

		case 1:	//�ɼ��ϵ�N��
			*pbRSD++ = 9;
			*pbRSD++ = DT_UNSIGN; //unsigned
			*pbRSD++ = *pbData;
			*pdwOAD = ByteToDWord(pbCSD);
			dwOAD = OoOadToDWord(pbCSD);	
			pbCSD += 4;
			break;

		case 2:	//������ʱ��ɼ�	��Ӧ����ʱ�꣺0x2021
			*pbRSD++ = 1; //RSD ����1
			OoDWordToOad(0x20210200, pbRSD);	//OAD=0x20210200	��¼ѡ��������
			pbRSD += 4;
			*pbRSD++ = 0x1C; //date_time_s

			*pdwOAD = ByteToDWord(pbCSD);
			dwOAD = OoOadToDWord(pbCSD);	
			pbCSD += 4;
			switch (dwOAD)
			{
				case 0x50030200:	//Сʱ����
					*pbRSD++ = tmNow.nYear/256;
					*pbRSD++ = tmNow.nYear%256;
					*pbRSD++ = tmNow.nMonth;
					*pbRSD++ = tmNow.nDay;
					*pbRSD++ = tmNow.nHour;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					break;

				case 0x50040200:	//�ն���
				case 0x50050200:	//�����ն���
					*pbRSD++ = tmNow.nYear/256;
					*pbRSD++ = tmNow.nYear%256;
					*pbRSD++ = tmNow.nMonth;
					*pbRSD++ = tmNow.nDay;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					break;

				case 0x50060200:	//�¶���
					*pbRSD++ = tmNow.nYear/256;
					*pbRSD++ = tmNow.nYear%256;
					*pbRSD++ = tmNow.nMonth;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					break;

				case 0x50070200:	//�궳��
					*pbRSD++ = tmNow.nYear/256;
					*pbRSD++ = tmNow.nYear%256;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					*pbRSD++ = 0;
					break;
			}
		case 3:	//��ʱ�����ɼ�TI
			break;
	}
	*wRSDLen = (WORD)(pbRSD - pbRSD0);

	bNum = *pbCSD;	//����OAD
	*wRCSDLen = 1 + bNum*4;
	memcpy(pbRCSD, pbCSD, *wRCSDLen);
	pbRCSD = pbRCSD0;
}

//������ֱ��ִ��͸����������
int CStdReader::DirectTransSchMsg(BYTE bSchNo, TransFilePara *pTransFilePara, TTransMsg *pTTransMsg)
{
	TTime tTime;
	BYTE bTxBuf[512], bRxBuf[512];
	BYTE *pbRxBuf;
	BYTE bCtrl, bR[6];
	WORD wTxLen, wRxLen;

	//��1376.2֡����֡
	memset(bRxBuf, 0, sizeof(bRxBuf));
	bRxBuf[0] = 0;	//0:͸������, 1:DL/T645-1997, 2:DL/T645-2007
	bRxBuf[1] = 0;	//ͨ����ʱ��ر�ʶ
	bRxBuf[2] = 0;	//�ӽڵ㸽���ڵ�����
	bRxBuf[3] = pTTransMsg->bMsgLen;
	memcpy(bRxBuf+4, pTTransMsg->bMsgBuf, pTTransMsg->bMsgLen);
	wRxLen = pTTransMsg->bMsgLen+4;
	bCtrl = 0x41;
	bR[0] = (1<<2); //D2ͨ��ģ���ʶ��0��ʾ�Լ�������ͨ��ģ�����;1��ʾ���ز����ͨ��ģ�������
	bR[1] = 0;		//�ŵ�
	bR[2] = 0xff;
	bR[3] = 0x0;
	bR[4] = 0;
	bR[5] = 0;
	wTxLen = Make1376_2Frm(pTransFilePara->bTsa, pTransFilePara->bTsaLen,  bCtrl, bR, AFN_RTFWD, FN(1), bRxBuf, wRxLen, bTxBuf);
	memset(bRxBuf, 0, sizeof(bRxBuf));
	//���̶��ֶ�����[�������_1BYTE + ����ִ��ʱ��_7byte + ͨ�ŵ�ַTSA + �������_1Byte + ������Ӧʱ��_7Byte + data_255Byte]
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
	BYTE *pbRespMsgTime = pbRxBuf;	//�ȱ��汨����Ӧʱ��
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

					pbFmtPtr += 3;	//���� 2�ֽڡ���ǰ���ı���ͨ������ʱ����+ 1�ֽڡ�ͨ��Э�����͡�
					wRxLen = *pbFmtPtr++;
					//����0xFE
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
	bR[0] = (1<<2); //D2ͨ��ģ���ʶ��0��ʾ�Լ�������ͨ��ģ�����;1��ʾ���ز����ͨ��ģ�������
	bR[1] = 0;		//�ŵ�
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
						BYTE *pData = m_TRcv13762.bDtBuf + 3;	//���� 2�ֽڡ���ǰ���ı���ͨ������ʱ����+ 1�ֽڡ�ͨ��Э�����͡�

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
										memcpy(pbRespBuf, tFrm.bAPDUData+3, tFrm.wAPDULen-3);	//-3:���� ����+�����е�ѡ��+PIID-ACD
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
	pbRespBuf[1] = 33;	//����ʱ
	pbRespBuf[2] = 0;	//�ϱ�
	pbRespBuf[3] = 0;	//ʱ���ǩ
	UnLockDirRd();

	return 4;
}

//��������ʼ������ID���ȣ���ʼ��һ����Ч���������õ�Ԫ
void CStdReader::InitTaskIdSched()
{
	memset((BYTE*)&m_TRunStateInfo, 0, sizeof(m_TRunStateInfo));
}

//������͸��������Ƴ���
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

			//��ʼ��͸��������ʼִ�в���
			if (!m_TRunStateInfo.tTransSchSate.fStart)
			{
				m_TRunStateInfo.tTransSchSate.fStart = true;
				m_TRunStateInfo.tTransSchSate.fFinsh = false;
				m_TRunStateInfo.tTransSchSate.iStart = -1;
				m_TRunStateInfo.tTransSchSate.iMsgSn = 0;
				m_TRunStateInfo.tCurExeTaskCfg = tTaskCfg;
			}

			//��ǰ͸�������Ѿ�ִ�����
			if (m_TRunStateInfo.tTransSchSate.fFinsh)
				SetTaskUpdateTime(tTaskCfg.bTaskId, dwCurSec);
			fRet = true;
		}
	}

	return fRet;
}

//������ͨ��ʱ����(�롢�֡�ʱ)�ж��Ƿ���ֱ��������
bool CStdReader::IsDirReadTi(TTimeInterv *pTi)
{
	if (pTi->bUnit>=TIME_UNIT_SECONDS && pTi->bUnit<=TIME_UNIT_HOUR)
		return true;
	return false;
}


#define MINNUM_MTR_NUM		10//200//10
#define MAXNUM_PERCENTAGE	(100)	//�ն���������ɰٷֱ�
#define MAXNUM_HOUR			4		//�ն������񳭶�����Сʱ
#define MAXNUM_MIN			00		//�ն������񳭶���������

//��������ѯ�ն�������ĳ���״̬
//���أ�����true��ʾ�ն������񳭶���ɣ����ն������񳬳�ʱ�����
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
	if (wTolPnNum >= MINNUM_MTR_NUM)	//����MINNUM_MTR_NUM������г������ȼ�ʱ�����
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

					wPnToAllTaskNum=0;	//���ַ��Ӧ��������Ч�������
					wPnToSucTaskNum=0;	//���ַ��Ӧ�����г����ɹ�����Ч�������
					bTaskIdx = 0;
					while (bTaskIdx < MTR_TASK_NUM)
					{
						if (pMtrRdCtrl->taskSucFlg[bTaskIdx].bValid == 1)
						{	
							wPnToAllTaskNum++;	

							if (GetTaskCfg(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, &tTaskCfg) /*&& !IsDirReadTi(&tTaskCfg.tiExe)*/)	//���ա��¶������Ž��г���ɹ����ж�
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
#if 0	//����������RCSD�жϳɹ���
											wTolCsdNum = pbTaskCSD[1];
											wSucCsdNum = CalcuBitNum(pMtrRdCtrl->taskSucFlg[bTaskIdx].bSucFlg, TASK_SUC_FLG_LEN);
											if (wTolCsdNum == wSucCsdNum)	//���ַ��Ӧ�Ĳɼ������������Ƿ�ɼ��ɹ�
												wPnToSucTaskNum++;
#else	//ͨ������ĳɹ���ʶ�жϳɹ���
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

					//��ȣ���ʾ�ñ��ַ��Ӧ�������ն�������ȫ����������
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

//������ִ�вɼ�����
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
//���������µ������·��ģ��
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
	if (wRtMtrNum==0 || abs(wDbMtrNum-wRtMtrNum)>5)	//������������5��ֱ�Ӳ�������ʼ��,�ٽ�ϵͳ��ĵ�����ӵ�·��ģ��
	{
		if (Afn01Fn02_ParmInit())
		{
			bAddNum = 0;
			pbPtr = bBuf + 1;

			for (WORD k=0; k<wDbMtrNum; k++)
			{
				if (tDbMtrInfo[k].bMtrPro == PRO_TYPE_69845)	//�������Э��698.45ת��Ϊ͸������Э��
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
	else //�ȶԵ���
	{
		WORD wValidRtMtrNum = 0;
		BYTE bRdRoutAddr[POINT_NUM][8]={0};
		BYTE bNoExitMtrMask[PN_MASK_SIZE] = {0};
		BYTE bRdRtMtrMask[PN_MASK_SIZE] = {0};
		WORD wRtTotalMtrNum;
		BYTE bRdMtrNum;

		//��ȡ·��ģ���ﵵ�����ȶ���ϵͳ�����Ƿ����
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
					pbPtr += 2;	//�����ӽڵ���Ϣ
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

					if (fSameMtrFlg)	//��ϵͳ���в����ڵĵ���¼�������������ͳһɾ��
						bRdRtMtrMask[wValidRtMtrNum/8] |= (1<<(wValidRtMtrNum%8));
					else
						bNoExitMtrMask[wValidRtMtrNum/8] |= (1<<(wValidRtMtrNum%8));
					wValidRtMtrNum ++;	
				}
			}

			wStartMtrSn += PER_RD_MTR_CNT;
		}

		//�ն���ĵ���ַ��·��ģ�����ַ��һ�£�ɾ����һ�µĵ��
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
		//�ȶԵ���
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

				if (tDbMtrInfo[wDbMtrIdx].bMtrPro == PRO_TYPE_69845)	//�������Э��698.45ת��Ϊ͸������Э��
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
					(tNowTime.nMinute>bDlyMin))	//�Ƿ����
			{
				m_TRunStateInfo.tLastUdpTime = tNowTime;
				m_TRunStateInfo.fRtPause = false;	//����·��
				DTRACE(DB_CCT, ("Interval day restart router.\n"));
				CheckMeterSearchTime();
				Afn12Fn01_RtRestart();
			}
			else if (m_TRunStateInfo.fRtPause && (m_bCctExeState!=CCT_SCH_MTR))
			{
				m_TRunStateInfo.fRtPause = false;	//�ָ�·��
				Afn12Fn03_RtResume();
			}

			//���3Сʱ����һ��·��
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
	bData[0] = 0x02;	//00H��͸�����䣻01H��DL/T645��1997��02H��DL/T645��2007��03H����λʶ���ܣ�04H��FFH������

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

//�����������ѱ�
//���أ�-1-�ѱ�ʧ�ܣ�0-�㲥����������1-���ڹ㲥
int CStdReader::StartSchMtr()
{
	int iBroadCastMin;

	//��ȡ����ʧ��ֱ���˳���������ǰ�ѱ�
	if (!GetSchMtrParam(&m_TSchMtrParm))
		return -1;

	//ʱ���ѱ� 
	if (m_fPeriodSchMtr)
	{
		//δ����Ϊÿ�������ѱ�
		if (!m_TSchMtrParm.fAutoSchMtr)
			return -1;
		//�����ѱ�ʱ��
		if (!IsSchMtrPeriod())
			return -1;
	}

	//��ע��1.ÿ�������ѱ�ǰ���	ÿ�������ѱ���0x60020900
	//		2.ÿ���ѱ�ǰ���		ÿ���ѱ�������������ѱ��Լ�ÿ�������ѱ�
	if (m_TSchMtrParm.bClrMtrChoice==1 && m_fPeriodSchMtr && !m_fClrFile)	//ÿ�������ѱ�ǰ���,���������������ѱ�
	{
		m_fClrFile = true;
		DeleteSearchMtrFile();
		DeleteCrossSearchMtrFile();
		DTRACE(DB_CCT, ("Daily cycle search meter before clear File.\n"));
	}	
	else if (m_TSchMtrParm.bClrMtrChoice==2 && !m_fClrFile)	//ÿ���ѱ�ǰ���
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
			if (iCount < iBroadCastSec)	//�����㲥�ѱ�ʱ��
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
	//�����������ѱ� && ��ʱ���ѱ� �� �����ѱ�ʱ�Σ�
	if (!m_fRightNowSchMtr && (m_fPeriodSchMtr && !IsSchMtrPeriod()))
		return true;

	if (m_fRightNowSchMtr)
	{
		int iKeptTime = GetRightNowSchMtrKeepTime();

		if(iKeptTime <= 0)	//0��ʾ����ʱ��ֱ���ѱ����
			iKeptTime = 120;	//����120min

		if (GetClick() - m_dwStartBoardCastClk > iKeptTime*60)	//���������ѱ����
			return true;
	}

	if (GetClick() - m_dwLastRptMtrClk >= 10*60)
	{
		BYTE bBuf[128];

		memset(bBuf, 0, sizeof(bBuf));
		if (Afn10Fn04_QueryRtRunInfo(bBuf)>0)
        {
		    if ((bBuf[0] & 0x03)==0x01)	//Bit0=1,Ϊ·����ɱ�־
            {      
			    m_fRptSchMtrEnd = true;
            }
                /*
            if (bBuf[13]==0x08 && bBuf[14]==0x08 && bBuf[15]==0x08)	//�ൺ����Ҫ��AFN=10 F4���ص������У����๤�������Ϊ08ʱ�ز��ӽڵ�����ע����Ϊ����
            {// �����ж��Ƿ�ͨ�ã���Ҫȷ��
                m_fRptSchMtrEnd = true;            
            }
            */
        }			

		if (m_fRptSchMtrEnd)
        {      
			return true;
        }

		m_dwLastRptMtrClk = GetClick();

        //�Ƿ��ٴμ���ӽڵ�����ע���д���ȶ�������е绪����������ز����൱������һ���ѱ�
		//StartNodeActive();	//����10���ӣ��ٴ������ӽڵ�ע��
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

//������ִ���������񣬰����ѱ�͸��������ֱ������
void CStdReader::DoOtherTask()
{
	if (!m_TRtStat.fPlcInit)
		return;

	if (DoSchMtrAddr())	//�����ѱ�
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

//������ͨ���������ز�·�ɷ�����
//���أ��ѷ��ͳ���
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

//�������ز�·�ɵ�����
//���أ����յ��ĳ���
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

//����:	����һ�����ݿ�,�ж��Ƿ���յ�һ��������ͨ��֡
//����:	�����Ѿ�ɨ������ֽ���,����յ�һ��������ͨ��֡�򷵻�����,���򷵻ظ���
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
					if (m_TRcv13762.bR[0] & 0x04)	//ͨ��ģ���ʶ,Ϊ0ʱ���޵�ַ��A
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

//�������ز�Э����������
//���أ���Բ��ܽ�����ֱ�ӷ���false
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
			if (bFn == FN(1))	//�ϱ��ӽڵ���Ϣ
				Afn06Fn01_RptNodeInfo();
			else if (bFn == FN(2))	//�ϱ���������
				Afn06Fn02_RptData();
			else if (bFn == FN(3))	//�ϱ�·�ɹ����䶯��Ϣ
				Afn06Fn03_RptRtInfo();
			else if (bFn == FN(4))	//�ϱ��ӽڵ���Ϣ���豸����
				Afn06Fn04_RptMtrInfo();
			else if (bFn == FN(5))	//�ϱ��ӽڵ��¼�
				Afn06Fn5_RptNodeEvt();
			else
				fRet = true;
			break;
		case AFN_RTRD:
			if (bFn == FN(1))	//·�����󳭶�����
				Afn14Fn1_RtReqRd();
			else if (bFn == FN(2))	//·����������ʱ��
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

//���������ݴ���
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

		if (len <= 0 || !m_fRxComlpete)	//���ȴ���ѭ�������������ݣ����߻����������ݣ������ղ�����������Ӧȥ��ʣ�µĽ��ջ���һ����
		{				//���ѭ���������������ݣ��ȴ����껺�����������ڽ��գ������п��ܻ����	
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
				m_LoopBuf.DeleteFromBuf(nScanLen); //ɾ���Ѿ�ɨ�������

				if (fIsDefHanleFrm)
				{
					if (DefHanleFrm() == false) //֡��ɹ���,����û�б�Ĭ��֡������������,Ҫ��������ȥ����
						return true;
				}
				else
					return true;
			}
			else if(len>=sizeof(bBuf)-10) //���������ˣ����ǻ�û��һ��������֡
			{
				m_LoopBuf.DeleteFromBuf(len); //ɾ���Ѿ�ɨ�������
			}
		}

		if (dwSeconds != 0)
			Sleep(100);	//��ֹ����������������ѭ��

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
	int iMonitorID = ReqThreadMonitorID("CStdReader-thrd", 4*60*60);	//�����̼߳��ID,

	

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
		*p++ = 0x01;	//�ϱ��ӽڵ������n
		memcpy(p, pbMtrAddr, 6);	//�ӽڵ�1ͨ�ŵ�ַ
		p += 6;
		*p++ = bPro;	//�ӽڵ�1ͨ��Э������
		*p++ = 0;	//�ӽڵ�1���	
		*p++ = 0;
		*p++ = 0;	//�ӽڵ�1�豸����

		dwOAD = 0xF2010200 + (bBaud&0x1f);
		g_CStdReader->SaveSchMtrResult(dwOAD, bBuf, p-bBuf, bAddrLen);
	}
}

CStdReader *g_CStdReader;

//645����ز���֡�ӿ�
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

//�ı���ز���֡�ӿ�
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
			wFrmLen = MakeExt645Frm(bAddr, DL645EXT_CMD_T188, bData, eLen, bpBuf);//�Ȳ��ܲɼ�����ַ 
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

//ע�⣬�Է���֡�ĵ�ַ��ID����û������һ������ȷ�ԱȽϵ�
int CStdReader::GetDL645_9707DataVal(BYTE *psData, BYTE bsLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *pbData, TRdItem *pRdItem, bool fAnaly645data)
{
	//BYTE bAfnPos = FRM_AFN_A;
	//Toad645Map* pOad645Map = GetOad645Map(dwOAD);
	int iLen = 0;
	int i=0, iDataLen, iIdLen,j;
	BYTE bCtrl;
	BYTE buf[250];

	memset(buf, 0, sizeof(buf));
	for ( i=0;i<bsLen;i++)  //���ص�645���Ŀ�����ǰ����
	{
		if (psData[i]==0x68 && psData[i+7]==0x68)
			break;				
	}

	if(i+1 == bsLen)
		return -1;

	bsLen -=i;
	bCtrl = psData[i+8];//ȡ�ÿ�����
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
				//Ԥ��ʱ����Ŀռ�,ȥ����
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
			else if (dwOAD == 0x20210200)	//�ն���ʱ��
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
				//Ԥ��ʱ����Ŀռ䣬ȥ����
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
			else if (dwOAD == 0x20210200)	//�ն���ʱ��
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
	else if (bCtrl== 0x94)//�Ե��Уʱ�ظ�Ϊ0x94
	{
		return 0;
	}
	else
	{
		goto MTRRET_ERR;
	}


MTRRET_ERR:
	//�������ʾ�Ѿ�����ʧ�ܣ������Ч����
	DTRACE(DB_CCT, ("Cct DL645_9707 return err\n"));
	return -1;

	//return 0;
}

//ע�⣬�Է���֡�ĵ�ַ��ID����û������һ������ȷ�ԱȽϵ�
int CStdReader::GetDL645_EXTDataVal(BYTE *psData, BYTE bsLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *pbData)
{
	//BYTE bAfnPos = FRM_AFN_A;
	//Toad645Map* pOad645Map = GetOad645ExtMap(dwOAD);
	int iLen = 0;
	int i=0, iDataLen, iIdLen,j;
	BYTE bCtrl;
	BYTE buf[250];

	memset(buf, 0, sizeof(buf));
	for ( i=0;i<bsLen;i++)  //���ص�645���Ŀ�����ǰ����
	{
		if (psData[i]==0x68 && psData[i+7]==0x68)
			break;				
	}

	if(i+1 == bsLen)
		return -1;

	bsLen -=i;
	bCtrl = psData[i+8];//ȡ��645֡�Ŀ�����
	if (bCtrl == 0x9f)
	{
		bCtrl = psData[i+10];////ȡ����չ������
		if (bCtrl == 0xc8)
			goto MTRRET_ERR;
		bCtrl = psData[i+20];////ȡ��T188֡������
		if (bCtrl != 0x81)
			goto MTRRET_ERR;

		int iDataLen = psData[i+21];
		DWORD dwMtrID = 0;
		dwMtrID = ByteToWord(&psData[i+22]);
		if (iDataLen <= 2)
		{
			goto MTRRET_ERR;
		}
		memcpy(buf, &psData[i+24+1], iDataLen-2-1);//ID������һ���ֽڵ����к�SER
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
	//�������ʾ�Ѿ�����ʧ�ܣ������Ч����
	DTRACE(DB_CCT, ("Cct DL645_EXT return err\n"));
	return -1;

	//return 0;
}

//����ַ�㲥Уʱ����
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
		//if (bBuf1[9] == 1)//����
		{
			//memset(bBuf2, 0, sizeof(bBuf2));
			GetCurTime(&tmTm);
			{
				tmMtr.nYear = OoLongUnsignedToWord(&pbData[1]);
				tmMtr.nMonth = pbData[3];
				tmMtr.nDay= pbData[4];
				tmMtr.nHour= pbData[5];
				tmMtr.nMinute= pbData[6];
				tmMtr.nSecond= pbData[7]+2;//��ʱ�ȹ̶���1���ͨ����ʱ
				if (IsInvalidTime(tmMtr))
					return -1;
			}

			//if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[5]*3600+bBuf1[6]*60+bBuf1[7]))
			//	return -1;

			dwSecM = TimeToSeconds(tmMtr);
			dwSecT = TimeToSeconds(tmTm);
			//if ((dwSecM > (dwSecT+300)) || (dwSecT > (dwSecM+300)))//ʱ������5���ӵĲ����
			//	return -1;

			if (dwSecM > (dwSecT+bBuf1[3]) ||dwSecT > (dwSecM+bBuf1[3]))//���ʱ�䳬��
			{
				bTsaLen = pbTsa[0];
				g_MtrClkPrg.bEvtSrcTSA[0] = DT_TSA;
				g_MtrClkPrg.bEvtSrcTSA[1] = bTsaLen+1;
				g_MtrClkPrg.bEvtSrcTSA[2] = bTsaLen-1;
				memcpy(&g_MtrClkPrg.bEvtSrcTSA[3], &pbTsa[1], bTsaLen);
				memcpy(&g_MtrClkPrg.bClock[0], pbData, 8);
				g_MtrClkPrg.bClkErr[0] = DT_INT;
				if (bProId == PROTOCOLNO_DLT645_V07 || bProId == PROTOCOLNO_DLT645)// 07��Уʱ�Ƿ����β�����
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
					bBuf1[15] = 0x00;	//ʱ���ǩ

					iStep = Make698_45Frm(pbTsa+1, bTsaLen, 0x43, 0, 0, SER_ADDR_TYPE_SIG, bBuf1, 16, pbInBuf+4);
					pbInBuf[3] = iStep;
					DTRACE(DB_CCT, ("OneAddrBroadcast 69845 write id=0x40000200.\r\n")); 
					Afn13Fn01_Broadcast(pbTsa+1, bTsaLen, pbInBuf, iStep+4, pRdItem, pbData, bProId, 0);
					//Уʱ���ٶ���ʱ�����
					iApduLen = GetRequestNormal(pRdItem->dwOAD, bApdu);
					//DTRACE(DB_CCT, ("Send RequestNormal Mtr:%s OAD:0x%s.\n",HexToStr(bDbTsa+1, bDbTsa[0], szTsa), HexToStr((BYTE*)&tRdItem.dwOAD, 4, szOAD, true)));
					bFrm69845[0] = PRO_TYPE_TRANS;
					bFrm69845[1] = 0x00;	//ͨ����ʱ��ر�ʶ
					bFrm69845[2] = 0x00;	//���������ʱ��ʶ

					bApdu[iApduLen++] = 0x00;	//ʱ���ǩ
					iLen69845 = Make698_45Frm(pbTsa+1, pbTsa[0], 0x43, 0, 0, SER_ADDR_TYPE_SIG, bApdu, iApduLen, &bFrm69845[4]);
					bFrm69845[3] = iLen69845;
					memset(bBuf1, 0, sizeof(bBuf1));
					iRet = Afn13Fn01_RtFwd(pbTsa+1, pbTsa[0], bFrm69845, iLen69845+4, NULL, pRdItem, bBuf1, tTMtrPara.bProId);
					if (iRet == 2)//��������ʱ��
						iRet = -1;

				}
				if (iRet > 0)
				{
					tmMtr.nYear = OoLongUnsignedToWord(&bBuf1[1]);
					tmMtr.nMonth = bBuf1[3];
					tmMtr.nDay= bBuf1[4];
					tmMtr.nHour= bBuf1[5];
					tmMtr.nMinute= bBuf1[6];
					tmMtr.nSecond= bBuf1[7]+1;//��ʱ�ȹ̶���1���ͨ����ʱ

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
						g_MtrClkPrg.bClkErr[1] =(bDel^0xff) + 1;//�����ĸ����Ǹ�������λȡ���ټ�1
					}
					else
					{
						dwDel = dwSecM - dwSecT;
						if (dwDel > 127)
							bDel = 127;
						else
							bDel = dwDel;
						g_MtrClkPrg.bClkErr[1] = bDel;//����Ϊ��
					}
				}
				else
					g_MtrClkPrg.bClkErr[1] = 2;//���������ʱ��͸���1��2�����ҵ����ֵ

				SetInfo(INFO_TERM_MTRCLKPRG);
				DTRACE(DB_CCT, ("send INFO_TERM_MTRCLKPRG \r\n")); 
				return 1;
			}
		}
	}
	return -1;
}

extern TTime tLastTime[31];
//����ַ�㲥Уʱ�����ܽӿ�
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
		if (bCheckTimePara[9] == 1)	//����
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
				if (IsSameDay(tNowTime, tLastTime[30]))	//һ��ִֻ��һ��
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
					*pbTx++ = 0x00;	//ͨ����ʱ��ر�ʶ
					*pbTx++ = 0x00;	//���������ʱ��ʶ

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
						*pApdu++ = 0x00;	//ʱ���ǩ
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

//�㲥Уʱ
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
	BYTE bDbTsa[TSA_LEN] = {0};	//ϵͳ����ʵ�ʵĵ���ַ��Ϣ��bDbTsa[0]���ַ���ȣ�bDbTsa[1...TSA_LEN-1]Ϊ�����ַ
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
	if(m_bCctExeState==CCT_SCH_MTR)//�ѱ�״̬�����й㲥
		return -1;

	//���ڹ㲥����ֻ���ǰ�645��ķ�һ���ٰ������ķ�һ��
	memset(bBuf1, 0, sizeof(bBuf1));
	tRdItem.bReqType = 1;
	tRdItem.dwOAD = 0x40000200;
	BYTE bBufY[18]={0x68, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x68, 0x08, 0x06, 0x01, 0x02, 0x03, 0x07, 0x03, 0x17, 0x5E, 0x16};
	if (OoProReadAttr(0x4204, 2, 0, bBuf1, sizeof(bBuf1), &iStep) > 0)
	{
		if (bBuf1[7] == 1)//����
		{
			GetCurTime(&tmTm);
			if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[3]*3600+bBuf1[4]*60+bBuf1[5]))
				return -1;

			if (tmTm.nYear == tLastTime[15].nYear && tmTm.nMonth== tLastTime[15].nMonth && tmTm.nDay== tLastTime[15].nDay)//һ��ִֻ��һ��
				return -1;

			Afn12Fn02_RtPause();
			memcpy( (BYTE *)&tLastTime[15].nYear,(BYTE *)&tmTm, sizeof(TTime));

			bFrm69845[0] = 0x00;//00H��͸�����䣻01H��DL/T645��1997��02H��DL/T645��2007��03H����λʶ���ܣ�04H��FFH������

			//if (bProId == PROTOCOLNO_DLT645_V07 || bProId == PROTOCOLNO_DLT645)// 07��Уʱ�Ƿ����β�����
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
				//ֱ����������ʱ�ȴ��������ٷ�����֡��,�����ڵȴ��ڼ����Ӧ���˱��������
				//�������һ����ô��
				if (m_dwLastWaitSec > 180)
					m_dwLastWaitSec = 180;
				Sleep(m_dwLastWaitSec*1000);

				//�ز��ķ��˹㲥֡�������Ҫ�ȴ�һ�����Ӱɣ������ȷ��
			}
			//if (bProId == PROTOCOLNO_DLT69845)
			{
				bBuf1[0] = 7;//ACTION_REQ
				bBuf1[1] = DL69845_APPSVR_GETREQUEST_NORMAL;	//GET-Request-NORMAL
				bBuf1[2] = 0; //PIID
				bBuf1[3] = 0x40;
				bBuf1[4] = 0x00;
				bBuf1[5] = 0x7f;// ����127
				bBuf1[6] = 0x00;
				bBuf1[7] = 0x1c;
				bBuf1[8] = tmTm.nYear>>8;
				bBuf1[9] = tmTm.nYear & 0x00ff;
				bBuf1[10] = tmTm.nMonth;
				bBuf1[11] = tmTm.nDay;
				bBuf1[12] = tmTm.nHour;
				bBuf1[13] = tmTm.nMinute;
				bBuf1[14] = tmTm.nSecond;
				bBuf1[15] = 0x00;	//ʱ���ǩ

				bBufY[0] = 0xAA;
				iStep = Make698_45Frm(bBufY, 1, 0x43, 3, 0, SER_ADDR_TYPE_SIG, bBuf1, 16, bFrm69845+2);//�����ĵ�ַ�����ȴ���!!!
				bFrm69845[1] = iStep;
				DTRACE(DB_CCT, ("OneAddrBroadcast 69845 write id=0x40000200.\r\n")); 
				memset(bRespData, 0, sizeof(bRespData));
				Afn05Fn3_StartBoardCast(bFrm69845, iStep+2);
				//ֱ����������ʱ�ȴ���,�����ڵȴ��ڼ����Ӧ���˱��������
				//�������һ����ô��
				if (m_dwLastWaitSec > 180)
					m_dwLastWaitSec = 180;
				Sleep(m_dwLastWaitSec*1000);
			}
			Afn12Fn03_RtResume();
		}
	}
	return 0;
}

//������·������ת���� AFN=14H-F1
//������@pbTsa ���ַ
//		@bTsaLen ���ַ����
//		@pbInbuf ΪF1����������
//		@wInLen ΪF1�����ݳ���
//		@pbOutBuf ΪF1�ĳ�������
//		@bProId ΪЭ������
//���أ��������ݳ���
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
		bR[0] = (1<<2); //D2ͨ��ģ���ʶ��0��ʾ�Լ�������ͨ��ģ�����;1��ʾ���ز����ͨ��ģ�������
		bR[1] = 0;		//�ŵ�
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

						pData += 3;	//2�ֽڵ�ǰ���ı���ͨ������ʱ�� + 1�ֽ�ͨ��Э������
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
	//�������ʾ�Ѿ�����ʧ�ܣ������Ч����
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
	bFrm69845[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bFrm69845[2] = 0x00;	//���������ʱ��ʶ
	pRdItem->dwOAD = 0x40000200;
	iLen69845 = DL645_9707MakeFrm(&tTMtrPara.bAddr[1], tTMtrPara.bAddr[0], tTMtrPara.bProId, tTMtrPara.bSubProId, pRdItem->dwOAD, &bFrm69845[4]);//ע�⣬�㽭�Ĳ���ô�ģ�tTMtrPara.bAddr[0]
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
	iLen69845 = DL645_9707MakeFrm(&tTMtrPara.bAddr[1], tTMtrPara.bAddr[0], tTMtrPara.bProId, tTMtrPara.bSubProId, pRdItem->dwOAD, &bFrm69845[4]);//ע�⣬�㽭�Ĳ���ô�ģ�tTMtrPara.bAddr[0]
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

//��������ȡ2007��645֧�ֵ��¼�����
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

	if (pErcRdCtrl == NULL) //��֧�ֵ�������
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
	pbTmp += 2; //Ԥ����ʵ�ʳ���
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
	//if (wOI>0x3008 && wOI!=0x300B) //�¼�����Դ
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
	//iRet = OoReadAttr(0x4300, ATTR10, bTmpBuf, NULL, NULL);	//��ȡ���ò���
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
	//			memcpy(pbTmp, &bTmpBuf[5*i+3], 4);	//ͨ��OAD
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
		if (dwRelaOAD==0x201E0200 || dwRelaOAD==0x20200200 || dwRelaOAD==0x20220200) //�����س���
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

//��������ȡ2007��645֧�ֵ��¼�����
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
	bTxBuf[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bTxBuf[2] = 0x00;	//���������ʱ��ʶ
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
					dwBaseOff = 12;	//����ʱ�䣨6��������ʱ�䣨6��
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
					dwBaseOff = 36;	//����ʱ�䣨6��+ ����ʱ�䣨6�� + ����ǰ����Ӧ���ݣ�24��	
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

//����������645��Э������
int CStdReader::ReadDLT_645(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen)
{
	int iLen;
	BYTE *pbRxBuf0 = pbRxBuf;
	BYTE bTxBuf[1024];
	BYTE *pbTx = bTxBuf;
	DWORD dwRealOAD;

	if ((pRdItem->dwOAD&0xff000000) == 0x30000000)	//07-645��ȫ�¼�����
	{
		PrintInfo(pRdItem, pMtrPara);
		memcpy(pbRxBuf, &pRdItem->dwEvtCnt, sizeof(DWORD));
		iLen = DL645V07AskItemErc(pRdItem, &pMtrPara->bAddr[1], pMtrPara->bAddr[0], pMtrPara->bProId, pRdItem->dwOAD, pbRxBuf);
		if (iLen <= 0)
		{
			pMtrRdCtrl->taskSucFlg[pMtrRdCtrl->schItem.bTaskIdx].fRecSaved = true;	//���⴦��ֱ���óɹ���ʶ��
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
			*pbTx++ = 0x00;	//ͨ����ʱ��ر�ʶ
			*pbTx++ = 0x00;	//���������ʱ��ʶ
			iLen = DL645_9707MakeFrm(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], pMtrPara->bProId, pMtrPara->bSubProId, pRdItem->dwOAD, pbTx+1);//ע�⣬�㽭�Ĳ���ô�ģ�tTMtrPara.bAddr[0]
			if (iLen <= 0)
				return -1;
			*pbTx++ = iLen;	//���ݳ���
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
				*pbTx++ = 0x00;	//ͨ����ʱ��ر�ʶ
				*pbTx++ = 0x00;	//���������ʱ��ʶ

				pRdItem->wRcsdIdx = i;
				memrcpy((BYTE* )&dwRealOAD, &pRdItem->bRCSD[1+i*5+1], 4);
				if ((pRdItem->dwOAD == 0x50040200) ||(pRdItem->dwOAD ==0x50050200)||(pRdItem->dwOAD ==0x50060200))//�ա��¡�����
				{
					if (dwRealOAD < 0x20000000) //����������
					{
						if (pRdItem->dwOAD == 0x50040200)
							dwRealOAD += 4;
						else
							dwRealOAD += 5;
					}
				}

				iLen = DL645_9707MakeFrm(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], pMtrPara->bProId, pMtrPara->bSubProId, dwRealOAD, pbTx+1);//ע�⣬�㽭�Ĳ���ô�ģ�tTMtrPara.bAddr[0]
				if (iLen <= 0)
					return -1;

				*pbTx++ = iLen;	//���ݳ���
				pbTx += iLen;
				RouterPause();
				iLen = Afn13Fn01_RtFwd(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], bTxBuf, pbTx-bTxBuf, pMtrRdCtrl, pRdItem, bTmpBuf, pMtrPara->bProId);
				if (iLen < 0)
				{
					if (dwRealOAD == 0x20210200)	//����-1����ʾ����ն���ʱ�����ն�ʱ�䲻ƥ��
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

//����������698.45��Э��ӿ�
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
	*pbTx++ = 0x00;	//ͨ����ʱ��ر�ʶ
	*pbTx++ = 0x00;	//���������ʱ��ʶ

	iLen = Make698_45Frm(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], 0x43, 0, 0, SER_ADDR_TYPE_SIG, bApdu, iApduLen, pbTx+1);
	if (iLen < 0)
		return -1;

	*pbTx++ = iLen;	//���ݳ���
	pbTx += iLen;

	RouterPause();
	PrintInfo(pRdItem, pMtrPara);
	iLen = Afn13Fn01_RtFwd(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], bTxBuf, pbTx-bTxBuf, pMtrRdCtrl, pRdItem, pbRxBuf, pMtrPara->bProId);
	if (iLen < 0)
	{
		if ((pRdItem->dwOAD&0xfff0ffff)==0x50000200)	//��������
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

//����������ˮ���ȱ�Э������
int CStdReader::ReadDLT_SBJC(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen)
{
	int iLen;
	BYTE *pbRxBuf0 = pbRxBuf;
	BYTE bTxBuf[1024];
	BYTE *pbTx = bTxBuf;
	DWORD dwRealOAD;

	*pbTx++ = PRO_TYPE_TRANS;
	*pbTx++ = 0x00;	//ͨ����ʱ��ر�ʶ
	*pbTx++ = 0x00;	//���������ʱ��ʶ

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

//����������14-01·�����󳭶�����Ϣ
int CStdReader::SetRouterRequestInfo(BYTE bState, BYTE *pBuf)
{
	BYTE *pBuf0 = pBuf;

	*pBuf++ = bState;
	*pBuf++ = 0x00;	//ͨ����ʱ����Ա�־
	*pBuf++ = 0x00;	//·���������ݳ���L
	*pBuf++ = 0x00;	//�ӽڵ㸽���ڵ�����n

	pBuf = pBuf0;

	return 4;
}

//���������¸澯�¼��ź�������
//������@wIndex ����δ֪�������
//		@fState false:�������wIndex��Ӧ�ı�ʶ����֮
bool SetSchMtrEvtMask(WORD wIndex, bool fState)
{
	if (g_CStdReader)
		return g_CStdReader->SetSchMtrEvtMask(wIndex, fState);

	return false;
}


//���������¸澯�¼��ź�������
bool UpdataSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen)
{
	if (g_CStdReader)
		return g_CStdReader->UpdataSchMtrEvtMask(pbMask, wMaskLen);

	return false;
}

//��������ȡ�澯�¼��ź�������
bool GetSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen)
{
	if (g_CStdReader)
		return g_CStdReader->GetSchMtrEvtMask(pbMask, wMaskLen);

	return false;
}

//����������澯�¼�������
void ClearSchMtrEvtMask()
{
	if (g_CStdReader)
		g_CStdReader->ClearSchMtrEvtMask();
}

//������ͨ��������ȡ�澯����
//������@iIndex �澯����
//		@pbBuf ���صĸ澯��������
//���أ�-1��ȡ����ʧ�ܣ�>0�澯���ݵĳ���
int GetSchMtrEvtData(int iIndex, BYTE *pbBuf)
{
	if (g_CStdReader)
		return g_CStdReader->GetSchMtrEvtData(iIndex, pbBuf);

	return -1;
}
