/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FaProto.h
 * ժ    Ҫ�����ļ���Ҫʵ���ն�ʹ��698-45����ͨѶ
 * ��ǰ�汾��1.0
 * ��    �ߣ��׳ɲ�
 * ������ڣ�2016��9��
 * ʵ�ַ�ʽ˵����1�����ڷֲ��ʵ�ַ�ʽ���ն������㣬һ������·�㣬һ����Ӧ�ò�
 *               2��������Զ�������·���жϽ��յ���֡�Ƿ�����APDU������ǣ�������·�������һ֡APDU��֮���ύ��Ӧ�ò㴦��
 *				 3��Ӧ�ò�ֲ����Ӧ�����ݰ��ְ������һ�����ݽϴ�����ܶ����������Ǿͷֽ�ɶ��������Ķ�֡APDU������ǿ�Ʋ𿪴���
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

#define APDU_MAX_SIZE		2040	//APDU���ռ�

#define PRE_ADJ_MAX_CNT		255	//��ȷ��ʱ������

#define FIRST_FRM	0x00	//��֡
#define LAST_FRM	0x40	//���֡
#define MID_FRM		0xC0	//�м�֡

//////////////////////////////////////////////////////////////
//���Զ���
#define ATTR_0 0
#define ATTR_1 1
#define ATTR_2 2
#define ATTR_3 3
#define ATTR_4 4
#define ATTR_5 5
#define ATTR_6 6

//��ַ���Ͷ���
#define ADDR_TYPE_SINGLE		0	//����ַ 
#define ADDR_TYPE_UNIVERSAL		1	//ͨ���ַ
#define ADDR_TYPE_GROUP			2	//���ַ
#define ADDR_TYPE_BROADCAST		3	//�㲥��ַ

//
#define MAXATTRI				30	//֧�����������Ŀ
#define MAXMETHOD				30	//֧����󷽷���Ŀ
#define MAXRECORD				10	//֧������¼��Ŀ

//�ն��Զ���Э�鲿�ֵĶ���
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
//�㽭Э�鶨�����

//////////////////////////////////////////////////////////////
///////////�����µĶ���//////////////////
//���ӷ�ʽ
#define CONNECTTYPE_LOCAL               0      //�������ӷ�ʽ
#define CONNECTTYPE_GPRS                1      //GPRS���ӷ�ʽ
#define CONNECTTYPE_UPLINK             	2      //UPLINK���ӷ�ʽ
#define CONNECTTYPE_ETH                3      //��̫�����ӷ�ʽ
#define CONNECTTYPE_MEGA16            	4      //�͵�Ƭ��ͨѶ

//���޸���
#define GBPRO_MAXSUMGROUP		(GB_MAXSUMGROUP-GB_MAXOFF)	//�ܼ������
#define GBPRO_MAXPOINT			(GB_MAXMETER-GB_MAXOFF)		//���������

//����ռ��С����
#define GB_FRM_SIZE   					2048				//��Э��֧��һ֡�Ļ����С
//ͨ��ƽ̨ͳһʹ�õ�֡���ȶ���
#define FAP_FRM_SIZE        			GB_FRM_SIZE

#define SMS_ADDR_INTER    				0x91
//�����µĶ������
//////////////////////////////////////////////////////////////

typedef struct{
	TProPara ProPara;
	WORD wConnectType;
	
	BYTE bRTUA[4];	//0-1:�ն˵���������;2-3:�ն˵�ַ
	BYTE bProtoVer;		//Э��汾�����ڿ���Э��İ汾����
}TFaProPara;

typedef struct{
	BYTE  	bTxBuf[FAP_FRM_SIZE];	//����֡������
	WORD  	wTxLen;					//����֡����
}TFapMsg;//

// typedef struct{	
// 	TProPara ProPara;
// 	WORD	wConfirmDelayTime;	//ȷ�ϳ�ʱ(s)
// 	bool 	fNeedConfirm1;		//��Ҫȷ�ϵķ���1
// 	bool 	fNeedConfirm2;		//��Ҫȷ�ϵķ���2
// 
// 	BYTE	bAuthType;			//��Ϣ��֤������
// 	WORD	wAuthPass;			//��Ϣ��֤����
// 	
// 	WORD	wGrpAddr[8];		//���ַ
// 	WORD	wAddr1;				//����������A1
// 	WORD	wAddr2;				//�ն˵�ַA2
// 	
// 	//��Э�鲻ͬ������
// 	WORD	wConnectType;		//��������			
// 	BYTE	bMasterNo;			//��վ���,ÿһ����վ����һ�������ϱ��Ķ�ָ��,����վ����2��ʱʹ�ô˲Σ�	
// }TGbProPara;//�������ò���


#define NOT_RPT		0
#define NOW_RPT		1
#define BEAT_RPT	2
#define FOLLOW_RPT	3

