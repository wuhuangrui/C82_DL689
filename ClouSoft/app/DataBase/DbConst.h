/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbStruct.h
 * ժ    Ҫ�����ļ���Ҫʵ�����ݿ�ĳ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�κ��
 * ������ڣ�2007��4��
 *********************************************************************************************************/
#ifndef DBCONST_H
#define DBCONST_H
#include "FaCfg.h"
#include "LibDbConst.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//���ݿ��׼��������
#define POINT_NUM		1200							//������ר��
#define PN_MASK_SIZE  ((POINT_NUM+7)/8)					//����������λ����Ĵ�С
#define PN_NUM			65								//����,���,�����,�������ɼ���ʹ��
#define PN_VALID_NUM	30								//��Ч������ĸ���

#define VIP_NUM			20								//�ص㻧��������
#define MFM_NUM			POINT_NUM						//�๦�ܱ�(Multi-function watt-hour meter)����
#define METER_NUM		POINT_NUM						//��ͨ��������,�������ص㻧
#define CCT_SAVE_NUM		1221						//����ʵ�ʱ���Ĳ��������
#define CCT_MFM_SAVE_NUM	64							//����ʵ�ʱ���Ĳ��������

#define BANK_NUM     	29

//�������������ݷ�BANK�洢ʹ�õ���BANK�Ŷ���
#define CCT_BN_SPM		21			//���������BANK
#define CCT_BN_MFM		21			//�๦�ܱ�����BANK

//���ݿ�BANK0��SECT����
#define SECT_ENERGY			0	//�����������
#define SECT_DEMAND			1	//������������
#define SECT_VARIABLE		2	//���������
#define SECT_EVENT			3	//�¼�����ӿ���
#define SECT_PARAM			4	//�α��������
#define SECT_FROZEN			5	//���������
#define SECT_ACQ_MONI		6	//�ɼ���������
#define SECT_COLLECT		7	//���������
#define SECT_CTRL			8	//���������
#define SECT_FILE_TRAN		9	//�ļ����������
#define SECT_ESAM			10	//ESAM�ӿ������
#define SECT_IN_OUT_PUT		11	//��������豸�����
#define SECT_DISP			12	//��ʾ�����
#define SECT_AC_DATA		13	//��������

//��չSECT���壬��Ҫ�������
#define SECT_STAT_PULSE_PARA	15	//ͳ�ơ��ܼ��顢������Ӧ������
#define SECT_EVENT_DATA		16	//�¼�����Ӧ������
#define SECT_FROZEN_DATA	17	//��������Ӧ����

#define SECT_NUM	 		18

#define IMG_NUM     0

#define INVALID_POINT     0

//�汾��Ϣ�ĳ��ȶ���
#define SOFT_VER_LEN	32	//����汾���ֽڳ���

#define INN_SOFT_VER_LEN  23 //�ڲ�����汾�ֽڳ���

#define OOB_SOFT_VER_LEN  46 //�����������汾�ֽڳ���

//�����㶯̬ӳ��
#define PNMAP_NUM		1		//�����㶯̬ӳ�䷽������
								 
//�����㶯̬ӳ�䷽��,������������1��ʼ��,0��ʾ��ӳ��
#define MTRPNMAP		PNUNMAP	//PNUNMAP/PMMAP1������궨�����л�����֧�ֵĲ������費��Ҫ��̬ӳ��
#define PNMAP_CCTMFM	PNUNMAP	//�����๦�ܱ�ӳ�䷽��,
#define PNMAP_VIP			1	//�����ص㻧ӳ�䷽��,

//698-45���ݿⳣ������
#define PNPARA_LEN	120// 1������������������ĳ��ȣ�������Ϣ��5��array��Ԥ��һ������
						//ʵ�������ĸ�����ϢΪ0������120�ģ�����0����

/////////////////////////////////////////////////////////////////////////////////////////////
//����泣������

//��ʼ�����ŵĶ���
//#define MTR_START_PN	16	//�����ʼ�����ţ� 1~15�����ܼ�����
#define MTR_START_PN	1	//�����ʼ������
#define GRP_START_PN	1	//�ܼ�����ʼ������
#define TURN_START_PN	1	//�ִ���ʼ������

//��������
#define GRP_NUM			(8+GRP_START_PN)	//�ܼ������
#define MAX_GRP_PN		4	//�ܼ������������ܼӲ��������
#define GRPPARA_LEN		(MAX_GRP_PN*25+2)	//�ܼ����������
#define TURN_NUM		2	//�ִθ���
#define	MAX_CTRLPARA_NUM 10	//���Ʋ������


