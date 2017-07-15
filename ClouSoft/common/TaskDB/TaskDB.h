/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TaskDB.h
 * 摘    要：任务数据库实现类（任务数据表管理，任务记录管理，任务搜索）
 *
 * 当前版本：1.1.23
 * 作    者：杨进
 * 完成日期：2007-12-28
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
 * 备    注：1、任务数据搜索目前只支持2级排序
 *           2、搜索条件个数支持到8个
 *           3、对于超过2个搜索字段的目前不支持既有公共字段又有私有字段的表
 *           4、对于超过2个搜索字段的搜索，读取记录抽取时只支持前2个搜索条件抽取
 *
************************************************************************************************************/
#pragma once
#ifndef _TASKDB_H
#define _TASKDB_H
#include "sysarch.h"
#include "syscfg.h"

#define TDB_PATH			USER_DATA_PATH
#define TDB_TAB_INDEX		"tdbindex.idx"

//#define TDB_SCH_MODE        1   //开放本行搜索时从最旧得记录开始查找，注销本行从最新的记录开始查找
//以下定义用户必须可以根据实际情况进行调整
////////////////////////////////////////////////////////////////////////////////////////
//#define TDB_FIELD_MAX_LEN	256	//供搜索时使用，一笔纪录所有字段最大总长度
//#define TDB_MAX_PATH		255 //数据表文件名最大长度
//#define TDB_MAX_TAB_NUM		255	//最多允许的表数量
//#define TDB_MAX_OPENED_TAB	32	//系统同一时刻最多允许的打开的表
//#define TDB_MAX_BUF_SIZE	0x80000		//最多缓存(1MB)的内容，超过这么多内容就不在内存中缓存，
										//而是存到文件中, 是TDB_BUF_BLOCK_SIZE整数倍
//#define TDB_BUF_BLOCK_SIZE	0x20000		//每个搜索可以使用的最大缓存(64KB)
//#define TDB_MAX_CONCUR_SCH	5			//允许并发搜索个数
#define TDB_MAX_SCH_FIELD	16			//一次搜索最多允许的条件个数
#define TDB_SCH_VAL_LEN    255			//关键字字段的最大长度
#define TDB_MAX_OPNUM		10			//一个字段最多允许比较的方法数
//#define TDB_RDBLOCK_SIZE	1024		//读取缓存大小(系统中一笔记录最长可能长度)
//#define TDB_PERI_CACHENUM	10			//需要按时段缓存数据的时候，在时段内最多缓存记录笔数
////////////////////////////////////////////////////////////////////////////////////////

#define TDB_INVALID_DATA	0xFF	//无效数据
////////////////////////////////////////错误类型
#define TDB_ERR_UNKNOWN			-1	//其它未知错误
#define TDB_ERR_OK				 0	//没有错误
#define TDB_ERR_TBEXIST			-1	//表已存在
#define	TDB_ERR_TBOVERNUM		-2	//表数目出系统允许的最大数量
#define TDB_ERR_TBNOEXIST		-3	//表不存在
#define TDB_ERR_TBLOCKED		-4	//当前表被锁定(当某个线程编辑一个表的时候，表被锁定禁止其它表编辑)
#define TDB_ERR_TBDATA			-5	//数据错，当应用向数据库插入记录时，如果记录长度超出数据库允许的长度返回该错误
#define TDB_ERR_INVALID_HANDLE	-6	//错误的句柄
#define TDB_ERR_OVER_CONCUR		-7	//超出允许并发的搜索数目
#define TDB_ERR_INVALID_FIELD	-8  //错误的字段
#define TDB_ERR_OVER_BUFFER		-9  //没有缓存了
#define TDB_ERR_TBSCHRULENUM	-10 //错误的搜索规则
#define TDB_ERR_DBLOCKED		-11 //数据库被锁定
#define TDB_ERR_UNINIT		    -12 //数据库尚未初始化
#define TDB_ERR_NOMEMORY	    -13 //内存不足
////////////////////////////////////////

//任务数据库目前支持数据类型
#define TDB_TIME	1	//时间类型 年月日时分	 5	从高字节到低字节比较，支持>、>=、<、<=、=
#define TDB_STR		2	//以零结尾的字符串	不限	使用strcmp，>、<、=
#define TDB_BCD		3	//BCD数组		不限	从高字节到低字节比较，支持>、>=、<、<=、=
#define TDB_BYTE	4	//BYTE数组		不限	从高字节到低字节比较，支持>、>=、<、<=、=
#define TDB_INT16	5	//16位带符号整形			求值比较，支持>、>=、<、<=、=
#define TDB_INT32	6	//32位带符号整形			求值比较，支持>、>=、<、<=、=
#define	TDB_INT64	7	//64位带符号整形			求值比较，支持>、>=、<、<=、=

