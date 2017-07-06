/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcHook.cpp
 * ժ    Ҫ�����ļ���Ҫ�������彻�ɿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *********************************************************************************************************/
#include "AcHook.h"
#include "DbAPI.h"
//#include "Sample.h"
#include "AcSample.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "DbOIAPI.h"

//����:���ɿ��ڷ�����������Ļص�����,һ�������������������¼
//	   �ṩ������Ϊ���ݵĲ�����,�Բ�����0xffff�Ĳ�������д���
//����:@wPn0 ���ɵĲ�����0,���������0xffff����д���
//	   @wPn1 ���ɵĲ�����1,���������0xffff����д���
void AcOnClrDemand(WORD wPn0, WORD wPn1)
{
	
}

//����:�ڽ���ִ���ն���ʱ�Ļص�����,ֻ�ṩ���ɲ�����,Ĭ�ϲ������ɱ���������
//	   �費��ҪҲͬʱִ��
//����:@wPn ���ɵĲ�����,���֧��˫������,��wPnָ��Ӧ���Ƿ�Ĭ�ϲ�����,
//			�����ⲿû�����ý��ɲ������ʱ��,wPnҲ���ܱ���ΪĬ�ϲ�����(��PN0)
//			��֧��˫������������,��������жϵ�wPnΪ��Ĭ�ϲ�����,��ִ��
//			wPn����Ӧ������ͬʱ,ҲӦ��ΪĬ�ϲ�����ִ����Ӧ����
//	   @time �ն����ʱ��,ֻʹ���ꡢ�¡��ա�ʱ�����ֶ�	
void AcOnDayFrz(WORD wPn, const TTime& time)
{
	DWORD dwTmp = DaysFrom2000(time);
	WriteItemEx(BN18, wPn, 0x0610, (BYTE *)&dwTmp);	//���һ���ն���ʱ��
	TrigerSaveBank(BN18, 0, -1); 
}

//����:���ɿ������ж�ĳ���ն����Ƿ��Ѿ�����Ļص�����,ֻ�ṩ���ɲ�����,Ĭ�ϲ������ɱ���������
//	   �費��ҪҲͬʱִ��
//����:@wPn ���ɵĲ�����,���֧��˫������,��wPnָ��Ӧ���Ƿ�Ĭ�ϲ�����,
//			�����ⲿû�����ý��ɲ������ʱ��,wPnҲ���ܱ���ΪĬ�ϲ�����(��PN0)
//			��֧��˫������������,��������жϵ�wPnΪ��Ĭ�ϲ�����,��ִ��
//			wPn����Ӧ������ͬʱ,ҲӦ��ΪĬ�ϲ�����ִ����Ӧ����
//	   @time �ն����ʱ��,ֻʹ���ꡢ�¡��ա�ʱ�����ֶ�	
bool AcIsDayFrozen(WORD wPn, const TTime& time)
{
	DWORD dwTmp;
	ReadItemEx(BN18, wPn, 0x0610, (BYTE *)&dwTmp);	//���һ���ն���ʱ��
	return (dwTmp == DaysFrom2000(time));
}

//����:�ڽ���ִ���¶���ʱ�Ļص�����,ֻ�ṩ���ɲ�����,Ĭ�ϲ������ɱ���������
//	   �費��ҪҲͬʱִ��
//����:@wPn ���ɵĲ�����,���֧��˫������,��wPnָ��Ӧ���Ƿ�Ĭ�ϲ�����,
//			�����ⲿû�����ý��ɲ������ʱ��,wPnҲ���ܱ���ΪĬ�ϲ�����(��PN0)
//			��֧��˫������������,��������жϵ�wPnΪ��Ĭ�ϲ�����,��ִ��
//			wPn����Ӧ������ͬʱ,ҲӦ��ΪĬ�ϲ�����ִ����Ӧ����
//	   @time �ն����ʱ��,ֻʹ���ꡢ�¡��ա�ʱ�����ֶ�	
void AcOnMonthFrz(WORD wPn, const TTime& time, BYTE bIdx)
{
	DWORD dwTmp = MonthFrom2000(time);
	WriteItemEx(BN18, wPn, 0x0611+bIdx, (BYTE *)&dwTmp); 	//���һ���¶���ʱ��
	TrigerSaveBank(BN18, 0, -1); 
}

