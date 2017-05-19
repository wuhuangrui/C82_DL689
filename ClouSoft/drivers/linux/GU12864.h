/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ks0108.cpp
 * 摘    要：VFD液晶驱动(GU128X64)
 * 当前版本：1.1
 
 * 修 改 者：杨进
 * 修改时间：2008-03-20
 * 修改内容：1、修正驱动Bug
 *           2、Print函数增加分页处理
 * 取代版本：1.0
 * 原 作 者：李立华
 * 完成日期：2007-03-23
************************************************************************************************************/
#ifndef   GU12864_H
#define   GU12864_H
#include "Lcd.h"
#include "apptypedef.h"

#define FONT_S88             0
#define FONT_S816            1

#define		LINEX		160
#define		LINEY		160

class CGU12864 : public CLcd
{
private:
	unsigned char m_ucShadowGDRam[16*64];
	unsigned char m_ucXpos;
	unsigned char m_ucYpos;
	unsigned char m_ucContrast;
	BYTE m_bFontSize;
	unsigned char m_ucGDRam[16*64];
	int m_iLcdFd;

public:
	CGU12864();
	~CGU12864();
	bool Init();						//初始化液晶
	void Reset(bool fReset=true);		//复位液晶
	void Clear(bool fRefresh=true);		//清除显示
	void BlightOn(bool fOn);			//打开背光
	void AdjustContrast(bool fAdd);		//增加对比度 true 增加背光 
										//false 降低背光	
	void SavePara();					//存储液晶参数	
	void WriteDat(unsigned short ucData); //向液晶8位数据	
	void WriteCmd(unsigned short ucCmd);  //向液晶8位命令
		
	void SetFontSize(BYTE bSize);
	void WriteChar(const unsigned char *ucCh, const int x, 
					const int y, bool fReverse = false, const bool fRefresh = true);		//向液晶写入1个Char
		
	void WriteZhChar(unsigned char *usWord, const int x, const int y, 
						bool fReverse = false, const bool fRefresh = true);		//向液晶写入1个Word
	
	void DrawPoint(const unsigned char x, const unsigned char y, const bool fReverse = false, const bool fRefresh = true);		//在坐标(x, y)位置划一个点
	
	void DrawLine(const unsigned char x1, const unsigned char y1, const unsigned char x2, const unsigned char y2, 
					const bool fReverse = false, const bool fRefresh = true);//在坐标(x1, y1)-(x2, y2)之间划一条线
	
	void DrawBitmap(const unsigned char *ucBitmap, const bool fRefresh = true);//绘图
	
	void DrawBitmap(const unsigned char *ucBitmap, const unsigned char x, const unsigned char y, 
					const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh = true);//绘图
	
	int Print(const char *str, const int x = -1, const int y = -1, 
				const bool fReverse = false, const bool fRefresh = true, const int iPage = 1);//此函数用于在液晶的指定位置打印字符
	
	void SetPos(const unsigned char x, const unsigned char y);
	
	void DrawRange(const unsigned char x1, const unsigned char y1, const unsigned char x2, 
					const unsigned char y2, const bool fReverse = false, const bool fRefresh = true);	//画矩形函数
	void ReverseArea(const unsigned char x, const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh=true);//反选一个范围
	void SetArea(BYTE bAddr, BYTE bBlock);
	void SetAddrMode(BYTE bMode);
	void SetBrightness(BYTE bBright);
	void SetPosition(BYTE bX, BYTE bY);
	bool IsBusy();//判断液晶是否忙
	void Delay(short sDelay);	
	void Refresh(bool fForce = false);//刷新显示
	void SaveHzk();
	void InitHzk();
};
#endif
