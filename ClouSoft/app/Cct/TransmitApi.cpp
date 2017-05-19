#include "stdafx.h"
#include "TransmitApi.h"
#include "CctTaskMangerOob.h"
#include "CctAPI.h"
#include "FaCfg.h"


//描述：透明采集方案入任务库
//参数：@pTTaskCfg 任务配置单元
//		@wMtrSn 表序号
//		@bMesgSn 报文序号
//		@pbInBuf 入库数据
//		@bInLen 入库数据长度
bool SaveTransAcqDataToTaskDB(TTaskCfg *pTTaskCfg, WORD wMtrSn, BYTE bMesgSn, const BYTE *pbInBuf, BYTE bInLen)
{
	TTime tTime, tNowTime;
	DWORD dwCurTime, dwStartTime, dwEndTime;
	BYTE bRecBuf[256];
	BYTE *p = bRecBuf;
	BYTE bTsa[16];
	BYTE bTsaLen;

	//透明采集方案字段分配： 方案执行时间（5） + TSA(16) + 报文序号（1）+ 报文响应时间（5）+ data

	//方案执行时间（5）
	GetTaskCurExeTime(pTTaskCfg, &dwCurTime, &dwStartTime, &dwEndTime);
	SecondsToTime(dwStartTime, &tTime);
	*p++ = tTime.nYear-2000;
	*p++ = tTime.nMonth;
	*p++ = tTime.nDay;
	*p++ = tTime.nHour;
	*p++ = tTime.nMinute;

	//TSA(16)
	memset(bTsa, 0, sizeof(bTsa));
	bTsaLen = GetMeterTsa(wMtrSn, bTsa);
	memcpy(p, bTsa, bTsaLen); 
	p += 16;	//固定16个字节地址长度

	//报文序号（1）
	*p++ = bMesgSn;

	//响应时间（5）
	GetCurTime(&tNowTime);
	*p++ = tNowTime.nYear-2000;	
	*p++ = tNowTime.nMonth;	
	*p++ = tNowTime.nDay;	
	*p++ = tNowTime.nHour;	
	*p++ = tNowTime.nMinute;	

	//data
	memcpy(p, pbInBuf, bInLen);	
	p += bInLen;

	return WriteCacheDataToTaskDB(pTTaskCfg->bSchNo, pTTaskCfg->bSchNo, bRecBuf, p-bRecBuf);
}

//描述：校验透明传输参数是否合法，该参数的长度
int CheckTransmitPara(BYTE *pbBuf)
{
	BYTE *pbBuf0 = pbBuf;
	BYTE bArryNum;
	BYTE bTsaLen;

	if (*pbBuf++ != DT_STRUCT)	//
		return -1;
	pbBuf++;
	if (*pbBuf++ != DT_UNSIGN)	//方案编号
		return -1;
	pbBuf++;
	if (*pbBuf++ != DT_ARRAY)	//方案内容集
		return -1;
	bArryNum = *pbBuf++;	
	for (BYTE bIndex=0; bIndex<bArryNum; bIndex++)	//方案内容
	{
		if (*pbBuf++ != DT_STRUCT)
			return -1;
		pbBuf++;	
		if (*pbBuf++ != DT_LONG_U)
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_TSA)	//通信地址
			return -1;
		pbBuf++;	//跳过TSA长度
		bTsaLen = *pbBuf++ + 1;
		pbBuf += bTsaLen;
		if (*pbBuf++ != DT_LONG_U)	//开始前脚本
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_LONG_U)	//完成后脚本
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_STRUCT)	//方案控制标识
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_BOOL)	//上报透明方案结果并等待后续报文
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_LONG_U)	//等待后续报文时间
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_ENUM)	//结果比对标识
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_STRUCT)	//结果比对参数
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_UNSIGN)	//特征字节
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_LONG_U)	//截取开始
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_LONG_U)	//截取结束
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_ARRAY)	//方案报文集
			return -1;
		BYTE bMsgLen;
		BYTE bMsgNum = *pbBuf++;
		for (BYTE j=0; j<bMsgNum; j++)
		{
			if (*pbBuf++ != DT_STRUCT)
				return -1;
			pbBuf++;
			if (*pbBuf++ != DT_UNSIGN)
				return -1;
			*pbBuf++;	//报文序号
			if (*pbBuf++ != DT_OCT_STR)
				return -1;
			bMsgLen = *pbBuf++;
			pbBuf += bMsgLen;
		}
	}
	if (*pbBuf++ != DT_LONG_U)
		return -1;
	pbBuf += 2;

	return pbBuf - pbBuf0;
}

