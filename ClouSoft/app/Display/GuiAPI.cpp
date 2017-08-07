#include "stdafx.h"
#include "sysarch.h"
#include "GuiAPI.h"
#include "GuiBmp.h"
#include "sysapi.h"
#include "Trace.h"
#include "Key.h"
#include "FaAPI.h"

char lcd_sim_mem[LCD_SIM_WIDTH * LCD_SIM_HEIGHT / 8];
volatile static char key = 0;

extern int DrawStateTask();


DWORD g_dwStateTaskClick = 0;
bool g_fLcdRefresh = true;
DWORD g_dwBlightOnClick = 0;
int	g_nDispMonitorID = -1;

TSem g_SemGUI;

int GetDispMonitorID()
{
	return g_nDispMonitorID;
}

void SetDispMonitorID(int iMonitorID)
{
	g_nDispMonitorID = iMonitorID;
}

void LcdLock(void)
{
	WaitSemaphore(g_SemGUI);
}
void LcdUnLock(void)
{
	SignalSemaphore(g_SemGUI);
}

void BackLcdDisp(char *disp)
{
	memcpy(disp + LCD_SIM_WIDTH * 2, (char *) lcd_sim_mem + LCD_SIM_WIDTH * 2, sizeof(lcd_sim_mem) - LCD_SIM_WIDTH * 4);
}
void RestoreLcdDisp(char *disp)
{
	LcdLock();
	memcpy((char *) lcd_sim_mem + LCD_SIM_WIDTH * 2, disp + LCD_SIM_WIDTH * 2, sizeof(lcd_sim_mem) - +LCD_SIM_WIDTH * 4);
	LcdUnLock();
}
void ClearScreen(void)
{
	LcdLock();
	memset((char *) lcd_sim_mem, 0, sizeof(lcd_sim_mem));
	LcdUnLock();
}

void SetPix(int x, int y, int value)
{
	x %= LCD_SIM_WIDTH;
	y %= LCD_SIM_HEIGHT;

	//lcd_sim_mem[y * LCD_SIM_WIDTH / 8 + x / 8] &= ~(1 << (x % 8));
	//lcd_sim_mem[y * LCD_SIM_WIDTH / 8 + x / 8] |= ((value & 1) << (x % 8));
	lcd_sim_mem[y * LCD_SIM_WIDTH_BYTE + (x>>3)] &= ~(1 << (7-(x&0x07)));
	lcd_sim_mem[y * LCD_SIM_WIDTH_BYTE + (x>>3)] |= ((value & 1) << (7-(x&0x07)));

}

void LcdRefresh()
{
	LcdLock();
#ifdef SYS_WIN
	extern unsigned char m_ucGDRam[160*160/8];
	memcpy(m_ucGDRam,lcd_sim_mem,sizeof(m_ucGDRam));
#else
	g_pLcd->DrawBitmap((BYTE*)lcd_sim_mem,true);
#endif
	LcdUnLock();
}


void BlightOn(bool fOn)
{
#ifndef SYS_WIN
	g_pLcd->BlightOn(fOn);

	if(fOn)
	{
		g_dwBlightOnClick = GetClick();
	}
#endif
}

void JudgeLcdBlight()
{
#ifndef SYS_WIN
	if(GetClick()-g_dwBlightOnClick > 60)
	{
		BlightOn(false);
		g_dwBlightOnClick = GetClick();
	}
#endif
}

#ifndef SYS_WIN
extern CKey  g_Key;	
int sys_getchar(void)
{
#if 0	
	WORD wKeyValue;
	wKeyValue = g_Key.GetKeyValue();
	if (wKeyValue == 4)
	{
		wKeyValue = 3;
	}
	else if(wKeyValue == 8)
	{
		wKeyValue = 4;
	}
	else if(wKeyValue == 16)
	{
		wKeyValue = 5;
	}
	else if(wKeyValue == 32)
	{
		wKeyValue = 6;
	}
	else if(wKeyValue == 6)
	{
		wKeyValue = 0;
	}
	
	return wKeyValue;
#else
	g_Key.FastGetKey();//---cqj
	WORD wKey = g_Key.GetKey();
	wKey &= 0xFF;
	//DTRACE(DB_CRITICAL,("sys_getchar:: monitor key=%04x!\r\n",wKey));
	return wKey;
#endif
}
#endif


struct KeyState GetKey(void)
{
	
	static char flag = 0;
	static int OldS = 0;
	struct KeyState keyState;
	int CurS = 0;
	
	keyState.key = GUI_GetKey();

	CurS = time((time_t*) NULL);


	if (flag == 0 && CurS >= 0)
	{
		flag = 1;
		OldS = CurS;
	}
	//if(time((time_t*) NULL) - OldS > 59)
	//{
	//	OldS = time((time_t*) NULL);
	//}
	

	if (keyState.key != 0 && CurS >= 0)
	{
		BlightOn(true);
		OldS = CurS;
	}
	
	if (keyState.key == (KEY_OK|KEY_ESC))
	{
	
	}
	else if (keyState.key == (KEY_UP|KEY_DOWN))
	{
		
	}
	else if (keyState.key == (KEY_LEFT|KEY_RIGHT))
	{
		
	}
	else if (keyState.key == (KEY_LEFT|KEY_OK|KEY_ESC))
	{
		 
	}
	else if (keyState.key == (KEY_LEFT|KEY_OK))
	{
		 
	}

	if(OldS > CurS)
		OldS = CurS;

	keyState.idle =  CurS - OldS;

	return keyState;
}
int GetMaxCharPerLine(void)
{
	return LCD_SIM_WIDTH / FONT_WIDTH;
}
int GetMaxLinePerScreen(void)
{
	return LCD_SIM_HEIGHT / FONT_HEIGHT;
}
int GetFontWidth()
{
	return FONT_WIDTH;
}
int GetFontHeight()
{
	return FONT_HEIGHT;
}
void ClearScreenRec(int x0, int y0, int x1, int y1)
{
	LcdLock();
	for (int i = x0; i < x1; i++)
	{
		for (int n = y0; n < y1; n++)
		{
			SetPix(i, n, 0);
		}
	}
	LcdUnLock();
}
void DrawBmp16x16(const unsigned char text[], int x, int y)
{
	LcdLock();
	for (int i = 0; i < 32; i += 2)
	{
		for (int bit = 0; bit < 8; bit++)
		{
			SetPix(x + 7 - bit, y + i / 2, 1 & (text[i] >> bit));
			SetPix(x + 8 + 7 - bit, y + i / 2, 1 & (text[i + 1] >> bit));
		}
	}
	LcdUnLock();
}

void DrawBmp8x16(const unsigned char text[], int x, int y)
{
	LcdLock();
	for (int i = 0; i < 16; i += 1)
	{
		for (int bit = 0; bit < 8; bit++)
		{
			SetPix(x + 7 - bit, y + i , 1 & (text[i] >> bit));
		//	SetPix(x + 8 + 7 - bit, y + i / 2, 1 & (text[i + 1] >> bit));
		}
	}
	LcdUnLock();
}


void DrawBmp8x8(const unsigned char text[], int x, int y)
{
	LcdLock();
	for (int i = 0; i < 8; i += 1)
	{
		for (int bit = 0; bit < 8; bit++)
		{
			SetPix(x + 7 - bit, y + i , 1 & (text[i] >> bit));
			//	SetPix(x + 8 + 7 - bit, y + i / 2, 1 & (text[i + 1] >> bit));
		}
	}
	LcdUnLock();
}
int DrawStringAtLock(const char *s, int x, int y, GUI_COLOR fColor, GUI_COLOR bColor)
{
	int lineCnt = 0;
	LcdLock();

	GUI_SetBkColor(bColor);
	GUI_SetColor(fColor);
	GUI_DispStringAt(s, x, y);
	for (unsigned int i = 0; i < strlen(s); i++)
	{
		if (s[i] == '\n')
		{
			lineCnt++;
			GUI_DispStringHCenterAt(s + i + 1, 0, y + GetFontHeight() * lineCnt);
		}
	}

	LcdUnLock();
	return lineCnt;
}
int DrawStringHCenterAtLock(const char *s, int x, int y, GUI_COLOR fColor, GUI_COLOR bColor)
{
	int lineCnt = 0;
	LcdLock();

	GUI_SetBkColor(bColor);
	GUI_SetColor(fColor);
	GUI_DispStringHCenterAt(s, x, y);
	for (unsigned int i = 0; i < strlen(s); i++)
	{
		if (s[i] == '\n')
		{
			lineCnt++;
			GUI_DispStringHCenterAt(s + i + 1, x, y + GetFontHeight() * lineCnt);
		}
	}

	LcdUnLock();
	return lineCnt;
}
void DrawRectLock(int x0, int y0, int x1, int y1)
{
	LcdLock();
	GUI_DrawRect(x0, y0, x1, y1);
	LcdUnLock();
}

int MessageBox(const char msg[], char retKey, int retTime)
{
	int time = 0;
	char buff[32];

	//char LcdDispBuff[LCD_SIM_HEIGHT * LCD_SIM_WIDTH / 8];
	//bak_lcd_disp(LcdDispBuff);

	int len = strlen(msg);
	int maxLinePerScreen = GetMaxLinePerScreen();
	int maxCharPerLine = GetMaxCharPerLine() - 2;	
	int tatolLine = len / 2 / maxCharPerLine + 1;
	for (unsigned int i = 0; i < strlen(msg); i++)
	{
		if (msg[i] == '\n')
		{
			tatolLine++;
		}
	}

	if (tatolLine > 7)
		tatolLine = 7;
		
	int y = (maxLinePerScreen - tatolLine) / 2;
	if (y < 1)
	{
		y = 1;
	}
	if (y >= maxLinePerScreen)
	{
		y = maxLinePerScreen - 1;
	}

	ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
	DrawRectLock(4, y * GetFontHeight() - 4, //
			LCD_GET_XSIZE() - 4, (y + tatolLine) * GetFontHeight());

	const char *p = msg;
	int muxflag = 0;
	while (len > 0)
	{
		if (len >= 2 * maxCharPerLine)
		{
			int x1 = 0;
			int y1 = 0;
			unsigned char *ustr = (unsigned char *)p;
			int i = 0;
			for(i=0; i<len; i++)
			{
				if(x1>maxCharPerLine*2)	//本行写不下，则自动换行
				{
					x1 = 0;
					y1++;
					break;
				}
				else if(x1>maxCharPerLine*2-1 && ustr[i]>=128)			//本行写不下汉字，则自动换行
				{
					x1 = 0;
					y1 += 1;
					break;
				}
				
				if(ustr[i] >= 128)			//最高位为1
				{				
					i++;
					x1 += 2;	//写入一个汉字光标右移两位（汉字）									
				}
				else if(*(ustr+i) < 32)//对于ASC码小过32的不用的字符自动舍去
				{			
					
				}
				else	//是英文字符
				{			
					x1++;	//写入一个字符光标右移1位			
				}
			}
			
			muxflag = 1;
			memcpy(buff, p, i);
			p += i;
			buff[i] = 0;
			len -= i;
			DrawStringHCenterAtLock(buff, LCD_GET_XSIZE() / 2, y * GetFontHeight(), GUI_WHITE, GUI_BLACK);
		} else
		{
			memcpy(buff, p, len);
			buff[len] = 0;
			len = 0;
			if (muxflag == 1)
			{
				DrawStringAtLock(buff, GetFontWidth(), y * GetFontHeight(), GUI_WHITE, GUI_BLACK);
			} else
			{
				DrawStringHCenterAtLock(buff, LCD_GET_XSIZE() / 2, y * GetFontHeight(), GUI_WHITE, GUI_BLACK);
			}
		}

		y++;
		if (y > 7)
			break;
	}
		
	LcdRefresh();
	while (time <= retTime / 100)
	{
		Sleep(100);
		time++;
		BYTE key = GetKey().key;
		if (key == retKey)
		{
			return 0;
		}
		else if (key == KEY_OK)
		{
			return 1;
		}
		
		DWORD dwTick = GetTick();
		if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
		{
			DrawStateTask();  //todo luom
			g_fLcdRefresh = true;
			g_dwStateTaskClick = dwTick;
		}
		
		if (g_fLcdRefresh)
		{			
			g_fLcdRefresh = false;
			LcdRefresh();			
		}
	}   
        
	//res_lcd_disp(LcdDispBuff);
	return 0;
}       
        
static char BinCharTable[] = {'0', '1',};
static char HexCharTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '.',
		':', ' ', };
//static char DecCharTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '-',' ', };
static char DecCharTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0};//添加0是因为strlen()使用

static char Dec8CharTable[] = { '1', '2', '3', '4', '5', '6', '7', '8',  0};//添加0是因为strlen()使用

static char AscCharTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
		'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		 'a', 'b', 'c', 'd', 'e', 'f', 'g',
		'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
		'.', '_', ':', '+', '-', '@', '!', '*', '#', '%', ',', '~', ' ', };

