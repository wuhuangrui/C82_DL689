/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbOIAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ��ϵͳ���ݿ���������ID֮���ת��
 * ��ǰ�汾��1.0
 * ��    �ߣ��׳ɲ�
 * ������ڣ�2016��8��
 *********************************************************************************************************/
#include "stdafx.h"
#include "DbOIAPI.h"
#include "DbFmt.h"
#include "LibDbConst.h"
#include "FaConst.h"
#include "sysdebug.h"
#include "ComAPI.h"
#include "LibDbAPI.h"
#include "OIObjInfo.h"
#include "DbHook.h"
#include "DbAPI.h"
#include "CctTaskMangerOob.h"
#include "CctAPI.h"
#include "Trace.h"
#include "TaskManager.h"
#include "FrzTask.h"
#include "bios.h"
#include "FaProto.h"
#ifndef SYS_WIN
#include "AcSample.h"
#endif
#include "FaAPI.h"

BYTE g_bOIFmt[] = {0x09, 0x02, LRF};
extern int GetFileTransAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wBufSize, int* piStart);
extern int SetFileTransAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbPara);

//����: ȡĳ������Ը���
//����:@wClass	��Ҫ��ȡ���Ե����
//����: ��ȷ�򷵻���Ӧ������Ը���,���򷵻�0(�����ڵ���)
BYTE OIGetAttrNum(WORD wClass)
{
	BYTE iOIAttrNum[] = {	0, 5, 3, 3, 3,		//0~4
							6, 3, 9, 2, 6,		//5~9
							4, 4, 19, 5, 3,		//10~14
							3, 3, 4, 3, 8,		//15~19
							5, 13, 4, 18, 12,	//20~24
							11, 5, 0, 0, 0};	//25~29
	if (wClass <= 29)
		return iOIAttrNum[wClass];
	else
		return 0;
}


char* OIFmtErrToStr(WORD wErr)
{
	static char* pszFmtErr[] = {"err-ok",
								"err-type-mismatch",
								"err-null",
								"err-len", 
	                            "err-unknown-type"};
	if (wErr <= 4)
		return pszFmtErr[wErr];
	else
		return "err-unknown";
}

static BYTE g_bFillByte[] = {0x00, 0x00, 0xff, 0xee, 
							 0x00, 0x00, 0x00, 0x00};

#define OI_NULL_DATA		0	//������

#define FMT_ERR_OK			0	//�޴���
#define FMT_ERR_TYPE		1	//���Ͳ�����
#define FMT_ERR_NULL		2	//�����ݴ���
#define FMT_ERR_LEN			3	//���ȴ���
#define FMT_ERR_UNK_TYPE	4	//δ֪����


//��������ȡ��ʽ����Ч���ȣ�ȥ��RLF\RLV\LRF\LRV���ȣ�����Ӧg_OIConvertClass�е�fmt��ʽ
//������@pFmt ��Ӧg_OIConvertClass.fmt��ʽ
//		@wFmtLen ��Ӧg_OIConvertClass.wFmtLen��ʽ����
//		@pwRetFmtLen ���ظ�ʽ����Ч����
//���أ�-1ʧ��
int OoGetDataTypeFmtValidLen(BYTE *pFmt, WORD wFmtLen, WORD *pwRetFmtLen)
{
	int iLen;
	BYTE *pFmt0 = pFmt;
	BYTE bFmt;
	WORD wValidFmtLen = 0;
	WORD wTmpValidFmtLen = 0;

	bFmt = *pFmt++;
	wValidFmtLen++;
	switch (bFmt)
	{
	case DT_ARRAY:
		pFmt++;	//DT_ARRAY��Ա����
		wValidFmtLen++;
		iLen = OoGetDataTypeFmtValidLen(pFmt, wFmtLen-(pFmt-pFmt0), &wTmpValidFmtLen);
		if (iLen < 0)
			goto ERR_RET;
		pFmt += iLen;
		wValidFmtLen += wTmpValidFmtLen;
		break;
	case DT_BIT_STR:
		pFmt++;	//DT_BIT_STR��Ա����
		wValidFmtLen++;
		pFmt++;	//ȥ��RLF\RLV\LRF\LRV����
		break;
	//TODO:����Ҫ�����
	default:
		goto ERR_RET;
	}
	
	*pwRetFmtLen = wValidFmtLen;
	iLen = pFmt - pFmt0;
	pFmt = pFmt0;
	return iLen;
ERR_RET:
	return -1;
}


//����: ȡ���������ĳ���
//������@ bType	�������ͣ�ͬData���������Ͷ���
//	   @ pbSrc������Ŀ����������OAD��ROAD��CSD
//���أ���ȷ�򷵻�������ĳ��ȣ����򷵻ظ���
int OoGetDataTypeLen(BYTE* pbSrc)
{
	BYTE* pbSrc0 = pbSrc;	
	BYTE bChoice = *pbSrc++;
	int  i, nRet, Offset = 0;
	WORD wCnt = 0;
	BYTE* p = pbSrc;

	switch(bChoice)
	{
	case DT_NULL:
		Offset = 0;
		break;

	case DT_ARRAY:
		Offset = ScanArray(pbSrc, false);
		break;

	case DT_STRUCT:
		Offset = 1;		//ǰ��Ľṹ������0x02ռ��1���ֽ�
		wCnt = *p++;
		for (i=0; i<wCnt; i++)
		{
			nRet = OoGetDataTypeLen(p);
			if (nRet > 0)
			{
				Offset += nRet;
				p += nRet;
			}
			else
			{
				return -1;
			}
		}
		break;

	case DT_BOOL:
		Offset = 1;
		break;

	case DT_BIT_STR:
		Offset = *pbSrc/8 + 1;
		break;
	case DT_DB_LONG:
	case DT_DB_LONG_U:
		Offset = 4;
		break;

	case DT_OCT_STR:
	case DT_VIS_STR:
	case DT_UTF8_STR:
		Offset = *pbSrc + 1;
		break;

	case DT_INT:
	case DT_UNSIGN:
		Offset = 1;
		break;

	case DT_LONG:
	case DT_LONG_U:
		Offset = 2;
		break;

	case DT_LONG64:
	case DT_LONG64_U:
		Offset = 8;
		break;	

	case DT_ENUM:
		Offset = 1;
		break;

	case DT_FLOAT32:
		Offset = 4;
		break;

	case DT_FLOAT64:
		Offset = 8;
		break;

	case DT_DATE_TIME:
		Offset = 10;
		break;

	case DT_DATE:
		Offset = 5;
		break;

	case DT_TIME:
		Offset = 3;
		break;

	case DT_DATE_TIME_S:
		Offset = 7;
		break;

	case DT_OI:
		Offset = 2;
		break;

	case DT_OAD:
	case DT_OMD:
		Offset = 4;
		break;

	case DT_ROAD:
		Offset = ScanROAD(pbSrc, false);
		break;

	case DT_TI:
		//Offset = 5;
		Offset = 3;
		break;

	case DT_TSA:
	case DT_MAC:
		Offset = pbSrc[0]+1;
		break;

	case DT_RN:
		break;

	case DT_REGION:
		break;

	case DT_SCALE_UNIT:
		break;

	case DT_RSD:
		Offset = ScanRSD(pbSrc, false);
		break;

	case DT_CSD:
		Offset = ScanCSD(pbSrc, false);
		break;

	case DT_MS:
		Offset = ScanMS(pbSrc, false);
		break;

	case DT_SID:
		break;

	case DT_SID_MAC:
		break;

	case DT_COMDCB:
		Offset = 5;
		break;

	case DT_RCSD:
		Offset = ScanRCSD(pbSrc, false);
		break;

	default:
		return -1;
	}
	if (Offset >= 0)
		pbSrc += Offset;
	else
		return -1;

	nRet = pbSrc-pbSrc0;
	pbSrc = pbSrc0;
	return nRet;
}

//���������������������͵��ܳ���
int ScanArray(BYTE* pbSrc, bool fRevOrder)
{
	BYTE* pbSrc0 = pbSrc;
	BYTE bNumber = *pbSrc++;
	BYTE bType;
	int Offset = 0;
	int nRet;

	for (BYTE i=0; i<bNumber; i++)
	{
		bType = *pbSrc++;
		switch(bType)
		{
		case DT_NULL:
			Offset = 0;
			break;

		case DT_ARRAY:
			Offset = ScanArray(pbSrc, false);
			break;

		case DT_STRUCT:
			break;

		case DT_BOOL:
			Offset = 1;
			break;

		case DT_BIT_STR:
		case DT_DB_LONG:
		case DT_DB_LONG_U:
		case DT_OCT_STR:
		case DT_VIS_STR:
		case DT_UTF8_STR:
		case DT_INT:
		case DT_LONG:
		case DT_UNSIGN:
		case DT_LONG_U:
		case DT_LONG64:
		case DT_LONG64_U:
		case DT_ENUM:
		case DT_FLOAT32:
		case DT_FLOAT64:
		case DT_DATE_TIME:
		case DT_DATE:
		case DT_TIME:
		case DT_DATE_TIME_S:
			break;

		case DT_OI:
			Offset = 2;
			break;

		case DT_OAD:
		case DT_OMD:
			Offset = 4;
			break;

		case DT_ROAD:
			Offset = ScanROAD(pbSrc, false);
			break;
		
		case DT_TI:
			Offset = 5;
			break;

		case DT_TSA:
		case DT_MAC:
			Offset = pbSrc[0]+1;
			break;

		case DT_RN:
			break;

		case DT_REGION:
			break;

		case DT_SCALE_UNIT:
			break;

		case DT_RSD:
			Offset = ScanRSD(pbSrc, false);
			break;

		case DT_CSD:
			Offset = ScanCSD(pbSrc, false);
			break;

		case DT_MS:
			Offset = ScanMS(pbSrc, false);
			break;

		case DT_SID:
			break;

		case DT_SID_MAC:
			break;

		case DT_COMDCB:
			Offset = 5;
			break;

		case DT_RCSD:
			Offset = ScanRCSD(pbSrc, false);
			break;

		default:
			return -1;
		}
		if (Offset >= 0)
			pbSrc += Offset;
		else
			return -1;
	}

	nRet = pbSrc-pbSrc0;
	pbSrc = pbSrc0;
	return nRet;
}

//���������ܱ���MS����������ƫ����
//		��ΪMS���ݲ��ֶ���string����ʱ������ֱ��ת��ַ˳��
int ScanMS(BYTE* pbSrc, bool fRevOrder)
{
	BYTE* pbSrc0 = pbSrc;
	BYTE bChoice = *pbSrc++;
	DWORD dwNum=0;
	WORD wOffset;
	BYTE bLen;
	int nRet;

	switch(bChoice)
	{
	case 0:	//�޵��ܱ�
	case 1:	//ȫ���û���ַ
		break;

	case 2://һ���û�����
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		pbSrc += dwNum;//����unsigned, len=1
		break;

	case 3:	//һ���û���ַ
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while(dwNum > 0)
		{
			bLen = *pbSrc++;
			pbSrc += bLen;
			dwNum--;
		}
		break;

	case 4: //һ���������
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while (dwNum > 0)
		{
			pbSrc += 2;
			dwNum--;
		}
		break;

	case 5:	//һ���û��������䣬seq of Region
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while(dwNum>0)
		{
			pbSrc++;	//ö��ֵ
			pbSrc += 2;	//��ʼֵ��һ���û�����
			pbSrc += 2;	//����ֵ��һ���û�����
			dwNum--;
		}
		break;

	case 6:	//һ���û���ַ���䣬seq of Region
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while(dwNum>0)
		{
			pbSrc++;	//ö��ֵ
			//��ʼֵ
			pbSrc++;	//type=TSA
			bLen = *pbSrc++;
			pbSrc += bLen;
			//����ֵ
			pbSrc++;	//type=TSA
			bLen = *pbSrc++;
			pbSrc += bLen;

			dwNum--;
		}
		break;

	case 7:	//һ������������䣬seq of Region
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while(dwNum>0)
		{
			pbSrc++;	//ö��ֵ
			pbSrc += 3;	//��ʼֵ��һ���û�����
			pbSrc += 3;	//����ֵ��һ���û�����
			dwNum--;
		}
		break;

	default:
		break;
	}

	nRet = pbSrc-pbSrc0;
	pbSrc = pbSrc0;
	return nRet;
}

//������ȡ��RSD���ݵ�ȫ������
int ScanRSD(BYTE* pbSrc, bool fRevOrder)
{
	BYTE* pbSrc0 = pbSrc;
	BYTE bMathod = *pbSrc++;
	DWORD dwOAD;
	int nOffset;
	const ToaMap* pOI;
	BYTE bNum, bType, b;
	int nRet;
	WORD wLen;

	switch(bMathod)
	{
	case 1:
		if (*pbSrc == DT_OAD)	
			pbSrc++;

		pbSrc += 4;	//OAD
		pbSrc += OoGetDataTypeLen(pbSrc);	//����
		break;

	case 2:
		if (*pbSrc == DT_OAD)	
			pbSrc++;
		pbSrc += 4;	//OAD
		pbSrc += OoGetDataTypeLen(pbSrc);	//��ʼֵ
		pbSrc += OoGetDataTypeLen(pbSrc);	//����ֵ
		pbSrc += OoGetDataTypeLen(pbSrc);	//���ֵ
		break;

	case 3:
		bNum = *pbSrc++;
		if (bNum == 0)
			break;
		while(bNum-- > 0)
		{
			if (*pbSrc == DT_OAD)	
				pbSrc++;
			
			pbSrc += 4;	//OAD
			pbSrc += OoGetDataTypeLen(pbSrc);	//��ʼֵ
			pbSrc += OoGetDataTypeLen(pbSrc);	//����ֵ
			pbSrc += OoGetDataTypeLen(pbSrc);	//���ֵ
		}
		break;

	case 4:	//ָ�����ܱ��ϣ��ƶ��ɼ���ʼʱ��
	case 5:	//ָ�����ܱ��ϣ��ƶ��ɼ��洢ʱ��
		//pbSrc++;//��ʽ����DateTimeBCD
		pbSrc += 7;
		//MS����
		nRet = ScanMS(pbSrc, fRevOrder);	//�ں�����pbSrc�Զ�������
		if (nRet > 0)
			pbSrc += nRet;
		else
			return nRet;
		break;

	case 6:	//ָ������ϣ�ָ���ɼ�����ʱ���������������ֵ
	case 7:	//ָ������ϣ�ָ���ɼ��洢ʱ���������������ֵ
	case 8:	//ָ������ϣ�ָ���ɼ���ʱ���������������ֵ
		//pbSrc++;//DateTimeBCD----�ɼ�����ʱ����ʼֵ
		pbSrc += 7;
		//pbSrc++;//DateTimeBCD----�ɼ�����ʱ�����
		pbSrc += 7;
		//pbSrc++;	//TI
		//pbSrc += 2;	//enm ��λ+ֵ
		pbSrc += 1;	//��λ+ֵ
		//pbSrc += 3;	//���ֵ long-unsigned 
		pbSrc += 2;	//��λ+ֵ
		//MS����
		nRet = ScanMS(pbSrc, fRevOrder);	//�ں�����pbSrc�Զ�������
		if (nRet > 0)
			pbSrc += nRet;
		else
			return nRet;
		break;

	case 9:
		pbSrc += 1;	//��N�μ�¼ unsigned
		break;

	case 10:
		pbSrc += 1;	//��N����¼ unsigned
		//MS����
		nRet = ScanMS(pbSrc, fRevOrder);	//�ں�����pbSrc�Զ�������
		if (nRet > 0)
			pbSrc += nRet;
		else
			return nRet;
		break;

	case 0:
		break;//tll 
	default:
		return -1;
	}
	
	nRet = pbSrc-pbSrc0;
	pbSrc = pbSrc0;

	return nRet;
}

//������ȡ��CSD���ݵ�ȫ������
int ScanCSD(BYTE* pbSrc, bool fRevOrder)
{
	BYTE* pbSrc0 = pbSrc;
	BYTE bChoice = *pbSrc++;	//choice
	BYTE bNum;
	switch(bChoice)
	{
		case 0:	//OAD
			//pbSrc++;	//type OAD
			pbSrc+=4;	//OAD
			break;

		case 1:	//ROAD
			//pbSrc++;	//type ROAD
			pbSrc += 4;	//OAD
			bNum = *pbSrc++;
			pbSrc += (WORD )bNum*4;	//type ROAD(1Byte) + OAD(4Byte)
			break;

		default:
			return -1;
	}
	WORD wLen = pbSrc-pbSrc0;
	pbSrc = pbSrc0;

	return wLen;
}

//����:����ROAD����ĳ���
//����:������
int ScanROAD(BYTE* pbSrc, bool fRevOrder)
{
	BYTE *pbPtr = pbSrc;
	BYTE bROADCnt;

	pbPtr += 4;
	bROADCnt = *pbPtr;	pbPtr++;
	pbPtr += bROADCnt*4;

	return pbPtr-pbSrc;
}

//����:����RCSD����ĳ���
//����:������
int ScanRCSD(BYTE* pbSrc, bool fRevOrder)
{
	BYTE *pbPtr = pbSrc;
	BYTE bCSDNum;
	int iLen;

	bCSDNum = *pbPtr++;
	for (BYTE i = 0; i < bCSDNum; i++)
	{
		if ((iLen=ScanCSD(pbPtr, fRevOrder)) == -1)
			return -1;
		pbPtr += iLen;
	}
	
	return pbPtr-pbSrc;
}