//描述：添加透明采集参数
int DoTransMethod127_Add(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iLen;
	char pszAllTableName[PER_TABLE_NAME_SIZE];
	char pszTabName[PER_TABLE_NAME_SIZE];
	BYTE bSchNo;
	BYTE *pbPara0 = pbPara;
	BYTE bArryNum;
	TransFilePara tTransFilePara;
	TTransMsg tTransMsg;

	iLen = CheckTransmitPara(pbPara);	//校验数据的合法性
	if (iLen == -1)
	{
		DTRACE(DB_FAPROTO, ("DoTransMethod127_Add(): CheckTransmitPara is erro!\n"));
		return -1;
	}

	pbPara++;	//struct	透明方案
	pbPara++;	
	pbPara++;	//unsigned	方案编号
	bSchNo = *pbPara++;
	pbPara++;	//array
	BYTE bSchNum = *pbPara++;
	for (BYTE i=0; i<bSchNum; i++)	//方案内容个数
	{
		memset((BYTE*)&tTransFilePara, 0, sizeof(tTransFilePara));
		pbPara++;	//struct
		pbPara++;
		pbPara++;	//long-unsigned 序号
		tTransFilePara.wSn = OoOiToWord(pbPara);
		pbPara += 2;
		pbPara++;	//TSA	通信地址
		pbPara++;	//跳过TSA长度
		tTransFilePara.bTsaLen = *pbPara++ + 1;
		memcpy(tTransFilePara.bTsa, pbPara, tTransFilePara.bTsaLen);
		pbPara += tTransFilePara.bTsaLen;
		pbPara++;	//long_unsigned	开始脚本
		tTransFilePara.wStartScript = ByteToWord(pbPara);	
		pbPara += 2;
		pbPara++;	//long_unsigned 结束脚本
		tTransFilePara.wStartScript = ByteToWord(pbPara);	
		pbPara += 2;
		pbPara++;	//struct 方案控制标志
		pbPara++;	
		pbPara++;	//bool 上报透明方案结果并等待后续报文
		tTransFilePara.tTSchCtrlFlg.fRptFlg = (*pbPara++==0)? false:true;
		pbPara++;	//long_unsigned 等待后续报文超时时间（秒）
		tTransFilePara.tTSchCtrlFlg.wMsgTimeOut = OoOiToWord(pbPara);	
		pbPara += 2;
		pbPara++;	//enum	结果比对标识
		tTransFilePara.tTSchCtrlFlg.bRltCmpFlg = *pbPara++;
		pbPara++;	//struct 结果比对参数
		pbPara++;
		pbPara++;	//unsigned 特征字节
		tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.bSpecByte = *pbPara++;
		pbPara++;	//long-unsigned 截取开始
		tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.wCutOutStart = ByteToWord(pbPara);
		pbPara += 2;		
		pbPara++;	//long-unsigned 截取长度
		tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.wCutOutLen = ByteToWord(pbPara);
		pbPara += 2;

		memset(pszTabName, 0, sizeof(pszTabName));
#ifdef GW_OOB_PROTO_UPDATA_20170406
		MK_TRANS_PATH_NAME(pszTabName, bSchNo, tTransFilePara.wSn);
#else		
		MK_TRANS_PATH_NAME(pszTabName, bSchNo, tTransFilePara.bTsa);
#endif
		pbPara++;	//array	方案报文集
		BYTE bMsgNum = *pbPara++;
		for (BYTE j=0; j<bMsgNum; j++)
		{
			memset((BYTE*)&tTransMsg, 0, sizeof(tTransMsg));
			pbPara++;	//DT_STRUCT 方案报文
			pbPara++;
			pbPara++;	//unsigned 报文序号
			tTransMsg.bSn = *pbPara++;
			pbPara++;	//oct-string 报文内容
			tTransMsg.bMsgLen = *pbPara++;
			memcpy(tTransMsg.bMsgBuf, pbPara, tTransMsg.bMsgLen);
			pbPara += tTransMsg.bMsgLen;
			tTransFilePara.bMsgSnMask[tTransMsg.bSn/8] |= (1<<(tTransMsg.bSn%8));
			if (!PartWriteFile(pszTabName, TRANS_FILE_MSG_OFFSET(tTransMsg.bSn), (BYTE*)&tTransMsg, sizeof(tTransMsg)))	//保存报文内容
				return -1;
			memset(pszAllTableName, 0, sizeof(pszAllTableName));
			MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);
			if (!WriteFile(pszAllTableName, (BYTE*)pszTabName, PER_TABLE_NAME_SIZE))	//保存表名
				return -1;
		}
	}

	pbPara++;
	tTransFilePara.wStgCnt = OoOiToWord(pbPara);
	pbPara += 2;

	if (!PartWriteFile(pszTabName, 0, (BYTE*)&tTransFilePara, TRANS_FILE_HEAD_LEN))	//保存帧头信息
		return -1;

	return 0;
}

int DoTransAddMeterFrameMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TransFilePara tTransFilePara;
	TTransMsg tTransMsg;
	char pszTableName[PER_TABLE_NAME_SIZE];
	char pszAllTableName[PER_TABLE_NAME_SIZE];
	BYTE *pbPara0 = pbPara;
	BYTE bSchNo;

	//方案编号
	pbPara++;	//unsigned
	bSchNo = *pbPara++;
	//通信地址
	pbPara++;	//TSA
	tTransFilePara.bTsaLen = *pbPara++;
	memcpy(tTransFilePara.bTsa, pbPara, tTransFilePara.bTsaLen);
	//方案控制标识
	pbPara++;	//struct
	pbPara++;	
	pbPara++;	//bool
	tTransFilePara.tTSchCtrlFlg.fRptFlg = (*pbPara++==0)? false:true;
	pbPara++;	//long_unsigned
	tTransFilePara.tTSchCtrlFlg.wMsgTimeOut = ByteToWord(pbPara);	
	pbPara += 2;
	pbPara++;	//enum
	tTransFilePara.tTSchCtrlFlg.bRltCmpFlg = *pbPara++;
	pbPara++;	//struct
	pbPara++;
	pbPara++;	//unsigned
	tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.bSpecByte = *pbPara++;
	pbPara++;	//long-unsigned
	tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.wCutOutStart = ByteToWord(pbPara);
	pbPara += 2;		
	pbPara++;	//long-unsigned
	tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.wCutOutLen = ByteToWord(pbPara);
	pbPara += 2;
	//方案报文集
	memset(pszTableName, 0, sizeof(pszTableName));
#ifdef GW_OOB_PROTO_UPDATA_20170406
	MK_TRANS_PATH_NAME(pszTableName, bSchNo, tTransFilePara.wSn);
#else		
	MK_TRANS_PATH_NAME(pszTableName, bSchNo, tTransFilePara.bTsa);
