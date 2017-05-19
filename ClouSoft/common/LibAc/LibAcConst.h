/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LibAcConst.h
 * 摘    要：本文件主要实现对交流采样常量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 * 备    注: 
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

#define SCN_NUM           6    //采样的通道数
#define NUM_PER_CYC       160  //160 每个周期采集的样点数
#define CYC_NUM		      16    //每个通道缓存多少个周期的样点
#define SBUF_SIZE         (NUM_PER_CYC*CYC_NUM)  //每个通道缓存的样点数
//#define FREQ_NUM          100  //频率取平均的个数
#define CALCU_PERIOD_NUM  20  //计算时取的周期个数
#define FREQ_CYC_NUM      256 //200     //频率计算的周期数

#define NUM_PER_CYC_45HZ  (NUM_PER_CYC*50/45)  //每个周期采集的样点数
#define NUM_PER_CYC_55HZ  (NUM_PER_CYC*50/55 + 1)  //每个周期采集的样点数

#define SIGMA_CYC_NUM     50


//#define E_CONST        6000
						//  (瓦 *秒) / 脉冲常数

#define  E_PER_PULSE  0x1C9C3800 //(10L*1000L*3600L*1000L*10*8/6000)    //每个脉冲等于多少个 瓦/10 * 毫秒/8
  					 //  (瓦 *秒 *毫秒*瓦/10*毫秒/8) / 脉冲常数
												
//#define E_PER_PULSE    0x1C9C3800  //(1000*3600*1000*10*8/E_CONST)*10    //每个脉冲等于多少个 瓦/10 * 毫秒/8
  						  //  (瓦 *秒 *毫秒*瓦/10*毫秒/8) / 脉冲常数

//#define E_PER_PULSE       0xABA9500 //0x112A880 //(1000*3600*1000*10*8/16000)    //每个脉冲等于多少个 瓦/10 * 毫秒/8
  						 //  (瓦 *秒 *毫秒*瓦/10*毫秒/8) / 脉冲常数



#define QUAD_POS_P        0x00
#define QUAD_NEG_P        0x01 
#define QUAD_POS_Q        0x00
#define QUAD_NEG_Q        0x02

//终端内计算的电能的序号
//正向有功
#define EP_POS_A	0
#define EP_POS_B	1
#define EP_POS_C	2
#define EP_POS_ABC	3

//反向有功
#define EP_NEG_A	4
#define EP_NEG_B	5
#define EP_NEG_C	6
#define EP_NEG_ABC	7

//正向无功
#define EQ_POS_A	8
#define EQ_POS_B	9
#define EQ_POS_C	10
#define EQ_POS_ABC	11

//反向无功
#define EQ_NEG_A	12
#define EQ_NEG_B	13
#define EQ_NEG_C	14
#define EQ_NEG_ABC	15

//A相无功1、2、3、4象限
#define EQ_Q1_A		16
#define EQ_Q2_A		17
#define EQ_Q3_A		18
#define EQ_Q4_A		19	

//B相无功1、2、3、4象限
#define EQ_Q1_B		20
#define EQ_Q2_B		21
#define EQ_Q3_B		22
#define EQ_Q4_B		23	

//C相无功1、2、3、4象限
#define EQ_Q1_C		24
#define EQ_Q2_C		25
#define EQ_Q3_C		26
#define EQ_Q4_C		27	

//ABC总无功1、2、3、4象限
#define EQ_Q1		28
#define EQ_Q2		29
#define EQ_Q3		30
#define EQ_Q4		31	

#define EQ_IND_A	32//A相感性无功
#define EQ_IND_B	33
#define EQ_IND_C	34

#define EQ_CAP_A	35//A相容性无功
#define EQ_CAP_B	36
#define EQ_CAP_C	37

//基波有功
#define EP_FUND_A		38
#define EP_FUND_B		39
#define EP_FUND_C		40
#define EP_FUND_ABC		41

//基波无功
#define EQ_FUND_A		42
#define EQ_FUND_B		43
#define EQ_FUND_C		44
#define EQ_FUND_ABC		45

//组合有功
#define EP_COM_A		46
#define EP_COM_B		47
#define EP_COM_C		48
#define EP_COM_ABC		49

//组合无功1
#define EQ_COM_A1		50
#define EQ_COM_B1		51
#define EQ_COM_C1		52
#define EQ_COM_ABC1		53

//组合无功2
#define EQ_COM_A2		54
#define EQ_COM_B2		55
#define EQ_COM_C2		56
#define EQ_COM_ABC2		57

