/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�LibAcConst.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԽ������������Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 * ��    ע: 
 *********************************************************************************************************/
#ifndef LIBACCONST_H
#define LIBACCONST_H
#include "syscfg.h"

#ifdef SYS_LINUX
#define FFT_NUM	     64
#else	//SYS_VDK
#define FFT_NUM	     128
#endif

#define RATE_NUM           4

#define FREQ_UNIT    256      //256
#define FREQ_SHIFT   8        
#define AVGP_NUM     16

#define SCN_NUM           6    //������ͨ����
#define NUM_PER_CYC       160  //160 ÿ�����ڲɼ���������
#define CYC_NUM		      16    //ÿ��ͨ��������ٸ����ڵ�����
#define SBUF_SIZE         (NUM_PER_CYC*CYC_NUM)  //ÿ��ͨ�������������
//#define FREQ_NUM          100  //Ƶ��ȡƽ���ĸ���
#define CALCU_PERIOD_NUM  20  //����ʱȡ�����ڸ���
#define FREQ_CYC_NUM      256 //200     //Ƶ�ʼ����������

#define NUM_PER_CYC_45HZ  (NUM_PER_CYC*50/45)  //ÿ�����ڲɼ���������
#define NUM_PER_CYC_55HZ  (NUM_PER_CYC*50/55 + 1)  //ÿ�����ڲɼ���������

#define SIGMA_CYC_NUM     50


//#define E_CONST        6000
						//  (�� *��) / ���峣��

#define  E_PER_PULSE  0x1C9C3800 //(10L*1000L*3600L*1000L*10*8/6000)    //ÿ��������ڶ��ٸ� ��/10 * ����/8
  					 //  (�� *�� *����*��/10*����/8) / ���峣��
												
//#define E_PER_PULSE    0x1C9C3800  //(1000*3600*1000*10*8/E_CONST)*10    //ÿ��������ڶ��ٸ� ��/10 * ����/8
  						  //  (�� *�� *����*��/10*����/8) / ���峣��

//#define E_PER_PULSE       0xABA9500 //0x112A880 //(1000*3600*1000*10*8/16000)    //ÿ��������ڶ��ٸ� ��/10 * ����/8
  						 //  (�� *�� *����*��/10*����/8) / ���峣��



#define QUAD_POS_P        0x00
#define QUAD_NEG_P        0x01 
#define QUAD_POS_Q        0x00
#define QUAD_NEG_Q        0x02

//�ն��ڼ���ĵ��ܵ����
//�����й�
#define EP_POS_A	0
#define EP_POS_B	1
#define EP_POS_C	2
#define EP_POS_ABC	3

//�����й�
#define EP_NEG_A	4
#define EP_NEG_B	5
#define EP_NEG_C	6
#define EP_NEG_ABC	7

//�����޹�
#define EQ_POS_A	8
#define EQ_POS_B	9
#define EQ_POS_C	10
#define EQ_POS_ABC	11

//�����޹�
#define EQ_NEG_A	12
#define EQ_NEG_B	13
#define EQ_NEG_C	14
#define EQ_NEG_ABC	15

//A���޹�1��2��3��4����
#define EQ_Q1_A		16
#define EQ_Q2_A		17
#define EQ_Q3_A		18
#define EQ_Q4_A		19	

//B���޹�1��2��3��4����
#define EQ_Q1_B		20
#define EQ_Q2_B		21
#define EQ_Q3_B		22
#define EQ_Q4_B		23	

//C���޹�1��2��3��4����
#define EQ_Q1_C		24
#define EQ_Q2_C		25
#define EQ_Q3_C		26
#define EQ_Q4_C		27	

//ABC���޹�1��2��3��4����
#define EQ_Q1		28
#define EQ_Q2		29
#define EQ_Q3		30
#define EQ_Q4		31	

#define EQ_IND_A	32//A������޹�
#define EQ_IND_B	33
#define EQ_IND_C	34

#define EQ_CAP_A	35//A�������޹�
#define EQ_CAP_B	36
#define EQ_CAP_C	37

//�����й�
#define EP_FUND_A		38
#define EP_FUND_B		39
#define EP_FUND_C		40
#define EP_FUND_ABC		41

//�����޹�
#define EQ_FUND_A		42
#define EQ_FUND_B		43
#define EQ_FUND_C		44
#define EQ_FUND_ABC		45

//����й�
#define EP_COM_A		46
#define EP_COM_B		47
#define EP_COM_C		48
#define EP_COM_ABC		49

//����޹�1
#define EQ_COM_A1		50
#define EQ_COM_B1		51
#define EQ_COM_C1		52
#define EQ_COM_ABC1		53

//����޹�2
#define EQ_COM_A2		54
#define EQ_COM_B2		55
#define EQ_COM_C2		56
#define EQ_COM_ABC2		57

//�����й�����ֵ
#define EP_ABS_ABC		58//A��B��C������й�����ֵ�ĺ�
#define EP_ABS_NEG		59//A��B��C����෴���й�
//�����޹�����ֵ
#define EQ_ABS_ABC		60//A��B��C������޹�����ֵ�ĺ�
#define EQ_ABS_NEG		61//A��B��C����෴���޹�

//��������
#define ES_POS_A	62
#define ES_POS_B	63
#define ES_POS_C	64
#define ES_POS_ABC	65

//��������
#define ES_NEG_A	66
#define ES_NEG_B	67
#define ES_NEG_C	68
#define ES_NEG_ABC	69




