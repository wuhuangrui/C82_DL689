/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbCctAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��Э���м�����ص����ݿ��׼�ӿ�֮�����չ�ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��4��
 * ��    ע��$Ϊ�˱��⼯������ϵͳ��ӿں�����DbAPI.cpp�еĽӿں���������һ��
 			  ���ⶨ�屾�ļ�
 *********************************************************************************************************/
#include "stdafx.h"
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "DbCctAPI.h"
#include "TaskConst.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//�������ӿ�

//����:��ȡ����������
//����:0��ʾ��Ч��1��ʾ���ɣ�2��ʾ���3��ʾ���壬4��ʾģ������5��ʾ����
BYTE CctGetPnProp(WORD wPn)
{
	if (wPn >= POINT_NUM)
		return INVALID_POINT;
		
	BYTE bProp[16];
	if (ReadItemEx(BN0, wPn, 0x8901, bProp) <= 0)
		return INVALID_POINT;

	return bProp[2];
}


bool IsCctPn(WORD wPn)
{
	return GetPnProp(wPn)==PN_PROP_CCT;
}

bool IsRJ45Pn(WORD wPn)
{
	return GetPnProp(wPn)==PN_PROP_RJ45;
}

bool IsBBCctPn(WORD wPn)
{
	return GetPnProp(wPn)==PN_PROP_BBCCT;
}

bool CctIsPnValid(WORD wPn) 
{ 
	return IsCctPn(wPn); 
}

//����:�Ƿ�Ϊ�����
bool IsSinglePhase(WORD wPn)
{	
	if (GetUserSubType(wPn) == 1)
		return true;
		
	return false;
}

//����:�Ƿ�Ϊ�๦�ܱ�
bool IsMultiFunc(WORD wPn)
{
	if (IsSinglePhase(wPn))
		return false;
	else
		return true;
}


//����:�Ƿ�Ϊ����ʱ�
//��ע:Ŀǰ�����϶๦�ܱ������ʱ��ǵ�ͬ�ĸ���,����Ŀǰ�Ƽ����ȶ�ʹ��IsMultiRate()��Ϊ�ж�,
// 		����ͬʱʹ�ö�����Ƶĺ������ж�ͬһ������
bool IsMultiRate(WORD wPn)
{
	BYTE bTmp[200];
	if (IsCctPn(wPn))
	{
		if (ReadItemEx(BN0, wPn, 0x8902, bTmp) >= 0)
	    {
			if ((bTmp[59]&0x3f)>1 || bTmp[59]==0) //����Ϊ0ʱ��GetPnRateNum(WORD wPn)����һ�£����������������
			    return true;
		    else
			    return false;
		}
	}

	return false;
}

//����:�Ƿ��Ǽ�����1�೭��������,��������Щ�ն��Լ����ɵļ���������,������ͨ������õ���
bool IsCctClass1MtrId(WORD wPn, WORD wID)
{
	static WORD wCctClass1MtrId[] = 
	{
		0x112f, 0x113f, 0x114f, 0x160f, 0x161f, 0x163f, 0x164f, 0x165f,//C1F25,C1F26,C1F27,C1F28 ,C1F29,C1F30,C1F31,C1F32
		0x115f, 0x116f, 0x117f, 0x118f, 0x119f,	0x11Af, 0x11Bf, 0x11Cf,	0x126f, //C1F33,C1F34,C1F35,C1F36,C1F37,C1F38,C1F39,C1F40,C1F49
		0x166f,	0x167f, 0x168f, 0x169f, 0x16af, 0x16bf, 0x16cf, 0x16df, //C1F129,C1F130,C1F131,C1F132,C1F133,C1F134,C1F135,C1F136
		0x200f, 0x201f, 0x202f, 0x203f, 0x204f, 0x205f, 0x206f, 0x207f, //C1F137,C1F138,C1F139,C1F140,C1F141,C1F142,C1F143,C1F144
		0x208f, 0x209f, 0x20af, 0x20bf, 0x20cf, 0x20df, 0x20ef, 0x210f, //C1F145,C1F146,C1F147,C1F148,C1F149,C1F150,C1F151,C1F152
	    0x220f, 0x221f, 0x222f, 0x223f, 0x224f,//C1F161,C1F165,C1F165,C1F167,C1F168
	};

	if (!IsCctPn(wPn))
		return false;

	for (WORD i=0; i<sizeof(wCctClass1MtrId)/sizeof(WORD); i++)
	{
		if (wID == wCctClass1MtrId[i])
			return true;
	}

	return false;
}


