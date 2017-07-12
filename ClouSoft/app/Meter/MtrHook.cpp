/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrHook.cpp
 * ժ    Ҫ�����ļ���Ҫ��������ͨ�Žӿڿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��4��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *			 $�����ﲻҪ�����inline,����Ϳ��ļ�һ�����ʱ�ض�λ
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaCfg.h"
#include "MtrHook.h"
#include "FaAPI.h"
#include "DbCctAPI.h"
#include "MeterAPI.h"
/*
//����:485����ȷ��(���в�����,�ֶ˿�),�ڹ��ϵķ���/�ָ�ʱ��Ļص�����,�������ɸ澯�¼�
void On485ErrEstb(WORD wPort)
{
	if (g_b485PortStatus == 0)
		g_b485PortStatus = 1;
}

//����:485���ϻָ�(���в�����,�ֶ˿�),�ڹ��ϵķ���/�ָ�ʱ��Ļص�����,�������ɸ澯�¼�
void On485ErrRecv(WORD wPort)		
{
	if (g_b485PortStatus == 1)
		g_b485PortStatus = 0;
}*/

//����:�������ȷ��(����������),�ڹ��ϵķ���/�ָ�ʱ��Ļص�����,�������ɸ澯�¼�
void OnMtrErrEstb(WORD wPn)	
{
	
	if (wPn > POINT0 && wPn <= POINT_NUM && g_bReadMtrStatus[wPn-1] == 0)
	{
		g_bReadMtrStatus[wPn-1] = 1;
		DTRACE(DB_METER_EXC, ("OnMtrErrEstb::wPn = %d estb.\r\n", wPn));
	}

}

//����:������ϻָ�(����������),�ڹ��ϵķ���/�ָ�ʱ��Ļص�����,�������ɸ澯�¼�
void OnMtrErrRecv(WORD wPn)	
{
	if (wPn > POINT0 && wPn <= POINT_NUM && g_bReadMtrStatus[wPn-1] == 1)
	{
		g_bReadMtrStatus[wPn-1] = 0;
		DTRACE(DB_METER_EXC, ("OnMtrErrRecv::wPn = %d Recv.\r\n", wPn));
	}
}


//����:��ȡ�����㳭����ϵ�״̬
bool IsMtrErr(WORD wPn)	
{
	if (g_bReadMtrStatus[wPn-1] == 1) //���Ϸ���
		return true;
	else							  //����
		return false;
}


/*
void DoMtrAnd485ErrErc()
{
	TTime tmNow;
	BYTE bAlrBuf[20];
	WORD wPn, i;
	static DWORD dwTime = 0;
	TBankItem tbItem[2];
	WORD wConstID[] = {0x9010, 0x9110};
	
	for (i=0; i<PN_NUM; i++)
	{
	  	memset(bAlrBuf, INVALID_DATA, sizeof(bAlrBuf));
		if (g_bReadMtrStatus[i] == 1 && !g_fMtrFailHapFlg[i])
		{
#ifdef PRO_698
			wPn = i+1;
			bAlrBuf[0] = wPn & 0xff;
			bAlrBuf[1] = 0x80 | (wPn >> 8) & 0x0f;
			ReadItemEx(BN11, wPn, 0x0b01, bAlrBuf+2);
			ReadItemEx(BN11, wPn, 0x0b02, bAlrBuf+7);
			ReadItemEx(BN11, wPn, 0x0b03, bAlrBuf+12);
			GetCurTime(&tmNow);
//			SaveAlrData( ERC_MTRRDFAIL, tmNow, bAlrBuf);
#endif
			g_fMtrFailHapFlg[i] = true;
		}
		else if (g_bReadMtrStatus[i] == 2)
		{
#ifdef PRO_698		
			if (dwTime == 0)
				dwTime = GetCurTime();
			if (GetCurTime() - dwTime < 20)
				continue;
			wPn = i+1;
			bAlrBuf[0] = wPn & 0xff;
			bAlrBuf[1] = (wPn >> 8) & 0x0f;
			GetCurTime(&tmNow);
			TimeToFmt15(tmNow, bAlrBuf+2);
			for (int j=0; j<2; j++)
			{
				tbItem[j].wBn = BN0;
				tbItem[j].wPn = wPn;
				tbItem[j].wID = wConstID[j];
			}
//			DirectReadMtr(tbItem, 2, bAlrBuf+7);
//			SaveAlrData( ERC_MTRRDFAIL, tmNow, bAlrBuf);
#endif			
			g_bReadMtrStatus[i] = 0;
			g_fMtrFailHapFlg[i] = false;
			dwTime = 0;
		}
	}

	if (g_b485PortStatus == 1 && !g_f485FailHapFlg)
	{
		BYTE bErr = 4;
		GetCurTime(&tmNow);
//		SaveAlrData(ERC_TERMERR, tmNow, &bErr);
		g_f485FailHapFlg = true;
	}
	else if (g_b485PortStatus == 2)
	{
		g_b485PortStatus = 0;
		g_f485FailHapFlg = false;
	}
}*/

