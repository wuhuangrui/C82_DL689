/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TermEvtTask.h
 * 摘    要：本文件主要实现DL/T 698.45 的事件
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年10月
 *********************************************************************************************************/
#ifndef TERMEVTTASK_H
#define TERMEVTTASK_H 

#include "DbConst.h"
	
//地方版本
#define TERM_STD					0								//国网标准版本
#define TERM_ZJ						1								//浙江版本
#define TERM_VER					TERM_ZJ//TERM_ZJ							//版本选择,浙江



//事件相关宏定义
#define DEFAULT_DELAY_SEC			60								//默认判定延时60秒
#define SYSDB_ITEM_MAXSIZE			254								//系统库数据项最大长度254
#define EVTREC_MAXSIZE				1024//512								//一笔记录的最大长度
#define MEMORY_EVTREC_SIZE			EVTREC_MAXSIZE					//申请的整笔空间的最大长度
#define EVT_ATTRTAB_LEN				(5*CAP_OAD_NUM+2)				//关联属性表长度
#define EVT_ATRR_MAXLEN				50								//事件属性数据最大长度，不包括关联属性表。最大是IC24的当前值记录表50个字节。
#define EVT_SRC_LEN					19								//事件发生源最大长度 18
#define TERM_PRG_OAD_NUM			10								//编程对象列表OAD最大个数,会不会少了?
#define TERM_PRG_LIST_LEN			(2+TERM_PRG_OAD_NUM*5)			//编程对象列表表长度
#define EVT_CLR_OMD_NUM				10								//事件清零列表OMD最大个数,会不会少了?
#define EVT_CLR_LIST_LEN			(2+EVT_CLR_OMD_NUM*5)			//事件清零列表长度
#define EVT_CLR_VALID				0x5A							//清零有效标志值
#define EVT_CLR_ID					0x0b12							//事件清零标识数据库ID，需要与DbCfg.cpp中的定义匹配
#define EVT_TRIG_PARA_LEN			32								//事件触发参数长度
#define EVT_TRIG_ID					0x0b13							//事件触发冻结命令数据库ID，需要与DbCfg.cpp中的定义匹配，数据存放顺序同入参，前EVT_SRC_LEN字节为IC7的事件发生源/IC24事件类别
#define CN_RPT_NUM					3								//最大上报通道个数
#define CN_RPT_STATE_LEN 			9								//通道上报状态struct的长度
#define CN_RPT_TOTAL_LEN			(2+CN_RPT_NUM*CN_RPT_STATE_LEN)	//事件上报状态"array 通道上报状态"数据总长度
#define EVT_UPDATE_CYC				1								//事件数据刷新周期2秒
#define EVT_ADDOAD_MAXNUM			70								//0x3200新增事件对象支持的最大个数，大于所支持的OAD个数，包括抄表事件
#define EVT_ADDOAD_MAXLEN			(5*EVT_ADDOAD_MAXNUM+2)			//0x3200新增事件对象数据长度 属性2（新增事件列表，只读）∷= array OAD
#define EVT_ADDOI_MAXLEN			(3*EVT_ADDOAD_MAXNUM+2)			//0x3200新增事件对象数据长度 属性3（需上报事件对象列表，只读）∷= array OI
#define OI_BALANCEDAY				0x4116
#define OI_FLUX						0x2200
#define SCH_MTR_SAVE_LEN			PN_MASK_SIZE					//最大支持的搜表数除以8，每位对应档桉一个配置序号。
#define SCH_MTR_SAVE_REC_NUM		10								// 3111每一笔搜表记录，允许存储的最大表搜表数
#define STEP_AREA_SAVE_REC_NUM		10								// 3112每一笔台区搜表记录，允许存储的最大表搜表数

#define ONE_SCH_MTR_RLT_LEN			56								//一个搜表结果长度

//事件接口类
#define IC24						24								//分项事件对象接口类		
#define IC7							7								//事件对象接口类

