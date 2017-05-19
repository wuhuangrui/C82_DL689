#include "stdafx.h"
#include "TransmitApi.h"
#include "CctTaskMangerOob.h"
#include "CctAPI.h"
#include "FaCfg.h"


//������͸���ɼ������������
//������@pTTaskCfg �������õ�Ԫ
//		@wMtrSn �����
//		@bMesgSn �������
//		@pbInBuf �������
//		@bInLen ������ݳ���
bool SaveTransAcqDataToTaskDB(TTaskCfg *pTTaskCfg, WORD wMtrSn, BYTE bMesgSn, const BYTE *pbInBuf, BYTE bInLen)
{
	TTime tTime, tNowTime;
	DWORD dwCurTime, dwStartTime, dwEndTime;
	BYTE bRecBuf[256];
	BYTE *p = bRecBuf;
	BYTE bTsa[16];
	BYTE bTsaLen;

	//͸���ɼ������ֶη��䣺 ����ִ��ʱ�䣨5�� + TSA(16) + ������ţ�1��+ ������Ӧʱ�䣨5��+ data

	//����ִ��ʱ�䣨5��
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
	p += 16;	//�̶�16���ֽڵ�ַ����

	//������ţ�1��
	*p++ = bMesgSn;

	//��Ӧʱ�䣨5��
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

//������У��͸����������Ƿ�Ϸ����ò����ĳ���
int CheckTransmitPara(BYTE *pbBuf)
{
	BYTE *pbBuf0 = pbBuf;
	BYTE bArryNum;
	BYTE bTsaLen;

	if (*pbBuf++ != DT_STRUCT)	//
		return -1;
	pbBuf++;
	if (*pbBuf++ != DT_UNSIGN)	//�������
		return -1;
	pbBuf++;
	if (*pbBuf++ != DT_ARRAY)	//�������ݼ�
		return -1;
	bArryNum = *pbBuf++;	
	for (BYTE bIndex=0; bIndex<bArryNum; bIndex++)	//��������
	{
		if (*pbBuf++ != DT_STRUCT)
			return -1;
		pbBuf++;	
		if (*pbBuf++ != DT_LONG_U)
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_TSA)	//ͨ�ŵ�ַ
			return -1;
		pbBuf++;	//����TSA����
		bTsaLen = *pbBuf++ + 1;
		pbBuf += bTsaLen;
		if (*pbBuf++ != DT_LONG_U)	//��ʼǰ�ű�
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_LONG_U)	//��ɺ�ű�
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_STRUCT)	//�������Ʊ�ʶ
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_BOOL)	//�ϱ�͸������������ȴ���������
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_LONG_U)	//�ȴ���������ʱ��
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_ENUM)	//����ȶԱ�ʶ
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_STRUCT)	//����ȶԲ���
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_UNSIGN)	//�����ֽ�
			return -1;
		pbBuf++;
		if (*pbBuf++ != DT_LONG_U)	//��ȡ��ʼ
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_LONG_U)	//��ȡ����
			return -1;
		pbBuf += 2;
		if (*pbBuf++ != DT_ARRAY)	//�������ļ�
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
			*pbBuf++;	//�������
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

