/*********************************************************************************************************
* Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�DbAPI.cpp
* ժ    Ҫ�����ļ���Ҫʵ��Э����ص����ݿ��׼�ӿ�֮�����չ�ӿ�
* ��ǰ�汾��1.0
* ��    �ߣ�᯼���
* ������ڣ�2007��8��
*********************************************************************************************************/
#include "stdafx.h"
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "MeterAPI.h"
#include "TaskManager.h"
#include "DataManager.h"
#include "LibAcConst.h"
#include "DbVer.h"
#include "MtrHook.h"
#include "CctAPI.h"
#include "ParaMgr.h"
#include "SchParaCfg.h"



//����:��ȡֱ��ģ��������������
//����:0��ʾ��Ч��>1��ʾģ�����������
BYTE GetDCPnProp(WORD wPn)
{
	if (wPn >= GB_MAXMEASURE)
		return INVALID_POINT;

	BYTE bProp[50]={0};
	if (ReadItemEx(BN0, wPn, 0x8904, bProp) <= 0)
		return INVALID_POINT;
	if (bProp[0] == 0)
		return INVALID_POINT;	

	return bProp[0];
}

//����:ֱ��ģ�����Ƿ���Ч
bool IsDCPnValid(WORD wPn)
{
	return GetDCPnProp(wPn) != INVALID_POINT;
}

//����:ֱ��ģ����1��Fn
bool IsDcC1Fn(BYTE bFn)
{
	if (bFn==73 || bFn==121)
		return true;

	return false;
}

//����:ֱ��ģ����2��Fn
bool IsDcC2Fn(BYTE bFn)
{
	if (bFn==129 || bFn==130 || bFn==138)
		return true;

	return false;
}

//����:��ȡ��ֱ��ģ��������������
//����:0��ʾ��Ч��2��ʾ���1��ʾ����//��4��ʾģ����
BYTE GetPnProp(WORD wPn)
{	
	if (wPn >= POINT_NUM)
		return INVALID_POINT;

	BYTE bProp[PNPARA_LEN]={0};
	if (ReadItemEx(BN0, wPn, 0x6000, bProp) <= 0)
		return INVALID_POINT;

	if (bProp[2]==PN_PROP_METER && IsMtrValid(ByteToWord(bProp)) )
		return PN_PROP_METER;

	else if (bProp[2] == PN_PROP_AC)
		return PN_PROP_AC;

	else if (bProp[2] == PN_PROP_PULSE)
	{
		bool fAllOk = true;

		for (BYTE i=0; i<bProp[3]; i++) //ֻҪ�������������������Ч����˲�������Ч
		{
			if ( !IsPluseValid(ByteToWord(&bProp[4+2*i])) ) 
				fAllOk = false;
		}

		if (fAllOk) 
			return PN_PROP_PULSE;
	}
	else if (bProp[2] == PN_PROP_CCT)
		return PN_PROP_CCT;

	else if (bProp[2] == PN_PROP_RJ45)//����RJ45����
		return PN_PROP_RJ45;

	else if (bProp[2] == PN_PROP_EPON)//���˶˿�
		return PN_PROP_RJ45;//PN_PROP_EPON;

	else if (bProp[2] == PN_PROP_BBCCT)//����ز�ͨ��
		return PN_PROP_BBCCT;

	return INVALID_POINT;
}

//����:ȡPN��Ӧ�Ķ˿ں�
//����:�����ȷ�򷵻ض˿ں�,���򷵻�0
BYTE GetPnPort(WORD wPn)
{
	TOobMtrInfo tMtrInfo;
	//BYTE bBuf[128];
	//BYTE bPortSn;

	if (!GetMeterInfo(wPn, &tMtrInfo))
		return 0xff;

	if (MtrPnToSn(wPn) > 0)
	{
// 		memset(bBuf, 0, sizeof(bBuf));
// 		if (ReadItemEx(BN0, PN0, tMtrInfo.dwPortOAD, bBuf) <= 0)
// 			return false;
// 		if (IsAllAByte(bBuf, 0, sizeof(bBuf)))	
		{
			if (tMtrInfo.dwPortOAD == 0xF2010201)	//485-1
				//return PORT_GB485;
				return LOGIC_PORT_MIN;		//1
			else if (tMtrInfo.dwPortOAD == 0xF2010202)	//485-2
				//return PORT_GB485;	
				return LOGIC_PORT_MIN+1;	//2
			else if (tMtrInfo.dwPortOAD == 0xF2080201)	//AC
				return PORT_AC;
			else if ((tMtrInfo.dwPortOAD&0xF2090200) == 0xF2090200)	//PLC
				return PORT_CCT_PLC;
		}
	}

	return 0;
}

//����:ȡ�ò�����ĵ��Э������
BYTE GetPnMtrPro(WORD wPn)
{	
	TOobMtrInfo tMtrInfo;
	//BYTE bBuf[128];

	if (!GetMeterInfo(wPn, &tMtrInfo))
		return 0xff;

	if (MtrPnToSn(wPn) > 0)
	{
		return tMtrInfo.bProType;
	}

	return 0;
}

//����:ȡ����Ч���ò��������
//����:
//���ø�ʽ���£�
//02 04 
//		12 00 04 
//		02 07 
//			85 06 10 00 00 00 22 24 
//			16 03 
//			16 03 
WORD GetValidPnNum()
{
	return g_wValidPnNum;
}

//����:��ȡ�ܼ�������
//����:0��ʾ��Ч������0��ʾ��Ч��
BYTE GetGrpProp(WORD wPn)
{
	if (wPn >= GB_MAXSUMGROUP)
		return INVALID_POINT;

	WORD wLen;
	BYTE *pbBuf, bType, bBuf[120];
	if (ReadItemEx(BN0, wPn, 0x2301, bBuf) > 0)
	{		
		const ToaMap* pOadMap = GetOIMap(0x23010200);
		for (BYTE i=0; i<bBuf[1]; i++)
		{
			pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType);
			if (pbBuf == NULL)
				return INVALID_POINT;

			if (MtrAddrToPn(pbBuf+3, pbBuf[3]+1)>0 || PulseAddrToPn(pbBuf+3, pbBuf[3]+1)>0) //��������� || ���������
				continue;
			else
				return INVALID_POINT;
		}
		return bBuf[1];
	}

	return INVALID_POINT;
}

#ifdef PRO_698
void SetFnFlg(BYTE* pbFnFlg, BYTE bFn)
{
	if (bFn==0 || bFn>248)
		return;

	pbFnFlg[(bFn-1)>>3] |= 1 << ((bFn-1)&0x07);
}

void SetFnFlg(BYTE* pbFnFlg, const BYTE* pbFn, WORD wNum)
{
	for (WORD i=0; i<wNum; i++)
	{
		SetFnFlg(pbFnFlg, pbFn[i]);
	}
}


//����:����1������ŵ�Ĭ��֧��FN
void SetDefFnCfgOfMain(BYTE bMain)
{
	WORD wOff;
	BYTE bBuf[C2_CFG_LEN+10];
	BYTE bVer;
	ReadItemEx(BN1,PN0,0x2000,&bVer);

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = bMain;   //����
	bBuf[1] = 2;       //С����

	//��0��
	wOff = 2;
	//bBuf[wOff++] = 0;        //С���
	//bBuf[wOff++] = 31;       //��Ϣ������
	//SetFnFlg(&bBuf[wOff], bM2S1Fn, sizeof(bM2S1Fn));
	wOff += 33;

	//��1��	
	bBuf[wOff++] = 1;        //С���
	bBuf[wOff++] = 31;       //��Ϣ������

#ifdef VER_HEBEI
	{
		SetFnFlg(&bBuf[wOff], g_bSingleFn_HeB, sizeof(g_bSingleFn_HeB));
	}
#endif

#ifdef VER_JIBEI
	{
		SetFnFlg(&bBuf[wOff], g_bSingleFn_JiBei, sizeof(g_bSingleFn_JiBei));
	}
#endif 

#ifdef VER_ZJ
	{
		SetFnFlg(&bBuf[wOff], g_bSingleFn, sizeof(g_bSingleFn));
	}
#endif

	wOff += 31;

	//��2��
	bBuf[wOff++] = 2;		 //С���
	bBuf[wOff++] = 31;       //��Ϣ������


#ifdef VER_HEBEI
	{
		SetFnFlg(&bBuf[wOff], g_bMultiFn_HeB, sizeof(g_bMultiFn_HeB));
	}
#endif

#ifdef VER_JIBEI
	{
		SetFnFlg(&bBuf[wOff], g_bMultiFn_JiBei, sizeof(g_bMultiFn_JiBei));
	}
#endif

#ifdef VER_ZJ
	{
		SetFnFlg(&bBuf[wOff], g_bMultiFn, sizeof(g_bMultiFn));
	}
#endif

	WriteItemEx(BN0, bMain, 0x027f, bBuf);	
}

#if FA_TYPE == FA_TYPE_K32

//485�˿�Ĭ�ϲ���
void SetDefault485PortPara()
{	
	int iLen;
	WORD wPn;
	BYTE bBuf[100];
	const WORD wID = 0xF201;	//485�˿�
	bool fTrigerSave = false;

	const BYTE bMtrRd485PortCfg[] = {
				DT_STRUCT, 0x03, 
					DT_VIS_STR, 0x10,	//�˿�������
						'0', '0', '0', '0', '0', '0', '0', '0',
						'0', '0', '0', '0', '0', '0', '0', '0',
					DT_COMDCB,	//�˿ڲ���
						0x03,	//2400bps --- ����ڲ�����Ĭ��2400
						0x02,	//żУ��
						0x08,	//8λ����λ
						0x01,	//ֹͣλ
						0x00,	//����
					DT_ENUM,	//�˿ڹ���
						0x01,	//����ͨ�ţ�0��������1����������2����ͣ�ã�3��
};	//485�����Ĭ�ϲ���

	const BYTE bLocal485PortCfg[] = {
				DT_STRUCT, 0x03, 
					DT_VIS_STR, 0x10,	//�˿�������
						'0', '0', '0', '0', '0', '0', '0', '0',
						'0', '0', '0', '0', '0', '0', '0', '0',
					DT_COMDCB,	//�˿ڲ���
						0x06,	//9600bps --- ά���ڲ�����Ĭ��9600
						0x02,	//żУ��
						0x08,	//8λ����λ
						0x01,	//ֹͣλ
						0x00,	//����
					DT_ENUM,	//�˿ڹ���
						0x00,	//����ͨ�ţ�0��������1����������2����ͣ�ã�3��
	};	//485ά����Ĭ�ϲ���

	for (wPn=0; wPn<LOGIC_PORT_NUM; wPn++)
	{
		memset(bBuf, 0, sizeof(bBuf));	
		iLen = ReadItemEx(BN0, wPn, wID, bBuf);
		if (iLen>0 && bBuf[0]!=DT_STRUCT)
		{
			if (wPn == LOGIC_PORT_NUM-1)	//��3·485��
				memcpy(bBuf, bLocal485PortCfg, sizeof(bMtrRd485PortCfg));	//485ά����
			else
				memcpy(bBuf, bMtrRd485PortCfg, sizeof(bMtrRd485PortCfg));	//485�����

			WriteItemEx(BN0, wPn, wID, bBuf);
			fTrigerSave = true;
		}
	}

	if (fTrigerSave)
		TrigerSaveBank(BN0, SECT11, -1);
}


