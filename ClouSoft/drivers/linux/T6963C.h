/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：lcd.h
 * 摘    要：本文件实现了对T6963C液晶控制器的操作
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2007-01-12
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/

#ifndef   T6963C_H
#define   T6963C_H
#include "Lcd.h"

class CT6963C: public CLcd
{
private:
	unsigned short m_ucLcdCtrl;
	unsigned char m_ucGDRam[30*128];
	unsigned char m_ucShadowGDRam[30*128];
	unsigned char m_ucXpos;
	unsigned char m_ucYpos;
	int m_iLcdFd;
public:
	CT6963C();
	~CT6963C();
	bool  Init();						//初始化液晶
	void Reset(bool bReset = true);		//复位液晶
	void Clear(bool fClrScreen = true);	//清除显示
	void BlightOn(bool fOn);			//打开背光
	void Blight(char cOn);			//打开背光
	void AdjustContrast(bool fAdd);		//增加对比度 true 增加背光 
										//false 降低背光
	void SavePara();					//存储液晶参数
	void WriteDat(unsigned short ucData); //向液晶8位数据
	void WriteCmd(unsigned short ucCmd);  //向液晶8位命令
	void WriteChar(const unsigned char *ucCh, const int x, 
					const int y, bool fReverse=false, const bool fRefresh=true);		//向液晶写入1个Char
	void WriteZhChar(unsigned char *usWord, const int x, 
						const int y, bool fReverse=false, const bool fRefresh=true);		//向液晶写入1个Word
	void DrawPoint(const unsigned char x, const unsigned char y, const bool fReverse=false, const bool fRefresh=true);		//在坐标(x, y)位置划一个点
	void DrawLine(const unsigned char x1, const unsigned char y1, 
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh=true);//在坐标(x1, y1)-(x2, y2)之间划一条线
	void DrawBitmap(const unsigned char *ucBitmap, const bool fRefresh=true);//绘图
	void DrawBitmap(const unsigned char *ucBitmap, const unsigned char x,
					const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh=true);//绘图
	int Print(const char *str, const int x=-1, 
				const int y=-1, const bool fReverse = false, const bool fRefresh=true, const int iPage=1);//此函数用于在液晶的指定位置打印字符
	void SetPos(const unsigned char x, const unsigned char y);
	void DrawRange(const unsigned char x1, const unsigned char y1,
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true);	//画矩形函数
	bool IsBusy();//判断液晶是否忙
	void Delay(short sDelay);//
	void Refresh(bool fForce = false);//刷新显示
	void SaveHzk();
	void InitHzk();
};
#endif
