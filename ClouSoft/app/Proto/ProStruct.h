#ifndef PROSTRUCT_H
#define PROSTRUCT_H

#include "Comm.h"

#define MTRFWD_BUFSIZE		2000
#define ASKQUEUESIZE			20		//上报队列大小
/*typedef struct{
	void*	dwFromMasterThis;		//主站协议指针
	DWORD	dwClick;				//发送消息时的时间

	TCommPara CommPara;				//串口参数
	WORD	wFrmTimeout;			//接收等待报文超时时间(ms)，0无效
	WORD	wByteTimeout;			//接收等待字节超时时间(ms)，0无效

	BYTE    bBufValidFlag;			//0正常响应 1无响应 
	WORD  	wBufLen;				//帧长度
	BYTE  	bBuf[MTRFWD_BUFSIZE];	//帧缓冲区
}TMtrFwdMsg;//一个透明传输消息结构
*/

//#define FAP_RPT_FRMSIZE        			1024
#define FAP_RPT_FRMSIZE					5000//MAXFRMSIZE
#define FAP_RPT_FRMTAIL        			204

typedef struct{
	BYTE	bClass;
	//BYTE	bTask;
	WORD	bTask;
	BYTE	bSeq;
	BYTE	bNeedConfirm;
	BYTE  	bTxCnt;				//发送次数
	BYTE  	bFrmNum;			//发送次数
	bool 	fIsLastFrm;
	BYTE	bRptInfBuf[16];		//TTmrRptInfC2和TTmrRptInfC1的大小的大者

	WORD  	wLen;				//发送帧长度
	BYTE  	bBuf[FAP_RPT_FRMSIZE];	//发送帧缓冲区
	DWORD	dwSendTime;			//上一次发送时间
	DWORD	dwCycTime;			//周期时间
	DWORD	dwTrigCnt;			//触发计数器值
}TFapRptMsg;//一个消息结构，用于主动上报一个组帧的纪录

typedef struct{
	BYTE	bMsgNum;
	TFapRptMsg AskMsg[ASKQUEUESIZE];
}TAskMsg;//一个消息结构，用于主动上报一个组帧的纪录
#endif //PROSTRUCT_H