//������Ķ���
#define GB_DATACLASS1	1	//˲ʱ����
#define GB_DATACLASS2	2	//��������
#define GB_DATACLASS3	3	//�¼�
#define GB_DATACLASS4	4	//����
#define GB_DATACLASS5	5	//����
#define GB_DATACLASS6	6	//�����ļ�
#define GB_MAXDATACLASS	7	//	6+1
#define GB_DATACLASS8	8	//���󱻼����ն������ϱ�
#define GB_DATACLASS9	9	//�����ն�����

//OO���ݸ�ʽ����
#define DT_NULL				 0
#define DT_ARRAY			 1
#define DT_STRUCT			 2
#define DT_BOOL				 3
#define DT_BIT_STR			 4
#define DT_DB_LONG			 5
#define DT_DB_LONG_U		 6
#define DT_OCT_STR			 9
#define DT_VIS_STR			 10
#define DT_UTF8_STR			 12

#define DT_INT				 15
#define DT_LONG				 16
#define DT_UNSIGN			 17
#define DT_LONG_U			 18
#define DT_LONG64			 20
#define DT_LONG64_U			 21
#define DT_ENUM				 22
#define DT_FLOAT32			 23	 
#define DT_FLOAT64			 24	 
#define DT_DATE_TIME		 25		 
#define DT_DATE				 26
#define DT_TIME				 27
#define DT_DATE_TIME_S		 28

#define DT_OI				 80
#define DT_OAD				 81
#define DT_ROAD				 82
#define DT_OMD				 83
#define DT_TI				 84
#define DT_TSA				 85
#define DT_MAC				 86
#define DT_RN				 87
#define DT_REGION			 88	 
#define DT_SCALE_UNIT		 89		 
#define DT_RSD				 90
#define DT_CSD				 91
#define DT_MS				 92
#define DT_SID				 93
#define DT_SID_MAC			 94	 
#define DT_COMDCB			 95	 
#define DT_RCSD				 96	


//�Զ�����������
#define DT_FRZRELA			 106
#define DT_ACQ_TYPE			 107	
#define DT_MTR_ANNEX		 108	//�������Ϣ��
#define DT_RPT_TYPE			 109	//

#define DT_PULSE_CFG		 110
#define DT_SCH_MTR_ANNEX	 111	//�ѱ�����Ϣ��
#define DT_OVER_PARA			112//ͳ�Ƶ�Խ���жϲ���  array Data
#define DT_OVER_RES			113//����ͳ��ֵ
#define DT_INSTANCE			114//instance-specific
#define DT_EVTACQ_TYPE			 115	//�¼��ɼ���ʽ

//����ARRAY��STRUCT�������Ա����,���������ʱ����1��
#define SPECIAL_NUM			0xFE	//tll ����


//���Զ���
#define ATTR1							1
#define ATTR2							2
#define ATTR3							3
#define ATTR4							4
#define ATTR5							5
#define ATTR6							6
#define ATTR7							7
#define ATTR8							8
#define ATTR9							9
#define ATTR10							10
#define ATTR11							11
#define ATTR12							12
#define ATTR13							13
#define ATTR14							14
#define ATTR15							15
#define ATTR16							16
#define ATTR17							17
#define ATTR18							18

//��������
#define OMD1							1
#define OMD2							2
#define OMD3							3
#define OMD4							4
#define OMD5							5

