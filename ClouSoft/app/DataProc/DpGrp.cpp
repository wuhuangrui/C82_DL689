/****************************************************************************************************
* Copyright (c) 2008,深圳科陆电子科技股份有限公司
* All rights reserved.
* 
* 文件名称：DpGrp.cpp
* 摘    要: 本文件提供总加组相关数据的实现
* 当前版本：1.0
* 作    者：李锦仙
* 完成日期：2017年4月
* 备    注：$总加组使用到的测量点的15分钟、日、月起点值由CDpMtr负责保存
****************************************************************************************************/
#include "stdafx.h"
#include "DpGrp.h"
#include "FaStruct.h"
#include "DbConst.h"
#include "DbAPI.h"
#include "FaAPI.h"
#include "MeterAPI.h"
#include "DbOIAPI.h"
#include "MtrHook.h"
#include <math.h>
#include "apptypedef.h"

static WORD g_wGrpCurPowerID[2] = {0x2302, 0x2303}; //当前总加功率
static WORD g_wGrpSlidePowerID[2] = {0x2304, 0x2305}; //当前总加功率
static WORD g_wGrpDayDeltaID[2] = {0x2306, 0x2307}; //当日累计
static WORD g_wGrpMonDeltaID[2] = {0x2308, 0x2309}; //当月累计
static WORD g_wGrpDayLeftDeltaID[2] = {0x035F, 0x036F}; //参数配置前剩下的日delta电量
static WORD g_wGrpMonLeftDeltaID[2] = {0x037F, 0x038F}; //当月起点累计
static WORD g_wGrpDayStartEnID[2] = {0x039F, 0x03AF}; //当日起点示值
static WORD g_wGrpMonStartEnID[2] = {0x03BF, 0x03CF}; //当月起点示值

//测量点按抄表间隔固定更新当前数据
static const WORD g_wCurID[] = {0xa010, 0xa020, 0xa030, 0xa040};
static const WORD g_wPulseCurID[] = {0x2419, 0x241a, 0x241b, 0x241c};

static WORD g_wDayStartID[] = {0x003F, 0x004F, 0x005F, 0x006F}; //日起始
static WORD g_wMonStartID[] = {0x009F, 0x00AF, 0x00BF, 0x00CF}; //月起始

#define GRP_START_E		0	//总加组的起点电能
#define GRP_CUR_E		1	//总加组的当前电能
#define MTR_START_E		2	//测量点的起点电能

CDpGrp::CDpGrp()
{

}

CDpGrp::~CDpGrp()
{
	
}

//描述：总加组初始化
//@bPn	总加组号
bool CDpGrp::Init(WORD  wPn)
{
	BYTE bNum = 0;

	if (wPn==0 || wPn>GB_MAXSUMGROUP) 
		return false;

	m_wPn = wPn;

	DTRACE(DB_DP, ("Init*********: Grp = %d\r\n", m_wPn));

	GetCurTime(&m_tmLast);	

	m_bMtrNum = 0;   //测量点总数

	//测量点参数
	SetArrVal32(m_iCT, 1, PN_NUM);					//总加组的相关测量点的CT参数,缺省为1
	SetArrVal32(m_iPT, 1, PN_NUM);					//总加组的相关测量点的PT参数,缺省为1

	memset((BYTE*)m_GrpInfP, 0, sizeof(m_GrpInfP));		//总加有功功率的相关测量点信息
	memset((BYTE*)m_GrpInfQ, 0, sizeof(m_GrpInfQ));		//总加无功功率的相关测量点信息
	memset((BYTE*)m_GrpInfEp, 0, sizeof(m_GrpInfEp));	//总加有功电能的相关测量点信息
	memset((BYTE*)m_GrpInfEq, 0, sizeof(m_GrpInfEq));	//总加有功电能的相关测量点信息
	
	//需求参数
	memset((BYTE*)&m_biRepP, 0, sizeof(m_biRepP));			//总加有功功率
	memset((BYTE*)&m_biRepQ, 0, sizeof(m_biRepQ));			//总加无功功率
	memset((BYTE*)&m_biRepEp, 0, sizeof(m_biRepEp));		//总加有功电量（含费率）
	memset((BYTE*)&m_biRepEq, 0, sizeof(m_biRepEq));		//总加无功电量（含费率）
	
	//累计数据
	memset((BYTE*)m_iDayDeltaE, 0, sizeof(m_iDayDeltaE)); //当日总加电量差值量（对应有功，无功）
	memset((BYTE*)m_iMonDeltaE, 0, sizeof(m_iMonDeltaE)); //当月总加电量差值量（对应有功，无功）		  

	//缓存数据
	memset((BYTE*)m_gfsdDayStart, 0, sizeof(m_gfsdDayStart));	//测量点日起始电量（含有功、无功）
	memset((BYTE*)m_gfsdMonStart, 0, sizeof(m_gfsdMonStart));	//测量点月起始电量（含有功、无功）
	
	m_fNewDayStartEnFlg[0] = false; //总加组日起点示值是否采用当前示值的标志
	m_fNewDayStartEnFlg[1] = false; 
	m_fNewMonStartEnFlg[0] = false; //总加组月起点示值是否采用当前示值的标志
	m_fNewMonStartEnFlg[1] = false; 
	m_dwEnNewStartSec = 0; //总加组要读的新的当前起点值的时间
	memset((BYTE*)m_dwDayStartEnSec, 0, sizeof(m_dwDayStartEnSec)); //总加组日起点示值的更新时标(含有功、无功)
	memset((BYTE*)m_dwMonStartEnSec, 0, sizeof(m_dwMonStartEnSec)); //总加组月起点示值的更新时标(含有功、无功)

	memset((BYTE*)m_iOldDayMtrE, 0, sizeof(m_iOldDayMtrE));				//测量点日起始示值（含有功、无功）	
	memset((BYTE*)m_dwOldDayMtrUpdSec, 0, sizeof(m_dwOldDayMtrUpdSec));	//测量点日起始示值对应的时标（含有功、无功）
	memset((BYTE*)m_iOldMonMtrE, 0, sizeof(m_iOldMonMtrE));				//测量点月起始示值（含有功、无功）	
	memset((BYTE*)m_dwOldMonMtrUpdSec, 0, sizeof(m_dwOldMonMtrUpdSec));	//测量点月起始示值对应的时标（含有功、无功）

	DWORD dwSec = GetCurTime();
	for (int i=0; i<2; i++)
	{
		m_dwDayDeltaESec[i] = dwSec;
		m_dwMonDeltaESec[i] = dwSec;
	}

	if (!InitPara(bNum))
		return false;	//无有效总加参数

	m_bMtrIntv = GetMeterInterv(); //终端抄表间隔设置
	if (m_bPnProp == 0) //全为交采脉冲等测量点则按1分钟计算
		m_bMtrIntv = 1;

	m_bPwrSlideMin = m_bMtrIntv;

	LoadData();
	LoadPara();

	return true;
}

//描述：比较抄表间隔是否发生变化以更新提交参数
//		检测CTPT参数是否发生变化以及时更新
void  CDpGrp::LoadPara()
{
	BYTE bBuf[PNPARA_LEN];
	WORD wCt,wPt;
	WORD wLen;
	BYTE *pbFeildFmt;
	WORD wFeildLen;
	const ToaMap *pOadMap;
	BYTE bType, *pbBuf, *pbTmp;

	for (int i=0; i<m_bMtrNum; i++)
	{
		if (m_GrpInfP[i].bProp == PN_PROP_METER)
		{
			if (ReadItemEx(BN0, m_GrpInfP[i].wPn, 0x6000, bBuf) > 0)
			{
				pOadMap = GetOIMap(0x60000200);
				pbBuf = OoGetField(bBuf+1, pOadMap->pFmt, pOadMap->wFmtLen, 2, &wLen, &bType, &pbFeildFmt, &wFeildLen);
				if (pbBuf == NULL)
					continue;
				pbTmp = OoGetField(pbBuf, pbFeildFmt, wFeildLen, 2, &wLen, &bType);
				if (pbTmp == NULL)
					continue;
				wPt = OoLongUnsignedToWord(pbTmp+1);
				pbTmp = OoGetField(pbBuf, pbFeildFmt, wFeildLen, 3, &wLen, &bType);
				if (pbTmp == NULL)
					continue;
				wCt = OoLongUnsignedToWord(pbTmp+1);
				
				if (wCt != 0)
					m_iCT[i] = wCt; //CT变比
				if (wPt != 0)
					m_iPT[i] = wPt; //PT变比
				DTRACE(DB_DP, ("LoadPara: ct = %d pt = %d!\r\n", m_iCT[i], m_iPT[i]));
			}
		}
		else
		{
			if (ReadItemEx(BN0, m_GrpInfP[i].wPn, 0x2402, bBuf) > 0)
			{
				pOadMap = GetOIMap(0x24020300);
				pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, 0, &wLen, &bType);
				if (pbBuf == NULL)
					continue;
				wPt = OoLongUnsignedToWord(pbBuf+1);
				pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, 1, &wLen, &bType);
				if (pbBuf == NULL)
					continue;
				wCt = OoLongUnsignedToWord(pbBuf+1);

				if (wCt != 0)
					m_iCT[i] = wCt; //CT变比
				if (wPt != 0)
					m_iPT[i] = wPt; //PT变比
				DTRACE(DB_DP, ("LoadPara: ct = %d pt = %d!\r\n", m_iCT[i], m_iPT[i]));
			}
		}
	}

	ReadItemEx(BN0, m_wPn, 0x230c, bBuf);
	if (bBuf[1] > 0)
		m_bPwrSlideMin = bBuf[1];
}

