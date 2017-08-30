#ifndef STDREADER_H
#define STDREADER_H

#include "CctAPI.h"
#include "LoopBuf.h"
#include "Comm.h"
#include "sysarch.h"
#include "sysapi.h"
#include "CctSchMtr.h"
#include "TransmitApi.h"

//�ز���λ��Ϣ
#define PLC_PHASE_UNK				0x00	
#define PLC_PHASE_A					0x01
#define PLC_PHASE_B					0x02
#define PLC_PHASE_C					0x03


//·���������󳭶���ʶ
#define RT_RD_FAIL					0x00	//����ʧ��
#define RT_RD_SUCC					0x01	//�����ɹ�
#define RT_RD_GOTO					0x02	//���Գ���

#define PRO_TYPE_TRANS				0x00
#define PRO_TYPE_645_97				0x01
#define PRO_TYPE_645_07				0x02

//��������ַ�ĵ�ַ����
#define SER_ADDR_TYPE_SIG			0x00	//����ַ
#define SER_ADDR_TYPE_COMM			0x01	//ͨ���ַ
#define SER_ADDR_TYPE_GRP			0x02	//���ַ��ַ 
#define SER_ADDR_TYPE_BC			0x03	//�㲥��ַ

#define PRO_69845_LEN_OFFSET		0x01	//������Lƫ��
#define PRO_69845_DATA_LEN			0x02	//������L�ֽ���
#define PRO_68945_CTRL_OFFSET		0x03	//������C
#define PRO_69845_HEAD_CS_OFFSET	0x04	//֡ͷУ��
#define PRO_69845_HEAD_CS_LEN		0x02	//֡ͷУ�鳤��
#define PRO_69845_FRAM_CS_LEN		0x02	//֡У�鳤��

#define PRO_3762_LEN_OFFSET			0x01	//����ƫ��
#define PRO_3762_DATA_LEN			0x02	//����

#define FN(x)						(x)


//AFN
#define AFN_CON		            	0x00	//ȷ�ϨM����
#define AFN_INIT			        0x01	//��ʼ��
#define AFN_TRAN	    	    	0x02	//����ת��
#define AFN_QRYDATA			    	0x03	//��ѯ����
#define FRM_LNKTEST			    	0x04	//��·�ӿڼ��
#define AFN_CTRL			    	0x05	//��������
#define AFN_REP			    		0x06	//�����ϱ�
#define AFN_QRYRT					0x10	//·�ɲ�ѯ
#define AFN_SETRT		    		0x11	//·������
#define AFN_CTRLRT					0x12	//·�ɿ���
#define AFN_RTFWD 		    		0x13	//·������ת��
#define AFN_RTRD 		    		0x14	//·�����ݳ���
#define AFN_TRSFILE   				0x15    //�ļ�����


//�ز��������ĵײ�ͨ����·����
#define AR_LNK_UNKNOW				0xff    //δ֪����
#define AR_LNK_ES					0x01	//�����ز�
#define AR_LNK_TC					0x02	//�ൺ����
#define AR_LNK_RSC					0x03	//��˹��
#define AR_LNK_XL					0x04	//о��
#define AR_LNK_CL					0x05	//��½����
#define AR_LNK_LS					0x06	//о��
#define AR_LNK_SGD					0x06	//�����
#define AR_LNK_GY					0x07	//��һ


//���ڳ���ģʽ    
#define CCT_RD_BY_TERM				0x01	//��������������
#define CCT_RD_BY_ROUTE				0x02	//·����������
#define CCT_RD_TERM_ROUTE			0x03	//���ֳ���ģʽ��֧��

//�ӽڵ�ͬ��ģʽ
#define CCT_NO_SYNC					0x00    //����Ҫ�·��ӽڵ���Ϣ
#define CCT_NEED_SYNC				0x01    //��Ҫ�·��ӽڵ���Ϣ

//·��ģ��ͨ�ŷ�ʽ
#define CCT_RD_TYPE_PLC				0x01    //խ���������ز�
#define CCT_RD_TYPE_HSPLC			0x02	//����������ز�
#define CCT_RD_TYPE_RADIO			0x03	//΢��������ͨ��



typedef struct{
	BYTE  bModule;		//ģ������
	BYTE  bModType;     //ͨ�ŷ�ʽ
	BYTE  bRtType;      //·�ɹ���ʽ
	BYTE  bNodeType;    //�ӽڵ���Ϣģʽ
	BYTE  bRdType;      //���ڳ���ģʽ
	BYTE  bTansTmOut;   //������ʱ����֧��
	BYTE  bRdFailType;  //ʧ�ܽڵ��л�����ʽ
	BYTE  bBdCfmType;   //�㲥����ȷ�Ϸ�ʽ
	BYTE  bBdChnlType;  //�㲥�����ŵ�ִ�з�ʽ
	BYTE  bChnlNum;     //�ŵ�����
	BYTE  bSpeedNum;    //��������
	BYTE  bVolMis;      //��ѹ����������Ϣ
	BYTE  bUpdateTmOut; //���������ȴ���ʱʱ��
	BYTE  bNodeTmOut;   //�ӽڵ������ʱʱ��
	BYTE  bTrySendCnt;	//��չ��Э��û�ж��壬��Բ�ͬ��·��ģ�鷢�Ͳ�ͬ�Ĵ���

	WORD  wBdTmOut;     //�㲥�������ʱʱ��
	WORD  wFrmMaxLen;   //���֧�ֵı��ĳ���
	WORD  wTansMaxLen;  //�ļ�����֧�ֵ���󵥸����ݰ�����
	WORD  wMaxNodeNum;  //֧�ֵ����ӽڵ�����
	WORD  wNodeNum;     //��ǰ�ӽڵ�����

	BYTE  bMainAdd[6];      //���ڵ��ַ
	BYTE  bProRelsDate[3];  //ͨ��ģ��ʹ�õ�Э�鷢������
	BYTE  bProRecdDate[3];  //ͨ��ģ��ʹ�õ�Э����󱸰�����
	BYTE  bFacCode[9];      //���̴��뼰�汾��Ϣ
	WORD  wSpeedArr[5];       // ͨ������
}RtRunModeOob; //����ͨ��ģ������ģʽ��Ϣ

typedef enum {
	CCT_RUN_OTHER,	//������������
	CCT_SCH_MTR,	//�ѱ�״̬
	CCT_RD_MTR,		//�����Զ���ֱ����
// 	CCT_AUTO_RD,	//�Զ�����		AFN=14-F1
// 	CCT_DIR_RD,		//ֱ���������	AFN=13-F1
}TCCT_RUN_STATE;

typedef struct {
	BYTE bPhase;	//�ز�ͨ����λ��0δ֪�࣬1��3ָ��1��2��3��
	BYTE bTsaLen;	//���ַ��Ч����
	BYTE bTsa[TSA_LEN];	//���ַ
	BYTE bRevTsa[TSA_LEN];	//���ַ����
	WORD wPn;
	WORD wNodeSn;	//�ӽڵ����
}TRtReqInfo;

typedef struct {
	WORD wProType;//=PROTOCOL_TYPE_698_45;	
	BYTE bHead;	//��ʼ�ַ� 0x68
	WORD wDataLen;	//����ʼ�ַ��ͽ����ַ�֮���֡�ֽ���
	BYTE bCtrl;	//������
	BYTE bSAFlg;	//��ַ��ʶ��bit0~3��ʾ��������ַ�ֽ�����bit4~5�߼���ַ��bit6~7Ϊ��������ַ��
					//����ʶ, 0��ʾ����ַ��1��ʾͨ���ַ��2��ʾ���ַ��3��ʾ�㲥��ַ
	BYTE bSALen;	//bSA����Ч����
	BYTE bSA[TSA_LEN];	//��������ַ�����鰴���ռ�
	BYTE bCA;	//�ͻ�����ַCA	ȡֵ��Χ0��255��ֵΪ0��ʾ����ע�ͻ�����ַ
	WORD wHCS;	//�Ƕ�֡ͷ���ֳ���ʼ�ַ���HCS����֮��������ֽڵ�У�飬У���㷨����¼D
	WORD wAPDULen;	//APDUӦ�ò�Э�����ݵ�Ԫ����
	BYTE bAPDUData[256];	//����256�ռ�洢APDU��Ԫ����
	WORD wFCS;	//�Ƕ���֡����ʼ�ַ��������ַ���FCS����֮��������ֽڵ�У�飬У���㷨����¼D
	BYTE bEnd;	//0x16
}TFrm69845;	//698.45֡��ʽ

