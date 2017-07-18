/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CctRdCtrl.cpp
 * ժ    Ҫ���ز�������ƹ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�CL
 * ������ڣ�2016��8��
 *********************************************************************************************************/

#include "stdafx.h"
#include "CctAPI.h"
#include "LibDbConst.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "CctTaskMangerOob.h"
#include "DbOIAPI.h"
#include "DbFmt.h"

//�������Ƚ���������Ĵ�С
//������@pbSrc Դ���飬��Ӧ�ն��ڲ�����
//		@wSrcLen ���鳤�ȣ���Ӧ�ն��ڲ����ݳ���
//		@pbCmp �Ƚ����飬Э�������
//		@wCmpLen �Ƚϳ��ȣ���ӦЭ������ݳ���
//���أ�0������ȣ�1����Դ���� > �Ƚ����ݣ� 2����Դ���� < �Ƚ�����
int AarryCompare(BYTE *pbSrc, WORD wSrcLen, BYTE *pbCmp, WORD wCmpLen)
{
	int iRet = -1;
	BYTE bSrc, bCmp;
	BYTE bSrcOctNum, bCmpOctNum;
	BYTE bH, bL;
	BYTE bSrcH, bSrcL;
	BYTE bCmpH, bCmpL;

	bSrcOctNum = 0;
	for (BYTE i=0; i<wSrcLen; i++)
	{
		bL = pbSrc[i]&0x0f;
		bH = (pbSrc[i]>>0x04)&0x0f;
		if (bL != 0x0f)
			bSrcOctNum++;
		if (bH != 0x0f)
			bSrcOctNum++;
	}

	bCmpOctNum = 0;
	for (BYTE i=0; i<wCmpLen; i++)
	{
		bL = pbCmp[i]&0x0f;
		bH = (pbCmp[i]>>0x04)&0x0f;
		if (bL != 0x0f)
			bCmpOctNum++;
		if (bH != 0x0f)
			bCmpOctNum++;
	}

	if (bSrcOctNum < bCmpOctNum)	//Դ���ݳ��� < �Ƚ����ݳ���
	{
		return 2;
	}
	else if (bSrcOctNum > bCmpOctNum)	//Դ���ݳ��� > �Ƚ����ݳ���
	{
		return 1;
	}
	else	//Դ���ݳ��� = �Ƚ����ݳ��ȣ����������ľ����������ݱȽ�
	{
		if (memcmp(pbSrc, pbCmp, wCmpLen) == 0)
		{
			iRet = 0;
		}		
		else
		{
			for (WORD i=0; i<bCmpOctNum; i++)
			{
				bSrc = *pbSrc++;
				bCmp = *pbCmp++;

				bSrcL = bSrc & 0x0f;
				bSrcH = (bSrc>>0x04)&0x0f;
				bCmpL = bCmp & 0x0f;
				bCmpH = (bCmp>>0x04)&0x0f;

				if (bSrcH > bCmpH)
				{
					iRet = 1;
					break;
				}
				else if (bSrcH < bCmpH)
				{
					iRet = 2;
					break;
				}

				if (bSrcL > bCmpL)
				{
					iRet = 1;
					break;
				}
				else if (bSrcL < bCmpL)
				{
					iRet = 2;
					break;
				}
			}
		}
	}

	return iRet;
}

//�������Ƚ��������εĴ�С
//������@wSrc1
//		@wSrc2
//���أ�0������ȣ�1����Դ1����Դ2�� 2����Դ1С��Դ2
int IntCompare(WORD wSrc1, WORD wSrc2, WORD wLen)
{
	int iRet = -1;

	if (wSrc1 == wSrc2)
	{
		iRet = 0;
	}
	else if (wSrc1 > wSrc2)
	{
		iRet = 1;
	}
	else
	{
		iRet = 2;
	}

	return iRet;
}

typedef unsigned short u16;
const static u16 fcstab[256] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
#define PPPINITFCS16 0xffff 	/* Initial FCS value */

static DWORD pppfcs_16(DWORD dwFcs, BYTE *pbCp, int ilen)
{
	while (ilen--)
	{
		dwFcs = (dwFcs >> 8) ^ fcstab[(dwFcs ^ *pbCp++) & 0xff];
	}

	dwFcs ^= 0xffff;

	return (dwFcs);
}

//������CRC16У���㷨ֵ
//������@pInBuf ��У������
//		@wInLen ��У�����ݳ���
//���أ�CRC16У��ֵ
WORD CheckCRC16(BYTE *pInBuf, int iInLen)
{
	return pppfcs_16(PPPINITFCS16, pInBuf, iInLen);
}

//�������ֽ��ۼӺ�
//������@pInBuf ����
//		@wInLen ���ݳ���
//���أ�CS�ۼӺ�
BYTE CheckCS(BYTE *pInBuf, WORD wInLen)
{
	WORD i = wInLen;
	BYTE bCs = 0;
	BYTE *pbPtr = pInBuf;

	while (i != 0) 
	{
		bCs += *pbPtr;
		pbPtr++;
		i--;
	}

	return bCs;
}

//������ͨ����������֣�������һ�������
int SchFirstMtrSn(const BYTE *pbMsk, WORD wLen)
{
	for (WORD wMtrMsk=0; wMtrMsk<PN_MASK_SIZE; wMtrMsk++)
	{
		if (pbMsk[wMtrMsk])
		{
			for (BYTE i = 0; i < 8; i++)
			{
				if (pbMsk[wMtrMsk/8] & (1<<i))
					return wMtrMsk*8 + i;
			}
		}
	}

	return POINT_NUM+1;
}

//������ͨ�������wMtrSn�ж��Ƿ���pbMsk�У�û�оͷ����¸�һ����Ч�ı����
int SchVlidMtrSn(WORD wMtrSn, const BYTE *pbMsk, WORD wLen)
{
	if (pbMsk[wMtrSn/8] & (1<<(wMtrSn%8)))	//��Ч
		return wMtrSn;

	for (WORD wMtrSnIndex=wMtrSn+1; wMtrSnIndex<POINT_NUM; wMtrSnIndex++)
	{
		if (pbMsk[wMtrSnIndex/8] & (1<<(wMtrSnIndex%8)))
		{
			return wMtrSnIndex;
		}
	}

	return -1;
}

bool DelMeterInfo(BYTE *pbTsa, BYTE bTsaLen)
{
	TOobMtrInfo tMtrInfo;
	WORD wPn;
	bool fRet=false;

	for (wPn=0; wPn<POINT_NUM; wPn++)
	{
		memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
		if (GetMeterInfo(wPn, &tMtrInfo) 
			&& memcmp(pbTsa, tMtrInfo.bTsa, bTsaLen)==0)
		{
			BYTE bBuf[256] = {0};
			WriteItemEx(BANK0, wPn, 0x6000, bBuf);
			TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);
			fRet = true;
			break;
		}
	}

	return fRet;
}

bool DelMeterInfo(WORD wPn)
{
	BYTE bBuf[256] = {0};
	WriteItemEx(BANK0, wPn, 0x6000, bBuf);
	TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);
	return true;
}

