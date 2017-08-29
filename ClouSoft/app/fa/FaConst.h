#ifndef CONST2_H
#define CONST2_H
#include "FaCfg.h"
#include "DbConst.h"



#define  PWROFF_VER       0x0001




#define ERR_OK       0x0
#define ERR_FORWARD  0x1
#define ERR_INVALID  0x2    //�������ݷǷ�
#define ERR_PERM     0x3
#define ERR_ITEM     0x4	//���ݿⲻ֧�ֵ�������
#define ERR_TIME     0x5    //ʱ��ʧЧ

#define ERR_UNSUP	 0x06	//���֧�ֵ��������
#define ERR_FAIL	 0x07	//����ʧ��,���糭��ʧ�ܵ�

#define ERR_ADDR     0x11
#define ERR_SEND     0x12
#define ERR_SMS      0x13 
#define ERR_PNUNMAP	 0x14	//������δӳ��

#define ERR_TIMEOUT  0x20
#define ERR_SYS      0x30

#define EMPTY_DATA 	 0
#define INVALID_DATA 0xff



//Ӧ��֪ͨ:
//������ʱ֪ͨ��,��Ҫ2���Ӳ��ύ��֪ͨ��Ϣ�ŵ�ǰ��,
//����1���ӿ����ύ�Ͽ��ID�ŵ�INFO_LONG_DELAY_END�涨�ĺ����
#define INFO_NONE	   			0
#define INFO_APP_RST			1
#define INFO_SYS_RST   			2
#define INFO_WK_PARA			3	//GPRS�����̲߳�������
#define INFO_FAP_PARA			4	//��վͨ��Э���������
#define INFO_COMM_RST			5	//����վͨ����Ҫ��λ
#define INFO_COMM_RLD			6	//����վͨ����Ҫ��װ����

#define INFO_230M_PARA			7	//��̨��������
#define INFO_MTR_EXC_RESET		8	//�����¼���������
//����ʱ
#define INFO_ACTIVE	   			9	//���Ż������弤����Ϣ
#define INFO_METER_PARA			10   //����������
#define INFO_RST_PARA			11  //������λ
#define INFO_RST_DATA			12	//���ݸ�λ
#define	INFO_VER_CHANGE			13	//�汾���
#define INFO_AC_PARA			14	//���ɲ������
#define INFO_COMTASK_PARA		15	//��ͨ����������
#define INFO_STAT_PARA			16	//ͳ������������
#define INFO_YX_PARA 	   		17	//ң�Ų������
#define INFO_RST_TERM_STAT		18	//��λ�ն�ͳ������
#define INFO_PULSE	   			19	//�����������
#define	INFO_CTPT_PARA			20	//������CTPT�������
#define INFO_PLC_PARA	   		21   //�ز���������
#define INFO_PLC_CLRRT	   		22	//��·��
#define INFO_PLC_STOPRD			23	 //ֹͣ����
#define INFO_PLC_RESUMERD		24	 //�ָ�����	
#define INFO_ADJTIME_PLC   		25	//���ز�����Уʱ
#define INFO_ADJTIME_485   		26	//��485����Уʱ
#define INFO_VIPLIST	   		27 	//�ص㻧����
#define INFO_GRP_PARA	   		28	//�ܼ���������
#define INFO_READ_DONE	   		29	//�������
#define INFO_DC_SAMPLE			30
#define INFO_LINK	   			31	//������������


#define INFO_DISCONNECT			32		//DISCONNECT
#define INFO_CTRL				35		//�����߳�
#define INFO_ENTERDOMAIN		36		//��λʱͨ�Ž���domain