typedef struct {
	WORD bProType;//=PROTOCOL_TYPE_1376_2;
	BYTE bHead;	//0x16
	WORD wDataLen;	//���ݳ��� ֡ͷ��֡β
	BYTE bCtrl;	//Bit7���䷽��λ��Bit6������ʶλ��Bit5~0ͨ�ŷ�ʽ
	BYTE bR[6];	//��Ϣ��R
	BYTE bSrcAddr[6];	//Դ��ַ
	BYTE bRelayAddr[6*6];	//�м̵�ַ
	BYTE bDesAddr[6];	//Ŀ�ĵ�ַ
	BYTE bAfn;	//Ӧ�ù�����
	BYTE bDt[2];	//���ݵ�Ԫ��ʶ
	WORD wDtLen;	//Dt���ݵ�Ԫ��Ч����
	BYTE bDtBuf[512];	//Dt���ݵ�Ԫ�������ռ�
	BYTE bCs;	//У���
	BYTE bEnd;	//0x16
}TFrm13762;	//1376.2֡��ʽ

typedef struct {
	bool fGetPlcInfo;	//��ȡģ����Ϣ
	bool fPlcInit;		//�ز�ģ���Ƿ��ʼ��
	bool fSetMainNode;	//�Ƿ��������ڵ��ַ
	bool fSyncAddr;		//�Ƿ�ͬ������
	bool fIsNeedRtReq;	//�Ƿ���Ҫ·������������˹������Ҫ�����š�������Ҫ
	BYTE bTermLen;	//�ն˵�ַ����
	BYTE bTermAddr[TSA_LEN];	//�ն��߼���ַ
	DWORD dwGetSyncAddrInfoClick;  //�յ�����ͬ���źŵ�ʱ��
}TRtStat;

typedef struct {
	int iStart;	//͸�����������Ǹ��ݡ�0x6019�ķ������ݡ�����һ�����ַ��Ӧһ�ű�
				//���еı�������һ���ļ��У��ò���iStart���������ڸ��ļ���ƫ��λ��
	int iMsgSn;		//͸�����䷽���������
	bool fStart;
	bool fFinsh;	//����͸�����񷽰��Ƿ�ִ�н���
}TransSchSate;	//͸����������

typedef struct {
	bool fDirRdFlg;			//ֱ�������磺��������
	bool fRtPause;				//true·���Ѿ���ͣ
	BYTE bRdFailTsa[TSA_LEN];	//�ϴγ���ʧ�ܵı��ַ
	TTime tLastUdpTime;	
	DWORD dwLstDirClk;			//�ϴ�����ֱ����ʱ��
	DWORD dwBcWaitClick;  // �㲥�������Ҫ�ȴ���ʱ��

	TTaskCfg tCurExeTaskCfg;	//��ǰ����ִ�е��������õ�Ԫ
	TransSchSate tTransSchSate;
}TCctRunStateInfo;	//�ز��߳�������Ϣ


class CStdReader : public CCctSchMeter{
private:
	CLoopBuf m_LoopBuf;

	CComm  m_Comm;

	TFrm13762 m_TRcv13762;

	TCctRunStateInfo m_TRunStateInfo;

	RtRunModeOob m_RtRunMdInfo;

	TRtStat m_TRtStat;

	BYTE m_bCctExeState;

	char *m_pszName;

	bool m_fRxComlpete;

	TSem m_semReader;

	int m_iPn;
	DWORD m_dwLastWaitSec;//��һ��·�ɻظ��ȴ�ʱ��,�·��㲥֡ʱ���øñ���
	bool m_fIsLastLocalModuleState; //  ��¼�䶯ǰ��һ���ز�ģ��״̬
public:
	
	CStdReader(void);
	
	virtual ~CStdReader(void){};	

	void Init();

	void InitRcv();

	void InitPhyPort();

	void ClearRcv();

	void  LockReader();

	void  UnLockReader();

	bool ModuleInfoCheck();
private:
	void LockDirRd();

	void UnLockDirRd();

private:
	bool InitRouter(BYTE bFn);


	bool GetInitState();

	bool Is3762Afn13Fn01(BYTE *pbData, int iLen);
	bool IsValid3762Frame(BYTE *pbData, int iLen, TFrm13762 &tmpInfo3762);
	bool Copy376ToBuf(BYTE *pbData, int iMaxLen, TFrm13762 *pRcv13762);

public:
	bool Afn01Fn01_HardwareInit();

	bool Afn01Fn02_ParmInit();

	bool Afn01Fn03_DataInit();

	bool Afn03Fn04_ReadMainNodeAddr(BYTE* pbBuf);

	void Afn03Fn10_RptRtRunInfo(BYTE* pbBuf);

	bool Afn05Fn01_SetMainNodeAddr();