static char cSoftKeyTable[][20] ={   { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',},
									{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',  },
									{ 'S', 'T','U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', },
									{ 'k', 'l','m', 'n','o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',},
									{ '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '-', '=', ',', '.', ' ', }//'/', ':',
								};
static char cSoftKeyTableHEX[][20] ={   { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B','C','D','E','F',' '},
	 								};

static char cSoftKeyTableDEC[][20] ={   { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',' '},
	 								};
void drawPreference()
{
	DrawBmp16x16(iconArrow[1], 0, 100);
	DrawBmp16x16(iconArrow[0], 16, 100);
	DrawStringAtLock("	键增减", 32, 100, GUI_WHITE, GUI_BLACK);
	DrawBmp16x16(iconArrow[3],0,116);
	DrawBmp16x16(iconArrow[2],16,116);
	DrawStringAtLock("	键移位", 32, 116, GUI_WHITE, GUI_BLACK);
}


void drawBY()
{
	DrawBmp16x16(iconBY[0], 143, 50);//右上
	DrawBmp16x16(iconBY[1], 1, 50);//左上
	DrawBmp16x16(iconBY[2], 1, 118);//左下
	DrawBmp16x16(iconBY[3], 143, 118);//右下
	
	DrawRectLock(9, 55, 151, 55);//画水平线
	DrawRectLock(9, 130, 151, 130);//画水平线
	DrawRectLock(1, 63, 1, 122);//画竖线
	DrawRectLock(159, 63, 159, 122);//画竖线
}

int getSoftKey(const char *const pTitle,  char *const pText, int retTime, int txtSize, BYTE bDataType)
{
	if((pText==NULL) || (retTime<=0) || (txtSize<=0))
	{
		return -1;
	}
      
	if ((bDataType!=1) && (bDataType!=2) && (bDataType!=3) && (bDataType!=4))
	{
		return -1;
	}

	//bDataType = DATA_DEC;

	struct KeyState keyState;
	char   txtBuf[96]  = {0};
	int    txtPos      = 0;//输入栏光标
	TCurSoftKeyPos   CurSoftKeyPos;  //软键盘光标
	bool fUpAreaActive   = true;	 //输入栏是否为当前活动区域
	//bool fDownAreaActive = false;	 //软键盘是否为当前活动区域
	bool  fIsHaveTitle = false;
	DWORD dwLastDisClick = 0;
	BYTE  bCount = 0;
	BYTE bCount2 = 0;

	//BYTE mCQJMAX;
	BYTE bmMAX;

	memset(&CurSoftKeyPos, 0x00, sizeof(CurSoftKeyPos));
	if ( strlen(pTitle) > 0 )
	{
		fIsHaveTitle = true;
	}

	memset(txtBuf, ' ', sizeof(txtBuf));
	txtSize += 2;
	if (txtSize > (int) sizeof(txtBuf))
	{
		txtSize = sizeof(txtBuf);
	}
	

	pText[txtSize - 2] = 0;
	int initTextSize = strlen(pText);
	if (initTextSize > txtSize - 2)
	{
		initTextSize = txtSize - 2;
	}
	if (initTextSize > 0)
	{
		memcpy(txtBuf, pText, initTextSize);
	}
	txtBuf[txtSize - 1] = 0;

	BYTE bTotalLine = 2;//---cqj
	BYTE bXPos = 0;
	BYTE bYPos = 16;
	for (int i = 0; i<bTotalLine; i++)//cqj
	{
		DrawStringAtLock("                            ", 0, (GetFontHeight()*(i+1)), GUI_WHITE, GUI_BLACK);
	}
	ClearScreenRec(0, 16, 160, 144);	//清屏。

	while (1)
	{
		for (int i=0; i<bTotalLine; i++)
		{	
			//DTRACE(0,("--------NO TO HERE ?"));
			DrawRectLock(0, 14, LCD_SIM_WIDTH - 1, (GetFontHeight()*(i+2))-1);
			drawBY();	
		}
	
		if(DATA_ASC == bDataType)
		{
			bmMAX = 5;
		}
		else if((DATA_HEX == bDataType) || (DATA_DEC == bDataType))
		{
			bmMAX = 1;
			DrawBmp16x16(iconArrow[0], 8, 95);
			DrawStringAtLock("	进入键盘 ", 24, 95, GUI_WHITE, GUI_BLACK);
			DrawBmp16x16(iconArrow[1], 8,111);
			DrawStringAtLock("	退出键盘 ", 24, 111, GUI_WHITE, GUI_BLACK);
		}
		if ( fIsHaveTitle )
		{
			DrawStringHCenterAtLock(pTitle, LCD_SIM_WIDTH / 2, bYPos, GUI_WHITE, GUI_BLACK);
		}

		for (int i=0; i<txtSize; i++)
		{
			char tmp[2] = { 0 };
			tmp[0] = txtBuf[i];
			bXPos = ( i * 8);//设置密码间隔，
			if (i == txtPos)
			{
				if (bCount < 6)//cqj ---
				{
					bCount++;
				}
				else
				{
					bCount = 0;
				}
				
				//if ( (GetClick()%2) == 1)
				if(fUpAreaActive)
				{
					if (bCount < 3)//慢闪
					{////聚焦
						DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_BLACK, GUI_WHITE);
					}
					else
					{//没有聚焦
						DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_WHITE, GUI_BLACK);
					}
				}
				else
				{
					DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_BLACK, GUI_WHITE);
				}
				
			} 
			else
			{//控制不止显示一位
				DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_WHITE, GUI_BLACK);
			}
		}

		//根据传进来的参数画不同的键盘
		if(DATA_ASC == bDataType)
		{
			for (int m=0; m<5; m++)//画键盘
			{
				BYTE bTmp = m + 4;
				for (int n=0; n<20; n++ )
				{	
					char tmp[2] = { 0};
					tmp[0] = cSoftKeyTable[m][n];
					if(m == 0)
					{
						if ( (m==CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
						{
							
							if (bCount2 < 6)//cqj ---
							{
								bCount2++;
							}
							else
							{
								bCount2 = 0;
							}
							
							if ( bCount2 > 2 )
							{
								//bCount2 = 0;
								DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_BLACK, GUI_WHITE);
							}
							else
							{
								//bCount2 = 0;
								DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_WHITE, GUI_BLACK);
							}
						}
						else
						{
							DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_WHITE, GUI_BLACK);
						}
					}
					else
					{
						if ( (m==CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
						{
							// DrawStringAtLock(tmp, n*8+8, bTmp*14, GUI_BLACK, GUI_WHITE);
							if (bCount2 < 6)//cqj ---
							{
								bCount2++;
							}
							else
							{
								bCount2 = 0;
							}
							
							if ( bCount2 > 2 )
							{
								//bCount2 = 0;
								DrawStringAtLock(tmp, n*8+8, bTmp*14, GUI_BLACK, GUI_WHITE);
							}
							else
							{
								//bCount2 = 0;
								DrawStringAtLock(tmp, n*8+8, bTmp*14, GUI_WHITE, GUI_BLACK);
							}
						}
						else
						{
							DrawStringAtLock(tmp, n*8+8, bTmp*14, GUI_WHITE, GUI_BLACK);
						}
					}
					
					if (((m==0) && (n==14))//---cqj---if (  ( (m==0) &&(n==9)  )
						||((m==3) && (n==15)))
					{
						break;
					}
				}			
			}
		}
		else if(DATA_HEX == bDataType)
		{
			for (int m=0; m<1; m++)//画键盘
			{
				BYTE bTmp = m + 4;
				for (int n=0; n<20; n++)
				{	
					char tmp[2] = { 0,0};
					tmp[0] = cSoftKeyTableHEX[m][n];
					if ( (m == CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
					{//	DTRACE(0, ("cqj---%d, %d, %d ", CurSoftKeyPos.bYPos,CurSoftKeyPos.bXPos,fUpAreaActive));
						if (bCount2 < 6)//cqj ---
						{
							bCount2++;
						}
						else
						{
							bCount2 = 0;
						}
						if ( bCount2 > 2 )
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_BLACK, GUI_WHITE);
						}
						else
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_WHITE, GUI_BLACK);
						}

						//DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_BLACK, GUI_WHITE);
					}
					else
					{
						DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_WHITE, GUI_BLACK);
					}
			
					if ((m==0) && (n==18))//---cqj---if (  ( (m==0) &&(n==9)  )
					{
						break;
					}
				}			
			}
			
		}
		else if(DATA_DEC == bDataType)
		{
			for (int m=0; m<1; m++)//画键盘
			{
				BYTE bTmp = m + 4;
				for (int n=0; n<20; n++)
				{	
					char tmp[2] = {0, 0};
					tmp[0] = cSoftKeyTableDEC[m][n];
					if ((m == CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
					{//	DTRACE(0, ("cqj---%d, %d, %d ", CurSoftKeyPos.bYPos,CurSoftKeyPos.bXPos,fUpAreaActive));
						if (bCount2 < 6)//cqj ---
						{
							bCount2++;
						}
						else
						{
							bCount2 = 0;
						}
						
						if (bCount2 > 2)
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_BLACK, GUI_WHITE);
						}
						else
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_WHITE, GUI_BLACK);
						}
						//DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_BLACK, GUI_WHITE);
					}
					else
					{
						DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_WHITE, GUI_BLACK);
					}
			
					if ((m==0) && (n==17))//---cqj---if (  ( (m==0) &&(n==9)  )
					{
						break;
					}
				}			
			}
		}
		
		LcdRefresh();
		//while (1)
		{
			Sleep(50);

			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{			
				g_fLcdRefresh = false;
				LcdRefresh();			
			}

			keyState = GetKey();

			if (keyState.key != 0) 
			{
				if (keyState.key == KEY_ESC)
				{
				#if 0
					if (txtSize > 2)
					{
						memcpy(pText, txtBuf, txtSize - 2);
					}
					ClearScreenRec(0, 128, 160, 144);
					return txtPos;
				#else
					ClearScreenRec(0, 128, 160, 144);//ClearScreenRec(0, 145, LCD_SIM_WIDTH2, 160);
				    return -1;
				#endif
				}
			}
			else
			{
				if (  keyState.idle > (retTime/1000)  )
				{
					return -1;
				}
			}
			/*
			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{			
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
			*/
		}

		switch(fUpAreaActive)
		{
		case  true:
			{
				if ( keyState.key == KEY_OK )
				{
					if (txtSize > 2)
					{
						memcpy(pText, txtBuf, txtSize - 2);
					}
					return txtPos;
				}
				if (keyState.key == KEY_UP)
				{
					;
				}
				if (keyState.key == KEY_DOWN)
				{
					fUpAreaActive = false;
				}
				if (keyState.key == KEY_LEFT)
				{
					if (txtPos > 0)
					{
						txtPos--;
					}
				}
				if (keyState.key == KEY_RIGHT)
				{
					if (txtPos < txtSize - 3)
					{
						txtPos++;
					}
				}
				break;
			}

		case false:
			{
				for (int i=0; i<bTotalLine; i++)
				{
				//	DrawRectLock(0, 14, LCD_SIM_WIDTH - 1, (GetFontHeight()*(i+2))-1);
				}

				if (keyState.key == KEY_OK)
				{
					fUpAreaActive = true;
				}
				if (keyState.key == KEY_UP)
				{
					switch (CurSoftKeyPos.bYPos)
					{
					case 0:
					{
						fUpAreaActive = true;
						break;
					}
					case 1:
						{//cqj---if( (CurSoftKeyPos.bXPos<=9) && (CurSoftKeyPos.bXPos>=0) )
							if( (CurSoftKeyPos.bXPos<=13) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos--;
								CurSoftKeyPos.bXPos -= 4;
							}
							break;
						}
					case 2:
						{
							//if( (CurSoftKeyPos.bXPos<=19) && (CurSoftKeyPos.bXPos>=0) )
							if( (CurSoftKeyPos.bXPos<=17) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos--;
								
							}
							break;
						}
					case 3:
					case 4:
						{
							//if( (CurSoftKeyPos.bXPos<=11) && (CurSoftKeyPos.bXPos>=0) )
							if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos--;
							}
							break;
						}
					default:
						{
							break;
						}
					}
				}
				if (keyState.key == KEY_DOWN)
				{
					switch (CurSoftKeyPos.bYPos)
					{
					case 0:
						{//---cqj
							//1 if( (CurSoftKeyPos.bXPos<=9) && (CurSoftKeyPos.bXPos>=0) )
							if(DATA_ASC == bDataType)//ascii码
							{
								if( (CurSoftKeyPos.bXPos<=14) && (CurSoftKeyPos.bXPos>=0) )
								{
									CurSoftKeyPos.bYPos++;
									CurSoftKeyPos.bXPos += 4;
								}
							}
							else if(DATA_HEX == bDataType)
							{
								//CurSoftKeyPos.bYPos++;
								//if(CurSoftKeyPos.bXPos == 15)
								//{
								
								//}
							}
							else if(DATA_DEC == bDataType)
							{

							}
							
							break;
						}
					case 1:
						{
							if( (CurSoftKeyPos.bXPos<=19) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos++;
							}
							break;
						}
					case 2:
						{
							if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos++;
							}
							break;
						}
					case 3:
						{
							//if( (CurSoftKeyPos.bXPos<=11) && (CurSoftKeyPos.bXPos>=0) )
							if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos++;
							}
							break;
						}
					default:
						{
							break;
						}
					}	
				}
				if (keyState.key == KEY_LEFT)
				{//DTRACE(0, ("the key is %d",CurSoftKeyPos.bXPos ));//CurSoftKeyPos.bXPos = 1,代表第一个
					if(DATA_ASC == bDataType)
					{
						if (CurSoftKeyPos.bXPos > 0)
						{
							CurSoftKeyPos.bXPos--;
						}
					}
					else if (DATA_HEX == bDataType)
					{
						if (CurSoftKeyPos.bXPos > 0)
						{
							CurSoftKeyPos.bXPos--;
						}
					}
					else if(DATA_DEC == bDataType)
					{
						if (CurSoftKeyPos.bXPos > 0)
						{
							CurSoftKeyPos.bXPos--;
						}
					}
					
				}
				if (keyState.key == KEY_RIGHT)
				{
					switch(CurSoftKeyPos.bYPos)
					{
					case 0:
						{
							//if (CurSoftKeyPos.bXPos < 9)//cqj
							if(DATA_ASC == bDataType)
							{
								if (CurSoftKeyPos.bXPos < 9)
								{
									CurSoftKeyPos.bXPos++;
								}
								
							}
							else if(DATA_HEX == bDataType)
							{
								if (CurSoftKeyPos.bXPos < 16)//  17--->BUF[17]
								{
									CurSoftKeyPos.bXPos++;
								}
								
							}
							else if(DATA_DEC == bDataType)
							{
								if (CurSoftKeyPos.bXPos < 10)//  17--->BUF[17]
								{
									CurSoftKeyPos.bXPos++;
								}
							}
						
							break;
						}

					case 1:
					case 2:		
					case 4:	
						{
							//if (CurSoftKeyPos.bXPos < 19)
							if (CurSoftKeyPos.bXPos < 17)
							{
								CurSoftKeyPos.bXPos++;
							}
							break;
						}

					case 3:
						{
							if (CurSoftKeyPos.bXPos < 15)// if (CurSoftKeyPos.bXPos < 12)   if (CurSoftKeyPos.bXPos < 11)
							{
								CurSoftKeyPos.bXPos++;
							}
							break;
						}

					default:
						{
							break;
						}
					}
	
				}

				if(DATA_ASC == bDataType)
				{
					txtBuf[txtPos] = cSoftKeyTable[CurSoftKeyPos.bYPos][CurSoftKeyPos.bXPos];
				}
				else if (DATA_HEX == bDataType)
				{//	DTRACE(0, ("cqj---txtBuf is %d",txtBuf[txtPos] ));
					txtBuf[txtPos] = cSoftKeyTableHEX[CurSoftKeyPos.bYPos][CurSoftKeyPos.bXPos];
				}
				else if (DATA_DEC == bDataType)
				{	
					txtBuf[txtPos] = cSoftKeyTableDEC[CurSoftKeyPos.bYPos][CurSoftKeyPos.bXPos];
				}
			break;
			}

		default:
			{
				break;
			}

		}

		
		for (int m=0; m < bmMAX; m++)//m <５
		{
			BYTE bMaxN = 0;
			switch (m)
			{
			case 0:
				{
					bMaxN = 19;//bMaxN = 10;---cqj
					break;
				}

			case 3:
				{
					bMaxN = 12;//---cqj--12
					break;
				}

			case 1:
			case 2:
			case 4:
				{
					bMaxN = 20;
					break;
				}

			default:
				{
					bMaxN = 20;
					break;
				}

			}

			for (int n=0; n<bMaxN; n++)
			{
				if(DATA_ASC == bDataType)
				{
					if ( txtBuf[txtPos] == cSoftKeyTable[m][n])
					{
						CurSoftKeyPos.bYPos = m;
						CurSoftKeyPos.bXPos = n;
					}
				}
				else if (DATA_HEX == bDataType)
				{
					if ( txtBuf[txtPos] == cSoftKeyTableHEX[m][n])
					{
						CurSoftKeyPos.bYPos = m;
						CurSoftKeyPos.bXPos = n;
					}
				}
				else if (DATA_DEC == bDataType)
				{
					if ( txtBuf[txtPos] == cSoftKeyTableDEC[m][n])
					{
						CurSoftKeyPos.bYPos = m;
						CurSoftKeyPos.bXPos = n;
					}
				}
				
			}
		}
		
	}
	return -1;
}



