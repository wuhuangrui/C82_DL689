/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CctRdCtrl.cpp
 * 摘    要：载波抄表控制管理
 * 当前版本：1.0
 * 作    者：CL
 * 完成日期：2016年8月
 *********************************************************************************************************/

#include "stdafx.h"
#include "CctAPI.h"
#include "LibDbConst.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "CctTaskMangerOob.h"
#include "DbOIAPI.h"
#include "DbFmt.h"

//描述：比较两个数组的大小
//参数：@pbSrc 源数组，对应终端内部数据
//		@wSrcLen 数组长度，对应终端内部数据长度
//		@pbCmp 比较数组，协议层数据
//		@wCmpLen 比较长度，对应协议层数据长度
//返回：0——相等，1——源数据 > 比较数据， 2——源数据 < 比较数据
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

	if (bSrcOctNum < bCmpOctNum)	//源数据长度 < 比较数据长度
	{
		return 2;
	}
	else if (bSrcOctNum > bCmpOctNum)	//源数据长度 > 比较数据长度
	{
		return 1;
	}
	else	//源数据长度 = 比较数据长度，需进行数组的具体数据内容比较
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

//描述：比较两个整形的大小
//参数：@wSrc1
//		@wSrc2
//返回：0——相等，1——源1大于源2， 2——源1小于源2
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

//描述：CRC16校验算法值
//参数：@pInBuf 需校验数据
//		@wInLen 需校验数据长度
//返回：CRC16校验值
WORD CheckCRC16(BYTE *pInBuf, int iInLen)
{
	return pppfcs_16(PPPINITFCS16, pInBuf, iInLen);
}

//描述：字节累加和
//参数：@pInBuf 数据
//		@wInLen 数据长度
//返回：CS累加和
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

//描述：通过电表屏蔽字，搜索第一个表序号
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