//���������͸���ɼ�����
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

	iLen = CheckTransmitPara(pbPara);	//У�����ݵĺϷ���
	if (iLen == -1)
	{
		DTRACE(DB_FAPROTO, ("DoTransMethod127_Add(): CheckTransmitPara is erro!\n"));
		return -1;
	}

	pbPara++;	//struct	͸������
	pbPara++;	
	pbPara++;	//unsigned	�������
	bSchNo = *pbPara++;
	pbPara++;	//array
	BYTE bSchNum = *pbPara++;
	for (BYTE i=0; i<bSchNum; i++)	//�������ݸ���
	{
		memset((BYTE*)&tTransFilePara, 0, sizeof(tTransFilePara));
		pbPara++;	//struct
		pbPara++;
		pbPara++;	//long-unsigned ���
		tTransFilePara.wSn = OoOiToWord(pbPara);
		pbPara += 2;
		pbPara++;	//TSA	ͨ�ŵ�ַ
		pbPara++;	//����TSA����
		tTransFilePara.bTsaLen = *pbPara++ + 1;
		memcpy(tTransFilePara.bTsa, pbPara, tTransFilePara.bTsaLen);
		pbPara += tTransFilePara.bTsaLen;
		pbPara++;	//long_unsigned	��ʼ�ű�
		tTransFilePara.wStartScript = ByteToWord(pbPara);	
		pbPara += 2;
		pbPara++;	//long_unsigned �����ű�
		tTransFilePara.wStartScript = ByteToWord(pbPara);	
		pbPara += 2;
		pbPara++;	//struct �������Ʊ�־
		pbPara++;	
		pbPara++;	//bool �ϱ�͸������������ȴ���������
		tTransFilePara.tTSchCtrlFlg.fRptFlg = (*pbPara++==0)? false:true;
		pbPara++;	//long_unsigned �ȴ��������ĳ�ʱʱ�䣨�룩
		tTransFilePara.tTSchCtrlFlg.wMsgTimeOut = OoOiToWord(pbPara);	
		pbPara += 2;
		pbPara++;	//enum	����ȶԱ�ʶ
		tTransFilePara.tTSchCtrlFlg.bRltCmpFlg = *pbPara++;
		pbPara++;	//struct ����ȶԲ���
		pbPara++;
		pbPara++;	//unsigned �����ֽ�
		tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.bSpecByte = *pbPara++;
		pbPara++;	//long-unsigned ��ȡ��ʼ
		tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.wCutOutStart = ByteToWord(pbPara);
		pbPara += 2;		
		pbPara++;	//long-unsigned ��ȡ����
		tTransFilePara.tTSchCtrlFlg.tTRltCmpPara.wCutOutLen = ByteToWord(pbPara);
		pbPara += 2;

		memset(pszTabName, 0, sizeof(pszTabName));
#ifdef GW_OOB_PROTO_UPDATA_20170406
		MK_TRANS_PATH_NAME(pszTabName, bSchNo, tTransFilePara.wSn);
#else		
		MK_TRANS_PATH_NAME(pszTabName, bSchNo, tTransFilePara.bTsa);
#endif
		pbPara++;	//array	�������ļ�
		BYTE bMsgNum = *pbPara++;
		for (BYTE j=0; j<bMsgNum; j++)
		{
			memset((BYTE*)&tTransMsg, 0, sizeof(tTransMsg));
			pbPara++;	//DT_STRUCT ��������
			pbPara++;
			pbPara++;	//unsigned �������
			tTransMsg.bSn = *pbPara++;
			pbPara++;	//oct-string ��������
			tTransMsg.bMsgLen = *pbPara++;
			memcpy(tTransMsg.bMsgBuf, pbPara, tTransMsg.bMsgLen);
			pbPara += tTransMsg.bMsgLen;
			tTransFilePara.bMsgSnMask[tTransMsg.bSn/8] |= (1<<(tTransMsg.bSn%8));
			if (!PartWriteFile(pszTabName, TRANS_FILE_MSG_OFFSET(tTransMsg.bSn), (BYTE*)&tTransMsg, sizeof(tTransMsg)))	//���汨������
				return -1;
			memset(pszAllTableName, 0, sizeof(pszAllTableName));
			MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);
			if (!WriteFile(pszAllTableName, (BYTE*)pszTabName, PER_TABLE_NAME_SIZE))	//�������
				return -1;
		}
	}

	pbPara++;
	tTransFilePara.wStgCnt = OoOiToWord(pbPara);
	pbPara += 2;

	if (!PartWriteFile(pszTabName, 0, (BYTE*)&tTransFilePara, TRANS_FILE_HEAD_LEN))	//����֡ͷ��Ϣ
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

	//�������
	pbPara++;	//unsigned
	bSchNo = *pbPara++;
	//ͨ�ŵ�ַ
	pbPara++;	//TSA
	tTransFilePara.bTsaLen = *pbPara++;
	memcpy(tTransFilePara.bTsa, pbPara, tTransFilePara.bTsaLen);
	//�������Ʊ�ʶ
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
	//�������ļ�
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
		if (!PartWriteFile(pszTableName, TRANS_FILE_MSG_OFFSET(tTransMsg.bSn), (BYTE*)&tTransMsg, sizeof(tTransMsg)))	//���汨������
			return -1;
		memset(pszAllTableName, 0, sizeof(pszAllTableName));
		MK_TRANS_TABLE_NAME(pszAllTableName, bSchNo);
		if (!WriteFile(pszAllTableName, (BYTE*)pszTableName, PER_TABLE_NAME_SIZE))	//�������
			return -1;
	}

	if (!PartWriteFile(pszTableName, 0, (BYTE*)&tTransFilePara, TRANS_FILE_HEAD_LEN))	//����֡ͷ��Ϣ
		return -1;

	return pbPara-pbPara0;
}

//������ɾ��һ��������һ�鷽������
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
		//Э�黹δ��ӱ���ţ�����Ӧ��Ҫ�ӣ�����������
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

//������ɾ��һ��͸������
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

//���������͸��������
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

//��������ȡ͸�����䷽������
//������@�������
//		@piStart ���������ƫ��λ�ã��״δ���ʱΪ-1,��ֵ����ɺ����ڲ�ʵ�֣��ⲿ�����������
//		@pbMsgSn �״δ���ʱΪ0����ֵ����ɺ����ڲ�ʵ�֣��ⲿ����������� 
//		@pTransFilePara �������ݼ�����
//		@pTTransMsg ��������
//���أ�trueΪ�����񷽰���֧�ֻ�ȫ���������
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

	if (!PartReadFile(pszTmpTableName, 0, (BYTE*)pTransFilePara, TRANS_FILE_HEAD_LEN))	//��Ӧ���ļ�������
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