int getSoftKeyPwd(const char *const pTitle,  char *const pText, int retTime, int txtSize, BYTE bDataType)
{
	if((pText==NULL) || (retTime<=0) || (txtSize<=0))
	{
		return -1;
	}
      
	if ((bDataType!=1) && (bDataType!=2) && (bDataType!=3) && (bDataType!=4))
	{
		return -1;
	}

	//bDataType = DATA_DEC;

	struct KeyState keyState;
	char   txtBuf[96]  = {0};
	int    txtPos      = 0;//输入栏光标
	TCurSoftKeyPos   CurSoftKeyPos;  //软键盘光标
	bool fUpAreaActive   = true;	 //输入栏是否为当前活动区域
	//bool fDownAreaActive = false;	 //软键盘是否为当前活动区域
	bool  fIsHaveTitle = false;
	DWORD dwLastDisClick = 0;
	BYTE  bCount = 0;
	BYTE bCount2 = 0;

	//BYTE mCQJMAX;
	BYTE bmMAX;

	memset(&CurSoftKeyPos, 0x00, sizeof(CurSoftKeyPos));
	if ( strlen(pTitle) > 0 )
	{
		fIsHaveTitle = true;
	}

	memset(txtBuf, ' ', sizeof(txtBuf));
	txtSize += 2;
	if (txtSize > (int) sizeof(txtBuf))
	{
		txtSize = sizeof(txtBuf);
	}
	

	pText[txtSize - 2] = 0;
	int initTextSize = strlen(pText);
	if (initTextSize > txtSize - 2)
	{
		initTextSize = txtSize - 2;
	}
	if (initTextSize > 0)
	{
		memcpy(txtBuf, pText, initTextSize);
	}
	txtBuf[txtSize - 1] = 0;

	BYTE bTotalLine = 2;//---cqj
	BYTE bXPos = 0;
	BYTE bYPos = 16;
	for (int i = 0; i<bTotalLine; i++)//cqj
	{
		DrawStringAtLock("                            ", 0, (GetFontHeight()*(i+1)), GUI_WHITE, GUI_BLACK);
	}
	ClearScreenRec(0, 16, 160, 144);	//清屏。

	while (1)
	{
		for (int i=0; i<bTotalLine; i++)
		{	
			//DTRACE(0,("--------NO TO HERE ?"));
			DrawRectLock(0, 14, LCD_SIM_WIDTH - 1, (GetFontHeight()*(i+2))-1);
			drawBY();	
		}
	
		if(DATA_ASC == bDataType)
		{
			bmMAX = 5;
		}
		else if((DATA_HEX == bDataType) || (DATA_DEC == bDataType))
		{
			bmMAX = 1;
			DrawBmp16x16(iconArrow[0], 8, 95);
			DrawStringAtLock("	进入键盘 ", 24, 95, GUI_WHITE, GUI_BLACK);
			DrawBmp16x16(iconArrow[1], 8,111);
			DrawStringAtLock("	退出键盘 ", 24, 111, GUI_WHITE, GUI_BLACK);
		}
		if ( fIsHaveTitle )
		{
			DrawStringHCenterAtLock(pTitle, LCD_SIM_WIDTH / 2, bYPos, GUI_WHITE, GUI_BLACK);
		}

		for (int i=0; i<txtSize; i++)
		{
			char tmp[2] = { 0 };
			tmp[0] = txtBuf[i];
			bXPos = ( i * 8);//设置密码间隔，
			if (i == txtPos)
			{
				if (bCount < 6)//cqj ---
				{
					bCount++;
				}
				else
				{
					bCount = 0;
				}
				
				//if ( (GetClick()%2) == 1)
				if(fUpAreaActive)
				{
					if (bCount < 3)//慢闪
					{////聚焦
						DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_BLACK, GUI_WHITE);
					}
					else
					{//没有聚焦
						DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_WHITE, GUI_BLACK);
					}
				}
				else
				{
					DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_BLACK, GUI_WHITE);
				}
				
			} 
			else
			{//控制不止显示一位
				DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_WHITE, GUI_BLACK);
			}
		}

		//根据传进来的参数画不同的键盘
		if(DATA_ASC == bDataType)
		{
			for (int m=0; m<5; m++)//画键盘
			{
				BYTE bTmp = m + 4;
				for (int n=0; n<20; n++ )
				{	
					char tmp[2] = { 0};
					tmp[0] = cSoftKeyTable[m][n];
					if(m == 0)
					{
						if ( (m==CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
						{
							
							if (bCount2 < 6)//cqj ---
							{
								bCount2++;
							}
							else
							{
								bCount2 = 0;
							}
							
							if ( bCount2 > 2 )
							{
								//bCount2 = 0;
								DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_BLACK, GUI_WHITE);
							}
							else
							{
								//bCount2 = 0;
								DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_WHITE, GUI_BLACK);
							}
						}
						else
						{
							DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_WHITE, GUI_BLACK);
						}
					}
					else
					{
						if ( (m==CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
						{
							// DrawStringAtLock(tmp, n*8+8, bTmp*14, GUI_BLACK, GUI_WHITE);
							if (bCount2 < 6)//cqj ---
							{
								bCount2++;
							}
							else
							{
								bCount2 = 0;
							}
							
							if ( bCount2 > 2 )
							{
								//bCount2 = 0;
								DrawStringAtLock(tmp, n*8+8, bTmp*14, GUI_BLACK, GUI_WHITE);
							}
							else
							{
								//bCount2 = 0;
								DrawStringAtLock(tmp, n*8+8, bTmp*14, GUI_WHITE, GUI_BLACK);
							}
						}
						else
						{
							DrawStringAtLock(tmp, n*8+8, bTmp*14, GUI_WHITE, GUI_BLACK);
						}
					}
					
					if (((m==0) && (n==14))//---cqj---if (  ( (m==0) &&(n==9)  )
						||((m==3) && (n==15)))
					{
						break;
					}
				}			
			}
		}
		else if(DATA_HEX == bDataType)
		{
			for (int m=0; m<1; m++)//画键盘
			{
				BYTE bTmp = m + 4;
				for (int n=0; n<20; n++)
				{	
					char tmp[2] = { 0,0};
					tmp[0] = cSoftKeyTableHEX[m][n];
					if ( (m == CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
					{//	DTRACE(0, ("cqj---%d, %d, %d ", CurSoftKeyPos.bYPos,CurSoftKeyPos.bXPos,fUpAreaActive));
						if (bCount2 < 6)//cqj ---
						{
							bCount2++;
						}
						else
						{
							bCount2 = 0;
						}
						if ( bCount2 > 2 )
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_BLACK, GUI_WHITE);
						}
						else
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_WHITE, GUI_BLACK);
						}

						//DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_BLACK, GUI_WHITE);
					}
					else
					{
						DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_WHITE, GUI_BLACK);
					}
			
					if ((m==0) && (n==18))//---cqj---if (  ( (m==0) &&(n==9)  )
					{
						break;
					}
				}			
			}
			
		}
		else if(DATA_DEC == bDataType)
		{
			for (int m=0; m<1; m++)//画键盘
			{
				BYTE bTmp = m + 4;
				for (int n=0; n<20; n++)
				{	
					char tmp[2] = {0, 0};
					tmp[0] = cSoftKeyTableDEC[m][n];
					if ((m == CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
					{//	DTRACE(0, ("cqj---%d, %d, %d ", CurSoftKeyPos.bYPos,CurSoftKeyPos.bXPos,fUpAreaActive));
						if (bCount2 < 6)//cqj ---
						{
							bCount2++;
						}
						else
						{
							bCount2 = 0;
						}
						
						if (bCount2 > 2)
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_BLACK, GUI_WHITE);
						}
						else
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_WHITE, GUI_BLACK);
						}
						//DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_BLACK, GUI_WHITE);
					}
					else
					{
						DrawStringAtLock(tmp, n*8+40, bTmp*14, GUI_WHITE, GUI_BLACK);
					}
			
					if ((m==0) && (n==17))//---cqj---if (  ( (m==0) &&(n==9)  )
					{
						break;
					}
				}			
			}
		}
		
		LcdRefresh();
		//while (1)
		{
			Sleep(50);

			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{			
				g_fLcdRefresh = false;
				LcdRefresh();			
			}

			keyState = GetKey();

			if (keyState.key != 0) 
			{
				if (keyState.key == KEY_ESC)
				{
				#if 0
					if (txtSize > 2)
					{
						memcpy(pText, txtBuf, txtSize - 2);
					}
					ClearScreenRec(0, 128, 160, 144);
					return txtPos;
				#else
					ClearScreenRec(0, 128, 160, 144);//ClearScreenRec(0, 145, LCD_SIM_WIDTH2, 160);
				    return -1;
				#endif
				}
			}
			else
			{
				if (  keyState.idle > (retTime/1000)  )
				{
					return -1;
				}
			}
			/*
			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{			
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
			*/
		}

		switch(fUpAreaActive)
		{
		case  true:
			{
				if ( keyState.key == KEY_OK )
				{
					if (txtSize > 2)
					{
						memcpy(pText, txtBuf, txtSize - 2);
					}
					return txtPos;
				}
				if (keyState.key == KEY_UP)
				{
					;
				}
				if (keyState.key == KEY_DOWN)
				{
					fUpAreaActive = false;
				}
				if (keyState.key == KEY_LEFT)
				{
					if (txtPos > 0)
					{
						txtPos--;
					}
				}
				if (keyState.key == KEY_RIGHT)
				{
					if (txtPos < txtSize - 3)
					{
						txtPos++;
					}
				}
				break;
			}

		case false:
			{
				for (int i=0; i<bTotalLine; i++)
				{
				//	DrawRectLock(0, 14, LCD_SIM_WIDTH - 1, (GetFontHeight()*(i+2))-1);
				}

				if (keyState.key == KEY_OK)
				{
					fUpAreaActive = true;
				}
				if (keyState.key == KEY_UP)
				{
					switch (CurSoftKeyPos.bYPos)
					{
					case 0:
					{
						fUpAreaActive = true;
						break;
					}
					case 1:
						{//cqj---if( (CurSoftKeyPos.bXPos<=9) && (CurSoftKeyPos.bXPos>=0) )
							if( (CurSoftKeyPos.bXPos<=13) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos--;
								CurSoftKeyPos.bXPos -= 4;
							}
							break;
						}
					case 2:
						{
							//if( (CurSoftKeyPos.bXPos<=19) && (CurSoftKeyPos.bXPos>=0) )
							if( (CurSoftKeyPos.bXPos<=17) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos--;
								
							}
							break;
						}
					case 3:
					case 4:
						{
							//if( (CurSoftKeyPos.bXPos<=11) && (CurSoftKeyPos.bXPos>=0) )
							if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos--;
							}
							break;
						}
					default:
						{
							break;
						}
					}
				}
				if (keyState.key == KEY_DOWN)
				{
					switch (CurSoftKeyPos.bYPos)
					{
					case 0:
						{//---cqj
							//1 if( (CurSoftKeyPos.bXPos<=9) && (CurSoftKeyPos.bXPos>=0) )
							if(DATA_ASC == bDataType)//ascii码
							{
								if( (CurSoftKeyPos.bXPos<=14) && (CurSoftKeyPos.bXPos>=0) )
								{
									CurSoftKeyPos.bYPos++;
									CurSoftKeyPos.bXPos += 4;
								}
							}
							else if(DATA_HEX == bDataType)
							{
								//CurSoftKeyPos.bYPos++;
								//if(CurSoftKeyPos.bXPos == 15)
								//{
								
								//}
							}
							else if(DATA_DEC == bDataType)
							{

							}
							
							break;
						}
					case 1:
						{
							if( (CurSoftKeyPos.bXPos<=19) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos++;
							}
							break;
						}
					case 2:
						{
							if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos++;
							}
							break;
						}
					case 3:
						{
							//if( (CurSoftKeyPos.bXPos<=11) && (CurSoftKeyPos.bXPos>=0) )
							if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
							{
								CurSoftKeyPos.bYPos++;
							}
							break;
						}
					default:
						{
							break;
						}
					}	
				}
				if (keyState.key == KEY_LEFT)
				{//DTRACE(0, ("the key is %d",CurSoftKeyPos.bXPos ));//CurSoftKeyPos.bXPos = 1,代表第一个
					if(DATA_ASC == bDataType)
					{
						if (CurSoftKeyPos.bXPos > 0)
						{
							CurSoftKeyPos.bXPos--;
						}
					}
					else if (DATA_HEX == bDataType)
					{
						if (CurSoftKeyPos.bXPos > 0)
						{
							CurSoftKeyPos.bXPos--;
						}
					}
					else if(DATA_DEC == bDataType)
					{
						if (CurSoftKeyPos.bXPos > 0)
						{
							CurSoftKeyPos.bXPos--;
						}
					}
					
				}
				if (keyState.key == KEY_RIGHT)
				{
					switch(CurSoftKeyPos.bYPos)
					{
					case 0:
						{
							//if (CurSoftKeyPos.bXPos < 9)//cqj
							if(DATA_ASC == bDataType)
							{
								if (CurSoftKeyPos.bXPos < 9)
								{
									CurSoftKeyPos.bXPos++;
								}
								
							}
							else if(DATA_HEX == bDataType)
							{
								if (CurSoftKeyPos.bXPos < 16)//  17--->BUF[17]
								{
									CurSoftKeyPos.bXPos++;
								}
								
							}
							else if(DATA_DEC == bDataType)
							{
								if (CurSoftKeyPos.bXPos < 10)//  17--->BUF[17]
								{
									CurSoftKeyPos.bXPos++;
								}
							}
						
							break;
						}

					case 1:
					case 2:		
					case 4:	
						{
							//if (CurSoftKeyPos.bXPos < 19)
							if (CurSoftKeyPos.bXPos < 17)
							{
								CurSoftKeyPos.bXPos++;
							}
							break;
						}

					case 3:
						{
							if (CurSoftKeyPos.bXPos < 15)// if (CurSoftKeyPos.bXPos < 12)   if (CurSoftKeyPos.bXPos < 11)
							{
								CurSoftKeyPos.bXPos++;
							}
							break;
						}

					default:
						{
							break;
						}
					}
	
				}

				if(DATA_ASC == bDataType)
				{
					txtBuf[txtPos] = cSoftKeyTable[CurSoftKeyPos.bYPos][CurSoftKeyPos.bXPos];
				}
				else if (DATA_HEX == bDataType)
				{//	DTRACE(0, ("cqj---txtBuf is %d",txtBuf[txtPos] ));
					txtBuf[txtPos] = cSoftKeyTableHEX[CurSoftKeyPos.bYPos][CurSoftKeyPos.bXPos];
				}
				else if (DATA_DEC == bDataType)
				{	
					txtBuf[txtPos] = cSoftKeyTableDEC[CurSoftKeyPos.bYPos][CurSoftKeyPos.bXPos];
				}
			break;
			}

		default:
			{
				break;
			}

		}

		
		for (int m=0; m < bmMAX; m++)//m <５
		{
			BYTE bMaxN = 0;
			switch (m)
			{
			case 0:
				{
					bMaxN = 19;//bMaxN = 10;---cqj
					break;
				}

			case 3:
				{
					bMaxN = 12;//---cqj--12
					break;
				}

			case 1:
			case 2:
			case 4:
				{
					bMaxN = 20;
					break;
				}

			default:
				{
					bMaxN = 20;
					break;
				}

			}

			for (int n=0; n<bMaxN; n++)
			{
				if(DATA_ASC == bDataType)
				{
					if ( txtBuf[txtPos] == cSoftKeyTable[m][n])
					{
						CurSoftKeyPos.bYPos = m;
						CurSoftKeyPos.bXPos = n;
					}
				}
				else if (DATA_HEX == bDataType)
				{
					if ( txtBuf[txtPos] == cSoftKeyTableHEX[m][n])
					{
						CurSoftKeyPos.bYPos = m;
						CurSoftKeyPos.bXPos = n;
					}
				}
				else if (DATA_DEC == bDataType)
				{
					if ( txtBuf[txtPos] == cSoftKeyTableDEC[m][n])
					{
						CurSoftKeyPos.bYPos = m;
						CurSoftKeyPos.bXPos = n;
					}
				}
				
			}
		}
		
	}
	return -1;
}