//描述：通过表序号wMtrSn判断是否在pbMsk中，没有就返回下个一个有效的表序号
int SchVlidMtrSn(WORD wMtrSn, const BYTE *pbMsk, WORD wLen)
{
	if (pbMsk[wMtrSn/8] & (1<<(wMtrSn%8)))	//有效
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

//描述：获取电表信息(通过表地址)
//参数：@wPn 电表测量点号
//		@tTMtrInfo 电表配置单元信息
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


// 是否合法的表计档案信息
int IsValidMtrInfo(BYTE *pMtrInfoBuf)
{
   	TOobMtrInfo TMtrInfo;
    TOobMtrInfo *pTMtrInfo = &TMtrInfo;
    BYTE bTsaLen;
    BYTE* pbBuf = pMtrInfoBuf;
	
    pbBuf++;	//0x6000的第一个字节为电表档案的有效长度，跳过
	if (*pbBuf++ != DT_STRUCT)	//struct
	    return -1;
    pbBuf++;	//struct 成员个数

    if (*pbBuf++ != DT_LONG_U)	//long-unsigned
        return -1;
    pTMtrInfo->wMtrSn = OoOiToWord(pbBuf);	
 
    pbBuf += 2;		
    if (*pbBuf++ != DT_STRUCT)	//struct
        return -1;
    pbBuf++;	//struct 成员个数
    if (*pbBuf++ != DT_TSA)	//TSA
        return -1;
    pbBuf++;	//TSA长度
    pTMtrInfo->bTsaLen = *pbBuf++ + 1;	//TSA内部octet数据长度， 表地址长度+1=表地址有效长度
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
        return -1;  // 波特率异常
    }
    if (*pbBuf++ != DT_ENUM)	//enum	//protocol type
		return -1;

    pTMtrInfo->bProType = *pbBuf++;
    if(pTMtrInfo->bProType >= PROTOCOLNO_MAXNO)
    {
        return -1;  // 协议异常
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
        return -1; // 接线方式异常
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
    pbBuf++;	//struct 成员个数
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

//描述：获取电表信息（通过测量点）
//参数：@wPn 电表测量点号
//		@tTMtrInfo 电表配置单元信息
bool GetMeterInfo(WORD wPn, TOobMtrInfo *pTMtrInfo)
{
	BYTE bBuf[256];
	BYTE *pbBuf = bBuf;
	BYTE bTsaLen;	//目前组态在下发电表档案时电表地址长度从0开始，待组态修正之后再纠正

	memset(bBuf, 0, sizeof(bBuf));
	if (ReadItemEx(BANK0, wPn, 0x6000, bBuf)>0 && !IsAllAByte(bBuf, 0, sizeof(bBuf)))
	{
		pTMtrInfo->wPn = wPn;
		pbBuf++;	//0x6000的第一个字节为电表档案的有效长度，跳过
		if (*pbBuf++ != DT_STRUCT)	//struct
			return false;
		pbBuf++;	//struct 成员个数

		if (*pbBuf++ != DT_LONG_U)	//long-unsigned
			return false;
		pTMtrInfo->wMtrSn = OoOiToWord(pbBuf);	pbBuf += 2;		
		if (*pbBuf++ != DT_STRUCT)	//struct
			return false;
		pbBuf++;	//struct 成员个数
		if (*pbBuf++ != DT_TSA)	//TSA
			return false;
		pbBuf++;	//TSA长度
		pTMtrInfo->bTsaLen = *pbBuf++ + 1;	//TSA内部octet数据长度， 表地址长度+1=表地址有效长度
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
		pbBuf++;	//struct 成员个数
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

#if 0	//后续在添加	
		if (*pbBuf++ != DT_ARRAY)	//array
			return false;
		pTMtrInfo->bAddInfoCnt = *pbBuf++;//附加信息个数
		if (pTMtrInfo->bAddInfoCnt > sizeof(pTMtrInfo->tTAddInfo)/sizeof(TAddInfo))
			pTMtrInfo->bAddInfoCnt = sizeof(pTMtrInfo->tTAddInfo);
		for (BYTE i = 0; i < pTMtrInfo->bAddInfoCnt; i++)
		{
			bool fVarLenFlg;
			BYTE bDataLen;

			pbBuf++;	//struct
			pbBuf++;	//struct成员个数
			pbBuf++;	//OAD
			bDataLen = OoGetDataLen(DT_OAD, pbBuf);	//
			pbBuf++;	//跳过类型
			if (fVarLenFlg)	//长度可变，先提取长度
			{
				bDataLen = *pbBuf;	
				pbBuf++;	//跳过长度
			}
			memcpy(pTMtrInfo->tTAddInfo[i].bData, pbBuf, bDataLen);	pbBuf += bDataLen;
		}
#endif

		return true;
	}
	
	return false;
}

//描述：设置电表信息
//参数：@wMtrSn 电表序号
//		@tTMtrInfo 电表配置单元信息
bool SetMeterInfo(WORD wPn, TOobMtrInfo tTMtrInfo)
{
	BYTE bBuf[128] = {0};
	BYTE *p = bBuf+1;

	//配置序号
	*p++ = DT_STRUCT;	//struct
	*p++ = 4;	//+struct 成员个数
	*p++ = DT_LONG_U;	//long-unsigned
	//WordToByte(tTMtrInfo.wMtrSn, p);	p += 2;
	OoWordToLongUnsigned(tTMtrInfo.wMtrSn, p);    p += 2;

	//基本信息
	*p++ = DT_STRUCT;	//struct	
	*p++ = 0x0a;	//+struct 成员个数
	
	*p++ = DT_TSA;	//TSA
	if (tTMtrInfo.bTsaLen == 0)
	{
		*p++ = 0x07;	//TSA内部octet长度
		*p++ = 0x05;
		memset(p, 0x00, 6);
		p += 6;
		tTMtrInfo.bTsaLen = 6;
	}
	else
	{
		*p++ = tTMtrInfo.bTsaLen+1;	//TSA长度
		*p++ = tTMtrInfo.bTsaLen-1;	//TSA内部octet长度
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
	//扩展信息
	*p++ = DT_STRUCT;	//struct
	*p++ = 4;	//+struct  成员个数
	*p++ = DT_TSA;	//TSA
	if (tTMtrInfo.bAcqTsaLen == 0)
	{
		*p++ = 0x07;	//TSA内部octet长度
		*p++ = 0x05;
		memset(p, 0x00, 6);
		p += 6;
		tTMtrInfo.bAcqTsaLen = 6;
	}
	else
	{
		*p++ = tTMtrInfo.bAcqTsaLen+1;	//TSA长度
		*p++ = tTMtrInfo.bAcqTsaLen-1;	//TSA内部octet长度
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
	
	//附加信息
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

//描述：判断表序号是否有效
//参数：&wMtrSn表序号
//返回：有效返回true，反之
bool IsMtrSnValid(WORD wMtrSn)
{
	return false;
}


//描述：获取电表地址的长度
//参数：@wMtrSn	电表序号
BYTE GetMeterTsaLen(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
		return tMtrInfo.bTsaLen; 	

	return 0;
}

//描述：通过表序号获取表地址
//参数：@wMtrSn 电表序号
//		@pbMtrAddr 表地址
//		@fRev 取出的表地址是否反向
//		@返回参数，表地址长度
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

//描述：通过地址获取表序号
//参数：@pbTsa 表地址
//		@bLen 地址长度
//返回：表序号
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

//描述：通过表地址获取表长度
//参数：@pbTsa 表地址
//返回：表地址长度
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
//描述：通过地址获取测量点号（内部映射的）
//参数：@pbTsa 表地址
//		@bLen 地址长度
//		@fRev 输入地址是否反向
//返回：测量点号
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

//描述:	获取电表地址长度,
//参数:	@wPn 测量点号
//返回:	如果成功则返回有效长度
int GetMeterAddrLen(WORD wPn)
{
	BYTE bBuf[PNPARA_LEN];

	if (ReadItemEx(BN0, wPn, 0x6000, bBuf)<=0)
		return -1;

	BYTE bAddL = bBuf[10]+1;

	return bAddL;
}

//描述：通过表序号修改表地址
//参数：@wMtrSn 表序号
//		@pbTsa 表地址
//		@bMtrLen 表地址长度
//返回：是否设置成功
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

//描述：通过表序号获取波特率
//参数：@wMtrSn 表序号
//返回： 波特率	
BYTE GetMeterBps(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bBps;
	}

	return 0;
}

//描述：设置波特率
//参数：@wMtrSn 表序号
//		@bBps 波特率
//返回：是否设置成功
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

//描述：获取规约类型
BYTE GetMeterPro(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bProType;
	}

	return 0;
}

//描述：设置表规约类型
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

//描述：获取表端口号
//参数：@wMtrSn 表序号
//		@tTPORT_PARAM 端口号参数
//返回：端口类型
extern BYTE GetMeterPort(WORD wMtrSn, TPORT_PARAM &tTPORT_PARAM)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		BYTE *bFmt[128] = {0};
		BYTE bBuf[64] = {0};
		BYTE *pbPtr = bBuf;
		WORD wFmtLen;

		//这里的参数需要读取任务库，OoReadAttr是系统库接口，后续修改!!!!!!
		if ((OoReadAttr((tMtrInfo.dwPortOAD>>16)&0xffff, (tMtrInfo.dwPortOAD>>8)&0xff, bBuf, bFmt, (WORD*)&wFmtLen))>0)
		{
			pbPtr++;	//struct
			pbPtr++;	//+struct 成员个数
			pbPtr++;	//visible-string
			tTPORT_PARAM.bPortDescLen = *pbPtr;	pbPtr++;	//+visible-string数据长度
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

		if (0xf2070200 == (tMtrInfo.dwPortOAD&0xffffff00))	//0xf201 属性02	
		{
			return PORT_GB485;
		}
		else if (0xf2090200 == (tMtrInfo.dwPortOAD&0xffffff00))	//0xf209 属性02
		{
			return PORT_CCT_PLC;
		}
	}

	return 0;
}

//描述：设置表端口
bool SetMeterPort(WORD wMtrSn, BYTE bPort)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		//将dwOAD对应的端口参数取出来
		//tMtrInfo.dwOAD = ;
		//设置OAD的参数
		if (SetMeterInfo(wMtrSn, tMtrInfo))
		{
			return true;
		}
	}

	return false;
}

