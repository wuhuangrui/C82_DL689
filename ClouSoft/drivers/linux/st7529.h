/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�st7529.cpp
 * ժ    Ҫ�����ļ�ʵ���˶�st7529Һ���Ĳ���
 *
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2007-07-23
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
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
    DWORD m_dwCheckClick; //���ڼ�������Ƿ�����
    unsigned char m_bCheckErrorCnt; //���ڼ�������Ƿ�����
    bool m_fNeedInitLcdDrv;//������ʼ����־
public:
	CST7529();
	~CST7529();
	bool ReInit(bool fResetCtrl);		//��ʼ��Һ��
    bool Init();						//��ʼ��Һ��
	void InitHzk();
	void Reset(bool fReset = true);		//��λҺ��
	void Clear(bool fRefresh=true);		//�����ʾ
	void BlightOn(bool fOn);			//�򿪱���
	void AdjustContrast(bool fAdd);		//���ӶԱȶ� true ���ӱ��� 
  void AdjContrast(unsigned char val);		//�ı�Һ���Աȶ�
										//false ���ͱ���
	void SavePara();					//�洢Һ������
	void WriteDat(unsigned short ucData); //��Һ��8λ����
	void WriteCmd(unsigned short ucCmd);  //��Һ��8λ����
    unsigned long ReadCmd(void);  //��Һ��״̬
    unsigned long ReadData(void);  //��Һ��״̬
	void WriteChar(const unsigned char *ucCh, const int x, 
					const int y, bool fReverse = false, const bool fRefresh = true);		//��Һ��д��1��Char
	void WriteZhChar(unsigned char *usWord, const int x, 
						const int y, bool fReverse = false, const bool fRefresh = true);		//��Һ��д��1��Word
	void DrawPoint(const unsigned char x, const unsigned char y, const bool fReverse = false, const bool fRefresh = true);		//������(x, y)λ�û�һ����
	void DrawLine(const unsigned char x1, const unsigned char y1, 
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true);//������(x1, y1)-(x2, y2)֮�仮һ����
	void DrawBitmap(const unsigned char *ucBitmap, const bool fRefresh = true);//��ͼ
	void DrawBitmap(const unsigned char *ucBitmap, const unsigned char x,
					const unsigned char y, const unsigned short usXlen, const unsigned short usYlen, const bool fRefresh = true);//��ͼ
	int Print(const char *str, const int x = -1, 
				const int y = -1, const bool fReverse = false, const bool fRefresh = true, const int iPage = 1);//�˺���������Һ����ָ��λ�ô�ӡ�ַ�
	void SetPos(const unsigned char x, const unsigned char y);
	void DrawRange(const unsigned char x1, const unsigned char y1,
					const unsigned char x2, const unsigned char y2, const bool fReverse = false, const bool fRefresh = true);	//�����κ���
	bool IsBusy();//�ж�Һ���Ƿ�æ
	void Delay(short sDelay);//
	void Refresh(bool fForce = false);//ˢ����ʾ
	void SaveHzk();
};

#endif