//����:ȷ���Ƿ��Ǽ�������������BANK��ID
bool IsCctDataBankId(WORD wBn, WORD wID)
{
	if (/*wBn==CCT_BN_SPM ||*/ wBn==CCT_BN_MFM)
		return true;
	else
		return false;
}

//����:���ݵ������,ȡ�ü�������������ݴ洢BANK
WORD CctGetPnBank(WORD wPn)
{
	if (IsCctPn(wPn) || IsRJ45Pn(wPn) || IsBBCctPn(wPn))
	{
		/*if (IsMultiFunc(wPn)) //��ѹ����һ�㹤��ҵ�û� �� ��ѹ��
			return CCT_BN_MFM;		//�๦�ܱ�����BANK
		else
			return CCT_BN_SPM;		//���������BANK*/

		return CCT_BN_MFM;	//Ŀǰ����BANK�洢,ͳһ���ն๦�ܱ�����,��֧��POINT_NUM��
	}

	return BN0;
}

//����:���ݲ�ͬ�������������ݷ�BANK�洢ԭ��,��ID��BANK��������ӳ��
WORD CctGetIdBank(WORD wBn, WORD wPn, WORD wID)
{
	if (IsCctDataBankId(wBn, wID))
		return CctGetPnBank(wPn);	//���ݵ������,ȡ�ü�������������ݴ洢BANK
	else
		return wBn; //Ĭ������·���ԭ����BANK��
}

void CctPostDbInit()
{
	WORD wPn;
	BYTE bRate = RATE_NUM; //������
		
	//�ѵ����ķ���������ʼ��Ϊ4,(0x8911-������,�������ID�е�����һ����ID)
	for (wPn=0; wPn<POINT_NUM; wPn++)
	{
		bRate = RATE_NUM; //������
		ReadItemEx(BN0, wPn, 0x8911, &bRate);
		WriteItemEx(CCT_BN_SPM, wPn, 0x8911, &bRate);
	}

#if PNMAP_CCTMFM==PNUNMAP	//�๦�ܱ�ķ�����,�ڲ���Ҫ�����㶯̬ӳ�������¿���ֱ�ӳ�ʼ��Ϊ4
							//����Ҫ�����㶯̬ӳ��������,Ҫ�ȵ�������ӳ������д,����ϵͳ���Ҳ���
	for (wPn=0; wPn<MFM_NUM; wPn++)
	{
		bRate = RATE_NUM; //������
		ReadItemEx(BN0, wPn, 0x8911, &bRate);
		WriteItemEx(CCT_BN_MFM, wPn, 0x8911, &bRate);
	}
#endif 

	//��ʼ������������λ
	//CctUpdPnMask();	 //�����ز��ڵ�����λ�ȽϺ�ʱ,�п��������Ź���λ
						 //�ŵ�InitCct���ʼ����Щ
}

//����:����������
WORD SearchPnFromMask(const BYTE* pbPnMask, WORD wStartPn)
{
	WORD i, j;
	i = wStartPn / 8;
	j = wStartPn % 8;
	for (; i<PN_MASK_SIZE; i++)
	{
		if (pbPnMask[i] != 0)
		{
			BYTE bBitMask = 1 << j;
			for (; j<8; j++,bBitMask<<=1)
			{
				if (pbPnMask[i] & bBitMask)
					return i*8+j;
			}
		}
		
		j = 0;
	}
	
	return POINT_NUM;
}