//分项事件接口类属性宏定义
#define IC24_LOGICNAME				1								//逻辑名			
#define IC24_ATTRTAB				2								//关联对象属性表	
#define IC24_CURRECNUM				3								//当前记录数		
#define IC24_MAXNUM					4								//最大记录数	
#define IC24_PARA					5								//配置参数		
#define IC24_RECORDTAB1				6								//事件记录表1	
#define IC24_RECORDTAB2				7								//事件记录表2	
#define IC24_RECORDTAB3				8								//事件记录表3		
#define IC24_RECORDTAB4				9								//事件记录表4	
#define IC24_CURRECLIST				10								//当前值记录表		
#define IC24_REPROTFLAG				11								//上报标识		
#define IC24_VALIDFLAG				12								//有效标识		
#define IC24_VLOSSSTA				13								//失压统计		

//事件接口类属性宏定义
#define IC7_LOGICNAME				1								//逻辑名			
#define IC7_RECORDTAB				2								//事件记录表		
#define IC7_ATTRTAB					3								//关联对象属性表	
#define IC7_CURRECNUM				4								//当前记录数		
#define IC7_MAXNUM					5								//最大记录数		
#define IC7_PARA					6								//配置参数			
#define IC7_CURRECLIST				7								//当前值记录表		
#define IC7_REPROTFLAG				8								//上报标识			
#define IC7_VALIDFLAG				9								//有效标识			

//事件接口类方法
#define EVT_RESET					1								//复位
#define EVT_RUN						2								//执行
#define EVT_TRIG					3								//触发一次记录
#define EVT_ADDATTR					4								//添加一个事件关联对象属性 
#define EVT_DELATTR					5								//删除一个事件关联对象属性


//事件OI
#define MTR_VLOSS					0x3000							//失压					1
#define MTR_VLESS					0x3001							//欠压					2
#define MTR_VOVER					0x3002							//过压					3
#define MTR_VBREAK					0x3003							//断相					4
#define MTR_ILOSS					0x3004							//失流					5
#define MTR_IOVER					0x3005							//过流					6
#define MTR_IBREAK					0x3006							//断流					7
#define MTR_PREVERSE				0x3007							//潮流反向				8
#define MTR_POVER					0x3008							//过载					9
#define MTR_PDMDOVER				0x3009							//正向有功需量超限		10
#define MTR_RPDMDOVER				0x300A							//反向有功需量超限		11
#define MTR_QDMDOVER				0x300B							//无功需量超限			12
#define MTR_PFUNDER					0x300C							//功率因数超下限		13
#define MTR_ALLVLOSS				0x300D							//全失压				14
#define MTR_SUPLYPOWDOWN			0x300E							//辅助电源掉电			不支持
#define MTR_VDISORDER				0x300F							//电压逆相序			15
#define MTR_IDISORDER				0x3010							//电流逆相序			16
#define MTR_POWERDOWN				0x3011							//掉电					不支持
#define MTR_PROGRAM					0x3012							//编程					不支持
#define MTR_MTRCLEAR				0x3013							//电表清零					17
#define MTR_DMDCLEAR				0x3014							//需量清零				18
#define MTR_EVTCLEAR				0x3015							//事件清零				19
#define MTR_SETCLOCK				0x3016							//校时					不支持
#define MTR_DAYSTAGE				0x3017							//时段表编程			不支持
#define MTR_TIMEZONE				0x3018							//时区表编程			不支持
#define MTR_WEEKREST				0x3019							//周休日编程			不支持
#define MTR_ACOUNTDAY				0x301A							//结算日编程			不支持
#define MTR_OPENCOVER				0x301B							//开盖					不支持
#define MTR_OPENTAILOVER			0x301C							//开端钮盒				不支持
#define MTR_VUNBALANCE				0x301D							//电压不平衡			20
#define MTR_IUNBALANCE 				0x301E							//电流不平衡			21
#define MTR_RELAYLAZHA				0x301F							//跳闸					不支持
#define MTR_RELAYHEZHA				0x3020							//合闸					不支持
#define MTR_HOLIDAY					0x3021							//节假日编程			不支持
#define MTR_MIXDPEXP				0x3022							//有功组合方式编程		不支持
#define MTR_MIXDQEXP				0x3023							//无功组合方式编程		不支持
#define MTR_TARIFFPRICE				0x3024							//费率参数表编程		不支持
#define MTR_STAIRPRICE				0x3025							//阶梯表编程			不支持
#define MTR_UPDATEKEY				0x3026							//密钥更新				不支持
#define MTR_CARDABNAORMAL			0x3027							//异常插卡				不支持
#define MTR_PURCHASE				0x3028							//购电记录				不支持
#define MTR_DECREASEPURSE			0x3029							//退费记录				不支持
#define MTR_MAGNTEITCINT			0x302A							//恒定磁场干扰记录		不支持
#define MTR_SWITCHABNORMAL			0x302B							//负荷开关误动作		不支持
#define MTR_POWERABNORMAL			0x302C							//电源异常				不支持
#define MTR_ISUNBALANCE				0x302D							//电流严重不平衡		22
#define MTR_CLKERR					0x302E							//时钟故障				23
#define MTR_MTRCHIPERR				0x302F							//计量芯片故障			24
#define MTR_MODULECHANGE			0x3030							//通信模块变更事件		不支持			OAD

