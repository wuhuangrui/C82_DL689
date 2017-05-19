/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ISD4002.c
 * ժ    Ҫ��
 *			 ����оƬ(ISD4002)����
 *
 * ��ǰ�汾��1.0.0
 * ��    �ߣ����
 * ������ڣ�2009-08-05
 * ժ    Ҫ��
 *
*********************************************************************************************************/
#ifndef _ISD4002_H_
#define _ISD4002_H_
#include "bios.h"
#include "sysarch.h"
#include "sysapi.h"

#define INVALID_HANDLE		0xFF

class CISD4002
{
private:
	int m_iHanlde;
	int m_iIntFd;
	bool m_fPlaying;
	DWORD m_dwLaskClick;
	TSem m_semISD4002;
	DWORD 	m_dwIndex[32];

protected:
	unsigned int m_uiPlayIndex;
	unsigned int m_uiLoopTimes;

public:
	CISD4002();
	~CISD4002();
	void Init(DWORD* pdwIndex, int iSize);
	void Close(void);
	void SendCmd(unsigned short usCmd);
	void StartLoopPlay(unsigned char ucIndex, unsigned int uiClick=600);
	bool Play(unsigned char ucIndex=INVALID_HANDLE, bool fImmediately=false);
	void DoLoopPlay();
	bool Stop();
	void StopLoopPlay();
	bool IsPlaying() { return m_fPlaying; };
	bool IsPlayComplete();
	void SetDenyTime(WORD wClick);
	unsigned int GetLoopPlayIndex() { return m_uiPlayIndex; };
	bool IsNeedLoopPlay() { return ((m_uiPlayIndex!=INVALID_HANDLE) && (GetClick()<m_uiLoopTimes));};
};
#endif /* _ISD4002_H_ */
