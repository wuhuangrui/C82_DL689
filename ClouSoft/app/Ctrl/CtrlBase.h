/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CtrlBase.h
 * 摘    要：本文件主要实现CCtrl类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
 ----------------------------------------------------------------------------------------------------------
--2008-04-09 17:45
岑工修改意见.
保电控投入时
1 如当前有遥控跳闸投入命令,删除该命令,并将所有遥控闸复位.
2 如当前有电控投入,将所有的控制状态复位,但并不解除投入状态.
3	如是购电控,将继续计算剩余电量.
4 如当前有功控投入,将所有的控制状态复位,但并不解除投入状态.

疑问：
1 临时下浮控的当前功率定值计算是否在保电解除后重新计算？
2 如果 Do()
{
	if (!IsValid())
	{
		ResetCtrl();
		return true;
	}

	return DoCtrl();
}

还按照这种结构来做,就比较难处理 购电控 在保电后还继续计算剩余电量的情况

二类数据 F65 F66 需要我这里来统计
在和岑工他们讨论后,确认,无论功控或电控,只要有有效的上限值就需要进行超限统计.

IsCtrlValid()
IsCmdValid()

--2008-04-10 08:48
岑工修改意见.
1. 临时下浮控的在保电解除后也解除.
2. 状态的判断,统计,放到DoCtrl()里面完成.
3. 当月电量定值(月电量定值*(1+月电量浮动系数))

--2008-04-17 1601
咨询潘工后,对2类数据F50,F52,F65,F66的统计应和相应的控制投入没有关系,根据数据时间的标定单位进行更新,如是日
数据,在换日后应进行清零,如是月数据,在换月后应进行清零.
*********************************************************************************************************/

#ifndef CTRL_H
#define CTRL_H

//所有轮次,当 TURN_NUM == 4, CTL_TURN_MASK == 00001111B
#define CTL_TURN_MASK				((BYTE)((1<<TURN_NUM)-1))

//所有轮次,当 GRP_NUM == 8, CTL_GRP_MASK == 11111111B
#define CTL_GRP_MASK				((BYTE)((1<<GRP_NUM)-1))

#define CTL_POWERON_LOCKTIME		(10*60)		//闸输出上电锁定时间(默认10分钟)<单位:秒>.
//#define CTL_POWERON_LOCKTIME			30		//该值仅在调试时使用.

#define CTL_POWER_ALR_MIN_TIME		(1*60)		//功控报警持续时间(最少时间)<单位:秒>

#define CTL_TMPCTRL_SLIP_INTERVAL	(1*60)		//临时下浮控时,计算滑差平均功率作为功控定值时的实时功率采样间隔.

#define MAX_CTLSTAT_ARRAY_NUM		10*GBC4_MAXSUMGROUP			//最大控制状态array个数
//Control Type
#define CTL_GUARANTEE			0
#define CTL_YkCtrl				1
#define CTL_ENG_MONTH			2
#define CTL_ENG_BUY				3
#define CTL_PWR_TMP				4
#define CTL_PWR_SHUTOUT			5
#define CTL_PWR_REST			6
#define CTL_PWR_PERIOD			7

#define CTL_YkCtrl_CLOSE			8
#define CTL_ENG_MONTH_CLOSE			9
#define CTL_ENG_BUY_CLOSE			10
#define CTL_PWR_TMP_CLOSE			11
#define CTL_PWR_SHUTOUT_CLOSE		12
#define CTL_PWR_REST_CLOSE			13
#define CTL_PWR_PERIOD_CLOSE		14

#define CTL_PWR_TMP_ALLCLOSE			15
#define CTL_PWR_SHUTOUT_ALLCLOSE		16
#define CTL_PWR_REST_ALLCLOSE			17
#define CTL_PWR_PERIOD_ALLCLOSE		18

#define CTL_TURNCLOSE_TICK           30     //合闸状态在系统库中停留的秒数；

//保电状态类型
#define QUIT_GUARANTEE			0
#define INPUT_GUARANTEE			1
#define AUTO_GUARANTEE			2

//控制的两大主类定义
#define PWR_CTL		0	//功控
#define ENG_CTL		1	//电控

