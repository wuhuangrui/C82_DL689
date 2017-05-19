#ifndef SOCKETIF_H
#define SOCKETIF_H
#include "syssock.h" 
#include "ProtoIf.h"

//服务器断开连接模式：
#define SVR_DISCON_EXIT		0	//断开连接后退出线程
#define SVR_DISCON_IDLE		1	//断开连接后处于空闲模式

#define RAND_ON_DORMAN			0x01	//休眠时进行随机延时
#define RAND_ON_FAIL			0x02	//失败时进行随机延时

typedef struct{
	TIfPara IfPara;
	bool 	fSvr;			//是否是服务器模式
	bool 	fUdp;			//是否是UDP通信方式
	BYTE  	bRandLoginFlg;	//随机登陆控制标志位:BIT0是否休眠随机延时，BIT1是否失败随机延时，其它位保留
							//主要针对国网：每次登录失败后，经过心跳周期0.5-1.5倍的随机延时（以秒或毫秒计）后重新登录
	BYTE	bSvrDisconMode;	//服务器断开连接模式：
	DWORD 	dwRemoteIP;		//主站IP
	WORD 	wRemotePort;	//主站端口
	DWORD 	dwBakIP1;		//备用主站IP
	WORD 	wBakPort1;		//备用主站端口
	DWORD 	dwBakIP2;		//备用主站IP
	WORD 	wBakPort2;		//备用主站端口
	
	DWORD 	dwLocalIP;
	WORD 	wLocalPort;
	bool	fEnableFluxStat;	//是否允许流量控制,只有本socket用的是GPRS通道时才支持
	bool	fEnSocketLed;		//是否控制Socket Led,只针对国网标准模块
	
	WORD 	wDisConnectByPeerNum;	//被对方断开连接，切换到休眠状态的次数
	
	bool 	fEnTcpKeepAlive; 		//是否开启TCP的keepalive属性
	WORD 	wKeepIdle; 				//如该连接在wKeepIdle秒内没有任何数据往来,则进行探测 
	WORD 	wKeepInterv; 			//探测时发包的时间间隔为wKeepInterv秒，如20秒
	WORD 	wKeepCnt; 				//探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
	
	//非复位参数
	WORD  wBeatTestTimes;	//心跳测试次数,为0,表示不自动掉线,只周期发心跳		
	DWORD dwBeatSeconds;	//心跳间隔
	DWORD dwBeatTimeouts;	//心跳超时时间,单位秒
}TSocketPara;

class CSocketIf : public CProtoIf
{
public:
    CSocketIf();
    virtual ~CSocketIf();

	TSocketPara* m_pSocketPara;
	
    bool Init(TSocketPara* pSocketPara);
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TSocketPara* pPara))
	{		//注册装载非复位参数的函数
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	void InitSvr(int socket);
	void SetMaxIpUseCnt() { m_wIPUseCnt = m_pIfPara->wConnectNum;  };

	//虚函数
	virtual void KeepAlive();
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
    virtual void LoadUnrstPara();
    virtual bool Connect();
    virtual bool Close();
	virtual bool DisConnect();
	virtual DWORD GetBeatSeconds() { return m_pSocketPara->dwBeatSeconds; };
	virtual WORD GetConnectNum();	//取连接次数,GPRS或socket连接的时候,如果有备用IP端口的话,连接次数乘2
	virtual void EnterDorman();
    virtual void OnDisConnectByPeer();
	virtual void OnConnectFail();
	virtual void OnLoginOK();
	virtual void OnLoginFail();
	virtual bool IsIfValid() { return m_Socket != INVALID_SOCKET; };
    virtual void DoIfRelated(); 
    
protected:
	bool (*m_pfnLoadUnrstPara)(TSocketPara* pPara);
	
	int  m_Socket;
	DWORD m_dwRemoteIP;
	WORD  m_wRemotePort;
	BYTE m_bMasterAddr[9];
	BYTE m_bBakIP;		// 当前使用的IP	0--主IP,  1--备1,  2--备2
	WORD m_wIPUseCnt;	//当前IP使用次数
	WORD m_wDisConnectByPeerCnt;

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
	DWORD GetRandDormanInterv();
	void StateToDorman(WORD wState);
	bool SetSocketLed(bool fLight);
};


#endif  //SOCKETIF_H




