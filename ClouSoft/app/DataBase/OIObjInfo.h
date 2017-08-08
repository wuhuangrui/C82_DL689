/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�OIObjInfo.h
 * ժ    Ҫ�����ļ���Ҫʵ����������OI���ݵĸ�ʽ����
 * ��ǰ�汾��1.0
 * ��    �ߣ��׳ɲ�
 * ������ڣ�2016��9��
 *********************************************************************************************************/
#ifndef OIOBJINFO_H
#define OIOBJINFO_H
#include "apptypedef.h"

#define DT_NULL		0
#define DT_UNCARE	255

//���ݷý����
//		-1��ʾ��OBIS�������Ծܾ�����
//		-2��ʾ��OBIS���󲻴���
//		-3��ʾ��BOIS���������ʶ��һ��
//		-4��ʾ����ԭ����ʲ��ɹ�
#define DA_READ_WRITE_DENIED	1		//��д�ܾ�
#define DA_OBJECT_UNAVAILABLE	2		//���󲻴���
#define DA_OBJ_CLASS_INCONSIST	3		//�����಻��
#define DA_OTHER_ERROR			4		//��������

//-------�򵥷Ƕ�������(bit-string octet-string visible-string)�ĸ�ʽ����-----
//�䳤��ʽ�ֽ��ֽڶ���
#define RLF	0	//RL����
#define RLV	1	//RL�䳤
#define LRF	2	//LR����
#define LRV	3	//LR�䳤

#define BRVS	(0x10 | LRF)	//��λ����

#define MAP_SYSDB	0
#define MAP_TASKDB	1
#define MAP_VAL		2
#define MAP_BYTE	3


#define OI_RD	1
#define OI_RW	2

#define MAX_MAINIP_NUM		2	//��վIP����
#define MAX_SMS_MAIN_NUM	5	//������վ����
#define MAX_SMS_SEND_NUM	5	//����֪ͨ����
#define MAX_232_PORT_NUM	2	//232ͨ������
#define MAX_485_PORT_NUM	3	//485ͨ������
#define MAX_HW_PORT_NUM		1	//����ͨ������
#define MAX_SW_PORT_NUM		8	//������ͨ������ (�Žڵ���YX8)
#define MAX_DC_PORT_NUM		1	//ֱ��ģ����ͨ������
#define MAX_RLY_PORT_NUM	4	//�̵���
#define MAX_ALRM_PORT_NUM	1	//�澯���
#define MAX_MUL_PORT_NUM	1	//�๦�ܶ���
#define MAX_PLC_PORT_NUM	1	//�ز�
#define MAX_PLUS_PORT_NUM	2	//����
#define MAX_GPRS_COM_NUM	2	//GPRSͨ������
#define MAX_ETH_COM_NUM		8	//��̫��ͨ������
#define MAX_TIME_SCH_MTR_NUM 4	//��ʱ�ѱ��������

#define OAD_OI_MASK				0xff001f00		//��ȡOI, ���ε�����������������Ԫ������
#define OAD_FEAT_MASK			0xffff1fff		//��ȡOAD, ֻ���ε���������
#define OAD_FEAT_BIT_OFFSET		13				//��ȡ��������������dwOAD&~OAD_FEAT_MASK��������13λ

#define BALANCE_DAY_NUM			3	//�����ո���

extern BYTE g_bOIFmt[];

typedef struct{
	DWORD dwOA;		//object attrib, ��WORD��OI����WORD������ֵ
	WORD  wClass;	//�ӿ���
	WORD  wMode;	//ӳ�䷽ʽ������ӳ�䵽ϵͳ����������
	WORD  wID;		//��ӦϵͳBN0��ID
	WORD  wPn;		//�������
	WORD  wVal;		//����ֵ��һ�����ڴ洢����ı������ֵ
	BYTE* pFmt;		//��ʽ������
	WORD  wFmtLen;	//��ʽ����������
	char* pszTableName;	//ӳ�䵽������Ӧ�������ļ���
}ToaMap;