extern bool GetBitStatus(BYTE* pbBuf, WORD wLen, WORD wIdx);	//获取某个缓冲区指定位的状态.
extern bool SetBitStatus(BYTE* pbBuf, WORD wLen, WORD wIdx, bool fStatus);	//设置某个缓冲区指定位的状态.
extern DWORD TranDataFmt19(BYTE* pb);	//转换数据格式19.
extern DWORD TranDataFmt20(BYTE* pb);	//转换数据格式20.
extern int GetIdxOfMostRight1(BYTE bFlgs);	//获取字节中最左边1的位置(0 ~ 7).
extern int GetIdxOfMostLeft1(BYTE bFlgs);
extern int GetIdxOfAll1InPst(BYTE bFlgs, int iIdx);	//获取指定位置在所有右边的1中的位置编号(0 ~ 7).
extern int GetSumOf1(BYTE bFlgs);		//获取所有1的个数.
extern int64 GetValidCurPwr(int iGrp);	//获取指定总加组有效当前有功功率.
extern int64 GetCurPwr(int iGrp);		//获取指定总加组的当前有功功率.
extern int64 GetSelEng(int iGrp, int iSel);	//获取指定总加组指定类型正向有功总电能.
extern bool IsGuarantee(void);						//判断是否处在保电状态.

//描述: 获取指定总加组当前已用正向有功总电能.
//参数:@iGrp	要获取的总加组.
//返回: 当前已用正向有功总电能.
inline int64 GetCurEng(int iGrp)
{
	return GetSelEng(iGrp, 0);
}

typedef struct
{
	bool	fIfOverLimit;		//是否超限.
	DWORD	dwClick;				//超限起始时间.
	int		iGrp;				//超限的总加组.
	int64	iEng;				//超限时的当前电量.
} TPwrOverLimitStat;

typedef struct
{
	bool	fIfOverLimit;		//是否超限.
	DWORD	dwClick;				//超限起始时间.
	int		iGrp;				//超限的总加组.
	int64	iEng;				//超限时的当前电量.
} TEngOverLimitStat;

typedef struct
{
	BYTE bCtrlType;           //告警种类；
	BYTE bCtrlTurn;			  //告警轮次；
} TCtrl;

//描述：将倒计时时间秒数转换成秒、分（BCD码）
//参数：@dwCountDown  倒计时时间秒数；@pbBuf-存储倒计时时间（秒，分）的缓冲区；
//返回：倒计时秒数对应的*分*秒
inline void CntDownToFmt(DWORD dwCntDown, BYTE* pbBuf)
{
	DWORD dwMin = dwCntDown / 60;
	pbBuf[1] = ByteToBcd((BYTE )dwMin);   //分；
	pbBuf[0] = ByteToBcd((BYTE )(dwCntDown - dwMin*60)); //秒；
}

//========================================== CCtrlBase =============================================
class CCtrlBase
{
public:
	CCtrlBase(void){}
	virtual ~CCtrlBase(){}

	virtual bool Init(void) = 0;				//初始化.
	virtual bool DoCtrl(void) = 0;				//运行控制.

protected:
	//本类中,使用了两个静态时标成员变量,之所以定义成静态成员变量,是因为在实际使用时,有多个
	//控制类都将基于这时标进行操作,为了在时间上实现精确的同步,因此在基类中定义这两个变量,
	//这连个变量都表示相同的时间,而之所以定义两个,是因为在代码中频繁交替地使用了两种格式的
	//时间值,而这两种格式转换起来又非常地不便且耗时,因此设计在LoadCtrl.DoCtrl()中转换一次,
	//以提高系统效率.
	static TTime	m_tmNow;					//当前时间.
	static DWORD	m_dwNow;					//当前时间.

	static TTime	m_tmOldTime;				//上次执行的时间.
	static DWORD	m_dwOldTime;				//上次执行的时间.
};

//============================================ CCtrl ===============================================
class CCtrl : public CCtrlBase
{
public:
	CCtrl(void) : m_fCtrlValid(false){}
	virtual ~CCtrl(){}

	virtual bool Init(void);					//初始化.
	bool IsValid(void)	{ return m_fCtrlValid; }; //当前控制是否投入.
	bool IsInCtrl(void)	{ return (m_fCtrlValid && m_fInCtrl); }; 	  //该控制是否处于控制状态
	virtual void DoCmdScan(void) = 0;			//扫描系统库中的命令.

protected:
	virtual void RstCtrl(void) = 0;				//复位内存中本类控制状态量.
	virtual void ClrCmd(void) = 0;				//清除内存中本类控制的控制命令.

	//---------------------------------------------------------------
	bool SetValidStatus(bool fStatus)			//设置投入状态(<ture>:投入; <false>:未投入).
	{
		return (m_fCtrlValid = fStatus);
	}
protected:
	bool m_fCtrlValid;	//控制投入状态.
	bool m_fInCtrl;		//该控制是否处于控制状态
	bool m_fGuarantee;	//该控制是否已经处于保电状态,主要用来判断保电的切换
};

