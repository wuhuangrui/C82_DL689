/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrHook.h
 * ժ    Ҫ�����ļ���Ҫ��������ͨ�Žӿڿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��4��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *			 $�����ﲻҪ�����inline,����Ϳ��ļ�һ�����ʱ�ض�λ
 *********************************************************************************************************/
#ifndef MTRHOOK_H
#define MTRHOOK_H
#include "apptypedef.h"
#include "Comm.h"

//�ڹ��ϵķ���/�ָ�ʱ��Ļص�����,�������ɸ澯�¼�
void On485ErrEstb(WORD wPort);		//485����ȷ��(���в�����,�ֶ˿�)
void On485ErrRecv(WORD wPort);		//485���ϻָ�(���в�����,�ֶ˿�)
void OnMtrErrEstb(WORD wPn);	//�������ȷ��(����������)
void OnMtrErrRecv(WORD wPn);	//������ϻָ�(����������)

bool IsMtrErr(WORD wPn);			//������ѯ���������״̬	
void DoMtrAnd485ErrErc();

//����:��ȡ485��������Ϣ,������ͨ���ز��ɼ����ɼ���485��
void Get485PnMask(BYTE* pbNodeMask);
const BYTE* Get485PnMask();

void GetPlcPnMask(BYTE* pbNodeMask);
const BYTE* GetPlcPnMask();

void Set485PnMask(WORD wPn);
void Clr485PnMask(WORD wPn);

void SetPlcPnMask(WORD wPn);
void ClrPlcPnMask(WORD wPn);

WORD MtrAddrToPn(const BYTE* pbTsa, BYTE bAddrLen);

#endif //MTRHOOK_H
