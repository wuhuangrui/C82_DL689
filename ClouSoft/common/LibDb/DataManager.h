#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "FaStruct.h"
#include "FaConst.h"
#include "sysarch.h"
#include "LibDbStruct.h"

#define IMG_MAX_TIMEOUTS  300   //5分钟

typedef struct{
	DWORD dwStartTime;  //镜像的起始时间,单位S
	DWORD dwEndTime;	//镜像的结束时间,单位S,最大超时的1/2间隔,且大于5分钟的取5分钟
}TImgCtrl;				//镜像的控制结构

class CDataManager 
{
public:
    CDataManager();
    virtual ~CDataManager();
    
	bool Init(TDbCtrl* pDbCtrl);
	TDataItem GetItem(WORD wPoint, WORD wID);

	int ItemsLen(WORD* pwItemID, WORD wLen);
	int Save(bool fSaveAll=true);
	int SavePara();
	int SaveData(bool fSaveAll=true);
	void TrigerSaveAll();
	void TrigerSavePara();
	void DoSave();
	bool IsImgItem(WORD wBank, WORD wPn, WORD wID);

	int ReadItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess& rItemAcess, DWORD dwStartTime=0, DWORD dwEndTime=0);
	int WriteItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess& rItemAcess, BYTE bPerm=0, BYTE* pbPassword=NULL, DWORD dwTime=0);
	int UpdReadMeterFail(WORD wBank, WORD wPoint, WORD wID, WORD wReason);
	TDataItem GetItemEx(WORD wBank, WORD wPoint, WORD wID);
	void NewImg(DWORD dwStartTime, WORD wInterval);
	void TimeAdjBackward(DWORD dwTime);
	void ClearData();
	void ClearPara();
	bool ClearBankData(WORD wBank, WORD wSect, int iPn=-1);
	bool ClrPnData(WORD wBank, WORD wSect, WORD wPnNum, WORD wPn);
	void SaveVolatile();
	void SetMeterPnMask(BYTE* pbMeterPnMask);
	
	void TrigerSaveBank(WORD wBank, WORD wSect, int iFile);
	void DoTrigerSaveBank();
	void DoSelfIntervSave();

	int UpgReadItem(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime);

	//测量点动态映射
	int SearchPnMap(BYTE bSch, WORD wPn);
	int NewPnMap(BYTE bSch, WORD wPn);
	bool DeletePnMap(BYTE bSch, WORD wPn);
	int GetPnMapRealNum(BYTE bSch);
	int MapToPn(BYTE bSch, WORD wMn);
	
protected:
	TDbCtrl* m_pDbCtrl;	//外界对数据库进行参数配置的数据库控制结构

	//为了访问方便,参数m_pDbCtrl中的部分变量拷贝出来直接使用
	WORD 		m_wSectNum;		//BANK0中的SECT数目
	TBankCtrl* 	m_pBank0Ctrl;
	WORD 		m_wBankNum;		//支持的BANK数目
	TBankCtrl* 	m_pBankCtrl;
	int			m_iSectImg;		//485抄表数据镜像段,如果没有则配成-1
	WORD		m_wImgNum;		//485抄表数据镜像个数
	WORD 		m_wPnMapNum;  	//支持的映射方案数目,整个数据库不支持测量点动态映射则设为0
	TPnMapCtrl*	m_pPnMapCtrl; 	//整个数据库不支持测量点动态映射则设为NULL
	TDbUpgCtrl* m_pDbUpgCtrl;	//数据库版本升级控制,配置为NULL表示没有升级配置

	TImgCtrl* m_pImgCtrl;
    int 	  m_nLastLen;
	BYTE* 	  m_pbMeterPnMask;

	DWORD m_dwSaveClick;
	bool m_fTrigerSaveAll;
	bool m_fTrigerSavePara;

	bool m_fTrigerSaveBank;
	bool m_fSavePnMap;		//用来触发保存测量点动态映射表
	BYTE m_bSectSaveFlg[SECT_MAX][BANK_FILE_FLG_SIZE];   //BANK_FILE_FLG_SIZE中每1位表示一个文件
	BYTE m_bBankSaveFlg[BANK_MAX][BANK_FILE_FLG_SIZE];   //BANK_FILE_FLG_SIZE中每1位表示一个文件

	BYTE m_bTmpSectSaveFlg[SECT_MAX][BANK_FILE_FLG_SIZE];
	BYTE m_bTmpBankSaveFlg[BANK_MAX][BANK_FILE_FLG_SIZE];

	//测量点动态映射
	TSem m_semPnMap;
	DWORD m_dwPnMapFileFlg;	//每1位置1表示一个文件发生了修改
	DWORD m_dwMemUsage;	  //内存使用量,单位字节,包括数据和时标存储空间
    
	bool InitDir();
	bool InitBank(TBankCtrl* pBankCtrl);
	int SaveBank(TBankCtrl* pBankCtrl, int iFile=-1);
	void LoadOneFileDefault(TBankCtrl* pBankCtrl, WORD wFile, DWORD dwOffset);
	void LoadBankDefault(TBankCtrl* pBankCtrl, int iFile, DWORD dwOffset);
	void ClearBank(TBankCtrl* pBankCtrl);
	bool IsMeterPn(WORD wPn);
	void DeleteBank(TBankCtrl* pBankCtrl);
	void DeleteBankFile(TBankCtrl* pBankCtrl);

	//测量点动态映射
	bool InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum);
	int SavePnMap();

	//数据库版本升级
	bool m_fDbUpg;
	bool InitUpgrade(TDbUpgCtrl* pDbUpgCtrl);
	void DoUpgrade(TDbUpgCtrl* pDbUpgCtrl);
};

#endif  //DATAMANAGER_H