//描述：初始化检查库内数据的合理性，并做相应的处理
void CDpGrp::LoadData()
{
	int i, j;
	TTime now;
	TIntvSec isNow;
	BYTE bBuf[64];

	int64 iGroupP;
	int64 iGrpE[BLOCK_ITEMNUM];	

	int64 iValBuf[BLOCK_ITEMNUM];
	int iValBuf2[5];

	GetCurTime(&now);

	DTRACE(DB_DP, ("LoadData*********: Grp = %d\r\n", m_wPn));

	//日累计值起点,有功，无功	
	TimeToIntvS(now, INTVDAY, &isNow);
	for (i=0; i<2; i++)
	{
		if (ReadItemEx(BN0, m_wPn, g_wGrpDayDeltaID[i], bBuf, isNow.dwS, isNow.dwEndS) > 0)//没读到
		{
			for (j=0; j<BLOCK_ITEMNUM;j++)
			{
				m_iDayDeltaE[i][j] = OoLong64ToInt64(&bBuf[3+9*i]);
			}
		}
		else
		{	
			ClsBlockE(BN0, g_wGrpDayDeltaID[i], &m_iDayDeltaE[i][0], INVALID_VAL64, isNow.dwS);				
		}

		if (ReadItemVal64(BN18, m_wPn, g_wGrpDayLeftDeltaID[i], iValBuf, isNow.dwS, isNow.dwEndS) <= 0)//没读到
		{	
			ClsBlockE(BN18, g_wGrpDayLeftDeltaID[i], iValBuf, 0, isNow.dwS);			
		}

		DTRACE(DB_DP, ("CDpGrp::LoadData Grp=%d wStartDeltaEnVal=%lld dwSec=%d \n",m_wPn,iValBuf[1], isNow.dwS));		
	}	

	//月累计值起点有功，无功		
	TimeToIntvS(now, INTVMON, &isNow);	
	for (i=0; i<2; i++)
	{
		if (ReadItemEx(BN0, m_wPn, g_wGrpMonDeltaID[i], bBuf, isNow.dwS, isNow.dwEndS) > 0)//没读到
		{
			for (j=0; j<BLOCK_ITEMNUM;j++)
			{
				m_iMonDeltaE[i][j] = OoLong64ToInt64(&bBuf[3+9*i]);
			}
		}
		else
		{		
			ClsBlockE(BN0, g_wGrpMonDeltaID[i], &m_iMonDeltaE[i][0], INVALID_VAL64, isNow.dwS);						
		}

		if (ReadItemVal64(BN18, m_wPn, g_wGrpMonLeftDeltaID[i], iValBuf, isNow.dwS, isNow.dwEndS) <= 0)//没读到
		{	
			ClsBlockE(BN18, g_wGrpMonLeftDeltaID[i], iValBuf, 0, isNow.dwS);			
		}	
	}	

	//当前总加功率，总加示值电量上电为无效，抄到数据立即更新
	iGroupP = INVALID_VAL64;
	bBuf[0] = DT_LONG64;
	OoInt64ToLong64(iGroupP, &bBuf[1]);
	TimeToIntvS(now, INTVCAL, &isNow);
	for (i=0; i<2; i++)
	{
		WriteItemEx(BN0, m_wPn, g_wGrpCurPowerID[i], bBuf, isNow.dwS);//当前总加功率
	}

	WriteItemEx(BN0, m_wPn, 0x0a07, bBuf, isNow.dwS); //当前总加有功电能，供控制使用	
	TrigerSaveBank(BN18, 0, -1); //功率统计起点值触发保存.
}


//描述：获取总加组参数及需提交的测量点的需求信息
//@bReqNum	返回本总加组需求ID的个数，所有类型的测量点均提交
//返回值：	若成功则为true
bool CDpGrp::InitPara(BYTE& bReqNum)
{	
	int i=0,j=0;
	WORD wLen;
	BYTE *pbFeildFmt;
	WORD wMtrPn, wFeildLen;
	const ToaMap *pOadMap;
	BYTE bType, *pbBuf, *pbTmp;
	BYTE bBuf[GRPPARA_LEN];
	BYTE bPnType,bIDFlag,bOp;
	
	if (ReadItemEx(BN0, m_wPn, 0x2301, bBuf) > 0)
	{		
		m_bMtrNum = bBuf[1];
		pOadMap = GetOIMap(0x23010200);
		for (i=0; i<m_bMtrNum; i++)
		{
			pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType, &pbFeildFmt, &wFeildLen);
			if (pbBuf == NULL)
				continue;
			wMtrPn = MtrAddrToPn(pbBuf+3, pbBuf[3]+1);//测量点号
			if (wMtrPn > 0)
			{			
				bPnType = PN_PROP_METER; //测量点类型
			}
			else if ((wMtrPn=PulseAddrToPn(pbBuf+3, pbBuf[3]+1)) > 0)
			{
				bPnType = PN_PROP_PULSE;
				wMtrPn -= 1; //内部脉冲测量点号从0开始
			}
			else
				continue;

			pbTmp = OoGetField(pbBuf, pbFeildFmt, wFeildLen, 1, &wLen, &bType);
			if (pbTmp == NULL)
				continue;
			bIDFlag = pbTmp[1]&0x01;
			pbTmp = OoGetField(pbBuf, pbFeildFmt, wFeildLen, 2, &wLen, &bType);
			if (pbTmp == NULL)
				continue;
			bOp = pbTmp[1]&0x01;

			m_GrpInfP[i].wPn	= wMtrPn; //测量点号
			m_GrpInfP[i].bProp	= bPnType; //测量点类型
			if (bPnType == PN_PROP_METER)
				m_GrpInfP[i].wCurId = 0xa052; //ID号
			else
				m_GrpInfP[i].wCurId = 0x2404; //ID号
			m_GrpInfP[i].bOp	= bOp; //运算符
			m_GrpInfP[i].bDir	= bIDFlag;

			m_GrpInfQ[i].wPn	= wMtrPn; //测量点号
			m_GrpInfQ[i].bProp	= bPnType; //测量点类型
			if (bPnType == PN_PROP_METER)
				m_GrpInfQ[i].wCurId = 0xa053; //ID号
			else
				m_GrpInfQ[i].wCurId = 0x2405; //ID号
			m_GrpInfQ[i].bOp	= bOp; //运算符
			m_GrpInfQ[i].bDir	= bIDFlag;

			m_GrpInfEp[i].wPn	= wMtrPn;	   //测量点号
			m_GrpInfEp[i].bProp = bPnType; //测量点类型
			m_GrpInfEp[i].bDir	= bIDFlag;
			m_GrpInfEq[i].wPn	= wMtrPn;	   //测量点号			
			m_GrpInfEq[i].bProp = bPnType; //测量点类型			
			m_GrpInfEq[i].bDir	= bIDFlag;
		
			m_GrpInfEp[i].bOp = bOp; //运算符			
			m_GrpInfEq[i].bOp = bOp; //运算符

			if (bIDFlag == 0)//正向
			{
				if (bPnType == PN_PROP_METER)
					m_GrpInfEp[i].wCurId = 0xa010;
				else
					m_GrpInfEp[i].wCurId = 0x2419;

				m_GrpInfEp[i].wDayStartId = 0x003F;
				m_GrpInfEp[i].wMonStartId  = 0x009F;

				if (bPnType == PN_PROP_METER)
					m_GrpInfEq[i].wCurId = 0xa030;
				else
					m_GrpInfEq[i].wCurId = 0x241a;

				m_GrpInfEq[i].wDayStartId = 0x004F;
				m_GrpInfEq[i].wMonStartId = 0x00AF;
				
			}
			else//反向
			{
				if (bPnType == PN_PROP_METER)
					m_GrpInfEp[i].wCurId = 0xa020;
				else
					m_GrpInfEp[i].wCurId = 0x241b;

				m_GrpInfEp[i].wDayStartId = 0x005F;
				m_GrpInfEp[i].wMonStartId  = 0x00BF;

				if (bPnType == PN_PROP_METER)
					m_GrpInfEq[i].wCurId = 0xa040;
				else
					m_GrpInfEq[i].wCurId = 0x241c;

				m_GrpInfEq[i].wDayStartId = 0x006F;
				m_GrpInfEq[i].wMonStartId = 0x00CF;
			}		

			if (bPnType == PN_PROP_METER)//只要有电表就按抄表间隔
			{
				m_bPnProp = 1;
			}
			
			m_biRepP[j].wPn = wMtrPn; //测量点号
			if (bPnType == PN_PROP_METER)
				m_biRepP[j].wID = 0xa052;
			else
				m_biRepP[j].wID = 0x2404;

			m_biRepP[j].wBn = BN0;

			m_biRepQ[j].wPn = wMtrPn; //测量点号
			if (bPnType == PN_PROP_METER)
				m_biRepQ[j].wID = 0xa053;
			else
				m_biRepQ[j].wID = 0x2405;

			m_biRepQ[j].wBn = BN0;

			m_biRepEp[j].wPn = wMtrPn; //测量点号
			m_biRepEq[j].wPn = wMtrPn; //测量点号
			m_biRepEp[j].wBn = BN0;
			m_biRepEq[j].wBn = BN0;				
			m_biRepEp[j].wID = m_GrpInfEp[i].wCurId;
			m_biRepEq[j].wID =  m_GrpInfEq[i].wCurId;	

			m_gfsdDayStart[0].biRepItem[i].wBn = BN18;
			m_gfsdDayStart[0].biRepItem[i].wPn = wMtrPn;
			m_gfsdDayStart[0].biRepItem[i].wID = m_GrpInfEp[i].wDayStartId;

			m_gfsdDayStart[1].biRepItem[i].wBn = BN18;
			m_gfsdDayStart[1].biRepItem[i].wPn = wMtrPn;
			m_gfsdDayStart[1].biRepItem[i].wID = m_GrpInfEq[i].wDayStartId;

			m_gfsdMonStart[0].biRepItem[i].wBn = BN18;
			m_gfsdMonStart[0].biRepItem[i].wPn = wMtrPn;
			m_gfsdMonStart[0].biRepItem[i].wID = m_GrpInfEp[i].wMonStartId;

			m_gfsdMonStart[1].biRepItem[i].wBn = BN18;
			m_gfsdMonStart[1].biRepItem[i].wPn = wMtrPn;
			m_gfsdMonStart[1].biRepItem[i].wID = m_GrpInfEq[i].wMonStartId;			
			j ++; //多少个电表测量点就有多少个ID				
		}
		bReqNum = m_bMtrNum; //= j;		

		return true;
	}

	return false;
}


