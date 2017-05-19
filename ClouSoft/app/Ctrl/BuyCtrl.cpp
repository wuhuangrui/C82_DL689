/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：BuyCtrl.cpp
 * 摘    要：本文件主要实现CBuyCtrl的类
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
 * 备注：读取剩余电能量（1类数据F23,数据库没有初始化，如果没有投入购电单号，则读取值为0,岑工说读取为无效值，不知道要不要做？19th,June）；
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "TaskManager.h"
#include "BuyCtrl.h"
#include "DpGrp.h"
#include "DbOIAPI.h"

//========================================== CBuyCtrl ==============================================
CBuyCtrl::CBuyCtrl(void)
{
	for (int i=0; i<GRP_NUM; i++)
	{
		m_iCurBuyRemainEng[i]	= 0;	//初始化为负值.
		m_iBaseEng[i]			= 0;	//初始化为负值.
		m_dwBillIdx[i]			= 0;	//初始化为负值.
		m_fUpBaseEng[i]			= false;
	}

	m_fAlrStauts = false;		//将报警状态取消.
	m_fEnergyFeeFlag = false;
	m_iCurFeeRatio = 0;
}

//描述: 初始化.
//返回: 如果初始化正常返回 true,否则返回 false.
bool CBuyCtrl::Init(void)
{
	ClrCmd();		//清除内存中本类控制的控制命令.
	RstCtrl();		//复位内存中本类控制状态量.
	SetValidStatus(false);	//设定控制退出状态.

	TBuyCtrlPara BuyCtrlPara;

	for (WORD i=1; i<=GRP_NUM; i++) //上电时初始化
	{
		if (IsGrpValid(i) && GetBuyCtrlPara(i, BuyCtrlPara))
		{
			m_dwBillIdx[i] = BuyCtrlPara.dwBillIdx;
			ReadItemVal64(BN0, i, 0x0a05, &m_iCurBuyRemainEng[i]); // 终端当前剩余电量（费）
			ReadItemVal64(BN0, i, 0x087f, &m_iBaseEng[i]);			//总加组 购电控投入时上一分钟电能量
		}
	}

	return true; //return GetSysStatus();
}

