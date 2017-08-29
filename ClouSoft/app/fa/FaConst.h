#ifndef CONST2_H
#define CONST2_H
#include "FaCfg.h"
#include "DbConst.h"



#define  PWROFF_VER       0x0001




#define ERR_OK       0x0
#define ERR_FORWARD  0x1
#define ERR_INVALID  0x2    //设置内容非法
#define ERR_PERM     0x3
#define ERR_ITEM     0x4	//数据库不支持的数据项
#define ERR_TIME     0x5    //时间失效

#define ERR_UNSUP	 0x06	//电表不支持的数据项等
#define ERR_FAIL	 0x07	//尝试失败,比如抄表失败等

#define ERR_ADDR     0x11
#define ERR_SEND     0x12
#define ERR_SMS      0x13 
#define ERR_PNUNMAP	 0x14	//测量点未映射

#define ERR_TIMEOUT  0x20
#define ERR_SYS      0x30

#define EMPTY_DATA 	 0
#define INVALID_DATA 0xff



//应用通知:
//把在延时通知中,需要2分钟才提交的通知消息放到前面,
//其它1分钟可以提交较快的ID放到INFO_LONG_DELAY_END规定的号码后
#define INFO_NONE	   			0
#define INFO_APP_RST			1
#define INFO_SYS_RST   			2
#define INFO_WK_PARA			3	//GPRS工作线程参数更改
#define INFO_FAP_PARA			4	//主站通信协议参数更改
#define INFO_COMM_RST			5	//与主站通信需要复位
#define INFO_COMM_RLD			6	//与主站通信需要重装参数

#define INFO_230M_PARA			7	//电台参数更改
#define INFO_MTR_EXC_RESET		8	//抄表事件数据清零
//无延时
#define INFO_ACTIVE	   			9	//短信或者振铃激活信息
#define INFO_METER_PARA			10   //电表参数更改
#define INFO_RST_PARA			11  //参数复位
#define INFO_RST_DATA			12	//数据复位
#define	INFO_VER_CHANGE			13	//版本变更
#define INFO_AC_PARA			14	//交采参数变更
#define INFO_COMTASK_PARA		15	//普通任务参数变更
#define INFO_STAT_PARA			16	//统计任务参数变更
#define INFO_YX_PARA 	   		17	//遥信参数变更
#define INFO_RST_TERM_STAT		18	//复位终端统计数据
#define INFO_PULSE	   			19	//脉冲参数更改
#define	INFO_CTPT_PARA			20	//测量点CTPT参数变更
#define INFO_PLC_PARA	   		21   //载波参数更改
#define INFO_PLC_CLRRT	   		22	//清路由
#define INFO_PLC_STOPRD			23	 //停止抄表
#define INFO_PLC_RESUMERD		24	 //恢复抄表	
#define INFO_ADJTIME_PLC   		25	//对载波进行校时
#define INFO_ADJTIME_485   		26	//对485进行校时
#define INFO_VIPLIST	   		27 	//重点户更改
#define INFO_GRP_PARA	   		28	//总加组参数变更
#define INFO_READ_DONE	   		29	//抄表完成
#define INFO_DC_SAMPLE			30
#define INFO_LINK	   			31	//级联参数更改


#define INFO_DISCONNECT			32		//DISCONNECT
#define INFO_CTRL				35		//控制线程
#define INFO_ENTERDOMAIN		36		//复位时通信进入domain

