/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbStruct.h
 * 摘    要：本文件主要实现数据库的常量定义
 * 当前版本：1.0
 * 作    者：魏红
 * 完成日期：2007年4月
 *********************************************************************************************************/
#ifndef DBCONST_H
#define DBCONST_H
#include "FaCfg.h"
#include "LibDbConst.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//数据库标准常量定义
#define POINT_NUM		1200							//集中器专用
#define PN_MASK_SIZE  ((POINT_NUM+7)/8)					//测量点屏蔽位数组的大小
#define PN_NUM			65								//负控,配变,网络表,电能量采集器使用
#define PN_VALID_NUM	30								//有效测量点的个数

#define VIP_NUM			20								//重点户电表的数量
#define MFM_NUM			POINT_NUM						//多功能表(Multi-function watt-hour meter)数量
#define METER_NUM		POINT_NUM						//普通电表的数量,不包括重点户
#define CCT_SAVE_NUM		1221						//集抄实际保存的测量点个数
#define CCT_MFM_SAVE_NUM	64							//集抄实际保存的测量点个数

#define BANK_NUM     	29

//集抄测量点数据分BANK存储使用到的BANK号定义
#define CCT_BN_SPM		21			//单相表数据BANK
#define CCT_BN_MFM		21			//多功能表数据BANK

//数据库BANK0的SECT定义
#define SECT_ENERGY			0	//电能量类对象
#define SECT_DEMAND			1	//最大需量类对象
#define SECT_VARIABLE		2	//变量类对象
#define SECT_EVENT			3	//事件对象接口类
#define SECT_PARAM			4	//参变量类对象
#define SECT_FROZEN			5	//冻结类对象
#define SECT_ACQ_MONI		6	//采集监控类对象
#define SECT_COLLECT		7	//集合类对象
#define SECT_CTRL			8	//控制类对象
#define SECT_FILE_TRAN		9	//文件传输类对象
#define SECT_ESAM			10	//ESAM接口类对象
#define SECT_IN_OUT_PUT		11	//输入输出设备类对象
#define SECT_DISP			12	//显示类对象
#define SECT_AC_DATA		13	//交采数据

//扩展SECT定义，主要针对数据
#define SECT_STAT_PULSE_PARA	15	//统计、总加组、脉冲相应的数据
#define SECT_EVENT_DATA		16	//事件类相应的数据
#define SECT_FROZEN_DATA	17	//冻结类相应数据

#define SECT_NUM	 		18

#define IMG_NUM     0

#define INVALID_POINT     0

//版本信息的长度定义
#define SOFT_VER_LEN	32	//软件版本的字节长度

#define INN_SOFT_VER_LEN  23 //内部软件版本字节长度

#define OOB_SOFT_VER_LEN  46 //面向对象软件版本字节长度

//测量点动态映射
#define PNMAP_NUM		1		//测量点动态映射方案数量
								 
//测量点动态映射方案,方案号索引从1开始算,0表示不映射
#define MTRPNMAP		PNUNMAP	//PNUNMAP/PMMAP1用这个宏定义来切换负控支持的测量点需不需要动态映射
#define PNMAP_CCTMFM	PNUNMAP	//集抄多功能表映射方案,
#define PNMAP_VIP			1	//集抄重点户映射方案,

//698-45数据库常量定义
#define PNPARA_LEN	120// 1个测量点参数最多允许的长度，附属信息给5个array，预估一个长度
						//实际正常的附属信息为0，不足120的，按照0给出

/////////////////////////////////////////////////////////////////////////////////////////////
//国标版常量定义

//起始测量号的定义
//#define MTR_START_PN	16	//电表起始测量号， 1~15留给总加组用
#define MTR_START_PN	1	//电表起始测量号
#define GRP_START_PN	1	//总加组起始测量号
#define TURN_START_PN	1	//轮次起始测量号

//个数定义
#define GRP_NUM			(8+GRP_START_PN)	//总加组个数
#define MAX_GRP_PN		4	//总加组允许的最大总加测量点个数
#define GRPPARA_LEN		(MAX_GRP_PN*25+2)	//总加组参数长度
#define TURN_NUM		2	//轮次个数
#define	MAX_CTRLPARA_NUM 10	//控制参数编号


