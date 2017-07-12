/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TermEvtTask.h
 * ժ    Ҫ�����ļ���Ҫʵ��DL/T 698.45 ���¼�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��10��
 *********************************************************************************************************/
#ifndef TERMEVTTASK_H
#define TERMEVTTASK_H 

#include "DbConst.h"
	
//�ط��汾
#define TERM_STD					0								//������׼�汾
#define TERM_ZJ						1								//�㽭�汾
#define TERM_VER					TERM_ZJ//TERM_ZJ							//�汾ѡ��,�㽭



//�¼���غ궨��
#define DEFAULT_DELAY_SEC			60								//Ĭ���ж���ʱ60��
#define SYSDB_ITEM_MAXSIZE			254								//ϵͳ����������󳤶�254
#define EVTREC_MAXSIZE				1024//512								//һ�ʼ�¼����󳤶�
#define MEMORY_EVTREC_SIZE			EVTREC_MAXSIZE					//��������ʿռ����󳤶�
#define EVT_ATTRTAB_LEN				(5*CAP_OAD_NUM+2)				//�������Ա���
#define EVT_ATRR_MAXLEN				50								//�¼�����������󳤶ȣ��������������Ա������IC24�ĵ�ǰֵ��¼��50���ֽڡ�
#define EVT_SRC_LEN					19								//�¼�����Դ��󳤶� 18
#define TERM_PRG_OAD_NUM			10								//��̶����б�OAD������,�᲻������?
#define TERM_PRG_LIST_LEN			(2+TERM_PRG_OAD_NUM*5)			//��̶����б����
#define EVT_CLR_OMD_NUM				10								//�¼������б�OMD������,�᲻������?
#define EVT_CLR_LIST_LEN			(2+EVT_CLR_OMD_NUM*5)			//�¼������б���
#define EVT_CLR_VALID				0x5A							//������Ч��־ֵ
#define EVT_CLR_ID					0x0b12							//�¼������ʶ���ݿ�ID����Ҫ��DbCfg.cpp�еĶ���ƥ��
#define EVT_TRIG_PARA_LEN			32								//�¼�������������
#define EVT_TRIG_ID					0x0b13							//�¼����������������ݿ�ID����Ҫ��DbCfg.cpp�еĶ���ƥ�䣬���ݴ��˳��ͬ��Σ�ǰEVT_SRC_LEN�ֽ�ΪIC7���¼�����Դ/IC24�¼����
#define CN_RPT_NUM					3								//����ϱ�ͨ������
#define CN_RPT_STATE_LEN 			9								//ͨ���ϱ�״̬struct�ĳ���
#define CN_RPT_TOTAL_LEN			(2+CN_RPT_NUM*CN_RPT_STATE_LEN)	//�¼��ϱ�״̬"array ͨ���ϱ�״̬"�����ܳ���
#define EVT_UPDATE_CYC				1								//�¼�����ˢ������2��
#define EVT_ADDOAD_MAXNUM			70								//0x3200�����¼�����֧�ֵ���������������֧�ֵ�OAD���������������¼�
#define EVT_ADDOAD_MAXLEN			(5*EVT_ADDOAD_MAXNUM+2)			//0x3200�����¼��������ݳ��� ����2�������¼��б�ֻ������= array OAD
#define EVT_ADDOI_MAXLEN			(3*EVT_ADDOAD_MAXNUM+2)			//0x3200�����¼��������ݳ��� ����3�����ϱ��¼������б�ֻ������= array OI
#define OI_BALANCEDAY				0x4116
#define OI_FLUX						0x2200
#define SCH_MTR_SAVE_LEN			PN_MASK_SIZE					//���֧�ֵ��ѱ�������8��ÿλ��Ӧ����һ��������š�
#define SCH_MTR_SAVE_REC_NUM		10								// 3111ÿһ���ѱ��¼������洢�������ѱ���
#define STEP_AREA_SAVE_REC_NUM		10								// 3112ÿһ��̨���ѱ��¼������洢�������ѱ���

#define ONE_SCH_MTR_RLT_LEN			56								//һ���ѱ�������

//�¼��ӿ���
#define IC24						24								//�����¼�����ӿ���		
#define IC7							7								//�¼�����ӿ���

