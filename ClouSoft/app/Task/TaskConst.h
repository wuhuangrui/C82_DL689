/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TaskConst.h
 * ժ    Ҫ�����ļ���Ҫ��������������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��10��
 *********************************************************************************************************/
#ifndef TASKCONST_H
#define TASKCONST_H

#define TASK_NUM   		256

//�������õ�Ԫ���ɼ������ڴ�ӳ�䣺
#define TASK_ID_NUM		256
#define SCH_NO_NUM		256
#define TASK_ID_MASK	(TASK_ID_NUM/8+1)

#define TASK_CFG_LEN			512		//�������ó���
#define TASK_CFG_REC_LEN		(TASK_CFG_LEN+10)		//��¼���Ȱ�������ֶ�, ����Ԥ���˼����ֽ�(Ԥ������С��10)


#define TASK_MONIINDEX_ID		0
#define TASK_MONIINDEX_STAT		1
#define TASK_MONIINDEX_STARTIME	2
#define TASK_MONIINDEX_ENDTIME	3
#define TASK_MONIINDEX_RDTOTAL	4
#define TASK_MONIINDEX_SUCNUM	5
#define TASK_MONIINDEX_SENDNUM	6
#define TASK_MONIINDEX_RCVNUM	7
#endif //TASKCONST_H

