#ifndef TRANSMITAPI_H
#define TRANSMITAPI_H

#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include "DbConst.h"
#include "FaCfg.h"
#include "sysdebug.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "CctTaskMangerOob.h"

typedef struct {
	BYTE bSn;	//�������
	BYTE bMsgLen;	//���ĵ���Ч����
	BYTE bMsgBuf[256];	//��������
}
#ifdef SYS_WIN
TTransMsg;	//������������
#else
__attribute__((packed)) TTransMsg;	//������������
#endif

typedef struct {
	BYTE bSpecByte;	//�����ֽ�
	WORD wCutOutStart;	//��ȡ��ʼλ��
	WORD wCutOutLen;	//��ȡ����
}
#ifdef SYS_WIN
TRltCmpPara;	//����ȶԲ���
#else
__attribute__((packed)) TRltCmpPara;	//����ȶԲ���
#endif

typedef struct 
{
	bool fRptFlg;	//�ϱ�͸������������ȴ���������
	WORD wMsgTimeOut;	//�ȴ����ĳ�ʱʱ��
	BYTE bRltCmpFlg;	//����ȶԱ�ʶ
	TRltCmpPara tTRltCmpPara;
}
#ifdef SYS_WIN
TSchCtrlFlg;	//�������Ʊ�ʶ
#else
__attribute__((packed))TSchCtrlFlg;	//�������Ʊ�ʶ
#endif

typedef struct 
{
	WORD wSn;	//���
	BYTE bTsaLen;
	BYTE bTsa[16];
	WORD wStartScript;	//��ʼǰ�ű�id
	WORD wEndScript;	//��ɺ�ű�id
	TSchCtrlFlg tTSchCtrlFlg;
	WORD wStgCnt;	//�洢���
	BYTE bMsgSnMask[32];	//���䱨�����������
	TTransMsg tTransMsg;	//���䱨��1
}
#ifdef SYS_WIN
TransFilePara;	//�����ļ�����
#else
__attribute__((packed))TransFilePara;	//�����ļ�����
#endif

#ifdef GW_OOB_PROTO_UPDATA_20170406
#define MK_TRANS_PATH_NAME(pszTableName, bSchNo, wSn)	(sprintf(pszTableName, "%sTranAcqSch_SchNo_%03d_Sn_%04d.para", USER_DATA_PATH, bSchNo, wSn))
#else
#define MK_TRANS_PATH_NAME(pszTableName, bSchNo, bTsa)	(sprintf(pszTableName, "%sTranAcqSch_SchNo_%03d_TSA_%02x%02x%02x%02x%02x%02x.para", USER_DATA_PATH,bSchNo,bTsa[5],bTsa[4],bTsa[3],bTsa[2],bTsa[1],bTsa[0]))
#endif

#define PER_TABLE_NAME_SIZE		64	//͸�����������ĳ���
#define MK_TRANS_TABLE_NAME(pszTableName, bSchNo)	(sprintf(pszTableName, "%sTransTableName_bSchNo%03d.map", USER_DATA_PATH, bSchNo))	//����͸������������������š���ַ��

#define TRANS_FILE_HEAD_LEN		offsetof(TransFilePara, tTransMsg)
#define TRANS_FILE_MSG_LEN		sizeof(TTransMsg)
#define TRANS_FILE_MSG_OFFSET(bMsgSn)	(TRANS_FILE_HEAD_LEN + bMsgSn*TRANS_FILE_MSG_LEN)

//������͸���ɼ������������
//������@pTTaskCfg �������õ�Ԫ
//		@wMtrSn �����
//		@bMesgSn �������
//		@pbInBuf �������
//		@bInLen ������ݳ���
bool SaveTransAcqDataToTaskDB(TTaskCfg *pTTaskCfg, WORD wMtrSn, BYTE bMesgSn, const BYTE *pbInBuf, BYTE bInLen);

//��������Ӹ���һ��͸�����������һ�鷽������
int DoTransMethod127_Add(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//���������һ�鱨��
int DoTransAddMeterFrameMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//������ɾ��һ��������һ�鷽������
int DoTransDelSchMtrAddrMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//������ɾ��һ��͸������
int DoTransDelGroupSchMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//���������͸��������
int DoTransClearMethod131(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//��������ȡ͸�����䷽������
bool GetTransSchParam(BYTE bSchNo, int *piStart, int *piMsgSn, TransFilePara *pTransFilePara, TTransMsg *pTTransMsg);
#endif