//DAR��������
#define DAR						0
#define DAR_SUCC				0	//�ɹ�
#define DAR_HW_INVALID			1	//Ӳ��ʧЧ
#define DAR_SW_INVALID			2	//���ʧЧ
#define DAR_RES_RW				3	//�ܾ���д
#define DAR_OBJ_UNDEF			4	//����δ����
#define DAR_CLASS_NOT_CONFORM	5	//����ӿ��಻����
#define DAR_OBJ_NON_EXIST		6	//����ӿ��಻����
#define DAR_TYPE_NO_MATCH		7	//���Ͳ�ƥ��  
#define DAR_OUT_OF_BOUNDS		8	//Խ��
#define DAR_BLK_NO_USE			9	//���ݿ鲻����
#define DAR_FRM_TRANS_CANCEL	10	//��֡������ȡ�� 
#define DAR_NO_FRAM_TRANS		11	//�����ڷ�֡����
#define DAR_BLK_WR_CANCEL		12	//��дȡ��
#define DAR_NO_EXIST_BLK_WR		13	//�����ڿ�д״̬
#define DAR_BLK_SN_INVALID		14	//���ݿ������Ч
#define DAR_PWD_ERR				15	//�����/δ��Ȩ 
#define DAR_RATE_NOT_CHG		16	//ͨ�����ʲ��ܸ���
#define DAR_YEAR_ZONE_OVER		17	//��ʱ������  
#define DAR_DAY_ZONE_OVER		18	//��ʱ������  
#define DAR_RATE_OVER			19	//�������� 
#define DAR_SEC_CERT_NO_MATCH	20	//��ȫ��֤��ƥ��
#define DAR_RPT_RECHG			21	//�ظ���ֵ 
#define DAR_ESAM_CERT_FAIL		22	//ESAM��֤ʧ��
#define DAR_SEC_CERT_FAIL		23	//��ȫ��֤ʧ��
#define DAR_CUSTOME_NO_MATCH	24	//�ͻ���Ų�ƥ��
#define DAR_CHG_CNT_ERR			25	//��ֵ��������
#define DAR_PCH_ELE_OVER_HOD	26	//���糬�ڻ�
#define DAR_ADDR_EXCEPT			27	//��ַ�쳣
#define DAR_SMT_DCP_ERR			28	//�Գƽ��ܴ���
#define DAR_NONE_SMT_DCP_ERR	29	//�ǶԳƽ��ܴ���
#define DAR_SGN_ERR				30	//ǩ������
#define DAR_MTR_HANG			31	//���ܱ����
#define DAR_TIME_TAG_INVALID	32	//ʱ���ǩ��Ч
#define DAR_REQ_TIMEOUT			33	//����ʱ 
#define DAR_OTHER				255	//����


//���ݸ�ʽ����
#define FMT_UNK	0	//δ֪���ݸ�ʽ
#define FMT1	(1)
#define FMT2	(2)
#define FMT3	(3)
#define FMT4	(4)
#define FMT5	(5)
#define FMT6	(6)
#define FMT7	(7)
#define FMT8	(8)
#define FMT9	(9)
#define FMT10	(10)
#define FMT11	(11)
#define FMT12	(12)
#define FMT13	(13)
#define FMT14	(14)
#define FMT15	(15)
#define FMT16	(16)
#define FMT17	(17)
#define FMT18	(18)
#define FMT19	(19)
#define FMT20	(20)
#define FMT21	(21)
#define FMT22	(22)
#define FMT23	(23)
#define FMT24	(24)
#define FMT25	(25)
#define FMT26	(26)
#define FMT27	(27)
#define FMT28	(28)
#define FMT29	(29)
#define FMT30	(30)

#define FMT_NUM		31
#define FMTEX_NUM	1

//�Ǹ�¼����չ��ʽ����80��ʼ
#define FMTEX_START 80
#define FMT_BIN		80


#define FMT_BCD	(24*0x100)
#define   FMT_ROUND   (1)
#define   FMT_NROUND  (0)

