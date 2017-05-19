/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterStruct.h
 * 摘    要：本文件主要实现抄表控制数据结构定义
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年9月
 *********************************************************************************************************/
#ifndef METERSTRUCT_H
#define METERSTRUCT_H
#include "Comm.h"
#include "MeterConst.h"
#include "apptypedef.h"
#include "ComStruct.h"
#include "MtrExc.h"

typedef struct {
	BYTE bUnit;	//间隔单位
	WORD wVal;	//间隔值
}TTimeInterv;	//时间间隔

typedef struct{
	DWORD dwOAD;		//object attrib, 高WORD帮OI，低WORD放属性值
	WORD  wID;		//对应97645的内部ID
	BYTE* pFmt;		//格式描述串
	WORD  wFmtLen;	//格式描述串长度
	WORD  wOoLen;
	WORD  w645Len;
}Toad645Map;

typedef struct{
	DWORD dwEvtOAD;					//事件OAD
	DWORD dwErcNumID;				//事件次数ID
	BYTE  bNum;						//必抄项ID个数
	DWORD dwRdID[6];		//必抄项ID列表
}TErcRdCtrl;	//全事件采集必抄项控制结构

//电表参数
typedef struct
{
	WORD	wPn;				//测量点号		
	BYTE	bProId;				//协议号
	BYTE	bSubProId;			//子协议号
	BYTE	bRateNum;			//费率数
	BYTE	bAddr[17];			//表地址	
	BYTE	bPassWd[6];			//密码
	BYTE	bRateTab[4];		//费率顺序
#ifdef EN_SBJC_V2
    BYTE	bMtrAddr[7];		//水气表地址
#endif    
	CComm* pComm;
	TCommPara CommPara;			//串口通信参数	
}TMtrPara; 

typedef struct 
{
	DWORD 	dwOAD;	
	WORD	wOffset;	//在bBuf中的偏移
	BYTE		bLen;	//长度
	BYTE		bValid;	//1有效，0无效
}TMtrItemMem; //电表临时数据项分配控制结构

#define MTR_TMP_ITEM_NUM		128		//II型集中器可以少点
#define MTR_TMP_BUF_SIZE			4096	//II型集中器可以少点

#define MTR_ADDR_LEN				17		//长度1 + 地址16

#define LOOP_MAX_CNT				2

typedef struct 
{
	DWORD 	dwTime;	//临时数据的间隔时标
	TMtrItemMem item[MTR_TMP_ITEM_NUM];
	BYTE bBuf[MTR_TMP_BUF_SIZE];
}TMtrTmpData; //电表临时数据

typedef struct 
{
	BYTE	bTaskIdx;		//任务索引，针对的是taskSucFlg
	WORD	wItemIdx;	//搜索到的下一个数据项的索引
	BYTE	bLoopCnt;	//表地址对应的所有任务遍历次数，暂时定义为LOOP_MAX_CNT
}TSchItem; //抄读数据项的搜索控制

typedef struct 
{
	BYTE bReqType;	//请求类型：[1] GetRequestNormal，[3] GetRequestRecord

	DWORD dwOAD;	//对象属性描述符
	WORD wRsdLen;	
	BYTE bRSD[128];	//记录选择描述符
	WORD wRcsdLen;
	BYTE bRCSD[128];	//记录列选择描述符
	BYTE bCSD[134];
	WORD wRcsdIdx;	//RCSD中关联OAD的偏移, 载波使用

	TTimeInterv tiExe;	 //执行频率

	DWORD dwEvtCnt;		//上一次抄读到的事件次数
}TRdItem; //抄读数据项

//#define ITEM_RD_TIME_NUM	3

//抄表事件结构
typedef struct{
	BYTE bState;	//状态机
	BYTE bReserve;	//保留字节
}TMtrClockErr;	//电能表时间超差

