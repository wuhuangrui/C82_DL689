/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FaProto.h
 * 摘    要：本文件主要实现终端使用698-45对外通讯
 * 当前版本：1.0
 * 作    者：孔成波
 * 完成日期：2016年9月
 * 实现方式说明：1：基于分层的实现方式，终端有两层，一层是链路层，一层是应用层
 *               2：两层各自独立，链路层判断接收到的帧是否完整APDU，如果是，就在链路层接收完一帧APDU，之后提交给应用层处理
 *				 3：应用层分层基于应用数据包分包，如果一包数据较大，如果能独立解析，那就分解成独立解析的多帧APDU，否则强制拆开处理
 *				 
 *********************************************************************************************************/
#ifndef FAPROTO_H
#define FAPROTO_H

#include "Comm.h"
#include "Queue.h"
#include "DbAPI.h"
#include "syscfg.h"
#include "sftp.h"
#include "FaStruct.h"
#include "ProStruct.h"
#include "Proto.h"
#include "OIObjInfo.h"
#include "FrmQue.h"
#include "TermEvtTask.h"

#define APDU_MAX_SIZE		2040	//APDU最大空间

#define PRE_ADJ_MAX_CNT		255	//精确对时最大个数

#define FIRST_FRM	0x00	//首帧
#define LAST_FRM	0x40	//最后帧
#define MID_FRM		0xC0	//中间帧

//////////////////////////////////////////////////////////////
//属性定义
#define ATTR_0 0
#define ATTR_1 1
#define ATTR_2 2
#define ATTR_3 3
#define ATTR_4 4
#define ATTR_5 5
#define ATTR_6 6

//地址类型定义
#define ADDR_TYPE_SINGLE		0	//单地址 
#define ADDR_TYPE_UNIVERSAL		1	//通配地址
#define ADDR_TYPE_GROUP			2	//组地址
#define ADDR_TYPE_BROADCAST		3	//广播地址

//
#define MAXATTRI				30	//支持最大属性数目
#define MAXMETHOD				30	//支持最大方法数目
#define MAXRECORD				10	//支持最大记录数目

//终端自定义协议部分的定义
#define FAP_CMD       0   
#define FAP_LEN       1	  
#define FAP_DATA      3   
#define FAP_DATA_EX   7   

#define FAP_CMD_GET     0x3f
#define FAP_CMD_UP      0x80
#define FAP_CMD_DIR 	0x80
#define FAP_CMD_DOWN    0x00
#define FAP_FIX_LEN     3    

#define FAP_CMD_READ_DATA   0x01
#define FAP_CMD_READ_TASK   0x02
#define FAP_CMD_WRITE_DATA  0x08
#define FAP_CMD_USERDEF     0x0f
//浙江协议定义结束

//////////////////////////////////////////////////////////////
///////////国标下的定义//////////////////
//连接方式
#define CONNECTTYPE_LOCAL               0      //本地连接方式
#define CONNECTTYPE_GPRS                1      //GPRS连接方式
#define CONNECTTYPE_UPLINK             	2      //UPLINK连接方式
#define CONNECTTYPE_ETH                3      //以太网连接方式
#define CONNECTTYPE_MEGA16            	4      //和单片机通讯

//极限个数
#define GBPRO_MAXSUMGROUP		(GB_MAXSUMGROUP-GB_MAXOFF)	//总加组个数
#define GBPRO_MAXPOINT			(GB_MAXMETER-GB_MAXOFF)		//测量点个数

//缓存空间大小定义
#define GB_FRM_SIZE   					2048				//本协议支持一帧的缓存大小
//通用平台统一使用的帧长度定义
#define FAP_FRM_SIZE        			GB_FRM_SIZE

#define SMS_ADDR_INTER    				0x91
//国标下的定义结束
//////////////////////////////////////////////////////////////

typedef struct{
	TProPara ProPara;
	WORD wConnectType;
	
	BYTE bRTUA[4];	//0-1:终端地市区县码;2-3:终端地址
	BYTE bProtoVer;		//协议版本，用于控制协议的版本升级
}TFaProPara;

typedef struct{
	BYTE  	bTxBuf[FAP_FRM_SIZE];	//发送帧缓冲区
	WORD  	wTxLen;					//发送帧长度
}TFapMsg;//

// typedef struct{	
// 	TProPara ProPara;
// 	WORD	wConfirmDelayTime;	//确认超时(s)
// 	bool 	fNeedConfirm1;		//需要确认的服务1
// 	bool 	fNeedConfirm2;		//需要确认的服务2
// 
// 	BYTE	bAuthType;			//消息认证方案号
// 	WORD	wAuthPass;			//消息认证参数
// 	
// 	WORD	wGrpAddr[8];		//组地址
// 	WORD	wAddr1;				//行政区划码A1
// 	WORD	wAddr2;				//终端地址A2
// 	
// 	//各协议不同的配置
// 	WORD	wConnectType;		//连接类型			
// 	BYTE	bMasterNo;			//主站身份,每一个主站各有一个主动上报的读指针,当主站多于2个时使用此参！	
// }TGbProPara;//公共配置参数


#define NOT_RPT		0
#define NOW_RPT		1
#define BEAT_RPT	2
#define FOLLOW_RPT	3