#define INFO_HAND_RESET			37	//�ֶ���λ
#define INFO_OTHR645PARA_CHANGE	38	//����645�����ı�
#define INFO_CUR_PARA	   		39	//645���߲�������									
#define INFO_CLR_ENERGY         40	//�����645����
#define INFO_CLR_DEMAND         41	//������
#define INFO_CLR_645EVENT       42	//��645�¼�����
#define INFO_CLR_ALL			43	//����������
#define INFO_RST_ALLPARA		44  //������в�������
#define INFO_COMM_ADDR			45	//�ն˵�ַ���޸�
#define INFO_COMM_APN			46  //ģ��APN���޸�
#define INFO_FC_NETWORK			47  //��Ѷ������
#define INFO_PLC_MTRSCHCMD		48		//Plc������������
#define INFO_PLC_STATUPDATA		49		//Plcͳ�Ƹ���
#define INFO_PLC_RDALLRTINFO 	50  //��ȡ���нڵ���м�·����Ϣ
#define INFO_PORTSCH_PARA		51  //�����˿ںſ��Ʋ���
#define INFO_USB_COPY 		    52  //USB����
#define INFO_USB_COPYEND 		53  //USB�������
#define INFO_PLC_UPDATE_ROUTE   54  //�ز�·��������
#define INFO_RADIO_PARA         55  //���߲������
#define INFO_RADIO_CHANNEL      56  //�л��ŵ�
#define INFO_PLC_WIRLESS_CHANGE 57  //�����ŵ����
#define INFO_TIME_SET			58  //������ʱ�䱻����
#define INFO_RJMTR_PARACHG		59	//RJ45���������޸�
#define INFO_PRIMTR_PARACHG		60	//RJ45���������޸�
#define INFO_FRZPARA_CHG			61	//����������
#define INFO_HARDWARE_INIT		62
#define INFO_MTR_UPDATE			63	//�����������ȫ�����
#define INFO_TASK_CFG_UPDATE	64	//��������������õ�Ԫ����
#define INFO_ACQ_SCH_UPDATE		65	//�������ɼ���������
#define INFO_EVT_RESET				66	//��������¼���λ���������¼�
#define INFO_EVT_MTRCLR			67	//�������������
#define INFO_EVT_DMDCLR			68	//���������������
#define INFO_EVT_EVTCLR				69	//��������¼�����
#define INFO_FRZDATA_RESET			70	//�������ݸ�λ
#define INFO_SYNC_MTR				71	//�ز�����ͬ��
#define INFO_SCH_MTR				72	//���������ѱ�

#define INFO_CLASS19_METHOD_RST				73	//�豸�ӿ���19--��λ
#define INFO_CLASS19_METHOD_EXE				74	//�豸�ӿ���19--ִ��
#define INFO_CLASS19_METHOD_DATA_INIT		75	//�豸�ӿ���19--���ݳ�ʼ��
#define INFO_CLASS19_METHOD_RST_FACT_PARA	76	//�豸�ӿ���19--�ָ���������
#define INFO_CLASS19_METHOD_EVT_INIT		78	//�豸�ӿ���19--�¼���ʼ��
#define INFO_CLASS19_METHOD_DEM_INIT		79	//�豸�ӿ���19--������ʼ��

#define INFO_PULSEDATA_RESET		80	//�������ݸ�λ

#define INFO_GPRS_OFFLINE					81
#define INFO_COMM_TERMIP					82