//��ͬ�汾��E,U,I,P,Q,cos�ĸ�ʽ������ܲ�һ��,��ʹ�����µĶ���
//��Щ���嶼�����645ID��,��645ID�ĸ�ʽ��Э��Ϊ׼
#define EFMT		FMT_BIN			//����
#define REFMT		FMT_BIN			//����
#define UFMT		FMT_BIN			//��ѹ
//#define IFMT		FMT25			//����
#define PFMT		FMT_BIN			//�й�����
#define QFMT		FMT_BIN			//�й�����
#define COSFMT		FMT_BIN			//��������			
#define DMFMT		FMT_BIN			//����
#define DMTFMT		FMT_BIN			//����ʱ��
#define ANGFMT		FMT_BIN			//�Ƕ�
//#define ANGFMT		FMT_BIN
#define VBRKCOUNTSFMT		FMT_BIN	//�������
#define VBRKACCUMTFMT		FMT_BIN	//�����ۼ�ʱ��
#define VBRKTIMEFMT			FMT_BCD	//������ʼ/����ʱ��
#define PROGTIMEFMT			FMT_BCD	//���ʱ��
#define DMCLEANTIMEFMT		FMT_BCD	//��������ʱ��
#define PROGCOUNTSFMT		FMT_BIN	//��̴���
#define DMCLEANCOUNTSFMT	FMT_BIN	//�����������
#define BATTWORKTFMT		FMT_BIN	//��ع���ʱ��
/*
#define EFMT		FMT14			//����
#define REFMT		FMT11			//����
#define UFMT		FMT7			//��ѹ
#define PFMT		FMT9			//�й�����
#define QFMT		FMT9			//�й�����
#define COSFMT		FMT5			//��������			
#define DMFMT		FMT23			//����
#define DMTFMT		FMT17			//����ʱ��
#define ANGFMT		FMT5			//�Ƕ�
#define ANGFMT		FMT5
#define VBRKCOUNTSFMT		FMT8	//�������
#define VBRKACCUMTFMT		FMT10	//�����ۼ�ʱ��
#define VBRKTIMEFMT			FMT17	//������ʼ/����ʱ��
#define PROGTIMEFMT			FMT17	//���ʱ��
#define DMCLEANTIMEFMT		FMT17	//��������ʱ��
#define PROGCOUNTSFMT		FMT8	//��̴���
#define DMCLEANCOUNTSFMT	FMT8	//�����������
#define BATTWORKTFMT		FMT10	//��ع���ʱ��

#define EFMT		FMT_BIN			//����
#define REFMT		FMT_BIN			//����
#define UFMT		FMT_BIN			//��ѹ
//#define IFMT		FMT25			//����
#define PFMT		FMT_BIN			//�й�����
#define QFMT		FMT_BIN			//�й�����
#define COSFMT		FMT_BIN			//��������			
#define DMFMT		FMT_BIN			//����
#define DMTFMT		FMT_BCD			//����ʱ��
#define ANGFMT		FMT_BIN			//�Ƕ�
//#define ANGFMT		FMT_BIN
#define VBRKCOUNTSFMT		FMT_BIN	//�������
#define VBRKACCUMTFMT		FMT_BIN	//�����ۼ�ʱ��
#define VBRKTIMEFMT			FMT_BCD	//������ʼ/����ʱ��
#define PROGTIMEFMT			FMT_BCD	//���ʱ��
#define DMCLEANTIMEFMT		FMT_BCD	//��������ʱ��
#define PROGCOUNTSFMT		FMT_BIN	//��̴���
#define DMCLEANCOUNTSFMT	FMT_BIN	//�����������
#define BATTWORKTFMT		FMT_BIN	//��ع���ʱ��
*/
#define DATETIMELEN	12

#define ELEN		4
#define RELEN		4
#define ULEN		2
#define PLEN		4//3//4
#define QLEN		4//3//4
#define COSLEN		2
#define DMLEN		4//3//4
#define DMTLEN		7//DATETIMELEN
#define ANGLEN		2

#ifdef PRO_GB2005
	#define IFMT		FMT6			//����
	#define ILEN		2
#else	//PRO_698
	#define IFMT		FMT_BIN			//����治ͬ
	#define ILEN		2
#endif

#define VBRKCOUNTSLEN		2	//�������
#define VBRKACCUMTLEN		4	//�����ۼ�ʱ��
#define VBRKTIMELEN			DATETIMELEN	//������ʼ/����ʱ��
#define PROGTIMELEN			DATETIMELEN	//���ʱ��
#define DMCLEANTIMELEN		DATETIMELEN	//��������ʱ��
#define PROGCOUNTSLEN		4	//��̴���
#define DMCLEANCOUNTSLEN	4	//�����������
#define BATTWORKTLEN		4	//��ع���ʱ��


#define INVALID_VAL 	(-0x7ffffff0)
#define INVALID_VAL64 	(-0x7ffffffffffffff0LL)

#define INVALID_TIME 	0	//��Ч��ʱ��


//������1-5��ϸ�ֶ���,��4�����ȫ��������ͨ�� 
#define CLASS_NULL				0	//����                          
#define CLASS_P0				1	//PN�޶���                      
#define CLASS_METER				2	//������                        
#define CLASS_SUMGROUP			3	//�ܼ���                        
#define CLASS_MEASURE			4	//ֱ��ģ����                    
#define CLASS_CONTROLTURN		5	//�����ִ�                      
#define CLASS_TASK				8	//�����                        
                           
