/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbHook.cpp
 * ժ    Ҫ�����ļ���Ҫ��������ϵͳ��Ĺҹ�/�ص�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 * ��    ע��$���ļ���Ҫ�������׼��ӿ�,�벻Ҫ������صĴ�����뵽���ļ�
 *			 $���ļ�����ĺ���,��ʽһ��,����������ͬ�汾�Ĳ�Ʒʱ,����������Ҫ�޸�
 *********************************************************************************************************/
#include "stdafx.h"
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "MeterAPI.h"
#include "DataManager.h"
#ifdef SYS_LINUX
#include "AcSample.h"
#include "Sample.h"
#endif
#define CMB_TO_SUB_ID_MAX	45
static WORD g_wCmbToSubID[][CMB_TO_SUB_ID_MAX] =		//DLMS BANK0�ڲ�ID��645ID��ӳ��
{
	{0x7000, 0x109f, 0x10Af, 0x110f, 0x111f}, //��ǰ�ܼ���/�޹����ʼ�ʣ�����	
	{0x7001, 0x8910, 0xb63f, 0xb64f, 0xb65f, 0xb61f, 0xb62f, 0xb6a0, 0xb67f,  0}, //��ǰ���༰����/�޹����ʡ����������������ѹ��������������������ڹ���
	{0x7002, 0x8910, 0xb31f, 0xb32f, 0xb33f, 0xb34f, 0},		 //A��B��C�������ͳ�����ݼ����һ�ζ����¼
//	{0x7003, 0x8910, 0xc701, 0xc700, 0xc870, 0xc810, 0xc811, 0xc820, 0xc821, 0xc830, 0xc831, 0xc840, 0xc841, 0xc850, 0xc851, 0}, //���ܱ�����ʱ�Ӽ����ܱ�״̬��Ϣ
	{0x7003, 0x8910, 0xc01f, 0xc870, 0xc810, 0xc811, 0xc820, 0xc821, 0xc830, 0xc831, 0xc840, 0xc841, 0xc850, 0xc851, 0}, //���ܱ�����ʱ�Ӽ����ܱ�״̬��Ϣ

	{0x7006, 0x8910, 0x901f, 0x902f, 0x911f, 0x912f, 0x913f, 0x915f, 0x916f, 0x914f, 0}, //��ǰ��/������/�޹�����ʾֵ��һ/�������޹�����ʾֵ���ܡ�����1~M��	
//	{0x7007, 0x8910, 0x941f, 0x942f, 0x951f, 0x952f, 0x953f, 0x955f, 0x956f, 0x954f, 0xc117, 0}, //��һ��������/������/�޹�����ʾֵ��һ/�������޹�����ʾֵ���ܡ�����1~M��	
	{0x7007, 0x8910, 0x941f, 0x942f, 0x951f, 0x952f, 0x953f, 0x955f, 0x956f, 0x954f, 0}, //��һ��������/������/�޹�����ʾֵ��һ/�������޹�����ʾֵ���ܡ�����1~M��	
	{0x7008, 0x8910, 0xa010, 0xb010, 0xa011, 0xb011, 0xa012, 0xb012, 0xa013, 0xb013, 0xa014, 0xb014,
	                 0xa020, 0xb020, 0xa021, 0xb021, 0xa022, 0xb022, 0xa023, 0xb023, 0xa024, 0xb024, 
					 0xa110, 0xb110, 0xa111, 0xb111, 0xa112, 0xb112, 0xa113, 0xb113, 0xa114, 0xb114, 
					 0xa120, 0xb120, 0xa121, 0xb121, 0xa122, 0xb122, 0xa123, 0xb123, 0xa124, 0xb124, 0}, //��������������/�޹��������������ʱ�䣨�ܡ�����1~M��
	{0x7009, 0x8910, 0xa410, 0xb410, 0xa411, 0xb411, 0xa412, 0xb412, 0xa413, 0xb413, 0xa414, 0xb414,
	                 0xa420, 0xb420, 0xa421, 0xb421, 0xa422, 0xb422, 0xa423, 0xb423, 0xa424, 0xb424, 
					 0xa510, 0xb510, 0xa511, 0xb511, 0xa512, 0xb512, 0xa513, 0xb513, 0xa514, 0xb514, 
					 0xa520, 0xb520, 0xa521, 0xb521, 0xa522, 0xb522, 0xa523, 0xb523, 0xa524, 0xb524, 0xc117, 0}, //��һ��������/������/�޹��������������ʱ�䣨�ܡ�����1~M��

//	{0x700a, 0x901f, 0x902f, 0x911f, 0x912f, 0x913f, 0x915f, 0x916f, 0x914f, 0},//�ն�����/������/�޹�����ʾֵ��һ/�������޹�����ʾֵ���ܡ�����1~M��	
//	{0x700a, 0x9a00, 0x9a1f, 0x9a2f, 0x9b1f, 0x9b2f, 0x9b3f, 0x9b5f, 0x9b6f, 0x9b4f, 0}, //�ն�����/������/�޹�����ʾֵ��һ/�������޹�����ʾֵ���ܡ�����1~M��	
	{0x700a, 0x9a1f, 0x9a2f, 0x9b1f, 0x9b2f, 0x9b3f, 0x9b5f, 0x9b6f, 0x9b4f, 0}, //�ն�����/������/�޹�����ʾֵ��һ/�������޹�����ʾֵ���ܡ�����1~M��	
	{0x700b, 0x8910, 0x941f, 0x942f, 0x951f, 0x952f, 0x953f, 0x955f, 0x956f, 0x954f, 0xc117, 0}, //��һ��������/������/�޹�����ʾֵ��һ/�������޹�����ʾֵ���ܡ�����1~M��----��ǰֵ�����˽���ʱ��0xc117	

	{0x7010, 0x33Af, 0x33Bf, 0x328F, 0x329F},		//�ܼ�����һ15�������ߵ�	
//	{0x7011, 0xb630, 0xb631, 0xb632, 0xb633, 0xb640, 0xb641, 0xb642, 0xb643, 0xb650, 0xb651, 0xb652, 0xb653, 0xb611, 0xb612, 0xb613, 0xb621, 0xb622, 0xb623, 0xb6a0, 0},//��������һ15����˲ʱ�����ߵ�	
	{0x7011, 0xb63f, 0xb64f, 0xb65f, 0xb61f, 0xb62f, 0xb6a0, 0},//��������һ15����˲ʱ�����ߵ�	
	{0x7012, 0x9010, 0x9020, 0x9110, 0x9120, 0},	//��������һ15���ӵ���ʾֵ���ߵ�
	{0x7013, 0x32af, 0x32cf, 0x32bf, 0x32df, 0},	//��������һ15���ӵ������ߵ�	

	{0x7221, 0x8910, 0xc9c1, 0xc9b0, 0xc9c5, 0xc990, 0xc991, 0xc9a5, 0xc9d0, 0xc9d1, 0xc9d2, 0},	//F167���ܱ����õ���Ϣ	

	{0x7020, 0x10BF, 0x10CF, 0},					//�ܼ��鵱�յ���
	{0x7030, 0x10DF, 0x10EF, 0},					//�ܼ��鵱�µ���
	{0x7120, 0x321f, 0x322f, 0},					//�ܼ�����һ�յ���	
	{0x7130, 0x324f, 0x325f, 0},					//�ܼ�����һ�µ���	
	{0x7021, 0x11DF, 0x11EF, 0x120F, 0x121F, 0},	//�����㵱�յ���
	{0x7031, 0x122F, 0x123F, 0x124F, 0x125F, 0},	//�����㵱�µ���
	{0x7121, 0x300f, 0x302f, 0x301f, 0x303f, 0},	//��������һ�յ���,Delat��
	{0x7131, 0x304f, 0x306f, 0x305f, 0x307f, 0},	//��������һ�µ���,Delat��
	{0x7220, 0x371f, 0},							//ֱ��ģ�����Ĳɼ�
	{0x5512, 0x1500, 0x1501, 0},					//���յ���ͨ������
	//{0x5514, 0x5510, 0x5511, 0x5513, 0x1500, 0x1501, 0},	//�ն�״̬
};