#define INFO_COMM_GPRS_RLD					83
#define INFO_COMM_ETH_RLD					84
#define INFO_RS232_PARACHG					85	//232�˿ڲ���
#define INFO_RS485_PARACHG					86	//485�˿ڲ���
#define INFO_INFRA_PARACHG					87	//����˿ڲ���
#define INFO_RELAY_PARACHG					88	//�̵����������
#define INFO_MULPORT_PARACHG				89	//�๦�ܶ��Ӳ���
#define INFO_PLC_TRANS_PARACHG				90	//�ز�͸��ת������
#define INFO_PLC_PARACHG					91	//�ز��˿ڲ���
#define INFO_CLASS14_STAT_CHG				92	//����ͳ�ƽӿ�
#define INFO_CLASS15_STAT_CHG				93	//�ۼ�ƽ���ӿ�
#define INFO_CLASS16_STAT_CHG				94	//��ֵ���߽ӿ�
#define INFO_TZ_DC_PARACHG					95	//ʱ��ʱ�η��ʲ���
#define INFO_TERM_PROG						96	//�ն˱�̼�¼
#define INFO_POWERCTRL_REC					97	//������բ��¼
#define INFO_ENERGYCTRL_REC					98	//�����բ��¼
#define INFO_ENERGYCTRL_ALARM				99	//��ظ澯
#define INFO_ENERGYBUY_PARACHG				100	//�����������
#define INFO_DEVICE_485_ERR					101	//�豸485���ϼ�¼
#define INFO_TERM_INIT						102	//�ն˳�ʼ��
#define INFO_TERM_VER_CHG					103	//�ն˰汾���
#define INFO_ADJ_TERM_TIME					104	//�ն˶�ʱ
#define INFO_ESAM_AUTH_FAIL					105	//�ն���Ϣ��֤ʧ��
#define INFO_YK_REC							106	//ң����բ��¼
#define INFO_RP_SCH_UPDATE					107	//�ϱ���������
#define INFO_EVT_EVTRESET					108	//��������¼���λ
#define INFO_EVT_CLREVTRESET				109	//��������¼������¼���λ
#define INFO_DEVICE_CCT_ERR					110	//�豸�ز�ͨ������
#define INFO_TERM_MTRCLKPRG					111	//�ն˶Ե��ܱ�Уʱ
#define INFO_SYNC_T188PARA					112 //ͬ��ˮ���ȱ�����Э��ת����
#define INFO_START_485_SCH_MTR				113	//485�����ѱ�
#define INFO_STOP_485_SCH_MTR				114	//485ֹͣ�ѱ�
#define INFO_ONE_BRAODCAST_ARG_CCT			115	//����ַ�㲥Уʱ��������
#define INFO_ONE_BRAODCAST_ARG_485			116	//����ַ�㲥Уʱ��������
#define INFO_PLC_MOD_CHANGED				117	//�ز�ģ�����
#define INFO_MTR_BRAODCAST_ARG_CCT			118	//�㲥Уʱ��������
#define INFO_MTR_BRAODCAST_ARG_485			119	//�㲥Уʱ��������


#define INFO_AUTODETECT_START				120
#define INFO_AUTODETECT_STOP				121
#define INFO_STOP_FEED_WDG					122 //��ֹι����Ϣ�����Ź�����������

#define INFO_MTR_INFO_UPDATE				123	//���������
#define INFO_TASK_CFG_DEL					124	//��������������õ�Ԫɾ��
#define INFO_ACQ_SCH_DEL					125	//�������ɼ�����ɾ��
#define INFO_PWROFF							126 //ͣ����Ϣ

#define INFO_LOCAL_SEND_LED					127	//���ط��͵�
#define INFO_LOCAL_RECV_LED					128	//���ؽ��յ�
#define INFO_GPRS_SEND_LED					129 //GPRS���͵�
#define INFO_GPRS_RECV_LED					130	//GRPS���յ�

#define INFO_MTR_ALL_CLEAR					131	//���������

#define INFO_UPD_MTR_CTRL					132	//���³�����ƽṹ
#define INFO_MTR_EXC_MEM					133	//�����¼���̬�ڴ����
#define INFO_END	   						134//����Ϣ,��Ϊ������Ϣ�Ľ���
									//�ѱ�֪ͨ�㶨��Ϊ���һ��

#define INFO_NUM	   	    		(INFO_END+1)
#define INFO_SHORT_DELAY_START	 	INFO_230M_PARA
#define INFO_NO_DELAY_START	 		INFO_ACTIVE

//��Ϣ����ʱ�����ʱ�Ķ���,��λ��
#define INFO_SHORT_DELAY	15
#define INFO_LONG_DELAY		30

