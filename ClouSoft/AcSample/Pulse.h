#ifndef PULSE_H
#define PULSE_H

#include "AcConst.h"
#include "Energy.h"
#include "Demand2.h"

#define IC12					12
#define OI_PULSE_BASE			0x2401
#define OI_PULSE_INPUT			0xF20A	//脉冲输入设备
#define OI_YX					0xF203

#define PULSE_HI_POSE_ID	0x2414	//脉冲正向有功高精度电能ID

#define PULSE_PN_NUM		8		//脉冲计量点个数

#define TICK_BUF_LENGTH 	2000
#define PUSLE_ADDR_LEN		16		//通信地址长度
#define PULSE_CFG_LEN		12
#define PULSE_CFG_ID_LEN	(PULSE_CFG_LEN*MAX_PULSE_TYPE + 2)	//冻结类关联属性表ID长度


//在添加1个脉冲配置单元中的偏移
#define OFFSET_PULSE_PORT				3		//脉冲输入端口OAD
#define OFFSET_PULSE_TYPE				8		//脉冲属性偏移
#define OFFSET_PULSE_CONST				10		//脉冲常数	


// 冻结接口类方法
#define OMD_PULSE_RESET					OMD1					//复位
#define OMD_PULSE_RUN					OMD2					//执行
#define OMD_PULSE_ADDCFG				OMD3					//添加脉冲输入单元
#define OMD_PULSE_DELCFG				OMD4					//删除一个脉冲输入单元

typedef struct
{
	DWORD	Pulse;
	short	WritePtr;
	DWORD	TickBuf[TICK_BUF_LENGTH];
}TPulseInData;

typedef struct
{
	WORD wPn;		//该路脉冲所属测量点号
	BYTE bPortNo;	//输入端口号
	BYTE bType;		//脉冲属性
	int64 i64Const;	//电表常数
}TPulseCfg;			//脉冲配置

typedef struct
{
    WORD  wPn;			//所属测量点号
    //bool  fPosEpValid;	//正向有功脉冲有效
    //bool  fPosEqValid;	//正向无功脉冲有效
    //WORD  wAutoDate[AUTO_DATE_NUM];
	//WORD  wRate;   		//当前的费率,可即时刷新
    //int64 i64PT;
	//int64 i64CT;
	//int64 iPowerRatio;	
	//TPulseCfg PulseCfg;
	TEnergyPara EnergyPara; //电能参数
	//TDemandPara	DemandPara; //需量参数
	BYTE		bAddr[PUSLE_ADDR_LEN];	//通信地址
}TPulsePara;		//脉冲参数

typedef struct{
	WORD wPn;	//测量点号
	BYTE bPortNo[MAX_PULSE_TYPE];	//输入端口号
	BYTE bIndex[MAX_PULSE_TYPE];	//脉冲序号
	bool fPosEpValid;	//正有有效
	bool fPosEqValid;	//正无有效
}TPulsePnDesc;	//脉冲测量点描述


bool PulseLoadPara(TPulseCfg* pPulseCfg, TPulsePara* pPulsePara);

class CPulse {	//脉冲类 (单路脉冲)
public:
	CPulse();
	virtual ~CPulse();

	bool Init(TPulseCfg* pPulseCfg);
	bool LoadPara();
	void Run(bool fPower);
	int64 CurPower() {return m_i64Power;};
	WORD  GetPn() { return m_wPn; };
	
	bool IsValid() { return m_fValid; };
	void  SetValid(bool fValid) { m_fValid = fValid; };
	
	void   SaveLog();
	bool   ClearLog();
	bool   ClearLogBlock(BYTE bPortNo);
	bool   ResetData();

private:
	WORD m_wPn;
	WORD m_wRate;

	TPulseCfg m_PulseCfg;
	TPulsePara m_PulsePara;

	CEnergy    m_Energy;    //多费率电能
	//CDemand2   m_Demand;	 //需量

	BYTE   m_bEnergyMinute;
	int64  m_i64Power;	//实时功率
	WORD   m_wPowerPtr;	
	DWORD  m_dwPulse[ENERGY_NUM_MAX];  	//各的电能脉冲数
	DWORD  m_dwLastPulse[ENERGY_NUM_MAX];  //上次计算时的电能脉冲数
	DWORD m_dwLastDateMin[AUTO_DATE_NUM];		//上次抄表日执行的分钟
	bool m_fDateAdjBackward[AUTO_DATE_NUM];	//抄表日执行时间往前调整

	bool   m_fValid;
	DWORD  m_dwLastAutoDate[AUTO_DATE_NUM];
	bool   m_fTrigerSave;	//在费率、月份或抄表日发生切换的时候，触发数据库去保存本测量点的数据
	//bool   m_fClrDemand;	//清需量标志,别的线程设置,交采线程去执行
	bool   m_fStopSaveLog;
	bool   m_fPowerUp;

	

	void   RunMeter();
	void   CalPower();	
};


class CPulseManager {	//脉冲管理类
	public:
		CPulseManager();
		virtual ~CPulseManager();
		
		bool Init();
		void CalcPwr();
		void CalcEnergy();
		void Run();
		BYTE GetPulseNum() { return m_bPulseNum; };
		BYTE GetPulsePnNum() { return m_bPulsePnNum; };
		WORD GetPulsePn(BYTE bPnIndex) { return m_PulsePnDesc[bPnIndex].wPn; };
		BYTE GetYMFlag() { return m_bYMFlag; }
		void SaveLog();
		void ClearLog();
		bool ResetPulseData();
		bool IsPulsePnInvalid(TPulsePnDesc* pPnDesc);	//该测量点脉冲是否有效
		void CalcStatEnergy();	//计算所有脉冲测量点统计数据
		void CalcPnStatEnergy(WORD wPnIndex, bool fClrDayEnergy, bool fClrMonthEnergy);		//计算/清0某个测量点的统计数据

	private:
		BYTE m_bYMFlag;			//脉冲占用遥信标志 1：占用， 0：不占用
		BYTE m_bPulseNum;		//脉冲总路数
		BYTE m_bPulsePnNum;		//脉冲总测量点数
		DWORD m_dwLastTick;
		DWORD m_dwLastStatClick;

		int64 m_i64LastE[MAX_PULSE_TYPE][RATE_NUM+1];	//上一次电能量（2419~241c）
	
		TPulsePnDesc m_PulsePnDesc[PULSE_PN_NUM];		//脉冲测量点描述		
		
		CPulse m_Pulse[MAX_YMNUM];		//各路脉冲对象
};


extern TPulseInData g_PulseInData[];
extern CPulseManager g_PulseManager;	//脉冲管理类

int OnResePulseCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int BatAddPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);
int OnRunPulseCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnAddPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnDelPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
#endif