	bool Afn05Fn3_StartBoardCast(BYTE *pbReqBuf, WORD wLen);

	void Afn06Fn01_RptNodeInfo();

	bool Afn06Fn02_RptData();

	void Afn06Fn03_RptRtInfo();

	void Afn06Fn04_RptMtrInfo();

	void Afn06Fn5_RptNodeEvt();

	WORD Afn10Fn01_RdRtNodeNum();

	int Afn10Fn02_RdNodeInfo(BYTE bRdMtrNum, WORD wStartSn, BYTE *pbOutBuf);

	int Afn10Fn04_QueryRtRunInfo(BYTE *pbRespBuf);

	bool Afn11Fn01_AddNode(BYTE *pbInBuf, BYTE bInLen);

	int Afn11Fn02_DelNode(BYTE *pbInBuf, BYTE bInLen); 

	bool Afn11Fn05_ActSlaveNodeRpt(BYTE *pbBuf, BYTE bLen);

	bool Afn11Fn06_StopSlaveNodeRpt();

	int DirAskProxy(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData);
private:

	int Set_OAD_to_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);
	int Act_OAD_to_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);
	int Do_uplink_request_to_698_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);
	int Read_OneOAD_from_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);
	int Read_RecordData_from_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);


	bool RtCtrl(BYTE bFn);

public:
	bool Afn12Fn01_RtRestart();

	bool Afn12Fn02_RtPause();

	bool Afn12Fn03_RtResume();

	int Afn13Fn01_RtFwd(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbInBuf, WORD wInLen, TMtrRdCtrl *pMtrRdCtrl, TRdItem *pRdItem, BYTE *pbOutBuf, BYTE bProId, bool fAnaly645data=false, bool fIs645Proxy=false, BYTE *pbRcvFrm=NULL, WORD *pwRcvLen=0);

	int Afn14Fn1_RtReqRd();

	bool Afn14Fn2_RtReqClk();

	//---------------------------------------------------------------
	void RouterPause();
	void RouterResume();
	//---------------------------------------------------------------

	bool ReadPlcModuleInfo();

	int DirectReadMeterData(BYTE *pbTsa, BYTE bTsaLen, BYTE bProId, DWORD dwID, BYTE *pbInBuf, WORD wInLen, BYTE *pbOutBuf, BYTE *pbRcvFrm, WORD *pwRcvLen);


//////////////////////////////ProtoProc//////////////////////////////////////
	WORD Make645Frm(BYTE* pbFrm, const BYTE* pbAddr, BYTE bCmd, BYTE bDataLen);

	int Make698_45Frm(BYTE *pbMtr, BYTE bMtrLen, BYTE bCtrl, BYTE bAFType, BYTE bCA, WORD wFramFmt, BYTE *pbAPDU, WORD wAPDULen, BYTE *pbRespBuf);

	bool DecodeFram69845(BYTE *pbInBuf, WORD wInLen, TFrm69845 *ptTFrmFmt);

	int DecodeReportApdu(BYTE *pApdu, WORD wApduLen, TRdItem *pRptItem, BYTE *pbBuf, bool fIsRptFlg=false);

	int Make1376_2Frm(BYTE *pbTsa, BYTE bTsaLen, BYTE bCtrl, BYTE *pbR,  BYTE bAfn, BYTE bFn, const void * pbInbuf, WORD wInLen, BYTE *pbRespBuf);

	int Pro1376_2ToBuf(TFrm13762 *ptTFrmFmt, BYTE *pbOutBuf);

	int MakeTransmitAcqSchFram();

	//WORD GetRequestNormal(DWORD dwOAD, BYTE* pbTxBuf);

	//WORD GetRequestRecord(DWORD dwOAD, BYTE* pbTxBuf, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD);

	//int GetResponseNormal(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbDstBuf);

	//int GetResponseRecord(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbRCSD, BYTE bLenRCSD, BYTE* pbDstBuf);

	void GetRSDAndRCSD(DWORD *pdwOAD, BYTE* pbRSD, WORD* wRSDLen, BYTE* pbRCSD, WORD* wRCSDLen, BYTE bMethod, BYTE* pbData, BYTE* pbCSD);

/////////////////////////////////End ProtoProc///////////////////////////////////

	int DirectTransSchMsg(BYTE bSchNo, TransFilePara *pTransFilePara, TTransMsg *pTTransMsg);

	int SaveTransSchMsg(BYTE bSchNo, BYTE *pbMsgBuf, WORD wMsgLen);

	int DoFwdData(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE *pbRespBuf, bool fGetApdu = false);
	int DoFwdData(BYTE * pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE * pbRespBuf, int iMaxResLen);