//����:��ʽ������,�Ѿ�����ת���ɷ�����������ʽ������-------�ṩ�����ɿ��������Ӹ�ʽ
//����:@pbSrc Դ����,δ��ʽ��
//	   @pbDst Ŀ������,��ʽ��
//	   @pbFmt ��ʽ������
//	   @wFmtLen ��ʽ�������ĳ���
//����:�����ȷ�򷵻�ת����Ĵ���ʽ��Ŀ�����ݵĳ���,���򷵻ظ���
//��ע:
//	   >���ֿ������ڴ洢��ʱ�򶼲�����0(������)���洢��,���������
//		a.������:�������Ϊ0
//		b.����������,�����սṹ��boolean,long,long-unsigned��:����ֱ����OI_NULL_DATA
//		  ���洢��,
//	   >�����ݴ洢�ĸĽ�����
//		a.�Ժ�������ÿ����ʽ������,��һ���ֽ����������Ч�ֽڵĶ���,��Ϊ�޷�����
//		  �������ݶ���ͬ������Ч�ֽ�,���Բ�ͬ���͵������ò�ͬ����Ч�ֽ�,
//		b.���԰�1����ɲ�������Ч�ֽ�,���ڲ������͵�������˵,����Ӧ������õ�,����Ҫ�����
//		  ����,������ĳ�����Ի��߲����ṹ���ĳ���ֶ������Ч����
//	   >Ŀǰֻ֧�ֶԿ�����(�������Ϊ0)��0(������)���������,
//		��������һ�ɲ�֧���Զ�ʶ��Դ�����еĿ�����
//	   >���������ݵ�����Ҫ���ض����ⲿ���������,����LN�嵥,�������ݵ�
int OIFmtData(BYTE* pbSrc, BYTE* pbDst, BYTE* pbFmt, WORD wFmtLen)
{
	BYTE bStack[100];
	BYTE* pbSrc0 = pbSrc;
	BYTE* pbDst0 = pbDst;
	BYTE* pbFmtEnd = pbFmt + wFmtLen;
	int iSP = 0;	//��ջָ��
	BYTE bFP = 0;	//��ʽָ��
	BYTE bTmpFP;
	BYTE bLen, bSrcLen, bArrLen, bByteLen;
	BYTE bFmtT, bVFmt, bVFill;
	bool fVLen;
	BYTE bNullArr = 0;
	WORD wOffset;
	BYTE bSrcType = 0xff;//DT_UNCARE; //don't care
	int iNullSP = 0;	//������������(�������Ϊ0)ʱ��iSP,��ɨ����һ�����ݺ�,
						//iSP�ظ���iNullSP��ֵ,��ʾ�Կ�����Ĵ������
	bool fPop;
	WORD wErr = 0;
	
	do
	{
		fPop = true;
		BYTE bType = pbFmt[bFP++];
		
		switch (bType)
		{
		case 1: //array
			bLen = pbFmt[bFP++];
			if (bLen == SPECIAL_NUM)
				bLen = 1;
			bSrcLen = *pbSrc++;
			if (bSrcLen == 0)
				bSrcLen = bLen;

			if (bNullArr)	//��һ�������黹û��,�Ӽ����ǿ�����
			{
				bSrcLen = 0;
			}
			else
			{
				if (bSrcLen == 0)	//�������鳤��Ϊ0�Ŀ�����,��0(������)����
				{
					*pbDst++ = 1;
					*pbDst++ = 0;
					bNullArr = 1;
					iNullSP = iSP;
				}
				else
				{
					*pbDst++ = bType;
					*pbDst++ = bSrcLen;
					bNullArr = 0;
				}
			}
			
			wOffset = pbSrc - pbSrc0;	//Խ�����鳤�Ⱥ����ʼλ��
			
			bStack[iSP++] = bLen;		//�ն��д洢����ʱԤ���ļ�¼����,��������Դ�����л��ж��ٱʼ�¼û������
			bStack[iSP++] = wOffset;	//����Դ����������Ԫ�ص���ʼƫ��,��������һ�ʼ�¼�ĳ���
			bStack[iSP++] = wOffset>>8;

			bStack[iSP++] = bFP;
			bStack[iSP++] = bType;
			bStack[iSP++] = bSrcLen; //��ʹ����Ԫ�ظ���Ϊ0,ҲҪ��һ����ջ�Ĳ���,
									 //�ó���ɨ���Ԫ�صĸ�ʽ����
			fPop = false;
			break;

		case 2: //structure
			bLen = pbFmt[bFP++];
			bStack[iSP++] = bType;
			bStack[iSP++] = bLen;
			
			if (!bNullArr)
			{
				*pbDst++ = bType;
				*pbDst++ = bLen;
			}

			fPop = false;
			break;

		case 4: //bit-string
			bLen = pbFmt[bFP++];
			bVFmt = pbFmt[bFP++];
			bFmtT = 0xF0 & bVFmt;	//ZJD 20080116_1437
			bVFmt &= 0x0F;			//ZJD 20080116_1437
			bByteLen = (bLen + 7) / 8;
			fVLen = (bVFmt&VF_VLEN)!=0; 
			bVFill = bVFmt>>VF_FILL_SHF;

			if (bNullArr)
			{
				if (fVLen && bVFill!=0)
					pbSrc++;

				pbSrc += bByteLen;
			}
			else
			{
				//ȷ��bSrcLen�Ϳ�����bByteLen
				if (fVLen)
				{
					if (bVFill == 0) //0-�����,��һ�������ֽڷŵ���ͷ
					{
						bByteLen = *pbSrc++;
					}
					else	//������ֽ������
					{
						bVFill = g_bFillByte[bVFill];
						bByteLen = OIGetStrLen(pbSrc, bByteLen, bVFill);	//bByteLen�����ı�
							//����·���û��ȫ��������,������������ϴ���BITs�������ܸ��·��Ĳ�һ��
					}

					bSrcLen = bByteLen * 8;
				}
				else //����,�������,Ҫ�󳤶ȱ���պ����
				{
					bSrcLen = bLen;
				}
				
				//��֡
				*pbDst++ = bType;
				*pbDst++ = bSrcLen;		//���ݳ���
				if (bByteLen > 0)
				{
					if (bVFmt&VF_LR)
						memcpy(pbDst, pbSrc+1, bByteLen);		//���������ֽ�
					else
						revcpy(pbDst, pbSrc+1, bByteLen);

					//------�¼��ⲿ��-------------
					if (bFmtT != 0)
						for (BYTE b=0; b<bByteLen; b++)			//2008-05-25  ��
							pbDst[b] = ByteBitReverse(pbDst[b]);	//���ֽڰ�λ���е���
					//------�¼��ⲿ��-------------
					pbDst += bByteLen;
				}

				pbSrc += (bLen + 7) / 8 + 1;			//����+����
			}
			break;

		case 9:	//octet-string            	[9] 		IMPLICIT OCTET STRING,
		case 10: //visible-string
			bLen = pbFmt[bFP++];
			bVFmt = pbFmt[bFP++];
			fVLen = (bVFmt&VF_VLEN)!=0; 
			bVFill = bVFmt>>VF_FILL_SHF;

			if (bNullArr)
			{
				if (fVLen && bVFill!=0)
					pbSrc++;

				pbSrc += bLen;
			}
			else
			{
				if (fVLen)
				{
					if (bVFill == 0) //0-�����,��һ�������ֽڷŵ���ͷ
					{
						bSrcLen = *pbSrc++;
					}
					else	//������ֽ������
					{
						bVFill = g_bFillByte[bVFill];
						bSrcLen = OIGetStrLen(pbSrc, bLen, bVFill);
					}
				}
				else //����,�������,Ҫ�󳤶ȱ���պ����
				{
					bSrcLen = bLen;
				}

				*pbDst++ = bType;
				*pbDst++ = bSrcLen;		//���ݳ���
				if (bSrcLen > 0)
				{
					if (bVFmt&VF_LR)
						memcpy(pbDst, pbSrc, bSrcLen);
					else
						revcpy(pbDst, pbSrc, bSrcLen);

					pbDst += bSrcLen;
				}

				pbSrc += bLen;
			}
			break;

		case 3:  //boolean                	[3]	 	IMPLICIT BOOLEAN    
		case 13: //bcd                   	[13] 	IMPLICIT Integer8,
		case 15: //integer                	[15] 	IMPLICIT Integer8,
		case 17: //unsigned              	[17] 	IMPLICIT Unsigned8,
		case DT_ENUM: //enum 					[22] 	IMPLICIT NULL
			if (bNullArr)	//���ֽ�����Ŀǰ��֧����Ч����
			{
				pbSrc++;
			}
			else
			{
				*pbDst++ = bType;
				*pbDst++ = *pbSrc++;
			}
			
			break;

		case DT_LONG: //long                  	[16] 	IMPLICIT Integer16,
		case DT_LONG_U: //long-unsigned         	[18] 	IMPLICIT Unsigned16,
			if (bNullArr)
			{
				pbSrc += 2;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 2);
				pbDst += 2;
				pbSrc += 2;
			}
			
			break;

		case 5: //double-long             	[5] 		IMPLICIT Integer32
		case 6: //double-long-unsigned    	[6] 		IMPLICIT Unsigned32
//		case 7: //floating-point           	[7] 		IMPLICIT OCTET STRING(SIZE(4))
		case DT_FLOAT32: //float32                	[DT_FLOAT32] 	IMPLICIT OCTET STRING (SIZE(4))
		//case DT_TIME: //time
			if (bNullArr)
			{
				pbSrc += 4;
			}
			else
			{
				*pbDst++ = bType;
//				if (IsAllAByte(pbSrc, INVALID_DATA, 4))
//					memset(pbSrc, 0x00, 4);
				revcpy(pbDst, pbSrc, 4);	//���ý���Ч����EEת��Ϊ0����Ҫ��Ϊ�ֳ��������0����Ч����
				pbDst += 4;
				pbSrc += 4;
			}

			break;

		case DT_LONG64: //long64                	[20] 	IMPLICIT Integer64
		case DT_LONG64_U: //long64-unsigned       	[21] 	IMPLICIT Unsigned64
		case DT_FLOAT64: //float64                	[24] 	IMPLICIT OCTET STRING (SIZE(8)),
			if (bNullArr)
			{
				pbSrc += 8;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 8);
				pbDst += 8;
				pbSrc += 8;
			}

			break;

		case DT_DATE_TIME: //date-time
			//����: 19 07 d7 08 07 ff 08 2b 39 ff 80 00 00  
			if (bNullArr)
			{
				pbSrc += 12;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 12);			
				pbDst += 12;
				pbSrc += 12;
			}

			break;

		case DT_DATE: //date
			if (bNullArr)
			{
				pbSrc += 5;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 5);			
				pbDst += 5;
				pbSrc += 5;
			}
			
			break;
		case DT_TIME: //time
			if (bNullArr)
			{
				pbSrc += 4;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 4);
				pbDst += 4;
				pbSrc += 4;
			}
			break;
		case DT_DATE_TIME_S: //time
			if (bNullArr)
			{
				pbSrc += 7;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 7);
				pbDst += 7;
				pbSrc += 7;
			}
			break;
		case DT_OAD://OAD
			*pbDst++ = bType;
			memset(pbDst, 0, 4);
			pbDst += 4;
			break;
		case DT_TSA://TSA

			break;
		case DT_OVER_PARA:
			pbSrc++;	

			*pbDst++ = 0x01;
			*pbDst++ = 0x00;

			break;
		case DT_INSTANCE:
			pbSrc++;	

			*pbDst++ = 0x00;//0x11
			//*pbDst++ = 0x00;

			break;
		case DT_TI:
			*pbDst++ = DT_TI;
			memset(pbDst, 0, 3);
			pbDst += 3;
			break;
		case DT_OVER_RES:
			BYTE bResNum;
			bResNum = *pbSrc ++;
			for (BYTE j=0; j<bResNum; j++)
			{
				pbSrc += 12;
			}
			break;
		default:
			wErr = 4;//FMT_ERR_UNK_TYPE;	//δ֪����
			goto OIFmtData_err;
		}
		
		if (fPop)	//��Ҫ��ջ
		{
			while (iSP > 0)
			{
				bLen = bStack[--iSP];	//����
				bType = bStack[--iSP];	//����
				if (bType == 0x01)		//array
				{	
					bTmpFP = bStack[--iSP];
					wOffset = (WORD )bStack[--iSP] * 0x100;
					wOffset += bStack[--iSP];
					bArrLen = bStack[--iSP];

					if (bArrLen > 0)	//��������Դ�����л��ж��ٱʼ�¼û������
						bArrLen--;
				}

				if (bLen > 0)
					bLen--;

				if (bLen > 0)	//�ṹ�����黹û��
				{
					if (bType == 0x01)	//array
					{
						wOffset = pbSrc - pbSrc0; //���黹û��,wOffset������һ�ʼ�¼����ʼ
												  //λ��,��֤������������һ�ʼ�¼�ĳ���
						bStack[iSP++] = bArrLen;
						bStack[iSP++] = wOffset;
						bStack[iSP++] = wOffset>>8;

						bFP = bTmpFP;
						bStack[iSP++] = bFP;
					}
					
					bStack[iSP++] = bType;	//����
					bStack[iSP++] = bLen;	//����
					break;
				}
				else
				{
					if (bType == 0x01)		//array
					{
						pbSrc += (pbSrc - pbSrc0 - wOffset) * bArrLen;
					}

					//һ���ṹ�е�ȫ���ֶ����ݶ�Ϊ��Ч����0,Ŀǰ�Ȳ��������ṹ����Ϊ0
					//���Ҫ�����Ļ�,ֻ���¼Ŀ�����ݽṹ����ʼ,�жϽṹȫ����Ա�Ƿ�Ϊ0
				}

				//�ýṹ����������,�˵�������һ��
			}
		}

		if (bNullArr && iNullSP==iSP)
			bNullArr = 0; 

	} while (iSP>0 && pbFmt<pbFmtEnd);

	return pbDst-pbDst0;

OIFmtData_err:
   	DTRACE(DB_FAPROTO, ("OIFmtData : %s, FP=%d, SP=%ld, src=%d, dst=%d\n",
						OIFmtErrToStr(wErr), 
						bFP, iSP, 
						pbSrc-pbSrc0, 
						pbDst-pbDst0)); 
	
	return -wErr;
}


//�������ж϶�ȡ��ID�Ƿ�Ϊ������OI�����������OI����Ҫ���ǲ������м��֡�����
bool IsRdPnOI(WORD wOI)
{
	switch(wOI)
	{
		case 0x6000:
		case 0x6001:
			return true;
	}

	return false;
}

bool IsNeedRdSpec(const ToaMap* pOI)
{
	switch (pOI->dwOA)
	{
	case 0x60000200:	//0x6000�����ж���������������0x6001ֻ�е�ǰ���õ�һ�����õ�Ԫ
	case 0x60020200:	//�����ѱ���
	case 0x60020500:	//��̨���ѱ���
	case 0x60120200:	//�������õ�Ԫ
	case 0x60140200:	//��ͨ�ɼ�����
	case 0x60160200:	//�¼��ɼ�����
	case 0x60180200:	//͸���ɼ�����
	case 0x60190200:	//͸���ɼ�����
	case 0x601E0200:	//�ɼ�����
	case 0x601C0200:	//͸���ɼ�����
	case 0x60510200:	//ʵʱ�ɼ�����
	case 0x40000200:	//����ʱ��--����2
	case 0xF2000200:	//RS232�˿�
	case 0xF2010200:	//485�˿�
	case 0xF2020200:	//����˿�
	case 0xF2030200:	//����������--����2
	case 0xF2050200:	//�̵������--����2
	case 0xF2060200:	//�澯���--����2
	case 0xF2060400:	//�澯���--����4
	case 0xF2070200:	//�๦�ܶ���--����2
	case 0xF2080200:	//���ɽӿ�--����2
	case 0xF2090200:	//�ز�/΢�������߽ӿ�--����2
	case 0xF20a0200:	//���������豸--����2
	case 0x80030200:	//һ��������Ϣ����2
	case 0x80040200:	//��Ҫ������Ϣ����2
	case 0x81030200:	//ʱ�ι���
	case 0x81030300:
	case 0x81030400:
	case 0x81030500:
	case 0x81040200:	//���ݿ�
	case 0x81040300:
	case 0x81040400:
	case 0x81040500:
	case 0x81050200:	//Ӫҵ��ͣ��
	case 0x81050300:
	case 0x81050400:
	case 0x81050500:
	//case 0x81060200:	//��ǰ�����¸���
	case 0x81060300:
	case 0x81060400:
	case 0x81060500:
	case 0x81070200:	//�����
	case 0x81070300:
	case 0x81070400:
	case 0x81070500:
	case 0x81080200:	//�µ��
	case 0x81080300:
	case 0x81080400:
	case 0x81080500:
		return true;
	}

	return false;
}

bool IsNeedWrSpec(const ToaMap* pOI)
{
	switch (pOI->dwOA)
	{
	case 0x40000200:	//����ʱ��--����2
	case 0xF2000200:	//RS232�˿�
	case 0xF2010200:	//485�˿�
	case 0xF2020200:	//����˿�
	case 0xF2030200:	//����������--����2	
	case 0xF2050200:	//�̵������--����2
	case 0xF2060200:	//�澯���--����2
	case 0xF2060400:	//�澯���--����4
	case 0xF2070200:	//�๦�ܶ���--����2
	case 0xF2080200:	//���ɽӿ�--����2
	case 0xF2090200:	//�ز�/΢�������߽ӿ�--����2
	case 0xF20a0200:	//���������豸--����2
		return true;
	}

	return false;
}