//����:���ɿ������ж�ĳ���¶����Ƿ��Ѿ�����Ļص�����,ֻ�ṩ���ɲ�����,Ĭ�ϲ������ɱ���������
//	   �費��ҪҲͬʱִ��
//����:@wPn ���ɵĲ�����,���֧��˫������,��wPnָ��Ӧ���Ƿ�Ĭ�ϲ�����,
//			�����ⲿû�����ý��ɲ������ʱ��,wPnҲ���ܱ���ΪĬ�ϲ�����(��PN0)
//			��֧��˫������������,��������жϵ�wPnΪ��Ĭ�ϲ�����,��ִ��
//			wPn����Ӧ������ͬʱ,ҲӦ��ΪĬ�ϲ�����ִ����Ӧ����
//	   @time �ն����ʱ��,ֻʹ���ꡢ�¡��ա�ʱ�����ֶ�	
bool AcIsMonthFrozen(WORD wPn, const TTime& time, BYTE bIdx)
{
	DWORD dwTmp;
	ReadItemEx(BN18, wPn, 0x0611+bIdx, (BYTE *)&dwTmp);	//���һ���¶���ʱ��
	return (dwTmp == MonthFrom2000(time));
}

//����:���ɿ���������������������ݵĻص�����,ֻ�ṩ���ɲ�����,Ĭ�ϲ������ɱ���������
//	   �費��ҪҲͬʱִ��
//����:@wPn ���ɵĲ�����,���֧��˫������,��wPnָ��Ӧ���Ƿ�Ĭ�ϲ�����,
//			�����ⲿû�����ý��ɲ������ʱ��,wPnҲ���ܱ���ΪĬ�ϲ�����(��PN0)
//			��֧��˫������������,��������жϵ�wPnΪ��Ĭ�ϲ�����,��ִ��
//			wPn����Ӧ������ͬʱ,ҲӦ��ΪĬ�ϲ�����ִ����Ӧ����
//	   @time �ն����ʱ��,ֻʹ���ꡢ�¡��ա�ʱ�����ֶ�	
void AcTrigerSavePn(WORD wPn)
{
	if (wPn == PN0)
	{
		TrigerSaveBank(BN0, SECT_ENERGY, PN0);	//��������
		TrigerSaveBank(BN0, SECT_DEMAND, PN0);	//��������
		TrigerSaveBank(BN0, SECT_VARIABLE, PN0);	//��������
	}
	else
	{
		TrigerSaveBank(BN0, SECT_ENERGY, PN0);	//��������
		TrigerSaveBank(BN0, SECT_DEMAND, PN0);	//��������
		TrigerSaveBank(BN0, SECT_VARIABLE, PN0);	//��������
		TrigerSaveBank(BN0, SECT_ENERGY, wPn);	//��������
		TrigerSaveBank(BN0, SECT_DEMAND, wPn);	//��������
		TrigerSaveBank(BN0, SECT_VARIABLE, wPn);	//��������
	}
}


//����:���ɿ���������������������ݵĻص�����,�ṩ������Ϊ���ݵĲ�����,�Բ�����0xffff
//	   �Ĳ�������д�������
//����:@wPn0 ���ɵĲ�����0,���������0xffff����д�������
//	   @wPn1 ���ɵĲ�����1,���������0xffff����д�������
void AcTrigerSavePn(WORD wPn0, WORD wPn1)
{
	if (wPn0 != 0xffff)
	{
		TrigerSaveBank(BN0, SECT_ENERGY, wPn0);	//��������
		TrigerSaveBank(BN0, SECT_DEMAND, wPn0);	//��������
		TrigerSaveBank(BN0, SECT_VARIABLE, wPn0);	//��������
	}
	
	if (wPn1 != 0xffff)
	{
		TrigerSaveBank(BN0, SECT_ENERGY, wPn1);	//��������
		TrigerSaveBank(BN0, SECT_DEMAND, wPn1);	//��������
		TrigerSaveBank(BN0, SECT_VARIABLE, wPn1);	//��������
	}
}