//========================================= CGrpCtrl ===============================================
class CGrpCtrl : public CCtrl
{
public:                             			
	CGrpCtrl();
	virtual ~CGrpCtrl(){}
	void DoCmdScan(void);							//扫描系统库各总加组的本类控制命令.

	int GetGrp(void)								//获取当前总加组.
	{
		return m_iGrp;
	}
	BYTE GetTurnsStatus(void)						//获得轮次状态.
	{
		return m_bTurnsStatus;
	}
	
	bool IfOverLimit(void)							//是否超限.
	{
		return m_fIfOverLimit;
	}
	WORD GetOpenTimes(void)							//获取跳闸次数,然后清零跳闸次数.
	{
		WORD w = m_wOpenTimes;

		m_wOpenTimes = 0;

		return w;
	}

	void MakeDisp(BYTE bTurnsStatus);              //显示生成函数；
protected:
	virtual char* CtrlType(char* psz) = 0;			//获得控制的类型(返回控制类型的字符串描述).
	virtual int CtrlType(void) = 0;					//获得控制的类型(返回整数类型).

	virtual int NewCmdAct(void) = 0;				//获取新命令的动作码.
	virtual DWORD NewCmdTime(void) = 0;				//获取新命令的接收时间.
	virtual int CurCmdAct(void) = 0;				//获取当前命令的动作码.
	virtual DWORD CurCmdTime(void) = 0;				//获取当前命令的接收时间.

	virtual bool GetSysCmd(int iGrp) = 0;			//获取系统库指定总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
	virtual void SaveNewCmd(void) = 0;				//将新命令(m_NewCmd)保存到m_CtrlCmd中.(注意: 对不同的类,m_CtrlCmd的结构是不同的)
	virtual bool ClrSysCmd(int iGrp) = 0;			//清除系统库指定总加组本类控制命令.

	virtual BYTE GetSysCtrlTurnsCfg(int iGrp) = 0;	//获取系统库指定总加组本类控轮次配置状况.
	virtual bool GetSysCtrlFlg(int iGrp) = 0;					//获取系统库指定总加组本类控制的状态标志.
	virtual bool SetSysCtrlFlg(int iGrp, bool fStatus) = 0;		//设置系统库指定总加组本类控制的状态标志.
	virtual bool RstSysCtrlStatus(int iGrp) = 0;	//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入状态标志,报警状态标志等等).

	//显示函数；
	virtual bool IsAlarmStatus() = 0;
	virtual BYTE GetCtrlType() = 0;
	virtual BYTE GetInvCtrlType() = 0;

	//---------------------------------------------------------------
	BYTE GetSysCtrlTurnsCfg(int iGrp, int iSel);						//获取系统库指定总加组指定控制类的轮次配置状况.
	BYTE GetSysCtrlFlgs(int iGrp, int iSel);							//获取系统库指定总加组指定控制类的所有标志位.
	bool ChgSysCtrlFlgs(int iGrp, BYTE bFlgs, bool fStatus, int iCtrlType);	//改变系统库指定总加组指定控制类的指定标志位状态.

	//显示函数；
	void RemoveDispItem(TCtrl tInvCtrl);
	void AddDispItem(TCtrl tTopCtrl);

	DWORD GetInitClick()
	{
		return m_dwInitClick;
	}

	bool IsOpenStatus()
	{//是否是跳闸状态；
		return (m_bTurnsStatus>0);
	}

protected:
	int				m_iGrp;							//当前总加组(负数表示没有任何总加组投入).
	int             m_iCtrlGrp;                     //当前处于控制状态的总加组
	BYTE			m_bTurnsStatus;					//当前的轮次状态.
	BYTE			m_bWarnStatus;					//当前的告警轮次.	
	bool			m_fIfOverLimit;					//是否超过限定值.
	WORD			m_wOpenTimes;					//跳闸次数.

	DWORD m_dwInitClick;				//显示合闸初始时刻；
	bool  m_fAlarmStatus;				//上周期是否处在告警状态；
	bool  m_fOpenStatus;				//上周期是否处在跳闸状态
	BYTE  m_bCloseTurn;					//合闸的轮次；
};

//========================================= CEngCtrl ===============================================
class CEngCtrl : public CGrpCtrl
{
public:                             			
	CEngCtrl(void){ m_fAlrStauts = 0; m_dwOpenTurnTime =0; };
	virtual ~CEngCtrl(){};
	virtual bool IsBeepAlr(void) = 0;			//是否声音报警.

protected:
	virtual bool GetSysStatus(void) = 0;			//用于初始化时,将系统库中本类控制的轮次状态,报警状态等同步到内存中对应的变量.