int EditTextBoxIP(int y, const char title[], char text[], int retTime, int txtSize, int dataType)
{//y:行标号， title:显示标题，text:显示文本,同时也是更改后保存的位置, retTime:超时时间，txtSize:文本大小，当超过 17 会换行， dataType:数据格式
	struct KeyState keyState;
	char txtBuf[96];
	
	int txtPos = 0;
	int curPosInCharTable = 0;
	int charTableSize = 0;
	int haveTitle = 0;
	char *charTable;

	//这个在密码出错的时候，如是是静态全局变量会出被清空。
	char DecIPTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  };

	y = y * GetFontHeight();

	if (strlen(title) > 0)
	{
		haveTitle = 1;
	}

	memset(txtBuf, ' ', sizeof(txtBuf));
	if (txtSize == 0)
	{
		txtSize = sizeof(txtBuf);
	} else
	{
		txtSize += 2;
		if (txtSize > (int) sizeof(txtBuf))
		{
			txtSize = sizeof(txtBuf);
		}
	}

	text[txtSize - 2] = 0;
	int initTextSize = strlen(text);
	if (initTextSize > txtSize - 2)
	{
		initTextSize = txtSize - 2;
	}
	if (initTextSize > 0)
	{
		memcpy(txtBuf, text, initTextSize);
	}
	txtBuf[txtSize - 1] = 0;


	charTable = DecIPTable;//---cqj,  ----取值列表
	charTableSize = sizeof(DecIPTable);

	//DTRACE(DB_CQJ_CRITICAL, ("the table is %s\n", DecIPTable));
#if 0
	switch (dataType) {
	case DATA_HEX:
		charTable = HexCharTable;
		charTableSize = sizeof(HexCharTable);
		break;
	case DATA_DEC:
		charTable = DecIPTable;//---cqj,  ----取值列表
		charTableSize = sizeof(DecIPTable);
		break;
	case DATA_BIN:
		charTable = BinCharTable;
		charTableSize = sizeof(BinCharTable);
		break;	
	default:
		charTable = AscCharTable;
		charTableSize = sizeof(AscCharTable);
		break;
	}
#endif
	int totalLine = txtSize / ((LCD_SIM_WIDTH - 8) / GetFontWidth()) / 2 + 2;
	int dispPosX = 0;
	int dispPosY = y;
	for (int i = 0; i < totalLine; i++)
	{
		DrawStringAtLock("                            ", 0, y + GetFontHeight() * i, GUI_WHITE, GUI_BLACK);
	}

	//白底黑字
	drawPreference();
	
	while (1)
	{
	
		if (haveTitle == 1)
		{
			for (int i = 0; i <= totalLine; i++)
			{
				DrawRectLock(0, y - 1, LCD_SIM_WIDTH - 1, y + GetFontHeight() * i - 1);
			}
			DrawStringHCenterAtLock(title, LCD_SIM_WIDTH / 2, y, GUI_WHITE, GUI_BLACK);
		}
		for (int i = 0; i < txtSize; i++)
		{
			char tmp[2] = { 0, 0, };
			tmp[0] = txtBuf[i];

			dispPosX = (4 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			dispPosY = y + i * GetFontWidth() / 2 / (LCD_SIM_WIDTH - 7) * GetFontHeight();

			if (i == txtPos)
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_BLACK, GUI_WHITE);
			} else
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_WHITE, GUI_BLACK);
			}
		}

		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}

		LcdRefresh();		
		while (1)
		{
			Sleep(50);			
			keyState = GetKey();
			if (keyState.key != 0)
			{
				break;
			}
		
			if (keyState.idle > retTime / 1000 || keyState.key == KEY_ESC)
			{
				return -1;
			}
			
			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
		}

		
			if (keyState.key == KEY_ESC)
			{
				#if 0
				return -1;
				#else
				if (txtSize > 2)
				{
					memcpy(text, txtBuf, txtSize - 2);
				}
				return txtPos;
				#endif
			}
			if (keyState.key == KEY_OK)
			{
				if (txtSize > 2)
				{
					memcpy(text, txtBuf, txtSize - 2);
				}
				return txtPos;
			}
			if (keyState.key == KEY_UP)
			{
				//DTRACE(DB_CQJ_CRITICAL, ("cqj---iCurPos is %d\n", curPosInCharTable));	
				//DTRACE(DB_CQJ_CRITICAL, ("cqj---txtPos is %d\n", txtPos));
				if((txtPos == 3) ||(txtPos == 7) ||(txtPos == 11))//---控制不能设置.
				{
						
				}
				else
				{
					if ((txtPos == 0) ||(txtPos == 4) ||(txtPos == 8)||(txtPos == 12))
					{
						if( curPosInCharTable < 2)//控制到2,立即变为255
						{
							curPosInCharTable++;
							if (curPosInCharTable == 2)
							{
								txtBuf[txtPos+1] = charTable[5];
								txtBuf[txtPos+2] = charTable[5];
							}
							
							//txtBuf[txtPos] = charTable[curPosInCharTable];//取值显示
						}
					}
					else
					{//((txtBuf[0]&0xf) == 2)是因为值是0x32
					//	DTRACE(DB_CQJ_CRITICAL, ("cqj value is %x\n",txtBuf[0] ));
					//	if ( ((txtBuf[0]&0xf) == 2)||((txtBuf[4]&0xf) == 2)||((txtBuf[8]&0xf) == 2)||((txtBuf[12]&0xf) == 2))
//						DTRACE(DB_CQJ_CRITICAL, ("cqj curPosInCharTable is %x\n",curPosInCharTable ));
						curPosInCharTable++;
						if ( ((txtBuf[0]&0xf) == 2) && (txtPos < 3) && (txtPos > 0))
						{//---第一个ip
							if(((txtBuf[1]&0xf) == 5)&&(txtPos == 1))//--控制-第0位为2，第1位不超过5
							{
								if( curPosInCharTable > 5)
								{
									txtBuf[txtPos+1] = charTable[5];
									curPosInCharTable--;
								}
							}
							else if(((txtBuf[1]&0xf) == 5)&&(txtPos == 2))//--控制-第一位为5，第二位不超过5
							{
								if( curPosInCharTable > 5)
								{
									curPosInCharTable--;
								}
							}
							else
							{
								if((txtPos == 1)&&(curPosInCharTable == 5))//---控制第一位为5，第二位立即变为5
								{	
									txtBuf[txtPos+1] = charTable[5];
								}
								if (curPosInCharTable >= charTableSize)
								{
									curPosInCharTable = 0;
								}
							}
						}
						else if ( ((txtBuf[4]&0xf) == 2) && (txtPos < 7)&& (txtPos > 4))	
						{//---第二个ip
							if(((txtBuf[5]&0xf) == 5)&&(txtPos == 5))//--控制-第0位为2，第1位不超过5
							{
								if( curPosInCharTable > 5)
								{
									txtBuf[txtPos+1] = charTable[5];
									curPosInCharTable--;
								}
							}
							else if(((txtBuf[5]&0xf) == 5)&&(txtPos == 6))//--控制-第一位为5，第二位不超过5
							{
								if( curPosInCharTable > 5)
								{
									curPosInCharTable--;
								}
							}
							else
							{
								if((txtPos == 5)&&(curPosInCharTable == 5))//---控制第一位为5，第二位立即变为5
								{	
									txtBuf[txtPos+1] = charTable[5];
								}
								if (curPosInCharTable >= charTableSize)
								{
									curPosInCharTable = 0;
								}
							}
						}
						else if( ((txtBuf[8]&0xf) == 2) && (txtPos < 11)&& (txtPos > 8))
						{//---第三个ip
							if(((txtBuf[9]&0xf) == 5)&&(txtPos == 9))//--控制-第0位为2，第1位不超过5
							{
								if( curPosInCharTable > 5)
								{
									txtBuf[txtPos+1] = charTable[5];
									curPosInCharTable--;
								}
							}
							else if(((txtBuf[9]&0xf) == 5)&&(txtPos == 10))//--控制-第一位为5，第二位不超过5
							{
								if( curPosInCharTable > 5)
								{
									curPosInCharTable--;
								}
							}
							else
							{
								if((txtPos == 9)&&(curPosInCharTable == 5))//---控制第一位为5，第二位立即变为5
								{	
									txtBuf[txtPos+1] = charTable[5];
								}
								if (curPosInCharTable >= charTableSize)
								{
									curPosInCharTable = 0;
								}
							}
						}
						else if( ((txtBuf[12]&0xf) == 2) && (txtPos < 15)&& (txtPos > 12))
						{//---第四个ip
							if(((txtBuf[13]&0xf) == 5)&&(txtPos == 13))//--控制-第0位为2，第1位不超过5
							{
								if( curPosInCharTable > 5)
								{
									txtBuf[txtPos+1] = charTable[5];
									curPosInCharTable--;
								}
							}
							else if(((txtBuf[13]&0xf) == 5)&&(txtPos == 14))//--控制-第一位为5，第二位不超过5
							{
								if( curPosInCharTable > 5)
								{
									curPosInCharTable--;
								}
							}
							else
							{
								if((txtPos == 13)&&(curPosInCharTable == 5))//---控制第一位为5，第二位立即变为5
								{	
									txtBuf[txtPos+1] = charTable[5];
								}
								if (curPosInCharTable >= charTableSize)
								{
									curPosInCharTable = 0;
								}
							}
						}
						else
						{
							
							if (curPosInCharTable >= charTableSize)
							{
								curPosInCharTable = 0;
							}
						}
						//txtBuf[txtPos] = charTable[curPosInCharTable];//取值显示
					}
					txtBuf[txtPos] = charTable[curPosInCharTable];//取值显示
				}	
			/*
				curPosInCharTable++;
				if (curPosInCharTable >= charTableSize)
				{
					curPosInCharTable = 0;
				}
				txtBuf[txtPos] = charTable[curPosInCharTable];
			*/
			}
			if (keyState.key == KEY_DOWN)
			{
				if((txtPos == 3) ||(txtPos == 7) ||(txtPos == 11))//---控制不能设置.
				{
						
				}
				else
				{
					//curPosInCharTable--;
					if ((txtPos == 0) ||(txtPos == 4) ||(txtPos == 8)||(txtPos == 12))
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else if (((txtBuf[0]&0xf) == 2)&&(txtPos == 1))//第一个ip控制
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else if (((txtBuf[1]&0xf) == 5)&&(txtPos == 2))
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else if (((txtBuf[4]&0xf) == 2)&&(txtPos == 5))//第二个ip控制
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else if (((txtBuf[5]&0xf) == 5)&&(txtPos == 6))
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else if (((txtBuf[8]&0xf) == 2)&&(txtPos == 9))//第三个ip控制
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else if (((txtBuf[9]&0xf) == 5)&&(txtPos == 10))
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else if (((txtBuf[12]&0xf) == 2)&&(txtPos == 13))//第四个ip控制
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else if (((txtBuf[13]&0xf) == 5)&&(txtPos == 14))
					{
						if( curPosInCharTable > 0)
						{
							curPosInCharTable--;
						}
					}
					else
					{
						curPosInCharTable--;
						if (curPosInCharTable < 0)
						{
							curPosInCharTable = charTableSize - 1;
						}
					}		
					txtBuf[txtPos] = charTable[curPosInCharTable];	
				}				
				//txtBuf[txtPos] = charTable[curPosInCharTable];	
			}
			if (keyState.key == KEY_LEFT)
			{
				if (txtPos > 0)
				{
					txtPos--;
				}
				else if (txtPos == 0)
				{
					txtPos = (txtSize - 3);
				}
			}
			if (keyState.key == KEY_RIGHT)
			{
				if (txtPos < txtSize - 3)//   0~11
				{
					txtPos++;
				}
				else 
				{
					txtPos = 0;
				}
			}
		}
		
		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}
	

	return -1;
}