//������ʼ����/Ӳ�̸�ʽ����Ĭ�ϲ���
void SetDefaultPara()
{
	SetDefault485PortPara();
}

#endif


//����:����Ĭ�ϴ�С��֧��������
void SetDefFnCfg()
{
	BYTE bBuf[C2_CFG_LEN+10];

	int ilen = ReadItemEx(BN0, 1, 0x027f, bBuf); //����ר���û�	
	if (ilen>0 && bBuf[0]==0xff)		
		SetDefFnCfgOfMain(1);    

	ilen = ReadItemEx(BN0, 2, 0x027f, bBuf); //����ר���û�		
	if (ilen>0 && bBuf[0]==0xff)	
		SetDefFnCfgOfMain(2);

	ilen = ReadItemEx(BN0, 3, 0x027f, bBuf); //��ѹ����һ�㹤��ҵ�û�	
	if (ilen>0 && bBuf[0]==0xff)				
		SetDefFnCfgOfMain(3);

	ilen = ReadItemEx(BN0, 4, 0x027f, bBuf); //��ѹ����һ�㹤��ҵ�û�		
	if (ilen>0 && bBuf[0]==0xff)
		SetDefFnCfgOfMain(4);

	ilen = ReadItemEx(BN0, 5, 0x027f, bBuf); //�����û�	
	if (ilen>0 && bBuf[0]==0xff)		
		SetDefFnCfgOfMain(5);

	ilen = ReadItemEx(BN0, 6, 0x027f, bBuf); //������俼�˼�����		
	if (ilen>0 && bBuf[0]==0xff)		
		SetDefFnCfgOfMain(6);		
}

//����:�˲������Ƿ�֧�ִ�Fn
bool IsFnSupport(WORD wPn, BYTE bFn, BYTE bClass)
{
	WORD i;
	BYTE bMain;
	BYTE bSub;

	BYTE bVer;
	ReadItemEx(BN1,PN0,0x2000,&bVer);
	
	if (bFn==161 && bClass==2)
		return true;

	if (!GetUserType(wPn, &bMain, &bSub)) //��ȡ�û��û�����ź�С���
		return false;

	WORD wID = (bClass==1 ? 0x026f : 0x027f);
	const BYTE* pbCfg = GetItemRdAddr(BN0, bMain, wID);
	if (pbCfg==NULL)
		return false;

	if (bMain>=USR_MAIN_CLASS_NUM || bSub>=USR_SUB_CLASS_NUM || bFn==0)
		return false;

	if (pbCfg[0]==0xff && bMain==0)	//����û������,�û������Ϊ0,��ȫ��֧��
	{
#ifdef VER_HEBEI
		{
			for (i=0; i<sizeof(g_bMultiFn_HeB); i++)
			{
				if (bFn == g_bMultiFn_HeB[i])
				return true;
			}
		}
#endif
#ifdef  VER_JIBEI
		{
			for (i=0; i<sizeof(g_bMultiFn_JiBei); i++)
			{
				if (bFn == g_bMultiFn_JiBei[i])
					return true;
			}
		}
#endif

#ifdef VER_ZJ
		{
			for (i=0; i<sizeof(g_bMultiFn); i++)
			{
				if (bFn == g_bMultiFn[i])
				return true;
			}
		}
#endif
		return false;
	}
	else if (pbCfg[0] >= USR_MAIN_CLASS_NUM)
		return false;

	//BYTE m = pbCfg[1];
	BYTE n;
	BYTE bMark = 1 << ((bFn-1)&0x07);
	BYTE pos = (bFn-1)>>3;
	const BYTE* p = &pbCfg[2+(WORD )bSub*33];	//ָ����Ӧ����û�С���
	if ((bSub!=0) && (*p==bSub))	//�û�С������,���С���Ϊ0������,��������Ľ����ж�
	{
		n = p[1];	 //����n
		if (pos >= n)
			return false;

		if ((bMark & p[2+pos]) != 0)
			return true;
		else
			return false;
	}
	else if (bSub == 0)	//С���Ϊ0,Ĭ��Ϊ�û����ඨ�����������������
	{
		BYTE bBuf[32];
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD sub=1; sub<USR_SUB_CLASS_NUM; sub++)
		{
			p = &pbCfg[2+(WORD )sub*33]; //ָ����Ӧ����û�С���

			if (*p == sub)	//�û�С������
			{
				p++;	//����С���
				n = *p++;	 //����n
				if (n > 31)
					continue;

				for (i=0; i<n; i++)
				{
					bBuf[i] |= *p++;
				}
			}
		}

		if ((bMark & bBuf[pos]) != 0)
			return true;
		else
			return false;

	}

	return false;
}

#ifdef EN_SBJC_V2
//����:��ȡ�û�����
BYTE GetMeterType(WORD wPn)
{  
    TOobMtrInfo tMtrInfo;
	//BYTE bBuf[PNPARA_LEN];

	if (!IsMtrPn(wPn))
        return 0;

	if (!GetMeterInfo(wPn, &tMtrInfo))
		return 0xff;

	if (MtrPnToSn(wPn) > 0)
	{
		return tMtrInfo.bUserType;
	}

	return 0;
}

//����:��ȡ��Э��
BYTE GetMeterSubPro(WORD wPn)
{  
	TOobMtrInfo tMtrInfo;
	//BYTE bBuf[PNPARA_LEN];

	//if (!IsMtrPn(wPn))
	//	return 0;

	if (!GetMeterInfo(wPn, &tMtrInfo))
		return 0xff;

	if (MtrPnToSn(wPn) > 0)
	{
		return tMtrInfo.bUserType;//tMtrInfo.bRate;
	}

	return 0;
}
#endif

//����:��ȡ�û�����
BYTE GetUserType(WORD wPn)
{
	BYTE bBuf[100];

	//TODO:�Ƿ���Ҫ�жϲ������Ƿ���Ч?
	// 	   Ŀǰֻ�ܼٶ�����ǰ���Ѿ��ж�����Ч��,��Ϊ�ӷ���ֵû���ж�

	ReadItemEx(BN0, wPn, 0x8902, bBuf);
//	return bBuf[F10_SN_LEN+66];
	return bBuf[16];
}

//����:��ȡ�û��û�����ź�С���
bool GetUserType(WORD wPn, BYTE* pbMain, BYTE* pbSub)
{
	if (!IsPnValid(wPn))
		return false;

	BYTE bBuf[100];
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return false;

//	BYTE bType = bBuf[F10_SN_LEN+66];
	BYTE bType = bBuf[16];
	*pbMain = (bType>>4) & 0x0f; 
	*pbSub = bType&0x0f;

	return true;
}

//����:��ȡ�û������
BYTE GetUserMainType(WORD wPn)
{
	BYTE bType = GetUserType(wPn);
	return (bType>>4) & 0x0f; 
}


//����:��ȡ�û�С���
BYTE GetUserSubType(WORD wPn)
{
	BYTE bType = GetUserType(wPn);
	return bType&0x0f; 
}
#endif

//����:���������Ƿ���ĳ�����͵Ĳ�����
bool IsPnType(WORD wPn, WORD wType)
{
	BYTE bProp = GetPnProp(wPn);
	if (bProp == PN_PROP_METER || bProp == PN_PROP_RJ45 || bProp == PN_PROP_EPON)
	{
		if (wType & PN_TYPE_MTR)
			return true;
	}
	else if (bProp == PN_PROP_AC)
	{
		if (wType & PN_TYPE_AC)
			return true;
	}
	else if (bProp == PN_PROP_PULSE)
	{
		if (wType & PN_TYPE_PULSE)
			return true;
	}
	else if (bProp == PN_PROP_CCT)
	{
		if (wType & PN_TYPE_CCT)
			return true;
	}

	if (wType & PN_TYPE_DC)
	{
		if(IsDCPnValid(wPn))
			return true;
	}

	if (wType & PN_TYPE_GRP)
	{
		if (IsGrpValid(wPn))
			return true;
	}

	if (wType & PN_TYPE_P0)
	{
		if (wPn == PN0)
			return true;
	}

	return false;
}

bool IsMtrPn(WORD wPn)
{
	BYTE bBuf[PNPARA_LEN];
	memset(bBuf, 0,sizeof(bBuf));	

	if (ReadItemEx(BN0, wPn, 0x6000, bBuf) <= 0)
		return false;

	if (MtrPnToSn(wPn) > 0)
	{
		BYTE bAddL = bBuf[9] + 1;
		if (bBuf[14+bAddL]==0xF2 && bBuf[15+bAddL]==0x01) //485�˿�OAD
		{
			return true;
		}
	}

	return false;
}

//����:�ܼ����Ƿ���Ч
bool IsGrpValid(WORD wPn)
{
	return GetGrpProp(wPn) != INVALID_POINT;
}

//����:�Ƿ�Ϊ�ܼ����1������Fn
bool IsGrpC1Fn(BYTE bFN)
{
	if ((bFN>=17 && bFN<=24) || (bFN>=81 && bFN<=84))
		return true;

	return false;
}

//����:�Ƿ�Ϊ�ܼ����2������Fn
bool IsGrpC2Fn(BYTE bFN)
{
	if ((bFN>=57 && bFN<=62) || (bFN>=65 && bFN<=66) || (bFN>=73 && bFN<=76))
		return true;

	return false;
}

//����:��ȡ�¼�������
//����:0��ʾ���¼���Ч,1��ʾ��Ҫ�¼�,2��ʾһ���¼�
BYTE GetErcType(BYTE bErc)
{
	BYTE bErcType[20];
	if (bErc>64 || bErc==0)
		return 0;
	
	BYTE i=(bErc-1)>>3;
	BYTE bit = (bErc-1)&7;
	if (ReadItemEx(BN0, PN0, 0x40d0, bErcType) >= 0) //F9���ն��¼���¼��������
	{
		if (!((bErcType[i+1]>>bit)&1)) //�¼���¼��Ч��־λ bitstring 1�ֽ�
			return 0;
	
		if ((bErcType[i+10]>>bit)&1)	//�¼���Ҫ�Եȼ���־λ
			return 1;	//1��ʾ��Ҫ�¼�
		else						
			return 2;	//2��ʾһ���¼�
	}

	return 0;
}

