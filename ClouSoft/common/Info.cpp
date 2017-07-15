/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Info.cpp
 * ժ    Ҫ�����ļ�ʵ��Ӧ�ò���̼߳���Ϣͨ�ŵĻ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��ע��֪ͨ��Ϣ��ԭ��
 * 		1.�����ڲ�����ϵͳ״̬�����ı�ʱ,֪ͨĳ���߳�����Ӧ����
 �� 		3.�����߶�ΰ���Ϣ�ó�true,�Խ�������˵�൱��ֻ�յ�һ����Ϣ
 �� 		4.��һ����ϢҪ��������߳�ʱ,Ӧ��ÿ���̶߳�����һ����Ϣ�ı�ʶ
 *********************************************************************************************************/
#include "stdafx.h"
#include "ComConst.h"
#include "FaConst.h"
#include "Info.h"
#include "sysarch.h"
#include "sysapi.h"

static DWORD g_dwInfoClick;
static TSem g_semInfo;
static bool g_fInfo[INFO_NUM];
static BYTE g_bInfoDelayFlg[INFO_NUM/8+1];

void InitInfo()
{
	g_dwInfoClick = 0;
	g_semInfo = NewSemaphore(1);
	memset(&g_fInfo, 0, sizeof(g_fInfo));
	memset(&g_bInfoDelayFlg, 0, sizeof(g_bInfoDelayFlg));
}

//����:����֪ͨ��Ϣ
//����:@wID ��Ϣ��ʶ
//	   @fInfo ��Ϣֵ
void SetInfo(WORD wID, bool fInfo)
{
	WaitSemaphore(g_semInfo);
	
	if (wID < INFO_NUM)
		g_fInfo[wID] = fInfo;
	
	SignalSemaphore(g_semInfo);	
}

//����:������ʱ�����֪ͨ��Ϣ,��Ҫ������ݿ���ĵĲ�����ͨ��Э���޸���,
//	   ���޸ĺ������ϾͰ���Ϣ���͸��߳�,������ͨ��Э�鲻���޸Ĳ�����,
//	   �ٰ���Ϣ���͸��߳�
//	   ��ʱ�ַ�Ϊ����ʱ�Ͷ���ʱ
//����:@wID ��Ϣ��ʶ
void SetDelayInfo(WORD wID)
{
	if (wID < INFO_NUM)
	{
		DWORD dwClick = GetClick();

		WaitSemaphore(g_semInfo);

		g_fInfo[wID] = true;
		g_dwInfoClick = dwClick;	//��ʹ����ʱ,ʱ�仹�Ǹ���
		if (wID < INFO_NO_DELAY_START)
			g_bInfoDelayFlg[wID/8] |= 1 << (wID%8);
		else if (wID==INFO_MTR_ALL_CLEAR || wID==INFO_TASK_CFG_UPDATE || wID==INFO_ACQ_SCH_UPDATE ||
			wID==INFO_RP_SCH_UPDATE || wID==INFO_CLASS19_METHOD_DATA_INIT || wID==INFO_MTR_INFO_UPDATE)
			g_bInfoDelayFlg[wID/8] |= 1 << (wID%8);
		
		SignalSemaphore(g_semInfo);
	}
}


//����:ȡ��֪ͨ��Ϣ,�����ϢΪtrue,���Զ�����Ϣ���false
//����:��Ϣֵ
bool GetInfo(WORD wID)
{
	bool fRet = false;
	
	if (wID < INFO_NUM)
	{
		DWORD dwClick = GetClick();
		
		WaitSemaphore(g_semInfo);
		
		WORD wIndex = wID / 8;
		BYTE bFlg = 1 << (wID%8);
		if (g_bInfoDelayFlg[wIndex] & bFlg) //��������ʱ�����֪ͨ��Ϣ
		{
			if (g_fInfo[wID])
			{	
				if (wID < INFO_SHORT_DELAY_START)
				{	
					if (dwClick-g_dwInfoClick > INFO_LONG_DELAY) //�ȵ�����ʱʱ�䵽�Ű���Ϣ��Ӧ���߳�
						fRet = true;
				}
				else if (wID < INFO_NO_DELAY_START)
				{	
					if (dwClick-g_dwInfoClick > INFO_SHORT_DELAY) //�ȵ�����ʱʱ�䵽�Ű���Ϣ��Ӧ���߳�
						fRet = true;
				}
				else if (wID==INFO_MTR_ALL_CLEAR || wID==INFO_TASK_CFG_UPDATE || wID==INFO_ACQ_SCH_UPDATE ||
					wID==INFO_RP_SCH_UPDATE || wID==INFO_CLASS19_METHOD_DATA_INIT || wID==INFO_MTR_INFO_UPDATE)
				{
					if (dwClick-g_dwInfoClick > INFO_SHORT_DELAY) //�ȵ�����ʱʱ�䵽�Ű���Ϣ��Ӧ���߳�
						fRet = true;
				}
				else
				{
					fRet = true;
				}
			}
		}
		else if (g_fInfo[wID]) //û��������ʱ�����֪ͨ��Ϣ
		{					   //�����ύ	
			fRet = true;
		}
		
		if (fRet)
		{
			g_bInfoDelayFlg[wIndex] &= ~bFlg;
			g_fInfo[wID] = false;
		}
		SignalSemaphore(g_semInfo);
	}
	
	return fRet;
}