//����:����������Ƿ�������Ч
//���ø�ʽ���£�
//02 04 
//		12 00 04 
//		02 07 
//			85 06 10 00 00 00 22 24 
//			16 03 
//			16 03 
bool IsPnValid(WORD wPn)
{
	BYTE bBuf[PNPARA_LEN+1];
	if (ReadItemEx(BN0, wPn, 0x6000,bBuf) > 0)
	{
		WORD wSn = OoOiToWord(&bBuf[4]);//((WORD )bBuf[4]<<8)+bBuf[3];
		BYTE bAddrLen = bBuf[9];
		if (wSn>0 && !IsAllAByte(&bBuf[9], 0x00, bAddrLen))
			return true;
	}
	
	return false;
}

//����:ȡ���ID����ID��ӳ������
WORD* CmbToSubID(WORD wBn, WORD wID)
{
	if (wBn!=BN0
#ifdef EN_CCT	//����������
		&& wBn!=CCT_BN_SPM && wBn!=CCT_BN_MFM	//�����õ���BANK
#endif //EN_CCT
		)
		return NULL;
		
	WORD wNum = sizeof(g_wCmbToSubID) / (sizeof(WORD)*CMB_TO_SUB_ID_MAX);
	for (WORD i=0; i<wNum; i++)
	{
		if (wID == g_wCmbToSubID[i][0])
		{
			return &g_wCmbToSubID[i][1];
		}
	}
	
	return NULL;
}


