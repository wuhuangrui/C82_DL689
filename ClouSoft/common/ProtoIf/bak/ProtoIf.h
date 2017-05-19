/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：PotoIf.h
 * 摘    要：本文件实现了通信接口基类定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef PROTOIF_H
#define PROTOIF_H

#include "apptypedef.h"
#include "sysarch.h"
#include "Proto.h"

#define SOCK_MAX_BYTES      1024  
#define EMBED_MAX_BYTES     800       
#define COMM_MAX_BYTES      512	
#define SMS_MAX_BYTES       140
#define ETHER_MAX_BYTES     1024
#define PPP_MAX_BYTES       1024

class CProto;

typedef struct{
	char* pszName;			//接口名称
	bool fNeedLogin;		//是否需要登录
	WORD wMaxFrmBytes; 		//接口的一帧最大发送字节数,不同协议可能规定不一样
	DWORD dwRstInterv;		//接口的复位间隔,单位秒
	DWORD dwConnectInterv;	//接口的连接间隔,单位秒
	WORD  wConnectNum;		//连接失败连续尝试的次数
	WORD  wLoginRstNum; 	//登录失败断开连接的次数
	WORD  wLoginNum; 		//登录失败连续尝试的次数
	DWORD dwLoginInterv; 	//登录间隔,单位秒
	WORD wRstNum;           //复位次数
	
	WORD wReSendNum;		//重发次数

	DWORD dwNoRxRstAppInterv; //无接收复位终端间隔,单位秒,0表示不复位
	DWORD dwNoRxRstIfInterv;  //无接收复位接口间隔,单位秒,0表示不复位

	//重连方案的参数
	WORD  wReTryNum;  		//复位或者连接重试的次数
	DWORD dwDormanInterv;	//休眠时间间隔, 单位秒, ,0表示禁止休眠模式

	//消息宏定义,避免编译过程中库用到的宏定义跟应用程序不一致
	WORD wInfoActive;		//INFO_ACTIVE
	WORD wInfoAppRst;		//INFO_APP_RST
}TIfPara;

//接口类型
#define IF_UNKNOWN		0
#define IF_GPRS         1
#define IF_COMM     	2
#define IF_SOCKET       3	//基于操作系统socket套接字
#define IF_R230M		4	//230M电台
#define IF_P2P			5	//专线Modem
#define IF_SMS			6	//短信模式

#define IF_RST_OK  		0  	//复位成功
#define IF_RST_HARDFAIL 1	//硬复位失败
#define IF_RST_SOFTFAIL 2	//软复位失败(协议层)

//接口状态机,接口的状态切换: (休眠)->(复位)->(连接)->(登录)->(传输)
#define IF_STATE_DORMAN  	0 //休眠
#define IF_STATE_RST  		1 //复位
#define IF_STATE_CONNECT 	2 //连接
#define IF_STATE_LOGIN  	3 //登录
#define IF_STATE_TRANS  	4 //传输	

#define IF_DEBUG_INTERV		(2*60)	//调试输出的间隔,单位秒
#define DATA_SRC_SMS		1
class CProtoIf
{
public:
    CProtoIf();
    virtual ~CProtoIf();
    
    //变量定义
    WORD m_wIfType;
    CProto* m_pProto;
    TIfPara* m_pIfPara;
    bool m_fExit;
    bool m_fExitDone;
	bool m_fUnrstParaChg;  	//非复位参数发生改变
	bool m_fNeedActive;		//需要激活
	WORD m_wRunCnt;         //运行次数
	
	WORD  m_wCnMode;
	
	//函数定义
	void Init();
	char* GetName();
	WORD GetIfType() { return m_wIfType; };
	CProto* GetProto() { return m_pProto; };	
    bool CanTrans();// { return m_wState>IF_STATE_CONNECT && m_wState<=IF_STATE_TRANS; }; //接口是否还处于传输状态
    WORD GetState() { return m_wState; };
    bool IsInDorman() { return m_dwDormanClick!=0; };
    void DoDorman();
	int	 GetLastErr() { return m_iLastErr; };
	DWORD GetWakeUpTime(void);
	void AttachProto(CProto* pProto) { m_pProto = pProto; };
			//绑定协议
	bool Init(TIfPara* pIfPara);
			//接口初始化,比如和协议建立连接、申请资源、分配内存等	
	void SetActive() { m_fNeedActive = true; };
			//设置接口处于激活状态,用于非连续在线的短信激活
	void InitRun();
	
