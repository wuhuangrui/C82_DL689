#ifndef DCSTRUCT_H
#define DCSTRUCT_H
#include "apptypedef.h"
#include "LibDbStruct.h"

typedef void (* TPfnDcValToFmt)(int val, BYTE* pbBuf, WORD wLen);

typedef struct{
	bool fDuoPn;  	//���������Ƿ�֧��˫����������,
					//��Ҫ����Թ��꽻�ɵĲ�����ſ�������,�̶��������0,����һ��������������
					//����BANK����չ��������,���ڶ����Լ��õ�������,����һ�㶼��֧��˫������
	WORD wBn;  		//BANK��
	WORD wID;     	//������ID,
					//���Ϊ��ID,��wInnerID��Ϊ��һ��ID������,wSubNumΪ��ID�ĸ���
	WORD wIdx;		//�ڲ����������
	WORD wSubNum;	//��ID�ĸ���
	WORD wLen;		//����������ĳ���
	TPfnDcValToFmt pfnDcValToFmt;	//��ʽת������
	
	//���²����ɳ����Զ���ʼ��
	TDataItem diPn0;
	TDataItem diPn;
}TDcValToDbCtrl;	//��������������

#endif
