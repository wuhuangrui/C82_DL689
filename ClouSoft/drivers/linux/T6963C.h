/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�lcd.h
 * ժ    Ҫ�����ļ�ʵ���˶�T6963CҺ���������Ĳ���
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2007-01-12
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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
	bool  Init();						//��ʼ��Һ��
	void Reset(bool bReset = true);		//��λҺ��
	void Clear(bool fClrScreen = true);	//�����ʾ
	void BlightOn(bool fOn);			//�򿪱���
	void Blight(char cOn);			//�򿪱���
	void AdjustContrast(bool fAdd);		//���ӶԱȶ� true ���ӱ��� 
										//false ���ͱ���
	void SavePara();					//�洢Һ������
	void WriteDat(unsigned short ucData); //��Һ��8λ����
	void WriteCmd(unsigned short ucCmd);  //��Һ��8λ����
	void WriteChar(const unsigned char *ucCh, const int x, 
					const int y, bool fReverse=false, const bool fRefresh=true);		//��Һ��д��1��Char
	void WriteZhChar(unsigned char *usWord, const int x, 
						const int y, bool fReverse=false, const bool fRefresh=true);		//��Һ��д��1��Word
	void DrawPoint(const unsigned char x, const unsigned char y, const bool fReverse=false, const bool fRefresh=true);		//������(x, y)λ�û�һ����
	void DrawLine(const unsigned char x1, const unsigned char y1, 
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh=true);//������(x1, y1)-(x2, y2)֮�仮һ����
	void DrawBitmap(const unsigned char *ucBitmap, const bool fRefresh=true);//��ͼ
	void DrawBitmap(const unsigned char *ucBitmap, const unsigned char x,
					const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh=true);//��ͼ
	int Print(const char *str, const int x=-1, 
				const int y=-1, const bool fReverse = false, const bool fRefresh=true, const int iPage=1);//�˺���������Һ����ָ��λ�ô�ӡ�ַ�
	void SetPos(const unsigned char x, const unsigned char y);
	void DrawRange(const unsigned char x1, const unsigned char y1,
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true);	//�����κ���
	bool IsBusy();//�ж�Һ���Ƿ�æ
	void Delay(short sDelay);//
	void Refresh(bool fForce = false);//ˢ����ʾ
	void SaveHzk();
	void InitHzk();
};
#endif
