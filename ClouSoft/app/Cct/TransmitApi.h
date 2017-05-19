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
	BYTE bSn;	//报文序号
	BYTE bMsgLen;	//报文的有效长度
	BYTE bMsgBuf[256];	//报文内容
}
#ifdef SYS_WIN
TTransMsg;	//方案报文内容
#else
__attribute__((packed)) TTransMsg;	//方案报文内容
#endif

typedef struct {
	BYTE bSpecByte;	//特征字节
	WORD wCutOutStart;	//截取开始位置
	WORD wCutOutLen;	//截取长度
}
#ifdef SYS_WIN
TRltCmpPara;	//结果比对参数
#else
__attribute__((packed)) TRltCmpPara;	//结果比对参数
#endif

typedef struct 
{
	bool fRptFlg;	//上报透明方案结果并等待后续报文
	WORD wMsgTimeOut;	//等待报文超时时间
	BYTE bRltCmpFlg;	//结果比对标识
	TRltCmpPara tTRltCmpPara;
}
#ifdef SYS_WIN
TSchCtrlFlg;	//方案控制标识
#else
__attribute__((packed))TSchCtrlFlg;	//方案控制标识
#endif

typedef struct 
{
	WORD wSn;	//序号
	BYTE bTsaLen;
	BYTE bTsa[16];
	WORD wStartScript;	//开始前脚本id
	WORD wEndScript;	//完成后脚本id
	TSchCtrlFlg tTSchCtrlFlg;
	WORD wStgCnt;	//存储深度
	BYTE bMsgSnMask[32];	//传输报文序号屏蔽字
	TTransMsg tTransMsg;	//传输报文1
}
#ifdef SYS_WIN
TransFilePara;	//传输文件参数
#else
__attribute__((packed))TransFilePara;	//传输文件参数
#endif

#ifdef GW_OOB_PROTO_UPDATA_20170406
#define MK_TRANS_PATH_NAME(pszTableName, bSchNo, wSn)	(sprintf(pszTableName, "%sTranAcqSch_SchNo_%03d_Sn_%04d.para", USER_DATA_PATH, bSchNo, wSn))
#else
#define MK_TRANS_PATH_NAME(pszTableName, bSchNo, bTsa)	(sprintf(pszTableName, "%sTranAcqSch_SchNo_%03d_TSA_%02x%02x%02x%02x%02x%02x.para", USER_DATA_PATH,bSchNo,bTsa[5],bTsa[4],bTsa[3],bTsa[2],bTsa[1],bTsa[0]))
#endif

#define PER_TABLE_NAME_SIZE		64	//透传方案表名的长度
#define MK_TRANS_TABLE_NAME(pszTableName, bSchNo)	(sprintf(pszTableName, "%sTransTableName_bSchNo%03d.map", USER_DATA_PATH, bSchNo))	//所有透传方案表名（方案编号、地址）

#define TRANS_FILE_HEAD_LEN		offsetof(TransFilePara, tTransMsg)
#define TRANS_FILE_MSG_LEN		sizeof(TTransMsg)
#define TRANS_FILE_MSG_OFFSET(bMsgSn)	(TRANS_FILE_HEAD_LEN + bMsgSn*TRANS_FILE_MSG_LEN)

//描述：透明采集方案入任务库
//参数：@pTTaskCfg 任务配置单元
//		@wMtrSn 表序号
//		@bMesgSn 报文序号
//		@pbInBuf 入库数据
//		@bInLen 入库数据长度
bool SaveTransAcqDataToTaskDB(TTaskCfg *pTTaskCfg, WORD wMtrSn, BYTE bMesgSn, const BYTE *pbInBuf, BYTE bInLen);

//描述：添加更新一个透明方案或添加一组方案内容
int DoTransMethod127_Add(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：添加一组报文
int DoTransAddMeterFrameMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：删除一个方案的一组方案内容
int DoTransDelSchMtrAddrMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：删除一组透明方案
int DoTransDelGroupSchMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：清空透明方案集
int DoTransClearMethod131(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：获取透明传输方案参数
bool GetTransSchParam(BYTE bSchNo, int *piStart, int *piMsgSn, TransFilePara *pTransFilePara, TTransMsg *pTTransMsg);
#endif