//����:@bType =0����е�ѹ�ж�, =1����е����ж�
bool CaluDisOrder34(const int* piAngle)
{	//3��4�ߵ�ѹ�͵���������
	DWORD iAngle[3];
	iAngle[0] = (piAngle[0] + 3600 - piAngle[1]) % 3600;
	iAngle[1] = (piAngle[1] + 3600 - piAngle[2]) % 3600;
	iAngle[2] = (piAngle[2] + 3600 - piAngle[0]) % 3600;
	
    if ((iAngle[0]>=2400-50 && iAngle[0]<=2400+50) ||  
    	(iAngle[1]>=2400-50 && iAngle[1]<=2400+50) ||  
    	(iAngle[2]>=2400-50 && iAngle[2]<=2400+50))
	{
    	return true;
	}
	else
	{
		return false;
	}
}


bool CaluDisOrderU33(const int* piAngle)
{
	int iAngle  = (short)(piAngle[0] + 3600 - piAngle[2]) % 3600;
   	if ((iAngle>550 && iAngle<650) || iAngle>3580 || iAngle<50)   //�������ʱ��Ϊ60��
   		return true;
	else
		return false;
}


bool CaluDisOrderI33(const int* piAngle)
{
	int iAngle  = (short)(piAngle[0] + 3600 - piAngle[2]) % 3600;
   	if (iAngle>1200-50 && iAngle<1200+50)  //fAngle<120+20 || fAngle>120-20
   		return true;
	else
		return false;
}


BYTE CalCurPolar(const int* piAngle, BYTE bConnectType, BYTE bLastPolar)
{
	const WORD wAngleMin34[3] = {900, 900, 900}; 
	const WORD wAngleMax34[3] = {2700, 2700, 2700};
	const WORD wAngleMin33[3] = {1200, 2700, 600}; 
	const WORD wAngleMax33[3] = {3000, 2700, 2400};
	const WORD* pwAngleMin;
	const WORD* pwAngleMax;
	
	BYTE bPolar = bLastPolar;
   	
   	if (bConnectType == CONNECT_3P4W)
   	{
		pwAngleMin = wAngleMin34;
		pwAngleMax = wAngleMax34;
   	}
   	else
   	{
   		//�������ߵ��ж�������:  90+45 < Uab-Ia < 270-45
   		//						 90+15 < Ucb-Ic < 180+15
		pwAngleMin = wAngleMin33;
		pwAngleMax = wAngleMax33;
   	}
   	
	for (WORD i=0; i<3; i++)
	{
		int iAngle = (piAngle[i]  + 3600 - piAngle[i+3]) % 3600;  //��ѹ���������
  		if (iAngle>pwAngleMin[i] && iAngle<pwAngleMax[i])	
		{	
   			bPolar |= 0x01<<i;
		}
		else if (iAngle<=pwAngleMin[i]-1 || iAngle>=pwAngleMax[i]+1)	//��һ�Ȳ��ûָ�
		{
			bPolar &= ~(0x01<<i);
		}
	}
   	
   	if (bConnectType == CONNECT_3P3W)
   	{
   		bPolar &= ~0x02;
   	}
   	return bPolar;
}