#define TERM_INIT					0x3100							//终端初始化事件		1
#define TERM_VERCHG					0x3101							//终端版本变更事件		2
#define TERM_YXCHG					0x3104							//终端状态量变位事件	3
#define TERM_POWOFF					0x3106							//终端停/上电事件		4			enum
#define TERM_DIGITOVER				0x3107							//终端直流模拟量越上限事件	不支持	OAD
#define TERM_DIGITUNDER				0x3108							//终端直流模拟量越下限事件	不支持	OAD
#define TERM_MSGAUTH				0x3109							//终端消息认证错误事件	5
#define TERM_DEVICEERR				0x310A							//设备故障记录			6			enum
#define TERM_FLUXOVER				0x3110							//月通信流量超限事件	7	
#define TERM_UNKNOWNMTR				0x3111							//发现未知电能表事件	8
#define TERM_STEPAREA				0x3112							//跨台区电能表事件	9
#define TERM_CLOCKPRG				0x3114							//终端对时事件		10
#define TERM_YKCTRLBREAK			0x3115							//遥控跳闸记录		11			OAD
#define TERM_EPOVER					0x3116							//有功总电能量差动越限事件记录		12
#define TERM_OUTPUTSTACHG			0x3117							//输出回路接入状态变位事件记录		不支持
#define TERM_TERMPRG				0x3118							//终端编程记录			13
#define TERM_CURCIRC				0x3119							//终端电流回路异常事件	14			enum
#define TERM_ONLINESTACHG			0x311A							//电能表在网状态切换事件不支持
#define TERM_MTRCLKPRG				0x311B							//终端对电表校时记录	15			TSA
#define TERM_POWCTRLBREAK			0x3200							//功控跳闸记录			16			OI
#define TERM_ELECTRLBREAK			0x3201							//电控跳闸记录			17			OI
#define TERM_PURCHPARACHG			0x3202							//购电参数设置记录		18			OI 				
#define TERM_ELECTRLALARM			0x3203							//电控告警事件记录		19			OI

#define EVT_TOTAL_NUM				0x48							//以上定义的事件总数，用于事件清零和事件触发方法。若有添加新事件，需要累加
#define INMTR_IC24EVT_NUM			0x0C							//内表IC24事件总数。IC24实际只有10个，但按0x300B来做定义，因此为0x0C
#define INMTR_EVT_NUM				0x31							//内表事件总数

#define PWR_OFF_RUN_CNT				8								//停电事件发生或恢复时保持的循环次数

//当前事件状态定义pEvtCtrl->pEvtBase[bItem].bJudgeState
#define EVT_JS_FORCE_END			0								//强制结束状态
#define EVT_JS_HP					1								//事件自然发生状态
#define EVT_JS_END					2								//事件自然结束状态


//事件状态机定义
#define EVT_S_BF_HP					1								//发生前。注，上电初始化后除了全失压事件重新获取掉电变量的状态机，其它都默认为0，即发生前
#define EVT_S_AFT_HP				2								//发生后
#define EVT_S_BF_END				3								//结束前
#define EVT_S_AFT_END				4								//结束后