#define TDB_FIELD_MAX   32  //目前只支持每笔记录最大32个字段,方便用一个DWORD作为读取记录时取字段的掩码

#define TDB_INVALID_FIELD  0xffff
#define TDB_ALL_FIELD	   0xffffffff
#define TDB_OP_EQ   0  //等于
#define TDB_OP_GE   1  //大于等于
#define TDB_OP_GT   2  //大于
#define TDB_OP_LT   3  //小于
#define TDB_OP_LE   4  //小于等于
#define TDB_OP_MIN  5  //查找最小值
#define TDB_OP_MAX  6  //查找最大值

#define TDB_IDX_ANY     0
#define TDB_IDX_SOLE    1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//任务数据库的定义
//数据表管理结构
typedef struct{
	BYTE bVer;										//数据表管理结构版本
	WORD wTabNum;									//当前表数目
	BYTE* bValidTab;		//BYTE bValidTab[TDB_MAX_TAB_NUM/8+1];			//
	BYTE** bTabName;	//BYTE bTabName[TDB_MAX_TAB_NUM][TDB_MAX_PATH];	//	
}TTabMgr;

//#define TDB_MGR_HEADLEN		((unsigned int)&((TTabMgr*)0)->bTabName)
#define TDB_MGR_SAVE_ALL	0xFFFFFFFF
#define TDB_MGR_SAVE_INDEX	0xFFFFFFFE
//表控制结构

typedef struct{
	TTime tmStart;
	TTime tmEnd;
}TTdbPeriTime;//在这个时段内写入的记录不立即写入Flash，而是存入缓存，由用户触发保存或过时段时保存

typedef struct{
	char* cTabName;	//char cTabName[TDB_MAX_PATH]; //表名
	int iIndex;//当前缓存记录索引，总是指向下一个缓存的位置
	int iNum;//当前缓存中已有的记录笔数
	BYTE *pbBuf;
}TTdbPeriCache;

//有公共字段时总记录个数等于wMaxRecPublicNum, 
//每笔记录包含1一笔含公共字段及wMaxRecPrivateNum笔含子字段的记录
//没有公共字段时总记录个数等于wMaxRecPrivateNum, 
typedef struct{
	BYTE bVer;		 //表控制结构的版本
	BYTE bPublicNum; //公用字段的个数
	BYTE bPrivateNum;  //子字段个数 
	DWORD dwMaxRecPublicNum;   //可记录公共个数
	DWORD dwMaxRecPrivateNum;   //可记录子个数
	DWORD dwCurRecNum;    //当前记录个数
	DWORD dwCurRecOffset; //当前记录
	WORD wField[TDB_FIELD_MAX][3];  //元素0--字段类型;元素1--长度;元素2--属性
	BYTE bNoCache;
	BYTE bReserve[255];  //预留空间
	BYTE bChkSum;   	//以上所有字段的校验和
}TTabCtrl;				//表控制结构

typedef struct{
	int			iHandle;	//数据表句柄
	char*		cTabName;	//char		cTabName[TDB_MAX_PATH]; //表名
	int			iOpenedCnt;
	DWORD		dwRecLen;	//单笔记录大小
	TSem		tSemaphore;	//防止同一个表被同时多次写入		
	TTabCtrl	tTabCtrl;	//表控制信息	
}TTdbActivedTabInfo;//活动状态的表信息

typedef struct{
	WORD wField;  //字段序号
	WORD wOpNum;  //比较方法数量
	WORD wOp[TDB_MAX_OPNUM];    //比较方法：TDB_OP_EQ/TDB_OP_GE/TDB_OP_GT/TDB_OP_LT/TDB_OP_LE
	BYTE bVal[TDB_MAX_OPNUM][TDB_SCH_VAL_LEN];  //比较值
}TTdbSchRule;			//任务记录搜索的一条规则，目前只支持多条规则的与

typedef struct{
	WORD	wID;	//搜索ID，用以区分不同的应用提交的搜索
	int		iTabIndex;	//被搜索的表格
	//查询该表格占用的缓存位置
	TTdbSchRule rTdbSchRule[TDB_MAX_SCH_FIELD];
	BYTE*	pbBufIdxTab;//BYTE	dwBufIdxTab[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE];		
	char*	pcFileName;	//char	cFileName[TDB_MAX_PATH];	//当可分配给搜索的缓存不够用的时候则使用文件缓存
									//此为缓存文件的路径(包含文件名)
}TTdbSchInfo;