//����:�����������йص��쳣,
//����:@pbPhaseStatus ��������ԭ����ֵ,���ҷ����µ�ֵ
//��ע:����״̬����
//	   D7	D6	D5			D4			D3			D2			D1			D0
//						Ic������	Ib������	Ia������	����������	��ѹ������	
void AcCaluPhaseStatus(const TAcPara& rAcPara, const int* piValue, const int* piAngle, const int* piCos, BYTE* pbPhaseStatus, WORD* pwPnStatus)
{
	DWORD dwNormU = rAcPara.dwUn * 7 / 10;
	DWORD dwNormI = rAcPara.dwIn * 5 / 100;
	bool fDisOrderU = (*pbPhaseStatus & DISORDER_U) != 0;
	bool fDisOrderI = (*pbPhaseStatus & DISORDER_I) != 0; 
	BYTE bCurPolar = (*pbPhaseStatus>>2) & 0x07;
	
 	if (rAcPara.bConnectType == CONNECT_3P4W)
  	{
		if (piValue[0]>dwNormU && piValue[1]>dwNormU && piValue[2]>dwNormU)  //�������ж�ֻ���ڵ�ѹ�ϸ�ʱ���ж�
		{
			fDisOrderU = CaluDisOrder34(piAngle);
			if (fDisOrderU)
				goto end;
			
			if (piValue[3]>dwNormI && piValue[4]>dwNormI && piValue[5]>dwNormI)   //�������ж�
			{
				fDisOrderI = CaluDisOrder34(&piAngle[3]);
				if (fDisOrderI == false)
					bCurPolar = CalCurPolar(piAngle, rAcPara.bConnectType, bCurPolar);
				else
					bCurPolar = 0;
			}
		}
		//else ά��ԭ�����ж�,��Ҫ��λԭ�����жϽ��,������Ϊ��ѹ�����Ĳ������𷴸����ϱ�
  	}
  	else if (rAcPara.bConnectType == CONNECT_3P3W)
    { 
    	//��������
    	if (piValue[0]>dwNormU && piValue[2]>dwNormU)  //�������ж�ֻ���ڵ�ѹ�ϸ�ʱ���ж�
    	{
			fDisOrderU = CaluDisOrderU33(piAngle);
			if (fDisOrderU)
				goto end;
			
			if (piValue[3]>dwNormI && piValue[5]>dwNormI)   //�������ж�
			{
				fDisOrderI = CaluDisOrderI33(&piAngle[3]);
				if (fDisOrderI == false)
					bCurPolar = CalCurPolar(piAngle, rAcPara.bConnectType, bCurPolar);
				else
					bCurPolar = 0;
			}
    	}
    	//else ά��ԭ�����ж�,��Ҫ��λԭ�����жϽ��,������Ϊ��ѹ�����Ĳ������𷴸����ϱ�
    }
    
end:
	if (fDisOrderU)
    {
    	*pbPhaseStatus = DISORDER_U; //�ڷ�����ѹ�������ʱ��,����������ͷ����Ա�־λ�����
    }
    else
    {	
    	*pbPhaseStatus &= ~DISORDER_U;
    	
		if (fDisOrderI)
	    	*pbPhaseStatus |= DISORDER_I; 
	    else
	    	*pbPhaseStatus &= ~DISORDER_I;
	    	
	    *pbPhaseStatus = (*pbPhaseStatus & 0x03) | (bCurPolar << 2); //��������˵���������,bCurPolar�������Ѿ�����
	}
	
	WriteItemEx(BANK2, POINT0, 0x1120, pbPhaseStatus); //��ѹ����������
}

//����:��ѹ������ƽ��ļ��㷽��, |UImax - UImin| / UImax
//����:@rAcPara �������ݼ�������
//		@piValue �������ݵ�������
//��ע:
static WORD CalculateImbalance(const TAcPara& rAcPara, const int* piValue)
{
	DWORD	dwValueA, dwValueB, dwValueC, dwMax, dwMin;
	WORD wImbalance;

	// ��ƽ�� �����������з���
	dwValueA = labs(piValue[0]);
	dwValueB = labs(piValue[1]);
	dwValueC = labs(piValue[2]);
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwValueA =%d ", dwValueA));
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwValueB =%d ", dwValueB));
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwValueC =%d ", dwValueC));
	
	if (dwValueA > dwValueC)
	{
		dwMax = dwValueA;	dwMin = dwValueC;
	}
	else
	{
		dwMax = dwValueC;	dwMin = dwValueA;
	}
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwMax =%d ", dwMax));
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwMin =%d ", dwMin));
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance rAcPara.bConnectType =%d ", rAcPara.bConnectType));
	if (rAcPara.bConnectType == CONNECT_3P4W)		// �������߲űȽ�B��
	{
		if (dwMax < dwValueB)		dwMax = dwValueB;
		if (dwMin > dwValueB)		dwMin = dwValueB;
	}
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwMax 1 =%d ", dwMax));
	if(dwMax == 0)
	{
		wImbalance = 0;
	}
	else
	{
		wImbalance = (WORD)((float)(dwMax - dwMin) / dwMax * 10000);
		if (wImbalance > 9999)
			wImbalance = 9999;
	}
	return wImbalance;
}