//描述：获取电能的顺序索引（有功、无功）
int CDpGrp::GetIdInxE(WORD wCurID)
{
	int iRet = -1;

	for (int i=0; i<2; i++)
	{
		if (wCurID==0xa010 || wCurID==0xa030 || wCurID==0x2419 || wCurID==0x241b)
		{
			iRet = 0;
		}
		else if (wCurID==0xa020 || wCurID==0xa040 || wCurID==0x241a || wCurID==0x241c)
		{
			iRet = 1;
		}			
	}

	return iRet;
}


//描述：比较抄表间隔是否发生变化，以更新提交参数
bool  CDpGrp::IsNeedReset()
{
	BYTE bMtrIntv = GetMeterInterv(); //终端抄表间隔设置
	if (m_bPnProp == 0) //全为交采脉冲等测量点则按1分钟计算
		bMtrIntv = 1;

	if (bMtrIntv != m_bMtrIntv)
	{			
		DTRACE(DB_DP, ("CDpGrp::IsNeedReset() GPN =%d!\n", m_wPn));
		return true;
	}
		
	return false;
}

void CDpGrp::UpdDayMonStartVal(BYTE bInterv, TTime tmNow)
{
	BYTE n;
	WORD wNum;
	TIntvSec isNow,isNew;//当前时间
	TIntvSec isOldStart;//原日的起点值时间	
	TIntvSec isOldDelt;	//原日的累计值时间		
	TIntvSec isNewDelt;	//新日的累计值时间
	TIntvSec isLastDay;	//上日电量的时间
	DWORD dwSec = 0;
	TTime tm = tmNow;
	TGrpInf* pGrpInf;
	WORD wDeltaID, wStartEnId, wCurID;
	int64 *piDeltaVal;
	TGrpFrzStartData*   pStartData;
	int64 iGrpE[BLOCK_ITEMNUM];	
	int64 iCurVal[BLOCK_ITEMNUM];
	BYTE bDataBuf[30];
	DWORD *pdwStartSec;
	bool *pfNewStartFlg;

	int64 iNewMtrE[BLOCK_ITEMNUM];
	int64 *piMtrE;
	DWORD *pdwMtrUpdSec;

	SetArrVal64(iGrpE, INVALID_VAL64, BLOCK_ITEMNUM);
	SetArrVal64(iNewMtrE, INVALID_VAL64, BLOCK_ITEMNUM);
	for (n=0; n<2; n++)	//有功、无功
	{
		TimeToIntvS(tmNow, bInterv, &isNow); //取当前日月间隔的起始时间

		if (n == 0)
			pGrpInf = m_GrpInfEp;
		else
			pGrpInf = m_GrpInfEq;

		if (bInterv == INTVDAY)
		{
			piDeltaVal = &m_iDayDeltaE[n][0];			//当日电量差值量
			pStartData	= &m_gfsdDayStart[n];			//测量点日起始量
			wDeltaID	= g_wGrpDayDeltaID[n];			//当日总加电能量
			wStartEnId = g_wGrpDayStartEnID[n];			//当日起点示值
			pdwStartSec = &m_dwDayStartEnSec[0];		//总加组日起点示值的更新时标(含有功、无功)
			pfNewStartFlg = &m_fNewDayStartEnFlg[n];	//总加组日起点示值（含有功、无功）是否采用当前示值的标志

			piMtrE = &m_iOldDayMtrE[n][0];				//测量点日起始示值（含有功、无功，不乘CTPT,用于检测该总加组的测量点的起始值是否有变化）
			pdwMtrUpdSec = &m_dwOldDayMtrUpdSec[n];		//测量点日起始示值对应的时标（含有功、无功）
		}
		else if (bInterv == INTVMON)		
		{
			piDeltaVal = &m_iMonDeltaE[n][0];
			pStartData	= &m_gfsdMonStart[n];	
			wDeltaID	= g_wGrpMonDeltaID[n];
			wStartEnId = g_wGrpMonStartEnID[n];
			pdwStartSec = &m_dwMonStartEnSec[0];
			pfNewStartFlg = &m_fNewMonStartEnFlg[n];

			piMtrE = &m_iOldMonMtrE[n][0];
			pdwMtrUpdSec = &m_dwOldMonMtrUpdSec[n];
		}
		else 
			return;

		//更新测量点日月起始值
		for (int i=0; i<m_bMtrNum; i++)
		{
			if (ReadItemVal64(pStartData->biRepItem[i].wBn, pStartData->biRepItem[i].wPn, pStartData->biRepItem[i].wID, &iCurVal[0], isNow.dwS, isNow.dwEndS) <= 0)
			{
				if (bInterv == INTVDAY)
				{
					for (int j=0; j<sizeof(g_wDayStartID)/sizeof(WORD); j++)
					{	
						if (g_wDayStartID[j] == pStartData->biRepItem[i].wID)
						{
							memset(bDataBuf, 0, sizeof(bDataBuf));

							if (m_GrpInfP[i].bProp == PN_PROP_METER)
								wCurID = g_wCurID[j];
							else
								wCurID = g_wPulseCurID[j];
									
							//更新日起点 有效无效均更新
							if (ReadItemEx(BN0, pStartData->biRepItem[i].wPn, wCurID, bDataBuf, isNow.dwS, isNow.dwEndS) > 0)
							{
								for (BYTE k=0; k<BLOCK_ITEMNUM; k++)
									iCurVal[k] = OoDoubleLongToInt(&bDataBuf[3+5*k]);
								WriteItemVal64(BN18, pStartData->biRepItem[i].wPn, g_wDayStartID[j], iCurVal, isNow.dwS);
								DTRACE(DB_DP, ("CDpMtr::UpdDayMonStartVal():PN=%d,g_wDayStartID=%x StartVal=%lld\n ",pStartData->biRepItem[i].wPn, g_wDayStartID[j], iCurVal[0]));
							}

							break;
						}
					}
				}
				else
				{
					for (int j=0; j<sizeof(g_wMonStartID)/sizeof(WORD); j++)
					{	
						if (g_wMonStartID[j] == pStartData->biRepItem[i].wID)
						{
							memset(bDataBuf, 0, sizeof(bDataBuf));

							if (m_GrpInfP[i].bProp == PN_PROP_METER)
								wCurID = g_wCurID[j];
							else
								wCurID = g_wPulseCurID[j];

							//更新日起点 有效无效均更新
							if (ReadItemEx(BN0, pStartData->biRepItem[i].wPn, wCurID, bDataBuf, isNow.dwS, isNow.dwEndS) > 0)
							{
								for (BYTE k=0; k<BLOCK_ITEMNUM; k++)
									iCurVal[k] = OoDoubleLongToInt(&bDataBuf[3+5*k]);
								WriteItemVal64(BN18, pStartData->biRepItem[i].wPn, g_wMonStartID[j], iCurVal, isNow.dwS);
								DTRACE(DB_DP, ("CDpMtr::UpdDayMonStartVal():PN=%d,g_wMonStartID=%x StartVal=%lld\n ",pStartData->biRepItem[i].wPn, g_wMonStartID[j], iCurVal[0]));	
							}

							break;
						}
					}
				}
			}
		}

		wNum = 0;
		if (QueryItemTime(isNow.dwS, isNow.dwEndS, pStartData->biRepItem, m_bMtrNum, &wNum) == m_bMtrNum)//查询电能本周期被更新过
		{
			if (m_bPnProp==1 && pStartData->dwUpdSec!=isNow.dwS && pStartData->dwUpdSec!=0) //起点发生变更时，要即时计算上一差值电量并转存
			{
				//无论是非准点切换日月冻结方式还是准点切换日月冻结方式，对总加组来说查到新起点时都是新间隔时间了					
				if (bInterv == INTVDAY)
				{
					dwSec = 24*60*60;
				}
				else if (bInterv == INTVMON)
				{
					if (tm.nMonth == 1)		
					{
						tm.nYear --;
						tm.nMonth = 12;		
					}
					else
						tm.nMonth --; 
					dwSec = DaysOfMonth(tm)*24*60*60;	//取上一月的起点(本月起始 － 上月时间)	
				}
				
 				isOldStart.dwS = isNow.dwS-dwSec;  //原日的起点(本日起始 － 日时间)
				isOldStart.dwEndS = isNow.dwEndS-dwSec; 
				isOldDelt.dwS = isNow.dwS-m_bMtrIntv*60; //原日的累计值时标写成最后一笔(本日起始 － 间隔时间)
				isOldDelt.dwEndS = isNow.dwS; 					
				isNewDelt.dwS = isNow.dwS;			  //新日的累计值时间
				isNewDelt.dwEndS = isNow.dwEndS;
				isLastDay.dwS = isNow.dwS;			  //上日电量的时间
				isLastDay.dwEndS = isNow.dwEndS;
					
				DeltaENew(wDeltaID, pGrpInf, bInterv, isNow, isOldStart, isOldDelt);
				DTRACE(DB_DP, ("CDpGrp::UpdDayMonStartVal GPN=%d deltDayVal=%lld dwSec=%d dwNewSec=%d!\n",m_wPn, m_iDayDeltaE[n][0], isOldDelt.dwS, isNow.dwS));		
				UpdClsFrzData(n, bInterv, isLastDay.dwS, isNewDelt.dwS); 	//冻结电量数据转存,写新日时标		
			}
			for (int i=0; i<m_bMtrNum; i++)
			{
				ReadItemVal64(pStartData->biRepItem[i].wBn, pStartData->biRepItem[i].wPn, pStartData->biRepItem[i].wID, &pStartData->iStartVal[i][0], isNow.dwS, isNow.dwEndS);					
			}

			//起点有更新，此处为正常的日月切换更新，需同步更新总加组起点示值
			int iRet = ReadItemVal64(BN18, m_wPn, wStartEnId, iGrpE, isNow.dwS, isNow.dwEndS);
			if (pdwStartSec[n]!=isNow.dwS || iRet<=0 || (iRet>0 && IsAllAVal64(&iGrpE[0], INVALID_VAL64, TOTAL_RATE_NUM))) 
			{
				if ( *pfNewStartFlg ) //需要更新起点
				{
					isNew = isNow;			
					isNew.dwS = m_dwEnNewStartSec; //必须读取新抄到的数据作为起点才行
					CalcuEnSum(iGrpE, n, bInterv, isNew, GRP_CUR_E);	//计算当前示值做总加组示值起点						
				}
				else
				{
					if (iRet<=0 || (iRet>0 && IsAllAVal64(&iGrpE[0], INVALID_VAL64, TOTAL_RATE_NUM)))
						CalcuEnSum(iGrpE, n, bInterv, isNow, GRP_START_E);		//计算原起点示值（可能同测量点示值）做总加组示值起点	
					//else  为更新过的有效起点，可直接启用
				}
				
				WriteItemVal64(BN18, m_wPn, wStartEnId, iGrpE, isNow.dwS); //写入起点示值		 
				//DTRACE(DB_DP, ("CDpGrp::UpdDayMonStartVal Grp=%d WID=0X%2x, wStartCurEnVal=%lld dwSec=%d \n",m_wPn, wStartEnId, iGrpE[1], isNow.dwS));

				//若更新成功
				if ( !IsAllAVal64(&iGrpE[0], INVALID_VAL64, TOTAL_RATE_NUM) ) 
				{
					TrigerSaveBank(BN18, 0, -1); //起点示值	触发保存.
					pdwStartSec[n] = isNow.dwS;
					*pfNewStartFlg = false;

					CalcuEnSum(iNewMtrE, n, bInterv, isNow, MTR_START_E); 
					memcpy((BYTE*)piMtrE, (BYTE*)&iNewMtrE, sizeof(iNewMtrE));
					*pdwMtrUpdSec = isNow.dwS;
				}
			}	
			else //当起点有效时，检测是否有电表的自然示度下降
			{					
				//总加组通过检测所记录的测量点的起点示值是否有更新，来检测若总加组的起点示值与电表的起点示值不同步时（比如CTPT变更），
				//之后电表测量点的是否有异常变化（比如自然的示度下降，而所算总加组示值又未降到总加组的起点示值以下）
				if (*pdwMtrUpdSec==pStartData->dwUpdSec && *pdwMtrUpdSec!=0)
				{
					CalcuEnSum(iNewMtrE, n, bInterv, isNow, MTR_START_E); 
					for (BYTE i=0; i<BLOCK_ITEMNUM; i++) //总及分费率的循环
					{
						if (iNewMtrE[i] != piMtrE[i])	//测量点当前起点算出的值与之前的起点算出的值不同，则表明测量点有更新
						{
							SetStartDeltaE(bInterv, wDeltaID, tmNow);	
							CalcuEnSum(iGrpE, n, bInterv, isNow, GRP_CUR_E);	//计算当前示值做总加组示值起点	
							WriteItemVal64(BN18, m_wPn, wStartEnId, iGrpE, isNow.dwS);	//写入起点示值

							memcpy((BYTE*)piMtrE, (BYTE*)&iNewMtrE, sizeof(iNewMtrE));	//更新记录的测量点起点，时标不用更新
							DTRACE(DB_DP, ("CDpGrp::UpdDayMonStartVal222 Grp=%d WID=0X%2x, wStartCurEnVal=%lld dwSec=%d \n",m_wPn, wStartEnId, iGrpE[1], isNow.dwS));
							break;
						}
					}
				}
			}

			pStartData->dwUpdSec = isNow.dwS;
		}	
	}
}

