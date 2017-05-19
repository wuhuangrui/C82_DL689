/****************************************************************************************************
* Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
* 
* �ļ����ƣ�DpGrp.cpp
* ժ    Ҫ: ���ļ��ṩ�ܼ���������ݵ�ʵ��
* ��ǰ�汾��1.0
* ��    �ߣ�������
* ������ڣ�2008��3��
* ��    ע��$�ܼ���ʹ�õ��Ĳ������15���ӡ��ա������ֵ��CDpMtr���𱣴�
****************************************************************************************************/
#include "stdafx.h"
#include "DpGrp.h"
#include "FaStruct.h"
#include "DbConst.h"
#include "DbAPI.h"
#include "FaAPI.h"
#include "MeterAPI.h"
#include "DbOIAPI.h"
#include "MtrHook.h"
#include <math.h>
#include "apptypedef.h"

static WORD g_wGrpCurPowerID[2] = {0x2302, 0x2303}; //��ǰ�ܼӹ���
static WORD g_wGrpDayDeltaID[2] = {0x2306, 0x2307}; //�����ۼ�
static WORD g_wGrpMonDeltaID[2] = {0x2308, 0x2309}; //�����ۼ�
static WORD g_wGrpDayLeftDeltaID[2] = {0x035F, 0x036F}; //��������ǰʣ�µ���delta����
static WORD g_wGrpMonLeftDeltaID[2] = {0x037F, 0x038F}; //��������ۼ�
static WORD g_wGrpDayStartEnID[2] = {0x039F, 0x03AF}; //�������ʾֵ
static WORD g_wGrpMonStartEnID[2] = {0x03BF, 0x03CF}; //�������ʾֵ

//�����㰴�������̶����µ�ǰ����
static const WORD g_wCurID[] = {0xa010, 0xa020, 0xa030, 0xa040};

static WORD g_wDayStartID[] = {0x003F, 0x004F, 0x005F, 0x006F}; //����ʼ
static WORD g_wMonStartID[] = {0x009F, 0x00AF, 0x00BF, 0x00CF}; //����ʼ

#define GRP_START_E		0	//�ܼ����������
#define GRP_CUR_E		1	//�ܼ���ĵ�ǰ����
#define MTR_START_E		2	//�������������

CDpGrp::CDpGrp()
{

}

CDpGrp::~CDpGrp()
{
	
}

//�������ܼ����ʼ��
//@bPn	�ܼ����
bool CDpGrp::Init(WORD  wPn)
{
	BYTE bNum = 0;

	if (wPn==0 || wPn>GB_MAXSUMGROUP) 
		return false;

	m_wPn = wPn;

	DTRACE(DB_DP, ("Init*********: Grp = %d\r\n", m_wPn));

	GetCurTime(&m_tmLast);	

	m_bMtrNum = 0;   //����������

	//���������
	SetArrVal32(m_iCT, 1, PN_NUM);					//�ܼ������ز������CT����,ȱʡΪ1
	SetArrVal32(m_iPT, 1, PN_NUM);					//�ܼ������ز������PT����,ȱʡΪ1

	memset((BYTE*)m_GrpInfP, 0, sizeof(m_GrpInfP));		//�ܼ��й����ʵ���ز�������Ϣ
	memset((BYTE*)m_GrpInfQ, 0, sizeof(m_GrpInfQ));		//�ܼ��޹����ʵ���ز�������Ϣ
	memset((BYTE*)m_GrpInfEp, 0, sizeof(m_GrpInfEp));	//�ܼ��й����ܵ���ز�������Ϣ
	memset((BYTE*)m_GrpInfEq, 0, sizeof(m_GrpInfEq));	//�ܼ��й����ܵ���ز�������Ϣ
	
	//�������
	memset((BYTE*)&m_biRepP, 0, sizeof(m_biRepP));			//�ܼ��й�����
	memset((BYTE*)&m_biRepQ, 0, sizeof(m_biRepQ));			//�ܼ��޹�����
	memset((BYTE*)&m_biRepEp, 0, sizeof(m_biRepEp));		//�ܼ��й������������ʣ�
	memset((BYTE*)&m_biRepEq, 0, sizeof(m_biRepEq));		//�ܼ��޹������������ʣ�
	
	//�ۼ�����
	memset((BYTE*)m_iDayDeltaE, 0, sizeof(m_iDayDeltaE)); //�����ܼӵ�����ֵ������Ӧ�й����޹���
	memset((BYTE*)m_iMonDeltaE, 0, sizeof(m_iMonDeltaE)); //�����ܼӵ�����ֵ������Ӧ�й����޹���		  

	//��������
	memset((BYTE*)m_gfsdDayStart, 0, sizeof(m_gfsdDayStart));	//����������ʼ���������й����޹���
	memset((BYTE*)m_gfsdMonStart, 0, sizeof(m_gfsdMonStart));	//����������ʼ���������й����޹���
	
	m_fNewDayStartEnFlg[0] = false; //�ܼ��������ʾֵ�Ƿ���õ�ǰʾֵ�ı�־
	m_fNewDayStartEnFlg[1] = false; 
	m_fNewMonStartEnFlg[0] = false; //�ܼ��������ʾֵ�Ƿ���õ�ǰʾֵ�ı�־
	m_fNewMonStartEnFlg[1] = false; 
	m_dwEnNewStartSec = 0; //�ܼ���Ҫ�����µĵ�ǰ���ֵ��ʱ��
	memset((BYTE*)m_dwDayStartEnSec, 0, sizeof(m_dwDayStartEnSec)); //�ܼ��������ʾֵ�ĸ���ʱ��(���й����޹�)
	memset((BYTE*)m_dwMonStartEnSec, 0, sizeof(m_dwMonStartEnSec)); //�ܼ��������ʾֵ�ĸ���ʱ��(���й����޹�)

	memset((BYTE*)m_iOldDayMtrE, 0, sizeof(m_iOldDayMtrE));				//����������ʼʾֵ�����й����޹���	
	memset((BYTE*)m_dwOldDayMtrUpdSec, 0, sizeof(m_dwOldDayMtrUpdSec));	//����������ʼʾֵ��Ӧ��ʱ�꣨���й����޹���
	memset((BYTE*)m_iOldMonMtrE, 0, sizeof(m_iOldMonMtrE));				//����������ʼʾֵ�����й����޹���	
	memset((BYTE*)m_dwOldMonMtrUpdSec, 0, sizeof(m_dwOldMonMtrUpdSec));	//����������ʼʾֵ��Ӧ��ʱ�꣨���й����޹���

	DWORD dwSec = GetCurTime();
	for (int i=0; i<2; i++)
	{
		m_dwDayDeltaESec[i] = dwSec;
		m_dwMonDeltaESec[i] = dwSec;
	}

	if (!InitPara(bNum))
		return false;	//����Ч�ܼӲ���

	m_bMtrIntv = GetMeterInterv(); //�ն˳���������
	if (m_bPnProp == 0) //ȫΪ��������Ȳ�������1���Ӽ���
		m_bMtrIntv = 1;

	LoadData();
	LoadPara();

	return true;
}

//�������Ƚϳ������Ƿ����仯�Ը����ύ����
//		���CTPT�����Ƿ����仯�Լ�ʱ����
void  CDpGrp::LoadPara()
{
	BYTE bBuf[PNPARA_LEN];
	WORD wCt,wPt;
	WORD wLen;
	BYTE *pbFeildFmt;
	WORD wFeildLen;
	const ToaMap *pOadMap;
	BYTE bType, *pbBuf, *pbTmp;

	for (int i=0; i<m_bMtrNum; i++)
	{
		if (m_GrpInfP[i].bProp == PN_PROP_METER)
		{
			if (ReadItemEx(BN0, m_GrpInfP[i].bPn, 0x6000, bBuf) > 0)
			{
				pOadMap = GetOIMap(0x60000200);
				pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, 3, &wLen, &bType, &pbFeildFmt, &wFeildLen);
				pbTmp = OoGetField(pbBuf, pbFeildFmt, wFeildLen, 2, &wLen, &bType);
				wPt = OoLongUnsignedToWord(pbTmp+1);
				pbTmp = OoGetField(pbBuf, pbFeildFmt, wFeildLen, 3, &wLen, &bType);
				wCt = OoLongUnsignedToWord(pbTmp+1);
				
				if (wCt != 0)
					m_iCT[i] = wCt; //CT���
				if (wPt != 0)
					m_iPT[i] = wPt; //PT���
				DTRACE(DB_DP, ("LoadPara: ct = %d pt = %d!\r\n", m_iCT[i], m_iPT[i]));
			}
		}
		else
		{
			if (ReadItemEx(BN0, m_GrpInfP[i].bPn, 0x2402, bBuf) > 0)
			{
				pOadMap = GetOIMap(0x24020200);
				pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, 0, &wLen, &bType);
				wPt = OoLongUnsignedToWord(pbBuf+1);
				pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, 1, &wLen, &bType);
				wCt = OoLongUnsignedToWord(pbBuf+1);

				if (wCt != 0)
					m_iCT[i] = wCt; //CT���
				if (wPt != 0)
					m_iPT[i] = wPt; //PT���
				DTRACE(DB_DP, ("LoadPara: ct = %d pt = %d!\r\n", m_iCT[i], m_iPT[i]));
			}
		}
	}
}