typedef struct{
	//-------------------应用给任务数据库的参数部分-----------------------------------------------
	//记录排序规则
	WORD wSortNum;		//排序规则个数
	WORD wSortFild[TDB_MAX_SCH_FIELD];    //字段ID，TDB_INVALID_FIELD表示该字段无效
	WORD wSortOp[TDB_MAX_SCH_FIELD];      //比较方法：TDB_OP_GE/TDB_OP_LE:按照升序或者降序的规则排序,允许有键值重复的情况出现
				      	  //		  TDB_OP_GT/TDB_OP_LT:按照升序或者降序的规则排序,不允许有键值重复的情况出现
	
	WORD wRecsToSch;	//总共要搜索的记录个数，搜索到这个数，任务数据库就停止搜索
	//---------------------任务数据库返回给应用的信息-------------------------------------------
	WORD wRecSize;			//每笔的记录大小
	WORD wRecsFound;		//总共符合要求的记录个数,在TdbOpenSch()中初始化
	BYTE bValFirstRec[TDB_MAX_SCH_FIELD][TDB_SCH_VAL_LEN];  //第一笔记录的搜索字段的值
							//符合要求的第一笔记录的时间,应用在按照间隔来抽取数据时,
							//可以根据返回的这个时间来推算以后的每笔记录的时间
	//-----------------------任务数据库用来控制搜索的部分---------------------------------------
	int iPublicRecStart;    //文件中开始搜索的起始记录索引，最开始由应用设定为-1表示第一次调用，
							//后面由TdbInitSch()设定为它下一回开始搜索的起始位置, 公共字段
	int iPrivateRecStart;	//子字段
					
//	WORD wIndexID;	//使用了任务数据库的索引管理器的标识	
}TTdbSchCtrl;	    //搜索控制结构,支持二级排序

typedef struct{
	//-------------------应用给任务数据库的参数部分-----------------------------------------------
	DWORD dwFiledNeed;      //需要拷贝的字段,0~31位各对应一个字段,0xffffffff表示全拷贝
							//现在限定每笔记录最多32个字段,有些字段如果没必要区分则可以合并为一个BYTE_STR,增加长度即可
							//有些时候需要取记录的时候不需要时间或者表地址,可以使用本字段来控制
							//同一采集间隔的多种数据合成一个任务来执行时, 比如采集器把所有的遥测量放到了一个任务上,
							//可以通过本字段来提取想要的数据项	
	
	//-----------------------任务数据库用来控制搜索的部分---------------------------------------
	int iRecStart;			//本次搜索的起始位置,基于索引表，最开始由应用初始化为-1;
							//后面随着TdbReadRec()的调用而改变
} TTdbReadCtrl;   //读记录控制结构

class CTaskDB
{
public:
	CTaskDB(void);
public:
	~CTaskDB(void);
private:
    bool        m_fInit;
	TTabMgr		m_TTabMgr;			//数据表管理结构
	TTdbActivedTabInfo*	m_ptatHandle;//TTdbActivedTabInfo	m_tatHandle[TDB_MAX_OPENED_TAB];		//当前被打开的表句柄集合
	TSem		m_tsTabMgrSemaphore;
	TSem		m_tsTabSemaphore;
	BYTE*		m_pbSchBuf;//BYTE		m_pbSchBuf[TDB_MAX_BUF_SIZE];		//用来缓存搜索结果索引, 使用过程中这块缓存被分成很多小块
													//分配给各个搜索用
	BYTE*		m_pbBufIdxTab;//BYTE		m_dwBufIdxTab[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE];
	TTdbSchInfo*	m_ptsiConcurSch;//TTdbSchInfo	m_tsiConcurSch[TDB_MAX_CONCUR_SCH];	//搜索
	BYTE*		m_pbReadBuffer;//BYTE		m_bReadBuffer[TDB_FIELD_MAX_LEN];	//用于搜索/排序过程中缓存一笔纪录
	BYTE*       m_pbRecBuffer;//BYTE        m_bRecBuffer[TDB_MAX_CONCUR_SCH*TDB_RDBLOCK_SIZE];   //读取记录缓存，用来减少读取记录的次数，提高速度
	BYTE*       m_pbDelBuffer;//BYTE        m_pbDelBuffer[TDB_MAX_CONCUR_SCH*TDB_RDBLOCK_SIZE];   //删除记录缓存，用来减少写记录的次数，提高速度
	bool		m_fLocked;	//数据表是否被锁定

