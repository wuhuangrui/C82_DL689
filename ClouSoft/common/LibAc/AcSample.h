/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcSample.cpp
 * 摘    要：本文件对73360采集到的样点进行计算,算出有效值,功率,功率因数,频率,电能,相角等
 * 当前版本：1.1
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/

#ifndef ACSAMPLE_H
#define ACSAMPLE_H
#include "syscfg.h"
#include "apptypedef.h"
#include "stdlib.h"
#include "math.h"
#include "sysarch.h"
//#include "Sample.h"
#include "Energy.h"
#include "Demand2.h"
#include "AcFmt.h"

#ifdef SYS_LINUX
	#include "filter2.h"
	#include "Att.h"
#else	//SYS_VDK
	#include <filter.h>
#endif

#define CT_NUM      64
#define CT_AVGNUM   15


typedef struct {
	WORD wPoint;  	//测量点
	WORD wRate;   	//当前的费率,可即时刷新
	DWORD dwConst; 	//脉冲常数
	BYTE bConnectType; //终端接线方式 1	1:单相;3:三项三线;4:三相四线
	DWORD dwUn;		   //额定电压,格式NNNNN.N
	DWORD dwIn;		   //额定电流,格式NNN.NNN
	BYTE bEpMode;	   //有功电能累加模式
					   //		D2			|		D1			|			D0			|
					   // 反向计入正向 		| 	反向不计量		| 		各相累加模式	|
					   // 0:不计入;1计入	| 0:计量;1不计量	| 0:代数和;1绝对值和	|
					   // D2D1=00:正->正,反->反;正反向分别计量
					   // D2D1=01:正->正;		只计正向,反向不计
					   // D2D1=10:正反->正,反->反;
					   // D2D1=11:正反->正		反向不计
	BYTE bCombEpMode;	//组合有功的计量方式
						//		D3			|		D2			|			D1			|			D0
						//		反向减		|	反向加			|			正向减		|		正向加
	BYTE bAEqMode;	   //正向无功电量累加标志,D7D6,D5D4,D3D2,D1D0,分别对应4~1象限无功累加标志,
					   //低位:1-加; 0-减;	
					   //高位:1-计算; 0-不计算
	BYTE bREqMode;	   //反向无功电量累加标志,D7D6,D5D4,D3D2,D1D0,分别对应4~1象限无功累加标志,
					   //低位:1-加; 0-减;	
					   //高位:1-计算; 0-不计算
	WORD wAutoDate[AUTO_DATE_NUM];    //自动抄表日,高字节表示抄表日,低字节表示抄表日的小时,可即时刷新
	WORD wDayFrzTime;  //日结算时刻,小时,BIN,0xffff表示不冻结,
	BYTE bAngleClockwise;	//角度方向,0表示角度按照逆时针方向表示,Ua,Ub,Uc分别为0,240,120
							//		   1表示角度按照顺时针方向表示,Ua,Ub,Uc分别为0,120,240
	bool fCalcuHarmonic;	//是否计算谐波
	WORD wHarmNum;			//谐波计算次数
	DWORD dwHarmonicIstart;//谐波计算启动电流
	WORD wEnergyID[];
	WORD wDemandID[];
	TEnergyPara EnergyPara; //电能参数
	TDemandPara	DemandPara; //需量参数
}TAcPara;	//交采参数,某些字段修改后电能计算程序程序可即时刷新


bool AcLoadPara(WORD wPn, TAcPara* pAcPara);

class CAcSample{
	friend void CalcuDcValue();
	
public:
	CAcSample();
	virtual ~CAcSample();

    int m_iValue[SCN_NUM+1];  //有效值 +6零序电流liuzx 20161202
    int m_iBaseVal[SCN_NUM];	//基波电压、电流
    int m_iP[4];     //分别是A,B,C和总
    int m_iQ[4];     //分别是A,B,C和总
    int m_iCos[4];   //分别是A,B,C和总
    int m_iFreq;
	int m_iAngle[SCN_NUM+1];//增加1路零序电流
	int m_iVal[AC_VAL_NUM];
	bool m_fDateTimeChg;//校时标志

	void Calcu();     		//每周波调用计算一次,算出有效值,功率,功率因数,频率,电能
	bool Init(WORD wPn);    //初始化内部成员
	void ReInit();
    void Transfer();  //数据转换到数据库中