//����Ҫ�Ŀռ仺��Ĵ�С
//#define MAXFRMSIZE				2120	//�շ�1��֡�Ļ��泤��
#define MAXFRMSIZE				5000	//�շ�1��֡�Ļ��泤��
#define MAXDATASIZE				3000	//�շ����ݵ�1�����޻��泤��

#define MAXLPDUNUM				3		//����������֡��󴰿ڳߴ磬��1��APDU���䵽��·�������LPDU������
#define APDUSIZE				6102	//���������ɴ����APDU�ߴ磬(ESAM���ն�֤��Ϊ2052���ֽ�)
#define LPDUSIZE				(APDUSIZE/MAXLPDUNUM)	//��·�㴫��֡������ֽ�������APDUSIZE��MAXLPDUNUM����
#define MAXAPDUNUM				8//((MAXDATASIZE/APDUSIZE)+1)		//1��APDATA���䵽Ӧ�ò������APDU������\\

//�����ʵ��ʹ�ÿ����б仯����һ�ײ�����¼��Ŀǰ����Ҫ���Ǻ�������վ�ͳ�������2�������2008.3.25
//˵�������������������1K�ֶΣ����������ݽ����1143B(�����ļ���87B)�����Էſ�wApduCutSize��1160B
typedef struct{//																��������վ		��������1K�ֶ�		����
	BYTE bType;//֧�ֵ�����															0				1				2
	WORD wMaxFrmSize;//<=MAXFRMSIZE													300				1300
	WORD wMaxDataSize;//==MAXDATASIZE												3000			3000
	WORD wLpduSize;//��·Э�̲���-2													253				1220-2
	WORD wMaxLpduNum;//min(MAXLPDUNUM,((wApduSize/wLpduSize)+1))					3				1
	WORD wApduSize;//Ӧ��Э�̲���													512				1200
	WORD wMaxApduNum;//min(MAXAPDUNUM,((wMaxDataSize/wApduSize)+1))					6				3
	WORD wApduCutSize;//Ӧ��Э�̲���-30												490				1160
}TNegoSizeDef;//Э�̲����Ķ���

//////////////////////////////////֡���Ͷ���//////////////////////////////////////////////
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

///////////////////////////////////�������///////////////////////////////////////////////
#define RESULT_OK				0x00
#define RESULT_NOK				0x01
#define RESULT_NO_RES			0x02
#define RESULT_REMOTE			0x03
#define RESULT_LOCAL_DL			0x04
#define RESULT_LOCAL_PHY		0x05

///////////////////////////////////Ӧ�ò�״̬����/////////////////////////////////////
#define APPST_NOK				0	//�ȴ���֤Э��
#define APPST_OK				1	//ͨ����֤Э��

///////////////////////////////////����״̬����/////////////////////////////////////
#define LNKST_NDM				0	//�Ͽ�״̬
#define LNKST_NRM				1	//����
#define LNKST_BUSY				2	//æ

///////////////////////////////////GPRS״̬///////////////////////////////////////////////
#define		MODEM_OPENED		(1<<0)		//ģ���
#define		GPRS_ONLINE			(1<<1)		//ģ������
#define		NO_SIGNAL			(1<<2)		//���ź�
#define		NO_NETWORK			(1<<3)		//������
#define		CONNECT_FAILED		(1<<4)		//������������

///////////////////////////////////λ�ö���//////////////////////////////////////////////
#define HDLCLOC_START					0	//��ʼ�ֽڵ�λ��
#define HDLCLOC_FRMTYPE1				1	//������ĵ�һ���ֽڵ�λ��
#define HDLCLOC_FRMTYPE2				2	//������ĵڶ����ֽڵ�λ��
#define HDLCLOC_ADDR					3	//��ַ��ĵ�һ���ֽڵ�λ��

///////////////////////////////////Դ�Դ�վ��������Ӧ��ģʽ�ı���////////////////////////////////////
#define LINK_REQ		1	//Ԥ��������
#define CONNECT_REQ		2	//����Ӧ����������
#define RELEASE_REQ		3	//�Ͽ�Ӧ����������
#define GET_REQ			5	//��ȡ����
#define SET_REQ			6	//��������
#define ACTION_REQ		7	//��������
#define REPORT_RES		8	//�ϱ�Ӧ��
#define PROXY_REQ		9	//��������
#define SECURITY_REQ	16	//��ȫ����

#define PROXY_GET_REQ_LIST			1	//�����ȡ���ɸ������������ɸ�������������
#define PROXY_GET_REQ_RECORD		2	//�����ȡһ����������һ����¼�Ͷ�����������
#define PROXY_SET_REQ_LIST			3	//�����������ɸ������������ɸ�������������
#define PROXY_SET_THEN_GET_REQ_LIST	4	//�������ú��ȡ���ɸ������������ɸ�������������
#define PROXY_ACT_REQ_LIST			5	//����������ɸ������������ɸ����󷽷�����
#define	PROXY_ACT_THEN_GET_REQ_LIST	6	//����������ȡ���ɸ������������ɸ����󷽷�����������
#define PROXY_TRANS_CMD_REQ			7	//����͸��ת����������

#define LINK_RESPONSE	129	//�ն���������01��link����վ�ش��response
#define CONNECT_RES		130	//����Ӧ��������Ӧ
#define RELEASE_RES		131	//�Ͽ�Ӧ��������Ӧ
#define RELEASE_NOTI	132	//�Ͽ�Ӧ������֪ͨ
#define GET_RES			133	//��ȡ��Ӧ
#define	SET_RES			134	//������Ӧ
#define ACTION_RES		135	//������Ӧ
#define REPORT_NOTI		136	//�ϱ�֪ͨ
#define PROXY_RES		137	//������Ӧ
#define SECURITY_RES	144	//��ȫ��Ӧ

//////////////////////////////////Get-request���������Ͷ���/////////////////////////////////////////////////
#define GET_NORMAL			1
#define GET_NORMAL_LIST		2
#define GET_RECORD			3
#define GET_RECORD_LIST		4
#define GET_NEXT			5

//////////////////////////////////Set-request���������Ͷ���/////////////////////////////////////////////////
#define SET_NORMAL				1	
#define SET_NORMAL_LIST			2
#define SET_GET_NORMAL_LIST		3

//////////////////////////////////Action-request���������Ͷ���/////////////////////////////////////////////////
#define ACT_NORMAL				1	
#define ACT_NORMAL_LIST			2
#define ACT_GET_NORMAL_LIST		3

//////////////////////////////////Report-Notification���������Ͷ���///////////////////////////////////////////
#define RP_NOTI_LIST				1	
#define RP_NOTI_REC_LIST			2

//////////////////////////////////PROXY-Request���������Ͷ���/////////////////////////////////////////////////
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

///////////////////////////////////���ݷ��ʽ���Ķ���///////////////////////////////
enum TLinkRequestSvr
{
	SVR_LINK = 0,		//��������
	SVR_BEAT = 1,		//����
	SVR_RELEASE = 2		//�Ͽ�����
};

///////////////////////////////////���ݷ��ʽ���Ķ���///////////////////////////////
#define DR_ERROK			0		//�ɹ�
#define	DR_HardFault		1		//Ӳ��ʧЧ
#define	DR_TempFail			2		//��ʱʧЧ
#define	DR_RWDenied			3		//��д�ܾ�
#define	DR_ObjUndefined		4		//����δ����
#define	DR_ObjIFInValid		5		//����ӿڲ�����
#define	DR_ObjInExist		6		//���󲻴���
#define	DR_TypeUnMatch		7		//���Ͳ�ƥ��
#define	DR_OverFlow			8		//Խ��
#define	DR_DataBlockErr		9		//���ݿ鲻����
#define	DR_SegFrmCancel		10		//��֡������ȡ��
#define	DR_NoSegFrmStat		11		//�����ڷ�֡����״̬
#define	DR_WRBlockCancel	12		//��дȡ��
#define	DR_WrBlockNoExist	13		//�����ڿ�д״̬
#define	DR_FrmNoErr			14		//���ݿ������Ч
#define	DR_PWErr			15		//�����/δ��Ȩ
#define	DR_BoteChgErr		16		//ͨѶ���ʲ��ɸ���
#define	DR_YearTimeZoneOver	17		//��ʱ������
#define	DR_PeriodNumOver	18		//��ʱ������
#define	DR_RateNumOver		19		//��������
#define	DR_SecCertUnMatch	20		//��ȫ��֤��ƥ��
#define	DR_RepRecharge		21		//�ظ���ֵ
#define	DR_ESAMErr			22		//ESAM��֤ʧ��
#define	DR_SecCertFail		23		//��ȫ��֤ʧ��
#define	DR_CPNUnMatch		24		//�ͻ���Ų�ƥ��
#define	DR_RechargeNumErr	25		//��ֵ��������
#define	DR_BuyElecOverFlow	26		//���糬�ڻ�
#define	DR_AddrErr			27		//��ַ�쳣
#define	DR_SymDecodeErr		28		//�Գƽ��ܴ���
#define	DR_UnSymDecodeErr	29		//�ǶԳƽ��ܴ���
#define	DR_SignErr			30		//ǩ������
#define	DR_MeterHalt		31		//���ܱ����
#define	DR_TimeTagErr		32		//ʱ���ǩ��Ч
#define	DR_Other			255		//����

///////////////////////////////////����ִ�н���Ķ���////////////////////////////////////
enum TActionResult
{
	AR_Success = 0,                //�ɹ�               
	AR_HardFault = 1,              //Ӳ������           
	AR_TempFail = 2,               //��ʱ����           
	AR_RWDenied = 3,               //��д�����ܾ�       
	AR_ObjUndefined = 4,           //����δ����         
	AR_ObjClsInconsistent = 9,     //��Ͷ���һ��     
	AR_ObjUnavailable = 11,        //������Ч           
	AR_TypeUnmatched = 12,         //���Ͳ�ƥ��         
	AR_ScopeViolated = 13,         //��Χ����           
	AR_DBlkUnavailable = 14,       //��Ч��             
	AR_LongActAborted = 15,        //̫���ķ������������˳� 
	AR_NoLongAct = 16,             //��Ӧ�ٲ���������       
	AR_Other = 250                 //��������
};                                        
  
//AARE��Э�̽��
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

#define RPTPROQUE_SIZE		128		//�ϱ�Э����д�С
#define RPTWAITQUE_SIZE		30		//5		//�ϱ���ȷ�ϵĶ��д�С

typedef struct
{//�����޸�˳��
	DWORD dwSendMaxBytes;//�������Ϣ�ֶ�
	DWORD dwRecvMaxBytes;//��������Ϣ�ֶ�
	DWORD dwSendWndSize;//���ʹ��ڴ�С
	DWORD dwRecvWndSize;//���մ��ڴ�С
}TLnkTrsPara;//��·�������

typedef struct
{
	bool    fOk;//ָ��������Է��ͱ�ʾ������ϣ��Խ��ձ�ʾ��������
	bool	fFinal;//�Ƿ����һ����֡
//	bool	fEnd;//�Ƿ����һ��֡
	WORD 	wFrmNo;//ͨѶ֡��ţ�0-4095ѭ��ʹ��
	WORD 	wLen;//���ݻ��泤��
	BYTE	bBuf[LPDUSIZE];//���ݻ�����
}TLPdu;//��·��1��pdu�Ľṹ

typedef struct
{
	bool	fValid;//��Ч��־
	
	BYTE	bPduNum;//Pdu��Ŀ
	BYTE	bStart;//����ָ��
	TLPdu	LPdu[MAXLPDUNUM];
}TLPduPool;//��·��1�������

typedef struct
{
	bool    fOk;		//ָ�����������ָ������ϣ�����ָ��������
	bool	fFinal;		//�Ƿ����һ����
	WORD 	wLen;		//���ݻ��泤��
	int		iStep;		//�ֿ������ʱ�򣬷ֲ�����
	WORD	wBlkNo;		//��֡���
	BYTE	bBuf[APDUSIZE];//���ݻ�����
}TAPdu;//Ӧ�ò�1��pdu�Ľṹ

typedef struct
{
	bool	fValid;	//��Ч��־
	BYTE	bSvrNo; //��������
	BYTE	bPduNum;//Pdu��Ŀ
	BYTE	bStart; //����ָ��
	TAPdu	APdu[MAXAPDUNUM];//���ݻ�����
}TAPduPool;//Ӧ�ò�1�������

typedef struct
{
	bool	fValid;	//��Ч��־
	WORD 	wLen;//���ݻ��泤��
	BYTE	bBuf[APDUSIZE];//���ݻ�����
}TInsertAPduPool;//Ӧ�ò�1�������

//����������˽ṹ��������¼����cmd/cmdno/attri/mothod֮�������
//��������������������Ӧ�ò�����
typedef struct
{
	bool fValid;//��Ч��־
	bool fFinal;//�Ƿ������һ�������裬iStep=-1�ظ�
	WORD wLen;  //���ݻ��泤��
	BYTE bBuf[MAXDATASIZE];//���ݻ�����
}TDataPool;//Ӧ�÷�������ݳ�


typedef struct {
	WORD wOI;
	BYTE bAttr;
	BYTE bIndex;
	//������������
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
	NullSecurity = 0,	//��������
	PasswordSecurity,	//һ������
	SymmetrySecurity,	//�ԳƼ���
	SignatureSecurity,	//����ǩ��
}TConnectMechanismType;

typedef struct
{
	BYTE bMechanismType;
	BYTE bOsCiphertext[128];
	BYTE bOsSignature[128];
}TConnMechanismInfo;

typedef struct
{
	BYTE bProtoVer[2];			//Э��汾��
	BYTE bProConformance[8];	//Э��һ���Կ�
	BYTE bFunConformance[16];	//����һ���Կ�
	WORD wSenFrmMaxLen;			//����֡���ߴ�
	WORD wRcvFrmMaxLen;			//����֡���ߴ�
	BYTE bRcvWindows;			//����֡��󴰿ڳߴ�
	WORD wHandleApduSize;		//�ɴ������APDU�ߴ�
	DWORD dwConnectTimeOut;		//Ӧ�����ӳ�ʱʱ��
	//TConnMechanismInfo tConnMechInfo;//�Ự��ԿЭ��
}TConnectPara;	//�������ӵĲ���

//���������̰汾��Ϣ
typedef struct
{
	BYTE bFacCode[4];		//���̴���
	BYTE bSoftVer[4];		//����汾
	BYTE bSoftVerDate[6];	//����汾����
	BYTE bHardVer[4];		//Ӳ���汾��
	BYTE bHardVerDate[6];	//Ӳ���汾����
	BYTE bFacInfo[8];		//������չ��Ϣ
}TFacVersion;

//connect �ͻ���
typedef struct
{
	TConnectPara	tConnPara;
	BYTE			bMechanismInfo[2];	//Э��λ˵����ʵ��֡˵��
}TRxCliPara;

typedef struct
{
	TFacVersion		tTermnInfo;
	TConnectPara	tConnPara;
}TTxSvrPara;

typedef struct
{
//���յ�һ֡��ȡ������Ϣ
	BYTE bFrmHeaderLen;	//֡ͷ����
	WORD wRFrmLen;		//����֡ȫ��
	BYTE bFunCode;		//����֡�Ĺ�����
	BYTE bAddrType;		//���յ�ַ����
	BYTE bCliAddr;		//�ͻ�����ַ
	BYTE bSvrAddLen;	//��������ַ����
	BYTE bSvrAddr[16];	//��ַTSA,oct-string,���16�ֽ�
	BYTE bAPDUoffset;	//APDU ƫ��
	WORD wAPDULen;		//APDU �ĳ��ȣ������ֱ֡�ӿ���

//���տͻ��˲���
	BYTE bCliAddrLen;		//�ͻ��˵�ַ����
	BYTE bCliAddrs[16];		//�ͻ��˵�ַ

//��֡����
	bool fIsSegSend;	//��֡��־λ
	WORD wRcvFrmSegNo;	//����֡��֡���
	WORD wSendFrmSegNo;	//����֡��֡��� 
	BYTE FrmSegType;	//��֡����
//����Э��
	TRxCliPara		tRxTrsPara;//���յ���·Э�̲���
	TTxSvrPara		tTxTrsPara;//���͵���·Э�̲���

	BYTE bCommStep;
	//2009-04-30 ����2009���б꼼����������
	BYTE m_bWaitMin;			//��·�ȴ���ʱ
	BYTE m_bLinkDetectCount;	//��·̽�����������	
}TLnkLayerComm;//��·����Ϣ��


typedef struct
{
	WORD wClass;//��
	BYTE bObis[6];//Obisֵ
	BYTE bAttri;//����

	BYTE bfAccess;//�Ƿ�Ϊѡ���Է���
	BYTE bSelectBuf[100];//ѡ���Է��ʵ�����

	BYTE bResult;//���ʵĽ��
}TCosemAttriDesc;//COSEM����������

typedef struct
{
	WORD wClass;//��
	BYTE bObis[6];//Obisֵ
	BYTE bMethod;//����

	BYTE bfOptPara;//�Ƿ�Ϊ����������
	BYTE *pbOptPara;//ָ����������������
	WORD wOptLen;//�����ĳ���

	BYTE bResult;//���������Ľ��
}TCosemMethodDesc;//COSEM����������

typedef struct
{
	WORD wOI;
	BYTE bAttr;
	BYTE bIndex;
	BYTE bResult;//���ʵĽ��
}TOIAccessDesc;

typedef struct 
{
	BYTE bOAD[4];		//��������������
	BYTE bRSDLen;
	BYTE bRSD[128];		//��¼ѡ��������
	BYTE bRCSDLen;
	BYTE bRCSD[256];	//��¼��ѡ��������
}TOIRecordDesc;	//��¼�����ݷ���

typedef struct
{
	WORD wOI;			//����ID
	BYTE bMethod;		//����
	BYTE bOpMode;		//ģʽ
	DWORD dwActRdOAD;	//���ú��ȡ��OAD
	BYTE bRdDelay;		//���ú��ȡOAD��ʱ��
	DWORD dwRdClick;	//���õ�ʱ��---���ڼ����ӳٶ�ȡʱ��

	bool fOptPara;//�Ƿ�Ϊ����������
	BYTE *pbOptPara;//ָ����������������
	int wOptLen;//�����ĳ���

	BYTE bResult;//���������Ľ��
}TOIMethodDesc;//OI����������

typedef struct
{
	//1.�����ٻ����������Ϣ��¼
	//1.1����
	BYTE bCmd;		//��¼ÿ1��֡�ķ���ֵaarq/get/set/action
	BYTE bCmdMod;	//��¼ÿ1��֡��ģʽֵ
	BYTE bPIID;		//��¼ÿ1��֡��Invoke-Id-And-Priorityֵ

	bool fNewServer;
	BYTE bServer;	//��¼һ�η���aarq/get/set/action,����һ������ķ���ֵ
	BYTE bServerMod;	//��¼һ�η���aarq/get/set/action��ģʽ,����һ������ģʽ��ֵ
	bool fMoreBlk;	//����bServer+bServerMod�б��Ƿ��ж��
	WORD wBlkNo;	//��ǰ���
	int iStep;	//�����⺯����Ҫ�Ĳ���,�����ֲ���ȡ�ظ����ݵ�
	int iTabIdx;	//�����ɼ���������

	//��Э�������
	BYTE bAskBuf[1024];	//Э���RSD\RCSD\OAD������
	BYTE *pbAskStart;
	BYTE *pbAskMid;
	BYTE *pbAskEnd;

	//1.2����
	BYTE bAskItemNum;	//�������������
	//1.3 ��¼
	BYTE bAskRecItemNum;	//�����¼������
	//1.4����
	BYTE bAskMethodNum;	//����ķ�������
	//1.5��������Ҫ��
	BYTE bAnsServer;	//��¼һ�η�����Ӧ�ķ���ֵ
	BYTE bAnsServerMod;	//��¼һ�η�����Ӧ����ģʽ��ֵ
	BYTE bAnsCmdMod;	//��¼ÿһ����Ӧ����ģʽ��ֵ
	bool fAnsMoreBlk;	//������Ӧ�����б��Ƿ��ж��
	DWORD dwAnsBlkNo;	//��Ӧ���ݵ�ǰ���

	//2.���������ϱ���������Ϣ��¼
	BYTE bEvtWPtr,bEvtRPtr;//�¼������ϱ��Ķ�дָ��

	BYTE bConnectSta;	
	BYTE bMyPIID;		//��¼�����ϱ���ÿ1��֡��Invoke-Id-And-Priorityֵ
}TAppLayerComm;//Ӧ�ò���Ϣ��


typedef struct
{
	bool fSecurityLayer; //�Ƿ������ȫ����Э���
	BYTE bAppDataUnit; // Ӧ�����ݵ�Ԫ
	BYTE bDataAuthInfo; // ������֤��Ϣ
	BYTE bRn[32]; // �����
	DWORD dwRnLen; // ���������
	BYTE bMac[4];
	BYTE bSID[4];
	BYTE bErrInfo;
} TSecurityParam; //��ȫ����

typedef struct
{
	BYTE bRn[32]; // �����
	BYTE bMac[4]; 
} TRptSecureParam; //�����ϱ���ȫ����

typedef enum
{
	SecureData_Plaintext = 0,	//����Ӧ�����ݵ�Ԫ
	SecureData_Ciphertext,		//����Ӧ�����ݵ�Ԫ
	SecureData_ErrorDAR,		//�쳣����
}TSecureDataUnitType;	//Ӧ�����ݵ�Ԫ  CHOICE

typedef enum
{
	AuthType_SIDMAC = 0,	//������֤��      [0]  SID_MAC��
	AuthType_RN,			//�����          [1]  RN��
	AuthType_RNMAC,			//�����+����MAC  [2]  RN_MAC��
	AuthType_SID,			//��ȫ��ʶ        [3]  SID
}TSecureAuthType;	//������֤��Ϣ  CHOICE

//������֤��      [0]  SID_MAC��
//�����          [1]  RN��
//�����+����MAC  [2]  RN_MAC��
//��ȫ��ʶ        [3]  SID




#define MAXATTRI				30	//֧�����������Ŀ
#define MAXMETHOD				30	//֧����󷽷���Ŀ

typedef struct
{
	bool fDedicatedKeyOpt;//�����м��ܵ�����

	bool fRespAllowed;//�Ƿ���Ҫ��Ӧ

	bool fProposedQualServ;//�Ƿ����Ƽ��ķ�������ֵ��һ��
	BYTE bProposedQualServ;//�Ƽ��ķ�������ֵ

	BYTE bProposedDlmsVer;//�Ƽ�ʹ�õİ汾
	
	BYTE bProposedConfirmance[3];//�Ƽ�ʹ�õ�һ���Կ�

	WORD wClientMaxPduSize;//�ͻ�����pdu��С
}TPdu_IniRequest;//Ӧ�ó�ʼ������

typedef struct
{
	bool fNegotiatedQualServ;//�Ƿ���Э�̵ķ�������ֵ��һ��
	BYTE bNegotiatedQualServ;//Э�̵ķ�������ֵ

	BYTE bNegotiatedDlmsVer;//Э��ʹ�õİ汾

	BYTE bProposedConfirmance[3];//Э��ʹ�õ�һ���Կ�

	WORD wServerMaxPduSize;//����������pdu��С

	WORD wVaaName;//������
}TPdu_IniResponse;//Ӧ�ó�ʼ��������Ӧ

#define		RPTMAXFRM_EVERYSEND		30		//����һ��AutoSend()��෢�͵�֡��
#define		FRM_QUE_MAX				5	//�����¼���Ϣ��֡��	
class CFaProto: public CProto
{
public:
    CFaProto();
    ~CFaProto();