//所需要的空间缓存的大小
//#define MAXFRMSIZE				2120	//收发1个帧的缓存长度
#define MAXFRMSIZE				5000	//收发1个帧的缓存长度
#define MAXDATASIZE				3000	//收发数据的1个极限缓存长度

#define MAXLPDUNUM				3		//服务器接收帧最大窗口尺寸，即1个APDU分配到链路层所需的LPDU最大个数
#define APDUSIZE				6102	//服务器最大可处理的APDU尺寸，(ESAM的终端证书为2052个字节)
#define LPDUSIZE				(APDUSIZE/MAXLPDUNUM)	//链路层传输帧的最大字节数，由APDUSIZE、MAXLPDUNUM决定
#define MAXAPDUNUM				8//((MAXDATASIZE/APDUSIZE)+1)		//1个APDATA分配到应用层所需的APDU最大个数\\

//缓存的实际使用可能有变化，用一套参数记录（目前至少要考虑黑龙江主站和程序下载2种情况）2008.3.25
//说明：程序下载如果按照1K分段，则数据内容将会达1143B(其中文件名87B)，所以放宽wApduCutSize到1160B
typedef struct{//																黑龙江主站		程序下载1K分段		其它
	BYTE bType;//支持的类型															0				1				2
	WORD wMaxFrmSize;//<=MAXFRMSIZE													300				1300
	WORD wMaxDataSize;//==MAXDATASIZE												3000			3000
	WORD wLpduSize;//链路协商参数-2													253				1220-2
	WORD wMaxLpduNum;//min(MAXLPDUNUM,((wApduSize/wLpduSize)+1))					3				1
	WORD wApduSize;//应用协商参数													512				1200
	WORD wMaxApduNum;//min(MAXAPDUNUM,((wMaxDataSize/wApduSize)+1))					6				3
	WORD wApduCutSize;//应用协商参数-30												490				1160
}TNegoSizeDef;//协商参数的定义

//////////////////////////////////帧类型定义//////////////////////////////////////////////
#define DL_FRM_NULL				0x00
#define DL_FRM_I				0x01
#define DL_FRM_RR				0x02
#define DL_FRM_RNR				0x03
#define DL_FRM_SNRM				0x04
#define DL_FRM_DISC				0x05
#define DL_FRM_UA				0x06
#define DL_FRM_DM				0x07
#define DL_FRM_FRMR				0x08
#define DL_FRM_UI				0x09

///////////////////////////////////结果定义///////////////////////////////////////////////
#define RESULT_OK				0x00
#define RESULT_NOK				0x01
#define RESULT_NO_RES			0x02
#define RESULT_REMOTE			0x03
#define RESULT_LOCAL_DL			0x04
#define RESULT_LOCAL_PHY		0x05

///////////////////////////////////应用层状态定义/////////////////////////////////////
#define APPST_NOK				0	//等待验证协商
#define APPST_OK				1	//通过验证协商

///////////////////////////////////连接状态定义/////////////////////////////////////
#define LNKST_NDM				0	//断开状态
#define LNKST_NRM				1	//连接
#define LNKST_BUSY				2	//忙

///////////////////////////////////GPRS状态///////////////////////////////////////////////
#define		MODEM_OPENED		(1<<0)		//模块打开
#define		GPRS_ONLINE			(1<<1)		//模块在线
#define		NO_SIGNAL			(1<<2)		//无信号
#define		NO_NETWORK			(1<<3)		//无网络
#define		CONNECT_FAILED		(1<<4)		//连不到服务器

///////////////////////////////////位置定义//////////////////////////////////////////////
#define HDLCLOC_START					0	//起始字节的位置
#define HDLCLOC_FRMTYPE1				1	//长度域的第一个字节的位置
#define HDLCLOC_FRMTYPE2				2	//长度域的第二个字节的位置
#define HDLCLOC_ADDR					3	//地址域的第一个字节的位置

///////////////////////////////////源自从站的请求响应及模式的编码////////////////////////////////////
#define LINK_REQ		1	//预连接请求
#define CONNECT_REQ		2	//建立应用连接请求
#define RELEASE_REQ		3	//断开应用连接请求
#define GET_REQ			5	//读取请求
#define SET_REQ			6	//设置请求
#define ACTION_REQ		7	//操作请求
#define REPORT_RES		8	//上报应答
#define PROXY_REQ		9	//代理请求
#define SECURITY_REQ	16	//安全请求

#define PROXY_GET_REQ_LIST			1	//代理读取若干个服务器的若干个对象属性请求
#define PROXY_GET_REQ_RECORD		2	//代理读取一个服务器的一个记录型对象属性请求
#define PROXY_SET_REQ_LIST			3	//代理设置若干个服务器的若干个对象属性请求
#define PROXY_SET_THEN_GET_REQ_LIST	4	//代理设置后读取若干个服务器的若干个对象属性请求
#define PROXY_ACT_REQ_LIST			5	//代理操作若干个服务器的若干个对象方法请求
#define	PROXY_ACT_THEN_GET_REQ_LIST	6	//代理操作后读取若干个服务器的若干个对象方法和属性请求
#define PROXY_TRANS_CMD_REQ			7	//代理透明转发命令请求