//��������ʼ�����������ݵĺ����ԣ�������Ӧ�Ĵ���
void CDpGrp::LoadData()
{
	int i, j;
	TTime now;
	TIntvSec isNow;
	BYTE bBuf[64];

	int64 iGroupP;
	int64 iGrpE[BLOCK_ITEMNUM];	

	int64 iValBuf[BLOCK_ITEMNUM];
	int iValBuf2[5];

	GetCurTime(&now);

	DTRACE(DB_DP, ("LoadData*********: Grp = %d\r\n", m_wPn));

	//���ۼ�ֵ���,�й����޹�	
	TimeToIntvS(now, INTVDAY, &isNow);
	for (i=0; i<2; i++)
	{
		if (ReadItemEx(BN0, m_wPn, g_wGrpDayDeltaID[i], bBuf, isNow.dwS, isNow.dwEndS) > 0)//û����
		{
			m_iDayDeltaE[i][0] = RATE_NUM;
			for (j=1; j<BLOCK_ITEMNUM;j++)
			{
				m_iDayDeltaE[i][j] = OoLong64ToInt64(&bBuf[3+9*i]);
			}
		}
		else
		{	
			ClsBlockE(BN0, g_wGrpDayDeltaID[i], &m_iDayDeltaE[i][0], INVALID_VAL64, isNow.dwS);				
		}

		if (ReadItemVal64(BN18, m_wPn, g_wGrpDayLeftDeltaID[i], iValBuf, isNow.dwS, isNow.dwEndS) <= 0)//û����
		{	
			ClsBlockE(BN18, g_wGrpDayLeftDeltaID[i], iValBuf, 0, isNow.dwS);			
		}

		DTRACE(DB_DP, ("CDpGrp::LoadData Grp=%d wStartDeltaEnVal=%lld dwSec=%d \n",m_wPn,iValBuf[1], isNow.dwS));		
	}	

	//���ۼ�ֵ����й����޹�		
	TimeToIntvS(now, INTVMON, &isNow);	
	for (i=0; i<2; i++)
	{
		if (ReadItemEx(BN0, m_wPn, g_wGrpMonDeltaID[i], bBuf, isNow.dwS, isNow.dwEndS) > 0)//û����
		{
			m_iMonDeltaE[i][0] = RATE_NUM;
			for (j=1; j<BLOCK_ITEMNUM;j++)
			{
				m_iMonDeltaE[i][j] = OoLong64ToInt64(&bBuf[3+9*i]);
			}
		}
		else
		{		
			ClsBlockE(BN0, g_wGrpMonDeltaID[i], &m_iMonDeltaE[i][0], INVALID_VAL64, isNow.dwS);						
		}

		if (ReadItemVal64(BN18, m_wPn, g_wGrpMonLeftDeltaID[i], iValBuf, isNow.dwS, isNow.dwEndS) <= 0)//û����
		{	
			ClsBlockE(BN18, g_wGrpMonLeftDeltaID[i], iValBuf, 0, isNow.dwS);			
		}	
	}	

	//��ǰ�ܼӹ��ʣ��ܼ�ʾֵ�����ϵ�Ϊ��Ч������������������
	TimeToIntvS(now, INTVCAL, &isNow);
	for (i=0; i<2; i++)
	{
		iGroupP = INVALID_VAL64;
		bBuf[0] = DT_LONG64;
		OoInt64ToLong64(iGroupP, &bBuf[1]);
		WriteItemEx(BN0, m_wPn, g_wGrpCurPowerID[i], bBuf, isNow.dwS);//��ǰ�ܼӹ���
	}
	WriteItemVal64(BN0, m_wPn, 0x0a07, &iGrpE[1], isNow.dwS); //��ǰ�ܼ��й����ܣ�������ʹ��	
	TrigerSaveBank(BN18, 0, -1); //����ͳ�����ֵ��������.
}


//��������ȡ�ܼ�����������ύ�Ĳ������������Ϣ
//@bReqNum	���ر��ܼ�������ID�ĸ������������͵Ĳ�������ύ
//����ֵ��	���ɹ���Ϊtrue
bool CDpGrp::InitPara(BYTE& bReqNum)
{	
	int i=0,j=0;
	WORD wLen;
	BYTE *pbFeildFmt;
	WORD wFeildLen;
	const ToaMap *pOadMap;
	BYTE bType, *pbBuf, *pbTmp;
	BYTE bBuf[GRPPARA_LEN];
	BYTE bMtrPn,bPnType,bIDFlag,bOp;
	
	if (ReadItemEx(BN0, m_wPn, 0x2301, bBuf) > 0)
	{		
		m_bMtrNum = bBuf[1];
		pOadMap = GetOIMap(0x23010200);
		for (i=0; i<m_bMtrNum; i++)
		{
			pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType, &pbFeildFmt, &wFeildLen);
			bMtrPn = MtrAddrToPn(pbBuf+1, pbBuf[1]+1);//�������
			bPnType = GetPnProp(bMtrPn); //����������
			pbTmp = OoGetField(pbBuf, pbFeildFmt, wFeildLen, 1, &wLen, &bType);
			bIDFlag = pbTmp[1]&0x01;
			pbTmp = OoGetField(pbBuf, pbFeildFmt, wFeildLen, 2, &wLen, &bType);
			bOp = pbTmp[1]&0x01;

			m_GrpInfP[i].bPn	= bMtrPn; //�������
			m_GrpInfP[i].bProp	= bPnType; //����������
			m_GrpInfP[i].wCurId = 0xa052; //ID��
			m_GrpInfP[i].bOp	= bOp; //�����
			m_GrpInfP[i].bDir	= bIDFlag;

			m_GrpInfQ[i].bPn	= bMtrPn; //�������
			m_GrpInfQ[i].bProp	= bPnType; //����������
			m_GrpInfQ[i].wCurId = 0xa053; //ID��
			m_GrpInfQ[i].bOp	= bOp; //�����
			m_GrpInfQ[i].bDir	= bIDFlag;

			m_GrpInfEp[i].bPn	= bMtrPn;	   //�������
			m_GrpInfEp[i].bProp = bPnType; //����������
			m_GrpInfEp[i].bDir	= bIDFlag;
			m_GrpInfEq[i].bPn	= bMtrPn;	   //�������			
			m_GrpInfEq[i].bProp = bPnType; //����������			
			m_GrpInfEq[i].bDir	= bIDFlag;
		
			m_GrpInfEp[i].bOp = bOp; //�����			
			m_GrpInfEq[i].bOp = bOp; //�����

			if (bIDFlag == 0)//����
			{
				m_GrpInfEp[i].wCurId = 0xa010;
				m_GrpInfEp[i].wDayStartId = 0x003F;
				m_GrpInfEp[i].wMonStartId  = 0x009F;

				m_GrpInfEq[i].wCurId = 0xa030;
				m_GrpInfEq[i].wDayStartId = 0x004F;
				m_GrpInfEq[i].wMonStartId = 0x00AF;
				
			}
			else//����
			{
				m_GrpInfEp[i].wCurId = 0xa020;
				m_GrpInfEp[i].wDayStartId = 0x005F;
				m_GrpInfEp[i].wMonStartId  = 0x00BF;

				m_GrpInfEq[i].wCurId = 0xa040;
				m_GrpInfEq[i].wDayStartId = 0x006F;
				m_GrpInfEq[i].wMonStartId = 0x00CF;
			}		

			if (bPnType == PN_PROP_METER)//ֻҪ�е��Ͱ�������
			{
				m_bPnProp = 1;
			}
			
			m_biRepP[j].wPn = bMtrPn; //�������
			m_biRepP[j].wID = 0xa052;
			m_biRepP[j].wBn = BN0;

			m_biRepQ[j].wPn = bMtrPn; //�������
			m_biRepQ[j].wID = 0xa053;
			m_biRepQ[j].wBn = BN0;

			m_biRepEp[j].wPn = bMtrPn; //�������
			m_biRepEq[j].wPn = bMtrPn; //�������
			m_biRepEp[j].wBn = BN0;
			m_biRepEq[j].wBn = BN0;				
			m_biRepEp[j].wID = m_GrpInfEp[i].wCurId;
			m_biRepEq[j].wID =  m_GrpInfEq[i].wCurId;	

			m_gfsdDayStart[0].biRepItem[i].wBn = BN18;
			m_gfsdDayStart[0].biRepItem[i].wPn = bMtrPn;
			m_gfsdDayStart[0].biRepItem[i].wID = m_GrpInfEp[i].wDayStartId;

			m_gfsdDayStart[1].biRepItem[i].wBn = BN18;
			m_gfsdDayStart[1].biRepItem[i].wPn = bMtrPn;
			m_gfsdDayStart[1].biRepItem[i].wID = m_GrpInfEq[i].wDayStartId;

			m_gfsdMonStart[0].biRepItem[i].wBn = BN18;
			m_gfsdMonStart[0].biRepItem[i].wPn = bMtrPn;
			m_gfsdMonStart[0].biRepItem[i].wID = m_GrpInfEp[i].wMonStartId;

			m_gfsdMonStart[1].biRepItem[i].wBn = BN18;
			m_gfsdMonStart[1].biRepItem[i].wPn = bMtrPn;
			m_gfsdMonStart[1].biRepItem[i].wID = m_GrpInfEq[i].wMonStartId;			
			j ++; //���ٸ�����������ж��ٸ�ID				
		}
		bReqNum = m_bMtrNum; //= j;		

		return true;
	}

	return false;
}


//��������ȡ���ܵ�˳���������й����޹���
int CDpGrp::GetIdInxE(WORD wCurID)
{
	int iRet = -1;

	for (int i=0; i<2; i++)
	{
		if (wCurID==0xa010 || wCurID==0xa030)
		{
			iRet = 0;
		}
		else if (wCurID==0xa020 || wCurID==0xa040)
		{
			iRet = 1;
		}			
	}

	return iRet;
}


