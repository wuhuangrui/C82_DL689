#ifndef PROSTRUCT_H
#define PROSTRUCT_H

#include "Comm.h"

#define MTRFWD_BUFSIZE		2000
#define ASKQUEUESIZE			20		//�ϱ����д�С
/*typedef struct{
	void*	dwFromMasterThis;		//��վЭ��ָ��
	DWORD	dwClick;				//������Ϣʱ��ʱ��

	TCommPara CommPara;				//���ڲ���
	WORD	wFrmTimeout;			//���յȴ����ĳ�ʱʱ��(ms)��0��Ч
	WORD	wByteTimeout;			//���յȴ��ֽڳ�ʱʱ��(ms)��0��Ч

	BYTE    bBufValidFlag;			//0������Ӧ 1����Ӧ 
	WORD  	wBufLen;				//֡����
	BYTE  	bBuf[MTRFWD_BUFSIZE];	//֡������
}TMtrFwdMsg;//һ��͸��������Ϣ�ṹ
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
	BYTE  	bTxCnt;				//���ʹ���
	BYTE  	bFrmNum;			//���ʹ���
	bool 	fIsLastFrm;
	BYTE	bRptInfBuf[16];		//TTmrRptInfC2��TTmrRptInfC1�Ĵ�С�Ĵ���

	WORD  	wLen;				//����֡����
	BYTE  	bBuf[FAP_RPT_FRMSIZE];	//����֡������
	DWORD	dwSendTime;			//��һ�η���ʱ��
	DWORD	dwCycTime;			//����ʱ��
	DWORD	dwTrigCnt;			//����������ֵ
}TFapRptMsg;//һ����Ϣ�ṹ�����������ϱ�һ����֡�ļ�¼

typedef struct{
	BYTE	bMsgNum;
	TFapRptMsg AskMsg[ASKQUEUESIZE];
}TAskMsg;//һ����Ϣ�ṹ�����������ϱ�һ����֡�ļ�¼
#endif //PROSTRUCT_H