#define LINK_RESPONSE	129	//终端主动发送01的link后，主站回答的response
#define CONNECT_RES		130	//建立应用连接响应
#define RELEASE_RES		131	//断开应用连接响应
#define RELEASE_NOTI	132	//断开应用连接通知
#define GET_RES			133	//读取响应
#define	SET_RES			134	//设置响应
#define ACTION_RES		135	//操作响应
#define REPORT_NOTI		136	//上报通知
#define PROXY_RES		137	//代理响应
#define SECURITY_RES	144	//安全响应

//////////////////////////////////Get-request的请求类型定义/////////////////////////////////////////////////
#define GET_NORMAL			1
#define GET_NORMAL_LIST		2
#define GET_RECORD			3
#define GET_RECORD_LIST		4
#define GET_NEXT			5

//////////////////////////////////Set-request的请求类型定义/////////////////////////////////////////////////
#define SET_NORMAL				1	
#define SET_NORMAL_LIST			2
#define SET_GET_NORMAL_LIST		3

//////////////////////////////////Action-request的请求类型定义/////////////////////////////////////////////////
#define ACT_NORMAL				1	
#define ACT_NORMAL_LIST			2
#define ACT_GET_NORMAL_LIST		3

//////////////////////////////////Report-Notification的请求类型定义///////////////////////////////////////////
#define RP_NOTI_LIST				1	
#define RP_NOTI_REC_LIST			2

//////////////////////////////////PROXY-Request的请求类型定义/////////////////////////////////////////////////
#define PROXY_GET_LIST			1
#define PROXY_GET_RECORD		2
#define PROXY_SET_LIST			3
#define PROXY_SET_GET_LIST		4
#define PROXY_ACT_LIST			5
#define PROXY_ACT_GET_LIST		6
#define PROXY_TRANS_COMM		7

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

const BYTE AARE		=			0x61;
const BYTE GETRES	=			0xc4;
const BYTE SETRES	=			0xc5;
const BYTE ACTRES	=			0xc7;
const BYTE EVTNOTE	=			0xc2;

const BYTE GETRES_NORMAL	=				1;
const BYTE GETRES_BLK		=				2;
const BYTE GETRES_LIST		=				3;
                                             
const BYTE SETRES_NORMAL	=				1;
const BYTE SETRES_BLK		=				2;
const BYTE SETRES_LASTBLK	=				3;
const BYTE SETRES_LIST_LASTBLK	=			4;
const BYTE SETRES_LIST		=				5;
                                             
const BYTE ACTRES_NORMAL	=				1;
const BYTE ACTRES_BLK		=				2;
const BYTE ACTRES_LIST		=				3;
const BYTE ACTRES_NEXTBLK	=				4;

///////////////////////////////////数据访问结果的定义///////////////////////////////
enum TLinkRequestSvr
{
	SVR_LINK = 0,		//建立连接
	SVR_BEAT = 1,		//心跳
	SVR_RELEASE = 2		//断开连接
};

///////////////////////////////////数据访问结果的定义///////////////////////////////
#define DR_ERROK			0		//成功
#define	DR_HardFault		1		//硬件失效
#define	DR_TempFail			2		//暂时失效
#define	DR_RWDenied			3		//读写拒绝
#define	DR_ObjUndefined		4		//对象未定义
#define	DR_ObjIFInValid		5		//对象接口不符合
#define	DR_ObjInExist		6		//对象不存在
#define	DR_TypeUnMatch		7		//类型不匹配
#define	DR_OverFlow			8		//越界
#define	DR_DataBlockErr		9		//数据块不可用
#define	DR_SegFrmCancel		10		//分帧传输已取消
#define	DR_NoSegFrmStat		11		//不处于分帧传输状态
#define	DR_WRBlockCancel	12		//块写取消
#define	DR_WrBlockNoExist	13		//不存在块写状态
#define	DR_FrmNoErr			14		//数据块序号无效
#define	DR_PWErr			15		//密码错/未授权
#define	DR_BoteChgErr		16		//通讯速率不可更改
#define	DR_YearTimeZoneOver	17		//年时区数超
#define	DR_PeriodNumOver	18		//日时段数超
#define	DR_RateNumOver		19		//费率数超
#define	DR_SecCertUnMatch	20		//安全认证不匹配
#define	DR_RepRecharge		21		//重复充值
#define	DR_ESAMErr			22		//ESAM验证失败
#define	DR_SecCertFail		23		//安全认证失败
#define	DR_CPNUnMatch		24		//客户编号不匹配
#define	DR_RechargeNumErr	25		//充值次数错误
#define	DR_BuyElecOverFlow	26		//购电超囤积
#define	DR_AddrErr			27		//地址异常
#define	DR_SymDecodeErr		28		//对称解密错误
#define	DR_UnSymDecodeErr	29		//非对称解密错误
#define	DR_SignErr			30		//签名错误
#define	DR_MeterHalt		31		//电能表挂起
#define	DR_TimeTagErr		32		//时间标签无效
#define	DR_Other			255		//其他

///////////////////////////////////方法执行结果的定义////////////////////////////////////
enum TActionResult
{
	AR_Success = 0,                //成功               
	AR_HardFault = 1,              //硬件故障           
	AR_TempFail = 2,               //临时错误           
	AR_RWDenied = 3,               //读写操作拒绝       
	AR_ObjUndefined = 4,           //对象未定义         
	AR_ObjClsInconsistent = 9,     //类和对象不一致     
	AR_ObjUnavailable = 11,        //对象无效           
	AR_TypeUnmatched = 12,         //类型不匹配         
	AR_ScopeViolated = 13,         //范围超出           
	AR_DBlkUnavailable = 14,       //无效块             
	AR_LongActAborted = 15,        //太长的方法操作服务退出 
	AR_NoLongAct = 16,             //不应再操作方法了       
	AR_Other = 250                 //其它错误
};                                        
  
