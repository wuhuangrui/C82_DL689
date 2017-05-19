/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ks0108.cpp
 * ժ    Ҫ��VFDҺ������(GU128X64)
 * ��ǰ�汾��1.1
 
 * �� �� �ߣ����
 * �޸�ʱ�䣺2008-03-20
 * �޸����ݣ�1����������Bug
 *           2��Print�������ӷ�ҳ����
 * ȡ���汾��1.0
 * ԭ �� �ߣ�������
 * ������ڣ�2007-03-23
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
	bool Init();						//��ʼ��Һ��
	void Reset(bool fReset=true);		//��λҺ��
	void Clear(bool fRefresh=true);		//�����ʾ
	void BlightOn(bool fOn);			//�򿪱���
	void AdjustContrast(bool fAdd);		//���ӶԱȶ� true ���ӱ��� 
										//false ���ͱ���	
	void SavePara();					//�洢Һ������	
	void WriteDat(unsigned short ucData); //��Һ��8λ����	
	void WriteCmd(unsigned short ucCmd);  //��Һ��8λ����
		
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
	
	void DrawRange(const unsigned char x1, const unsigned char y1, const unsigned char x2, 
					const unsigned char y2, const bool fReverse = false, const bool fRefresh = true);	//�����κ���
	void ReverseArea(const unsigned char x, const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh=true);//��ѡһ����Χ
	void SetArea(BYTE bAddr, BYTE bBlock);
	void SetAddrMode(BYTE bMode);
	void SetBrightness(BYTE bBright);
	void SetPosition(BYTE bX, BYTE bY);
	bool IsBusy();//�ж�Һ���Ƿ�æ
	void Delay(short sDelay);	
	void Refresh(bool fForce = false);//ˢ����ʾ
	void SaveHzk();
	void InitHzk();
};
#endif