//����������OI�Ķ�ȡ����Ҫ������������������ݵȣ��漰�����¼�����ݵ�
int OIRead_Spec(ToaMap* pOI, BYTE* pbBuf, WORD wBufSize, int* piStart)
{
	int iLen = -1, iRv = -1, i, iRet;
	WORD wMaxNum, wTotNum=0, wSigFrmPnNum;
	WORD wSn;
	BYTE *pbTmp;
	static WORD wSnLoc = 0;
	BYTE bBuf[PNPARA_LEN+1];
	BYTE *pbSch, bType;
	WORD wPnNum=0, wLen;
	BYTE bTaskNum=0, bSchNum=0;
	TTaskCfg tTaskCfg;

	switch(pOI->wID)
	{
		case 0x4000:	//����ʱ��--����2
			TTime now;
			GetCurTime(&now);
			pbTmp = pbBuf;
			*pbTmp++ = DT_DATE_TIME_S;
			*pbTmp++ = now.nYear/256;
			*pbTmp++ = now.nYear%256;
			*pbTmp++ = now.nMonth;
			*pbTmp++ = now.nDay;
			*pbTmp++ = now.nHour;
			*pbTmp++ = now.nMinute;
			*pbTmp++ = now.nSecond;

			*piStart = -1;
			return pbTmp - pbBuf;
		case 0x6000://�ɼ��������ñ����������
			wMaxNum = POINT_NUM;
			pbTmp = pbBuf;
			wSigFrmPnNum = wBufSize/PNPARA_LEN;
			if (*piStart == -1) //��һ������
			{
				wTotNum = GetValidPnNum();
				if (wTotNum == 0) //����Ӧ����
				{
					*pbBuf = EMPTY_DATA;
					return 1;
				}
				else
				{
					wSnLoc = 1;
					*pbTmp++ = 0x01; //����
					if (wSigFrmPnNum >=  wTotNum)
						*pbTmp++ = wTotNum; //������  //Ŀǰ���PN_NUM�飬����Ҫ�Գ����������
					else
						*pbTmp++ = wSigFrmPnNum;
				}
			}
			for (i=wSnLoc; i<wMaxNum; i++)
			{
				wSn = i;
				if (!IsPnValid(wSn))//δ���ù�����
					continue;
					
				memset(bBuf, 0, sizeof(bBuf));
				iLen = ReadItemEx(BN0, wSn, pOI->wID, bBuf);					
				if (iLen <= 0)
				{
					DTRACE(DB_FAPROTO, ("OIRead_Spec:wID = %d, wSn=%d, ReadItemEx failed !!\r\n", pOI->wID, wSn));
					return -DA_OTHER_ERROR;	//������������
				}
				wPnNum++;
				if (wPnNum>=wSigFrmPnNum)
				{
					wSnLoc = i;
					(*piStart)++;	
					break;
				}

				iLen = bBuf[0];	//�����Ч���ݳ���
				iRet = OoScanData(bBuf+1, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType);	//�����ֽ�˳��

				if (iLen <= sizeof(bBuf)-1)
				{
					DTRACE(DB_FAPROTO, ("OIRead_Spec:wID = %d, iLen=%d, bBuf[0]=0x%02x. zqzq!!\r\n", pOI->wID, iLen, bBuf[0]));
					memcpy(pbTmp, bBuf+1, iLen);
					pbTmp += iLen;
				}
				else
				{
					DTRACE(DB_FAPROTO, ("OIRead_Spec:wID = %d, iLen=%d, bBuf[0]=0x%02x. error!!!!!\r\n", pOI->wID, iLen, bBuf[0]));
				}				
			}
			if (i == wMaxNum) //����ȫ������
				*piStart = -1;

			return (pbTmp-pbBuf);	

		case 0x6012:	//�������õ�Ԫ
			wMaxNum = TASK_NUM;
			pbTmp = pbBuf;
			wSigFrmPnNum = wBufSize/PNPARA_LEN;
			if (*piStart == -1) //��һ������
			{
				wTotNum = GetTaskNum();
				if (wTotNum == 0) //����Ӧ����
				{
					*pbBuf = EMPTY_DATA;
					return 1;
				}
				else
				{
					wSnLoc = 1;
					*pbTmp++ = 0x01; //����
					if (wSigFrmPnNum >=  wTotNum)
						*pbTmp++ = wTotNum; //������  //Ŀǰ���PN_NUM�飬����Ҫ�Գ����������
					else
						*pbTmp++ = wSigFrmPnNum;
				}
			}
			for (i=wSnLoc; i<wMaxNum; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if ((iLen=GetTaskConfigFromTaskDb(i, bBuf)) <= 0)
					continue;

				if (iLen <= 0)
				{
					DTRACE(DB_FAPROTO, ("OIRead_Spec:wID = %d, wSn=%d, GetTaskCfgFromTaskDb()  failed !!\r\n", pOI->wID, i));
					return -DA_OTHER_ERROR;	//������������
				}
				bTaskNum++;
				if (bTaskNum>=wSigFrmPnNum)
				{
					wSnLoc = i;
					(*piStart)++;	
					break;
				}

				if (iLen <= sizeof(bBuf))
				{
					memcpy(pbTmp, bBuf, iLen);
					pbTmp += iLen;
				}
			}
			if (i == wMaxNum) //����ȫ������
				*piStart = -1;

			return (pbTmp-pbBuf);	
		case 0x6014:	//��ͨ�ɼ�����
		case 0x6016:	//�¼��ɼ�����
		case 0x6018:	//͸���ɼ�����
		case 0x601C:	//�ϱ�����
		case 0x6051:	//ʵʱ�ɼ�����
			if (*piStart == -1)
				wSnLoc = 0;
			wMaxNum = TASK_NUM;
			pbTmp = pbBuf+2;
			if (pOI->wID == 0x6014)
				wSigFrmPnNum = wBufSize/PNPARA_LEN;
			else
				wSigFrmPnNum = wBufSize/512;

			BYTE bSchType;
			if (pOI->wID == 0x6014)
				bSchType = SCH_TYPE_COMM;
			else if (pOI->wID == 0x6016)
				bSchType = SCH_TYPE_EVENT;
			else if (pOI->wID == 0x6018)
				bSchType = SCH_TYPE_TRANS;
			else if (pOI->wID == 0x601C)
				bSchType = SCH_TYPE_REPORT;
			else 
				bSchType = SCH_TYPE_REAL;

			for (i=wSnLoc; i<wMaxNum; i++)
			{
				iRet = GetSchFromTaskDb(i, bSchType, pbTmp);
				if (iRet < 0)
					continue;
				bSchNum++;
				pbTmp += iRet;
				if (bSchNum >= wSigFrmPnNum)
				{
					pbBuf[0] = 0x01;	//array
					pbBuf[1] = bSchNum;
					wSnLoc = i;
					return (pbTmp-pbBuf);	
				}
			}

			if (bSchNum == 0)
			{
				*pbBuf = EMPTY_DATA;
				return 1;
			}
			pbBuf[0] = 0x01;	//array
			pbBuf[1] = bSchNum;
			if (i == wMaxNum) //����ȫ������
				*piStart = -1;

			return (pbTmp-pbBuf);	
		case 0x6002:
			return GetSchMtrResult(piStart, pbBuf, wBufSize, 0, 0);
			break;
		case 0x6003:
			return GetCrossSchMtrResult(piStart, pbBuf, wBufSize);
		case 0x601E:
			return GetAllAcqRuleInfo(piStart, pbBuf, wBufSize);

		case 0x8003://һ��������Ϣ
		case 0x8004://��Ҫ������Ϣ
			wMaxNum = GB_MAXCOMCHNNOTE;
			pbTmp = pbBuf+2;
			bSchNum = 0;
			for (wSn=0; wSn<wMaxNum; wSn++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				iLen = ReadItemEx(BN0, wSn, pOI->wID, bBuf);					
				if (iLen <= 0)
				{
					DTRACE(DB_FAPROTO, ("OIRead_Spec:wID = %d, wSn=%d, ReadItemEx failed !!\r\n", pOI->wID, wSn));
					return -DA_OTHER_ERROR;	//������������
				}
				if (!IsAllAByte(bBuf, 0, iLen)) //����Ч����
				{
					iLen = bBuf[0];	//������Ϣ��Ч���ݳ���
					memcpy(pbTmp, bBuf+1, iLen);
					pbTmp += iLen;
					bSchNum++;
				}
			}

			if (bSchNum == 0)
			{
				*pbBuf = EMPTY_DATA;
				return 1;
			}

			pbBuf[0] = DT_ARRAY;	//array
			pbBuf[1] = bSchNum;			
			return (pbTmp-pbBuf);

		case 0x8103:
		case 0x8104:
		case 0x8105:
		//case 0x8106:
		case 0x8107:
		case 0x8108:
		case 0x8230:
		case 0x8231:
		case 0x8232:
		case 0x8240:
		case 0x8241:
		case 0x8242:
		case 0x8250:
		case 0x8251:
		case 0x8252:
		case 0x8260:
		case 0x8261:
		case 0x8262:
		case 0x8270:
		case 0x8271:
		case 0x8272:
		case 0x8280:
		case 0x8281:
		case 0x8282:
			wMaxNum = GRP_NUM;
			pbTmp = pbBuf+2;
			bSchNum = 0;
			for (wSn=GRP_START_PN; wSn<wMaxNum; wSn++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				iLen = ReadItemEx(BN0, wSn, pOI->wID, bBuf);					
				if (iLen <= 0)
				{
					DTRACE(DB_FAPROTO, ("OIRead_Spec:wID = %d, wSn=%d, ReadItemEx failed !!\r\n", pOI->wID, wSn));
					return -DA_OTHER_ERROR;	//������������
				}
				if (!IsAllAByte(bBuf, 0, iLen)) //����Ч����
				{
					memcpy(pbTmp, bBuf, iLen);
					pbTmp += iLen;
					bSchNum++;
				}
			}

			if (bSchNum == 0)
			{
				*pbBuf = EMPTY_DATA;
				return 1;
			}

			pbBuf[0] = DT_ARRAY;	//array
			pbBuf[1] = bSchNum;			
			return (pbTmp-pbBuf);

		case 0xF200:	//232
		case 0xF201:	//485
		case 0xF202:	//infra
				BYTE bMaxPortNum, bPortNum, bPortSn;
				BYTE bType;
				WORD wLen;
				bPortNum = 0;
				pbTmp = pbBuf + 2;
				if (pOI->wID == 0xF200)
					bMaxPortNum = MAX_232_PORT_NUM;
				else if (pOI->wID == 0xF201)
						bMaxPortNum = MAX_485_PORT_NUM;
				else
					bMaxPortNum = 1;
				for (bPortSn=0; bPortSn<bMaxPortNum; bPortSn++)
				{
					memset(bBuf, 0, sizeof(bBuf));
					wLen = ReadItemEx(BN0, bPortSn, pOI->wID, bBuf);
					TraceBuf(DB_CRITICAL, ("\r\n OIRead_Spec :bBuf->"), bBuf, wLen);
					TraceBuf(DB_CRITICAL, ("\r\n OIRead_Spec :pOI->pFmt->"), pOI->pFmt, pOI->wFmtLen);
//						wLen = 0;
					//ע���ʽ�ַ���ǰ��array�ͳ�Ա����Ҫȥ��
					iLen = OoScanData(bBuf, pOI->pFmt+2, pOI->wFmtLen-2, false, -1, &wLen, &bType);
//	 					if (iLen > 0)
//	 						memcpy(pbBuf, bTmpBuf, iLen);	//��ȡɨ�赽��������Ч����
					if (iLen>0 && !IsAllAByte(bBuf, 0, iLen))
					{
						bPortNum++;
						memcpy(pbTmp, bBuf, iLen);
						pbTmp += iLen;
						
						TraceBuf(DB_CRITICAL, ("\r\n OIRead_Spec :pbTmp->"), bBuf, iLen);
					}
				}
				pbBuf[0] = DT_ARRAY;
				pbBuf[1] = bPortNum;

			return pbTmp - pbBuf;
		case 0xF203:
		case 0xF205:
		case 0xF206:
		case 0xF207:
		case 0xF208:
		case 0xF209:
		case 0xF20a:
		case 0xF801:
			BYTE bSwValidNum;
			BYTE bSwSn;

			bSwValidNum = 0;
			pbTmp = pbBuf + 2;
			for (bSwSn=0; bSwSn<pOI->wVal; bSwSn++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				iLen = ReadItemEx(BN0, bSwSn, pOI->wID, bBuf);
				wLen = 0;
				iLen = OoScanData(bBuf, pOI->pFmt+2, pOI->wFmtLen-2, false, -1, &wLen, &bType);
				if (iLen>0 && !IsAllAByte(bBuf, 0, iLen))
				{
					bSwValidNum++;
					memcpy(pbTmp, bBuf, iLen);
					pbTmp += iLen;
				}
			}

			pbBuf[0] = DT_ARRAY;
			pbBuf[1] = bSwValidNum;
		
			return pbTmp - pbBuf;
		default:
			break;
	}

	return -1;
}

//����������OI��д����Ҫ������������������ݵȣ��漰�����¼�����ݵ�
int OIWrite_Spec(const ToaMap* pOI, BYTE* pbBuf)
{
	BYTE *pbBuf0 = pbBuf;

	switch (pOI->wID)
	{
	case 0x4000:	//����ʱ��--����2
		TTime tm;
		if (DT_DATE_TIME_S == *pbBuf++)
		{
			GetCurTime(&tm);
			g_AdjTermTime.bClock[0] = DT_DATE_TIME_S;
			OoTimeToDateTimeS(&tm, &g_AdjTermTime.bClock[1]);	//��¼Уʱǰʱ��

			memset((BYTE*)&tm, 0, sizeof(tm));
			tm.nYear = pbBuf[0]*256 + pbBuf[1];
			pbBuf += 2;
			tm.nMonth = *pbBuf++;
			tm.nDay = *pbBuf++;
			tm.nHour = *pbBuf++;
			tm.nMinute = *pbBuf++;
			tm.nSecond = *pbBuf++;
			
			SetSysTime(tm);
			#ifndef SYS_WIN
			AcDateTimeChg();
			#endif

			//SetInfo(INFO_ADJ_TERM_TIME);
			DealSpecTrigerEvt(TERM_CLOCKPRG);	//hyl ֱ�Ӵ洢�ն˶�ʱ�¼�
			return pbBuf - pbBuf0;
		}

		break;
	case 0xF200:	//232
	case 0xF201:	//485
	case 0xF202:	//infra
	case 0xF203:
	case 0xF205:
	case 0xF206:
	case 0xF207:
	case 0xF208:
	case 0xF209:
	case 0xF20a:
	case 0xF801:
		BYTE bMaxPortNum, bPortNum, bPortSn;
		BYTE bType;
		WORD wTotalLen = 0;
		if (DT_ARRAY == *pbBuf)
		{
			wTotalLen = 0;
			bPortNum = 0;
			if (pOI->wID == 0xF200)
				bMaxPortNum = MAX_232_PORT_NUM;
			else if (pOI->wID == 0xF201)
					bMaxPortNum = MAX_485_PORT_NUM;
			else if (pOI->wID == 0xF202)
				bMaxPortNum = 1;
			else
				bMaxPortNum = pOI->wVal;
			
			DTRACE(DB_CRITICAL, ("OIWrite_Spec : pbBuf[1]=%d, bMaxPortNum =%d!\n",pbBuf[1],bMaxPortNum));
			
			if(pbBuf[1]<=bMaxPortNum)
			{
				bMaxPortNum = pbBuf[1];//ʵ�ʵ��豸������
			}
			
			for (bPortSn=0; bPortSn<bMaxPortNum; bPortSn++)
			{
				WORD  wLen,wLenTmp;
				BYTE *pbPtr;
				BYTE bType,bBuf[32];
				pbPtr = OoGetField(pbBuf, pOI->pFmt, pOI->wFmtLen, bPortSn, &wLen, &bType); 
				if(wLen>0)
				{
					memset(bBuf, 0, sizeof(bBuf));
					memcpy(bBuf, pbPtr, wLen);
					DTRACE(DB_CRITICAL, ("OIWrite_Spec : bPortSn = %d, wLen = %d\r\n",bPortSn,wLen));
					TraceBuf(DB_CRITICAL, ("\r\n OIWrite_Spec :bBuf->"), bBuf, wLen);
					wLenTmp = WriteItemEx(BN0, bPortSn, pOI->wID, pbPtr);
					if(wLenTmp>0)
					{
						wTotalLen += wLen; 
					}
					else
					{
						return -1; 
					}
				}
			}
			return wTotalLen;
		}
		break;
	}

	return -1;
}