//��������ȡ�����Ϣ(ͨ�����ַ)
//������@wPn ���������
//		@tTMtrInfo ������õ�Ԫ��Ϣ
bool GetMeterInfo(BYTE *pbTsa, BYTE bTsaLen, TOobMtrInfo *pTMtrInfo)
{
	WORD wPn;

	for (wPn=0; wPn<POINT_NUM; wPn++)
	{
		memset((BYTE*)pTMtrInfo, 0, sizeof(TOobMtrInfo));
		if (GetMeterInfo(wPn, pTMtrInfo) 
			/*&& bLen==tMtrInfo.bTsaLen*/
			&& memcmp(pbTsa, pTMtrInfo->bTsa, bTsaLen)==0)
		{
			return true;
		}
	}

	return false;	
}


// �Ƿ�Ϸ��ı�Ƶ�����Ϣ
int IsValidMtrInfo(BYTE *pMtrInfoBuf)
{
   	TOobMtrInfo TMtrInfo;
    TOobMtrInfo *pTMtrInfo = &TMtrInfo;
    BYTE bTsaLen;
    BYTE* pbBuf = pMtrInfoBuf;
	
    pbBuf++;	//0x6000�ĵ�һ���ֽ�Ϊ���������Ч���ȣ�����
	if (*pbBuf++ != DT_STRUCT)	//struct
	    return -1;
    pbBuf++;	//struct ��Ա����

    if (*pbBuf++ != DT_LONG_U)	//long-unsigned
        return -1;
    pTMtrInfo->wMtrSn = OoOiToWord(pbBuf);	
 
    pbBuf += 2;		
    if (*pbBuf++ != DT_STRUCT)	//struct
        return -1;
    pbBuf++;	//struct ��Ա����
    if (*pbBuf++ != DT_TSA)	//TSA
        return -1;
    pbBuf++;	//TSA����
    pTMtrInfo->bTsaLen = *pbBuf++ + 1;	//TSA�ڲ�octet���ݳ��ȣ� ���ַ����+1=���ַ��Ч����
    if (TMtrInfo.bTsaLen > 0 && TMtrInfo.bTsaLen <= sizeof(pTMtrInfo->bTsa))	//TSA len
		bTsaLen = pTMtrInfo->bTsaLen;
	else
		return -1;

    memcpy(pTMtrInfo->bTsa, pbBuf, bTsaLen);
	pbBuf += bTsaLen;	//TSA Addr
    if (*pbBuf++ != DT_ENUM)	//enum
		return -1;

    pTMtrInfo->bBps = *pbBuf++;	//bps
    if(pTMtrInfo->bBps>10 && pTMtrInfo->bBps!=255)
    {
        return -1;  // �������쳣
    }
    if (*pbBuf++ != DT_ENUM)	//enum	//protocol type
		return -1;

    pTMtrInfo->bProType = *pbBuf++;
    if(pTMtrInfo->bProType >= PROTOCOLNO_MAXNO)
    {
        return -1;  // Э���쳣
    }
    if (*pbBuf++ != DT_OAD)	//OAD
        return -1;

    pTMtrInfo->dwPortOAD = OoOadToDWord(pbBuf);
	pbBuf += 4;	//PORT
    if (*pbBuf++ != DT_OCT_STR)	//octet-string
        return -1;
    pTMtrInfo->bCodeLen = *pbBuf++;	//code len
    if(pTMtrInfo->bCodeLen > sizeof(pTMtrInfo->bCode))
		return -1;

    memcpy(pTMtrInfo->bCode, pbBuf, pTMtrInfo->bCodeLen);
    pbBuf += pTMtrInfo->bCodeLen;	//code
    if (*pbBuf++ != DT_UNSIGN)	//unsigned 
        return -1;
    pTMtrInfo->bRate = *pbBuf++;	//rate
    if (*pbBuf++ != DT_UNSIGN)	//unsigned
	    return -1;
    pTMtrInfo->bUserType = *pbBuf++;	//user type
    if (*pbBuf++ != DT_ENUM)	//enum
        return -1;
    pTMtrInfo->bLine = *pbBuf++;	//connect line
    if(pTMtrInfo->bLine>3)
    {
        return -1; // ���߷�ʽ�쳣
    }

    if (*pbBuf++ != DT_LONG_U)
        return -1;
    pTMtrInfo->wRateVol = OoLongUnsignedToWord(pbBuf);
	pbBuf += 2;
    if (*pbBuf++ != DT_LONG_U)
        return -1;
    pTMtrInfo->wRateCurr = OoLongUnsignedToWord(pbBuf);
	pbBuf += 2;

    if (*pbBuf++ != DT_STRUCT)	//struct
        return -1;
    pbBuf++;	//struct ��Ա����
    if (*pbBuf++ != DT_TSA)	//TSA
        return -1;
    pbBuf++;
    pTMtrInfo->bAcqTsaLen = *pbBuf++ + 1;	//TSA Len
    bTsaLen = 0;
    if (pTMtrInfo->bAcqTsaLen>=0 && pTMtrInfo->bAcqTsaLen<sizeof(pTMtrInfo->bAcqTsa))	//TSA len
        bTsaLen = pTMtrInfo->bAcqTsaLen;
	else
		return -1;

    memcpy(pTMtrInfo->bAcqTsa, pbBuf, bTsaLen);	
    pbBuf += bTsaLen;	//TSA
    if (*pbBuf++ != DT_OCT_STR)	//octet-string
        return -1;
    pTMtrInfo->bAssetLen = *pbBuf++;	//assert Len
    if(pTMtrInfo->bAssetLen > sizeof(pTMtrInfo->bAsset))
		return -1;

    memcpy(pTMtrInfo->bAsset, pbBuf, pTMtrInfo->bAssetLen);	
    pbBuf += pTMtrInfo->bAssetLen;	//assert
    if (*pbBuf++ != DT_LONG_U)	//PT
        return -1;
    pTMtrInfo->wPT = OoLongUnsignedToWord(pbBuf);
	pbBuf += 2;	//PT
    if (*pbBuf++ != DT_LONG_U)	//CT
        return -1;
    pTMtrInfo->wCT = OoLongUnsignedToWord(pbBuf);
	pbBuf += 2;	//CT

    return 0;
}