//����:��ȡ��������Ϣ
//����:@
void Get485PnMask(BYTE* pbNodeMask)
{
	ReadItemEx(BN17, PN0, 0x6002, pbNodeMask); //485����������λ  
}

//����:��ȡ��������Ϣ
//����:@
void GetPlcPnMask(BYTE* pbNodeMask)
{
	ReadItemEx(BN17, PN0, 0x6003, pbNodeMask); //�ز�����������λ  
}

//����:��ȡ��������Ϣ�Ķ�ָ��
//����:@
const BYTE* Get485PnMask( )
{
	return GetItemRdAddr(BN17, PN0, 0x6002); //485����������λ
}

//����:��ȡ��������Ϣ�Ķ�ָ��
//����:@
const BYTE* GetPlcPnMask( )
{
	return GetItemRdAddr(BN17, PN0, 0x6003); //�ز�����������λ
}

//����:����485����������λ
void Set485PnMask(WORD wPn)
{
	BYTE bPnMask[PN_MASK_SIZE];

	ReadItemEx(BN17, PN0, 0x6002, bPnMask); //485����������λ

	bPnMask[wPn/8] |= 1<<(wPn%8);

	WriteItemEx(BN17, PN0, 0x6002, bPnMask); //485����������λ,���µ����ݿ�
}

//����:�����ز�����������λ
void SetPlcPnMask(WORD wPn)
{
	BYTE bPnMask[PN_MASK_SIZE];

	ReadItemEx(BN17, PN0, 0x6003, bPnMask); //�ز�����������λ

	bPnMask[wPn/8] |= 1<<(wPn%8);

	WriteItemEx(BN17, PN0, 0x6003, bPnMask); //�ز�����������λ,���µ����ݿ�
}

//����:���485����������λ
void Clr485PnMask(WORD wPn)
{	
	BYTE bPnMask[PN_MASK_SIZE];

	ReadItemEx(BN17, PN0, 0x6002, bPnMask); //485����������λ

	bPnMask[wPn/8] &= ~(1<<(wPn%8));

	WriteItemEx(BN17, PN0, 0x6002, bPnMask); //485����������λ,���µ����ݿ�
}

//����:����ز�����������λ
void ClrPlcPnMask(WORD wPn)
{	
	BYTE bPnMask[PN_MASK_SIZE];

	ReadItemEx(BN17, PN0, 0x6003, bPnMask); //�ز�����������λ

	bPnMask[wPn/8] &= ~(1<<(wPn%8));

	WriteItemEx(BN17, PN0, 0x6003, bPnMask); //�ز�����������λ,���µ����ݿ�
}

//����:ͨ������ַת��Ϊ�������
//����:@pbTsa ����ַ,
//	   @bAddrLen ���ַ����
//����:����ҵ�ƥ��Ĳ������򷵻ز������,���򷵻�0
WORD MtrAddrToPn(const BYTE* pbTsa, BYTE bAddrLen)
{
	BYTE bMtrAddr[17];
	WORD wPn = 0;
	const BYTE* pbPnMask = Get485PnMask();	//ȡ�÷��ز���485����λ.
	if (pbPnMask == NULL)
		return 0;

	while (1)
	{
		wPn = SearchPnFromMask(pbPnMask, wPn);

		if (wPn >= POINT_NUM)
			break;

		GetMeterAddr(wPn, bMtrAddr);
		if (memcmp(pbTsa, bMtrAddr, bAddrLen) == 0)
		{
			return wPn;
		}
		wPn++;
	}

	return 0;
}

//����:�����Ƿ�����������
//����:@pbTsa ����ַ,
//	   @bAddrLen ���ַ����
//����:����ҵ�ƥ��Ĳ������򷵻ز������,���򷵻�0
WORD PulseAddrToPn(const BYTE* pbTsa, BYTE bAddrLen)
{
	BYTE bMtrAddr[TSA_LEN+1];
	WORD wPn = 0;

	while (1)
	{
		if (wPn >= PULSE_PN_NUM)
			break;

		ReadItemEx(BN0, wPn, 0x2401, bMtrAddr);
		if (memcmp(pbTsa+2, bMtrAddr+2, bAddrLen-2) == 0)
		{
			return wPn+1;
		}
		wPn++;
	}

	return 0;
}