#define GB_MAXOFF				1           //ע��PN�͵Ĵ�1��ʼ��0����                                
#define GB_MAXCONTROLTURN		(4+GB_MAXOFF)	//�����ִ�                      
#define GB_MAXMETER				PN_NUM	//�����                        
#define GB_MAXPULSE				PN_NUM	//�������                      
#define GB_MAXMEASURE			(4+GB_MAXOFF)	//ֱ����������                  
#define GB_MAXSTATE  			(4+GB_MAXOFF)	//״̬����                      
#define GB_MAXBRANCH			(8+GB_MAXOFF)	//��·��                        
#define GB_MAXTASK				(64+GB_MAXOFF)  //������                        
#define GB_MAXSUMGROUP			(8+GB_MAXOFF)   //�ܼ����	 
#define GB_MAXCOMCHNNOTE		(5+GB_MAXOFF)	//��ͨ������Ϣ����
#define GB_MAXIMPCHNNOTE		(5+GB_MAXOFF)	//��Ҫ������Ϣ����

#define GB_MAXERCODE			31  //�¼�����      
#define GB_MAXCOMMTHREAD		4	//ͨ���̸߳��� 
                                                                    
//���������������Ϊ���޿��ܵ�����                                  
#define GBC4_MAXMETER			GB_MAXMETER				//�����            
#define GBC4_MAXSUMGROUP		GB_MAXSUMGROUP			//�ܼ���            
#define GBC4_MAXMEASURE			GB_MAXMEASURE			//ֱ����������      
#define GBC4_MAXTASK			GB_MAXTASK				//������            
#define GBC4_MAXCONTROLTURN		GB_MAXCONTROLTURN		//�����ִ�      
#define GBC4_MAXPULSE			GB_MAXPULSE				//������    
#define GBC4_MTRPORTNUM			32						//����F33ʱ��ͨ�Ŷ˿���

#define USR_MAIN_CLASS_NUM		16				//�û�������
#define USR_SUB_CLASS_NUM		16				//�û�������


												 
//��������������ּ����䳤�����ĳ��ȿռ䶨�� 
#define GBC4IDLEN_F10			(GBC4_MAXMETER*17+1)
#define GBC4IDLEN_F11			(GBC4_MAXPULSE*5+1)
#define GBC4IDLEN_F13			(GBC4_MAXMEASURE*3+1)
#define GBC4IDLEN_F14			(256)
#define GBC4IDLEN_F15			(256)
#define GBC4IDLEN_F27			(256)//(512)
#define GBC4IDLEN_F41			(137)
#define GBC4IDLEN_F65			(256)//(512)
#define GBC4IDLEN_F66			(256)//(512)


//�������弸���䳤�����ĳ��ȿռ䶨�� 
//#define GBC1IDLEN_F16			(((PN_NUM+7)>>3)+1) 
#define GBC1IDLEN_F169			(2)

//�������弸���䳤�����ĳ��ȿռ䶨�� 
#define GBC9IDLEN_F2			(((GBC4_MTRPORTNUM-1)*12)+17)	
#define GBC9IDLEN_F6			((USR_MAIN_CLASS_NUM*(GBC4_MTRPORTNUM-1+1))+2)

#define ADDONS_NULL		0
#define ADDONS_TIME		1
#define ADDONS_CURVE	2
#define ADDONS_DAYFRZ	3
#define ADDONS_MONFRZ	4
#define ADDONS_COPYDAY	5
#define ADDONS_EC		6

//������
#define RATE_NUM		4
#define TOTAL_RATE_NUM  (RATE_NUM+1)	//��+�ַ��ʵĸ���

//г������
#define HARMONIC_NUM		21

//�˿�����
#define	PN_PORT_INVALID	0	//��Ч�˿�( ���������Ч���ã�����ΪPN_PROP_INVALID ʱ���˴β����������޸Ĳ�����Ч)

//����������
#define	PN_PROP_AC		1	//����
#define PN_PROP_METER	2	//���
#define PN_PROP_PULSE	3	//����
#define PN_PROP_DC		4	//ֱ��ģ����
#define PN_PROP_CCT		5	//����������(����������·,�Ժ�����Ҫ������չΪPN_PROP_PLC,PN_PROP_CCT485��,����Ϊ�˱�����չ̫��,�����Ƽ�ʹ��PN_PROP_CCT)
#define PN_PROP_RJ45	6	//����RJ45����
#define PN_PROP_EPON	7	//���˶˿�
#define PN_PROP_BBCCT	8	//����ز�ͨ��
//#define PN_PROP_EXTAC   8	//��ӽ���װ��
#define PN_PROP_UNSUP	0xff	//��ʱ��֧�ֵĲ���������

//����ڲ��������Ե�����һ�ֲ��������Ͷ���,ÿ������ռһλ
#define PN_TYPE_P0		0x01	//������0
#define PN_TYPE_AC		0x02	//����
#define PN_TYPE_MTR		0x04	//���
#define PN_TYPE_PULSE	0x08	//����
#define PN_TYPE_DC		0x10	//ֱ��ģ����

#define PN_TYPE_CCT		0x20	//����������

#define PN_TYPE_GRP		0x40	//�ܼ���

#define PN_TYPE_MSR		(PN_TYPE_AC|PN_TYPE_MTR|PN_TYPE_PULSE) //������

//////////////////////////////////////////////////////////////////////////////////////
//GB2005��698�����ϵͳ��������Ĳ�ͬ����
#ifdef PRO_GB2005

	#define FMT22TOCUR_SCALE	10

	#define F25_LEN				8
	#define F25_CONN_OFFSET		7

	#define F26_VOLUPPER_OFFSET	6
	#define F26_VOLLOWER_OFFSET 8		
	#define F26_CURUPPER_OFFSET	10
	#define F26_CURUP_OFFSET	12	
	#define F26_ZCURUP_OFFSET	14
	#define F26_SUPER_OFFSET	16
	#define F26_SUP_OFFSET		19
	#define F26_VUNB_OFFSET		22
	#define F26_IUNB_OFFSET		(F26_VUNB_OFFSET+2)
	#define MTR_PARA_LEN		70		//8902
	#define MTR_ADDR_OFFSET		4		//8902��
	
	#define MTRPRO_TO_IFMT_SCALE 1		//���Э���ĵ�����ʽ����վͨ��Э���ʽ������ת��
	
	#define NO_CUR				5		//�޵����Ĺ̶���ֵ
	#define STD_UN				2200	//��׼���ѹ
	#define STD_IN				500		//��׼�����
										 
	#define F10_LEN_PER_PN		17		//F10��ÿ������������ĳ���
	#define F10_MTRNUM_LEN		1		//F10�б��ε��ܱ�/��������װ����������n�ĳ���
	#define F10_SN_LEN			1		//F10�е��ܱ�/��������װ����ŵĳ���
	#define F10_PN_LEN			1		//F10������������ŵĳ���							
#else	//PRO_698

	#define FMT22TOCUR_SCALE	100

	#define F25_LEN				13
	#define F25_CONN_OFFSET		12

	#define MTR_PARA_LEN		24		//8902
	#define MTR_ADDR_OFFSET		2		//8902��
	
	#define MTRPRO_TO_IFMT_SCALE 10		//���Э���ĵ�����ʽ����վͨ��Э���ʽ������ת��
	
	#define NO_CUR				50		//�޵����Ĺ̶���ֵ
	#define STD_UN				2200	//��׼���ѹ
	#define STD_IN				5000	//��׼�����
										 
	#define F10_LEN_PER_PN		17		//F10��ÿ������������ĳ���
	#define F10_MTRNUM_LEN		2		//F10�б��ε��ܱ�/��������װ����������n�ĳ���
	#define F10_SN_LEN			10		//F10�е��ܱ�/��������װ����ŵĳ���
	#define F10_PN_LEN			2		//F10������������ŵĳ���							

	#define C1_CFG_LEN	(1+1+(1+1+31)*USR_SUB_CLASS_NUM) 	//(�����+С�������+(�û�С���+��Ϣ������n+31)*USR_SUB_CLASS_NUM)
	#define C2_CFG_LEN	(1+1+(1+1+31)*USR_SUB_CLASS_NUM)	//(�����+С�������+(�û�С���+��Ϣ������n+31)*USR_SUB_CLASS_NUM)
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
//DLMS����ض���
#define CURVEOBJ_NUM		48//43			//���������ĸ����������ݡ�������
#define TIME_NUM			1			//ʱ�Ӷ���ĸ���
#define TTABLE_NUM			1			//ʱ������ĸ���
#define SINGLETTABLE_NUM	1			//��������ʱ������ĸ���
#define EXTREG_NUM			1			//��չ�Ĵ�������ĸ���
#define DEFAULT_NUM			1			//ȱʡ�������

#define HLJ_MAXCONTROLTURN	(8+GB_MAXOFF)	//�����ִ�   

#define EXC_REC_LENTH   50  //�����澯�¼���¼����󳤶ȣ�(�����Խ�����õ��ܼ���Ĳ��������>3��һ�㲻�ᳬ�����˶��峤���㹻��)

//�¼����ߵ�����¼����
#define MAXNUM_EVENT	255
#define MAXNUM_ONEERC3	(EXC_REC_LENTH-2)/6		//ERC3��䳤��Ŀ��������

#define MAX_CUR_2_1_DAT		GRP_NUM*96*7			//���ڶ����ܼ������߱�������������7������,ÿ�ʼ�¼30�ֽڣ�����5376�ʣ�ռflash�ռ�161280
#define MAX_CUR_3_1_DAT		GRP_NUM*31				//�ն����ܼ������߱�������������1�������ݣ�ÿ�ʼ�¼54�ֽڣ�����248�ʣ�ռflash�ռ�13392
#define MAX_CUR_4_1_DAT		GRP_NUM*12				//�¶����ܼ������߱�������������1�����ݣ�ÿ�ʼ�¼54�ֽڣ�����248�ʣ�ռflash�ռ�5184
#define MAX_CUR_5_1_DAT		96						//���ڶ����ն�ģ�������ݣ�����һ��

#define MAX_CUR_2_2_DAT		POINT_NUM*96			//���ڶ��Ṧ�ʣ���ѹ���������ߣ�����һ�죬ÿ�ʼ�¼70�ֽڣ���ౣ��48000�ʣ�ռflash�ռ�3360000
#define MAX_CUR_3_3_DAT		1000					//�ն����������ͳ�����ݼ����һ�ζ����¼������1����,ÿ�ʼ�¼148�ֽڣ���ౣ��1000�ʣ���Ϊ�����������ģ�ռflash�ռ�148000

#define MAX_CUR_2_4_DAT		POINT_NUM*96			//���ڶ������������޹��ܵ���ʾֵ���ߣ�����500��������㣬����һ������,ÿ�ʼ�¼32�ֽڣ�ռflash�ռ�1536000
#define MAX_CUR_3_4_DAT		POINT_NUM*7				//�ն�����ܼ������ޣ�4���ʵ���ʾֵ���ߣ�����500�����㣬����7������,ÿ�ʼ�¼176�ֽڣ�ռflash�ռ�616000
#define MAX_CUR_4_4_DAT		POINT_NUM*6				//�¶�����ܼ������ޣ�4���ʵ���ʾֵ���ߣ�����500�����㣬����6������,ÿ�ʼ�¼176�ֽڣ�ռflash�ռ�528000
#define MAX_CUR_5_4_DAT		POINT_NUM*6				//�����ն�����ܼ������ޣ�4���ʵ���ʾֵ���ߣ�����500�����㣬����6������,ÿ�ʼ�¼176�ֽڣ�ռflash�ռ�528000

#define MAX_CUR_3_5_DAT		POINT_NUM*3				//�ն������޹��������������ʱ��,500ֻ������3������,ÿ�ʼ�¼336�ֽڣ�ռflash�ռ�504000
#define MAX_CUR_4_5_DAT		POINT_NUM*3				//�¶������޹��������������ʱ��,500ֻ������3��������,ÿ�ʼ�¼336�ֽڣ�ռflash�ռ�504000
#define MAX_CUR_5_5_DAT		POINT_NUM*3				//�����ն������޹��������������ʱ�䣬,500ֻ������3��������,ÿ�ʼ�¼336�ֽڣ�ռflash�ռ�504000

#define MAX_CUR_2_6_DAT		POINT_NUM*96			//���ڶ������������޹����������ߣ�����500��������㣬���Ա���һ������,ÿ�ʼ�¼32�ֽڣ�ռflash�ռ�1536000
#define MAX_CUR_3_6_DAT		POINT_NUM*7				//�ն������������޹����������ߣ�����500��������㣬����7������,ÿ�ʼ�¼176�ֽڣ�ռflash�ռ�616000
#define MAX_CUR_4_6_DAT		POINT_NUM*6				//�¶������������޹����������ߣ�����500��������㣬����6������,ÿ�ʼ�¼176�ֽڣ�ռflash�ռ�528000

#define MAX_CUR_3_7_DAT		POINT_NUM*7				//�ն����ܼ���������ʼ�����ʱ�䣬�й�����Ϊ0ʱ�䣬����7�����ݣ�ÿ�ʼ�¼88�ֽڣ�ռflash�ռ�308000
#define MAX_CUR_4_7_DAT		POINT_NUM*6				//�¶����ܼ���������ʼ�����ʱ�䣬�й�����Ϊ0ʱ�䣬����6�����ݣ�ÿ�ʼ�¼88�ֽڣ�ռflash�ռ�264000

