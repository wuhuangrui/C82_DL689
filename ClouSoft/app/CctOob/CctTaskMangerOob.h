#ifndef CCT_TASK_ID_H
#define CCT_TASK_ID_H

#include "TaskDB.h"
#include "ComStruct.h"
#include "DbStruct.h"
#include "MeterStruct.h"
#include "TaskConst.h"
#include "OIObjInfo.h"
#include "DbConst.h"

#define TSA_LEN			17

#define SCH_TYPE_NUM	6	//�ɼ��������͸���

#define SCH_TYPE_COMM	1	//��ͨ�ɼ�����
#define SCH_TYPE_EVENT	2	//�¼��ɼ�����
#define SCH_TYPE_TRANS	3	//͸���ɼ�����
#define SCH_TYPE_REPORT	4	//�ϱ��¼�����
#define SCH_TYPE_SCRIPT	5	//�ű��¼�����
#define SCH_TYPE_REAL	6	//ʵʱ��ط���

#define MS_ONE_GROUP_USER_TYPE		32	//һ���û�����
#define MS_ONE_GROUP_USER_TYPE_REGION		32	//һ���û�����

#define ACQRULE_TABLE_NAME_LEN 128

extern TSem	g_semMtrUdp; //���������
extern TSem	g_semTskCfg; //�������õ�Ԫ
extern TSem	g_semSchCfg;  //�ɼ���������

typedef struct {
	BYTE *pbCfg;
	WORD wCfgLen;
	BYTE bSchNo;
	BYTE bSchType;	
}TMemMalloc;

extern TMemMalloc g_TaskMem[TASK_ID_NUM];
extern TMemMalloc g_SchMem[TASK_ID_NUM];


typedef struct {
	DWORD dwOAD;
	BYTE bData[16];
}TAddInfo;	//������Ϣ

typedef struct {
	WORD wPn;
	WORD wMtrSn;	//����������
	BYTE bTsaLen;	//���ַ����
	BYTE bTsa[16];	//���ַ
	BYTE bBps;	//������
	BYTE bProType;	//��Լ����
	DWORD dwPortOAD;	//�˿�
	BYTE bCodeLen;	//ͨ�����볤��
	BYTE bCode[16];	//ͨ������
	BYTE bRate;	//���ʸ���
	BYTE bUserType;	//�û�����
	BYTE bLine;	//���߷�ʽ
	WORD wRateVol;	//���ѹ
	WORD wRateCurr;	//�����
	BYTE bAcqTsaLen;	//�ɼ�����ַ����
	BYTE bAcqTsa[16];	//�ɼ�����ַ
	BYTE bAssetLen;	//�ʲ��ų���
	BYTE bAsset[16];	//�ʲ���
	WORD wPT;	//
	WORD wCT;	//
	BYTE bAddInfoCnt;	//������Ϣ�������ݶ����8��
	TAddInfo tTAddInfo[8];	//������Ϣ
}TOobMtrInfo;	//�ɼ��������õ�Ԫ 

typedef struct {
	DWORD dwOAD;	//��������������
	BYTE bOADNum;	//������������������ �������������Ϊ16��
	DWORD dwOADArry[16];	//��������OAD
}TROAD;

typedef struct {
	BYTE bChoice;	//0��OAD	1��ROAD	
	DWORD dwOAD;	//��������������
	TROAD tTROAD;	//��¼�Ͷ�������������
}TCSD;


typedef struct {
	BYTE bRegionType;	//�������� 0-ǰ�պ󿪣�1-ǰ����գ�2-ǰ�պ�գ�3-ǰ����
	BYTE bUserStart;	//������ʼֵ
	BYTE bUserEnd;	//�������ֵ
}TUserRegion;	//һ���û���������

typedef struct {
	BYTE bRegionType;	//�������� 0-ǰ�պ󿪣�1-ǰ����գ�2-ǰ�պ�գ�3-ǰ����
	BYTE bTsaStart[TSA_LEN];	//������ʼֵ
	BYTE bStartTsaLen;
	BYTE bTsaEnd[TSA_LEN];	//�������ֵ
	BYTE bEndTsaLen;
}TTsaRegion;	//һ���û���ַ����

typedef struct {
	BYTE bRegionType;	//�������� 0-ǰ�պ󿪣�1-ǰ����գ�2-ǰ�պ�գ�3-ǰ����
	WORD wCfgSnStart;	//������ʼֵ
	WORD wCfgSnEnd;	//�������ֵ
}TCfgMtrSnRegion;	//һ�������������


typedef struct {
	BYTE bBps;	//������
	BYTE bCheckBit;	//У��λ
	BYTE bDataBit;	//����λ
	BYTE bStopBit;	//ֹͣλ
	BYTE bFlowCtrl;	//������
}TCOM_PARAM;	//ͨ�Ų���

typedef struct {
	BYTE bVendorCode[2];	//���̴���
	BYTE bChipCode[2];		//оƬ����
	BYTE bVerDate[5];		//�汾���� YYMMDDhhmm
	WORD wSfwVer;			//����汾
}TSWF_VER_INFO;	//�汾��Ϣ

typedef struct {
	char pszPortDesc[32];			//�˿�������
	TCOM_PARAM tTCOM_PARAM;			//ͨ�Ų���
	TSWF_VER_INFO tTSWF_VER_INFO;	//�汾��Ϣ	
}TCCT_MODE_INFO;					//����ģ����Ϣ

typedef struct {
	BYTE bPortDescLen;			//�˿�����������Ч����
	char pszPortDesc[32];		//�˿�������
	TCOM_PARAM tTCOM_PARAM;		//�˿ڲ���
	BYTE bPortFunc;				//�˿ڹ��� ����ͨ�ţ�0��������1����������2����ͣ�ã�3��
}TPORT_PARAM;	//�˿ڲ���

#define PERIOD_MAX 24

typedef struct {
	BYTE bStarHour;
	BYTE bStarMin;
	BYTE bEndHour;
	BYTE bEndMin;
}TPeriod; //ʱ��

typedef struct {
	BYTE bTaskId; 	//����ID
	TTimeInterv tiExe;	 //ִ��Ƶ��
	BYTE bSchType; 	//��������
	BYTE bSchNo; 	//�������
	TTime tmStart;	//��ʼʱ��
	TTime tmEnd;		//����ʱ��
	TTimeInterv tiDelay;	 //��ʱ
	BYTE bPrio;		//ִ�����ȼ� ��Ҫ��1������Ҫ��2������Ҫ��3�������ܣ�4��
	BYTE bState;		//״̬	1������2ͣ��
	WORD wPreScript;	//��ʼǰ�ű�
	WORD wPostScript;	//��ɺ�ű�
	BYTE bPeriodType;	//ʱ������
	BYTE bPeriodNum;	//ʱ�α����
	TPeriod period[PERIOD_MAX];	//ʱ��
	BYTE bFwdTsa[TSA_LEN]; //͸�������е�ͨ�ŵ�ַ
}TTaskCfg;  //�������õ�Ԫ

typedef struct {
	BYTE bAcqType;	//�ɼ�����
	BYTE bAcqData[4];	//�ɼ�����
}TAcqType;	//�ɼ���ʽ

typedef struct {
	BYTE bSchNo;	//�������
	WORD wStgCnt;	//�洢���
	TAcqType tTAcqType;	//�ɼ���ʽ
	//--------------------------------
	BYTE bCSDNum;	//��¼��ѡ�������tTCSD[x]�ĸ���
	TCSD tTCSD[20];	//��ʱ����20�������������������������һ�����ܣ�����ͳ���Ƿ�ɹ�
	//--------------------------------

	//BYTE bCsdChoice;	//CSD���б�ѡ��0��OAD��1��ROAD
	BYTE bMsChoice;	//���ܱ���MS
	BYTE bMtrMask[PN_MASK_SIZE];	//���ܱ���bMS�еĵ��������
	BYTE bStgTimeScale;	//�洢ʱ��
}TCommAcqSchCfg;	//��ͨ�ɼ�����

typedef struct {
	BYTE bSchNo;	//�������
	BYTE bROADNum;	//tTROAD��ROAD����Ч����
	TROAD tTROAD[32];	//�ݶ���λ32��ROAD
	BYTE bMsChoice;	//���ܱ���MS
	BYTE bMtrMask[PN_MASK_SIZE];	//���ܱ���bMS�еĵ��������
	bool fRptFlg;	//�ϱ���ʶ bool [true:�����ϱ���false:���ϱ�]
	WORD wStgCnt;	//�洢���
}TEvtAcqSchCfg;	//�¼��ɼ�����

typedef struct{
	//698.45����
	BYTE *pCSD;	//0-OAD, 1-ROAD

	//645-07����
	BYTE *pbDlt07;	//ָ�����AcqCmd_2007

	//645-97����
	BYTE *pbDlt97;	//ָ�����AcqCmd_1997

	//AcqCmd_Trans
	BYTE *pbTrans;	//ָ�����AcqCmd_Trans
}TAcqRuleInfo;

typedef struct {
	//698.45����
	BYTE bChoice;	//0-OAD, 1-ROAD
	BYTE *pCSD;
}TSearchId;

typedef struct {
	BYTE bMtrPro;

	BYTE bMain645Num;	//��ID����
	DWORD dwMain645Id[8];	//��ID

	BYTE bSlave645Num;	//��ID����
	DWORD dwSlave645Id[8];	//��ID
}T645IdInfo;

//�ɼ���������ﱣ�����еı�
typedef struct {
	BYTE bMsk[16];	//��Ч������
	char szTableName[ACQRULE_TABLE_NAME_LEN];	//����
	/*
	char szTableName1[128];	//����
	char szTableName2[128];	//����
	......
	*/
}TAcqRuleTable;

#define MK_ACQRULE_TABLE_NAME(pszTableName)	(sprintf(pszTableName, "%sAcqRuleTable", USER_DATA_PATH))	//���вɼ�����������浽���ļ���
#define ACQRULE_FILE_HEAD_LEN		offsetof(TAcqRuleTable, szTableName)
#define ACQRULE_FILE_MSG_LEN		ACQRULE_TABLE_NAME_LEN
#define ACQRULE_FILE_MSG_OFFSET(index)	(ACQRULE_FILE_HEAD_LEN + index*ACQRULE_FILE_MSG_LEN)

void InitMtrMask();

const BYTE * GetMtrMask(BYTE bBank, WORD wPn, WORD wID);

void InitTaskMap();

const BYTE* GetTaskCfgTable(WORD wTaskId);

void InitSchMap();

BYTE* GetSchCfg(TTaskCfg* pTaskCfg, int *iLen);

BYTE* GetSchCfg(BYTE bIndex, int *iLen);

int GetSchParamFromMemory(BYTE bSchNo, BYTE *pbBuf);

int GetTaskNum(); 

bool GetTaskCfg(BYTE bIndex, TTaskCfg *pTaskCfg, bool bIsRdTab = false); 

//������ȡ����ͨ�ɼ���������
//����: @pTaskCfg���������
//	     @pTAcqSchCfg�����������������
//����: Ϊ���ȡ�ɹ�
bool GetCommonSchCfg(TTaskCfg* pTaskCfg, TCommAcqSchCfg* pTCommAcqSchCfg, BYTE *pbArryCSD = NULL);

//������ȡ���¼��ɼ���������
//����: @pTaskCfg���������
//	     @pTAcqSchCfg�����������������
//����: Ϊ���ȡ�ɹ�
bool GetEventSchCfg(TTaskCfg* pTaskCfg, TEvtAcqSchCfg* pTEvtAcqSchCfg);

bool GetTaskCyleUnit(TMtrRdCtrl* pMtrRdCtrl);

BYTE GetTaskCfgSn();

int ParserMsParam(BYTE *pbBuf, BYTE *pbMtrMask, WORD wMtrMaskLen);

bool GetRSDMS(BYTE *pbRSD, BYTE *pbMtrMask, WORD wMaskSize);

bool GetSchMS(BYTE *pbBuf, const TOmMap *p, BYTE bIndex, BYTE *pbMtrMask, WORD wMaskSize);

bool OoParseField(TFieldParser* pParser, BYTE* pFmt, WORD wFmtLen, bool fParseItem);

int FieldCmp(BYTE bCmpType, BYTE* pbCmpField, BYTE bSrcType, BYTE* pbSrcField);

int ReadFromROAD_1(BYTE* pbSelROAD, BYTE* pbSrcROAD, BYTE* pbSelData, BYTE* pbSrcData);
int ReadFromROAD(BYTE* pbSelROAD, BYTE* pbSrcROAD, BYTE* pbSelData, BYTE* pbSrcData);

int CreateTable(char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields, DWORD dwRecNumMax);

bool SaveRecord(char* pszTableName, BYTE* pbRec);