//描述：计算某时刻的总加组电能量的差值
//		计算方式为计算每个总加组的示值与总加组的起点示值相减，再加上累计值起点
//@wResultID 返回的运算结果对应的ID
//@pGrpInf	 本总加组对应的测量点信息结构数组
//@bType	 差值的几种类型：2表示日差值 3表示月差值
//@isNow	 表示示值的时间
//@isStart	 表示日月起点的时间
//@isDelta	 表示累计值的时间
void CDpGrp::DeltaENew(WORD wResultID, TGrpInf* pGrpInf, BYTE bInterv, TIntvSec isNow, TIntvSec isStart, TIntvSec isDelta)
{
	int64 iCurVal[BLOCK_ITEMNUM];	//总加组的总及分费率当前示值	
	int64 iStartVal[BLOCK_ITEMNUM];	//总加组的总及分费率起点示值	
	int64 iLeftDeltaE[BLOCK_ITEMNUM];	//总加组的总及分费率在参数配置前剩下的电能delta值	
	int64 iGrpE[BLOCK_ITEMNUM];		//总加组的总及分费率示值
	bool fUpd = false;
	bool fAdd = false;
	int64 *piResVal;
	DWORD *pdwTime, *pwStartTime;	
	WORD *pwStartId, *pwLeftDeltaId, *pwDeltaId;
	int iRv = -1;

	SetArrVal64(iCurVal, INVALID_VAL64, BLOCK_ITEMNUM);
	SetArrVal64(iStartVal, INVALID_VAL64, BLOCK_ITEMNUM);
	SetArrVal64(iLeftDeltaE, INVALID_VAL64, BLOCK_ITEMNUM);
	SetArrVal64(iGrpE, 0, BLOCK_ITEMNUM); //计算累计的量		

	int n = GetIdInxE(pGrpInf->wCurId);	
	if (n < 0)
		return;
		
	//电量计算
	//累计值
	if (bInterv == INTVDAY) //日累计
	{	
		piResVal = &m_iDayDeltaE[n][0];				//当日总加电量差值量（对应有功、无功）
		pdwTime = &m_dwDayDeltaESec[n];				//当日电量的时标
		pwStartId = &g_wGrpDayStartEnID[n];			//总加组日起点示值
		pwStartTime =  &m_dwDayStartEnSec[n];		//总加组日起点示值的更新时标(含有功、无功)
		pwLeftDeltaId = &g_wGrpDayLeftDeltaID[n];	//参数配置前剩下的日delta电量
		pwDeltaId = &g_wGrpDayDeltaID[n];			//当日累计

	}		
	else if (bInterv == INTVMON) //月累计
	{		
		piResVal = &m_iMonDeltaE[n][0];		
		pdwTime = &m_dwMonDeltaESec[n];
		pwStartId = &g_wGrpMonStartEnID[n];
		pwStartTime =  &m_dwMonStartEnSec[n];
		pwLeftDeltaId = &g_wGrpMonLeftDeltaID[n];
		pwDeltaId = &g_wGrpMonDeltaID[n];
	}	

	//计算当前示值，格式为测量点示值格式
	CalcuEnSum(iCurVal, n, INTVCAL, isNow, GRP_CUR_E);	
	if ( IsAllAVal64(&iCurVal[0], INVALID_VAL64, TOTAL_RATE_NUM) )
		return; //当前值无效则不更新累计值数据	

	//起点时标不对则返回
	if (ReadItemVal64(BN18, m_wPn, *pwStartId, iStartVal, isStart.dwS, isStart.dwEndS) <= 0)
		return; //起点时标不对则返回
	else if ( IsAllAVal64(&iStartVal[0], INVALID_VAL64, TOTAL_RATE_NUM) )
		return; //起点为无效数据则不更新
	else if ((iRv=IsMtrEngDec(n, bInterv, isNow)) > 0)
	{
		//示度下降或异常情况,需要重新设置当前值为起点，当前累计数据不更新
		TTime now;
		SecondsToTime(isNow.dwS, &now);							
		CalcuEnSum(iGrpE, n, bInterv, isNow, GRP_CUR_E);	//计算当前示值做总加组示值起点	
		if ( memcmp((BYTE*)&iStartVal[0], (BYTE*)&iGrpE[0], sizeof(int64)*TOTAL_RATE_NUM) !=0 ) //避免反复操作
		{
			SetStartDeltaE(bInterv, *pwDeltaId, now);	
			WriteItemVal64(BN18, m_wPn, *pwStartId, iGrpE, isStart.dwS); //写入起点示值		 
			*pwStartTime = isStart.dwS;						//更新记录时标		
			DTRACE(DB_DP, ("CDpGrp::DeltaENew***** sub is error: Grp=%d, WID=0X%2x, get new iStartVal=%lld, dwSec=%d,\r\n", m_wPn, *pwStartId, iGrpE[0], isNow.dwS));

			TrigerSaveBank(BN18, 0, -1); //即时保存的起点数据
		}
		return;
	}
	//else if (iRv < 0) //测量点上一周期示值的镜像读不到，意味着刚上电或参数变更，可能漏判示度下降或误判变更的起点	
	//{
	//	DTRACE(DB_DP, ("CDpGrp::DeltaENew####1, Grp=%d wResultID=0X%2x mPn iLastCurVal is not arrive!,\r\n", m_wPn, wResultID));
	//	return;
	//}
	else	
		fUpd = true;	
	
	BYTE bAddDeltaE = 0xff;
	ReadItemEx(BN10, 0, 0xa143, &bAddDeltaE);
	//参数设置需要加上原有累计值，且累计值起点有效则累加;
	if (bAddDeltaE==0 && ReadItemVal64(BN18, m_wPn, *pwLeftDeltaId, iLeftDeltaE, isStart.dwS, isStart.dwEndS) > 0)
	{
		fAdd = true;
	}

	int iRet = 0;
	if (fUpd)
	{
		//计算除费率数之外的各项的电能（含总及4个费率）
		for (BYTE i=0; i<BLOCK_ITEMNUM; i++) //总及分费率的循环
		{		
			//考虑到费率不齐的时候						
			if (iGrpE[i] == INVALID_VAL64)
				continue;

			//计算delta值,格式为测量点示值格式
			iRet = DataDelta(&iGrpE[i], iCurVal[i], iStartVal[i]);
			if (iRet == 0) //有无效数据
			{							  //当前值 - 起始值
				iGrpE[i] = INVALID_VAL64;
				continue;
			}	
			else if (iRet < 0) 	//总加组支持小值－大值		
			{		
				//考虑到费率不齐的时候
				if (iCurVal[i]==INVALID_VAL64 || iStartVal[i]==INVALID_VAL64)
				{
					iGrpE[i] = INVALID_VAL64;
					continue;
				}
				else
					iGrpE[i] = iCurVal[i]-iStartVal[i];				
			}			

			//加上原始累计值
			if ( fAdd && iLeftDeltaE[i] != INVALID_VAL64)		
			{
				DataSum(&iGrpE[i], iGrpE[i], iLeftDeltaE[i], 0);
			}

			if (iGrpE[i] < 0)
				DTRACE(DB_DP, ("CDpGrp::DeltaENew####2 sub is error: Grp=%d, WID=0X%2x, iGrpE[i]=%lld, iCurVal[i]=%lld, iStartVal[i]=%lld, iLeftDeltaE[i]=%lld, dwSec=%d,\r\n", m_wPn, wResultID, iGrpE[i],iCurVal[i],iStartVal[i],iLeftDeltaE[i],isNow.dwS));
		}			
	}

	//只要有一个数据无效则所有数据无效	
	memcpy((BYTE*)piResVal, (BYTE*)iGrpE, sizeof(iGrpE));
	if (wResultID>=0x2306 && wResultID<=0x2309)
	{
		BYTE bBuf[64];
		bBuf[0] = DT_ARRAY;
		bBuf[1] = RATE_NUM+1;
		for (int i=0; i<BLOCK_ITEMNUM;i++)
		{
			bBuf[2+9*i] = DT_LONG64;
			OoInt64ToLong64(iGrpE[i], &bBuf[3+9*i]);
		}
		WriteItemEx(BN0, m_wPn, wResultID, bBuf, isDelta.dwS);
	}
	else
		WriteItemVal64(BN0, m_wPn, wResultID, iGrpE, isDelta.dwS);	
	*pdwTime = isDelta.dwS;	
}