//�������Ƚϳ������Ƿ����仯���Ը����ύ����
bool  CDpGrp::IsNeedReset()
{
	BYTE bMtrIntv = GetMeterInterv(); //�ն˳���������
	if (m_bPnProp == 0) //ȫΪ��������Ȳ�������1���Ӽ���
		bMtrIntv = 1;
	
	if (bMtrIntv != m_bMtrIntv)
	{			
		DTRACE(DB_DP, ("CDpGrp::IsNeedReset() GPN =%d!\n", m_wPn));
		return true;
	}
		
	return false;
}

void CDpGrp::UpdDayMonStartVal(BYTE bInterv, TTime tmNow)
{
	BYTE n;
	WORD wNum;
	TIntvSec isNow,isNew;//��ǰʱ��
	TIntvSec isOldStart;//ԭ�յ����ֵʱ��	
	TIntvSec isOldDelt;	//ԭ�յ��ۼ�ֵʱ��		
	TIntvSec isNewDelt;	//���յ��ۼ�ֵʱ��
	TIntvSec isLastDay;	//���յ�����ʱ��
	DWORD dwSec = 0;
	TTime tm = tmNow;
	TGrpInf* pGrpInf;
	WORD wDeltaID, wStartEnId;
	int64 *piDeltaVal;
	TGrpFrzStartData*   pStartData;
	int64 iGrpE[BLOCK_ITEMNUM];	
	int64 iCurVal[BLOCK_ITEMNUM];
	DWORD *pdwStartSec;
	bool *pfNewStartFlg;

	int64 iNewMtrE[BLOCK_ITEMNUM];
	int64 *piMtrE;
	DWORD *pdwMtrUpdSec;

	SetArrVal64(iGrpE, INVALID_VAL64, BLOCK_ITEMNUM);
	SetArrVal64(iNewMtrE, INVALID_VAL64, BLOCK_ITEMNUM);
	for (n=0; n<2; n++)	//�й����޹�
	{	
		TimeToIntvS(tmNow, bInterv, &isNow); //ȡ��ǰ���¼������ʼʱ��

		if (n == 0)
				pGrpInf = m_GrpInfEp;	
			else 
				pGrpInf = m_GrpInfEq;

		if (bInterv == INTVDAY)		
		{	
			piDeltaVal = &m_iDayDeltaE[n][0];			//���յ�����ֵ��
			pStartData	= &m_gfsdDayStart[n];			//����������ʼ��
			wDeltaID	= g_wGrpDayDeltaID[n];			//�����ܼӵ�����
			wStartEnId = g_wGrpDayStartEnID[n];			//�������ʾֵ
			pdwStartSec = &m_dwDayStartEnSec[0];		//�ܼ��������ʾֵ�ĸ���ʱ��(���й����޹�)
			pfNewStartFlg = &m_fNewDayStartEnFlg[n];	//�ܼ��������ʾֵ�����й����޹����Ƿ���õ�ǰʾֵ�ı�־

			piMtrE = &m_iOldDayMtrE[n][0];				//����������ʼʾֵ�����й����޹�������CTPT,���ڼ����ܼ���Ĳ��������ʼֵ�Ƿ��б仯��
			pdwMtrUpdSec = &m_dwOldDayMtrUpdSec[n];		//����������ʼʾֵ��Ӧ��ʱ�꣨���й����޹���
		}
		else if (bInterv == INTVMON)		
		{
			piDeltaVal = &m_iMonDeltaE[n][0];
			pStartData	= &m_gfsdMonStart[n];	
			wDeltaID	= g_wGrpMonDeltaID[n];
			wStartEnId = g_wGrpMonStartEnID[n];
			pdwStartSec = &m_dwMonStartEnSec[0];
			pfNewStartFlg = &m_fNewMonStartEnFlg[n];

			piMtrE = &m_iOldMonMtrE[n][0];
			pdwMtrUpdSec = &m_dwOldMonMtrUpdSec[n];
		}
		else 
			return;

		//���²�����������ʼֵ
		for (int i=0; i<m_bMtrNum; i++)
		{
			if (ReadItemVal64(pStartData->biRepItem[i].wBn, pStartData->biRepItem[i].wPn, pStartData->biRepItem[i].wID, &pStartData->iStartVal[i][0], isNow.dwS, isNow.dwEndS) <= 0)
			{
				if (bInterv == INTVDAY)
				{
					for (int j=0; j<sizeof(g_wDayStartID)/sizeof(WORD); j++)
					{	
						SetArrVal64(iCurVal, INVALID_VAL64, BLOCK_ITEMNUM);
						iCurVal[0] = m_bRateNum; //������

						//��������� ��Ч��Ч������
						if (ReadItemVal64(BN0, pStartData->biRepItem[i].wPn, g_wCurID[j], &iCurVal[1], isNow.dwS, isNow.dwEndS) > 0) //645ID�޷�����
						{
							WriteItemVal64(BN18, pStartData->biRepItem[i].wPn, g_wDayStartID[j], iCurVal, isNow.dwS);
							DTRACE(DB_DP, ("CDpMtr::DayChange():PN=%d,g_wDayStartID=%x StartVal=%lld\n ",pStartData->biRepItem[i].wPn, g_wDayStartID[j], iCurVal[1]));		
						}	
					}
				}
				else
				{
					for (int j=0; j<sizeof(g_wMonStartID)/sizeof(WORD); j++)
					{	
						SetArrVal64(iCurVal, INVALID_VAL64, BLOCK_ITEMNUM);
						iCurVal[0] = m_bRateNum; //������

						//��������� ��Ч��Ч������
						if (ReadItemVal64(BN0, pStartData->biRepItem[i].wPn, g_wCurID[j], &iCurVal[1], isNow.dwS, isNow.dwEndS) > 0) //645ID�޷�����
						{
							WriteItemVal64(BN18, pStartData->biRepItem[i].wPn, g_wMonStartID[j], iCurVal, isNow.dwS);
							DTRACE(DB_DP, ("CDpMtr::DayChange():PN=%d,g_wDayStartID=%x StartVal=%lld\n ",pStartData->biRepItem[i].wPn, g_wMonStartID[j], iCurVal[1]));		
						}	
					}
				}
			}
		}

		wNum = 0;
		if (QueryItemTime(isNow.dwS, isNow.dwEndS, pStartData->biRepItem, m_bMtrNum, &wNum) == m_bMtrNum)//��ѯ���ܱ����ڱ����¹�
		{
			if (m_bPnProp==1 && pStartData->dwUpdSec!=isNow.dwS && pStartData->dwUpdSec!=0) //��㷢�����ʱ��Ҫ��ʱ������һ��ֵ������ת��
			{
				//�����Ƿ�׼���л����¶��᷽ʽ����׼���л����¶��᷽ʽ�����ܼ�����˵�鵽�����ʱ�����¼��ʱ����					
				if (bInterv == INTVDAY)
				{
					dwSec = 24*60*60;
				}
				else if (bInterv == INTVMON)
				{
					if (tm.nMonth == 1)		
					{
						tm.nYear --;
						tm.nMonth = 12;		
					}
					else
						tm.nMonth --; 
					dwSec = DaysOfMonth(tm)*24*60*60;	//ȡ��һ�µ����(������ʼ �� ����ʱ��)	
				}
				
 				isOldStart.dwS = isNow.dwS-dwSec;  //ԭ�յ����(������ʼ �� ��ʱ��)
				isOldStart.dwEndS = isNow.dwEndS-dwSec; 
				isOldDelt.dwS = isNow.dwS-m_bMtrIntv*60; //ԭ�յ��ۼ�ֵʱ��д�����һ��(������ʼ �� ���ʱ��)
				isOldDelt.dwEndS = isNow.dwS; 					
				isNewDelt.dwS = isNow.dwS;			  //���յ��ۼ�ֵʱ��
				isNewDelt.dwEndS = isNow.dwEndS;
				isLastDay.dwS = isNow.dwS;			  //���յ�����ʱ��
				isLastDay.dwEndS = isNow.dwEndS;
					
				DeltaENew(wDeltaID, pGrpInf, bInterv, isNow, isOldStart, isOldDelt);
				//DeltaE(wDeltaID, pGrpInf, bType, isNow, isOldStart, isOldDelt);		
				DTRACE(DB_DP, ("CDpGrp::UpdDayMonStartVal GPN=%d deltDayVal=%lld dwSec=%d dwNewSec=%d!\n",m_wPn, m_iDayDeltaE[n][1], isOldDelt.dwS, isNow.dwS));		
				UpdClsFrzData(n, bInterv, isLastDay.dwS, isNewDelt.dwS); 	//�����������ת��,д����ʱ��		
			}
			for (int i=0; i<m_bMtrNum; i++)
			{
				ReadItemVal64(pStartData->biRepItem[i].wBn, pStartData->biRepItem[i].wPn, pStartData->biRepItem[i].wID, &pStartData->iStartVal[i][0], isNow.dwS, isNow.dwEndS);					
			}

			//����и��£��˴�Ϊ�����������л����£���ͬ�������ܼ������ʾֵ
			int iRet = ReadItemVal64(BN18, m_wPn, wStartEnId, iGrpE, isNow.dwS, isNow.dwEndS);
			if (pdwStartSec[n]!=isNow.dwS || iRet<=0 || (iRet>0 && IsAllAVal64(&iGrpE[1], INVALID_VAL64, TOTAL_RATE_NUM))) 
			{
				if ( *pfNewStartFlg ) //��Ҫ�������
				{
					isNew = isNow;			
					isNew.dwS = m_dwEnNewStartSec; //�����ȡ�³�����������Ϊ������
					CalcuEnSum(iGrpE, n, bInterv, isNew, GRP_CUR_E);	//���㵱ǰʾֵ���ܼ���ʾֵ���						
				}
				else
				{
					if (iRet<=0 || (iRet>0 && IsAllAVal64(&iGrpE[1], INVALID_VAL64, TOTAL_RATE_NUM)))
						CalcuEnSum(iGrpE, n, bInterv, isNow, GRP_START_E);		//����ԭ���ʾֵ������ͬ������ʾֵ�����ܼ���ʾֵ���	
					//else  Ϊ���¹�����Ч��㣬��ֱ������
				}
				
				WriteItemVal64(BN18, m_wPn, wStartEnId, iGrpE, isNow.dwS); //д�����ʾֵ		 
				//DTRACE(DB_DP, ("CDpGrp::UpdDayMonStartVal Grp=%d WID=0X%2x, wStartCurEnVal=%lld dwSec=%d \n",m_wPn, wStartEnId, iGrpE[1], isNow.dwS));

				//�����³ɹ�
				if ( !IsAllAVal64(&iGrpE[1], INVALID_VAL64, TOTAL_RATE_NUM) ) 
				{
					TrigerSaveBank(BN18, 0, -1); //���ʾֵ	��������.
					pdwStartSec[n] = isNow.dwS;
					*pfNewStartFlg = false;

					CalcuEnSum(iNewMtrE, n, bInterv, isNow, MTR_START_E); 
					memcpy((BYTE*)piMtrE, (BYTE*)&iNewMtrE, sizeof(iNewMtrE));
					*pdwMtrUpdSec = isNow.dwS;
				}
			}	
			else //�������Чʱ������Ƿ��е�����Ȼʾ���½�
			{					
				//�ܼ���ͨ���������¼�Ĳ���������ʾֵ�Ƿ��и��£���������ܼ�������ʾֵ��������ʾֵ��ͬ��ʱ������CTPT�������
				//֮�����������Ƿ����쳣�仯��������Ȼ��ʾ���½����������ܼ���ʾֵ��δ�����ܼ�������ʾֵ���£�
				if (*pdwMtrUpdSec==pStartData->dwUpdSec && *pdwMtrUpdSec!=0)
				{
					CalcuEnSum(iNewMtrE, n, bInterv, isNow, MTR_START_E); 
					for (BYTE i=1; i<BLOCK_ITEMNUM; i++) //�ܼ��ַ��ʵ�ѭ��
					{
						if (iNewMtrE[i] != piMtrE[i])	//�����㵱ǰ��������ֵ��֮ǰ����������ֵ��ͬ��������������и���
						{
							SetStartDeltaE(bInterv, wDeltaID, tmNow);	
							CalcuEnSum(iGrpE, n, bInterv, isNow, GRP_CUR_E);	//���㵱ǰʾֵ���ܼ���ʾֵ���	
							WriteItemVal64(BN18, m_wPn, wStartEnId, iGrpE, isNow.dwS);	//д�����ʾֵ

							memcpy((BYTE*)piMtrE, (BYTE*)&iNewMtrE, sizeof(iNewMtrE));	//���¼�¼�Ĳ�������㣬ʱ�겻�ø���
							DTRACE(DB_DP, ("CDpGrp::UpdDayMonStartVal222 Grp=%d WID=0X%2x, wStartCurEnVal=%lld dwSec=%d \n",m_wPn, wStartEnId, iGrpE[1], isNow.dwS));
							break;
						}
					}
				}
			}

			pStartData->dwUpdSec = isNow.dwS;
		}	
	}
}
//����������ĳʱ�̵��ܼ���������Ĳ�ֵ
//		���㷽ʽΪ����ÿ���ܼ����ʾֵ���ܼ�������ʾֵ������ټ����ۼ�ֵ���
//@wResultID ���ص���������Ӧ��ID
//@pGrpInf	 ���ܼ����Ӧ�Ĳ�������Ϣ�ṹ����
//@bType	 ��ֵ�ļ������ͣ�2��ʾ�ղ�ֵ 3��ʾ�²�ֵ
//@isNow	 ��ʾʾֵ��ʱ��
//@isStart	 ��ʾ��������ʱ��
//@isDelta	 ��ʾ�ۼ�ֵ��ʱ��
void CDpGrp::DeltaENew(WORD wResultID, TGrpInf* pGrpInf, BYTE bInterv, TIntvSec isNow, TIntvSec isStart, TIntvSec isDelta)
{
	int64 iCurVal[BLOCK_ITEMNUM];	//�ܼ�����ܼ��ַ��ʵ�ǰʾֵ	
	int64 iStartVal[BLOCK_ITEMNUM];	//�ܼ�����ܼ��ַ������ʾֵ	
	int64 iLeftDeltaE[BLOCK_ITEMNUM];	//�ܼ�����ܼ��ַ����ڲ�������ǰʣ�µĵ���deltaֵ	
	int64 iGrpE[BLOCK_ITEMNUM];		//�ܼ�����ܼ��ַ���ʾֵ
	bool fUpd = false;
	bool fAdd = false;
	int64 *piResVal;
	DWORD *pdwTime, *pwStartTime;	
	WORD *pwStartId, *pwLeftDeltaId, *pwDeltaId;
	int iRv = -1;

	SetArrVal64(iCurVal, INVALID_VAL64, BLOCK_ITEMNUM);
	SetArrVal64(iStartVal, INVALID_VAL64, BLOCK_ITEMNUM);
	SetArrVal64(iLeftDeltaE, INVALID_VAL64, BLOCK_ITEMNUM);
	SetArrVal64(iGrpE, 0, BLOCK_ITEMNUM); //�����ۼƵ���		

	int n = GetIdInxE(pGrpInf->wCurId);	
	if (n < 0)
		return;
		
	//��������
	//�ۼ�ֵ
	if (bInterv == INTVDAY) //���ۼ�
	{	
		piResVal = &m_iDayDeltaE[n][0];				//�����ܼӵ�����ֵ������Ӧ�й����޹���
		pdwTime = &m_dwDayDeltaESec[n];				//���յ�����ʱ��
		pwStartId = &g_wGrpDayStartEnID[n];			//�ܼ��������ʾֵ
		pwStartTime =  &m_dwDayStartEnSec[n];		//�ܼ��������ʾֵ�ĸ���ʱ��(���й����޹�)
		pwLeftDeltaId = &g_wGrpDayLeftDeltaID[n];	//��������ǰʣ�µ���delta����
		pwDeltaId = &g_wGrpDayDeltaID[n];			//�����ۼ�

	}		
	else if (bInterv == INTVMON) //���ۼ�
	{		
		piResVal = &m_iMonDeltaE[n][0];		
		pdwTime = &m_dwMonDeltaESec[n];
		pwStartId = &g_wGrpMonStartEnID[n];
		pwStartTime =  &m_dwMonStartEnSec[n];
		pwLeftDeltaId = &g_wGrpMonLeftDeltaID[n];
		pwDeltaId = &g_wGrpMonDeltaID[n];
	}	

	//���㵱ǰʾֵ����ʽΪ������ʾֵ��ʽ
	CalcuEnSum(iCurVal, n, INTVCAL, isNow, GRP_CUR_E);	
	if ( IsAllAVal64(&iCurVal[1], INVALID_VAL64, TOTAL_RATE_NUM) )
		return; //��ǰֵ��Ч�򲻸����ۼ�ֵ����	

	//���ʱ�겻���򷵻�
	if (ReadItemVal64(BN18, m_wPn, *pwStartId, iStartVal, isStart.dwS, isStart.dwEndS) <= 0)
		return; //���ʱ�겻���򷵻�
	else if ( IsAllAVal64(&iStartVal[1], INVALID_VAL64, TOTAL_RATE_NUM) )
		return; //���Ϊ��Ч�����򲻸���
	else if ((iRv=IsMtrEngDec(n, bInterv, isNow)) > 0)
	{
		//ʾ���½����쳣���,��Ҫ�������õ�ǰֵΪ��㣬��ǰ�ۼ����ݲ�����
		TTime now;
		SecondsToTime(isNow.dwS, &now);							
		CalcuEnSum(iGrpE, n, bInterv, isNow, GRP_CUR_E);	//���㵱ǰʾֵ���ܼ���ʾֵ���	
		if ( memcmp((BYTE*)&iStartVal[1], (BYTE*)&iGrpE[1], sizeof(int64)*TOTAL_RATE_NUM) !=0 ) //���ⷴ������
		{
			SetStartDeltaE(bInterv, *pwDeltaId, now);	
			WriteItemVal64(BN18, m_wPn, *pwStartId, iGrpE, isStart.dwS); //д�����ʾֵ		 
			*pwStartTime = isStart.dwS;						//���¼�¼ʱ��		
			DTRACE(DB_DP, ("CDpGrp::DeltaENew***** sub is error: Grp=%d, WID=0X%2x, get new iStartVal=%lld, dwSec=%d,\r\n", m_wPn, *pwStartId, iGrpE[1], isNow.dwS));

			TrigerSaveBank(BN18, 0, -1); //��ʱ������������
		}
		return;
	}
	else if (iRv < 0) //��������һ����ʾֵ�ľ������������ζ�Ÿ��ϵ��������������©��ʾ���½������б�������	
	{
		DTRACE(DB_DP, ("CDpGrp::DeltaENew####1, Grp=%d wResultID=0X%2x mPn iLastCurVal is not arrive!,\r\n", m_wPn, wResultID));
		if (ReadItemVal64(BN0, m_wPn, wResultID, iGrpE, isStart.dwS, isStart.dwEndS)<=0 || IsAllAVal64(&iGrpE[1], INVALID_VAL64, TOTAL_RATE_NUM))
		{
			SetArrVal64(iGrpE, 0, BLOCK_ITEMNUM); //ûд����ֵ0
			iGrpE[0] = RATE_NUM;
			WriteItemVal64(BN0, m_wPn, wResultID, iGrpE, isDelta.dwS); //�����ۼƵ���	
		}
		return;
	}
	else	
		fUpd = true;	
	
	BYTE bAddDeltaE = 0xff;
	ReadItemEx(BN10, 0, 0xa143, &bAddDeltaE);
	//����������Ҫ����ԭ���ۼ�ֵ�����ۼ�ֵ�����Ч���ۼ�;
	if (bAddDeltaE==0 && ReadItemVal64(BN18, m_wPn, *pwLeftDeltaId, iLeftDeltaE, isStart.dwS, isStart.dwEndS) > 0)
	{
		fAdd = true;
	}

	int iRet = 0;
	if (fUpd)
	{
		iGrpE[0] = RATE_NUM;
		//�����������֮��ĸ���ĵ��ܣ����ܼ�4�����ʣ�
		for (BYTE i=1; i<BLOCK_ITEMNUM; i++) //�ܼ��ַ��ʵ�ѭ��
		{		
			//���ǵ����ʲ����ʱ��						
			if (iGrpE[i] == INVALID_VAL64)
				continue;

			//����deltaֵ,��ʽΪ������ʾֵ��ʽ
			iRet = DataDelta(&iGrpE[i], iCurVal[i], iStartVal[i]);
			if (iRet == 0) //����Ч����
			{							  //��ǰֵ - ��ʼֵ
				iGrpE[i] = INVALID_VAL64;
				continue;
			}	
			else if (iRet < 0) 	//�ܼ���֧��Сֵ����ֵ		
			{		
				//���ǵ����ʲ����ʱ��
				if (iCurVal[i]==INVALID_VAL64 || iStartVal[i]==INVALID_VAL64)
				{
					iGrpE[i] = INVALID_VAL64;
					continue;
				}
				else
					iGrpE[i] = iCurVal[i]-iStartVal[i];				
			}			
			
			//����ԭʼ�ۼ�ֵ
			if ( fAdd && iLeftDeltaE[i] != INVALID_VAL64)		
			{
				DataSum(&iGrpE[i], iGrpE[i], iLeftDeltaE[i], 0);
			}

			if (iGrpE[i] < 0)
				DTRACE(DB_DP, ("CDpGrp::DeltaENew####2 sub is error: Grp=%d, WID=0X%2x, iGrpE[i]=%lld, iCurVal[i]=%lld, iStartVal[i]=%lld, iLeftDeltaE[i]=%lld, dwSec=%d,\r\n", m_wPn, wResultID, iGrpE[i],iCurVal[i],iStartVal[i],iLeftDeltaE[i],isNow.dwS));
		}			
	}

	//ֻҪ��һ��������Ч������������Ч	
	memcpy((BYTE*)piResVal, (BYTE*)iGrpE, sizeof(iGrpE));
	if (wResultID>=0x2306 && wResultID<=0x2309)
	{
		BYTE bBuf[64];
		bBuf[0] = DT_ARRAY;
		bBuf[1] = RATE_NUM+1;
		for (int i=0; i<BLOCK_ITEMNUM-1;i++)
		{
			bBuf[2+9*i] = DT_LONG64;
			OoInt64ToLong64(iGrpE[i+1], &bBuf[3+9*i]);
		}
		WriteItemEx(BN0, m_wPn, wResultID, bBuf, isDelta.dwS);
	}
	else
		WriteItemVal64(BN0, m_wPn, wResultID, iGrpE, isDelta.dwS);	
	*pdwTime = isDelta.dwS;	
}

