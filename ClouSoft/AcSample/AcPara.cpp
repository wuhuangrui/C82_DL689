/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcSample.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԽ�������������װ�غͱ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 * ��    ע: ���ļ���Ҫ�������θ��汾������Ĳ�����
 *********************************************************************************************************/
#include "syscfg.h"
#include "Sample.h"
#include "ComAPI.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "Pulse.h"

const static WORD g_wPulseCurEnergyID[2][MAX_PULSE_TYPE] = 
{
	//����������ID
	{
		0x2414, 0x2415,	//���С�����
		0x2416, 0x2417, //���С�����
	},	//�߾���
	
	//��Ӧ���ڲ�����ID
	{
		EP_POS_ABC, EQ_POS_ABC,	//���С�����
		EP_NEG_ABC, EQ_NEG_ABC, //���С�����
	},	
};

/*static WORD g_wPulseCurDemandID[2][MAX_PULSE_TYPE] =
{
	//����������ID
	{
		0xa01f, 0xa11f, //���С������������
		0xa02f, 0xa12f, //���С������������
	},
	
	//��Ӧ���ڲ�����ID
	{
		EP_POS_ABC, EQ_POS_ABC,	//���С������������
		EP_NEG_ABC, EQ_NEG_ABC, //���С������������
	}
};

static WORD g_wPulseCurTimeID[MAX_PULSE_TYPE] = 
{
	0xb01f, 0xb11f,  //���С������������ʱ��
	0xb02f, 0xb12f,  //���С������������ʱ��
};*/

const static WORD g_wAcCurEnergyID[3][AC_ENERGY_NUM] = 
{
	//����������ID
	{/*
		0x901f, 0x902f, 								//���������й�
		0x911f, 0x912f, 								//����޹�1��2
		0x913f, 0x915f, 0x916f, 0x914f, 				//һ�����������޹�
		
		0x9070,0x9071,0x9072,							//A/B/C���������й�����
		0x9080,0x9081,0x9082,							//A/B/C���෴���й�����
		0x9170,0x9171,0x9172,							//A/B/C���������޹�����,����
		0x9180,0x9181,0x9182,							//A/B/C���෴���޹�����,����
		
		0x900f,//����й�
*/
/*		
		0x0010, 0x0020,				//���������й�
		0x0030, 0x0040, 			//����޹�1��2
		0x0050, 0x0060, 0x0070, 0x0080, 		//һ�����������޹�
		
		0x0011, 0x0012, 0x0013,							//A/B/C���������й�����
		0x0021, 0x0022, 0x0023,							//A/B/C���������й�����
		0x0031, 0x0032, 0x0033,							//A/B/C���������й�����
		0x0041, 0x0042, 0x0043,							//A/B/C���������й�����
		
		0x0001,//����й�
	*/
		0x0610, 0x0620,							//���������й�
		0x0630, 0x0640, 						//����޹�1��2
		0x0650, 0x0660, 0x0670, 0x0680, 		//һ�����������޹�
		
		0x0611, 0x0612, 0x0613,					//A/B/C���������й�����
		0x0621, 0x0622, 0x0623,					//A/B/C���෴���й�����
		0x0631, 0x0632, 0x0633,					//A/B/C����޹�1����
		0x0641, 0x0642, 0x0643,					//A/B/C����޹�2����
		
		0x0601,									//����й�	

		0x0631, 0x0632, 0x0633,					//A/B/C��������޹�1
		0x0641, 0x0642, 0x0643, 				//A/B/C��������޹�2

		0x0651, 0x0652, 0x0653,					//A/B/C����һ�����޹�
		0x0661, 0x0662, 0x0663, 				//A/B/C����������޹�
		0x0671, 0x0672, 0x0673, 				//A/B/C�����������޹�
		0x0681, 0x0682, 0x0683, 				//A/B/C�����������޹�
//��ʱ��������
//			0x0690,	0x06a0,							//��/�������ڵ���
//			0x0691, 0x0692, 0x0693, 				//A/B/C�����������ڵ���
//			0x06a1, 0x06a2, 0x06a3, 				//A/B/C���෴�����ڵ���
		
	}, 
	
	//��Ӧ���ڲ�����ID
	{
		EP_POS_ABC, EP_NEG_ABC,					//���������й�
		EQ_COM_ABC1, EQ_COM_ABC2, 				//������޹�1��2
		EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 			//һ�����������޹�

		EP_POS_A,EP_POS_B,EP_POS_C,				//A/B/C���������й�����
		EP_NEG_A,EP_NEG_B,EP_NEG_C,				//A/B/C���෴���й�����

		EQ_POS_A,EQ_POS_B,EQ_POS_C,				//A/B/C��������޹�����
		EQ_NEG_A,EQ_NEG_B,EQ_NEG_C,				//A/B/C���������޹�����		
		EP_COM_ABC,								//����й���
		EQ_COM_A1,EQ_COM_B1,EQ_COM_C1, 			//A/B/C��������޹�1
		EQ_COM_A2,EQ_COM_B2,EQ_COM_C2,			//A/B/C��������޹�2

		EQ_Q1_A, EQ_Q1_B, EQ_Q1_C, 				//A/B/C����һ�����޹�
		EQ_Q2_A, EQ_Q2_B, EQ_Q2_C,				//A/B/C����������޹�
		EQ_Q3_A, EQ_Q3_B, EQ_Q3_C,				//A/B/C�����������޹�
		EQ_Q4_A, EQ_Q4_B, EQ_Q4_C,				//A/B/C�����������޹�

//			ES_POS_ABC, ES_NEG_ABC,					//������������
//			ES_POS_A,ES_POS_B,ES_POS_C, 			//A/B/C�����������ڵ���
//			ES_NEG_A,ES_NEG_B,ES_NEG_C, 			//A/B/C���෴�����ڵ���

		
	},
	{
		0x0010, 0x0020,									//���������й�
		0x0030, 0x0040, 								//����޹�1��2
		0x0050, 0x0060, 0x0070, 0x0080, //һ�����������޹�
		
		0x0011, 0x0012, 0x0013,					//A/B/C���������й�����
		0x0021, 0x0022, 0x0023,					//A/B/C���������й�����
		0x0031, 0x0032, 0x0033,					//A/B/C���������й�����
		0x0041, 0x0042, 0x0043,					//A/B/C���������й�����
		
		0x0001,//����й�	
		0x0031, 0x0032, 0x0033, 		//A/B/C��������޹�1
		0x0041, 0x0042, 0x0043, 		//A/B/C��������޹�2

		0x0051, 0x0052, 0x0053,					//A/B/C����һ�����޹�
		0x0061, 0x0062, 0x0063, 				//A/B/C����������޹�
		0x0071, 0x0072, 0x0073, 				//A/B/C�����������޹�
		0x0081, 0x0082, 0x0083, 				//A/B/C�����������޹�

//			0x0090,	0x00a0,							//��/�������ڵ���
//			0x0091, 0x0092, 0x0093, 				//A/B/C�����������ڵ���
//			0x00a1, 0x00a2, 0x00a3, 				//A/B/C���෴�����ڵ���
	}
};