//����:�Ӳ���������λ���������������һ��������
//����:�����ȷ��������,���򷵻�0
WORD SchLastPnFromMask(const BYTE* pbPnMask)
{
	int i, j;
	i = (POINT_NUM-1) / 8;
	j = (POINT_NUM-1) % 8;
	for (; i>=0; i--)
	{
		if (pbPnMask[i] != 0)
		{
			for (; j>=0; j--)
			{
				BYTE bBitMask = 1 << j;
				if (pbPnMask[i] & bBitMask)
					return (WORD )(i*8+j);
			}
		}
		
		j = 7;
	}
	
	return 0;
}


//����:�Ƿ��Ѿ����ڸòɼ��ն˵Ľڵ�
//����:@pbNodeMask ɨ��������Ѿ�ɨ�赽��ͨ���ɼ��ն˵��ز��ڵ�����λ
//	   @pbAddr �ز��ն˵ĵ�ַ
bool IsAcqNodeExist(BYTE* pbNodeMask, BYTE* pbAddr)
{
	BYTE bAddr[6];
	WORD wNode = 0;
	while (1)
	{
		wNode = SearchPnFromMask(pbNodeMask, wNode);
		if (wNode >= POINT_NUM)
			return false;
		
//		if (GetPlcNodeAddr(wNode, bAddr))
		{
			if (memcmp(pbAddr, bAddr, 6) == 0)	//��ǰ�ͳ��ֹ�,���ǵ�һ��
				return true;
		}

		wNode++;
	}
	
	return false;
}

//����:�ص㻧����������λ
void UpdateVipMask()
{	
	WORD wPn, wSn;
	WORD wPos;
	BYTE bMask;
	BYTE bNum;
	BYTE bBuf[50];
	BYTE bVipMask[PN_MASK_SIZE];
	BYTE bOldMask[PN_MASK_SIZE];

	memset(bOldMask, 0, sizeof(bOldMask));
	ReadItemEx(BN17, PN0, 0x7031, bOldMask); //�ȱ����ɵ� 0x7031 �ص㻧����������λ

	memset(bVipMask, 0, sizeof(bVipMask));

	if (ReadItemEx(BANK0, PN0, 0x023f, bBuf) > 0)
	{
		WORD wPn;
		bNum = bBuf[0];
		if (bNum > 20)
			bNum = 20;

		for (WORD i=0; i<bNum; i++)
		{
			wSn = ByteToWord(&bBuf[1+2*i]);
			wPn = wSn;//MtrSnToPn(wSn); //ͨ��װ����Ż�ò�����ֵ,�����ȷ�򷵻�װ�����,���򷵻�0
//			if (!IsPlcLink(wPn))		//���ǵ����ز�����Ҳ�ڴ˴�����Ҫ�жϣ�ֻ��PLC����������ص㻧һ˵
				continue;

			if (wPn!=0 && IsCctPn(wPn))	//ֻ�м���������������ص㻧
				bVipMask[wPn/8] |= 1 << (wPn%8);
		}
	}

	WriteItemEx(BN17, PN0, 0x7031, bVipMask); //0x7031 �ص㻧����������λ

#if PNMAP_VIP!=PNUNMAP
	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		bMask = 1<<(wPn&0x07);
		wPos = wPn>>3;
		if (bVipMask[wPos] & bMask)
		{	//��Ч�Ĳ����㲻����ǰ��û����,����������һ�²�����ӳ��,������ʲô������
			
			if (NewPnMap(PNMAP_VIP, wPn) < 0)	//Ϊ���ɹ̶�ӳ��һ������PN0
			{	
				DTRACE(DB_CRITICAL, ("UpdateVipMask: fail to new vip PnMap for wPn=%d\r\n", wPn));
			}

			if ((bOldMask[wPos] & bMask) == 0) //��ǰû�����ù�,�����õ��ص㻧
				CctVipPnData(wPn);

//			UpdVipRdFlg(wPn);	//�����ڽ���ӳ���Ÿ��³�����־,�������������
		}
		else
		{
			DeletePnMap(PNMAP_VIP, wPn);	//�Ƕ๦�ܱ������Ч�Ĳ����㶼ɾ��һ��ӳ��,������й©
		}
	}
