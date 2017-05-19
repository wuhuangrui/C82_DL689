/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Modem.h
 * 摘    要：本文件实现了通信MODEM的基类定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：可作为GPRS,CDMA和电话MODEM的基类
 *********************************************************************************************************/
#ifndef MODEM_H
#define MODEM_H
#include "DrvConst.h"
#include "Comm.h"
#include "drivers.h"
#include "LibDbStruct.h"
#include "FaConst.h"

#define GSN_LEN    		 15
#define PHONE_NUM_LEN    32
#define ACTIVE_SMS_LEN   32

#define MODEM_STEP_INIT	 0	//初始化
#define MODEM_STEP_RST	 1	//复位模块
#define MODEM_STEP_SIM	 2	//检测SIM卡
#define MODEM_STEP_REG	 3	//注册网咯

#define MODEM_NO_ERROR	 0	//没有错误
#define MODEM_RST_FAIL	 1	//模块复位失败
#define MODEM_SIM_FAIL	 2  //检测SIM失败
#define	MODEM_REG_FAIL	 3	//注册网咯失败

typedef struct {
	//CComm*  pComm;
	char szAPN[64];
	
	bool fEnSms;			//是否允许短信
	bool fMstSmsAddrValid;   //主站短信地址有效标志
	BYTE bDeftSmsc[32];
	BYTE bDeftSmscLen;
	BYTE bDeftMstSmsAddr[32];
	BYTE bDeftMstSmsAddrLen;
	BYTE bMstAddr[9];
	char szMstPhone[32];
	
	//激活参数
	char szActiveSms[ACTIVE_SMS_LEN];
	char szActiveNumber[PHONE_NUM_LEN];
	WORD wDormantInterv;
	bool fSetXgauth;//友方模块是否需要设置用户名和密码
}TModemPara;  

//LAC、小区号CELL_ID、频点号ARFCN，接收功率Rx level，发射功率Rx level
typedef struct {
	DWORD dwCellID;	// 16进制
	WORD wLac;		// 16进制
	WORD wArfCN;
	WORD wRxLev;	// 单位:dBm
	WORD wTxPwr;	// 单位:dBm
} TNetInfo;

typedef struct {
	BYTE bManuftr[4];	//厂商代号	ASCII	4
	BYTE bmodel[8];		//模块型号	ASCII	8
	BYTE bSoftVer[4];	//软件版本号	ASCII	4
	BYTE bSoftDate[6];	//软件发布日期：日月年	见附录A.20	3
	BYTE bHardVer[4];	//硬件版本号	ASCII	4
	BYTE bHardDate[6];	//硬件发布日期：日月年	见附录A.20	3
	BYTE bCCID[20];					//ＳＩＭ卡ICCID	ASCII	20
	BYTE bCNUM[8];		//本机号码
	BYTE bSysmode;
	BYTE bMnc;
}TModemInfo;	//模块信息

class CModem
{
public:
    CModem();
    virtual ~CModem();
  	
  	TModemPara*	m_pModemPara;
  	