//描述：获取通信密码
//参数：@wMtrSn 表序号
//		@pbBuf 表密码
//返回：密码长度
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

//描述：设置通信密码
//参数：@wMtrSn 表序号
//		@pbBuf 数据
//		@bLen 数据长度
//返回：是否设置成功
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

//描述：获取费率个数
BYTE GetMeterRate(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bRate;
	}

	return 0;
}

//描述：设置费率个数
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

//描述：获取电表用户类型
BYTE GetMeterUserType(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bUserType;
	}

	return 0;
}

//描述：设置表用户类型
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

//描述：获取接线方式
BYTE GetMeterLine(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.bLine;
	}

	return 0;
}

//描述：设置表接线方式
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

//描述：获取采集器地址
//参数：@wMtrSn 表序号
//		@pbAcqAddr 采集器地址
//返回：采集器地址的长度
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

//描述：设置采集器地址
//参数：@wMtrSn 表序号
//		@pbAcqAddr 采集器地址
//		@bAcqLen 采集器地址长度
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

//描述：设置资产号
//参数：@wMtrSn 表序号
//		@pbAsset 资产号
//返回：资产号长度
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

//描述：设置资产号
//参数：@wMtrSn 表序号
//		@pbAsset 资产号
//		@bLen 资产号长度
//返回：是否设置成功
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