//�����ڲ����ݵ�����
#define AC_VAL_UA	0
#define AC_VAL_UB	1
#define AC_VAL_UC	2
#define AC_VAL_IA	3
#define AC_VAL_IB	4
#define AC_VAL_IC	5
#define AC_VAL_I0	6	//�������

#define AC_VAL_P	7
#define AC_VAL_PA	8
#define AC_VAL_PB	9
#define AC_VAL_PC	10
#define AC_VAL_Q	11
#define AC_VAL_QA	12
#define AC_VAL_QB	13
#define AC_VAL_QC	14

#define AC_VAL_COS	15
#define AC_VAL_COSA	16
#define AC_VAL_COSB	17
#define AC_VAL_COSC	18

#define AC_VAL_ANG_UA	19
#define AC_VAL_ANG_UB	20
#define AC_VAL_ANG_UC	21
#define AC_VAL_ANG_IA	22
#define AC_VAL_ANG_IB	23
#define AC_VAL_ANG_IC	24
#define AC_VAL_ANG_I0	25	//��������Ƕ�

#define AC_VAL_F			26	//Ƶ��
#define AC_VAL_PHASESTATUS	27	//����״̬
#define AC_VAL_MTRSTATUS	28	//���״̬��

#define AC_VAL_S			29
#define AC_VAL_SA			30	
#define AC_VAL_SB			31
#define AC_VAL_SC			32

#define AC_VAL_PNSTATUS		33	//������״̬��

#define AC_BASE_VAL_UA	34
#define AC_BASE_VAL_UB	35
#define AC_BASE_VAL_UC	36
#define AC_BASE_VAL_IA	37
#define AC_BASE_VAL_IB	38
#define AC_BASE_VAL_IC	39

// һ����ƽ������
#define AC_VAL_AVG_P		40
#define AC_VAL_AVG_PA		41
#define AC_VAL_AVG_PB		42
#define AC_VAL_AVG_PC		43
#define AC_VAL_AVG_Q		44
#define AC_VAL_AVG_QA		45
#define AC_VAL_AVG_QB		46
#define AC_VAL_AVG_QC		47
#define AC_VAL_AVG_S		48
#define AC_VAL_AVG_SA		49	
#define AC_VAL_AVG_SB		50
#define AC_VAL_AVG_SC		51

//��ǰ����
#define AC_VAL_DEMAND_P			52
#define AC_VAL_DEMAND_Q			53
#define AC_VAL_DEMAND_S			54


//	//���
//	#define AC_VAL_ANG_A	55
//	#define AC_VAL_ANG_B	56
//	#define AC_VAL_ANG_C	57

#define AC_VAL_NUM			55






#define FLAG_ADD	3//�ӷ�
#define FLAG_SUB	2//����

//�޹������ۼӱ�־����ʱ�õ��ĳ�������
#define QADD	FLAG_ADD//��
#define QSUB	FLAG_SUB//��
#define Q1ADD	QADD//��һ���޼�
#define Q1SUB	QSUB//��һ���޼�
#define Q2ADD	(QADD<<2)//�ڶ����޼�
#define Q2SUB	(QSUB<<2)//�ڶ����޼�
#define Q3ADD	(QADD<<4)//�������޼�
#define Q3SUB	(QSUB<<4)//�������޼�
#define Q4ADD	(QADD<<6)//�������޼�
#define Q4SUB	(QSUB<<6)//�������޼�

//�й������ۼӱ�־
#define PADD	FLAG_ADD//��
#define PSUB	FLAG_SUB//��
#define P_POSADD	PADD//�����й���
#define P_POSSUB	PSUB//�����й���
#define P_NEGADD	(PADD<<2)//�����й���
#define P_NEGSUB	(PSUB<<2)//�����й���

//����״̬��־
#define DISORDER_U      0x01
#define DISORDER_I      0x02

#define	ID_TO_RATENUM(id)	( (id&0x000f)==0x000f ? RATE_NUM+1 : 1 )

/////////////////////////////////////////////////////////////////////////////
//����
#define ACLOG_ENABLE	1 

#define AD_CHK_DELAY      (NUM_PER_CYC*50*2)
						
#define CONNECT_1P    	1	//�����
#define CONNECT_3P3W    3
#define CONNECT_3P4W    4

#define COS_N           1000//   N.NNN   

//һ����������������
#define HARM_NUM_MAX	21	//г������������
#define ENERGY_NUM_MAX	80	//��������������
#define DEMAND_NUM_MAX 	50	//��������������

//�¶��Ჿ�ֺ궨��
#define	AUTO_DATE_NUM		3//����ʱ�Σ��ֳ�3��

#define PGM_PULSE_DEMAND	0    //�������ڿɱ���������
#define PGM_PULSE_TOU		1	 //ʱ��Ͷ�пɱ���������

#define ENERGY_LOG_MAX_NUM			50//((256-1)/ENERGY_LOG_LEN)//������ౣ������ݸ���51��,
							  //������󳤶�256byte��ȥ��1BYTE������ʣ�ʣ�±������

#define DEMAND_LOG_MAX_NUM			35	//((128-1)/ENERGY_LOG_LEN)//������ౣ������ݸ���25��,
										//�����ط�ռ�ü����ֽ�ʵ���ܴ�24��
										//������󳤶�256byte��ȥ��1BYTE������ʣ�ʣ�±������

#endif //LIBACCONST_H