//�����¼��ӿ������Ժ궨��
#define IC24_LOGICNAME				1								//�߼���			
#define IC24_ATTRTAB				2								//�����������Ա�	
#define IC24_CURRECNUM				3								//��ǰ��¼��		
#define IC24_MAXNUM					4								//����¼��	
#define IC24_PARA					5								//���ò���		
#define IC24_RECORDTAB1				6								//�¼���¼��1	
#define IC24_RECORDTAB2				7								//�¼���¼��2	
#define IC24_RECORDTAB3				8								//�¼���¼��3		
#define IC24_RECORDTAB4				9								//�¼���¼��4	
#define IC24_CURRECLIST				10								//��ǰֵ��¼��		
#define IC24_REPROTFLAG				11								//�ϱ���ʶ		
#define IC24_VALIDFLAG				12								//��Ч��ʶ		
#define IC24_VLOSSSTA				13								//ʧѹͳ��		

//�¼��ӿ������Ժ궨��
#define IC7_LOGICNAME				1								//�߼���			
#define IC7_RECORDTAB				2								//�¼���¼��		
#define IC7_ATTRTAB					3								//�����������Ա�	
#define IC7_CURRECNUM				4								//��ǰ��¼��		
#define IC7_MAXNUM					5								//����¼��		
#define IC7_PARA					6								//���ò���			
#define IC7_CURRECLIST				7								//��ǰֵ��¼��		
#define IC7_REPROTFLAG				8								//�ϱ���ʶ			
#define IC7_VALIDFLAG				9								//��Ч��ʶ			

//�¼��ӿ��෽��
#define EVT_RESET					1								//��λ
#define EVT_RUN						2								//ִ��
#define EVT_TRIG					3								//����һ�μ�¼
#define EVT_ADDATTR					4								//���һ���¼������������� 
#define EVT_DELATTR					5								//ɾ��һ���¼�������������


//�¼�OI
#define MTR_VLOSS					0x3000							//ʧѹ					1
#define MTR_VLESS					0x3001							//Ƿѹ					2
#define MTR_VOVER					0x3002							//��ѹ					3
#define MTR_VBREAK					0x3003							//����					4
#define MTR_ILOSS					0x3004							//ʧ��					5
#define MTR_IOVER					0x3005							//����					6
#define MTR_IBREAK					0x3006							//����					7
#define MTR_PREVERSE				0x3007							//��������				8
#define MTR_POVER					0x3008							//����					9
#define MTR_PDMDOVER				0x3009							//�����й���������		10
#define MTR_RPDMDOVER				0x300A							//�����й���������		11
#define MTR_QDMDOVER				0x300B							//�޹���������			12
#define MTR_PFUNDER					0x300C							//��������������		13
#define MTR_ALLVLOSS				0x300D							//ȫʧѹ				14
#define MTR_SUPLYPOWDOWN			0x300E							//������Դ����			��֧��
#define MTR_VDISORDER				0x300F							//��ѹ������			15
#define MTR_IDISORDER				0x3010							//����������			16
#define MTR_POWERDOWN				0x3011							//����					��֧��
#define MTR_PROGRAM					0x3012							//���					��֧��
#define MTR_MTRCLEAR				0x3013							//�������					17
#define MTR_DMDCLEAR				0x3014							//��������				18
#define MTR_EVTCLEAR				0x3015							//�¼�����				19
#define MTR_SETCLOCK				0x3016							//Уʱ					��֧��
#define MTR_DAYSTAGE				0x3017							//ʱ�α���			��֧��
#define MTR_TIMEZONE				0x3018							//ʱ������			��֧��
#define MTR_WEEKREST				0x3019							//�����ձ��			��֧��
#define MTR_ACOUNTDAY				0x301A							//�����ձ��			��֧��
#define MTR_OPENCOVER				0x301B							//����					��֧��
#define MTR_OPENTAILOVER			0x301C							//����ť��				��֧��
#define MTR_VUNBALANCE				0x301D							//��ѹ��ƽ��			20
#define MTR_IUNBALANCE 				0x301E							//������ƽ��			21
#define MTR_RELAYLAZHA				0x301F							//��բ					��֧��
#define MTR_RELAYHEZHA				0x3020							//��բ					��֧��
#define MTR_HOLIDAY					0x3021							//�ڼ��ձ��			��֧��
#define MTR_MIXDPEXP				0x3022							//�й���Ϸ�ʽ���		��֧��
#define MTR_MIXDQEXP				0x3023							//�޹���Ϸ�ʽ���		��֧��
#define MTR_TARIFFPRICE				0x3024							//���ʲ�������		��֧��
#define MTR_STAIRPRICE				0x3025							//���ݱ���			��֧��
#define MTR_UPDATEKEY				0x3026							//��Կ����				��֧��
#define MTR_CARDABNAORMAL			0x3027							//�쳣�忨				��֧��
#define MTR_PURCHASE				0x3028							//�����¼				��֧��
#define MTR_DECREASEPURSE			0x3029							//�˷Ѽ�¼				��֧��
#define MTR_MAGNTEITCINT			0x302A							//�㶨�ų����ż�¼		��֧��
#define MTR_SWITCHABNORMAL			0x302B							//���ɿ�������		��֧��
#define MTR_POWERABNORMAL			0x302C							//��Դ�쳣				��֧��
#define MTR_ISUNBALANCE				0x302D							//�������ز�ƽ��		22
#define MTR_CLKERR					0x302E							//ʱ�ӹ���				23
#define MTR_MTRCHIPERR				0x302F							//����оƬ����			24
#define MTR_MODULECHANGE			0x3030							//ͨ��ģ�����¼�		��֧��			OAD

