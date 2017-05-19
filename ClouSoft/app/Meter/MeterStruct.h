/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterStruct.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֳ���������ݽṹ����
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��9��
 *********************************************************************************************************/
#ifndef METERSTRUCT_H
#define METERSTRUCT_H
#include "Comm.h"
#include "MeterConst.h"
#include "apptypedef.h"
#include "ComStruct.h"
#include "MtrExc.h"

typedef struct {
	BYTE bUnit;	//�����λ
	WORD wVal;	//���ֵ
}TTimeInterv;	//ʱ����

typedef struct{
	DWORD dwOAD;		//object attrib, ��WORD��OI����WORD������ֵ
	WORD  wID;		//��Ӧ97645���ڲ�ID
	BYTE* pFmt;		//��ʽ������
	WORD  wFmtLen;	//��ʽ����������
	WORD  wOoLen;
	WORD  w645Len;
}Toad645Map;

typedef struct{
	DWORD dwEvtOAD;					//�¼�OAD
	DWORD dwErcNumID;				//�¼�����ID
	BYTE  bNum;						//�س���ID����
	DWORD dwRdID[6];		//�س���ID�б�
}TErcRdCtrl;	//ȫ�¼��ɼ��س�����ƽṹ

//������
typedef struct
{
	WORD	wPn;				//�������		
	BYTE	bProId;				//Э���
	BYTE	bSubProId;			//��Э���
	BYTE	bRateNum;			//������
	BYTE	bAddr[17];			//���ַ	
	BYTE	bPassWd[6];			//����
	BYTE	bRateTab[4];		//����˳��
#ifdef EN_SBJC_V2
    BYTE	bMtrAddr[7];		//ˮ�����ַ
#endif    
	CComm* pComm;
	TCommPara CommPara;			//����ͨ�Ų���	
}TMtrPara; 

typedef struct 
{
	DWORD 	dwOAD;	
	WORD	wOffset;	//��bBuf�е�ƫ��
	BYTE		bLen;	//����
	BYTE		bValid;	//1��Ч��0��Ч
}TMtrItemMem; //�����ʱ�����������ƽṹ

#define MTR_TMP_ITEM_NUM		128		//II�ͼ����������ٵ�
#define MTR_TMP_BUF_SIZE			4096	//II�ͼ����������ٵ�

#define MTR_ADDR_LEN				17		//����1 + ��ַ16

#define LOOP_MAX_CNT				2

typedef struct 
{
	DWORD 	dwTime;	//��ʱ���ݵļ��ʱ��
	TMtrItemMem item[MTR_TMP_ITEM_NUM];
	BYTE bBuf[MTR_TMP_BUF_SIZE];
}TMtrTmpData; //�����ʱ����

typedef struct 
{
	BYTE	bTaskIdx;		//������������Ե���taskSucFlg
	WORD	wItemIdx;	//����������һ�������������
	BYTE	bLoopCnt;	//���ַ��Ӧ���������������������ʱ����ΪLOOP_MAX_CNT
}TSchItem; //�������������������

typedef struct 
{
	BYTE bReqType;	//�������ͣ�[1] GetRequestNormal��[3] GetRequestRecord

	DWORD dwOAD;	//��������������
	WORD wRsdLen;	
	BYTE bRSD[128];	//��¼ѡ��������
	WORD wRcsdLen;
	BYTE bRCSD[128];	//��¼��ѡ��������
	BYTE bCSD[134];
	WORD wRcsdIdx;	//RCSD�й���OAD��ƫ��, �ز�ʹ��

	TTimeInterv tiExe;	 //ִ��Ƶ��

	DWORD dwEvtCnt;		//��һ�γ��������¼�����
}TRdItem; //����������

//#define ITEM_RD_TIME_NUM	3

//�����¼��ṹ
typedef struct{
	BYTE bState;	//״̬��
	BYTE bReserve;	//�����ֽ�
}TMtrClockErr;	//���ܱ�ʱ�䳬��

typedef struct{
	BYTE bState;		//״̬��
  	BYTE bAddr[MTR_ADDR_LEN];
	uint64 ui64PosE;		//�����������й�
	uint64 ui64NegE;		//�����ڷ����й�
	bool fInvalid[2];
}TMtrEnergyDec;//���ܱ�ʾ���½��¼�

typedef struct{
	BYTE bState;		//״̬��
  	BYTE bAddr[MTR_ADDR_LEN];
	uint64 ui64PosE;		//�����������й�
	uint64 ui64NegE;		//�����ڷ����й�
	DWORD dwSeconds[2];	//����ʼ����ʱ��
	bool fInvalid[2];
}TMtrFlew;//����������


typedef struct{
	BYTE bState;		//״̬��
  	BYTE bAddr[MTR_ADDR_LEN];
	uint64 ui64PosE;		//�����������й�
	uint64 ui64NegE;		//�����ڷ����й�
	DWORD dwSeconds[2];	//����ʼ����ʱ��
	bool fInvalid[2];
}TMtrEnergyErr;//����������


typedef struct{
	BYTE bState;		//״̬��
  	BYTE bAddr[MTR_ADDR_LEN];
	uint64 ui64PosE;		//�����������й�
	uint64 ui64NegE;		//�����ڷ����й�
	DWORD dwSeconds;
	bool fInvalid;
}TMtrStop;//���ܱ�ͣ���¼�


typedef struct{
	BYTE bState;	//״̬��
	BYTE bReserve;	//�����ֽ�
}TMtrRdFail;	//����ʧ��

#define MTEDATACHG_CSD_LEN	60
#define MTEDATACHG_DATA_LEN	200

typedef struct{
	BYTE bState;		//״̬��
  	BYTE bAddr[MTR_ADDR_LEN];
	BYTE bOldCSD[MTEDATACHG_CSD_LEN];		//������ݶ���  CSD,�����ж��Ƿ������и��£����и��£���Ҫ����ˢ��DATA���ݣ�Ȼ������ж��Ƿ�������¼�
	BYTE bCSD[MTEDATACHG_CSD_LEN];			//������ݶ���  CSD
	BYTE bOldData[MTEDATACHG_DATA_LEN];	//�仯ǰ����    Data
	BYTE bNewData[MTEDATACHG_DATA_LEN];	//�仯������    Data
}TMtrDataChg;//���ܱ����ݱ����ؼ�¼

typedef struct
{
	DWORD dwItemRdTime[ITEM_RD_TIME_NUM];	//���������С����С�ʱ��
											//��SaveMtrDataHook()�и��±���Ա
	WORD  wLastRecPhyIdx[MTR_EXC_NUM];		//�����Ǹ������¼������һ�μ�¼�Ĵ洢λ��
	DWORD dwLastStatClick[MTR_EXC_NUM];		//�����Ǹ������¼���ͳ��ʱ��
	BYTE  bTryReadCnt[MTR_EXC_NUM];		//�����Ǹ������¼��ĳ��Գ������
											
	TMtrClockErr mtrClockErr;  //���ܱ�ʱ�ӳ����¼�
	TMtrEnergyErr mtrEnergyErr; //�����������¼�
	TMtrEnergyDec mtrEnergyDec; //���ܱ�ʾ���½�
	TMtrStop mtrStop;	//���ܱ�ͣ��
	TMtrFlew mtrFlew;	//���ܱ����
	TMtrRdFail mtrRdFail;
	TMtrDataChg mtrDataChg;	//���ܱ����ݱ����ؼ�¼
}TMtrExcTmp; //����¼���ʱ����



#define TASK_SUC_FLG_LEN 8

#define ALLOC_TAB_LEN		20//16		//��̬�ڴ�������ֽڳ���
#define MEMORY_BLOCK_SIZE	64	//ÿ���С128���ֽ�
#define MTR_TAB_NUM 		96		//��̬�ڴ��������

#define MTR_TASK_NUM 64
#define MTR_CACHE_NUM  20


typedef struct
{
	BYTE 	bValid;		//1��Ч��0��Ч
	BYTE 	bTaskId;	//����ID
	bool	fRecSaved;	//�����¼�Ƿ��Ѿ����
	bool	fReRd;		//�Ƿ��ǲ���
	BYTE	bCSDItemNum; //�����賭��CSD����
	DWORD 	dwTime;		//����ִ��ʱ��
	BYTE 	bSucFlg[TASK_SUC_FLG_LEN];	//�����ɹ���ʶλ��ÿһλ��1��ʾ�ɹ����������������еĶ�Ӧ������
}TTaskSucFlg; //���񳭶��ɹ���־