//AARE的协商结果
//association-result
#define RESULT_REJECTED_ACCEPTED	0
#define RESULT_REJECTED_PERMANENT	1
#define RESULT_REJECTED_TRANSIENT	2
//association-source-diagnostic
#define SOURCE_ACSE_SERVICE_USER			1
#define SOURCE_ACSE_SERVICE_PROVIDER		2
//ACSE_SERVICE_USER
#define USER_NULL							0
#define USER_NO_REASON_GIVEN				1
#define USER_CONTEXTNAME_NOT_SUPPORTED		2
#define USER_MECHANISMNAME_NOT_RECONGNISED	11
#define USER_MECHANISMNAME_REQUIRED			12
#define USER_AUTHENTICATION_FAILURE			13
#define USER_AUTHENTICATION_REQUIRED		14
//ACSE_SERVICE_PROVIDER
#define PROVIDER_NULL						0
#define PROVIDER_NO_REASON_GIVEN			1
#define PROVIDER_NO_COMMON_ACSE_VERSION		2

#define RPTPROQUE_SIZE		128		//上报协议队列大小
#define RPTWAITQUE_SIZE		30		//5		//上报待确认的队列大小

typedef struct
{//请勿修改顺序！
	DWORD dwSendMaxBytes;//最大发送信息字段
	DWORD dwRecvMaxBytes;//最大接收信息字段
	DWORD dwSendWndSize;//发送窗口大小
	DWORD dwRecvWndSize;//接收窗口大小
}TLnkTrsPara;//链路传输参数

typedef struct
{
	bool    fOk;//指明结果，对发送表示发送完毕，对接收表示接收完整
	bool	fFinal;//是否最后一个分帧
//	bool	fEnd;//是否最后一个帧
	WORD 	wFrmNo;//通讯帧序号，0-4095循环使用
	WORD 	wLen;//数据缓存长度
	BYTE	bBuf[LPDUSIZE];//数据缓存区
}TLPdu;//链路层1个pdu的结构

typedef struct
{
	bool	fValid;//有效标志
	
	BYTE	bPduNum;//Pdu数目
	BYTE	bStart;//消费指针
	TLPdu	LPdu[MAXLPDUNUM];
}TLPduPool;//链路层1个缓存池

typedef struct
{
	bool    fOk;		//指明结果，发送指发送完毕，接收指接收完整
	bool	fFinal;		//是否最后一个块
	WORD 	wLen;		//数据缓存长度
	int		iStep;		//分块请求的时候，分布请求
	WORD	wBlkNo;		//分帧编号
	BYTE	bBuf[APDUSIZE];//数据缓存区
}TAPdu;//应用层1个pdu的结构

typedef struct
{
	bool	fValid;	//有效标志
	BYTE	bSvrNo; //服务类型
	BYTE	bPduNum;//Pdu数目
	BYTE	bStart; //消费指针
	TAPdu	APdu[MAXAPDUNUM];//数据缓存区
}TAPduPool;//应用层1个缓存池

typedef struct
{
	bool	fValid;	//有效标志
	WORD 	wLen;//数据缓存长度
	BYTE	bBuf[APDUSIZE];//数据缓存区
}TInsertAPduPool;//应用层1个缓存池

//对于下行码此结构被用来记录除了cmd/cmdno/attri/mothod之外的内容
//对于上行码则是完整的应用层的组包
typedef struct
{
	bool fValid;//有效标志
	bool fFinal;//是否是最后一个请求步骤，iStep=-1回复
	WORD wLen;  //数据缓存长度
	BYTE bBuf[MAXDATASIZE];//数据缓存区
}TDataPool;//应用服务的数据池


typedef struct {
	WORD wOI;
	BYTE bAttr;
	BYTE bIndex;
	//属性设置内容
	BYTE *pbSetAttr;
	WORD wSetAttrLen;
	//-----------------
	BYTE *pbOAD;
	WORD wOADLen;
	BYTE *pbRSD;
	WORD wRSDLen;
	BYTE *pbRCSD;
	WORD wRCSDLen;
}TApduInfo;

typedef enum
{
	NullSecurity = 0,	//公共连接
	PasswordSecurity,	//一般密码
	SymmetrySecurity,	//对称加密
	SignatureSecurity,	//数字签名
}TConnectMechanismType;

typedef struct
{
	BYTE bMechanismType;
	BYTE bOsCiphertext[128];
	BYTE bOsSignature[128];
}TConnMechanismInfo;

typedef struct
{
	BYTE bProtoVer[2];			//协议版本号
	BYTE bProConformance[8];	//协议一致性块
	BYTE bFunConformance[16];	//功能一致性块
	WORD wSenFrmMaxLen;			//发送帧最大尺寸
	WORD wRcvFrmMaxLen;			//接收帧最大尺寸
	BYTE bRcvWindows;			//接收帧最大窗口尺寸
	WORD wHandleApduSize;		//可处理最大APDU尺寸
	DWORD dwConnectTimeOut;		//应用连接超时时间
	//TConnMechanismInfo tConnMechInfo;//会话密钥协商
}TConnectPara;	//请求连接的参数