//描述: '购电控'控制.
//返回: 正常则返回 true,否则返回 false.
bool CBuyCtrl::DoCtrl(void)
{
	BYTE bBuf1[100],bBuf2[100];
	DoCmdScan();		//扫描系统库中的命令.
	UpdateBuyRemainEng();
	if (!IsValid())
	{
		RstCtrl();
		return true;
	}
	UpdateSysRemainEng();		//更新所有总加组剩余电量.
/*	if (IsGuarantee())	//检测是否处在保电状态.
	{
		RstCtrl();					//复位内存中本类控制的所有相关状态.
		return true;
	}
*/
	int i;
	BYTE b = GetSysCtrlTurnsCfg(m_iGrp); //取电控轮次设定

	if ((m_bTurnsStatus&~b) != 0)
	{//如非受控轮次发生跳闸的,应对这些轮次进行复位(置位允许合闸状态).
		m_bTurnsStatus &= b;
	}
	//检测是否有可跳闸.
	i = GetIdxOfMostRight1(b & ~GetTurnsStatus());	//获取相应总加组当前可跳闸的轮次号.
	m_bWarnStatus = i+1;
	if (!GetBuyCtrlPara(m_iGrp, m_BuyCtrlPara))
	{
		DTRACE(DB_LOADCTRL, ("CBuyCtrl::DoCtrl: There is somethig wrong when get BuyCtrl parameter of Group[%d] !\n", m_iGrp));
		return false;	//读取购电控参数出错.
	}

	char cTime[20];

	m_iBuyRemain = m_iCurBuyRemainEng[m_iGrp];
	
	if (m_iBuyRemain > m_BuyCtrlPara.iAlarmLimit)
	{//剩余电量没到报警线.
		RstCtrl();		//复位内存中本类控制的所有相关状态.
	}
	else if (m_iBuyRemain > m_BuyCtrlPara.iActLimit)
	{//剩余电量已到报警线,但还没到跳闸线.

		SubRstCtrl();				//复位内存中本类控制的部分相关状态.
		if (i >= 0)
		{//还有可跳闸.
			if (!m_fAlrStauts)
			{
				SetSysCtrlAlr(m_iGrp, true); //设定电控告警状态的购电控越限
#ifdef PRO_698
				//保存告警记录.
				/*BYTE bBuf[20];

				bBuf[0] = (BYTE)m_iGrp;					//总加组
				bBuf[1] = GetSysCtrlTurnsCfg(m_iGrp);		//轮次
				bBuf[2] = 0x02;							//电控类别
				Val64ToFmt3(m_iBuyRemain, bBuf+3, 4);//告警时剩余电能量
				Val64ToFmt3(m_BuyCtrlPara.iActLimit, bBuf+7, 4);//购电控跳闸门限；*/
				//记录当前跳闸记录到系统库中.
				//SaveAlrData(ERC_ENGALARM, m_tmNow, bBuf);
#endif
				DTRACE(DB_LOADCTRL, ("CBuyCtrl::DoCtrl: Grp%d alarm start at %s, remain-energy is %lld, alarm-limit is %lld, act-limit is %lld\n",
									 m_iGrp, TimeToStr(m_tmNow, cTime), m_iBuyRemain, m_BuyCtrlPara.iAlarmLimit, m_BuyCtrlPara.iActLimit));
			}
			m_fAlrStauts = true;
			//SaveDisp();
		}
		else
		{//无闸可跳.
			if (m_fAlrStauts)
				SetSysCtrlAlr(m_iGrp, false); //清电控告警状态的购电控越限
			m_fAlrStauts = false;
		}
	}
	else if (IsGuarantee())	//检测是否处在保电状态.
	{
		RstCtrl();					//复位内存中本类控制的所有相关状态.
		m_fIfOverLimit = true;
		return true;
	}
	else	//(m_iBuyRemain <= m_BuyCtrlPara.iActLimit)
	{
		m_fIfOverLimit = true;

		if (i < 0)									//检测是否有可跳闸.
		{
			if (m_fAlrStauts)
				SetSysCtrlAlr(m_iGrp, false); //清电控告警状态的购电控越限
			m_fAlrStauts = false; //无闸可跳了,报警没有意义,禁止报警.
			return true;
		}

		if (!m_fAlrStauts)
			SetSysCtrlAlr(m_iGrp, true);
		m_fAlrStauts = true;
		//SaveDisp();

		DWORD dwTurnInv = GetEngTurnInv(i+TURN_START_PN);	//获取相应轮次的功控报警持续时间.
		if (m_dwOpenTurnTime > m_dwNow)	//时间往前调回去了
			m_dwOpenTurnTime = 0;

		if (m_dwOpenTurnTime == 0)					//本购从来没跳过闸,则直接跳最小轮次的闸,同时记录下本次跳闸时间.
			m_dwOpenTurnTime = m_dwNow;
		else if (m_dwNow < m_dwOpenTurnTime+dwTurnInv)		//上次跳闸后,必须隔60秒才能再跳下一轮闸
			return true;
		else
			m_dwOpenTurnTime = m_dwNow;				//距离上次跳闸已超过60秒,可以再次跳闸,同时记录下本次跳闸时间.

		m_bTurnsStatus |= 0x01 << i;
		//SaveDisp();
		//SetSysTurnsStatus(m_iGrp, m_bTurnsStatus);	//将这些闸进行跳闸,(实际上由于每次只跳1轮,因此对原来的状态量来说,只会改变1位)
		m_wOpenTimes++;								//跳闸次数增加1.

		//保存跳闸记录.
		/*BYTE bBuf[1+1+1+4+4];

		bBuf[0] = (BYTE)m_iGrp;					//总加组
		bBuf[1] = (BYTE)(0x01<<i);				//轮次
		bBuf[2] = 0x02;							//电控类别
		Val64ToFmt3(m_iBuyRemain, bBuf+3, 4);//跳闸时剩余电能量
		Val64ToFmt3(m_BuyCtrlPara.iActLimit, bBuf+7, 4);//够电控跳闸门限；*/
		//记录当前跳闸记录到系统库中.
		//SaveAlrData(ERC_ENGCTL, m_tmNow, bBuf);

		DTRACE(DB_LOADCTRL, ("CBuyCtrl::DoCtrl: Turn%d of Grp%d open at %s, remain-energy is %lld, act-limit is %lld\n",
							 i+GRP_START_PN, m_iGrp, TimeToStr(m_tmNow, cTime), m_iBuyRemain, m_BuyCtrlPara.iActLimit));
		//***发出声光信号;
	}

	return true;
}

