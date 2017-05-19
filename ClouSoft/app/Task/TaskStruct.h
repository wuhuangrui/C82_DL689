/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TaskStruct.h
 * ժ    Ҫ�����ļ���Ҫ���������������ݽṹ
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��10��
 *********************************************************************************************************/
#ifndef TASKSTRUCT_H
#define TASKSTRUCT_H
#include "apptypedef.h"
#include "TaskConst.h"
#include "MeterStruct.h"

#define TERM_MEM_SIZE 	MTR_MEM_SIZE//8192		//�ն˶�̬�ڴ��С
#define TERM_TAB_NUM		64			//�ն˶�̬�ڴ��������
typedef struct 
{
	BYTE  bGlobal[ALLOC_TAB_LEN];	//ȫ���ڴ�����
	//�൱��allocTab[]�Ļ�
	TAllocTab allocTab[TERM_TAB_NUM];//��̬�ڴ�����
	BYTE bMem[TERM_MEM_SIZE];	//��̬�ڴ����ռ�
}TTermMem; //�ն˶�̬�ڴ���ƽṹ

#endif //TASKSTRUCT_H