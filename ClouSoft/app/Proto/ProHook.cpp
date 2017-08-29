/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProHook.cpp
 * ժ    Ҫ�����ļ���Ҫ��������ͨ�Žӿڿ�Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��3��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ProAPI.h"
#include "Modem.h"
#include "Trace.h"

//����:�ۼ������Ľӿں���
//��ע:����ת�����ͳ�������غ���,
// 	   ���ֻ������ʱ����ͳ����ĳ�Ա����,����ÿ�ν��շ��Ͷ�дϵͳ��,��Ӱ��ͨ��Ч��
void AddFlux(DWORD dwLen)
{
//	if (!IsDownSoft())
		g_StatMgr.AddFlux(dwLen);
}

//����:�ۼ������Ľӿں���
bool IsFluxOver()	
{
    if (IsDownSoft())
    	return false;
	DWORD dwMonFlux = 0;
    ReadItemEx(BN0, PN0, 0x1501, (BYTE*)&dwMonFlux);

	DWORD dwFluxLimit = 0;
	ReadItemEx(BN0, PN0, 0x024f, (BYTE*)&dwFluxLimit); //F36���ն�����ͨ��������������,��ͨ����������Ϊ0����ʾϵͳ����Ҫ�ն˽�����������

	if (dwFluxLimit>0 && dwMonFlux>dwFluxLimit)
		return true;
	else
		return false;
}

//����:�ص�����,�������ɸ澯��¼����;
//��ע:ֻ���ڵ�һ�γ�������ʱ�����ɸ澯��¼,����ɱ������������ĵ��ú������ж�
void GprsOnFluxOver()	
{

	BYTE bAlrBuf[16];
    
	
	
	TTime now;
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	GetCurTime(&now);

	ReadItemEx(BN0, PN0, 0x1501, &bAlrBuf[0]); //�����ѷ�����ͨ������
	ReadItemEx(BN0, PN0, 0x024f, &bAlrBuf[4]); //��ͨ����������,F36���ն�����ͨ��������������

//	SaveAlrData(ERC_FLUXOVER, now, bAlrBuf);
	DTRACE(DB_METER_EXC, ("GprsOnFluxOver: ########## \n"));
}


//����:GPRS�Ƿ�������ʱ��
bool GprsIsInPeriod()
{
	BYTE bBuf[16];
	if (ReadItemEx(BN0, PN0, 0x008f, bBuf) > 0) //C1F8
	{
        //ʱ������ģʽ��������ʱ�α�־��D0~D23��λ˳���Ӧ��ʾ0~23��
		//��"1"��ʾ��������ʱ�Σ���"0"��ʾ��ֹ����ʱ�Σ�������ʱ�ε��趨ֵ��ͬʱ���ϲ�Ϊһ����ʱ�Ρ�

		TTime now;
		GetCurTime(&now);
		if (bBuf[5+now.nHour/8] & (1<<(now.nHour%8)))
			return true;
	}

	return false;
}

//����:GPRSͨ���ĸ澯���������������Ƿ�����,������Ҫ���ߵ�ʱ��,�ȴ��澯��������������
//	   ȫ�������ٵ���.������õȴ��澯����������ȫ������Ϳ��Ե���,ֱ�ӷ���true
//����:@dwStartClick �����������ʼʱ��,���������ڶ��ٷ�����û����Ҳ�������
//����:����澯����������ȫ�������򷵻�true,���򷵻�false
bool GprsIsTxComplete(DWORD dwStartClick) 
{
	return true;
}

void ProThrdHook(CProtoIf* pIf, CProto* pProto)
{
	static BYTE bDispCnt = 0;
	
	if (pIf->GetIfType()==IF_GPRS || pIf->GetIfType()==IF_SOCKET)
	{
		WORD wState=pIf->GetState();
		if (wState!=IF_STATE_TRANS && wState!=IF_STATE_LOGIN)
		{
			if (bDispCnt != 1)
			{
				char szTmp[21];
				memset(szTmp, 0, sizeof(szTmp));
#ifdef ENGLISH_DISP
				strcpy(szTmp, "OffL");
#else
				strcpy(szTmp, "����");
#endif
				WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);	
				bDispCnt = 1;
			}
		}
		else
		{
			char szTmp[21];
			memset(szTmp, 0, sizeof(szTmp));
			if ((GetClick()-pProto->GetRcvClick()) > 12)
			{
				if (bDispCnt != 2)
				{
#ifdef ENGLISH_DISP
					strcpy(szTmp, "Idle");
#else
					strcpy(szTmp, "����");
#endif
					WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);
					bDispCnt = 2;
				}
			}
			else
			{
				if (bDispCnt != 3)
				{
#ifdef ENGLISH_DISP
					strcpy(szTmp, "Comu");
#else
					strcpy(szTmp, "ͨѶ");
#endif
					WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);
					bDispCnt = 3;
				}
			}
		}
	}
}