	bool TrigerAdj(BYTE* bBuf);
	bool IsPowerOff();   //交流采样检测到调电了
    WORD GetPhaseStatus();
	void SaveLog();
	bool ClearLog();
	void ClearDemand(); //清需量标志,别的线程设置,交采线程去执行
	void DateTimeChg(void);//时间变化处理
	bool GetChipFaultStatus();
	bool SetPulseRatio(BYTE* pbRx);
	BYTE GetPulseRatio(void);
	
private:
	WORD m_wPn;
	WORD m_wFreq;	//放大1000倍
	bool m_fParaChg;	
	TAcPara m_AcPara;	
	
	//参数配置
	DWORD m_dwLastAutoDate[AUTO_DATE_NUM];//上次结算日(上次月转存时间)
	DWORD m_dwLastDayFrzTime;//上次冻结时刻(上次日转存时间)
	DWORD m_dwCTRatio;
	DWORD m_dwPTRatio;
	DWORD m_dwPowerRatio;
	DWORD m_dwUn;
	DWORD m_dwIn;

	
	//频率跟踪
	DWORD m_dwFreqPnts;  //频率跟踪FREQ_CYC_NUM个周期(标准每周期NUM_PER_CYC点)点数 * FREQ_UNIT
	
	//电能及采样计算
	int64 m_iBarrelEp[4];	//有功累计到A,B,C,总有功电能累计的桶
	int64 m_iBarrelEq[4]; //无功累计到A,B,C,总无功电能累计的桶
	int64  m_iSigmaEP[4];  	//计算有功电能和功率用到的累加值,为了保留更多的有效位,且不至于大电流电压的时候溢出,所以用64位long long
	int64  m_iSigmaEQ[4];     //计算无功电能和功率用到的累加值

#ifdef SYS_LINUX
	bool m_fATT7022;
	CAtt m_Att;
	WORD   m_wQuad[4];			//象限 驱动库未转换顺序，使用0123
#else //SYS_VDK
	//频率跟踪
	WORD m_wFreqPtr;     //频率跟踪的计算指针
	WORD m_wZeroCnt;     //频率跟踪已经跟踪到的过零点数
	WORD m_wFreqPntCnt;  //频率跟踪的参与计算频率两个过零点间的点数
	WORD m_wZeroPntCnt;	 //频率跟踪的相邻两个过零点间的点数
	WORD m_wFreqCn;      //用来计算频率跟踪的通道只能是0,1,2即Ua,Ub,Uc
	bool m_fPrePos;      //频率跟踪的上一个样点是正数
	short m_sZero1[3];   //过零点1,分别存放:正、负、位置索引
	short m_sZero2[3];   //过零点2,分别存放:正、负、位置索引
	WORD  m_wFreqRstCnt;

	int64  m_dwSigmaValue[SCN_NUM];	//计算有效值用到的累加值
	int  m_iDcValue[SCN_NUM];		//每个通道的直流分量
	int  m_iDcSum[SCN_NUM];
	WORD m_wSigmaPtr;				//使用SIGMA算法的指针
	WORD m_wShiftPtrQ;			    //计算无功时电压相对于电流往前移相90度的指针
	int  m_iShiftFracP[3];			//有功相位校正的小数部分
	int  m_iShiftFracQ[3];			//无功90度移相及相位校正的小数部分
	WORD m_wSigmaPntCnt;			//SIGMA累计的点数
	int  m_iFracP[3];				//相位校正角度	
	int  m_iAdj[SCN_NUM];         	//校正参数	
	//bool m_fPhaseAdj;				//相位校正标志
	WORD m_wAdjStep;
	//WORD m_wAdjUn;
	DWORD m_dwAdjU[3];
	DWORD m_dwAdjI[3];
	DWORD m_dwAdjP[3];
	DWORD m_dwAdjS[3];
	WORD m_wShfitI[3];
	
	DWORD m_dwPosSum;
	WORD m_wCalcuPtr;  //当前计算指针
	WORD m_wCycPntNum;
	WORD m_wFftInterv;   //在计算FFT抽取样点时用到的点间隔(扩大FREQ_UNIT倍)
	WORD m_wFftCycCnt;	 //用来进行每秒计算一次FFT的周期计数
	
	//溢出检测	
	short m_sLastSample[SCN_NUM];
	WORD  m_wOverCnt[SCN_NUM];
	WORD  m_wMaxOver[SCN_NUM];
	
	WORD  m_wQuad;			//象限	
	
#endif //SYS_VDK
	
	//谐波检测	
	WORD m_wHarPercent[HARM_NUM_MAX*SCN_NUM];	//谐波含有率
	WORD m_wHarVal[HARM_NUM_MAX*SCN_NUM];		//谐波有效值

	TDataItem m_diAngle;
	//CT检测
	unsigned short m_wCtValue[3][CT_NUM];   
	WORD           m_wCtPtr;
	unsigned short m_wCtAvg[3][CT_AVGNUM];  
	WORD           m_wCtAvgPtr;