//��������ȡ�����Ϣ��ͨ�������㣩
//������@wPn ���������
//		@tTMtrInfo ������õ�Ԫ��Ϣ
bool GetMeterInfo(WORD wPn, TOobMtrInfo *pTMtrInfo)
{
	BYTE bBuf[256];
	BYTE *pbBuf = bBuf;
	BYTE bTsaLen;	//Ŀǰ��̬���·������ʱ����ַ���ȴ�0��ʼ������̬����֮���پ���

	memset(bBuf, 0, sizeof(bBuf));
	if (ReadItemEx(BANK0, wPn, 0x6000, bBuf)>0 && !IsAllAByte(bBuf, 0, sizeof(bBuf)))
	{
		pTMtrInfo->wPn = wPn;
		pbBuf++;	//0x6000�ĵ�һ���ֽ�Ϊ���������Ч���ȣ�����
		if (*pbBuf++ != DT_STRUCT)	//struct
			return false;
		pbBuf++;	//struct ��Ա����

		if (*pbBuf++ != DT_LONG_U)	//long-unsigned
			return false;
		pTMtrInfo->wMtrSn = OoOiToWord(pbBuf);	pbBuf += 2;		
		if (*pbBuf++ != DT_STRUCT)	//struct
			return false;
		pbBuf++;	//struct ��Ա����
		if (*pbBuf++ != DT_TSA)	//TSA
			return false;
		pbBuf++;	//TSA����
		pTMtrInfo->bTsaLen = *pbBuf++ + 1;	//TSA�ڲ�octet���ݳ��ȣ� ���ַ����+1=���ַ��Ч����
		if (pTMtrInfo->bTsaLen > 0)	//TSA len
			bTsaLen = pTMtrInfo->bTsaLen;
		memcpy(pTMtrInfo->bTsa, pbBuf, bTsaLen);	pbBuf += bTsaLen;	//TSA Addr
		if (*pbBuf++ != DT_ENUM)	//enum
			return false;
		pTMtrInfo->bBps = *pbBuf++;	//bps
		if (*pbBuf++ != DT_ENUM)	//enum	//protocol type
			return false;
		pTMtrInfo->bProType = *pbBuf++;
		if (*pbBuf++ != DT_OAD)	//OAD
			return false;
		pTMtrInfo->dwPortOAD = OoOadToDWord(pbBuf);	pbBuf += 4;	//PORT
		if (*pbBuf++ != DT_OCT_STR)	//octet-string
			return false;
		pTMtrInfo->bCodeLen = *pbBuf++;	//code len
		memcpy(pTMtrInfo->bCode, pbBuf, pTMtrInfo->bCodeLen);	pbBuf += pTMtrInfo->bCodeLen;	//code
		if (*pbBuf++ != DT_UNSIGN)	//unsigned 
			return false;
		pTMtrInfo->bRate = *pbBuf++;	//rate
		if (*pbBuf++ != DT_UNSIGN)	//unsigned
			return false;
		pTMtrInfo->bUserType = *pbBuf++;	//user type
		if (*pbBuf++ != DT_ENUM)	//enum
			return false;
		pTMtrInfo->bLine = *pbBuf++;	//connect line

		if (*pbBuf++ != DT_LONG_U)
			return false;
		//pTMtrInfo->wRateVol = ByteToWord(pbBuf);	pbBuf += 2;
		pTMtrInfo->wRateVol = OoLongUnsignedToWord(pbBuf);	pbBuf += 2;
		if (*pbBuf++ != DT_LONG_U)
			return false;
		//pTMtrInfo->wRateCurr = ByteToWord(pbBuf);	pbBuf += 2;
		pTMtrInfo->wRateCurr = OoLongUnsignedToWord(pbBuf);	pbBuf += 2;

		if (*pbBuf++ != DT_STRUCT)	//struct
			return false;
		pbBuf++;	//struct ��Ա����
		if (*pbBuf++ != DT_TSA)	//TSA
			return false;
		pbBuf++;
		pTMtrInfo->bAcqTsaLen = *pbBuf++ + 1;	//TSA Len
		bTsaLen = 0;
		if (pTMtrInfo->bAcqTsaLen >= 0)	//TSA len
			bTsaLen = pTMtrInfo->bAcqTsaLen;
		memcpy(pTMtrInfo->bAcqTsa, pbBuf, bTsaLen);	pbBuf += bTsaLen;	//TSA
		if (*pbBuf++ != DT_OCT_STR)	//octet-string
			return false;
		pTMtrInfo->bAssetLen = *pbBuf++;	//assert Len
		memcpy(pTMtrInfo->bAsset, pbBuf, pTMtrInfo->bAssetLen);	pbBuf += pTMtrInfo->bAssetLen;	//assert
		if (*pbBuf++ != DT_LONG_U)	//PT
			return false;
		//pTMtrInfo->wPT = ByteToWord(pbBuf);	pbBuf += 2;	//PT
		pTMtrInfo->wPT = OoLongUnsignedToWord(pbBuf);	pbBuf += 2;	//PT
		if (*pbBuf++ != DT_LONG_U)	//CT
			return false;
		//pTMtrInfo->wCT = ByteToWord(pbBuf);	pbBuf += 2;	//CT
		pTMtrInfo->wCT = OoLongUnsignedToWord(pbBuf);	pbBuf += 2;	//CT

#if 0	//���������	
		if (*pbBuf++ != DT_ARRAY)	//array
			return false;
		pTMtrInfo->bAddInfoCnt = *pbBuf++;//������Ϣ����
		if (pTMtrInfo->bAddInfoCnt > sizeof(pTMtrInfo->tTAddInfo)/sizeof(TAddInfo))
			pTMtrInfo->bAddInfoCnt = sizeof(pTMtrInfo->tTAddInfo);
		for (BYTE i = 0; i < pTMtrInfo->bAddInfoCnt; i++)
		{
			bool fVarLenFlg;
			BYTE bDataLen;

			pbBuf++;	//struct
			pbBuf++;	//struct��Ա����
			pbBuf++;	//OAD
			bDataLen = OoGetDataLen(DT_OAD, pbBuf);	//
			pbBuf++;	//��������
			if (fVarLenFlg)	//���ȿɱ䣬����ȡ����
			{
				bDataLen = *pbBuf;	
				pbBuf++;	//��������
			}
			memcpy(pTMtrInfo->tTAddInfo[i].bData, pbBuf, bDataLen);	pbBuf += bDataLen;
		}
#endif

		return true;
	}
	
	return false;
}

