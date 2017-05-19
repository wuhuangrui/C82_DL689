/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ISD4002.h
 * ժ    Ҫ��
 *			 ����230MHz��̨(ND250A)��̡��ٿؼ�������·(ISD4002)����
 *			 MCU: ATMEGA48, Crystal: 11.0592 MHz
 *
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2007-01-25
 * ժ    Ҫ��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 * ժ    Ҫ��
*********************************************************************************************************/
#ifndef _ISD4002_H_
#define _ISD4002_H_
#include "bios.h"

#define INVALID_HANDLE		0xFF

class CISD4002
{
private:
	bool m_fPlaying;
	DWORD m_dwLaskClick;
protected:
	unsigned int m_uiPlayIndex;
	unsigned int m_uiLoopTimes;

public:
	CISD4002();
	~CISD4002();
	void Init(void);
	void Send8BitCmd(unsigned char ucCmd);
	void Send16BitCmd(unsigned short ucCmd);
	bool Forward(unsigned char ucIndex);
	void StartLoopPlay(unsigned char ucIndex, unsigned int uiClick=600);
	bool Play(unsigned char ucIndex=INVALID_HANDLE, bool fImmediately=false);
	bool Stop();
	void StopLoopPlay();
	bool IsPlaying() { return m_fPlaying; };
	bool IsPlayComplete();
	void SetDenyTime(WORD wClick);
	bool IsNeedLoopPlay() { return false;};
};
#endif /* _ISD4002_H_ */