//����:ȡ���ID����ID�ĸ���
WORD CmbToSubIdNum(WORD wBn, WORD wID)
{
	WORD* pwSubID = CmbToSubID(wBn, wID);
	if (pwSubID == NULL)
		return 1;
	
	WORD wNum = 0;	
	while (*pwSubID++ != 0)	//�����IDת�������ζ���ID�Ķ�
		wNum++;
	
	return wNum;
}

int PostWriteItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, BYTE bPerm, BYTE* pbPassword, int nRet)
{
	//BYTE bBuf[100], bOldBuf[100], bProp, bOldProp, bProType, bNum, bPn, bOldPn, bPulseProp;
	//WORD wNum, wSn, wPoint, wUsePn;//wPointΪ�·��Ĳ�����ţ�wObjΪ��ţ�wUsePnΪ��Ŷ�Ӧ���Ѿ�ʹ�õĲ������
	//BYTE* pbTmp;
	//WORD i, j, n, step, wFindObj;
	//bool fSame, fChg;
	BYTE bTmpBuf[80];

	if (wBank==BN0 && wID==0x4001)	//�ն˵�ַ
	{		
		if (GetItemLen(BN10, 0xa1d0) > 0)
		{
			if (pbBuf[1] > TSA_LEN-2)	//pbBuf[1]ΪOCTSTRING���ȣ�TSA_LEN-2=15
				pbBuf[1] = TSA_LEN-2;

			memset(bTmpBuf, 0, sizeof(bTmpBuf));
			memcpy(bTmpBuf, pbBuf, pbBuf[1]+2);		//+2, ����1Byte + ����1Byte
			WriteItemEx(BN10, PN0, 0xa1d0, bTmpBuf);
		}
	}
	#ifdef SYS_LINUX
	else if(wBank==BN2 && wID == 0x5039)
	{
		AcSetPulseRatio(pbBuf);//���ܵ���InitSample();
		TraceBuf(DB_FAPROTO,"\r\nPostWriteItemExHook: wBank=BN2,wID=0x5039,pbBuf= ",pbBuf,1);

	}
	else if((wBank==BN25 && 
				((wID >= 0x5001 && wID <= 0x5005)
				  ||(wID == 0x500f) 
				  )
			)
			||
			(wBank==BN28 && 
				((wID >= 0x0011 && wID <= 0x0019)
				  ||(wID == 0x001f) 
				  )
			)
			
		  )
	{// ���ɲ����仯
		BYTE bCnt = 0;
		AcSetPulseRatio(&bCnt);//���ܵ���InitSample();
//			InitSample();
//			DTRACE(DB_FAPROTO,("PostWriteItemExHook: AC_PARA change-> InitSample\r\n"));
	}
	#endif

	return nRet;
}