	//配置参数
	WORD		m_wMaxPath;		//数据表文件名最大长度
	WORD		m_wMaxTabNum;	//最多允许的表数量
	DWORD		m_dwMaxFieldLen; //一笔纪录最大总长度
	WORD		m_wMaxOpenedTabNum;//系统同一时刻最多允许的打开的表的数量
	DWORD		m_dwMaxBufSize;	//任务库最大可用缓存大小（为m_dwBufBlkSize的整数倍）[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE]
	DWORD		m_dwBufBlkSize;//任务库使用的缓存最小块大小（一般设置为（作为关键字的最长的字段长度[可以超过1个]+8）*n，n为最大记录数量）
	DWORD		m_dwMaxCCSchNum;//允许并发搜索的个数
	DWORD		m_dwMaxRecLen;//读取缓存大小(一笔记录最长的长度)
	//DWORD		m_dwMaxSchField;//一次搜索最多允许的参与搜索的关键字个数
	//DWORD		m_dwKeyLen;//最长的关键字字段长度
	DWORD		m_dwPeriCacheNum;//需要按时段缓存数据的时候，在时段内最多缓存记录笔数
	WORD		m_wSchMode;//0-从最新的记录开始查找，1-从最旧得记录开始查找
	char		m_szTdbDataPath[255];//任务数据库数据存放位置

private:
	int			GetTableIndex(const char* pszName);//指定表索引位置
	int			GetEmptyIndex();//在数据表管理结构中找出一个空位置
	int			SaveMgrData(DWORD dwOffset);//保存数据表管理结构信息 0xFFFFFFFF 只保存索引
	BYTE		CheckSum(const BYTE* bBuf, const WORD wLen);
	inline int	GetActiveTabInfo(int fd);	//fd为-1时，返回一个空的位置
	int			AllocBuf();		//为搜索分配缓存
	int			AllocSchID();				//分配一个搜索ID
	int			ReadOneRec(int fd, int iSchId, BYTE* pbBuf, DWORD dwOffset, DWORD dwLen);
	int         DeleteOneRec(int fd, int iSchId, DWORD dwOffset, DWORD dwLen);
	bool		Compare(BYTE *pbSrcBuf, BYTE *pbDesBuf, DWORD dwLen, WORD wOp, WORD wFieldType);
				//表含有公共字段，搜索条件中只有公共字段
	int			OpenPublicSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl &rTdbSchCtrl, BYTE *p);
				//表含有公共字段，搜索条件中只有子字段
	int			OpenPrivateSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl &rTdbSchCtrl, BYTE *p);
				//表含有公共字段，搜索条件中既有公共字段又有子字段
	int			OpenAllSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl &rTdbSchCtrl, BYTE *p);
				//表含不含有公共字段
	int			OpenNoPublicSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl &rTdbSchCtrl, BYTE *p);
	int			DeleteNoPublicSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum);
	void		SortRec(int iSchId, TTdbSchCtrl &rTdbSchCtrl, const WORD wRuleNum, BYTE *p, int iIndex);//对搜索结果排序
	bool		IsAllByte(BYTE bDesByte, WORD wLen, BYTE *p);
	int 		GetBuffer(int iSchId, BYTE *pbBuf, DWORD dwOffset, DWORD dwLen);
	int         SetBuffer(int iSchId, DWORD dwOffset, DWORD dwLen);
	int 		FreeBuffer(int iSchId);
	bool		IsLocked() { return m_fLocked; };

public:
	void		SetMaxPath(const WORD wMaxPath) { m_wMaxPath = wMaxPath; };		//数据表文件名最大长度
	void		SetMaxTabNum(const WORD wMaxTabNum) { m_wMaxTabNum = wMaxTabNum; };	//最多允许的表数量
	void		SetMaxFieldLen(const DWORD dwMaxFieldLen) { m_dwMaxFieldLen = dwMaxFieldLen; }; //一笔纪录最大总长度
	void		SetMaxOpenedTabNum(const WORD wMaxOpenedTabNum) { m_wMaxOpenedTabNum = wMaxOpenedTabNum; };//系统同一时刻最多允许的打开的表的数量
	void		SetMaxBufSize(const DWORD dwMaxBufSize) { m_dwMaxBufSize = dwMaxBufSize; };	//任务库最大可用缓存大小（为m_dwBufBlkSize的整数倍）[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE]
	void		SetBufBlkSize(const DWORD dwBufBlkSize) { m_dwBufBlkSize = dwBufBlkSize; };//任务库使用的缓存最小块大小（一般设置为（作为关键字的最长的字段长度[可以超过1个]+8）*n，n为最大记录数量）
	void		SetMaxSchNum(const DWORD dwMaxCCSchNum) { m_dwMaxCCSchNum = dwMaxCCSchNum; };//允许并发搜索的个数
	void		SetMaxRecLen(const DWORD dwMaxRecLen) { m_dwMaxRecLen = dwMaxRecLen; };//读取缓存大小(一笔记录最长的长度)