#define DC_CHN_MAX		2		//ֱ��ģ��ͨ����

//�ط��汾����
#define LOCAL_GD      1
#define LOCAL_JX      2
#define LOCAL_HUABEI  3
#define LOCAL_ZJ  	  4

//���Э���ڲ�����
//863Э��Լ����01:97-645,02:07-645��03:62056
#define PROTOCOLNO_NULL			0
#define PROTOCOLNO_DLT645		1	//DL645
#define PROTOCOLNO_DLT645_V07	2//30	//2007��645��
#define PROTOCOLNO_DLT69845		3	//DL698.45
#ifdef EN_SBJC_V2
#define PROTOCOLNO_SBJC     	4	//ˮ���ȱ�
#endif
#define PROTOCOLNO_MAXNO		5	//���ĵ��Э��ţ�Ŀǰ������40

#ifdef EN_SBJC_V2
//ˮ���ȱ�Э���ڲ�����
#define PROTOCOLNO_STD_T188				0	//��׼T188(Ĭ�ϣ�1f 90)����½ˮ������T188(1f 90��90 1f)����ˮˮ������ˮ������ˮ�������ȱ�
#define PROTOCOLNO_F10CONFIG			1	//��������F10�����ʸ���������	��һ��������֧�ֶ���Э��ʱʹ�ã�
#define PROTOCOLNO_HUAXU_T188_MBUS		2	//����T188_mbus		��90 1f��
#define PROTOCOLNO_HUAXU_T188_RS485		3	//����T188_RS485	��90 1f��
#define PROTOCOLNO_ANSHAN_RS232			4	//��ɽRS232
#define PROTOCOLNO_MINSHENG_WIRELESS	5	//��������
#define PROTOCOLNO_JINGQI_645			6	//���645
#define PROTOCOLNO_ZHENGTAI_T188		7	//��̩T188	��1f 90��
#define PROTOCOLNO_DENENG_T188			8	//����T188	��90 1f��
#define PROTOCOLNO_BEILIN_T188			9	//����T188	��90 1f��
#define PROTOCOLNO_JSJD_MBUS			10	//�����пƾ���
#define PROTOCOLNO_JSLX_RS485			11	//��������
#endif

#define	CCT_MTRPRO_97	1	//97��645
#define	CCT_MTRPRO_07	2	//07��645
#define CCT_MTRPRO_T188   3   //T188Э��
#define CCT_MTRPRO_69845  4   //698.45

#define FLG_FORMAT_DISK   		0x34aebd24
#define FLG_DEFAULT_CFG   		0x8a5bc4be
#define FLG_REMOTE_DOWN   		0xbe7245cd
#define FLG_HARD_RST   	  		0x4ab8ce90
#define FLG_DISABLE_METEREXC    0xce7821bd
#define FLG_ENERGY_CLR    		0xee6ad23f
#define FLG_APP_RST             1


#define DM_MAX_FILESIZE   (200*1024)

#define VIP_MAX_NUM    	  10
#define AREA_VIP_MAX_NUM	1		//̨���ܱ���
#define INVALID_POINT     0
#define ALL_POINT		  0xffff	

#define METER_TYPE_VIP		0x80
#define METER_TYPE_TYPE		0x7F


//�ļ��ļ���ʶ
#define FILE_LOGFILE			0
#define FILE_TRANSFER_STYLE1	1
#define FILE_TRANSFER_STYLE2	2
#define FILE_TRANSFER_STYLE3	201	//FTP�����������

//�ļ�������ʽ����
#define FILE_OPERTOR_DOWN		1
#define FILE_OPERTOR_UP			2
#define FILE_OPERTOR_DELETE		3

#define GB_INVALID_VALUE		0xee