#define INFO_HAND_RESET			37	//手动复位
#define INFO_OTHR645PARA_CHANGE	38	//其它645参数改变
#define INFO_CUR_PARA	   		39	//645曲线参数更改									
#define INFO_CLR_ENERGY         40	//清电量645命令
#define INFO_CLR_DEMAND         41	//清需量
#define INFO_CLR_645EVENT       42	//清645事件命令
#define INFO_CLR_ALL			43	//总清零命令
#define INFO_RST_ALLPARA		44  //清楚所有参数数据
#define INFO_COMM_ADDR			45	//终端地址有修改
#define INFO_COMM_APN			46  //模块APN有修改
#define INFO_FC_NETWORK			47  //友讯达组网
#define INFO_PLC_MTRSCHCMD		48		//Plc搜索抄表命令
#define INFO_PLC_STATUPDATA		49		//Plc统计更新
#define INFO_PLC_RDALLRTINFO 	50  //读取所有节点的中继路由信息
#define INFO_PORTSCH_PARA		51  //搜索端口号控制参数
#define INFO_USB_COPY 		    52  //USB拷贝
#define INFO_USB_COPYEND 		53  //USB拷贝解除
#define INFO_PLC_UPDATE_ROUTE   54  //载波路由器升级
#define INFO_RADIO_PARA         55  //无线参数变更
#define INFO_RADIO_CHANNEL      56  //切换信道
#define INFO_PLC_WIRLESS_CHANGE 57  //无线信道变更
#define INFO_TIME_SET			58  //集中器时间被更改
#define INFO_RJMTR_PARACHG		59	//RJ45网络表参数修改
#define INFO_PRIMTR_PARACHG		60	//RJ45网络表参数修改
#define INFO_FRZPARA_CHG			61	//冻结参数变更
#define INFO_HARDWARE_INIT		62
#define INFO_MTR_UPDATE			63	//面向对象电表档案全部清除
#define INFO_TASK_CFG_UPDATE	64	//面向对象任务配置单元更新
#define INFO_ACQ_SCH_UPDATE		65	//面向对象采集方案更新
#define INFO_EVT_RESET				66	//面向对象事件复位，清所有事件
#define INFO_EVT_MTRCLR			67	//面向对象电表清零
#define INFO_EVT_DMDCLR			68	//面向对象需量清零
#define INFO_EVT_EVTCLR				69	//面向对象事件清零
#define INFO_FRZDATA_RESET			70	//冻结数据复位
#define INFO_SYNC_MTR				71	//载波表档案同步
#define INFO_SCH_MTR				72	//立即启动搜表

#define INFO_CLASS19_METHOD_RST				73	//设备接口类19--复位
#define INFO_CLASS19_METHOD_EXE				74	//设备接口类19--执行
#define INFO_CLASS19_METHOD_DATA_INIT		75	//设备接口类19--数据初始化
#define INFO_CLASS19_METHOD_RST_FACT_PARA	76	//设备接口类19--恢复出厂参数
#define INFO_CLASS19_METHOD_EVT_INIT		78	//设备接口类19--事件初始化
#define INFO_CLASS19_METHOD_DEM_INIT		79	//设备接口类19--需量初始化

#define INFO_PULSEDATA_RESET		80	//脉冲数据复位

#define INFO_GPRS_OFFLINE					81
#define INFO_COMM_TERMIP					82

#define INFO_COMM_GPRS_RLD					83
#define INFO_COMM_ETH_RLD					84
#define INFO_RS232_PARACHG					85	//232端口参数
#define INFO_RS485_PARACHG					86	//485端口参数
#define INFO_INFRA_PARACHG					87	//红外端口参数
#define INFO_RELAY_PARACHG					88	//继电器输出参数
#define INFO_MULPORT_PARACHG				89	//多功能端子参数
#define INFO_PLC_TRANS_PARACHG				90	//载波透明转发参数
#define INFO_PLC_PARACHG					91	//载波端口参数
#define INFO_CLASS14_STAT_CHG				92	//区间统计接口
#define INFO_CLASS15_STAT_CHG				93	//累加平均接口
#define INFO_CLASS16_STAT_CHG				94	//极值工具接口
#define INFO_TZ_DC_PARACHG					95	//时区时段费率参数
#define INFO_TERM_PROG						96	//终端编程记录
#define INFO_POWERCTRL_REC					97	//功控跳闸记录
#define INFO_ENERGYCTRL_REC					98	//电控跳闸记录
#define INFO_ENERGYCTRL_ALARM				99	//电控告警
#define INFO_ENERGYBUY_PARACHG				100	//购电参数设置
#define INFO_DEVICE_485_ERR					101	//设备485故障记录
#define INFO_TERM_INIT						102	//终端初始化
#define INFO_TERM_VER_CHG					103	//终端版本变更
#define INFO_ADJ_TERM_TIME					104	//终端对时
#define INFO_ESAM_AUTH_FAIL					105	//终端消息认证失败
#define INFO_YK_REC							106	//遥控跳闸记录
#define INFO_RP_SCH_UPDATE					107	//上报方案更新
#define INFO_EVT_EVTRESET					108	//面向对象事件复位
#define INFO_EVT_CLREVTRESET				109	//面向对象事件清零事件复位
#define INFO_DEVICE_CCT_ERR					110	//设备载波通道故障
#define INFO_TERM_MTRCLKPRG					111	//终端对电能表校时
#define INFO_SYNC_T188PARA					112 //同步水气热表档案到协议转换器
#define INFO_START_485_SCH_MTR				113	//485启动搜表
#define INFO_STOP_485_SCH_MTR				114	//485停止搜表
#define INFO_ONE_BRAODCAST_ARG_CCT			115	//单地址广播校时参数更改
#define INFO_ONE_BRAODCAST_ARG_485			116	//单地址广播校时参数更改
#define INFO_PLC_MOD_CHANGED				117	//载波模块更新
#define INFO_MTR_BRAODCAST_ARG_CCT			118	//广播校时参数更改
#define INFO_MTR_BRAODCAST_ARG_485			119	//广播校时参数更改


