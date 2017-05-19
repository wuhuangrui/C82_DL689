/***********************************************************************************************
* Copyright (c) 2008,深圳科陆电子科技股份有限公司
* All rights reserved.
* 
* 文件名称：DpGrp.h
* 摘    要: 本文件提供总加组相关数据的实现
* 当前版本：1.0
* 作    者：潘香玲
* 完成日期：2008年3月
* 备    注：
***********************************************************************************************/
#ifndef   DPGRP_INCLUDED
#define   DPGRP_INCLUDED

#include "DataProc.h"
#include "DbConst.h"
#include "LibDbStruct.h"

typedef struct{		
	TBankItem biRepItem[PN_NUM];				//测量点起始ID
	int64	  iStartVal[PN_NUM][BLOCK_ITEMNUM];	//测量点起始数据	
	DWORD	  dwUpdSec;							//本组数据的更新时标	
}TGrpFrzStartData;

typedef struct{
	BYTE bPn;		//测量点号	
	BYTE bProp;		//测量点号类型	
	BYTE bOp;		//运算符
	BYTE bDir;		//方向符
	WORD wCurId;		//运算ID
	WORD wDayStartId;	//运算当日差值时对应的起点ID
	WORD wMonStartId;	//运算当月差值时对应的起点ID	
}TGrpInf;

//总加组当前控制状态
typedef struct{
	int64 CurPwrVal;	//当前功控定值
	int FloatRate;		//当前功率下浮控浮动系数
	BYTE bAllPwrCtrlOutPutSta;	//功控跳闸输出状态
	BYTE bMonthCtrlOutPutSta;	//月电控跳闸输出状态
	BYTE bBuyCtrlOutPutSta;	//购电控跳闸输出状态
	BYTE bPCAlarmState;	//功控越限告警状态 
	BYTE bECAlarmState;	//电控越限告警状态
}TGrpCurCtrlSta;

//总加组控制设置状态
typedef struct{
	BYTE bSchemeNum;	//时段控定值方案号
	BYTE bValidFlag;	//功控时段有效标志位
	BYTE bPwrCtrlSta;	//功控状态
	BYTE bEngCtrlSta;	//电控状态
	BYTE bPwrCtrlTurnSta;	//功控轮次状态
	BYTE bEngCtrlTurnSta;	//电控轮次状态
}TGrpCtrlSetSta;

//总加组的类
class CDpGrp:public CDataProc
{
public:
	CDpGrp(void);
	virtual ~CDpGrp(void);

	bool Init(WORD  wPn);	
	void DoDataProc();

	void LoadData();
	void LoadPara();	
	bool IsNeedReset();	
	void SetStartDeltaE(BYTE bInterv, WORD wDeltaEId, TTime& tm); 
	//void SetStartCurEn(BYTE bType, WORD wCurEnId, TTime& tm);
	void SetNewStartEnFlg(bool fFlg, DWORD dwNewTime);
	
private:		
	void CalcuCurData(TTime tmNow);
	void PowerStat(TTime tmNow);

	void DayChange(TTime tmNow);
	void MonChange(TTime tmNow);	
	void UpdDayMonStartVal(BYTE bInterv, TTime tmNow);
	void UpdClsFrzData(BYTE bIdx, BYTE bInterv, DWORD dwOldS, DWORD dwNewS);
	void DeltaENew(WORD wResultID, TGrpInf* pGrpInf, BYTE bInterv, TIntvSec isNow, TIntvSec isStart, TIntvSec isDelta);	
	void CalcuEnSum(int64* pVal64, BYTE n, BYTE bInterv, TIntvSec isNow, BYTE bEnType);
	//描述：在正常运行时（非测量点变更时）判断总加组的测量点当前是否示度下降
	int IsMtrEngDec(BYTE n, BYTE bInterv, TIntvSec isNow);	

	bool InitPara(BYTE& bReqNum);	
	int  GetIdInxE(WORD wCurID);