//服务器厂商版本信息
typedef struct
{
	BYTE bFacCode[4];		//厂商代码
	BYTE bSoftVer[4];		//软件版本
	BYTE bSoftVerDate[6];	//软件版本日期
	BYTE bHardVer[4];		//硬件版本号
	BYTE bHardVerDate[6];	//硬件版本日期
	BYTE bFacInfo[8];		//厂家扩展信息
}TFacVersion;

//connect 客户端
typedef struct
{
	TConnectPara	tConnPara;
	BYTE			bMechanismInfo[2];	//协议位说明，实例帧说明
}TRxCliPara;

typedef struct
{
	TFacVersion		tTermnInfo;
	TConnectPara	tConnPara;
}TTxSvrPara;

typedef struct
{
//接收到一帧后取出的信息
	BYTE bFrmHeaderLen;	//帧头长度
	WORD wRFrmLen;		//接收帧全长
	BYTE bFunCode;		//接收帧的功能码
	BYTE bAddrType;		//接收地址类型
	BYTE bCliAddr;		//客户机地址
	BYTE bSvrAddLen;	//服务器地址长度
	BYTE bSvrAddr[16];	//地址TSA,oct-string,最大16字节
	BYTE bAPDUoffset;	//APDU 偏移
	WORD wAPDULen;		//APDU 的长度，方便分帧直接拷贝

//接收客户端参数
	BYTE bCliAddrLen;		//客户端地址长度
	BYTE bCliAddrs[16];		//客户端地址

//分帧参数
	bool fIsSegSend;	//分帧标志位
	WORD wRcvFrmSegNo;	//接收帧分帧序号
	WORD wSendFrmSegNo;	//发送帧分帧序号 
	BYTE FrmSegType;	//分帧类型
//参数协商
	TRxCliPara		tRxTrsPara;//接收的链路协商参数
	TTxSvrPara		tTxTrsPara;//发送的链路协商参数

	BYTE bCommStep;
	//2009-04-30 根据2009年招标技术条件新增
	BYTE m_bWaitMin;			//链路等待超时
	BYTE m_bLinkDetectCount;	//链路探测次数计数器	
}TLnkLayerComm;//链路层信息块


typedef struct
{
	WORD wClass;//类
	BYTE bObis[6];//Obis值
	BYTE bAttri;//属性

	BYTE bfAccess;//是否为选择性访问
	BYTE bSelectBuf[100];//选择性访问的描述

	BYTE bResult;//访问的结果
}TCosemAttriDesc;//COSEM属性描述块

typedef struct
{
	WORD wClass;//类
	BYTE bObis[6];//Obis值
	BYTE bMethod;//方法

	BYTE bfOptPara;//是否为带参数访问
	BYTE *pbOptPara;//指向所带参数的内容
	WORD wOptLen;//参数的长度

	BYTE bResult;//方法操作的结果
}TCosemMethodDesc;//COSEM方法描述块

typedef struct
{
	WORD wOI;
	BYTE bAttr;
	BYTE bIndex;
	BYTE bResult;//访问的结果
}TOIAccessDesc;

typedef struct 
{
	BYTE bOAD[4];		//对象属性描述符
	BYTE bRSDLen;
	BYTE bRSD[128];		//记录选择描述符
	BYTE bRCSDLen;
	BYTE bRCSD[256];	//记录列选择描述符
}TOIRecordDesc;	//记录型数据访问

typedef struct
{
	WORD wOI;			//对象ID
	BYTE bMethod;		//方法
	BYTE bOpMode;		//模式
	DWORD dwActRdOAD;	//设置后读取的OAD
	BYTE bRdDelay;		//设置后读取OAD的时间
	DWORD dwRdClick;	//设置的时间---用于计算延迟读取时间

	bool fOptPara;//是否为带参数访问
	BYTE *pbOptPara;//指向所带参数的内容
	int wOptLen;//参数的长度

	BYTE bResult;//方法操作的结果
}TOIMethodDesc;//OI方法描述块

typedef struct
{
	//1.下行召唤命令触发的信息记录
	//1.1服务
	BYTE bCmd;		//记录每1个帧的服务值aarq/get/set/action
	BYTE bCmdMod;	//记录每1个帧的模式值
	BYTE bPIID;		//记录每1个帧的Invoke-Id-And-Priority值

	bool fNewServer;
	BYTE bServer;	//记录一次服务aarq/get/set/action,即第一个命令的服务值
	BYTE bServerMod;	//记录一次服务aarq/get/set/action的模式,即第一个命令模式的值
	bool fMoreBlk;	//根据bServer+bServerMod判别是否有多块
	WORD wBlkNo;	//当前块号
	int iStep;	//操作库函数需要的参数,用来分布获取回复数据的
	int iTabIdx;	//方案采集任务表检索

	//对协议层整理
	BYTE bAskBuf[1024];	//协议层RSD\RCSD\OAD缓冲区
	BYTE *pbAskStart;
	BYTE *pbAskMid;
	BYTE *pbAskEnd;

	//1.2属性
	BYTE bAskItemNum;	//请求的属性数量
	//1.3 记录
	BYTE bAskRecItemNum;	//请求记录的数量
	//1.4方法
	BYTE bAskMethodNum;	//请求的方法数量
	//1.5仅发送需要的
	BYTE bAnsServer;	//记录一次服务响应的服务值
	BYTE bAnsServerMod;	//记录一次服务响应命令模式的值
	BYTE bAnsCmdMod;	//记录每一块响应命令模式的值
	bool fAnsMoreBlk;	//根据响应数据判别是否有多块
	DWORD dwAnsBlkNo;	//响应数据当前块号

	//2.上行主动上报触发的信息记录
	BYTE bEvtWPtr,bEvtRPtr;//事件主动上报的读写指针

	BYTE bConnectSta;	
	BYTE bMyPIID;		//记录自行上报的每1个帧的Invoke-Id-And-Priority值
}TAppLayerComm;//应用层信息块


