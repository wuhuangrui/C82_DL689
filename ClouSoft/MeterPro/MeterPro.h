/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterPro.h
 * 摘    要：本文件主要包含抄表协议的基本API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年8月
 * 备    注：
 *********************************************************************************************************/
#ifndef METERPRO_H
#define METERPRO_H

#include "ComAPI.h"
#include "Trace.h"
#include "DbConst.h"
#include "FaConst.h"
#include "sysdebug.h"
#include "Comm.h"
#include "MeterStruct.h"
#include "DbOIAPI.h"

#define MET_INVALID_VAL		0xffffffff  //无效数据

#define TXRX_RETRYNUM	2	//重发次数

#define MTR_FRM_SIZE  512

#define COM_TIMEOUT  1000	//串口缺省延时

////////////////////////////////
//电表协议指针
struct TMtrPro
{
	TMtrPara* pMtrPara;	//电表参数
	BYTE	bThrId;
	//void*	pvMtrPro;	//类似this指针
	BYTE*	pbRxBuf; 
	BYTE*	pbTxBuf;
	BYTE*	pbCurve;

	//各电表协议成员函数
	int	(* pfnAskItem)(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD); //获取数据的对外接口函数
	int	(* pfnDirAskItem)(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData); //代理接口函数

	bool (* pfnRcvBlock)(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //解析接收函数
	void (* pfnGetProPrintType)(BYTE* pbPrintPro, char* pszProName);//获取打印协议名称
	int	(* pfnWriteItem)(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen); //获取数据的对外接口函数
}; 

////////////////////////////////////////////////////////////////////////////////////////////
//MtrProIf私有成员变量
//共用收发缓存
extern BYTE m_bInvdData;
extern BYTE m_MtrTxBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];
extern BYTE m_MtrRxBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];
extern BYTE m_CurveBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];

/////////////////////////////////////////////////////////////////////////////////////
//各电表协议公共函数
void InitMeterPro();
int SchStrPos(char* pStr, int iStrLen, char c);
WORD GetCRC16(BYTE* pbyt,int iCount);
float POW10(signed char n); //10的n次方
int64 POW10N(signed char n);//10的n次方（n> 0）
int64 Round(int64 iVal64);  //四舍五入	
bool IsRateId(WORD wID);		//是否跟费率有关的计量ID
bool IsDemdId(WORD wID);		//是否需量ID（含需量发生时间）
bool IsDemdTime(WORD wID);		//是否需量时间
bool IsLastMonthId(WORD wID);	//是否上月ID	
bool IsPhaseEngId(WORD wID);	//是否分相电能ID
bool Is645NotSuptId(WORD wID);	//是否是645（97版）不支持需要快速返回的ID
BYTE GetBlockIdNum(WORD wID);	//取块ID的子ID个数
BYTE Get645TypeLength(WORD wID);//取645类型的数据长度
WORD SetCommDefault(WORD wID, BYTE* pbBuf); //设置通用格式ID的无效值
void CheckRate(BYTE* pbRateTab, BYTE* pbData, BYTE nlen);	//调整分费率
void CheckDecimal(BYTE bToltLen, BYTE bItemLen, BYTE bNewDec, BYTE bOldDec, BYTE* pbBuf); //调整小数位	
void CheckDecimalNew(BYTE bDstLen, BYTE bSrcLen, BYTE bDstDec, BYTE bSrcDec, BYTE* pbDstBuf, BYTE* pbSrcBuf);//调整某项的小数位
WORD Data645ToComm(WORD wID, BYTE* pbBuf, WORD wLen);//97版645协议格式数据转公共格式

WORD Id645V07toDL645(WORD wExtId);//将非2007版645协议读取的扩展ID转为读相应645ID，以兼容698终端上的读取
WORD Data645to645V07(WORD w645Id, BYTE* pbBuf, WORD wLen);//将97版645读回的数据转为2007版64的数据格式，以兼容698终端上的读取
Toad645Map* GetOad645Map(DWORD dwOAD);
Toad645Map* GetOad645ExtMap(DWORD dwOAD);
TErcRdCtrl* GetOad07645ErcMap(DWORD dwOAD);
DWORD GetMtrEvtTimesID(BYTE bBit);
TErcRdCtrl* GetRd07645ErcMap(DWORD dwErcNumID);

/////////////////////////////////////////////////////////////////////////////////////
//串口操作函数
bool MtrProOpenComm(CComm* pComm, TCommPara* pCommPara);//要判断串口是否已经打开,参数是否需要改变
DWORD MtrProSend(CComm* pComm, BYTE* pbTxBuf, DWORD wLen); //要添加发送码打印
DWORD MtrProRcv(CComm* pComm, BYTE* pbRxBuf, DWORD dwBufSize); //要添加接收码打印

/////////////////////////////////////////////////////////////////////////////////////////
//内部接口函数
bool ReadCommFrm(struct TMtrPro* pMtrPro, void* pTmpInf, DWORD dwDelayTime, DWORD dwMaxSec, DWORD dwMinSec, DWORD dwTimeOut, 
				 DWORD dwReadLength, DWORD dwBufSize, BYTE* pbBuf, DWORD dwFrmSize);

#endif //METERPRO_H