//��������ȡ�������ԣ��������ݴ���ʽ������ͨѶֱ�ӵ���
//������wOI���ʶ�������
//		bAttr����OI������
//		pbBuf�����ػ�����
//		pbOpt: ���ʶ���ѡ��������ڼ�¼������ʹ��
//		piStart�������Ͳ���,������¼��ζ����ݵĲ��裬���ڶ�֡ͨѶ����
//				�״ζ�ȡΪ-1����ʼ���ڲ����ṹ������riStep��ֵ�޸�Ϊ>=0����,������ʱ�����ڲ����ṹ��riStep��������ʲô����
//				����ȡ����Ϊ���һ������ʱ���ٽ�riStep��ֵ��Ϊ-1��ʾ�޺�������
//����:�����ȷ���ض���pbBuf�����ݵĳ���,���򷵻ظ���
int OoProReadAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wBufSize, int* piStart)
{
	BYTE bAttrNum, bType;
	BYTE bTmpBuf[APDUSIZE];
	BYTE *pbTmpBuf = bTmpBuf;
	BYTE *pbBuf0 = pbBuf;		//���浱ǰ����ָ��
	int iLen = 0;
	WORD wID, wLen, wPn;
	BYTE bBuf[20];

	if (wBufSize == 0)
		wBufSize = APDUSIZE;

	if (bAttr == 0)	//��������
	{
		//��Ϊÿ���඼������2����������2ȡ��class��
		DWORD dwOIAtt = ((DWORD )wOI<<16) + 0x0200;
		const ToaMap* pOI = GetOIMap(dwOIAtt);
		if (pOI == NULL)
		{
			DTRACE(DB_OIIF, ("OIReadObj: Read wOI:%x failed !!\r\n", wOI));
			return -1;
		}
		bAttrNum = OIGetAttrNum(pOI->wClass);
		*pbBuf++ = 0x02; //�ṹ
		pbBuf += EncodeLength(bAttrNum, pbBuf);	//����ĳ�Ա����
		WORD wMaxBufLen = wBufSize;
		for (BYTE i=1; i<=bAttrNum; i++)
		{
			//�������0���з���ʱ,����˳���������1������n,�����������ڸ���ԭ���ܷ��ʵ�,
			//����Ӧ�ط��� null-data.<�μ� IEC62056-53, 58ҳ>
			if ((iLen=OoProReadAttr(wOI, i, bIndex, pbBuf, wMaxBufLen, piStart)) <= 0)
			{
				DTRACE(DB_OIIF, ("OIReadObj: Read wOI:%x,Attribute %d failed \r\n", wOI, i));
				*pbBuf = EMPTY_DATA;	//null-data;
				iLen = 1;
			}
			pbBuf += iLen;
			wMaxBufLen -= iLen;
		}
		return (int)(pbBuf-pbBuf0);
	}
	else if (bAttr == 1) //�߼���
	{
		*pbBuf++ = 0x09;			//��ʽ
		*pbBuf++ = 0x02;			//����
		*pbBuf++ = (BYTE )(wOI>>8);
		*pbBuf++ = (BYTE )wOI;
		return (int)(pbBuf-pbBuf0);
	}
	else //��������
	{
		DWORD dwOIAtt = ((DWORD )wOI<<16) + ((DWORD )bAttr<<8);
		const ToaMap* pOI = GetOIMap(dwOIAtt);
		if ((wOI>=0xF000) && (wOI<=0xF002))
		{
			iLen = GetFileTransAttr(wOI, bAttr, 0, pbBuf, wBufSize, piStart);
			return iLen;
		}
		if (pOI == NULL)
		{
			DTRACE(DB_OIIF, ("OIReadObj: Read wOI:%x,Attribute %d failed \r\n", wOI, bAttr));
			return -1;
		}

		if (IsNeedRdSpec(pOI) && bIndex==0)//����OI�Ķ�ȡ��������������ʱ���
		{
			 iLen = OIRead_Spec((ToaMap *)pOI, pbBuf, wBufSize, piStart);
			 if (dwOIAtt == 0xF2030200)		//ң��״̬������λ��־������2),�������
			 {
			 	for (WORD i=0; i<MAX_SW_PORT_NUM; i++)
				{
					memset(bBuf, 0, sizeof(bBuf));
					if (ReadItemEx(BN0, i, 0xF203, bBuf) > 0)  //�洢ң��״̬��
					{
						bBuf[5] = 0;
						WriteItemEx(BN0, i, 0xF203, bBuf);  //�洢ң��״̬��
					}
				}
			 }

			 return iLen;
		}
		else
		{
			if (IsNeedRdSpec(pOI))
			{
				wPn = bIndex - 1;	//����OAD�ĵ�bIndex�����Զ�Ӧ��bIndex-1��������
				bIndex = 0;			//����OI������Ϊ��ֳ���OAD������ֱ�Ӷ���OAD�����ݣ���ͬ�ڶ�����OAD�ĵڼ�������
			}
			else
			{
				wPn = pOI->wPn;
			}

			if ((wOI&0xf000) == 0x3000)	// ���ڱ�򳭱��¼���¼���������ݷ��أ��������
			{
				if (GetTermEvtCtrl(wOI) != NULL)	//TermEvtTask.h/cpp��֧�ֵ������¼����������������¼�
				{
					iLen = GetEvtRecord(wOI, bAttr, bIndex, pbBuf, wBufSize);
					if (iLen > 0) 
						return iLen;
				}
				else
				{
					iLen = GetMtrExcEvtRecord(wOI, bAttr, bIndex, pbBuf, wBufSize);
					if (iLen > 0)
						return iLen;
				}
			}
			else if ((dwOIAtt&OAD_OI_MASK) == 0x50000200)	//�����¼
			{
				return ReadFrzData(dwOIAtt, pbBuf, wBufSize, piStart);
			}
			if (pOI->wMode == MAP_VAL)	//��Ҫ�����ֱ�ӳ������㼰��λ�ȹ̶����ݸ�ʽ
			{
				memcpy(pbBuf, (BYTE *)&(pOI->wVal), 2);
				iLen = 2;
			}
			else if (pOI->wMode == MAP_BYTE)	//��Ҫ�����ֱ��1�ֽڳ����̶����ݸ�ʽ
			{
				pbBuf[0] = DT_UNSIGN;
				memcpy(pbBuf+1, (BYTE *)&(pOI->wVal), 1);
				iLen = 2;
			}
			else if(pOI->wMode == MAP_SYSDB)
			{
				iLen = ReadItemEx(BN0, wPn, pOI->wID, bTmpBuf);
				if (iLen <= 0)
				{
					DTRACE(DB_FAPROTO, ("OoProReadAttr: There is something wrong when call wID:%02x !\n", pOI->wID));
					return -1;
				}
				if (pOI->pFmt != NULL && pOI->wFmtLen > 0)
				{
					if (IsAllAByte(bTmpBuf, 0x00, iLen))	//���ȫ����0x00����Ӧ����û��д������ʽ������Ϊ0
					{
						iLen = OIFmtData(bTmpBuf, pbBuf, pOI->pFmt, pOI->wFmtLen);
					}
					else
					{
						if (bIndex == 0)	//���Ե�ȫ������
						{
							if (IsNeedRdSpec(pOI) && (wOI&0xFF00)==0xf200)	//�������OAD��ȥ��DT_ARRAY���������
							{
								wLen = 0;
								iLen = OoScanData(bTmpBuf, pOI->pFmt+2, pOI->wFmtLen-2, false, -1, &wLen, &bType);
							}
							else
							{
								iLen = OoScanData(bTmpBuf, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType);
							}

							if (iLen < 0)
							{
								DTRACE(DB_FAPROTO, ("OoProReadAttr: OoDataScan Data failed, wID=0x%04x, iLen=%d !\n", pOI->wID, iLen));
								return -1;
							}
							else
							{
								memcpy(pbBuf, bTmpBuf, iLen);	//��ȡɨ�赽��������Ч����
							}
						}
						else	//�����е�����������
						{
							BYTE *pbPtr = OoGetField(bTmpBuf, pOI->pFmt, pOI->wFmtLen, bIndex-1, &wLen, &bType);
							if (pbPtr == NULL)
							{
								DTRACE(DB_FAPROTO, ("OoProReadAttr: OoGetField sub-attributes  failed, wOI=0x%04x, bAttr=0x%02x, index=0x%02x!\n", wOI, bAttr, bIndex));
								return -1;
							}
							else
							{
								memcpy(pbBuf, pbPtr, wLen);
								iLen = wLen;
							}
						}
					}			
				}
				else
					memcpy(pbBuf, bTmpBuf, iLen);
			}
			else	//ʹ�ñ��ӿڳ�������������---�������
				return -1;
		}

		return iLen;
	}
	
	return -1;
}

//����:ͨ��Э����õ�д��������ֵ�������������OoPro2AppScan()���ֽ���˳������Զ�����
//������@wOI	�����ʶ
//	   @bAtrr	���Ա�ʶ�������� bit-string��SIZE��8����
//	@bIndex������Ԫ������
//	   @pbBuf	Ҫд������ֵ�Ļ�������Ҫд������Ϊͨ��Э���ֽ�˳��
int OoProWriteAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wLen, bool fIsSecurityLayer)
{
	BYTE bTmpBuf[3000], bType, bPassWord[16];	//��ʱ���ݴ��,ȥ��ʽ������ݽ����������,������һ��д�뵽ϵͳ���ݿ�.
	BYTE bDbBuf[3000];
	BYTE *pbTmpBuf = bTmpBuf;
	BYTE bPerm = 0x11;
	ToaMap* pOI = NULL;
	WORD wID, wDataLen;
	int iLen = -1;
	int iLen0;
	//int iDataLen = 0;
	//BYTE bOADBuf[4];

	memset(bTmpBuf, 0, sizeof(bTmpBuf));//��ֹ���䳤���ݸ���Ϊ0ʱ�����ص����嵥�����������ȡ���ֵ
	if ((bAttr == 0) || ((bAttr == 1)))	//0����,�߼���������ֻ����
	{
		return -1;	//��д�ܾ�
	}
	else //��������
	{
		DWORD dwOIAtt = ((DWORD )wOI<<16) + ((DWORD )bAttr<<8);
		const ToaMap* pOI = GetOIMap(dwOIAtt);
		if ((wOI>=0xF000) && (wOI<=0xF002))
		{
			iLen = SetFileTransAttr(wOI, bAttr, bIndex, pbBuf);
			return iLen;
		}
		if (pOI == NULL)
			return -1;

		if (pOI->wMode == MAP_VAL || pOI->wMode == MAP_BYTE)	//����ǹ̶����ݸ�ʽ�����������
			return -1;

		if (bIndex == 0)	//ȫ����
		{
			pbTmpBuf = pbBuf;
			wID = pOI->wID;
			int nRet = OoScanData(pbTmpBuf, pOI->pFmt, pOI->wFmtLen, false, -1, &wDataLen, &bType);
			if (nRet > 0)
			{
				if (IsNeedWrSpec(pOI))
				{
					iLen = OIWrite_Spec(pOI, pbTmpBuf);
					goto RET_ATTR;
				}
				else 
				{
					//д��Ӧ������֮ǰ�����ж�����ϵͳ���е�����ֵ�Ƿ�һ�£���һ�¾�дϵͳ��
					memset(bDbBuf, 0, sizeof(bDbBuf));
					iLen0 = ReadItemEx(BN0, pOI->wPn, pOI->wID, bDbBuf);
					if (iLen0 > 0)	
					{
						int nRdRet = OoScanData(bDbBuf, pOI->pFmt, pOI->wFmtLen, false, -1, &wDataLen, &bType);
						if (nRdRet>0 && nRet==nRdRet && (memcmp(bDbBuf, pbTmpBuf, nRdRet) == 0))
						{
							iLen = nRdRet;
							goto RET_ATTR;
						}
						else
						{
							iLen = nRet;
						}
					}

					if ((dwOIAtt&0xf0000000) == 0x30000000) // �¼�
					{						
						ReInitEvtPara(dwOIAtt);
						if (dwOIAtt==0x31060900 && pbTmpBuf[1]==1)	//ͣ���¼���Ч�Դ���Ч��Ϊ��Ч
						{
							UpdateTermPowerOffTime();
						}
					}
					else if ((dwOIAtt&0xf0000000) == 0x21000300)	//ͳ�Ʋ��������֪ͨ�������
					{
						OnStatParaChg();
					}

					if ((iLen0 = WriteItemEx(BN0, pOI->wPn, pOI->wID, pbTmpBuf, bPerm, bPassWord)) <= 0)
					{
						DTRACE(DB_FAPROTO, ("DlmsWriteAttrToDB: There is something wrong when call WriteItemEx()\n"));
						return -1;	//������������
					}
				}
			}
			else
			{
				DTRACE(DB_FAPROTO, ("OoProWriteAttr: OoDataScan Data failed, nRet=%d !\n", nRet));
				return -1;
			}

			if (iLen == -1)//��Ӧ��ȥ��ʽֱ��д�����
				iLen = iLen0;
		}
		else	//������
		{
			int iSrcLen;
			WORD wFmtLen;
			BYTE bSrc[1024];
			BYTE *pbFmt;

			memset(bSrc, 0, sizeof(bSrc));
			iSrcLen = OoReadAttr(wOI, bAttr, bSrc, &pbFmt, &wFmtLen);
			if (iSrcLen > 0)
			{
			//	OoDWordToOad(GetOAD(wOI, bAttr, bIndex), bOADBuf);
			//	iDataLen = OoGetDataLen(DT_OAD, bOADBuf);	//+1:ȥ����������
			//	if (!(iDataLen >= wLen))
			//		wLen = iDataLen;	
				if (fIsSecurityLayer)
					iSrcLen = OoWriteField(bSrc, iSrcLen, pbFmt, wFmtLen, bIndex-1, pbBuf, wLen);	//-4: 4�ֽ�OAD��Esam����û��ʱ���ǩ
				else
					iSrcLen = OoWriteField(bSrc, iSrcLen, pbFmt, wFmtLen, bIndex-1, pbBuf, wLen);	//-5: 4�ֽ�OAD + 1�ֽ�ʱ���ǩ	//�˴�Ӧ�������洦��wLen����������OAD��ʱ���ǩ���Ѿ�ȥ����
				if (iSrcLen > 0)
					return OoProWriteAttr(wOI, bAttr, 0, bSrc, iSrcLen, fIsSecurityLayer);
			}
		}

//		SetParaChg(wClass, pbObis);
RET_ATTR:
		OutBeepMs(50);
		TrigerSave();
		return iLen;
	}

	return -1;
}

BYTE OIGetStrLen(BYTE* pbStr, BYTE bLen, BYTE bFill)
{
	for (BYTE i=0; i<bLen; i++)
	{
		if (*pbStr++ == bFill)
			return i;
	}

	return bLen;
}

//����:Ӧ�ò���õĶ�ȡ��������ֵ���൱�ڴ����ݿ���ֱ��ȡ�����ݣ����õ���ʽ������OoProReadAttr()���Ի��ڱ�������ʵ��
//������@wOI	�����ʶ
//		@bAtrr	���Ա�ʶ�������� bit-string��SIZE��8����
//		@pbBuf	��������ֵ�Ļ���������������ΪӦ���ֽ�˳��
//		@ppFmt	�������ظ�ʽ�����������Ϊ�ղ�����
//		@pwFmtLen	�������ظ�ʽ�������ĳ��ȣ����Ϊ�ղ�����
int OoReadAttr(WORD wOI, BYTE bAttr, BYTE* pbBuf, BYTE** ppFmt, WORD* pwFmtLen)
{
	WORD wLen;
	BYTE bType;
	BYTE bBuf[4096];

	DWORD dwOIAtt = ((DWORD )wOI<<16) + ((DWORD )bAttr<<8);
	const ToaMap* pOI = GetOIMap(dwOIAtt);
	if (pOI == NULL)
		return -1;

	memset(bBuf, 0, sizeof(bBuf));
	int nRet = ReadItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
	if (nRet > 0)
	{
		if (pOI->pFmt != NULL && pOI->wFmtLen > 0)
		{
			if (IsAllAByte(bBuf, 0x00, nRet))	//���ȫ����0x00����Ӧ����û��д������ʽ������Ϊ0
			{
				nRet = OIFmtData(bBuf, pbBuf, pOI->pFmt, pOI->wFmtLen);
			}
			else
			{
				if (IsNeedRdSpec(pOI) && (wOI&0xFF00)==0xf200)	//�������OAD��ȥ��DT_ARRAY���������
				{
					wLen = 0;
					nRet = OoScanData(bBuf, pOI->pFmt+2, pOI->wFmtLen-2, false, -1, &wLen, &bType);
				}
				else
				{
					nRet = OoScanData(bBuf, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType);
				}

				if (nRet <= 0)
					return -1;
				memcpy(pbBuf, bBuf, nRet);
			}
		}

		if (ppFmt != NULL)
			*ppFmt = pOI->pFmt;
		if (pwFmtLen != NULL)
			*pwFmtLen = pOI->wFmtLen;

		return nRet;
	}
	else
	{
		DTRACE(DB_DB, ("OoReadAttr: ReadItemEx wOI:%02x, Attr:%d, wID:%02x failed\n", wOI, bAttr, pOI->wID));
		return -1;
	}
}

//����:Ӧ�ò���õ�д��������ֵ���൱��ֱ�Ӱ�����д�����ݿ�
//������@wOI	�����ʶ
//		@bAtrr	���Ա�ʶ�������� bit-string��SIZE��8����
//		@bIndex������Ԫ������-----��ʱ���ÿ��ǰ�
//		@pbBuf	Ҫд������ֵ�Ļ�������Ҫд������ΪӦ���ֽ�˳��
int OoWriteAttr(WORD wOI, BYTE bAttr, BYTE* pbBuf)
{
	DWORD dwOIAtt = ((DWORD )wOI<<16) + ((DWORD )bAttr<<8);
	const ToaMap* pOI = GetOIMap(dwOIAtt);
	if (pOI == NULL)
		return -1;

	int nRet = WriteItemEx(BN0, pOI->wPn, pOI->wID, pbBuf);
	if (nRet < 0)
		DTRACE(DB_DB, ("OoWriteAttr: WriteItemEx wOI:%02x, Attr:%d, wID:%02x failed\n", wOI, bAttr, pOI->wID));

	return nRet;
}