#endif 
}

//��������ʼ������������ add by CPJ at 2012-06-15
void InitPnProp()
{
    BYTE bPort = 0, bProp = PN_PROP_UNSUP, bPortFun = 0;
	for(WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		if(!IsPnValid(wPn))
			continue;

		bPort = GetPnPort(wPn);
		if(bPort>=LOGIC_PORT_MIN && bPort<=LOGIC_PORT_MAX) //ֻӰ���߼��˿�2��3��4
		{
			bPortFun = GetLogicPortFun(bPort);
			if(GetPnProp(wPn)==PN_PROP_METER && (bPortFun==PORT_FUN_ACQ||bPortFun==PORT_FUN_JC485)) //���ز���������߼��˿ڹ���Ϊ����485
		   {
               bProp = PN_PROP_CCT;
			   WriteItemEx(BN0, wPn, 0x8901, &bProp);
		   }
		   else if(GetPnProp(wPn)==PN_PROP_CCT && bPortFun==PORT_FUN_RDMTR) //�ز�����������߼��˿ڹ���Ϊ���س����
		   {
			   bProp = PN_PROP_METER;
			   WriteItemEx(BN0, wPn, 0x8901, &bProp);
		   }
		}	
	}
}


//����:�弯������������
void CctClrPnData(WORD wPn)
{
	WORD i;						
	BYTE bBuf[PN_MASK_SIZE+10];
	if (wPn >= POINT_NUM)
		return;

	DTRACE(DB_CCT, ("CctClrPnData: wPn=%d\r\n", wPn));
/*
	//��BN16(��ǰ���)��������Ĳ����㳭�ձ�־
	WORD wComTaskNum = CctGetComTaskNum();
	for (i=0; i<wComTaskNum; i++)
	{
		ReadItemEx(BN16, i, 0x6001, bBuf);  //0x6001 PN_MASK_SIZE+5 (��ǰ���)��������Ĳ����㳭�ձ�־,ÿ�������Ӧһ��������
		bBuf[wPn/8] &= ~(1<<(wPn%8));
		WriteItemEx(BN16, i, 0x6001, bBuf);
	}*/

	//��BN17�����Ǳ�������:
	const static WORD wBank17Id[] = {0x7010, 0x7011, 0x7012, 0x7013, 0x7014};
	memset(bBuf, 0, sizeof(bBuf));
	for (WORD i=0; i<sizeof(wBank17Id)/sizeof(WORD); i++)
	{
		WriteItemEx(BN17, wPn, wBank17Id[i], bBuf, (DWORD )0);	//��������ʱ��
	}

	DbClrPnData(CCT_BN_SPM, SECT0, POINT_NUM, wPn);		//���������BANK
	BYTE bRate = RATE_NUM; //������
	ReadItemEx(BN0, wPn, 0x8911, &bRate);	
	WriteItemEx(CCT_BN_SPM, wPn, 0x8911, &bRate); //��������дһ�Σ���Ȼ������=0
}


void CctVipPnData(WORD wPn)
{
	BYTE bBuf[PN_MASK_SIZE+10];
	if (wPn >= POINT_NUM)
		return;

	DTRACE(DB_CCT, ("CctVipPnData: wPn=%d\r\n", wPn));

	//��BN16������ʱ����
	const static WORD wBank16Id[] = {0x6110, 0x6111, 0x6112, 0x6113, 0x6114, 0x6115, 0x6116, 0x6117, 0x6120};
	memset(bBuf, 0, sizeof(bBuf));
	for (WORD i=0; i<sizeof(wBank16Id)/sizeof(WORD); i++)
	{
		WriteItemEx(BN16, wPn, wBank16Id[i], bBuf, (DWORD )0);	//��������ʱ��
	}
}