int EditTextBoxDate(int y, const char title[], char text[], int retTime, int txtSize, int dataType)
{//y:行标号， title:显示标题，text:显示文本, retTime:超时时间，txtSize:文本大小，当超过 17 会换行， dataType:数据格式
	struct KeyState keyState;
	char txtBuf[96];
	
	int txtPos = 0;
	int curPosInCharTable = 0;
	int charTableSize = 0;
	int haveTitle = 0;
	char *charTable;

	//这个在密码出错的时候，如是是静态全局变量会出被清空。
	char DecIPTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  };

	y = y * GetFontHeight();

	if (strlen(title) > 0)
	{
		haveTitle = 1;
	}

	memset(txtBuf, ' ', sizeof(txtBuf));
	if (txtSize == 0)
	{
		txtSize = sizeof(txtBuf);
	} else
	{
		txtSize += 2;
		if (txtSize > (int) sizeof(txtBuf))
		{
			txtSize = sizeof(txtBuf);
		}
	}

	text[txtSize - 2] = 0;
	int initTextSize = strlen(text);
	if (initTextSize > txtSize - 2)
	{
		initTextSize = txtSize - 2;
	}
	if (initTextSize > 0)
	{
		memcpy(txtBuf, text, initTextSize);
	}
	txtBuf[txtSize - 1] = 0;


	charTable = DecIPTable;//---cqj,  ----取值列表
	charTableSize = sizeof(DecIPTable);

	int totalLine = txtSize / ((LCD_SIM_WIDTH - 8) / GetFontWidth()) / 2 + 2;
	int dispPosX = 0;
	int dispPosY = y;
	for (int i = 0; i < totalLine; i++)
	{
		DrawStringAtLock("                            ", 0, y + GetFontHeight() * i, GUI_WHITE, GUI_BLACK);
	}
	
	drawPreference();

	while (1)
	{
	
		if (haveTitle == 1)
		{
			for (int i = 0; i <= totalLine; i++)
			{
				DrawRectLock(0, y - 1, LCD_SIM_WIDTH - 1, y + GetFontHeight() * i - 1);
			}
			DrawStringHCenterAtLock(title, LCD_SIM_WIDTH / 2, y, GUI_WHITE, GUI_BLACK);
		}
		for (int i = 0; i < txtSize; i++)
		{
			char tmp[2] = { 0, 0, };
			tmp[0] = txtBuf[i];

			dispPosX = (4 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			dispPosY = y + i * GetFontWidth() / 2 / (LCD_SIM_WIDTH - 7) * GetFontHeight();

			if (i == txtPos)
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_BLACK, GUI_WHITE);
			} else
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_WHITE, GUI_BLACK);
			}
		}
#if 0
		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}
#endif
		LcdRefresh();		
		while (1)
		{
			Sleep(50);			
			keyState = GetKey();
			if (keyState.key != 0)
			{
				break;
			}
		
			if (keyState.idle > retTime / 1000 || keyState.key == KEY_ESC)
			{
				return -1;
			}
			
			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
		}

		
			if (keyState.key == KEY_ESC)
			{
				return -1;
			}
			if (keyState.key == KEY_OK)
			{
				if (txtSize > 2)
				{
					memcpy(text, txtBuf, txtSize - 2);
				}
				return txtPos;
			}
			if (keyState.key == KEY_UP)
			{
				if((txtPos == 2) ||(txtPos == 5) )//---控制不能设置.
				{
						
				}
				else
				{
					curPosInCharTable++;
					if (curPosInCharTable >= charTableSize)
					{
						curPosInCharTable = 0;
					}
					txtBuf[txtPos] = charTable[curPosInCharTable];//取值显示		
				}
			}
			if (keyState.key == KEY_DOWN)
			{
				if((txtPos == 2) ||(txtPos == 5) )//---控制不能设置.
				{
						
				}
				else
				{
					curPosInCharTable--;
					if (curPosInCharTable < 0)
					{
						curPosInCharTable = charTableSize - 1;
					}
							
					txtBuf[txtPos] = charTable[curPosInCharTable];	
				}				
				//txtBuf[txtPos] = charTable[curPosInCharTable];	
			}
			if (keyState.key == KEY_LEFT)
			{
				if (txtPos > 0)
				{
					txtPos--;
				}
				else if (txtPos == 0)
				{
					txtPos = (txtSize - 3);
				}
			}
			if (keyState.key == KEY_RIGHT)
			{
				if (txtPos < txtSize - 3)//   0~11
				{
					txtPos++;
				}
				else 
				{
					txtPos = 0;
				}
			}
		
		}
	return -1;
}

int EditTextBoxTime(int y, const char title[], char text[], int retTime, int txtSize, int dataType)
{//y:行标号， title:显示标题，text:显示文本, retTime:超时时间，txtSize:文本大小，当超过 17 会换行， dataType:数据格式
	struct KeyState keyState;
	char txtBuf[96];
	
	int txtPos = 0;
	int curPosInCharTable = 0;
	int charTableSize = 0;
	int haveTitle = 0;
	char *charTable;

	//这个在密码出错的时候，如是是静态全局变量会出被清空。
	char DecIPTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  };

	y = y * GetFontHeight();

	if (strlen(title) > 0)
	{
		haveTitle = 1;
	}

	memset(txtBuf, ' ', sizeof(txtBuf));
	if (txtSize == 0)
	{
		txtSize = sizeof(txtBuf);
	} else
	{
		txtSize += 2;
		if (txtSize > (int) sizeof(txtBuf))
		{
			txtSize = sizeof(txtBuf);
		}
	}

	text[txtSize - 2] = 0;
	int initTextSize = strlen(text);
	if (initTextSize > txtSize - 2)
	{
		initTextSize = txtSize - 2;
	}
	if (initTextSize > 0)
	{
		memcpy(txtBuf, text, initTextSize);
	}
	txtBuf[txtSize - 1] = 0;


	charTable = DecIPTable;//---cqj,  ----取值列表
	charTableSize = sizeof(DecIPTable);

	int totalLine = txtSize / ((LCD_SIM_WIDTH - 8) / GetFontWidth()) / 2 + 2;
	int dispPosX = 0;
	int dispPosY = y;
	for (int i = 0; i < totalLine; i++)
	{
		DrawStringAtLock("                            ", 0, y + GetFontHeight() * i, GUI_WHITE, GUI_BLACK);
	}
	drawPreference();
	while (1)
	{
	
		if (haveTitle == 1)
		{
			for (int i = 0; i <= totalLine; i++)
			{
				DrawRectLock(0, y - 1, LCD_SIM_WIDTH - 1, y + GetFontHeight() * i - 1);
			}
			DrawStringHCenterAtLock(title, LCD_SIM_WIDTH / 2, y, GUI_WHITE, GUI_BLACK);
		}
		for (int i = 0; i < txtSize; i++)
		{
			char tmp[2] = { 0, 0, };
			tmp[0] = txtBuf[i];

			dispPosX = (4 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			dispPosY = y + i * GetFontWidth() / 2 / (LCD_SIM_WIDTH - 7) * GetFontHeight();

			if (i == txtPos)
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_BLACK, GUI_WHITE);
			} else
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_WHITE, GUI_BLACK);
			}
		}
#if 0
		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}
#endif
		LcdRefresh();		
		while (1)
		{
			Sleep(50);			
			keyState = GetKey();
			if (keyState.key != 0)
			{
				break;
			}
		
			if (keyState.idle > retTime / 1000 || keyState.key == KEY_ESC)
			{
				return -1;
			}
			
			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
		}

		
			if (keyState.key == KEY_ESC)
			{
				return -1;
			}
			if (keyState.key == KEY_OK)
			{
				if (txtSize > 2)
				{
					memcpy(text, txtBuf, txtSize - 2);
				}
				return txtPos;
			}
			if (keyState.key == KEY_UP)
			{
				if((txtPos == 2) ||(txtPos == 5) )//---控制不能设置.
				{
						
				}
				else
				{
					curPosInCharTable++;
					if (curPosInCharTable >= charTableSize)
					{
						curPosInCharTable = 0;
					}
					txtBuf[txtPos] = charTable[curPosInCharTable];//取值显示		
				}
			}
			if (keyState.key == KEY_DOWN)
			{
				if((txtPos == 2) ||(txtPos == 5) )//---控制不能设置.
				{
						
				}
				else
				{
					curPosInCharTable--;
					if (curPosInCharTable < 0)
					{
						curPosInCharTable = charTableSize - 1;
					}
							
					txtBuf[txtPos] = charTable[curPosInCharTable];	
				}				
				//txtBuf[txtPos] = charTable[curPosInCharTable];	
			}
			if (keyState.key == KEY_LEFT)
			{
				if (txtPos > 0)
				{
					txtPos--;
				}
				else if (txtPos == 0)
				{
					txtPos = (txtSize - 3);
				}
			}
			if (keyState.key == KEY_RIGHT)
			{
				if (txtPos < txtSize - 3)//   0~11
				{
					txtPos++;
				}
				else 
				{
					txtPos = 0;
				}
			}
		
		}
	return -1;
}


int EditTextBox(int y, const char title[], char text[], int retTime, int txtSize, int dataType)
{
	struct KeyState keyState;
	char txtBuf[96];
	int txtPos = 0;
	int curPosInCharTable = 0;
	int charTableSize = 0;
	char *charTable;
	int haveTitle = 0;

	y = y * GetFontHeight();

	if (strlen(title) > 0)
	{
		haveTitle = 1;
	}

	memset(txtBuf, ' ', sizeof(txtBuf));
	if (txtSize == 0)
	{
		txtSize = sizeof(txtBuf);
	} else
	{
		txtSize += 2;
		if (txtSize > (int) sizeof(txtBuf))
		{
			txtSize = sizeof(txtBuf);
		}
	}

	text[txtSize - 2] = 0;
	int initTextSize = strlen(text);
	if (initTextSize > txtSize - 2)
	{
		initTextSize = txtSize - 2;
	}
	if (initTextSize > 0)
	{
		memcpy(txtBuf, text, initTextSize);
	}
	txtBuf[txtSize - 1] = 0;

	switch (dataType) {
	case DATA_HEX:
		charTable = HexCharTable;
		charTableSize = sizeof(HexCharTable);
		break;
	case DATA_DEC:
		charTable = DecCharTable;
		charTableSize = sizeof(DecCharTable);
		break;
	case DATA_BIN:
		charTable = BinCharTable;
		charTableSize = sizeof(BinCharTable);
		break;	
	default:
		charTable = AscCharTable;
		charTableSize = sizeof(AscCharTable);
		break;
	}

	int totalLine = txtSize / ((LCD_SIM_WIDTH - 8) / GetFontWidth()) / 2 + 2;
	int dispPosX = 0;
	int dispPosY = y;
	for (int i = 0; i < totalLine; i++)
	{
		DrawStringAtLock("                            ", 0, y + GetFontHeight() * i, GUI_WHITE, GUI_BLACK);
	}
	while (1)
	{
		if (haveTitle == 1)
		{
			for (int i = 0; i <= totalLine; i++)
			{
				DrawRectLock(0, y - 1, LCD_SIM_WIDTH - 1, y + GetFontHeight() * i - 1);
			}
			DrawStringHCenterAtLock(title, LCD_SIM_WIDTH / 2, y, GUI_WHITE, GUI_BLACK);
		}
		for (int i = 0; i < txtSize; i++)
		{
			char tmp[2] = { 0, 0, };
			tmp[0] = txtBuf[i];

			dispPosX = (4 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			dispPosY = y + i * GetFontWidth() / 2 / (LCD_SIM_WIDTH - 7) * GetFontHeight();

			if (i == txtPos)
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_BLACK, GUI_WHITE);
			} else
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_WHITE, GUI_BLACK);
			}
		}

		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}

		LcdRefresh();
				
		while (1)
		{
			Sleep(50);
			keyState = GetKey();
			if (keyState.key != 0)
			{
				break;
			}
		
			if (keyState.idle > retTime / 1000 || keyState.key == KEY_ESC)
			{
				return -1;
			}
			
			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}
			
			if (g_fLcdRefresh)
			{			
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
		}

		if (keyState.key == KEY_ESC)
		{
			return -1;
		}
		if (keyState.key == KEY_OK)
		{
			if (txtSize > 2)
			{
				memcpy(text, txtBuf, txtSize - 2);
			}
			return txtPos;
		}
		if (keyState.key == KEY_UP)
		{
			curPosInCharTable++;
			if (curPosInCharTable >= charTableSize)
			{
				curPosInCharTable = 0;
			}
			txtBuf[txtPos] = charTable[curPosInCharTable];
		}
		if (keyState.key == KEY_DOWN)
		{
			curPosInCharTable--;
			if (curPosInCharTable < 0)
			{
				curPosInCharTable = charTableSize - 1;
			}
			txtBuf[txtPos] = charTable[curPosInCharTable];
		}
		if (keyState.key == KEY_LEFT)
		{
			if (txtPos > 0)
			{
				txtPos--;
			}
		}
		if (keyState.key == KEY_RIGHT)
		{
			if (txtPos < txtSize - 3)
			{
				txtPos++;
			}
		}
	
		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}
	}

	return -1;
}