//����:ȡ�ò�����Ľ��߷�ʽ
BYTE GetConnectType(WORD wPn)
{
	BYTE bBuf[32];
	ReadItemEx(BN0, wPn, 0x8902, bBuf);
	if (bBuf[17]==1)
		return CONNECT_3P3W;
	else if (bBuf[17]==2)
		return CONNECT_3P4W; 
	else
		return CONNECT_1P;
	/*
	ReadItemEx(BN0, wPn, 0x4100, bBuf);  //F25
	if (bBuf[F25_CONN_OFFSET] == 1)		 //��������
		return CONNECT_3P3W; //�ն˽��߷�ʽ 1	1:����;3:��������;4:��������
	else if (bBuf[F25_CONN_OFFSET] == 3) 	 //�����
		return CONNECT_1P;
	else //if (bBuf[F25_CONN_OFFSET] == 2) //��������
		return CONNECT_3P4W; 
*/
}


#ifdef EN_AC
void CopyAcData(WORD wFromPn, WORD wToPn)
{
	WORD wID[] = {
		0x901f,0x902f,0x907f,0x908f,0x911f,0x912f,0x913f,0x914f,0x915f,0x916f,0x917f,0x918f,
		0x941f,0x942f,0x947f,0x948f,0x951f,0x952f,0x953f,0x954f,0x955f,0x956f,0x957f,0x958f,
		0x981f,0x982f,0x987f,0x988f,0x991f,0x992f,0x993f,0x994f,0x995f,0x996f,0x997f,0x998f,
		0xa01f,0xa02f,0xa11f,0xa12f,0xa13f,0xa14f,0xa15f,0xa16f,
		0xa41f,0xa42f,0xa51f,0xa52f,0xa53f,0xa54f,0xa55f,0xa56f,
		0xa81f,0xa82f,0xa91f,0xa92f,0xa93f,0xa94f,0xa95f,0xa96f,
		0xb01f,0xb02f,0xb11f,0xb12f,0xb13f,0xb14f,0xb15f,0xb16f,
		0xb41f,0xb42f,0xb51f,0xb52f,0xb53f,0xb54f,0xb55f,0xb56f,
		0xb81f,0xb82f,0xb91f,0xb92f,0xb93f,0xb94f,0xb95f,0xb96f,
		0xb21f,0xb31f,0xb32f,0xb33f,0xb34f};

		int iLen;
		BYTE bBuf[256];
		for (WORD i=0; i<sizeof(wID)/sizeof(WORD); i++)
		{
			iLen = ReadItemEx(BN0, wFromPn, wID[i], bBuf); 
			if (iLen > 0)
				WriteItemEx(BN0, wToPn, wID[i], bBuf); 
		}
}
#endif


//����:��ʼ�����ɲ�����,�Զ�����0xa044(����Ĭ�ϲ������)���ý��ɲ��������F10
void InitAcPn()
{
//#ifndef	SYS_WIN	//��ʵ���ն��ϲ��Զ����ý��ɲ�����	
	
//#endif	//EN_AC
}

//#ifdef VER_ZJ
//ͬ���ն˵�ַ����չ����
//ע��ú����������ⲿ����汾�б��
void SyncTermAddr()
{
	BYTE bTmp[60];
	BYTE bBuf[60];

	if (g_bInnerSoftVer[16]=='0' && g_bInnerSoftVer[17]=='0' && g_bInnerSoftVer[18]=='1' && g_bInnerSoftVer[19]=='g')	//�ڲ�����汾��
	{
		DTRACE(DB_CRITICAL, ("SaveSoftVerChg update term addr to BN10, A1DO.\r\n"));
		memset(bBuf, 0, sizeof(bBuf));
		memset(bTmp, 0, sizeof(bTmp));
		ReadItemEx(BN0, PN0, 0x4001, bBuf);
		ReadItemEx(BN10, PN0, 0xa1d0, bTmp);
		if (memcmp(bBuf, bTmp, 17) != 0)	//��ͬ
		{
			WriteItemEx(BN10, PN0, 0xa1d0, bBuf);	//���ն˵�ַ����4001���µ�BN10, a1d0
			TrigerSaveBank(BN10, 0, -1);
		}
	}
}
//#endif


//�ն�����汾�������
void SaveSoftVerChg()
{
	BYTE bBuf[128];
	
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BANK0, PN0, 0x4303, bBuf);
	if (memcmp(bBuf, g_bTermSoftVer, sizeof(g_bTermSoftVer)) != 0)
	{
		WriteItemEx(BANK0, PN0, 0x4303, g_bTermSoftVer);
		TrigerSaveBank(BANK0, SECT_PARAM, -1);
		//��������汾����¼�
		SetInfo(INFO_TERM_VER_CHG);

		//#ifdef VER_ZJ
		//�ر�ע�⣺���ⲿ����汾������δ����������ܲ����������
		SyncTermAddr();
		//#endif
		
	}
}



//�����Զ�Ӧ�������ļ�
void ApllyCfgAuto(void)
{
	char command[64] = {0};
	BYTE bCfgBuf[256] = {0};
	char  szPathNameEx[PATHNAME_LEN+1] = { 0 };
	int iCfgLen = readfile(USER_CFG_PATH"cfgpermit.cfg", bCfgBuf, sizeof(bCfgBuf));
	int iRet = 0;

	DTRACE(DB_FA, ("ApllyCfgAuto Step0 iCfgLen=%d.\r\n", iCfgLen));
	if (iCfgLen <= 0)
		return;

	if ('\n'==bCfgBuf[iCfgLen-1] || '\a'==bCfgBuf[iCfgLen-1] || '\r'==bCfgBuf[iCfgLen-1])
	{
		bCfgBuf[iCfgLen-1] = 0;
	}

	memcpy(szPathNameEx, bCfgBuf, iCfgLen);
	DTRACE(DB_FA, ("ApllyCfgAuto szPathNameEx=%s!\r\n", szPathNameEx));
	iRet = g_pmParaMgr.LoadPara(szPathNameEx);
	if ( iRet == 0)
	{
		DTRACE(DB_FA, ("ApllyCfgAuto Step1 start to apply dft.\r\n"));
		g_pmParaMgr.Parse();

		sprintf(command, "rm -rf "USER_CFG_PATH"cfgpermit.cfg");
		system(command);

		TrigerSavePara();
		while (GetClick()<=10)	//�ϵ糬��10��Żᴥ������
		{
			Sleep(50);
		}
		
		DoFaSave();
		Sleep(100);

		ResetCPU();
	}

	DTRACE(DB_FA, ("ApllyCfgAuto Step2 iRet = %d exit.\r\n", iRet));
}


//�ն�����汾������
void TermSoftVerChg()
{
#if 0
	memset((BYTE*)&g_SoftVerChg, 0, sizeof(g_SoftVerChg)); //ȫ�ֵĻ�����

	BYTE  bSoftVer[SOFT_VER_LEN];
 
 	ReadItemEx(BN0, PN0, 0x100f, bSoftVer);
 	if (memcmp(bSoftVer+16, g_bSoftVer+16, 4) != 0)//ֻҪ�Ƚϰ汾�ţ���
 	{
		g_SoftVerChg.time = GetCurTime();
		g_SoftVerChg.bVerInfo[0] = 0x08; //bit λ��
		g_SoftVerChg.bVerInfo[1] = 0x02;
		memcpy(&g_SoftVerChg.bVerInfo[2], &bSoftVer[16], 4);		//���ݿ�֮ǰ����İ汾
		memcpy(&g_SoftVerChg.bVerInfo[6], &g_bSoftVer[16], 4);		//��ǰ������汾
		//SaveAlrData(ERC_INIT_VER, tm, bBuf);	//д�汾����¼�			
		WriteItemEx(BN0, PN0, 0x100f, g_bSoftVer);
		TrigerSaveBank(BN0, SECT_KEEP_PARA, 0);
 	}

// 	int iRet= -1;
// 	iRet =WriteItemEx(BANK0, PN0, 0x4303, g_bTermSoftVer);

#endif
}

void PostDbInit()
{ 	
#ifdef PRO_698
//	SetDefFnCfg();		//����Ĭ�ϴ�С��֧��������
	InitMtrSnToPn();	//����������ŵ�InitAcPn()ǰ,��Ϊ�漰��װ����ŵ�������ŵ�ӳ��
#if FA_TYPE == FA_TYPE_K32
	SetDefaultPara();	//������ʼ��/��ʽ��Ӳ�̺���Ĭ�ϵĲ���
#endif

#endif

	TermSoftVerChg();

	//SaveAdjParaAferFormat();
	//InitAcPn();

	UpdMeterPnMask();
	UpdPnMask();
	ClrPnChgMask();
#ifdef EN_CCT
	CctPostDbInit();
#endif

	TrigerSaveBank(BN11, 0, -1); //��ʱͳ�Ƶ��������
}

extern TBankCtrl g_Bank0Ctrl[SECT_NUM];
extern TBankCtrl g_BankCtrl[BANK_NUM];
//extern TPnMapCtrl g_PnMapCtrl[PNMAP_NUM];
TDbCtrl g_DbCtrl; //�������ݿ���в������õ����ݿ���ƽṹ

//����:ϵͳ���ݿ��ʼ��
bool InitDB(void)
{
	memset(&g_DbCtrl, 0, sizeof(g_DbCtrl));

	//BANK0�Ŀ����ֶ�
	g_DbCtrl.wSectNum = SECT_NUM;	//BANK0�е�SECT��Ŀ
	g_DbCtrl.pBank0Ctrl = g_Bank0Ctrl;

	//BANK�����ֶ�
	g_DbCtrl.wBankNum = BANK_NUM;	//֧�ֵ�BANK��Ŀ
	g_DbCtrl.pBankCtrl = g_BankCtrl;

	g_DbCtrl.iSectImg = -1;			//485�������ݾ����,���û�������-1
	g_DbCtrl.wImgNum = 0;				//485�������ݾ������
	g_DbCtrl.wSectPnData = 0;	//����485����������,��Ҫ�в�����������֮��Ӧ,���򱾲������ó�0����

	//�����㶯̬ӳ������ֶ�
//	g_DbCtrl.wPnMapNum = PNMAP_NUM;		//֧�ֵ�ӳ�䷽����Ŀ,�������ݿⲻ֧�ֲ����㶯̬ӳ������Ϊ0
	g_DbCtrl.wPnMapNum = 0;		//֧�ֵ�ӳ�䷽����Ŀ,�������ݿⲻ֧�ֲ����㶯̬ӳ������Ϊ0
	g_DbCtrl.pPnMapCtrl = NULL;//g_PnMapCtrl;	//�������ݿⲻ֧�ֲ����㶯̬ӳ������ΪNULL

	g_DbCtrl.wPnMaskSize = PN_MASK_SIZE; //����������λ�Ĵ�С,�������������������λ�ռ�
	g_DbCtrl.pszDbPath = USER_PARA_PATH; //ϵͳ��һЩ�����ļ��Ĵ��Ŀ¼,һ������ΪUSER_PARA_PATH

	g_DbCtrl.wSaveInterv = 15;			//������,��λ����
	if (!InitDbLib(&g_DbCtrl)) //�汾����¼��õ������
		return false;

	PostDbInit();
	
	return true;
}

