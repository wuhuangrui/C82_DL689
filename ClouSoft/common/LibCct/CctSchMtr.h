#ifndef CCTSCHMTR_H
#define CCTSCHMTR_H

#include "ComStruct.h"
#include "TaskManager.h"

typedef struct {
	//0x6002 属性08
	bool fAutoSchMtr;	//是否启用每天周期搜表
	bool fAutoUpdMtr;	//是否自动更新采集档案
	bool fIsGenEvt;		//是否产生相关事件
	BYTE bClrMtrChoice;	//清空搜表结果选项
}TSchMtrParam;	//搜表参数 

typedef struct {
	TTime tStartTime;	//开始时间 
	WORD wKeptTime;	//搜表时长（min）
}TSchMtrTimeParam;	//定时搜表参数

typedef struct {
	DWORD dwOAD;	//对象属性描述值
	BYTE bData[16];	//属性值
}TSchAddInfo;	//搜到的附加信息

typedef struct {
	DWORD dwPort;
	BYTE bMtrLen;	//搜表地址长度
	BYTE bMtr[TSA_LEN];	//搜表地址
	BYTE bAcqLen;	//所属采集地址长度
	BYTE bAcqAddr[TSA_LEN];	//所属采集地址
	BYTE bMtrPro;	//规约类型	0-未知, 1-DL/T645-1997, 2-DL/T645-2007, 3-DL/T698.45, 4-CJ/T 188-2004
	BYTE bPhase;	//相位	0-未知, 1-A, 2-B, 3-C
	BYTE bSQ;		//信号品质
	TTime tSchMtrSucTime;	//搜表成功时间
	BYTE bCS;	//前面部分数据的校验和
	//BYTE bSchAddInfoCnt;	//搜到的附加信息个数
	//TSchAddInfo tTSchAddInfo[8];	
}
/*#ifdef SYS_WIN*/
TSchMtrRlt;	//搜表结果
// #else
// __attribute__((packed))TSchMtrRlt;	//搜表结果
// #endif


typedef struct {
	BYTE bMtrAddr[TSA_LEN+1];	//通信地址, bMtrAddr[0]是地址长度
	BYTE bMainAddr[TSA_LEN+1];	//主节点地址, bMtrAddr[0]是地址长度
	TTime tUpdTime;	//变更时间
}
/*#ifdef SYS_WIN*/
TCrossSchMtrRlt;	//跨台区搜表结果
// #else
// __attribute__((packed))TCrossSchMtrRlt;	//跨台区搜表结果
// #endif


#define MK_SCH_MTR_FILE(pszTabName)				sprintf(pszTabName,"%sSearchMtrFile.dat", USER_DATA_PATH)
#define PER_MTR_OFFSET_LEN(X)					(sizeof(TSchMtrRlt)*(X-1))	//X>0
#define PER_RLT_LEN								sizeof(TSchMtrRlt)	//一个搜表结果的长度

#define MK_CROSS_SCH_MTR_FILE(pszTabName)		sprintf(pszTabName, "%sSearchCrossMtrFile.dat", USER_DATA_PATH)
#define PER_CROSS_MTR_OFFSET_LEN(X)				(sizeof(TCrossSchMtrRlt)*(X-1))	//X>0
#define PER_CROSS_RLT_LEN						sizeof(TCrossSchMtrRlt)	


typedef enum {
	SCH_MTR_EMPTY = 0,		//搜表状态未运行
	START_BOARD_SCH_MTR,	//启动广播搜表
	START_NODE_ACTIVE,		//启动节点注册
	WAIT_MTR_REPORT,		//等待电表上报
	FINISH_SCH_MTR,			//完成搜表	
}TSEARCH_STATE;

class CCctSchMeter{
public:
	CCctSchMeter(void);

	virtual ~CCctSchMeter(void){};

	TSchMtrParam m_TSchMtrParm;

	bool m_fStartBoardCast;	//启动广播标识

	DWORD m_dwStartBoardCastClk;	//启动广播搜表的时间

	bool m_fRightNowSchMtr;	//立即启动搜表

	bool m_fPeriodSchMtr;	//时段搜表

	bool m_fRptSchMtrEnd;	//搜表任务结束

	bool m_fClrFile;	//是否清除文件

	BYTE m_bActCnt;	//主动注册次数

	DWORD m_dwLastRptMtrClk;	//上一次搜表与路由模块交互的时钟滴答

	TSem m_tSchMtrSem;

	TSem m_tAlarmSem;	//事件更新信号量

	bool DoSchMtrAddr();

private:
	BYTE m_bSchMtrState;

	bool m_fUdpMtrToDB;	//是否需要更新档案到系统库

public:
	//描述：获取搜表参数
	bool GetSchMtrParam(TSchMtrParam *pSchMtrParam);