//����:��������Ĺҹ�,������������Զ�����,����һЩ�Ǳ�׼�Ĳ���
//����:@nRet �ڵ���ReadItemEx()ʱ�ķ���ֵ,Ӧ��Ϊ����
int PostReadItemExHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, int nRet)
{
	BYTE bBuf[90]={0};
	//BYTE* pbTmp = pbBuf;
	WORD j;
	int len;
	DWORD dwIP = 0;
	BYTE  bConnType = 1;
	
	if (wBank==BN0 && wID==0x4010) //SAPҪ���߼��豸�������
	{
		/*WORD wSap;

		pbBuf[0] = 2;
		wSap = 1;
		memcpy(pbBuf+1, (BYTE*)&wSap, 2);
		ReadItemEx(BN0, PN0, 0x4000, pbBuf+3); //�����߼��豸��
		wSap = 17;
		memcpy(pbBuf+19, (BYTE*)&wSap, 2);
		ReadItemEx(BN0, PN0, 0x4001, pbBuf+21); //�����߼��豸��*/
	}
	else if (wBank==BN0 && wID==0x100f) //�汾��Ϣ����ն��豸��Ҫ������豸�������
	{
		ReadItemEx(BN0, PN0, 0x4000, pbBuf); //�滻�����߼��豸��
	}
	else if (wBank==BN0 && wID==0x4001)	//�ն˵�ַ
	{
		ReadItemEx(BN10, PN0, 0xa1d0, pbBuf);
	}
	else if (wBank==BN0 && wID==0x4507)	//�ź�ǿ��
	{
		ReadItemEx(BN2, PN0, 0x6003, pbBuf);
	}
	else if (wBank==BN0 && wID==0x4509)	//GPRS���ű���IP
	{		
		ReadItemEx(BN1, PN0, 0x2032, &bConnType);
		if(bConnType == 1)
		{
#ifndef SYS_WIN
			dwIP = GetLocalAddr("ppp0");
#endif

			pbBuf[0] = DT_OCT_STR;
			pbBuf[1] = 0x04;
			pbBuf[2] = (BYTE)dwIP&0xff;
			pbBuf[3] = (BYTE)(dwIP>>8)&0xff;
			pbBuf[4] = (BYTE)(dwIP>>16)&0xff;
			pbBuf[5] = (BYTE)(dwIP>>24)&0xff;
		}
		else
		{
			ReadItemEx(BN2, PN0, 0x2055, bBuf);

			pbBuf[0] = DT_OCT_STR;
			pbBuf[1] = 0x04;
			memcpy(&pbBuf[2], bBuf, 4);
			
		}	
	}

	return nRet;
}


//����:�����������Ĺҹ�,������������Զ�����,����һЩ�Ǳ�׼�Ĳ���
//����:@nRet �ڵ���ReadItemEx()ʱ�ķ���ֵ,Ӧ��Ϊ����
int PostReadCmbIdHook(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime, int nRet)
{
	//NOTICE:Ŀǰ���е�wID������g_wCmbToSubID[],����ʱ����wID�����ж�
	if (wID == 0x126f) //��λ��û�г���ʱ��
		return nRet;	

	WORD* pwSubID = CmbToSubID(wBank, wID);	

	//Ŀǰ�����ն˳���ʱ��ֱ���滻Ϊ��������������ʱ��
	TTime time;
	SecondsToTime(dwTime, &time);	
	if (*pwSubID==0x8910)
	{
		GetCurTime(&time);
		TimeToDateTime(time, pbBuf);	
	}	

	return nRet;
}

//����:�����������Ĺҹ�,������������Զ�����,����һЩ�Ǳ�׼�Ĳ���
//����:@nRet �ڵ���ReadItemEx()ʱ�ķ���ֵ,Ӧ��Ϊ����
int PostReadItemValHook(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD dwTime, int nRet)
{
	//NOTICE:Ŀǰ���е�wID������g_wCmbToSubID[],����ʱ����wID�����ж�
	if (wID == 0x126f) //��λ��û�г���ʱ��
		return nRet;

	*piVal32 = dwTime;
	
	return nRet;
}

//����:�����������Ĺҹ�,������������Զ�����,����һЩ�Ǳ�׼�Ĳ���
//����:@nRet �ڵ���ReadItemEx()ʱ�ķ���ֵ,Ӧ��Ϊ����
int PostReadItemVal64Hook(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD dwTime, int nRet)
{
	//NOTICE:Ŀǰ���е�wID������g_wCmbToSubID[],����ʱ����wID�����ж�
	if (wID == 0x126f) //��λ��û�г���ʱ��
		return nRet;

	*piVal64 = dwTime;
	
	return nRet;
}


bool PswCheck(BYTE bPerm, BYTE* pbPassword)
{
	/*
	if (!IsTimeEmpty(g_tmAccessDenied))
	{										//д������24Сʱ
		TTime now;
		GetCurTime(&now);
		if (MinutesPast(g_tmAccessDenied, now) < 24*60)
			return false;
		else								//����ʱ�����
			SetEmptyTime(&g_tmAccessDenied);
	}
	
	BYTE bPassword[3];
	if (bPerm == DI_LOW_PERM)
	{
        g_DataManager.ReadTermnPara(0x8021, bPassword);   //�ͼ�Ȩ������
	}
	else  //DI_HIGH_PERM
	{
		g_DataManager.ReadTermnPara(0x8022, bPassword);   //�߼�Ȩ������
	}

    if (pbPassword[0]!=bPassword[0] || pbPassword[1]!=bPassword[1] || pbPassword[2]!=bPassword[2])
	{
		g_wAccessDeniedCnt++;
		if (g_wAccessDeniedCnt >= 3)
		{
			GetCurTime(&g_tmAccessDenied);
			g_wAccessDeniedCnt = 0;
		}
		
	    return false;
	}
	
	g_wAccessDeniedCnt = 0;   //ֻҪ������ȷ,�����������
	*/

	return true;
}