	TTime m_tmLast;	//用于记录判断准点的时间切换

	BYTE  m_bMtrNum; //测量点总数

	//参数信息
	int m_iCT[PN_NUM];			//总加组的相关测量点的CT参数,对应总加组的测量点配置序号而不是测量点号
	int m_iPT[PN_NUM];			//总加组的相关测量点的PT参数
	
	TGrpInf m_GrpInfP[PN_NUM];	//总加有功功率的相关测量点信息,对应总加组的测量点配置序号而不是测量点号
	TGrpInf m_GrpInfQ[PN_NUM];	//总加无功功率的相关测量点信息
	TGrpInf m_GrpInfEp[PN_NUM];	//总加有功电能的相关测量点信息
	TGrpInf m_GrpInfEq[PN_NUM];	//总加无功电能的相关测量点信息

	//总加组的数据需要提交的ID
	TBankItem m_biRepP[PN_NUM]; //总加有功功率
	TBankItem m_biRepQ[PN_NUM]; //总加无功功率
	TBankItem m_biRepEp[PN_NUM]; //总加有功电量（含费率）
	TBankItem m_biRepEq[PN_NUM]; //总加无功电量（含费率）

	//累计数据
	int64 m_iDayDeltaE[2][BLOCK_ITEMNUM]; //当日总加电量差值量（对应有功、无功）
	DWORD m_dwDayDeltaESec[2];	//当日电量的时标
	int64 m_iMonDeltaE[2][BLOCK_ITEMNUM]; //当月总加电量差值量（对应有功、无功）	
	DWORD m_dwMonDeltaESec[2];	//当月电量的时标

	//总加组缓存的测量点的日月起始数据，以解决与测量点数据更新的不同步问题
	TGrpFrzStartData m_gfsdDayStart[2]; //测量点日起始电量（含有功、无功）
	TGrpFrzStartData m_gfsdMonStart[2]; //测量点月起始电量（含有功、无功）

	int64 m_iOldDayMtrE[2][BLOCK_ITEMNUM];	//测量点日起始示值（含有功、无功，不乘CTPT,用于检测该总加组的测量点的起始值是否有变化）	
	DWORD m_dwOldDayMtrUpdSec[2];			//测量点日起始示值对应的时标（含有功、无功）
	int64 m_iOldMonMtrE[2][BLOCK_ITEMNUM];	//测量点月起始示值（含有功、无功，不乘CTPT,用于检测该总加组的测量点的起始值是否有变化）
	DWORD m_dwOldMonMtrUpdSec[2];			//测量点月起始示值对应的时标（含有功、无功）

	bool m_fNewDayStartEnFlg[2]; //总加组日起点示值（含有功、无功）是否采用当前示值的标志
	bool m_fNewMonStartEnFlg[2]; //总加组月起点示值（含有功、无功）是否采用当前示值的标志
	DWORD m_dwEnNewStartSec;	//总加组要读的新的当前起点值的时间
	DWORD m_dwDayStartEnSec[2];	//总加组日起点示值的更新时标(含有功、无功)
	DWORD m_dwMonStartEnSec[2];	//总加组月起点示值的更新时标(含有功、无功)
};

extern void SetGrpParaChg(bool fFlg);
extern bool IsGrpParaChg();
extern void SetCtrlGrpParaChg(bool fFlg);
extern bool IsCtrlGrpParaChg();
void UpdGrpDataProcess(bool fPowerUp);
extern void RunGrpDataProcess();
bool GetGrpCurCtrlSta(int iGrp, TGrpCurCtrlSta *pGrpCurCtrlSta);
bool SetGrpCurCtrlSta(int iGrp, TGrpCurCtrlSta *pGrpCurCtrlSta);
bool GetGrpCtrlSetSta(int iGrp,	TGrpCtrlSetSta *pGrpCtrlSetSta);
bool SetGrpCtrlSetSta(int iGrp,	TGrpCtrlSetSta *pGrpCtrlSetSta);
#endif
