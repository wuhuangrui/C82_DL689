/*
**********************************************************************************************
* Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
* 
* �ļ����ƣ�DataProc.h
* ժ    Ҫ: ���ļ��ṩһ�����ݼ��м����ݵ�ʵ��
* ��ǰ�汾��1.0
* ��    �ߣ�������
* ������ڣ�2008��3��
* ��    ע��
**********************************************************************************************
*/
#ifndef   DATAPROC_INCLUDED
#define   DATAPROC_INCLUDED

#include  "ComStruct.h"
#include  "Trace.h"
#include  "DbConst.h"


#define BLOCK_ITEMNUM		(RATE_NUM+2)	//���������ĵ��ܿ�������Ŀ��

#define		INTVCAL		1	//������
#define		INTV15M  	2	//15���Ӽ��
#define		INTVDAY		3	//�ռ��
#define		INTVMON		4	//�¼��

#define		NOCHANGE	0
#define		DAYCHANGE	1
#define		MONCHANGE	2

typedef struct{
	DWORD dwS;	 //�����ʼʱ��
	DWORD dwEndS;//�������ʱ��	
}TIntvSec; //�������ݵ�ʱ����ֹ����ṹ

//���������
class CDataProc
{
public:
	CDataProc(void);
	virtual ~CDataProc(void);

	//����������
	bool virtual Init(WORD  wPn) { return false; };
	void virtual DoDataProc(){};

	void TimeToIntvS(TTime time, BYTE bInterv, TIntvSec* pIntervS);
	int  DataDelta( int64* iResVal, int64 iSrcVal1, int64 iSrcVal2);
	bool DataSum( int64* iResVal, int64 iSrcVal1, int64 iSrcVal2, BYTE bFlag);
	void ClsBlockE(WORD wBank, WORD wID, int64* piValBuf, int64 iDstVal, DWORD dwSec);

public:	
	WORD m_wPn;      //������ţ����ܼ���ţ� 	
	BYTE m_bPnProp;  //����������
	BYTE m_bMtrIntv; //������ĳ�����ʱ�� 		
	BYTE m_bRateNum; //ʵ�ʷ�����
	
};


#endif
