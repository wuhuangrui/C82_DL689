/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TaskConst.h
 * 摘    要：本文件主要用来定义任务常量
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年10月
 *********************************************************************************************************/
#ifndef TASKCONST_H
#define TASKCONST_H

#define TASK_NUM   		256

//任务配置单元、采集方案内存映射：
#define TASK_ID_NUM		256
#define SCH_NO_NUM		256
#define TASK_ID_MASK	(TASK_ID_NUM/8+1)

#define TASK_CFG_LEN			512		//任务配置长度
#define TASK_CFG_REC_LEN		(TASK_CFG_LEN+10)		//记录长度包括多个字段, 这里预留了几个字节(预留长度小于10)


#define TASK_MONIINDEX_ID		0
#define TASK_MONIINDEX_STAT		1
#define TASK_MONIINDEX_STARTIME	2
#define TASK_MONIINDEX_ENDTIME	3
#define TASK_MONIINDEX_RDTOTAL	4
#define TASK_MONIINDEX_SUCNUM	5
#define TASK_MONIINDEX_SENDNUM	6
#define TASK_MONIINDEX_RCVNUM	7
#endif //TASKCONST_H