//��������ģ��汾��CCID����Ϣ���µ�ϵͳ�⣬��MODEM��Ļص��ӿڣ��ڸ����������Ϣ�����
void UpdModemInfo(TModemInfo* pModemInfo)
{
	DWORD dwOAD;
	BYTE bBuf[128] = {0};
	BYTE bTmpBuf[128] = {0};
	BYTE *p = bBuf;
	BYTE bChannel = 0;
	int iLen;

	if (pModemInfo != NULL)	//���ж��Ƿ�Ϊ��ָ��
	{
		ReadItemEx(BANK17, PN0, 0x6010, &bChannel);
		dwOAD = 0x45000500 + bChannel*0x00010000;

		*p++ = DT_STRUCT;
		*p++ = 0x06;
		*p++ = DT_VIS_STR;
		*p++ = 0x04;
		memcpy(p, pModemInfo->bManuftr, 4);
		p += 4;
		*p++ = DT_VIS_STR;
		*p++ = 0x04;
		memcpy(p, pModemInfo->bSoftVer, 4);
		p += 4;
		*p++ = DT_VIS_STR;
		*p++ = 0x06;
		memcpy(p, pModemInfo->bSoftDate, 6);
		p += 6;
		*p++ = DT_VIS_STR;
		*p++ = 0x04;
		memcpy(p, pModemInfo->bHardVer, 4);
		p += 4;
		*p++ = DT_VIS_STR;
		*p++ = 0x06;
		memcpy(p, pModemInfo->bHardDate, 6);
		p += 6;
		*p++ = DT_VIS_STR;
		*p++ = 0x08;
		memset(p, '0', 8);
		p += 8;

		iLen = ReadItemEx(BN0, bChannel, 0x4503, bTmpBuf);
		if (iLen>0 && memcmp(bTmpBuf, bBuf, iLen)!=0)
			WriteItemEx(BN0, bChannel, 0x4503, bBuf);

		TraceBuf(DB_FAPROTO, "UpdModemInfo:", bBuf, p-bBuf);

		memset(bBuf, 0, sizeof(bBuf));
		memset(bTmpBuf, 0, sizeof(bTmpBuf));
		p = bBuf;
		*p++ = DT_VIS_STR;
		*p++ = 20;
		memcpy(p, pModemInfo->bCCID, 20);

		iLen = ReadItemEx(BN0, bChannel, 0x4505, bTmpBuf);
		if (iLen>0 && memcmp(bTmpBuf, bBuf, iLen)!=0)
			WriteItemEx(BN0, bChannel, 0x4505, bBuf);
	}
}

void UpdSIMNum(TModemInfo* pModemInfo)
{
	BYTE bChannel = 0, bBuf[20] = {0};
	
	if (pModemInfo == NULL)	//���ж��Ƿ�Ϊ��ָ��
		return;

	ReadItemEx(BANK17, PN0, 0x6010, &bChannel);

	ReadItemEx(BN0, bChannel, 0x4508, bBuf);
	if (memcmp(&bBuf[2], pModemInfo->bCNUM, 16) == 0)	//��ͬ��ˢ��
		return;

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = DT_VIS_STR;
	bBuf[1] = 16;
	memcpy(&bBuf[2], pModemInfo->bCNUM, 16);
	WriteItemEx(BN0, bChannel, 0x4508, bBuf);
}

void UpdSIMCIMI(BYTE* pbBuf)
{
	BYTE bChannel = 0, bBuf[20] = {0};
	if (pbBuf == NULL)	//���ж��Ƿ�Ϊ��ָ��
		return;
	
	ReadItemEx(BANK17, PN0, 0x6010, &bChannel);

	ReadItemEx(BN0, bChannel, 0x4506, bBuf);
	if (memcmp(&bBuf[2], pbBuf, 15) == 0)	//��ͬ��ˢ��
		return;

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = DT_VIS_STR;
	bBuf[1] = 15;
	memcpy(&bBuf[2], pbBuf, 15);
	WriteItemEx(BN0, bChannel, 0x4506, bBuf);
}

void UpdSysInfo(BYTE* pbBuf)
{
	//�粻��Ҫ������ʽ���˺�����ֱ��return

	//Modem���ȡ��Ӫ�̼�������ʽ��Ϣ�����˺���д��
	//pbBuf[0]--bSysMode������ʽ��0:No service  2:2G(����GSM,EDGE,CDMA)  3:3G(���� WCDMA,TD-SCDMA,EVDO)  4:4G(���� FDD-LTE,TDD-LTE) 
	//pbBuf[1]--bMnc������Ӫ�̴��룬00:ע��ʧ��  01:�й��ƶ�  02:�й���ͨ  03:�й����� 
	if(pbBuf == NULL)
		return;
	WriteItemEx(BN2, PN0, 0x2108, pbBuf);
	//TraceBuf(DB_FAFRM, "UpdSysInfo: SysMode and Mnc is -> ", pbBuf, 2);

}

void GetSysInfo(BYTE* pbBuf)
{
	//�粻��Ҫ������ʽ���˺�����ֱ��return

	if(pbBuf == NULL)
		return;
	ReadItemEx(BN2, PN0, 0x2108, pbBuf);	//�˴�IDͬUpdSysInfo�е�ID
	//TraceBuf(DB_FAFRM, "UpdSysInfo: SysMode and Mnc is -> ", pbBuf, 2);
}

BYTE GetNetStandard(void)
{
	//�粻��Ҫ����������ʽ���˺�����ֱ��return

	//��Ҫ������ʽʱʹ��,��Modem�����
	BYTE bNetType = 0x01;	// 02: 2G; 03: 3G; 04: 4G; ����Ϊ����Ӧ,������
	//ReadItemEx(BN10, PN0, 0xxxxx, &bNetType);	//��ʾ����ѡȡҪ��������ʽд���ID����ID��������չ
	return bNetType;
}