#endif
	pbPara++;	//array
	BYTE bMsgNum = *pbPara++;
	for (BYTE j=0; j<bMsgNum; j++)
	{
		memset((BYTE*)&tTransMsg, 0, sizeof(tTransMsg));
		pbPara++;	//unsigned
		tTransMsg.bSn = *pbPara++;
		pbPara++;	//oct-string
		tTransMsg.bMsgLen = *pbPara++;
		memcpy(tTransMsg.bMsgBuf, pbPara, tTransMsg.bMsgLen);
		tTransFilePara.bMsgSnMask[tTransMsg.bSn/8] |= (1<<(tTransMsg.bSn%8));
		if (!PartWriteFile(pszTableName, TRANS_FILE_MSG_OFFSET(tTransMsg.bSn), (BYTE*)&tTransMsg, sizeof(tTransMsg)))	//保存报文内容
			return -1;
		memset(pszAllTableName, 0, sizeof(pszAllTableName));
		MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);
		if (!WriteFile(pszAllTableName, (BYTE*)pszTableName, PER_TABLE_NAME_SIZE))	//保存表名
			return -1;
	}

	if (!PartWriteFile(pszTableName, 0, (BYTE*)&tTransFilePara, TRANS_FILE_HEAD_LEN))	//保存帧头信息
		return -1;

	return pbPara-pbPara0;
}

//描述：删除一个方案的一组方案内容
int DoTransDelSchMtrAddrMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	char pszTableName[PER_TABLE_NAME_SIZE];
	char pszAllTableName[PER_TABLE_NAME_SIZE];
	BYTE bTsaLen;
	BYTE bTsa[16];
	BYTE *pbPara0 = pbPara;
	BYTE bSchNo;

	pbPara++;	//unsigned
	bSchNo = *pbPara++;
	pbPara++;	//array
	BYTE bTsaNum = *pbPara++;
	for (BYTE i=0; i<bTsaNum; i++)
	{
		WORD wSn;
		//协议还未添加表序号，这里应该要加，先这样处理
#ifdef GW_OOB_PROTO_UPDATA_20170406
		pbPara++;
		wSn = OoOiToWord(pbPara);
		pbPara += 2;
#else
#endif
		pbPara++;	//TSA
		pbPara++;
		bTsaLen = *pbPara++ + 1;
		memset(bTsa, 0, sizeof(bTsa));
		memcpy(bTsa, pbPara, bTsaLen);
		pbPara += bTsaLen;
		memset(pszTableName, 0, sizeof(pszTableName));
#ifdef GW_OOB_PROTO_UPDATA_20170406
		MK_TRANS_PATH_NAME(pszTableName, bSchNo, wSn);
#else		
		MK_TRANS_PATH_NAME(pszTableName, bSchNo, bTsa);
#endif
		DeleteFile(pszTableName);
		MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);
		DWORD dwOffset = 0;
		do {
			char pszTmpTableName[PER_TABLE_NAME_SIZE];

			memset(pszTmpTableName, 0, sizeof(pszTmpTableName));
			MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);
			if (!PartReadFile(pszAllTableName, dwOffset, (BYTE*)pszTmpTableName, PER_TABLE_NAME_SIZE))
				break;
			if (memcmp(pszTmpTableName, pszTableName, PER_TABLE_NAME_SIZE) == 0)
			{
				BYTE bBufZero[PER_TABLE_NAME_SIZE];
				DeleteFile(pszTmpTableName);	
				memset(bBufZero, 0, sizeof(bBufZero));
				PartWriteFile(pszAllTableName, dwOffset, bBufZero, PER_TABLE_NAME_SIZE);	
			}
			dwOffset += PER_TABLE_NAME_SIZE;

		}while (1);
	}

	return pbPara-pbPara0;
}

