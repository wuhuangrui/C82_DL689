/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TG12864.h
 * ժ    Ҫ�����ļ�ʵ���˶�ST7920Һ���Ĳ���
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2007-03-23
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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
	bool Init();						//��ʼ��Һ��
	void Reset(bool fReset=false);		//��λҺ��
	void Close();                       //�ر�Һ��
	void Clear(bool fRefresh = true);	//�����ʾ
	void BlightOn(bool fOn);			//�򿪱���
	void AdjustContrast(bool fAdd);		//���ӶԱȶ� true ���ӱ��� 
										//false ���ͱ���
	void SavePara();					//�洢Һ������
	void WriteDat(unsigned short ucData); //��Һ��8λ����
	void WriteDat(unsigned short ucData, unsigned char ucCS); //��Һ��8λ����
	void WriteCmd(unsigned short ucCmd);  //��Һ��8λ����
	void WriteCmd(unsigned short ucCmd, unsigned char ucCS);  //��Һ��8λ����
		
	void SetFontSize(BYTE bSize);
	void WriteChar(const unsigned char *ucCh, const int x, 
					const int y, bool fReverse = false, const bool fRefresh = true);		//��Һ��д��1��Char
		
	void WriteZhChar(unsigned char *usWord, const int x, const int y, 
						bool fReverse = false, const bool fRefresh = true);		//��Һ��д��1��Word
	
	void DrawPoint(const unsigned char x, const unsigned char y, const bool fReverse = false, const bool fRefresh = true);		//������(x, y)λ�û�һ����
	
	void DrawLine(const unsigned char x1, const unsigned char y1, const unsigned char x2, const unsigned char y2, 
					const bool fReverse = false, const bool fRefresh = true);//������(x1, y1)-(x2, y2)֮�仮һ����
	
	void DrawBitmap(const unsigned char *ucBitmap, const bool fRefresh = true);//��ͼ
	
	void DrawBitmap(const unsigned char *ucBitmap, const unsigned char x, const unsigned char y, 
					const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh = true);//��ͼ
	
	int Print(const char *str, const int x = -1, const int y = -1, 
				const bool fReverse = false, const bool fRefresh = true, const int iPage = 1);//�˺���������Һ����ָ��λ�ô�ӡ�ַ�
				
	void SetPos(const unsigned char x, const unsigned char y);
	void DrawRange(const unsigned char x1, const unsigned char y1,
					const unsigned char x2, const unsigned char y2, const bool fReverse=false, const bool fRefresh=true);	//�����κ���
	void ReverseArea(const unsigned char x, const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh=true);//��ѡһ����Χ
	void Delay(short sDelay);
	bool IsBusy();//�ж�Һ���Ƿ�æ
	void Refresh(bool fForce = false);//ˢ����ʾ
	void SaveHzk();
	void InitHzk();
	unsigned char GetContrast();
};
#endif