//描述：获取PT
extern WORD GetMeterPT(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.wPT;
	}

	return 0;
}

//描述：设置PT
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

//描述：获取CT
WORD GetMeterCT(WORD wMtrSn)
{
	TOobMtrInfo tMtrInfo;

	if (GetMeterInfo(wMtrSn, &tMtrInfo))
	{
		return tMtrInfo.wCT;
	}

	return 0;
}

//描述：设置CT
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

//描述：data_time_s 转系统time
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

//描述：时间间隔TI转秒
//返回：秒
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

//描述：时段调整
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

//描述：获取任务当前间隔的时标
//参数：@pTaskCfg 任务配置单元参数
//		@ pdwCurSec返回任务当前间隔的时标
//		@pdwStartSec返回当前任务的开始执行时间
//		@ pdwEndSec	返回当前任务的结束时间
//返回:如果时间刚好符合则返回0,如果超时则返回1,如果时间没到则返回-1
//备注：当前时间是否在周期执行内，当前时间是否在时段抄表内
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
	if (dwCurSec>TimeToSeconds(pTaskCfg->tmEnd) || dwCurSec<TimeToSeconds(pTaskCfg->tmStart))	//不在任务起始结束时间范围
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
			AddIntervs(tStartTime, TIME_UNIT_MONTH, nInterv*tiExe.wVal);	//起始时间按间隔归整

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
			AddIntervs(tStartTime, TIME_UNIT_MONTH, nInterv*tiExe.wVal*12);	//起始时间按间隔归整

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

	//判断当前的时间是否大于"任务起始时间+延时时间"，这里比较特别，日冻结起
	//始时间可能定在02：43分执行,如果不加该语句将导致数据在00：00就执行
	if (tiExe.bUnit!=TIME_UNIT_MINUTE && tiExe.bUnit!=TIME_UNIT_SECONDS)
	{
		if (dwCurSec<(TimeToSeconds(tDayStartTime)+dwDelaySec))
			return -1;
	}
	
	if (tiExe.bUnit>=TIME_UNIT_SECONDS && tiExe.bUnit<=TIME_UNIT_DAY)
	{
		*pdwCurSec = dwCurSec/dwIntervSec * dwIntervSec;	//间隔起始时间
		*pdwStartSec = *pdwCurSec + dwDelaySec;	//间隔起始时间+延时时间
		*pdwEndSec = *pdwCurSec + dwIntervSec;	//间隔结束时间

		if (tiExe.bUnit==TIME_UNIT_HOUR && tiExe.wVal>1)	//间隔单位为小时且间隔大于1时，间隔起始和结束时间需把基准时间里的小时加上
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

	if (dwCurSec>=*pdwStartSec && dwCurSec<*pdwEndSec)	//当前时间在执行周期内
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
				case 0:	//前闭后开
					if ((dwCurSec>=dwPerStartSec) && (dwCurSec<dwPerEndSec))
						return 0;
					break;
				case 1:	//前开后闭
					if ((dwCurSec>dwPerStartSec) && (dwCurSec<=dwPerEndSec))
						return 0;
					break;
				case 2:	//前闭后闭
					if ((dwCurSec>=dwPerStartSec) && (dwCurSec<=dwPerEndSec))
						return 0;
					break;
				case 3:	//前开后开
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

//描述：在pbMtrMask中，从wPn开始搜索下一个有效测量点
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

//描述：设置任务Id当前执行时间
//参数：@bTaskId 任务ID
//		@dwExeTime 任务ID经过周期换算之后的起始时间
void SetTaskUpdateTime(BYTE bTaskId, DWORD dwExeTime)
{
	DWORD dwNowTime;

	dwNowTime = GetCurTime();
	WriteItemEx(BANK16, bTaskId, 0x6001, (BYTE*)&dwExeTime, dwNowTime);
}

//描述：获取任务Id当前执行时间
//参数：@bTaskId 任务ID
//返回：任务ID执行的起始时间
DWORD GetTaskUpdateTime(BYTE bTaskId)
{
	DWORD dwTime = 0;

	ReadItemEx(BANK16, bTaskId, 0x6001, (BYTE*)&dwTime);

	return dwTime;
}

//描述：清除任务ID的更新时间
//参数：@bTaskId 任务id
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
	bTxBuf[1] = 0x00;	//通信延时相关标识
	bTxBuf[2] = 0x00;	//附属相关延时标识
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


//描述：获取载波档案同步节点个数
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

//描述：将系统库表地址转化为载波同步地址档案(6个字节地址长度)
//		存在3中情况：
//			1. 表地址长度<6个字节，高字节需用0填充
//			2. 表地址长度>6个字节，从低字节开始截取有效的6个字节
//			3. 表地址长度=6个字节，直接拷贝
//		表地址：160224670028，档案同步下发的地址为280067240216
BYTE EncodeRouterMtrAddr(BYTE *pbTsa, BYTE bTsaLen)
{
	BYTE i;
	BYTE bRevTsa[TSA_LEN] = {0};
	
	if (bTsaLen >= 6)	
	{
		revcpy(bRevTsa, pbTsa, 6);
	}
	else	//小于6，先高低字节调整，在用零补全为6个字节
	{
		revcpy(bRevTsa, pbTsa, bTsaLen);
		for (i=bTsaLen; i<6; i++)
			pbTsa[i] = 0;
	}

	memcpy(pbTsa, bRevTsa, 6);

	return 6;
}

//描述：将路由响应/请求的地址转换为系统地址
BYTE DecodeRouterMtrAddr(BYTE *pbTsa, BYTE bTsaLen)
{
	BYTE bRevTsa[TSA_LEN] = {0};

	revcpy(bRevTsa, pbTsa, bTsaLen);
	memcpy(pbTsa, bRevTsa, 6);

	return 6;
}

//描述：路由响应/请求的电表地址转换为对应的测量点
//		地址匹配两种情况：
//		1. 路由请求的地址全匹配，即6字节全部比对
//		2. 路由请求的地址有效匹配，即从高字节开始，去除0x00的地址
//返回：对应的测量点，失败返回0
WORD RouterMtrAddrConvertPn(BYTE *pbTsa)
{
	int i;
	WORD wPn;
	BYTE bZeroPos = 6;
	BYTE bBakTsa[TSA_LEN] = {0};

	memcpy(bBakTsa, pbTsa, 6);

	//全地址匹配
	DecodeRouterMtrAddr(bBakTsa, 6);
	wPn = GetMeterPn(bBakTsa, 6);
	if (wPn==0 && pbTsa[5]==0)	//wPn未找到，且第6个字节等于0
	{
		//地址有效匹配
		memcpy(bBakTsa, pbTsa, 6);
		for (i=5; i>=0; i--)
		{
			if (bBakTsa[i] != 0)	//高字节是否为0
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
		if ((pRdItem->dwOAD&0xff000000) == 0x30000000)	//07-645表全事件抄读
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