//����������ʾֵ���ݣ���ʽͬ���������ݵĸ�ʽ
//		ֻҪ��һ����������Ч���ܼ�ʾֵΪ��Ч����
//@pVal64	����ʾֵ�����²��ɹ��򷵻���Ч����
//@n		
//@bType	
//@fFlg	 =0 ��ʾ�����ܼ�������ʾֵ =1 ��ʾ�����ܼ��鵱ǰ��ʾֵ
void CDpGrp::CalcuEnSum(int64* pVal64, BYTE n, BYTE bInterv, TIntvSec isNow, BYTE bEnType)
{
	int i,j;
	BYTE bBuf[64];
	int64 iTmpBuf[PN_NUM][BLOCK_ITEMNUM];	
	
	int64 iGrpE[BLOCK_ITEMNUM];	
	
	TGrpInf*   pGrpInf;
	TGrpFrzStartData* pStartData;	
	
	SetArrVal64(iGrpE, 0, BLOCK_ITEMNUM);
	iGrpE[0] = RATE_NUM; //��һ��Ϊ������

	if (n == 0)						
		pGrpInf = m_GrpInfEp;			
	else				
		pGrpInf = m_GrpInfEq;	

	if (bEnType == GRP_CUR_E) //�ܼ��鵱ǰֵ��ʾֵ
	{		
		//���в���������ݾ����¹���������ɹ�������ʧ���Լ���֧�ֵ����ݣ�	
		for (i=0; i<m_bMtrNum; i++)
		{		
			if (ReadItemEx(BN0, pGrpInf[i].bPn, pGrpInf[i].wCurId, bBuf, isNow.dwS, isNow.dwEndS) > 0 )//��Ч����
			{
				//��������Ч���������
				for (j=1; j<BLOCK_ITEMNUM; j++) //�����������֮��ĸ���ĵ��ܣ����ܼ�4�����ʣ�
				{
					iTmpBuf[i][j] = OoLong64ToInt64(&bBuf[3+j*9]);
					if (iTmpBuf[i][j] != INVALID_VAL64)
					{
						iTmpBuf[i][j] *=  m_iCT[i]*m_iPT[i];
						DataSum(&iGrpE[j], iGrpE[j], iTmpBuf[i][j], pGrpInf[i].bOp);							
					}
				}			
			}	
			else //������ȫ����Ч��������ʧ�ܻ�֧�ֵ����ݣ�����������Ϊ��Ч����
			{
				SetArrVal64(&iGrpE[1], INVALID_VAL64, TOTAL_RATE_NUM);
				break;
			}
		}
	}		
	else //����ʾֵ
	{
		if (bInterv == INTVDAY)
			pStartData = &m_gfsdDayStart[n];
		else if (bInterv == INTVMON)
			pStartData = &m_gfsdMonStart[n];

		for (i=0; i<m_bMtrNum; i++)
		{		
			//��������Ч���������
			if ( !IsAllAVal64(&pStartData->iStartVal[i][1], INVALID_VAL64, TOTAL_RATE_NUM) )
			{
				memcpy((BYTE*)iTmpBuf, (BYTE*)pStartData->iStartVal, sizeof(iTmpBuf));

				for (j=1; j<BLOCK_ITEMNUM; j++) //�����������֮��ĸ���ĵ��ܣ����ܼ�4�����ʣ�
				{
					if (iTmpBuf[i][j] != INVALID_VAL64)
					{
						if (bEnType == GRP_START_E)//�ܼ�������ʾֵ
						{
							iTmpBuf[i][j] *=  m_iCT[i]*m_iPT[i];
						}
						else //if (bEnType == MTR_START_E)
						{				
							//iTmpBuf[i][j] *= 1;
							//���ܼ���Ĳ���������ʾֵ������CTPT,���ڼ����ܼ���Ĳ����������Ƿ���£�
							//�ܼ���ͨ���������¼�Ĳ���������ʾֵ�Ƿ��и��£���������ܼ�������ʾֵ��������ʾֵ��ͬ��ʱ������CTPT�������
							//֮�����������Ƿ����쳣�仯��������Ȼ��ʾ���½����������ܼ���ʾֵ��δ�����ܼ�������ʾֵ���£�
						}

						DataSum(&iGrpE[j], iGrpE[j], iTmpBuf[i][j], pGrpInf[i].bOp);							
					}
				}			
			}			
			else //������ȫ����Ч��������ʧ�ܻ�֧�ֵ����ݣ�����������Ϊ��Ч����
			{
				SetArrVal64(&iGrpE[1], INVALID_VAL64, TOTAL_RATE_NUM);
				break;
			}
		}
	}
	memcpy((BYTE*)pVal64, (BYTE*)iGrpE, sizeof(iGrpE));
}