//事件上报
#define EVT_STAGE_UNCARE			0								//不关注
#define EVT_STAGE_HP				1								//发生
#define EVT_STAGE_END				2								//结束
#define EVT_STAGE_TASK				3								//全事件采集上报

typedef struct {
	BYTE bRela;						//关联对象属性表
	BYTE bCurRecNum; 				//当前记录数
	BYTE bMaxRecNum;				//最大记录数
	BYTE bPara;						//配置参数
	BYTE bRecTabStart;				//事件记录表起始
	BYTE bCurVal;					//当前值记录表
	BYTE bRepFlg;					//上报标识
	BYTE bValidFlg;					//有效标识
} TEvtAttr; 						//事件属性的定义，用来屏蔽CLASS7和CLASS24的差异

typedef struct {
	bool fInitOk;					//初始化正确
	bool fExcValid;					//事件有效（经过滤波后）
	BYTE bState;					//状态机
	BYTE bJudgeState;				//当前事件状态	
	BYTE bMemType;					//动态内存的使用类型，使用MEM_TYPE_*的定义
	WORD wTrigEvtDelaySec;			//触发事件的延时时间
	DWORD dwEstClick;				//发生时刻
	DWORD dwRecvClick;				//恢复时刻
	DWORD dwStaClick;				//时间统计Click
} TEvtBase;							//事件基类

typedef struct TEvtCtrl{
	//配置成员
	WORD wOI;						//事件的OI
	BYTE bClass;					//事件的类
	BYTE bItemNum;					//分项个数
	BYTE bDelaySec;					//事件判定延时时间	
	BYTE* pbFixField;				//void*
	WORD wFixFieldLen;				//void*的长度
	BYTE* pbSrcFmt;					//事件发生源数据格式
	WORD wSrcFmtLen;				//事件发生源数据格式长度
	const BYTE* pbDefCfg;			//关联属性表默认配置参数
	WORD wDefCfgLen;				//关联属性表默认配置参数长度

	//数据成员定义
	TEvtBase* pEvtBase;				//事件的基类数据
	void* pEvtPriv;					//单独某个事件的私有数据结构

	//接口函数定义
	bool (*pfnInitEvt)(struct TEvtCtrl* pEvtCtrl);	//事件初始化函数
	int (*pfnJudgeEvt)(struct TEvtCtrl* pEvtCtrl);	//事件判断函数
	bool (*pfnDoEvt)(struct TEvtCtrl* pEvtCtrl);	//事件执行函数
	DWORD dwLastClick;				//事件时标
	DWORD dwNewClick;				//事件时标
}  TTermEvtCtrl;					//事件控制结构


typedef struct{
	BYTE  bYear[2];
	BYTE  nMonth;
	BYTE  nDay;
	BYTE  nHour;
	BYTE  nMinute;
	BYTE  nSecond;
}TDTime;

typedef struct{
	DWORD	dwDmdVal;	
	TDTime	tTime;	
} TDmdVal;							//需量超限事件私有成员

typedef struct{
	TDmdVal	tDmd[4];	
} TDmd;	

typedef struct{
	DWORD dwVLossStaClick;			//用于累计总时间
} TVLoss;							//失压事件私有成员

typedef struct{
	TEvtBase tEvtBase;					
} TAllVLoss;						//全失压事件私有成员，为掉电变量

typedef struct{
	BYTE bOMD[EVT_CLR_LIST_LEN];	//array OMD，带格式
} TEvtClr;							//事件清零事件私有成员

typedef struct{
	bool fInit;
	BYTE bStaByte;
} TYXChgCtrl;						//遥信变位事件私有成员


/*typedef struct{
	TEvtBase tEvtBase;					
}TPowOffBase;*/