	int LPduSegFrmHandle();
	bool CombinApduFrm(TLPduPool RxLpduPool, TDataPool* ptRxApduPool);
	bool HandleFrm();//Э�鴦��ӿ�
	virtual void DoProRelated();	//��һЩЭ����صķǱ�׼������
////////////////////////////LINK layer ////////////////////////////////////////////
	int  RcvBlock(BYTE *pbBuf,int wLen);//�����Ч֡

	CQueue 	m_TrsQueue;  //ת���ı�����Ϣ����
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

	//�ȶ���ӿڣ����������Ӿ���Ĺ��� 20161026  CL
	int SetTimeFlg(){return 0;};
	int GetTimeFlg(){return 0;};
	int SetRptFlg(){return 0;};	
	int GetRptFlg(){return 0;};

	//Э���APDU����
	//int ProNormalToApduInfo(BYTE *pbApdu, TApduInfo *pApduInfo);
	int ProRecordToApduInfo(BYTE *pbApdu, TApduInfo *pApduInfo);
private:
	bool AddrCheck();
	bool Link_Request(BYTE *pApdu, WORD wApduLen);
	bool Link_Responce(BYTE *pApdu, WORD wApduLen);
	void Connect_response(BYTE* pApdu);

	BYTE m_bOffsetAPDU;

	TLnkLayerComm m_LnkComm;//��·����Ϣ��¼��
	TLPduPool	m_RxLPduPool;//To APP
	TLPduPool	m_TxLPduPool;//From APP

	BYTE	m_bRxBuf[MAXFRMSIZE];//����֡�Ļ�����
	BYTE	m_nRxStep;
	WORD	m_wRxPtr,m_nRxCnt;

	BYTE	m_bTxBuf[MAXFRMSIZE];//����֡�Ļ�����
	WORD	m_wTxPtr;
	BYTE	m_bAutoBuf[MAXFRMSIZE];//�����ϱ�����֡�Ļ�����
	WORD	m_wAutoPtr;

	DWORD	m_dwProRxClick;
	DWORD   m_dwProTxClick;
	BYTE    m_bNoComuSta;
	DWORD	m_dwClickFrmOk;
	DWORD	m_dwBeatClick;

	bool    m_fPwrOnRun;
	BYTE	m_bBeatNum;	//��������
	BYTE	m_bValidNum;	//m_iK[]�������Ч����
	int		m_iK[PRE_ADJ_MAX_CNT];

	WORD	m_wCurTaskId;
	WORD	m_wLastTaskId;
	int		m_iStart;
	int		m_iRdPn;	//���ζ��Ĳ�����	
	int		m_iRsd10Pn;
private:
	TNegoSizeDef  m_NegoSizeDef;//�ռ�Э�̲�����
	void InitCommSize(BYTE bType);
	void CheckMNegoPara();

private:
	//����Ϊ��·����մ�����
	int  VeryFrm();//��·����ռ���

	//����Ϊ��·�㷢�ʹ�����
	int Tx_RegisterBeat();
	int Tx_PriorFrm(bool fFinal);

	int MakeFrm(BYTE *pbBuf, WORD wLen);
	int MakeAutoFrm(bool fFinal);//��·�������ϱ�����֡��ͳһ���ڣ����֡ͷ֡β��У���룬ʵ��������

private:
	//����Ϊ��·����غ���
	void Tx_NegotiatoryPara();//��·�����Э�̲�����֡
	void ResetLnkPara();//������·��λ���е���·����
	void ClearVar(BYTE bConcentSta);//�������
	WORD GetTxHeaderLen(){ return 11; };//�������Թ̶�
	BYTE NextNSeq(BYTE bN){ return (bN+1)&0x7; };//�շ���������һ����Чֵ
	void ClsLPduPool(TLPduPool *p);//����·���PDU�����

	int ToSecurityLayer();//�ṩ��Ӧ�ò���õĽӿ�,�Ⱦ���ȫ�㣬�ٵ�����·��
	int ToLnkLayer();//�ṩ��Ӧ�ò���õĽӿ�
	int DoLPdu();//����һ��LPdu����֡�ͷ���
	int DoInsertLPdu();//����һ�������LPdu����֡�ͷ��ͣ���ʱLPdu���ڲ����APdu��

	void SetWaitTime(BYTE bWaitMin);		//��¼��֡�ȴ���ʱ
	void StartWaitTimer(void);
	void ResetWaitTimer(void);

