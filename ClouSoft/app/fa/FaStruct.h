#ifndef FASTRUCT_H
#define FASTRUCT_H

#include "ComStruct.h"
#include "DbStruct.h"
#include "FaConst.h"
#include "ThreadMonitor.h"
#include "TermEvtTask.h"

typedef struct{
	DWORD time;			//发生时间
	BYTE bVerInfo[10];	//版本信息
}TSoftVerChg;		//缓存的版本变更事件

typedef struct{
	DWORD time;			//发生时间
	WORD wNum;
	WORD wClass[MAXNUM_ONEERC3];
	BYTE bObis[MAXNUM_ONEERC3][6];	//一次应用服务最多同时处理50个OBIS对象
}TParaChg;			//缓存的参数变更事件

typedef struct{
	WORD wVer;              //版本 
	bool fTmpValid;         //掉电暂存变量有效标志
	bool fGPRSConected;     //GPRS已连接
	BYTE bRemoteDownIP[8];  //远程下载软件的服务器IP地址
	bool fAlrPowerOff;		//掉电前上报了停电告警
	WORD wRstNum;			//复位次数
	WORD wMonitorRstNum;	//线程监控复位次数
	char szMonitorRstThrd[THRD_NAME_LEN];	//线程监控复位最后一次复位的线程名称
	short iRemoteDownPort;
	TTime tPoweroff;        //上次停电时间
	TTime tPowerOn;			//上次上电时间
	BYTE  bParaEvtType;		//记录掉电前参数初始化事件的类型：0无效 1重要 2一般
	TSoftVerChg ParaInit;	//参数初始化事件
	TAllVLoss tAllVLoss;	//全失压私有变量
	//TPowOffBase tPowOffBase;			//终端掉上电状态
}TPowerOffTmp;   //掉电暂存变量

typedef struct{
	bool fInit;					//本结构已初始化
	WORD wTaskID;				//任务标识
	WORD wMaxFrmBytes;			//每帧里的最多能放的字节个数(剩下给电表记录的空间大小)
	BYTE bFrmNum;				//本次读任务数据命令需要恢复的帧数
	WORD wMeterNum;				//要读的电表数
	WORD wDayNum;				//要读的天数/现有符合要求记录的天数
	WORD wRecLenOneMeter;		//每笔记录的长度
	WORD wRecsPerDay;			//每天每个电表要有多少笔记录
	TTime tmStartDate;			//要读的日期
	WORD wRecStart;				//符合要求记录的起始位置
	WORD wRecsFound;			//符合要求记录的条数(相对于一个电表来说)
	WORD wRecsInvalid;			//对于那些最后一天还有几点没抄完的重点户数据,需要补成无效数据的记录条数
	WORD wFrmStartPoint[0x100+2]; //每帧的起始测量点号,最后一个如果被设成0xffff,表示最后一帧一直搜索到最后一个测量点
}TReadTaskCtrl;					  //读任务数据的控制结构

typedef struct{
	bool fFirst;	    //查找开始
	bool fFinal;		//查找结束
	WORD wTxRecLen;		//发送的每个记录的长度
	bool fAllMeter;		//广播地址
	WORD wItemIndex;	//要查找的ID在本任务的ID配置表的索引
	WORD wSortIndex;	//已经查找到了排序表的那一项
	WORD wMeterIndex;	//电表的索引
	WORD wRecsFound;	//本轮找到的记录个数
}TReadTaskCtrlJx;		//读任务数据的控制结构
#endif //FASTRUCT_H