typedef struct{
	bool fPowerOff;
	bool fOldPowerOff;
	WORD wRecvCnt;
	WORD wEstbCnt;
	bool fInit;
	BYTE bAttr;						//属性标志，用于固定字段
	BYTE bEvtSrcEnum;				//事件发生源，用于固定字段 enum{停电(0)，上电(1)}
	BYTE bRptFlag;					//停上电事件上报标志
	BYTE bStep;						//状态机
	bool fIsUp;						//上电
	bool fIsUpRec;					//上电事件需要记录，上电记录
	bool fMtrOrTerm;
}TPowOff;							//终端停/上电事件

typedef struct{
	BYTE bEvtSrcEnum;				//事件发生源，用于固定字段
}TDeviceErr;						//设备故障记录

typedef struct{
	BYTE bShMtrFlag[SCH_MTR_SAVE_LEN];//搜不到的表标志
	BYTE bSaveFlag[SCH_MTR_SAVE_LEN];//本次事件存储的表标志
	BYTE bRunStep;					//运行步骤
}TUnKnMtr;							//发现未知电能表事件

typedef struct{		
	BYTE bClock[8];					//校时前时钟date_time_s，用于固定字段，存储示例: DT_DATE_TIME_S 07 E0 0B 0A 10 1E 03 , 即完成安数据类型存储
}TAdjTermTime;						//发现未知电能表事件

typedef struct{
	BYTE bShMtrFlag[SCH_MTR_SAVE_LEN];//搜不到的表标志
	BYTE bSaveFlag[SCH_MTR_SAVE_LEN];//本次事件存储的表标志
	BYTE bRunStep;					//运行步骤
}TStepArea;							//跨台区电能表事件 注，区分TUnKnMtr单独定义，避免后续需要添加特殊变量时不好做

typedef struct{
	BYTE bEvtSrcOAD[5];				//事件发生源OAD,用于固定字段，存储示例 DT_OAD F2 05 02 01
	BYTE bArrayPow[74];				//控后2分钟总加组功率 array long64, 共2 + 9*8 = 74字节。
									//存储两个功率-1.2345kW(0xffffffffffffCFC7)，及1.2345kW(0x3039)示例: 
									//DT_ARRAY 0x02 DT_LONG64 ff ff ff ff ff ff cf c7 DT_LONG64 00 00 00 00 00 00 30 39 DT_LONG64 00 00 00 00 00 00 00 00 DT_LONG64 00 00 00 00 00 00 00 00
									//				DT_LONG64 00 00 00 00 00 00 00 00 DT_LONG64 00 00 00 00 00 00 00 00 DT_LONG64 00 00 00 00 00 00 00 00 DT_LONG64 00 00 00 00 00 00 00 00
}TYKCtrl;							//遥控跳闸记录

typedef struct{
	BYTE bCompEng[9];				//越限时对比总加组有功总电能量 long64（单位：kWh，换算：-4），存储-1.2345kW/元示例:DT_LONG64 ff ff ff ff ff ff cf c7。
	BYTE bReferEng[9];				//越限时参照总加组有功总电能量 long64（单位：kWh，换算：-4），存储-1.2345kW/元示例:DT_LONG64 ff ff ff ff ff ff cf c7。
	BYTE bRelaErr[2];				//越限时差动越限相对偏差值 integer（单位：%，换算：0），存储示例: DT_INT 0A 。
	BYTE bAbsoErr[9];				//越限时差动越限绝对偏差值 long64（单位：kWh，换算：-4），存储-1.2345kW/元示例:DT_LONG64 ff ff ff ff ff ff cf c7。
}TEpOver;							//有功总电能量差动越限事件记录

typedef struct{
	BYTE bOAD[TERM_PRG_LIST_LEN];	//编程对象列表  array OAD
}TTermPrg;							//终端编程记录

typedef struct{
	BYTE bEvtSrcEnum;				//事件发生源，用于固定字段
}TCurCirc;							//终端电流回路异常事件

typedef struct{
	BYTE bEvtSrcTSA[18];			//事件发生源TSA，用于固定字段，存储示例: DT_TSA 06 11 22 33 44 55 66 00 00 00 00 00 00 00 00 00 00 ，即完成安数据类型存储
	BYTE bClock[8];					//校时前时钟date_time_s，用于固定字段，存储示例: DT_DATE_TIME_S 07 E0 0B 0A 10 1E 03 , 即完成安数据类型存储
	BYTE bClkErr[2];				//时钟误差integer（单位：秒，无换算），用于固定字段，存储示例: DT_INT 0A , 即完成安数据类型存储
}TMtrClkPrg;						//终端对电表校时记录

