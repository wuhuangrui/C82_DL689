/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrMapCfg.c
 * ժ    Ҫ���������OAD��9707645���ӳ�����ñ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��11��
 *********************************************************************************************************/
#include "stdafx.h"
#include "MeterStruct.h"
#include "DbConst.h"

extern BYTE g_bComEngDataFmt[3];
extern BYTE g_bEngDataFmt[3];
extern BYTE g_bMaxDemFmt[6];

extern BYTE g_b645ExtDataFmt[1];
extern BYTE g_b645ExtTempFmt[4];
extern BYTE g_b645ExtStaFmt[4];
extern BYTE g_bVoltDataFmt[3];
extern BYTE g_bCurDataFmt[3];
extern BYTE g_bPowerDataFmt[3];
extern BYTE g_bCosDataFmt[3];
extern BYTE g_bMtrSubRunStateFmt[3];
extern BYTE g_bFrzRecTimeFmt[1];
extern BYTE g_bServAddrFmt[3];
extern BYTE g_bPurchaseRate[1];
extern BYTE g_bEvtTimeFmt[1];
extern BYTE g_bPwrPrice[1];

BYTE g_bEngTatolFmt[1] = {DT_DB_LONG_U};
BYTE g_bPowerTatolFmt[1] = {DT_DB_LONG};

Toad645Map g_OodTo645ExtMap[] = {
	//dwOAD,		wID,			pFmt,					wFmtLen,			wOoLen,		w645Len
	{0x25000200,	0x901f,		g_b645ExtDataFmt,	sizeof(g_b645ExtDataFmt),		5,			22},		//�ۼ�ˮ���ȣ�����
	{0x25010200,	0x901f,		g_b645ExtDataFmt,	sizeof(g_b645ExtDataFmt),		5,			22},		//�ۼ�������
	{0x25020200,	0x901f,		g_b645ExtDataFmt,	sizeof(g_b645ExtDataFmt),		5,			43},		//�ۼ�����
	{0x25030200,	0x901f,		g_b645ExtDataFmt,	sizeof(g_b645ExtDataFmt),		5,			43},		//�ȹ���
	{0x25040200,	0x901f,		g_b645ExtDataFmt,	sizeof(g_b645ExtDataFmt),		5,			43},		//�ۼƹ���ʱ��
	{0x25050200,	0x901f,		g_b645ExtTempFmt,	sizeof(g_b645ExtTempFmt),		12,			43},		//ˮ��
	{0x25060200,	0x901f,		g_b645ExtStaFmt,	sizeof(g_b645ExtStaFmt),		6,			43},		//���Ǳ�״̬ST
};