//数据类的定义
#define GB_DATACLASS1	1	//瞬时数据
#define GB_DATACLASS2	2	//冻结数据
#define GB_DATACLASS3	3	//事件
#define GB_DATACLASS4	4	//参数
#define GB_DATACLASS5	5	//控制
#define GB_DATACLASS6	6	//传输文件
#define GB_MAXDATACLASS	7	//	6+1
#define GB_DATACLASS8	8	//请求被级联终端主动上报
#define GB_DATACLASS9	9	//请求终端配置

//OO数据格式定义
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


//自定义数据类型
#define DT_FRZRELA			 106
#define DT_ACQ_TYPE			 107	
#define DT_MTR_ANNEX		 108	//电表附加信息域
#define DT_RPT_TYPE			 109	//

#define DT_PULSE_CFG		 110
#define DT_SCH_MTR_ANNEX	 111	//搜表附加信息域
#define DT_OVER_PARA			112//统计的越限判断参数  array Data
#define DT_OVER_RES			113//区间统计值
#define DT_INSTANCE			114//instance-specific
#define DT_EVTACQ_TYPE			 115	//事件采集方式

//定义ARRAY和STRUCT的特殊成员个数,空数据填充时填入1，
#define SPECIAL_NUM			0xFE	//tll 试试


//属性定义
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

//方法定义
#define OMD1							1
#define OMD2							2
#define OMD3							3
#define OMD4							4
#define OMD5							5

//DAR数据类型
#define DAR						0
#define DAR_SUCC				0	//成功
#define DAR_HW_INVALID			1	//硬件失效
#define DAR_SW_INVALID			2	//软件失效
#define DAR_RES_RW				3	//拒绝读写
#define DAR_OBJ_UNDEF			4	//对象未定义
#define DAR_CLASS_NOT_CONFORM	5	//对象接口类不符合
#define DAR_OBJ_NON_EXIST		6	//对象接口类不存在
#define DAR_TYPE_NO_MATCH		7	//类型不匹配  
#define DAR_OUT_OF_BOUNDS		8	//越界
#define DAR_BLK_NO_USE			9	//数据块不可用
#define DAR_FRM_TRANS_CANCEL	10	//分帧传输已取消 
#define DAR_NO_FRAM_TRANS		11	//不处于分帧传输
#define DAR_BLK_WR_CANCEL		12	//块写取消
#define DAR_NO_EXIST_BLK_WR		13	//不存在块写状态
#define DAR_BLK_SN_INVALID		14	//数据块序号无效
#define DAR_PWD_ERR				15	//密码错/未授权 
#define DAR_RATE_NOT_CHG		16	//通信速率不能更改
#define DAR_YEAR_ZONE_OVER		17	//年时区数超  
#define DAR_DAY_ZONE_OVER		18	//日时区数超  
#define DAR_RATE_OVER			19	//费率数超 
#define DAR_SEC_CERT_NO_MATCH	20	//安全认证不匹配
#define DAR_RPT_RECHG			21	//重复充值 
#define DAR_ESAM_CERT_FAIL		22	//ESAM验证失败
#define DAR_SEC_CERT_FAIL		23	//安全认证失败
#define DAR_CUSTOME_NO_MATCH	24	//客户编号不匹配
#define DAR_CHG_CNT_ERR			25	//充值次数错误
#define DAR_PCH_ELE_OVER_HOD	26	//购电超囤积
#define DAR_ADDR_EXCEPT			27	//地址异常
#define DAR_SMT_DCP_ERR			28	//对称解密错误
#define DAR_NONE_SMT_DCP_ERR	29	//非对称解密错误
#define DAR_SGN_ERR				30	//签名错误
#define DAR_MTR_HANG			31	//电能表挂起
#define DAR_TIME_TAG_INVALID	32	//时间标签无效
#define DAR_REQ_TIMEOUT			33	//请求超时 
#define DAR_OTHER				255	//其它


