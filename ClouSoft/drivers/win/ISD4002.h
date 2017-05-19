/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ISD4002.h
 * 摘    要：
 *			 处理230MHz电台(ND250A)编程、操控及语音电路(ISD4002)控制
 *			 MCU: ATMEGA48, Crystal: 11.0592 MHz
 *
 * 当前版本：1.0
 * 作    者：刘春华
 * 完成日期：2007-01-25
 * 摘    要：
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 摘    要：
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
