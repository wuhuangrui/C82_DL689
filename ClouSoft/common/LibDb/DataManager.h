#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "FaStruct.h"
#include "FaConst.h"
#include "sysarch.h"
#include "LibDbStruct.h"

#define IMG_MAX_TIMEOUTS  300   //5����

typedef struct{
	DWORD dwStartTime;  //�������ʼʱ��,��λS
	DWORD dwEndTime;	//����Ľ���ʱ��,��λS,���ʱ��1/2���,�Ҵ���5���ӵ�ȡ5����
}TImgCtrl;				//����Ŀ��ƽṹ

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

	//�����㶯̬ӳ��
	int SearchPnMap(BYTE bSch, WORD wPn);
	int NewPnMap(BYTE bSch, WORD wPn);
	bool DeletePnMap(BYTE bSch, WORD wPn);
	int GetPnMapRealNum(BYTE bSch);
	int MapToPn(BYTE bSch, WORD wMn);
	
protected:
	TDbCtrl* m_pDbCtrl;	//�������ݿ���в������õ����ݿ���ƽṹ

	//Ϊ�˷��ʷ���,����m_pDbCtrl�еĲ��ֱ�����������ֱ��ʹ��
	WORD 		m_wSectNum;		//BANK0�е�SECT��Ŀ
	TBankCtrl* 	m_pBank0Ctrl;
	WORD 		m_wBankNum;		//֧�ֵ�BANK��Ŀ
	TBankCtrl* 	m_pBankCtrl;
	int			m_iSectImg;		//485�������ݾ����,���û�������-1
	WORD		m_wImgNum;		//485�������ݾ������
	WORD 		m_wPnMapNum;  	//֧�ֵ�ӳ�䷽����Ŀ,�������ݿⲻ֧�ֲ����㶯̬ӳ������Ϊ0
	TPnMapCtrl*	m_pPnMapCtrl; 	//�������ݿⲻ֧�ֲ����㶯̬ӳ������ΪNULL
	TDbUpgCtrl* m_pDbUpgCtrl;	//���ݿ�汾��������,����ΪNULL��ʾû����������

	TImgCtrl* m_pImgCtrl;
    int 	  m_nLastLen;
	BYTE* 	  m_pbMeterPnMask;

	DWORD m_dwSaveClick;
	bool m_fTrigerSaveAll;
	bool m_fTrigerSavePara;

	bool m_fTrigerSaveBank;
	bool m_fSavePnMap;		//����������������㶯̬ӳ���
	BYTE m_bSectSaveFlg[SECT_MAX][BANK_FILE_FLG_SIZE];   //BANK_FILE_FLG_SIZE��ÿ1λ��ʾһ���ļ�
	BYTE m_bBankSaveFlg[BANK_MAX][BANK_FILE_FLG_SIZE];   //BANK_FILE_FLG_SIZE��ÿ1λ��ʾһ���ļ�

	BYTE m_bTmpSectSaveFlg[SECT_MAX][BANK_FILE_FLG_SIZE];
	BYTE m_bTmpBankSaveFlg[BANK_MAX][BANK_FILE_FLG_SIZE];

	//�����㶯̬ӳ��
	TSem m_semPnMap;
	DWORD m_dwPnMapFileFlg;	//ÿ1λ��1��ʾһ���ļ��������޸�
	DWORD m_dwMemUsage;	  //�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�
    
	bool InitDir();
	bool InitBank(TBankCtrl* pBankCtrl);
	int SaveBank(TBankCtrl* pBankCtrl, int iFile=-1);
	void LoadOneFileDefault(TBankCtrl* pBankCtrl, WORD wFile, DWORD dwOffset);
	void LoadBankDefault(TBankCtrl* pBankCtrl, int iFile, DWORD dwOffset);
	void ClearBank(TBankCtrl* pBankCtrl);
	bool IsMeterPn(WORD wPn);
	void DeleteBank(TBankCtrl* pBankCtrl);
	void DeleteBankFile(TBankCtrl* pBankCtrl);

	//�����㶯̬ӳ��
	bool InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum);
	int SavePnMap();

	//���ݿ�汾����
	bool m_fDbUpg;
	bool InitUpgrade(TDbUpgCtrl* pDbUpgCtrl);
	void DoUpgrade(TDbUpgCtrl* pDbUpgCtrl);
};

#endif  //DATAMANAGER_H


