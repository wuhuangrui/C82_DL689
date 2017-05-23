/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbOIAPI.h
 * 摘    要：本文件主要实现系统数据库和面向对象ID之间的转换
 * 当前版本：1.0
 * 作    者：孔成波
 * 完成日期：2016年8月
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

//描述: 取某类的属性个数
//参数:@wClass	需要获取属性的类号
//返回: 正确则返回相应类的属性个数,否则返回0(不存在的类)
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

#define OI_NULL_DATA		0	//空数据

#define FMT_ERR_OK			0	//无错误
#define FMT_ERR_TYPE		1	//类型不符合
#define FMT_ERR_NULL		2	//空数据错误
#define FMT_ERR_LEN			3	//长度错误
#define FMT_ERR_UNK_TYPE	4	//未知类型


//描述：获取格式的有效长度（去掉RLF\RLV\LRF\LRV长度），对应g_OIConvertClass中的fmt格式
//参数：@pFmt 对应g_OIConvertClass.fmt格式
//		@wFmtLen 对应g_OIConvertClass.wFmtLen格式长度
//		@pwRetFmtLen 返回格式的有效长度
//返回：-1失败
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
		pFmt++;	//DT_ARRAY成员个数
		wValidFmtLen++;
		iLen = OoGetDataTypeFmtValidLen(pFmt, wFmtLen-(pFmt-pFmt0), &wTmpValidFmtLen);
		if (iLen < 0)
			goto ERR_RET;
		pFmt += iLen;
		wValidFmtLen += wTmpValidFmtLen;
		break;
	case DT_BIT_STR:
		pFmt++;	//DT_BIT_STR成员个数
		wValidFmtLen++;
		pFmt++;	//去掉RLF\RLV\LRF\LRV长度
		break;
	//TODO:有需要再添加
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


//描述: 取得数据项本身的长度
//参数：@ bType	数据类型，同Data的数据类型定义
//	   @ pbSrc数据项目描述，可以OAD、ROAD、CSD
//返回：正确则返回数据项的长度，否则返回负数
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
		Offset = 1;		//前面的结构体类型0x02占了1个字节
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

//描述：计算数组数据类型的总长度
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

//描述：电能表集合MS的数据类型偏移量
//		因为MS数据部分都是string，暂时不考虑直接转地址顺序
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
	case 0:	//无电能表
	case 1:	//全部用户地址
		break;

	case 2://一组用户类型
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		pbSrc += dwNum;//类型unsigned, len=1
		break;

	case 3:	//一组用户地址
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while(dwNum > 0)
		{
			bLen = *pbSrc++;
			pbSrc += bLen;
			dwNum--;
		}
		break;

	case 4: //一组配置序号
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while (dwNum > 0)
		{
			pbSrc += 2;
			dwNum--;
		}
		break;

	case 5:	//一组用户类型区间，seq of Region
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while(dwNum>0)
		{
			pbSrc++;	//枚举值
			pbSrc += 2;	//起始值：一组用户类型
			pbSrc += 2;	//结束值：一组用户类型
			dwNum--;
		}
		break;

	case 6:	//一组用户地址区间，seq of Region
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while(dwNum>0)
		{
			pbSrc++;	//枚举值
			//起始值
			pbSrc++;	//type=TSA
			bLen = *pbSrc++;
			pbSrc += bLen;
			//结束值
			pbSrc++;	//type=TSA
			bLen = *pbSrc++;
			pbSrc += bLen;

			dwNum--;
		}
		break;

	case 7:	//一组配置序号区间，seq of Region
		wOffset = DecodeLength(pbSrc, &dwNum);
		pbSrc += wOffset;
		while(dwNum>0)
		{
			pbSrc++;	//枚举值
			pbSrc += 3;	//起始值：一组用户类型
			pbSrc += 3;	//结束值：一组用户类型
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

//描述：取得RSD数据的全部长度
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
		pbSrc += OoGetDataTypeLen(pbSrc);	//数据
		break;

	case 2:
		if (*pbSrc == DT_OAD)	
			pbSrc++;
		pbSrc += 4;	//OAD
		pbSrc += OoGetDataTypeLen(pbSrc);	//起始值
		pbSrc += OoGetDataTypeLen(pbSrc);	//结束值
		pbSrc += OoGetDataTypeLen(pbSrc);	//间隔值
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
			pbSrc += OoGetDataTypeLen(pbSrc);	//起始值
			pbSrc += OoGetDataTypeLen(pbSrc);	//结束值
			pbSrc += OoGetDataTypeLen(pbSrc);	//间隔值
		}
		break;

	case 4:	//指定电能表集合，制定采集起始时间
	case 5:	//指定电能表集合，制定采集存储时间
		//pbSrc++;//格式描述DateTimeBCD
		pbSrc += 7;
		//MS集合
		nRet = ScanMS(pbSrc, fRevOrder);	//在函数中pbSrc自动增加了
		if (nRet > 0)
			pbSrc += nRet;
		else
			return nRet;
		break;

	case 6:	//指定电表集合，指定采集启动时间区间内连续间隔值
	case 7:	//指定电表集合，指定采集存储时间区间内连续间隔值
	case 8:	//指定电表集合，指定采集到时间区间内连续间隔值
		//pbSrc++;//DateTimeBCD----采集启动时间起始值
		pbSrc += 7;
		//pbSrc++;//DateTimeBCD----采集启动时间结束
		pbSrc += 7;
		//pbSrc++;	//TI
		//pbSrc += 2;	//enm 单位+值
		pbSrc += 1;	//单位+值
		//pbSrc += 3;	//间隔值 long-unsigned 
		pbSrc += 2;	//单位+值
		//MS集合
		nRet = ScanMS(pbSrc, fRevOrder);	//在函数中pbSrc自动增加了
		if (nRet > 0)
			pbSrc += nRet;
		else
			return nRet;
		break;

	case 9:
		pbSrc += 1;	//上N次记录 unsigned
		break;

	case 10:
		pbSrc += 1;	//上N条记录 unsigned
		//MS集合
		nRet = ScanMS(pbSrc, fRevOrder);	//在函数中pbSrc自动增加了
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

//描述：取得CSD数据的全部长度
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

//描述:计算ROAD本身的长度
//返回:本身长度
int ScanROAD(BYTE* pbSrc, bool fRevOrder)
{
	BYTE *pbPtr = pbSrc;
	BYTE bROADCnt;

	pbPtr += 4;
	bROADCnt = *pbPtr;	pbPtr++;
	pbPtr += bROADCnt*4;

	return pbPtr-pbSrc;
}

//描述:计算RCSD本身的长度
//返回:本身长度
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