#define TERM_INIT					0x3100							//�ն˳�ʼ���¼�		1
#define TERM_VERCHG					0x3101							//�ն˰汾����¼�		2
#define TERM_YXCHG					0x3104							//�ն�״̬����λ�¼�	3
#define TERM_POWOFF					0x3106							//�ն�ͣ/�ϵ��¼�		4			enum
#define TERM_DIGITOVER				0x3107							//�ն�ֱ��ģ����Խ�����¼�	��֧��	OAD
#define TERM_DIGITUNDER				0x3108							//�ն�ֱ��ģ����Խ�����¼�	��֧��	OAD
#define TERM_MSGAUTH				0x3109							//�ն���Ϣ��֤�����¼�	5
#define TERM_DEVICEERR				0x310A							//�豸���ϼ�¼			6			enum
#define TERM_FLUXOVER				0x3110							//��ͨ�����������¼�	7	
#define TERM_UNKNOWNMTR				0x3111							//����δ֪���ܱ��¼�	8
#define TERM_STEPAREA				0x3112							//��̨�����ܱ��¼�	9
#define TERM_CLOCKPRG				0x3114							//�ն˶�ʱ�¼�		10
#define TERM_YKCTRLBREAK			0x3115							//ң����բ��¼		11			OAD
#define TERM_EPOVER					0x3116							//�й��ܵ������Խ���¼���¼		12
#define TERM_OUTPUTSTACHG			0x3117							//�����·����״̬��λ�¼���¼		��֧��
#define TERM_TERMPRG				0x3118							//�ն˱�̼�¼			13
#define TERM_CURCIRC				0x3119							//�ն˵�����·�쳣�¼�	14			enum
#define TERM_ONLINESTACHG			0x311A							//���ܱ�����״̬�л��¼���֧��
#define TERM_MTRCLKPRG				0x311B							//�ն˶Ե��Уʱ��¼	15			TSA
#define TERM_POWCTRLBREAK			0x3200							//������բ��¼			16			OI
#define TERM_ELECTRLBREAK			0x3201							//�����բ��¼			17			OI
#define TERM_PURCHPARACHG			0x3202							//����������ü�¼		18			OI 				
#define TERM_ELECTRLALARM			0x3203							//��ظ澯�¼���¼		19			OI

#define EVT_TOTAL_NUM				0x48							//���϶�����¼������������¼�������¼���������������������¼�����Ҫ�ۼ�
#define INMTR_IC24EVT_NUM			0x0C							//�ڱ�IC24�¼�������IC24ʵ��ֻ��10��������0x300B�������壬���Ϊ0x0C
#define INMTR_EVT_NUM				0x31							//�ڱ��¼�����

#define PWR_OFF_RUN_CNT				8								//ͣ���¼�������ָ�ʱ���ֵ�ѭ������

//��ǰ�¼�״̬����pEvtCtrl->pEvtBase[bItem].bJudgeState
#define EVT_JS_FORCE_END			0								//ǿ�ƽ���״̬
#define EVT_JS_HP					1								//�¼���Ȼ����״̬
#define EVT_JS_END					2								//�¼���Ȼ����״̬