//描述：生成购电控越限时的显示参数；
void CBuyCtrl::SaveDisp()
{
	BYTE bBuf[13];
	bBuf[0] = m_bTurnsStatus;
	Val64ToFmt(m_iBuyRemain, bBuf+1, FMT3, 4);
	Val64ToFmt(m_BuyCtrlPara.iAlarmLimit, bBuf+5, FMT3, 4);
	Val64ToFmt(m_BuyCtrlPara.iActLimit, bBuf+9, FMT3, 4);
	WriteItemEx(BN0, PN0, 0x0940, bBuf); //购电控告警信息界面
}

void CBuyCtrl::SubRstCtrl(void)
{
	m_bTurnsStatus	 = 0x00;			//将轮次状态全部设为合闸.
	m_dwOpenTurnTime = 0;				//上次跳闸时间设为0;
	m_fIfOverLimit	 = false;
}

//描述: 复位内存中本类控制状态量.
void CBuyCtrl::RstCtrl(void)
{
	
	SetSysCtrlAlr(m_iGrp, false); //清C1F6'终端当前控制状态'中电控告警状态的购电控越限
	m_fAlrStauts = false;		//将报警状态取消.
	SubRstCtrl();
}

//描述: 获取某总加组的本类控制命令,并把命令放到 m_NewCmd中.(注意: 对不同的类,m_NewCmd的结构是不同的)
//参数:@iGrp	要获取命令的总加组.
//返回: 如果获取成功且为有效命令 true,否则返回 false.
bool CBuyCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8273, bCmd, &m_NewCmd.dwTime) != sizeof(bCmd)) //读取相应总加组的"购电控投入命令".
	{
		DTRACE(DB_LOADCTRL, ("CBuyCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	m_NewCmd.bAct = bCmd[0];
	if (m_NewCmd.bAct!=1 && m_NewCmd.bAct!=2) 
		return false;

	if (m_NewCmd.dwTime == 0) //终端时间往前调了,系统库中的命令时间被清掉了
	{
		if (CurCmdTime()!=0 && iGrp==m_iGrp) //旧的命令已经被处理,只是系统库中的命令时间被清掉了
		{
			m_NewCmd.dwTime = GetCurTime(); //更新为一个合理一点的时间,方便以后不同命令时间的比较
			WriteItemEx(BN0, (WORD)iGrp, 0x8273, bCmd, m_NewCmd.dwTime);	//把相应总加组的"时段控命令"写会数据库
		}
		//else 让命令还是保持投入,只是命令的时间被清掉了,但也不能给命令一个明确的时间
		//	   它可能在投入后还没来得及扫描就被清除了,也可能是个已经被控制清除的旧命令,
		// 	   只是突然掉电没保存住;
		// 	   那除非它是系统中唯一被投入的命令,否则其它命令的时间比它后,它不会被执行
	}

	return true;
}

//描述: 清除系统库本总加组本类控制命令.
//参数:@iGrp	要清除命令的总加组.
//返回: 如果清除成功返回 true,否则返回 false.
bool CBuyCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[10];
	m_fUpBaseEng[m_iGrp] = false;
	m_iBaseEng[m_iGrp]=0;
	WriteItemVal64(BN0, m_iGrp, 0x087f, &m_iBaseEng[m_iGrp]); //清楚开始的电量

	memset( bCmd, 0, sizeof(bCmd) );
	WriteItemEx(BN0, (WORD)iGrp, 0x8273, bCmd);	//写相应轮次的"购电控投入投入命令"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0); //触发保存.

	return true;
}

//描述: 更新系统库当前总加组剩余购电量(剩余电量 = 剩余电量起始值 - (当前电量 - 当前电量起始值)).
//参数:@iGrp		要获取的总加组.
//返回: 当前的剩余电量.
void CBuyCtrl::UpdateSysRemainEng()
{
    bool fTrigerSave = false;
    int64 iEng = 0;
	BYTE bTmpBuf[9] = {0};
	BYTE bBuf[50] = {0};
	if (IsGrpValid(m_iGrp) )
	{
		if (m_iBaseEng[m_iGrp] == 0) //上电后首次从系统库中读取；
		{
			//ReadItemVal64(BN0, m_iGrp, 0x0a05, &m_iCurBuyRemainEng[m_iGrp]); // 终端当前剩余电量（费）
			ReadItemEx(BN0, m_iGrp, 0x230a, bTmpBuf);	// 终端当前剩余电量（费）
			m_iCurBuyRemainEng[m_iGrp] = OoLong64ToInt64(bTmpBuf+1);

			iEng = GetGroupEng(m_iGrp); //放在最前面取总加组示值，确保总加组示值有效时先更新给起始值m_iBaseEng
			if (iEng>=0 && (!m_fUpBaseEng[m_iGrp]))
			{
				m_iBaseEng[m_iGrp] = iEng; //更新起始值
				m_fUpBaseEng[m_iGrp] = true;
			}
		}
		if (IsCtrlGrpParaChg()) //总加组参数变更标志，需停止当前控制
		{			
			//if (GetInfo(INFO_GRP_CHG_OVER)) //总加组更新完毕
			{
				if (m_iBaseEng[m_iGrp] !=0)
				{
					m_iBaseEng[m_iGrp] = 0;
					m_fUpBaseEng[m_iGrp] = false;
					ReadItemEx(BN0, m_iGrp, 0x2308, bBuf);
					OoInt64ToLong64(m_iBaseEng[m_iGrp], bBuf+3);
					WriteItemEx(BN0, m_iGrp, 0x2308, bBuf);//清楚开始的电量
					bTmpBuf[0] = DT_LONG64;
					OoInt64ToLong64(m_iCurBuyRemainEng[m_iGrp], bTmpBuf+1);
					WriteItemEx(BN0, m_iGrp, 0x230a, bTmpBuf);//总加组 终端当前剩余电量（费）

					/*WriteItemVal64(BN0, m_iGrp, 0x087f, &m_iBaseEng[m_iGrp]); //清楚开始的电量
					WriteItemVal64(BN0, m_iGrp, 0x110f, &m_iCurBuyRemainEng[m_iGrp]); //C1F23 总加组 终端当前剩余电量（费）
					WriteItemVal64(BN0, m_iGrp, 0x0a05, &m_iCurBuyRemainEng[m_iGrp]); // 总加组 终端当前剩余电量（费）*/				
					TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存,防止掉电丢失购电单.
				}
				SetCtrlGrpParaChg(false); //清除控制总加组变更标志
				DTRACE(DB_LOADCTRL, ("CBuyCtrl::UpdateSysRemainEng: m_iBaseEng=%lld,iEng=%lld. !\r\n", m_iBaseEng[m_iGrp],iEng));	
				DTRACE(DB_LOADCTRL, ("CBuyCtrl::UpdateSysRemainEng: m_iCurBuyRemainEng[m_iGrp]=%lld. !\r\n", m_iCurBuyRemainEng[m_iGrp]));	
			}
			return;
		}
		if (m_tmNow.nMinute != m_tmOldTime.nMinute)	//忽略时,日,月,年的检测,提高效率,且不会引起误差.
		{	//假如有新的购电单或时钟分钟发生了切换,则需更新剩余电量.

			iEng = GetGroupEng(m_iGrp);
			if (iEng>0 && iEng>m_iBaseEng[m_iGrp] && m_iBaseEng[m_iGrp]>=0 && IsValid()) //chenxi,添加如果读不到总加组电量时的防护；
			{
				if (m_fEnergyFeeFlag)
				{
					int64 iTmpEng = (iEng - m_iBaseEng[m_iGrp]) * m_iCurFeeRatio;
					m_iCurBuyRemainEng[m_iGrp] -= iTmpEng;
				}
				else
					m_iCurBuyRemainEng[m_iGrp] -= (iEng - m_iBaseEng[m_iGrp]);
			}

			if (iEng < m_iBaseEng[m_iGrp]) //出现电量变小的情况，
			{
				m_bCount++;
				DTRACE(DB_LOADCTRL, ("CBuyCtrl::Eng ERR: m_iBaseEng=%lld,iEng=%lld. !\r\n", m_iBaseEng[m_iGrp],iEng));	
				if (m_bCount < 3 ) return;
			}
			m_bCount = 0;
			//更新系统库中的剩余电量.
			//WriteItemVal64(BN0, m_iGrp, 0x110f, &m_iCurBuyRemainEng[m_iGrp]); //C1F23 总加组 终端当前剩余电量（费）
			//WriteItemVal64(BN0, m_iGrp, 0x0a05, &m_iCurBuyRemainEng[m_iGrp]); // 总加组 终端当前剩余电量（费）
			bTmpBuf[0] = DT_LONG64;
			OoInt64ToLong64(m_iCurBuyRemainEng[m_iGrp], bTmpBuf+1);
			WriteItemEx(BN0, m_iGrp, 0x230a, bTmpBuf);	// 总加组 终端当前剩余电量（费）

			if (iEng > 0 && IsValid())
			{
				m_iBaseEng[m_iGrp] = iEng; //更新上一分钟总加组电量；
				ReadItemEx(BN0, m_iGrp, 0x2308, bBuf);
				OoInt64ToLong64(m_iBaseEng[m_iGrp], &bBuf[3]);
				WriteItemEx(BN0, m_iGrp, 0x2308, bBuf);				//保存上一分钟总加组电量,防止掉电；
				//WriteItemVal64(BN0, m_iGrp, 0x087f, &m_iBaseEng[m_iGrp]); //保存上一分钟总加组电量,防止掉电；
				TTime tmNow;
				GetCurTime(&tmNow);
				
				if (tmNow.nSecond == 0)//每分钟保存一次剩余电量
				{
					fTrigerSave = true;
				}
			}
		}
	}
		
	if (fTrigerSave)
		TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存,防止掉电丢失购电单.
}
//描述: 更新系统库当前总加组剩余购电量(剩余电量 = 剩余电量起始值 - (当前电量 - 当前电量起始值)).
//参数:@iGrp		要获取的总加组.
//返回: 当前的剩余电量.
void CBuyCtrl::UpdateBuyRemainEng()
{
    bool fTrigerSave = false;
	TBuyCtrlPara BuyCtrlPara;
	BYTE bTmpBuf[9] = {0};
	for (WORD i=1; i<=GRP_NUM; i++)
	{
		if (IsGrpValid(i) && GetBuyCtrlPara(i, BuyCtrlPara))
		{
			if (m_iBaseEng[i] == 0) //上电后首次从系统库中读取；
			{
				//ReadItemVal64(BN0, i, 0x0a05, &m_iCurBuyRemainEng[i]); // 终端当前剩余电量（费）
				memset(bTmpBuf, 0, sizeof(bTmpBuf));
				ReadItemEx(BN0, i, 0x230a, bTmpBuf);
				m_iCurBuyRemainEng[i] = OoLong64ToInt64(&bTmpBuf[1]);
			}

			if (BuyCtrlPara.dwBillIdx!=m_dwBillIdx[i])	
			{	//假如有新的购电单或时钟分钟发生了切换,则需更新剩余电量.
				int64 iEng = GetGroupEng(i);
				if (BuyCtrlPara.dwBillIdx != m_dwBillIdx[i])
				{//有新的有效购电单.
					//WriteItemVal64(BN0, i, 0x082f, &m_iCurBuyRemainEng[i]);	//保存本次购电前剩余电量.

					//保存购电参数设置记录事件(Erc19)
					/*BYTE bBuf[26];
					bBuf[0] = (i) & 0x3f;                      //购电总加组号；
					memcpy(bBuf+1, &BuyCtrlPara.dwBillIdx, 4);              //购电单号；
					bBuf[5] = BuyCtrlPara.bFlag;						    //追加/刷新标志；
					Val64ToFmt(BuyCtrlPara.iBuyEng, bBuf+6, FMT3, 4);		//购电量值；
					Val64ToFmt(BuyCtrlPara.iAlarmLimit, bBuf+10, FMT3, 4);  //报警门限；
					Val64ToFmt(BuyCtrlPara.iActLimit, bBuf+14, FMT3, 4);    //跳闸门限；
					Val64ToFmt(m_iCurBuyRemainEng[i], bBuf+18, FMT3, 4);    //本次购电前剩余电量；*/

					if (BuyCtrlPara.bFlag == 0x55)//追加购电
					{
						if (m_iBaseEng[i]>0 && iEng>0 && iEng>=m_iBaseEng[i] && IsValid())
							m_iCurBuyRemainEng[i] -= (iEng - m_iBaseEng[i]);
						m_iCurBuyRemainEng[i] += BuyCtrlPara.iBuyEng;
						DTRACE(DB_LOADCTRL, ("CBuyCtrl::UpdateSysBuyRemainEng: rx a buy energy bill that increase energy %lld, current remain energy = %lld !\n", 
											 BuyCtrlPara.iBuyEng, m_iCurBuyRemainEng[i]));
					}
					else //(BuyCtrlPara.bFlag == 0xaa)//刷新购电
					{
						m_iCurBuyRemainEng[i] = BuyCtrlPara.iBuyEng;
						DTRACE(DB_LOADCTRL, ("CBuyCtrl::UpdateSysBuyRemainEng: rx a buy energy bill that change energy to %lld, current remain energy = %lld !\n", 
											 BuyCtrlPara.iBuyEng, m_iCurBuyRemainEng[i]));
					}

					//Val64ToFmt(m_iCurBuyRemainEng[i], bBuf+22, FMT3, 4);   //本次购电后剩余电量；
					//SaveAlrData(ERC_BUYPARA, m_tmNow, bBuf);

					//WriteItemVal64(BN0, i, 0x083f, &m_iCurBuyRemainEng[i]); //保存购电后的剩余电量.
					m_dwBillIdx[i] = BuyCtrlPara.dwBillIdx; //更新购电单号；
				}				
				//更新系统库中的剩余电量.
				//WriteItemVal64(BN0, i, 0x110f, &m_iCurBuyRemainEng[i]); //C1F23 总加组 终端当前剩余电量（费）
				//WriteItemVal64(BN0, i, 0x0a05, &m_iCurBuyRemainEng[i]); // 总加组 终端当前剩余电量（费）	
				if (bTmpBuf[0] != DT_LONG64)
					bTmpBuf[0] = DT_LONG64;
				OoInt64ToLong64(m_iCurBuyRemainEng[i], &bTmpBuf[1]);
				WriteItemEx(BN0, i, 0x230a, bTmpBuf); // 总加组 终端当前剩余电量（费）
				fTrigerSave = true;
			}
			//m_fEnergyFeeFlag = GetCurFeeRatio(&m_iCurFeeRatio);		//面向对象协议里未要求费控，暂时注释掉	--QLS 17.01.16
		}
	}
	
	if (fTrigerSave)
		TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存,防止掉电丢失购电单.
}
//描述: 获取当前总加组'购电控'参数.
//参数:@iGrp		要获取的总加组.
//	   @rPara		引用的参数结构,本函数读到数据后,通过本结构将参数传回.
//返回: 成功返回 true,否则返回 false.
bool CBuyCtrl::GetBuyCtrlPara(int iGrp, TBuyCtrlPara& rPara)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bBuf[43];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8107, bBuf) != sizeof(bBuf))		//从相应的测量点读"购电控参数"ID
	{
		DTRACE(DB_LOADCTRL, ("CBuyCtrl::GetBuyCtrlPara: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	rPara.dwBillIdx = OoDoubleLongUnsignedToDWord(&bBuf[6]);
	rPara.bFlag		  = (bBuf[11]==0) ? 0x55:0xaa;
	rPara.iBuyEng	  = OoLong64ToInt64(&bBuf[15]);
	rPara.iAlarmLimit = OoLong64ToInt64(&bBuf[24]);
	rPara.iActLimit	  = OoLong64ToInt64(&bBuf[33]);

//	if (rPara.iActLimit > rPara.iAlarmLimit)
//		return false;

	if (rPara.bFlag!=0x55 && rPara.bFlag!=0xaa)   //追加/刷新标志
		return false;

	return true;
}

int64 CBuyCtrl::GetGroupEng(int iGrp)
{
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	BYTE bBuf[50] = {0};
	int64 i64Value = 0;

	if (OoReadAttr(0x2300+(WORD)iGrp, 0x09, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("GetSelEng: There is something wrong when call OoReadAttr() !\n"));
		return -1;
	}

	i64Value = OoLong64ToInt64(&bBuf[3]);

	if (i64Value < 0) //取用绝对值
		i64Value = -i64Value;

	return i64Value;
}

//描述: 获取当前电能量费率
//参数:@iCurFeeRatio 要获取的当前电能量费率
//返回: 购电费控返回true,购电量控返回 false.
bool CBuyCtrl::GetCurFeeRatio(int64 *piCurFeeRatio)
{
	BYTE bBuf[200], bEnergyFeeTime[48];
	int i, n;
	int64 iEnergyFeeRatio[50];
	bool fEnergyFeeFlag = false;
	TTime now;
	WORD minutes;
	memset( bBuf, 0, sizeof(bBuf));
	memset( bEnergyFeeTime, 0, sizeof(bEnergyFeeTime));

	if (ReadItemEx(BN0, PN0, 0x016f, bBuf) > 0)
	{
#ifdef PRO_698	
		BYTE bFeeRatioNum = bBuf[0];
		BYTE bStartNum = 1;
#else
		BYTE bFeeRatioNum = 14;
		BYTE bStartNum = 0;
#endif
		for(i=0;i<bFeeRatioNum;i++)
		{
			iEnergyFeeRatio[i] = Fmt3ToVal64(bBuf+bStartNum+4*i, 4);
			if (iEnergyFeeRatio[i] == 0)
				iEnergyFeeRatio[i] = 1;
			if (iEnergyFeeRatio[i] != 1)
				fEnergyFeeFlag = true;
		}
	}
#ifdef PRO_698	
	if (ReadItemEx(BN0, PN0, 0x015f, bBuf) > 0)
	{
		for(i=0;i<48;i++)
		{
			bEnergyFeeTime[i] = bBuf[i];
			//if(bEnergyFeeTime[i]>47)  //Old 376.1
			if(bEnergyFeeTime[i]>11)   //New 376.1
				bEnergyFeeTime[i] = 0;
		}
	}
#else
	if (ReadItemEx(BN0, PN0, 0x015f, bBuf) > 0)
	{
		for(i=0,n=0;i<48;i+=2,n++)
		{
			bEnergyFeeTime[i] = (bBuf[n]&0x0f);
			bEnergyFeeTime[i+1] = ((bBuf[n]&0xf0)>>4);
		}
	}
#endif
	GetCurTime(&now);
	minutes=now.nHour*60+now.nMinute;
	n=minutes/30;
	*piCurFeeRatio = iEnergyFeeRatio[(bEnergyFeeTime[n])];
	
	return fEnergyFeeFlag;
}
