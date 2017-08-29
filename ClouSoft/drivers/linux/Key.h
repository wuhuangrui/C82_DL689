#ifndef 	KEY_H
#define 	KEY_H

#include "bios.h"

//底层硬件I/O口定义
#define    UPKEY				0x01
#define    LEFTKEY				0x02
#define    RIGHTKEY				0x04
#define    DOWNKEY				0x08
#define    EXITKEY				0x10
#define    OKKEY				0x20


//按键键值宏定义4上翻，1下翻，2进入，3退出
#define    KEYUP						1
#define    KEYLEFT						2
#define    KEYRIGHT						3
#define    KEYDOWN          			4
#define    KEYEXIT          			5
#define    KEYOK            			6
#define    KEYOKESC                     7

#define	   LONGKEY			0x100

#define    LKEYUP			(LONGKEY+KEYUP)
#define    LKEYLEFT			(LONGKEY+KEYLEFT)
#define    LKEYRIGHT		(LONGKEY+KEYRIGHT)
#define    LKEYDOWN         (LONGKEY+KEYDOWN)
#define    LKEYEXIT         (LONGKEY+KEYEXIT)
#define    LKEYOK           (LONGKEY+KEYOK)


#define    LONGCLICKINTERVAL	600

//#define    LKEYLEFT         11
//#define    LKEYRIGHT        12
			
//#define    KEYDATABUSADD	BASE_ADR_KEY

#define KEY					0x10
#define DOUBLECLICKINTERVAL	1100
#define LONGPRESS			5000

#define KEYBUFLEN	10


//按键类
class CKey
{
private:
	int m_iMonitorID;
	WORD m_wKeyBuf[KEYBUFLEN];	//接收按键信息的缓冲区，0无按键，1单击，2双击，3长按
	BYTE m_bKeyHead;
	BYTE m_bKeyTail;
	BYTE m_bPreKeyType;
	int  m_iKeyFd;
	BYTE m_bLastPtr;
	BYTE m_bPressing;
	BYTE m_bPressPtr;
	DWORD m_dwKeyPressed;
	BYTE m_bKeyUpShake;
	BYTE m_bKeyDownShake;

	
public:
    CKey(void);
    ~CKey(void);
    bool Init(void);
    bool Close(void);
	WORD GetKey(void);	//取按键
	void SetKey(WORD bKeyInf);	//设置按键	
	DWORD GetKeyValue(void);    //扫描键值
	void MonitorKey(void);		//监视按键
	void FastGetKey(void);		//快速监视按键
	
	BYTE KeyValue2Type(DWORD dwKeyVal); // 
	
	int ReqMonitorID();
};

extern CKey  g_Key;		//定义一个按键对象

#endif
