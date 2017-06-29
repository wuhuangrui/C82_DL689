/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FaCfg.h
 * ժ    Ҫ�����ļ���Ҫ�����궨����Щ�ڸ����ն��в�һ���ĳ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��4��
 *********************************************************************************************************/
#ifndef FACFG_H
#define FACFG_H
#include "syscfg.h"

//�ط��汾ѡ��
//#define VER_698_JIBEI				1	//����
//#define VER_698_SHANDONG			1	//ɽ��


#define FA_TYPE_D82			1
#define FA_TYPE_C82			2
#define FA_TYPE_K32			3

//�ն�����ѡ��
//#define	FA_TYPE			FA_TYPE_K32
//#define	FA_TYPE			FA_TYPE_C82
#define	FA_TYPE			FA_TYPE_C82

//���������ԣ���ʽ�����汾��ȥ�� 20161027  CL
//#define GW_OOB_DEBUG_MTR_RATE_CUR_VOL	1

// #define GW_OOB_DEBUG_0x31050700	1
// #define GW_OOB_DEBUG_0x31060600	1
// #define GW_OOB_DEBUG_0x310C0600	1
// #define GW_OOB_DEBUG_0x310D0600	1
// #define GW_OOB_DEBUG_0x310E0600	1
// #define GW_OOB_DEBUG_0x310F0600	1
// #define GW_OOB_DEBUG_0x31050600  1

#define GW_OOB_DEBUG_0x40010201	1	//̨��������9�·ݱ��ַΪTSA,����OCT-STRING

#define GW_OOB_PROTO_UPDATA_20170406	1//20170406�������



//�������ֹ�������Զ��ģ�鿪�ػ�ʱ��
#define MODULE_FOR_GW	0
#define MODULE_FOR_NW	1

#define EN_SPECIAL_TRANSFORMER	1
#define EN_CONCENTRATOR			1

#define MTREXC_ADDR_TPYE_TSA	1	//�����¼��е���ַ����ΪTSA,�������Ƶ���ַ���͵ĺ궨��

#if FA_TYPE == FA_TYPE_D82
	#define FA_NAME		"CL790D82-45"
	//#define EN_CCT		1	//�Ƿ�����������
	//#define EN_CCT485		1	//�Ƿ�������485��
    #define EN_CTRL		1	//�Ƿ�������ƹ���
	//#define EN_VARCPS		1	//�Ƿ������޹�����(VAR compensator)����
#elif FA_TYPE == FA_TYPE_C82
	#define FA_NAME		"CL818C82"
	//#define EN_CCT			1	//�Ƿ�����������
	//#define EN_CCT485		1	//�Ƿ�������485��
	//#define EN_CTRL		1	//�Ƿ�������ƹ���
	//#define EN_VARCPS		1	//�Ƿ������޹�����(VAR compensator)����
#else
	#define FA_NAME		"CL818K32"
#endif

#define PRO_698		1
#define PRO_DLMS	1

#define DLMS_863_VER	1//863Э������汾���Կ������в��
//#define ENGLISH_DISP	1
#define ETH_RDMTR_INV	5	//ʹ���������˳�����

#define PN_S_PHASE_SURPROID	7
#define PN_3_PHASE_SURPROID	6

#ifndef SYS_WIN
#define EN_AC			1	//�Ƿ������ɹ���
#define EN_ETHTOGPRS   1   //   �Ƿ�������̫����GPRS�л�
#endif

//#define EN_INMTR		1	//�Ƿ������ڲ�DL645
//#define EN_ESAM			1	//�Ƿ�����ʹ�ü���ģ��

//#define EN_MTR_UNSUPID	1	//�Ƿ�֧�ֵ��֧��ID���ж�
#define EN_SBJC_V2   	   1		//�ı�������
#define EN_SBJC_V2_CVTEXTPRO		1

#define MAXPNMASK         0x7F

//�߼�����ڶ���
#define PORT_AC			1	//�ն˽�������ͨ�Žӿ�
#define PORT_GB485		2	//���긺��485�����
//#define PORT_CCT_485	3	//������485�����	
#define PORT_IN485		4	//�ڱ�485��
#define PORT_RJ45		5	//�ڱ�485��
#define PORT_CCT_PLC	31	//�������ز������߳����

#define MAX_CCT_485     4   //��༯��¼485��.
//Ӳ���������
#define YXNUM	2
#define YKNUM	5


//����������ص���Ŷ���
#define  DB_CRITICAL	   0	
#define  DB_DB			   1
#define  DB_LOADCTRL       2
#define  DB_FAPROTO        3
#define  DB_FAFRM     	   4	
#define  DB_POINT          5  
#define  DB_FA             6 
#define  DB_TASK           7
#define  DB_645            8
#define  DB_645FRM         9
#define  DB_OIIF           10
#define  DB_SYS            11
#define  DB_GLOBAL         12
#define  DB_ABB            13
#define  DB_FUJ			   14
#define  DB_EDMI		   15
#define  DB_FS  		   16


