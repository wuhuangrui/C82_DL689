/***********************************************************************************************
* Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
* 
* �ļ����ƣ�DpGrp.h
* ժ    Ҫ: ���ļ��ṩ�ܼ���������ݵ�ʵ��
* ��ǰ�汾��1.0
* ��    �ߣ�������
* ������ڣ�2008��3��
* ��    ע��
***********************************************************************************************/
#ifndef   DPGRP_INCLUDED
#define   DPGRP_INCLUDED

#include "DataProc.h"
#include "DbConst.h"
#include "LibDbStruct.h"

typedef struct{		
	TBankItem biRepItem[PN_NUM];				//��������ʼID
	int64	  iStartVal[PN_NUM][BLOCK_ITEMNUM];	//��������ʼ����	
	DWORD	  dwUpdSec;							//�������ݵĸ���ʱ��	
}TGrpFrzStartData;

typedef struct{
	BYTE bPn;		//�������	
	BYTE bProp;		//�����������	
	BYTE bOp;		//�����
	BYTE bDir;		//�����
	WORD wCurId;		//����ID
	WORD wDayStartId;	//���㵱�ղ�ֵʱ��Ӧ�����ID
	WORD wMonStartId;	//���㵱�²�ֵʱ��Ӧ�����ID	
}TGrpInf;

//�ܼ��鵱ǰ����״̬
typedef struct{
	int64 CurPwrVal;	//��ǰ���ض�ֵ
	int FloatRate;		//��ǰ�����¸��ظ���ϵ��
	BYTE bAllPwrCtrlOutPutSta;	//������բ���״̬
	BYTE bMonthCtrlOutPutSta;	//�µ����բ���״̬
	BYTE bBuyCtrlOutPutSta;	//�������բ���״̬
	BYTE bPCAlarmState;	//����Խ�޸澯״̬ 
	BYTE bECAlarmState;	//���Խ�޸澯״̬
}TGrpCurCtrlSta;

//�ܼ����������״̬
typedef struct{
	BYTE bSchemeNum;	//ʱ�οض�ֵ������
	BYTE bValidFlag;	//����ʱ����Ч��־λ
	BYTE bPwrCtrlSta;	//����״̬
	BYTE bEngCtrlSta;	//���״̬
	BYTE bPwrCtrlTurnSta;	//�����ִ�״̬
	BYTE bEngCtrlTurnSta;	//����ִ�״̬
}TGrpCtrlSetSta;

//�ܼ������
class CDpGrp:public CDataProc
{
public:
	CDpGrp(void);
	virtual ~CDpGrp(void);

	bool Init(WORD  wPn);	
	void DoDataProc();

	void LoadData();
	void LoadPara();	
	bool IsNeedReset();	
	void SetStartDeltaE(BYTE bInterv, WORD wDeltaEId, TTime& tm); 
	//void SetStartCurEn(BYTE bType, WORD wCurEnId, TTime& tm);
	void SetNewStartEnFlg(bool fFlg, DWORD dwNewTime);
	
private:		
	void CalcuCurData(TTime tmNow);
	void PowerStat(TTime tmNow);

	void DayChange(TTime tmNow);
	void MonChange(TTime tmNow);	
	void UpdDayMonStartVal(BYTE bInterv, TTime tmNow);
	void UpdClsFrzData(BYTE bIdx, BYTE bInterv, DWORD dwOldS, DWORD dwNewS);
	void DeltaENew(WORD wResultID, TGrpInf* pGrpInf, BYTE bInterv, TIntvSec isNow, TIntvSec isStart, TIntvSec isDelta);	
	void CalcuEnSum(int64* pVal64, BYTE n, BYTE bInterv, TIntvSec isNow, BYTE bEnType);
	//����������������ʱ���ǲ�������ʱ���ж��ܼ���Ĳ����㵱ǰ�Ƿ�ʾ���½�
	int IsMtrEngDec(BYTE n, BYTE bInterv, TIntvSec isNow);	

	bool InitPara(BYTE& bReqNum);	
	int  GetIdInxE(WORD wCurID);

	TTime m_tmLast;	//���ڼ�¼�ж�׼���ʱ���л�

	BYTE  m_bMtrNum; //����������

	//������Ϣ
	int m_iCT[PN_NUM];			//�ܼ������ز������CT����,��Ӧ�ܼ���Ĳ�����������Ŷ����ǲ������
	int m_iPT[PN_NUM];			//�ܼ������ز������PT����
	
	TGrpInf m_GrpInfP[PN_NUM];	//�ܼ��й����ʵ���ز�������Ϣ,��Ӧ�ܼ���Ĳ�����������Ŷ����ǲ������
	TGrpInf m_GrpInfQ[PN_NUM];	//�ܼ��޹����ʵ���ز�������Ϣ
	TGrpInf m_GrpInfEp[PN_NUM];	//�ܼ��й����ܵ���ز�������Ϣ
	TGrpInf m_GrpInfEq[PN_NUM];	//�ܼ��޹����ܵ���ز�������Ϣ

	//�ܼ����������Ҫ�ύ��ID
	TBankItem m_biRepP[PN_NUM]; //�ܼ��й�����
	TBankItem m_biRepQ[PN_NUM]; //�ܼ��޹�����
	TBankItem m_biRepEp[PN_NUM]; //�ܼ��й������������ʣ�
	TBankItem m_biRepEq[PN_NUM]; //�ܼ��޹������������ʣ�

	//�ۼ�����
	int64 m_iDayDeltaE[2][BLOCK_ITEMNUM]; //�����ܼӵ�����ֵ������Ӧ�й����޹���
	DWORD m_dwDayDeltaESec[2];	//���յ�����ʱ��
	int64 m_iMonDeltaE[2][BLOCK_ITEMNUM]; //�����ܼӵ�����ֵ������Ӧ�й����޹���	
	DWORD m_dwMonDeltaESec[2];	//���µ�����ʱ��

	//�ܼ��黺��Ĳ������������ʼ���ݣ��Խ������������ݸ��µĲ�ͬ������
	TGrpFrzStartData m_gfsdDayStart[2]; //����������ʼ���������й����޹���
	TGrpFrzStartData m_gfsdMonStart[2]; //����������ʼ���������й����޹���

	int64 m_iOldDayMtrE[2][BLOCK_ITEMNUM];	//����������ʼʾֵ�����й����޹�������CTPT,���ڼ����ܼ���Ĳ��������ʼֵ�Ƿ��б仯��	
	DWORD m_dwOldDayMtrUpdSec[2];			//����������ʼʾֵ��Ӧ��ʱ�꣨���й����޹���
	int64 m_iOldMonMtrE[2][BLOCK_ITEMNUM];	//����������ʼʾֵ�����й����޹�������CTPT,���ڼ����ܼ���Ĳ��������ʼֵ�Ƿ��б仯��
	DWORD m_dwOldMonMtrUpdSec[2];			//����������ʼʾֵ��Ӧ��ʱ�꣨���й����޹���

	bool m_fNewDayStartEnFlg[2]; //�ܼ��������ʾֵ�����й����޹����Ƿ���õ�ǰʾֵ�ı�־
	bool m_fNewMonStartEnFlg[2]; //�ܼ��������ʾֵ�����й����޹����Ƿ���õ�ǰʾֵ�ı�־
	DWORD m_dwEnNewStartSec;	//�ܼ���Ҫ�����µĵ�ǰ���ֵ��ʱ��
	DWORD m_dwDayStartEnSec[2];	//�ܼ��������ʾֵ�ĸ���ʱ��(���й����޹�)
	DWORD m_dwMonStartEnSec[2];	//�ܼ��������ʾֵ�ĸ���ʱ��(���й����޹�)
};

extern void SetGrpParaChg(bool fFlg);
extern bool IsGrpParaChg();
extern void SetCtrlGrpParaChg(bool fFlg);
extern bool IsCtrlGrpParaChg();
void UpdGrpDataProcess(bool fPowerUp);
extern void RunGrpDataProcess();
bool GetGrpCurCtrlSta(int iGrp, TGrpCurCtrlSta *pGrpCurCtrlSta);
bool SetGrpCurCtrlSta(int iGrp, TGrpCurCtrlSta *pGrpCurCtrlSta);
bool GetGrpCtrlSetSta(int iGrp,	TGrpCtrlSetSta *pGrpCtrlSetSta);
bool SetGrpCtrlSetSta(int iGrp,	TGrpCtrlSetSta *pGrpCtrlSetSta);
#endif