typedef struct
{
	bool fSecurityLayer; //是否包含安全传输协议层
	BYTE bAppDataUnit; // 应用数据单元
	BYTE bDataAuthInfo; // 数据验证信息
	BYTE bRn[32]; // 随机数
	DWORD dwRnLen; // 随机数长度
	BYTE bMac[4];
	BYTE bSID[4];
	BYTE bErrInfo;
} TSecurityParam; //安全参数

typedef struct
{
	BYTE bRn[32]; // 随机数
	BYTE bMac[4]; 
} TRptSecureParam; //主动上报安全参数

typedef enum
{
	SecureData_Plaintext = 0,	//明文应用数据单元
	SecureData_Ciphertext,		//密文应用数据单元
	SecureData_ErrorDAR,		//异常错误
}TSecureDataUnitType;	//应用数据单元  CHOICE

typedef enum
{
	AuthType_SIDMAC = 0,	//数据验证码      [0]  SID_MAC，
	AuthType_RN,			//随机数          [1]  RN，
	AuthType_RNMAC,			//随机数+数据MAC  [2]  RN_MAC，
	AuthType_SID,			//安全标识        [3]  SID
}TSecureAuthType;	//数据验证信息  CHOICE

//数据验证码      [0]  SID_MAC，
//随机数          [1]  RN，
//随机数+数据MAC  [2]  RN_MAC，
//安全标识        [3]  SID




#define MAXATTRI				30	//支持最大属性数目
#define MAXMETHOD				30	//支持最大方法数目

typedef struct
{
	bool fDedicatedKeyOpt;//出否有加密的密码

	bool fRespAllowed;//是否需要响应

	bool fProposedQualServ;//是否有推荐的服务质量值这一项
	BYTE bProposedQualServ;//推荐的服务质量值

	BYTE bProposedDlmsVer;//推荐使用的版本
	
	BYTE bProposedConfirmance[3];//推荐使用的一致性块

	WORD wClientMaxPduSize;//客户最大的pdu大小
}TPdu_IniRequest;//应用初始化请求

typedef struct
{
	bool fNegotiatedQualServ;//是否有协商的服务质量值这一项
	BYTE bNegotiatedQualServ;//协商的服务质量值

	BYTE bNegotiatedDlmsVer;//协商使用的版本

	BYTE bProposedConfirmance[3];//协商使用的一致性块

	WORD wServerMaxPduSize;//服务器最大的pdu大小

	WORD wVaaName;//虚拟名
}TPdu_IniResponse;//应用初始化请求响应

#define		RPTMAXFRM_EVERYSEND		30		//调用一次AutoSend()最多发送的帧数
#define		FRM_QUE_MAX				5	//接收事件消息的帧数	
class CFaProto: public CProto
{
public:
    CFaProto();
    ~CFaProto();

	int LPduSegFrmHandle();
	bool CombinApduFrm(TLPduPool RxLpduPool, TDataPool* ptRxApduPool);
	bool HandleFrm();//协议处理接口
	virtual void DoProRelated();	//做一些协议相关的非标准的事情
////////////////////////////LINK layer ////////////////////////////////////////////
	int  RcvBlock(BYTE *pbBuf,int wLen);//检测有效帧

	CQueue 	m_TrsQueue;  //转发的报文消息队列
	void DoNoComuTimeout();
	int MakeLinkFrm(BYTE bLinkSvr);
	int MakeSegFrm(BYTE bType, WORD wSeg);

	bool Beat();
	void Release_response(BYTE* pApdu);

	int Get_response(BYTE* pApdu);
	int Get_request_normal();
	int Get_request_normal_list();
	int Get_request_record();
	int Get_request_record_list();
	int Get_request_next();

	int Set_response(BYTE* pApdu, WORD wLen);
	int Set_Request_Normal();
	int Set_Request_Normal_List();
	int Set_Then_Get_Request_Normal_List(BYTE *pbApdu, WORD wApduLen);

	int Act_response(BYTE* pApdu, WORD wApduLen);
	int Act_Response_Normal();
	int Act_Response_List();
	int Act_Then_Rd_List();

	int ProxyResponse();
	int ProxyResponse(BYTE bPoxyType);
	int ProxyGetRequestList();
	int ProxyGetRequestRecord();
	int ProxySetRequestList();
	int ProxySetThenGetRequestList();
	int ProxyActionRequestList();
	int ProxyActionThenGetRequestList();
	int ProxyTransCommandRequest();

	int SecurityRequest(BYTE* pApdu, WORD *pwAPDULen);
	int SecurityResponse(BYTE* pApdu, WORD *pwAPDULen);
	int MakeSecureErrFrm(void);