Toad645Map g_OodTo645Map[] = {	
	//dwOAD,		wID,			pFmt,					wFmtLen,			wOoLen,		w645Len
	{0x00010200,	0x900f,		g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),		27,			20},		//����ǰ������й�����	

	{0x00100200,	0x901f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ�������й�����
	{0x00100201,	0x9010,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			7,			 5},		//����ǰ�������й��ܵ���
	{0x00100202,	0x3701,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//�����й���������
	{0x00100204,	0x9a1f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն��������й�����					
	{0x00100205,	0x941f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶��������й�����
	{0x00110200,	0x9070,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��A�������й�����		
	{0x00120200,	0x9071,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��B�������й�����		
	{0x00130200,	0x9072,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��C�������й�����		

	{0x00200200,	0x902f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ�������й�����		
	{0x00200201,	0x9020,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			7,			 5},		//����ǰ�������й��ܵ���
	{0x00200202,	0x3702,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//�����й���������
	{0x00200204,	0x9a2f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն��ᷴ���й�����		
	{0x00200205,	0x942f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶��ᷴ���й�����		
	{0x00210200,	0x9080,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��A�෴���й�����		
	{0x00220200,	0x9081,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��B�෴���й�����		
	{0x00230200,	0x9082,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��C�෴���й�����	

	{0x00300200,	0x911f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ�����򣨸��ԣ��޹�����					
	{0x00300202,	0x3703,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//���򣨸��ԣ��޹���������
	{0x00300204,	0x9b1f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն������򣨸��ԣ��޹�����	
	{0x00300205,	0x951f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶������򣨸��ԣ��޹�����	
	{0x00310200,	0x9170,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��A�ࣨ���ԣ��޹�����		
	{0x00320200,	0x9171,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��B�ࣨ���ԣ��޹�����		
	{0x00330200,	0x9172,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��C�ࣨ���ԣ��޹�����			

	{0x00400200,	0x912f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ���������ԣ��޹�����
	{0x00400202,	0x3704,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//�������ԣ��޹���������
	{0x00400204,	0x9b2f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն��ᷴ�����ԣ��޹�����
	{0x00400205,	0x952f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶��ᷴ�����ԣ��޹�����
	{0x00410200,	0x9180,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��A�ࣨ���ԣ��޹�����		
	{0x00420200,	0x9181,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��B�ࣨ���ԣ��޹�����		
	{0x00430200,	0x9182,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��C�ࣨ���ԣ��޹�����		

	{0x00500200,	0x913f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ��I���ޣ����ԣ��޹�����					
	{0x00500202,	0x3745,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//I���ޣ����ԣ��޹���������
	{0x00500204,	0x9b3f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն���I���ޣ����ԣ��޹�����
	{0x00500205,	0x953f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶���I���ޣ����ԣ��޹�����
	{0x00600200,	0x915f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ��II�����޹�����
	{0x00600202,	0x3746,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//II�����޹���������
	{0x00600204,	0x9b5f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն���II�����޹�����
	{0x00600205,	0x955f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶���II�����޹�����
	{0x00700200,	0x916f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ��III�����޹�����
	{0x00700202,	0x3747,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//III�����޹���������
	{0x00700204,	0x9b6f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն���III�����޹�����
	{0x00700205,	0x956f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶���III�����޹�����
	{0x00800200,	0x914f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ��IV���ޣ����ԣ��޹�����	    
	{0x00800202,	0x3748,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//IV���ޣ����ԣ��޹���������
	{0x00800204,	0x9b4f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն���IV���ޣ����ԣ��޹�����
	{0x00800205,	0x954f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶���IV���ޣ����ԣ��޹�����

	{0x10100200,	0xa01f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ�������й�����
	{0x10100204,	0x9c0f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�Σ��ն��������й�����
	{0x10100205,	0xa41f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶��������й�����

	{0x10200200,	0xa02f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ�������й�����
	{0x10200204,	0x9c2f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�Σ��ն��ᷴ���й�����
	{0x10200205,	0xa42f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶��ᷴ���й�����

	{0x10300200,	0xa11f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ�������޹�����
	{0x10300204,	0x9c1f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�Σ��ն��������޹�����
	{0x10300205,	0xa41f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶��������޹�����

	{0x10400200,	0xa12f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ�������޹�����
	{0x10400204,	0x9c3f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�Σ��ն��ᷴ���޹�����
	{0x10400205,	0xa42f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶��ᷴ���޹�����

	{0x10500200,	0xa13f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ��I���ޣ����ԣ��޹�����				
	{0x10500205,	0xa53f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶���I���ޣ����ԣ��޹�����
	{0x10600200,	0xa15f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ��II�����޹�����
	{0x10600205,	0xa55f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶���II�����޹�����
	{0x10700200,	0xa16f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ��III�����޹�����
	{0x10700205,	0xa56f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶���III�����޹�����
	{0x10800200,	0xa14f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ��IV���ޣ����ԣ��޹�����
	{0x10800205,	0xa54f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶���IV���ޣ����ԣ��޹�����

	{0x20000200,	0xb61f,		g_bVoltDataFmt,		sizeof(g_bVoltDataFmt),			11,			6},			//����ǰ����ѹ					
	{0x20000202,	0x3689,		g_bVoltDataFmt,		sizeof(g_bVoltDataFmt),			11,			6},			//��ѹ����
	{0x20010200,	0xb62f,		g_bCurDataFmt,		sizeof(g_bCurDataFmt),			17,			9},			//����ǰ������ 	//��������������������ô�죿	    
	{0x20010202,	0x3692,		g_bCurDataFmt,		sizeof(g_bCurDataFmt),			17,			9},			//��������
	{0x20040200,	0xB63f,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},		//����ǰ���й����� 
	{0x20040202,	0x3681,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},		//�й���������
	{0x20050200,	0xB64f,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},		//����ǰ���޹�����
	{0x20050202,	0x3685,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},		//�޹���������
	{0x200A0200,	0xB65f,		g_bCosDataFmt,		sizeof(g_bCosDataFmt),			14,			8},			//��������
	{0x200A0202,	0x3705,		g_bCosDataFmt,		sizeof(g_bCosDataFmt),			14,			8},			//������������


	{0x20140201,	0xc860,		g_bMtrSubRunStateFmt,	sizeof(g_bMtrSubRunStateFmt),		4,			2},			//�������״̬��1
	{0x20140202,	0xc861,		g_bMtrSubRunStateFmt,	sizeof(g_bMtrSubRunStateFmt),		4,			2},			//�������״̬��2
	{0x20140203,	0xc862,		g_bMtrSubRunStateFmt,	sizeof(g_bMtrSubRunStateFmt),		4,			2},			//�������״̬��3
	{0x20140204,	0xc863,		g_bMtrSubRunStateFmt,	sizeof(g_bMtrSubRunStateFmt),		4,			2},			//�������״̬��4
	{0x20140205,	0xc864,		g_bMtrSubRunStateFmt,	sizeof(g_bMtrSubRunStateFmt),		4,			2},			//�������״̬��5
	{0x20140206,	0xc865,		g_bMtrSubRunStateFmt,	sizeof(g_bMtrSubRunStateFmt),		4,			2},			//�������״̬��6
	{0x20140207,	0xc866,		g_bMtrSubRunStateFmt,	sizeof(g_bMtrSubRunStateFmt),		4,			2},			//�������״̬��7


	{0x201A0200,	0xea61,		g_bPwrPrice,		sizeof(g_bPwrPrice),			5,			4},		//��ǰ���
	{0x201B0200,	0xea62,		g_bPwrPrice,		sizeof(g_bPwrPrice),			5,			4},		//��ǰ���ʵ��
	{0x201C0200,	0xea60,		g_bPwrPrice,		sizeof(g_bPwrPrice),			5,			4},		//��ǰ���ݵ��


	{0x20210200,	0x9a00,		g_bFrzRecTimeFmt,	sizeof(g_bFrzRecTimeFmt),		8,			5},			//����1�Σ��ն���ʱ��
	{0x202a0200,	0xc032,		g_bServAddrFmt,		sizeof(g_bServAddrFmt),			17,			6},			//����ַ

	{0x202e0200,	0xea65,		g_bPurchaseRate,	sizeof(g_bPurchaseRate),		5,			4},			//�ۼƹ�����(Ŀǰ��OADδ�ڡ��������Э����645Э������ݶ�Ӧ��ϵ�ο���20170421�����ҵ���Ӧ��ϵ)

	{0x40000200,	0xc010,		g_bEvtTimeFmt,		sizeof(g_bEvtTimeFmt),			8,			3},			//����
	{0x40000209,	0xC011,		g_bEvtTimeFmt,		sizeof(g_bEvtTimeFmt),			22,			12},		//Ϊ645���ʱ����һ������OAD���Ա����⴦��
	{0x50020200,	0x901f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//������������
	{0x50030200,	0x901f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//Сʱ��������
	{0x50040200,	0x901f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//������
	{0x50050200,	0x901f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����������
	{0x50060200,	0x901f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//������

	//07��û���������ɹ����洢��ID,���⴦��ֱ�����ն���ʱ����� Add CL 
	{0x60400200,	0x9a00,		g_bFrzRecTimeFmt,	sizeof(g_bFrzRecTimeFmt),		8,			5},			//�ɼ�����ʱ�� 
	{0x60410200,	0x9a00,		g_bFrzRecTimeFmt,	sizeof(g_bFrzRecTimeFmt),		8,			5},			//�ɼ��ɹ�ʱ��
	{0x60420200,	0x9a00,		g_bFrzRecTimeFmt,	sizeof(g_bFrzRecTimeFmt),		8,			5},			//�ɼ��洢ʱ��

};

//ȫ�¼��ɼ��س�����ƽṹ
static TErcRdCtrl g_ErcRdCtrl[] = {	
	//�¼�OAD, �¼�����ID, �س���ID����,  �س���ID�б�
	//=====================================================================================================
	{0x30000700,        0x10010001,   2,	       {0x10010101, 0x10012501}},	//A��ʧѹ
	{0x30000800,        0x10020001,   2,	       {0x10020101, 0x10022501}},	//B��ʧѹ
	{0x30000900,        0x10030001,   2,	       {0x10030101, 0x10032501}},	//C��ʧѹ

	{0x30010700,        0x11010001,   2,	       {0x11010101, 0x11012501}},	//A��Ƿѹ
	{0x30010800,        0x11020001,   2,	       {0x11020101, 0x11022501}},	//B��Ƿѹ
	{0x30010900,        0x11030001,   2,	       {0x11030101, 0x11032501}},	//C��Ƿѹ

	{0x30020700,        0x12010001,   2,	       {0x12010101, 0x12012501}},	//A���ѹ
	{0x30020800,        0x12020001,   2,	       {0x12020101, 0x12022501}},	//B���ѹ
	{0x30020900,        0x12030001,   2,	       {0x12030101, 0x12032501}},	//C���ѹ

	{0x30030700,        0x13010001,   2,	       {0x13010101, 0x13012501}},	//A�����
	{0x30030800,        0x13020001,   2,	       {0x13020101, 0x13022501}},	//B�����
	{0x30030900,        0x13030001,   2,	       {0x13030101, 0x13032501}},	//C�����

	{0x30040700,        0x18010001,   2,	       {0x18010101, 0x18012101}},	//A��ʧ��
	{0x30040800,        0x18020001,   2,	       {0x18020101, 0x18022101}},	//B��ʧ��
	{0x30040900,        0x18030001,   2,	       {0x18030101, 0x18032101}},	//C��ʧ��

	{0x30050700,        0x19010001,   2,	       {0x19010101, 0x19012101}},	//A�����
	{0x30050800,        0x19020001,   2,	       {0x19020101, 0x19022101}},	//B�����
	{0x30050900,        0x19030001,   2,	       {0x19030101, 0x19032101}},	//C�����

	{0x30060700,        0x1a010001,   2,	       {0x1a010101, 0x1a012101}},	//A�����
	{0x30060800,        0x1a020001,   2,	       {0x1a020101, 0x1a022101}},	//B�����
	{0x30060900,        0x1a030001,   2,	       {0x1a030101, 0x1a032101}},	//C�����

	{0x30070700,        0x1b010001,   2,	       {0x1b010101, 0x1b011201}},	//A�๦�ʷ���
	{0x30070800,        0x1b020001,   2,	       {0x1b020101, 0x1b021201}},	//B�๦�ʷ���
	{0x30070900,        0x1b030001,   2,	       {0x1b030101, 0x1b031201}},	//C�๦�ʷ���

	{0x30080700,        0x1c010001,   2,	       {0x1c010101, 0x1c011201}},	//A�����
	{0x30080800,        0x1c020001,   2,	       {0x1c020101, 0x1c021201}},	//B�����
	{0x30080900,        0x1c030001,   2,	       {0x1c030101, 0x1c031201}},	//C�����

	{0x30090200,        0x03120000,   1,	       {0x03120101}},//�����й��������ޣ��޷���ǰ�ͽ������������
	{0x300A0200,        0x03120000,   1,	       {0x03120201}},//�����й��������ޣ��޷���ǰ�ͽ������������
	{0x300B0200,        0x03120000,   1,	       {0x03120301/*, 0x03120401, 0x03120501, 0x03120601*/}},//�޹��������ޣ��޷���ǰ�ͽ������������
	{0x300C0200,        0x1f000001,   2,	       {0x1f000101, 0x1f000601}},	//��������������
	{0x300D0200,        0x03050000,   1,	       {0x03050001}},				//ȫʧѹ���޷���ǰ�ͽ������������
	{0x300E0200,        0x03060000,   1,	       {0x03060001}},				//������Դʧ�磬�޷���ǰ�ͽ������������
	{0x300F0200,        0x14000001,   2,	       {0x14000101, 0x14001201}},	//��ѹ������
	{0x30100200,        0x15000001,   2,	       {0x15000101, 0x15001201}},	//����������
	{0x30110200,        0x03110000,   1,	       {0x03110001}},				//���ܱ�����¼����޷���ǰ�ͽ������������
	{0x30120200,        0x03300000,   1,	       {0x03300001}},				//����¼����޷���ǰ�ͽ������������
	{0x30130200,        0x03300100,   1,	       {0x03300101}},				//���ܱ������¼����з���ʱ�̵�������(�������й���1-4�����޹�)�����������ͬһID��
	{0x30140200,        0x03300200,   1,	       {0x03300201}},				//���ܱ��������㣬�޷���ǰ�ͽ������������
	{0x30150200,        0x03300300,   1,	       {0x03300301}},				//���ܱ��¼����㣬�޷���ǰ�ͽ������������
	{0x30160200,        0x03300400,   1,	       {0x03300401}},				//Уʱ�¼����޷���ǰ�ͽ������������
	{0x30170200,        0x03300500,   1,	       {0x03300501}},				//ʱ�α��̣��޷���ǰ�ͽ������������
	{0x30180200,        0x03300600,   1,	       {0x03300601}},				//ʱ�����̣��޷���ǰ�ͽ������������
	{0x30190200,        0x03300700,   1,	       {0x03300701}},				//�����ձ�̣��޷���ǰ�ͽ������������
	{0x301A0200,        0x03300c00,   1,	       {0x03300c01}},				//�����ձ�̣��޷���ǰ�ͽ������������
	{0x301B0200,        0x03300d00,   1,	       {0x03300d01}},				//������¼����з���ǰ�ͽ������������(�������й���1-4�����޹�)�����������ͬһID��
	{0x301C0200,        0x03300e00,   1,	       {0x03300e01}},				//����Ŧ���¼����з���ǰ�ͽ������������(�������й���1-4�����޹�)�����������ͬһID��
	{0x301D0200,        0x16000001,   2,	       {0x16000101, 0x16001301}},	//��ѹ��ƽ��
	{0x301E0200,        0x17000001,   2,	       {0x17000101, 0x17001301}},	//������ƽ��
	{0x301F0200,        0x1d000001,   1,	       {0x1d000101}},				//��բ�������з���ʱ�̵�������(�������й���1-4�����޹�)
	{0x30200200,        0x1e000001,   1,	       {0x1e000101}},				//��բ�������з���ʱ�̵�������(�������й���1-4�����޹�)
	{0x30210200,        0x03300800,   1,	       {0x03300801}},				//�ڼ��ձ�̣��޷���ǰ�ͽ������������
	{0x30220200,        0x03300900,   1,	       {0x03300901}},				//�й���Ϸ�ʽ��̣��޷���ǰ�ͽ������������
	{0x30230200,        0x03300a00,   1,	       {0x03300a01}},				//�޹���Ϸ�ʽ1��̣��޷���ǰ�ͽ������������
	{0x30240200,        0x03300f00,   1,	       {0x03300f01}},				//���ʲ������̣��޷���ǰ�ͽ������������
	{0x30250200,        0x03301000,   1,	       {0x03301001}},				//���ݱ��̣��޷���ǰ�ͽ������������
	{0x30260200,        0x03301200,   1,	       {0x03301201}},				//��Կ���£��޷���ǰ�ͽ������������
	{0x30270200,        0x03301300,   1,	       {0x03301301}},				//���ܱ��쳣�忨�¼����з���ʱ�̵�������(�������й�)�����������ͬһID��
	{0x302A0200,        0x03350000,   1,	       {0x03350001}},				//�㶨�ų����ţ��з���ǰ�ͽ������������(�������й�)�����������ͬһID��
	{0x302B0200,        0x03360000,   1,	       {0x03360001}},				//���ɿ����������з���ǰ�ͽ������������(�������й�)�����������ͬһID��
	{0x302C0200,        0x03370000,   1,	       {0x03370001}},				//��Դ�쳣���з���ʱ�̵�������(�������й�)�����������ͬһID��
	{0x302D0200,        0x20000001,   2,	       {0x20000101, 0x20001301}},	//�������ز�ƽ��
};

//������ʹ�ö��ַ�����ӳ��ID
Toad645Map* BinarySearchProId(Toad645Map* pOad645Map, WORD num, DWORD dwOAD)
{
	int little, big, mid;

	if (dwOAD<pOad645Map[0].dwOAD  || dwOAD>pOad645Map[num-1].dwOAD)
		return NULL;

	little = 0;
	big = num - 1;
	while (little <= big)
	{                               
		mid = (little + big) / 2;       //����

		if (pOad645Map[mid].dwOAD == dwOAD) 
		{
			return pOad645Map + mid;
		}
		else if (dwOAD > pOad645Map[mid].dwOAD)
		{
			little = mid + 1;
		} 
		else  
		{
			big = mid - 1;
		}
	}

	return NULL;
}

//����:���OAD�б��ж�Ӧ��645Э��ĸ�����Ϣ������������,�ڲ�ID��
//		@dwOAD:��������������
//���أ��ҵ���Ӧ������Ϣ�򷵻�����ָ��
Toad645Map* GetOad645Map(DWORD dwOAD)
{
	Toad645Map* pOad645Map = NULL;
	pOad645Map = BinarySearchProId(g_OodTo645Map, sizeof(g_OodTo645Map)/sizeof(Toad645Map), dwOAD);
	return pOad645Map;
}

//����:���OAD�б��ж�Ӧ��T188Э��ĸ�����Ϣ������������,�ڲ�ID��
//		@dwOAD:��������������
//���أ��ҵ���Ӧ������Ϣ�򷵻�����ָ��
Toad645Map* GetOad645ExtMap(DWORD dwOAD)
{
	Toad645Map* pOad645Map = NULL;
	pOad645Map = BinarySearchProId(g_OodTo645ExtMap, sizeof(g_OodTo645ExtMap)/sizeof(Toad645Map), dwOAD);
	return pOad645Map;
}

//������ʹ�ö��ַ�����ӳ��ID
TErcRdCtrl* BinarySearchProEvtId(TErcRdCtrl* pErcRdCtrl, WORD num, DWORD dwOAD)
{
	int little, big, mid;

	if (dwOAD<pErcRdCtrl[0].dwEvtOAD  || dwOAD>pErcRdCtrl[num-1].dwEvtOAD)
		return NULL;

	little = 0;
	big = num - 1;
	while (little <= big)
	{                               
		mid = (little + big) / 2;       //����

		if (pErcRdCtrl[mid].dwEvtOAD == dwOAD) 
		{
			return pErcRdCtrl + mid;
		}
		else if (dwOAD > pErcRdCtrl[mid].dwEvtOAD)
		{
			little = mid + 1;
		} 
		else  
		{
			big = mid - 1;
		}
	}

	return NULL;
}

//����:���OAD�б��ж�Ӧ��07645Э����¼������������
//		@dwOAD:��������������
//���أ��ҵ���Ӧ������Ϣ�򷵻�����ָ��
TErcRdCtrl* GetOad07645ErcMap(DWORD dwOAD)
{
	TErcRdCtrl* pErcRdCtrl = NULL;
	pErcRdCtrl = BinarySearchProEvtId(g_ErcRdCtrl, sizeof(g_ErcRdCtrl)/sizeof(TErcRdCtrl), dwOAD);
	return pErcRdCtrl;
}