int PermCheck(TItemDesc* pItemDesc, BYTE bPerm, BYTE* pbPassword)
{
	if (pbPassword == NULL)   //����Ѵ�����ĵ��������������ã���Ҫ��֤���룬��ϵͳ�Ķ�д�����κ�����
		return ERR_OK;
		
	/*if (!IsTimeEmpty(g_tmAccessDenied))
	{										//д������24Сʱ
		TTime now;
		GetCurTime(&now);
		if (MinutesPast(g_tmAccessDenied, now) < 24*60)
			return -(ERR_PERM + (int )pItemDesc->wLen*0x100);
		else								//����ʱ�����
			SetEmptyTime(&g_tmAccessDenied);
	}*/
	
	
	if (bPerm==DI_LOW_PERM && pItemDesc->wPerm==DI_HIGH_PERM)   //��ʹ�õͼ�Ȩ��������ʸ߼�Ȩ������
	{
		return -(ERR_PERM + (int )pItemDesc->wLen*0x100);
	}

	if ((pItemDesc->wRW & DI_WRITE) != DI_WRITE)   //������д
	{
		return -(ERR_INVALID + (int )pItemDesc->wLen*0x100); //û��ĸ����ʵĴ�����룬ֻ�����������ݷǷ�
	}


	/*BYTE bPassword[3];
	if (bPerm == DI_LOW_PERM)
	{
        g_DataManager.ReadTermnPara(0x8021, bPassword);   //�ͼ�Ȩ������
	}
	else  //DI_HIGH_PERM
	{
		g_DataManager.ReadTermnPara(0x8022, bPassword);   //�߼�Ȩ������
	}

    if (pbPassword[0]!=bPassword[0] || pbPassword[1]!=bPassword[1] || pbPassword[2]!=bPassword[2])
	{
		g_wAccessDeniedCnt++;
		if (g_wAccessDeniedCnt >= 3)
		{
			GetCurTime(&g_tmAccessDenied);
			g_wAccessDeniedCnt = 0;
		}
		
	    return -(ERR_PERM + (int )pItemDesc->wLen*0x100);
	}*/
	
	if (!PswCheck(bPerm, pbPassword))
	{
		return -(ERR_PERM + (int )pItemDesc->wLen*0x100);
	}
	
	g_dwWrOpClick = GetClick();
	
	return ERR_OK;
}

bool IsSectHaveSampleData(WORD wSect)
{
#if 0
	if (wSect == SECT_PN_DATA)
		return true;
	else
#endif
		return false;

}


//����:���ݲ�ͬ�Ĵ�����bErr����ȡ��ϵͳ����Ч���ݵĶ���
//��ע:���罭�չ涨Ҫ���ֲ�֧��������(0xee)�ͳ���ʧ��������(0xef)
BYTE GetInvalidData(BYTE bErr)
{
	return INVALID_DATA;
}

//����:�Ƿ�����Ч���ݣ���Ч���ݿ��ܴ��ڶ��ֶ���
bool IsInvalidData(BYTE* p, WORD wLen)
{
	if (IsAllAByte(p, INVALID_DATA, wLen))
		return true;

	return false;
}

//����:��ȡ��ϵͳ�ĵ�������ļ���·��
void GetMtrProCfgPath(char* pbCfgPath)
{
	sprintf(pbCfgPath,"%s", USER_CFG_PATH);	
}

//����:���ݿ������õ�Ĭ��ת������
//����:iPnNum ���°汾ϵͳ����ȡ��wToId�Ĳ��������
void DefaultUpgFun(WORD wFrmBn, WORD wFrmId, WORD wToBn, WORD wToId, int iPnNum)
{
	BYTE bBuf[1024];
	if (iPnNum <= 0)
		return;

	DWORD dwTime;
	for (WORD wPn=0; wPn<iPnNum; wPn++)
	{
		dwTime = 0;
		if (UpgReadItem(wFrmBn, wPn, wFrmId, bBuf, &dwTime) > 0)
		{
			WriteItemEx(wToBn, wPn, wToId, bBuf, dwTime);
		}
	}
}

//����:��ȡ��ϵͳ����Ч���ݵĶ���
BYTE GetDbInvalidData()
{
	return INVALID_DATA;	
}