typedef struct{
	BYTE bState;		//状态机
  	BYTE bAddr[MTR_ADDR_LEN];
	uint64 ui64PosE;		//上周期正向有功
	uint64 ui64NegE;		//上周期反向有功
	bool fInvalid[2];
}TMtrEnergyDec;//电能表示度下降事件

typedef struct{
	BYTE bState;		//状态机
  	BYTE bAddr[MTR_ADDR_LEN];
	uint64 ui64PosE;		//上周期正向有功
	uint64 ui64NegE;		//上周期反向有功
	DWORD dwSeconds[2];	//正向开始发生时间
	bool fInvalid[2];
}TMtrFlew;//电能量飞走


typedef struct{
	BYTE bState;		//状态机
  	BYTE bAddr[MTR_ADDR_LEN];
	uint64 ui64PosE;		//上周期正向有功
	uint64 ui64NegE;		//上周期反向有功
	DWORD dwSeconds[2];	//正向开始发生时间
	bool fInvalid[2];
}TMtrEnergyErr;//电能量超差


typedef struct{
	BYTE bState;		//状态机
  	BYTE bAddr[MTR_ADDR_LEN];
	uint64 ui64PosE;		//上周期正向有功
	uint64 ui64NegE;		//上周期反向有功
	DWORD dwSeconds;
	bool fInvalid;
}TMtrStop;//电能表停走事件


typedef struct{
	BYTE bState;	//状态机
	BYTE bReserve;	//保留字节
}TMtrRdFail;	//抄表失败

#define MTEDATACHG_CSD_LEN	60
#define MTEDATACHG_DATA_LEN	200

typedef struct{
	BYTE bState;		//状态机
  	BYTE bAddr[MTR_ADDR_LEN];
	BYTE bOldCSD[MTEDATACHG_CSD_LEN];		//监控数据对象  CSD,用于判断是否任务有更新，若有更新，需要重新刷新DATA数据，然后才能判断是否产生新事件
	BYTE bCSD[MTEDATACHG_CSD_LEN];			//监控数据对象  CSD
	BYTE bOldData[MTEDATACHG_DATA_LEN];	//变化前数据    Data
	BYTE bNewData[MTEDATACHG_DATA_LEN];	//变化后数据    Data
}TMtrDataChg;//电能表数据变更监控记录

typedef struct
{
	DWORD dwItemRdTime[ITEM_RD_TIME_NUM];	//依次是正有、反有、时钟
											//在SaveMtrDataHook()中更新本成员
	WORD  wLastRecPhyIdx[MTR_EXC_NUM];		//依次是各抄表事件的最近一次记录的存储位置
	DWORD dwLastStatClick[MTR_EXC_NUM];		//依次是各抄表事件的统计时标
	BYTE  bTryReadCnt[MTR_EXC_NUM];		//依次是各抄表事件的尝试抄表次数
											
	TMtrClockErr mtrClockErr;  //电能表时钟超差事件
	TMtrEnergyErr mtrEnergyErr; //电能量超差事件
	TMtrEnergyDec mtrEnergyDec; //电能表示度下降
	TMtrStop mtrStop;	//电能表停走
	TMtrFlew mtrFlew;	//电能表飞走
	TMtrRdFail mtrRdFail;
	TMtrDataChg mtrDataChg;	//电能表数据变更监控记录
}TMtrExcTmp; //电表事件临时数据



#define TASK_SUC_FLG_LEN 8

#define ALLOC_TAB_LEN		20//16		//动态内存分配表的字节长度
#define MEMORY_BLOCK_SIZE	64	//每块大小128个字节
#define MTR_TAB_NUM 		96		//动态内存分配表个数

#define MTR_TASK_NUM 64
#define MTR_CACHE_NUM  20