//	void		SetMaxSchField(const DWORD dwMaxSchField) { m_dwMaxSchField = dwMaxSchField; };//一次搜索最多允许的参与搜索的关键字个数
//	void		SetKeyWordLen(const DWORD dwKeyLen) { m_dwKeyLen = dwKeyLen; };//最长的关键字字段长度
	void		SetPeriCacheNum(const DWORD dwPeriCacheNum) { m_dwPeriCacheNum = dwPeriCacheNum; };//需要按时段缓存数据的时候，在时段内最多缓存记录笔数
	void		SetSchMode(const WORD wMode) { m_wSchMode = wMode; };//0-从最新的记录开始查找，1-从最旧得记录开始查找
	void		SetTdbDataPath(char* pszPath) { if (strlen(pszPath) < sizeof (m_szTdbDataPath)) strcpy(m_szTdbDataPath, pszPath); }

	WORD		GetMaxPath() { return m_wMaxPath; };		//数据表文件名最大长度
	WORD		GetMaxTabNum() { return m_wMaxTabNum; };	//最多允许的表数量
	DWORD		GetMaxFieldLen() { return m_dwMaxFieldLen; }; //一笔纪录最大总长度
	WORD		GetMaxOpenedTabNum() { return m_wMaxOpenedTabNum; };//系统同一时刻最多允许的打开的表的数量
	DWORD		GetMaxBufSize() { return m_dwMaxBufSize; };	//任务库最大可用缓存大小（为m_dwBufBlkSize的整数倍）[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE]
	DWORD		GetBufBlkSize() { return m_dwBufBlkSize; };//任务库使用的缓存最小块大小（一般设置为（作为关键字的最长的字段长度[可以超过1个]+8）*n，n为最大记录数量）
	DWORD		GetMaxSchNum() { return m_dwMaxCCSchNum; };//允许并发搜索的个数
	DWORD		GetMaxRecLen() { return m_dwMaxRecLen; };//读取缓存大小(一笔记录最长的长度)
//	DWORD		GetMaxSchField() { return m_dwMaxSchField; };//一次搜索最多允许的参与搜索的关键字个数
//	DWORD		GetKeyWordLen() { return m_dwKeyLen; };//最长的关键字字段长度
	DWORD		GetPeriCacheNum() { return m_dwPeriCacheNum; };//需要按时段缓存数据的时候，在时段内最多缓存记录笔数
	WORD		GetSchMode() { return m_wSchMode; };//0-从最新的记录开始查找，1-从最旧得记录开始查找
	char*		GetTdbDataPath() { return m_szTdbDataPath;};

	bool		Init(char* pszPath=NULL);	//初始化数据表管理信息
	int			CreateTable(const char* pszName, TTabCtrl& rTabCtrl);//创建指定表
	int			ClearRec(const char* pszName);//清除指定表的记录
	int         DeleteSchRec(int fd, TTdbSchRule *pTdbSchRule, WORD wRuleNum); //删除符合搜索条件的记录
	int			DeleteTable(const char* pszName);//删除指定表
	int			OpenTable(const char* pszName, int iOpenFlag);//打开指定表
	int			CloseTable(const int fd);//关闭指定表
	int			AppendRec(const int fd, int index, const BYTE* pbRec); //添加指定位置一笔记录
	int			AppendRec(const int fd, const BYTE* pbRec, bool fUseCache=true);//添加一条记录
	int			ReadRec(const int iSchId, TTdbSchRule* pTdbSchRule, const WORD wRuleNum, TTdbReadCtrl& rTdbReadCtrl, BYTE* pbBuf);
	int         ReadRec(const int fd, int index, BYTE* pbBuf, int iLen);
	int			ReadRec(const int fd, int* piStart, BYTE* pbBuf, WORD wBufSize, bool fFromEnd, WORD* pwRetNum);
	int         GetRecNum(const int fd);
	int			GetMaxRecNum(const int fd);
	int         GetRecPtr(const int fd);
				//读出相关记录
	int			OpenSch(const int fd, TTdbSchRule* pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl& rTdbSchCtrl);
				//请求搜索相关记录
	int			CloseSch(const int iSchId);//结束搜索	
	inline bool IsInit() { return m_fInit; };
	void		Lock();
	void		UnLock();
};


