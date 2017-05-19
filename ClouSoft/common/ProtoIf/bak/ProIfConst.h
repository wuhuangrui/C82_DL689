/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProIfConst.h
 * ժ    Ҫ�����ļ���Ҫ��������ͨ�Žӿڵĳ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��12��
 * ��    ע��
 *********************************************************************************************************/
#ifndef PROIFCONST_H
#define PROIFCONST_H

//GPRS��������
#define GPRS_STEP_IDLE		0	//����
#define GPRS_STEP_RST		1	//��λģ��
#define GPRS_STEP_APN		2	//��ʼ��APN
#define GPRS_STEP_SIM		3	//���SIM
#define GPRS_STEP_REG		4	//ע������
#define GPRS_STEP_SIGN		5	//�����ź�ǿ��
#define GPRS_STEP_DIAL		6	//���Ž�����
#define GPRS_STEP_AUTH		7	//PPP��֤
#define GPRS_STEP_IP		8	//��ȡIP������
#define GPRS_STEP_PPP		9	//pppЭ��
#define GPRS_STEP_ONLINE	10	//GPRS����
#define GPRS_STEP_SMS		11	//��������

//����״̬
#define GPRS_ERR_UNK   	   -1	//��û�г�ʼ��
#define GPRS_ERR_OK			0	//û�д���
#define GPRS_ERR_RST		1	//��λģ��ʧ��
#define GPRS_ERR_SIM		2	//���SIM��ʧ��
#define GPRS_ERR_REG		3	//ע������ʧ��
#define GPRS_ERR_PPP		4	//����ʧ��
#define GPRS_ERR_AUTH		5	//PPP��֤ʧ��
#define GPRS_ERR_IP			6	//��ȡIPʧ��
#define GPRS_ERR_CON		7	//������վʧ��

#endif //PROIFCONST_H