	//电能表
	CEnergy 	m_Energy;    //多费率电能
	CDemand2    m_Demand;	 //需量
	//TRatePeriod m_RatePeriod[RATE_PERIOD_NUM];  //电表的时段费率定义
	WORD   m_wRate;
	int    m_iPulse[ENERGY_NUM_MAX];  	//各的电能脉冲数
	int    m_iLastPulse[ENERGY_NUM_MAX];  //上次计算时的电能脉冲数
	BYTE   m_bEnergyMinute;
	int    m_iPe;    //用来计算电能的三相总有功
	int    m_iQe;	 //用来计算电能的三相总无功	
	int64  m_iEPerPulse;
	int    m_iS[4];   //分别是A,B,C和总
	
	WORD m_wSlideNum;      //一个需量周期内的滑差时间的个数
	WORD m_wDemandPeriod;  //需量周期
	WORD m_wSlideInterv;   //滑差时间
	TDataItem m_diDemandPeriod; //需量周期
	TDataItem m_diSlideInterv;  //滑差时间
	DWORD m_dwLastDateMin[AUTO_DATE_NUM];		//上次抄表日执行的分钟
	bool m_fDateAdjBackward[AUTO_DATE_NUM];	//抄表日执行时间往前调整
	DWORD m_dwDemandPulse[DEMAND_NUM_MAX];
	DWORD m_dwDemandTick[DEMAND_NUM_MAX];
	DWORD m_dwLastTick;
	bool m_fPowerOff;   //交流采样检测到调电了

	//测量点状态字及与相序有关的异常
	bool m_fDisOrderU; 
	bool m_fDisOrderI;
	BYTE m_bCurPolar;
	BYTE m_bPhaseStatus;
    WORD m_wPnStatus;
	
	bool m_fTrigerSave;	//在费率、月份或抄表日发生切换的时候，触发数据库去保存本测量点的数据
	bool m_fStopSaveLog;
	bool m_fClrDemand;	//清需量标志,别的线程设置,交采线程去执行



#ifdef SYS_LINUX
	void CopyToFftBuf();
	void AddPulse();
	void UpdateAcValue();
	
#else //SYS_VDK
	//频率跟踪
	void GetZero(short* psZero, WORD wFreqPtr);
	int CalcuFreq(short* psZero1, short* psZero2, WORD wFreqPntCnt);
	void ResetFreqSync();
	void FreqSync(WORD wEndPtr);
	
	void UpdateFreq();
	void CopyToFftBuf();
	unsigned short CalcuValue(complex_fract16* cplx);
	int CalcuP(complex_fract16* cplxI, complex_fract16* cplxV);
	int CalcuQ(complex_fract16* cplxI, complex_fract16* cplxV);
	void SaveAdj();
	void InitAdj();
	void PhaseAdjust();
	void AdErrCheck(bool fCaluTimeout);
	
	void ChannelOverflow(WORD wCn, short sSample);
#endif

	
	void CalcuS();
	long CalcuCos(long P,long Q);
	void CalcuAngle();
	int CalcuZero();
	void RunMeter();
	void PwrToPulse();
	int PwrToEnergy(int iPwr, WORD wPnt, int64& riBarrelE);
	
	void LoadPara();
	
	void CaluPhaseStatus();
	bool CaluDisOrder34(int* piAngle);
	bool CaluDisOrderU33(int* piAngle);
	bool CaluDisOrderI33(int* piAngle);
	BYTE CalCurPolar(int* piAngle, BYTE bConnectType);
	
	//谐波含量计算
	void DoHarmonic();
	unsigned short CalcuTotalHarmonic(complex_fract16* cplx, int iBaseHarVal, int* piBasePercent);
	unsigned short CalcuHarmonic(complex_fract16* cplx, int total, double dGainFactor);
	void HarmonicAntiJitter(void);
};

void AcCalcu();     		//每周波调用计算一次,算出有效值,功率,功率因数,频率,电能
bool AcInit(WORD wPn);    //初始化内部成员
bool AcTrigerAdj(BYTE* bBuf);
bool AcIsPowerOff();   //交流采样检测到调电了
void AcSaveLog();
bool AcClearLog();
void AcClearDemand();
#ifdef SYS_VDK
WORD* AcGetMaxOver();
#endif
void AcDateTimeChg();//校时or时间发生变化时调用，强制判断是否需要费率切换及电量入库
bool AcSetPulseRatio(BYTE* pbRx);
BYTE AcGetPulseRatio(void);

bool AcGetFaultStatus();//获取芯片故障状态

#endif //ACSAMPLE_H