	//先定义接口，后续再增加具体的功能 20161026  CL
	int SetTimeFlg(){return 0;};
	int GetTimeFlg(){return 0;};
	int SetRptFlg(){return 0;};	
	int GetRptFlg(){return 0;};

	//协议层APDU解析
	//int ProNormalToApduInfo(BYTE *pbApdu, TApduInfo *pApduInfo);
	int ProRecordToApduInfo(BYTE *pbApdu, TApduInfo *pApduInfo);
private:
	bool AddrCheck();
	bool Link_Request(BYTE *pApdu, WORD wApduLen);
	bool Link_Responce(BYTE *pApdu, WORD wApduLen);
	void Connect_response(BYTE* pApdu);

	BYTE m_bOffsetAPDU;

	TLnkLayerComm m_LnkComm;//链路层信息记录块
	TLPduPool	m_RxLPduPool;//To APP
	TLPduPool	m_TxLPduPool;//From APP

	BYTE	m_bRxBuf[MAXFRMSIZE];//接收帧的缓存区
	BYTE	m_nRxStep;
	WORD	m_wRxPtr,m_nRxCnt;

	BYTE	m_bTxBuf[MAXFRMSIZE];//发送帧的缓存区
	WORD	m_wTxPtr;
	BYTE	m_bAutoBuf[MAXFRMSIZE];//主动上报发送帧的缓存区
	WORD	m_wAutoPtr;

	DWORD	m_dwProRxClick;
	DWORD   m_dwProTxClick;
	BYTE    m_bNoComuSta;
	DWORD	m_dwClickFrmOk;
	DWORD	m_dwBeatClick;

	bool    m_fPwrOnRun;
	BYTE	m_bBeatNum;	//心跳个数
	BYTE	m_bValidNum;	//m_iK[]数组的有效个数
	int		m_iK[PRE_ADJ_MAX_CNT];

	WORD	m_wCurTaskId;
	WORD	m_wLastTaskId;
	int		m_iStart;
	int		m_iRdPn;	//本次读的测量点	
	int		m_iRsd10Pn;
private:
	TNegoSizeDef  m_NegoSizeDef;//空间协商参数块
	void InitCommSize(BYTE bType);
	void CheckMNegoPara();

private:
	//以下为链路层接收处理函数
	int  VeryFrm();//链路层接收检验

	//以下为链路层发送处理函数
	int Tx_RegisterBeat();
	int Tx_PriorFrm(bool fFinal);

	int MakeFrm(BYTE *pbBuf, WORD wLen);
	int MakeAutoFrm(bool fFinal);//链路层主动上报发送帧的统一出口，完成帧头帧尾和校验码，实现物理发送

private:
	//以下为链路层相关函数
	void Tx_NegotiatoryPara();//链路层参数协商参数组帧
	void ResetLnkPara();//建立链路后复位所有的链路参数
	void ClearVar(BYTE bConcentSta);//清零变量
	WORD GetTxHeaderLen(){ return 11; };//本例可以固定
	BYTE NextNSeq(BYTE bN){ return (bN+1)&0x7; };//收发计数的下一个有效值
	void ClsLPduPool(TLPduPool *p);//清链路层的PDU缓存池

	int ToSecurityLayer();//提供给应用层调用的接口,先经安全层，再调用链路层
	int ToLnkLayer();//提供给应用层调用的接口
	int DoLPdu();//处理一个LPdu的组帧和发送
	int DoInsertLPdu();//处理一个插入的LPdu的组帧和发送（此时LPdu等于插入的APdu）

	void SetWaitTime(BYTE bWaitMin);		//记录回帧等待超时
	void StartWaitTimer(void);
	void ResetWaitTimer(void);

	int GetErrOfGet(int iRetVal);
	int GetErrOfSet(int iRetVal);
	int GetErrOfAct(int iRetVal);
////////////////////////////APP layer ////////////////////////////////////////////
private:
	TAppLayerComm m_AppComm;//应用层信息记录块
	TSecurityParam m_SecurityParam;//加密层信息
	TRptSecureParam m_RptSecureParam; //主动上报用的加密层信息
	TAPduPool	m_RxAPduPool;//To APP data manage
	TAPduPool	m_TxAPduPool;//From APP data manage	
	TAPdu		m_RxAPdu;		//接收到的APDU帧
	TAPdu		m_TxAPdu;		//698-45暂时在应用层考虑每次一个自解析的帧分包下去，后续的帧直接使用get-next处理就可以了
	TInsertAPduPool m_TxInsAPduPool;//插入式传送帧，目前用于处理过程错误返回，因为需要保留上一个服务的信息，所以需另开缓存

	void ClsAPduPool(TAPduPool *p);//清应用层的PDU缓存池
	void NewAppServer();//接收一个新的服务后所做的预备工作
	int GetReq(BYTE *pbBuf,WORD wLen);//处理GET服务请求
	int SetReq(BYTE *pbBuf,WORD wLen);//处理SET服务请求
	int ActReq(BYTE *pbBuf,WORD wLen);//处理ACT服务请求

	int GetResErr(BYTE bGetMod, BYTE bErr);//对GET服务的出错响应处理
//	int SetRes(BYTE bSetMod);//对SET服务的响应处理
//	int ActRes(BYTE bActMod, DWORD bErr_BlkNo);//对ACT服务的出错响应处理