//描述：更新示值数据，格式同测量点数据的格式
//		只要有一个测量点无效则总加示值为无效数据
//@pVal64	返回示值，更新不成功则返回无效数据
//@n		
//@bType	
//@fFlg	 =0 表示更新总加组起点的示值 =1 表示更新总加组当前的示值
void CDpGrp::CalcuEnSum(int64* pVal64, BYTE n, BYTE bInterv, TIntvSec isNow, BYTE bEnType)
{
	int i,j;
	BYTE bBuf[64];
	int64 iTmpBuf[PN_NUM][BLOCK_ITEMNUM];	
	
	int64 iGrpE[BLOCK_ITEMNUM];	
	
	TGrpInf*   pGrpInf;
	TGrpFrzStartData* pStartData;	
	
	SetArrVal64(iGrpE, 0, BLOCK_ITEMNUM);

	if (n == 0)						
		pGrpInf = m_GrpInfEp;			
	else				
		pGrpInf = m_GrpInfEq;	

	if (bEnType == GRP_CUR_E) //总加组当前值的示值
	{		
		//所有测量点的数据均更新过（含抄表成功、抄表失败以及不支持的数据）	
		for (i=0; i<m_bMtrNum; i++)
		{		
			if (ReadItemEx(BN0, pGrpInf[i].wPn, pGrpInf[i].wCurId, bBuf, isNow.dwS, isNow.dwEndS) > 0 )//有效数据
			{
				//若数据有效则进行运算
				for (j=0; j<BLOCK_ITEMNUM; j++) //计算除费率数之外的各项的电能（含总及4个费率）
				{
					iTmpBuf[i][j] = OoDoubleLongToInt(&bBuf[3+j*5]);
					if (iTmpBuf[i][j] != INVALID_VAL64)
					{
						if (pGrpInf[i].bProp == PN_PROP_METER)
							iTmpBuf[i][j] *=  (m_iCT[i]*m_iPT[i]*100);
						else
							iTmpBuf[i][j] *=  (m_iCT[i]*m_iPT[i]);
						DataSum(&iGrpE[j], iGrpE[j], iTmpBuf[i][j], pGrpInf[i].bOp);							
					}
				}			
			}	
			else //若数据全部无效（含抄表失败或不支持的数据）则立即更新为无效数据
			{
				SetArrVal64(&iGrpE[0], INVALID_VAL64, TOTAL_RATE_NUM);
				break;
			}
		}
	}		
	else //起点的示值
	{
		if (bInterv == INTVDAY)
			pStartData = &m_gfsdDayStart[n];
		else if (bInterv == INTVMON)
			pStartData = &m_gfsdMonStart[n];

		for (i=0; i<m_bMtrNum; i++)
		{		
			//若数据有效则进行运算
			if ( !IsAllAVal64(&pStartData->iStartVal[i][0], INVALID_VAL64, TOTAL_RATE_NUM) )
			{
				memcpy((BYTE*)iTmpBuf, (BYTE*)pStartData->iStartVal, sizeof(iTmpBuf));

				for (j=0; j<BLOCK_ITEMNUM; j++) //计算除费率数之外的各项的电能（含总及4个费率）
				{
					if (iTmpBuf[i][j] != INVALID_VAL64)
					{
						if (bEnType == GRP_START_E)//总加组起点的示值
						{
							if (pGrpInf[i].bProp == PN_PROP_METER)
								iTmpBuf[i][j] *=  (m_iCT[i]*m_iPT[i]*100);
							else
								iTmpBuf[i][j] *=  (m_iCT[i]*m_iPT[i]);
						}
						else //if (bEnType == MTR_START_E)
						{				
							//iTmpBuf[i][j] *= 1;
							//该总加组的测量点的起点示值（不乘CTPT,用于检测该总加组的测量点的起点是否更新）
							//总加组通过检测所记录的测量点的起点示值是否有更新，来检测若总加组的起点示值与电表的起点示值不同步时（比如CTPT变更），
							//之后电表测量点的是否有异常变化（比如自然的示度下降，而所算总加组示值又未降到总加组的起点示值以下）
						}

						DataSum(&iGrpE[j], iGrpE[j], iTmpBuf[i][j], pGrpInf[i].bOp);							
					}
				}			
			}			
			else //若数据全部无效（含抄表失败或不支持的数据）则立即更新为无效数据
			{
				SetArrVal64(&iGrpE[0], INVALID_VAL64, TOTAL_RATE_NUM);
				break;
			}
		}
	}
	memcpy((BYTE*)pVal64, (BYTE*)iGrpE, sizeof(iGrpE));
}


//描述：在正常运行时（非测量点变更时）判断总加组的测量点当前是否示度下降
int CDpGrp::IsMtrEngDec(BYTE n, BYTE bInterv, TIntvSec isNow)
{
	int i,j;
	BYTE bCurBuf[64], bLastBuf[64];
	int64 iCurVal[BLOCK_ITEMNUM];
	int64 iLastCurVal[BLOCK_ITEMNUM];
	int iLen1, iLen3;		
	
	TGrpInf*   pGrpInf;

	if (n == 0)						
		pGrpInf = m_GrpInfEp;			
	else				
		pGrpInf = m_GrpInfEq;	
	
	//所有测量点的数据均更新过（含抄表成功、抄表失败以及不支持的数据）	
	for (i=0; i<m_bMtrNum; i++)
	{			
		SetArrVal64(iCurVal, INVALID_VAL64, BLOCK_ITEMNUM);
		SetArrVal64(iLastCurVal, INVALID_VAL64, BLOCK_ITEMNUM);

		iLen3 = ReadItemEx(BN0, pGrpInf[i].wPn, pGrpInf[i].wCurId+0x0100, bLastBuf, isNow.dwS-m_bMtrIntv*60, isNow.dwEndS-m_bMtrIntv*60);	//取前一抄表间隔的值
		if (iLen3 > 0)
		{
			for (j=0; j<BLOCK_ITEMNUM; j++) //计算除费率数之外的各项的电能（含总及4个费率）
			{
				iLastCurVal[j] = OoDoubleLongToInt(&bLastBuf[3+j*5]);
			}
		}

		iLen1 = ReadItemEx(BN0, pGrpInf[i].wPn, pGrpInf[i].wCurId, bCurBuf, isNow.dwS, isNow.dwEndS);	  //取当前抄表间隔的值
		if (iLen1 > 0)
		{
			for (j=0; j<BLOCK_ITEMNUM; j++) //计算除费率数之外的各项的电能（含总及4个费率）
			{
				iCurVal[j] = OoDoubleLongToInt(&bCurBuf[3+j*5]);
			}
		}

		if (iLen1<=0 || IsAllAVal64(&iCurVal[0], INVALID_VAL64, TOTAL_RATE_NUM)) 
			return 0; //当前值无效则不判断		
		if (iLen3<=0 || IsAllAVal64(&iLastCurVal[0], INVALID_VAL64, TOTAL_RATE_NUM)) 
			return -1; //上一次值无效则不判断	

		//若数据有效则进行判断
		for (j=0; j<BLOCK_ITEMNUM; j++) //计算除费率数之外的各项的电能（含总及4个费率）
		{
			if ( iCurVal[j] < iLastCurVal[j])  //只要有某个测量点某费率的示度下降，则为示度下降		
			{
				//DTRACE(DB_DP, ("CDpGrp::IsMtrEngDec***** sub is error: Grp=%d, Mpn=%d,  WID=0X%2x, iCurVal=%lld, iLastCurVal=%lld, iResVal not updated dwSec=%d,\r\n", m_wPn, pGrpInf[i].bPn, pGrpInf[i].wCurId, iCurVal[j], iLastCurVal[j], isNow.dwS));
				return 1;	
			}
		}				
	}

	return 0;	
}

