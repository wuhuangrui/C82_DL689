/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：GprsWorker.h
 * 摘    要：本文件主要实现GPRS工作线程
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年8月
 * 备    注：
 *********************************************************************************************************/
#ifndef GPRSWORKER_H
#define GPRSWORKER_H
#include "Modem.h"
#include "FrmQue.h"
#include "LibDbStruct.h"

//GPRS接口的状态
#define GPRS_STATE_IDLE		0
#define GPRS_STATE_OL		1
#define GPRS_STATE_SMS		2

#define GPRS_RST_ON_IDLE	0
#define GPRS_RST_ON_SMS		1

#define SOCK_LED_IDLE		0
#define SOCK_LED_LIGHT		1
#define SOCK_LED_DARK		2

typedef struct{
	TModemPara	ModemPara;
	TCommPara CommPara; //串口配置

	WORD  wConnectWait;	    	//连接等待时间
	WORD  wRstNum;
	WORD  wDormanToPwroffCnt;	//上网失败进入休眠状态达到这么多次后关模块电源
	bool  fEnableRingActive; 	 //允许振铃激活
	bool  fEnableAutoSendActive; //允许主动上报激活
	bool  fEnSocketLed;		//是否控制Socket Led,只针对国网标准模块	
	bool  fEnMux;			//是否允许串口复用
	
	//消息宏定义,避免编译过程中库用到的宏定义跟应用程序不一致
	WORD wInfoActive;		//INFO_ACTIVE

	char szPppUser[64];
	char szPppPsw[64];
	bool fEmbedProtocol;	//是否使用模块协议栈
	bool fDetailDiagnose;	//是否进行详细诊断
	//TDataItem diCID;		//保存SIM卡序列号
	TNetInfo *ptNetInfo;	//获取网络信息
	//TDataItem diIMSI;		//保存IMSI
	//TDataItem diCGMR;		//保存模块软件版本号
	TDataItem diGPS;		//保存GPS状态信息
	WORD wUpdTxPwrInterv;	//更新发射功率的间隔，单位秒，为0表示不更新
}TGprsWorkerPara;

class CGprsWorker
{
public:
    CGprsWorker();
    virtual ~CGprsWorker();

	CModem* m_pModem;
	TGprsWorkerPara* m_pWorkerPara;
    
	bool Init(TGprsWorkerPara* pWorkerPara);
	void AttachModem(CModem* pModem) { m_pModem = pModem; } ;
	void LoadUnrstPara();
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TGprsWorkerPara* pWorkerPara))
	{		//注册装载非复位参数的函数
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	WORD GetState() { return m_wState; };
    DWORD GetOnlineReqFlg() { return m_dwOnlineReqFlg;};
	bool IsOnline() { return m_wState==GPRS_STATE_OL; };
	bool IsKeepOnline() { return m_dwOnlineReqFlg != 0; };
	int  GetLastErr(void) { return m_iLastErr; }
	int  ReqUserID();
	void ReleaseUserID(int iUser);
	void ReqOnline(int iUser);
	void ReqOffline(int iUser);
	void ReqDorman(int iUser);
	void SetWorkMode(int iUser, BYTE bRstOnSms);
	void RunThread();
	void SetSockLedAct(BYTE bAct) { m_bLedAct = bAct;}	//0初值不动作，1表示点亮，2表示熄灭
	
	WORD SignStrength() { return m_wSignStrength; };
	WORD GetErrBytes(); //{ return m_pModem->GetErrBytes(); };
	int GetPppPd() { return m_iPd; };
	
	WORD ReceiveSms(BYTE* pbFrm) { return m_SmsRxFrmQue.Remove(pbFrm, 100); };
	bool SendSms(BYTE* pbFrm, WORD wLen) { return m_SmsTxFrmQue.Append(pbFrm, wLen, 2000); };
	bool SendTextSms(BYTE* pbSmsAddr, BYTE bSmsAddrLen, BYTE* pbTxBuf, WORD wLen);
	CModem* GetModem() { return m_pModem; };
	//bool SetNetInfo(BYTE bNetType);
	int GetGprsWorkLastErr();
	WORD GetGprsWorkStep();
	
protected:
	bool (*m_pfnLoadUnrstPara)(TGprsWorkerPara* pWorkerPara);
	int	   m_iLastErr;//上一次错误
	CComm  m_Comm;
	int    m_iPd;	//ppp的设备号,只在lwip中用到
	WORD m_wSignStrength;
	TSem  m_semWorker;
	TSem  m_semWorkerLocker;
	WORD m_wState;
	bool m_fModemPwrOn;
	WORD m_wFailCnt;		//上网失败计数
	WORD m_wDormanCnt;	//主用户请求进入休眠状态计数
	
	DWORD m_dwUser;
	DWORD m_dwOnlineReqFlg;	//对应位为1表示该用户要上线
	DWORD m_dwRstOnSmsFlag;	//对应位为1表示该用户下线后要处于短信状态
	
	DWORD m_dwDebugClick;
	DWORD m_dwSmsOverflowClick;
	DWORD m_dwSignClick;
	DWORD m_dwUpdTxPwrClick;
	DWORD m_dwMuxClick;	
	DWORD m_dwUpdSysInfoClick;

	CFrmQue m_SmsRxFrmQue;	//短信接收的帧队列
	CFrmQue m_SmsTxFrmQue;	//短信发送的帧队列
	
	WORD m_wGprsWorkStep;
	BYTE m_bLedAct;

	//平台相关的代码
	bool ClosePpp(void);
	bool OpenPpp(void);
	bool UpdateSignStrength();
	int ResetGprs(WORD wCnMode);
	bool ReadTxPwrAndSign(BYTE* pbTxPwr, int16* pbSign);
	void UpdTxPwr();
	void UpdSysInfo();
	void DoLed();

	void ResetToOfflineState();
	void ResetToOnlineState();

	void DoIdleState();
	void DoOnlineState();
	void DoSmsState();	
	bool DoOnlineSms();
	int ResetModem(bool fModemPwrOn, WORD wCnMode);
};

int ReqOnline(int iGprsUser);
bool ReqOffline(int iGprsUser);
bool ReqDorman(int iUser);
WORD GetSignStrength(void);
int GetGprsWorkLastErr();
WORD GetGprsWorkStep();
CGprsWorker* GetGprsWorker();
void GprsWorkerSetSockLed(BYTE bAct);

TThreadRet GprsWorkerThread(void* pvArg);

#endif  //GPRSWORKER_H