//����������������ʱ���ǲ�������ʱ���ж��ܼ���Ĳ����㵱ǰ�Ƿ�ʾ���½�
int CDpGrp::IsMtrEngDec(BYTE n, BYTE bInterv, TIntvSec isNow)
{
	int i,j;
	BYTE bCurBuf[64], bLastBuf[64];
	int64 iCurVal[BLOCK_ITEMNUM];
	int64 iLastCurVal[BLOCK_ITEMNUM];
	int iLen1, iLen3;		
	
	TGrpInf*   pGrpInf;

	if (n == 0)						
		pGrpInf = m_GrpInfEp;			
	else				
		pGrpInf = m_GrpInfEq;	
	
	//���в���������ݾ����¹���������ɹ�������ʧ���Լ���֧�ֵ����ݣ�	
	for (i=0; i<m_bMtrNum; i++)
	{			
		SetArrVal64(iCurVal, INVALID_VAL64, BLOCK_ITEMNUM);
		SetArrVal64(iLastCurVal, INVALID_VAL64, BLOCK_ITEMNUM);

		iLen3 = ReadItemEx(BN0, pGrpInf[i].bPn, pGrpInf[i].wCurId, bLastBuf, isNow.dwS-m_bMtrIntv*60, isNow.dwEndS-m_bMtrIntv*60);	//ȡǰһ��������ֵ
		iLen1 = ReadItemEx(BN0, pGrpInf[i].bPn, pGrpInf[i].wCurId, bCurBuf, isNow.dwS, isNow.dwEndS);	  //ȡ��ǰ��������ֵ

		for (j=1; j<BLOCK_ITEMNUM; j++) //�����������֮��ĸ���ĵ��ܣ����ܼ�4�����ʣ�
		{
			iLastCurVal[j] = OoLong64ToInt64(&bLastBuf[3+j*9]);
			iCurVal[j] = OoLong64ToInt64(&bCurBuf[3+j*9]);
		}

		if (iLen1<=0 || IsAllAVal64(&iCurVal[1], INVALID_VAL64, TOTAL_RATE_NUM)) 
			return 0; //��ǰֵ��Ч���ж�		
		if (iLen3<=0 || IsAllAVal64(&iLastCurVal[1], INVALID_VAL64, TOTAL_RATE_NUM)) 
			return -1; //��һ��ֵ��Ч���ж�	
		
		//��������Ч������ж�
		for (j=1; j<BLOCK_ITEMNUM; j++) //�����������֮��ĸ���ĵ��ܣ����ܼ�4�����ʣ�
		{
			if ( iCurVal[j] < iLastCurVal[j])  //ֻҪ��ĳ��������ĳ���ʵ�ʾ���½�����Ϊʾ���½�		
			{
				//DTRACE(DB_DP, ("CDpGrp::IsMtrEngDec***** sub is error: Grp=%d, Mpn=%d,  WID=0X%2x, iCurVal=%lld, iLastCurVal=%lld, iResVal not updated dwSec=%d,\r\n", m_wPn, pGrpInf[i].bPn, pGrpInf[i].wCurId, iCurVal[j], iLastCurVal[j], isNow.dwS));
				return 1;	
			}
		}				
	}

	return 0;	
}