//����:��ѹ������ƽ��ļ��㷽��, |UImax - UImin| / UImax
//����:@rAcPara �������ݼ�������
//		@piValue �������ݵ�������
//��ע:
void AcCaluImbalanceRate(const TAcPara& rAcPara, const int* piValue)
{
	WORD wImbalance;
	BYTE bBuf[10];
	
	// 2026 6 ��ѹ��ƽ����
	// �������ͣ�long-unsigned����λ��%�����㣺-2
	wImbalance = CalculateImbalance(rAcPara,&piValue[0]);
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[0] =%d ", piValue[0]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[1] =%d ", piValue[1]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[2] =%d ", piValue[2]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate U_Imbalance =%d ", wImbalance));
	bBuf[0] = DT_LONG_U;
	OoWordToLongUnsigned(wImbalance, &bBuf[1]);
	OoWriteAttr(0x2026, 0x02, bBuf);
//		TraceBuf(DB_CRITICAL, "\r\n####AcCaluImbalanceRate-> ", bBuf, 3); 		

	// 2027 6 ������ƽ����
	// �������ͣ�long-unsigned����λ��%�����㣺-2
	wImbalance = CalculateImbalance(rAcPara,&piValue[3]);
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[3] =%d ", piValue[3]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[4] =%d ", piValue[4]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[5] =%d ", piValue[5]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate I_Imbalance =%d ", wImbalance));

	bBuf[0] = DT_LONG_U;
	OoWordToLongUnsigned(wImbalance, &bBuf[1]);
	OoWriteAttr(0x2027, 0x02, bBuf);
//		TraceBuf(DB_CRITICAL, "\r\n####AcCaluImbalanceRate-> ", bBuf, 3); 		
	
}

//����:ת�浱ǰ����
//����:@wId ������������ID
//		@piValue �������ݴ���ʽ��������
//��ע:
void AcTransSaveCurDemand(WORD wId, BYTE* pbValue)
{
	switch(wId)
	{
		case 0x3117:
		OoWriteAttr(0x2017, 0x02, pbValue);
		break;
		case 0x3118:
		OoWriteAttr(0x2018, 0x02, pbValue);
		break;
		default:
			break;
	}	
}





//����:���㰲ʱ��
//����:@rAcPara �������ݼ�������
//		@piValue �������ݵ�������
//		@piValue �������ݲ�����0:���������ۼƣ�����ֵΪ�ۼ�
//��ע:
void AcCaluAmpereHours(const TAcPara& rAcPara, const int* piValue, BYTE bOpt)
{
	int iAHs[4];
	BYTE bBuf[30];
	int i;
	
	memset(iAHs, 0, sizeof(iAHs));
	// 2029 6 ����Сʱ��
	// �������ͣ�double-long-unsigned����λ��Ah�����㣺-2
	if(bOpt!=0)
	{
		OoReadVal(0x20290200, iAHs, 4);
		for(i=0;i<4;i++)
		{
			iAHs[i] *= 10;
			DTRACE(DB_AC,("\r\n####AcCaluAmpereHours 0 dwAHs[%d] =%d ", i, iAHs[i]));
		}
		for(i=0;i<3;i++)
		{
			iAHs[0] += labs(piValue[i]);
			iAHs[i+1] += labs(piValue[i]);
			DTRACE(DB_AC,("\r\n####AcCaluAmpereHours 1 dwAHs[0] =%d,dwAHs[%d] =%d, piValue[%d]=%d ", iAHs[0], i, iAHs[i], i, piValue[i]));
		}
		
	}
	for(i=0;i<4;i++)
	{
		if ((i==1) && (rAcPara.dwConst == CONNECT_3P3W))		// �������߲�����B��
		{
			iAHs[2] = 0;
			continue;
		}
		iAHs[i] /= 10;
		DTRACE(DB_AC,("\r\n####AcCaluAmpereHours 2 dwAHs[%d] =%d ", i, iAHs[i]));
	}
	
	//���
	bBuf[0] = DT_ARRAY;
	bBuf[1] = 4;
	for(i=0;i<4;i++)
	{
		bBuf[2+i*5] = DT_DB_LONG_U;
		OoDWordToDoubleLongUnsigned(iAHs[i],&bBuf[3+i*5]);
	}
	OoWriteAttr(0x2029, 0x02, bBuf);
	TraceBuf(DB_AC, "\r\n####AcCaluAmpereHours-> ", bBuf, 22); 	
}





//����:�趨�ɱ�������������
//����:��
void PgmPulseInit()
{
}   

//����:�����ɱ���������
//����:@wType ��������,Ϊ�������ڡ�ʱ��Ͷ��
void PgmPulseOut(WORD wType)
{
}