//��̬�ڴ����Ͷ���
#define MEM_TYPE_NONE				0    //��ʹ�ö�̬�ڴ�
#define MEM_TYPE_TASK				1    //�����¼
#define MEM_TYPE_MTREXC			2    //�����쳣
#define MEM_TYPE_EVT_ACQ  		3	 //ȫ�¼��ɼ�����ʱ����
#define MEM_TYPE_TERM_EVTREC		4	 //�ն��ڲ��¼���¼�������ڱ���ն��¼�
#define MEM_TYPE_TERM_EVTITEM		5	 //�ն��¼�����ʱ������
#define MEM_TYPE_CURVE_FLG		6	 //���߳ɹ���־

#define MTR_MEM_SIZE 	10240//8192

typedef struct 
{
	DWORD	dwId;		//��1�ֽ�Ϊ����
	//��3�ֽ�Ϊ�������ݹ�����dwId�ĵ�3�ֽ�,�ֱ�Ϊ��
	//�����¼:����ID��
	//�����쳣:�¼�OI
	//�ն��ڲ��¼�:�¼�OI+��������
	//ȫ�¼��ɼ����ɼ��������
	WORD	wDataLen;	//���ݵ���Ч����
	BYTE 	bAllocTab[ALLOC_TAB_LEN];	//��ʱ��¼�����
	//ÿһλ��1��ʾռ����Ӧ��64���ֽڿռ䣬8192�Ŀռ乲��16���ֽ�
}TAllocTab; //��ͨ�ɼ�������ʱ��¼�����

typedef struct
{
	BYTE  bChkSum;	//��bTsa���ṹ��β��У���
	BYTE  bTsa[17];		//���ַ������У�����ַ�Ƿ����ı�
	BYTE	bTaskSN;	//�������õ����кţ������Ƚ��Ƿ��������÷����˸ı�
	//ϵͳ���б��浱ǰ���µ����к�
	//������ִ��ǰ���ж��������к��Ƿ����ı䣬
	//��������ı䣬���ϸ���bTaskSN��
	//�ٵ���taskSucFlg����ʱ��¼�ռ�ķ���

	TSchItem schItem; 			//�������������������
	TMtrTmpData  mtrTmpData;	//�����ʱ����
	TMtrExcTmp mtrExcTmp; //����¼���ʱ����

	TTaskSucFlg taskSucFlg[MTR_TASK_NUM];	//���񳭶��ɹ���ʶλ
	
	//��̬�ڴ����
	BYTE  bGlobal[ALLOC_TAB_LEN];	//ȫ�ֶ�̬�ڴ������൱��allocTab[]�Ļ� 
	TAllocTab allocTab[MTR_TAB_NUM];	//��̬�ڴ�����

	BYTE bMem[MTR_MEM_SIZE];	//��̬�ڴ����ռ�
}TMtrRdCtrl; //��������ƽṹ

#define CACHE_STATUS_FREE			0	//�հף�Ϊ����
#define CACHE_STATUS_IDLE			1	//���У�û�б�ĳ�������߳�ʹ��
#define CACHE_STATUS_INUSE		2	//���ã����ڱ�ĳ�������߳�ʹ��

typedef struct
{
	BYTE  bStatus;			   //״̬��CACHE_STATUS_FREE��
								//		CACHE_STATUS_IDLE��
								//		CACHE_STATUS_INUSE
	WORD wPn;					//�������
	BYTE  bTsa[17];		 //���ַ
	DWORD dwCacheTime;      //���浽�ڴ��ʱ��,����10����д���ļ�
	DWORD dwLastAcessTime;  //������ʱ�䣬������ʱ�������������û���ʵ�
	bool  fDirty;            //�Ƿ�Ϊ�࣬Ϊ��ʱ���б�Ҫ����
	bool fTrigerSave; //ǿ�ƴ��������־
	TMtrRdCtrl mtrRdCtrl; 	//�������������
}TMtrCacheCtrl;  //�������ƽṹ

extern TMtrCacheCtrl g_MtrCacheCtrl[MTR_CACHE_NUM];

#endif