//描述:格式化数据,把净数据转换成符合面向对象格式的数据-------提供给交采库给数据添加格式
//参数:@pbSrc 源数据,未格式化
//	   @pbDst 目的数据,格式化
//	   @pbFmt 格式描述串
//	   @wFmtLen 格式描述串的长度
//返回:如果正确则返回转换后的带格式的目标数据的长度,否则返回负数
//备注:
//	   >各种空数据在存储的时候都不是用0(空数据)来存储的,有两种情况
//		a.空数组:数组个数为0
//		b.其它空数据,包括空结构和boolean,long,long-unsigned等:都是直接以OI_NULL_DATA
//		  来存储的,
//	   >空数据存储的改进方法
//		a.以后可能针对每个格式描述串,第一个字节用来存放无效字节的定义,因为无法做到
//		  所有数据都用同样的无效字节,所以不同类型的数据用不同的无效字节,
//		b.可以把1定义成不接受无效字节,对于参数类型的数据来说,这种应该是最常用的,参数要设就设
//		  完整,不允许某个属性或者参数结构里的某个字段设成无效数据
//	   >目前只支持对空数组(数组个数为0)用0(空数据)来替代上送,
//		其它数据一律不支持自动识别源数据中的空数据
//	   >其它空数据的上送要靠特定的外部程序来完成,比如LN清单,曲线数据等
int OIFmtData(BYTE* pbSrc, BYTE* pbDst, BYTE* pbFmt, WORD wFmtLen)
{
	BYTE bStack[100];
	BYTE* pbSrc0 = pbSrc;
	BYTE* pbDst0 = pbDst;
	BYTE* pbFmtEnd = pbFmt + wFmtLen;
	int iSP = 0;	//堆栈指针
	BYTE bFP = 0;	//格式指针
	BYTE bTmpFP;
	BYTE bLen, bSrcLen, bArrLen, bByteLen;
	BYTE bFmtT, bVFmt, bVFill;
	bool fVLen;
	BYTE bNullArr = 0;
	WORD wOffset;
	BYTE bSrcType = 0xff;//DT_UNCARE; //don't care
	int iNullSP = 0;	//刚遇到空数组(数组个数为0)时的iSP,当扫描完一轮数据后,
						//iSP回复到iNullSP的值,表示对空数组的处理完毕
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

			if (bNullArr)	//上一级空数组还没完,子级还是空数组
			{
				bSrcLen = 0;
			}
			else
			{
				if (bSrcLen == 0)	//对于数组长度为0的空数组,以0(空数据)上送
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
			
			wOffset = pbSrc - pbSrc0;	//越过数组长度后的起始位置
			
			bStack[iSP++] = bLen;		//终端中存储数组时预留的记录笔数,用来计数源数据中还有多少笔记录没有跳过
			bStack[iSP++] = wOffset;	//记下源数据中数组元素的起始偏移,用来计算一笔记录的长度
			bStack[iSP++] = wOffset>>8;

			bStack[iSP++] = bFP;
			bStack[iSP++] = bType;
			bStack[iSP++] = bSrcLen; //即使数组元素个数为0,也要有一次入栈的操作,
									 //让程序扫描过元素的格式定义
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
				//确定bSrcLen和拷贝的bByteLen
				if (fVLen)
				{
					if (bVFill == 0) //0-不填充,加一个长度字节放到开头
					{
						bByteLen = *pbSrc++;
					}
					else	//用填充字节来填充
					{
						bVFill = g_bFillByte[bVFill];
						bByteLen = OIGetStrLen(pbSrc, bByteLen, bVFill);	//bByteLen发生改变
							//如果下发的没把全部下下来,在这种情况下上传的BITs数量可能跟下发的不一致
					}

					bSrcLen = bByteLen * 8;
				}
				else //定长,不能填充,要求长度必须刚好相等
				{
					bSrcLen = bLen;
				}
				
				//组帧
				*pbDst++ = bType;
				*pbDst++ = bSrcLen;		//数据长度
				if (bByteLen > 0)
				{
					if (bVFmt&VF_LR)
						memcpy(pbDst, pbSrc+1, bByteLen);		//跳过长度字节
					else
						revcpy(pbDst, pbSrc+1, bByteLen);

					//------新加这部分-------------
					if (bFmtT != 0)
						for (BYTE b=0; b<bByteLen; b++)			//2008-05-25  杨
							pbDst[b] = ByteBitReverse(pbDst[b]);	//对字节按位进行倒序
					//------新加这部分-------------
					pbDst += bByteLen;
				}

				pbSrc += (bLen + 7) / 8 + 1;			//长度+内容
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
					if (bVFill == 0) //0-不填充,加一个长度字节放到开头
					{
						bSrcLen = *pbSrc++;
					}
					else	//用填充字节来填充
					{
						bVFill = g_bFillByte[bVFill];
						bSrcLen = OIGetStrLen(pbSrc, bLen, bVFill);
					}
				}
				else //定长,不能填充,要求长度必须刚好相等
				{
					bSrcLen = bLen;
				}

				*pbDst++ = bType;
				*pbDst++ = bSrcLen;		//数据长度
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
			if (bNullArr)	//单字节数据目前不支持无效数据
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
				revcpy(pbDst, pbSrc, 4);	//不用将无效数据EE转换为0，主要是为现场区分真的0和无效数据
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
			//例子: 19 07 d7 08 07 ff 08 2b 39 ff 80 00 00  
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
			wErr = 4;//FMT_ERR_UNK_TYPE;	//未知类型
			goto OIFmtData_err;
		}
		
		if (fPop)	//需要出栈
		{
			while (iSP > 0)
			{
				bLen = bStack[--iSP];	//长度
				bType = bStack[--iSP];	//类型
				if (bType == 0x01)		//array
				{	
					bTmpFP = bStack[--iSP];
					wOffset = (WORD )bStack[--iSP] * 0x100;
					wOffset += bStack[--iSP];
					bArrLen = bStack[--iSP];

					if (bArrLen > 0)	//用来计数源数据中还有多少笔记录没有跳过
						bArrLen--;
				}

				if (bLen > 0)
					bLen--;

				if (bLen > 0)	//结构或数组还没完
				{
					if (bType == 0x01)	//array
					{
						wOffset = pbSrc - pbSrc0; //数组还没完,wOffset填入下一笔记录的起始
												  //位置,保证最后算出来的是一笔记录的长度
						bStack[iSP++] = bArrLen;
						bStack[iSP++] = wOffset;
						bStack[iSP++] = wOffset>>8;

						bFP = bTmpFP;
						bStack[iSP++] = bFP;
					}
					
					bStack[iSP++] = bType;	//类型
					bStack[iSP++] = bLen;	//长度
					break;
				}
				else
				{
					if (bType == 0x01)		//array
					{
						pbSrc += (pbSrc - pbSrc0 - wOffset) * bArrLen;
					}

					//一个结构中的全部字段数据都为无效数据0,目前先不把整个结构归整为0
					//如果要规整的话,只需记录目的数据结构的起始,判断结构全部成员是否都为0
				}

				//该结构或数组已完,退到它的上一层
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


//描述：判断读取的ID是否为测量点OI，如果测量点OI，需要考虑测量点中间分帧的情况
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
	case 0x60000200:	//0x6000可以有多个测量点参数，而0x6001只有当前配置的一个配置单元
	case 0x60020200:	//所有搜表结果
	case 0x60020500:	//跨台区搜表结果
	case 0x60120200:	//任务配置单元
	case 0x60140200:	//普通采集方案
	case 0x60160200:	//事件采集方案
	case 0x60180200:	//透明采集方案
	case 0x60190200:	//透明采集方案
	case 0x601E0200:	//采集规则
	case 0x601C0200:	//透明采集方案
	case 0x60510200:	//实时采集方案
	case 0x40000200:	//日期时间--属性2
	case 0xF2000200:	//RS232端口
	case 0xF2010200:	//485端口
	case 0xF2020200:	//红外端口
	case 0xF2030200:	//开关量输入--属性2
	case 0xF2050200:	//继电器输出--属性2
	case 0xF2060200:	//告警输出--属性2
	case 0xF2060400:	//告警输出--属性4
	case 0xF2070200:	//多功能端子--属性2
	case 0xF2080200:	//交采接口--属性2
	case 0xF2090200:	//载波/微功率无线接口--属性2
	case 0xF20a0200:	//脉冲输入设备--属性2
	case 0x80030200:	//一般中文信息属性2
	case 0x80040200:	//重要中文信息属性2
	case 0x81030200:	//时段功控
	case 0x81030300:
	case 0x81030400:
	case 0x81030500:
	case 0x81040200:	//厂休控
	case 0x81040300:
	case 0x81040400:
	case 0x81040500:
	case 0x81050200:	//营业报停控
	case 0x81050300:
	case 0x81050400:
	case 0x81050500:
	//case 0x81060200:	//当前功率下浮控
	case 0x81060300:
	case 0x81060400:
	case 0x81060500:
	case 0x81070200:	//购电控
	case 0x81070300:
	case 0x81070400:
	case 0x81070500:
	case 0x81080200:	//月电控
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
	case 0x40000200:	//日期时间--属性2
	case 0xF2000200:	//RS232端口
	case 0xF2010200:	//485端口
	case 0xF2020200:	//红外端口
	case 0xF2030200:	//开关量输入--属性2	
	case 0xF2050200:	//继电器输出--属性2
	case 0xF2060200:	//告警输出--属性2
	case 0xF2060400:	//告警输出--属性4
	case 0xF2070200:	//多功能端子--属性2
	case 0xF2080200:	//交采接口--属性2
	case 0xF2090200:	//载波/微功率无线接口--属性2
	case 0xF20a0200:	//脉冲输入设备--属性2
		return true;
	}

	return false;
}