	int GetErrOfGet(int iRetVal);
	int GetErrOfSet(int iRetVal);
	int GetErrOfAct(int iRetVal);
////////////////////////////APP layer ////////////////////////////////////////////
private:
	TAppLayerComm m_AppComm;//Ӧ�ò���Ϣ��¼��
	TSecurityParam m_SecurityParam;//���ܲ���Ϣ
	TRptSecureParam m_RptSecureParam; //�����ϱ��õļ��ܲ���Ϣ
	TAPduPool	m_RxAPduPool;//To APP data manage
	TAPduPool	m_TxAPduPool;//From APP data manage	
	TAPdu		m_RxAPdu;		//���յ���APDU֡
	TAPdu		m_TxAPdu;		//698-45��ʱ��Ӧ�ò㿼��ÿ��һ���Խ�����֡�ְ���ȥ��������ֱ֡��ʹ��get-next����Ϳ�����
	TInsertAPduPool m_TxInsAPduPool;//����ʽ����֡��Ŀǰ���ڴ�����̴��󷵻أ���Ϊ��Ҫ������һ���������Ϣ��������������

	void ClsAPduPool(TAPduPool *p);//��Ӧ�ò��PDU�����
	void NewAppServer();//����һ���µķ����������Ԥ������
	int GetReq(BYTE *pbBuf,WORD wLen);//����GET��������
	int SetReq(BYTE *pbBuf,WORD wLen);//����SET��������
	int ActReq(BYTE *pbBuf,WORD wLen);//����ACT��������

	int GetResErr(BYTE bGetMod, BYTE bErr);//��GET����ĳ�����Ӧ����
//	int SetRes(BYTE bSetMod);//��SET�������Ӧ����
//	int ActRes(BYTE bActMod, DWORD bErr_BlkNo);//��ACT����ĳ�����Ӧ����

	int GetData();//ȡ���ݲ����ͷֿ�
	int GetListData();//ȡ�б����ݲ����ͷֿ�
	int SetData();//�趨���ݺͷֿ���ϴ���
	int SetListData();//�趨�б����ݺͷֿ���ϴ���
	int ActData();//���������ͷֿ鴦��
	int ActListData();//�б��������ͷֿ鴦��

	int DecodeChoice(BYTE *pbBuf);//�����ߵ�ѡ���Է�����Choice�ĵ�������

////////////////////////////APP manage ////////////////////////////////////////////
private:
	TDataPool	m_RxDPool;
	TDataPool	m_TxDPool;  
	void ClsDataPool(TDataPool *p){ memset(p,0,sizeof(TDataPool)); };//��Ӧ�ò�����ݻ����
	int GetEvent(BYTE *pb);//����ϱ��¼���Ӧ�ò���Ϣ��֡
	int AutoReport();//����ϱ��¼���һ֡����ͷ���

	bool GetEventWritePtr(BYTE& bWrPtr);
	BYTE GetEvtRptFlg();

////////////////////////////old function////////////////////////////////////////////
////////////////////////////old function////////////////////////////////////////////
public:    
	bool Init(TFaProPara* pFaProPara);

	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TFaProPara* pFaProPara))
	{		//ע��װ�طǸ�λ�����ĺ���
		m_pfnLoadUnrstPara = pfnUnrstPara;
	};

	//��Э�鹲���麯��	
	bool Login();
//	bool Beat(){ Tx_RegisterBeat(); return true; };
	void OnConnectOK();
	bool IsNeedAutoSend() { return false; }; //�Ƿ���Ҫ�����ϱ�
	void LoadUnrstPara(){ return; };
	DWORD BeatMinutes(){ return m_wBeatMinutes; };
	WORD GetCnType() { return m_wCnType;};
	
	void OnBroken();

public:
    //��Э�鹲�ñ���
	bool 		m_fConnected;
	WORD 		m_wRunCnt;
	bool 		m_fErrRst;		//ͨ�ŷ�������,��Ҫ��λ�ն˱�־
	WORD 		m_wCnType;
	WORD 		m_wMaxFrmBytes;   //������ֽ���
	BYTE 		m_bConnectTypefLocal; //���ӷ�ʽ
	
protected:
	bool (*m_pfnLoadUnrstPara)(TFaProPara* pFaProPara);
	TFaProPara* m_pFaProPara;
	//��Э�鹲�ñ���
	WORD 		m_wBeatMinutes;  //�������,��λ����
	BYTE 		m_bSftpBuf[1200];
	CSftp 		*m_pSftpClient;
	char 		m_szCmdLine[100];
//�����ϱ����֣�����+������
public:
	//CProMngRpt* m_pProMngRpt;
	//CProWaitQue m_WaitQue;			//���ͳ�ȥ��Ҫȷ�ϵ�֡����Ϊ��ȷ����
	//TAskMsg		m_AskQue;			//������������ʹ��
	CQueue m_Queue;     //Э���̵߳ı�����Ϣ����
	TFapRptMsg  m_pRptMsg[RPTWAITQUE_SIZE];	// ���ͳ�ȥ��Ҫȷ�ϵ�֡����Ϊ��ȷ����
	DWORD m_dwOldTick;
	BYTE		m_bRptMsgNum;	//���ͳ�ȥ��Ҫȷ�ϵ�֡����Ŀ
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
	BYTE GetMyPIID(){//��6λ��Ч0~63
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
	//�����㽭�����չЭ��֡����
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