//�¼�״̬������
#define EVT_S_BF_HP					1								//����ǰ��ע���ϵ��ʼ�������ȫʧѹ�¼����»�ȡ���������״̬����������Ĭ��Ϊ0��������ǰ
#define EVT_S_AFT_HP				2								//������
#define EVT_S_BF_END				3								//����ǰ
#define EVT_S_AFT_END				4								//������

//�¼��ϱ�
#define EVT_STAGE_UNCARE			0								//����ע
#define EVT_STAGE_HP				1								//����
#define EVT_STAGE_END				2								//����
#define EVT_STAGE_TASK				3								//ȫ�¼��ɼ��ϱ�

typedef struct {
	BYTE bRela;						//�����������Ա�
	BYTE bCurRecNum; 				//��ǰ��¼��
	BYTE bMaxRecNum;				//����¼��
	BYTE bPara;						//���ò���
	BYTE bRecTabStart;				//�¼���¼����ʼ
	BYTE bCurVal;					//��ǰֵ��¼��
	BYTE bRepFlg;					//�ϱ���ʶ
	BYTE bValidFlg;					//��Ч��ʶ
} TEvtAttr; 						//�¼����ԵĶ��壬��������CLASS7��CLASS24�Ĳ���

typedef struct {
	bool fInitOk;					//��ʼ����ȷ
	bool fExcValid;					//�¼���Ч�������˲���
	BYTE bState;					//״̬��
	BYTE bJudgeState;				//��ǰ�¼�״̬	
	BYTE bMemType;					//��̬�ڴ��ʹ�����ͣ�ʹ��MEM_TYPE_*�Ķ���
	WORD wTrigEvtDelaySec;			//�����¼�����ʱʱ��
	DWORD dwEstClick;				//����ʱ��
	DWORD dwRecvClick;				//�ָ�ʱ��
	DWORD dwStaClick;				//ʱ��ͳ��Click
} TEvtBase;							//�¼�����

typedef struct TEvtCtrl{
	//���ó�Ա
	WORD wOI;						//�¼���OI
	BYTE bClass;					//�¼�����
	BYTE bItemNum;					//�������
	BYTE bDelaySec;					//�¼��ж���ʱʱ��	
	BYTE* pbFixField;				//void*
	WORD wFixFieldLen;				//void*�ĳ���
	BYTE* pbSrcFmt;					//�¼�����Դ���ݸ�ʽ
	WORD wSrcFmtLen;				//�¼�����Դ���ݸ�ʽ����
	const BYTE* pbDefCfg;			//�������Ա�Ĭ�����ò���
	WORD wDefCfgLen;				//�������Ա�Ĭ�����ò�������

	//���ݳ�Ա����
	TEvtBase* pEvtBase;				//�¼��Ļ�������
	void* pEvtPriv;					//����ĳ���¼���˽�����ݽṹ

	//�ӿں�������
	bool (*pfnInitEvt)(struct TEvtCtrl* pEvtCtrl);	//�¼���ʼ������
	int (*pfnJudgeEvt)(struct TEvtCtrl* pEvtCtrl);	//�¼��жϺ���
	bool (*pfnDoEvt)(struct TEvtCtrl* pEvtCtrl);	//�¼�ִ�к���
	DWORD dwLastClick;				//�¼�ʱ��
	DWORD dwNewClick;				//�¼�ʱ��
}  TTermEvtCtrl;					//�¼����ƽṹ


typedef struct{
	BYTE  bYear[2];
	BYTE  nMonth;
	BYTE  nDay;
	BYTE  nHour;
	BYTE  nMinute;
	BYTE  nSecond;
}TDTime;

typedef struct{
	DWORD	dwDmdVal;	
	TDTime	tTime;	
} TDmdVal;							//���������¼�˽�г�Ա

typedef struct{
	TDmdVal	tDmd[4];	
} TDmd;	

typedef struct{
	DWORD dwVLossStaClick;			//�����ۼ���ʱ��
} TVLoss;							//ʧѹ�¼�˽�г�Ա

typedef struct{
	TEvtBase tEvtBase;					
} TAllVLoss;						//ȫʧѹ�¼�˽�г�Ա��Ϊ�������

typedef struct{
	BYTE bOMD[EVT_CLR_LIST_LEN];	//array OMD������ʽ
} TEvtClr;							//�¼������¼�˽�г�Ա