	//描述：是否在搜表时段参数(60020900)
	//返回：>0表示在搜表时间范围内
	bool IsSchMtrPeriod();

	//描述：设置搜表状态
	//参数：true-搜表中，false-空闲
	int SetSchMtrState(bool fState);

	//描述：校验表地址
	void CheckMtrAddr(TSchMtrRlt *pSchMtrRlt);

	//描述：写一个搜表结果
	//参数：	@pbBuf 搜表结果
	bool SaveOneSchMtrResult(TSchMtrRlt *pSchMtrRlt);

	//描述：替换一个搜表结果
	//参数：@wIdx 索引
	//		@pSchMtrRlt 要替换的档案
	bool ReplaceOneSchMtrResult(WORD wIdx, TSchMtrRlt *pSchMtrRlt);

	//描述：通过文件索引，删除对应的表档案
	void DelSchMtrResult(WORD wIdx);

	//描述：检索搜表档案是否缺少
	//备注：在替换原有的档案之后会存在两种情况
	//	1.在替换表地址后，查询下原采集器是否已经不存在，不存在就要补充一个ACQ+NULL的采集器
	//  2.查询是否有为NULL的采集器档案
	void LoopSchMtrResult(TSchMtrRlt *pSchMtrRlt);

	void LoopMtrSysDb(TOobMtrInfo tMtrInfo);

	//描述：存储搜表结果
	//参数：	@pbBuf 搜表结果
	int SaveSchMtrResult(DWORD dwPortOad, BYTE *pbBuf, WORD wLen, BYTE bMtrAddrLen = 6);

	//描述：获取搜表结果
	//参数：@piStart 首次传入为-1，搜表结果未读取完就返回相应的值，读取结束返回0xFFFE
	//		@fGetAll 如果是获取所有数据，必须配置*piStart=-1 且 fGetAll = true
	//返回： -1结束，>0本次读取的次数
	int GetSchMtrResult(int *piStart, TSchMtrRlt *pSchMtrRlt, bool fGetAll = true);

	//描述：获取搜表结果
	int GetSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen, DWORD dwStartSec = 0, DWORD dwEndSec = 0);

	//描述：获取跨台区搜表结果
	int GetCrossSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen);

	//描述：所有台区搜表结果记录数
	int CalSchMtrNum();

	//描述：存储跨台区搜表结果
	//参数：	@pbBuf 搜表结果
	int SaveCrossSchMtrResult(BYTE *pbBuf, WORD wLen);

	int SaveOneCrossSchMtrResult(TCrossSchMtrRlt *pSchMtrRlt);

	//描述：更新一块电表到数据库的电表档案中
	void UpdataSchMtrToSysDb(TSchMtrRlt *pSchMtrRlt);

	//描述：寻找一个空的表序号存储搜回的表档案
	void SearchEmptySaveMeter(TSchMtrRlt *pSchMtrRlt, BYTE *pbMtrMask);

	//描述：生成搜表事件
	void SaveAlarmEvent(TSchMtrRlt *pSchMtrRlt);

	void SchMtrRltConvertMtrInfo(TSchMtrRlt *pSchMtrRlt, TOobMtrInfo *pMtrInfo);

	void MtrInfoConvertSchMtrRlt(TOobMtrInfo *pMtrInfo, TSchMtrRlt *pSchMtrRlt);

	//描述：删除搜表结果
	void DeleteSearchMtrFile();

	//描述：删除跨台区结果
	void DeleteCrossSearchMtrFile();

	int GetRightNowSchMtrKeepTime();

	//描述：激活从节点等待时间
	int GetNodeActWaitMin();

	//描述：更新电表档案标识
	void SetUdpMtrFlg(bool fState);

	//描述：获取电表档案标识
	bool GetUdpMtrFlg();

	//描述：更新告警事件信号屏蔽字
	//参数：@wIndex 发现未知电表索引
	//		@fState false:清除索引wIndex对应的标识，反之
	bool SetSchMtrEvtMask(WORD wIndex, bool fState);

	//描述：更新告警事件信号屏蔽字
	bool UpdataSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen);
	//描述：获取告警事件信号屏蔽字
	bool GetSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen);

	//描述：清除告警事件屏蔽字
	void ClearSchMtrEvtMask();

	//描述：通过索引获取告警数据
	//参数：@iIndex 告警索引
	//		@pbBuf 返回的告警数据内容
	//返回：-1获取数据失败，>0告警数据的长度
	int GetSchMtrEvtData(int iIndex, BYTE *pbBuf);

	//描述：校验上次搜表时间至现在是否超过30天
	void CheckMeterSearchTime();

protected:

	virtual bool StartBoardCast(int iMin) { iMin; return false;};

	virtual int StartSchMtr(){return -1;};

	virtual bool StartNodeActive(){return false;}

	virtual bool WaitMtrReport(){return false;};

	virtual int FinishSchMtr(){return -1;};

};

#endif