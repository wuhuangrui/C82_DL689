 /*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FapLink.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ּ�����������ն˵Ŀ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��8��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 * ��ע:1.g_FapLink��g_FaProto�Ľ�����ʽ:ͨ��CFapLink�ĳ�Աm_Queue��ȫ�ֱ���
 *		  g_Queue,����Ҫ�Է����͵ı��ļ��뵽������ȥ
 *		2.���ն�����ն˵��л�:����վ��ַ�ĸı䵼����/���ն˷����л���ʱ��,
 *		  ���������ն��߳�void CFapLink::DoMasterThread()��������,��Ϊ������
 *		  ����IsMasterTerm()������ʲô�������,ʲô���鲻����
 *		3.���������ĸ���:��ReInit()��̬ˢ�²���
*********************************************************************************************************/

#include "stdafx.h"
#include "FaCfg.h"
#include "FaAPI.h"
#include "Queue.h"
#include "FaProto.h"
#include "ProPara.h"

CQueue g_LinkDownQueue;       //ת�������ն˵ı��Ķ���
CQueue g_LinkUpQueue;           //ת������վ�ı��Ķ���
bool g_fEnableSlaveReport = false;      //�Ƿ����������ϱ�
DWORD g_dwSlaveAddr[FAP_LINK_SLAVE_NUM];  //���ն˵�ַ
/////////////////////////////////////////////////////////////////////////////////////////////////////
//CFapLink

//����:���ռ���ͨ������,Ϊ����Ҫ�Ĵ��ն�ת�������ϱ��ı���
void LinkOneSlave(BYTE index,CFaProto* pProto)
{
	//����Ӧ��֡���ж��Ƿ�ת����վ
	//ѭ����վ���ն�֮��ı��ķ��ͣ�
	//ֱ����ʱ�����ն�Ӧ���������ϱ�
	TFapMsg* pMsgMaster;
	//������նˣ��������������ϱ���������
	//pProto->MakeLinkFrm(0,g_dwSlaveAddr[index]);
	//����Ӧ��֡
	int i=0;
	g_fEnableSlaveReport=true;
	while ( (i< 10)&&g_fEnableSlaveReport)
	{
		Sleep(1000);
		pProto->RcvFrm(); //���յ���һ֡,���Ѿ�������д���
		pMsgMaster = (TFapMsg* )(g_LinkDownQueue.Remove(1));   //�����Ƿ�����վ�Ļ�֡
		if (pMsgMaster != NULL)
		{
			DTRACE(DB_FAPROTO, ("CFapLink::LinkOneSlave : forward a frm\n"));
			pProto->Send(pMsgMaster->bTxBuf, pMsgMaster->wTxLen);  //����վ�Ļ�֡ת�������ն�
			delete pMsgMaster;
		}
		i++;
	}
	DTRACE(DB_FAPROTO, ("CFapLink::LinkOneSlave : forward a frm,g_fEnableSlaveReport=%d\n",g_fEnableSlaveReport));
 	return;
}

//����:�м���Щ��վ�����ն˵�ͨ��
void DoForward(CFaProto* pProto)
{
	TFapMsg* pMsgMaster;
	//ѭ��3�δӽ��ն��У�������վ�ı���
	//����վ�ı���ת�����ն�
	//�����ն˵ı���
	//���ն˵ı��ķŵ����Ͷ��У�ת������վ
	for (int i=0;i<3;i++)
	{
		pMsgMaster = (TFapMsg* )(g_LinkDownQueue.Remove(1));   //�����Ƿ�����վ�Ļ�֡
		if (pMsgMaster != NULL)
		{
			DTRACE(DB_FAPROTO, ("CFapLink::DoForward : forward a frm\n"));
			pProto->Send(pMsgMaster->bTxBuf, pMsgMaster->wTxLen);  //����վ�Ļ�֡ת�������ն�
			delete pMsgMaster;
		}
		pProto->RcvFrm(); //���յ���һ֡,���Ѿ�������д���
	}
	return ;
}

//����:���м��������ն��߳�
TThreadRet DoMasterThread(void* pvArg)
{
	DWORD dwLinkClick[FAP_LINK_SLAVE_NUM];
	CFaProto* pProto = (CFaProto* )pvArg;
	CProtoIf* pIf = pProto->GetIf();
	memset((BYTE*) dwLinkClick, 0, sizeof(dwLinkClick));
	DTRACE(DB_FAPROTO, ("DoMasterThread : started!\n"));
	while (1)
	{		
		if (pIf->m_fExit) break;

		Sleep(1000);				
		if (GetInfo(INFO_LINK))	
		{
			LoadLinkTermPara();
		}

		if (IsMasterTerm() == false)	//���ն˼����� �������ն˵Ĳ�ѯ���ϴ����ݸ����ն�
		{
			pProto->RcvFrm(); //���յ���һ֡,���Ѿ�������д���
			pIf->AutoSend();
			continue;
		}

		//���ն˼�����
		DoForward(pProto); //�м���Щ��վ�����ն˵�ͨ��
		for (BYTE i=0; i<FAP_LINK_SLAVE_NUM; i++) //�����ն������ϱ�����
		{
			if (g_dwSlaveAddr[i] == 0)	//��Ч��ַ
				continue;	//break;
			if (GetClick()-dwLinkClick[i] > g_wLinkInterv)
			{
				dwLinkClick[i] = GetClick();
				DTRACE(DB_FAPROTO, ("DoMasterThread : LinkOneSlave!\n"));
				LinkOneSlave(i,pProto); //���ռ���ͨ������,Ϊ����Ҫ�Ĵ��ն�ת�������ϱ��ı���
			}
		}
	}
	return 0;
}
