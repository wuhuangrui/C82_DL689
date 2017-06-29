/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Lcd.h
 * ժ    Ҫ��Һ�������Ļ���
 *
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009-07
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
************************************************************************************************************/
#ifndef LCD_H
#define LCD_H
#include "apptypedef.h"

class CLcd
{
public:
	CLcd();
	virtual ~CLcd();
	virtual bool Init() = 0;						//��ʼ��Һ��
	virtual bool ReInit(bool fResetCtrl) = 0;
	virtual bool ReInitLcdHandle() = 0;
	virtual void InitHzk() = 0;
	virtual void Reset(bool fReset = true) = 0;		//��λҺ��
	virtual void Clear(bool fRefresh=true) = 0;		//�����ʾ
	virtual void BlightOn(bool fOn) = 0;			//�򿪱���
	virtual void AdjustContrast(bool fAdd) = 0;		//���ӶԱȶ� true ���ӱ��� 
	virtual void AdjContrast(unsigned char val) =0 ;		//�ı�Һ���Աȶ� 
										//false ���ͱ���
	virtual void SavePara() = 0;					//�洢Һ������
	virtual void WriteDat(unsigned short ucData) = 0; //��Һ��8λ����
	virtual void WriteCmd(unsigned short ucCmd) = 0;  //��Һ��8λ����
	virtual void WriteChar(const unsigned char *ucCh, const int x, 
					const int y, bool fReverse = false, const bool fRefresh = true) = 0;		//��Һ��д��1��Char
	virtual void WriteZhChar(unsigned char *usWord, const int x, 
						const int y, bool fReverse = false, const bool fRefresh = true) = 0;		//��Һ��д��1��Word
	virtual void DrawPoint(const unsigned char x, const unsigned char y, const bool fReverse = false, 
						   const bool fRefresh = true) = 0;		//������(x, y)λ�û�һ����
	virtual void DrawLine(const unsigned char x1, const unsigned char y1, 
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true) = 0;//������(x1, y1)-(x2, y2)֮�仮һ����
	virtual void DrawBitmap(const unsigned char *ucBitmap, const bool fRefresh = true) = 0;//��ͼ
	virtual void DrawBitmap(const unsigned char *ucBitmap, const unsigned char x,
					const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh = true) = 0;//��ͼ
	virtual int Print(const char *str, const int x = -1, 
				const int y = -1, const bool fReverse = false, const bool fRefresh = true, const int iPage = 1) = 0;//�˺���������Һ����ָ��λ�ô�ӡ�ַ�
	virtual void SetPos(const unsigned char x, const unsigned char y) = 0;
	virtual void DrawRange(const unsigned char x1, const unsigned char y1,
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true) = 0;	//�����κ���
	virtual bool IsBusy() = 0;//�ж�Һ���Ƿ�æ
	virtual void Delay(short sDelay) = 0;//
	virtual void Refresh(bool fForce = false) = 0;//ˢ����ʾ
	virtual void SaveHzk() = 0;
	virtual void ReverseArea(const unsigned char x, const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh=true);//��ѡһ����Χ
	virtual void SetFontSize(BYTE bSize);
};
#endif