#define INFO_AUTODETECT_START				120
#define INFO_AUTODETECT_STOP				121
#define INFO_STOP_FEED_WDG					122 //禁止喂狗消息，看门狗测试命令用

#define INFO_MTR_INFO_UPDATE				123	//电表档案更新
#define INFO_TASK_CFG_DEL					124	//面向对象任务配置单元删除
#define INFO_ACQ_SCH_DEL					125	//面向对象采集方案删除
#define INFO_PWROFF							126 //停电消息

#define INFO_LOCAL_SEND_LED					127	//本地发送灯
#define INFO_LOCAL_RECV_LED					128	//本地接收灯
#define INFO_GPRS_SEND_LED					129 //GPRS发送灯
#define INFO_GPRS_RECV_LED					130	//GRPS接收灯

#define INFO_MTR_ALL_CLEAR					131	//电表档案更新

#define INFO_UPD_MTR_CTRL					132	//更新抄表控制结构
#define INFO_MTR_EXC_MEM					133	//抄表事件动态内存分配
#define INFO_END	   						134//空消息,作为所有消息的结束
									//把本通知恒定作为最后一个

#define INFO_NUM	   	    		(INFO_END+1)
#define INFO_SHORT_DELAY_START	 	INFO_230M_PARA
#define INFO_NO_DELAY_START	 		INFO_ACTIVE

//消息长延时与短延时的定义,单位秒
#define INFO_SHORT_DELAY	15
#define INFO_LONG_DELAY		30

#define DC_CHN_MAX		2		//直流模拟通道数

//地方版本定义
#define LOCAL_GD      1
#define LOCAL_JX      2
#define LOCAL_HUABEI  3
#define LOCAL_ZJ  	  4

//电表协议内部定义
//863协议约定，01:97-645,02:07-645，03:62056
#define PROTOCOLNO_NULL			0
#define PROTOCOLNO_DLT645		1	//DL645
#define PROTOCOLNO_DLT645_V07	2//30	//2007版645表
#define PROTOCOLNO_DLT69845		3	//DL698.45
#ifdef EN_SBJC_V2
#define PROTOCOLNO_SBJC     	4	//水气热表
#endif
#define PROTOCOLNO_MAXNO		5	//最大的电表协议号，目前不超过40

#ifdef EN_SBJC_V2
//水气热表协议内部定义
#define PROTOCOLNO_STD_T188				0	//标准T188(默认，1f 90)：科陆水表、捷先T188(1f 90或90 1f)、金水水表、宁波水表、成星水表、蓝毅热表
#define PROTOCOLNO_F10CONFIG			1	//档案参数F10“费率个数”配置	（一个集中器支持多种协议时使用）
#define PROTOCOLNO_HUAXU_T188_MBUS		2	//华旭T188_mbus		（90 1f）
#define PROTOCOLNO_HUAXU_T188_RS485		3	//华旭T188_RS485	（90 1f）
#define PROTOCOLNO_ANSHAN_RS232			4	//鞍山RS232
#define PROTOCOLNO_MINSHENG_WIRELESS	5	//民生无线
#define PROTOCOLNO_JINGQI_645			6	//旌旗645
#define PROTOCOLNO_ZHENGTAI_T188		7	//正泰T188	（1f 90）
#define PROTOCOLNO_DENENG_T188			8	//德能T188	（90 1f）
#define PROTOCOLNO_BEILIN_T188			9	//贝林T188	（90 1f）
#define PROTOCOLNO_JSJD_MBUS			10	//江苏中科君达
#define PROTOCOLNO_JSLX_RS485			11	//江阴立信
#endif