//���������õ����Ϣ
//������@wMtrSn ������
//		@tTMtrInfo ������õ�Ԫ��Ϣ
bool SetMeterInfo(WORD wPn, TOobMtrInfo tTMtrInfo)
{
	BYTE bBuf[128] = {0};
	BYTE *p = bBuf+1;

	//�������
	*p++ = DT_STRUCT;	//struct
	*p++ = 4;	//+struct ��Ա����
	*p++ = DT_LONG_U;	//long-unsigned
	//WordToByte(tTMtrInfo.wMtrSn, p);	p += 2;
	OoWordToLongUnsigned(tTMtrInfo.wMtrSn, p);    p += 2;

	//������Ϣ
	*p++ = DT_STRUCT;	//struct	
	*p++ = 0x0a;	//+struct ��Ա����
	
	*p++ = DT_TSA;	//TSA
	if (tTMtrInfo.bTsaLen == 0)
	{
		*p++ = 0x07;	//TSA�ڲ�octet����
		*p++ = 0x05;
		memset(p, 0x00, 6);
		p += 6;
		tTMtrInfo.bTsaLen = 6;
	}
	else
	{
		*p++ = tTMtrInfo.bTsaLen+1;	//TSA����
		*p++ = tTMtrInfo.bTsaLen-1;	//TSA�ڲ�octet����
		memcpy(p, tTMtrInfo.bTsa, tTMtrInfo.bTsaLen); p += tTMtrInfo.bTsaLen;
	}
	*p++ = DT_ENUM;//enum
	*p++ = tTMtrInfo.bBps;
	*p++ = DT_ENUM;	//enum
	*p++ = tTMtrInfo.bProType;
	*p++ = DT_OAD;	//OAD
	OoDWordToOad(tTMtrInfo.dwPortOAD, p);	p += 4;
	*p++ = DT_OCT_STR;	//octet-string
	*p++ = tTMtrInfo.bCodeLen;
	memcpy(p, tTMtrInfo.bCode, tTMtrInfo.bCodeLen);	p += tTMtrInfo.bCodeLen;
	*p++ = DT_UNSIGN;	//unsigned
	*p++ = tTMtrInfo.bRate;
	*p++ = DT_UNSIGN;	//unsigned
	*p++ = tTMtrInfo.bUserType;
	*p++ = DT_ENUM;	//enum
	*p++ = tTMtrInfo.bLine;

	*p++ = DT_LONG_U;   //long-unsigned
	OoWordToLongUnsigned(tTMtrInfo.wRateVol, p);
	p += 2;
	*p++ = DT_LONG_U;   //long-unsigned
	OoWordToLongUnsigned(tTMtrInfo.wRateCurr, p);
	p += 2;
	//��չ��Ϣ
	*p++ = DT_STRUCT;	//struct
	*p++ = 4;	//+struct  ��Ա����
	*p++ = DT_TSA;	//TSA
	if (tTMtrInfo.bAcqTsaLen == 0)
	{
		*p++ = 0x07;	//TSA�ڲ�octet����
		*p++ = 0x05;
		memset(p, 0x00, 6);
		p += 6;
		tTMtrInfo.bAcqTsaLen = 6;
	}
	else
	{
		*p++ = tTMtrInfo.bAcqTsaLen+1;	//TSA����
		*p++ = tTMtrInfo.bAcqTsaLen-1;	//TSA�ڲ�octet����
		memcpy(p, tTMtrInfo.bAcqTsa, tTMtrInfo.bAcqTsaLen);	p += tTMtrInfo.bAcqTsaLen;
	}

	*p++ = DT_OCT_STR;	//octet-string
	*p++ = tTMtrInfo.bAssetLen;
	memcpy(p, tTMtrInfo.bAsset, tTMtrInfo.bAssetLen);	p += tTMtrInfo.bAssetLen;
	*p++ = DT_LONG_U;
	//*p = WordToByte(tTMtrInfo.wPT, p);	p += 2;
	OoWordToLongUnsigned(tTMtrInfo.wPT, p);    p += 2;
	*p++ = DT_LONG_U;
	//*p = WordToByte(tTMtrInfo.wCT, p);	p += 2;
	OoWordToLongUnsigned(tTMtrInfo.wCT, p);    p += 2;
	
	//������Ϣ
#if 0
	*pbBuf = 1;	//array
	*pbBuf = tTMtrInfo.bAddInfoCnt;	pbBuf++;
	for (BYTE i = 0; i < tTMtrInfo.bAddInfoCnt; i++)
	{
		*pbBuf = DT_OAD;	pbBuf++;	//OAD
		DWordToByte(tTMtrInfo.tTAddInfo[i].dwOAD, pbBuf);	pbBuf += 4;
	}
#else
	*p++ = 1;	//array
	*p++ = 0;	
#endif
	bBuf[0] = p - (bBuf+1);

	bool fSameMtr = MeterInfoCompare(wPn, bBuf);
	if (!fSameMtr)
	{
		if (WriteItemEx(BANK0, wPn, 0x6000, bBuf) > 0)
		{
			SetRdMtrCtrlMask(wPn);
			TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);
			return true;
		}
	}

	return false;
}

//�������жϱ�����Ƿ���Ч
//������&wMtrSn�����
//���أ���Ч����true����֮
bool IsMtrSnValid(WORD wMtrSn)
{
	return false;
}


//��������ȡ����ַ�ĳ���
//������@wMtrSn	������
BYTE GetMeterTsaLen(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
		return tMtrInfo.bTsaLen; 	

	return 0;
}

//������ͨ������Ż�ȡ���ַ
//������@wMtrSn ������
//		@pbMtrAddr ���ַ
//		@fRev ȡ���ı��ַ�Ƿ���
//		@���ز��������ַ����
BYTE GetMeterTsa(WORD wMtrSn, BYTE *pbTsa, bool fRev)
{
	TOobMtrInfo tMtrInfo;

	memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
	if (GetMeterInfo(wMtrSn, &tMtrInfo) && !IsAllAByte((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo)))
	{
		BYTE bTsaRev[TSA_LEN];

		memset(bTsaRev, 0, sizeof(bTsaRev));
		if (fRev)
		{
			revcpy(bTsaRev, tMtrInfo.bTsa, tMtrInfo.bTsaLen);
			memcpy(pbTsa, bTsaRev, tMtrInfo.bTsaLen);
		}
		else
			memcpy(pbTsa, tMtrInfo.bTsa, tMtrInfo.bTsaLen);

		return tMtrInfo.bTsaLen;
	}

	return 0;
}

//������ͨ����ַ��ȡ�����
//������@pbTsa ���ַ
//		@bLen ��ַ����
//���أ������
WORD GetMeterSn(BYTE *pbTsa, BYTE bLen)
{
	TOobMtrInfo tMtrInfo;

	for (WORD wMtrSn=0; wMtrSn<POINT_NUM; wMtrSn++)
	{
		memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
		if (GetMeterInfo(wMtrSn, &tMtrInfo) 
			/*&& bLen==tMtrInfo.bTsaLen*/
			&& (memcmp(pbTsa, tMtrInfo.bTsa, bLen)==0))
		{
			return tMtrInfo.wMtrSn;
		}
	}

	return 0;
}

//������ͨ�����ַ��ȡ����
//������@pbTsa ���ַ
//���أ����ַ����
WORD GetMeterLen(BYTE *pbTsa)
{
	TOobMtrInfo tMtrInfo;

	for (WORD wMtrSn=0; wMtrSn<POINT_NUM; wMtrSn++)
	{
		memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
		if (GetMeterInfo(wMtrSn, &tMtrInfo) 
			/*&& bLen==tMtrInfo.bTsaLen*/
			&& (memcmp(pbTsa, tMtrInfo.bTsa, tMtrInfo.bTsaLen)==0))
		{
			return tMtrInfo.bTsaLen;
		}
	}

	return 0;
}
//������ͨ����ַ��ȡ������ţ��ڲ�ӳ��ģ�
//������@pbTsa ���ַ
//		@bLen ��ַ����
//		@fRev �����ַ�Ƿ���
//���أ��������
WORD GetMeterPn(BYTE *pbTsa, BYTE bLen, bool fRev)
{
	TOobMtrInfo tMtrInfo;
	BYTE bTsaRev[TSA_LEN];

	memset(bTsaRev, 0, sizeof(bTsaRev));
	if (fRev)
		revcpy(bTsaRev, pbTsa, bLen);
	else
		memcpy(bTsaRev, pbTsa, bLen);

	for (WORD wMtrSn=0; wMtrSn<POINT_NUM; wMtrSn++)
	{
		memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
		if (GetMeterInfo(wMtrSn, &tMtrInfo) 
			/*&& bLen==tMtrInfo.bTsaLen*/
			&& (memcmp(bTsaRev, tMtrInfo.bTsa, bLen)==0))
		{
			return tMtrInfo.wPn;
		}
	}

	return 0;
}