//���������������ύ�ܼ��鵱ǰˢ�����ݵ�����
//		���㵱ǰ�ܼ��С��޹������Լ��С��޹�������,�Լ����յ����ۼ�����
//����ֵ��
void CDpGrp::CalcuCurData(TTime tmNow)
{
	int i,n;
	TIntvSec isNow;	
	WORD wNum;
	BYTE bBuf[64];
	int64 iTmpBuf[PN_NUM][BLOCK_ITEMNUM];
	
	int64 iGroupP;
	int64 iGrpE[BLOCK_ITEMNUM];	

	TBankItem* pItem;
	TGrpInf*   pGrpInf;
	
	//TODO:ʹ��CMeterSched::GetCurIntervS()��ȡ��ǰ�������ʼʱ��
	TimeToIntvS(tmNow, INTVCAL, &isNow); //��Ϊ��ǰ����������ʼ����
			//���ͬʱ���ڵ������彻�ɲ�����,
			//����ʼʱ��ȡ���ǳ���������ʼʱ��,����ʵ�������彻�ɲ�����
			//ÿ��ȡ�Ļ��ǵ�ǰ����������,�����������һ�������ȡ�Ķ���
			//��ǰ�������������

	//���㵱ǰ�ܼӹ���	
	for (n=0; n<2; n++)	//�й����޹�
	{
		iGroupP = 0;
		if (n == 0)		
		{
			pItem = m_biRepP;
			pGrpInf = m_GrpInfP;
		}
		else
		{
			pItem = m_biRepQ;
			pGrpInf = m_GrpInfQ;
		}
		
		wNum = 0;
		if (QueryItemTime(isNow.dwS, isNow.dwEndS, pItem, m_bMtrNum, &wNum)==m_bMtrNum && m_dwDayStartEnSec[n]!=0)
		{
			//���в���������ݾ����¹���������ɹ�������ʧ���Լ���֧�ֵ����ݣ�
			for (i=0; i<m_bMtrNum; i++)
			{			
				if (ReadItemEx(BN0, pGrpInf[i].bPn, pGrpInf[i].wCurId, bBuf, isNow.dwS, isNow.dwEndS) > 0 )
				{	
					iTmpBuf[i][0] = OoLong64ToInt64(&bBuf[3]);
					if (iTmpBuf[i][0]!=INVALID_VAL64 && pGrpInf[i].bDir==1 && iTmpBuf[i][0]>0) //������ȡ��
						iTmpBuf[i][0] = -iTmpBuf[i][0];
					else if (iTmpBuf[i][0]!=INVALID_VAL64 && pGrpInf[i].bDir==0 && iTmpBuf[i][0]<0) //������ȡ��
						iTmpBuf[i][0] = -iTmpBuf[i][0];

					//��������Ч���������
					iTmpBuf[i][0] *= m_iCT[i]*m_iPT[i];
					DataSum(&iGroupP, iGroupP, iTmpBuf[i][0], pGrpInf[i].bOp);	
				}
				else //������ȫ����Ч��������ʧ�ܻ�֧�ֵ����ݣ�����������Ϊ��Ч����
				{
					iGroupP = INVALID_VAL64;
					break;
				}
			}
			iGroupP = INVALID_VAL64;
			bBuf[0] = DT_LONG64;
			OoInt64ToLong64(iGroupP, &bBuf[1]);
			WriteItemEx(BN0, m_wPn, g_wGrpCurPowerID[n], bBuf, isNow.dwS);
		}
	}
	
	//���㵱ǰ����ʾֵ,�Լ����յ����ۼƵ����������������������ݵ�ת��
	for (n=0; n<2; n++)	//�й����޹�
	{		
		SetArrVal64(iGrpE, 0, BLOCK_ITEMNUM);

		if (n == 0)		
		{
			pItem = m_biRepEp;
			pGrpInf = m_GrpInfEp;			
		}
		else
		{
			pItem = m_biRepEq;
			pGrpInf = m_GrpInfEq;
		}

		wNum = 0;
		if (QueryItemTime(isNow.dwS, isNow.dwEndS, pItem, m_bMtrNum, &wNum)==m_bMtrNum && m_dwDayStartEnSec[n]!=0)//��ѯ���ܱ����ڱ����¹�
		{
			CalcuEnSum(iGrpE, n, INTVCAL, isNow, GRP_CUR_E);			//���㵱ǰʾֵ
			bBuf[0] = DT_ARRAY;
			bBuf[1] = BLOCK_ITEMNUM-1;
			for (i=1; i<BLOCK_ITEMNUM; i++)
			{
				bBuf[2+i*9] = DT_LONG64;
				OoInt64ToLong64(iGrpE[i], &bBuf[3+i*9]);
			}
			if (n == 0)
			{
				WriteItemEx(BN0, m_wPn, 0x0a07, bBuf, isNow.dwS); //��ǰ�ܼ��й����ܣ�������ʹ��	
			}

			//����Delta����
			//���¸��ݲ�����ĵ�ǰֵ�������㣬��������ֱ��ȡ������Ĳ�ֵ��Ӽ������⵽��ǰ���ݵ������ֵ���п��ܻ�δ����
			//�����ۼ��ܼӵ���	
			TIntvSec isStart;			
			TimeToIntvS(tmNow, INTVDAY, &isStart);
			//DeltaE(g_wGrpDayDeltaID[n], pGrpInf, INTVDAY, isNow, isStart, isNow);
			DeltaENew(g_wGrpDayDeltaID[n], pGrpInf, INTVDAY, isNow, isStart, isNow);
			//�����ۼ��ܼӵ���
			TimeToIntvS(tmNow, INTVMON, &isStart);
			//DeltaE(g_wGrpMonDeltaID[n], pGrpInf, INTVMON, isNow, isStart, isNow);
			DeltaENew(g_wGrpMonDeltaID[n], pGrpInf, INTVMON, isNow, isStart, isNow);
		}
	}
}

// �����������յ��µ��ۼƵ��ܲ�ֵ����ת�浽��һ�ղ������յ��µ��ۼƵ�������
//@bIdx		Ҫת��ĵ�������������Ӧ���С����ޡ����С����ޣ�
//@bType	��ת�滹����ת��ı�ʶ
//@dwSec	Ҫת���ʱ��
void CDpGrp::UpdClsFrzData(BYTE bIdx, BYTE bInterv, DWORD dwOldS, DWORD dwNewS)
{
	BYTE bBuf[64];
	TTime tm1,tm2;
	SecondsToTime(dwOldS, &tm1);
	SecondsToTime(m_dwDayDeltaESec[bIdx], &tm2);
	int64 iValBuf[BLOCK_ITEMNUM];

	if (bInterv == INTVDAY)
	{
		ClsBlockE(BN0, g_wGrpDayDeltaID[bIdx], &m_iDayDeltaE[bIdx][0], 0, dwNewS); //��������		
		DTRACE(DB_DP, ("CDpGrp::UpdClsFrzData DAY GPN=%d g_wGrpDayDeltaID=%x Val=%lld, dwSec=%d!\n",m_wPn,g_wGrpDayDeltaID[bIdx],m_iDayDeltaE[bIdx][1],dwNewS));

		ClsBlockE(BN18, g_wGrpDayLeftDeltaID[bIdx], iValBuf, 0, dwNewS); //��������ۼ�����	
		ClsBlockE(BN18, g_wGrpDayStartEnID[bIdx], iValBuf, INVALID_VAL64, dwNewS); //�������ʾֵ����	
	}
	else if (bInterv == INTVMON)
	{
		ClsBlockE(BN0, g_wGrpMonDeltaID[bIdx], &m_iMonDeltaE[bIdx][0], 0, dwNewS); //��������		
		DTRACE(DB_DP, ("CDpGrp::UpdClsFrzData MON GPN=%d g_wGrpDayDeltaID=%x Val=%lld, dwSec=%d!\n",m_wPn,g_wGrpMonDeltaID[bIdx],m_iMonDeltaE[bIdx][1],dwNewS));

		ClsBlockE(BN18, g_wGrpMonLeftDeltaID[bIdx], iValBuf, 0, dwNewS); //��������ۼ�����	
		ClsBlockE(BN18, g_wGrpMonStartEnID[bIdx], iValBuf, INVALID_VAL64, dwNewS); //�������ʾֵ����
	}
}


