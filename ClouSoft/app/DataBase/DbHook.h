/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbHook.h
 * ժ    Ҫ�����ļ���Ҫ��������ϵͳ��Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *********************************************************************************************************/
#ifndef DBHOOK_H
#define DBHOOK_H
#include "apptypedef.h"

/////////////////////////////////////////////////////////////////////////
//ϵͳ��Ĵ������Ҫ�Ĺҹ�/�ص���������
bool IsPnValid(WORD wPn);
WORD* CmbToSubID(WORD wBn, WORD wID);
int PostWriteItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, BYTE bPerm, BYTE* pbPassword, int nRet);
int PostReadItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet);
int PostReadCmbIdHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime, int nRet);
int PostReadItemValHook(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD dwTime, int nRet);
int PostReadItemVal64Hook(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD dwTime, int nRet);
bool PswCheck(BYTE bPerm, BYTE* pbPassword);
int PermCheck(TItemDesc* pItemDesc, BYTE bPerm, BYTE* pbPassword);

//����:���ݶ��Ƿ���Ųɼ�������(���罻��,����)��ʵʱ����,
//	   ���ݿ��ڴ�ʱ���ȡ�ɼ�������ĸö�������ʱ,���Զ���ʱ��ʧЧ
//	   ��ʵ�ֽ����������ݵ�ʱ�겻��,Ĭ�ϵ�ǰ���ݾ������·��ϵ�����
bool IsSectHaveSampleData(WORD wSect);
BYTE GetInvalidData(BYTE bErr=ERR_OK); 	//��ȡ��ϵͳ����Ч���ݵĶ���
bool IsInvalidData(BYTE* p, WORD wLen);	//�Ƿ�����Ч���ݣ���Ч���ݿ��ܴ��ڶ��ֶ���
void GetMtrProCfgPath(char* pbCfgPath);//��ȡ��ϵͳ�ĵ�������ļ���·��
void DefaultUpgFun(WORD wFrmBn, WORD wFrmId, WORD wToBn, WORD wToId, int iPnNum);
BYTE GetDbInvalidData(); //��ȡ��ϵͳ����Ч���ݵĶ���

/////////////////////////////////////////////////////////////////////////
//��ʵ�ֹҹ�/�ص�����ʱ��Ҫ���ⶨ��ĺ���
WORD CmbToSubIdNum(WORD wBn, WORD wID);


#endif //DBHOOK_H