//�������(���,����,����)����
void ClrPnData(WORD wPn)
{
	wPn;
#if 0
	static WORD wBank0Id[] = {
		0x11Df, 0x11ef, 0x120f, 0x121f, //�����ۼ�
		0x122f, 0x123f, 0x124f, 0x125f, //�����ۼ�								
		0x126f, 0x127f, 0x128f, //г�����ݲ��ڲ����������Ҫ����������Ƕȣ�
		0x300f, 0x301f, 0x302f, 0x303f, //C2F5+,C2F6+,C2F7+,C2F8+		//��һ��
		0x304f, 0x305f, 0x306f, 0x307f, //C2F21+,C2F22+,C2F23+,C2F24+ //��һ��
		0x308f, 0x309f, 0x30Af, 0x30Bf, 0x30Cf, 0x30Df, 0x30Ef, //C2F25+~C2F38+ //��һ��(��)ͳ��
		0x310f, 0x311f, 0x312f, 0x313f, 0x314f, 0x315f, 
		0x316f, 0x317f, 0x318f, 0x319f, //C2F41+~C2F44+ ��һ��(��)�������ؼ�����
		0x32af, 0x32bf, 0x32cf, 0x32df, //C2F97+~C2F100+ ��һ���ߵ�
		0x32ef, 0x330f, 0x331f, 0x332f, 0x333f, 0x334f, 0x335f, 0x336f, 0x337f,};////C2F113+~C2F130+ //��һ��(��)г��ͳ��

		BYTE bBuf[256];
		if (wPn >= POINT_NUM)
			return;

		DTRACE(DB_DP, ("ClrPnData() wPn=%d******2! \r\n", wPn));

		ClearBankData(BN0, SECT_PN_DATA, wPn);
		//ClearBankData(BN0, SECT_IMG, wPn);
		ClearBankData(BN0, SECT_EXT_PN_DATA, wPn);
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD i=0; i<sizeof(wBank0Id)/sizeof(WORD); i++)
		{
			WriteItemEx(BN0, wPn, wBank0Id[i], bBuf, (DWORD )0);	//��������ʱ��
		}

		static WORD wBank11Id[] = {//0x003f, 0x004f, 0x005f, 0x006f, //C1F41+,C1F42+,C1F43+,C1F44+ //�������
			//0x009f, 0x00af, 0x00bf, 0x00cf, //C1F45+,C1F46+,C1F47+,C1F48+	 //�������
			0x00df, 0x00ef, 0x010f, 0x011f, 0x012f, 0x013f, 0x014f, 0x015f, //C2F27+~C2F38+ //����ͳ��
			0x017f, 0x018f, 0x019f, 0x01af, 0x01bf, 0x01cf, 0x01df, 0x01ef, 0x020f, 0x021f, 
			// 0x031f, 0x032f, 0x033f, 0x034f,//C2F97+~C2F100+ //�������
			0x036f, 0x037f, 0x038f, 0x039f, 0x03af, 0x03bf, 0x03cf, 0x03df,}; //C2F113+~C2F123+ //����(��)г��ͳ��
			for (WORD i=0; i<sizeof(wBank11Id)/sizeof(WORD); i++)
			{
				WriteItemEx(BN11, wPn, wBank11Id[i], bBuf, (DWORD )0);	//��������ʱ��
			}

			static WORD wBank18Id[] = {0x003f, 0x004f, 0x005f, 0x006f, //C1F41+,C1F42+,C1F43+,C1F44+ //�������
				0x009f, 0x00af, 0x00bf, 0x00cf, //C1F45+,C1F46+,C1F47+,C1F48+	 //�������
				0x031f, 0x032f, 0x033f, 0x034f,//C2F97+~C2F100+ //�������
				0x040f, 0x041f, 0x042f, 0x043f, 0x044f, 0x045f, 0x046f, 0x047f,//�������������ʱ���ۼ�ֵ
			};
			for (WORD i=0; i<sizeof(wBank18Id)/sizeof(WORD); i++)
			{
				WriteItemEx(BN18, wPn, wBank18Id[i], bBuf, (DWORD )0);	//��������ʱ��
			}
#endif
}



//�������������������
#define MAX_IDS_NUM		10
typedef struct
{
	WORD	wId;						//����ʾֵID
	WORD	wPnDataIds[MAX_IDS_NUM];	//����ʾֵID
	WORD	wBank0Ids[MAX_IDS_NUM];		//�������ʶ
	WORD	wBank11Ids[MAX_IDS_NUM];	//����������ݳ���
	WORD	wBank18Ids[MAX_IDS_NUM];	//��¼�����ݳ���,Ϊ��Ч��������ĳ����ܺ�
}TPulseEngInfo;

void ClrPulsePnData(WORD wPn)
{
	wPn;
#if 0
	static WORD wBank0Id[] = { //������������ص�							 							
		0x126f, 0x127f, 0x128f, //г�����ݲ��ڲ����������Ҫ����������Ƕȣ�
		0x302f, 0x303f, //C2F7+,C2F8+		//��һ��					   		  
		0x30Af, 0x30Bf, 0x30Cf, 0x30Df, 0x30Ef, //C2F27+~C2F38+ //��һ��(��)ͳ��
		0x312f, 0x313f, 0x314f, 0x315f, 
		0x316f, 0x317f, 0x318f, 0x319f, //C2F41+~C2F44+ ��һ��(��)�������ؼ�����							  
		0x32ef, 0x330f, 0x331f, 0x332f, 0x333f, 0x334f, 0x335f, 0x336f, 0x337f,};////C2F113+~C2F130+ //��һ��(��)г��ͳ��

		static WORD wBank11Id[] = {					   		 
			0x00df, 0x00ef, 0x012f, 0x013f, 0x014f, 0x015f, //C2F27+~C2F38+ //����ͳ��
			0x019f, 0x01af, 0x01bf, 0x01cf, 0x01df, 0x01ef, 0x020f, 0x021f, 							
			0x036f, 0x037f, 0x038f, 0x039f, 0x03af, 0x03bf, 0x03cf, 0x03df,}; //C2F113+~C2F123+ //����(��)г��ͳ��

			//��ͨ����Ŀ��ƽṹ:ÿ����һ��
			static TPulseEngInfo g_PulseEngInfo[4] =
			{
				//wId;		wPnDataIds[MAX_IDS_NUM];			wBank0Ids[MAX_IDS_NUM];			wBank11Ids[MAX_IDS_NUM];			wBank18Ids[MAX_IDS_NUM]
				{0x901f,	{0x901f, 0x941f, 0xa01f, 0xa41f, 0xb01f, 0xb41f},//��ǰ(����)���ܡ���ǰ(����)��������ǰ(����)����ʱ��
				{0x11Df, 0x122f, 0x300f, 0x304f, 0x308f, 0x309f, 0x310f, 0x311f, 0x32af},//����(��)�ۼơ�����(��)�ۼơ���һ��(��)ͳ�ơ���һ���ߵ�	
				{0x010f, 0x011f, 0x017f, 0x018f,},//C2F25+~C2F26+ //��һ��(��)ͳ��
				{0x003f, 0x009f, 0x031f, 0x040f, 0x044f,},//C1F41+ //����(��)��㡢������㡢��������(��)���ʱ���ۼ�ֵ
				},
				{0x911f,	{0x911f, 0x951f, 0xa11f, 0xa51f, 0xb11f, 0xb51f},//��ǰ(����)���ܡ���ǰ(����)��������ǰ(����)����ʱ��
				{0x11Ef, 0x123f, 0x301f, 0x305f, 0x32bf,},//����(��)�ۼơ�����(��)�ۼơ���һ���ߵ�	
				{}, //��һ��(��)ͳ��
				{0x004f, 0x00af, 0x032f, 0x041f, 0x045f,},//C1F41+ //����(��)��㡢������㡢��������(��)���ʱ���ۼ�ֵ
				},
				{0x902f,	{0x902f, 0x942f, 0xa02f, 0xa42f, 0xb02f, 0xb42f},//��ǰ(����)���ܡ���ǰ(����)��������ǰ(����)����ʱ��
				{0x120f, 0x124f, 0x302f, 0x306f, 0x308f, 0x309f, 0x310f, 0x311f, 0x32cf},//����(��)�ۼơ�����(��)�ۼơ���һ��(��)ͳ�ơ���һ���ߵ�	
				{0x010f, 0x011f, 0x017f, 0x018f,},//C2F25+~C2F26+ //��һ��(��)ͳ��
				{0x005, 0x00bf, 0x033f, 0x042f, 0x046f,},//C1F41+ //����(��)��㡢������㡢��������(��)���ʱ���ۼ�ֵ
				},
				{0x912f,	{0x912f, 0x952f, 0xa12f, 0xa52f, 0xb12f, 0xb52f},//��ǰ(����)���ܡ���ǰ(����)��������ǰ(����)����ʱ��
				{0x121f, 0x125f, 0x303f, 0x307f, 0x32df,},//����(��)�ۼơ�����(��)�ۼơ���һ���ߵ�	
				{}, //��һ��(��)ͳ��
				{0x006f, 0x00cf, 0x034f, 0x043f, 0x047f,},//C1F41+ //����(��)��㡢������㡢��������(��)���ʱ���ۼ�ֵ
				}
			};

			BYTE i,n,bOff;
			BYTE bBuf[4][256], bPnChgMask = 0, bPnDataIdNum[4] = {0};
			memset(bBuf, 0, sizeof(bBuf));	

			if (wPn >= PN_NUM)
				return;

			DTRACE(DB_DP, ("ClrPulsePnData() wPn=%d******2! \r\n", wPn));

			bPnChgMask = GetPulsePnChgMask(wPn);	//��ȡ������������־	

			for (i=0; i<sizeof(wBank0Id)/sizeof(WORD); i++)
			{
				WriteItemEx(BN0, wPn, wBank0Id[i], &bBuf[0][0], (DWORD )0);	//��������ʱ��
			}

			for (i=0; i<sizeof(wBank11Id)/sizeof(WORD); i++)
			{
				WriteItemEx(BN11, wPn, wBank11Id[i], &bBuf[0][0], (DWORD )0);	//��������ʱ��
			}	

			for (n=0; n<4; n++) //��Ӧ���С����ޡ����С�����
			{
				if ((bPnChgMask & (1<<n)) == (1<<n)) //�����������������б仯 bit0~3��Ӧ���С����ޡ����С�����
				{
					for (i=0; i<MAX_IDS_NUM; i++)
					{
						if (g_PulseEngInfo[n].wBank0Ids[i] != 0)
							WriteItemEx(BN0, wPn, g_PulseEngInfo[n].wBank0Ids[i], &bBuf[n][0], (DWORD )0);	//��BANK0������ʱ��
						if (g_PulseEngInfo[n].wBank11Ids[i] != 0)
							WriteItemEx(BN0, wPn, g_PulseEngInfo[n].wBank11Ids[i], &bBuf[n][0], (DWORD )0);	//��BANK11������ʱ��
						if (g_PulseEngInfo[n].wBank18Ids[i] != 0)
							WriteItemEx(BN0, wPn, g_PulseEngInfo[n].wBank18Ids[i], &bBuf[n][0], (DWORD )0);	//��BANK18������ʱ��
					}
				}
				else
				{
					//Ҫ�����Ĳ��������ݸ���
					for (i=0; i<MAX_IDS_NUM; i++)
					{
						if (g_PulseEngInfo[n].wPnDataIds[i] != 0)
							bPnDataIdNum[n] ++;
					}

					//����Ҫ�����Ĳ��������� 
					ReadItemEx(BN0, wPn, g_PulseEngInfo[n].wPnDataIds, bPnDataIdNum[n], &bBuf[n][0]);
				}		
			}

			ClearBankData(BN0, SECT_PN_DATA, wPn);
			//ClearBankData(BN0, SECT_IMG, wPn);
//			ClearBankData(BN0, SECT_EXT_PN_DATA, wPn);
			int iRet = -1;
			DWORD wSec = GetCurTime();

			//д��Ҫ����������
			for (n=0; n<4; n++) //��Ӧ���С����ޡ����С�����
			{
				if ((bPnChgMask & (1<<n)) != (1<<n)) //�����������������б仯 bit0~3��Ӧ���С����ޡ����С�����
				{
					bOff = 0;
					for (i=0; i<bPnDataIdNum[n]; i++)
					{
						iRet = WriteItemEx(BN0, wPn, g_PulseEngInfo[n].wPnDataIds[i], &bBuf[n][bOff], wSec);
						if (iRet > 0)
							bOff += iRet;
					}
				}
			}
#endif
}