//������׼�����л�
//��ע����ҪΪ����ͳ�������Լ����ܼ��������������޵��ʱ��׼���л�
void CDpGrp::DayChange(TTime tmNow)
{		
	TIntvSec  isNow;
	DWORD dwCurS, dwCurEndS;
	int64 iValBuf[BLOCK_ITEMNUM];
	BYTE i, j;
	TTime time;

	GetCurTime(&time);
	TimeToIntvS(tmNow, INTVDAY, &isNow);		

	//�ն����������ת��
	if (m_bPnProp == 0)//�޵�������
	{		
		for (i=0; i<2; i++)
		{
			//ת�����ռ��嵱�� ���ڵ����ղſ���д��Ч�ۼ�����			
			UpdClsFrzData(i, INTVDAY, isNow.dwS, isNow.dwS);			
			DTRACE(DB_DP, ("CDpGrp::DayChange()1:PN=%d,DayDeltaID=%x \n ",m_wPn, g_wGrpDayDeltaID[i]));
		}		

		if (m_bPnProp != PN_PROP_METER)
		{
			dwCurS = isNow.dwS;
			dwCurEndS = isNow.dwEndS;
		}

		for (i=0; i<sizeof(g_wDayStartID)/sizeof(WORD); i++)
		{	
			for (j=0; j<m_bMtrNum; j++)
			{
				SetArrVal64(iValBuf, INVALID_VAL64, BLOCK_ITEMNUM);
				iValBuf[0] = m_bRateNum; //������

				//��������� ��Ч��Ч������				
				ReadItemVal64(BN0, m_GrpInfEp[j].bPn, g_wCurID[i], &iValBuf[1], dwCurS, dwCurEndS);		
				WriteItemVal64(BN18, m_GrpInfEp[j].bPn, g_wDayStartID[i], iValBuf, isNow.dwS);
				DTRACE(DB_DP, ("CDpMtr::DayChange():PN=%d,g_wDayStartID=%x StartVal=%lld\n ",m_GrpInfEp[j].bPn, g_wDayStartID[i], iValBuf[1]));		
			}
		}
	}	

	TrigerSaveBank(BN18, 0, -1); //����ͳ�����ֵ��������.
}

//������׼�����л�
//��ע����ҪΪ����ͳ�������Լ����ܼ��������������޵��ʱ��׼���л�
void CDpGrp::MonChange(TTime tmNow)
{
	TIntvSec  isNow;
	DWORD dwCurS, dwCurEndS;
	int64 iValBuf[BLOCK_ITEMNUM];
	BYTE i, j;
	TTime time;

	GetCurTime(&time);
	TimeToIntvS(tmNow, INTVMON, &isNow);

	//�¶����������ת��
	if (m_bPnProp == 0)//�޵�������
	{
		for (BYTE i=0; i<2; i++)
		{
			//ת�����¼��嵱�� 	
			UpdClsFrzData(i, INTVMON, isNow.dwS, isNow.dwS);	
			DTRACE(DB_DP, ("CDpGrp::DayChange()1:PN=%d,DayDeltaID=%x \n ",m_wPn, g_wGrpMonDeltaID[i]));
		}	

		if (m_bPnProp != PN_PROP_METER)
		{
			dwCurS = isNow.dwS;
			dwCurEndS = isNow.dwEndS;
		}

		for (i=0; i<sizeof(g_wMonStartID)/sizeof(WORD); i++)
		{	
			for (j=0; j<m_bMtrNum; j++)
			{
				SetArrVal64(iValBuf, INVALID_VAL64, BLOCK_ITEMNUM);
				iValBuf[0] = m_bRateNum; //������

				//��������� ��Ч��Ч������				
				ReadItemVal64(BN0, m_GrpInfEp[j].bPn, g_wCurID[i], &iValBuf[1], dwCurS, dwCurEndS);		
				WriteItemVal64(BN18, m_GrpInfEp[j].bPn, g_wMonStartID[i], iValBuf, isNow.dwS);
				DTRACE(DB_DP, ("CDpMtr::DayChange():PN=%d,g_wDayStartID=%x StartVal=%lld\n ",m_GrpInfEp[j].bPn, g_wMonStartID[i], iValBuf[1]));		
			}
		}
	}

	TrigerSaveBank(BN18, 0, -1); //����ͳ�����ֵ��������.
}                    
                
//������������Ⱥ���
void CDpGrp::DoDataProc()
{
	if ( IsNeedReset() )
	{
		Init(m_wPn);
	}

	TTime now; 
	GetCurTime(&now);

	if (now.nDay!=m_tmLast.nDay || now.nMonth!=m_tmLast.nMonth || now.nYear!=m_tmLast.nYear)
	{
		DayChange(now);		
	}	
	if (now.nMonth!=m_tmLast.nMonth || now.nYear!=m_tmLast.nYear)
	{
		MonChange(now);		
	}	
	m_tmLast = now;

	UpdDayMonStartVal(INTVDAY, now);
	UpdDayMonStartVal(INTVMON, now);	

	CalcuCurData(now);

} 


//��������¼�ܼ������µ���������ۼ�ֵ,��ʽFMT3
void CDpGrp::SetStartDeltaE(BYTE bInterv, WORD wDeltaEId, TTime& tm)
{	
	BYTE n = 0;
	BYTE bBuf[64];
	TIntvSec isNow;
	int64 iValBuf[BLOCK_ITEMNUM];
	memset((BYTE*)iValBuf, 0, sizeof(iValBuf));

	TimeToIntvS(tm, bInterv, &isNow);

	if (wDeltaEId==g_wGrpDayDeltaID[0] || wDeltaEId==g_wGrpMonDeltaID[0]) //�й�		
		n = 0;		
	else if (wDeltaEId==g_wGrpDayDeltaID[1] || wDeltaEId==g_wGrpMonDeltaID[1])//�޹�
		n = 1;		

	if (bInterv == INTVDAY) //��������д����Ч���ݣ�������ʱд��0
	{		
		ReadItemEx(BN0, m_wPn, g_wGrpDayDeltaID[n], bBuf, isNow.dwS, isNow.dwEndS);
		WriteItemEx(BN18, m_wPn, g_wGrpDayLeftDeltaID[n], bBuf, isNow.dwS);
		DTRACE(DB_DP, ("CDpGrp::SetStartDeltaE m_wPn=%d wStartDeltaEnVal=%lld dwSec=%d \n",m_wPn,iValBuf[1], isNow.dwS));	
	}
	else if (bInterv == INTVMON)
	{
		ReadItemEx(BN0, m_wPn, g_wGrpMonDeltaID[n], bBuf, isNow.dwS, isNow.dwEndS);
		WriteItemEx(BN18, m_wPn, g_wGrpMonLeftDeltaID[n], bBuf, isNow.dwS);
	}
}

//�����������Ƿ���������ʾֵ
void CDpGrp::SetNewStartEnFlg(bool fFlg, DWORD dwNewTime)
{
	m_fNewDayStartEnFlg[0] = fFlg;
	m_fNewDayStartEnFlg[1] = fFlg;
	m_fNewMonStartEnFlg[0] = fFlg;
	m_fNewMonStartEnFlg[1] = fFlg;

	m_dwEnNewStartSec = dwNewTime;

	BYTE bBuf[64];
	int64 iGroupP;
	int64 iGrpE[BLOCK_ITEMNUM];
	TTime now;
	TIntvSec isNow;

	SecondsToTime(dwNewTime, &now);
	TimeToIntvS(now, INTVCAL, &isNow);	

	//��ǰ�ܼӹ��ʣ��ܼ�ʾֵ������Ϊ��Ч�����������ٸ���
	TimeToIntvS(now, INTVCAL, &isNow);
	for (BYTE i=0; i<2; i++)
	{
		iGroupP = INVALID_VAL64;
		iGroupP = INVALID_VAL64;
		bBuf[0] = DT_LONG64;
		OoInt64ToLong64(iGroupP, &bBuf[1]);
		WriteItemEx(BN0, m_wPn, g_wGrpCurPowerID[i], bBuf, isNow.dwS);//��ǰ�ܼӹ���	
	}
	bBuf[0] = DT_ARRAY;
	bBuf[1] = 1;
	bBuf[2] = DT_LONG64;
	OoInt64ToLong64(iGrpE[1], &bBuf[3]);
	WriteItemEx(BN0, m_wPn, 0x0a07, bBuf, isNow.dwS); //��ǰ�ܼ��й����ܣ�������ʹ��			
}

extern bool g_fGrpParaChg;
void SetGrpParaChg(bool fFlg)
{
	g_fGrpParaChg = fFlg;
	DTRACE(DB_METER, ("SetGrpParaChg: para chg = %d!\r\n", g_fGrpParaChg));
}

bool IsGrpParaChg()
{
	return g_fGrpParaChg;
}

extern bool g_fCtrlGrpParaChg;
void SetCtrlGrpParaChg(bool fFlg)
{
	g_fCtrlGrpParaChg = fFlg;
	DTRACE(DB_METER, ("SetCtrlGrpParaChg: para chg = %d!\r\n", g_fCtrlGrpParaChg));
}

bool IsCtrlGrpParaChg()
{
	return g_fCtrlGrpParaChg;
}

