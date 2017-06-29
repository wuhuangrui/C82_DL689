#ifndef 	KEY_H
#define 	KEY_H

#include "bios.h"

//�ײ�Ӳ��I/O�ڶ���
#define    UPKEY				0x01
#define    LEFTKEY				0x02
#define    RIGHTKEY				0x04
#define    DOWNKEY				0x08
#define    EXITKEY				0x10
#define    OKKEY				0x20


//������ֵ�궨��4�Ϸ���1�·���2���룬3�˳�
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


//������
class CKey
{
private:
	int m_iMonitorID;
	WORD m_wKeyBuf[KEYBUFLEN];	//���հ�����Ϣ�Ļ�������0�ް�����1������2˫����3����
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
	WORD GetKey(void);	//ȡ����
	void SetKey(WORD bKeyInf);	//���ð���	
	DWORD GetKeyValue(void);    //ɨ���ֵ
	void MonitorKey(void);		//���Ӱ���
	void FastGetKey(void);		//���ټ��Ӱ���
	
	BYTE KeyValue2Type(DWORD dwKeyVal); // 
	
	int ReqMonitorID();
};

extern CKey  g_Key;		//����һ����������

#endif