typedef struct{
	BYTE bEvtSrcOI[3];				//事件发生源 OI, 用于固定字段，存储示例 DT_OI 23 01
	BYTE bHpAfPow[9];				//事件发生后2分钟功率 long64(单位：W，换算-1),存储-1234.5W示例:DT_LONG64 ff ff ff ff ff ff cf c7。
	BYTE bCtrlOI[3];				//控制对象OI，存储示例 DT_OI F2 05
	BYTE bBreakCnt[3];				//跳闸轮次bit-string(SIZE(8))，存储示例DT_BIT_STR 08 03
	BYTE bPowCtrlVal[9];			//功控定值long64（单位：kW，换算-4）,存储-1.2345kW示例:DT_LONG64 ff ff ff ff ff ff cf c7。
	BYTE bHpBfPow[9];				//跳闸发生前总加有功功率long64（单位：kW，换算-4）,存储-1.2345kW示例:DT_LONG64 ff ff ff ff ff ff cf c7。
}TPowCtrl;							//功控跳闸记录

typedef struct{
	BYTE bEvtSrcOI[3];				//事件发生源OI，用于固定字段，存储示例 DT_OI 81 07
	BYTE bCtrlOI[3];				//控制对象OI，存储示例 DT_OI F2 05
	BYTE bBreakCnt[3];				//跳闸轮次bit-string(SIZE(8))，存储示例DT_BIT_STR 08 03
	BYTE bEleCtrlVal[9];			//电控定值long64（单位：kWh，换算-4），存储-1.2345kW示例:DT_LONG64 ff ff ff ff ff ff cf c7。
	BYTE bHpEng[9];					//跳闸发生时总加电能量long64（单位：kwh/元，换算-4），存储-1.2345kW/元示例:DT_LONG64 ff ff ff ff ff ff cf c7。
}TEleCtrl;							//电控跳闸记录

typedef struct{
	BYTE bEvtSrcOI[3];				//事件发生源 OI, 用于固定字段，存储示例 DT_OI 23 01
}TPurchParaChg;						//购电参数设置记录

typedef struct{
	BYTE bEvtSrcOI[3];				//事件发生源OI，用于固定字段，存储示例 DT_OI 81 07
	BYTE bCtrlOI[3];				//控制对象OI，存储示例 DT_OI F2 05
	BYTE bEleAlrCtrlVal[9];			//电控定值long64（单位：kWh，换算-4），存储-1.2345kW示例:DT_LONG64 ff ff ff ff ff ff cf c7。
}TEleAlram;							//电控告警事件记录

typedef struct {
	DWORD dwOAD;					//事件OAD
									//分项事件用属性标识不同的项目，记录索引用属性内元素索引来标识
	WORD  wRecIdx;					//任务库中的记录索引
	BYTE   bStage;					//发生阶段：0不关注；1发生；2结束

	BYTE bSchNo;					//方案号
	WORD wIdex;						//RCSD索引
	BYTE bRcsd[128];				//RCSD
	WORD wRcsdLen;					//RCSD长度
} TEvtMsg;							//事件上报消息

extern TAdjTermTime g_AdjTermTime;
extern TEpOver g_EpOver;

void SetTermEvtOadDefCfg(struct TEvtCtrl* pEvtCtrl);

TTermEvtCtrl* GetTermEvtCtrl(WORD wOI);
const TEvtAttr* GetEvtAttr(TEvtCtrl* pEvtCtrl);
void GetOIAttrIndex(DWORD dwOAD, WORD* pwOI, BYTE* pbAttr, BYTE* pbIndex);
bool GetEvtFieldParser(struct TEvtCtrl* pEvtCtrl, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize);
DWORD GetEvtMainOadDataLen(DWORD dwOAD);
bool IsEvtBeforeOAD(DWORD dwOAD);
bool IsSpecialOAD(DWORD dwOAD);
bool IsOADNeedAcqData(DWORD dwOAD, BYTE bState);