//描述：按抄表间隔提交总加组当前刷新数据的需求
//		计算当前总加有、无功功率以及有、无功电能量,以及当日当月累计数据
//返回值：
void CDpGrp::CalcuCurData(TTime tmNow)
{
	int i,n;
	TIntvSec isNow;	
	WORD wNum;
	BYTE bBuf[64];
	int64 iTmpBuf[PN_NUM][BLOCK_ITEMNUM];
	
	int64 iGroupP;
	int64 iGrpE[BLOCK_ITEMNUM];	

	TBankItem* pItem;
	TGrpInf*   pGrpInf;
	
	//TODO:使用CMeterSched::GetCurIntervS()来取当前间隔的起始时间
	TimeToIntvS(tmNow, INTVCAL, &isNow); //归为当前抄表间隔的起始秒数
			//如果同时存在电表和脉冲交采测量点,
			//则起始时间取的是抄表间隔的起始时间,但是实际上脉冲交采测量点
			//每次取的还是当前的最新数据,而电表数据在一个间隔内取的都是
			//当前间隔抄到的数据

	//计算当前总加功率	
	for (n=0; n<2; n++)	//有功、无功
	{
		iGroupP = 0;
		if (n == 0)		
		{
			pItem = m_biRepP;
			pGrpInf = m_GrpInfP;
		}
		else
		{
			pItem = m_biRepQ;
			pGrpInf = m_GrpInfQ;
		}
		
		wNum = 0;
		if (QueryItemTime(isNow.dwS, isNow.dwEndS, pItem, m_bMtrNum, &wNum)==m_bMtrNum && m_dwDayStartEnSec[n]!=0)
		{
			//所有测量点的数据均更新过（含抄表成功、抄表失败以及不支持的数据）
			for (i=0; i<m_bMtrNum; i++)
			{			
				if (ReadItemEx(BN0, pGrpInf[i].wPn, pGrpInf[i].wCurId, bBuf, isNow.dwS, isNow.dwEndS) > 0 )
				{
					if (pGrpInf[i].bProp == PN_PROP_METER)
						iTmpBuf[i][0] = OoDoubleLongToInt(&bBuf[3]);
					else
						iTmpBuf[i][0] = OoDoubleLongToInt(&bBuf[1]);

					if (iTmpBuf[i][0]!=INVALID_VAL64 && pGrpInf[i].bDir==1 && iTmpBuf[i][0]>0) //反向功率取反
						iTmpBuf[i][0] = -iTmpBuf[i][0];
					else if (iTmpBuf[i][0]!=INVALID_VAL64 && pGrpInf[i].bDir==0 && iTmpBuf[i][0]<0) //正向功率取正
						iTmpBuf[i][0] = -iTmpBuf[i][0];

					//若数据有效则进行运算
					iTmpBuf[i][0] *= m_iCT[i]*m_iPT[i];
					DataSum(&iGroupP, iGroupP, iTmpBuf[i][0], pGrpInf[i].bOp);	
				}
				else //若数据全部无效（含抄表失败或不支持的数据）则立即更新为无效数据
				{
					iGroupP = INVALID_VAL64;
					break;
				}
			}

			bBuf[0] = DT_LONG64;
			OoInt64ToLong64(iGroupP, &bBuf[1]);
			WriteItemEx(BN0, m_wPn, g_wGrpCurPowerID[n], bBuf, isNow.dwS);
			WriteItemEx(BN0, m_wPn, g_wGrpSlidePowerID[n], bBuf, isNow.dwS);
		}
	}
	
	//计算当前电量示值,以及当日当月累计电量，并处理上日上月数据的转存
	for (n=0; n<2; n++)	//有功、无功
	{		
		SetArrVal64(iGrpE, 0, BLOCK_ITEMNUM);

		if (n == 0)		
		{
			pItem = m_biRepEp;
			pGrpInf = m_GrpInfEp;			
		}
		else
		{
			pItem = m_biRepEq;
			pGrpInf = m_GrpInfEq;
		}

		wNum = 0;
		if (QueryItemTime(isNow.dwS, isNow.dwEndS, pItem, m_bMtrNum, &wNum)==m_bMtrNum && m_dwDayStartEnSec[n]!=0)//查询电能本周期被更新过
		{
			CalcuEnSum(iGrpE, n, INTVCAL, isNow, GRP_CUR_E);			//计算当前示值
			bBuf[0] = DT_ARRAY;
			bBuf[1] = BLOCK_ITEMNUM;
			for (i=0; i<BLOCK_ITEMNUM; i++)
			{
				bBuf[2+i*9] = DT_LONG64;
				OoInt64ToLong64(iGrpE[i], &bBuf[3+i*9]);
			}
			if (n == 0)
			{
				WriteItemEx(BN0, m_wPn, 0x0a07, &bBuf[2], isNow.dwS); //当前总加有功电能，供控制使用	
			}

			//计算Delta数据
			//重新根据测量点的当前值与起点计算，而不采用直接取测量点的差值相加减，因检测到当前数据到达，而差值量有可能还未计算
			//当日累计总加电能	
			TIntvSec isStart;			
			TimeToIntvS(tmNow, INTVDAY, &isStart);
			DeltaENew(g_wGrpDayDeltaID[n], pGrpInf, INTVDAY, isNow, isStart, isNow);

			//当月累计总加电能
			TimeToIntvS(tmNow, INTVMON, &isStart);
			DeltaENew(g_wGrpMonDeltaID[n], pGrpInf, INTVMON, isNow, isStart, isNow);
		}
	}
}

// 描述：将当日当月的累计电量清零
//@bIdx		要清零的电量的索引（对应正有、正无、反有、反无）
//@bType	日清零还是月清零的标识
//@dwSec	要清零的时标
void CDpGrp::UpdClsFrzData(BYTE bIdx, BYTE bInterv, DWORD dwOldS, DWORD dwNewS)
{
	BYTE bBuf[64];
	TTime tm1,tm2;
	SecondsToTime(dwOldS, &tm1);
	SecondsToTime(m_dwDayDeltaESec[bIdx], &tm2);
	int64 iValBuf[BLOCK_ITEMNUM];

	if (bInterv == INTVDAY)
	{
		ClsBlockE(BN0, g_wGrpDayDeltaID[bIdx], &m_iDayDeltaE[bIdx][0], 0, dwNewS); //当日清零		
		DTRACE(DB_DP, ("CDpGrp::UpdClsFrzData DAY GPN=%d g_wGrpDayDeltaID=%x Val=%lld, dwSec=%d!\n",m_wPn,g_wGrpDayDeltaID[bIdx],m_iDayDeltaE[bIdx][0],dwNewS));

		ClsBlockE(BN18, g_wGrpDayLeftDeltaID[bIdx], iValBuf, 0, dwNewS); //当日起点累计清零	
		ClsBlockE(BN18, g_wGrpDayStartEnID[bIdx], iValBuf, INVALID_VAL64, dwNewS); //当日起点示值清零	
	}
	else if (bInterv == INTVMON)
	{
		ClsBlockE(BN0, g_wGrpMonDeltaID[bIdx], &m_iMonDeltaE[bIdx][0], 0, dwNewS); //当月清零		
		DTRACE(DB_DP, ("CDpGrp::UpdClsFrzData MON GPN=%d g_wGrpDayDeltaID=%x Val=%lld, dwSec=%d!\n",m_wPn,g_wGrpMonDeltaID[bIdx],m_iMonDeltaE[bIdx][0],dwNewS));

		ClsBlockE(BN18, g_wGrpMonLeftDeltaID[bIdx], iValBuf, 0, dwNewS); //当月起点累计清零	
		ClsBlockE(BN18, g_wGrpMonStartEnID[bIdx], iValBuf, INVALID_VAL64, dwNewS); //当月起点示值清零
	}
}

