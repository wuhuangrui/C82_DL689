/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CvtExtPro.h
 * 摘    要：本文件给出了接口转换器扩展协议的相关定义
 * 当前版本：1.0
 * 作    者：邱林生
 * 完成日期：2016年03月
 * 备    注：
 *********************************************************************************************************/
#ifndef CVTEXTPRO_H
#define CVTEXTPRO_H
#include "stdafx.h"
#include "MeterPro.h"

#define PN_NUM_IN_CVT	  64
#define CVT_MAX_NUM		  10
#define CVT_ADDR_LEN	  6

//#define CVT_MAX_NUM		  1

typedef struct  
{
	BYTE bCvtAddr[6];
	BYTE bPort;
	BYTE bMtrType[PN_NUM_IN_CVT];
	BYTE bMtrAddr[PN_NUM_IN_CVT][7];
	BYTE bPnNum;
	WORD wCrc; //改为校验和，CRC要求档案顺序一致才能算对
}TCvtInfo;

typedef struct
{
	BYTE bCvtNum;
	BYTE bAllCvtAddr[CVT_MAX_NUM][CVT_ADDR_LEN];
	BYTE bAllPort[CVT_MAX_NUM];
}TAllCvtInfo;

int MakeCvtExtFrm(BYTE* pbAddr, const BYTE* pbDataArea, BYTE bDataAreaLen, BYTE* pbFrm); //组接口转换器扩展协议帧
int DoCvtFwdFunc(BYTE bPort, DWORD dwCvtID, BYTE* pbCvtAddr, BYTE* pbRxFrm, DWORD dwRxBufSize, BYTE* pbTxBuf, WORD wTxBufLen);//485端口转发函数
int CvtExtHandleFrm(DWORD dwCvtID, BYTE* pbRxFrm, BYTE* pbTxFrm, BYTE* pbDataBuf, DWORD dwRxSize);//接口转换器扩展协议帧处理
void InitTCvtInfo(TCvtInfo *tpCvtInfo); //初始化转换器信息
bool SyncDocsToCvt(TAllCvtInfo *ptAllCvt); //同步档案的具体实现
int SelectDocs(TCvtInfo *tpCvtInfo, BYTE *bpBuf, WORD wBufLen, BYTE *bpAddBuf, BYTE *bpDelBuf, WORD *wpAddBufLen, WORD *wpDelBufLen);//比较集中器和转换器的档案，找到转换器中要增加的和要删除的档案
int GetAllDocs(TCvtInfo *tpCvtInfo, BYTE *pbTxBuf); //获得所有档案并打包
void DoSyncDocs(); //执行同步档案
int CvtRcvBlock(BYTE* pbBlock, DWORD dwLen, BYTE* pbRxBuf, DWORD* pwRxLen, DWORD dwBufSize);//描述：从串口缓冲区中找满足转换器扩展协议的报文
int CheckNumAndCrc(TCvtInfo *tpCvtInfo, BYTE *pbDocNum);//查询档案个数和CRC
int QueryDocs(TCvtInfo *tpCvtInfo, BYTE bDocNum, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbTxBuf, BYTE *pbDataBuf, DWORD dwRxBufSize); //查询出转换器里的所有档案
int AddDocs(TCvtInfo tCvt, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbAddBuf, BYTE *pbDataBuf, DWORD dwRxBufSize); //增加档案
int DelDocs(TCvtInfo tCvt, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbDelBuf, BYTE *pbDataBuf, DWORD dwRxBufSize); //删除档案
int ClearDocs(BYTE bPort, BYTE *pbCvtAddr, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbTxBuf, BYTE *pbDataBuf, DWORD dwRxBufSize); //清空档案
bool PortInit(); //初始化端口信号量
int GetCvtNums(TAllCvtInfo *ptAllCvt); //获取转换器的数量和地址信息
void GetCvtInfo(TCvtInfo *ptCvtInfo, BYTE *pbInfobuf); //从集中器中统计出转换器下的配置信息
int FindCvtAddrToClear(TAllCvtInfo *tBackCvt, const BYTE bBackNum, TAllCvtInfo *ptAllCvt, const BYTE bCmpNum, TAllCvtInfo *tClearCvt); //找到需要清空档案的转换器地址，函数返回要清空的转换器个数

#endif //CVTEXTPRO_H