//�������Ƿ�Э���׼����������
bool IsStdType(BYTE bType)
{
	if (bType<=32 || (bType>=DT_OI && bType<=DT_RCSD))
		return true;

	return false;
}

//����:ɨ�����ʽ�����ݣ���ȷ�������Ƿ���ϸ�ʽҪ�������������������ֽ�˳����з��򣬿��Զ��������ݸ�ʽ����ɨ�裬Ҳ����ɨ�赽ĳ���ֶμ�����
//������@pbSrc		Դ����
//	   @pbFmt		��ʽ������
//	   @wFmtLen		��ʽ�������ĳ���
//	   @fRevOrder	�Ƿ���Ҫ�������ֽ�˳����з���
//	   @iIndex		>=0:Ҫɨ�赽���ֶε�������-1ȫ��ɨ��
//	   @pwLen		���������ֶε����ݳ���
//	   @pbType		�������ظ��ֶε�����
//     @ppFieldFmt 	��iIndex>=0ʱ�����������ֶεĸ�ʽ��,�������Ҫ���Դ���NULL
// 	   @pwFieldFmtLen ��iIndex>=0ʱ�����������ֶεĸ�ʽ���ĳ���,�������Ҫ���Դ���NULL
//����:��ȷ��iIndex==-1������ȫ��ɨ��ĳ��ȣ�iIndex>=0:�ֶε���ʼλ��
//     ���󣺷��ظ���
int OoScanData(BYTE* pbSrc, BYTE* pbFmt, WORD wFmtLen, bool fRevOrder, int iIndex, WORD* pwLen, BYTE* pbType, BYTE** ppFieldFmt, WORD* pwFieldFmtLen)
{
	BYTE bStack[100], bTmpBuf[100];
	BYTE* pbSrc0 = pbSrc;
	BYTE* pbStart = NULL;  //�ҵ����ֶε���ʼλ��
	BYTE* pbFmtEnd = pbFmt + wFmtLen;
	int iSP = 0, iTmpSP=0;	//��ջָ��(������ͬ��ջ)
	BYTE bFP = 0;	//��ʽָ��
	BYTE bNum=0, bTmpFP=0xff;
	DWORD bLen, bSrcLen, bArrLen, dwNum;
	BYTE bFmtT, bVFmt, bVFill;
	bool fVLen, fPop, fFind, fGetFirst=false;
	BYTE bPushCnt = 0;
	BYTE bAcqTyp;	//�ɼ�����
	BYTE bSrcType = DT_UNCARE; //don't care
	int iNullSP = 0;	//������null dataʱ��iSP,��ɨ����һ�����ݺ�,iSP�ظ���iNullSP��ֵ,
						//��ʾ��null data�Ĵ������
	WORD wErr = 0;
	int nDataLen, nRetLen;
	WORD wIdx = 0;

	BYTE bAttrType = pbSrc[0];
	BYTE bAttrNum = pbSrc[1];

	if (pbFmt == NULL)
	{
		DTRACE(DB_DB, ("OoScanData: pbFmt is NULL, return!!\n"));
		return -1;
	}		

	//������������ԣ���ô��һ������������������01��02
	if ((bAttrType != pbFmt[0]) /*|| (iIndex>=0 && bAttrType!=0x01 && bAttrType!=0x02)*/)
	{//��һ������ݱ����������ṹ
		DTRACE(DB_DB, ("OoScanData: The First data-type:%d error !!\n", bAttrType));
		return -1;
	}
	
	if (iIndex >= bAttrNum && iIndex>0)
	{//������������ʽ�ַ������� lzx 20170217
		DTRACE(DB_DB, ("OoScanData: iIndex overflow, return!!\n"));
		return -1;
	}		

	fFind = false;
	//�ӽṹ������֮����������Ϳ�ʼ
	do
	{
		fPop = true;
		BYTE bType = pbFmt[bFP++];
		if (bSrcType != DT_NULL)	//��ǰ���ݲ��Ǵ��ڿսṹ���������һ��
		{
			bSrcType = *pbSrc++;
			if (bSrcType == DT_NULL) //null data
				iNullSP = iSP;		 //�ڵ�һ��������ʱ���¶�ջָ��,�����ж�ʲôʱ���������
		}

		if (bSrcType!=DT_NULL && bSrcType!=bType && IsStdType(bType))	//���Ͳ�����
		{
			wErr = FMT_ERR_TYPE;
			goto OoScanData_err;
		}

		if (iIndex==0 && !fGetFirst)	//�������������Ϊ��һ��������Ҫ�ȴ�����
		{
			fFind = true;
			fGetFirst = true;
			pbStart = &pbSrc[1];
			*pbType = pbSrc[1];

			if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
				*ppFieldFmt = &pbFmt[bFP+1];
		}

		if (fFind && iIndex>0)	//iIndex>=0��ʾҪ����ĳ������
		{
			*pbType = bType;
			fFind = false;
		}

		switch (bType)
		{
		case 0: 
			break;
		case 1: //array
			bLen = pbFmt[bFP++];		//array�ĸ���
			if (bSrcType == DT_NULL)	//������������ϼ������ṹΪ������
			{
				if (bLen == 0)	//��ʽ���еĳ���Ϊ0
				{		//�ڵ�ǰ��ʽ������������,���Դ����Ϊ��,			
						//���ʽ���еĳ��Ȳ���Ϊ0,����֪����Ϊ��Ч���ݵ�����Ԫ�ظ���
					if (bFP == 2)	//��ʽ���ĵ�һ��Ԫ��������,����Ϊ��������
					{
						pbSrc++;
						//return 1;
					}

					wErr = FMT_ERR_NULL;
					goto OoScanData_err;
				}

				bSrcLen = 0;
			}
			else
			{
				bSrcLen = *pbSrc++;
				if (bLen == 0)	//��ʽ���еĳ���û�޶�,ȡ�����еĳ���
				{
					if (bFP != 2)	//ֻ�����ڵ�һ��Ԫ��������������Ԥ��Ԫ�ظ���Ϊ0
					{
						wErr = FMT_ERR_LEN;
						goto OoScanData_err;
					}

					if (bSrcLen == 0)
					{	
						wErr = FMT_ERR_LEN;
						goto OoScanData_err;
					}

					//bLen = bSrcLen;
				}
				else	//�������
				{
					if (bSrcLen > bLen)	//�ڸ�ʽ���ĳ�����ȷ�������,Դ�����еĳ��Ȳ�����
					{	
						wErr = FMT_ERR_LEN;
						goto OoScanData_err;
					}

					if (bSrcLen == 0)	//��ջ��ʱ��bSrcLen=0
					{	//Դ���ݳ���bSrcLenΪ0,ҲҪ����һ��ɨ��,����������Ԫ�صĳ���
						bSrcType = DT_NULL; //Դ���ݸ���Ϊ0,����������ԱҲ�൱�������˿�����
						iNullSP = iSP; 
						
						//bFP++;
					}
				}
			}
			
			bStack[iSP++] = bFP;
			bStack[iSP++] = bType;
			bStack[iSP++] = bSrcLen;	//ʣ���¼��
										//��ʹ����Ԫ�ظ���Ϊ0,ҲҪ��һ����ջ�Ĳ���,
										//�ó���ɨ���Ԫ�صĸ�ʽ����
			fPop = false;
			bPushCnt++;
			break;

		case 2: //structure
		case DT_FRZRELA: //structure
			bLen = pbFmt[bFP++];

			if (bSrcType != DT_NULL)
			{
				bSrcLen = *pbSrc++;		//�ṹ����
				if (bSrcLen != bLen)	//���Ȳ�����
				{	
					wErr = FMT_ERR_LEN;
					goto OoScanData_err;
				}
			}

			bStack[iSP++] = bType;
			bStack[iSP++] = bLen;
			fPop = false;
			bPushCnt++;
			break;

		case 4: //bit-string
			bLen = pbFmt[bFP++];
			bVFmt = pbFmt[bFP++];		
			bLen = (bLen + 7) / 8;			//ת��Ϊ�ֽ���
			if (bSrcType != DT_NULL)
			{
				bSrcLen = *pbSrc++;				//λ����
				bSrcLen = (bSrcLen + 7) / 8;	//ת��Ϊ�ֽ���

				if (bSrcLen>bLen ||
					(!fVLen && bSrcLen!=bLen)) //����,�������,Ҫ���·��ĳ��ȱ���պ����
				{
					wErr = FMT_ERR_LEN;
					goto OoScanData_err;
				}
				//�ݶ�bitstring����Ҫ���������Ҫ����������
				pbSrc += bSrcLen;
			}
			break;

		case 9:	//octet-string            	[9] 		IMPLICIT OCTET STRING,
		case 10: //visible-string
		case 12: //UTF8-string
			//�ݶ�string����Ҫ���������Ҫ����������


			bFP += DecodeLength(&pbFmt[bFP], &bLen);
			//bLen = pbFmt[bFP++];
			bVFmt = pbFmt[bFP++];

			fVLen = (bVFmt&VF_VLEN)!=0; 
			bVFill = bVFmt>>VF_FILL_SHF;

			if (bSrcType != DT_NULL)
			{
				pbSrc += DecodeLength(pbSrc, &bSrcLen);
 				//bSrcLen = *pbSrc++;	//����

				if (bSrcLen > bLen)
				{
					wErr = FMT_ERR_LEN;
					goto OoScanData_err;
				}
				
				if (bSrcLen>bLen ||
					(!fVLen && bSrcLen!=bLen)) //����,�������,Ҫ���·��ĳ��ȱ���պ����
				{
					wErr = FMT_ERR_LEN;
					goto OoScanData_err;
				}
				pbSrc += bSrcLen;	//�����Ƿ񶨳�������������ݣ�ֻ����·������ݰ��Ƿ���Ҫ��������ִ��
			}
			break;

		case 3:  //boolean                	[3]	 	IMPLICIT BOOLEAN    
		case 13: //bcd                   	[13] 	IMPLICIT Integer8,
		case 15: //integer                	[15] 	IMPLICIT Integer8,
		case 17: //unsigned              	[17] 	IMPLICIT Unsigned8,
		case DT_ENUM: //enum 					[22] 	IMPLICIT NULL
		case DT_LONG: //long                  	[16] 	IMPLICIT Integer16,
		case DT_LONG_U: //long-unsigned         	[18] 	IMPLICIT Unsigned16,
		case 5: //double-long             	[5] 	IMPLICIT Integer32
		case 6: //double-long-unsigned    	[6] 	IMPLICIT Unsigned32
		case 7: //floating-point           	[7] 	IMPLICIT OCTET STRING(SIZE(4))
		case DT_FLOAT32: //float32                	[23] 	IMPLICIT OCTET STRING (SIZE(4))
		case DT_LONG64: //long64                	[20] 	IMPLICIT Integer64
		case DT_LONG64_U: //long64-unsigned       	[21] 	IMPLICIT Unsigned64
		case DT_FLOAT64: //float64                	[24] 	IMPLICIT OCTET STRING (SIZE(8)),
		case DT_DATE_TIME: //date-time
		case DT_DATE: //date
		case DT_TIME: //time
		case DT_DATE_TIME_S: //DatetimeBCD
		case 29: //DatetimeBCD_H
		case 30: //DatetimeBCD_S
		case 31: //DateBCD
		case 32: //TimeBCD
		case 80://OI--long-unsigned

			if (bType==3 || bType==13 || bType==15 || bType==17 || bType==DT_ENUM)
				bSrcLen = 1;
			else if (bType==DT_LONG || bType==DT_LONG_U || bType==80)
				bSrcLen = 2;
			else if (bType==5 || bType==6 || bType==7 || bType==DT_FLOAT32 || bType==31)
				bSrcLen = 4;
			else if (bType==DT_LONG64 || bType==DT_LONG64_U || bType==DT_FLOAT64)
				bSrcLen = 8;
			else if (bType == DT_DATE_TIME)
				bSrcLen = 10;
			else if (bType == DT_DATE)
				bSrcLen = 5;
			else if (bType == DT_TIME)
				bSrcLen = 3;
			else if (bType == DT_DATE_TIME_S)
				bSrcLen = 7;
			else if (bType == 29)
				bSrcLen = 9;
			else if (bType == 30)
				bSrcLen = 6;
			else if (bType == 32)
				bSrcLen = 3;
			
			if (bSrcType != DT_NULL)
			{
// 				if (fRevOrder)
// 				{
// 					revcpy(bTmpBuf, pbSrc, bSrcLen);
// 					memcpy(pbSrc, bTmpBuf, bSrcLen);
// 				}
				pbSrc += bSrcLen;
			}
			break;

		case DT_OAD://OAD
		case DT_OMD://OMD----���Ͷ��ǽṹ���ʵģ�����ֱ�ӵ���
			if (bSrcType != DT_NULL)
			{
				pbSrc += 4;
			}
			break;
			
		case DT_ROAD://ROAD
			if (bSrcType != DT_NULL)
			{
				dwNum = 0;
				pbSrc+=4;	//OAD
				nDataLen = DecodeLength(pbSrc, &dwNum);
				pbSrc+=nDataLen;
				pbSrc+=dwNum*4;
			}
			break;

		case DT_TI://TI
			if (bSrcType != DT_NULL)
			{
				//pbSrc+=2;	//����ʽemu
				pbSrc+=3;	//����ʽlong-unsigned----------�ٶ�seq�ڲ����ݴ���ʽ��
			}
			break;

		case DT_TSA://TSA
			if (bSrcType != DT_NULL)
			{
				bNum = *pbSrc++;
				if (bNum > 16)
					goto OoScanData_err;
				pbSrc += bNum;
			}
			break;

		case DT_MAC://MAC
		case DT_RN://RN
			bNum = *pbSrc++;
			pbSrc += bNum;
			break;


		case DT_REGION://Region
			pbSrc+=2;	//����ʽemu
			break;

		case DT_SCALE_UNIT://Scaler-Unit
			pbSrc+=2;	//����ʽinteger
			pbSrc+=2;	//����ʽemu
			break;

		case DT_RSD://RSD 
			nDataLen = ScanRSD(pbSrc, fRevOrder);	//�ú����Զ��޸���pbSrcƫ��
			if (nDataLen > 0)
				pbSrc += nDataLen;
			else
				goto OoScanData_err;
			break;

		case DT_CSD://CSD
			nDataLen = ScanCSD(pbSrc, fRevOrder);
			if (nDataLen > 0)
				pbSrc += nDataLen;
			else
				goto OoScanData_err;
			break;

		case DT_MS://MS
			//nDataLen = ScanMS(pbSrc, fRevOrder);
			BYTE bPnMask[PN_MASK_SIZE];
			nDataLen = ParserMsParam(pbSrc, bPnMask, sizeof(bPnMask));
			if (nDataLen > 0)
				pbSrc += nDataLen;
			else
				goto OoScanData_err;
			break;

		case DT_SID://SID
			pbSrc++;	//double-long-unsigned
			pbSrc += 4;
			WORD wLen;
			nDataLen = DecodeLength(pbSrc, (DWORD *)&wLen);
			pbSrc += nDataLen+wLen;
			break;

		case DT_SID_MAC://SID_MAC
			//SID
			pbSrc++;	//double-long-unsigned
			pbSrc += 4;
			nDataLen = DecodeLength(pbSrc, (DWORD *)&wLen);
			pbSrc += nDataLen+wLen;
			//MAC
			nDataLen = DecodeLength(pbSrc, (DWORD *)&wLen);
			pbSrc += nDataLen+wLen;
			break;

		case DT_COMDCB://COMDCB,����ͨѶ����
			/*pbSrc+=2;	//enm����+������
			pbSrc+=2;	//enm����+У��λ
			pbSrc+=2;	//enm����+����λ
			pbSrc+=2;	//enm����+ֹͣλ
			pbSrc+=2;	//enm����+����*/
			pbSrc++;	//������
			pbSrc++;	//У��λ
			pbSrc++;	//����λ
			pbSrc++;	//ֹͣλ
			pbSrc++;	//����
			break;
		case DT_ACQ_TYPE:
//			pbSrc++;	//struct ȡ�ø�ʽ��ʱ���Ѿ�ȡ��
			pbSrc++;	//struct num
			pbSrc++;	//unsigned
			bAcqTyp = *pbSrc++;
			if (bAcqTyp==0 || bAcqTyp==2)
			{
				pbSrc++;	//NULL
			}
			else if (bAcqTyp == 1)
			{
				pbSrc += 2;
			}
			else
			{
				pbSrc += 4;	//TI
			}
			break;
		case DT_MTR_ANNEX:
			BYTE bMtrAnnex;	
// 			if (*pbSrc++ != DT_ARRAY)	//array
// 			{
// 				wErr = FMT_ERR_TYPE;
// 				goto OoScanData_err;
// 			}
			bMtrAnnex = *pbSrc++;	//array����
			for (BYTE i=0; i<bMtrAnnex; i++)
			{
				BYTE bDataLen;
				BYTE bDataLen2;
				pbSrc++;    //struct type   add by lisonwu  20170411
				pbSrc++;    //struct num    add by lisonwu  20170411
				pbSrc++;	//FMT OAD
				bDataLen = OoGetDataLen(DT_OAD, pbSrc);
				pbSrc += 4;	//OAD
				bDataLen2 = OoGetDataTypeLen(pbSrc);
				if (bDataLen2 != bDataLen)  //�ж��ڲ����ݳ������ⲿ��վ�·����ݳ���һ���ԣ���һ���򱨴� add by lisonwu 20170411
					goto OoScanData_err;
				pbSrc += bDataLen2;
			}
			break;
		case DT_RPT_TYPE:
			//			pbSrc++;	//struct ȡ�ø�ʽ��ʱ���Ѿ�ȡ��
			pbSrc++;	//struct num
			pbSrc++;	//unsigned
			BYTE bRptType;	
			bRptType = *pbSrc++;
			if (bRptType==0 )
			{
				pbSrc += 5;
			}
			else
			{
				pbSrc += 2;
				pbSrc += 5;
				pbSrc += OoGetDataTypeLen(pbSrc);
				pbSrc += OoGetDataTypeLen(pbSrc);
			}
			break;
		case DT_OVER_PARA:
			BYTE bDataNum;
			bDataNum = *pbSrc;
			pbSrc++;	

			//bSrcLen = 0;
			for (BYTE j=0; j<bDataNum; j++)
			{
				bSrcType = *pbSrc ++;
				if (bSrcType==3 || bSrcType==13 || bSrcType==15 || bSrcType==17 || bSrcType==DT_ENUM)
					pbSrc += 1;
				else if (bSrcType==DT_LONG || bSrcType==DT_LONG_U || bSrcType==80)
					pbSrc += 2;
				else //if (bSrcType==5 || bSrcType==6 || bSrcType==7 || bSrcType==DT_FLOAT32 || bSrcType==31)
					pbSrc += 4;
			}
			break;
		case DT_OVER_RES:
			BYTE bResNum;
			bResNum = *pbSrc ++;
			for (BYTE j=0; j<bResNum; j++)
			{
				pbSrc += 12;
			}
			break;
		case DT_INSTANCE:
			if (bSrcType==3 || bSrcType==13 || bSrcType==15 || bSrcType==17 || bSrcType==DT_ENUM)
				pbSrc += 1;
			else if (bSrcType==DT_LONG || bSrcType==DT_LONG_U || bSrcType==80)
				pbSrc += 2;
			else //if (bSrcType==5 || bSrcType==6 || bSrcType==7 || bSrcType==DT_FLOAT32 || bSrcType==31)
				pbSrc += 4;

			break;
		case DT_SCH_MTR_ANNEX:
			//pbSrc++;	�����Ѿ�ȡ����Arry
			BYTE bArryNum;
			bArryNum = *pbSrc++;
			for (BYTE i=0; i<bArryNum; i++)
			{
				if (*pbSrc++ != DT_STRUCT)
					goto OoScanData_err;
				if (*pbSrc++ != 2)
					goto OoScanData_err;
				if (*pbSrc++ != DT_OAD)
					goto OoScanData_err;
				int iRet = OoGetDataTypeLen(pbSrc);
				if (iRet < 0)
					goto OoScanData_err;
				pbSrc += iRet;
			}
			break;
		case DT_EVTACQ_TYPE:
			pbSrc++;	//struct num
			pbSrc++;	//unsigned
			bAcqTyp = *pbSrc++;
			if (bAcqTyp == 1)
			{
				pbSrc++;	//NULL
			}
			else
			{
				pbSrc++;	//array����
				bNum = *pbSrc++;

				for (BYTE i=0; i<bNum; i++)
				{
					pbSrc++; //ROAD
					nDataLen = ScanROAD(pbSrc, fRevOrder);	//�ú����Զ��޸���pbSrcƫ��
					if (nDataLen > 0)
						pbSrc += nDataLen;
					else
						goto OoScanData_err;
				}
			}
			break;
		default:
			wErr = FMT_ERR_UNK_TYPE;	//δ֪����
			goto OoScanData_err;
		}
		
		if (fPop)	//��Ҫ��ջ
		{
			while (iSP > 0)
			{
				bLen = bStack[--iSP];	//����
				bType = bStack[--iSP];	//����
				if (bType == 0x01)		//array
				{
					bTmpFP = bStack[--iSP];
				}

				if (bLen > 0)
					bLen--;

				if (bLen > 0)	//�ṹ�����黹û��
				{
					BYTE bOldFP = bFP; //�ƶ����FP
					if (bType == 0x01)		//array
					{
						bFP = bTmpFP;
						bStack[iSP++] = bFP;
					}
					
					bStack[iSP++] = bType;	//����
					bStack[iSP++] = bLen;	//����

					if (bPushCnt == 1)
					{
						wIdx++;
						if (wIdx==iIndex && !fFind)	//��������������
						{
							pbStart = pbSrc;
							if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
								*ppFieldFmt = &pbFmt[bFP];

							fFind = true;
						}

						if (bPushCnt==1 && iIndex>=0 && wIdx>iIndex) //ɨ������Ҫ�ҵ�Ԫ��
						{
							*pwLen = pbSrc - pbStart;
							nRetLen = pbStart - pbSrc0;		//ֱ�ӷ��ػ����ƫ��
							if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
								*pwFieldFmtLen = &pbFmt[bOldFP] - *ppFieldFmt;

							return nRetLen;					//ֱ�ӷ��ػ����ƫ��
						}
					}
					break;		//������ջѭ��,������һ���ṹ��Ա������Ԫ��
				}
				else 	//�ṹ����������ˣ���������һ����Ա�Ľ���
				{
					bPushCnt--;  //��ջһ���ṹ������
				}

				if (bSrcType==DT_NULL && iNullSP==iSP) //null data
					bSrcType = DT_UNCARE; //don't care

				//���ĵ�һ��Ԫ�أ��ж�wIdx�Ƿ�Ҫ����
				if ((iSP==0 && bPushCnt==0))   //(iSP!=0 && bPushCnt==1) || 
				{
					wIdx++;
					if (iIndex >= 0)	//������������
					{
						if (wIdx == iIndex)	//��������������
						{
							pbStart = pbSrc;
							if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
								*ppFieldFmt = &pbFmt[bFP];

							fFind = true;
							break;
						}
						else if (wIdx > iIndex)
						{
							*pwLen = pbSrc - pbStart;
							nRetLen = pbStart - pbSrc0;		//ֱ�ӷ��ػ����ƫ��
							if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
								*pwFieldFmtLen = &pbFmt[bFP] - *ppFieldFmt;

							return nRetLen;					//ֱ�ӷ��ػ����ƫ��
						}
					}

					//if (bLen == 0)	//���Ǿ�������ĳ������ͬʱ����߲�����Ԫ�ر������
					//	break;
				}
			}//end while(iSP > 0)

		}

		if (bSrcType==DT_NULL && iNullSP==iSP) //null data
			bSrcType = DT_UNCARE; //don't care

	} while (iSP>0 && pbFmt+bFP<pbFmtEnd);
	
	nRetLen = pbSrc-pbSrc0;
	pbSrc = pbSrc0;
	return nRetLen;

OoScanData_err:
   	DTRACE(DB_FAPROTO, ("OoDataFieldScan : %s, FP=%d, SP=%ld, src=%d\n",
						OIFmtErrToStr(wErr), 
						bFP, iSP, 
						pbSrc-pbSrc0)); 
	pbSrc = pbSrc0;
	
	return -1;
}