//描述：准点日切换
//备注：主要为功率统计数据以及当总加组所含测量点无电表时的准点切换
void CDpGrp::DayChange(TTime tmNow)
{		
	TIntvSec  isNow;
	DWORD dwCurS, dwCurEndS;
	int64 iValBuf[BLOCK_ITEMNUM];
	BYTE i, j, bDataBuf[30];
	TTime time;
	WORD wCurID;

	GetCurTime(&time);
	TimeToIntvS(tmNow, INTVDAY, &isNow);		

	//日电量数据清零
	if (m_bPnProp == 0)//无电表测量点
	{
		for (i=0; i<2; i++)
		{
			//当日数据清零
			UpdClsFrzData(i, INTVDAY, isNow.dwS, isNow.dwS);
			DTRACE(DB_DP, ("CDpGrp::DayChange()1:PN=%d,DayDeltaID=%x \n ",m_wPn, g_wGrpDayDeltaID[i]));
		}

		if (m_bPnProp != PN_PROP_METER)
		{
			dwCurS = isNow.dwS;
			dwCurEndS = isNow.dwEndS;
		}

		for (i=0; i<sizeof(g_wDayStartID)/sizeof(WORD); i++)
		{	
			for (j=0; j<m_bMtrNum; j++)
			{
				SetArrVal64(iValBuf, INVALID_VAL64, BLOCK_ITEMNUM);

				if (m_GrpInfP[j].bProp == PN_PROP_METER)
					wCurID = g_wCurID[i];
				else
					wCurID = g_wPulseCurID[i];

				//更新日起点 有效无效均更新	
				memset(bDataBuf, 0, sizeof(bDataBuf));
				if (ReadItemEx(BN0, m_GrpInfEp[j].wPn, wCurID, bDataBuf, dwCurS, dwCurEndS) > 0 )
				{	
					for (BYTE k=0; k<BLOCK_ITEMNUM; k++)
						iValBuf[k] = OoDoubleLongToInt(&bDataBuf[3+5*k]);
				}
				WriteItemVal64(BN18, m_GrpInfEp[j].wPn, g_wDayStartID[i], iValBuf, isNow.dwS);
				DTRACE(DB_DP, ("CDpMtr::DayChange():PN=%d,g_wDayStartID=%x StartVal=%lld\n ",m_GrpInfEp[j].wPn, g_wDayStartID[i], iValBuf[0]));
			}
		}
	}

	TrigerSaveBank(BN18, 0, -1); //功率统计起点值触发保存.
}

//描述：准点月切换
//备注：主要为功率统计数据以及当总加组所含测量点无电表时的准点切换
void CDpGrp::MonChange(TTime tmNow)
{
	TIntvSec  isNow;
	DWORD dwCurS, dwCurEndS;
	int64 iValBuf[BLOCK_ITEMNUM];
	BYTE i, j, bDataBuf[30];
	TTime time;
	WORD wCurID;

	GetCurTime(&time);
	TimeToIntvS(tmNow, INTVMON, &isNow);

	//月电量数据清零
	if (m_bPnProp == 0)//无电表测量点
	{
		for (BYTE i=0; i<2; i++)
		{
			//当月数据清零
			UpdClsFrzData(i, INTVMON, isNow.dwS, isNow.dwS);	
			DTRACE(DB_DP, ("CDpGrp::MonChange()1:PN=%d,MonDeltaID=%x \n ",m_wPn, g_wGrpMonDeltaID[i]));
		}	

		if (m_bPnProp != PN_PROP_METER)
		{
			dwCurS = isNow.dwS;
			dwCurEndS = isNow.dwEndS;
		}

		for (i=0; i<sizeof(g_wMonStartID)/sizeof(WORD); i++)
		{	
			for (j=0; j<m_bMtrNum; j++)
			{
				SetArrVal64(iValBuf, INVALID_VAL64, BLOCK_ITEMNUM);

				if (m_GrpInfP[j].bProp == PN_PROP_METER)
					wCurID = g_wCurID[i];
				else
					wCurID = g_wPulseCurID[i];

				//更新日起点 有效无效均更新	
				memset(bDataBuf, 0, sizeof(bDataBuf));
				if (ReadItemEx(BN0, m_GrpInfEp[j].wPn, wCurID, bDataBuf, dwCurS, dwCurEndS) > 0 )
				{	
					for (BYTE k=0; k<BLOCK_ITEMNUM; k++)
						iValBuf[k] = OoDoubleLongToInt(&bDataBuf[3+5*k]);
				}
				WriteItemVal64(BN18, m_GrpInfEp[j].wPn, g_wMonStartID[i], iValBuf, isNow.dwS);
				DTRACE(DB_DP, ("CDpMtr::MonChange():PN=%d,g_wMonStartID=%x StartVal=%lld\n ",m_GrpInfEp[j].wPn, g_wMonStartID[i], iValBuf[0]));		
			}
		}
	}

	TrigerSaveBank(BN18, 0, -1); //功率统计起点值触发保存.
}

//描述：主体调度函数
void CDpGrp::DoDataProc()
{
	if ( IsNeedReset() )
	{
		Init(m_wPn);
	}

	TTime now; 
	GetCurTime(&now);

	if (now.nDay!=m_tmLast.nDay || now.nMonth!=m_tmLast.nMonth || now.nYear!=m_tmLast.nYear)
	{
		DayChange(now);		
	}	
	if (now.nMonth!=m_tmLast.nMonth || now.nYear!=m_tmLast.nYear)
	{
		MonChange(now);		
	}	
	m_tmLast = now;

	UpdDayMonStartVal(INTVDAY, now);
	UpdDayMonStartVal(INTVMON, now);	

	CalcuCurData(now);
} 

//描述：记录总加组最新的日月起点累计值,格式FMT3
void CDpGrp::SetStartDeltaE(BYTE bInterv, WORD wDeltaEId, TTime& tm)
{	
	BYTE i, n = 0;
	BYTE bBuf[64];
	TIntvSec isNow;
	int64 iValBuf[BLOCK_ITEMNUM];
	memset((BYTE*)iValBuf, 0, sizeof(iValBuf));

	TimeToIntvS(tm, bInterv, &isNow);

	if (wDeltaEId==g_wGrpDayDeltaID[0] || wDeltaEId==g_wGrpMonDeltaID[0]) //有功		
		n = 0;		
	else if (wDeltaEId==g_wGrpDayDeltaID[1] || wDeltaEId==g_wGrpMonDeltaID[1])//无功
		n = 1;		

	if (bInterv == INTVDAY) //读到数据写入有效数据，读不到时写入0
	{		
		if (ReadItemEx(BN0, m_wPn, g_wGrpDayDeltaID[n], bBuf, isNow.dwS, isNow.dwEndS) > 0)
		{
			for (i=0; i<BLOCK_ITEMNUM; i++)
				iValBuf[i] = OoLong64ToInt64(&bBuf[3+9*i]);
		}
		WriteItemVal64(BN18, m_wPn, g_wGrpDayLeftDeltaID[n], iValBuf, isNow.dwS);
		DTRACE(DB_DP, ("CDpGrp::SetStartDeltaE m_wPn=%d wDayStartDeltaEnVal=%lld dwSec=%d \n",m_wPn,iValBuf[0], isNow.dwS));	
	}
	else if (bInterv == INTVMON)
	{
		if (ReadItemEx(BN0, m_wPn, g_wGrpMonDeltaID[n], bBuf, isNow.dwS, isNow.dwEndS) > 0)
		{
			for (i=0; i<BLOCK_ITEMNUM; i++)
				iValBuf[i] = OoLong64ToInt64(&bBuf[3+9*i]);
		}
		WriteItemVal64(BN18, m_wPn, g_wGrpMonLeftDeltaID[n], iValBuf, isNow.dwS);
		DTRACE(DB_DP, ("CDpGrp::SetStartDeltaE m_wPn=%d wMonStartDeltaEnVal=%lld dwSec=%d \n",m_wPn,iValBuf[0], isNow.dwS));
	}
}

//描述：设置是否采用新起点示值
void CDpGrp::SetNewStartEnFlg(bool fFlg, DWORD dwNewTime)
{
	m_fNewDayStartEnFlg[0] = fFlg;
	m_fNewDayStartEnFlg[1] = fFlg;
	m_fNewMonStartEnFlg[0] = fFlg;
	m_fNewMonStartEnFlg[1] = fFlg;

	m_dwEnNewStartSec = dwNewTime;

	BYTE bBuf[64];
	int64 iGroupP;
	int64 iGrpE = INVALID_VAL64;
	TTime now;
	TIntvSec isNow;

	SecondsToTime(dwNewTime, &now);
	TimeToIntvS(now, INTVCAL, &isNow);	

	//当前总加功率，总加示值电量置为无效，抄到数据再更新
	TimeToIntvS(now, INTVCAL, &isNow);
	for (BYTE i=0; i<2; i++)
	{
		iGroupP = INVALID_VAL64;
		bBuf[0] = DT_LONG64;
		OoInt64ToLong64(iGroupP, &bBuf[1]);
		WriteItemEx(BN0, m_wPn, g_wGrpCurPowerID[i], bBuf, isNow.dwS);//当前总加功率	
		WriteItemEx(BN0, m_wPn, g_wGrpSlidePowerID[i], bBuf, isNow.dwS);//当前总加功率
	}

	bBuf[0] = DT_LONG64;
	OoInt64ToLong64(iGrpE, &bBuf[1]);
	WriteItemEx(BN0, m_wPn, 0x0a07, bBuf, isNow.dwS); //当前总加有功电能，供控制使用			
}

extern bool g_fMtrParaChg;
void SetMtrParaChg(bool fFlg)
{
	g_fMtrParaChg = fFlg;
	DTRACE(DB_METER, ("SetMtrParaChg: para chg =%d!\r\n", g_fMtrParaChg));
}

bool IsMtrParaChg()
{
	return g_fMtrParaChg;
}

extern bool g_fGrpParaChg;
void SetGrpParaChg(bool fFlg)
{
	g_fGrpParaChg = fFlg;
	DTRACE(DB_METER, ("SetGrpParaChg: para chg = %d!\r\n", g_fGrpParaChg));
}

bool IsGrpParaChg()
{
	return g_fGrpParaChg;
}

extern bool g_fCtrlGrpParaChg;
void SetCtrlGrpParaChg(bool fFlg)
{
	g_fCtrlGrpParaChg = fFlg;
	DTRACE(DB_METER, ("SetCtrlGrpParaChg: para chg = %d!\r\n", g_fCtrlGrpParaChg));
}

