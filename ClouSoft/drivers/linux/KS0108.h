/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TG12864.h
 * 摘    要：本文件实现了对ST7920液晶的操作
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2007-03-23
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#ifndef KS0108_H
#define KS0108_H
#include "Lcd.h"

#define FONT_S88             0
#define FONT_S816            1

class CKS0108 : public CLcd
{
private:
	unsigned char m_ucGDRam[16*64];
	unsigned char m_ucShadowGDRam[16*64];
	unsigned char m_ucXpos;
	unsigned char m_ucYpos;
	unsigned char m_ucContrast;
	BYTE m_bFontSize;	
	int m_iLcdFd;

public:
	CKS0108();
	~CKS0108();
	bool Init();						//初始化液晶
	void Reset(bool fReset=false);		//复位液晶
	void Close();                       //关闭液晶
	void Clear(bool fRefresh = true);	//清除显示
	void BlightOn(bool fOn);			//打开背光
	void AdjustContrast(bool fAdd);		//增加对比度 true 增加背光 
										//false 降低背光
	void SavePara();					//存储液晶参数
	void WriteDat(unsigned short ucData); //向液晶8位数据
	void WriteDat(unsigned short ucData, unsigned char ucCS); //向液晶8位数据
	void WriteCmd(unsigned short ucCmd);  //向液晶8位命令
	void WriteCmd(unsigned short ucCmd, unsigned char ucCS);  //向液晶8位命令
		
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
	void DrawRange(const unsigned char x1, const unsigned char y1,
					const unsigned char x2, const unsigned char y2, const bool fReverse=false, const bool fRefresh=true);	//画矩形函数
	void ReverseArea(const unsigned char x, const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh=true);//反选一个范围
	void Delay(short sDelay);
	bool IsBusy();//判断液晶是否忙
	void Refresh(bool fForce = false);//刷新显示
	void SaveHzk();
	void InitHzk();
	unsigned char GetContrast();
};
#endif