typedef struct{
	bool fInit;
	BYTE bStaByte;
} TYXChgCtrl;						//ң�ű�λ�¼�˽�г�Ա


/*typedef struct{
	TEvtBase tEvtBase;					
}TPowOffBase;*/

typedef struct{
	bool fPowerOff;
	bool fOldPowerOff;
	WORD wRecvCnt;
	WORD wEstbCnt;
	bool fInit;
	BYTE bAttr;						//���Ա�־�����ڹ̶��ֶ�
	BYTE bEvtSrcEnum;				//�¼�����Դ�����ڹ̶��ֶ� enum{ͣ��(0)���ϵ�(1)}
	BYTE bRptFlag;					//ͣ�ϵ��¼��ϱ���־
	BYTE bStep;						//״̬��
	bool fIsUp;						//�ϵ�
	bool fIsUpRec;					//�ϵ��¼���Ҫ��¼���ϵ��¼
	bool fMtrOrTerm;
}TPowOff;							//�ն�ͣ/�ϵ��¼�

typedef struct{
	BYTE bEvtSrcEnum;				//�¼�����Դ�����ڹ̶��ֶ�
}TDeviceErr;						//�豸���ϼ�¼

typedef struct{
	BYTE bShMtrFlag[SCH_MTR_SAVE_LEN];//�Ѳ����ı��־
	BYTE bSaveFlag[SCH_MTR_SAVE_LEN];//�����¼��洢�ı��־
	BYTE bRunStep;					//���в���
}TUnKnMtr;							//����δ֪���ܱ��¼�

typedef struct{		
	BYTE bClock[8];					//Уʱǰʱ��date_time_s�����ڹ̶��ֶΣ��洢ʾ��: DT_DATE_TIME_S 07 E0 0B 0A 10 1E 03 , ����ɰ��������ʹ洢
}TAdjTermTime;						//����δ֪���ܱ��¼�

typedef struct{
	BYTE bShMtrFlag[SCH_MTR_SAVE_LEN];//�Ѳ����ı��־
	BYTE bSaveFlag[SCH_MTR_SAVE_LEN];//�����¼��洢�ı��־
	BYTE bRunStep;					//���в���
}TStepArea;							//��̨�����ܱ��¼� ע������TUnKnMtr�������壬���������Ҫ����������ʱ������

typedef struct{
	BYTE bEvtSrcOAD[5];				//�¼�����ԴOAD,���ڹ̶��ֶΣ��洢ʾ�� DT_OAD F2 05 02 01
	BYTE bArrayPow[74];				//�غ�2�����ܼ��鹦�� array long64, ��2 + 9*8 = 74�ֽڡ�
									//�洢��������-1.2345kW(0xffffffffffffCFC7)����1.2345kW(0x3039)ʾ��: 
									//DT_ARRAY 0x02 DT_LONG64 ff ff ff ff ff ff cf c7 DT_LONG64 00 00 00 00 00 00 30 39 DT_LONG64 00 00 00 00 00 00 00 00 DT_LONG64 00 00 00 00 00 00 00 00
									//				DT_LONG64 00 00 00 00 00 00 00 00 DT_LONG64 00 00 00 00 00 00 00 00 DT_LONG64 00 00 00 00 00 00 00 00 DT_LONG64 00 00 00 00 00 00 00 00
}TYKCtrl;							//ң����բ��¼

typedef struct{
	BYTE bCompEng[9];				//Խ��ʱ�Ա��ܼ����й��ܵ����� long64����λ��kWh�����㣺-4�����洢-1.2345kW/Ԫʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
	BYTE bReferEng[9];				//Խ��ʱ�����ܼ����й��ܵ����� long64����λ��kWh�����㣺-4�����洢-1.2345kW/Ԫʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
	BYTE bRelaErr[2];				//Խ��ʱ�Խ�����ƫ��ֵ integer����λ��%�����㣺0�����洢ʾ��: DT_INT 0A ��
	BYTE bAbsoErr[9];				//Խ��ʱ�Խ�޾���ƫ��ֵ long64����λ��kWh�����㣺-4�����洢-1.2345kW/Ԫʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
}TEpOver;							//�й��ܵ������Խ���¼���¼

typedef struct{
	BYTE bOAD[TERM_PRG_LIST_LEN];	//��̶����б�  array OAD
}TTermPrg;							//�ն˱�̼�¼

