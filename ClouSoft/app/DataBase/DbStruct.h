/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbStruct.h
 * ժ    Ҫ�����ļ���Ҫ����������汾��������ݽṹ
 * ��ǰ�汾��1.1
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 *********************************************************************************************************/
#ifndef DBSTRUCT_H
#define DBSTRUCT_H
#include "sysarch.h"
#include "LibDbStruct.h"

typedef struct{
	BYTE  bValid:1;        				//��Ч�ı�־  
	BYTE  bNotFix:7;					//�����Ƿ�̶��ı�־
	BYTE  bBank;        				//��ӦBank��ʶ    
    WORD  wID;         					//��ӦID��ʶ  
	BYTE  bType;						//PN����  
	WORD  wPNMax;						//PN����  
}TConvertFNIDDesc;

#define PARSE_MAX	64
typedef struct{
	//�����ṹ�ľ�����������
	BYTE* 	pbCfg;				//�����ṹ�ľ�����������
								//����ͨ�ɼ���������¼��ѡ��  array CSD
								//�ľ�����������
	WORD wCfgLen;				//�����ṹ�ĳ���

	//�������ֶν����Ľ��
	WORD 	wNum;					//�����������ֶθ���
	WORD 	wPos[PARSE_MAX];			//�ֶε���ʼλ��
	WORD	wLen[PARSE_MAX];			//�ֶε����ݳ���
	BYTE	bType[PARSE_MAX];		//�ֶ���������

	//���õ��ֶν�һ�������ɶ�Ӧ���������Ϣ
	WORD  wTotalLen;				//������������ܳ���
	WORD  wItemOffset[PARSE_MAX];	//�������ڼ�¼�е�ƫ�ƣ�
	WORD  wItemLen[PARSE_MAX];		//�������ڼ�¼�еĳ��ȣ�
	//ͨ��OoGetDataLen()ȡ��
}TFieldParser;	//�ֶν�����

#endif //DBSTRUCT_H