//������ɨ�����ݣ�����ͬ���͵ĳ��ȼ�ƫ�Ʒ���
//������pbSrc��Դ����buf��pbFmt�����ݸ�ʽ��wFmtLen�����ݸ�ʽ����
//���أ��������ظ��������͵ĳ��ȣ����򷵻ظ���
int OoDataFieldScan(BYTE* pbData, BYTE* pbFmt, WORD wFmtLen)
{
	WORD wLen;
	BYTE bType;

	int nRet = OoScanData(pbData, pbFmt, wFmtLen, false, -1, &wLen, &bType);
	return nRet;
}


//����:��һ����ʽ������ȡ������ĳ���ֶε�ָ�룬���ý����ֶ����ݵĻ�������
//		�����OoReadField()����ʡ�ڴ�
//������@pbSrc		Դ����
//	    @pFmt		Դ���ݵĸ�ʽ������
//	    @wFmtLen	Դ���ݵĸ�ʽ�������ĳ���
//	    @wIndex		�ֶε�����
//	    @pwLen		���������ֶε����ݳ���
//	    @pbType		�������ظ��ֶε����ͣ���Ҫ����ֶ��д���CHOICE�����ͣ������õ���ʵ������
//      @ppFieldFmt ��iIndex>=0ʱ�����������ֶεĸ�ʽ��,�������Ҫ���Դ���NULL
// 	    @pwFieldFmtLen ��iIndex>=0ʱ�����������ֶεĸ�ʽ���ĳ���,�������Ҫ���Դ���NULL
//����:�����ȷ�򷵻�ָ���ֶε�ָ��,���򷵻�NULL
BYTE* OoGetField(BYTE* pbData, BYTE* pbFmt, WORD wFmtLen, WORD wIndex, WORD* pwLen, BYTE* pbType, BYTE** ppFieldFmt, WORD* pwFieldFmtLen)
{
	BYTE* pbtr = pbData;
	int nRet = OoScanData(pbData, pbFmt, wFmtLen, false, wIndex, pwLen, pbType, ppFieldFmt, pwFieldFmtLen);
	if (nRet > 0)
		return pbtr+nRet;
	else
		return NULL;
}

//����: ��һ����ʽ�������޸�����ĳ���ֶε�����
//������@pbSrc	Դ����
//	   @ pFmt	Դ���ݵĸ�ʽ������
//	   @ wFmtLen	Դ���ݵĸ�ʽ�������ĳ���
//	   @wIndex �ֶε�����
//	   @pbField	Ҫ�޸ĵ��ֶ�����
int OoWriteField(BYTE* pbSrc, WORD wSrcLen, BYTE* pFmt, WORD wFmtLen, WORD wIndex, BYTE* pbField, WORD wFieldLen)
{
	WORD wLen;
	WORD wBakLen;
	BYTE bBakSrc[256];
	BYTE bType;
	BYTE *pbTmp;

	pbTmp = OoGetField(pbSrc, pFmt, wFmtLen, wIndex, &wLen, &bType);
	if (pbTmp != NULL)
	{
		wBakLen = wSrcLen - ((pbTmp+wLen) - pbSrc);
		if (wBakLen > sizeof(bBakSrc))
			return -1;
		memset(bBakSrc, 0, sizeof(bBakSrc));
		memcpy(bBakSrc, pbTmp+wLen, wBakLen);	//����Դ����pbSrc��wIndex���������
		memcpy(pbTmp, pbField, wFieldLen);	//�滻���ֶ�����
		memcpy(pbTmp+wFieldLen, bBakSrc, wBakLen);	//׷���޸��ֶκ������

		return OoScanData(pbSrc, pFmt, wFmtLen, false, -1, NULL, NULL);
	}
	
	return -1;
}

//����: ��һ�������ṹ�ĳ�Ա��������ֵ����֧�ֽṹǶ��
//������@pbSrc	Դ����
//	   @ pFmt	Դ���ݵĸ�ʽ������
//	   @ wFmtLen	Դ���ݵĸ�ʽ�������ĳ���
//	   @ piVal �������ط�����������ֵ
//����:�����ȷ�򷵻س�Ա�ĸ���,���򷵻�-1
//����:�����ȷ�򷵻����ݵĳ��ȣ�������ʽ�ַ���,���򷵻�-1
int OoParseVal(BYTE* pbSrc, BYTE* pFmt, WORD wFmtLen, int* piVal)
{
	int iLen;
	int *piVal0 = piVal;
	BYTE bFmtType;
	BYTE bNum;
	BYTE bSubFmtType;
	BYTE *pbSrc0 = pbSrc;//tll

	bFmtType = *pFmt++;
	switch (bFmtType)
	{
	case DT_STRUCT:
		if (*pbSrc++ != DT_STRUCT)
			goto RET_PARSE;
		bNum = *pbSrc++;	//��Ա����
		if (*pFmt++ != bNum)
			return -1;
		for (BYTE i=0; i<bNum; i++)
		{
			if (OoParseVal(pbSrc, pFmt, 1, piVal) < 0)
				return -1;
			pFmt++;	//������ܻ�������,oct-string�������ݳ��˸�ʽ���������ݳ��ȣ�CL-20161117
			piVal++;
			pbSrc += OoGetDataTypeLen(pbSrc);
		}
		break;
	case DT_ARRAY:
		if (*pbSrc++ != DT_ARRAY)
			goto RET_PARSE;
		bNum = *pbSrc++;	//��Ա����
		if (*pFmt++ != bNum)
			return -1;
		bSubFmtType = *pbSrc;
		for (BYTE i=0; i<bNum; i++)
		{
			if (OoParseVal(pbSrc, &bSubFmtType, 1, piVal) < 0)
				return -1;
			piVal++;
			pbSrc += OoGetDataTypeLen(pbSrc);
		}
		break;
	case DT_DB_LONG:
	case DT_DB_LONG_U:
	case DT_FLOAT32:
		bFmtType = *pbSrc++;
		if (bFmtType!=DT_DB_LONG && bFmtType!=DT_DB_LONG_U && bFmtType!=DT_FLOAT32)
			goto RET_PARSE;
		*piVal++ = OoOadToDWord(pbSrc);
		pbSrc += 4;
		break;
	case DT_LONG_U:
	case DT_LONG:
		bFmtType = *pbSrc++;
		if (bFmtType!=DT_LONG_U && bFmtType!=DT_LONG)
			goto RET_PARSE;
		*piVal++ = OoOiToWord(pbSrc);
		pbSrc += 2;
		break;
	case DT_UNSIGN:
	case DT_INT:
		bFmtType = *pbSrc++;
		if (bFmtType!=DT_UNSIGN && bFmtType!=DT_INT)
			goto RET_PARSE;
		*piVal++ = *pbSrc++;
		break;
	default:
		DTRACE(DB_FAPROTO, ("OoParseVal: unsupport FmtTyp=%d.\n", bFmtType));
		goto RET_PARSE;
	}

	//iLen = piVal - piVal0;
	iLen = pbSrc - pbSrc0;
	piVal = piVal0;
	return iLen;

RET_PARSE:
	return -1;
}