bool InitTmpMem(struct TEvtCtrl* pEvtCtrl, TFieldParser* pDataFields);
bool InitEvt(struct TEvtCtrl* pEvtCtrl);
bool InitVLoss(struct TEvtCtrl* pEvtCtrl);
bool InitDmd(struct TEvtCtrl* pEvtCtrl);
bool InitAVLoss(struct TEvtCtrl* pEvtCtrl);
bool InitEvtClr(struct TEvtCtrl* pEvtCtrl);
void InitTermEvt();

int VLossJudge(struct TEvtCtrl* pEvtCtrl);
int VLessJudge(struct TEvtCtrl* pEvtCtrl);
int VOverJudge(struct TEvtCtrl* pEvtCtrl);
int VBreakJudge(struct TEvtCtrl* pEvtCtrl);
int ILossJudge(struct TEvtCtrl* pEvtCtrl);
int IOverJudge(struct TEvtCtrl* pEvtCtrl);
int IBreakJudge(struct TEvtCtrl* pEvtCtrl);
int PReverseJudge(struct TEvtCtrl* pEvtCtrl);
int POverJudge(struct TEvtCtrl* pEvtCtrl);
int PDmdOverJudge(struct TEvtCtrl* pEvtCtrl);
int RPDmdOverJudge(struct TEvtCtrl* pEvtCtrl);
int QDmdOverJudge(struct TEvtCtrl* pEvtCtrl);
int PfUnderJudge(struct TEvtCtrl* pEvtCtrl);
int AVLossJudge(struct TEvtCtrl* pEvtCtrl);
int DisOrderJudge(struct TEvtCtrl* pEvtCtrl);
int MtrClrJudge(struct TEvtCtrl* pEvtCtrl);
int DmdClrJudge(struct TEvtCtrl* pEvtCtrl);
int EvtClrJudge(struct TEvtCtrl* pEvtCtrl);
int VUnBalanceJudge(struct TEvtCtrl* pEvtCtrl);
int IUnBalanceJudge(struct TEvtCtrl* pEvtCtrl);
int TermErrJudge(struct TEvtCtrl* pEvtCtrl);

bool OoReadOad(DWORD dwOAD, BYTE* pbBuf, WORD wDataLen, WORD wBufSize);
bool MakeEvtSpecField(DWORD dwROAD, DWORD dwFieldOAD, BYTE* pbField, WORD wFieldLen, WORD wFieldSize);
int EvtGetRecData(DWORD dwROAD, BYTE* pRecBuf, WORD wBufSize);
void UpdateState(struct TEvtCtrl* pEvtCtrl);
void UpdateRecMem(struct TEvtCtrl* pEvtCtrl, BYTE bSaveType);
void UpdateItemMem(struct TEvtCtrl* pEvtCtrl, BYTE bSaveType);
bool UpdateEvtStaData(struct TEvtCtrl* pEvtCtrl);
void UpdateVLossPriv(struct TEvtCtrl* pEvtCtrl);
void UpdateDmdPriv(struct TEvtCtrl* pEvtCtrl);
void UpdateAVLossPriv(struct TEvtCtrl* pEvtCtrl);
bool SaveTermEvtRec(struct TEvtCtrl* pEvtCtrl);
bool DoNullEvt(struct TEvtCtrl* pEvtCtrl);
bool DoEvt(struct TEvtCtrl* pEvtCtrl);
bool DoVLoss(struct TEvtCtrl* pEvtCtrl);
bool DoDmd(struct TEvtCtrl* pEvtCtrl);
bool DoAVLoss(struct TEvtCtrl* pEvtCtrl);
void DoTermEvt();

bool InitYXEvtCtrl(struct TEvtCtrl* pEvtCtrl);
int DoYXChgJudge(struct TEvtCtrl* pEvtCtrl);

