/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterConst.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֳ�������������弰���ó�������
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��9��
 *********************************************************************************************************/
#ifndef METERCONST_H
#define METERCONST_H
#include "syscfg.h"

//������������ȼ�����4��
#define MTR_PRIO_FIRST			1	//��Ҫ
#define MTR_PRIO_SECOND			2	//��Ҫ
#define MTR_PRIO_THREE			3	//��Ҫ
#define MTR_PRIO_FOUR			4	//����

//����������
#define RD_ERR_UNKNOWN		-3		//δ֪��ʶ
#define RD_ERR_HALT			-2		//��ͣ����
#define RD_ERR_UNTIME		-1		//ʱ��δ��
#define RD_ERR_OK			0		//�޴�����ȫ����
#define RD_ERR_UNFIN		1		//û����
#define RD_ERR_PWROFF		2		//ͣ��
#define RD_ERR_485			3		//485�������
#define RD_ERR_PARACHG		4		//���������
#define RD_ERR_INTVCHG		5		//���������
#define RD_ERR_DIR			6		//����ֱ��
#define RD_ERR_STOPRD		7		//ֹͣ����
#define RD_ERR_RDFAIL		8		//����ʧ��
#define RD_ERR_CHKTSK		9		//�Ƿ���ȫ��������ķ���ֵ

const static int g_iInSnToPhyPort[] = {COMM_LINK, COMM_METER};	

//ע��:������Ķ���Ͳ�Ҫ�ٸ���,�����������,
// 	   ���Ǿ͹̶���Ϊ�߼��˿ڸ����ϵ�ϰ��,�Ǵ���->���˳��ʼ���1,2...
#define LOGIC_PORT_NUM	(sizeof(g_iInSnToPhyPort)/sizeof(int))

#define LOGIC_PORT_MIN	1									//��С���߼��˿ڶ���
#define LOGIC_PORT_MAX	(LOGIC_PORT_MIN+LOGIC_PORT_NUM-1)	//�����߼��˿ڶ���

#endif //METERCONST_H