typedef struct
{
	BYTE 	bValid;		//1有效，0无效
	BYTE 	bTaskId;	//任务ID
	bool	fRecSaved;	//任务记录是否已经入库
	bool	fReRd;		//是否是补抄
	BYTE	bCSDItemNum; //任务需抄读CSD个数
	DWORD 	dwTime;		//任务执行时标
	BYTE 	bSucFlg[TASK_SUC_FLG_LEN];	//抄读成功标识位，每一位置1表示成功抄读到任务配置中的对应数据项
}TTaskSucFlg; //任务抄读成功标志

//动态内存类型定义
#define MEM_TYPE_NONE				0    //不使用动态内存
#define MEM_TYPE_TASK				1    //任务记录
#define MEM_TYPE_MTREXC			2    //抄表异常
#define MEM_TYPE_EVT_ACQ  		3	 //全事件采集的临时数据
#define MEM_TYPE_TERM_EVTREC		4	 //终端内部事件记录，包括内表和终端事件
#define MEM_TYPE_TERM_EVTITEM		5	 //终端事件的临时数据项
#define MEM_TYPE_CURVE_FLG		6	 //曲线成功标志

#define MTR_MEM_SIZE 	10240//8192

typedef struct 
{
	DWORD	dwId;		//高1字节为类型
	//低3字节为函数传递过来的dwId的低3字节,分别为：
	//任务记录:任务ID、
	//抄表异常:事件OI
	//终端内部事件:事件OI+分相属性
	//全事件采集：采集方案编号
	WORD	wDataLen;	//数据的有效长度
	BYTE 	bAllocTab[ALLOC_TAB_LEN];	//临时记录分配表
	//每一位置1表示占用相应的64个字节空间，8192的空间共需16个字节
}TAllocTab; //普通采集方案临时记录分配表

typedef struct
{
	BYTE  bChkSum;	//从bTsa到结构结尾的校验和
	BYTE  bTsa[17];		//表地址，用于校验电表地址是否发生改变
	BYTE	bTaskSN;	//任务配置的序列号，用来比较是否任务配置发生了改变
	//系统库中保存当前最新的序列号
	//在任务执行前先判断配置序列号是否发生改变，
	//如果发生改变，马上更新bTaskSN，
	//再调整taskSucFlg和临时记录空间的分配

	TSchItem schItem; 			//抄读数据项的搜索控制
	TMtrTmpData  mtrTmpData;	//电表临时数据
	TMtrExcTmp mtrExcTmp; //电表事件临时数据

	TTaskSucFlg taskSucFlg[MTR_TASK_NUM];	//任务抄读成功标识位
	
	//动态内存管理
	BYTE  bGlobal[ALLOC_TAB_LEN];	//全局动态内存分配表，相当于allocTab[]的或 
	TAllocTab allocTab[MTR_TAB_NUM];	//动态内存分配表

	BYTE bMem[MTR_MEM_SIZE];	//动态内存分配空间
}TMtrRdCtrl; //电表抄读控制结构

#define CACHE_STATUS_FREE			0	//空白，为分配
#define CACHE_STATUS_IDLE			1	//空闲，没有被某个抄表线程使用
#define CACHE_STATUS_INUSE		2	//在用：正在被某个抄表线程使用

typedef struct
{
	BYTE  bStatus;			   //状态：CACHE_STATUS_FREE、
								//		CACHE_STATUS_IDLE、
								//		CACHE_STATUS_INUSE
	WORD wPn;					//测量点号
	BYTE  bTsa[17];		 //表地址
	DWORD dwCacheTime;      //缓存到内存的时间,超过10分钟写回文件
	DWORD dwLastAcessTime;  //最后访问时间，缓存满时，用来导出最旧没访问的
	bool  fDirty;            //是否为脏，为脏时才有必要导出
	bool fTrigerSave; //强制触发保存标志
	TMtrRdCtrl mtrRdCtrl; 	//电表抄读控制数据
}TMtrCacheCtrl;  //电表缓存控制结构

extern TMtrCacheCtrl g_MtrCacheCtrl[MTR_CACHE_NUM];

#endif
