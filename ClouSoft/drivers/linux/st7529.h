/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：st7529.cpp
 * 摘    要：本文件实现了对st7529液晶的操作
 *
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2007-07-23
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
*********************************************************************************************************/
#ifndef   ST7529_H
#define   ST7529_H

#include "Lcd.h"


#define		LINEX		160
#define		LINEY		160

class CST7529 : public CLcd
{
private:
	unsigned char m_ucGDRam[160*160/8];
	unsigned char m_ucShadowGDRam[160*160/8];
	unsigned char m_ucXpos;
	unsigned char m_ucYpos;
	unsigned char m_bContrast;
	int m_iLcdFd;
    DWORD m_dwCheckClick; //用于检测驱动是否死机
    unsigned char m_bCheckErrorCnt; //用于检测驱动是否死机
    bool m_fNeedInitLcdDrv;//驱动初始化标志
public:
	CST7529();
	~CST7529();
	bool ReInit(bool fResetCtrl);		//初始化液晶
    bool Init();						//初始化液晶
	void InitHzk();
	void Reset(bool fReset = true);		//复位液晶
	void Clear(bool fRefresh=true);		//清除显示
	void BlightOn(bool fOn);			//打开背光
	void AdjustContrast(bool fAdd);		//增加对比度 true 增加背光 
  void AdjContrast(unsigned char val);		//改变液晶对比度
										//false 降低背光
	void SavePara();					//存储液晶参数
	void WriteDat(unsigned short ucData); //向液晶8位数据
	void WriteCmd(unsigned short ucCmd);  //向液晶8位命令
    unsigned long ReadCmd(void);  //读液晶状态
    unsigned long ReadData(void);  //读液晶状态
	void WriteChar(const unsigned char *ucCh, const int x, 
					const int y, bool fReverse = false, const bool fRefresh = true);		//向液晶写入1个Char
	void WriteZhChar(unsigned char *usWord, const int x, 
						const int y, bool fReverse = false, const bool fRefresh = true);		//向液晶写入1个Word
	void DrawPoint(const unsigned char x, const unsigned char y, const bool fReverse = false, const bool fRefresh = true);		//在坐标(x, y)位置划一个点
	void DrawLine(const unsigned char x1, const unsigned char y1, 
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true);//在坐标(x1, y1)-(x2, y2)之间划一条线
	void DrawBitmap(const unsigned char *ucBitmap, const bool fRefresh = true);//绘图
	void DrawBitmap(const unsigned char *ucBitmap, const unsigned char x,
					const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh = true);//绘图
	int Print(const char *str, const int x = -1, 
				const int y = -1, const bool fReverse = false, const bool fRefresh = true, const int iPage = 1);//此函数用于在液晶的指定位置打印字符
	void SetPos(const unsigned char x, const unsigned char y);
	void DrawRange(const unsigned char x1, const unsigned char y1,
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true);	//画矩形函数
	bool IsBusy();//判断液晶是否忙
	void Delay(short sDelay);//
	void Refresh(bool fForce = false);//刷新显示
	void SaveHzk();
};

#endif