//////////////////////////////////////////////////////////////////////////////////////
//全局定义
extern CTaskDB* GetTaskDB();

//描述：初始化任务数据库
//参数：@ pszPath 任务库保存路径
inline bool TdbInit(char* pszPath=NULL)
{
	return GetTaskDB()->Init(pszPath);
}

//描述：向数据库申请创建一张表
//参数：@pszName 表的名字,是一个不带路径的文件名,数据库会自动把文件保存到数据库的目录下
//      @wRecSize 每笔记录的大小
//      @wRecNum  记录的个数
//返回：如果正确则返回true，否则返回false
inline int TdbCreateTable(char* pszName, TTabCtrl& rTabCtrl)
{
	return GetTaskDB()->CreateTable(pszName, rTabCtrl);
}

//描述：清空指定表的记录，但是不删除表
//参数：@pszName 表的名字
//返回：无错误返回TDB_ERR_OK ,其余错误类型见头文件中"错误类型"
inline int TdbClearRec(char* pszName)
{
	return GetTaskDB()->ClearRec(pszName);
}

//描述：向数据库申请删除一张表
//参数：@pszName 表的名字
//返回：如果正确则返回true，否则返回false
inline int TdbDeleteTable(char* pszName)
{
	return GetTaskDB()->DeleteTable(pszName);
}

//描述：向数据库申请打开一张表
//参数：@pszName 表的名字
//	   @ iOpenFlag 定义同文件系统中打开文件函数open中的flag相同
//		 (有以下类型O_RDONLY/O_WRONLY/O_RDWR)
//返回：如果正确则返回表的标识符，大于等于0，否则返回负数
inline int TdbOpenTable(char* pszName, int iOpenFlag)
{
	return GetTaskDB()->OpenTable(pszName, iOpenFlag);
}

//描述：向数据库申请关闭一张表
//参数：@pszName 表的名字
//返回：如果正确则返回true，否则返回false
inline int TdbCloseTable(int fd)
{
	return GetTaskDB()->CloseTable(fd);
}

//描述：向数据库的表fd添加一笔记录
//参数：@fd 数据库表的标识符
//      @pbRec 指向记录的缓冲区
//返回：如果正确则返回true，否则返回false
inline int TdbAppendRec(int fd, BYTE* pbRec)
{
	return GetTaskDB()->AppendRec(fd, pbRec);
}

//描述：向数据库的表fd添加指定位置的一笔纪录
//参数：@fd 数据库表的标识符
//      @index 记录位置
//	    @pbRec 指向记录的缓冲区
//返回：如果正确则返回记录的长度,小于表示错误,0表示无此位置的记录
inline int TdbAppendRec(const int fd, int index, const BYTE* pbRec)
{
	return GetTaskDB()->AppendRec(fd, index, pbRec);
}

//描述：删除任务数据库符合条件的记录
//     @fd 数据库表的标识符
//     @pTdbSchRule 指向任务记录搜索规则数组的指针
//     @wRuleNum 规则的条数
//返回：如果正确则返回本次删除条数，否则返回负数
inline int TdbDeleteSchRec(int fd, TTdbSchRule *pTdbSchRule, WORD wRuleNum)
{
	return GetTaskDB()->DeleteSchRec(fd, pTdbSchRule, wRuleNum);
}

//描述：初始化搜索引擎
//     @fd 数据库表的标识符
//     @pTdbSchRule 指向任务记录搜索规则数组的指针
//     @wRuleNum 规则的条数
//     @rTdbSchCtrl 搜索控制结构
//返回：如果正确则返回索引表id>=0，否则返回负数
inline int TdbOpenSch(int fd, TTdbSchRule* pTdbSchRule, WORD wRuleNum, TTdbSchCtrl& rTdbSchCtrl)
{
	return GetTaskDB()->OpenSch(fd, pTdbSchRule, wRuleNum, rTdbSchCtrl);
}

//描述：关闭搜索引擎
//参数：@id 索引表id
//返回：如果正确则返回true，否则返回false
inline int TdbCloseSch(int id)
{
	return GetTaskDB()->CloseSch(id);
}

