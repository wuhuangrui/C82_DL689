#ifndef FASTRUCT_H
#define FASTRUCT_H

#include "ComStruct.h"
#include "DbStruct.h"
#include "FaConst.h"
#include "ThreadMonitor.h"
#include "TermEvtTask.h"

typedef struct{
	DWORD time;			//����ʱ��
	BYTE bVerInfo[10];	//�汾��Ϣ
}TSoftVerChg;		//����İ汾����¼�

typedef struct{
	DWORD time;			//����ʱ��
	WORD wNum;
	WORD wClass[MAXNUM_ONEERC3];
	BYTE bObis[MAXNUM_ONEERC3][6];	//һ��Ӧ�÷������ͬʱ����50��OBIS����
}TParaChg;			//����Ĳ�������¼�

typedef struct{
	WORD wVer;              //�汾 
	bool fTmpValid;         //�����ݴ������Ч��־
	bool fGPRSConected;     //GPRS������
	BYTE bRemoteDownIP[8];  //Զ����������ķ�����IP��ַ
	bool fAlrPowerOff;		//����ǰ�ϱ���ͣ��澯
	WORD wRstNum;			//��λ����
	WORD wMonitorRstNum;	//�̼߳�ظ�λ����
	char szMonitorRstThrd[THRD_NAME_LEN];	//�̼߳�ظ�λ���һ�θ�λ���߳�����
	short iRemoteDownPort;
	TTime tPoweroff;        //�ϴ�ͣ��ʱ��
	TTime tPowerOn;			//�ϴ��ϵ�ʱ��
	BYTE  bParaEvtType;		//��¼����ǰ������ʼ���¼������ͣ�0��Ч 1��Ҫ 2һ��
	TSoftVerChg ParaInit;	//������ʼ���¼�
	TAllVLoss tAllVLoss;	//ȫʧѹ˽�б���
	//TPowOffBase tPowOffBase;			//�ն˵��ϵ�״̬
}TPowerOffTmp;   //�����ݴ����

typedef struct{
	bool fInit;					//���ṹ�ѳ�ʼ��
	WORD wTaskID;				//�����ʶ
	WORD wMaxFrmBytes;			//ÿ֡�������ܷŵ��ֽڸ���(ʣ�¸�����¼�Ŀռ��С)
	BYTE bFrmNum;				//���ζ���������������Ҫ�ָ���֡��
	WORD wMeterNum;				//Ҫ���ĵ����
	WORD wDayNum;				//Ҫ��������/���з���Ҫ���¼������
	WORD wRecLenOneMeter;		//ÿ�ʼ�¼�ĳ���
	WORD wRecsPerDay;			//ÿ��ÿ�����Ҫ�ж��ٱʼ�¼
	TTime tmStartDate;			//Ҫ��������
	WORD wRecStart;				//����Ҫ���¼����ʼλ��
	WORD wRecsFound;			//����Ҫ���¼������(�����һ�������˵)
	WORD wRecsInvalid;			//������Щ���һ�컹�м���û������ص㻧����,��Ҫ������Ч���ݵļ�¼����
	WORD wFrmStartPoint[0x100+2]; //ÿ֡����ʼ�������,���һ����������0xffff,��ʾ���һ֡һֱ���������һ��������
}TReadTaskCtrl;					  //���������ݵĿ��ƽṹ

typedef struct{
	bool fFirst;	    //���ҿ�ʼ
	bool fFinal;		//���ҽ���
	WORD wTxRecLen;		//���͵�ÿ����¼�ĳ���
	bool fAllMeter;		//�㲥��ַ
	WORD wItemIndex;	//Ҫ���ҵ�ID�ڱ������ID���ñ������
	WORD wSortIndex;	//�Ѿ����ҵ�����������һ��
	WORD wMeterIndex;	//��������
	WORD wRecsFound;	//�����ҵ��ļ�¼����
}TReadTaskCtrlJx;		//���������ݵĿ��ƽṹ
#endif //FASTRUCT_H

