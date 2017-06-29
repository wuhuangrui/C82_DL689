#ifndef FASTRUCT_H
#define FASTRUCT_H

#include "ComStruct.h"
#include "DbStruct.h"
#include "ThreadMonitor.h"
#include "TermEvtTask.h"

typedef struct{
	WORD wVer;              //�汾 
	bool fTmpValid;         //�����ݴ������Ч��־
	bool fAlrPowerOff;		//����ǰ�ϱ���ͣ��澯
	WORD wRstNum;			//��λ����
	WORD wMonitorRstNum;	//�̼߳�ظ�λ����
	char szMonitorRstThrd[THRD_NAME_LEN];	//�̼߳�ظ�λ���һ�θ�λ���߳�����
	TTime tPoweroff;        //�ϴ�ͣ��ʱ��
	TTime tPowerOn;			//�ϴ��ϵ�ʱ��
	TAllVLoss tAllVLoss;	//ȫʧѹ˽�б���
}TPowerOffTmp;   //�����ݴ����

#endif //FASTRUCT_H