//描述：从数据库中根据搜索规则读取记录,如果是二级记录的表,则返回的记录是公共字段加上子记录
//参数：@fd 数据库表的标识符
//      @pTdbSchRule 指向任务记录搜索规则数组的指针
//      @wRuleNum 规则的条数
//      @rTdbSchCtrl 搜索控制结构
//	    @pbBuf 用来返回任务记录的缓冲区
//返回：如果正确则返回缓存区内记录的长度,小于表示错误,0表示无数据
inline int TdbReadRec(int iSchId, 
			   TTdbSchRule* pTdbSchRule, WORD wRuleNum, TTdbReadCtrl& rTdbReadCtrl, BYTE* pbBuf)
{
	return GetTaskDB()->ReadRec(iSchId, pTdbSchRule, wRuleNum, rTdbReadCtrl, pbBuf);
}

//描述：从数据库中读取指定位置的一笔纪录
//参数：@fd 数据库表的标识符
//      @index 记录位置
//	    @pbBuf 用来返回任务记录的缓冲区
//返回：如果正确则返回缓存区内记录的长度,小于表示错误,0表示无数据
inline int TdbReadRec(const int fd, int index, BYTE* pbBuf, int iLen)
{
	return GetTaskDB()->ReadRec(fd, index, pbBuf, iLen);
}

//描述：从任务库中指定索引连续读取多笔记录
//参数：@fd数据库表的标识符
//	   @piStart用来传入及返回记录索引的起始位置,第一次调用传递-1，
//				后续调用者不能改变,搜索结束时，本函数把它设置为-2
//@pbBuf用来返回任务记录的缓冲区
//     @wBufSize缓冲区的大小
//     @fFromEnd是否从最后一笔记录开始倒着搜索，否则从第一笔记录顺序搜索
//   @pwRetNum本次读取到的记录笔数，只有在文件结束时才会没达到要求笔数
//返回：如果正确则返回缓存区内记录的长度,小于0表示错误,0表示无数据
inline int TdbReadRec(const int fd, int* piStart, BYTE* pbBuf, WORD wBufSize, bool fFromEnd, WORD* pwRetNum)
{
	return GetTaskDB()->ReadRec(fd, piStart, pbBuf, wBufSize, fFromEnd, pwRetNum);
}

//描述：当前表记录数
//参数：@fd 数据库表的句柄
//返回：当前表的记录数,小于表示错误
inline int TdbGetRecNum(const int fd)
{
    return GetTaskDB()->GetRecNum(fd);
}



//描述：当前表最大可记录笔数
//参数：@fd 数据库表的句柄
//返回：当前表的最大可记录笔数,小于表示错误
inline int TdbGetMaxRecNum(const int fd)
{
	//return GetTaskDB()->GetMaxRecNum(fd);
	return 1200;   //for test whr 20170715
}


//描述：当前表记录指针
//参数：@fd 数据库表的句柄
//返回：当前表的记录指针,小于表示错误
inline int TdbGetRecPtr(const int fd)
{
    return GetTaskDB()->GetRecPtr(fd);
}

//锁定数据库，如删除操作时时间可能比较长，则需要先锁定数据库。
inline void TdbLock()
{
    GetTaskDB()->Lock();
}

inline void TdbUnLock()
{
    GetTaskDB()->UnLock();
}

//任务库是否已初始化
inline bool TdbIsInit() 
{ 
    return GetTaskDB()->IsInit(); 
}

//设置数据表文件名最大长度(默认值:255)
inline void TdbSetMaxPath(const WORD wMaxPath)
{
	GetTaskDB()->SetMaxPath(wMaxPath);
}

//设置最多允许的表数量(默认值:255)
inline void TdbSetMaxTabNum(const WORD wMaxTabNum)
{
	GetTaskDB()->SetMaxTabNum(wMaxTabNum);
}

//设置一笔纪录最大总长度(默认值:256)
inline void TdbSetMaxFieldLen(const DWORD dwMaxFieldLen)
{
	GetTaskDB()->SetMaxFieldLen(dwMaxFieldLen);
}

//设置系统同一时刻最多允许的打开的表的数量(默认值:32)
inline void TdbSetMaxOpenedTabNum(const WORD wMaxOpenedTabNum)
{
	GetTaskDB()->SetMaxOpenedTabNum(wMaxOpenedTabNum);
}

