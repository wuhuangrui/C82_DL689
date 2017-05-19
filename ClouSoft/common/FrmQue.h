/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FrmQue.h
 * ժ    Ҫ�����ļ���Ҫ����������CFrmQue(֡����)
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��11��
 * ��    ע��CFrmQue(֡������)�Ĺ���������CQueue(������),Ϊ�첽���̼߳��
 *           ���ݽ����ṩ����
 *           ��Ҫ����������:
 *			 CQueueΪ������е�ÿ�������ṩ����һ��ָ��
 *           CFrmQueΪ������е�ÿ�������ṩһ���̶���С�Ļ�����(frame),
 *			 ��Ҳ����������ΪCFrmQue������,���frame����һ����ָ����,����
 *			 ��Ϣ�����ݵȸ��㷺������
 *********************************************************************************************************/
#ifndef FRMQUE_H
#define FRMQUE_H
#include "apptypedef.h"
#include "sysarch.h"

class CFrmQue
{
public:
	CFrmQue();
	virtual ~CFrmQue();
	bool Init(WORD wMaxFrms, WORD wFrmSize);
    bool Append(BYTE* pbFrm, WORD wLen, DWORD dwMilliseconds);
    WORD Remove(BYTE* pbFrm, DWORD dwMilliseconds);
	void RemoveAll();
	int GetMsgNum() { return m_wFrmNum; };
	bool IsFull() { return m_wFrmNum==m_wMaxFrms; };
		
protected:
    TSem m_hsemMail;
    TSem m_hsemSpace;
    TSem m_hmtxQ;

	WORD m_wFrmSize;		//ÿ֡����ܻ�����ֽ���
	WORD m_wMaxFrms;		//����ܻ����֡����
	WORD m_wFrmNum;			//ʵ�ʻ����֡����
    WORD m_wFirst;
	WORD m_wLast;
	
    BYTE*  m_pbFrms;		//�������һ������֡
    WORD*  m_pwFrmBytes;	//ÿ��֡��ʵ���ֽ���
};

#endif //FRMQUE_H