#define MAX_CUR_3_8_DAT		POINT_NUM*7				//�ն��ᣬ�յ�ѹͳ�����ݣ�����7�����ݣ�ÿ�ʼ�¼114+16�ֽڣ�ռflash�ռ�455000
#define MAX_CUR_4_8_DAT		POINT_NUM*6				//�¶��ᣬ�µ�ѹͳ�����ݣ�����6�����ݣ�ÿ�ʼ�¼114+16�ֽڣ�ռflash�ռ�390000

#define MAX_CUR_3_9_DAT		POINT_NUM*7				//�ն����ղ�ƽ��Խ���ۼ�ʱ�䣬����7�����ݣ�ÿ�ʼ�¼32+16�ֽڣ�ռflash�ռ�168000
#define MAX_CUR_4_9_DAT		POINT_NUM*6				//�¶����²�ƽ��Խ���ۼ�ʱ�䣬����6�����ݣ�ÿ�ʼ�¼32+16�ֽڣ�ռflash�ռ�342000

#define MAX_CUR_3_10_DAT	POINT_NUM*7				//�ն����յ���Խ�����ݣ�����7�����ݣ�ÿ�ʼ�¼86�ֽڣ�ռflash�ռ�301000
#define MAX_CUR_4_10_DAT	POINT_NUM*6				//�¶����µ���Խ�����ݣ�����6�����ݣ�ÿ�ʼ�¼86�ֽڣ�ռflash�ռ�258000

#define MAX_CUR_3_11_DAT	POINT_NUM*7				//�ն��������ڹ���Խ���ۼ�ʱ�䣬����7�����ݣ�ÿ�ʼ�¼20�ֽڣ�ռflash�ռ�70000
#define MAX_CUR_4_11_DAT	POINT_NUM*6				//�¶��������ڹ���Խ���ۼ�ʱ�䣬����6�����ݣ�ÿ�ʼ�¼20�ֽڣ�ռflash�ռ�60000

#define MAX_CUR_3_12_DAT	POINT_NUM*7				//�ն��ᣬ�չ������������ۼ�ʱ�䣬����7�����ݣ�ÿ�ʼ�¼22�ֽڣ�ռflash�ռ�77000
#define MAX_CUR_4_12_DAT	POINT_NUM*6				//�¶��ᣬ�¹������������ۼ�ʱ�䣬����6�����ݣ�ÿ�ʼ�¼22�ֽڣ�ռflash�ռ�66000

#define MAX_CUR_3_13_DAT	64						//�ն��ᣬ�ն��ո�λʱ�䣬�ո�λ�ۼƴ���,��¼64�ʣ�ÿ�ʼ�¼20�ֽڣ�flash�ռ�1280
#define MAX_CUR_4_13_DAT	24						//�¶��ᣬ�ն��¸�λʱ�䣬�¸�λ�ۼƴ���,��¼24�ʣ�ÿ�ʼ�¼20�ֽڣ�flash�ռ�480

#define MAX_CUR_3_14_DAT	64						//�ն��ᣬ�ն��տ���ͳ������,��¼64�ʣ�ÿ�ʼ�¼20�ֽڣ�flash�ռ�1280
#define MAX_CUR_4_14_DAT	24						//�¶��ᣬ�ն��¿���ͳ������,��¼24�ʣ�ÿ�ʼ�¼20�ֽڣ�flash�ռ�480

#define MAX_CUR_3_15_DAT	GRP_NUM*31				//�ն��ᣬ�ܼ��������С�й����ʼƷ���ʱ�䣬�й�����Ϊ0�ۼ�ʱ��,��¼248�ʣ�ÿ�ʼ�¼50�ֽڣ�flash�ռ�12400
#define MAX_CUR_4_15_DAT	GRP_NUM*12				//�¶��ᣬ�ܼ��������С�й����ʼƷ���ʱ�䣬�й�����Ϊ0�ۼ�ʱ��,��¼96�ʣ�ÿ�ʼ�¼50�ֽڣ�flash�ռ�4800

#define MAX_CUR_3_16_DAT	31						//�ն�г���ն������ֵ������ʱ��,����31�죬��¼31�ʣ�ÿ�ʼ�¼814�ֽڣ�flash�ռ�25234
#define MAX_CUR_3_17_DAT	31						//�ն�г���ն������ֵ������ʱ��,����31�죬��¼31�ʣ�ÿ�ʼ�¼814�ֽڣ�flash�ռ�25234

//#define RATELEN	0  //��������
#define RATELEN	1	//������

#if (RATELEN==0)  //��������
	#define RATEOFFSET 1
#else
	#define RATEOFFSET 0
#endif

#endif //DBCONST_H