//����:	��ȡ����ַ����,
//����:	@wPn �������
//����:	����ɹ��򷵻���Ч����
int GetMeterAddrLen(WORD wPn)
{
	BYTE bBuf[PNPARA_LEN];

	if (ReadItemEx(BN0, wPn, 0x6000, bBuf)<=0)
		return -1;

	BYTE bAddL = bBuf[10]+1;

	return bAddL;
}

//������ͨ��������޸ı��ַ
//������@wMtrSn �����
//		@pbTsa ���ַ
//		@bMtrLen ���ַ����
//���أ��Ƿ����óɹ�
bool SetMeterTsa(WORD wMtrSn, BYTE *pbTsa, BYTE bMtrLen)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bTsaLen = bMtrLen;
		memcpy(tMtrInfo.bTsa, pbTsa, bMtrLen);
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//������ͨ������Ż�ȡ������
//������@wMtrSn �����
//���أ� ������	
BYTE GetMeterBps(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bBps;
	}

	return 0;
}

//���������ò�����
//������@wMtrSn �����
//		@bBps ������
//���أ��Ƿ����óɹ�
bool SetMeterBps(WORD wMtrSn, BYTE bBps)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bBps = bBps;
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡ��Լ����
BYTE GetMeterPro(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bProType;
	}

	return 0;
}

//���������ñ��Լ����
bool SetMeterPro(WORD wMtrSn, BYTE bPro)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bBps = bPro;
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡ��˿ں�
//������@wMtrSn �����
//		@tTPORT_PARAM �˿ںŲ���
//���أ��˿�����
extern BYTE GetMeterPort(WORD wMtrSn, TPORT_PARAM &tTPORT_PARAM)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		BYTE *bFmt[128] = {0};
		BYTE bBuf[64] = {0};
		BYTE *pbPtr = bBuf;
		WORD wFmtLen;

		//����Ĳ�����Ҫ��ȡ����⣬OoReadAttr��ϵͳ��ӿڣ������޸�!!!!!!
		if ((OoReadAttr((tMtrInfo.dwPortOAD>>16)&0xffff, (tMtrInfo.dwPortOAD>>8)&0xff, bBuf, bFmt, (WORD*)&wFmtLen))>0)
		{
			pbPtr++;	//struct
			pbPtr++;	//+struct ��Ա����
			pbPtr++;	//visible-string
			tTPORT_PARAM.bPortDescLen = *pbPtr;	pbPtr++;	//+visible-string���ݳ���
			if (tTPORT_PARAM.bPortDescLen > sizeof(tTPORT_PARAM.pszPortDesc))
				tTPORT_PARAM.bPortDescLen = sizeof(tTPORT_PARAM.pszPortDesc);
			memcpy(tTPORT_PARAM.pszPortDesc, pbPtr, tTPORT_PARAM.bPortDescLen);
			pbPtr++;	//COMDCB
			tTPORT_PARAM.tTCOM_PARAM.bBps = *pbPtr;	pbPtr++;
			tTPORT_PARAM.tTCOM_PARAM.bCheckBit = *pbPtr; pbPtr++;
			tTPORT_PARAM.tTCOM_PARAM.bDataBit = *pbPtr;	pbPtr++;
			tTPORT_PARAM.tTCOM_PARAM.bStopBit = *pbPtr;	pbPtr++;
			tTPORT_PARAM.tTCOM_PARAM.bFlowCtrl = *pbPtr;	pbPtr++;
		}

		if (0xf2070200 == (tMtrInfo.dwPortOAD&0xffffff00))	//0xf201 ����02	
		{
			return PORT_GB485;
		}
		else if (0xf2090200 == (tMtrInfo.dwPortOAD&0xffffff00))	//0xf209 ����02
		{
			return PORT_CCT_PLC;
		}
	}

	return 0;
}

//���������ñ�˿�
bool SetMeterPort(WORD wMtrSn, BYTE bPort)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		//��dwOAD��Ӧ�Ķ˿ڲ���ȡ����
		//tMtrInfo.dwOAD = ;
		//����OAD�Ĳ���
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡͨ������
//������@wMtrSn �����
//		@pbBuf ������
//���أ����볤��
BYTE GetMeterPwd(WORD wMtrSn, BYTE *pbBuf)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		memcpy(pbBuf, tMtrInfo.bCode,tMtrInfo.bCodeLen);
		return tMtrInfo.bCodeLen;
	}

	return 0;
}

//����������ͨ������
//������@wMtrSn �����
//		@pbBuf ����
//		@bLen ���ݳ���
//���أ��Ƿ����óɹ�
bool SetMeterPwd(WORD wMtrSn, BYTE *pbBuf, BYTE bLen)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bCodeLen = bLen;
		memcpy(tMtrInfo.bCode, pbBuf, bLen);
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡ���ʸ���
BYTE GetMeterRate(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bRate;
	}

	return 0;
}

//���������÷��ʸ���
bool SetMeterRate(WORD wMtrSn, BYTE bRate)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bRate = bRate;
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡ����û�����
BYTE GetMeterUserType(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bUserType;
	}

	return 0;
}

//���������ñ��û�����
bool SetMeterUserType(WORD wMtrSn, BYTE bUserType)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bUserType = bUserType;
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡ���߷�ʽ
BYTE GetMeterLine(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bLine;
	}

	return 0;
}

//���������ñ���߷�ʽ
bool SetMeterLine(WORD wMtrSn, BYTE bLine)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bLine = bLine;
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡ�ɼ�����ַ
//������@wMtrSn �����
//		@pbAcqAddr �ɼ�����ַ
//���أ��ɼ�����ַ�ĳ���
BYTE GetMeterAcqTsa(WORD wMtrSn, BYTE *pbAcqAddr)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		memcpy(pbAcqAddr, tMtrInfo.bAcqTsa, tMtrInfo.bAcqTsaLen);
		return tMtrInfo.bAcqTsaLen;
	}

	return 0;
}

//���������òɼ�����ַ
//������@wMtrSn �����
//		@pbAcqAddr �ɼ�����ַ
//		@bAcqLen �ɼ�����ַ����
bool SetMeterAcqTsa(WORD wMtrSn, BYTE *pbAcqAddr, BYTE bAcqLen)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bAcqTsaLen = bAcqLen;
		memcpy(tMtrInfo.bAcqTsa, pbAcqAddr, bAcqLen);
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//�����������ʲ���
//������@wMtrSn �����
//		@pbAsset �ʲ���
//���أ��ʲ��ų���
BYTE GetMeterAsset(WORD wMtrSn, BYTE *pbAsset)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		memcpy(pbAsset, tMtrInfo.bAsset, tMtrInfo.bAssetLen);
		return tMtrInfo.bAcqTsaLen;
	}

	return 0;
}

//�����������ʲ���
//������@wMtrSn �����
//		@pbAsset �ʲ���
//		@bLen �ʲ��ų���
//���أ��Ƿ����óɹ�
bool SetMeterAsset(WORD wMtrSn, BYTE *pbAsset, BYTE bLen)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.bAssetLen = bLen;
		memcpy(tMtrInfo.bAsset, pbAsset, bLen);
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡPT
extern WORD GetMeterPT(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.wPT;
	}

	return 0;
}