  	char* GetATDT() { return m_pszATDT; };  	
  	void SetComm(CComm* pComm) { m_pComm = pComm; };
  	bool Init(TModemPara* pModemPara);
	bool AvoidSmsOverflow(void);
	int ATCommand(char* pszCmd, char* pszAnsOK, char* pszAnsErr1=NULL, char* pszAnsErr2=NULL, WORD nWaitSeconds=0);
	int ATCommand(char* pszCmd, const char** ppszAns, WORD wNum, WORD nWaitSeconds);
  	bool HaveRing() //是否收到过振铃信号
  	{ 
  		if (m_fHaveRing) { m_fHaveRing = false; return true; }
  		else return false;
  	}
	bool CheckActive();
	WORD GetModemStep() { return m_wModemStep; };
	WORD GetErrBytes() {return m_wPreErrByte;};
	bool EnMux();
	//bool GetSYSINFO();
	//bool RcvSYSINFO();
	bool UpdateMyTime();
	//虚函数
	virtual bool Connect(bool fUdp, DWORD dwRemoteIP, WORD wRemotePort) { return false; };
	virtual int InitAPN(char *cAPN){ return MODEM_NO_ERROR; };
	virtual bool TestModem() { return false; };
	virtual int ResetModem(){ return MODEM_NO_ERROR; };
	virtual bool ResetEmbed() { return false; };
	virtual bool ResetSMS(){ return false; };
	virtual bool ConnectEmbed(){ return false; };
	virtual bool GprsPoweroff(void){ return false; };
	virtual int RxSmsFrm(BYTE* pbSms, WORD wSmsLen, BYTE* pbSmsc, BYTE* pbSmscLen, BYTE* pbSender, BYTE* pbSenderLen, BYTE* pbFrm);
    virtual bool SendSms(BYTE* pbTxBuf, WORD wLen);
    virtual bool SendTextSms(BYTE* pbSmsAddr, BYTE bSmsAddrLen, BYTE* pbTxBuf, WORD wLen);
	virtual bool SendEmbed(BYTE* pbTxBuf, WORD wLen) { return false; };
	virtual WORD ReceiveEmbed(BYTE* pbRxBuf, WORD wBufSize) { return 0; };
    virtual WORD ReceiveSms(BYTE* pbRxBuf, WORD wBufSize, bool fQuerySms);
	virtual BYTE* RxSmsHead(BYTE* bBuf, WORD wLen){ return 0; };
#ifdef SYS_LINUX
	virtual void PowerOff() { ModemPowerOff(); };
	virtual void PowerOn() { ModemPowerOn(); };
#else
	virtual void PowerOff() { ; };
	virtual void PowerOn() { ; };
#endif
	BYTE GetModuleType() { return m_wModuleType; };
	BYTE GetModulemRegion(){ return m_wModuleregion; };
	virtual int UpdateSignStrength();
	virtual bool IsSignValid(WORD wSignStrength);// { return m_fSignValid; };
	virtual WORD Send(BYTE* pbTxBuf, WORD wLen) { return wLen; };
	virtual int Receive(BYTE* pbRxBuf, WORD wBufSize) { return 0; }
	virtual bool Close() { return true; };
	virtual bool DisConnect() { return true; };
	virtual bool PPPOpen(char* pszNum, char* pszName, char* pszPass) { return true; };
	virtual bool PPPClose() { return true; };
	//virtual bool GetCID(TDataItem diCID) { return true; };
	virtual int GetNetInfo(TNetInfo *pNetInfo) { return 0; };
    virtual bool GetIMSI(TDataItem diIMSI);
    virtual bool GetGMR(TDataItem diCGMR);	
    virtual bool GetGPS(TDataItem diGPS) { return true; };
	virtual bool GetSYSINFO(){ return true; };
    DWORD GetRxSmsClick();
    int GetRxSmsAddr(unsigned char* pbBuf);
    bool GetSmscNumber();
	//virtual bool SetMYNETINFO(BYTE bNetType);
protected:
	CComm*  m_pComm;
	
	WORD  m_wModuleType;
	WORD  m_wModuleregion;
	BYTE  m_bGSN[GSN_LEN];
	//WORD  m_wSignStrength;
	char* m_pszCSQ;
	char* m_pszRxSms;
	char* m_pszRxSmsHead;
	char* m_pszATDT;
	bool m_fChnUnicom;//中国联通
	bool m_fChnCdma;//中国电信 用来初始化cdma的短信
	
	DWORD m_dwRxSmsClick;  	 //接收到短信的时间,为0时表示没在本次发送前没收到过短信
	BYTE  m_bRxSmsc[32];	 //接收到短信的短信中心号码	
	BYTE  m_bRxSmscLen;		 //接收到短信的短信中心号码的长度	
	BYTE  m_bRxMasterSmsAddr[32]; //接收到短信的电话号码
	BYTE  m_bRxMasterSmsAddrLen;  //接收到短信的电话号码的长度
	WORD  m_wPreErrByte; //GPRS误码率

	DWORD m_dwSmsTxClick;
	bool m_fQuerySms;		//是否以查询的方式接收短信
	bool m_fHaveRing;		//收到振铃
	
	bool m_fSmsDelAllSupported;  //支持短信全部删除指令
	
	char m_szRingNumber[PHONE_NUM_LEN+1];
	//bool m_fSignValid;
	WORD m_wModemStep;
	TNetInfo m_tNetInfo[16];
	
	int WaitModemAnswer(char* pszAnsOK, char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds);
	int WaitModemAnswer(const char** ppszAns, WORD wNum, WORD nWaitSeconds);
	bool ATCmdTest(WORD wTimes);
	WORD MakeSMS(BYTE* pbSmsc, BYTE bSmscLen, BYTE* pbDest, BYTE bDestLen, BYTE* pbFrm, BYTE bFrmLen, BYTE* pbSms, BYTE* pbCmgsLen, BYTE bDCS=0x04);
	int RxSmsPdu(WORD wNO, BYTE* pbBuf);
	void DeleteSms(WORD wMaxNO);
	WORD HandleSms(BYTE* pbInfo, WORD wInfoLen, BYTE* pbRxBuf);
	WORD HandleSms(WORD wNo, BYTE* pbRxBuf);
	void GetRingNumber(BYTE* pbInfo, WORD wLen);
};


#endif  //MODEM_H



