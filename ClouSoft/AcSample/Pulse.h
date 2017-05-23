#ifndef PULSE_H
#define PULSE_H

#include "AcConst.h"
#include "Energy.h"
#include "Demand2.h"

#define IC12					12
#define OI_PULSE_BASE			0x2401
#define OI_PULSE_INPUT			0xF20A	//���������豸
#define OI_YX					0xF203

#define PULSE_HI_POSE_ID	0x2414	//���������й��߾��ȵ���ID

#define PULSE_PN_NUM		8		//������������

#define TICK_BUF_LENGTH 	2000
#define PUSLE_ADDR_LEN		16		//ͨ�ŵ�ַ����
#define PULSE_CFG_LEN		12
#define PULSE_CFG_ID_LEN	(PULSE_CFG_LEN*MAX_PULSE_TYPE + 2)	//������������Ա�ID����


//�����1���������õ�Ԫ�е�ƫ��
#define OFFSET_PULSE_PORT				3		//��������˿�OAD
#define OFFSET_PULSE_TYPE				8		//��������ƫ��
#define OFFSET_PULSE_CONST				10		//���峣��	


// ����ӿ��෽��
#define OMD_PULSE_RESET					OMD1					//��λ
#define OMD_PULSE_RUN					OMD2					//ִ��
#define OMD_PULSE_ADDCFG				OMD3					//����������뵥Ԫ
#define OMD_PULSE_DELCFG				OMD4					//ɾ��һ���������뵥Ԫ

typedef struct
{
	DWORD	Pulse;
	short	WritePtr;
	DWORD	TickBuf[TICK_BUF_LENGTH];
}TPulseInData;

typedef struct
{
	WORD wPn;		//��·���������������
	BYTE bPortNo;	//����˿ں�
	BYTE bType;		//��������
	int64 i64Const;	//�����
}TPulseCfg;			//��������

typedef struct
{
    WORD  wPn;			//�����������
    //bool  fPosEpValid;	//�����й�������Ч
    //bool  fPosEqValid;	//�����޹�������Ч
    //WORD  wAutoDate[AUTO_DATE_NUM];
	//WORD  wRate;   		//��ǰ�ķ���,�ɼ�ʱˢ��
    //int64 i64PT;
	//int64 i64CT;
	//int64 iPowerRatio;	
	//TPulseCfg PulseCfg;
	TEnergyPara EnergyPara; //���ܲ���
	//TDemandPara	DemandPara; //��������
	BYTE		bAddr[PUSLE_ADDR_LEN];	//ͨ�ŵ�ַ
}TPulsePara;		//�������

typedef struct{
	WORD wPn;	//�������
	BYTE bPortNo[MAX_PULSE_TYPE];	//����˿ں�
	BYTE bIndex[MAX_PULSE_TYPE];	//�������
	bool fPosEpValid;	//������Ч
	bool fPosEqValid;	//������Ч
}TPulsePnDesc;	//�������������


bool PulseLoadPara(TPulseCfg* pPulseCfg, TPulsePara* pPulsePara);

class CPulse {	//������ (��·����)
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

	CEnergy    m_Energy;    //����ʵ���
	//CDemand2   m_Demand;	 //����

	BYTE   m_bEnergyMinute;
	int64  m_i64Power;	//ʵʱ����
	WORD   m_wPowerPtr;	
	DWORD  m_dwPulse[ENERGY_NUM_MAX];  	//���ĵ���������
	DWORD  m_dwLastPulse[ENERGY_NUM_MAX];  //�ϴμ���ʱ�ĵ���������
	DWORD m_dwLastDateMin[AUTO_DATE_NUM];		//�ϴγ�����ִ�еķ���
	bool m_fDateAdjBackward[AUTO_DATE_NUM];	//������ִ��ʱ����ǰ����

	bool   m_fValid;
	DWORD  m_dwLastAutoDate[AUTO_DATE_NUM];
	bool   m_fTrigerSave;	//�ڷ��ʡ��·ݻ򳭱��շ����л���ʱ�򣬴������ݿ�ȥ���汾�����������
	//bool   m_fClrDemand;	//��������־,����߳�����,�����߳�ȥִ��
	bool   m_fStopSaveLog;
	bool   m_fPowerUp;

	

	void   RunMeter();
	void   CalPower();	
};


class CPulseManager {	//���������
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
		bool IsPulsePnInvalid(TPulsePnDesc* pPnDesc);	//�ò����������Ƿ���Ч
		void CalcStatEnergy();	//�����������������ͳ������
		void CalcPnStatEnergy(WORD wPnIndex, bool fClrDayEnergy, bool fClrMonthEnergy);		//����/��0ĳ���������ͳ������

	private:
		BYTE m_bYMFlag;			//����ռ��ң�ű�־ 1��ռ�ã� 0����ռ��
		BYTE m_bPulseNum;		//������·��
		BYTE m_bPulsePnNum;		//�����ܲ�������
		DWORD m_dwLastTick;
		DWORD m_dwLastStatClick;

		int64 m_i64LastE[MAX_PULSE_TYPE][RATE_NUM+1];	//��һ�ε�������2419~241c��
	
		TPulsePnDesc m_PulsePnDesc[PULSE_PN_NUM];		//�������������		
		
		CPulse m_Pulse[MAX_YMNUM];		//��·�������
};


extern TPulseInData g_PulseInData[];
extern CPulseManager g_PulseManager;	//���������

int OnResePulseCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int BatAddPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);
int OnRunPulseCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnAddPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnDelPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
#endif