//����������PT
bool SetMeterPT(WORD wMtrSn, WORD wPT)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.wPT = wPT;
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//��������ȡCT
WORD GetMeterCT(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.wCT;
	}

	return 0;
}

//����������CT
bool SetMeterCT(WORD wMtrSn, WORD wCT)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		tMtrInfo.wCT = wCT;
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//������data_time_s תϵͳtime
DWORD DataTimeToSysTime(BYTE *pbBuf, TTime &tTime)
{

	BYTE *pbPtr = pbBuf;

	memset((BYTE*)&tTime, 0, sizeof(tTime));
	tTime.nYear = ByteToWord(pbPtr);	pbPtr += 2;
	tTime.nMonth = *pbPtr++;
	tTime.nDay = *pbPtr++;
	tTime.nHour = *pbPtr++;
	tTime.nMinute = *pbPtr++;
	tTime.nSecond = *pbPtr++;

	return TimeToSeconds(tTime);
}

//������ʱ����TIת��
//���أ���
DWORD TiToSecondes(TTimeInterv *pTI)
{
	DWORD dwSec = 0;
	BYTE bUnit = pTI->bUnit;

	switch(bUnit)
	{
	case 0://TIME_UNIT_SECONDS:
		dwSec = pTI->wVal;
		break;
	case 1://TIME_UNIT_MINUTE:
		dwSec = pTI->wVal*60;
		break;
	case 2://TIME_UNIT_HOUR:
		dwSec = pTI->wVal*60*60;
		break;
	case 3://TIME_UNIT_DAY:
		dwSec = pTI->wVal*60*60*24;
		break;
	case 4://TIME_UNIT_MONTH:
		break;
	default:
		break;
	}

	return dwSec;
}

//������ʱ�ε���
void PeriodAdj(TPeriod *p)
{
	if (p->bStarHour > 23)
	{
		p->bStarHour = 23;
		p->bStarMin = 59;
	}
	else if (p->bStarMin > 59)
	{
		p->bStarMin = 59;
	}

	if (p->bEndHour > 23)
	{
		p->bEndHour = 23;
		p->bEndMin = 59;
	}
	else if (p->bEndMin > 59)
	{
		p->bEndMin = 59;
	}
}

//��������ȡ����ǰ�����ʱ��
//������@pTaskCfg �������õ�Ԫ����
//		@ pdwCurSec��������ǰ�����ʱ��
//		@pdwStartSec���ص�ǰ����Ŀ�ʼִ��ʱ��
//		@ pdwEndSec	���ص�ǰ����Ľ���ʱ��
//����:���ʱ��պ÷����򷵻�0,�����ʱ�򷵻�1,���ʱ��û���򷵻�-1
//��ע����ǰʱ���Ƿ�������ִ���ڣ���ǰʱ���Ƿ���ʱ�γ�����
int GetTaskCurExeTime(TTaskCfg* pTaskCfg, DWORD* pdwCurSec, DWORD* pdwStartSec, DWORD* pdwEndSec)
{
	TTime tStartTime, tEndTime, tNowTime;
	TTime tDayStartTime;
	TTimeInterv tiExe = pTaskCfg->tiExe;
	DWORD dwDelaySec = TiToSecondes((TTimeInterv*)&pTaskCfg->tiDelay);
	DWORD dwCurSec, dwIntervSec=0;
	int nInterv = 0;

	if (pTaskCfg->bState == 2)
		return -1;

	dwCurSec = GetCurTime();
	if (dwCurSec>TimeToSeconds(pTaskCfg->tmEnd) || dwCurSec<TimeToSeconds(pTaskCfg->tmStart))	//����������ʼ����ʱ�䷶Χ
		return -1;

	GetCurTime(&tNowTime);
	tStartTime = tEndTime = tNowTime;
	tDayStartTime = tNowTime;

	switch (tiExe.bUnit)
	{
	case TIME_UNIT_SECONDS:
		dwIntervSec = tiExe.wVal;
		break;
	case TIME_UNIT_MINUTE:
		dwIntervSec = tiExe.wVal*60;
		tDayStartTime.nSecond = pTaskCfg->tmStart.nSecond;
		break;
	case TIME_UNIT_HOUR:
		dwIntervSec = tiExe.wVal*60*60;
		if (tiExe.wVal > 1)
			tDayStartTime.nHour = pTaskCfg->tmStart.nHour;
		
		tDayStartTime.nMinute = pTaskCfg->tmStart.nMinute;
		tDayStartTime.nSecond = pTaskCfg->tmStart.nSecond;
		break;
	case TIME_UNIT_DAY:
		dwIntervSec = tiExe.wVal*24*60*60;
		tDayStartTime.nHour = pTaskCfg->tmStart.nHour;
		tDayStartTime.nMinute = pTaskCfg->tmStart.nMinute;
		tDayStartTime.nSecond = pTaskCfg->tmStart.nSecond;
		break;
	case TIME_UNIT_MONTH:
		tStartTime = pTaskCfg->tmStart;
		nInterv = IntervsPast(tStartTime, tNowTime, TIME_UNIT_MONTH, tiExe.wVal);
		if (nInterv > 0)
			AddIntervs(tStartTime, TIME_UNIT_MONTH, nInterv*tiExe.wVal);	//��ʼʱ�䰴�������

		if (tStartTime.nDay > DaysOfMonth(tStartTime))
			tStartTime.nDay = DaysOfMonth(tStartTime);

		tEndTime = tStartTime;
		AddIntervs(tEndTime, TIME_UNIT_MONTH, tiExe.wVal);
		if (tEndTime.nDay > DaysOfMonth(tEndTime))
			tEndTime.nDay = DaysOfMonth(tEndTime);

		tDayStartTime = tStartTime;
		break;

	case TIME_UNIT_YEAR:
		tStartTime = pTaskCfg->tmStart;
		nInterv = IntervsPast(tStartTime, tNowTime, TIME_UNIT_MONTH, tiExe.wVal*12);
		if (nInterv > 0)
			AddIntervs(tStartTime, TIME_UNIT_MONTH, nInterv*tiExe.wVal*12);	//��ʼʱ�䰴�������

		if (tStartTime.nDay > DaysOfMonth(tStartTime))
			tStartTime.nDay = DaysOfMonth(tStartTime);

		tEndTime = tStartTime;
		AddIntervs(tEndTime, TIME_UNIT_MONTH, tiExe.wVal*12);
		if (tEndTime.nDay > DaysOfMonth(tEndTime))
			tEndTime.nDay = DaysOfMonth(tEndTime);		

		tDayStartTime = tStartTime;
		break;
	default:
		return -1;
	}

	//�жϵ�ǰ��ʱ���Ƿ����"������ʼʱ��+��ʱʱ��"������Ƚ��ر��ն�����
	//ʼʱ����ܶ���02��43��ִ��,������Ӹ���佫����������00��00��ִ��
	if (tiExe.bUnit!=TIME_UNIT_MINUTE && tiExe.bUnit!=TIME_UNIT_SECONDS)
	{
		if (dwCurSec<(TimeToSeconds(tDayStartTime)+dwDelaySec))
			return -1;
	}
	
	if (tiExe.bUnit>=TIME_UNIT_SECONDS && tiExe.bUnit<=TIME_UNIT_DAY)
	{
		*pdwCurSec = dwCurSec/dwIntervSec * dwIntervSec;	//�����ʼʱ��
		*pdwStartSec = *pdwCurSec + dwDelaySec;	//�����ʼʱ��+��ʱʱ��
		*pdwEndSec = *pdwCurSec + dwIntervSec;	//�������ʱ��

		if (tiExe.bUnit==TIME_UNIT_HOUR && tiExe.wVal>1)	//�����λΪСʱ�Ҽ������1ʱ�������ʼ�ͽ���ʱ����ѻ�׼ʱ�����Сʱ����
		{
			*pdwStartSec += (tDayStartTime.nHour*60*60+tDayStartTime.nMinute*60+tDayStartTime.nSecond);
			*pdwEndSec  += (tDayStartTime.nHour*60*60+tDayStartTime.nMinute*60+tDayStartTime.nSecond);
		}
	}
	else
	{
		*pdwCurSec = TimeToSeconds(tStartTime);
		*pdwStartSec = *pdwCurSec + dwDelaySec;
		*pdwEndSec = TimeToSeconds(tEndTime);
	}

	if (*pdwStartSec >= *pdwEndSec)
		return -1;

	if (dwCurSec>=*pdwStartSec && dwCurSec<*pdwEndSec)	//��ǰʱ����ִ��������
	{
		if (pTaskCfg->bPeriodNum != 0)
		{
			for (BYTE i=0; i<pTaskCfg->bPeriodNum; i++)
			{
				DWORD dwPerStartSec, dwPerEndSec;
				TTime tPerStartTime, tPerEndTime;

				GetCurTime(&tPerStartTime);
				tPerStartTime.nSecond = 0;
				PeriodAdj(&pTaskCfg->period[i]);
				tPerStartTime.nMinute = pTaskCfg->period[i].bStarMin;
				tPerStartTime.nHour = pTaskCfg->period[i].bStarHour;
				dwPerStartSec = TimeToSeconds(tPerStartTime);
				GetCurTime(&tPerEndTime);
				tPerEndTime.nSecond = 0;
				tPerEndTime.nMinute = pTaskCfg->period[i].bEndMin;
				tPerEndTime.nHour = pTaskCfg->period[i].bEndHour;
				dwPerEndSec = TimeToSeconds(tPerEndTime);

				switch(pTaskCfg->bPeriodType)
				{
				case 0:	//ǰ�պ�
					if ((dwCurSec>=dwPerStartSec) && (dwCurSec<dwPerEndSec))
						return 0;
					break;
				case 1:	//ǰ�����
					if ((dwCurSec>dwPerStartSec) && (dwCurSec<=dwPerEndSec))
						return 0;
					break;
				case 2:	//ǰ�պ��
					if ((dwCurSec>=dwPerStartSec) && (dwCurSec<=dwPerEndSec))
						return 0;
					break;
				case 3:	//ǰ����
					if ((dwCurSec>dwPerStartSec) && (dwCurSec<dwPerEndSec))
						return 0;
					break;
				 default:
                    break;
				}
			}
		}
		else
		{
			return 0;
		}
	}


	return -1;
}