//���ܼ�������
void ClrGrpPnData(WORD wPn)
{
	static WORD wBank0Id[] = {0x2302, 0x2303, 0x2304, 0x2305, //�ܼӹ���
	0x2306, 0x2307, 0x2308, 0x2309, //�ܼӵ���
	0x230a, 0x230b, 0x230f, 0x2310,}; //����״̬
 
	BYTE bBuf[128];
	if (wPn >= GB_MAXSUMGROUP)
		return;

	DTRACE(DB_DP, ("ClrGrpPnData() wPn=%d******2! \r\n", wPn));

	memset(bBuf, 0, sizeof(bBuf));
	for (WORD i=0; i<sizeof(wBank0Id)/sizeof(WORD); i++)
	{
		WriteItemEx(BN0, wPn, wBank0Id[i], bBuf, (DWORD )0);	//��������ʱ��
	}

	static WORD wBank18Id[] = {0x003f, 0x004f, 0x005f, 0x006f, 0x009f, 0x00af, 0x00bf, 0x00cf, //�������ۼ�
		0x035f, 0x036f, 0x037f, 0x038f, 0x039f, 0x03af, 0x03bf, 0x03cf};//����(��)�ܼ��ۼ���㼰ʾֵ���

	for (WORD i=0; i<sizeof(wBank18Id)/sizeof(WORD); i++)
	{
		WriteItemEx(BN18, 0, wBank18Id[i], bBuf, (DWORD )0);	//��������ʱ��
		WriteItemEx(BN18, wPn, wBank18Id[i], bBuf, (DWORD )0);	//��������ʱ��
	}
}


//����:�������ݿ�ʹ�õĵ��������־λ
void UpdMeterPnMask()
{
	//BYTE bProp;
	BYTE bMtrPnMask[PN_MASK_SIZE];

	memset(bMtrPnMask, 0, sizeof(bMtrPnMask));

	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		BYTE bPnProp = GetPnProp(wPn);
		if (bPnProp == PN_PROP_METER || bPnProp == PN_PROP_RJ45 || bPnProp == PN_PROP_EPON)
			bMtrPnMask[wPn/8] |= 1<<(wPn%8);
	}
	SetMeterPnMask(bMtrPnMask);	//���µ����ݿ�
}

//����:���³����߳�ʹ�õĲ���������仯��־λ
void SetPnChgMask(WORD wPn)
{	
	BYTE bPnChgMask[PN_MASK_SIZE];

	ReadItemEx(BN11, PN0, 0x0601, bPnChgMask); //������仯��־,0��ʾ�ޱ仯,1��ʾ�б仯	

	bPnChgMask[wPn/8] |= 1<<(wPn%8);

	WriteItemEx(BN11, PN0, 0x0601, bPnChgMask); //������仯��־,���µ����ݿ�

	memset(bPnChgMask, 0, sizeof(bPnChgMask));
	ReadItemEx(BN17, PN0, 0x7006, bPnChgMask); //�����㳭�յ�������һ������λ
	bPnChgMask[wPn/8] &= ~(1<<(wPn%8));
	WriteItemEx(BN17, PN0, 0x7006, bPnChgMask); //�����㳭�յ�������һ������λ
}

//����:��ѯ�����߳�ʹ�õĲ���������仯��־λ
bool GetPnChgMask(WORD wPn)
{	
	BYTE bPnChgMask[PN_MASK_SIZE];

	ReadItemEx(BN11, PN0, 0x0601, bPnChgMask); //������仯��־,0��ʾ�ޱ仯,1��ʾ�б仯	

	return (bPnChgMask[wPn/8] & (1<<(wPn%8))) != 0;
}

//����:��������߳�ʹ�õĲ���������仯��־λ
void ClrPnChgMask()
{	
	BYTE bPnChgMask[PN_MASK_SIZE];

	memset(bPnChgMask, 0, sizeof(bPnChgMask));
	WriteItemEx(BN11, PN0, 0x0601, bPnChgMask); //�������ޱ仯
}

//����:�����������������仯��־λ
void SetPulsePnChgMask(WORD wPn, BYTE bBit)
{	
	BYTE bPnChgMask = 0;

	ReadItemEx(BN11, wPn, 0x0602, &bPnChgMask); //������仯��־

	bPnChgMask |= 1<<bBit; //bit0~3��Ӧ���С����ޡ����С�����,0��ʾ�ޱ仯,1��ʾ�б仯	

	WriteItemEx(BN11, wPn, 0x0602, &bPnChgMask); //���µ����ݿ�
}

//����:��ѯ�������������仯��־λ
BYTE GetPulsePnChgMask(WORD wPn)
{	
	BYTE bPnChgMask = 0;

	ReadItemEx(BN11, wPn, 0x0602, &bPnChgMask); //bit0~3��Ӧ���С����ޡ����С�����,0��ʾ�ޱ仯,1��ʾ�б仯	

	return bPnChgMask;
}

//����:����������������仯��־λ
void ClrPulsePnChgMask(WORD wPn)
{	
	BYTE bPnChgMask = 0;

	WriteItemEx(BN11, wPn, 0x0602, &bPnChgMask); //�������ޱ仯
}

void UpdPnMask()
{
#if MTRPNMAP!=PNUNMAP
	bool fPnMapFail = false;
#endif //MTRPNMAP!=PNUNMAP
	WORD wPn;
	WORD wPos;
	BYTE bProp;
	BYTE bMask;
	BYTE bPnMask[PN_MASK_SIZE];
	BYTE bOldPnMask[PN_MASK_SIZE];

	DTRACE(DB_DP, ("UpdPnMask()******1! \r\n"));

	memset(bPnMask, 0, sizeof(bPnMask));

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		bProp = GetPnProp(wPn);
		if (bProp==PN_PROP_METER || bProp==PN_PROP_AC || bProp==PN_PROP_PULSE)
			bPnMask[wPn/8] |= 1<<(wPn%8);
	}

	ReadItemEx(BN11, PN0, 0x0600, bOldPnMask); //����������λ

#if MTRPNMAP!=PNUNMAP
	if (NewPnMap(MTRPNMAP, PN0) < 0)	//Ϊ���ɹ̶�ӳ��һ������PN0
		fPnMapFail = true;
#endif //MTRPNMAP!=PNUNMAP

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		bMask = 1<<(wPn%8);
		wPos = wPn/8;
		if ((bPnMask[wPos]&bMask) ^ (bOldPnMask[wPos]&bMask))
		{	
			DTRACE(DB_DP, ("ClrPnData() wPn=%d******1! \r\n", wPn));
			//��������Ч��Ч�����ı�
			//SetPnChgMask(wPn);	//�ñ仯��־(���ӻ�ɾ��)
			ClrPnData(wPn);			
		}
		else if (GetPnChgMask(wPn)) //��������������ı�
		{
			DTRACE(DB_DP, ("ClrPnData() wPn=%d******1! \r\n", wPn));
			if (GetPnProp(wPn) == PN_PROP_PULSE)
				ClrPulsePnData(wPn);
			else
				ClrPnData(wPn);			
		}

		ClrPulsePnChgMask(wPn);

#if MTRPNMAP!=PNUNMAP
		//����ǰ���־λ�Ĳ�һ��,��������,��Ч�Ĳ����㶼��������һ�²�����ӳ��
		//��Ч�Ĳ����㶼ɾ��һ��ӳ��
		if (bPnMask[wPos] & bMask)	//��������Ч
		{
			if (NewPnMap(MTRPNMAP, wPn) < 0)
				fPnMapFail = true;
		}
		else if ((bPnMask[wPos]&bMask) == 0) //��������Ч
		{
			DeletePnMap(MTRPNMAP, wPn);
		}
#endif //MTRPNMAP!=PNUNMAP
	}

#if MTRPNMAP!=PNUNMAP
	if (fPnMapFail)	//��һ������ʧ��,�п����Ǻ���Ĳ����㻹û�ͷ�,ǰ��Ĳ�������������
	{				//������һ�־�����
		NewPnMap(MTRPNMAP, PN0); //Ϊ���ɹ̶�ӳ��һ������PN0

		for (wPn=1; wPn<POINT_NUM; wPn++)
		{
			bMask = 1<<(wPn%8);
			wPos = wPn/8;

			if (bPnMask[wPos] & bMask)	//��������Ч
			{
				NewPnMap(MTRPNMAP, wPn);
			}
		}
	}
