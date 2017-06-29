#ifndef FASTRUCT_H
#define FASTRUCT_H

#include "ComStruct.h"
#include "DbStruct.h"
#include "ThreadMonitor.h"
#include "TermEvtTask.h"

typedef struct{
	WORD wVer;              //版本 
	bool fTmpValid;         //掉电暂存变量有效标志
	bool fAlrPowerOff;		//掉电前上报了停电告警
	WORD wRstNum;			//复位次数
	WORD wMonitorRstNum;	//线程监控复位次数
	char szMonitorRstThrd[THRD_NAME_LEN];	//线程监控复位最后一次复位的线程名称
	TTime tPoweroff;        //上次停电时间
	TTime tPowerOn;			//上次上电时间
	TAllVLoss tAllVLoss;	//全失压私有变量
}TPowerOffTmp;   //掉电暂存变量

#endif //FASTRUCT_H