//描述：特殊OI的读取，主要包括测量点参数，数据等，涉及多个记录的数据等
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
		case 0x4000:	//日期时间--属性2
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
		case 0x6000://采集档案配置表，测量点参数
			wMaxNum = POINT_NUM;
			pbTmp = pbBuf;
			wSigFrmPnNum = wBufSize/PNPARA_LEN;
			if (*piStart == -1) //第一次来读
			{
				wTotNum = GetValidPnNum();
				if (wTotNum == 0) //无相应参数
				{
					*pbBuf = EMPTY_DATA;
					return 1;
				}
				else
				{
					wSnLoc = 1;
					*pbTmp++ = 0x01; //数组
					if (wSigFrmPnNum >=  wTotNum)
						*pbTmp++ = wTotNum; //总条数  //目前最多PN_NUM组，不需要对长度特殊编码
					else
						*pbTmp++ = wSigFrmPnNum;
				}
			}
			for (i=wSnLoc; i<wMaxNum; i++)
			{
				wSn = i;
				if (!IsPnValid(wSn))//未设置过参数
					continue;
					
				memset(bBuf, 0, sizeof(bBuf));
				iLen = ReadItemEx(BN0, wSn, pOI->wID, bBuf);					
				if (iLen <= 0)
				{
					DTRACE(DB_FAPROTO, ("OIRead_Spec:wID = %d, wSn=%d, ReadItemEx failed !!\r\n", pOI->wID, wSn));
					return -DA_OTHER_ERROR;	//返回其它错误
				}
				wPnNum++;
				if (wPnNum>=wSigFrmPnNum)
				{
					wSnLoc = i;
					(*piStart)++;	
					break;
				}

				iLen = bBuf[0];	//电表有效数据长度
				iRet = OoScanData(bBuf+1, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType);	//调整字节顺序

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
			if (i == wMaxNum) //参数全部发完
				*piStart = -1;

			return (pbTmp-pbBuf);	

		case 0x6012:	//任务配置单元
			wMaxNum = TASK_NUM;
			pbTmp = pbBuf;
			wSigFrmPnNum = wBufSize/PNPARA_LEN;
			if (*piStart == -1) //第一次来读
			{
				wTotNum = GetTaskNum();
				if (wTotNum == 0) //无相应参数
				{
					*pbBuf = EMPTY_DATA;
					return 1;
				}
				else
				{
					wSnLoc = 1;
					*pbTmp++ = 0x01; //数组
					if (wSigFrmPnNum >=  wTotNum)
						*pbTmp++ = wTotNum; //总条数  //目前最多PN_NUM组，不需要对长度特殊编码
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
					return -DA_OTHER_ERROR;	//返回其它错误
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
			if (i == wMaxNum) //参数全部发完
				*piStart = -1;

			return (pbTmp-pbBuf);	
		case 0x6014:	//普通采集方案
		case 0x6016:	//事件采集方案
		case 0x6018:	//透明采集方案
		case 0x601C:	//上报方案
		case 0x6051:	//实时采集方案
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
			if (i == wMaxNum) //参数全部发完
				*piStart = -1;

			return (pbTmp-pbBuf);	
		case 0x6002:
			return GetSchMtrResult(piStart, pbBuf, wBufSize, 0, 0);
			break;
		case 0x6003:
			return GetCrossSchMtrResult(piStart, pbBuf, wBufSize);
		case 0x601E:
			return GetAllAcqRuleInfo(piStart, pbBuf, wBufSize);

		case 0x8003://一般中文信息
		case 0x8004://重要中文信息
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
					return -DA_OTHER_ERROR;	//返回其它错误
				}
				if (!IsAllAByte(bBuf, 0, iLen)) //有有效数据
				{
					iLen = bBuf[0];	//中文信息有效数据长度
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
					return -DA_OTHER_ERROR;	//返回其它错误
				}
				if (!IsAllAByte(bBuf, 0, iLen)) //有有效数据
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
					//注意格式字符串前面array和成员数量要去掉
					iLen = OoScanData(bBuf, pOI->pFmt+2, pOI->wFmtLen-2, false, -1, &wLen, &bType);
//	 					if (iLen > 0)
//	 						memcpy(pbBuf, bTmpBuf, iLen);	//获取扫描到的数据有效长度
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

//描述：特殊OI的写，主要包括测量点参数，数据等，涉及多个记录的数据等
int OIWrite_Spec(const ToaMap* pOI, BYTE* pbBuf)
{
	BYTE *pbBuf0 = pbBuf;

	switch (pOI->wID)
	{
	case 0x4000:	//日期时间--属性2
		TTime tm;
		if (DT_DATE_TIME_S == *pbBuf++)
		{
			GetCurTime(&tm);
			g_AdjTermTime.bClock[0] = DT_DATE_TIME_S;
			OoTimeToDateTimeS(&tm, &g_AdjTermTime.bClock[1]);	//记录校时前时间

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
			DealSpecTrigerEvt(TERM_CLOCKPRG);	//hyl 直接存储终端对时事件
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
				bMaxPortNum = pbBuf[1];//实际的设备对象数
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

//描述：读取对象属性，返回数据带格式，适用通讯直接调用
//参数：wOI访问对象描述
//		bAttr访问OI的属性
//		pbBuf：返回缓冲区
//		pbOpt: 访问对象选择符，对于记录型数据使用
//		piStart：引用型参数,用来记录多次读数据的步骤，用于多帧通讯返回
//				首次读取为-1，初始化内部读结构，并把riStep的值修改为>=0的数,后续读时根据内部读结构和riStep决定返回什么数据
//				当所取数据为最后一笔数据时，再将riStep的值置为-1表示无后续数据
//返回:如果正确返回读到pbBuf的数据的长度,否则返回负数
int OoProReadAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wBufSize, int* piStart)
{
	BYTE bAttrNum, bType;
	BYTE bTmpBuf[APDUSIZE];
	BYTE *pbTmpBuf = bTmpBuf;
	BYTE *pbBuf0 = pbBuf;		//保存当前缓冲指针
	int iLen = 0;
	WORD wID, wLen, wPn;
	BYTE bBuf[20];

	if (wBufSize == 0)
		wBufSize = APDUSIZE;

	if (bAttr == 0)	//所有属性
	{
		//因为每个类都有属性2，先用属性2取得class吧
		DWORD dwOIAtt = ((DWORD )wOI<<16) + 0x0200;
		const ToaMap* pOI = GetOIMap(dwOIAtt);
		if (pOI == NULL)
		{
			DTRACE(DB_OIIF, ("OIReadObj: Read wOI:%x failed !!\r\n", wOI));
			return -1;
		}
		bAttrNum = OIGetAttrNum(pOI->wClass);
		*pbBuf++ = 0x02; //结构
		pbBuf += EncodeLength(bAttrNum, pbBuf);	//数组的成员个数
		WORD wMaxBufLen = wBufSize;
		for (BYTE i=1; i<=bAttrNum; i++)
		{
			//因对属性0进行访问时,将按顺序访问属性1到属性n,各个属性由于各种原因不能访问的,
			//则将相应地返回 null-data.<参见 IEC62056-53, 58页>
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
	else if (bAttr == 1) //逻辑名
	{
		*pbBuf++ = 0x09;			//格式
		*pbBuf++ = 0x02;			//长度
		*pbBuf++ = (BYTE )(wOI>>8);
		*pbBuf++ = (BYTE )wOI;
		return (int)(pbBuf-pbBuf0);
	}
	else //其他属性
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

		if (IsNeedRdSpec(pOI) && bIndex==0)//特殊OI的读取，如测量点参数，时间等
		{
			 iLen = OIRead_Spec((ToaMap *)pOI, pbBuf, wBufSize, piStart);
			 if (dwOIAtt == 0xF2030200)		//遥信状态量及变位标志（属性2),读后清除
			 {
			 	for (WORD i=0; i<MAX_SW_PORT_NUM; i++)
				{
					memset(bBuf, 0, sizeof(bBuf));
					if (ReadItemEx(BN0, i, 0xF203, bBuf) > 0)  //存储遥信状态量
					{
						bBuf[5] = 0;
						WriteItemEx(BN0, i, 0xF203, bBuf);  //存储遥信状态量
					}
				}
			 }

			 return iLen;
		}
		else
		{
			if (IsNeedRdSpec(pOI))
			{
				wPn = bIndex - 1;	//特殊OAD的第bIndex个属性对应第bIndex-1个测量点
				bIndex = 0;			//特殊OI都是人为拆分成子OAD，这里直接读子OAD的内容，等同于读特殊OAD的第几个属性
			}
			else
			{
				wPn = pOI->wPn;
			}

			if ((wOI&0xf000) == 0x3000)	// 读内表或抄表事件记录，读到数据返回，否则继续
			{
				if (GetTermEvtCtrl(wOI) != NULL)	//TermEvtTask.h/cpp所支持的所有事件，即不包括抄表事件
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
			else if ((dwOIAtt&OAD_OI_MASK) == 0x50000200)	//冻结记录
			{
				return ReadFrzData(dwOIAtt, pbBuf, wBufSize, piStart);
			}
			if (pOI->wMode == MAP_VAL)	//主要是针对直接抄读换算及单位等固定数据格式
			{
				memcpy(pbBuf, (BYTE *)&(pOI->wVal), 2);
				iLen = 2;
			}
			else if (pOI->wMode == MAP_BYTE)	//主要是针对直接1字节常量固定数据格式
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
					if (IsAllAByte(bTmpBuf, 0x00, iLen))	//如果全部是0x00，那应该是没有写过，格式不可能为0
					{
						iLen = OIFmtData(bTmpBuf, pbBuf, pOI->pFmt, pOI->wFmtLen);
					}
					else
					{
						if (bIndex == 0)	//属性的全部数据
						{
							if (IsNeedRdSpec(pOI) && (wOI&0xFF00)==0xf200)	//输入输出OAD需去掉DT_ARRAY和数组个数
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
								memcpy(pbBuf, bTmpBuf, iLen);	//获取扫描到的数据有效长度
							}
						}
						else	//属性中的子属性数据
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
			else	//使用本接口抄读冻结类数据---陈亮完成
				return -1;
		}

		return iLen;
	}
	
	return -1;
}

//描述:通信协议调用的写对象属性值，本函数会调用OoPro2AppScan()对字节码顺序进行自动调整
//参数：@wOI	对象标识
//	   @bAtrr	属性标识及其特征 bit-string（SIZE（8））
//	@bIndex属性内元素索引
//	   @pbBuf	要写入属性值的缓冲区，要写入数据为通信协议字节顺序
int OoProWriteAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wLen, bool fIsSecurityLayer)
{
	BYTE bTmpBuf[3000], bType, bPassWord[16];	//临时数据存放,去格式后的数据将存放在这里,用于下一步写入到系统数据库.
	BYTE bDbBuf[3000];
	BYTE *pbTmpBuf = bTmpBuf;
	BYTE bPerm = 0x11;
	ToaMap* pOI = NULL;
	WORD wID, wDataLen;
	int iLen = -1;
	int iLen0;
	//int iDataLen = 0;
	//BYTE bOADBuf[4];

	memset(bTmpBuf, 0, sizeof(bTmpBuf));//防止当变长数据个数为0时（如重点表计清单清除）设置下取随机值
	if ((bAttr == 0) || ((bAttr == 1)))	//0属性,逻辑名属性是只读的
	{
		return -1;	//读写拒绝
	}
	else //其他属性
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

		if (pOI->wMode == MAP_VAL || pOI->wMode == MAP_BYTE)	//如果是固定数据格式，不允许更改
			return -1;

		if (bIndex == 0)	//全属性
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
					//写对应的属性之前，先判断下与系统库中的属性值是否一致，不一致就写系统库
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

					if ((dwOIAtt&0xf0000000) == 0x30000000) // 事件
					{						
						ReInitEvtPara(dwOIAtt);
						if (dwOIAtt==0x31060900 && pbTmpBuf[1]==1)	//停电事件有效性从无效变为有效
						{
							UpdateTermPowerOffTime();
						}
					}
					else if ((dwOIAtt&0xf0000000) == 0x21000300)	//统计参数变更，通知冻结更新
					{
						OnStatParaChg();
					}

					if ((iLen0 = WriteItemEx(BN0, pOI->wPn, pOI->wID, pbTmpBuf, bPerm, bPassWord)) <= 0)
					{
						DTRACE(DB_FAPROTO, ("DlmsWriteAttrToDB: There is something wrong when call WriteItemEx()\n"));
						return -1;	//返回其它错误
					}
				}
			}
			else
			{
				DTRACE(DB_FAPROTO, ("OoProWriteAttr: OoDataScan Data failed, nRet=%d !\n", nRet));
				return -1;
			}

			if (iLen == -1)//对应不去格式直接写的情况
				iLen = iLen0;
		}
		else	//子属性
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
			//	iDataLen = OoGetDataLen(DT_OAD, bOADBuf);	//+1:去除数据类型
			//	if (!(iDataLen >= wLen))
			//		wLen = iDataLen;	
				if (fIsSecurityLayer)
					iSrcLen = OoWriteField(bSrc, iSrcLen, pbFmt, wFmtLen, bIndex-1, pbBuf, wLen);	//-4: 4字节OAD，Esam加密没有时间标签
				else
					iSrcLen = OoWriteField(bSrc, iSrcLen, pbFmt, wFmtLen, bIndex-1, pbBuf, wLen);	//-5: 4字节OAD + 1字节时间标签	//此处应该在外面处理wLen参数传进来OAD和时间标签就已经去掉了
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

//描述:应用层调用的读取对象属性值，相当于从数据库中直接取到数据，并得到格式描述，OoProReadAttr()可以基于本函数来实现
//参数：@wOI	对象标识
//		@bAtrr	属性标识及其特征 bit-string（SIZE（8））
//		@pbBuf	返回属性值的缓冲区，返回数据为应用字节顺序
//		@ppFmt	用来返回格式描述串，如果为空不返回
//		@pwFmtLen	用来返回格式描述串的长度，如果为空不返回
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
			if (IsAllAByte(bBuf, 0x00, nRet))	//如果全部是0x00，那应该是没有写过，格式不可能为0
			{
				nRet = OIFmtData(bBuf, pbBuf, pOI->pFmt, pOI->wFmtLen);
			}
			else
			{
				if (IsNeedRdSpec(pOI) && (wOI&0xFF00)==0xf200)	//输入输出OAD需去掉DT_ARRAY和数组个数
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

//描述:应用层调用的写对象属性值，相当于直接把数据写入数据库
//参数：@wOI	对象标识
//		@bAtrr	属性标识及其特征 bit-string（SIZE（8））
//		@bIndex属性内元素索引-----暂时不用考虑吧
//		@pbBuf	要写入属性值的缓冲区，要写入数据为应用字节顺序
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

//描述：是否协议标准的数据类型
bool IsStdType(BYTE bType)
{
	if (bType<=32 || (bType>=DT_OI && bType<=DT_RCSD))
		return true;

	return false;
}

//描述:扫描带格式的数据，以确定数据是否符合格式要求，如果有需求则把数据字节顺序进行反序，可以对整个数据格式进行扫描，也可以扫描到某个字段即返回
//参数：@pbSrc		源数据
//	   @pbFmt		格式描述串
//	   @wFmtLen		格式描述串的长度
//	   @fRevOrder	是否需要把数据字节顺序进行反序
//	   @iIndex		>=0:要扫描到的字段的索引，-1全部扫描
//	   @pwLen		用来返回字段的数据长度
//	   @pbType		用来返回该字段的类型
//     @ppFieldFmt 	当iIndex>=0时，用来返回字段的格式串,如果不需要可以传入NULL
// 	   @pwFieldFmtLen 当iIndex>=0时，用来返回字段的格式串的长度,如果不需要可以传入NULL
//返回:正确：iIndex==-1：返回全部扫描的长度；iIndex>=0:字段的起始位置
//     错误：返回负数
int OoScanData(BYTE* pbSrc, BYTE* pbFmt, WORD wFmtLen, bool fRevOrder, int iIndex, WORD* pwLen, BYTE* pbType, BYTE** ppFieldFmt, WORD* pwFieldFmtLen)
{
	BYTE bStack[100], bTmpBuf[100];
	BYTE* pbSrc0 = pbSrc;
	BYTE* pbStart = NULL;  //找到的字段的起始位置
	BYTE* pbFmtEnd = pbFmt + wFmtLen;
	int iSP = 0, iTmpSP=0;	//堆栈指针(两个不同堆栈)
	BYTE bFP = 0;	//格式指针
	BYTE bNum=0, bTmpFP=0xff;
	DWORD bLen, bSrcLen, bArrLen, dwNum;
	BYTE bFmtT, bVFmt, bVFill;
	bool fVLen, fPop, fFind, fGetFirst=false;
	BYTE bPushCnt = 0;
	BYTE bAcqTyp;	//采集类型
	BYTE bSrcType = DT_UNCARE; //don't care
	int iNullSP = 0;	//刚遇到null data时的iSP,当扫描完一轮数据后,iSP回复到iNullSP的值,
						//表示对null data的处理完毕
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

	//如果查找子属性，那么第一个类型描述，必须是01或02
	if ((bAttrType != pbFmt[0]) /*|| (iIndex>=0 && bAttrType!=0x01 && bAttrType!=0x02)*/)
	{//第一层的数据必须是数组或结构
		DTRACE(DB_DB, ("OoScanData: The First data-type:%d error !!\n", bAttrType));
		return -1;
	}
	
	if (iIndex >= bAttrNum && iIndex>0)
	{//所读索引超格式字符串个数 lzx 20170217
		DTRACE(DB_DB, ("OoScanData: iIndex overflow, return!!\n"));
		return -1;
	}		

	fFind = false;
	//从结构或数组之后的数据类型开始
	do
	{
		fPop = true;
		BYTE bType = pbFmt[bFP++];
		if (bSrcType != DT_NULL)	//当前数据不是处于空结构或数组的下一层
		{
			bSrcType = *pbSrc++;
			if (bSrcType == DT_NULL) //null data
				iNullSP = iSP;		 //在第一个空数据时记下堆栈指针,用来判断什么时候空数据完
		}

		if (bSrcType!=DT_NULL && bSrcType!=bType && IsStdType(bType))	//类型不符合
		{
			wErr = FMT_ERR_TYPE;
			goto OoScanData_err;
		}

		if (iIndex==0 && !fGetFirst)	//如果请求子属性为第一个，那需要先处理下
		{
			fFind = true;
			fGetFirst = true;
			pbStart = &pbSrc[1];
			*pbType = pbSrc[1];

			if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
				*ppFieldFmt = &pbFmt[bFP+1];
		}

		if (fFind && iIndex>0)	//iIndex>=0表示要抄读某个子域
		{
			*pbType = bType;
			fFind = false;
		}

		switch (bType)
		{
		case 0: 
			break;
		case 1: //array
			bLen = pbFmt[bFP++];		//array的个数
			if (bSrcType == DT_NULL)	//本级数组或者上级数组或结构为空数据
			{
				if (bLen == 0)	//格式串中的长度为0
				{		//在当前格式是数组的情况下,如果源数据为空,			
						//则格式串中的长度不能为0,否则不知道置为无效数据的数组元素个数
					if (bFP == 2)	//格式串的第一个元素是数组,数组为空则允许
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
				if (bLen == 0)	//格式串中的长度没限定,取数据中的长度
				{
					if (bFP != 2)	//只允许在第一个元素是数组的情况下预留元素个数为0
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
				else	//正常情况
				{
					if (bSrcLen > bLen)	//在格式串的长度明确的情况下,源数据中的长度不符合
					{	
						wErr = FMT_ERR_LEN;
						goto OoScanData_err;
					}

					if (bSrcLen == 0)	//入栈的时候bSrcLen=0
					{	//源数据长度bSrcLen为0,也要进行一次扫描,用来算数组元素的长度
						bSrcType = DT_NULL; //源数据个数为0,则后面数组成员也相当于遇到了空数据
						iNullSP = iSP; 
						
						//bFP++;
					}
				}
			}
			
			bStack[iSP++] = bFP;
			bStack[iSP++] = bType;
			bStack[iSP++] = bSrcLen;	//剩余记录数
										//即使数组元素个数为0,也要有一次入栈的操作,
										//让程序扫描过元素的格式定义
			fPop = false;
			bPushCnt++;
			break;

		case 2: //structure
		case DT_FRZRELA: //structure
			bLen = pbFmt[bFP++];

			if (bSrcType != DT_NULL)
			{
				bSrcLen = *pbSrc++;		//结构长度
				if (bSrcLen != bLen)	//长度不符合
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
			bLen = (bLen + 7) / 8;			//转换为字节数
			if (bSrcType != DT_NULL)
			{
				bSrcLen = *pbSrc++;				//位长度
				bSrcLen = (bSrcLen + 7) / 8;	//转换为字节数

				if (bSrcLen>bLen ||
					(!fVLen && bSrcLen!=bLen)) //定长,不能填充,要求下发的长度必须刚好相等
				{
					wErr = FMT_ERR_LEN;
					goto OoScanData_err;
				}
				//暂定bitstring不需要反序，如果需要，将来增加
				pbSrc += bSrcLen;
			}
			break;

		case 9:	//octet-string            	[9] 		IMPLICIT OCTET STRING,
		case 10: //visible-string
		case 12: //UTF8-string
			//暂定string不需要反序，如果需要，将来增加


			bFP += DecodeLength(&pbFmt[bFP], &bLen);
			//bLen = pbFmt[bFP++];
			bVFmt = pbFmt[bFP++];

			fVLen = (bVFmt&VF_VLEN)!=0; 
			bVFill = bVFmt>>VF_FILL_SHF;

			if (bSrcType != DT_NULL)
			{
				pbSrc += DecodeLength(pbSrc, &bSrcLen);
 				//bSrcLen = *pbSrc++;	//长度

				if (bSrcLen > bLen)
				{
					wErr = FMT_ERR_LEN;
					goto OoScanData_err;
				}
				
				if (bSrcLen>bLen ||
					(!fVLen && bSrcLen!=bLen)) //定长,不能填充,要求下发的长度必须刚好相等
				{
					wErr = FMT_ERR_LEN;
					goto OoScanData_err;
				}
				pbSrc += bSrcLen;	//不论是否定长，都不填充数据，只针对下发的数据按是否需要调换数据执行
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
		case DT_OMD://OMD----类型都是结构性质的，不能直接倒叙
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
				//pbSrc+=2;	//带格式emu
				pbSrc+=3;	//带格式long-unsigned----------假定seq内部数据带格式了
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
			pbSrc+=2;	//带格式emu
			break;

		case DT_SCALE_UNIT://Scaler-Unit
			pbSrc+=2;	//带格式integer
			pbSrc+=2;	//带格式emu
			break;

		case DT_RSD://RSD 
			nDataLen = ScanRSD(pbSrc, fRevOrder);	//该函数自动修改了pbSrc偏移
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

		case DT_COMDCB://COMDCB,串口通讯参数
			/*pbSrc+=2;	//enm类型+波特率
			pbSrc+=2;	//enm类型+校验位
			pbSrc+=2;	//enm类型+数据位
			pbSrc+=2;	//enm类型+停止位
			pbSrc+=2;	//enm类型+流控*/
			pbSrc++;	//波特率
			pbSrc++;	//校验位
			pbSrc++;	//数据位
			pbSrc++;	//停止位
			pbSrc++;	//流控
			break;
		case DT_ACQ_TYPE:
//			pbSrc++;	//struct 取得格式的时候已经取了
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
			bMtrAnnex = *pbSrc++;	//array个数
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
				if (bDataLen2 != bDataLen)  //判定内部数据长度与外部主站下发数据长度一致性，不一致则报错 add by lisonwu 20170411
					goto OoScanData_err;
				pbSrc += bDataLen2;
			}
			break;
		case DT_RPT_TYPE:
			//			pbSrc++;	//struct 取得格式的时候已经取了
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
			//pbSrc++;	上面已经取得了Arry
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
				pbSrc++;	//array类型
				bNum = *pbSrc++;

				for (BYTE i=0; i<bNum; i++)
				{
					pbSrc++; //ROAD
					nDataLen = ScanROAD(pbSrc, fRevOrder);	//该函数自动修改了pbSrc偏移
					if (nDataLen > 0)
						pbSrc += nDataLen;
					else
						goto OoScanData_err;
				}
			}
			break;
		default:
			wErr = FMT_ERR_UNK_TYPE;	//未知类型
			goto OoScanData_err;
		}
		
		if (fPop)	//需要出栈
		{
			while (iSP > 0)
			{
				bLen = bStack[--iSP];	//长度
				bType = bStack[--iSP];	//类型
				if (bType == 0x01)		//array
				{
					bTmpFP = bStack[--iSP];
				}

				if (bLen > 0)
					bLen--;

				if (bLen > 0)	//结构或数组还没完
				{
					BYTE bOldFP = bFP; //移动后的FP
					if (bType == 0x01)		//array
					{
						bFP = bTmpFP;
						bStack[iSP++] = bFP;
					}
					
					bStack[iSP++] = bType;	//类型
					bStack[iSP++] = bLen;	//长度

					if (bPushCnt == 1)
					{
						wIdx++;
						if (wIdx==iIndex && !fFind)	//抄读到了子属性
						{
							pbStart = pbSrc;
							if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
								*ppFieldFmt = &pbFmt[bFP];

							fFind = true;
						}

						if (bPushCnt==1 && iIndex>=0 && wIdx>iIndex) //扫描完所要找的元素
						{
							*pwLen = pbSrc - pbStart;
							nRetLen = pbStart - pbSrc0;		//直接返回缓冲的偏移
							if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
								*pwFieldFmtLen = &pbFmt[bOldFP] - *ppFieldFmt;

							return nRetLen;					//直接返回缓冲的偏移
						}
					}
					break;		//跳出出栈循环,继续下一个结构成员或数组元素
				}
				else 	//结构或数组结束了，可以算做一个成员的结束
				{
					bPushCnt--;  //出栈一个结构或数组
				}

				if (bSrcType==DT_NULL && iNullSP==iSP) //null data
					bSrcType = DT_UNCARE; //don't care

				//消耗掉一个元素，判断wIdx是否要递增
				if ((iSP==0 && bPushCnt==0))   //(iSP!=0 && bPushCnt==1) || 
				{
					wIdx++;
					if (iIndex >= 0)	//请求了子属性
					{
						if (wIdx == iIndex)	//抄读到了子属性
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
							nRetLen = pbStart - pbSrc0;		//直接返回缓冲的偏移
							if (ppFieldFmt!=NULL && pwFieldFmtLen!=NULL)
								*pwFieldFmtLen = &pbFmt[bFP] - *ppFieldFmt;

							return nRetLen;					//直接返回缓冲的偏移
						}
					}

					//if (bLen == 0)	//不是具体搜索某个子域，同时在最高层所有元素遍历完成
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

//描述：扫描数据，将不同类型的长度及偏移返回
//参数：pbSrc：源数据buf，pbFmt：数据格式，wFmtLen：数据格式长度
//返回：正常返回该数据类型的长度，否则返回负数
int OoDataFieldScan(BYTE* pbData, BYTE* pbFmt, WORD wFmtLen)
{
	WORD wLen;
	BYTE bType;

	int nRet = OoScanData(pbData, pbFmt, wFmtLen, false, -1, &wLen, &bType);
	return nRet;
}


//描述:从一个格式化串中取得其中某个字段的指针，不用接收字段内容的缓冲区，
//		相对于OoReadField()更节省内存
//参数：@pbSrc		源数据
//	    @pFmt		源数据的格式描述串
//	    @wFmtLen	源数据的格式描述串的长度
//	    @wIndex		字段的索引
//	    @pwLen		用来返回字段的数据长度
//	    @pbType		用来返回该字段的类型，主要针对字段中存在CHOICE的类型，用来得到真实的类型
//      @ppFieldFmt 当iIndex>=0时，用来返回字段的格式串,如果不需要可以传入NULL
// 	    @pwFieldFmtLen 当iIndex>=0时，用来返回字段的格式串的长度,如果不需要可以传入NULL
//返回:如果正确则返回指向字段的指针,否则返回NULL
BYTE* OoGetField(BYTE* pbData, BYTE* pbFmt, WORD wFmtLen, WORD wIndex, WORD* pwLen, BYTE* pbType, BYTE** ppFieldFmt, WORD* pwFieldFmtLen)
{
	BYTE* pbtr = pbData;
	int nRet = OoScanData(pbData, pbFmt, wFmtLen, false, wIndex, pwLen, pbType, ppFieldFmt, pwFieldFmtLen);
	if (nRet > 0)
		return pbtr+nRet;
	else
		return NULL;
}

//描述: 从一个格式化串中修改其中某个字段的数据
//参数：@pbSrc	源数据
//	   @ pFmt	源数据的格式描述串
//	   @ wFmtLen	源数据的格式描述串的长度
//	   @wIndex 字段的索引
//	   @pbField	要修改的字段内容
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
		memcpy(bBakSrc, pbTmp+wLen, wBakLen);	//备份源数据pbSrc中wIndex后面的数据
		memcpy(pbTmp, pbField, wFieldLen);	//替换新字段数据
		memcpy(pbTmp+wFieldLen, bBakSrc, wBakLen);	//追加修改字段后的数据

		return OoScanData(pbSrc, pFmt, wFmtLen, false, -1, NULL, NULL);
	}
	
	return -1;
}

//描述: 把一个数组或结构的成员分析成数值，不支持结构嵌套
//参数：@pbSrc	源数据
//	   @ pFmt	源数据的格式描述串
//	   @ wFmtLen	源数据的格式描述串的长度
//	   @ piVal 用来返回分析出来的数值
//返回:如果正确则返回成员的个数,否则返回-1
//返回:如果正确则返回数据的长度（包括格式字符）,否则返回-1
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
		bNum = *pbSrc++;	//成员个数
		if (*pFmt++ != bNum)
			return -1;
		for (BYTE i=0; i<bNum; i++)
		{
			if (OoParseVal(pbSrc, pFmt, 1, piVal) < 0)
				return -1;
			pFmt++;	//这里可能会有问题,oct-string具体内容除了格式、还有数据长度，CL-20161117
			piVal++;
			pbSrc += OoGetDataTypeLen(pbSrc);
		}
		break;
	case DT_ARRAY:
		if (*pbSrc++ != DT_ARRAY)
			goto RET_PARSE;
		bNum = *pbSrc++;	//成员个数
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

//描述: 把单个数值类型、一个数组或结构的成员分析成数值，不支持结构嵌套
//参数：@dwOAD	 关联对象属性描述符
//	   @ piVals 用来返回分析出来的数值的数组
//	   @ wValMax  piVals数组允许接收的个数，超过返回错误
//返回:如果正确则返回成员的个数,否则返回-1
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
		if (bIdx != 0)	//取子属性
		{
			BYTE *p = OoGetField(bBuf, pbFmt, wFmtLen, bIdx-1, &wLen, &bType, &pFieldFmt, &wFieldFmtLen);
			if (p == NULL)
				goto Ret_OoReadVal;

			//if (wValMax > wFieldFmtLen)//tll 还是觉得不能用这个来判断异常
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
			//if (wValMax > wFmtLen)//tll 还是觉得不能用这个来判断异常
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
				//memcpy(piVals, iVals, wValMax);// 这样拷只是按字节个数拷了，不是按int的单元个数拷
			else
				memcpy(piVals, iVals, iRet);
			*/
		}

		return iRet;
	}

Ret_OoReadVal:

	return -1;
}


//描述: 获取Data类型的值
//参数：@ pbData	 协议格式的数据
//	   @ piVal 用来返回分析出来的数值
//返回:如果正确则返回数据（包含格式）的字节长度,否则返回-1
int OoGetDataVal(BYTE* pbData, int* piVal)
{
	BYTE bFmt;

	bFmt = *pbData;
	return OoParseVal(pbData, &bFmt, 1, piVal);
}

//描述: 值到协议格式的转换
//参数：@bValType数据类型
//	   @ pbData	 用来返回协议格式的数据,包含格式字节
//	   @ piVal 用来返回分析出来的数值
//返回:如果正确则返回数据（包含格式）的字节长度,否则返回-1
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

//描述: 取得dwOAD的数值类型
//参数：@ dwOAD	 对象属性描述符
//	   @ pbValType用来返回数据类型
//返回：如果正确且是数值类型则返回true,否则返回false
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

	if (bIndex != 0)	//子属性
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


//描述:从一个格式化串中读取其中某个字段的数据，方便应用层从一个结构或数组的数据中提取字段值或数组元素，字段/元素只能是第一层的字段/元素，不能处理嵌套到第二层的字段/元素。
//参数：@pbSrc	源数据
//	   @pFmt	源数据的格式描述串
//	   @wFmtLen	源数据的格式描述串的长度
//	@wIndex字段的索引
//	   @pbField	用来返回字段的缓冲区
//	   @pbType	用来返回该字段的类型，主要针对字段中存在CHOICE的类型，
//                   用来得到真实的类型
//返回:如果正确则返回字段的长度,否则返回负数
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

//描述:应用层调用的写对象属性值，相当于直接把数据写入数据库
//参数：@wOI	对象标识
//		@bAtrr	属性标识及其特征 bit-string（SIZE（8））
//		@bIndex属性内元素索引-----暂时不用考虑吧
//		@pbBuf	要写入属性值的缓冲区，要写入数据为应用字节顺序
int OoWriteAttr2(WORD wOI, BYTE bAtrr, BYTE* pbBuf)
{
	DWORD dwOIAtt = ((DWORD )wOI<<16) + ((DWORD )bAtrr<<8);
	const ToaMap* pOI = GetOIMap(dwOIAtt);
	if (pOI == NULL)
		return -1;

	int nRet = WriteItemEx(BN0, pOI->wPn, pOI->wID, pbBuf);
	return nRet;
}

//特殊OMD，不需要扫描格式
bool IsSpecOMD(const TOmMap* pOmMap)
{
	switch(pOmMap->dwOM)
	{
	case 0x21000100:	//统计复位
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
	case 0x22040100:	//复位复位次数结果
	case 0x43000100:	//设备接口类19--复位
	case 0x43000200:	//设备接口类19--执行
	case 0x43000300:	//设备接口类19--数据初始化
	case 0x43000400:	//设备接口类19--恢复出厂参数
	case 0x43000500:	//设备接口类19--事件初始化
	case 0x43000600:	//设备接口类19--需量初始化
	case 0x45000100:	//公网设备初始化
	case 0x45010100:	//公网设备初始化
	case 0x45100100:	//以太网设备初始化
	case 0x45110100:	//以太网设备初始化
	case 0x45120100:	//以太网设备初始化
	case 0x45130100:	//以太网设备初始化
	case 0x45140100:	//以太网设备初始化
	case 0x45150100:	//以太网设备初始化
	case 0x45160100:	//以太网设备初始化
	case 0x45170100:	//以太网设备初始化
	case 0x60008100:	//更新配置单元的基本信息对象
	case 0x60008200:	//更新配置单元的扩展信息以及附属信息，对象为NULL表示不更新
	case 0x60008300:	//删除配置单元，通过配置序号删除
	case 0x60008400:	//删除配置单元，通过基本信息对象删除
	case 0x60008500:	//删除配置单元，通过通信地址及端口删除
	case 0x60008600:	//删除所有电表
	case 0x60028000:	//清空台区采集方案
	case 0x60028100:	//清空跨台区采集方案
	case 0x60128000:	//电表：删除一组配置单元
	case 0x60128100:	//电表：清空任务配置表
	case 0x60148000:	//删除一组普通采集方案
	case 0x60148100:	//清空普通采集方案集
	//case 0x60148200:	//重置方案的记录列选择
	case 0x60168000:	//删除一组事件采集方案
	case 0x60168100:	//清空事件采集方案集
	case 0x60187f00:	//添加更新一个透明方案或添加一组方案内容
	case 0x60188000:	//添加一组报文
	case 0x60188100:	//删除一个方案的一组方案内容
	case 0x60188200:	//删除一组透明方案
	case 0x60188300:	//清空透明方案集
	case 0x601C8000:	//删除一组上报方案
	case 0x601C8100:	//清空上报方案集
	case 0x601E7f00:	//增加采集规则方案
	case 0x601E8000:	//删除一组采集规则方案
	case 0x601E8100:	//清空采集规则方案
	case 0x80017F00:	//投入保电
	case 0x80018000:	//解除保电
	case 0x80018100:	//解除自动保电
	case 0x80028000:	//取消催费告警
	case 0xF0010700:	//OI=F001 方法：07	启动下载
	case 0xF0010800:
	case 0xF0010900:
	case 0xF0010A00:
	case 0xF0020700:
	case 0xF0020800:	//OI=F000 方法：08	下载文件
		return true;
	default:
		return false;
	}

	return false;
}

//描述:给通信协议调用的执行某个对象的方法
//参数：@wOI		对象标识
//	   @bMethod	对象方法编号
//	   @ bOpMode	操作模式
//	   @pbPara		方法参数操作执行结果          DAR
//	@piParaLen	用来返回pbPara扫描过的长度，方便解帧时跳到下一个位置
//	@pbRes		用来返回（操作执行结果DAR及操作返回数据Data  OPTIONAL）的长度
//返回：如果正确则返回pbRes中结果的长度,否则返回负数
int DoObjMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int* piParaLen, BYTE* pvAddon, BYTE* pbRes)
{
	//搜索对象方法对应的映射表
	const TOmMap* pOmMap = GetOmMap((((DWORD )wOI)<<16)+(((DWORD )bMethod<<8)));
	if (pOmMap == NULL)
		return -1;

	*piParaLen = 0;

	int iParaLen = 0;
	if (!IsSpecOMD(pOmMap))
	{
		if (pbPara != NULL)	//hyl pbPara有可能入参为空
		{
			//扫描参数，求参数长度、对字节顺序进行调整
			iParaLen = OoDataFieldScan(pbPara, pOmMap->pFmt, pOmMap->wFmtLen);
			if (iParaLen< 0)
				return -1;
		}
	}

	//执行对象方法
	return pOmMap->pfnDoMethod(wOI, bMethod, bOpMode, pbPara, iParaLen, pOmMap->pvAddon, pOmMap->pFmt, pOmMap->wFmtLen, pbRes, piParaLen);
}

//描述：把源数据解析成一个个字段的偏移和长度，方便访问
//参数：@pParser	字段解析器
//	   @pFmt	源数据的格式描述串
//	   @wFmtLen	源数据的格式描述串的长度
//	   @fParseItem	为false时只解析到配置本身，
//					为true时把字段对应的数据项的长度及偏移也计算出来
//返回:如果正确则返回true,否则返回false
bool OoParseField2(TFieldParser* pParser, BYTE* pFmt, WORD wFmtLen, bool fParseItem)
{
	return true;
}

//描述：读取字段解析器里某个字段
//参数：@pParser	字段解析器
//	   @wIndex字段的索引
//	   @pbBuf用来返回字段的内容
//	   @pbType用来返回字段的类型
//	   @pwDataLen用来返回字段的数据长度
//返回:如果正确则返回字段描述本身的长度,否则返回负数
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

//描述：解析协议层RCSD内部关联OAD个数 如：01 50 04 02 00 02 00 10 02 00 00 20 02 00
//		内部关联个数为02
int OoScanRcsdInOadNum(BYTE *pbRcsd)
{
	BYTE bNum;

	if (*pbRcsd++ != 0x01)
		return -1;
	pbRcsd += 4;	//主OAD
	
	bNum = *pbRcsd;

	return bNum;
}

//描述: 取得数据项的长度
//参数：@ bType	数据类型，同Data的数据类型定义
//	   @ pItemDesc数据项目描述，可以OAD、ROAD、CSD
//返回：正确则返回数据项的长度，否则返回负数
//描述: 取得数据项的长度
//参数：@ bType	数据类型，同Data的数据类型定义
//	   @ pItemDesc数据项目描述，可以OAD、ROAD、CSD
//返回：正确则返回数据项的长度，否则返回负数
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
		dwOIAtt &= OAD_FEAT_MASK;	//获取OAD, 只屏蔽掉属性特征
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
				//读子属性
				pOAMap = GetOIMap(dwOIAtt & 0xFFFFFF00);
				if (pOAMap != NULL)
				{
					memset(bBuf, 0, sizeof(bBuf));

					iDataLen = OoReadAttr(dwOIAtt>>16, (dwOIAtt>>8)&0xff, bBuf, &pbFmt, &wFmtLen);
					if (iDataLen > 0)	//特殊OI，ID本身定义的就是第N个属性
					{
						if (!IsNeedRdSpec(pOAMap))	//非特殊OI
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
		iDataLen = GetEvtMainOadDataLen(OoOadToDWord(pbPtr));	//事件ROAD的描述符OAD纳入数据长度统计范围
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
			//计算RCSD中每个CSD的偏移
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

//描述：通过数据内容返回格式描述(主要针对采集方案)
//参数：@pbBuf 源数据
//		@pbFmtBuf 返回的格式描述串
//		@返回的格式描述串长度
//返回：<0失败，>=0表示某个格式在pbBuf中占据的偏移
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
		//其它格式需要时在添加
		break;
	}

	wFmtLen = pbFmtPtr-pbFmtBuf;

	return pbPtr - pbBuf;
}

// 获取属性OAD
DWORD GetOAD(WORD wOI, BYTE bAtrr, BYTE bIndex)
{
	return ((DWORD )wOI<<16) | ((DWORD )bAtrr<<8) | bIndex;
}

//把抄表返回的净数据转换成对象数据的格式并加上数据类型字符
//	@pbSrc 是从表端返回的BCD格式数据串
//	@pbDst 转换成对象数据的格式并加上数据类型字符的数据串
//	@pbFmt 各自OAD所对应的对象格式串
//	@wFmtLen 对象格式串长度
//	返回转换后pbDst的总长度
int OIFmtDataExt(BYTE* pbSrc, BYTE bsLen, BYTE* pbDst, BYTE* pbFmt, WORD wFmtLen, DWORD dwOAD)
{
	BYTE bStack[100];
	BYTE* pbSrc0 = pbSrc;
	BYTE* pbDst0 = pbDst;
	BYTE* pbFmtEnd = pbFmt + wFmtLen;
	int iSP = 0;	//堆栈指针
	BYTE bFP = 0;	//格式指针
	BYTE bTmpFP;
	BYTE bLen, bSrcLen, bArrLen, bByteLen;
	BYTE bFmtT, bVFmt, bVFill;
	bool fVLen;
	BYTE bNullArr = 0;
	WORD wOffset;
	BYTE bSrcType = 0xff;//DT_UNCARE; //don't care
	int iNullSP = 0;	//刚遇到空数组(数组个数为0)时的iSP,当扫描完一轮数据后,
						//iSP回复到iNullSP的值,表示对空数组的处理完毕
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

			if (bNullArr)	//上一级空数组还没完,子级还是空数组
			{
				bSrcLen = 0;
			}
			else
			{
				if (bSrcLen == 0)	//对于数组长度为0的空数组,以0(空数据)上送
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
			
			wOffset = pbSrc - pbSrc0;	//越过数组长度后的起始位置
			
			bStack[iSP++] = bLen;		//终端中存储数组时预留的记录笔数,用来计数源数据中还有多少笔记录没有跳过
			bStack[iSP++] = wOffset;	//记下源数据中数组元素的起始偏移,用来计算一笔记录的长度
			bStack[iSP++] = wOffset>>8;

			bStack[iSP++] = bFP;
			bStack[iSP++] = bType;
			bStack[iSP++] = bSrcLen; //即使数组元素个数为0,也要有一次入栈的操作,
									 //让程序扫描过元素的格式定义
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
				//确定bSrcLen和拷贝的bByteLen
				if (fVLen)
				{
					if (bVFill == 0) //0-不填充,加一个长度字节放到开头
					{
						bByteLen = *pbSrc++;
					}
					else	//用填充字节来填充
					{
						bVFill = g_bFillByte[bVFill];
						bByteLen = OIGetStrLen(pbSrc, bByteLen, bVFill);	//bByteLen发生改变
							//如果下发的没把全部下下来,在这种情况下上传的BITs数量可能跟下发的不一致
					}

					bSrcLen = bByteLen * 8;
				}
				else //定长,不能填充,要求长度必须刚好相等
				{
					bSrcLen = bLen;
				}
				
				//组帧
				*pbDst++ = bType;
				*pbDst++ = bSrcLen;		//数据长度
				if (bByteLen > 0)
				{
					if (bVFmt&VF_LR)
						memcpy(pbDst, pbSrc, bByteLen);		//跳过长度字节
					else
						revcpy(pbDst, pbSrc, bByteLen);

					//------新加这部分-------------
					if (bFmtT != 0)
						for (BYTE b=0; b<bByteLen; b++)			//2008-05-25  杨
							pbDst[b] = ByteBitReverse(pbDst[b]);	//对字节按位进行倒序
					//------新加这部分-------------
					pbDst += bByteLen;
				}

				//pbSrc += (bLen + 7) / 8 + 1;			//长度+内容
				pbSrc += (bLen + 7) / 8;			//长度+内容
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
					if (bVFill == 0) //0-不填充,加一个长度字节放到开头
					{
						bSrcLen = *pbSrc++;
					}
					else	//用填充字节来填充
					{
						bVFill = g_bFillByte[bVFill];
						bSrcLen = OIGetStrLen(pbSrc, bLen, bVFill);
					}
				}
				else //定长,不能填充,要求长度必须刚好相等
				{
					bSrcLen = bLen;
				}

				*pbDst++ = bType;
				*pbDst++ = bSrcLen;		//数据长度
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
			if (bNullArr)	//单字节数据目前不支持无效数据
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
					memset(pbDst, 0xFE, 2);//填充我们约定的无效值					
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
				//revcpy(pbDst, pbSrc, 4);	//不用将无效数据EE转换为0，主要是为现场区分真的0和无效数据
				WORD wIO = dwOAD >> 16;
				BYTE btLen = 4;
				if (wIO==0x2001 ||(wIO>=0x2004&&wIO<=0x2009) ||wIO==0x2017 ||wIO==0x2018 ||wIO==0x2019)
					btLen = 3;//这些数据在07协议中是3字节
				if ((bsLen < btLen) && (bsLen!=0))
					return -1;
				if (IsAllAByte(pbSrc, INVALID_DATA_MTR, bsLen) || bsLen==0)
				{
					*pbDst++ = bType;
					memset(pbDst, 0xFE, 4);//填充我们约定的无效值					
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
						dwval <<= 8;//对象中水流量是4个小数位
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
			//例子: 19 07 d7 08 07 ff 08 2b 39 ff 80 00 00  
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
			wErr = 4;//FMT_ERR_UNK_TYPE;	//未知类型
			goto OIFmtData_err;
		}
		
		if (fPop)	//需要出栈
		{
			while (iSP > 0)
			{
				bLen = bStack[--iSP];	//长度
				bType = bStack[--iSP];	//类型
				if (bType == 0x01)		//array
				{	
					bTmpFP = bStack[--iSP];
					wOffset = (WORD )bStack[--iSP] * 0x100;
					wOffset += bStack[--iSP];
					bArrLen = bStack[--iSP];

					if (bArrLen > 0)	//用来计数源数据中还有多少笔记录没有跳过
						bArrLen--;
				}

				if (bLen > 0)
					bLen--;

				if (bLen > 0)	//结构或数组还没完
				{
					if (bType == 0x01)	//array
					{
						wOffset = pbSrc - pbSrc0; //数组还没完,wOffset填入下一笔记录的起始
												  //位置,保证最后算出来的是一笔记录的长度
						bStack[iSP++] = bArrLen;
						bStack[iSP++] = wOffset;
						bStack[iSP++] = wOffset>>8;

						bFP = bTmpFP;
						bStack[iSP++] = bFP;
					}
					
					bStack[iSP++] = bType;	//类型
					bStack[iSP++] = bLen;	//长度
					break;
				}
				else
				{
					if (bType == 0x01)		//array
					{
						pbSrc += (pbSrc - pbSrc0 - wOffset) * bArrLen;
					}

					//一个结构中的全部字段数据都为无效数据0,目前先不把整个结构归整为0
					//如果要规整的话,只需记录目的数据结构的起始,判断结构全部成员是否都为0
				}

				//该结构或数组已完,退到它的上一层
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