int EditDateBox(int y, char title1[], char title2[], char text[], int retTime, int txtSize, int dataType)
{
	struct KeyState mkey;
	char txtBuf[96];
	int txtPos = 0;
	int curPosInCharTable = 0;
	int charTableSize = 0;
	char *charTable;
	int haveTitle = 0;
	int haveTitle2 = 0;

	ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());

	y = y * GetFontHeight();

	if (strlen(title1) > 0)
	{
		haveTitle = 1;
	}

	if(strlen(title2) > 0)
	{
		haveTitle2 = 1;
	}


	memset(txtBuf, ' ', sizeof(txtBuf));
	if (txtSize == 0)
	{
		txtSize = sizeof(txtBuf);
	} else
	{
		txtSize += 2;
		if (txtSize > (int) sizeof(txtBuf))
		{
			txtSize = sizeof(txtBuf);
		}
	}

	text[txtSize - 2] = 0;
	int initTextSize = strlen(text);
	if (initTextSize > txtSize - 2)
	{
		initTextSize = txtSize - 2;
	}
	if (initTextSize > 0)
	{
		memcpy(txtBuf, text, initTextSize);
	}
	txtBuf[txtSize - 1] = 0;

	switch (dataType) {
	case DATA_HEX:
		charTable = HexCharTable;
		charTableSize = sizeof(HexCharTable);
		break;
	case DATA_DEC:
		charTable = DecCharTable;
		charTableSize = sizeof(DecCharTable);
		break;
	case DATA_BIN:
		charTable = BinCharTable;
		charTableSize = sizeof(BinCharTable);
		break;	
	default:
		charTable = AscCharTable;
		charTableSize = sizeof(AscCharTable);
		break;
	}

	int totalLine = txtSize / ((LCD_SIM_WIDTH - 8) / GetFontWidth()) / 2 + 2;
	int dispPosX = 0;
	int dispPosY = y;
	for (int i = 0; i < totalLine; i++)
	{
		DrawStringAtLock("                            ", 0, y + GetFontHeight() * i, GUI_WHITE, GUI_BLACK);
	}

	//将选中光标移动到字符串最后一个字符
	txtPos = txtSize - 3;

	while (1)
	{
		if (haveTitle == 1)
		{
			DrawStringHCenterAtLock(title1, LCD_SIM_WIDTH / 2, GetFontHeight() + 2, GUI_WHITE, GUI_BLACK);
		}

		if(haveTitle2 == 1)
		{
			DrawStringAtLock(title2, LCD_SIM_WIDTH / 20 , 3 * GetFontHeight() + 8, GUI_WHITE, GUI_BLACK);
		}

		for (int i = 0; i < txtSize; i++)
		{
			char tmp[2] = { 0, 0, };
			tmp[0] = txtBuf[i];

			dispPosX = (LCD_SIM_WIDTH / 4 + 5 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			dispPosY = y + i * GetFontWidth() / 2 / (LCD_SIM_WIDTH - 7) * GetFontHeight() + 10;

			if (i == txtPos)
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_BLACK, GUI_WHITE);
			} else
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_WHITE, GUI_BLACK);
			}
		}

		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}

		LcdRefresh();

		while (1)
		{
			Sleep(50);
			mkey = GetKey();
			if (mkey.key != 0)
			{
				break;
			}

			if (mkey.idle > retTime / 1000 || mkey.key == KEY_ESC)
			{
				return -1;
			}

			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{			
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
		}

		if (mkey.key == KEY_ESC)
		{
			return -1;
		}
		if (mkey.key == KEY_OK)
		{
			if (txtSize > 2)
			{
				memcpy(text, txtBuf, txtSize - 2);
			}
			return txtPos;
		}
		if (mkey.key == KEY_UP)
		{
			curPosInCharTable++;
			if (curPosInCharTable >= charTableSize)
			{
				curPosInCharTable = 0;
			}
			txtBuf[txtPos] = charTable[curPosInCharTable];
		}
		if (mkey.key == KEY_DOWN)
		{
			curPosInCharTable--;
			if (curPosInCharTable < 0)
			{
				curPosInCharTable = charTableSize - 1;
			}
			txtBuf[txtPos] = charTable[curPosInCharTable];
		}
		if (mkey.key == KEY_LEFT)
		{
			if (txtPos > 0)
			{
				txtPos--;
			}
		}
		if (mkey.key == KEY_RIGHT)
		{
			if (txtPos < txtSize - 3)
			{
				txtPos++;
			}
		}

		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}
	}

	return -1;
}