const static WORD g_wAcCurDemandID[3][AC_DEMAND_NUM] =
{
	//����������ID
	{
		0x1010, 0x1020, 						//���������й��������
		0x1030, 0x1040, 						//����޹�1��2�������
		0x1050, 0x1060, 0x1070, 0x1080, 		//һ�����������޹��������
		
		0x1011, 0x1012, 0x1013,					//A/B/C���������й�����
		0x1021, 0x1022, 0x1023,					//A/B/C���෴���й�����
		0x1031, 0x1032, 0x1033,					//A/B/C����޹�1����
		0x1041, 0x1042, 0x1043,					//A/B/C����޹�2����	
		0x1051, 0x1052, 0x1053, 				//A/B/Cһ�����޹�	
		0x1061, 0x1062, 0x1063, 				//A/B/C�������޹�	
		0x1071, 0x1072, 0x1073, 				//A/B/C�������޹�	
		0x1081, 0x1082, 0x1083, 				//A/B/C�������޹�	

//			0x2017, 0x2018,							//����ֵ�й��޹�����	
//			0x1090,	0x10a0,							//��/�������ڵ���
//			0x1091, 0x1092, 0x1093, 				//A/B/C�����������ڵ���
//			0x10a1, 0x10a2, 0x10a3, 				//A/B/C���෴�����ڵ���

	},
	
	//��Ӧ���ڲ�����ID
	{
		EP_POS_ABC, EP_NEG_ABC,			//���������й�
		EQ_COM_ABC1, EQ_COM_ABC2, 		//����޹�1��2�������
		EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 	//һ�����������޹��������
		
		EP_POS_A,EP_POS_B,EP_POS_C,		//A/B/C���������й�����
		EP_NEG_A,EP_NEG_B,EP_NEG_C,		//A/B/C���෴���й�����

		EQ_COM_A1,EQ_COM_B1,EQ_COM_C1,		//A/B/C��������޹�1����
		EQ_COM_A2,EQ_COM_B2,EQ_COM_C2,		//A/B/C��������޹�2����	
		
		EQ_Q1_A, EQ_Q1_B, EQ_Q1_C,				//A/B/C����һ�����޹�
		EQ_Q2_A, EQ_Q2_B, EQ_Q2_C,				//A/B/C����������޹�
		EQ_Q3_A, EQ_Q3_B, EQ_Q3_C,				//A/B/C�����������޹�
		EQ_Q4_A, EQ_Q4_B, EQ_Q4_C,				//A/B/C�����������޹�
		EP_ABS_ABC,EQ_ABS_ABC,					//����ֵ�й��޹�����	
//			ES_POS_ABC, ES_NEG_ABC,					//������������
//			ES_POS_A,ES_POS_B,ES_POS_C, 			//A/B/C�����������ڵ���
//			ES_NEG_A,ES_NEG_B,ES_NEG_C, 			//A/B/C���෴�����ڵ���
	},


	//����Ϊ��ǰʵʱ��������������ID�������������,���BANK2
	{
		0x3010, 0x3020, 						//���������й��������
		0x3030, 0x3040, 						//����޹�1��2�������
		0x3050, 0x3060, 0x3070, 0x3080, 		//һ�����������޹��������
		
		0x3011, 0x3012, 0x3013,					//A/B/C���������й�����
		0x3021, 0x3022, 0x3023,					//A/B/C���෴���й�����
		0x3031, 0x3032, 0x3033,					//A/B/C����޹�1����
		0x3041, 0x3042, 0x3043,					//A/B/C����޹�2����	
		0x3051, 0x3052, 0x3053, 				//A/B/Cһ�����޹�	
		0x3061, 0x3062, 0x3063, 				//A/B/C�������޹�	
		0x3071, 0x3072, 0x3073, 				//A/B/C�������޹�	
		0x3081, 0x3082, 0x3083, 				//A/B/C�������޹�	

		0x3117, 0x3118,							//����ֵ�й��޹�����	
//			0x1090,	0x10a0,							//��/�������ڵ���
//			0x1091, 0x1092, 0x1093, 				//A/B/C�����������ڵ���
//			0x10a1, 0x10a2, 0x10a3, 				//A/B/C���෴�����ڵ���

	},
		
};

//Ҫ����1,2,3,4���޵�˳��
//	static WORD g_wAcCurTimeID[AC_DEMAND_NUM] = 
//	{
//		0xb01f, 0xb02f, //���������й��������ʱ��
//		0xb11f, 0xb12f, //���������޹��������ʱ��
//		0xb13f, 0xb15f, 0xb16f, 0xb14f, //һ�����������޹��������ʱ��
//	};
const static WORD g_wAcCurTimeID[AC_DEMAND_NUM] = 
{
		0x1010, 0x1020, 						//���������й��������
		0x1030, 0x1040, 						//����޹�1��2�������
		0x1050, 0x1060, 0x1070, 0x1080, 		//һ�����������޹��������
		
		0x1011, 0x1012, 0x1013,					//A/B/C���������й�����
		0x1021, 0x1022, 0x1023,					//A/B/C���෴���й�����
		0x1031, 0x1032, 0x1033,					//A/B/C����޹�1����
		0x1041, 0x1042, 0x1043,					//A/B/C����޹�2����	
		0x1051, 0x1052, 0x1053, 				//A/B/Cһ�����޹�	
		0x1061, 0x1062, 0x1063, 				//A/B/C�������޹�	
		0x1071, 0x1072, 0x1073, 				//A/B/C�������޹�	
		0x1081, 0x1082, 0x1083, 				//A/B/C�������޹�	

//			0x2017, 0x2018,							//����ֵ�й��޹�����	
//			0x1090,	0x10a0,							//��/�������ڵ���
//			0x1091, 0x1092, 0x1093, 				//A/B/C�����������ڵ���
//			0x10a1, 0x10a2, 0x10a3, 				//A/B/C���෴�����ڵ���

};


//���������0xff��ʾ����,��ϵļ��㷽ʽ��Ҫ����
//wDelta ��ֵ��ʾ���ܵ���С�̶ȵ�������
//wCalcPara ���ڱ�ʾ��ID�ڷ�ID���ۼӷ�ʽ����������ID�ĵ�һ����FLAG_ADD����FLAG_SUB���ڶ�������ID��FLAG_ADD<<2,��FLAG_SUB<<2
//												���ǵ�һ������ID��(FLAG_ADD | FLAG_ADD<<2),
// д0��ʾʹ��Ĭ�ϲ�����ϵͳ�ڲ�Ĭ�ϲ���ֻ������й����޹�
//����ID������ʶʹ��0xff��ʾ������0xff��ID���ۼ����ID
static TEnergyCorrect g_tEnergyCorrect[] = {
//	��ֵ	���㷽ʽ		�ܺ�ID				����ID
	{3, 	0,				EQ_COM_ABC1, 	{EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 0xff}},
	{3, 	0,				EQ_COM_ABC2, 	{EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 0xff},},
	{3, 	0,				EP_COM_ABC, 	{EP_POS_ABC, EP_NEG_ABC, 0xff},},
};

//У׼ʱ���¶ȶ�д�ӿ�
BYTE GetTempValue()
{
	BYTE bBuf[4];
	ReadItemEx(BN25, PN0, 0x5022, bBuf);
	return bBuf[0];
}

void SaveTempValue(BYTE bValue)
{
	WriteItemEx(BN25, PN0, 0x5022, &bValue);
	TrigerSaveBank(BN25, 0, -1);
}


//��ϵ����ۼӷ�ʽ���ж�
//����й����޹�����Ϸ�ʽ��������ʹ��Ĭ�ϵĲ���
void LoadDefaultCorrectPara(TEnergyCorrect* pCorrect, TAcPara* pAcPara)
{
	int i = 0;

	for (i=0; i<pAcPara->EnergyPara.wEnergyCorrectNum; i++)
	{
		if (pCorrect->wCalcPara == 0)//0��ʱ���ʾʹ��Ĭ�ϲ���
		{
			if (pCorrect->wSumID>=EP_COM_A && pCorrect->wSumID<=EP_COM_ABC)//����й�
				pCorrect->wCalcPara = pAcPara->bCombEpMode;
			else if (pCorrect->wSumID>=EQ_COM_A1 && pCorrect->wSumID<=EQ_COM_ABC1)//����޹�
				pCorrect->wCalcPara = pAcPara->bAEqMode;
			else if (pCorrect->wSumID>=EQ_COM_A2 && pCorrect->wSumID<=EQ_COM_ABC2)//����޹�
				pCorrect->wCalcPara = pAcPara->bREqMode;
		}

		pCorrect++;
	}
}