bool SaveRecordByPhyIdx(char* pszTableName, WORD wPhyIdx, BYTE* pbRec);

bool SaveHisRecord(char* pszTableName, int index, BYTE* pbRec);

void InitSchTable();

bool WriteCacheDataToTaskDB(BYTE bSchNo, BYTE bSchType, BYTE *pbRecBuf, WORD wRecLen, WORD wIdex = 0, int* piRecPhyIdx = NULL);

int ReadRecord(BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, int *piTabIdx, int* piStart, BYTE* pbBuf, WORD wBufSize, WORD* pwRetNum);

int ReadTable(char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, int* piStart, WORD wSchNum, WORD* pwRetNum, BYTE* pbBuf);

int OoCopyOneOadData(BYTE *pbSrc, BYTE *pFmt, WORD wFmtLen, BYTE *pbDst, WORD *pwRetFmtLen, WORD *pwRetSrcOffset);

int OoFormatSrcData(BYTE *pbSrc, WORD wSrcLen, BYTE *pFmt, WORD wFmtLen, BYTE *pbDst);

int OoCopyData(BYTE* pbOAD, BYTE* pbDst, BYTE* pbSrc, WORD wItemOffset, WORD wLen, BYTE* pFmt, WORD wFmtLen);

int SearchTable(BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, WORD wRcsdIdx, int *piTabIdx, char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields);

int TableMatch(BYTE* pbRCSD, WORD wRcsdIdx, BYTE* pbFixCfg, BYTE* pbFixFmt, WORD wFixFmtLen,BYTE* pbDataCfg, BYTE* pbDataFmt, WORD wDataFmtLen);

int RcsdMatch(BYTE* pbRCSD, WORD wRcsdIdx, BYTE* pbCfg, BYTE* pbFmt, WORD wFmtLen);

int ReadRecField(BYTE* pbRec, WORD wOffset, TFieldParser* pParser, BYTE* pbOAD, BYTE* pbCSD, BYTE* pbBuf);

DWORD GetEvtTaskTableFixFieldLen();

DWORD GetEvtTaskRecLastSerialNumber(BYTE bSchNo, BYTE bCSDIndex);

DWORD GetEvtTaskRecLastSerialNumber(BYTE* pbTsa, BYTE bTsaLen, BYTE* pbCSD, BYTE bLenCSD);

bool RecMatch (BYTE* pbRec, TFieldParser* pFixFields, BYTE* pbRSD);

bool SearchField (TFieldParser* pFixFields, BYTE* pbOAD, WORD* pwIndex, WORD* pwOffset, WORD* wFieldLen);

bool OadMatch(BYTE bOp, BYTE* pbOAD, BYTE* pbData, BYTE* pbFieldData, WORD wFieldLen);

bool DateTimeSMatch(BYTE bOp, BYTE* pbDateTimeS, BYTE* pbFieldData);

bool MsMatch(BYTE* pbMS, BYTE* pbFieldData);

bool TiMatch(BYTE* pbStartTime, BYTE* pbEndTime, BYTE* pbTI, BYTE* pbFieldData);

bool SaveRecord(char* pszTableName, BYTE* pbRec, int* piRecPhyIdx);

int MsToMtrNum(BYTE *pbMs);

int DayFrzTimeMatch(BYTE *pSrc, BYTE *pRcsd);

extern bool SearchAcqRule645ID(BYTE *pbCSD, BYTE bMtrPro, T645IdInfo *pT645IdInfo);
extern int GetAcqRuleTableName(BYTE *pbCSD, char *pszTabName, WORD wTabNameLen, TAcqRuleInfo *pTAcqRuleInfo);
extern int GetOneAcqRuleInfo(BYTE *pbPara, char *pszTabName, WORD wTabNameLen, TAcqRuleInfo *pTAcqRuleInfo);
extern int GetAcqRuleFromTaskDB(char *pszTabName, BYTE *pbOAD, BYTE *pbRespBuf);
extern bool SaveAcqRuleTableName(char *pszSaveTabName);
extern bool DeleteAcqRuleTableName(char *pszDelTabName);
extern bool GetAcqRuleTableName(int *piStart, char *pbRespTab, WORD wMaxTabNameLen);
extern int GetAllAcqRuleInfo(int *piStart, BYTE *pRespBuf, WORD wMaxLen);

#endif