typedef struct{
	BYTE bEvtSrcEnum;				//�¼�����Դ�����ڹ̶��ֶ�
}TCurCirc;							//�ն˵�����·�쳣�¼�

typedef struct{
	BYTE bEvtSrcTSA[18];			//�¼�����ԴTSA�����ڹ̶��ֶΣ��洢ʾ��: DT_TSA 06 11 22 33 44 55 66 00 00 00 00 00 00 00 00 00 00 ������ɰ��������ʹ洢
	BYTE bClock[8];					//Уʱǰʱ��date_time_s�����ڹ̶��ֶΣ��洢ʾ��: DT_DATE_TIME_S 07 E0 0B 0A 10 1E 03 , ����ɰ��������ʹ洢
	BYTE bClkErr[2];				//ʱ�����integer����λ���룬�޻��㣩�����ڹ̶��ֶΣ��洢ʾ��: DT_INT 0A , ����ɰ��������ʹ洢
}TMtrClkPrg;						//�ն˶Ե��Уʱ��¼

typedef struct{
	BYTE bEvtSrcOI[3];				//�¼�����Դ OI, ���ڹ̶��ֶΣ��洢ʾ�� DT_OI 23 01
	BYTE bHpAfPow[9];				//�¼�������2���ӹ��� long64(��λ��W������-1),�洢-1234.5Wʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
	BYTE bCtrlOI[3];				//���ƶ���OI���洢ʾ�� DT_OI F2 05
	BYTE bBreakCnt[3];				//��բ�ִ�bit-string(SIZE(8))���洢ʾ��DT_BIT_STR 08 03
	BYTE bPowCtrlVal[9];			//���ض�ֵlong64����λ��kW������-4��,�洢-1.2345kWʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
	BYTE bHpBfPow[9];				//��բ����ǰ�ܼ��й�����long64����λ��kW������-4��,�洢-1.2345kWʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
}TPowCtrl;							//������բ��¼

typedef struct{
	BYTE bEvtSrcOI[3];				//�¼�����ԴOI�����ڹ̶��ֶΣ��洢ʾ�� DT_OI 81 07
	BYTE bCtrlOI[3];				//���ƶ���OI���洢ʾ�� DT_OI F2 05
	BYTE bBreakCnt[3];				//��բ�ִ�bit-string(SIZE(8))���洢ʾ��DT_BIT_STR 08 03
	BYTE bEleCtrlVal[9];			//��ض�ֵlong64����λ��kWh������-4�����洢-1.2345kWʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
	BYTE bHpEng[9];					//��բ����ʱ�ܼӵ�����long64����λ��kwh/Ԫ������-4�����洢-1.2345kW/Ԫʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
}TEleCtrl;							//�����բ��¼

typedef struct{
	BYTE bEvtSrcOI[3];				//�¼�����Դ OI, ���ڹ̶��ֶΣ��洢ʾ�� DT_OI 23 01
}TPurchParaChg;						//����������ü�¼

typedef struct{
	BYTE bEvtSrcOI[3];				//�¼�����ԴOI�����ڹ̶��ֶΣ��洢ʾ�� DT_OI 81 07
	BYTE bCtrlOI[3];				//���ƶ���OI���洢ʾ�� DT_OI F2 05
	BYTE bEleAlrCtrlVal[9];			//��ض�ֵlong64����λ��kWh������-4�����洢-1.2345kWʾ��:DT_LONG64 ff ff ff ff ff ff cf c7��
}TEleAlram;							//��ظ澯�¼���¼

typedef struct {
	DWORD dwOAD;					//�¼�OAD
									//�����¼������Ա�ʶ��ͬ����Ŀ����¼������������Ԫ����������ʶ
	WORD  wRecIdx;					//������еļ�¼����
	BYTE   bStage;					//�����׶Σ�0����ע��1������2����

	BYTE bSchNo;					//������
	WORD wIdex;						//RCSD����
	BYTE bRcsd[128];				//RCSD
	WORD wRcsdLen;					//RCSD����
} TEvtMsg;							//�¼��ϱ���Ϣ

extern TAdjTermTime g_AdjTermTime;
extern TEpOver g_EpOver;

void SetTermEvtOadDefCfg(struct TEvtCtrl* pEvtCtrl);

