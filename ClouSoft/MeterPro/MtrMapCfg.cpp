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
extern BYTE g_bVarDmdFmt[1];
extern BYTE g_bCosDataFmt[3];
extern BYTE g_bMtrSubRunStateFmt[3];
extern BYTE g_bMtrBlkRunStateFmt[5];
extern BYTE g_bFrzRecTimeFmt[1];
extern BYTE g_bServAddrFmt[3];
extern BYTE g_bPurchaseRate[1];
extern BYTE g_bEvtTimeFmt[1];
extern BYTE g_bPwrPrice[1];
extern BYTE g_bBitStringTypeFmt[2];

extern BYTE g_bPhaseVoltDataFmt[1];
extern BYTE g_bPhaseCurDataFmt[1];
extern BYTE g_bPhasePowerDataFmt[1];
extern BYTE g_bPhaseCosDataFmt[1];

extern BYTE g_bFeeMaxDemFmt[4];
extern BYTE g_bAngleFmt[3];
extern BYTE g_bPhaseAngleFmt[1];




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
	{0x00000200,	0x900f,		g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),		27,			20},		//����ǰ������й�����	
	{0x00000201,	0x9000,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ������й��ܵ���	
	{0x00000202,	0x9001,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ������й������
	{0x00000203,	0x9002,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ������й������
	{0x00000204,	0x9003,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ������й�ƽ����
	{0x00000205,	0x9004,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ������й��ȵ���

	{0x00100200,	0x901f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ�������й�����
	{0x00100201,	0x9010,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й��ܵ���
	{0x00100202,	0x9011,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й������
	{0x00100203,	0x9012,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й������
	{0x00100204,	0x9013,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й�ƽ����
	{0x00100205,	0x9014,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й��ȵ���

	{0x00100212,	0x3701,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//�����й���������
	{0x00100213,	0x3701,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//�����й��ܵ�������
	{0x00100214,	0x9a1f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն��������й�����					
	{0x00100215,	0x941f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶��������й�����

	{0x00110200,	0x9070,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��A�������й�����		
	{0x00120200,	0x9071,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��B�������й�����		
	{0x00130200,	0x9072,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��C�������й�����		

	{0x00200200,	0x902f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ�������й�����
	{0x00200201,	0x9020,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й��ܵ���
	{0x00200202,	0x9021,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й������
	{0x00200203,	0x9022,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й������
	{0x00200204,	0x9023,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й�ƽ����
	{0x00200205,	0x9024,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//����ǰ�������й��ȵ���
	
	{0x00200212,	0x3702,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//�����й���������
	{0x00200213,	0x3702,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//�����й��ܵ�������
	{0x00200214,	0x9a2f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն��ᷴ���й�����		
	{0x00200215,	0x942f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶��ᷴ���й�����		

	{0x00210200,	0x9080,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	    //����ǰ��A�෴���й�����		
	{0x00220200,	0x9081,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	    //����ǰ��B�෴���й�����		
	{0x00230200,	0x9082,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	    //����ǰ��C�෴���й�����	

	{0x00300200,	0x911f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ�����򣨸��ԣ��޹�����
	{0x00300201,	0x9110,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ�����򣨸��ԣ��޹��ܵ���
	{0x00300202,	0x9111,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ�����򣨸��ԣ��޹������
	{0x00300203,	0x9112,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ�����򣨸��ԣ��޹������
	{0x00300204,	0x9113,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ�����򣨸��ԣ��޹�ƽ����
	{0x00300205,	0x9114,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ�����򣨸��ԣ��޹��ȵ���

	{0x00300212,	0x3703,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//���򣨸��ԣ��޹���������
	{0x00300213,	0x3703,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//���򣨸��ԣ��޹��ܵ�������
	{0x00300214,	0x9b1f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն������򣨸��ԣ��޹�����	
	{0x00300215,	0x951f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶������򣨸��ԣ��޹�����	

	{0x00310200,	0x9170,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��A�ࣨ���ԣ��޹�����		
	{0x00320200,	0x9171,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��B�ࣨ���ԣ��޹�����		
	{0x00330200,	0x9172,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��C�ࣨ���ԣ��޹�����			

	{0x00400200,	0x912f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ���������ԣ��޹�����
	{0x00400201,	0x9120,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ���������ԣ��޹��ܵ���
	{0x00400202,	0x9121,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ���������ԣ��޹������
	{0x00400203,	0x9122,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ���������ԣ��޹������
	{0x00400204,	0x9123,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ���������ԣ��޹�ƽ����
	{0x00400205,	0x9124,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ���������ԣ��޹��ȵ���

	{0x00400212,	0x3704,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//�������ԣ��޹���������
	{0x00400213,	0x3704,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//�������ԣ��޹��ܵ�������
	{0x00400214,	0x9b2f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն��ᷴ�����ԣ��޹�����
	{0x00400215,	0x952f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶��ᷴ�����ԣ��޹�����

	{0x00410200,	0x9180,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��A�ࣨ���ԣ��޹�����		
	{0x00420200,	0x9181,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��B�ࣨ���ԣ��޹�����		
	{0x00430200,	0x9182,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},	   //����ǰ��C�ࣨ���ԣ��޹�����		

	{0x00500200,	0x913f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ��I���ޣ����ԣ��޹�����
	{0x00500201,	0x9130,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��I���ޣ����ԣ��޹��ܵ���
	{0x00500202,	0x9131,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��I���ޣ����ԣ��޹������
	{0x00500203,	0x9132,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��I���ޣ����ԣ��޹������
	{0x00500204,	0x9133,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��I���ޣ����ԣ��޹�ƽ����
	{0x00500205,	0x9134,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��I���ޣ����ԣ��޹��ȵ���

	{0x00500212,	0x3745,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//I���ޣ����ԣ��޹���������
	{0x00500213,	0x3745,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//I���ޣ����ԣ��޹��ܵ�������
	{0x00500214,	0x9b3f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն���I���ޣ����ԣ��޹�����
	{0x00500215,	0x953f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶���I���ޣ����ԣ��޹�����

	{0x00600200,	0x915f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ��II�����޹�����
	{0x00600201,	0x9150,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��II�����޹��ܵ���
	{0x00600202,	0x9151,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��II�����޹������
	{0x00600203,	0x9152,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��II�����޹������
	{0x00600204,	0x9153,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��II�����޹�ƽ����
	{0x00600205,	0x9154,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��II�����޹��ȵ���

	{0x00600212,	0x3746,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//II�����޹���������
	{0x00600213,	0x3746,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//II�����޹��ܵ�������
	{0x00600214,	0x9b5f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն���II�����޹�����
	{0x00600215,	0x955f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶���II�����޹�����

	{0x00700200,	0x916f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ��III�����޹�����
	{0x00700201,	0x9160,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��III�����޹��ܵ���
	{0x00700202,	0x9161,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��III�����޹������
	{0x00700203,	0x9162,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��III�����޹������
	{0x00700204,	0x9163,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��III�����޹�ƽ����
	{0x00700205,	0x9164,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��III�����޹��ȵ���

	{0x00700212,	0x3747,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//III�����޹���������
	{0x00700213,	0x3747,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//III�����޹��ܵ�������
	{0x00700214,	0x9b6f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն���III�����޹�����
	{0x00700215,	0x956f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶���III�����޹�����

	{0x00800200,	0x914f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����ǰ��IV���ޣ����ԣ��޹�����
	{0x00800201,	0x9140,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��IV���ޣ����ԣ��޹��ܵ���
	{0x00800202,	0x9141,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��IV���ޣ����ԣ��޹������
	{0x00800203,	0x9142,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��IV���ޣ����ԣ��޹������
	{0x00800204,	0x9143,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��IV���ޣ����ԣ��޹�ƽ����
	{0x00800205,	0x9144,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			4},			//����ǰ��IV���ޣ����ԣ��޹��ȵ���

	{0x00800212,	0x3748,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//IV���ޣ����ԣ��޹���������
	{0x00800213,	0x3748,		g_bEngTatolFmt,		sizeof(g_bEngTatolFmt),			5,			 4},		//IV���ޣ����ԣ��޹��ܵ�������
	{0x00800214,	0x9b4f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�Σ��ն���IV���ޣ����ԣ��޹�����
	{0x00800215,	0x954f,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),			27,			20},		//����1�����գ��¶���IV���ޣ����ԣ��޹�����

	{0x10100200,	0xa01f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ�������й�����
	{0x10100201,	0xa010,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�������
	{0x10100202,	0xa011,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�������
	{0x10100203,	0xa012,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�������
	{0x10100204,	0xa013,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�ƽ����
	{0x10100205,	0xa014,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�������

	{0x10100214,	0x9c0f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�Σ��ն��������й�����
	{0x10100215,	0xa41f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶��������й�����

	{0x10200200,	0xa02f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ�������й�����
	{0x10200201,	0xa020,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�������
	{0x10200202,	0xa021,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�������
	{0x10200203,	0xa022,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�������
	{0x10200204,	0xa023,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�ƽ����
	{0x10200205,	0xa024,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������й�������

	{0x10200214,	0x9c2f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�Σ��ն��ᷴ���й�����
	{0x10200215,	0xa42f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶��ᷴ���й�����

	{0x10300200,	0xa11f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ�������޹�����
	{0x10300201,	0xa110,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�������
	{0x10300202,	0xa111,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�������
	{0x10300203,	0xa112,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�������
	{0x10300204,	0xa113,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�ƽ����
	{0x10300205,	0xa114,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�������

	{0x10300214,	0x9c1f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�Σ��ն��������޹�����
	{0x10300215,	0xa41f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶��������޹�����

	{0x10400200,	0xa12f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ�������޹�����
	{0x10400201,	0xa120,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�������
	{0x10400202,	0xa121,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�������
	{0x10400203,	0xa122,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�������
	{0x10400204,	0xa123,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�ƽ����
	{0x10400205,	0xa124,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ�������޹�������

	{0x10400214,	0x9c3f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�Σ��ն��ᷴ���޹�����
	{0x10400215,	0xa42f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶��ᷴ���޹�����

	{0x10500200,	0xa13f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ��I���ޣ����ԣ��޹�����
	{0x10500201,	0xa130,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��I���ޣ����ԣ��޹�������
	{0x10500202,	0xa131,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��I���ޣ����ԣ��޹�������
	{0x10500203,	0xa132,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��I���ޣ����ԣ��޹�������
	{0x10500204,	0xa133,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��I���ޣ����ԣ��޹�ƽ����
	{0x10500205,	0xa134,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��I���ޣ����ԣ��޹�������
	{0x10500215,	0xa53f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶���I���ޣ����ԣ��޹�����

	{0x10600200,	0xa15f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ��II�����޹�����
	{0x10600201,	0xa150,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��II�����޹�������
	{0x10600202,	0xa151,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��II�����޹�������
	{0x10600203,	0xa152,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��II�����޹�������
	{0x10600204,	0xa153,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��II�����޹�ƽ����
	{0x10600205,	0xa154,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��II�����޹�������
	{0x10600215,	0xa55f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶���II�����޹�����

	{0x10700200,	0xa16f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ��III�����޹�����
	{0x10700201,	0xa160,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��III�����޹�������
	{0x10700202,	0xa161,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��III�����޹�������
	{0x10700203,	0xa162,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��III�����޹�������
	{0x10700204,	0xa163,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��III�����޹�ƽ����
	{0x10700205,	0xa164,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��III�����޹�������
	{0x10700215,	0xa56f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶���III�����޹�����

	{0x10800200,	0xa14f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����ǰ��IV���ޣ����ԣ��޹�����
	{0x10800201,	0xa140,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��IV���ޣ����ԣ��޹�������
	{0x10800202,	0xa141,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��IV���ޣ����ԣ��޹�������
	{0x10800203,	0xa142,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��IV���ޣ����ԣ��޹�������
	{0x10800204,	0xa143,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��IV���ޣ����ԣ��޹�ƽ����
	{0x10800205,	0xa144,		g_bFeeMaxDemFmt,	sizeof(g_bFeeMaxDemFmt),		15,			8},			//����ǰ��IV���ޣ����ԣ��޹�������
	{0x10800215,	0xa54f,		g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),			77,			40},		//����1�����գ��¶���IV���ޣ����ԣ��޹�����
	
	{0x20000200,	0xb61f,		g_bVoltDataFmt,		sizeof(g_bVoltDataFmt),			11,			6},			//����ǰ����ѹ					
	{0x20000201,	0xb611,		g_bPhaseVoltDataFmt,sizeof(g_bPhaseVoltDataFmt),	3,			2},			//����ǰ��A���ѹ
	{0x20000202,	0xb612,		g_bPhaseVoltDataFmt,sizeof(g_bPhaseVoltDataFmt),	3,			2},			//����ǰ��B���ѹ					
	{0x20000203,	0xb613,		g_bPhaseVoltDataFmt,sizeof(g_bPhaseVoltDataFmt),	3,			2},			//����ǰ��C���ѹ					
	{0x20000212,	0x3689,		g_bVoltDataFmt,		sizeof(g_bVoltDataFmt),			11,			6},			//��ѹ����

	{0x20010200,	0xb62f,		g_bCurDataFmt,		sizeof(g_bCurDataFmt),			17,			9},			//����ǰ������ 	//��������������������ô�죿	    
	{0x20010201,	0xb621,		g_bPhaseCurDataFmt,	sizeof(g_bPhaseCurDataFmt),		5,			3},			//����ǰ��A�����
	{0x20010202,	0xb622,		g_bPhaseCurDataFmt,	sizeof(g_bPhaseCurDataFmt),		5,			3},			//����ǰ��B�����
	{0x20010203,	0xb623,		g_bPhaseCurDataFmt,	sizeof(g_bPhaseCurDataFmt),		5,			3},			//����ǰ��C�����
	{0x20010400,	0xb6a0,		g_bPhaseCurDataFmt,	sizeof(g_bPhaseCurDataFmt),		5,			3},			//����ǰ���������
	{0x20010212,	0x3692,		g_bCurDataFmt,		sizeof(g_bCurDataFmt),			17,			9},			//��������

	{0x20020300,	0xb66f,		g_bAngleFmt,		sizeof(g_bAngleFmt),			11,			6},			//����ǰ�����
	{0x20020301,	0xb660,		g_bPhaseAngleFmt,	sizeof(g_bPhaseAngleFmt),		3,			2},			//����ǰ��A�����(�������ѹ�н�)
	{0x20020302,	0xb661,		g_bPhaseAngleFmt,	sizeof(g_bPhaseAngleFmt),		3,			2},			//����ǰ��B�����(�������ѹ�н�)
	{0x20020303,	0xb662,		g_bPhaseAngleFmt,	sizeof(g_bPhaseAngleFmt),		3,			2},			//����ǰ��C�����(�������ѹ�н�)	

	{0x20040200,	0xB63f,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},		//����ǰ���й����� 
	{0x20040201,	0xB630,		g_bPhasePowerDataFmt,sizeof(g_bPhasePowerDataFmt),	5,			3},			//����ǰ�����й����� 
	{0x20040202,	0xB631,		g_bPhasePowerDataFmt,sizeof(g_bPhasePowerDataFmt),	5,			3},			//����ǰ��A���й����� 
	{0x20040203,	0xB632,		g_bPhasePowerDataFmt,sizeof(g_bPhasePowerDataFmt),	5,			3},			//����ǰ��B���й����� 
	{0x20040204,	0xB633,		g_bPhasePowerDataFmt,sizeof(g_bPhasePowerDataFmt),	5,			3},			//����ǰ��C���й����� 
	{0x20040212,	0x3681,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},		//�й���������

	{0x20050200,	0xB64f,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},		//����ǰ���޹�����
	{0x20050201,	0xB640,		g_bPhasePowerDataFmt,sizeof(g_bPhasePowerDataFmt),	5,			3},			//����ǰ�����޹�����
	{0x20050202,	0xB641,		g_bPhasePowerDataFmt,sizeof(g_bPhasePowerDataFmt),	5,			3},			//����ǰ��A���޹�����
	{0x20050203,	0xB642,		g_bPhasePowerDataFmt,sizeof(g_bPhasePowerDataFmt),	5,			3},			//����ǰ��B���޹�����
	{0x20050204,	0xB643,		g_bPhasePowerDataFmt,sizeof(g_bPhasePowerDataFmt),	5,			3},			//����ǰ��C���޹�����
	{0x20050212,	0x3685,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},		//�޹���������

	{0x20060200,	0xB67f,		g_bPowerDataFmt,	sizeof(g_bPowerDataFmt),		22,			12},	//˲ʱ���ڹ������ݿ�
	{0x20060201,	0xB670,		g_bVarDmdFmt,		sizeof(g_bVarDmdFmt),			5,			4},		//˲ʱ�����ڹ���
	{0x20060202,	0xB671,		g_bVarDmdFmt,		sizeof(g_bVarDmdFmt),			5,			4},		//˲ʱA���ڹ���
	{0x20060203,	0xB672,		g_bVarDmdFmt,		sizeof(g_bVarDmdFmt),			5,			4},		//˲ʱB���ڹ���
	{0x20060204,	0xB673,		g_bVarDmdFmt,		sizeof(g_bVarDmdFmt),			5,			4},		//˲ʱC���ڹ���

	{0x200A0200,	0xB65f,		g_bCosDataFmt,		sizeof(g_bCosDataFmt),			14,			8},			//��������
	{0x200A0201,	0xB650,		g_bPhaseCosDataFmt,	sizeof(g_bPhaseCosDataFmt),		3,			2},			//�ܹ�������
	{0x200A0202,	0xB651,		g_bPhaseCosDataFmt,	sizeof(g_bPhaseCosDataFmt),		3,			2},			//A�๦������
	{0x200A0203,	0xB652,		g_bPhaseCosDataFmt,	sizeof(g_bPhaseCosDataFmt),		3,			2},			//B�๦������
	{0x200A0204,	0xB653,		g_bPhaseCosDataFmt,	sizeof(g_bPhaseCosDataFmt),		3,			2},			//C�๦������
	{0x200A0212,	0x3705,		g_bCosDataFmt,		sizeof(g_bCosDataFmt),			14,			8},			//������������

	{0x20140200,	0xc86f,		g_bMtrBlkRunStateFmt,	sizeof(g_bMtrBlkRunStateFmt),		30,		   14},			//�������״̬�����ݿ�
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

	{0x41100200,	0xea63,		g_bBitStringTypeFmt, sizeof(g_bBitStringTypeFmt),	3,			1},			//���ܱ�����������

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
	{0x300B0600,        0x03120000,   1,	       {0x03120301}},//��һ�����޹��������ޣ��޷���ǰ�ͽ������������
	{0x300B0700,        0x03120000,   1,	       {0x03120401}},//�ڶ������޹��������ޣ��޷���ǰ�ͽ������������
	{0x300B0800,        0x03120000,   1,	       {0x03120501}},//���������޹��������ޣ��޷���ǰ�ͽ������������
	{0x300B0900,        0x03120000,   1,	       {0x03120601}},//���������޹��������ޣ��޷���ǰ�ͽ������������
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

//��������ƽṹ
static const DWORD g_dwMtrEvtTimesRdList[] = {
	//wID,D0		D1			D2			D3			D4			D5			D6			D7 		
	//=====================================================================================================
	0x03360000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,//7
	0x00000000, 0x00000000, 0x03300D00, 0x03300E00, 0x03350000, 0x03370000, 0x1D000001, 0x1E000001, //15
	0x10010001, 0x11010001, 0x12010001, 0x18010001, 0x19010001, 0x1C010001, 0x1B010001, 0x13010001, //23
	0x1A010001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, //31
	0x10020001, 0x11020001, 0x12020001, 0x18020001, 0x19020001, 0x1C020001, 0x1B020001, 0x13020001, //39
	0x1A020001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, //47
	0x10030001, 0x11030001, 0x12030001, 0x18030001, 0x19030001, 0x1C030001, 0x1B030001, 0x13030001, //55
	0x1A030001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, //63
	0x14000001, 0x15000001, 0x16000001, 0x17000001, 0x03060000, 0x03110000, 0x03120000, 0x1F000001, //71
	0x20000001, 0x21000000, 0x03050000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, //79
	0x03300000, 0x03300100, 0x03300200, 0x03300300, 0x03300400, 0x03300500, 0x03300600, 0x03300700, //87
	0x03300800, 0x03300900, 0x03300a00, 0x03300b00, 0x03300c00, 0x03300f00, 0x03301000, 0x03301200, //85

	/*0x19010001, 0x19020001, 0x19030001, //A B C������ܴ���
	0x03300D00, //������ܴ���
	0x03350000, //�㶨�ų������ܴ���
	0x03300100, //��������ܴ���
	0x03110000, //�����ܴ���
	0x03370000, //��Դ�쳣�ܴ���
	0x03300E00, //����ť���ܴ���
	0x10000001, 0x10010001, 0x10020001,  0x10030001, //ʧѹ�ܴ���A B C��ʧѹ�ܴ���
	0x11010001, 0x11020001,  0x11030001, //A B C��Ƿѹ�ܴ���
	0x12010001, 0x12020001,  0x12030001, //A B C���ѹ�ܴ���
	0x16000001, //��ѹ��ƽ���ܴ���
	0x17000001, //������ƽ���ܴ���
	0x03300400, //Уʱ�ܴ���
	0x18010001, 0x18020001,  0x18030001, //A B C��ʧ���ܴ���
	0x13010001, 0x13020001,  0x13030001, //A B C������ܴ���
	0x03050000, //ȫʧѹ�ܴ��������ۼ�ʱ��
	0x14000001, //��ѹ�������ܴ���
	0x15000001, //�����������ܴ���
	0x21000000, //���������ܴ���
	0x1A010001, 0x1A020001, 0x1A030001, //A B C������ܴ���
	0x1B010001, 0x1B020001, 0x1B030001, //A B C���й����ʷ����ܴ���
	0x1C010001, 0x1C020001, 0x1C030001, //A B C������ܴ���
	0x03360000, //���ɿ��������ܴ���
	0x03300200, //���������ܴ���
	0x03300300, //�¼������ܴ���
	0x03300000, //����ܴ���
	0x1D000001, //��բ����
	0x1E000001, //��բ����
	0x03120000, //�����й����������ܴ���
	0x1F000001, //�ܹ������س������ܴ���
	0x20000001, //�������ز�ƽ���ܴ���*/
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

//����:��ȡ�����ϱ�״̬�ֶ�Ӧλ�¼�����ID
//����:@bBit ����¼�IDλ��
//����:�����ȷ�򷵻ض�Ӧ���¼�����ID,���򷵻�0
DWORD GetMtrEvtTimesID(BYTE bBit)
{
	if (bBit < sizeof(g_dwMtrEvtTimesRdList)/sizeof(DWORD))
	{
		return g_dwMtrEvtTimesRdList[bBit];
	}

	return 0;
}

//����:���OAD�б��ж�Ӧ��07645Э����¼������������
//		@dwErcNumID:�¼�����ID
//���أ��ҵ���Ӧ������Ϣ�򷵻�����ָ��
//��ע����������ֻ��ȡ�������й��ģ��������������޸�
TErcRdCtrl* GetRd07645ErcMap(DWORD dwErcNumID)
{
	for (WORD i=0; i<sizeof(g_ErcRdCtrl)/sizeof(TErcRdCtrl); i++)
	{
		if (g_ErcRdCtrl[i].dwErcNumID == dwErcNumID)
		{
			return &g_ErcRdCtrl[i];
		}
	}

	return NULL;
}