#define	CCT_MTRPRO_97	1	//97版645
#define	CCT_MTRPRO_07	2	//07版645
#define CCT_MTRPRO_T188   3   //T188协议
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
#define AREA_VIP_MAX_NUM	1		//台区总表数
#define INVALID_POINT     0
#define ALL_POINT		  0xffff	

#define METER_TYPE_VIP		0x80
#define METER_TYPE_TYPE		0x7F


//文件文件标识
#define FILE_LOGFILE			0
#define FILE_TRANSFER_STYLE1	1
#define FILE_TRANSFER_STYLE2	2
#define FILE_TRANSFER_STYLE3	201	//FTP软件下载命令

//文件操作方式定义
#define FILE_OPERTOR_DOWN		1
#define FILE_OPERTOR_UP			2
#define FILE_OPERTOR_DELETE		3

#define GB_INVALID_VALUE		0xee

//铁电的日志文件ID
#define LOG_ENERGY		0		//交采电能
#define LOG_DEMAND		1		//交采需量
#define LOG_TERMSTAT	2		//终端统计信息
#define LOG_PULSE_ENERGY1	3		//第一路脉冲电能
//#define LOG_PULSE_DEMAND1	5//11		//第一路脉冲需量
#define LOG_ENERGY_BAR		7		//交采电能不足最小计量单位的
#define LOG_ENABLE	1 

#define INVALID_645_DATA		0xff
#define ERR_OVER_RECNUM			-20

//485口功能
#define PORT_FUN_RDMTR		0		//抄表口
#define PORT_FUN_INMTR		1		//被抄口
#define PORT_FUN_LINK		2		//级联口
#define PORT_FUN_VARCPS		3		//接无功补偿装置
#define PORT_FUN_ACQ		4		//采集口
#define PORT_FUN_JC485		5		//集抄485口.
#define PORT_FUN_DEBUG		0xFF	//debug输出(只对3口有效)

#define ALARM_LED_CTRL	0	//控制线程调用
#define ALARM_LED_645	1	//645事件线程调用

#define PROG_VALID_MIN		60		//编程开关有效时间 (单位:分钟)

#define TRANSMIT_TIME_OUT   50      //文件传输中的转发超时时间 单位s

//const static int g_iInSnToPhyPort[] = {COMM_METER, COMM_LINK, COMM_DEBUG};	

//语音信息
#define PARA_CHANGE			0	//运行参数更改
#define NEW_INFO			1	//最新信息
#define CTL_PERIOD_ENABLE	2	//时段控投入
#define CTL_PERIOD_DISNABLE	3	//时段控解除
#define CTL_REST_ENABLE		4	//厂休控投入
#define CTL_REST_DISNABLE	5	//厂休控解除
#define CTL_MONTH_ENABLE	6	//月电控投入
#define CTL_MONTH_DISNABLE	7	//月电控解除
#define CTL_BUY_ENABLE		8	//购电控投入
#define CTL_BUY_DISNABLE	9	//购电控解除
#define CTL_TMP_ENABLE		10	//下浮控投入
#define CTL_TMP_DISNABLE	11	//下浮控解除
#define PERMIT_CLOSE		12	//允许合闸
#define OVERLOAD			13	//超负荷请限电
#define ENERGY_GET_LOW		14	//电量快用完
#define ENERGY_GET_ZERO		15	//电量用完，已跳闸
#define ALR_MON_CTL			16	//月电量控制告警
#define CTL_SHUT_ENABLE		17	//营业报停控投入
#define CTL_SHUT_DISNABLE	18	//营业报停控解除
#define CTL_URG_ENABLE		19	//催费告警投入
#define ALR_URG_CTL			20	//催费告警
#define CTL_URG_DISNABLE	21	//催费告警解除
#define CTL_GUAR_ENABLE		22	//保电投入
#define CTL_GUAR_DISNABLE	23	//保电解除
#define YK_FIR_OPEN			24	//遥控第1轮跳闸
#define YK_SEC_OPEN			25	//遥控第2轮跳闸
#define YK_THI_OPEN			26	//遥控第3轮跳闸
#define YK_FOUR_OPEN		27	//遥控第4轮跳闸
#define YK_FIR_CLOSE		28	//遥控第1轮合闸
#define YK_SEC_CLOSE		29	//遥控第2轮合闸
#define YK_THI_CLOSE		30	//遥控第3轮合闸
#define YK_FOUR_CLOSE		31	//遥控第4轮合闸

#define PLC_MODULE_EXIST	0	//载波模块已安装好

#endif  //CONST2_H



 