//数据格式定义
#define FMT_UNK	0	//未知数据格式
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

//非附录的扩展格式，从80开始
#define FMTEX_START 80
#define FMT_BIN		80


#define FMT_BCD	(24*0x100)
#define   FMT_ROUND   (1)
#define   FMT_NROUND  (0)

//不同版本的E,U,I,P,Q,cos的格式定义可能不一样,请使用以下的定义
//这些定义都是针对645ID的,非645ID的格式以协议为准
#define EFMT		FMT_BIN			//电能
#define REFMT		FMT_BIN			//电能
#define UFMT		FMT_BIN			//电压
//#define IFMT		FMT25			//电流
#define PFMT		FMT_BIN			//有功功率
#define QFMT		FMT_BIN			//有功功率
#define COSFMT		FMT_BIN			//功率因素			
#define DMFMT		FMT_BIN			//需量
#define DMTFMT		FMT_BIN			//需量时间
#define ANGFMT		FMT_BIN			//角度
//#define ANGFMT		FMT_BIN
#define VBRKCOUNTSFMT		FMT_BIN	//断相次数
#define VBRKACCUMTFMT		FMT_BIN	//断相累计时间
#define VBRKTIMEFMT			FMT_BCD	//断相起始/结束时刻
#define PROGTIMEFMT			FMT_BCD	//编程时间
#define DMCLEANTIMEFMT		FMT_BCD	//需量清零时间
#define PROGCOUNTSFMT		FMT_BIN	//编程次数
#define DMCLEANCOUNTSFMT	FMT_BIN	//需量清零次数
#define BATTWORKTFMT		FMT_BIN	//电池工作时间
/*
#define EFMT		FMT14			//电能
#define REFMT		FMT11			//电能
#define UFMT		FMT7			//电压
#define PFMT		FMT9			//有功功率
#define QFMT		FMT9			//有功功率
#define COSFMT		FMT5			//功率因素			
#define DMFMT		FMT23			//需量
#define DMTFMT		FMT17			//需量时间
#define ANGFMT		FMT5			//角度
#define ANGFMT		FMT5
#define VBRKCOUNTSFMT		FMT8	//断相次数
#define VBRKACCUMTFMT		FMT10	//断相累计时间
#define VBRKTIMEFMT			FMT17	//断相起始/结束时刻
#define PROGTIMEFMT			FMT17	//编程时间
#define DMCLEANTIMEFMT		FMT17	//需量清零时间
#define PROGCOUNTSFMT		FMT8	//编程次数
#define DMCLEANCOUNTSFMT	FMT8	//需量清零次数
#define BATTWORKTFMT		FMT10	//电池工作时间

#define EFMT		FMT_BIN			//电能
#define REFMT		FMT_BIN			//电能
#define UFMT		FMT_BIN			//电压
//#define IFMT		FMT25			//电流
#define PFMT		FMT_BIN			//有功功率
#define QFMT		FMT_BIN			//有功功率
#define COSFMT		FMT_BIN			//功率因素			
#define DMFMT		FMT_BIN			//需量
#define DMTFMT		FMT_BCD			//需量时间
#define ANGFMT		FMT_BIN			//角度
//#define ANGFMT		FMT_BIN
#define VBRKCOUNTSFMT		FMT_BIN	//断相次数
#define VBRKACCUMTFMT		FMT_BIN	//断相累计时间
#define VBRKTIMEFMT			FMT_BCD	//断相起始/结束时刻
#define PROGTIMEFMT			FMT_BCD	//编程时间
#define DMCLEANTIMEFMT		FMT_BCD	//需量清零时间
#define PROGCOUNTSFMT		FMT_BIN	//编程次数
#define DMCLEANCOUNTSFMT	FMT_BIN	//需量清零次数
#define BATTWORKTFMT		FMT_BIN	//电池工作时间
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
	#define IFMT		FMT6			//电流
	#define ILEN		2
#else	//PRO_698
	#define IFMT		FMT_BIN			//国电版不同
	#define ILEN		2
#endif

#define VBRKCOUNTSLEN		2	//断相次数
#define VBRKACCUMTLEN		4	//断相累计时间
#define VBRKTIMELEN			DATETIMELEN	//断相起始/结束时刻
#define PROGTIMELEN			DATETIMELEN	//编程时间
#define DMCLEANTIMELEN		DATETIMELEN	//需量清零时间
#define PROGCOUNTSLEN		4	//编程次数
#define DMCLEANCOUNTSLEN	4	//需量清零次数
#define BATTWORKTLEN		4	//电池工作时间


#define INVALID_VAL 	(-0x7ffffff0)
#define INVALID_VAL64 	(-0x7ffffffffffffff0LL)

#define INVALID_TIME 	0	//无效的时间


//数据类1-5的细分定义,因4类的最全，采用做通用 
#define CLASS_NULL				0	//备用                          
#define CLASS_P0				1	//PN无定义                      
#define CLASS_METER				2	//测量点                        
#define CLASS_SUMGROUP			3	//总加组                        
#define CLASS_MEASURE			4	//直流模拟量                    
#define CLASS_CONTROLTURN		5	//控制轮次                      
#define CLASS_TASK				8	//任务号                        
                           
#define GB_MAXOFF				1           //注意PN型的从1开始，0空余                                
#define GB_MAXCONTROLTURN		(4+GB_MAXOFF)	//功控轮次                      
#define GB_MAXMETER				PN_NUM	//电表数                        
#define GB_MAXPULSE				PN_NUM	//脉冲点数                      
#define GB_MAXMEASURE			(4+GB_MAXOFF)	//直流测量点数                  
#define GB_MAXSTATE  			(4+GB_MAXOFF)	//状态量数                      
#define GB_MAXBRANCH			(8+GB_MAXOFF)	//分路数                        
#define GB_MAXTASK				(64+GB_MAXOFF)  //任务数                        
#define GB_MAXSUMGROUP			(8+GB_MAXOFF)   //总加组号	 
#define GB_MAXCOMCHNNOTE		(5+GB_MAXOFF)	//普通中文信息条数
#define GB_MAXIMPCHNNOTE		(5+GB_MAXOFF)	//重要中文信息条数

#define GB_MAXERCODE			31  //事件代码      
#define GB_MAXCOMMTHREAD		4	//通信线程个数 
                                                                    
//单独定义参数部分为极限可能的容量                                  
#define GBC4_MAXMETER			GB_MAXMETER				//电表数            
#define GBC4_MAXSUMGROUP		GB_MAXSUMGROUP			//总加组            
#define GBC4_MAXMEASURE			GB_MAXMEASURE			//直流测量点数      
#define GBC4_MAXTASK			GB_MAXTASK				//任务数            
#define GBC4_MAXCONTROLTURN		GB_MAXCONTROLTURN		//功控轮次      
#define GBC4_MAXPULSE			GB_MAXPULSE				//脉冲数    
#define GBC4_MTRPORTNUM			32						//配置F33时的通信端口数

#define USR_MAIN_CLASS_NUM		16				//用户大类数
#define USR_SUB_CLASS_NUM		16				//用户大类数


												 
//单独定义参数部分几个变长参数的长度空间定义 
#define GBC4IDLEN_F10			(GBC4_MAXMETER*17+1)
#define GBC4IDLEN_F11			(GBC4_MAXPULSE*5+1)
#define GBC4IDLEN_F13			(GBC4_MAXMEASURE*3+1)
#define GBC4IDLEN_F14			(256)
#define GBC4IDLEN_F15			(256)
#define GBC4IDLEN_F27			(256)//(512)
#define GBC4IDLEN_F41			(137)
#define GBC4IDLEN_F65			(256)//(512)
#define GBC4IDLEN_F66			(256)//(512)


//单独定义几个变长参数的长度空间定义 
//#define GBC1IDLEN_F16			(((PN_NUM+7)>>3)+1) 
#define GBC1IDLEN_F169			(2)

//单独定义几个变长参数的长度空间定义 
#define GBC9IDLEN_F2			(((GBC4_MTRPORTNUM-1)*12)+17)	
#define GBC9IDLEN_F6			((USR_MAIN_CLASS_NUM*(GBC4_MTRPORTNUM-1+1))+2)

#define ADDONS_NULL		0
#define ADDONS_TIME		1
#define ADDONS_CURVE	2
#define ADDONS_DAYFRZ	3
#define ADDONS_MONFRZ	4
#define ADDONS_COPYDAY	5
#define ADDONS_EC		6

//费率数
#define RATE_NUM		4
#define TOTAL_RATE_NUM  (RATE_NUM+1)	//总+分费率的个数

//谐波次数
#define HARMONIC_NUM		21

//端口设置
#define	PN_PORT_INVALID	0	//无效端口( 针对脉冲无效设置，设置为PN_PROP_INVALID 时，此次测量点属性修改操作无效)

//测量点属性
#define	PN_PROP_AC		1	//交采
#define PN_PROP_METER	2	//电表
#define PN_PROP_PULSE	3	//脉冲
#define PN_PROP_DC		4	//直流模拟量
#define PN_PROP_CCT		5	//集抄测量点(包括各种链路,以后有需要可以扩展为PN_PROP_PLC,PN_PROP_CCT485等,不过为了避免扩展太多,还是推荐使用PN_PROP_CCT)
#define PN_PROP_RJ45	6	//网络RJ45抄表
#define PN_PROP_EPON	7	//光纤端口
#define PN_PROP_BBCCT	8	//宽带载波通道
//#define PN_PROP_EXTAC   8	//外接交采装置
#define PN_PROP_UNSUP	0xff	//暂时不支持的测量点类型

//相对于测量点属性的另外一种测量点类型定义,每种类型占一位
#define PN_TYPE_P0		0x01	//测量点0
#define PN_TYPE_AC		0x02	//交采
#define PN_TYPE_MTR		0x04	//电表
#define PN_TYPE_PULSE	0x08	//脉冲
#define PN_TYPE_DC		0x10	//直流模拟量

#define PN_TYPE_CCT		0x20	//集抄测量点

#define PN_TYPE_GRP		0x40	//总加组

#define PN_TYPE_MSR		(PN_TYPE_AC|PN_TYPE_MTR|PN_TYPE_PULSE) //测量点

//////////////////////////////////////////////////////////////////////////////////////
//GB2005和698相对于系统库数据项的不同定义
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
	#define MTR_ADDR_OFFSET		4		//8902中
	
	#define MTRPRO_TO_IFMT_SCALE 1		//电表协议库的电流格式到主站通信协议格式的量程转换
	
	#define NO_CUR				5		//无电流的固定阀值
	#define STD_UN				2200	//标准额定电压
	#define STD_IN				500		//标准额定电流
										 
	#define F10_LEN_PER_PN		17		//F10中每个测量点参数的长度
	#define F10_MTRNUM_LEN		1		//F10中本次电能表/交流采样装置配置数量n的长度
	#define F10_SN_LEN			1		//F10中电能表/交流采样装置序号的长度
	#define F10_PN_LEN			1		//F10中所属测量点号的长度							
#else	//PRO_698

	#define FMT22TOCUR_SCALE	100

	#define F25_LEN				13
	#define F25_CONN_OFFSET		12

	#define MTR_PARA_LEN		24		//8902
	#define MTR_ADDR_OFFSET		2		//8902中
	
	#define MTRPRO_TO_IFMT_SCALE 10		//电表协议库的电流格式到主站通信协议格式的量程转换
	
	#define NO_CUR				50		//无电流的固定阀值
	#define STD_UN				2200	//标准额定电压
	#define STD_IN				5000	//标准额定电流
										 
	#define F10_LEN_PER_PN		17		//F10中每个测量点参数的长度
	#define F10_MTRNUM_LEN		2		//F10中本次电能表/交流采样装置配置数量n的长度
	#define F10_SN_LEN			10		//F10中电能表/交流采样装置序号的长度
	#define F10_PN_LEN			2		//F10中所属测量点号的长度							

	#define C1_CFG_LEN	(1+1+(1+1+31)*USR_SUB_CLASS_NUM) 	//(大类号+小类号组数+(用户小类号+信息类组数n+31)*USR_SUB_CLASS_NUM)
	#define C2_CFG_LEN	(1+1+(1+1+31)*USR_SUB_CLASS_NUM)	//(大类号+小类号组数+(用户小类号+信息类组数n+31)*USR_SUB_CLASS_NUM)
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
//DLMS的相关定义
#define CURVEOBJ_NUM		48//43			//曲线类对象的个数（含数据、参数）
#define TIME_NUM			1			//时钟对象的个数
#define TTABLE_NUM			1			//时间表对象的个数
#define SINGLETTABLE_NUM	1			//单个动作时间表对象的个数
#define EXTREG_NUM			1			//扩展寄存器对象的个数
#define DEFAULT_NUM			1			//缺省对象个数

#define HLJ_MAXCONTROLTURN	(8+GB_MAXOFF)	//控制轮次   

#define EXC_REC_LENTH   50  //单条告警事件记录的最大长度；(除差动组越限配置的总加组的测量点个数>3（一般不会超过）此定义长度足够用)

//事件曲线的最大记录条数
#define MAXNUM_EVENT	255
#define MAXNUM_ONEERC3	(EXC_REC_LENTH-2)/6		//ERC3里变长数目的最大个数

#define MAX_CUR_2_1_DAT		GRP_NUM*96*7			//周期冻结总加组曲线保存条数，保存7天数据,每笔记录30字节，保存5376笔，占flash空间161280
#define MAX_CUR_3_1_DAT		GRP_NUM*31				//日冻结总加组曲线保存条数，保存1个月数据，每笔记录54字节，保存248笔，占flash空间13392
#define MAX_CUR_4_1_DAT		GRP_NUM*12				//月冻结总加组曲线保存条数，保存1年数据，每笔记录54字节，保存248笔，占flash空间5184
#define MAX_CUR_5_1_DAT		96						//周期冻结终端模拟量数据，保存一天

#define MAX_CUR_2_2_DAT		POINT_NUM*96			//周期冻结功率，电压，电流曲线，保存一天，每笔记录70字节，最多保存48000笔，占flash空间3360000
#define MAX_CUR_3_3_DAT		1000					//日冻结三相断相统计数据及最近一次断相记录，保存1个月,每笔记录148字节，最多保存1000笔，因为基本抄不到的，占flash空间148000

#define MAX_CUR_2_4_DAT		POINT_NUM*96			//周期冻结正反向有无功总电能示值曲线，按照500测量点估算，保存一天数据,每笔记录32字节，占flash空间1536000
#define MAX_CUR_3_4_DAT		POINT_NUM*7				//日冻结电能及四象限，4费率电能示值曲线，按照500测量点，保存7天数据,每笔记录176字节，占flash空间616000
#define MAX_CUR_4_4_DAT		POINT_NUM*6				//月冻结电能及四象限，4费率电能示值曲线，按照500测量点，保存6月数据,每笔记录176字节，占flash空间528000
#define MAX_CUR_5_4_DAT		POINT_NUM*6				//抄表日冻结电能及四象限，4费率电能示值曲线，按照500测量点，保存6月数据,每笔记录176字节，占flash空间528000

#define MAX_CUR_3_5_DAT		POINT_NUM*3				//日冻结有无功最大需量及发生时间,500只表，保存3天数据,每笔记录336字节，占flash空间504000
#define MAX_CUR_4_5_DAT		POINT_NUM*3				//月冻结有无功最大需量及发生时间,500只表，保存3个月数据,每笔记录336字节，占flash空间504000
#define MAX_CUR_5_5_DAT		POINT_NUM*3				//抄表日冻结有无功最大需量及发生时间，,500只表，保存3个月数据,每笔记录336字节，占flash空间504000

#define MAX_CUR_2_6_DAT		POINT_NUM*96			//周期冻结正反向有无功电能量曲线，按照500测量点估算，可以保存一天数据,每笔记录32字节，占flash空间1536000
#define MAX_CUR_3_6_DAT		POINT_NUM*7				//日冻结正反向有无功电能量曲线，按照500测量点估算，保存7天数据,每笔记录176字节，占flash空间616000
#define MAX_CUR_4_6_DAT		POINT_NUM*6				//月冻结正反向有无功电能量曲线，按照500测量点估算，保存6月数据,每笔记录176字节，占flash空间528000

#define MAX_CUR_3_7_DAT		POINT_NUM*7				//日冻结总及三相最大功率及发生时间，有功功率为0时间，保存7天数据，每笔记录88字节，占flash空间308000
#define MAX_CUR_4_7_DAT		POINT_NUM*6				//月冻结总及三相最大功率及发生时间，有功功率为0时间，保存6月数据，每笔记录88字节，占flash空间264000

#define MAX_CUR_3_8_DAT		POINT_NUM*7				//日冻结，日电压统计数据，保存7天数据，每笔记录114+16字节，占flash空间455000
#define MAX_CUR_4_8_DAT		POINT_NUM*6				//月冻结，月电压统计数据，保存6天数据，每笔记录114+16字节，占flash空间390000

#define MAX_CUR_3_9_DAT		POINT_NUM*7				//日冻结日不平衡越限累计时间，保存7天数据，每笔记录32+16字节，占flash空间168000
#define MAX_CUR_4_9_DAT		POINT_NUM*6				//月冻结月不平衡越限累计时间，保存6月数据，每笔记录32+16字节，占flash空间342000

#define MAX_CUR_3_10_DAT	POINT_NUM*7				//日冻结日电流越限数据，保存7天数据，每笔记录86字节，占flash空间301000
#define MAX_CUR_4_10_DAT	POINT_NUM*6				//月冻结月电流越限数据，保存6月数据，每笔记录86字节，占flash空间258000

#define MAX_CUR_3_11_DAT	POINT_NUM*7				//日冻结日视在功率越限累计时间，保存7天数据，每笔记录20字节，占flash空间70000
#define MAX_CUR_4_11_DAT	POINT_NUM*6				//月冻结月视在功率越限累计时间，保存6月数据，每笔记录20字节，占flash空间60000

#define MAX_CUR_3_12_DAT	POINT_NUM*7				//日冻结，日功率因数区段累计时间，保存7天数据，每笔记录22字节，占flash空间77000
#define MAX_CUR_4_12_DAT	POINT_NUM*6				//月冻结，月功率因数区段累计时间，保存6月数据，每笔记录22字节，占flash空间66000

#define MAX_CUR_3_13_DAT	64						//日冻结，终端日复位时间，日复位累计次数,记录64笔，每笔记录20字节，flash空间1280
#define MAX_CUR_4_13_DAT	24						//月冻结，终端月复位时间，月复位累计次数,记录24笔，每笔记录20字节，flash空间480

#define MAX_CUR_3_14_DAT	64						//日冻结，终端日控制统计数据,记录64笔，每笔记录20字节，flash空间1280
#define MAX_CUR_4_14_DAT	24						//月冻结，终端月控制统计数据,记录24笔，每笔记录20字节，flash空间480

#define MAX_CUR_3_15_DAT	GRP_NUM*31				//日冻结，总加组最大，最小有功功率计发生时间，有功功率为0累计时间,记录248笔，每笔记录50字节，flash空间12400
#define MAX_CUR_4_15_DAT	GRP_NUM*12				//月冻结，总加组最大，最小有功功率计发生时间，有功功率为0累计时间,记录96笔，每笔记录50字节，flash空间4800

#define MAX_CUR_3_16_DAT	31						//终端谐波日冻结最大值及发生时间,保存31天，记录31笔，每笔记录814字节，flash空间25234
#define MAX_CUR_3_17_DAT	31						//终端谐波日冻结最大值及发生时间,保存31天，记录31笔，每笔记录814字节，flash空间25234

//#define RATELEN	0  //不带费率
#define RATELEN	1	//带费率

#if (RATELEN==0)  //不带费率
	#define RATEOFFSET 1
#else
	#define RATEOFFSET 0
#endif

#endif //DBCONST_H