#endif //MTRPNMAP!=PNUNMAP

	WriteItemEx(BN11, PN0, 0x0600, bPnMask); //����������λ
}

//����:ȡ�ý��ɵĲ������
WORD GetAcPn()
{
	BYTE bProp[16];
	for (WORD wPn=1; wPn<POINT_NUM; wPn++)
	{		
		int iLen = ReadItemEx(BN0, wPn, 0x8901, bProp);
		if (iLen==12 && bProp[2]==PN_PROP_AC)
			return wPn;
	}

	return PN0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�����������������ڲ�������ŵ���ع�ϵ����ͽӿں������ڲ�������Ŷ�Ӧ6000�ĵ�������λ��(��1��ʼ)
//�����Чֵ����[1,65535],0��Ч
WORD g_wMtrPnToSn[POINT_NUM];
WORD g_wValidPnNum = 0;

void InitMtrSnToPn()
{
	WORD wPn, wSn;
	BYTE bBuf[PNPARA_LEN];

	g_wValidPnNum = 0;
	memset(g_wMtrPnToSn, 0, sizeof(g_wMtrPnToSn));

	for (wPn=MTR_START_PN; wPn<POINT_NUM; wPn++)
	{
		if (ReadItemEx(BN0, wPn, 0x6000, bBuf) > 0)
		{
			wSn = OoOiToWord(&bBuf[4]);
			if (wSn != 0)
			{
				g_wValidPnNum++;
				g_wMtrPnToSn[wPn] = wSn;
				if (IsMtrPn(wPn))
					Set485PnMask(wPn);
			}
		}
	}
}

//����:ͨ��װ����Ż�ò�����ֵ
//����:�����ȷ�򷵻ز������,���򷵻�0
WORD MtrSnToPn(WORD wSn)
{
	if (wSn != 0)
	{
		for (WORD wPn=MTR_START_PN; wPn<POINT_NUM; wPn++)
		{
			if (g_wMtrPnToSn[wPn] == wSn)
			{
				return wPn;
			}
		}
	}

	return INVALID_POINT;
}

//����:����װ����ŵ�������ŵ�ӳ��
void SetMtrSnToPn(WORD wPn, WORD wSn)
{
	if (wPn>=MTR_START_PN && wPn<POINT_NUM) 
	{
		if (g_wMtrPnToSn[wPn]==0 && wSn>0) //ԭ��û��ռ�ã�������
		{
			g_wValidPnNum++;
		}
		else if (g_wMtrPnToSn[wPn]>0 && wSn==0 && g_wValidPnNum>0) //ԭ����ռ�ã�����û��
		{
			g_wValidPnNum--;
		}

		g_wMtrPnToSn[wPn] = wSn;
		/*
		if (IsMtrPn(wPn))
			Set485PnMask(wPn);
		else
			Clr485PnMask(wPn);
		*/
	}
}

//ͨ��������ֵ���װ�����
WORD MtrPnToSn(WORD wPn)
{
	return (wPn>=MTR_START_PN && wPn<=POINT_NUM) ? g_wMtrPnToSn[wPn] : 0;
}

//����:ȡ��һ���յĲ�����λ��
//����:�����ȷ�򷵻ز������,���򷵻�0
WORD GetEmptyPn()
{
	for (WORD wPn=MTR_START_PN; wPn<POINT_NUM; wPn++)
	{
		if (g_wMtrPnToSn[wPn] == 0)
		{
			return wPn;
		}
	}

	return INVALID_POINT;
}

//����:���ݵ��ܱ�װ�����ɾ�������
//����:@wSN ���ܱ�װ�����
//����:�����ȷɾ���򷵻���Ӧ�Ĳ������,���򷵻�0
WORD DeletSN(WORD wSn)
{
	WORD wPn;
	BYTE bBuf[PNPARA_LEN];

	wPn = MtrSnToPn(wSn);
	if (wPn != INVALID_POINT)
	{
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BN0, wPn, 0x6000, bBuf);

		SetMtrSnToPn(wPn, 0);	//���ӳ��
	}

	return wPn;
}

//����:�Ƿ�V2007��645Э�������
bool IsV07Mtr(WORD wPn)
{
	BYTE bBuf[128];
//	BYTE bProp = 0;	

	if (ReadItemEx(BN0, wPn, 0x8901, bBuf) <= 0)
		return false;

	if (bBuf[2] != PN_PROP_METER)
		return false;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return false;	

	if (bBuf[10] == PROTOCOLNO_DLT645_V07)
		return true;
	else
		return false;	
}

//////////////////////////////////////////////////////////////////////////
//����Ϊ������62056Э�����õ��ĸ����ӿں���
// ���ӵ�����
int AddMtrPara(BYTE* pbBuf)
{	
	WORD wSn = ByteToWord(pbBuf);

	if (wSn >= POINT_NUM)
		return -1;

	for (WORD i=1; i<POINT_NUM; i++)
	{	
		if ( PnToMtrSn(i) == wSn) //ԭ����Ч���������ͬ����
		{
			SetPnChgMask(i); //�ñ仯��־(���ӻ��޸�)
			break;
		}
	}

#ifdef	DLMS_863_VER//�˴����⴦����,����ǵ������Э�����7��������6
	if (pbBuf[17] == 0)
		pbBuf[15] = PN_S_PHASE_SURPROID;
	else
		pbBuf[15] = PN_3_PHASE_SURPROID;
#endif	

	return WriteItemEx(BN0, wSn, 0x8902, pbBuf);
}

// ɾ��������
int DelMtrPara(BYTE* pbBuf)
{
	int iRv = -1;
	BYTE bBuf[50];
	memset(bBuf, 0, sizeof(bBuf));

	WORD wSn = ByteToWord(pbBuf);

	if (wSn >= POINT_NUM)
		return -1;

	if (wSn != 0)
	{
		for (WORD i=1; i<POINT_NUM; i++)
		{	
			if ( PnToMtrSn(i) == wSn) //ԭ����Ч���������ͬ����
			{
				//SetPnChgMask(i); //�ñ仯��־(���ӻ��޸�)
				//break;
				return 0;		//2009-09-02����Ч������Ĳ���ɾ
			}
		}

		return WriteItemEx(BN0, wSn, 0x8902, bBuf);
	}
	else
	{
		for (WORD j=1; j<POINT_NUM; j++)//�������
		{	
			if ( GetPnProp(j) == PN_PROP_METER) //ԭ����Ч�ĵ�������ȫ�����			
				//SetPnChgMask(j); //�ñ仯��־(���ӻ��޸�)		
				return 0;		//2009-09-02����Ч������Ĳ���ɾ
		}

		for (WORD i=1; i<POINT_NUM; i++)//���
		{
			if (WriteItemEx(BN0, i, 0x8902, bBuf) > 0)
				iRv = 1;
		}	
	}
	return iRv;
}

//�����������
int AddPlusePara(BYTE* pbBuf)
{
	bool fChg = false;
	int iNum = 0;
	WORD wSnBuf[4];
	memset((BYTE*)&wSnBuf, 0, sizeof(wSnBuf));

	WORD wSn = ByteToWord(pbBuf);

	if (wSn >= PN_NUM)
		return -1;

	for (WORD i=1; i<PN_NUM; i++)
	{	
		if ((iNum=PnToPluseSn(i, wSnBuf)) > 0)
		{
			for(int j=0; j<iNum; j++)
			{
				if (wSnBuf[j] == wSn) //ԭ����Ч���������ͬ����
				{
					SetPnChgMask(i); //�ñ仯��־(���ӻ��޸�)
					SetPulsePnChgMask(i, j); //��ԭ�����������仯��־
					fChg = true;
					break;
				}
			}
		}
		if (fChg) break;
	}

	return WriteItemEx(BN0, wSn, 0x8903, pbBuf);
}
// ɾ���������
int DelPlusePara(BYTE* pbBuf)
{
	int iRv = -1;
	BYTE bBuf[50];
	memset(bBuf, 0, sizeof(bBuf));

	bool fChg = false;
	int iNum = 0;
	WORD wSnBuf[4];
	memset((BYTE*)&wSnBuf, 0, sizeof(wSnBuf));

	WORD wSn = ByteToWord(pbBuf);

	if (wSn >= PN_NUM)
		return -1;

	if (wSn != 0)
	{
		for (WORD i=1; i<PN_NUM; i++)
		{	
			if ((iNum=PnToPluseSn(i, wSnBuf)) > 0)
			{
				for(int j=0; j<iNum; j++)
				{
					if (wSnBuf[j] == wSn) //ԭ����Ч���������ͬ����
					{
						//SetPnChgMask(i); //�ñ仯��־(���ӻ��޸�)
						//SetPulsePnChgMask(i, j); //��ԭ�����������仯��־
						//fChg = true;
						//break;
						return 0;		//2009-09-02����Ч������Ĳ���ɾ
					}
				}
			}
			if (fChg) break;			
		}
		return WriteItemEx(BN0, wSn, 0x8903, bBuf);
	}
	else
	{
		for (WORD j=1; j<PN_NUM; j++)//�������
		{	
			if ( GetPnProp(j) == PN_PROP_PULSE) //ԭ����Ч�����������ȫ�����		
			{
				return 0;		//2009-09-02����Ч������Ĳ���ɾ
				//SetPnChgMask(j); //�ñ仯��־(���ӻ��޸�)	

				//if ((iNum=PnToPluseSn(j, wSnBuf)) > 0) 
				//{
				//	for(int k=0; k<iNum; k++) //ԭ����Ч����������ȫ�����	
				//	{										
				//		SetPulsePnChgMask(j, k); //��ԭ�����������仯��־								
				//	}
				//}
			}
		}

		for (WORD i=1; i<PN_NUM; i++)//���
		{
			if (WriteItemEx(BN0, i, 0x8903, bBuf) > 0)
				iRv = 1;
		}
	}

	return iRv;
}


//�����ܼ������
int AddGrpPara(BYTE* pbBuf)
{
	WORD wPn = *pbBuf;

	if (wPn >= GB_MAXSUMGROUP)
		return -1;

	return WriteItemEx(BN0, wPn, 0x8905, pbBuf);
}