//����: �ѵ�����ֵ���͡�һ�������ṹ�ĳ�Ա��������ֵ����֧�ֽṹǶ��
//������@dwOAD	 ������������������
//	   @ piVals �������ط�����������ֵ������
//	   @ wValMax  piVals����������յĸ������������ش���
//����:�����ȷ�򷵻س�Ա�ĸ���,���򷵻�-1
int OoReadVal(DWORD dwOAD, int* piVals, WORD wValMax)
{
	int iRet;
	int iVals[64];
	WORD wOI, wFmtLen, wLen;
	WORD wFieldFmtLen;
	BYTE bBuf[256];
	BYTE bAttr, bIdx, bType;
	BYTE *pbFmt;
	BYTE *pFieldFmt;

	wOI = dwOAD>>16;
	bAttr = dwOAD>>8;
	bIdx = dwOAD&0xff;

	memset(bBuf, 0, sizeof(bBuf));
	memset((BYTE*)&iVals, 0, sizeof(iVals));
	if (OoReadAttr(wOI, bAttr, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		if (bIdx != 0)	//ȡ������
		{
			BYTE *p = OoGetField(bBuf, pbFmt, wFmtLen, bIdx-1, &wLen, &bType, &pFieldFmt, &wFieldFmtLen);
			if (p == NULL)
				goto Ret_OoReadVal;

			//if (wValMax > wFieldFmtLen)//tll ���Ǿ��ò�����������ж��쳣
			//	goto Ret_OoReadVal;

			iRet = OoParseVal(p, pFieldFmt, wFmtLen, iVals);
			if (iRet<0)// && (wValMax>iRet))
				goto Ret_OoReadVal;
			for (BYTE j=0; j<wValMax; j++)//tll
			{
				piVals[j] = iVals[j];
			}
			
			/*if (iRet > wValMax)//tll
				memcpy(piVals, iVals, wValMax);
			else
				memcpy(piVals, iVals, iRet);*/
			
		}
		else
		{
			//if (wValMax > wFmtLen)//tll ���Ǿ��ò�����������ж��쳣
			//	goto Ret_OoReadVal;
			
			iRet = OoParseVal(bBuf, pbFmt, wFmtLen, iVals);
			//if ((iRet<0) && (wValMax>iRet))
			if (iRet<0)// || (iRet>wValMax))//tll
				goto Ret_OoReadVal;
			for (BYTE j=0; j<wValMax; j++)//tll
			{
				piVals[j] = iVals[j];
			}
			/*if (iRet > wValMax)//tll
				//memcpy(piVals, iVals, wValMax);// ������ֻ�ǰ��ֽڸ������ˣ����ǰ�int�ĵ�Ԫ������
			else
				memcpy(piVals, iVals, iRet);
			*/
		}

		return iRet;
	}

Ret_OoReadVal:

	return -1;
}


//����: ��ȡData���͵�ֵ
//������@ pbData	 Э���ʽ������
//	   @ piVal �������ط�����������ֵ
//����:�����ȷ�򷵻����ݣ�������ʽ�����ֽڳ���,���򷵻�-1
int OoGetDataVal(BYTE* pbData, int* piVal)
{
	BYTE bFmt;

	bFmt = *pbData;
	return OoParseVal(pbData, &bFmt, 1, piVal);
}

//����: ֵ��Э���ʽ��ת��
//������@bValType��������
//	   @ pbData	 ��������Э���ʽ������,������ʽ�ֽ�
//	   @ piVal �������ط�����������ֵ
//����:�����ȷ�򷵻����ݣ�������ʽ�����ֽڳ���,���򷵻�-1
int OoValToFmt(BYTE bValType, int iVal, BYTE* pbData)
{
	WORD wLen;
	BYTE *pbData0 = pbData;

	*pbData++ = bValType;
	switch (bValType)
	{
	case DT_DB_LONG:
	case DT_DB_LONG_U:
	case DT_FLOAT32:
		pbData += OoDWordToOad(iVal, pbData);
		break;
	case DT_LONG_U:
	case DT_LONG:
		pbData += OoWordToLongUnsigned(iVal, pbData);
		break;
	case DT_UNSIGN:
	case DT_INT:
		*pbData++ = (BYTE)iVal;
		break;
	default:
		DTRACE(DB_FAPROTO, ("OoValToFmt: unsupport FmtTyp=%d.\n", bValType));
		goto OoValToFmt_RET;
	}
	
	wLen = pbData - pbData0;
	pbData = pbData0;
	return wLen;

OoValToFmt_RET:
	return -1;
}

//����: ȡ��dwOAD����ֵ����
//������@ dwOAD	 ��������������
//	   @ pbValType����������������
//���أ������ȷ������ֵ�����򷵻�true,���򷵻�false
bool OoGetValType (DWORD dwOAD, BYTE* pbValType)
{
	WORD wFmtLen;
	WORD wLen;
	WORD wOI;
	BYTE bAttr;
	BYTE bIndex;
	BYTE *pbFmt;
	BYTE bType;
	BYTE bBuf[256];

	wOI = (dwOAD>>16)&0xffff;
	bAttr = (dwOAD>>8)&0xff;
	bIndex = dwOAD&0xff;
	
	if (OoReadAttr(wOI, bAttr, bBuf, &pbFmt, &wFmtLen) <= 0)
		return false;

	if (bIndex != 0)	//������
	{
		if (OoGetField(bBuf, pbFmt, wFmtLen, bIndex-1, &wLen, &bType) == NULL)
			return false;
		*pbValType = bType;
	}
	else
	{
		*pbValType = pbFmt[0];
	}

	return true;
}


//����:��һ����ʽ�����ж�ȡ����ĳ���ֶε����ݣ�����Ӧ�ò��һ���ṹ���������������ȡ�ֶ�ֵ������Ԫ�أ��ֶ�/Ԫ��ֻ���ǵ�һ����ֶ�/Ԫ�أ����ܴ���Ƕ�׵��ڶ�����ֶ�/Ԫ�ء�
//������@pbSrc	Դ����
//	   @pFmt	Դ���ݵĸ�ʽ������
//	   @wFmtLen	Դ���ݵĸ�ʽ�������ĳ���
//	@wIndex�ֶε�����
//	   @pbField	���������ֶεĻ�����
//	   @pbType	�������ظ��ֶε����ͣ���Ҫ����ֶ��д���CHOICE�����ͣ�
//                   �����õ���ʵ������
//����:�����ȷ�򷵻��ֶεĳ���,���򷵻ظ���
int OoReadField(BYTE* pbSrc, BYTE* pbFmt, WORD wFmtLen, WORD wIndex, BYTE* pbField, BYTE* pbType)
{
	WORD wLen=0;
	BYTE* pbTmp = OoGetField(pbSrc, pbFmt, wFmtLen, wIndex, &wLen, pbType);
	if (pbTmp != NULL)
	{
		memcpy(pbField, pbTmp, wLen);
		return wLen;
	}

	return -1;
}

//����:Ӧ�ò���õ�д��������ֵ���൱��ֱ�Ӱ�����д�����ݿ�
//������@wOI	�����ʶ
//		@bAtrr	���Ա�ʶ�������� bit-string��SIZE��8����
//		@bIndex������Ԫ������-----��ʱ���ÿ��ǰ�
//		@pbBuf	Ҫд������ֵ�Ļ�������Ҫд������ΪӦ���ֽ�˳��
int OoWriteAttr2(WORD wOI, BYTE bAtrr, BYTE* pbBuf)
{
	DWORD dwOIAtt = ((DWORD )wOI<<16) + ((DWORD )bAtrr<<8);
	const ToaMap* pOI = GetOIMap(dwOIAtt);
	if (pOI == NULL)
		return -1;

	int nRet = WriteItemEx(BN0, pOI->wPn, pOI->wID, pbBuf);
	return nRet;
}

//����OMD������Ҫɨ���ʽ
bool IsSpecOMD(const TOmMap* pOmMap)
{
	switch(pOmMap->dwOM)
	{
	case 0x21000100:	//ͳ�Ƹ�λ
	case 0x21010100:
	case 0x21020100:
	case 0x21030100:
	case 0x21040100:
	case 0x21100100:
	case 0x21110100:
	case 0x21120100:
	case 0x21130100:
	case 0x21140100:
	case 0x21200100:
	case 0x21210100:
	case 0x21220100:
	case 0x21230100:
	case 0x21240100:
	case 0x21300100:
	case 0x21310100:
	case 0x21320100:
	case 0x21330100:
	case 0x21400100:
	case 0x21410100:
	case 0x22000100:
	case 0x22030100:
	case 0x22040100:	//��λ��λ�������
	case 0x43000100:	//�豸�ӿ���19--��λ
	case 0x43000200:	//�豸�ӿ���19--ִ��
	case 0x43000300:	//�豸�ӿ���19--���ݳ�ʼ��
	case 0x43000400:	//�豸�ӿ���19--�ָ���������
	case 0x43000500:	//�豸�ӿ���19--�¼���ʼ��
	case 0x43000600:	//�豸�ӿ���19--������ʼ��
	case 0x45000100:	//�����豸��ʼ��
	case 0x45010100:	//�����豸��ʼ��
	case 0x45100100:	//��̫���豸��ʼ��
	case 0x45110100:	//��̫���豸��ʼ��
	case 0x45120100:	//��̫���豸��ʼ��
	case 0x45130100:	//��̫���豸��ʼ��
	case 0x45140100:	//��̫���豸��ʼ��
	case 0x45150100:	//��̫���豸��ʼ��
	case 0x45160100:	//��̫���豸��ʼ��
	case 0x45170100:	//��̫���豸��ʼ��
	case 0x60008100:	//�������õ�Ԫ�Ļ�����Ϣ����
	case 0x60008200:	//�������õ�Ԫ����չ��Ϣ�Լ�������Ϣ������ΪNULL��ʾ������
	case 0x60008300:	//ɾ�����õ�Ԫ��ͨ���������ɾ��
	case 0x60008400:	//ɾ�����õ�Ԫ��ͨ��������Ϣ����ɾ��
	case 0x60008500:	//ɾ�����õ�Ԫ��ͨ��ͨ�ŵ�ַ���˿�ɾ��
	case 0x60008600:	//ɾ�����е��
	case 0x60028000:	//���̨���ɼ�����
	case 0x60028100:	//��տ�̨���ɼ�����
	case 0x60128000:	//���ɾ��һ�����õ�Ԫ
	case 0x60128100:	//�������������ñ�
	case 0x60148000:	//ɾ��һ����ͨ�ɼ�����
	case 0x60148100:	//�����ͨ�ɼ�������
	//case 0x60148200:	//���÷����ļ�¼��ѡ��
	case 0x60168000:	//ɾ��һ���¼��ɼ�����
	case 0x60168100:	//����¼��ɼ�������
	case 0x60187f00:	//��Ӹ���һ��͸�����������һ�鷽������
	case 0x60188000:	//���һ�鱨��
	case 0x60188100:	//ɾ��һ��������һ�鷽������
	case 0x60188200:	//ɾ��һ��͸������
	case 0x60188300:	//���͸��������
	case 0x601C8000:	//ɾ��һ���ϱ�����
	case 0x601C8100:	//����ϱ�������
	case 0x601E7f00:	//���Ӳɼ����򷽰�
	case 0x601E8000:	//ɾ��һ��ɼ����򷽰�
	case 0x601E8100:	//��ղɼ����򷽰�
	case 0x80017F00:	//Ͷ�뱣��
	case 0x80018000:	//�������
	case 0x80018100:	//����Զ�����
	case 0x80028000:	//ȡ���߷Ѹ澯
	case 0xF0010700:	//OI=F001 ������07	��������
	case 0xF0010800:
	case 0xF0010900:
	case 0xF0010A00:
	case 0xF0020700:
	case 0xF0020800:	//OI=F000 ������08	�����ļ�
		return true;
	default:
		return false;
	}

	return false;
}

//����:��ͨ��Э����õ�ִ��ĳ������ķ���
//������@wOI		�����ʶ
//	   @bMethod	���󷽷����
//	   @ bOpMode	����ģʽ
//	   @pbPara		������������ִ�н��          DAR
//	@piParaLen	��������pbParaɨ����ĳ��ȣ������֡ʱ������һ��λ��
//	@pbRes		�������أ�����ִ�н��DAR��������������Data  OPTIONAL���ĳ���
//���أ������ȷ�򷵻�pbRes�н���ĳ���,���򷵻ظ���
int DoObjMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int* piParaLen, BYTE* pvAddon, BYTE* pbRes)
{
	//�������󷽷���Ӧ��ӳ���
	const TOmMap* pOmMap = GetOmMap((((DWORD )wOI)<<16)+(((DWORD )bMethod<<8)));
	if (pOmMap == NULL)
		return -1;

	*piParaLen = 0;

	int iParaLen = 0;
	if (!IsSpecOMD(pOmMap))
	{
		if (pbPara != NULL)	//hyl pbPara�п������Ϊ��
		{
			//ɨ���������������ȡ����ֽ�˳����е���
			iParaLen = OoDataFieldScan(pbPara, pOmMap->pFmt, pOmMap->wFmtLen);
			if (iParaLen< 0)
				return -1;
		}
	}

	//ִ�ж��󷽷�
	return pOmMap->pfnDoMethod(wOI, bMethod, bOpMode, pbPara, iParaLen, pOmMap->pvAddon, pOmMap->pFmt, pOmMap->wFmtLen, pbRes, piParaLen);
}

//��������Դ���ݽ�����һ�����ֶε�ƫ�ƺͳ��ȣ��������
//������@pParser	�ֶν�����
//	   @pFmt	Դ���ݵĸ�ʽ������
//	   @wFmtLen	Դ���ݵĸ�ʽ�������ĳ���
//	   @fParseItem	Ϊfalseʱֻ���������ñ���
//					Ϊtrueʱ���ֶζ�Ӧ��������ĳ��ȼ�ƫ��Ҳ�������
//����:�����ȷ�򷵻�true,���򷵻�false
bool OoParseField2(TFieldParser* pParser, BYTE* pFmt, WORD wFmtLen, bool fParseItem)
{
	return true;
}

//��������ȡ�ֶν�������ĳ���ֶ�
//������@pParser	�ֶν�����
//	   @wIndex�ֶε�����
//	   @pbBuf���������ֶε�����
//	   @pbType���������ֶε�����
//	   @pwDataLen���������ֶε����ݳ���
//����:�����ȷ�򷵻��ֶ���������ĳ���,���򷵻ظ���
int ReadParserField(TFieldParser* pParser, WORD wIndex, BYTE* pbBuf, BYTE* pbType, WORD*  pwItemOffset, WORD* pwItemLen)
{
	if (wIndex > pParser->wNum)
		return -1;

	memcpy(pbBuf, &pParser->pbCfg[pParser->wPos[wIndex]], pParser->wLen[wIndex]);
	*pbType = pParser->bType[wIndex];
	*pwItemOffset = pParser->wItemOffset[wIndex];
	*pwItemLen = pParser->wItemLen[wIndex];

	return pParser->wLen[wIndex];
}

int OoReadAttr2(WORD wOI, BYTE bAtrr, BYTE* pbBuf, BYTE** ppFmt, WORD* pwFmtLen)
{
	return 0;
}

//����������Э���RCSD�ڲ�����OAD���� �磺01 50 04 02 00 02 00 10 02 00 00 20 02 00
//		�ڲ���������Ϊ02
int OoScanRcsdInOadNum(BYTE *pbRcsd)
{
	BYTE bNum;

	if (*pbRcsd++ != 0x01)
		return -1;
	pbRcsd += 4;	//��OAD
	
	bNum = *pbRcsd;

	return bNum;
}

//����: ȡ��������ĳ���
//������@ bType	�������ͣ�ͬData���������Ͷ���
//	   @ pItemDesc������Ŀ����������OAD��ROAD��CSD
//���أ���ȷ�򷵻�������ĳ��ȣ����򷵻ظ���
//����: ȡ��������ĳ���
//������@ bType	�������ͣ�ͬData���������Ͷ���
//	   @ pItemDesc������Ŀ����������OAD��ROAD��CSD
//���أ���ȷ�򷵻�������ĳ��ȣ����򷵻ظ���
int OoGetDataLen(BYTE bType, BYTE* pItemDesc)
{
	const ToaMap *pOAMap = NULL;
	DWORD dwOIAtt;
	WORD wFmtLen, wLen;
	int iDataLen = -1;
	int iRet;
	BYTE *pbPtr = pItemDesc;
	BYTE bChoice;
	BYTE bOADNum;
	BYTE bCSDNum;
	BYTE bNum;
	BYTE bIndex;
	BYTE bBuf[128];
	BYTE *pbFmt;

	switch(bType)
	{
	case DT_ARRAY:
		iDataLen = 0;
		bNum = *pbPtr++;
		for (BYTE i = 0; i < bNum; i++)
		{
			iDataLen += OoGetDataLen(pbPtr[0], pbPtr+1);
			pbPtr += OoGetDataTypeLen(pbPtr);
		}
		break;

	case DT_OAD: //OAD
		iDataLen = 0;
		dwOIAtt = OoOadToDWord(pbPtr);
		dwOIAtt &= OAD_FEAT_MASK;	//��ȡOAD, ֻ���ε���������
		bIndex = (BYTE)(dwOIAtt & 0xff);
		pOAMap = GetOIMap(dwOIAtt);
		if (pOAMap != NULL)
		{
			if ((iRet=GetItemLen(BANK0, pOAMap->wID)) < 0)
				return -1;
			iDataLen = iRet;
		}
		else
		{
			if (bIndex == 0)
				return -1;
			else
			{
				//��������
				pOAMap = GetOIMap(dwOIAtt & 0xFFFFFF00);
				if (pOAMap != NULL)
				{
					memset(bBuf, 0, sizeof(bBuf));

					iDataLen = OoReadAttr(dwOIAtt>>16, (dwOIAtt>>8)&0xff, bBuf, &pbFmt, &wFmtLen);
					if (iDataLen > 0)	//����OI��ID������ľ��ǵ�N������
					{
						if (!IsNeedRdSpec(pOAMap))	//������OI
						{
							pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, bIndex-1, &wLen, &bType);	
							if (pbPtr == NULL)
								return -1;
							iRet = OoGetDataTypeLen(pbPtr);
							if (iRet < 0)
								return -1;

							iDataLen = iRet;
						}
					}
				}
			}
		}
		break;
	case DT_ROAD: //ROAD
		iDataLen = GetEvtMainOadDataLen(OoOadToDWord(pbPtr));	//�¼�ROAD��������OAD�������ݳ���ͳ�Ʒ�Χ
		pbPtr += 4;
		bOADNum = *pbPtr++;
		//iDataLen = 0;
		for (BYTE i = 0; i < bOADNum; i++)
		{
			if ((iRet=OoGetDataLen(DT_OAD, pbPtr)) < 0)
				return -1;
			iDataLen += iRet;
			pbPtr += 4;
		}
		break;
	case DT_CSD: //CSD
		iDataLen = 0;
		bChoice = *pbPtr++;
		if (bChoice == 0)	//OAD
		{
			if ((iRet=OoGetDataLen(DT_OAD, pbPtr)) < 0)
				return -1;
			iDataLen = iRet;
		}
		else	//ROAD
		{
			if ((iRet=OoGetDataLen(DT_ROAD, pbPtr)) < 0)
				return -1;
			iDataLen = iRet;
		}
		break;
	case DT_RCSD: //RCSD
		iDataLen = 0;
		bCSDNum = *pbPtr++;
		for (BYTE i = 0; i < bCSDNum; i++)
		{
			if ((iRet=OoGetDataLen(pbPtr[0], pbPtr+1)) < 0)
				return -1;
			iDataLen += iRet;
			//����RCSD��ÿ��CSD��ƫ��
			if (*pbPtr++ == 1)	//choice
			{
				pbPtr += 4;
				bOADNum = *pbPtr++;
				pbPtr += (bOADNum*4);
			}
			else
			{			
				pbPtr += 4;
			}
		}
		break;
	default:
		DTRACE(DB_FA, ("Error---Can`t support fmt=%d.\n", bType));
		return -1;
	}

	return iDataLen;
}