	void SetDisConnect(DWORD dwDormanInterv=0); //在收到外部的断开连接命令时,调用本函数通知接口
	void SetIdle();		  //在收到外部的处于空闲状态的命令时,调用本函数通知接口

	WORD GetCnMode() { return m_wCnMode; };

	DWORD GetRxClick() { return m_dwRxClick; };           //最近一次接收到报文的时间

    //接口定义
    //virtual bool Init(void* pvArg) = 0;
    virtual void AutoSend();
    virtual WORD GetMaxFrmBytes() { return m_pIfPara->wMaxFrmBytes; };
    virtual bool Send(BYTE* pbTxBuf, WORD wLen) = 0;
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize) = 0;
    virtual void KeepAlive() { };
    		//接口保活探测,比如心跳检测
	virtual void LoadUnrstPara() { };	//装载非复位参数
	virtual bool IsNeedLogin() { return m_pIfPara->fNeedLogin; };
			//本接口的通信协议是否需要登录
    virtual bool Connect() { return true; };
    		//建立连接,比如打开串口或者建立socket连接等
    virtual bool DisConnect();
    		//断开连接,不管是主动断开还是被动断开均可调用,比如关闭串口或者断开socket连接等
    virtual int ResetIf(){ return IF_RST_OK; };
    		//复位接口,比如复位PPP连接和MODEM等
    virtual bool RequestOffline() { return true; };
    virtual void EnterDorman();
    		//使接口进入休眠方式
    virtual bool Close() { return true; };
			//关闭接口,比如释放资源等
	virtual void DoIfRelated(); 
			//做一些各个模块相关的非标准的事情,比如非连续在线方式下,GPRS和SMS间的切换
	virtual WORD GetConnectNum() { return m_pIfPara->wConnectNum; };
			//取连接次数,GPRS或socket连接的时候,如果有备用IP端口的话,连接次数乘2
			
    //回调函数
	virtual void OnConnectOK();
			//在接口由断开转为连接的时候调用
	virtual void OnConnectFail();
			//在接口连接失败时调用,比如多少次失败后复位接口
	virtual	void OnResetFail(); 
			//在接口复位失败时调用,比如多少次失败后进入休眠
	virtual	void OnResetOK();
			//在接口复位成功时调用
	virtual void OnLoginFail();
			//在协议登陆失败时调用,比如多少次失败后断开连接			
	virtual void OnLoginOK();
			//在协议登陆成功时调用
	virtual void OnRcvFrm();
			//在通信协议收到正确帧时调用,主要更新链路状态,比如心跳等
	
	virtual bool SetUnrstPara(void *pvPara) { return false;};	//装载非复位参数
	virtual bool GetUnrstPara(void *pvPara) { return false;};	//获得非复位参数
	virtual bool IsIfValid() { return false; };
protected:
	
	//变量定义
	WORD  m_wState;				 //接口状态机
	WORD  m_wResetFailCnt;		 //复位失败次数
	WORD  m_wConnectFailCnt;     //连接失败次数
	WORD  m_wLoginFailCnt;       //登陆失败次数
	DWORD m_dwRxClick;           //最近一次接收到报文的时间
	DWORD m_dwBeatClick;         //最近一次发送心跳报文的时间
	
	DWORD m_dwDormanClick;		 //进入休眠的开始时间
	bool  m_fRstInConnectFail;	 //本接口在连接失败到重试次数后复位接口	
	DWORD m_dwDebugClick;
	int	  m_iLastErr;
	DWORD m_dwRstIfClick;		 //记录接口上次复位时刻或者接收到报文的时刻
	WORD  m_wDormanState; 		 //暂时休眠的状态，休眠完还要转回到该状态

	bool m_fDisConnCmd;			//收到外部的断开连接命令
	bool m_fSetIdleCmd;			//收到外部的处于空闲状态的命令
	DWORD m_dwDormanInterv;		//动态设定的休眠间隔，单位秒
	BYTE  m_bGprsDataSrc;	//GPRS通信时，1:数据来源为短信，其他：来源socket
};

#endif //PROTOIF_H
 
