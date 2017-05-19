#ifndef GPRSIF_H
#define GPRSIF_H
#include "ProtoIf.h"
#include "SocketIf.h"
#include "Modem.h"
#include "GprsWorker.h"
#include "EmbedGprsIf.h"

//通道的通信模式
#define CN_MODE_SOCKET      0	//基于TCP/IP的通信模式
#define CN_MODE_SMS      	1	//短信
#define CN_MODE_EMBED     	2	//模块嵌入式协议栈
#define CN_MODE_COMM     	3	//串口通信模式
#define CN_MODE_CMD     	4	//命令模式
#define CN_MODE_ETHSCK      5	//以太网SOCKET通信模式

#define CN_MODE_NUM      	6

//在线模式
#define ONLINE_M_PERSIST    1	//永久在线模式
#define ONLINE_M_ACTIVE     2	//激活模式/非连续在线模式
#define ONLINE_M_PERIOD		3	//时段在线模式
#define ONLINE_M_SMS		4   //短信方式
#define ONLINE_M_JIT		5	//JUST IN TIME 按需要即时上线,如单独的上报端口
#define ONLINE_M_DMINSMS	6   //休眠时进入短信模式

#define ONLINE_MODE			0
#define NONONLINE_MODE		1


class CGprsIf : public CSocketIf
{
public:
    CGprsIf();
    virtual ~CGprsIf();

	//CModem* m_pModem;
	TGprsPara* m_pGprsPara;
    
	bool Init(TGprsPara* pGprsPara);
	bool ReInit(TGprsPara* pGprsPara);
	bool ResetCnMode(WORD wCnMode); //在运行过程中可以重新设置通道模式，方便在GPRS socket/模块协议栈和以太网间切换
	void LoadUnrstPara();
	WORD GetParaCnMode() { return m_pGprsPara->wCnMode; }	//取得通道模式的设置参数值
	
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TGprsPara* pGprsPara))
	{		//注册装载非复位参数的函数
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	//虚函数
    virtual WORD GetMaxFrmBytes();
    virtual int ResetIf();
    virtual bool RequestOffline();
	virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
	virtual bool Connect();
	virtual	bool DisConnect();
	virtual	void EnterDorman();
    virtual void OnDisConnectByPeer();
	virtual	bool Close();
	virtual	void OnResetFail(); 
	virtual	void OnConnectFail();
	virtual void KeepAlive();
	virtual bool IsNeedLogin();
	virtual DWORD GetBeatSeconds();
	virtual void OnLoginOK();
	virtual void OnLoginFail();
	virtual void DoIfRelated();
	virtual void OnRcvFrm();
	WORD SignStrength() { return m_wSignStrength;};
    CEmbedGprsIf* GetEmbedGprsIf();
	
protected:
	CEmbedGprsIf m_embdGprsIf;
	bool (*m_pfnLoadUnrstPara)(TGprsPara* pGprsPara);
	
	//CComm  m_Comm;
	//int    m_iPd;	//ppp的设备号,只在lwip中用到
	int m_iGprsUser;
	
	WORD  m_wBaseCnType;  //基本通道类型,从短信方式切换回去时返回到的通道类型
	BYTE  m_bRstMode;	  //GPRS模块的复位模式,复位后处于空闲状态还是短信状态

	//统计数据
	WORD m_wGprsTxCnt;
	WORD m_wGprsRxCnt;
	WORD m_wSmsTxCnt;
	WORD m_wSmsRxCnt;

	DWORD m_dwErrRstClick;
	DWORD m_dwSignClick;
	DWORD m_dwSmsOverflowClick;
	
	DWORD m_dwPeriodDropInterv; //时段在线模式的激活方式自动掉线时间,单位分钟
	DWORD m_dwFluxOverClick;	//流量超标的起始时标

	WORD  m_wSignStrength;

	BYTE GetGprsRstMode();
	bool CheckActivation();
	//void DoIfRelated();
	char* CnModeToStr(WORD wMode);
	char* OnlineModeToStr(WORD wMode);
};

#endif  //GPRSIF_H