//������ͨ���������ݷ��ظ�ʽ����(��Ҫ��Բɼ�����)
//������@pbBuf Դ����
//		@pbFmtBuf ���صĸ�ʽ������
//		@���صĸ�ʽ����������
//���أ�<0ʧ�ܣ�>=0��ʾĳ����ʽ��pbBuf��ռ�ݵ�ƫ��
int OoScanFmt(BYTE *pbBuf, BYTE *pbFmtBuf, WORD &wFmtLen)
{
	BYTE *pbPtr = pbBuf;
	BYTE *pbFmtPtr = pbFmtBuf;
	BYTE bFmtType;

	bFmtType = *pbPtr++;
	switch (bFmtType)
	{
	case 1:	//array
		*pbFmtPtr++ = 1;
		BYTE bArryNum;
		bArryNum = *pbPtr++;
		*pbFmtPtr++ = bArryNum;
		for (BYTE i = 0; i < bArryNum; i++)
		{
			pbPtr += OoScanFmt(pbPtr, pbFmtPtr, wFmtLen);
			pbFmtPtr += wFmtLen; 
		}
		break;
	case DT_OAD:	//OAD
		*pbFmtPtr++ = DT_OAD;
		pbPtr += 4;
		break;
	case DT_ROAD:	//ROAD
		*pbFmtPtr++ = DT_ROAD;
		pbPtr += 4;
		BYTE bROADNum;
		bROADNum = *pbPtr++;
		*pbFmtPtr++ = bROADNum;
		for (BYTE i = 0; i < bROADNum; i++)
		{
			pbPtr += 4;
			pbFmtPtr++; 
		}
		break;
	case DT_CSD:	//CSD
		BYTE bChoice;
		*pbFmtPtr++ = DT_CSD;
		bChoice = *pbPtr++;
		if (bChoice == 0)	//OAD
		{
			pbPtr += 4;
		}
		else	//ROAD
		{
					pbPtr += 4;	//OAD
			BYTE bROADNum = *pbPtr++;
			*pbFmtPtr++ = bROADNum;
			for (BYTE i = 0; i < bROADNum; i++)
			{
				pbPtr += 4;
			}
		}
		break;
	default:
		//������ʽ��Ҫʱ�����
		break;
	}

	wFmtLen = pbFmtPtr-pbFmtBuf;

	return pbPtr - pbBuf;
}

// ��ȡ����OAD
DWORD GetOAD(WORD wOI, BYTE bAtrr, BYTE bIndex)
{
	return ((DWORD )wOI<<16) | ((DWORD )bAtrr<<8) | bIndex;
}

//�ѳ����صľ�����ת���ɶ������ݵĸ�ʽ���������������ַ�
//	@pbSrc �Ǵӱ�˷��ص�BCD��ʽ���ݴ�
//	@pbDst ת���ɶ������ݵĸ�ʽ���������������ַ������ݴ�
//	@pbFmt ����OAD����Ӧ�Ķ����ʽ��
//	@wFmtLen �����ʽ������
//	����ת����pbDst���ܳ���
int OIFmtDataExt(BYTE* pbSrc, BYTE bsLen, BYTE* pbDst, BYTE* pbFmt, WORD wFmtLen, DWORD dwOAD)
{
	BYTE bStack[100];
	BYTE* pbSrc0 = pbSrc;
	BYTE* pbDst0 = pbDst;
	BYTE* pbFmtEnd = pbFmt + wFmtLen;
	int iSP = 0;	//��ջָ��
	BYTE bFP = 0;	//��ʽָ��
	BYTE bTmpFP;
	BYTE bLen, bSrcLen, bArrLen, bByteLen;
	BYTE bFmtT, bVFmt, bVFill;
	bool fVLen;
	BYTE bNullArr = 0;
	WORD wOffset;
	BYTE bSrcType = 0xff;//DT_UNCARE; //don't care
	int iNullSP = 0;	//������������(�������Ϊ0)ʱ��iSP,��ɨ����һ�����ݺ�,
						//iSP�ظ���iNullSP��ֵ,��ʾ�Կ�����Ĵ������
	bool fPop;
	WORD wErr = 0;

	if (pbFmt == NULL)
		return -1;
	
	do
	{
		fPop = true;
		BYTE bType = pbFmt[bFP++];
		
		switch (bType)
		{
		case 1: //array
			bLen = pbFmt[bFP++];
			if (bLen == SPECIAL_NUM)
				bLen = 1;
			//bSrcLen = *pbSrc++;
			//if (bSrcLen == 0)
				bSrcLen = bLen;

			if (bNullArr)	//��һ�������黹û��,�Ӽ����ǿ�����
			{
				bSrcLen = 0;
			}
			else
			{
				if (bSrcLen == 0)	//�������鳤��Ϊ0�Ŀ�����,��0(������)����
				{
					*pbDst++ = 1;
					*pbDst++ = 0;
					bNullArr = 1;
					iNullSP = iSP;
				}
				else
				{
					*pbDst++ = bType;
					*pbDst++ = bSrcLen;
					bNullArr = 0;
				}
			}
			
			wOffset = pbSrc - pbSrc0;	//Խ�����鳤�Ⱥ����ʼλ��
			
			bStack[iSP++] = bLen;		//�ն��д洢����ʱԤ���ļ�¼����,��������Դ�����л��ж��ٱʼ�¼û������
			bStack[iSP++] = wOffset;	//����Դ����������Ԫ�ص���ʼƫ��,��������һ�ʼ�¼�ĳ���
			bStack[iSP++] = wOffset>>8;

			bStack[iSP++] = bFP;
			bStack[iSP++] = bType;
			bStack[iSP++] = bSrcLen; //��ʹ����Ԫ�ظ���Ϊ0,ҲҪ��һ����ջ�Ĳ���,
									 //�ó���ɨ���Ԫ�صĸ�ʽ����
			fPop = false;
			break;

		case 2: //structure
			bLen = pbFmt[bFP++];
			bStack[iSP++] = bType;
			bStack[iSP++] = bLen;
			
			if (!bNullArr)
			{
				*pbDst++ = bType;
				*pbDst++ = bLen;
			}

			fPop = false;
			break;

		case 4: //bit-string
			bLen = pbFmt[bFP++];
			bVFmt = pbFmt[bFP++];
			bFmtT = 0xF0 & bVFmt;	//ZJD 20080116_1437
			bVFmt &= 0x0F;			//ZJD 20080116_1437
			bByteLen = (bLen + 7) / 8;
			fVLen = (bVFmt&VF_VLEN)!=0; 
			bVFill = bVFmt>>VF_FILL_SHF;

			if (bNullArr)
			{
				if (fVLen && bVFill!=0)
					pbSrc++;

				pbSrc += bByteLen;
			}
			else
			{
				//ȷ��bSrcLen�Ϳ�����bByteLen
				if (fVLen)
				{
					if (bVFill == 0) //0-�����,��һ�������ֽڷŵ���ͷ
					{
						bByteLen = *pbSrc++;
					}
					else	//������ֽ������
					{
						bVFill = g_bFillByte[bVFill];
						bByteLen = OIGetStrLen(pbSrc, bByteLen, bVFill);	//bByteLen�����ı�
							//����·���û��ȫ��������,������������ϴ���BITs�������ܸ��·��Ĳ�һ��
					}

					bSrcLen = bByteLen * 8;
				}
				else //����,�������,Ҫ�󳤶ȱ���պ����
				{
					bSrcLen = bLen;
				}
				
				//��֡
				*pbDst++ = bType;
				*pbDst++ = bSrcLen;		//���ݳ���
				if (bByteLen > 0)
				{
					if (bVFmt&VF_LR)
						memcpy(pbDst, pbSrc, bByteLen);		//���������ֽ�
					else
						revcpy(pbDst, pbSrc, bByteLen);

					//------�¼��ⲿ��-------------
					if (bFmtT != 0)
						for (BYTE b=0; b<bByteLen; b++)			//2008-05-25  ��
							pbDst[b] = ByteBitReverse(pbDst[b]);	//���ֽڰ�λ���е���
					//------�¼��ⲿ��-------------
					pbDst += bByteLen;
				}

				//pbSrc += (bLen + 7) / 8 + 1;			//����+����
				pbSrc += (bLen + 7) / 8;			//����+����
			}
			break;

		case 9:	//octet-string            	[9] 		IMPLICIT OCTET STRING,
		case 10: //visible-string
			bLen = pbFmt[bFP++];
			bVFmt = pbFmt[bFP++];
			fVLen = (bVFmt&VF_VLEN)!=0; 
			bVFill = bVFmt>>VF_FILL_SHF;

			if (bNullArr)
			{
				if (fVLen && bVFill!=0)
					pbSrc++;

				pbSrc += bLen;
			}
			else
			{
				if (fVLen)
				{
					if (bVFill == 0) //0-�����,��һ�������ֽڷŵ���ͷ
					{
						bSrcLen = *pbSrc++;
					}
					else	//������ֽ������
					{
						bVFill = g_bFillByte[bVFill];
						bSrcLen = OIGetStrLen(pbSrc, bLen, bVFill);
					}
				}
				else //����,�������,Ҫ�󳤶ȱ���պ����
				{
					bSrcLen = bLen;
				}

				*pbDst++ = bType;
				*pbDst++ = bSrcLen;		//���ݳ���
				if (bSrcLen > 0)
				{
					if (bVFmt&VF_LR)
						memcpy(pbDst, pbSrc, bSrcLen);
					else
						revcpy(pbDst, pbSrc, bSrcLen);

					pbDst += bSrcLen;
				}

				pbSrc += bLen;
			}
			break;

		case 3:  //boolean                	[3]	 	IMPLICIT BOOLEAN    
		case 13: //bcd                   	[13] 	IMPLICIT Integer8,
		case 15: //integer                	[15] 	IMPLICIT Integer8,
		case 17: //unsigned              	[17] 	IMPLICIT Unsigned8,
		case DT_ENUM: //enum 					[22] 	IMPLICIT NULL
			if (bNullArr)	//���ֽ�����Ŀǰ��֧����Ч����
			{
				pbSrc++;
			}
			else
			{
				if (IsAllAByte(pbSrc, INVALID_DATA_MTR, 1) || bsLen==0)
				{
					*pbDst++ = NULL;
					pbDst += 1;
				}
				else
				{
					*pbDst++ = bType;
					*pbDst++ = BcdToByte(*pbSrc++);
					//pbSrc++;
					bsLen--;
				}
			}
			
			break;

		case DT_LONG: //long                  	[16] 	IMPLICIT Integer16,
		case DT_LONG_U: //long-unsigned         	[18] 	IMPLICIT Unsigned16,
			if (bNullArr)
			{
				pbSrc += 2;
			}
			else
			{
				//revcpy(pbDst, pbSrc, 2);
				//pbDst += 2;
				//pbSrc += 2;
				if ((bsLen < 2) && (bsLen!=0))
					return -1;
				if (IsAllAByte(pbSrc, INVALID_DATA_MTR, 2) || bsLen==0)
				{
					*pbDst++ = bType;
					memset(pbDst, 0xFE, 2);//�������Լ������Чֵ					
					pbDst += 2;
					pbSrc += 2;
					bsLen -= 2;
				}
				else
				{
					WORD dwval = 0;
					*pbDst++ = bType;
					dwval = BcdToDWORD(pbSrc,2);
					revcpy(pbDst, (BYTE*)&dwval, 2);
					pbDst += 2;
					pbSrc += 2;
					bsLen -= 2;
				}
			}
			
			break;

		case 5: //double-long             	[5] 		IMPLICIT Integer32
		case 6: //double-long-unsigned    	[6] 		IMPLICIT Unsigned32
//		case 7: //floating-point           	[7] 		IMPLICIT OCTET STRING(SIZE(4))
		case DT_FLOAT32: //float32                	[DT_FLOAT32] 	IMPLICIT OCTET STRING (SIZE(4))
		//case DT_TIME: //time
			if (bNullArr)
			{
				pbSrc += 4;
			}
			else
			{
//				if (IsAllAByte(pbSrc, INVALID_DATA, 4))
//					memset(pbSrc, 0x00, 4);
				//revcpy(pbDst, pbSrc, 4);	//���ý���Ч����EEת��Ϊ0����Ҫ��Ϊ�ֳ��������0����Ч����
				WORD wIO = dwOAD >> 16;
				BYTE btLen = 4;
				if (wIO==0x2001 ||(wIO>=0x2004&&wIO<=0x2009) ||wIO==0x2017 ||wIO==0x2018 ||wIO==0x2019)
					btLen = 3;//��Щ������07Э������3�ֽ�
				if ((bsLen < btLen) && (bsLen!=0))
					return -1;
				if (IsAllAByte(pbSrc, INVALID_DATA_MTR, bsLen) || bsLen==0)
				{
					*pbDst++ = bType;
					memset(pbDst, 0xFE, 4);//�������Լ������Чֵ					
					pbDst += 4;
					pbSrc += btLen;
					bsLen -= btLen;
				}
				else
				{
					DWORD dwval = 0;
					*pbDst++ = bType;
					dwval = BcdToDWORD(pbSrc,btLen);
					if (wIO==0x2500)
						dwval <<= 8;//������ˮ������4��С��λ
					revcpy(pbDst, (BYTE*)&dwval, 4);
					pbDst += 4;
					pbSrc += btLen;
					bsLen -= btLen;
				}
			}

			break;
//#define INVALID_DATA 0xff
		case DT_LONG64: //long64                	[20] 	IMPLICIT Integer64
		case DT_LONG64_U: //long64-unsigned       	[21] 	IMPLICIT Unsigned64
		case DT_FLOAT64: //float64                	[24] 	IMPLICIT OCTET STRING (SIZE(8)),
			if (bNullArr)
			{
				pbSrc += 8;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 8);
				pbDst += 8;
				pbSrc += 8;
			}

			break;

		case DT_DATE_TIME: //date-time
			//����: 19 07 d7 08 07 ff 08 2b 39 ff 80 00 00  
			if (bNullArr)
			{
				pbSrc += 12;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 12);			
				pbDst += 12;
				pbSrc += 12;
			}

			break;

		case DT_DATE: //date
			if (bNullArr)
			{
				pbSrc += 5;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 5);			
				pbDst += 5;
				pbSrc += 5;
			}
			
			break;
		case DT_TIME: //time
			if (bNullArr)
			{
				pbSrc += 4;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 4);
				pbDst += 4;
				pbSrc += 4;
			}
			break;
		case DT_DATE_TIME_S: //time
			if (bNullArr)
			{
				pbSrc += 7;
			}
			else
			{
				*pbDst++ = bType;
				revcpy(pbDst, pbSrc, 7);
				pbDst += 7;
				pbSrc += 7;
			}
			break;
		case DT_OAD://OAD
			*pbDst++ = bType;
			memset(pbDst, 0, 4);
			pbDst += 4;
			break;
		case DT_TSA://TSA

			break;
		case DT_OVER_PARA:
			pbSrc++;	

			*pbDst++ = 0x01;
			*pbDst++ = 0x00;

			break;
		case DT_INSTANCE:
			pbSrc++;	

			*pbDst++ = 0x00;//0x11
			//*pbDst++ = 0x00;

			break;
		case DT_TI:
			*pbDst++ = DT_TI;
			memset(pbDst, 0, 3);
			pbDst += 3;
			break;
		case DT_OVER_RES:
			BYTE bResNum;
			bResNum = *pbSrc ++;
			for (BYTE j=0; j<bResNum; j++)
			{
				pbSrc += 12;
			}
			break;
		default:
			wErr = 4;//FMT_ERR_UNK_TYPE;	//δ֪����
			goto OIFmtData_err;
		}
		
		if (fPop)	//��Ҫ��ջ
		{
			while (iSP > 0)
			{
				bLen = bStack[--iSP];	//����
				bType = bStack[--iSP];	//����
				if (bType == 0x01)		//array
				{	
					bTmpFP = bStack[--iSP];
					wOffset = (WORD )bStack[--iSP] * 0x100;
					wOffset += bStack[--iSP];
					bArrLen = bStack[--iSP];

					if (bArrLen > 0)	//��������Դ�����л��ж��ٱʼ�¼û������
						bArrLen--;
				}

				if (bLen > 0)
					bLen--;

				if (bLen > 0)	//�ṹ�����黹û��
				{
					if (bType == 0x01)	//array
					{
						wOffset = pbSrc - pbSrc0; //���黹û��,wOffset������һ�ʼ�¼����ʼ
												  //λ��,��֤������������һ�ʼ�¼�ĳ���
						bStack[iSP++] = bArrLen;
						bStack[iSP++] = wOffset;
						bStack[iSP++] = wOffset>>8;

						bFP = bTmpFP;
						bStack[iSP++] = bFP;
					}
					
					bStack[iSP++] = bType;	//����
					bStack[iSP++] = bLen;	//����
					break;
				}
				else
				{
					if (bType == 0x01)		//array
					{
						pbSrc += (pbSrc - pbSrc0 - wOffset) * bArrLen;
					}

					//һ���ṹ�е�ȫ���ֶ����ݶ�Ϊ��Ч����0,Ŀǰ�Ȳ��������ṹ����Ϊ0
					//���Ҫ�����Ļ�,ֻ���¼Ŀ�����ݽṹ����ʼ,�жϽṹȫ����Ա�Ƿ�Ϊ0
				}

				//�ýṹ����������,�˵�������һ��
			}
		}

		if (bNullArr && iNullSP==iSP)
			bNullArr = 0; 

	} while (iSP>0 && pbFmt<pbFmtEnd);

	return pbDst-pbDst0;

OIFmtData_err:
   	DTRACE(DB_FAPROTO, ("OIFmtData : %s, FP=%d, SP=%ld, src=%d, dst=%d\n",
						OIFmtErrToStr(wErr), 
						bFP, iSP, 
						pbSrc-pbSrc0, 
						pbDst-pbDst0)); 
	
	return -wErr;
}