typedef struct{
	DWORD dwOM;		//Object Attribute����WORD��OI����WORD�ŷ�����ʶ
	WORD wClass;	//�ӿ���
	BYTE* pFmt;		//�����ĸ�ʽ������
	WORD wFmtLen;	//�����ĸ�ʽ�������ĳ���
	int (*pfnDoMethod)(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int *piRetLen);
	void* pvAddon;	//���Ӳ�������Բ�ͬ�����в�ͬ
}TOmMap;//���󷽷�ӳ���

extern ToaMap g_OIConvertClass[];
extern const ToaMap* GetOIMap(DWORD dwOIAtt);
extern TOmMap g_OmMap[];
extern const TOmMap* GetOmMap(DWORD dwOIMethod);

extern BYTE g_bTskUnitFmtDesc[];
extern BYTE g_bCommFmtDesc[];
extern BYTE g_bEvtFmtDesc[];
extern BYTE g_bZJEvtFmtDesc[14];
extern BYTE g_bTranFmtDesc[];
extern BYTE g_bRptFmtDesc[];
extern BYTE g_bRealFmtDesc[];

int DoRightNowStartSchMtr(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClearSchMtrFile(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClearCrossSchMtrFile(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);


int DoClass8BroadcastTimeMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);

int DoClass8ClockSourceMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen = NULL);
int DoClass8ClockSourceMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen = NULL);


int DoDevInterfaceClass19(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoGprsInterfaceClass25(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoEthInterfaceClass26(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method127_AddMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method128_AddMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method129_UpdataMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method130_UpdataMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method131_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method132_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method133_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method134_DelAllMtr(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//��������ӻ����һ���������õ�Ԫ
int AddCommonMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//���������͸���ɼ�����
int AddTransMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//������ɾ��һ�����õ�Ԫ			
int DelCommonMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//������������õ�Ԫ			
int ClrCommonMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//��������ӻ����һ��ɼ�����
int AddAcqRuleMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//������ɾ��һ��ɼ�����
int DelAcqRuleMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//��������ղɼ������
int ClrAcqRuleMethod131(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//��������������״̬
int UdpTaskState130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//0x6014 ������130	���÷����ļ�¼��ѡ��
int ResetSchRecordCSD(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//0x6016 ������130	�����ϱ�������ʶ
int UpdateRptFlgMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);


//��������ȡ�������õ�Ԫ
extern int GetTaskConfigFromTaskDb(BYTE bTaskId, BYTE *pbRespBuf);

//��������ȡ�ɼ����������ɼ�����
extern int GetSchFromTaskDb(BYTE bSchNo, BYTE bSchType, BYTE *pbRespBuf);

//////////////////////////////////////////////////////////////////////////
//�ܼ��鷽��
int ClrGrpCfgMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int AddGrpCfgMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int BatAddGrpCfgMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DelGrpCfgMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//ң�ط���
int YkCtrlTriAlertMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int YkCtrlDisAlertMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int YkCtrlOpenMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int YkCtrlCloseMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//���緽��
extern int InputGuaranteeMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
extern int QuitGuaranteeMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
extern int QuitAutoGuaranteeMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//�߷Ѹ澯����
int InputUrgeFeeMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int QuitUrgeFeeMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//������Ϣ����
int AddChineseInfoMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DelChineseInfoMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//���Ʒ���
int AddCtrlUnitMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DelCtrlUnitMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int InputCtrlMethod6(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int QuitCtrlMethod7(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int InputCtrlMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

WORD GetOiClass(WORD wOI);

BYTE* GetSchFmt(BYTE bSchType, WORD* pwFmtLen);

char* GetSchTableName(BYTE bSchType);

int GetEvtTaskTableName(BYTE bSchNo, BYTE bCSDIndex, char* pszTableName);

//�����������
int ComPortParaCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int RelayParaCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int MulPortCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int CctTransmitMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int ResetStatAll(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);



#endif