TTermEvtCtrl* GetTermEvtCtrl(WORD wOI);
const TEvtAttr* GetEvtAttr(TEvtCtrl* pEvtCtrl);
void GetOIAttrIndex(DWORD dwOAD, WORD* pwOI, BYTE* pbAttr, BYTE* pbIndex);
bool GetEvtFieldParser(struct TEvtCtrl* pEvtCtrl, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize);
DWORD GetEvtMainOadDataLen(DWORD dwOAD);
bool IsEvtBeforeOAD(DWORD dwOAD);
bool IsSpecialOAD(DWORD dwOAD);
bool IsOADNeedAcqData(DWORD dwOAD, BYTE bState);

bool InitTmpMem(struct TEvtCtrl* pEvtCtrl, TFieldParser* pDataFields);
bool InitEvt(struct TEvtCtrl* pEvtCtrl);
bool InitVLoss(struct TEvtCtrl* pEvtCtrl);
bool InitDmd(struct TEvtCtrl* pEvtCtrl);
bool InitAVLoss(struct TEvtCtrl* pEvtCtrl);
bool InitEvtClr(struct TEvtCtrl* pEvtCtrl);
void InitTermEvt();

int VLossJudge(struct TEvtCtrl* pEvtCtrl);
int VLessJudge(struct TEvtCtrl* pEvtCtrl);
int VOverJudge(struct TEvtCtrl* pEvtCtrl);
int VBreakJudge(struct TEvtCtrl* pEvtCtrl);
int ILossJudge(struct TEvtCtrl* pEvtCtrl);
int IOverJudge(struct TEvtCtrl* pEvtCtrl);
int IBreakJudge(struct TEvtCtrl* pEvtCtrl);
int PReverseJudge(struct TEvtCtrl* pEvtCtrl);
int POverJudge(struct TEvtCtrl* pEvtCtrl);
int PDmdOverJudge(struct TEvtCtrl* pEvtCtrl);
int RPDmdOverJudge(struct TEvtCtrl* pEvtCtrl);
int QDmdOverJudge(struct TEvtCtrl* pEvtCtrl);
int PfUnderJudge(struct TEvtCtrl* pEvtCtrl);
int AVLossJudge(struct TEvtCtrl* pEvtCtrl);
int DisOrderJudge(struct TEvtCtrl* pEvtCtrl);
int MtrClrJudge(struct TEvtCtrl* pEvtCtrl);
int DmdClrJudge(struct TEvtCtrl* pEvtCtrl);
int EvtClrJudge(struct TEvtCtrl* pEvtCtrl);
int VUnBalanceJudge(struct TEvtCtrl* pEvtCtrl);
int IUnBalanceJudge(struct TEvtCtrl* pEvtCtrl);
int TermErrJudge(struct TEvtCtrl* pEvtCtrl);

bool OoReadOad(DWORD dwOAD, BYTE* pbBuf, WORD wDataLen, WORD wBufSize);
bool MakeEvtSpecField(DWORD dwROAD, DWORD dwFieldOAD, BYTE* pbField, WORD wFieldLen, WORD wFieldSize);
int EvtGetRecData(DWORD dwROAD, BYTE* pRecBuf, WORD wBufSize);
void UpdateState(struct TEvtCtrl* pEvtCtrl);
void UpdateRecMem(struct TEvtCtrl* pEvtCtrl, BYTE bSaveType);
void UpdateItemMem(struct TEvtCtrl* pEvtCtrl, BYTE bSaveType);
bool UpdateEvtStaData(struct TEvtCtrl* pEvtCtrl);
void UpdateVLossPriv(struct TEvtCtrl* pEvtCtrl);
void UpdateDmdPriv(struct TEvtCtrl* pEvtCtrl);
void UpdateAVLossPriv(struct TEvtCtrl* pEvtCtrl);
bool SaveTermEvtRec(struct TEvtCtrl* pEvtCtrl);
bool DoNullEvt(struct TEvtCtrl* pEvtCtrl);
bool DoEvt(struct TEvtCtrl* pEvtCtrl);
bool DoVLoss(struct TEvtCtrl* pEvtCtrl);
bool DoDmd(struct TEvtCtrl* pEvtCtrl);
bool DoAVLoss(struct TEvtCtrl* pEvtCtrl);
void DoTermEvt();

bool InitYXEvtCtrl(struct TEvtCtrl* pEvtCtrl);
int DoYXChgJudge(struct TEvtCtrl* pEvtCtrl);

