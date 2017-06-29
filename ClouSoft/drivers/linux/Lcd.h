/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Lcd.h
 * 摘    要：液晶驱动的基类
 *
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009-07
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
************************************************************************************************************/
#ifndef LCD_H
#define LCD_H
#include "apptypedef.h"

class CLcd
{
public:
	CLcd();
	virtual ~CLcd();
	virtual bool Init() = 0;						//初始化液晶
	virtual bool ReInit(bool fResetCtrl) = 0;
	virtual bool ReInitLcdHandle() = 0;
	virtual void InitHzk() = 0;
	virtual void Reset(bool fReset = true) = 0;		//复位液晶
	virtual void Clear(bool fRefresh=true) = 0;		//清除显示
	virtual void BlightOn(bool fOn) = 0;			//打开背光
	virtual void AdjustContrast(bool fAdd) = 0;		//增加对比度 true 增加背光 
	virtual void AdjContrast(unsigned char val) =0 ;		//改变液晶对比度 
										//false 降低背光
	virtual void SavePara() = 0;					//存储液晶参数
	virtual void WriteDat(unsigned short ucData) = 0; //向液晶8位数据
	virtual void WriteCmd(unsigned short ucCmd) = 0;  //向液晶8位命令
	virtual void WriteChar(const unsigned char *ucCh, const int x, 
					const int y, bool fReverse = false, const bool fRefresh = true) = 0;		//向液晶写入1个Char
	virtual void WriteZhChar(unsigned char *usWord, const int x, 
						const int y, bool fReverse = false, const bool fRefresh = true) = 0;		//向液晶写入1个Word
	virtual void DrawPoint(const unsigned char x, const unsigned char y, const bool fReverse = false, 
						   const bool fRefresh = true) = 0;		//在坐标(x, y)位置划一个点
	virtual void DrawLine(const unsigned char x1, const unsigned char y1, 
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true) = 0;//在坐标(x1, y1)-(x2, y2)之间划一条线
	virtual void DrawBitmap(const unsigned char *ucBitmap, const bool fRefresh = true) = 0;//绘图
	virtual void DrawBitmap(const unsigned char *ucBitmap, const unsigned char x,
					const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh = true) = 0;//绘图
	virtual int Print(const char *str, const int x = -1, 
				const int y = -1, const bool fReverse = false, const bool fRefresh = true, const int iPage = 1) = 0;//此函数用于在液晶的指定位置打印字符
	virtual void SetPos(const unsigned char x, const unsigned char y) = 0;
	virtual void DrawRange(const unsigned char x1, const unsigned char y1,
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true) = 0;	//画矩形函数
	virtual bool IsBusy() = 0;//判断液晶是否忙
	virtual void Delay(short sDelay) = 0;//
	virtual void Refresh(bool fForce = false) = 0;//刷新显示
	virtual void SaveHzk() = 0;
	virtual void ReverseArea(const unsigned char x, const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh=true);//反选一个范围
	virtual void SetFontSize(BYTE bSize);
};
#endif