	virtual bool GetSysCtrlAlr(int iGrp) = 0;		//获取系统库指定总加组本类控制的报警状态标志.
	virtual bool SetSysCtrlAlr(int iGrp, bool fStatus) = 0;		//设置系统库指定总加组本类控制的报警状态标志.

	virtual bool SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus) = 0;			//设定系统库指定总加组本控制类的轮次状态.
	virtual bool ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus) = 0;	//改变系统库指定总加组本控制类的相应轮次状态.

protected:
	bool GetSysEngStatus(int iSel);					//用于初始化时,将系统库中指定类控制的轮次状态,报警状态等同步到内存中对应的变量.

	BYTE GetSysEngAlrFlgs(int iGrp);											//获取系统库指定总加组电能量控制类的所有报警状态标志位.
	bool ChgSysEngAlrFlgs(int iGrp, BYTE bFlgs, bool fStatus);					//改变系统库指定总加组指定控制类的指定报警状态标志.

	BYTE GetSysCtrlTurnsCfg(int iGrp)											//获取指定总加组本类控轮次配置状况.
	{
		return CGrpCtrl::GetSysCtrlTurnsCfg(iGrp, 1);
	}
	bool SetSysEngTurnsStatus(int iGrp, BYTE bTurnsStatus, int iSel);			//设定系统库指定总加组指定控制类的轮次状态.
	bool ChgSysEngTurnsStatus(int iGrp, BYTE bTurns, bool fStatus, int iSel);	//改变系统库指定总加组指定控制类的相应轮次状态.
	bool RstSysCtrlStatus(int iGrp)												//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入标志等等).
	{
		return SetSysCtrlFlg(iGrp, false);
	}

	bool IsAlarmStatus()
	{
		return m_fAlrStauts;
	}
	DWORD GetEngTurnInv(int iTurn);
protected:
	bool					m_fAlrStauts;										//报警状态.
	DWORD					m_dwOpenTurnTime;									//上次跳闸时间.
};

//========================================= CPwrCtrl ===============================================
class CPwrCtrl : public CGrpCtrl
{
public:
	CPwrCtrl(void);
	virtual ~CPwrCtrl(){}
	bool IsAlr(void)
	{
		return ((m_dwAlrTime != 0) && GetCtrlType()<CTL_YkCtrl_CLOSE);
	}
	bool IsBeepAlr(void)								//是否声音报警.
	{
		return (m_dwAlrTime != 0);
	}
	
	void SumOverLimitPara(int& riGrp, DWORD& rdwTime, int64& riEng);//累加功率定值超限参数(相应总加组的新增超限时间及超限电量).

	virtual	void DoSaveOpenRec(void) = 0;							//保存跳闸记录.
	

protected:
	void DoSavePwrCtrlOpenRec(int iSel);							//保存功控跳闸记录.
	BYTE GetSysCtrlTurnsCfg(int iGrp)								//获取指定总加组本类控轮次配置状况.
	{
		return CGrpCtrl::GetSysCtrlTurnsCfg(iGrp, 0);
	}
	bool RstSysCtrlStatus(int iGrp)									//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入标志等等).
	{
		return SetSysCtrlFlg(iGrp, false);
	}

	int64 GetPwrSafeLimit(void);									//获取功控保安定值.
	DWORD GetPwrAlrPersistTime(int iTurn);							//获取指定轮次的功控报警持续时间.
	int64 GetPwrLimit(void)											//获取当前的功率定值,即上限.
	{
		return m_iPwrLimit;
	}

	DWORD GetPwrSlideInterv(int iGrp);

	bool SetSysCurPwrLimit (int iGrp, int64 iPwrLimit);				//设定指定总加组当前功控定值.

	bool IsAlarmStatus()
	{
		return (m_dwAlrTime > 0);	
	}
	
	void SaveDisp(WORD wDelayTime, DWORD dwStime, int64 iCurPwr);
	bool RestoreTurnStatus();
	
protected:
	int64					m_iPwrLimit;							//当前的功率定值,即上限.
	DWORD					m_dwFrzDly;								//功控跳闸后功率冻结延时.
	DWORD					m_dwAlrTime;							//报警启动的时间.
	DWORD					m_dwRstTime;							//跳闸恢复启动的时间.	
	DWORD					m_dwGuaranteeAlrTime;							//保电状态下报警启动的时间.

	int64					m_iCurPwrLimit;							//当前功率定值.

	TPwrOverLimitStat		m_OLStat;								//超限统计.

	DWORD				m_dwPwrStartClick;	//记下功率开始计算的时间,0表示之前没投入,从没投入转为投入,要等待功控滑差时间才能取功率
};

class CAllPwrCtrl;	//用于提供给'临时下浮控','营业报停控','厂休控','时段控'声明友元.

#endif  //CTRL_H