//��������pbMtrMask�У���wPn��ʼ������һ����Ч������
int SearchNextPnFromMask(BYTE *pbMtrMask, WORD wPn)
{
	WORD wNextPn;

	for (wNextPn=wPn+1; wNextPn<POINT_NUM; wNextPn++)
	{
		if (pbMtrMask[wNextPn/8] & (1<<(wNextPn%8)))
			break;
	}

	if (wNextPn >= POINT_NUM)
		return -1;

	return wNextPn;
}

//��������������Id��ǰִ��ʱ��
//������@bTaskId ����ID
//		@dwExeTime ����ID�������ڻ���֮�����ʼʱ��
void SetTaskUpdateTime(BYTE bTaskId, DWORD dwExeTime)
{
	DWORD dwNowTime;

	dwNowTime = GetCurTime();
	WriteItemEx(BANK16, bTaskId, 0x6001, (BYTE*)&dwExeTime, dwNowTime);
}

//��������ȡ����Id��ǰִ��ʱ��
//������@bTaskId ����ID
//���أ�����IDִ�е���ʼʱ��
DWORD GetTaskUpdateTime(BYTE bTaskId)
{
	DWORD dwTime = 0;

	ReadItemEx(BANK16, bTaskId, 0x6001, (BYTE*)&dwTime);

	return dwTime;
}

//�������������ID�ĸ���ʱ��
//������@bTaskId ����id
void ClearTaskIdUpateTime(BYTE bTaskId)
{
	WriteItemEx(BANK16, bTaskId, 0x6001, 0);
}


int CctProxy(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData)
{
	return g_CStdReader->DirAskProxy(bType, bChoice, bTsa, bTsaLen, pApdu, wApduLen, wTimeOut, pbData);
}

int CctTransmit(BYTE *pbTsa, BYTE bTsaLen, BYTE *pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE *pbRespBuf)
{
	int iTxLen;
	BYTE bTxBuf[1024];

	memset(bTxBuf, 0, sizeof(bTxBuf));
	bTxBuf[0] = PRO_TYPE_TRANS;
	bTxBuf[1] = 0x00;	//ͨ����ʱ��ر�ʶ
	bTxBuf[2] = 0x00;	//���������ʱ��ʶ
	bTxBuf[3] = wReqLen;
	memcpy(&bTxBuf[4], pbReqBuf, wReqLen);
	iTxLen = wReqLen + 4;
	return g_CStdReader->DoFwdData(pbTsa, bTsaLen, bTxBuf, iTxLen, wTimeOut, pbRespBuf, true);
}

int MaskToNum(BYTE *pbMask, WORD wLen)
{
	const BYTE bBitCnt[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
	WORD wNum = 0;

	if (pbMask != NULL)
	{
		for (WORD i=0; i<wLen; i++)
		{
			if (pbMask[i] != 0)
			{
				wNum += bBitCnt[pbMask[i]&0x0f];
				wNum += bBitCnt[(pbMask[i]>>4)&0x0f];
			}
		}
	}

	return wNum;
}


//��������ȡ�ز�����ͬ���ڵ����
WORD GetPlcNodeAddr(TMtrInfo *pMtrInfo, WORD wMtrNum)
{
	TOobMtrInfo tMtrInfo;
	WORD wNum;
	BYTE bTmpAddr[TSA_LEN];
	bool fSameMtrFlg = false;

	wNum = 0;
	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
		if (GetMeterInfo(wPn, &tMtrInfo) > 0)
		{
			if ((tMtrInfo.dwPortOAD & 0xFFFFFF00) != 0xF2090200)
				continue;
			if (IsAllAByte(tMtrInfo.bTsa, 0xee, tMtrInfo.bTsaLen))
				continue;
			memset(bTmpAddr, 0, sizeof(bTmpAddr));
			/*if (tMtrInfo.bAcqTsaLen>=6 && !IsAllAByte(tMtrInfo.bAcqTsa, 0, tMtrInfo.bAcqTsaLen))
			{
				memcpy(bTmpAddr, tMtrInfo.bAcqTsa, tMtrInfo.bAcqTsaLen);
			}
			else */if (tMtrInfo.bTsaLen!=0 && !IsAllAByte(tMtrInfo.bTsa, 0, tMtrInfo.bTsaLen))
			{
				memcpy(bTmpAddr, tMtrInfo.bTsa, tMtrInfo.bTsaLen);
			}
			else
				continue;

			fSameMtrFlg = false;
			for (WORD k=0; k<wNum; k++)
			{
				if (memcmp(pMtrInfo[k].bTsa, bTmpAddr, 6) == 0)
				{
					fSameMtrFlg = true;
					break;
				}
			}
			if (!fSameMtrFlg)
			{
				memcpy(pMtrInfo[wNum].bTsa, bTmpAddr, 6);
				pMtrInfo[wNum].bMtrPro = tMtrInfo.bProType;
				pMtrInfo[wNum].bTsaLen = tMtrInfo.bTsaLen;
				wNum++;
				if (wNum >= wMtrNum)
					break;
			}
		}
	}

	return wNum;
}