//�������־�ļ�ID
#define LOG_ENERGY		0		//���ɵ���
#define LOG_DEMAND		1		//��������
#define LOG_TERMSTAT	2		//�ն�ͳ����Ϣ
#define LOG_PULSE_ENERGY1	3		//��һ·�������
//#define LOG_PULSE_DEMAND1	5//11		//��һ·��������
#define LOG_ENERGY_BAR		7		//���ɵ��ܲ�����С������λ��
#define LOG_ENABLE	1 

#define INVALID_645_DATA		0xff
#define ERR_OVER_RECNUM			-20

//485�ڹ���
#define PORT_FUN_RDMTR		0		//�����
#define PORT_FUN_INMTR		1		//������
#define PORT_FUN_LINK		2		//������
#define PORT_FUN_VARCPS		3		//���޹�����װ��
#define PORT_FUN_ACQ		4		//�ɼ���
#define PORT_FUN_JC485		5		//����485��.
#define PORT_FUN_DEBUG		0xFF	//debug���(ֻ��3����Ч)

#define ALARM_LED_CTRL	0	//�����̵߳���
#define ALARM_LED_645	1	//645�¼��̵߳���

#define PROG_VALID_MIN		60		//��̿�����Чʱ�� (��λ:����)

#define TRANSMIT_TIME_OUT   50      //�ļ������е�ת����ʱʱ�� ��λs

//const static int g_iInSnToPhyPort[] = {COMM_METER, COMM_LINK, COMM_DEBUG};	

//������Ϣ
#define PARA_CHANGE			0	//���в�������
#define NEW_INFO			1	//������Ϣ
#define CTL_PERIOD_ENABLE	2	//ʱ�ο�Ͷ��
#define CTL_PERIOD_DISNABLE	3	//ʱ�οؽ��
#define CTL_REST_ENABLE		4	//���ݿ�Ͷ��
#define CTL_REST_DISNABLE	5	//���ݿؽ��
#define CTL_MONTH_ENABLE	6	//�µ��Ͷ��
#define CTL_MONTH_DISNABLE	7	//�µ�ؽ��
#define CTL_BUY_ENABLE		8	//�����Ͷ��
#define CTL_BUY_DISNABLE	9	//����ؽ��
#define CTL_TMP_ENABLE		10	//�¸���Ͷ��
#define CTL_TMP_DISNABLE	11	//�¸��ؽ��
#define PERMIT_CLOSE		12	//�����բ
#define OVERLOAD			13	//���������޵�
#define ENERGY_GET_LOW		14	//����������
#define ENERGY_GET_ZERO		15	//�������꣬����բ
#define ALR_MON_CTL			16	//�µ������Ƹ澯
#define CTL_SHUT_ENABLE		17	//Ӫҵ��ͣ��Ͷ��
#define CTL_SHUT_DISNABLE	18	//Ӫҵ��ͣ�ؽ��
#define CTL_URG_ENABLE		19	//�߷Ѹ澯Ͷ��
#define ALR_URG_CTL			20	//�߷Ѹ澯
#define CTL_URG_DISNABLE	21	//�߷Ѹ澯���
#define CTL_GUAR_ENABLE		22	//����Ͷ��
#define CTL_GUAR_DISNABLE	23	//������
#define YK_FIR_OPEN			24	//ң�ص�1����բ
#define YK_SEC_OPEN			25	//ң�ص�2����բ
#define YK_THI_OPEN			26	//ң�ص�3����բ
#define YK_FOUR_OPEN		27	//ң�ص�4����բ
#define YK_FIR_CLOSE		28	//ң�ص�1�ֺ�բ
#define YK_SEC_CLOSE		29	//ң�ص�2�ֺ�բ
#define YK_THI_CLOSE		30	//ң�ص�3�ֺ�բ
#define YK_FOUR_CLOSE		31	//ң�ص�4�ֺ�բ

#define PLC_MODULE_EXIST	0	//�ز�ģ���Ѱ�װ��

#endif  //CONST2_H



 
