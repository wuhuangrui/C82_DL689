#ifndef 	KEY_H
#define 	KEY_H

//按键键值宏定义4上翻，1下翻，2进入，3退出
#define    KEYUP						0x01
#define    KEYLEFT						0x02
#define    KEYRIGHT						0x03
#define    KEYDOWN          			0x04
#define    KEYEXIT          			0x05
#define    KEYOK            			0x06

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

#endif





