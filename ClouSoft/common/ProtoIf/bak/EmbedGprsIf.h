#ifndef EMBEDGPRSIF_H
#define EMBEDGPRSIF_H
#include "syssock.h" 
#include "ProtoIf.h"
#include "SocketIf.h"
#include "Modem.h"


typedef struct{
	//CModem* pModem;  //MODEM在传进来的时候已经进行了必要的初始化，只是差串口没初始化好
	TSocketPara SocketPara;
	//TModemPara	ModemPara;
	//TCommPara CommPara; //串口配置
	
	WORD  wCnMode;	//通道模式,短信方式下设置为CN_MODE_SMS,
					//GPRS方式下设置为CN_MODE_SOCKET或CN_MODE_EMBED,
					//非连续在线方式下特指GPRS的通道模式,设置为CN_MODE_SOCKET或CN_MODE_EMBED,
	
	WORD  wSmsMaxFrmBytes;	//短信方式下的最大帧长度,其它方式的帧长度还是取IfPara.wMaxFrmBytes
	WORD  wRstNum;
	bool  fEnableRingActive; 	 //允许振铃激活
	bool  fEnableAutoSendActive; //允许主动上报激活
	DWORD dwActiveDropInterv; 	 //非连续在线模式的自动掉线时间,单位分钟
	DWORD dwPowerupDropInterv; 	 //上电激活的自动掉线时间,单位分钟
								 //设为0自动取消上电激活 	
	DWORD dwPowerupBeatMinutes;	 //上电激活的心跳间隔

	BYTE bOnlineMode;	//在线模式
	//BYTE bOnlinePeriod[24];
		//当模块工作在时段在线方式
		//每四位二进制数表示该30分钟时段内的心跳间隔值（单位：5分钟），
		//从低字节到高字节依次表示48个时段的心跳时段设置
	bool fEnableFluxCtrl;	//是否允许流量控制
	bool fRstOnSms;			//是否复位到短信模式，主要针对激活模式和时段在线模式
	//char szPppUser[64];
	//char szPppPsw[64];
}TGprsPara;


class CEmbedGprsIf : public CProtoIf
{
public:
    CEmbedGprsIf();
    virtual ~CEmbedGprsIf();

	CModem* m_pModem;
	TGprsPara* m_pTEmbedGprsPara;
	
    bool Init(TGprsPara* pTGprsPara);
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TGprsPara* pPara))
	{		//注册装载非复位参数的函数
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	void InitSvr(int socket);
	void ResetIPUseCnt() {m_wIPUseCnt = 0;};
	void SetMaxIpUseCnt() { m_wIPUseCnt = m_pIfPara->wConnectNum; };
	
	//虚函数
	virtual void KeepAlive();
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
    virtual void LoadUnrstPara();
    virtual bool Connect();
    virtual bool Close();
	virtual bool DisConnect();
	virtual DWORD GetBeatSeconds() { return m_pTEmbedGprsPara->SocketPara.dwBeatSeconds; };
	virtual WORD GetConnectNum();	//取连接次数,GPRS或socket连接的时候,如果有备用IP端口的话,连接次数乘2
	virtual void EnterDorman();
	virtual bool IsIfValid();
	virtual void OnRcvFrm();
	virtual void OnConnectFail();
	virtual void OnLoginOK();
	//virtual void OnConnectOK();
		
protected:
	bool (*m_pfnLoadUnrstPara)(TGprsPara* pPara);
	
	bool  m_fIfValid;
	DWORD m_dwRemoteIP;
	WORD  m_wRemotePort;
	BYTE m_bMasterAddr[9];
	bool m_fBakIP;
	WORD m_wIPUseCnt;	//当前IP使用次数
	
	//统计数据
	WORD m_wGprsTxCnt;
	WORD m_wGprsRxCnt;
	WORD m_wSmsTxCnt;
	WORD m_wSmsRxCnt;

	DWORD m_dwErrRstClick;

	bool m_fServerMode;
	BYTE m_bMasterIP[4];
	WORD m_wMasterPort;
	//BYTE m_bLocalIP[4];
	WORD m_wLocalPort;
	char m_szAPN[32];
	char m_szPppUser[64];
	char m_szPppPsw[64];
		
	char* m_pszCSQ;
	char* m_pszRxSms;
	char* m_pszRxSmsHead;
	char* m_pszATDT;
	
	void PutToLoopBuf(BYTE* pbBuf, WORD wLen);
	void RxToLoopBuf();
	WORD RxFromLoopBuf(BYTE* pbRxBuf, WORD wBufSize);
	bool InitSock();
	bool LoadPara();
};


#endif  //EMBEDGPRSIF_H




