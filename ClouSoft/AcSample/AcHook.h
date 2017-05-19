/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcHook.h
 * ժ    Ҫ�����ļ���Ҫ�������彻�ɿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *********************************************************************************************************/
#ifndef ACHOOK_H
#define ACHOOK_H
#include "apptypedef.h"
#include "ComStruct.h"
#include "AcSample.h"

/////////////////////////////////////////////////////////////////////////
//���ɿ�Ĵ������Ҫ��׼�Ĺҹ�/�ص���������
void AcOnClrDemand(WORD wPn0, WORD wPn1);
void AcOnDayFrz(WORD wPn, const TTime& time);
bool AcIsDayFrozen(WORD wPn, const TTime& time);
void AcOnMonthFrz(WORD wPn, const TTime& time, BYTE bIdx);
bool AcIsMonthFrozen(WORD wPn, const TTime& time, BYTE bIdx);
void AcTrigerSavePn(WORD wPn);
void AcTrigerSavePn(WORD wPn0, WORD wPn1);
void AcCaluPhaseStatus(const TAcPara& rAcPara, const int* piValue, const int* piAngle, const int* piCos, BYTE* pbPhaseStatus, WORD* pwPnStatus);
void AcCaluImbalanceRate(const TAcPara& rAcPara, const int* piValue);
void AcTransSaveCurDemand(WORD wId, BYTE* pbValue);
void AcCaluAmpereHours(const TAcPara& rAcPara, const int* piValue, BYTE bOpt);

//����:�趨�ɱ�������������
//����:��
void PgmPulseInit(); 

//����:�����ɱ���������
//����:@wType ��������,Ϊ�������ڡ�ʱ��Ͷ��
void PgmPulseOut(WORD wType);
#endif //ACHOOK_H