#define  DB_VBREAK         17   //��ѹ����
#define  DB_VMISS          18   //��ѹȱ��
#define  DB_POLAR          19   //����������
#define  DB_IOVER          20   //�����������
#define  DB_OVLOAD         21   //���ɹ���
#define  DB_OVDEC          22   //����ͬ�õ�
#define  DB_IUNBAL         23   //���ฺ�ɲ�ƽ��
#define  DB_METER_EXC      24   //����쳣��
#define  DB_OVCPS          25   //�޹�����Ƿ����
#define  DB_DIFF           26   //�
#define  DB_EXC1           27   //�쳣1
#define  DB_CPS            28   //�޹�����
#define  DB_EXC3           29   //�쳣3
#define  DB_EXC4           30   //�쳣4
#define  DB_EXC5           31   //�쳣5
#define  DB_LANDIS         32
#define  DB_OSTAR          33  //����
#define  DB_ZHEJ	       34
#define  DB_DLMS	       35	//��������
#define  DB_DLMS_RJ		   36	//863-DLMS ���糭��
#define  DB_DLMS_ERR	   37	//863-DLMS �����쳣
#define	 DB_DLMS_FlOW	   38
#define  DB_HND				DB_DLMS_RJ
#define  DB_WS		       DB_DLMS_ERR	//��ʢ��
#define  DB_COMPENSATE     DB_DLMS_FlOW   //�޹�����
#define  DB_TASKDB         39	//�������ݿ�
#define SOCKETS_DEBUG      	46  //��̫����GPRS�л�ģʽ
/*
#define PPP_DEBUG          	40
#define ETHARP_DEBUG       	41
#define NETIF_DEBUG        	42
#define PBUF_DEBUG         	43
#define API_LIB_DEBUG      	44
#define API_MSG_DEBUG      	45
#define SOCKETS_DEBUG      	46
#define ICMP_DEBUG         	47
#define INET_DEBUG         	48
#define IP_DEBUG           	49
#define IP_REASS_DEBUG      50
#define RAW_DEBUG           51
#define MEM_DEBUG           52
#define MEMP_DEBUG          53
#define SYS_DEBUG           54
#define TCP_DEBUG           55
#define TCP_INPUT_DEBUG     56
#define TCP_FR_DEBUG        57
#define TCP_RTO_DEBUG       58
#define TCP_REXMIT_DEBUG    59
#define TCP_CWND_DEBUG      60
#define TCP_WND_DEBUG       61
#define TCP_OUTPUT_DEBUG    62
#define TCP_RST_DEBUG       63
#define TCP_QLEN_DEBUG      64
#define UDP_DEBUG           65
#define TCPIP_DEBUG         66
#define PPP_DEBUG           67
#define SLIP_DEBUG          68
#define DHCP_DEBUG          69

#define	 LOG_CRITICAL    80
#define	 LOG_ERR         81
#define	 LOG_NOTICE      82
#define	 LOG_WARNING     83
#define	 LOG_INFO        84
#define	 LOG_DETAIL      85
#define	 LOG_DEBUG       86
*/
#define DB_METER		90

#define DB_HT3A			91		//��ͨ
#define DB_HL645		91		//��¡
#define DB_AH645		91		//����
#define DB_HB645		91		//����
#define DB_TJ645		91		//���
#define DB_DL645V07		91		//07��645Э��
#define DB_NMG645		91		//���ɹ�645��(��г��)
#define DB_BJ645		91		//����97��645��
#define DB_645_Q		8		//97��645���޹���
#define DB_MODBUS		91		//modbusЭ��
#define DB_SHCMD09		91

#ifdef EN_SBJC_V2
#define DB_Ext645		91		//�ı���չ645
#endif

#define DB_DP		 	92		//���ݴ���
#define DB_MSCHED	 	93		//������
#define DB_CCT	 		94		//����
#define DB_CCTRXFRM		95		//��������֡
#define DB_CCTTXFRM		96		//��������֡
#define	DB_MULTITEMP    97		//��ͨ���¶ȼ�Э��
#define DB_FAPROTO645	97		//�ڲ�645
#define	DB_MTRX   	 	98		//����߼���Ϣ
#define DB_CCT_EXC		99		//�����¼�


#define DB_DISPLAY		100		//��ʾ
#define DB_INMTR		101		//�ڱ�
#define DB_CCT_TEST     102     //��ʱ������ 
#define DB_RJ45_RD      103     //RJ45����
#define DB_RJ45_TASK    104     //RJTASK��������
#define DB_PRIME_RD     105     //Prime����
#define DB_DL69845		106		//698.45Э��
#define DB_ESAM			107		//ESAM

//�������ظ��Ķ���
#define DB_LANDLMS		DB_LANDIS		//������DLMS
#define	DB_1107			DB_LANDIS		//ɽ��A1700
#define DB_LANZMC		DB_LANDIS		//������ZMC��

//�������ظ��Ķ���
#define DB_ABB2		DB_ABB		//ABBԲ��
#define	DB_EMAIL	DB_ABB		//EMAIL���
#define DB_COMPENMTR	111		//��������
#define DB_CTRL			112     //���ƿ���
#ifdef EN_SBJC_V2_CVTEXTPRO	   
#define DB_CVTEXT		113      //�ӿ�ת������չ
#endif
#define DB_CCT_SCH		114		//�ѱ�
#define DB_AC			115		//������Ϣ

//�ط��汾���壬ʹ��ƴ��ȫ��
#define VER_STD          0   //��׼����������

//1-3 ���ø��ض��壬Ŀǰû��ʹ��
#define VER_GUANGDONG    1   //�㶫��
#define VER_JIANGXI      2   //������
#define VER_CHENGDE      3   //�е°�

//���¼�������4��ʼʹ��
#define VER_HEBEI        4   //�ӱ���
#define VER_JIBEI        5   //������
#endif //FACFG_H