//��������ĳ�ʼ��
bool PulseLoadPara(TPulseCfg* pPulseCfg, TPulsePara* pPulsePara)
{
    WORD i, wOI;
    int iLen;
    bool fValid = true;
    BYTE bBuf[32];
    
    WORD wPn = pPulseCfg->wPn;
    BYTE bPulseType = pPulseCfg->bType;
    BYTE bPortIndex = pPulseCfg->bPortNo - 1;

    //if (wPn == PN0)		//���ɲ�����
    //	return false;

	wOI = OI_PULSE_BASE + bPortIndex;
    memset(pPulsePara, 0, sizeof(TPulsePara));
	pPulsePara->wPn = wPn;
	//pPulsePara->wRate = GetRate(wPn);
	//-------���ܲ���---------------------------
	if (ReadItemEx(BN0, PN0, 0x400C, bBuf)<=0)  //����
	{
		/*
		��ʱ����(p��14) 				unsigned��
		��ʱ�α�����q��8��				unsigned��
		��ʱ����(ÿ���л���)��m��14��	unsigned��
		��������k��63�� 				unsigned��
		������������n��254��			unsigned
		*/
		//DT_STRUCT,05,
		bBuf[9] = RATE_NUM; 
	}
	pPulsePara->EnergyPara.wRateNum = bBuf[9];

	//-------���ܲ���---------------------------
	pPulsePara->EnergyPara.wPn0 = wPn;  	//���������
	pPulsePara->EnergyPara.wPn1 = 0xffff;

	pPulsePara->EnergyPara.wRate = 1;   	//��ǰ�ķ���,�ɼ�ʱˢ��	
	pPulsePara->EnergyPara.dwConst= pPulseCfg->i64Const; 	//���峣��
	pPulsePara->EnergyPara.wEpFrac = 4;	//�й����ܵ�С��λ��
	pPulsePara->EnergyPara.wEqFrac = 4;	//�޹����ܵ�С��λ��
#ifdef ACLOG_ENABLE
	pPulsePara->EnergyPara.fEnableLog = true; //֧������д������
#else
	pPulsePara->EnergyPara.fEnableLog = false; //��֧������д������
#endif
	pPulsePara->EnergyPara.wLogID = LOG_PULSE_ENERGY1 + bPortIndex;	 //��־�ļ�ID
	pPulsePara->EnergyPara.wSignID = 0;  //������ŵ��������ID,�ŵ�������������,0��ʾ���������
	pPulsePara->EnergyPara.i64EpMax = EP_MAX; //�й����ܵ����ֵ
	pPulsePara->EnergyPara.i64EqMax = EQ_MAX; //�޹����ܵ����ֵ

	pPulsePara->EnergyPara.wTypeNum = PULSE_ENERGY_NUM; //wID[3][ENERGY_TYPE_MAX]��ʵ�ʵ������͵ĸ���
	pPulsePara->EnergyPara.wLogNum = PULSE_ENERGY_NUM;//���籣�����
	for (i=0; i<PULSE_ENERGY_NUM; i++)
	{
		pPulsePara->EnergyPara.wInnerID[i] = g_wPulseCurEnergyID[1][bPulseType]; //�ڲ�����ĵ���ID
		pPulsePara->EnergyPara.wID[0][i] = g_wPulseCurEnergyID[0][bPulseType];  //����	�߾��ȵ���
		pPulsePara->EnergyPara.wPlusID[i] = 0;  //����	�;��ȵ���
		pPulsePara->EnergyPara.wID[1][i] = 0;	 //����
		pPulsePara->EnergyPara.wID[2][i] = 0;	 //g_wPulseCurEnergyID[0][bPulseType] + 0x0400  //����
		pPulsePara->EnergyPara.wID[3][i] = 0;	//g_wPulseCurEnergyID[0][bPulseType] + 0x0800;  //������
		
		if ((g_wPulseCurEnergyID[0][bPulseType]&0x1) == 0)  //�й�
		{
			pPulsePara->EnergyPara.fEp[i] = true;
			pPulsePara->EnergyPara.fSign[i] = false;	//�й���֧�ַ���
		}
		else
		{	
			pPulsePara->EnergyPara.fEp[i] = false;
			pPulsePara->EnergyPara.fSign[i] = false;		//�޹���֧�ַ���
		}
	}
	
	//------------��������----------------------
	//�������ڼ�����ʱ��
	/*WORD wDemandPeriod, wSlideInterv, wSlideNum;
	ReadItemEx(BN0, wPn, 0xc111, bBuf); //��������
	wDemandPeriod = BcdToByte(bBuf[0]);

	ReadItemEx(BN0, wPn, 0xc112, bBuf); //����ʱ��
	wSlideInterv = BcdToByte(bBuf[0]);
	
	if (wDemandPeriod==0 || wSlideInterv==0 || 
		wDemandPeriod>60)
	{
		wDemandPeriod = 15;	//15
		wSlideInterv = 1;
	}

	//����ʽ�����������뻬��ʱ���У��
	if (wDemandPeriod!=wSlideInterv && //��������ʽ�������������������뻬��ʱ��������ó�ͬһ��ֵ
		(wDemandPeriod%wSlideInterv!=0 || wDemandPeriod/wSlideInterv<5))
	{	//�������ڱ����ǻ���ʱ���5��������(������)
		wDemandPeriod = 15;
		wSlideInterv = 1;
	}

	wSlideNum = wDemandPeriod/wSlideInterv; //һ�����������ڵĻ���ʱ��ĸ���
	
	pPulsePara->DemandPara.wPn0 = wPn; //���������
	pPulsePara->DemandPara.wPn1 = 0xffff;	
	
	pPulsePara->DemandPara.wRate = 1;    //��ǰ�ķ���,�ɼ�ʱˢ��
	pPulsePara->DemandPara.dwConst = pPulseCfg->i64Const; 	//���峣��ȡ�����й�
	pPulsePara->DemandPara.wFrac = 4;		//����С��λ��,��׼��ʽ��645���NN.NNNN(kw/kvar),��Ϊ4
#ifdef ACLOG_ENABLE
	pPulsePara->DemandPara.fEnableLog = true; //֧������д������
#else
	pPulsePara->DemandPara.fEnableLog = false; //��֧������д������
#endif
	pPulsePara->DemandPara.wLogID = LOG_PULSE_DEMAND1 + bPortIndex;	 //��־�ļ�ID
	pPulsePara->DemandPara.wMeteringDay = 1;	//������
	pPulsePara->DemandPara.wMeteringHour = 0;	//�����յ�Сʱ
	pPulsePara->DemandPara.wSlipNum = wSlideNum;	//������,ͨ�������������/����ʱ�����,�ɼ�ʱˢ�� 15
	pPulsePara->DemandPara.dwSlipInterv = wSlideInterv;	//����ʱ��,��λ���� 1
	
	pPulsePara->DemandPara.wTypeNum = PULSE_DEMAND_NUM; //wID[3][DEMAND_TYPE_MAX]��ʵ�ʵ������͵ĸ���
	pPulsePara->DemandPara.wLogNum = PULSE_DEMAND_NUM;
	for (i=0; i<PULSE_DEMAND_NUM; i++)
	{
		pPulsePara->DemandPara.wInnerID[i] = g_wPulseCurDemandID[1][bPulseType]; //�ڲ�����ĵ���ID
		
		pPulsePara->DemandPara.wDemandID[0][i] = g_wPulseCurDemandID[0][bPulseType]; //����  
		pPulsePara->DemandPara.wDemandID[1][i] = g_wPulseCurDemandID[0][bPulseType] + 0x0400; //���� 
		pPulsePara->DemandPara.wDemandID[2][i] = g_wPulseCurDemandID[0][bPulseType] + 0x0800; //������
		
		pPulsePara->DemandPara.wTimeID[0][i] = g_wPulseCurTimeID[bPulseType]; //����  
		pPulsePara->DemandPara.wTimeID[1][i] = g_wPulseCurTimeID[bPulseType] + 0x0400; //���� 
		pPulsePara->DemandPara.wTimeID[2][i] = 0;	//g_wPulseCurTimeID[bPulseType] + 0x0800; //������
	}

	//�Զ�������,���ֽڱ�ʾ������,���ֽڱ�ʾ�����յ�Сʱ
	if (ReadItemEx(BN0, PN0, 0xc117, bBuf) <= 0)  //�Զ�������
	{
		bBuf[1] = 0x01; bBuf[0] = 0x00;	//1�����
	}
	
	if (bBuf[1]==0x00 && bBuf[0]==0x00)
	{
		bBuf[1] = 0x01; bBuf[0] = 0x00;	//1�����
	}
		
	memset(pPulsePara->wAutoDate, 0xff, sizeof(pPulsePara->wAutoDate));
	pPulsePara->wAutoDate[0] = (WORD )BcdToByte(bBuf[1])*0x100 + BcdToByte(bBuf[0]);
	//pPulsePara->wDayFrzTime = 0xffff;  //�ս���ʱ��,Сʱ,BIN,0xffff��ʾ������,
	
	iLen = OoReadAttr(wOI, ATTR2, bBuf, &pbFmt, &wFmtLen);	//ͨ�ŵ�ַ
	if (iLen > 0)
	{
		memset(pPulsePara->bAddr, 0, PUSLE_ADDR_LEN);	//������Чʱ ��ʼ��Ϊȫ0
		if (bBuf[0] == DT_OCT_STR)
		{
			iLen = (bBuf[1]<PUSLE_ADDR_LEN) ? bBuf[1] : PUSLE_ADDR_LEN;		//bBuf[1]Ϊʵ�ʳ���
			if (iLen > 0)
				memcpy(pPulsePara->bAddr, bBuf+2, iLen);	
		}
	}*/

	return true;
}