#if 0
	
#if 0
	int DirectReadMeterCommonData(TTaskCfg *pTTaskCfg, TCommAcqSchCfg *pTCommAcqSchCfg, WORD wMtrSn);
#endif

	int DirectReadMeterTransmitData(WORD wMtrSn, const BYTE *pbReqBuf, BYTE bReqLen, BYTE *pbRespBuf);
#endif

	void InitTaskIdSched();

	bool DoTaskIdSched();

	bool IsDirReadTi(TTimeInterv *pTi);

	bool QueryFrzTaskReadState();

	bool DoAllAcqSch();

	bool DoCommSch();

	bool DoTransSch();

	bool SyncMeterAddr();

	void DoOtherTask();

	bool DoReadRptErc();
	//------------------------------------------------------------
	bool StartBoardCast(int iMin);
	
	int StartSchMtr();

	bool StartNodeActive();

	bool WaitMtrReport();

	int FinishSchMtr();

	//------------------------------------------------------------

	void CctRunStateMonitor();

	DWORD Send(BYTE *pSendBuf, DWORD dwLen);

	DWORD Receive(BYTE *pRecvBuf, DWORD wLen);

	BYTE GetCtrl(BYTE bRcvCtrl);

	WORD DtToFn(BYTE* pbDt);

	void FnToDt(WORD wFn, BYTE* pbDt);

	int RcvFrame(BYTE *pbBlock, int nLen);

	bool DefHanleFrm();

	bool RxHandleFrm(DWORD dwSeconds, bool fIsDefHanleFrm=true);

	void DoAutoRead();

	void CctRunStateCheck();

	void RunThread();
	int DL645_9707MakeFrm(BYTE *pbMtr, BYTE bMtrLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *bpBuf);
	int DL645_EXTMakeFrm(BYTE *pbMtr, BYTE bMtrLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *bpBuf);
	int GetDL645_9707DataVal(BYTE *psData, BYTE bsLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *pbData, TRdItem *pRdItem=NULL, WORD wPn=0);
	int GetDL645_EXTDataVal(BYTE *psData, BYTE bsLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *pbData);
	int OneAddrBroadcast(BYTE *pbTsa, BYTE *pbInBuf, WORD wInLen, TMtrPara tTMtrPara, TRdItem *pRdItem, BYTE *pbData, BYTE bProId);
	int Afn13Fn01_Broadcast(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbInBuf, WORD wInLen, TRdItem *pRdItem, BYTE *pbOutBuf, BYTE bProId, BYTE bWaitTm);
	int ReadDL645_9707Time(BYTE * pDbTsa, TMtrPara tTMtrPara, TRdItem *pRdItem, BYTE *pbData, BYTE bProId);
	int MtrBroadcast();
	int BroadcastAdjustTime();
	int Broadcast(BYTE *pbTsa, BYTE bTsaLen, BYTE *pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE *pbRespBuf, BYTE bPro = 0);

	//ȫ�¼��ӿ�
	int DL645V07AskItemErc(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId, DWORD dwOAD, BYTE* pbRxBuf);
	int DL645V07ProIdTxRx(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId, DWORD dwProId, BYTE* pbRxBuf, WORD wMaxRxLen);
	int DL645V07AskItemErcHapEndEng(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId,  DWORD dwOAD, TErcRdCtrl* pErcRdCtrl, BYTE* pbDstBuf);
	bool SaveRepErcData(BYTE *pbBuf, WORD wLen);

	//��ͬЭ�����ͱ����ӿ�
	int ReadDLT_645(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen);
	int ReadDLT_69845(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen);
	int ReadDLT_SBJC(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen);
	
	int SetRouterRequestInfo(BYTE bState, BYTE *pBuf);

};


void NewCctThread();

int GetSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen, DWORD dwStartSec, DWORD dwEndSec);

int GetCrossSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen);

void SaveSearchPnToDb(BYTE* pbMtrAddr, BYTE bAddrLen, BYTE bPro, TTime tmNow, BYTE bPort);

bool SetSchMtrEvtMask(WORD wIndex, bool fState);

bool UpdataSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen);

bool GetSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen);

void ClearSchMtrEvtMask();

int GetSchMtrEvtData(int iIndex, BYTE *pbBuf);

extern CStdReader *g_CStdReader;
#endif