bool IsCtrlGrpParaChg()
{
	return g_fCtrlGrpParaChg;
}

CDpGrp *g_GroupPn[GB_MAXSUMGROUP];
//参数更新
void UpdGrpDataProcess(bool fPowerUp)
{
	TTime now;
	GetCurTime(&now);
	bool fNewStartFlg = false;

	 //非上电第一次，则一定为参数更新(新增或修改),需要更新总加组起点
	if ( !fPowerUp )
	{
		fNewStartFlg = true;
	}

	DTRACE(DB_DP, ("UpdGrpDataProcess()! \r\n"));

	for (BYTE i=1; i<GB_MAXSUMGROUP; i++)
	{	
		if ( IsGrpValid(i) )//总加组有效
		{		
			if (g_GroupPn[i] == NULL)//新增
			{
				g_GroupPn[i] = new CDpGrp;
			}
			else 
			{
				//之前已有的总加组参数变更则要记下原累计值，作为新的累计值起点
				//差值起点要更新后才能记
				for (BYTE j=0; j<2; j++)
				{
					g_GroupPn[i]->SetStartDeltaE(INTVDAY, g_wGrpDayDeltaID[j], now);					
					g_GroupPn[i]->SetStartDeltaE(INTVMON, g_wGrpMonDeltaID[j], now);							
				}
				//fNewStartFlg = true;
			}

			if ( !g_GroupPn[i]->Init(i) )//重新初始化			
			{
				delete g_GroupPn[i];
				g_GroupPn[i] = NULL;
				//需要清除相应总加组的数据
				ClrGrpPnData(i);
			}
			else
			{
				if ( fNewStartFlg )
					g_GroupPn[i]->SetNewStartEnFlg(true, TimeToSeconds(now)); 
			}			
		}
		else	//无效
		{
			if (g_GroupPn[i] != NULL)//有效变无效
			{
				delete g_GroupPn[i];			
				g_GroupPn[i] = NULL;
				//需要清除相应总加组的数据
				ClrGrpPnData(i);
			}					
		}
	}
}                  

//处理线程
void RunGrpDataProcess()
{
	//如果参数有变更，先停止计算
	if (/*IsMtrParaChg() ||*/ IsGrpParaChg()) 
		return;

	for (int i=1; i<GB_MAXSUMGROUP; i++)
	{		
		if (g_GroupPn[i] != NULL)//新增
		{
			g_GroupPn[i]->DoDataProc();
		}
	}
}

//获取总加组当前控制状态
bool GetGrpCurCtrlSta(int iGrp, TGrpCurCtrlSta *pGrpCurCtrlSta)
{
	BYTE bBuf[32] = {0};
	BYTE *ptr = bBuf;
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;

	if (OoReadAttr(0x2300+iGrp, 0x11, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("GetGrpCurCtrlSta: There is something wrong when call OoReadAttr() !\n"));
		return false;
	}

	ptr += 2;	//结构及成员个数
	ptr++;		//DT_LONG64
	pGrpCurCtrlSta->CurPwrVal = OoLong64ToInt64(ptr);	//当前功控定值
	ptr += 8;	
	ptr++;		//DT_INT
	//memcpy(pGrpCurCtrlSta->FloatRate, ptr, 4);			//当前功率下浮控浮动系数
	pGrpCurCtrlSta->FloatRate = *ptr++;
	ptr += 2;	//DT_BIT_STR和字节数
	pGrpCurCtrlSta->bAllPwrCtrlOutPutSta = ByteBitReverse(*ptr);		//功控跳闸输出状态
	ptr++;
	ptr += 2;	//DT_BIT_STR和字节数
	pGrpCurCtrlSta->bMonthCtrlOutPutSta = ByteBitReverse(*ptr);			//月电控跳闸输出状态
	ptr++;
	ptr += 2;	//DT_BIT_STR和字节数
	pGrpCurCtrlSta->bBuyCtrlOutPutSta = ByteBitReverse(*ptr);			//购电控跳闸输出状态
	ptr++;
	ptr += 2;	//DT_BIT_STR和字节数
	pGrpCurCtrlSta->bPCAlarmState = ByteBitReverse(*ptr);				//功控越限告警状态
	ptr++;
	ptr += 2;	//DT_BIT_STR和字节数
	pGrpCurCtrlSta->bECAlarmState = ByteBitReverse(*ptr);				//电控越限告警状态
	ptr++;

	return true;
}

//设置总加组当前控制状态
bool SetGrpCurCtrlSta(int iGrp, TGrpCurCtrlSta *pGrpCurCtrlSta)
{
	BYTE bBuf[32] = {0};
	BYTE *ptr = bBuf;

	if (pGrpCurCtrlSta != NULL)
	{
		*ptr++ = DT_STRUCT;
		*ptr++ = 7;							//结构成员个数
		*ptr++ = DT_LONG64;					//当前功控定值
		OoInt64ToLong64(pGrpCurCtrlSta->CurPwrVal, ptr);
		ptr += 8;
		*ptr++ = DT_INT;					//当前功率下浮控浮动系数
		*ptr++ = pGrpCurCtrlSta->FloatRate;
		*ptr++ = DT_BIT_STR;				//功控跳闸输出状态	
		*ptr++ = 8;
		*ptr++ = ByteBitReverse(pGrpCurCtrlSta->bAllPwrCtrlOutPutSta);
		*ptr++ = DT_BIT_STR;				//月电控跳闸输出状态
		*ptr++ = 8;
		*ptr++ = ByteBitReverse(pGrpCurCtrlSta->bMonthCtrlOutPutSta);
		*ptr++ = DT_BIT_STR;				//购电控跳闸输出状态
		*ptr++ = 8;
		*ptr++ = ByteBitReverse(pGrpCurCtrlSta->bBuyCtrlOutPutSta);
		*ptr++ = DT_BIT_STR;				//功控越限告警状态
		*ptr++ = 8;
		*ptr++ = ByteBitReverse(pGrpCurCtrlSta->bPCAlarmState);
		*ptr++ = DT_BIT_STR;				//电控越限告警状态
		*ptr++ = 8;
		*ptr++ = ByteBitReverse(pGrpCurCtrlSta->bECAlarmState);

		if (OoWriteAttr(0x2300+iGrp, 0x11, bBuf) < 0)
		{
			DTRACE(DB_LOADCTRL, ("SetGrpCurCtrlSta: There is something wrong when call OoWriteAttr() !\n"));
			return false;
		}

		return true;
	}

	return false;
}

//获取总加组控制设置状态
bool GetGrpCtrlSetSta(int iGrp,	TGrpCtrlSetSta *pGrpCtrlSetSta)
{
	BYTE bBuf[20] = {0};
	BYTE *ptr = bBuf;
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;

	if (OoReadAttr(0x2300+iGrp, 0x10, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("GetGrpCtrlSetSta: There is something wrong when call OoReadAttr() !\n"));
		return false;
	}

	ptr += 2;		//结构及成员个数
	ptr++;			//DT_UNSIGN
	pGrpCtrlSetSta->bSchemeNum = *ptr;		//时段控定值方案号
	ptr++;
	ptr += 2;			//DT_BIT_STR和字节个数
	pGrpCtrlSetSta->bValidFlag = *ptr;		//功控时段有效标志位
	ptr++;
	ptr += 2;			//DT_BIT_STR和字节个数
	pGrpCtrlSetSta->bPwrCtrlSta = *ptr;		//功控状态
	ptr++;
	ptr += 2;			//DT_BIT_STR和字节个数
	pGrpCtrlSetSta->bEngCtrlSta = *ptr;		//电控状态
	ptr++;
	ptr += 2;			//DT_BIT_STR和字节个数
	pGrpCtrlSetSta->bPwrCtrlTurnSta = *ptr;	//功控轮次状态
	ptr++;
	ptr += 2;			//DT_BIT_STR和字节个数
	pGrpCtrlSetSta->bEngCtrlTurnSta = *ptr;	//电控轮次状态
	ptr++;

	return true;
}

//设置总加组控制设置状态
bool SetGrpCtrlSetSta(int iGrp,	TGrpCtrlSetSta *pGrpCtrlSetSta)
{
	BYTE bBuf[20] = {0};
	BYTE *ptr = bBuf;
	
	if (pGrpCtrlSetSta != NULL)
	{
		*ptr++ = DT_STRUCT;
		*ptr++ = 6;					//结构成员个数
		*ptr++ = DT_UNSIGN;			
		*ptr++ = pGrpCtrlSetSta->bSchemeNum;		//时段控定值方案号
		*ptr++ = DT_BIT_STR;						
		*ptr++ = 8;
		*ptr++ = pGrpCtrlSetSta->bValidFlag;		//功控时段有效标志位
		*ptr++ = DT_BIT_STR;
		*ptr++ = 8;
		*ptr++ = pGrpCtrlSetSta->bPwrCtrlSta;		//功控状态
		*ptr++ = DT_BIT_STR;
		*ptr++ = 8;
		*ptr++ = pGrpCtrlSetSta->bEngCtrlSta;		//电控状态
		*ptr++ = DT_BIT_STR;
		*ptr++ = 8;
		*ptr++ = pGrpCtrlSetSta->bPwrCtrlTurnSta;	//功控轮次状态
		*ptr++ = DT_BIT_STR;
		*ptr++ = 8;
		*ptr++ = pGrpCtrlSetSta->bEngCtrlTurnSta;	//电控轮次状态
		
		if (OoWriteAttr(0x2300+iGrp, 0x10, bBuf) < 0)
		{
			DTRACE(DB_LOADCTRL, ("SetGrpCtrlSetSta: There is something wrong when call OoWriteAttr() !\n"));
			return false;
		}

		return true;
	}

	return false;
}