//ɾ���ܼ������
int DelGrpPara(BYTE* pbBuf)
{
	int iRv = -1;
	BYTE bBuf[50];
	memset(bBuf, 0, sizeof(bBuf));

	WORD wPn = *pbBuf;

	if (wPn >= GB_MAXSUMGROUP)
		return -1;

	if (wPn != 0)
	{
		if ( IsGrpOfCtrl(wPn) ) //2009-09-02����Ч���Ʋ����Ĳ���ɾ
			return 0;

		return WriteItemEx(BN0, wPn, 0x8905, bBuf);
	}
	else
	{
		for (WORD i=1; i<GB_MAXSUMGROUP; i++)
		{
			if ( IsGrpOfCtrl(i) ) //2009-09-02����Ч���Ʋ����Ĳ���ɾ
				return 0;
		}

		for (WORD i=1; i<GB_MAXSUMGROUP; i++)
		{
			if (WriteItemEx(BN0, i, 0x8905, bBuf) > 0)
				iRv = 1;
		}
	}

	return iRv;
}

//���Ӽ��������
int AddPointPara(BYTE* pbBuf)
{
	int iRv1,iRv2,iRv3,iRv4;

	WORD wPn = ByteToWord(pbBuf);

	if (wPn >= POINT_NUM)
		return -1;

	if ((iRv1=WriteItemEx(BN0, wPn, 0x8901, pbBuf)) <= 0)//����������
		return -1;
	
	SetPnChgMask(wPn); //�ñ仯��־(���ӻ��޸�)	

	if ((iRv2=WriteItemEx(BN0, wPn, 0x4100, pbBuf+iRv1)) <= 0)//�������������
		return -1;
	if ((iRv3=WriteItemEx(BN0, wPn, 0x4101, pbBuf+iRv1+iRv2)) <= 0)//��������ֵ����
		return -1;
	if ((iRv4=WriteItemEx(BN0, wPn, 0x4102, pbBuf+iRv1+iRv2+iRv3)) <= 0)//�������쳣��ֵ����
		return -1;

	return iRv1+iRv2+iRv3+iRv4;
}


//ɾ�����������
int DelPointPara(BYTE* pbBuf)
{
	int iRet = -1;
	int iRv1,iRv2,iRv3,iRv4;

	BYTE bBuf[100];
	memset(bBuf, 0, sizeof(bBuf));

	WORD wPn = ByteToWord(pbBuf);
	
	if (wPn >= POINT_NUM)
		return -1;

	if (wPn != 0)
	{
		if ( IsPointOfGrp(wPn) ) //2009-09-02����Ч�ܼ���Ĳ���ɾ
			return 0;

		if ((iRv1=WriteItemEx(BN0, wPn, 0x8901, bBuf)) <= 0)//����������
			return -1;

		SetPnChgMask(wPn); //�ñ仯��־(���ӻ��޸�)	

		if ((iRv2=WriteItemEx(BN0, wPn, 0x4100, bBuf)) <= 0)//�������������
			return -1;
		if ((iRv3=WriteItemEx(BN0, wPn, 0x4101, bBuf)) <= 0)//��������ֵ����
			return -1;
		if ((iRv4=WriteItemEx(BN0, wPn, 0x4102, bBuf)) <= 0)//�������쳣��ֵ����
			return -1;
		return iRv1+iRv2+iRv3+iRv4;
	}
	else
	{
		for (WORD i=1; i<POINT_NUM; i++)
		{
			if ( IsPointOfGrp(i) ) //2009-09-02����Ч�ܼ���Ĳ���ɾ
				return 0;
		}

		for (WORD i=1; i<POINT_NUM; i++)
		{
			SetPnChgMask(i); //�ñ仯��־(���ӻ��޸�)	

			if ((iRv1=WriteItemEx(BN0, i, 0x8901, bBuf)) > 0)//����������
				iRet = 1;
			if ((iRv2=WriteItemEx(BN0, i, 0x4100, bBuf)) > 0)//�������������
				iRet = 1;
			if ((iRv3=WriteItemEx(BN0, i, 0x4101, bBuf)) > 0)//��������ֵ����
				iRet = 1;
			if ((iRv4=WriteItemEx(BN0, i, 0x4102, bBuf)) > 0)//�������쳣��ֵ����
				iRet = 1;
		}
		return iRet;
	}
}

//����:�������Ƿ���Ч
bool IsMtrValid(WORD wSn)
{
	if (wSn >= POINT_NUM)
		return false;
		
	BYTE bProp[50]={0};
	if (ReadItemEx(BN0, wSn, 0x8902, bProp) <= 0)
		return false;

	if (ByteToWord(bProp)!=wSn || wSn==0)
		return false;

	return true;
}

//����:��������Ƿ���Ч
bool IsPluseValid(WORD wSn)
{
	if (wSn >= PN_NUM)
		return false;
		
	BYTE bProp[50]={0};
	if (ReadItemEx(BN0, wSn, 0x8903, bProp) <= 0)
		return false;

	if (ByteToWord(bProp)!=wSn || wSn==0)
		return false;

	return true;
}

//����:����������Դ�Ƿ���Ч
bool IsSnParaValid(WORD wID, WORD wSn)
{
	if (wID == 0x8902)
		return IsMtrValid(wSn);
	else if (wID == 0x8903)
		return IsPluseValid(wSn);
	else
		return false;
}

//����:��ȡ������Ŷ�Ӧ��������Դ���
//����<0��ʾ�Ƿ�
int PnToMtrSn(WORD wPn)
{	
	if (wPn >= POINT_NUM)
		return -1;
		
	BYTE bProp[128]={0};
	if (ReadItemEx(BN0, wPn, 0x8901, bProp) <= 0)
		return -1;

	if (bProp[2]==PN_PROP_METER && bProp[3]==1)
		return ByteToWord(bProp+4);	
	
	return -1;
}

//����:��ȡ������Ŷ�Ӧ��������Դ�������
//����������Դ��ŵĸ�����<0��ʾ�Ƿ�
int PnToPluseSn(WORD wPn, WORD* pwSn)
{	
	if (wPn >= PN_NUM)
		return -1;
		
	BYTE bProp[128]={0};
	if (ReadItemEx(BN0, wPn, 0x8901, bProp) <= 0)
		return -1;

	if (bProp[2]==PN_PROP_PULSE && bProp[3]>=1 && bProp[3]<=4)
	{
		for (BYTE i=0; i<bProp[3]; i++)
			pwSn[i] = ByteToWord(bProp+4+2*i);
		return bProp[3];	
	}
	
	return -1;
}
//����:����Ƿ�֧�������ն���
bool IsMtrFrzSelf(WORD wPn)
{
	return IsV07Mtr(wPn);	
}



//�������������������ÿ���96�����ã�������������ʱ
void ClearCurveFrzFlg(WORD wPn)
{
	BYTE bIsSaveFlg[19];//�����ݿ�ÿ��96�����״̬
	TTime tNow;		
	GetCurTime(&tNow);

	memset(bIsSaveFlg, 0, 12);
	bIsSaveFlg[12] = (BYTE)(tNow.nYear-2000); 
	bIsSaveFlg[13] = tNow.nMonth;
	bIsSaveFlg[14] = tNow.nDay;

	WriteItemEx(BN0, wPn, 0xd881, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd889, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd901, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd905, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd945, bIsSaveFlg);	
	DTRACE(DB_DP, ("ClearCurveFrzFlg : day change has to clear wPn=%d\n", wPn));	
}

//���������ò����������
void SetPnRateNum(WORD wPn, BYTE bRateNum)
{ 
	WriteItemEx(BN0, wPn, 0x8911, &bRateNum);
#ifdef EN_CCT
	WORD wBn = CctGetPnBank(wPn);
	if (wBn != BN0)
		WriteItemEx(wBn, wPn, 0x8911, &bRateNum);
#endif
}

//����:��ȡ������ķ�����
BYTE GetPnRateNum(WORD wPn)
{
	BYTE bRateNum = RATE_NUM;	
#ifdef EN_CCT
	WORD wBn = CctGetPnBank(wPn);
#else
	WORD wBn = BN0;
#endif
	ReadItemEx(wBn, wPn, 0x8911, &bRateNum);	

	if (bRateNum==0 || bRateNum>RATE_NUM)
		bRateNum = RATE_NUM;

	return bRateNum;
}

//����:�Ƿ���ݵ���ص������ݵ�ʵ�ʳ����޸Ĳ�����ķ�����
bool IsChgRateNumByMtr()
{
	BYTE bVal = 0;
	ReadItemEx(BN10, PN0, 0xa1a1, &bVal);
	return ((bVal==0)?false:true);
}

//����:���ݴ�С�������07�����,�Գ�������ʱת�ɳ������㶳������
bool IsSinglePhaseV07Mtr(WORD wPn)
{
	wPn;
	return false;
}

//����:���ݴ�С�������97�����,�Գ�������ʱת�ɳ������㶳������
bool IsSinglePhaseV97Mtr(WORD wPn)
{
	wPn;
	return false;
}
//����:����Э���������ֻ֧�ֵ�����645Э�黹��֧�ֿ鳭��97��645Э��
//����:1Ϊ֧�ֿ鳭��97��645Э��,2Ϊֻ֧�ֵ�����645Э��.3Ϊ����Э��
BYTE IsSIDV97Mtr(WORD wPn)
{
	BYTE bBuf[128];
	//BYTE bProp = 0;	

	if (ReadItemEx(BN0, wPn, 0x8901, bBuf) <= 0)
		return 0;

	if (bBuf[2] != PN_PROP_METER)
		return 0;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return 0;	

	if (bBuf[10] == PROTOCOLNO_DLT645)
		return 1;
#ifdef PROTOCOLNO_DLT645_SID
	else if (bBuf[10] == PROTOCOLNO_DLT645_SID)
		return 2;
#endif

	return 1;	
}

//����:��������ǲ��������ܼ����
bool IsPointOfGrp(WORD wPn)
{
	if (wPn >= PN_NUM)
		return false;
		
	BYTE bBuf[50]={0};

	for (WORD i=1; i<GB_MAXSUMGROUP; i++)
	{
		ReadItemEx(BN0, i, 0x8905, bBuf);

		if (bBuf[1] != 0)
		{
			for (WORD j=0; j<bBuf[1]; j++)
			{
				if (ByteToWord(&bBuf[j*4+2]) == wPn) //�˲������������Ч�ܼ���
					return true;
			}
		}
	}

	return false;
}