	int GetData();//取数据操作和分块
	int GetListData();//取列表数据操作和分块
	int SetData();//设定数据和分块组合处理
	int SetListData();//设定列表数据和分块组合处理
	int ActData();//方法操作和分块处理
	int ActListData();//列表方法操作和分块处理

	int DecodeChoice(BYTE *pbBuf);//对曲线的选择性访问中Choice的单独解码

////////////////////////////APP manage ////////////////////////////////////////////
private:
	TDataPool	m_RxDPool;
	TDataPool	m_TxDPool;  
	void ClsDataPool(TDataPool *p){ memset(p,0,sizeof(TDataPool)); };//清应用层的数据缓存池
	int GetEvent(BYTE *pb);//完成上报事件的应用层信息组帧
	int AutoReport();//完成上报事件的一帧编码和发送

	bool GetEventWritePtr(BYTE& bWrPtr);
	BYTE GetEvtRptFlg();

////////////////////////////old function////////////////////////////////////////////
////////////////////////////old function////////////////////////////////////////////
public:    
	bool Init(TFaProPara* pFaProPara);

	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TFaProPara* pFaProPara))
	{		//注册装载非复位参数的函数
		m_pfnLoadUnrstPara = pfnUnrstPara;
	};

	//各协议共用虚函数	
	bool Login();
//	bool Beat(){ Tx_RegisterBeat(); return true; };
	void OnConnectOK();
	bool IsNeedAutoSend() { return false; }; //是否需要主动上报
	void LoadUnrstPara(){ return; };
	DWORD BeatMinutes(){ return m_wBeatMinutes; };
	WORD GetCnType() { return m_wCnType;};
	
	void OnBroken();

public:
    //各协议共用变量
	bool 		m_fConnected;
	WORD 		m_wRunCnt;
	bool 		m_fErrRst;		//通信发生错误,需要复位终端标志
	WORD 		m_wCnType;
	WORD 		m_wMaxFrmBytes;   //最大传送字节数
	BYTE 		m_bConnectTypefLocal; //连接方式
	
protected:
	bool (*m_pfnLoadUnrstPara)(TFaProPara* pFaProPara);
	TFaProPara* m_pFaProPara;
	//各协议共用变量
	WORD 		m_wBeatMinutes;  //心跳间隔,单位分钟
	BYTE 		m_bSftpBuf[1200];
	CSftp 		*m_pSftpClient;
	char 		m_szCmdLine[100];
//主动上报部分（变量+函数）
public:
	//CProMngRpt* m_pProMngRpt;
	//CProWaitQue m_WaitQue;			//发送出去需要确认的帧，此为待确认区
	//TAskMsg		m_AskQue;			//请求任务数据使用
	CQueue m_Queue;     //协议线程的报文消息队列
	TFapRptMsg  m_pRptMsg[RPTWAITQUE_SIZE];	// 发送出去需要确认的帧，此为待确认区
	DWORD m_dwOldTick;
	BYTE		m_bRptMsgNum;	//发送出去需要确认的帧的数目
	void WaitQueClear();
	bool WaitQueDelete(BYTE bIdx);
	bool WaitQueInsert(TFapRptMsg *pMsg);
	bool IsFull(){ return (m_bRptMsgNum>=RPTWAITQUE_SIZE); };
	bool WaitQueGetMsg(BYTE bIdx,TFapRptMsg*pMsg );
	int WaitQueGetSize(){ return RPTWAITQUE_SIZE; };
	int WaitQueGetNum(){ return m_bRptMsgNum; };
	void TaskRpt(BYTE * pbNSend);
	bool AutoSend();
	void ReRpt(BYTE * pbNSend);
	void WaitQue(BYTE bClass, BYTE bTxCn, DWORD dwCycTime, WORD bNum, BYTE *pBuf, WORD wLen, BYTE  bStage=0);
	void EventRpt(BYTE * pbNSend);
	BYTE GetMyPIID(){//低6位有效0~63
	BYTE bPIID = m_AppComm.bMyPIID; 
	m_AppComm.bMyPIID++; 
	if (m_AppComm.bMyPIID > 63)
		m_AppComm.bMyPIID = 2;
	return (bPIID);
	};
	int Rpt_response(BYTE* pApdu, WORD wApduLen);
	int Rpt_SecureLayer(BYTE *pApdu, WORD wApduLen, BYTE *pSecureApdu);
	
	int AppendEvtMsg(TEvtMsg* pEvtMsg);
	void SetCnOAD();
	CFrmQue m_queEvt;
	DWORD m_dwCnOAD;
	
private:
	//兼容浙江版的扩展协议帧定义
	int ZJHandleFrm(BYTE* pbRxBuf, BYTE* pbTxBuf);
	int ZJUserDef(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
	int ZJReadDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf, bool fWordPn);
	int	ZJWriteDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf, bool fWordPn);	
	int ZJReExtCmd(BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf);
	int ZJReplyErr(BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf);
	int ZJMakeFrm(WORD wDataLen, BYTE* pbRxBuf, BYTE* pbTxBuf, bool fErr);
	int ZJSftpDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
	//int ZJBatchDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
	bool ZJRunCmd(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
	int ZJTrigerAdj(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
	int ZJLoadParaFile(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf);
};//end class CFaProto;

#endif  //FAPROTO_H