//三相有功绝对值
#define EP_ABS_ABC		58//A、B、C三项单相有功绝对值的和
#define EP_ABS_NEG		59//A、B、C三项单相反向有功
//三相无功绝对值
#define EQ_ABS_ABC		60//A、B、C三项单相无功绝对值的和
#define EQ_ABS_NEG		61//A、B、C三项单相反向无功

//正向视在
#define ES_POS_A	62
#define ES_POS_B	63
#define ES_POS_C	64
#define ES_POS_ABC	65

//反向视在
#define ES_NEG_A	66
#define ES_NEG_B	67
#define ES_NEG_C	68
#define ES_NEG_ABC	69




//交采内部数据的索引
#define AC_VAL_UA	0
#define AC_VAL_UB	1
#define AC_VAL_UC	2
#define AC_VAL_IA	3
#define AC_VAL_IB	4
#define AC_VAL_IC	5
#define AC_VAL_I0	6	//零序电流

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
#define AC_VAL_ANG_I0	25	//零序电流角度

#define AC_VAL_F			26	//频率
#define AC_VAL_PHASESTATUS	27	//相序状态
#define AC_VAL_MTRSTATUS	28	//电表状态字

#define AC_VAL_S			29
#define AC_VAL_SA			30	
#define AC_VAL_SB			31
#define AC_VAL_SC			32

#define AC_VAL_PNSTATUS		33	//测量点状态字

#define AC_BASE_VAL_UA	34
#define AC_BASE_VAL_UB	35
#define AC_BASE_VAL_UC	36
#define AC_BASE_VAL_IA	37
#define AC_BASE_VAL_IB	38
#define AC_BASE_VAL_IC	39

// 一分钟平均功率
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

//当前需量
#define AC_VAL_DEMAND_P			52
#define AC_VAL_DEMAND_Q			53
#define AC_VAL_DEMAND_S			54


//	//相角
//	#define AC_VAL_ANG_A	55
//	#define AC_VAL_ANG_B	56
//	#define AC_VAL_ANG_C	57

#define AC_VAL_NUM			55






#define FLAG_ADD	3//加法
#define FLAG_SUB	2//减法

//无功电量累加标志配置时用到的常量定义
#define QADD	FLAG_ADD//加
#define QSUB	FLAG_SUB//减
#define Q1ADD	QADD//第一象限加
#define Q1SUB	QSUB//第一象限减
#define Q2ADD	(QADD<<2)//第二象限加
#define Q2SUB	(QSUB<<2)//第二象限减
#define Q3ADD	(QADD<<4)//第三象限加
#define Q3SUB	(QSUB<<4)//第三象限减
#define Q4ADD	(QADD<<6)//第四象限加
#define Q4SUB	(QSUB<<6)//第四象限减

//有功电量累加标志
#define PADD	FLAG_ADD//加
#define PSUB	FLAG_SUB//减
#define P_POSADD	PADD//正向有功加
#define P_POSSUB	PSUB//正向有功减
#define P_NEGADD	(PADD<<2)//反向有功加
#define P_NEGSUB	(PSUB<<2)//反向有功减

//相序状态标志
#define DISORDER_U      0x01
#define DISORDER_I      0x02

#define	ID_TO_RATENUM(id)	( (id&0x000f)==0x000f ? RATE_NUM+1 : 1 )

/////////////////////////////////////////////////////////////////////////////
//配置
#define ACLOG_ENABLE	1 

#define AD_CHK_DELAY      (NUM_PER_CYC*50*2)
						
#define CONNECT_1P    	1	//单相表
#define CONNECT_3P3W    3
#define CONNECT_3P4W    4

#define COS_N           1000//   N.NNN   

//一下量的最大允许个数
#define HARM_NUM_MAX	21	//谐波最大允许次数
#define ENERGY_NUM_MAX	80	//电量最大允许个数
#define DEMAND_NUM_MAX 	50	//需量最大允许个数

//月冻结部分宏定义
#define	AUTO_DATE_NUM		3//抄表时段，分成3段

#define PGM_PULSE_DEMAND	0    //需量周期可编程脉冲输出
#define PGM_PULSE_TOU		1	 //时段投切可编程脉冲输出

#define ENERGY_LOG_MAX_NUM			50//((256-1)/ENERGY_LOG_LEN)//铁电最多保存的数据个数51个,
							  //铁电最大长度256byte，去掉1BYTE保存费率，剩下保存电量

#define DEMAND_LOG_MAX_NUM			35	//((128-1)/ENERGY_LOG_LEN)//铁电最多保存的数据个数25个,
										//其他地方占用几个字节实际能存24个
										//铁电最大长度256byte，去掉1BYTE保存费率，剩下保存电量

#endif //LIBACCONST_H