int EditSpecBox(int y, char title1[], char text[], int retTime, int txtSize, int dataType,bool fMask)
{
	struct KeyState mkey;
	char txtBuf[96];
	int txtPos = 0;
	int curPosInCharTable = 0;
	int charTableSize = 0;
	char *charTable;
	int haveTitle = 0;
	bool fFirst = false;
	bool fModify = false;
	if(1 == txtSize)
	{
		sprintf(text,"%01d", 1);//
	}
	else if(2 == txtSize)
	{
		sprintf(text,"%02d", 1);//
	}
	else if(4 == txtSize)
	{
		sprintf(text,"%04d", 1);//
	}

	ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());

	y = y * GetFontHeight();

	if (strlen(title1) > 0)
	{
		haveTitle = 1;
	}

	memset(txtBuf, ' ', sizeof(txtBuf));
	if (txtSize == 0)
	{
		txtSize = sizeof(txtBuf);
	} else
	{
		txtSize += 2;
		if (txtSize > (int) sizeof(txtBuf))
		{
			txtSize = sizeof(txtBuf);
		}
	}

	text[txtSize - 2] = 0;
	int initTextSize = strlen(text);
	if (initTextSize > txtSize - 2)
	{
		initTextSize = txtSize - 2;
	}
	if (initTextSize > 0)
	{
		memcpy(txtBuf, text, initTextSize);
	}
	txtBuf[txtSize - 1] = 0;

	switch (dataType) {
	case DATA_HEX:
		charTable = HexCharTable;
		charTableSize = sizeof(HexCharTable);
		break;
	case DATA_DEC:
		charTable = DecCharTable;
		charTableSize = sizeof(DecCharTable);
		break;
	case DATA_DEC8:
		charTable = Dec8CharTable;
		charTableSize = sizeof(Dec8CharTable);
		break;
	case DATA_BIN:
		charTable = BinCharTable;
		charTableSize = sizeof(BinCharTable);
		break;	
	default:
		charTable = AscCharTable;
		charTableSize = sizeof(AscCharTable);
		break;
	}

	int totalLine = txtSize / ((LCD_SIM_WIDTH - 8) / GetFontWidth()) / 2 + 2;
	int dispPosX = 0;
	int dispPosY = y;


	//将选中光标移动到字符串最后一个字符
	txtPos = txtSize - 3;

	while (1)
	{
		if (haveTitle == 1)
		{
			DrawStringHCenterAtLock(title1, LCD_SIM_WIDTH / 2, y -  GetFontHeight()/4 + 2, GUI_WHITE, GUI_BLACK);
		}

		for (int i = 0; i < txtSize; i++)
		{
			char tmp[2] = { 0, 0, };
			tmp[0] = txtBuf[i];
			dispPosX = (LCD_SIM_WIDTH / 3 + 19 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			
			if (txtSize == 6)//4个字符显示
			{
				dispPosX = (LCD_SIM_WIDTH / 3 + 5 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			}
			else if(txtSize == 4)//2个字符显示
			{
				dispPosX = (LCD_SIM_WIDTH / 3 + 19 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			}
			else if(txtSize == 3)//单个字符显示
			{
				dispPosX = (LCD_SIM_WIDTH / 3 + 26 + i * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			}
		//	dispPosY = y + i * GetFontWidth() / 2 / (LCD_SIM_WIDTH - 7) * GetFontHeight() + 10;

			if (i == txtPos)
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_BLACK, GUI_WHITE);

				if( fMask && ((!fFirst && i < txtSize - 2) || !fModify))
				{
					fFirst = true;
					DrawStringAtLock("*", dispPosX, dispPosY + GetFontHeight(), GUI_BLACK, GUI_WHITE);	
				}

			} else
			{
				DrawStringAtLock(tmp, dispPosX, dispPosY + GetFontHeight(), GUI_WHITE, GUI_BLACK);

				if(fMask && (i < txtSize - 2))
					DrawStringAtLock("*", dispPosX, dispPosY + GetFontHeight(), GUI_WHITE, GUI_BLACK);
			}


		}


		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}

		LcdRefresh();

		if(fMask && fModify)
		{
			Sleep(300);

			dispPosX = (LCD_SIM_WIDTH / 3 + 5 + txtPos * GetFontWidth() / 2) % (LCD_SIM_WIDTH - 7);
			dispPosY = y + txtPos * GetFontWidth() / 2 / (LCD_SIM_WIDTH - 7) * GetFontHeight() + 10;

			DrawStringAtLock("*", dispPosX, dispPosY + GetFontHeight(), GUI_BLACK, GUI_WHITE);

			LcdRefresh();

			fModify = false;
		}

		while (1)
		{
			Sleep(50);
			mkey = GetKey();
			if (mkey.key != 0)
			{
				break;
			}

			if (mkey.idle > retTime / 1000 || mkey.key == KEY_ESC)
			{
				return -1;
			}

			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}

			if (g_fLcdRefresh)
			{			
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
		}

		if (mkey.key == KEY_ESC)
		{
			return -1;
		}
		if (mkey.key == KEY_OK)
		{
			if (txtSize > 2)
			{
				memcpy(text, txtBuf, txtSize - 2);
			}
			return txtPos;
		}
		if (mkey.key == KEY_UP)
		{
			curPosInCharTable++;
			if (curPosInCharTable >= charTableSize)
			{
				curPosInCharTable = 0;
			}
			txtBuf[txtPos] = charTable[curPosInCharTable];
			fModify = true;
		}
		if (mkey.key == KEY_DOWN)
		{
			curPosInCharTable--;
			if (curPosInCharTable < 0)
			{
				curPosInCharTable = charTableSize - 1;
			}
			txtBuf[txtPos] = charTable[curPosInCharTable];
			fModify = true;
		}
		if (mkey.key == KEY_LEFT)
		{
			if (txtPos > 0)
			{
				txtPos--;
			}
		}
		if (mkey.key == KEY_RIGHT)
		{
			if (txtPos < txtSize - 3)
			{
				txtPos++;
			}
		}

		for (int i = 0; i < charTableSize; i++)
		{
			if (txtBuf[txtPos] == charTable[i])
			{
				curPosInCharTable = i;
				break;
			}
		}
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////
CListBoxEx::CListBoxEx()
{
	curItem = 0;
	startItem = 0;
	totalItem = 0;
	haveTitle = 0;
	RollStatus = 0;
	containItem = 0;
	Medium = 0;
}


CListBoxEx::CListBoxEx(BYTE tempcontainItem)
{
	curItem = 0;
	startItem = 0;
	totalItem = 0;
	haveTitle = 0;
	RollStatus = 0;
	containItem = tempcontainItem;
	Medium = 0;
}

CListBoxEx::~CListBoxEx()
{

}

	void CListBoxEx::RollStatusFunc1(BYTE rolway)
	{
		if (rolway == 1)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[3], 144, 48); 
				DrawBmp16x16(iconRoll[3], 144, 64); 			
				DrawBmp16x16(iconRoll[3], 144, 80); 		
				DrawBmp16x16(iconRoll[2], 144, 96); 		
			break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 64);
				DrawBmp16x16(iconRoll[3], 144, 80);
				DrawBmp16x16(iconRoll[3], 144, 96); 
				DrawBmp16x16(iconRoll[3], 144, 112);	
				DrawBmp16x16(iconRoll[2], 144, 128);
			break;
			default:
				;
			}
		}
		else if (rolway == 2)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 48); 		
			break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 72);
				DrawBmp16x16(iconRoll[2], 144, 88);
			break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 112);
				DrawBmp16x16(iconRoll[2], 144, 128);
			break;
			default:
				;
			}
		}
		else if (rolway == 3)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 48); 		
			break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 56);
				DrawBmp16x16(iconRoll[2], 144, 72);
			break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 80);
				DrawBmp16x16(iconRoll[2], 144, 96);
			break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 112);
				DrawBmp16x16(iconRoll[2], 144, 128);
			break;
			default:
				;
			}
		}
		else if(rolway == 4)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 48); 		
			break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 51);
				DrawBmp16x16(iconRoll[2], 144, 64);
			break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 67);
				DrawBmp16x16(iconRoll[2], 144, 83);
			break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 86);
				DrawBmp16x16(iconRoll[2], 144, 102);
			break;
			case 5:
				DrawBmp16x16(iconRoll[0], 144, 112);
				DrawBmp16x16(iconRoll[2], 144, 128);			
			break;
			default:
				;
			}
		}
		else if (rolway == 5)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 48); 		
			break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 48);
				DrawBmp16x16(iconRoll[2], 144, 64);
			break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 64);
				DrawBmp16x16(iconRoll[2], 144, 80);
			break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 80);
				DrawBmp16x16(iconRoll[2], 144, 96);
			break;
			case 5:
				DrawBmp16x16(iconRoll[0], 144, 96);
				DrawBmp16x16(iconRoll[2], 144, 112);			
			break;
			case 6:
				DrawBmp16x16(iconRoll[0], 144, 112);
				DrawBmp16x16(iconRoll[2], 144, 128);			
			break;
			default:
				;
			}
		}
		else if (rolway == 6)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 44); 		
				break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 46);
				DrawBmp16x16(iconRoll[2], 144, 58);
				break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 60);
				DrawBmp16x16(iconRoll[2], 144, 72);
				break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 74);
				DrawBmp16x16(iconRoll[2], 144, 86);
				break;
			case 5:
				DrawBmp16x16(iconRoll[0], 144, 88);
				DrawBmp16x16(iconRoll[2], 144, 100);			
				break;
			case 6:
				DrawBmp16x16(iconRoll[0], 144, 102);
				DrawBmp16x16(iconRoll[2], 144, 114);			
				break;
			case 7:
				DrawBmp16x16(iconRoll[0], 144, 116);
				DrawBmp16x16(iconRoll[2], 144, 128);			
				break;
			default:
				;
			}
		}
		else if (rolway == 7)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 44); 		
				break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 44);
				DrawBmp16x16(iconRoll[2], 144, 56);
				break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 56);
				DrawBmp16x16(iconRoll[2], 144, 68);
				break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 68);
				DrawBmp16x16(iconRoll[2], 144, 80);
				break;
			case 5:
				DrawBmp16x16(iconRoll[0], 144, 80);
				DrawBmp16x16(iconRoll[2], 144, 92);			
				break;
			case 6:
				DrawBmp16x16(iconRoll[0], 144, 92);
				DrawBmp16x16(iconRoll[2], 144, 104);			
				break;
			case 7:
				DrawBmp16x16(iconRoll[0], 144, 104);
				DrawBmp16x16(iconRoll[2], 144, 116);			
				break;
			case 8:
				DrawBmp16x16(iconRoll[0], 144, 116);
				DrawBmp16x16(iconRoll[2], 144, 128);			
				break;
			default:
				;
			}
		}
		else if (rolway == 8)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 40); 		
				break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 43);
				DrawBmp16x16(iconRoll[2], 144, 51);
				break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 54);
				DrawBmp16x16(iconRoll[2], 144, 62);
				break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 65);
				DrawBmp16x16(iconRoll[2], 144, 73);
				break;
			case 5:
				DrawBmp16x16(iconRoll[0], 144, 76);
				DrawBmp16x16(iconRoll[2], 144, 84);			
				break;
			case 6:
				DrawBmp16x16(iconRoll[0], 144, 87);
				DrawBmp16x16(iconRoll[2], 144, 95);			
				break;
			case 7:
				DrawBmp16x16(iconRoll[0], 144, 98);
				DrawBmp16x16(iconRoll[2], 144, 106);			
				break;
			case 8:
				DrawBmp16x16(iconRoll[0], 144, 109);
				DrawBmp16x16(iconRoll[2], 144, 117);			
				break;
			case 9:
				DrawBmp16x16(iconRoll[0], 144, 120);
				DrawBmp16x16(iconRoll[2], 144, 128);			
				break;
			default:
				;
			}
		}
		else if (rolway == 9)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 38); 		
				break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 42);
				DrawBmp16x16(iconRoll[2], 144, 48);
				break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 52);
				DrawBmp16x16(iconRoll[2], 144, 58);
				break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 62);
				DrawBmp16x16(iconRoll[2], 144, 68);
				break;
			case 5:
				DrawBmp16x16(iconRoll[0], 144, 72);
				DrawBmp16x16(iconRoll[2], 144, 78); 		
				break;
			case 6:
				DrawBmp16x16(iconRoll[0], 144, 82);
				DrawBmp16x16(iconRoll[2], 144, 88); 		
				break;
			case 7:
				DrawBmp16x16(iconRoll[0], 144, 92);
				DrawBmp16x16(iconRoll[2], 144, 98); 		
				break;
			case 8:
				DrawBmp16x16(iconRoll[0], 144, 102);
				DrawBmp16x16(iconRoll[2], 144, 108); 		
				break;
			case 9:
				DrawBmp16x16(iconRoll[0], 144, 112);
				DrawBmp16x16(iconRoll[2], 144, 118);			
				break;
			case 10:
				DrawBmp16x16(iconRoll[0], 144, 122);
				DrawBmp16x16(iconRoll[2], 144, 128);			
				break;
			default:
				;
			}
		}
		else if (rolway == 10)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 38); 		
				break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 41);
				DrawBmp16x16(iconRoll[2], 144, 47);
				break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 50);
				DrawBmp16x16(iconRoll[2], 144, 56);
				break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 59);
				DrawBmp16x16(iconRoll[2], 144, 65);
				break;
			case 5:
				DrawBmp16x16(iconRoll[0], 144, 68);
				DrawBmp16x16(iconRoll[2], 144, 74); 		
				break;
			case 6:
				DrawBmp16x16(iconRoll[0], 144, 77);
				DrawBmp16x16(iconRoll[2], 144, 83); 		
				break;
			case 7:
				DrawBmp16x16(iconRoll[0], 144, 86);
				DrawBmp16x16(iconRoll[2], 144, 92); 		
				break;
			case 8:
				DrawBmp16x16(iconRoll[0], 144, 95);
				DrawBmp16x16(iconRoll[2], 144, 101); 		
				break;
			case 9:
				DrawBmp16x16(iconRoll[0], 144, 104);
				DrawBmp16x16(iconRoll[2], 144, 110);			
				break;
			case 10:
				DrawBmp16x16(iconRoll[0], 144, 113);
				DrawBmp16x16(iconRoll[2], 144, 119);			
				break;
			case 11:
				DrawBmp16x16(iconRoll[0], 144, 122);
				DrawBmp16x16(iconRoll[2], 144, 128);			
				break;
			default:
				;
			}
		}
		else if (rolway == 11)
		{
			switch (RollStatus)
			{
			case 1:
				DrawBmp16x16(iconRoll[0], 144, 32);
				DrawBmp16x16(iconRoll[2], 144, 40); 		
			break;
			case 2:
				DrawBmp16x16(iconRoll[0], 144, 40);
				DrawBmp16x16(iconRoll[2], 144, 48);
			break;
			case 3:
				DrawBmp16x16(iconRoll[0], 144, 48);
				DrawBmp16x16(iconRoll[2], 144, 56);
			break;
			case 4:
				DrawBmp16x16(iconRoll[0], 144, 56);
				DrawBmp16x16(iconRoll[2], 144, 64);
			break;
			case 5:
				DrawBmp16x16(iconRoll[0], 144, 64);
				DrawBmp16x16(iconRoll[2], 144, 72); 		
			break;
			case 6:
				DrawBmp16x16(iconRoll[0], 144, 72);
				DrawBmp16x16(iconRoll[2], 144, 80); 		
			break;
			case 7:
				DrawBmp16x16(iconRoll[0], 144, 80);
				DrawBmp16x16(iconRoll[2], 144, 88); 		
			break;
			case 8:
				DrawBmp16x16(iconRoll[0], 144, 88);
				DrawBmp16x16(iconRoll[2], 144, 96); 		
			break;
			case 9:
				DrawBmp16x16(iconRoll[0], 144, 96);
				DrawBmp16x16(iconRoll[2], 144, 104);			
			break;
			case 10:
				DrawBmp16x16(iconRoll[0], 144, 104);
				DrawBmp16x16(iconRoll[2], 144, 112);			
			break;
			case 11:
				DrawBmp16x16(iconRoll[0], 144, 112);
				DrawBmp16x16(iconRoll[2], 144, 120);			
			break;
			case 12:
				DrawBmp16x16(iconRoll[0], 144, 120);
				DrawBmp16x16(iconRoll[2], 144, 128);			
			break;
			default:
				;
			}
		}
		else
		{
			
		}
	}
	
	
	//滚动条变量
	/************************************************************
	函数名:RollStatusFunc
	函数功能:设置滚动条状态
	输入参数:
	输出参数:
	返回值:
	Author:chen qiao jun 
	Date:
	************************************************************/
	void CListBoxEx::RollStatusFunc()
	{
	
		if (RolWay>=1 && RolWay<=11)
		{
			RollStatusFunc1(RolWay);
		}
	}

	


	int CListBoxEx::Show(int x, const char title[], struct ListBoxExItem menu[], int retKey, int retTime,bool fFocus)
	{
start:	struct KeyState mkey;
		BYTE recoverkey = 0;//修复弹起按键的移动
		char usbBuf[5] = "USB";
		int iMonitorID = GetDispMonitorID();
	#if 0
		char title[20];
		if (title2 != NULL)
		{	
			memset(title, 0, 20);
			memcpy(title,title2,strlen(title2) );
		}
	#endif
		
		key = -1;
		if (title!=NULL && strlen(title) > 0)
		{
			haveTitle = 1;
		}
	
		mkey.key = 'R';
		mkey.idle = 0;
	
		WORD TmptotalItem = 0; 
		char szText[64];
	
		while (menu[TmptotalItem].text != NULL )//&& menu[TmptotalItem].text != 0)
		{
			TmptotalItem++;
		}
		//DTRACE(0, ("---------TmptotalItem is %d", TmptotalItem));
			
		if (TmptotalItem < 1)
		{
			return -1;
		}
		
		if (TmptotalItem < totalItem)
		{
			startItem = 0;
			curItem = 0;
		}
		
		totalItem = TmptotalItem;
		ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());//为了不清掉头尾
		
		RolWay = totalItem - 7;//选择滚动条数目
	//	DTRACE(DB_CRITICAL, ("cqj --rolway is %d\n", totalItem));
		while (1)
		{//ListBoxExItem = totalItem
			
			if ( (totalItem>8) || ((totalItem==8) && (haveTitle==1)) )
			{
				RollStatus = startItem + 1;
	
				if(containItem == 0)//新增加成员
				{
					RollStatusFunc();//控制翻页不显示
				}
			}

			if (iMonitorID>0 && iMonitorID<THRD_MNTR_NUM)
				UpdThreadRunClick(iMonitorID);

			if (mkey.key != 0)
			{
				int cnt = 0, no = 0;
				while (menu[startItem + cnt].text != 0)  //这个地方不要和0比较，text是指向字符串的指针，和NULL比较更合理  --QLS
				{
					//Medium = (160 - (7*strlen(menu[startItem + cnt].text)))/2;//决定是否中间显示
					
					if (haveTitle > 0 )
					{//title
						DrawStringHCenterAtLock(title, LCD_GET_XSIZE() / 2, GetFontHeight(), GUI_WHITE, GUI_BLACK);
					}
	
					if (cnt + 2 < GetMaxLinePerScreen() - haveTitle)
					{
						if (menu[startItem + cnt].bNo == 0xFF)
						{
							sprintf(szText,"%d.%s",startItem + cnt + 1 - no,menu[startItem + cnt].text);
	
						} else if (menu[startItem + cnt].bNo == 0xFE)
						{
							sprintf(szText,"%s",menu[startItem + cnt].text);
							no++;
						} else
						{
							sprintf(szText,"%d.%s",menu[startItem + cnt].bNo,menu[startItem + cnt].text);
							no++;
						}
					//	DTRACE(DB_CRITICAL, ("cqj-- display,startItem is %d, cnt is %d,totalitem is %d\n",startItem,cnt, totalItem));
						if ((startItem + cnt) == curItem && fFocus)
						{
							if (menu[startItem + cnt].bNo == 0xFE)//没有全黑
							{
								DrawStringAtLock(szText, x+Medium/* + GetFontWidth()/2*/, (cnt + 1			//20140527-1
										+ haveTitle) * GetFontHeight(), GUI_BLACK, GUI_WHITE);
							} else
							{
								DrawStringAtLock(szText, x+Medium/* + GetFontWidth()/2*/, (cnt + 1			//20140527-1
										+ haveTitle) * GetFontHeight(), GUI_BLACK, GUI_WHITE);
							}
						} 
						else
						{
							if (menu[startItem + cnt].bNo == 0xFE)
							{
								DrawStringAtLock(szText, x+Medium/* + GetFontWidth()/2*/, (cnt + 1			//20140527-1
										+ haveTitle) * GetFontHeight(), GUI_WHITE, GUI_BLACK);
							} else
							{
								DrawStringAtLock(szText, x+Medium/* + GetFontWidth()/2*/, (cnt + 1			//20140527-1
										+ haveTitle) * GetFontHeight(), GUI_WHITE, GUI_BLACK);
							}
						}
					}
					cnt++;
				}
				g_fLcdRefresh = true;
			}
	
			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				int iRet = DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;

				if (iRet == 100)
				{
					goto start;
				}
			}
			
			if (g_fLcdRefresh)
			{			
				g_fLcdRefresh = false;
				LcdRefresh();			
			}
	
			//while (1)
			{
				//Sleep(50);
				mkey = GetKey();
				//DTRACE(0, ("the key idle is %d",mkey.idle));
			
				//if (mkey.key != 0)
				//{
				//	break;
				//}
				if(title != NULL)
				{
					if ((!IsInUsbProcess()) && (memcmp((char *)title, usbBuf, 3) == 0))//---cqj
					{
						return MENU_TIMEOUT;			
					}
				}
			
				if(!IsMountUsb())
				{
					if (mkey.idle > retTime / 1000)//idle 是1s加1
					{
						BlightOn(false);
						g_fLcdRefresh = true;
						key = KEY_NULL;
						//clear_screen_rec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
						return MENU_TIMEOUT;
					}
				}
			}
		
			Sleep(20);
			//DTRACE(DB_CRITICAL, ("cqj-- mkey.key is %d\n",mkey.key));//---没有按键按下，状态是0
	
			if (mkey.key == 0)
				continue;
			
			//DTRACE(DB_CRITICAL, ("cqj-- retKey is %d\n",retKey));//固定
			
			//DTRACE(DB_CRITICAL, ("cqj--item is %d, curItem is %d\n",item, curItem));
	
			item = curItem;
			if (mkey.key == (retKey & 0xFF) || mkey.key == ((retKey >> 8) & 0xFF) || mkey.key == ((retKey >> 16) & 0xFF)
					|| mkey.key == ((retKey >> 24) & 0xFF) || mkey.key == KEY_ESC)
			{
				key = mkey.key;
				g_fLcdRefresh = true;
				break;//根据传进来参数，提前退出
			}
			if (mkey.key == KEY_OK)
			{
				key = KEY_OK;
				if (menu[curItem].func != NULL)
				{
					if (menu[curItem].func(menu[curItem].arg) >= 0)//鐢辫皟鐢ㄦ帶鍒舵槸鍚﹂��鍑?				
					{
						g_fLcdRefresh = true;
						break;
					}
				} else
				{
					g_fLcdRefresh = true;
					break;
				}
				ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
			}
			else if (mkey.key == KEY_DOWN)
			{
				curItem++;
				if (curItem >= totalItem)
				{
					curItem = 0;
				}
				//add by qiaojun.chen
				ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
			}
			else if (mkey.key == KEY_UP)
			{
				curItem--;		
				if (curItem < 0)
				{
					curItem = totalItem - 1;
				}
				//add by qiaojun.chen
				ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
			}
			
			while (startItem + GetMaxLinePerScreen() - haveTitle <= curItem + 2)
			{
				startItem++;
				if (startItem > totalItem)
				{
					startItem = totalItem;
				}
			}
			while (startItem > curItem)
			{
				startItem--;
			}
		}
	
		ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
		return 0;
	}



	///////////////////////
	int getSoftKeyMAC(const char *const pTitle,  char *const pText, int retTime, int txtSize)
	{
		BYTE bDataType = DATA_HEX;
		if((pText==NULL) || (retTime<=0) || (txtSize<=0))
		{
			return -1;
		}

		//bDataType = DATA_DEC;

		struct KeyState keyState;
		char   txtBuf[96]  = {0};
		int    txtPos      = 0;//输入栏光标
		TCurSoftKeyPos   CurSoftKeyPos;  //软键盘光标
		bool fUpAreaActive   = true;	 //输入栏是否为当前活动区域
		//bool fDownAreaActive = false;	 //软键盘是否为当前活动区域
		bool  fIsHaveTitle = false;
		DWORD dwLastDisClick = 0;
		BYTE  bCount = 0;
		BYTE bCount2 = 0;

		//BYTE mCQJMAX;
		BYTE bmMAX;

		memset(&CurSoftKeyPos, 0x00, sizeof(CurSoftKeyPos));
		if ( strlen(pTitle) > 0 )
		{
			fIsHaveTitle = true;
		}

		memset(txtBuf, ' ', sizeof(txtBuf));
		txtSize += 2;
		if (txtSize > (int) sizeof(txtBuf))
		{
			txtSize = sizeof(txtBuf);
		}


		pText[txtSize - 2] = 0;
		int initTextSize = strlen(pText);
		if (initTextSize > txtSize - 2)
		{
			initTextSize = txtSize - 2;
		}
		if (initTextSize > 0)
		{
			memcpy(txtBuf, pText, initTextSize);
		}
		txtBuf[txtSize - 1] = 0;

		BYTE bTotalLine = 2;//---cqj
		BYTE bXPos = 0;
		BYTE bYPos = 16;
		for (int i = 0; i<bTotalLine; i++)//cqj
		{
			DrawStringAtLock("                            ", 0, (GetFontHeight()*(i+1)), GUI_WHITE, GUI_BLACK);
		}
		ClearScreenRec(0, 16, 160, 144);	//清屏。

		while (1)
		{
			for (int i=0; i<bTotalLine; i++)
			{	
				//DTRACE(0,("--------NO TO HERE ?"));
				DrawRectLock(0, 14, LCD_SIM_WIDTH - 1, (GetFontHeight()*(i+2))-1);
				drawBY();	
			}
			if(DATA_ASC == bDataType)
			{
				bmMAX = 5;
			}
			else if((DATA_HEX == bDataType) || (DATA_DEC == bDataType))
			{
				bmMAX = 1;
				DrawBmp16x16(iconArrow[0], 8, 95);
				DrawStringAtLock("	进入键盘 ", 24, 95, GUI_WHITE, GUI_BLACK);
				DrawBmp16x16(iconArrow[1], 8,111);
				DrawStringAtLock("	退出键盘 ", 24, 111, GUI_WHITE, GUI_BLACK);
			}
			if ( fIsHaveTitle )
			{
				DrawStringHCenterAtLock(pTitle, LCD_SIM_WIDTH / 2, bYPos, GUI_WHITE, GUI_BLACK);
			}

			for (int i=0; i<txtSize; i++)
			{
				char tmp[2] = { 0 };
				tmp[0] = txtBuf[i];
				bXPos = ( i * 8);//设置密码间隔，
				if (i == txtPos)
				{
					if (bCount < 6)//cqj ---
					{
						bCount++;
					}
					else
					{
						bCount = 0;
					}

					//if ( (GetClick()%2) == 1)
					if(fUpAreaActive)
					{
						if (bCount < 3)//慢闪
						{////聚焦
							DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_BLACK, GUI_WHITE);
						}
						else
						{//没有聚焦
							DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_WHITE, GUI_BLACK);
						}
					}
					else
					{
						DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_BLACK, GUI_WHITE);
					}

				} 
				else
				{//控制不止显示一位
					DrawStringAtLock(tmp, bXPos, GetFontHeight()+bYPos, GUI_WHITE, GUI_BLACK);
				}
			}

			//根据传进来的参数画不同的键盘

			for (int m=0; m<1; m++)//画键盘
			{
				BYTE bTmp = m + 4;
				for (int n=0; n<20; n++)
				{	
					char tmp[2] = { 0,0};
					tmp[0] = cSoftKeyTableHEX[m][n];
					if ( (m == CurSoftKeyPos.bYPos) && (n==CurSoftKeyPos.bXPos) && (!fUpAreaActive))
					{//	DTRACE(0, ("cqj---%d, %d, %d ", CurSoftKeyPos.bYPos,CurSoftKeyPos.bXPos,fUpAreaActive));
						if (bCount2 < 6)//cqj ---
						{
							bCount2++;
						}
						else
						{
							bCount2 = 0;
						}
						if ( bCount2 > 2 )
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_BLACK, GUI_WHITE);
						}
						else
						{
							//bCount2 = 0;
							DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_WHITE, GUI_BLACK);
						}

						//DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_BLACK, GUI_WHITE);
					}
					else
					{
						DrawStringAtLock(tmp, n*8+16, bTmp*14, GUI_WHITE, GUI_BLACK);
					}

					if ((m==0) && (n==18))//---cqj---if (  ( (m==0) &&(n==9)  )
					{
						break;
					}
				}			
			}




			LcdRefresh();
			//while (1)
			{
				Sleep(50);

				DWORD dwTick = GetTick();
				if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
				{
					DrawStateTask();
					g_fLcdRefresh = true;
					g_dwStateTaskClick = dwTick;
				}

				if (g_fLcdRefresh)
				{			
					g_fLcdRefresh = false;
					LcdRefresh();			
				}

				keyState = GetKey();

				if (keyState.key != 0) 
				{
					if (keyState.key == KEY_ESC)
					{
#if 1
						if (txtSize > 2)
						{
							memcpy(pText, txtBuf, txtSize - 2);
						}
						ClearScreenRec(0, 128, 160, 144);
						return txtPos;
#else
						ClearScreenRec(0, 128, 160, 144);//ClearScreenRec(0, 145, LCD_SIM_WIDTH2, 160);
						return -1;
#endif
					}
				}
				else
				{
					if (  keyState.idle > (retTime/1000)  )
					{
						return -1;
					}
				}

			}

			switch(fUpAreaActive)
			{
			case  true:
				{
					if ( keyState.key == KEY_OK )
					{
						if (txtSize > 2)
						{
							memcpy(pText, txtBuf, txtSize - 2);
						}
						return txtPos;
					}
					if (keyState.key == KEY_UP)
					{
						;
					}
					if (keyState.key == KEY_DOWN)
					{
						fUpAreaActive = false;
					}
					if (keyState.key == KEY_LEFT)
					{
						if (txtPos > 0)
						{
							txtPos--;
							if ((txtPos + 1)%3 == 0)
							{
								txtPos--;

							}
						}
					}
					if (keyState.key == KEY_RIGHT)
					{
						if (txtPos < txtSize - 3)
						{
							txtPos++;
							if ((txtPos + 1)%3 == 0)
							{
								txtPos++;

							}
						}
					}
					break;
				}

			case false:
				{
					//for (int i=0; i<bTotalLine; i++)
					//{
					//	DrawRectLock(0, 14, LCD_SIM_WIDTH - 1, (GetFontHeight()*(i+2))-1);
					//}

					if (keyState.key == KEY_OK)
					{
						fUpAreaActive = true;
					}
					if (keyState.key == KEY_UP)
					{
						switch (CurSoftKeyPos.bYPos)
						{
						case 0:
							{
								fUpAreaActive = true;
								break;
							}
						case 1:
							{//cqj---if( (CurSoftKeyPos.bXPos<=9) && (CurSoftKeyPos.bXPos>=0) )
								if( (CurSoftKeyPos.bXPos<=13) && (CurSoftKeyPos.bXPos>=4) )
								{
									CurSoftKeyPos.bYPos--;
									CurSoftKeyPos.bXPos -= 4;
								}
								else if((CurSoftKeyPos.bXPos<4) || (CurSoftKeyPos.bXPos>13))
								{
									fUpAreaActive = true;
								}
								break;
							}
						case 2:
							{
								//if( (CurSoftKeyPos.bXPos<=19) && (CurSoftKeyPos.bXPos>=0) )
								if( (CurSoftKeyPos.bXPos<=17) && (CurSoftKeyPos.bXPos>=0) )
								{
									CurSoftKeyPos.bYPos--;

								}
								break;
							}
						case 3:
						case 4:
							{
								//if( (CurSoftKeyPos.bXPos<=11) && (CurSoftKeyPos.bXPos>=0) )
								if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
								{
									CurSoftKeyPos.bYPos--;
								}
								break;
							}
						default:
							{
								break;
							}
						}
					}

					if (keyState.key == KEY_DOWN)
					{
						switch (CurSoftKeyPos.bYPos)
						{
						case 0:
							{//---cqj
								//1 if( (CurSoftKeyPos.bXPos<=9) && (CurSoftKeyPos.bXPos>=0) )

								//if(DATA_HEX == bDataType)
								//{
								//	//CurSoftKeyPos.bYPos++;
								//	//if(CurSoftKeyPos.bXPos == 15)
								//	//{

								//	//}
								//}

								break;
							}
						case 1:
							{
								if( (CurSoftKeyPos.bXPos<=19) && (CurSoftKeyPos.bXPos>=0) )
								{
									CurSoftKeyPos.bYPos++;
								}
								break;
							}
						case 2:
							{
								if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
								{
									CurSoftKeyPos.bYPos++;
								}
								break;
							}
						case 3:
							{
								//if( (CurSoftKeyPos.bXPos<=11) && (CurSoftKeyPos.bXPos>=0) )
								if( (CurSoftKeyPos.bXPos<=15) && (CurSoftKeyPos.bXPos>=0) )
								{
									CurSoftKeyPos.bYPos++;
								}
								break;
							}
						default:
							{
								break;
							}
						}
					}
					if (keyState.key == KEY_LEFT)
					{//DTRACE(0, ("the key is %d",CurSoftKeyPos.bXPos ));//CurSoftKeyPos.bXPos = 1,代表第一个
						switch(CurSoftKeyPos.bYPos)
						{
						case 0:


							if (CurSoftKeyPos.bXPos > 0)
							{
								CurSoftKeyPos.bXPos--;

							}
							else
							{
								CurSoftKeyPos.bXPos = 16;
							}

							break;
						case 1:
							if (CurSoftKeyPos.bXPos > 0)
							{
								CurSoftKeyPos.bXPos--;
							}
							else
							{
								CurSoftKeyPos.bXPos = 9;
								CurSoftKeyPos.bYPos--;
							}
							break;
						case 2:
						case 3:
							if (CurSoftKeyPos.bXPos > 0)
							{
								CurSoftKeyPos.bXPos--;
							}
							else
							{
								CurSoftKeyPos.bXPos = 17;

								CurSoftKeyPos.bYPos--;
							}	
							break;
						case 4:
							if (CurSoftKeyPos.bXPos > 0)
							{
								CurSoftKeyPos.bXPos--;
							}
							else
							{
								CurSoftKeyPos.bXPos = 15;
								CurSoftKeyPos.bYPos--;
							}	
							break;
						}



					}
					if (keyState.key == KEY_RIGHT)
					{
						switch(CurSoftKeyPos.bYPos)
						{
						case 0:
							{
								//if (CurSoftKeyPos.bXPos < 9)//cqj


								if (CurSoftKeyPos.bXPos < 16)//  17--->BUF[17]
								{
									CurSoftKeyPos.bXPos++;
								}
								else
								{
									CurSoftKeyPos.bXPos = 0;
								}



								break;
							}

						case 1:
						case 2:		
						case 4:	
							{
								//if (CurSoftKeyPos.bXPos < 19)
								if (CurSoftKeyPos.bXPos < 17)
								{
									CurSoftKeyPos.bXPos++;
								}
								else 
								{
									CurSoftKeyPos.bXPos = 0;
									if(CurSoftKeyPos.bYPos < 4)
										CurSoftKeyPos.bYPos++;
									else
									{
										CurSoftKeyPos.bXPos = 0;
										CurSoftKeyPos.bYPos = 0;
									}
								}
								break;
							}

						case 3:
							{
								if (CurSoftKeyPos.bXPos < 15)// if (CurSoftKeyPos.bXPos < 12)   if (CurSoftKeyPos.bXPos < 11)
								{
									CurSoftKeyPos.bXPos++;
								}
								else
								{
									CurSoftKeyPos.bXPos = 0;
									CurSoftKeyPos.bYPos++;
								}
								break;
							}

						default:
							{
								break;
							}
						}

					}

					//	if (DATA_HEX == bDataType)
					{//	DTRACE(0, ("cqj---txtBuf is %d",txtBuf[txtPos] ));
						txtBuf[txtPos] = cSoftKeyTableHEX[CurSoftKeyPos.bYPos][CurSoftKeyPos.bXPos];
					}

					break;
				}

			default:
				{
					break;
				}

			}


			for (int m=0; m < bmMAX; m++)//m <５
			{
				BYTE bMaxN = 0;
				switch (m)
				{
				case 0:
					{
						bMaxN = 19;//bMaxN = 10;---cqj
						break;
					}

				case 3:
					{
						bMaxN = 12;//---cqj--12
						break;
					}

				case 1:
				case 2:
				case 4:
					{
						bMaxN = 20;
						break;
					}

				default:
					{
						bMaxN = 20;
						break;
					}

				}

				for (int n=0; n<bMaxN; n++)
				{

					//if (DATA_HEX == bDataType)
					{
						if ( txtBuf[txtPos] == cSoftKeyTableHEX[m][n])
						{
							CurSoftKeyPos.bYPos = m;
							CurSoftKeyPos.bXPos = n;
						}
					}

				}
			}

		}
		return -1;
	}
	//////////////////////////

