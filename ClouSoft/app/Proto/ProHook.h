/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProHook.h
 * ժ    Ҫ�����ļ���Ҫ��������ͨ�Žӿڿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��3��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *			 $�����ﲻҪ�����inline,����Ϳ��ļ�һ�����ʱ�ض�λ
 *********************************************************************************************************/
#ifndef PROHOOK_H
#define PROHOOK_H
#include "Modem.h"

//����ͳ���õ��Ľӿں���
void AddFlux(DWORD dwLen);	 	//�ۼ������Ľӿں���
bool IsFluxOver();	//�����Ƿ񳬹��¶�ֵ

bool GprsIsInPeriod();	//GPRS�Ƿ�������ʱ��
void GprsOnFluxOver();	//�ص�����,�������ɸ澯��¼����;
bool GprsIsTxComplete(DWORD dwStartClick); //�澯���������������Ƿ�����

void ProThrdHook(CProtoIf* pIf, CProto* pProto);
void UpdModemInfo(TModemInfo* pModemInfo);
void UpdSIMNum(TModemInfo* pModemInfo);
void UpdSIMCIMI(BYTE* pbBuf);

void UpdSysInfo(BYTE* pbBuf);
void GetSysInfo(BYTE* pbBuf);
BYTE GetNetStandard(void);

#endif //PROHOOK_H