//����:�ܼ����Ƿ������˿��Ʋ���
bool IsGrpOfCtrl(WORD wPn)
{
	BYTE bBuf[300];

	if (wPn==0 || wPn>=GB_MAXSUMGROUP)
		return false;

	for (WORD i=0; i<MAX_CTRLPARA_NUM; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));

		if (ReadItemEx(BN0, wPn, 0x5020+i, bBuf) > 0)
		{
			if (bBuf[0] != 0)	//���ò����Ŀ������Ͳ�Ϊ0���������		 		
				return true;	
		}
	}

	return false;
}
//����:�����㲨����	
DWORD GetMeterBaudRate(WORD wPn)
{	
	BYTE bPort = GetPnPort(wPn);
	int iPortFum = GetLogicPortFun(bPort);
	if (iPortFum == PORT_FUN_ACQ)  //��������485�ɼ�����,�����ʾ͹̶���.
	{
		BYTE bBuf[4];
		DWORD dwCBR = 0;		
		ReadItemEx(BN15, PN0, 0x5326, bBuf);
		dwCBR = NumToBaudrateTest(bBuf[0]);		
		return dwCBR;
	}


	BYTE bBuf[128];

	//BYTE bProp = 0;	
	if (ReadItemEx(BN0, wPn, 0x8901, bBuf) <= 0)
		return CBR_2400;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return CBR_2400;	

	DWORD dwBaudRate = GbValToBaudrate(bBuf[9]); //42	

	return dwBaudRate;
}
DWORD NumToBaudrateTest(BYTE n)
{
	switch (n)
	{
	case 0:
		return CBR_300;
	case 1:
		return CBR_600;
	case 2:   
		return CBR_1200;
	case 3:   
		return CBR_2400;
	case 4:
		return CBR_4800;	
	case 5:
		return CBR_9600;
	case 6:
		return CBR_19200;
	default:
		return CBR_9600;
	}

	return CBR_2400;
}

//����:�ÿ��Ʋ����Ƿ�Ͷ��
bool IsCtrlEnable()
{	
	return false;
}



//ȡ����Ч���������
WORD GetPnNum()
{
	BYTE bMtrMask[PN_MASK_SIZE];
	WORD wPnSun = 0;

	memset(bMtrMask, 0, sizeof(bMtrMask));
	memcpy(bMtrMask, GetMtrMask(BANK17, PN0, 0x6001), PN_MASK_SIZE);
	for (WORD wMtrMask=0; wMtrMask<PN_MASK_SIZE; wMtrMask++)
	{
		if (bMtrMask[wMtrMask] != 0)
		{
			for (BYTE bBit=0; bBit<8; bBit++)
			{
				if (bMtrMask[wMtrMask] & (1<<(bBit)))
				{
					wPnSun ++;
				}
			}
		}
	}
	return wPnSun;
}

TSem g_semRdMtrCtrl = NewSemaphore(1);

//���������ø��³�����ƽṹ������
void SetRdMtrCtrlMask(WORD wPn)
{
	BYTE bMtrCtrlMask[PN_MASK_SIZE];

	WaitSemaphore(g_semRdMtrCtrl);
	memset(bMtrCtrlMask, 0, sizeof(bMtrCtrlMask));
	ReadItemEx(BANK17, PN0, 0x6005, bMtrCtrlMask);
	bMtrCtrlMask[wPn/8] |= 1<<(wPn%8);
	WriteItemEx(BANK17, PN0, 0x6005, bMtrCtrlMask);
	DTRACE(DB_CRITICAL, ("SetRdMtrCtrlMask(): wPn=%d.\r\n", wPn));
	SetDelayInfo(INFO_MTR_UPDATE);
	SignalSemaphore(g_semRdMtrCtrl);
}

//��������������ƽṹ������
void DoRdMtrCtrlMask()
{
	WORD wPn;
	BYTE bMtrCtrlMask[PN_MASK_SIZE];

	WaitSemaphore(g_semRdMtrCtrl);
	memset(bMtrCtrlMask, 0, sizeof(bMtrCtrlMask));
	ReadItemEx(BANK17, PN0, 0x6005, bMtrCtrlMask);
	for (WORD i=0; i<PN_MASK_SIZE; i++)
	{
		if (bMtrCtrlMask[i])
		{
			for (BYTE j=0; j<8; j++)
			{
				if (bMtrCtrlMask[i] & (1<<j))
				{
					wPn = i*8 + j;
					DeleteOneMtrRdCtrl(wPn);
					DTRACE(DB_CRITICAL, ("DoRdMtrCtrlMask(): wPn=%d.\r\n", wPn));
				}
			}
		}
	}
	memset(bMtrCtrlMask, 0, sizeof(bMtrCtrlMask));
	WriteItemEx(BANK17, PN0, 0x6005, bMtrCtrlMask);
	SignalSemaphore(g_semRdMtrCtrl);
}

//�����������Ϣ�Ƚ��Ƿ���ͬ
//���أ���ͬ����true����֮false
bool MeterInfoCompare(WORD wPn, BYTE *pbBuf)
{
	BYTE bDbBuf[128];

	memset(bDbBuf, 0, sizeof(bDbBuf));
	if (ReadItemEx(BN0, wPn, 0x6000, bDbBuf)>0 && !IsAllAByte(bDbBuf, 0, sizeof(bDbBuf)))
	{
		if (bDbBuf[0]==pbBuf[0] && memcmp(bDbBuf, pbBuf, pbBuf[0]+1)==0)
			return true;
	}

	return false;
}


typedef struct {
	WORD wOI;	//��������ID
	WORD wDbId;	//��������ID��Ӧϵͳ���е�ID������
	const TSchFieldCfg *pSchFieldCfg;	//��Ӧ&g_TSchFieldCfg[]
}TOiMapSchId;

TOiMapSchId g_tOiMap[] = {	{0x6014, 0x6006, &g_TSchFieldCfg[0]},	//��ͨ�ɼ���������������
							{0x6016, 0x6007, &g_TSchFieldCfg[1]},	//�¼��ɼ�����
							{0x6051, 0x6008, &g_TSchFieldCfg[2]},	//͸���ɼ�����
							{0x601C, 0x6009, &g_TSchFieldCfg[3]},	//�ϱ��ɼ�����
							{0x6012, 0x600a, &g_TSchFieldCfg[4]}};

TSem g_semSchUdp = NewSemaphore(1);

void SetSchUpdateMask(WORD wOI, WORD wSchId)
{
	BYTE bBuf[TASK_NUM_MASK];

	WaitSemaphore(g_semSchUdp);
	for (BYTE i=0; i<sizeof(g_tOiMap)/sizeof(g_tOiMap[0]); i++)
	{
		if (g_tOiMap[i].wOI == wOI)
		{
			memset(bBuf, 0, sizeof(bBuf));
			ReadItemEx(BANK17, PN0, g_tOiMap[i].wDbId, bBuf);
			bBuf[wSchId/8] |= 1<<(wSchId%8);
			WriteItemEx(BANK17, PN0, g_tOiMap[i].wDbId, bBuf);
			break;
		}
	}
	SignalSemaphore(g_semSchUdp);
}

//������������Ӧ�������������Ӧ�ķ���
void ClearSchData()
{
	int iRet;
	WORD wSchNo;
	BYTE bBuf[TASK_NUM_MASK];
	char pszTabName[64];
	bool fSchUdp = false;
	bool fDelAllEvtFlg = true;

	WaitSemaphore(g_semSchUdp);
	DTRACE(DB_CRITICAL, ("ClearSchData(): clear sch data.\r\n"));
	for (BYTE index=0; index<sizeof(g_tOiMap)/sizeof(g_tOiMap[0]); index++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BANK17, PN0, g_tOiMap[index].wDbId, bBuf);
		if (!IsAllAByte(bBuf, 0, sizeof(bBuf)))
		{
			for (BYTE i=0; i<TASK_NUM_MASK; i++)
			{
				if (bBuf[i])
				{
					for (BYTE j=0; j<8; j++)
					{
						if (bBuf[i] & (1<<j))
						{
							wSchNo = i*8 + j;

							memset(pszTabName, 0, sizeof(pszTabName));
							if (g_tOiMap[index].pSchFieldCfg->bSchType == SCH_TYPE_EVENT)
							{
#ifndef SYS_WIN
								if (IsAllAByte(bBuf+1, 0xff, sizeof(bBuf)-2))	//�����ʾ��������¼�����
								{
									if (fDelAllEvtFlg)
									{
										fDelAllEvtFlg = false;
										memset(pszTabName, 0, sizeof(pszTabName));
										sprintf(pszTabName, "rm -rf %s%s_*", USER_DATA_PATH, g_tOiMap[index].pSchFieldCfg->pszTableName);
										system(pszTabName);
										DTRACE(DB_TASK, ("DelSchData all: %s.\n", pszTabName));
									}
								}
								else	//�����ʾ��յ����¼�����
								{
									fDelAllEvtFlg = false;
									memset(pszTabName, 0, sizeof(pszTabName));
									sprintf(pszTabName, "rm -rf %s%s_%03d_*", USER_DATA_PATH, g_tOiMap[index].pSchFieldCfg->pszTableName, wSchNo);
									system(pszTabName);
									DTRACE(DB_TASK, ("DelSchData one: %s.\n", pszTabName));
								}
#endif
								SchRefreshMtrRdCtrl((BYTE )wSchNo);
							}
							else if (g_tOiMap[index].pSchFieldCfg->bSchType == SCH_TYPE_COMM)
							{
								sprintf(pszTabName, "%s_%03d.dat", g_tOiMap[index].pSchFieldCfg->pszTableName, wSchNo);
								iRet = TdbClearRec(pszTabName); 
								DTRACE(DB_TASK, ("DelSchData: %s, iRet=%d.\n", pszTabName, iRet));
								SchRefreshMtrRdCtrl((BYTE )wSchNo);
							}
							else if (g_tOiMap[index].pSchFieldCfg->bSchType == SCH_TYPE_SCRIPT)
							{
								sprintf(pszTabName, "TaskCfgUnit_%03d.para", wSchNo);
								DTRACE(DB_TASK, ("DelSchData: %s.\n", pszTabName));
								TaskRefreshMtrRdCtrl((BYTE )wSchNo);
							}
							else if (g_tOiMap[index].pSchFieldCfg->bSchType == SCH_TYPE_REPORT)
							{
								//BANK16 0x6001~0x6008 Ϊ�ϱ���صĲ���
								for (WORD wID=0x6001; wID<=0x6008; wID++)
								{
									DWORD dwZero = 0;
									WriteItemEx(BANK16, wSchNo, wID, (BYTE*)&dwZero);
								}
							}

							fSchUdp = true;
						}
					}
				}
			}
		}
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BANK17, PN0, g_tOiMap[index].wDbId, bBuf);
	}

	if (fSchUdp)
	{
		BYTE bTaskSN = 0;
		ReadItemEx(BANK16, PN0, 0x6011, &bTaskSN);
		bTaskSN++;	
		WriteItemEx(BANK16, PN0, 0x6011, &bTaskSN);
	}

	SignalSemaphore(g_semSchUdp);
}

void ClearReportParam()
{
	DWORD dwZero = 0;

	//BANK16 0x6001~0x6008 Ϊ�ϱ���صĲ���
	for (WORD wID=0x6001; wID<=0x6008; wID++)
	{
		for (WORD i=0; i<TASK_NUM; i++)
			WriteItemEx(BANK16, i, wID, (BYTE*)&dwZero);
	}
}