//描述：删除一组透明方案
int DoTransDelGroupSchMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	char pszAllTableName[PER_TABLE_NAME_SIZE];
	BYTE bTsaLen;
	BYTE bTsa[16];
	BYTE *pbPara0 = pbPara;
	BYTE bSchNo;

	pbPara++;	//array
	BYTE bSchNoNum = *pbPara++;
	for (BYTE i=0; i<bSchNoNum; i++)
	{
		pbPara++;	//unsigned
		bSchNo = *pbPara++;
		MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);
		DWORD dwOffset = 0;
		do {
			char *pcRet;
			char pszTmpTableName[PER_TABLE_NAME_SIZE];

			memset(pszTmpTableName, 0, sizeof(pszTmpTableName));
			if (!PartReadFile(pszAllTableName, dwOffset, (BYTE*)pszTmpTableName, PER_TABLE_NAME_SIZE))
				break;
			char *pszSearchStr = "bSchNo";
			if ((pcRet=strstr(pszTmpTableName, pszSearchStr)) != NULL)
			{
				char pszNumStr[3];
				BYTE bTmpSchNo;
				memcpy(pszNumStr, pcRet+strlen(pszSearchStr), sizeof(pszNumStr));
				bTmpSchNo = atoi(pszNumStr);
				if (bTmpSchNo == bSchNo)
					DeleteFile(pszTmpTableName);
			}
			dwOffset += PER_TABLE_NAME_SIZE;
		}while (1);
	}

	return pbPara-pbPara0;
}

//描述：清空透明方案集
int DoTransClearMethod131(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	DWORD dwOffset;
	char pszAllTableName[PER_TABLE_NAME_SIZE];
	BYTE *pbPtr = pbPara;

	for (BYTE bSchNo=0; bSchNo<255; bSchNo++)
	{
		dwOffset = 0;
		do {
			char pszTmpTableName[PER_TABLE_NAME_SIZE];

			memset(pszTmpTableName, 0, sizeof(pszTmpTableName));
			MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);
			if (!PartReadFile(pszAllTableName, dwOffset, (BYTE*)pszTmpTableName, PER_TABLE_NAME_SIZE))
				break;
			if (!IsAllAByte((BYTE*)pszTmpTableName, 0, PER_TABLE_NAME_SIZE))
				DeleteFile(pszTmpTableName);
			dwOffset += PER_TABLE_NAME_SIZE;
		}while (1);
	}

	return pbPtr-pbPara;
}

//描述：获取透明传输方案参数
//参数：@方案编号
//		@piStart 任务表名的偏移位置，首次传入时为-1,该值变更由函数内部实现，外部调用无须更改
//		@pbMsgSn 首次传入时为0，该值变更由函数内部实现，外部调用无须更改 
//		@pTransFilePara 方案内容集参数
//		@pTTransMsg 方案报文
//返回：true为该任务方案不支持或全部抄读完成
bool GetTransSchParam(BYTE bSchNo, int *piStart, int *piMsgSn, TransFilePara *pTransFilePara, TTransMsg *pTTransMsg)
{
	char pszAllTableName[PER_TABLE_NAME_SIZE];
	char pszTmpTableName[PER_TABLE_NAME_SIZE];

	memset(pszAllTableName, 0, sizeof(pszAllTableName));
	MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);

	if (*piStart == -1)
		*piStart = 0;
label:
	memset(pszTmpTableName, 0, sizeof(pszTmpTableName));
	if (!PartReadFile(pszAllTableName, *piStart, (BYTE*)pszTmpTableName, PER_TABLE_NAME_SIZE))
		return true;

	if (IsAllAByte((BYTE*)pszTmpTableName, 0, PER_TABLE_NAME_SIZE))	
	{
		*piStart += (*piStart+1);
		*piMsgSn = 0;
		goto label;
	}

	if (!PartReadFile(pszTmpTableName, 0, (BYTE*)pTransFilePara, TRANS_FILE_HEAD_LEN))	//对应的文件不存在
	{
		*piStart += (*piStart+1);
		*piMsgSn = 0;
		goto label;
	}

label2:
	*piMsgSn = *piMsgSn + 1;
	if (!(pTransFilePara->bMsgSnMask[*piMsgSn/8] & (1<<(*piMsgSn%8))))
	{
		if (*piMsgSn >= 255)
		{
			*piStart += (*piStart+1);
			*piMsgSn = 0;
			goto label;
		}
		goto label2;
	}

	if (!PartReadFile(pszTmpTableName, TRANS_FILE_MSG_OFFSET(*piMsgSn), (BYTE*)pTTransMsg, TRANS_FILE_MSG_LEN))	
		goto label2;

	return false;
}