//设置任务库最大可用缓存大小（为SetBufBlkSize的整数倍）(默认值:0x80000)
inline void TdbSetMaxBufSize(const DWORD dwMaxBufSize)
{
	GetTaskDB()->SetMaxBufSize(dwMaxBufSize);
}

//设置任务库使用的缓存最小块大小（一般设置为（作为关键字的最长的
//字段长度[可以超过1个]+8）*n，n为最大记录数量）(默认值:0x20000)
inline void TdbSetBufBlkSize(const DWORD dwBufBlkSize)
{
	GetTaskDB()->SetBufBlkSize(dwBufBlkSize);
}

//设置允许并发搜索的个数(默认值:5)
inline void TdbSetMaxSchNum(const DWORD dwMaxCCSchNum)
{
	GetTaskDB()->SetMaxSchNum(dwMaxCCSchNum);
}

//设置读取缓存大小(一笔记录最长的长度)(默认值:1024)
inline void TdbSetMaxRecLen(const DWORD dwMaxRecLen)
{
	GetTaskDB()->SetMaxRecLen(dwMaxRecLen);
}

/*
//设置一次搜索最多允许的参与搜索的关键字个数(默认值:8)
inline void TdbSetMaxSchField(const DWORD dwMaxSchField)
{
	GetTaskDB()->SetMaxSchField(dwMaxSchField);
}

//设置最长的关键字字段长度(默认值:255)
inline void TdbSetKeyWordLen(const DWORD dwKeyLen)
{
	GetTaskDB()->SetKeyWordLen(dwKeyLen);
}
*/

//设置需要按时段缓存数据的时候，在时段内最多缓存记录笔数(默认值:10)
inline void	TdbSetPeriCacheNum(const DWORD dwPeriCacheNum)
{
	GetTaskDB()->SetPeriCacheNum(dwPeriCacheNum);
}

//0-从最新的记录开始查找，1-从最旧得记录开始查找
inline void TdbSetSchMode(const WORD wMode)
{
	GetTaskDB()->SetSchMode(wMode);
}

//数据表文件名最大长度
inline WORD	TdbGetMaxPath() 
{ 
	return GetTaskDB()->GetMaxPath();
}

//最多允许的表数量
inline WORD	TdbGetMaxTabNum() 
{ 
	return GetTaskDB()->GetMaxTabNum(); 
}

//一笔纪录最大总长度
inline DWORD TdbGetMaxFieldLen()
{ 
	return GetTaskDB()->GetMaxFieldLen(); 
}

//系统同一时刻最多允许的打开的表的数量
inline WORD TdbGetMaxOpenedTabNum() 
{ 
	return GetTaskDB()->GetMaxOpenedTabNum(); 
}

//任务库最大可用缓存大小（为m_dwBufBlkSize的整数倍）[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE]
inline DWORD TdbGetMaxBufSize() 
{ 
	return GetTaskDB()->GetMaxBufSize();
}

//任务库使用的缓存最小块大小（一般设置为（作为关键字的最长的字段长度[可以超过1个]+8）*n，n为最大记录数量）
inline DWORD TdbGetBufBlkSize() 
{ 
	return GetTaskDB()->GetBufBlkSize(); 
}

//允许并发搜索的个数
inline DWORD TdbGetMaxSchNum() 
{ 
	return GetTaskDB()->GetMaxSchNum();
}

//读取缓存大小(一笔记录最长的长度)
inline DWORD TdbGetMaxRecLen() 
{ 
	return GetTaskDB()->GetMaxRecLen(); 
}

/*
//一次搜索最多允许的参与搜索的关键字个数
inline DWORD TdbGetMaxSchField() 
{ 
	return GetTaskDB()->GetMaxSchField(); 
}

//最长的关键字字段长度
inline DWORD TdbGetKeyWordLen() 
{ 
	return GetTaskDB()->GetKeyWordLen(); 
}
*/

//需要按时段缓存数据的时候，在时段内最多缓存记录笔数
inline DWORD TdbGetPeriCacheNum() 
{ 
	return GetTaskDB()->GetPeriCacheNum(); 
}

//0-从最新的记录开始查找，1-从最旧得记录开始查找
inline WORD TdbGetSchMode()
{
	return GetTaskDB()->GetSchMode();
}

//设置任务数据库数据存放的地方
inline void SetTdbDataPath(char* pszPath)
{
	GetTaskDB()->SetTdbDataPath(pszPath);
}

//取得当前任务数据库数据存放的地方
inline char* TdbGetTdbDataPath()
{
	return GetTaskDB()->GetTdbDataPath();
}
#endif