//�Ƿ�Ϊ��ǰ��ϵ���ID
bool IsCombCurEnergyID(WORD wID)
{
	WORD i;
	const WORD wCombEngID[] = {0x900f,0x911f, 0x912f,};
				
	for (i=0; i<sizeof(wCombEngID)/sizeof(WORD); i++)
	{
		if (wID == wCombEngID[i])
			return true;
	}

	return false;	
}

//���ɲ����ĳ�ʼ��
bool AcLoadPara(WORD wPn, TAcPara* pAcPara)
{
	WORD i;
	BYTE bBuf[32];
	int iRet,iTmp;

	memset(pAcPara, 0, sizeof(TAcPara));
	pAcPara->wPoint = wPn;
	pAcPara->wRate = 1;   	//��ǰ�ķ���,�ɼ�ʱˢ��
	
	//----------���峣��-----------
	iRet = ReadItemEx(BN25, PN0, 0x5004, bBuf); //0x5004 3 ���峣��,BCD��
	if (iRet<=0 || IsAllAByte(bBuf, 0, 3))
		ReadItemEx(BN4, wPn, 0xc030, bBuf); //0xa010 3 ���峣��,BCD��
	pAcPara->dwConst = BcdToDWORD(bBuf, 3);
	if (pAcPara->dwConst == 0)
	{
		pAcPara->dwConst = 6400;
		DWORDToBCD(pAcPara->dwConst, bBuf, 3);
//			WriteItemEx(BN4, PN0, 0xc030, bBuf); //0xa010 3 ���峣��,BCD��
//			WriteItemEx(BN4, PN0, 0xc031, bBuf); //0xa010 3 ���峣��,BCD��
//			WriteItemEx(BN4, wPn, 0xc030, bBuf); //0xa010 3 ���峣��,BCD��
//			WriteItemEx(BN4, wPn, 0xc031, bBuf); //0xa010 3 ���峣��,BCD��
	}
	
	g_iEPerPulse = 3600L * 1000L * 10 * 8 / pAcPara->dwConst * 10L * 1000L; //0x1C9C3800; (10L*1000L*3600L*1000L*10*8/6000)    
					//ÿ��������ڶ��ٸ� ��/100 * ����/8

	if (pAcPara->dwConst > 10000)   //��3��4��12A,220Vʱÿ�������� (���峣��/454) ��������
	{			  //22.02
		g_PulseWidthTop = 10*8;     //ÿ�����50������
		g_PulseWidthBottom = 10*8;
	}
	else if (pAcPara->dwConst > 4800)   
	{				//10.57	������
		g_PulseWidthTop = 20*8;  	  //ÿ�����25������
		g_PulseWidthBottom = 20*8;
	}
	else if (pAcPara->dwConst > 2400)   
	{				   //5.28	������
		g_PulseWidthTop = 40*8;    //ÿ�����12������
		g_PulseWidthBottom = 40*8;
	}
	else   //==2400
	{
		g_PulseWidthTop = 10*8;    //ÿ�����6������
		g_PulseWidthBottom = 10*8;
	}
	
/*	
	//----------���߷�ʽ------------
	pAcPara->bConnectType = GetConnectType(wPn); //�ն˽��߷�ʽ 1	1:����;3:��������;4:��������

	//----------���ѹ,����---------
	if (ReadItemEx(BN0, wPn, 0x019f, bBuf) > 0)
	{		
		pAcPara->dwUn = (DWORD )Fmt7ToVal(bBuf+4, 2); //���ѹֵ
		pAcPara->dwIn = (DWORD )Fmt22ToVal(bBuf+6, 1)*100; //������ֵ(1���ֽڣ�1��С��λ��
		if (pAcPara->dwUn == 0)
			pAcPara->dwUn = 2200;	//���ѹ,��ʽNNNNN.N
		if (pAcPara->dwIn == 0)
			pAcPara->dwIn = 5000;	//�����,��ʽNNN.NNN
	}
	else
	{
		pAcPara->dwUn = 2200;	//���ѹ,��ʽNNNNN.N
		pAcPara->dwIn = 5000;	//�����,��ʽNNN.NNN
	}*/
	
	ReadItemEx(BN25, PN0, 0x5003, bBuf);
	if (bBuf[0] == 0 || bBuf[0] > 4)
		pAcPara->bConnectType = 4;
	else
		pAcPara->bConnectType = bBuf[0];
	
	ReadItemEx(BN25, PN0, 0x5001, bBuf);
	pAcPara->dwUn = (DWORD )BcdToDWORD(bBuf, 3)/10; //���ѹֵ2200,һλС��
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	pAcPara->dwIn = (DWORD )BcdToDWORD(bBuf, 3); //�����ֵ
	if (pAcPara->dwUn == 0)
		pAcPara->dwUn = 2200;	//���ѹ,��ʽNNNNN.N
	if (pAcPara->dwIn == 0)
		pAcPara->dwIn = 1500;	//�����,��ʽNNN.NNN

	//�й������ۼ�ģʽ
	
	if (ReadItemEx(BN0, PN0, 0x4112, bBuf) <= 0)  //�й����ܼ�����ʽѡ��
		pAcPara->bCombEpMode |= (P_POSADD|P_NEGADD);
	else
	{
		pAcPara->bCombEpMode = 0;
		//����й����ۻ���ʽ
		if ((bBuf[2] & 0x03) == 0x01)
			pAcPara->bCombEpMode |= P_POSADD;
		else if ((bBuf[2] & 0x03) == 0x02)
			pAcPara->bCombEpMode |= P_POSSUB;
		
		if (((bBuf[2]>>2) & 0x03) == 0x01)
			pAcPara->bCombEpMode |= P_NEGADD;
		else if (((bBuf[2]>>2) & 0x03) == 0x02)
			pAcPara->bCombEpMode |= P_NEGSUB;
	}
	DTRACE(DB_AC,("\r\n####AcLoadPara pAcPara->bCombEpMode=%d", pAcPara->bCombEpMode));
				//		D2			|		D1			|			D0			|
				// �����������		| 	���򲻼���		| 		�����ۼ�ģʽ	|
				// 0:������;1����	| 0:����;1������	| 0:������;1����ֵ��	|
				// D2D1=00:��->��,��->��;������ֱ����
				// D2D1=01:��->��;		ֻ������,���򲻼�
				// D2D1=10:����->��,��->��;
				// D2D1=11:����->��		���򲻼�
	//�й������ۼ�ģʽ
	if (ReadItemEx(BN10, PN0, 0xa122, bBuf) <= 0)  //�й����ܼ�����ʽѡ��
		bBuf[0] = 0;
	pAcPara->bEpMode = bBuf[0];

	//�޹������ۼӱ�־
	//��/�����޹������ۼӱ�־,D7D6,D5D4,D3D2,D1D0,�ֱ��Ӧ4~1�����޹��ۼӱ�־,
	//��λ:1-��; 0-��;	
	//��λ:1-����; 0-������
	if (ReadItemEx(BN0, PN0, 0x4113, bBuf) <= 0)  //�����޹������ۼӱ�־
		pAcPara->bAEqMode = Q1ADD | Q2ADD;
	else
	{
		for (int i = 0; i < 4; i++)//�ֱ��ʾ��һ����������������
		{
			if (((bBuf[2]>>i*2) & 0x03) == 0x01)
				pAcPara->bAEqMode |= (QADD<<i*2);
			else if (((bBuf[2]>>i*2) & 0x03) == 0x02)
				pAcPara->bAEqMode |= (QSUB<<i*2);
		}
	}
	DTRACE(DB_AC,("\r\n####AcLoadPara pAcPara->bAEqMode=%d", pAcPara->bAEqMode));

	if (ReadItemEx(BN0, PN0, 0x4114, bBuf) <= 0)  //�����޹������ۼӱ�־
		pAcPara->bREqMode = Q3ADD | Q4ADD;
	else
	{
		for (int i = 0; i < 4; i++)
		{
			if (((bBuf[2]>>i*2) & 0x03) == 0x01)
				pAcPara->bREqMode |= (QADD<<i*2);
			else if (((bBuf[2]>>i*2) & 0x03) == 0x02)
				pAcPara->bREqMode |= (QSUB<<i*2);
		}
	}
	DTRACE(DB_AC,("\r\n####AcLoadPara pAcPara->bREqMode=%d", pAcPara->bREqMode));


	//�Զ�������,���ֽڱ�ʾ������,���ֽڱ�ʾ�����յ�Сʱ
	if (ReadItemEx(BN0, PN0, 0x4116, bBuf) <= 0)  //�Զ�������
	{
		bBuf[5] = 0x01; bBuf[7] = 0x00;	//1�����
	}
//		bBuf[0]= DT_ARRAY;
//		bBuf[1]= 0x03;
//		bBuf[2]= DT_STRUCT;
//		bBuf[3]= 0x02;
//		bBuf[4]= DT_UNSIGN;
//		bBuf[5]= data;
//		bBuf[6]= DT_UNSIGN;
//		bBuf[7]= data;
	if (bBuf[5]==0x00 && bBuf[7]==0x00)
	{
		bBuf[5] = 0x01; bBuf[7] = 0x00;	//1�����
	}
	memset(pAcPara->wAutoDate, 0xff, sizeof(pAcPara->wAutoDate));

	pAcPara->wAutoDate[0] = (WORD )(bBuf[5])*0x100 + bBuf[7];//(WORD )BcdToByte(bBuf[1])*0x100 + BcdToByte(bBuf[0]);
	pAcPara->wDayFrzTime = 0xffff;  //�ս���ʱ��,Сʱ,BIN,0xffff��ʾ������,								   
	
	//�Ƕȷ���							   
	ReadItemEx(BN10, PN0, 0xa015, &pAcPara->bAngleClockwise);
			   	//0xa015 1	�Ƕȷ���,0��ʾ�ǶȰ�����ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,240,120
				//					 1��ʾ�ǶȰ���˳ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,120,240
	//AcLoadAdj(pAcPara); //У����������λУ���Ƕ�
	
	pAcPara->fCalcuHarmonic = true;		//�Ƿ����г��
	pAcPara->wHarmNum = HARMONIC_NUM;	//г���������
	
	//-------���ܲ���---------------------------
	if (ReadItemEx(BN0, PN0, 0x400C, bBuf)<=0)  //����
	{
		/*
		��ʱ����(p��14) 				unsigned��
		��ʱ�α�����q��8��				unsigned��
		��ʱ����(ÿ���л���)��m��14��	unsigned��
		��������k��63�� 				unsigned��
		������������n��254��			unsigned
		*/
		//DT_STRUCT,05,
		bBuf[9] = RATE_NUM; 
	}
	pAcPara->EnergyPara.wRateNum = bBuf[9];
		
	pAcPara->EnergyPara.wPn0 = PN0;  	//Ĭ�ϲ�����
	if (wPn == PN0)	//��ѡ������Ҳ��PN0,����Ҫ�ظ�д��
		pAcPara->EnergyPara.wPn1 = 0xffff;
	else	
		pAcPara->EnergyPara.wPn1 = wPn;
		
	pAcPara->EnergyPara.wRate = 1;   	//��ǰ�ķ���,�ɼ�ʱˢ��
	pAcPara->EnergyPara.dwConst= pAcPara->dwConst; 	//���峣��
	pAcPara->EnergyPara.wEpFrac = 4;	//�й����ܵ�С��λ��
	pAcPara->EnergyPara.wEqFrac = 4;//2;	//�޹����ܵ�С��λ��
#ifdef ACLOG_ENABLE
	pAcPara->EnergyPara.fEnableLog = true; //֧������д������
#else
	pAcPara->EnergyPara.fEnableLog = false; //��֧������д������
#endif
	pAcPara->EnergyPara.wLogID = LOG_ENERGY;	 //��־�ļ�ID
	pAcPara->EnergyPara.wSignID = 0xc800;  //������ŵ��������ID,�ŵ�������������,0��ʾ���������
	pAcPara->EnergyPara.i64EpMax = EP_MAX; //�й����ܵ����ֵ
	pAcPara->EnergyPara.i64EqMax = EQ_MAX; //�޹����ܵ����ֵ

	pAcPara->EnergyPara.wLogBarID = LOG_ENERGY_BAR; //���ɵ��ܲ�����С������λ����־�ļ�ID
	pAcPara->EnergyPara.wLogBarNum = AC_ENERGY_BAR_NUM; ////���治����С������λ�����֣����浽42��

	pAcPara->EnergyPara.wTypeNum = AC_ENERGY_NUM; //wID[3][ENERGY_TYPE_MAX]��ʵ�ʵ������͵ĸ���
	pAcPara->EnergyPara.wLogNum = AC_ENG_LOG_NUM;//���籣��������͵ĸ���

	pAcPara->EnergyPara.ptCorrect = (TEnergyCorrect *)g_tEnergyCorrect;//���ڼ�����ϵ��ܳ��ֲ�ֵ������
	pAcPara->EnergyPara.wEnergyCorrectNum = sizeof(g_tEnergyCorrect)/sizeof(TEnergyCorrect);

	for (i=0; i<AC_ENERGY_NUM; i++)
	{
		pAcPara->EnergyPara.wInnerID[i] = g_wAcCurEnergyID[1][i]; //�ڲ�����ĵ���ID
		pAcPara->EnergyPara.wPlusID[i] = g_wAcCurEnergyID[2][i];  //�;���
		pAcPara->EnergyPara.wID[0][i] = g_wAcCurEnergyID[0][i];  //�߾���
		pAcPara->EnergyPara.wID[1][i] = 0;	 //����
		pAcPara->EnergyPara.wID[2][i] = 0;//g_wAcCurEnergyID[0][i] + 0x0600;	//����
		pAcPara->EnergyPara.wID[3][i] = 0;//g_wAcCurEnergyID[0][i] + 0x0a00;  	//������
		
		if (g_wAcCurEnergyID[2][i]<0x0030)//|| g_wAcCurEnergyID[0][i]==0x900f)  //����
			pAcPara->EnergyPara.fEp[i] = true;
		else
			pAcPara->EnergyPara.fEp[i] = false;
	}
	
	//------------��������----------------------
	pAcPara->DemandPara.wRateNum = pAcPara->EnergyPara.wRateNum;
	pAcPara->DemandPara.fSingleDemandId = false;
	pAcPara->DemandPara.bDemTimeLen = 7;

	//�������ڼ�����ʱ��
	WORD wDemandPeriod, wSlideInterv, wSlideNum;
	ReadItemEx(BN0, wPn, 0x4100, bBuf); //��������
	wDemandPeriod = bBuf[1];//BcdToByte(bBuf[0]);

	ReadItemEx(BN0, wPn, 0x4101, bBuf); //����ʱ��
	wSlideInterv = bBuf[1];//BcdToByte(bBuf[0]);
	//DTRACE(DB_CRITICAL,("\r\n####AcLoadPara wDemandPeriod=%d ,wSlideInterv=%d", wDemandPeriod,wSlideInterv));
	
	if (wDemandPeriod==0 || wSlideInterv==0 || 
		wDemandPeriod>60)
	{
		wDemandPeriod = 15;	//15
		wSlideInterv = 1;
	}

	//����ʽ�����������뻬��ʱ���У��
	if (wDemandPeriod!=wSlideInterv && //��������ʽ�������������������뻬��ʱ��������ó�ͬһ��ֵ
		(wDemandPeriod%wSlideInterv!=0 || wDemandPeriod/wSlideInterv<5))
	{	//�������ڱ����ǻ���ʱ���5��������(������)
		wDemandPeriod = 15;
		wSlideInterv = 1;
	}

	wSlideNum = wDemandPeriod/wSlideInterv; //һ�����������ڵĻ���ʱ��ĸ���
	
	pAcPara->DemandPara.wPn0 = PN0; //Ĭ�ϲ�����
	if (wPn == PN0)	//��ѡ������Ҳ��PN0,����Ҫ�ظ�д��
		pAcPara->DemandPara.wPn1 = 0xffff;
	else	
		pAcPara->DemandPara.wPn1 = wPn;
	
	pAcPara->DemandPara.wRate = 1;    //��ǰ�ķ���,�ɼ�ʱˢ��
	pAcPara->DemandPara.dwConst = pAcPara->dwConst; 	//���峣��
	pAcPara->DemandPara.wFrac = 4;		//����С��λ��,��׼��ʽ��645���NN.NNNN(kw/kvar),��Ϊ4
#ifdef ACLOG_ENABLE
	pAcPara->DemandPara.fEnableLog = true; //֧������д������
#else
	pAcPara->DemandPara.fEnableLog = false; //��֧������д������
#endif
	pAcPara->DemandPara.wLogID = LOG_DEMAND;	 //��־�ļ�ID
	pAcPara->DemandPara.wMeteringDay = 1;	//������
	pAcPara->DemandPara.wMeteringHour = 0;	//�����յ�Сʱ
	pAcPara->DemandPara.wSlipNum = wSlideNum;	//������,ͨ�������������/����ʱ�����,�ɼ�ʱˢ�� 15
	pAcPara->DemandPara.dwSlipInterv = wSlideInterv;	//����ʱ��,��λ���� 1
	
	pAcPara->DemandPara.wTypeNum = AC_DEMAND_NUM; //wID[3][DEMAND_TYPE_MAX]��ʵ�ʵ������͵ĸ���
	pAcPara->DemandPara.wLogNum = AC_DMD_LOG_NUM;//Ҫ���浽����ĸ���
	pAcPara->DemandPara.bDemTimeLen = DMTLEN;
	for (i=0; i<AC_DEMAND_NUM; i++)
	{
		pAcPara->DemandPara.wInnerID[i] = g_wAcCurDemandID[1][i]; //�ڲ�����ĵ���ID
		
		pAcPara->DemandPara.wDemandID[0][i] = g_wAcCurDemandID[0][i]; //����  
		pAcPara->DemandPara.wDemandID[1][i] = 0;//g_wAcCurDemandID[0][i] + 0x0600; //���� 
		pAcPara->DemandPara.wDemandID[2][i] = 0;//g_wAcCurDemandID[0][i] + 0x0a00; //������
		pAcPara->DemandPara.wDemandID[3][i] = g_wAcCurDemandID[2][i]; //��ǰʵʱ����
		
//			pAcPara->DemandPara.wTimeID[0][i] = g_wAcCurTimeID[i]; //����  
//			pAcPara->DemandPara.wTimeID[1][i] = g_wAcCurTimeID[i] + 0x0600; //���� 
//			pAcPara->DemandPara.wTimeID[2][i] = g_wAcCurTimeID[i] + 0x0a00; //������
//			pAcPara->DemandPara.wTimeID[0][i] = g_wAcCurDemandID[0][i]; //����  
//			pAcPara->DemandPara.wTimeID[1][i] = g_wAcCurDemandID[0][i] + 0x0600; //���� 
//			pAcPara->DemandPara.wTimeID[2][i] = g_wAcCurDemandID[0][i] + 0x0a00; //������
	}
	LoadDefaultCorrectPara(g_tEnergyCorrect, pAcPara);//�����ۼӵ����ݽ���У��Ĳ�������Ĭ�ϴ���
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//ʱ�η���
TDataItem m_diRatePeriod; //��������


//��645����ʱ�α���ȡ���ʣ�����֧��8����ʱ�α����������ֻ֧�ֵ�1��ʱ�α�0xc33f
bool InitRatePeriod(BYTE *pSrc, TRatePeriod* pRatePeriod, WORD wRatePeriodNum)
{
    BYTE bRateNum=0; 
    TRatePeriod* pRatePeriod0 = pRatePeriod;


	//ʵ��ʱ����
	DTRACE(DB_AC, ("InitRatePeriod:wRatePeriodNum=%d.\r\n",wRatePeriodNum));
	bRateNum = wRatePeriodNum;
	if (bRateNum>RATE_PERIOD_NUM || bRateNum==0)
		bRateNum = RATE_PERIOD_NUM;
	DTRACE(DB_AC, ("InitRatePeriod pSrc[0]=%d.pSrc[1]=%d.\r\n",pSrc[0],pSrc[1]));
	/*
//			0x01,MAX_DAY_CHART_NUM,//�����ʱ�����ݶ�Ϊ8
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	*/

	BYTE* pbBuf = pSrc+2;//0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
	for (WORD i=0; i<bRateNum; i++)
	{
		DTRACE(DB_AC, ("InitRatePeriod 0: i=%d pbBuf[0]=%d.pbBuf[1]=%d.\r\n",i,pbBuf[0],pbBuf[1]));
		pbBuf += 2;//0x02,0x03
		DTRACE(DB_AC, ("InitRatePeriod 1: i=%d pbBuf[1]=%d.pbBuf[3]=%d.\r\n",i,pbBuf[1],pbBuf[3]));
		pRatePeriod->dwStartTime = pbBuf[1]*100+pbBuf[3];
		pRatePeriod->wRate = pbBuf[5];
		DTRACE(DB_AC, ("InitRatePeriod 2: i=%d pbBuf[5]=%d.\r\n",i,pbBuf[5]));
		if (pRatePeriod->wRate > RATE_NUM)
			pRatePeriod->wRate = 1;
		pbBuf += 6;
		pRatePeriod++;
	}
	
	pRatePeriod = pRatePeriod0;
	WORD i,j,k,t;
	for (i=0; i<bRateNum-1; i++)
	{
	    if (!pRatePeriod[i].wRate)
	    	break;

	    k = i;
	    for (j=i+1; j<bRateNum; j++)
	    {
	        if (!pRatePeriod[j].wRate)
	        	break;
	    	if (pRatePeriod[j].dwStartTime < pRatePeriod[k].dwStartTime)
	    		k = j;
	    }
	    if (k != i)
	    {
	    	t = pRatePeriod[i].dwStartTime;
	    	pRatePeriod[i].dwStartTime = pRatePeriod[k].dwStartTime;
	    	pRatePeriod[k].dwStartTime = t;

	    	t = pRatePeriod[i].wRate;
	    	pRatePeriod[i].wRate = pRatePeriod[k].wRate;
	    	pRatePeriod[k].wRate = t;	    		    	
	    }
	}
	return true;
}



bool LoadTOU(WORD wPoint, TTOU* pTOU)
{
	WORD i,wDI;
	BYTE* p;
	BYTE bBuf[550];
	TTime now,tTime;
	GetCurTime(&now);
	
	memset(pTOU, 0, sizeof(TTOU));
	
	//-------���ܲ���---------------------------
	if (ReadItemEx(BN0, PN0, 0x400C, bBuf))  //����
	{
		/*
		��ʱ����(p��14) 				unsigned��
		��ʱ�α�����q��8��				unsigned��
		��ʱ����(ÿ���л���)��m��14��	unsigned��
		��������k��63�� 				unsigned��
		������������n��254��			unsigned
		*/
		//DT_STRUCT,05,
		//��ʱ����
		pTOU->wZoneNum = bBuf[3];//BcdToByte(bBuf[0]);	  
		if (pTOU->wZoneNum > MAX_ZONE_NUM)
			pTOU->wZoneNum = MAX_ZONE_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wZoneNum=%d.\r\n",pTOU->wZoneNum));
		
		//��ʱ�α���
		pTOU->wDayChartNum = bBuf[5];//BcdToByte(bBuf[1]);	  
		if (pTOU->wDayChartNum > MAX_DAY_CHART_NUM)
			pTOU->wDayChartNum = MAX_DAY_CHART_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wDayChartNum=%d.\r\n",pTOU->wDayChartNum));
		
		//��ʱ����
		pTOU->wPeriodNum = bBuf[7];//BcdToByte(bBuf[2]);
		if (pTOU->wPeriodNum>RATE_PERIOD_NUM || pTOU->wPeriodNum==0)
			pTOU->wPeriodNum = RATE_PERIOD_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wPeriodNum=%d.\r\n",pTOU->wPeriodNum));
		
		//������
		pTOU->wRateNum = bBuf[9];//RATE_NUM;
		if (pTOU->wRateNum>RATE_NUM || pTOU->wRateNum==0)
			pTOU->wRateNum = RATE_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wRateNum=%d.\r\n",pTOU->wRateNum));

		//����������
		pTOU->wHolidayNum = bBuf[11];//BcdToDWORD(bBuf, 2);
		if (pTOU->wHolidayNum > MAX_HOLIDAY_NUM)
			pTOU->wHolidayNum = MAX_HOLIDAY_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wHolidayNum=%d.\r\n",pTOU->wHolidayNum));
	}


	//�������ձ�
	if (ReadItemEx(BN0, PN0, 0x4011, bBuf)) 
	{
		p = bBuf+2;//01,0x14
		for (i=0; i<pTOU->wHolidayNum; i++) 
		{
			p += 3;//0x02,0x02,DT_DATE
			pTOU->zHoliday[i].nYear = ((WORD )p[0]<<8) | p[1];
			pTOU->zHoliday[i].nMonth = p[2];
			pTOU->zHoliday[i].nDay =  p[3];
			pTOU->zHoliday[i].nWeek =  p[4];
			//p[5] DT_UNSIGN
			pTOU->zHoliday[i].wDayChart = p[6];
			p += 7;
		}
	}
	

	//������״̬��
	if (ReadItemEx(BN0, PN0, 0x4012, bBuf) != 3)
		goto fail;
	pTOU->bRestStatus = bBuf[2];
	
	//�����ղ��õ���ʱ�α��		
	if (ReadItemEx(BN0, PN0, 0x4013, bBuf) != 2)
		goto fail;

	pTOU->wRestDayChart = bBuf[1];

	//����ʱ�����л�ʱ��
	if (ReadItemEx(BN0, PN0, 0x4008, bBuf)==8)  
	{
		memcpy(pTOU->bTimeZoneSwitchTime, bBuf+1, 7);
	}
	OoDateTimeSToTime(bBuf+1,&tTime);
	tTime.nSecond = 0;
	if(IsInvalidTime(tTime)==false)
	{
		if(MinutesFrom2000(now)>=MinutesFrom2000(tTime))
		{//��Ҫ�л�
			DTRACE(DB_AC, ("LoadTOU bTimeZoneSwitchTime.\n"));
			//�л�������
			if (ReadItemEx(BN0, PN0, 0x4015, bBuf))
			{
				WriteItemEx(BN0, PN0, 0x4014, bBuf);
			}
			//�л���־
			bBuf[0] = 0;
			ReadItemEx(BN4, PN0, 0xc900, bBuf);
			bBuf[0] |= 0x01;
			WriteItemEx(BN4, PN0, 0xc900, bBuf);
			memset(bBuf,0,8);
			bBuf[0] = DT_DATE_TIME_S;
			WriteItemEx(BN0, PN0, 0x4008, bBuf);
		}
	}


	//��ǰ����ʱ����
	if (ReadItemEx(BN0, PN0, 0x4014, bBuf))
	{
		p = bBuf+2;//01,0x0e
		for (i=0; i<pTOU->wZoneNum; i++) 
		{
			p += 2;//0x02,0x03
			pTOU->zZone[i].wDayChart = p[1];
			pTOU->zZone[i].nDay =  p[3];
			pTOU->zZone[i].nMonth =  p[5];
			p += 6;
		}
	}


	//������ʱ���л�ʱ��
	if (ReadItemEx(BN0, PN0, 0x4009, bBuf)==8)  
	{
		memcpy(pTOU->bDayChartSwitchTime, bBuf+1, 7);
	}
	OoDateTimeSToTime(bBuf+1,&tTime);
	tTime.nSecond = 0;
	if(IsInvalidTime(tTime)==false)
	{
		if(MinutesFrom2000(now)>=MinutesFrom2000(tTime))
		{//��Ҫ�л�
			DTRACE(DB_AC, ("LoadTOU bDayChartSwitchTime.\n"));
			//�л�������
			ReadItemEx(BN0, PN0, 0x4017, bBuf);
			WriteItemEx(BN0, PN0, 0x4016, bBuf);

			//�л���־
			bBuf[0] = 0;
			ReadItemEx(BN4, PN0, 0xc900, bBuf);
			bBuf[0] |= 0x02;
			WriteItemEx(BN4, PN0, 0xc900, bBuf);
			
			memset(bBuf,0,8);
			bBuf[0] = DT_DATE_TIME_S;
			WriteItemEx(BN0, PN0, 0x4009, bBuf);
		}
	}

//		DTRACE(DB_AC, ("LoadTOU 10.\n"));
	//��ǰ����ʱ�α�
	if (ReadItemEx(BN0, PN0, 0x4016, bBuf)<=0)
	{
		goto fail;
	}
	
	/*
	0x01,MAX_DAY_CHART_NUM,//�����ʱ�����ݶ�Ϊ8
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	*/
	p = bBuf+2;//0x01,MAX_DAY_CHART_NUM,//�����ʱ�����ݶ�Ϊ8
	for (i=0; i<pTOU->wDayChartNum; i++) 
	{
		if (InitRatePeriod(p, &pTOU->rpDayChart[i][0],pTOU->wPeriodNum) == false)
		{
			goto fail;
		}
		p += 66;
	}
	DTRACE(DB_AC, ("LoadTOU true.\n"));
	return true;

fail:
	DTRACE(DB_AC, ("LoadTOU fail.\n"));
	memset(pTOU, 0, sizeof(TTOU));
	return false;
}




WORD GetRate(TRatePeriod* pRatePeriod, WORD wPeriodNum)
{
	TTime now;
	GetCurTime(&now);
	DWORD dwTime = now.nHour*100 + now.nMinute;	
	WORD i;
	DTRACE(DB_AC, ("GetRate wPeriodNum=%d.\r\n",wPeriodNum));
	for (i=0; i<wPeriodNum; i++)
	{
		DTRACE(DB_AC, ("GetRate pRatePeriod[%d].dwStartTime=%d.\r\n",i,pRatePeriod[i].dwStartTime));
		DTRACE(DB_AC, ("GetRate pRatePeriod[%d].wRate=%d.\r\n",i,pRatePeriod[i].wRate));
		if (pRatePeriod[i].dwStartTime>dwTime || 
			(pRatePeriod[i].dwStartTime==0 && pRatePeriod[i].wRate==0))
		{
			break;
		}
	}

	WORD wRate;
	if (i == 0)   //�ȵ�һ������        //����Ҫ�����ʱ�οɿ������ã�
	{                                  // ����ǰʱ��ȵ�һ��ʱ�λ�����ȡ���һ������
	    if (wPeriodNum < 1)
		{
	    	wRate = 3;	//1		Ĭ��Ϊƽ����
		}
	    else
		{
			  //wRate = pRatePeriod[wPeriodNum-1].wRate;
			wRate = 0;
			for (WORD k=wPeriodNum; k>0; k--)
			{
				WORD wLastValidRate = pRatePeriod[k-1].wRate;	//�Ӻ�����ǰ����Ч�ķ���
				if (wLastValidRate>=1 && wLastValidRate<=RATE_NUM)
				{
					wRate = wLastValidRate;
					break;
				}
			}
		}
	}	
	else
		wRate = pRatePeriod[i-1].wRate;  //���һ�������ʵ�ʱ�εĿ�ʼʱ��ȵ�ǰʱ�仹��
										 //��ȡ��ʱ�ε�ǰһ��ʱ��
		
	if (wRate<1 || wRate>RATE_NUM)
		wRate = 3;	//1		Ĭ��Ϊƽ����
		
	BYTE bTmp[10];
	bTmp[0] = wRate;
	WriteItemEx(BANK2, POINT0, 0x1042, bTmp);
	DTRACE(DB_AC, ("GetRate::wRate=%d.\n", wRate));

	return wRate;
}


//����:��645Э��涨������ʱ�η��ʱ��и��ݵ�ǰʱ��ѡ��ǰ���ʺ�
WORD GetRate(TTOU* pTOU)
{
	WORD i;
	WORD wDayChart = 0; //��ʱ�α��,0��ʾ��Ч
	TTime now;
	GetCurTime(&now);
	
	//ѡ����ʱ�α��,˳����:��������->������->��ʱ��
	
	//���ȴӹ������ձ�ѡ��
	for (i=0; i<pTOU->wHolidayNum; i++)
	{	
		if (now.nMonth==pTOU->zHoliday[i].nMonth &&
			now.nDay==pTOU->zHoliday[i].nDay && now.nYear==pTOU->zHoliday[i].nYear)
		{
			wDayChart = pTOU->zHoliday[i].wDayChart;
			break;
		}
	}
	DTRACE(DB_AC, ("GetRate wDayChart0=%d.\r\n",wDayChart));
	
	if (wDayChart == 0)  //��ûȷ����ʱ�α��,�����������ѡ��
	{
		if ((pTOU->bRestStatus & (1<<now.nWeek-1)) == 0)  //��Ϣ
		{
			wDayChart = pTOU->wRestDayChart;  //�����ղ��õ���ʱ�α��
		}
	}
	DTRACE(DB_AC, ("GetRate wDayChart1=%d.\r\n",wDayChart));
	if (wDayChart == 0)  //��ûȷ����ʱ�α��,�����ʱ����ѡ��
	{
		DWORD dwNowday = now.nMonth*31 + now.nDay;
		for (i=0; i<pTOU->wZoneNum; i++)
		{	
			if (dwNowday < pTOU->zZone[i].nMonth*31+pTOU->zZone[i].nDay)
			{
				break;
			}
		}
		
		if (i != 0)
		{
			wDayChart = pTOU->zZone[i-1].wDayChart;
		}
	}
	DTRACE(DB_AC, ("GetRate wDayChart2=%d.\r\n",wDayChart));

	if (wDayChart > pTOU->wDayChartNum)
		wDayChart = 0;
	DTRACE(DB_AC, ("GetRate wDayChart3=%d.\r\n",wDayChart));
		
	if (wDayChart != 0)  
	{
		wDayChart--;  //����Ϊ������±�
	}
	DTRACE(DB_AC, ("GetRate wDayChart4=%d.\r\n",wDayChart));
	//else //��ѡ������ʱ�α�ŵ������,ȡ��һ��ʱ�α��
	BYTE bTmp[10];
	bTmp[0] = wDayChart;
	WriteItemEx(BANK2, POINT0, 0x1043, bTmp);
	
	DTRACE(DB_AC, ("GetRate wDayChart5=%d.\r\n",wDayChart));
	return GetRate(pTOU->rpDayChart[wDayChart], pTOU->wPeriodNum);
}

//����:ȡ�ò�����wPn�ĵ�ǰ���ʺ�
//��ע:���ڹ���Ľ��ɺ�������˵,���в����㶼ȡF21,�����������ֲ�����
WORD GetRate(WORD wPn)
{
	DTRACE(DB_AC, ("GetRate:: wPn = %d, AcPn=%d.\n", wPn, GetAcPn()));
//	    if (wPn == PN0 || wPn == GetAcPn())	//���ɲ�����
    {
    	TTOU  m_Tou;
    	DTRACE(DB_AC, ("acpn:: wPn = %d.\n", wPn));
        memset((BYTE* )&m_Tou, 0, sizeof(TTOU));
        //InitRatePeriod(wPn);
        LoadTOU(PN0, &m_Tou);	//645����ֻ���ڲ�����0
    	return GetRate(&m_Tou);
    }

//		DTRACE(DB_AC, ("not ac pn!!!\n"));
//		BYTE bRate;
//		TTime now;
//		BYTE bBuf[50];
//		ReadItem(m_diRatePeriod, bBuf);
//		
//		GetCurTime(&now);
//		WORD wIdx = ((WORD )now.nHour*60 + now.nMinute) / 30;
//		if (wIdx >= 48)	//ʱ�����
//			return 1;	//���ط���1
//	
//	#ifdef PRO_698
//		bRate = bBuf[wIdx];
//	#else
//		bRate = bBuf[wIdx>>1];
//		if (wIdx & 1)
//			bRate >>= 4;
//			
//		bRate &= 0x0f;
//	#endif
//			
//		if (bRate >= 4)	//���֧��4����
//			return 1;
//		else
//			return bRate+1;
}

//����:��ʼ��������wPn��ʱ�η���
//��ע:���ڹ���Ľ��ɺ�������˵,���в����㶼ȡF21,ֻ��Ҫ��ʼ��һ��
bool InitRatePeriod(WORD wPn)
{
//		m_diRatePeriod = GetItemEx(BN0, PN0, 0x015f);	//F21 �ն˵���������ʱ�κͷ�����
	return true;
}
#ifdef SYS_LINUX
void SaveAttPara(TAttPara* pAcPara)
{
	BYTE bBuf[100];
	BYTE* pbBuf = bBuf;

	ReadItemEx(BN25, PN0, 0x500f, bBuf);

	DWORDToBCD(pAcPara->dwUn*10, pbBuf, 3);
	pbBuf += 3;
	
	DWORDToBCD(pAcPara->dwIn, pbBuf, 3);
	pbBuf += 3;

	*pbBuf = pAcPara->bConnectType;
	pbBuf++;
	
	DWORDToBCD(pAcPara->dwConst, pbBuf, 3);
	pbBuf += 3;

	WriteItemEx(BN25, PN0, 0x500f, bBuf);	//����BN25 0x500f

	bBuf[0] = DT_UNSIGN;
	if(pAcPara->bConnectType==3)
	{
		bBuf[1] = 2;//��������
	}
	else
	{
		bBuf[1] = 3;//��������
	}
	WriteItemEx(BN0, PN0, 0x4010, bBuf);	
	TrigerSaveBank(BN25, 0, -1);
	DoTrigerSaveBank();
}

void LoadAttPara(WORD wPn, TAttPara* pAcPara)
{
	BYTE bBuf[100];
	BYTE bZeroBuf[10];
	
	memset(bBuf, 0, sizeof(bBuf));
	memset(bZeroBuf, 0, sizeof(bZeroBuf));

	//----------���ѹ,����,���߷�ʽ,���峣��---------
	int iLen = ReadItemEx(BN25, PN0, 0x500f, bBuf);
	if (iLen > 0)
	{
		if (memcmp(bBuf, bZeroBuf, 10) == 0)	//BN25��չ����500fû��,��ȡ����3101
		{
		}
		else
		{
			pAcPara->dwUn = BcdToDWORD(bBuf, 3)/10; //NNNN.NN
			pAcPara->dwIn = BcdToDWORD(bBuf+3, 3); //NNN.NNN

			if (bBuf[6] == CONNECT_3P3W)
				pAcPara->bConnectType = CONNECT_3P3W;
			else if (bBuf[6] == CONNECT_1P)
				pAcPara->bConnectType = CONNECT_1P;
			else
				pAcPara->bConnectType = CONNECT_3P4W;

			pAcPara->dwConst = BcdToDWORD(bBuf+7, 3);
		}
	}

	/*if (iLen <=0 )	//����500fû����,��ȡF25
	{
		ReadItemEx(BN0, wPn, 0x019f, bBuf);
		pAcPara->dwUn =  Fmt7ToVal(bBuf+4, 2); //���ѹֵ
		pAcPara->dwIn =  Fmt22ToVal(bBuf+6, 1) * 100; //�����ֵ
		pAcPara->bConnectType = GetConnectType(wPn);
		if (ReadItemEx(BN_645_PARA, MTRPN, 0xc030, bBuf) > 0)  //0xa010 3 ���峣��,BCD��
			pAcPara->dwConst = BcdToDWORD(bBuf, 3);
		else
			pAcPara->dwConst = 6400;
		ReadItemEx(BN3, PN0, 0x30c0, bBuf);
		WriteItemEx(BN25, PN0, 0x5005, bBuf);

		SaveAttPara(pAcPara);
	}*/

	if (pAcPara->dwUn == 0)
		pAcPara->dwUn = 2200;	//���ѹ,��ʽNNNNN.N
	if (pAcPara->dwIn == 0)
		pAcPara->dwIn = 5000;	//�����,��ʽNNN.NNN
	if (pAcPara->bConnectType==0 || pAcPara->bConnectType > 4)
		pAcPara->bConnectType=CONNECT_3P4W;
	if (pAcPara->dwConst == 0)
		pAcPara->dwConst = 6400;

	//�й������ۼ�ģʽ
	if (ReadItemEx(BN10, PN0, 0xa122, bBuf) <= 0)  //�й����ܼ�����ʽѡ��
		bBuf[0] = 0;

				//		D2			|		D1			|			D0			|
				// �����������		| 	���򲻼���		| 		�����ۼ�ģʽ	|
				// 0:������;1����	| 0:����;1������	| 0:������;1����ֵ��	|
				// D2D1=00:��->��,��->��;������ֱ����
				// D2D1=01:��->��;		ֻ������,���򲻼�
				// D2D1=10:����->��,��->��;
				// D2D1=11:����->��		���򲻼�

	pAcPara->bEpMode = bBuf[0];
}

void SaveAdjPara(BYTE *pbBuf)
{
	WriteItemEx(BN25, PN0, 0x5005, pbBuf);  //0x5005 72 ATT7022У������
	TrigerSaveBank(BN25, 0, -1);
}

void LoadAdjPara(BYTE *pbBuf)
{
	ReadItemEx(BN25, PN0, 0x5005, pbBuf);  //0x5005 72 ATT7022У������
}

int LoadAdjParak(DWORD *dwK) //�Ŵ���100000��
{
    BYTE bBuf[4];
    if (ReadItemEx(BN25, PN0, 0x5023, bBuf) > 0)
    {
        *dwK = bBuf[0]|(bBuf[1]<<8)|(bBuf[2]<<16)|(bBuf[3]<<24);
        return 4;
    }
    return -1;
}

void SaveNewAdjPara(BYTE* pbBuf)
{
	WriteItemEx(BN28, PN0, 0x001f, pbBuf);	//����BN3 0x503f
	TrigerSaveBank(BN28, 0, -1);
	DoTrigerSaveBank();
}

void LoadNewAdjPara(BYTE *pbBuf)
{
	ReadItemEx(BN28, PN0, 0x001f, pbBuf);  //0x503f 
}


#endif
