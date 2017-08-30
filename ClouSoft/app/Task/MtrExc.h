/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrExc.h
 * ժ    Ҫ�����ļ���Ҫʵ���������Э��ĳ����¼���
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��10��
 *********************************************************************************************************/

#ifndef MTREXC_H
#define MTREXC_H
#include "DbStruct.h"
#include "MeterStruct.h"
#include "TermEvtTask.h"

//#define TSA_OCTSTR_TYPE				1		//TSA���Ͷ��壺TSA(0x55) + OCT_STR_LEN(N+2) + TSA_LEN(N) + ADDR[0] ~ ADDR[N]

#define TERM_EXC_NUM				33		//�ն��¼�����
#define MTR_EXC_NUM					7		//sizeof(g_wMtrExcOI)/sizeof(WORD) �����¼�����
#define ITEM_RD_TIME_NUM			3		//ʱ�����

#define MAX_JUDGE_OAD				5		//�жϳ����¼��賭��OAD������


#define MTR_EXC_REC_LEN				1024	//�ݶ�����

#define SAMPLE_MTR_NUM				4		//ͣ����󳭶��ĵ��ܱ����
#define SAMPLE_CFG_ID_LEN			(SAMPLE_MTR_NUM*19 + 33)	//ͣ�����ݲɼ����ò�������(13+19*SAMPLE_MTR_NUM+20) = 109

#define DIFF_COMP_CFG_NUM			10
#define DIFF_COMP_CFG_ID_LEN		(DIFF_COMP_CFG_NUM*28 + 2)	//�й��ܵ��ܲ�����ò�������

#define RESPONSE_TYPE_NORAML		1
#define RESPONSE_TYPE_RECORD		2


//��ǰ�¼�״̬--��˲ʱ״̬��
#define ERC_STATE_MIDDLE			0	//�м�̬
#define ERC_STATE_HAPPEN			1	//����
#define ERC_STATE_RECOVER			2	//�ָ�


#define OI_MTR_CLOCK_ERR		0x3105	//���ܱ�ʱ�ӳ���
#define OI_MTR_ENERGY_DEC		0x310B	//���ܱ�ʾ���½�
#define OI_MTR_ENERGY_ERR		0x310C	//����������
#define OI_MTR_FLEW				0x310D	//���ܱ����
#define OI_MTR_STOP				0x310E	//���ܱ�ͣ��
#define OI_MTR_RD_FAIL			0x310F	//����ʧ���¼�
#define OI_MTR_DATA_CHG			0x311C	//���ܱ����ݱ����ؼ�¼

#define OI_UN					0x4104	//���ѹ

#define OAD_EVT_RPT_STATE		0x33000200	//��׼�¼��ϱ�״̬OAD

#define	MTREXC_CLR_ID			0x0b15
#define	MTREXC_CLR_VALID		0x5A

//��dwItemRdTime[ITEM_RD_TIME_NUM]�е����
#define EP_POS_ITEM_INDEX		0	//����
#define EP_NEG_ITEM_INDEX		1	//����
#define CLOCK_ITEM_INDEX		2	//ʱ��

#define TERM_EVENT_DEBUG

typedef struct
{
	WORD wOI;					//OI
	TFieldParser* pFixField;	//�ֶν�����
	BYTE* pbFmt;				//�̶��ֶθ�ʽ��
	WORD wFmtLen;				//�̶��ֶθ�ʽ������
}TMtrExcFixUnitDes;	//�¼���¼��Ԫ�̶��ֶ�����

int DoMtrExcMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoMtrExcMethod2(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoMtrExcMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoMtrExcMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoMtrExcMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
//bool AllocateMtrExcMem(BYTE* pbGlobal, TAllocTab* pAllocTab, WORD wTabNum);

void InitMtrExc();
void ClrMtrExc();
void ReInitEvtPara(DWORD dwOAD);
void MtrExcOnRxFaResetCmd();
int GetMtrExcIndex(WORD wOI);

void SetMtrExcTableFlag(WORD wOI);
void ClrMtrExcTableFlag(WORD wOI);
bool IsMtrExcTableCreate(WORD wOI);
int GetMtrExcEvtRecord(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbRecBuf, WORD wBufSize);
bool GetMtrExcFieldParser(WORD wOI, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize);
bool UpdateMtrExcRptState(DWORD dwCnOAD, TEvtMsg* pEvtMsg, BYTE bRptState);
int OoProRptMtrExcRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize);

void SetMtrExcOadDefCfg(WORD wOI);

//========================���ܱ����ݱ����ؼ�¼==========================
//���������ܱ����ݱ����ؼ�¼����ȡ����OAD
int GetMtrDataChgCSD(BYTE* bCfg);
extern bool RefreshMtrExcMem();

#endif //MTREXC_H
