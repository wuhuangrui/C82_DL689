#ifndef GUIAPI_H
#define GUIAPI_H

#include <stddef.h>           /* needed for definition of NULL */
#include <stddef.h>
//#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
//#include <pthread.h>

//#include "common.h"
#include "DrvAPI.h"
#include "GUI.H"
#include "GuiBmp.h"


#define LISTBOXVALUE     (1)


#define LCD_SIM_WIDTH 	160
#define LCD_SIM_HEIGHT 	160
#define LCD_SIM_WIDTH_BYTE	20

#define FONT_HEIGHT		(14+2)
#define FONT_WIDTH		(14)

#define	   KEY_NULL						0
#define    KEY_UP						0x01
#define    KEY_LEFT						0x02
#define    KEY_RIGHT					0x03
#define    KEY_DOWN          			0x04
#define    KEY_ESC          			0x05
#define    KEY_OK            			0x06
#define	   KEY_OK_ESC					0x07

#define DATA_HEX	1
#define DATA_DEC	2
#define DATA_ASC	3
#define DATA_BIN	4
#define DATA_DEC8	5


#define MENU_TIMEOUT	1000

struct KeyState
{
	char key;
	int idle;
};

typedef struct
{
	BYTE bXPos;
	BYTE bYPos;
}TCurSoftKeyPos;


struct ListBoxExItem
{
	char *text;
	BYTE bNo;
	int (*func)(void *);
	void *arg;
};

struct ListBoxExPageItem
{
	const char* pszItemName;
	WORD wID;
	BYTE *pbBuf;
};

void LcdLock(void);
void LcdUnLock(void);

struct KeyState GetKey(void);

int GetMaxCharPerLine(void);
int GetMaxLinePerScreen(void);
int GetFontWidth();
int GetFontHeight();
void DrawBmp16x16(const char text[], int x, int y);
void DrawBmp8x16(const char text[], int x, int y);
void DrawBmp8x8(const char text[], int x, int y);


int DrawStringAtLock(const char *s, int x, int y, GUI_COLOR fColor, GUI_COLOR bColor);
int DrawStringHCenterAtLock(const char *s, int x, int y, GUI_COLOR fColor, GUI_COLOR bColor);
void DrawRectLock(int x0, int y0, int x1, int y1);

int MessageBox(const char msg[], char retKey, int retTime);
int EditTextBox(int y, const char title[], char text[], int retTime, int txtSize, int dataType);
int EditSpecBox(int y, char title1[], char text[], int retTime, int txtSize, int dataType,bool fMask=false);
int EditDateBox(int y, char title1[], char title2[], char text[], int retTime, int txtSize, int dataType);
int getSoftKeyPwd(const char *const pTitle,  char *const pText, int retTime, int txtSize, BYTE bDataType);
int getSoftKey(const char *const pTitle,  char *const pText, int retTime, int txtSize, BYTE bDataType);
int EditTextBoxDate(int y, const char title[], char text[], int retTime, int txtSize, int dataType);
int EditTextBoxTime(int y, const char title[], char text[], int retTime, int txtSize, int dataType);
int EditTextBoxIP(int y, const char title[], char text[], int retTime, int txtSize, int dataType);
int getSoftKeyMAC(const char * const pTitle,  char * const pText, int retTime, int txtSize);

class CListBoxEx
{
public:
	CListBoxEx();
	
	CListBoxEx(BYTE tempcontainItem);
	~CListBoxEx();
	int Show(int x, const char title[], struct ListBoxExItem item[], int retKey, int retTime,bool fFocus=true);
	void RollStatusFunc();//画滚动条
	void RollStatusFunc1(BYTE rolway);

	int key;
	short item;

private:
	short curItem;
	short startItem;
	short totalItem;
	short haveTitle;

	BYTE RollStatus ;//1:有上箭头，2:有上下箭头，3:有下箭头= 0;
	BYTE containItem;//用于翻页，由于翻页没有超过范围
	BYTE RolWay;
	BYTE Medium;
};


extern void LcdRefresh();
extern void BlightOn(bool fOn);
extern void JudgeLcdBlight();

extern void BackLcdDisp(char *disp);
extern void RestoreLcdDisp(char *disp);
extern void SetPix(int x, int y, int value);
extern void ClearScreen(void);
extern void DrawRawData(char data[], int size, int x, int y);
extern void ClearScreenRec(int x0, int y0, int x1, int y1);
extern int GetDispMonitorID();
extern void SetDispMonitorID(int iMonitorID);



#ifdef SYS_WIN
extern int sys_getchar(void);
#endif

#endif /* GUIAPI_H */