void ClearEvtStaData(struct TEvtCtrl* pEvtCtrl);
void ClearVLossPriv(struct TEvtCtrl* pEvtCtrl);
void ClearVAllLossPriv(struct TEvtCtrl* pEvtCtrl);
void ClearOneEvt(struct TEvtCtrl* pEvtCtrl);
void ClearTermEvt(struct TEvtCtrl* pEvtCtrl);
void DealSpecTrigerEvt(WORD wOI);	
void GetEvtClearOMD(WORD wOI, BYTE bMethod, BYTE bOpMode);
void GetTermPrgOAD(WORD wOI, BYTE bAttr, BYTE Index);

char* GetEvtRecFileName(DWORD dwROAD);
bool GetEvtRecFieldParser(DWORD dwROAD, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize);
int GetEvtRecord(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbRecBuf, WORD wBufSize);
void ReInitMrtEvtPara(DWORD dwOAD);

int DoTermEvtMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtMethod2(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtIC7Method3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtIC24Method3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

bool SendEvtMsg(DWORD dwCnOAD, DWORD dwEvtOAD,WORD wRecIdx, BYTE bStage, BYTE bSchNo=0, WORD wIdex=0, BYTE* pbRCSD=NULL, WORD wRcsdLen=0);
int GetEvtRec(TEvtMsg* pEvtMsg, BYTE* pbRecBuf, WORD wBufSize, BYTE bType);
bool UpdateEvtRptState(DWORD dwCnOAD, TEvtMsg* pEvtMsg, BYTE bRptState);


bool InitPowOff(struct TEvtCtrl* pEvtCtrl);
bool InitDeviceErr(struct TEvtCtrl* pEvtCtrl);
bool InitUnKnMtr(struct TEvtCtrl* pEvtCtrl);
bool InitStepArea(struct TEvtCtrl* pEvtCtrl);
bool InitYKCtrl(struct TEvtCtrl* pEvtCtrl);
bool InitEpOver(struct TEvtCtrl* pEvtCtrl);
bool InitTermPrg(struct TEvtCtrl* pEvtCtrl);
bool InitCurCirc(struct TEvtCtrl* pEvtCtrl);
bool InitMtrClkPrg(struct TEvtCtrl* pEvtCtrl);
bool InitPowCtrl(struct TEvtCtrl* pEvtCtrl);
bool InitEleCtrl(struct TEvtCtrl* pEvtCtrl);
bool InitPurchParaChg(struct TEvtCtrl* pEvtCtrl);
bool InitEleAlram(struct TEvtCtrl* pEvtCtrl);
int TermInitJudge(struct TEvtCtrl* pEvtCtrl);
int TermVerChgJudge(struct TEvtCtrl* pEvtCtrl);
int PowOffJudge(struct TEvtCtrl* pEvtCtrl);
int GsgQAuthJudge(struct TEvtCtrl* pEvtCtrl);
int DeviceErrJudge(struct TEvtCtrl* pEvtCtrl);
int FluxOverJudge(struct TEvtCtrl* pEvtCtrl);
bool IsNeedSaveShMtrEvt(BYTE *pShMtrFlag, BYTE *pSaveFlag);
int UnKnMtrJudge(struct TEvtCtrl* pEvtCtrl);
int StepAreaJudge(struct TEvtCtrl* pEvtCtrl);
int TermClockPrgJudge(struct TEvtCtrl* pEvtCtrl);
int YKCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl);
int EpOverJudge(struct TEvtCtrl* pEvtCtrl);
int TermPrgJudge(struct TEvtCtrl* pEvtCtrl);
int CurCircJudge(struct TEvtCtrl* pEvtCtrl);
int MtrClkPrgJudge(struct TEvtCtrl* pEvtCtrl);
int PowCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl);
int EleCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl);
int PurChParaChgJudge(struct TEvtCtrl* pEvtCtrl);
int EleCtrlAlarmJudge(struct TEvtCtrl* pEvtCtrl);
int OnInfoTrigerEvtJudge(struct TEvtCtrl* pEvtCtrl, BYTE bInfoType);

void AddEvtOad(DWORD dwOAD, bool fRptFlag);
void DelEvtOad(DWORD dwOAD, WORD wOI);

#endif //TERMEVTTASK_H