CDpGrp *g_GroupPn[GB_MAXSUMGROUP];
//��������
void UpdGrpDataProcess(bool fPowerUp)
{
	TTime now;
	GetCurTime(&now);
	bool fNewStartFlg = false;

	 //���ϵ��һ�Σ���һ��Ϊ��������(�������޸�),��Ҫ�����ܼ������
	if ( !fPowerUp )
	{
		fNewStartFlg = true;
	}

	DTRACE(DB_DP, ("UpdGrpDataProcess()! \r\n"));

	for (BYTE i=1; i<GB_MAXSUMGROUP; i++)
	{	
		if ( IsGrpValid(i) )//�ܼ�����Ч
		{		
			if (g_GroupPn[i] == NULL)//����
			{
				g_GroupPn[i] = new CDpGrp;
			}
			else 
			{
				//֮ǰ���е��ܼ�����������Ҫ����ԭ�ۼ�ֵ����Ϊ�µ��ۼ�ֵ���
				//��ֵ���Ҫ���º���ܼ�
				for (BYTE j=0; j<2; j++)
				{
					g_GroupPn[i]->SetStartDeltaE(INTVDAY, g_wGrpDayDeltaID[j], now);					
					g_GroupPn[i]->SetStartDeltaE(INTVMON, g_wGrpMonDeltaID[j], now);							
				}
				//fNewStartFlg = true;
			}

			if ( !g_GroupPn[i]->Init(i) )//���³�ʼ��			
			{
				delete g_GroupPn[i];
				g_GroupPn[i] = NULL;
				//��Ҫ�����Ӧ�ܼ��������
				ClrGrpPnData(i);
			}
			else
			{
				if ( fNewStartFlg )
					g_GroupPn[i]->SetNewStartEnFlg(true, TimeToSeconds(now)); 
			}			
		}
		else	//��Ч
		{
			if (g_GroupPn[i] != NULL)//��Ч����Ч
			{
				delete g_GroupPn[i];			
				g_GroupPn[i] = NULL;
				//��Ҫ�����Ӧ�ܼ��������
				ClrGrpPnData(i);
			}					
		}
	}
}                  

//�����߳�
void RunGrpDataProcess()
{
	//��������б������ֹͣ����
	if (/*IsMtrParaChg() ||*/ IsGrpParaChg()) 
		return;

	for (int i=1; i<GB_MAXSUMGROUP; i++)
	{		
		if (g_GroupPn[i] != NULL)//����
		{
				g_GroupPn[i]->DoDataProc();
		}
	}
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
 
//��ȡ�ܼ��鵱ǰ����״̬
bool GetGrpCurCtrlSta(int iGrp, TGrpCurCtrlSta *pGrpCurCtrlSta)
{
	BYTE bBuf[32] = {0};
	BYTE *ptr = bBuf;
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;

	if (OoReadAttr(0x2300+iGrp, 0x11, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("GetGrpCurCtrlSta: There is something wrong when call OoReadAttr() !\n"));
		return false;
	}

	ptr += 2;	//�ṹ����Ա����
	ptr++;		//DT_LONG64
	pGrpCurCtrlSta->CurPwrVal = OoLong64ToInt64(ptr);	//��ǰ���ض�ֵ
	ptr += 8;	
	ptr++;		//DT_INT
	//memcpy(pGrpCurCtrlSta->FloatRate, ptr, 4);			//��ǰ�����¸��ظ���ϵ��
	pGrpCurCtrlSta->FloatRate = OoDoubleLongToInt(ptr);
	ptr += 4;
	ptr += 2;	//DT_BIT_STR���ֽ���
	pGrpCurCtrlSta->bAllPwrCtrlOutPutSta = *ptr;		//������բ���״̬
	ptr++;
	ptr += 2;	//DT_BIT_STR���ֽ���
	pGrpCurCtrlSta->bMonthCtrlOutPutSta = *ptr;			//�µ����բ���״̬
	ptr++;
	ptr += 2;	//DT_BIT_STR���ֽ���
	pGrpCurCtrlSta->bBuyCtrlOutPutSta = *ptr;			//�������բ���״̬
	ptr++;
	ptr += 2;	//DT_BIT_STR���ֽ���
	pGrpCurCtrlSta->bPCAlarmState = *ptr;				//����Խ�޸澯״̬
	ptr++;
	ptr += 2;	//DT_BIT_STR���ֽ���
	pGrpCurCtrlSta->bECAlarmState = *ptr;				//���Խ�޸澯״̬
	ptr++;

	return true;
}

//�����ܼ��鵱ǰ����״̬
bool SetGrpCurCtrlSta(int iGrp, TGrpCurCtrlSta *pGrpCurCtrlSta)
{
	BYTE bBuf[32] = {0};
	BYTE *ptr = bBuf;

	if (pGrpCurCtrlSta != NULL)
	{
		*ptr++ = DT_STRUCT;
		*ptr++ = 7;							//�ṹ��Ա����
		*ptr++ = DT_LONG64;					//��ǰ���ض�ֵ
		OoInt64ToLong64(pGrpCurCtrlSta->CurPwrVal, ptr);
		ptr += 8;
		*ptr++ = DT_INT;					//��ǰ�����¸��ظ���ϵ��
		OoIntToDoubleLong(pGrpCurCtrlSta->FloatRate, ptr);
		ptr += 4;
		*ptr++ = DT_BIT_STR;				//������բ���״̬	
		*ptr++ = 1;
		*ptr++ = pGrpCurCtrlSta->bAllPwrCtrlOutPutSta;
		*ptr++ = DT_BIT_STR;				//�µ����բ���״̬
		*ptr++ = 1;
		*ptr++ = pGrpCurCtrlSta->bMonthCtrlOutPutSta;
		*ptr++ = DT_BIT_STR;				//�������բ���״̬
		*ptr++ = 1;
		*ptr++ = pGrpCurCtrlSta->bBuyCtrlOutPutSta;
		*ptr++ = DT_BIT_STR;				//����Խ�޸澯״̬
		*ptr++ = 1;
		*ptr++ = pGrpCurCtrlSta->bPCAlarmState;
		*ptr++ = DT_BIT_STR;				//���Խ�޸澯״̬
		*ptr++ = 1;
		*ptr++ = pGrpCurCtrlSta->bECAlarmState;

		if (OoWriteAttr(0x2300+iGrp, 0x11, bBuf) < 0)
		{
			DTRACE(DB_LOADCTRL, ("SetGrpCurCtrlSta: There is something wrong when call OoWriteAttr() !\n"));
			return false;
		}

		return true;
	}

	return false;
}

//��ȡ�ܼ����������״̬
bool GetGrpCtrlSetSta(int iGrp,	TGrpCtrlSetSta *pGrpCtrlSetSta)
{
	BYTE bBuf[20] = {0};
	BYTE *ptr = bBuf;
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;

	if (OoReadAttr(0x2300+iGrp, 0x10, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("GetGrpCtrlSetSta: There is something wrong when call OoReadAttr() !\n"));
		return false;
	}

	ptr += 2;		//�ṹ����Ա����
	ptr++;			//DT_UNSIGN
	pGrpCtrlSetSta->bSchemeNum = *ptr;		//ʱ�οض�ֵ������
	ptr++;
	ptr += 2;			//DT_BIT_STR���ֽڸ���
	pGrpCtrlSetSta->bValidFlag = *ptr;		//����ʱ����Ч��־λ
	ptr++;
	ptr += 2;			//DT_BIT_STR���ֽڸ���
	pGrpCtrlSetSta->bPwrCtrlSta = *ptr;		//����״̬
	ptr++;
	ptr += 2;			//DT_BIT_STR���ֽڸ���
	pGrpCtrlSetSta->bEngCtrlSta = *ptr;		//���״̬
	ptr++;
	ptr += 2;			//DT_BIT_STR���ֽڸ���
	pGrpCtrlSetSta->bPwrCtrlTurnSta = *ptr;	//�����ִ�״̬
	ptr++;
	ptr += 2;			//DT_BIT_STR���ֽڸ���
	pGrpCtrlSetSta->bEngCtrlTurnSta = *ptr;	//����ִ�״̬
	ptr++;

	return true;
}

//�����ܼ����������״̬
bool SetGrpCtrlSetSta(int iGrp,	TGrpCtrlSetSta *pGrpCtrlSetSta)
{
	BYTE bBuf[20] = {0};
	BYTE *ptr = bBuf;
	
	if (pGrpCtrlSetSta != NULL)
	{
		*ptr++ = DT_STRUCT;
		*ptr++ = 6;					//�ṹ��Ա����
		*ptr++ = DT_UNSIGN;			
		*ptr++ = pGrpCtrlSetSta->bSchemeNum;		//ʱ�οض�ֵ������
		*ptr++ = DT_BIT_STR;						
		*ptr++ = 1;
		*ptr++ = pGrpCtrlSetSta->bValidFlag;		//����ʱ����Ч��־λ
		*ptr++ = DT_BIT_STR;
		*ptr++ = 1;
		*ptr++ = pGrpCtrlSetSta->bPwrCtrlSta;		//����״̬
		*ptr++ = DT_BIT_STR;
		*ptr++ = 1;
		*ptr++ = pGrpCtrlSetSta->bEngCtrlSta;		//���״̬
		*ptr++ = DT_BIT_STR;
		*ptr++ = 1;
		*ptr++ = pGrpCtrlSetSta->bPwrCtrlTurnSta;	//�����ִ�״̬
		*ptr++ = DT_BIT_STR;
		*ptr++ = 1;
		*ptr++ = pGrpCtrlSetSta->bEngCtrlTurnSta;	//����ִ�״̬
		
		if (OoWriteAttr(0x2300+iGrp, 0x10, bBuf) < 0)
		{
			DTRACE(DB_LOADCTRL, ("SetGrpCtrlSetSta: There is something wrong when call OoWriteAttr() !\n"));
			return false;
		}

		return true;
	}

	return false;
}