void ClearEvtStaData(struct TEvtCtrl* pEvtCtrl);
void ClearVLossPriv(struct TEvtCtrl* pEvtCtrl);
void ClearVAllLossPriv(struct TEvtCtrl* pEvtCtrl);
void ClearOneEvt(struct TEvtCtrl* pEvtCtrl);
void ClearTermEvt(struct TEvtCtrl* pEvtCtrl);
void DealSpecTrigerEvt(WORD wOI);	
void GetEvtClearOMD(WORD wOI, BYTE bMethod, BYTE bOpMode);
void GetTermPrgOAD(WORD wOI, BYTE bAttr, BYTE Index);

char* GetEvtRecFileName(DWORD dwROAD);
bool GetEvtRecFieldParser(DWORD dwROAD, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize);
int GetEvtRecord(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbRecBuf, WORD wBufSize);
void ReInitMrtEvtPara(DWORD dwOAD);

int DoTermEvtMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtMethod2(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtIC7Method3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtIC24Method3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoTermEvtMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

bool SendEvtMsg(DWORD dwCnOAD, DWORD dwEvtOAD,WORD wRecIdx, BYTE bStage, BYTE bSchNo=0, WORD wIdex=0, BYTE* pbRCSD=NULL, WORD wRcsdLen=0);
int GetEvtRec(TEvtMsg* pEvtMsg, BYTE* pbRecBuf, WORD wBufSize, BYTE bType);
bool UpdateEvtRptState(DWORD dwCnOAD, TEvtMsg* pEvtMsg, BYTE bRptState);


bool InitPowOff(struct TEvtCtrl* pEvtCtrl);
bool InitDeviceErr(struct TEvtCtrl* pEvtCtrl);
bool InitUnKnMtr(struct TEvtCtrl* pEvtCtrl);
bool InitStepArea(struct TEvtCtrl* pEvtCtrl);
bool InitYKCtrl(struct TEvtCtrl* pEvtCtrl);
bool InitEpOver(struct TEvtCtrl* pEvtCtrl);
bool InitTermPrg(struct TEvtCtrl* pEvtCtrl);
bool InitCurCirc(struct TEvtCtrl* pEvtCtrl);
bool InitMtrClkPrg(struct TEvtCtrl* pEvtCtrl);
bool InitPowCtrl(struct TEvtCtrl* pEvtCtrl);
bool InitEleCtrl(struct TEvtCtrl* pEvtCtrl);
bool InitPurchParaChg(struct TEvtCtrl* pEvtCtrl);
bool InitEleAlram(struct TEvtCtrl* pEvtCtrl);
int TermInitJudge(struct TEvtCtrl* pEvtCtrl);
int TermVerChgJudge(struct TEvtCtrl* pEvtCtrl);
int PowOffJudge(struct TEvtCtrl* pEvtCtrl);
int GsgQAuthJudge(struct TEvtCtrl* pEvtCtrl);
int DeviceErrJudge(struct TEvtCtrl* pEvtCtrl);
int FluxOverJudge(struct TEvtCtrl* pEvtCtrl);
bool IsNeedSaveShMtrEvt(BYTE *pShMtrFlag, BYTE *pSaveFlag);
int UnKnMtrJudge(struct TEvtCtrl* pEvtCtrl);
int StepAreaJudge(struct TEvtCtrl* pEvtCtrl);
int TermClockPrgJudge(struct TEvtCtrl* pEvtCtrl);
int YKCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl);
int EpOverJudge(struct TEvtCtrl* pEvtCtrl);
int TermPrgJudge(struct TEvtCtrl* pEvtCtrl);
int CurCircJudge(struct TEvtCtrl* pEvtCtrl);
int MtrClkPrgJudge(struct TEvtCtrl* pEvtCtrl);
int PowCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl);
int EleCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl);
int PurChParaChgJudge(struct TEvtCtrl* pEvtCtrl);
int EleCtrlAlarmJudge(struct TEvtCtrl* pEvtCtrl);
int OnInfoTrigerEvtJudge(struct TEvtCtrl* pEvtCtrl, BYTE bInfoType);

void AddEvtOad(DWORD dwOAD, bool fRptFlag);
void DelEvtOad(DWORD dwOAD, WORD wOI);

#endif //TERMEVTTASK_H