//��������ϵͳ����ַת��Ϊ�ز�ͬ����ַ����(6���ֽڵ�ַ����)
//		����3�������
//			1. ���ַ����<6���ֽڣ����ֽ�����0���
//			2. ���ַ����>6���ֽڣ��ӵ��ֽڿ�ʼ��ȡ��Ч��6���ֽ�
//			3. ���ַ����=6���ֽڣ�ֱ�ӿ���
//		���ַ��160224670028������ͬ���·��ĵ�ַΪ280067240216
BYTE EncodeRouterMtrAddr(BYTE *pbTsa, BYTE bTsaLen)
{
	BYTE i;
	BYTE bRevTsa[TSA_LEN] = {0};
	
	if (bTsaLen >= 6)	
	{
		revcpy(bRevTsa, pbTsa, 6);
	}
	else	//С��6���ȸߵ��ֽڵ����������㲹ȫΪ6���ֽ�
	{
		revcpy(bRevTsa, pbTsa, bTsaLen);
		for (i=bTsaLen; i<6; i++)
			pbTsa[i] = 0;
	}

	memcpy(pbTsa, bRevTsa, 6);

	return 6;
}

//��������·����Ӧ/����ĵ�ַת��Ϊϵͳ��ַ
BYTE DecodeRouterMtrAddr(BYTE *pbTsa, BYTE bTsaLen)
{
	BYTE bRevTsa[TSA_LEN] = {0};

	revcpy(bRevTsa, pbTsa, bTsaLen);
	memcpy(pbTsa, bRevTsa, 6);

	return 6;
}

//������·����Ӧ/����ĵ���ַת��Ϊ��Ӧ�Ĳ�����
//		��ַƥ�����������
//		1. ·������ĵ�ַȫƥ�䣬��6�ֽ�ȫ���ȶ�
//		2. ·������ĵ�ַ��Чƥ�䣬���Ӹ��ֽڿ�ʼ��ȥ��0x00�ĵ�ַ
//���أ���Ӧ�Ĳ����㣬ʧ�ܷ���0
WORD RouterMtrAddrConvertPn(BYTE *pbTsa)
{
	int i;
	WORD wPn;
	BYTE bZeroPos = 6;
	BYTE bBakTsa[TSA_LEN] = {0};

	memcpy(bBakTsa, pbTsa, 6);

	//ȫ��ַƥ��
	DecodeRouterMtrAddr(bBakTsa, 6);
	wPn = GetMeterPn(bBakTsa, 6);
	if (wPn==0 && pbTsa[5]==0)	//wPnδ�ҵ����ҵ�6���ֽڵ���0
	{
		//��ַ��Чƥ��
		memcpy(bBakTsa, pbTsa, 6);
		for (i=5; i>=0; i--)
		{
			if (bBakTsa[i] != 0)	//���ֽ��Ƿ�Ϊ0
				break;
			bZeroPos = i;
		}
		DecodeRouterMtrAddr(bBakTsa, bZeroPos);
		wPn = GetMeterPn(bBakTsa, bZeroPos);
	}

	memcpy(pbTsa, bBakTsa, 6);

	return wPn;
}

void GetRooterTermAddr(BYTE *pbTermAddr, BYTE &bTermAddrLen)
{
	BYTE bBuf[32];
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(0x4001, 0x02, bBuf, NULL, NULL) <= 0 || IsAllAByte(bBuf, 0x00, 2) )
	{
		char *pszDefAddr = "\x11\x22\x33\x44\x55\x66";
		memcpy(pbTermAddr, pszDefAddr, strlen(pszDefAddr));
	}
	else
	{
		BYTE bTsaLen;
		bTsaLen = bBuf[1];
		if (bTsaLen > 6)
        {      
			bTsaLen = 6;
        }
		memcpy(pbTermAddr, &bBuf[2], bTsaLen);
	}
    
    bTermAddrLen = 0x06;
}


void PrintInfo(TRdItem *pRdItem, TMtrPara *pMtrPara)
{
	char szTsa[32] = {0};
	char szOAD[16] = {0};
	char szROAD[256] = {0};

	if (pMtrPara->bProId == PROTOCOLNO_DLT69845)
	{
		if (pRdItem->bReqType == 1)
		{
			DTRACE(DB_CCT, ("Send 698.45 RequestNormal Meter:%s MtrPro:%d, OAD:0x%s.\n", HexToStr(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], szTsa), pMtrPara->bProId, HexToStr((BYTE*)&pRdItem->dwOAD, 4, szOAD, true)));
		}
		else
		{
			sprintf(szROAD, "Cnt:%d ROAD: ", pRdItem->bRCSD[0]);
			for (BYTE i=0; i<pRdItem->bRCSD[0]; i++)
				sprintf(szROAD+strlen(szROAD), "%s. ", HexToStr(&pRdItem->bRCSD[2+i*5], 4, szOAD));
			DTRACE(DB_CCT, ("Send 698.45 RequestRecord Meter:%s MtrPro:%d, OAD:%s. %s\n", HexToStr(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], szTsa), pMtrPara->bProId, HexToStr((BYTE*)&pRdItem->dwOAD, 4, szOAD, true), szROAD));
		}
	}
	else if (pMtrPara->bProId==PROTOCOLNO_DLT645 || pMtrPara->bProId==PROTOCOLNO_DLT645_V07)
	{
		if ((pRdItem->dwOAD&0xff000000) == 0x30000000)	//07-645��ȫ�¼�����
		{
			sprintf(szROAD, "Cnt:%d ROAD: ", pRdItem->bRCSD[0]);
			for (BYTE i=0; i<pRdItem->bRCSD[0]; i++)
				sprintf(szROAD+strlen(szROAD), "%s. ", HexToStr(&pRdItem->bRCSD[2+i*5], 4, szOAD));
			DTRACE(DB_CCT, ("Send 698.45 Event RequestRecord Meter:%s, MtrPro:%d, OAD:%s. %s\n", HexToStr(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], szTsa),  pMtrPara->bProId, HexToStr((BYTE*)&pRdItem->dwOAD, 4, szOAD, true), szROAD));
		}
		else
		{
			;
		}
	}
	else	//SBJC
	{
		;
	}
}