/*********************************************************************************************************
* Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�MeterPro.h
* ժ    Ҫ�����ļ���Ҫ��������Э��Ļ�������������ʵ��
* ��ǰ�汾��1.0
* ��    �ߣ������
* ������ڣ�2016��8��
* ��    ע��
*********************************************************************************************************/

#include "stdafx.h"
#include "MeterPro.h"
#include "DbAPI.h"
#include "FaAPI.h"
#include "MtrCtrl.h"

#define VER_STR	"Ver1.0.0"

const WORD CRC_16[] = 
	{0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 
	 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 
	 0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D, 
	 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 
	 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 
	 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 
	 0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 
	 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 
	 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067, 
	 0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 
	 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
	 0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, 
	 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, 
	 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 
	 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 
	 0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
	};

////////////////////////////////////////////////////////////////////////////////////////////
//MtrProIf˽�г�Ա����
BYTE m_bInvdData; 	 //ϵͳ��Ч���ݵĶ���
//�����շ�����
BYTE m_MtrTxBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];
BYTE m_MtrRxBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];
BYTE m_CurveBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];

void InitMeterPro()
{
	DTRACE(DB_CRITICAL, ("InitMeterPro: "VER_STR" init ok.\r\n"));
	m_bInvdData = GetInvalidData(INVALID_DATA);
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool MtrProOpenComm(CComm* pComm, TCommPara* pCommPara)
{
	bool fCommOpen = false;
	TCommPara CommPara;

	if (pComm == NULL)
		return false;

	pComm->SetTimeouts(1000); //����ȱʡ��ʱ��1S,�Է���ĳЩ��Э��ĵ��ܶ���

	if (pComm->IsOpen())
	{
		if (pComm->GetCommPara(&CommPara)) //�����ڲ����м�⴮���Ƿ��
		{
			fCommOpen = true;
			if (pCommPara->wPort==CommPara.wPort
				&& pCommPara->dwBaudRate==CommPara.dwBaudRate
				&& pCommPara->bByteSize==CommPara.bByteSize
				&& pCommPara->bParity==CommPara.bParity
				&& pCommPara->bStopBits==CommPara.bStopBits)
			{
				return true;	//���ڲ�������ͬ������²��������ٴ򿪴���			
			}
		}
	}

	if (fCommOpen) 
	{
		if (!pComm->Close())
		{
			DTRACE(DB_METER, ("CMeterPro::OpenComm : fail to close COM=%d.\r\n", pCommPara->wPort));
			return false;
		}
	}

	memcpy(&CommPara, pCommPara, sizeof(CommPara));
	if (pComm->Open(CommPara.wPort, CommPara.dwBaudRate, CommPara.bByteSize, CommPara.bStopBits, CommPara.bParity))
	{
		return true;
	}
	else
	{
		DTRACE(DB_METER, ("CMeterPro::OpenComm : fail to open COM=%d.\r\n", pCommPara->wPort));
		return false;
	}
}

DWORD MtrProSend(CComm* pComm, BYTE* pbTxBuf, DWORD wLen)
{
	if (pComm==NULL || wLen==0)
		return 0;

	TraceBuf(DB_645FRM, "MtrProSend --> ", pbTxBuf, wLen);

	if (pComm->Write(pbTxBuf, wLen) != wLen)
		return 0;

	return wLen;
}

DWORD MtrProRcv(CComm* pComm, BYTE* pbRxBuf, DWORD dwBufSize)
{
	if (pComm == NULL)
		return 0;

	DWORD dwTmpClick;
	DWORD dwNewClick;	
	DWORD dwLen = 0;
	DWORD dwPtr = 0;
	BYTE bBuf[MTR_FRM_SIZE];

	dwTmpClick = GetClick();
	dwNewClick = dwTmpClick;
	while (dwNewClick-dwTmpClick < 3)    //n�γ��Զ�ȡ����
	{
		dwLen = pComm->Read(bBuf, MTR_FRM_SIZE, 200);

		if ((dwLen+dwPtr) >= dwBufSize)
		{
			DTRACE(DB_645FRM, ("MtrProRcv ReadComm Buffer not enough!\r\n"));
			break;
		}
		else
			memcpy(pbRxBuf+dwPtr, bBuf, dwLen);

		dwPtr += dwLen;

		dwNewClick = GetClick();
	}

	TraceBuf(DB_645FRM, "MtrProRcv <-- ", pbRxBuf, dwPtr);

	return dwPtr;
}

WORD GetCRC16(BYTE* pbyt, int iCount)
{
    WORD wCRC = 0;
	int i;

    for (i=0; i<iCount; i++)
	{
        wCRC = ((WORD)(wCRC<<8)) ^ (CRC_16[(wCRC>>8) ^ pbyt[i]]);
    }
    
    return wCRC;
}

void CheckRate(BYTE* pbRateTab, BYTE* pbData, BYTE nlen)
{
	BYTE tBuf[50];
	memset (tBuf, m_bInvdData, 50);
	if (pbRateTab[0]!=0 && pbRateTab[0]<5)
		memcpy(tBuf+(pbRateTab[0]-1)*nlen, pbData, nlen);
	if (pbRateTab[1]!=0 && pbRateTab[1]<5)
		memcpy(tBuf+(pbRateTab[1]-1)*nlen, pbData+nlen, nlen);
	if (pbRateTab[2]!=0 && pbRateTab[2]<5)
		memcpy(tBuf+(pbRateTab[2]-1)*nlen, pbData+nlen*2, nlen);
	if (pbRateTab[3]!=0 && pbRateTab[3]<5)
		memcpy(tBuf+(pbRateTab[3]-1)*nlen, pbData+nlen*3, nlen);
	memcpy(pbData, tBuf, 4*nlen);
}

//�������Ƿ��������ݣ�������������ʱ�䣩
//�������Ƿ��ն�������ID
bool IsDayFrzDemdId(WORD wID)
{
	if ((wID>>4)>=0x9c0 && (wID>>4)<=0x9c3)
		return true;
	else
		return false;
}

//�������Ƿ��ն�������ʱ��
bool IsDayFrzDemdTime(WORD wID)
{
	if ((wID>>4)>=0x9c8 && (wID>>4)<=0x9cb)
		return true;
	else
		return false;
}

bool IsDemdId(WORD wID)
{
	if ((wID>>8)==0xa0 || (wID>>8)==0xa1
		|| (wID>>8)==0xa4 || (wID>>8)==0xa5
		|| (wID>>8)==0xb0 || (wID>>8)==0xb1
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5
		|| IsDayFrzDemdId(wID) || IsDayFrzDemdTime(wID)) //�ն�������
		return true;
	else 
		return false;
}

//�������Ƿ���������
bool IsLastMonthId(WORD wID)
{
	if ((wID>>8)==0x94 || (wID>>8)==0x95
		|| (wID>>8)==0xa4 || (wID>>8)==0xa5
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5)
		return true;
	else 
		return false;
}

//�������Ƿ�����ʱ��
bool IsDemdTime(WORD wID)
{
	if ((wID>>8)==0xb0 || (wID>>8)==0xb1		
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5
		|| IsDayFrzDemdTime(wID))
		return true;
	else 
		return false;
}

//�Ƿ�������йصļ���ID
bool  IsRateId(WORD wID)
{
	if ((wID>>8)==0x90 || (wID>>8)==0x91		
		|| (wID>>8)==0xa0 || (wID>>8)==0xa1
		|| (wID>>8)==0xb0 || (wID>>8)==0xb1
		|| (wID>>8)==0x94 || (wID>>8)==0x95
		|| (wID>>8)==0xa4 || (wID>>8)==0xa5
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5
		|| (wID>>8)==0x98 || (wID>>8)==0x99
		|| (wID>>8)==0xa8 || (wID>>8)==0xa9
		|| (wID>>8)==0xb8 || (wID>>8)==0xb9)
		return true;
	else 
		return false;
}

BYTE Get645TypeLength(WORD wID)
{
	BYTE len = 0;

	if ((wID>>12)==0x9 && !IsDayFrzDemdId(wID) && !IsDayFrzDemdTime(wID))	//����
	{
		len = 4;
	}
	else if ((wID>>12)==0xa || IsDayFrzDemdId(wID))	//����
	{
		len = 3;
	}
	else if ((wID>>4)==0xB61 //��ѹ
		|| (wID>>4)==0xB62  //����
		|| (wID>>4)==0xB64  //�޹�����
		|| (wID>>4)==0xB65	//��������
		|| (wID>>4)==0xB31	//�������
		|| wID==0xB212 || wID==0xB213 //��̴���
		|| wID==0xC117 || wID==0xC211 || wID==0xC511) //�Զ�������
	{
		len = 2;
	}
	else if ((wID>>4)==0xB63  //�й�����
		|| (wID>>4)==0xB67	//���ڹ���
		|| (wID>>4)==0xB32	//����ʱ��
		|| wID==0xB214	//���ʱ��
		|| wID==0xC011	//ʱ��
		|| wID==0xC030 || wID==0xC031)//���峣��   
	{
		len = 3;
	}
	else  if ( IsDemdTime(wID) )  //����ʱ��
	{
		len = 4;
	}
	else if ((wID>>4)==0xB33  //����ʱ��
		|| (wID>>4)==0xB34
		|| wID==0xC010 //����
		|| wID==0xB210 || wID==0xB211 //���ʱ��  
		|| wID==0xC119 || wID==0xC11A //��/�޹�������ʼ����
		|| wID==0xC212 || wID==0xC510)
	{
		len = 4;
	}
	else if ((wID>>4)==0xC02 //״̬��
		|| (wID>>4)==0xC31 //��ʱ��
		|| (wID>=0xC111 && wID<=0xC118 && wID!=0xC117)
		|| wID==0xC41E)
	{
		len = 1;
	}
	else if (((wID>>4)>=0xc32 && (wID>>4)<=0xc3a) || (wID>>4)==0xC41) //���ʲ���
	{
		len = 3;
	}
	else if (wID>=0xc032 && wID<=0xc034)
	{
		len = 6;
	}
	else if (wID == 0xC040)
	{
		len = 7;
	}

	return len;
}

WORD  SetCommDefault(WORD wID, BYTE* pbBuf)
{
	BYTE num = 0;
	WORD i, len = 0;

	if ((wID&0xf) == 0xf)
		num = GetBlockIdNum(wID);
	else 
		num = 1;

	if ((wID>>12)==0x9 && !IsDayFrzDemdId(wID) && !IsDayFrzDemdTime(wID)) //����
	{		
		//SetArrVal64((int64*)pbBuf, INVALID_VAL64, num);		
		int64 DDTmpVal[TOTAL_RATE_NUM];
		len = num*sizeof(int64);
		for (i=0; i<num; i++)
			DDTmpVal[i] = INVALID_VAL64;
		memcpy(pbBuf, &DDTmpVal, len);
	}
	else if ((wID>>12)==0xa || IsDayFrzDemdId(wID))	//����DWORD
	{		
		//SetArrVal32((int32*)pbBuf, INVALID_VAL, num);	
		int32 Int32TmpVal[TOTAL_RATE_NUM];
		len = num*sizeof(int32);
		for (i=0; i<num; i++)
			Int32TmpVal[i] = INVALID_VAL;
		memcpy(pbBuf, &Int32TmpVal, len);
	}
	else if ((wID>>4)==0xB61  //��ѹ
		|| (wID>>4)==0xB62  //����
		|| (wID>>4)==0xB63  //�й�����
		|| (wID>>4)==0xB64  //�޹�����
		|| (wID>>4)==0xB65 //��������
		|| (wID>>4)==0xB66 //��λ��
		|| (wID>>4)==0xB67) //���ڹ���
	{
		//SetArrVal32((int32*)pbBuf, INVALID_VAL, num);	
		int32 Int32TmpVal[6];
		len = num*sizeof(int32);
		for (i=0; i<num; i++)
			Int32TmpVal[i] = INVALID_VAL;
		memcpy(pbBuf, &Int32TmpVal, len);
	}
	else  if ( IsDemdTime(wID) )  //����ʱ��
	{	
		len = num*4;
		memset(pbBuf, (BYTE)m_bInvdData, len);			
	}
	else if (wID == 0xc010)
	{	
		len = 3;
		memset(pbBuf, (BYTE)m_bInvdData, len);
	}
	else if (wID == 0xc011)
	{		
		len = 4;
		memset(pbBuf, (BYTE)m_bInvdData, len);
	}
 
	return len;

}

float POW10(signed char n)//10��n�η�
{
	float f = 1;
	BYTE sign = 0;
	int i;
	
	if (n < 0)
	{
		n = (0xff-n)+1;
		sign = 1;
	}	
	
	for (i=0; i<n; i++)
	{
		if (sign == 1)
			f = f/10;
		else 
			f = f*10;
	}	
	
	return f;
}

int64 POW10N(signed char n)//10��n�η���n> 0��
{
	int64 iVal64 = 1, i;	
	for (i=0; i<n; i++)
	{
		iVal64 = iVal64*10;
	}	
	
	return iVal64;
}

int64 Round(int64 iVal64)
{
	return (iVal64%10<5) ? iVal64/10:(iVal64/10 + 1); //����//����	
}

void CheckDecimal(BYTE bToltLen, BYTE bItemLen, BYTE bNewDec, BYTE bOldDec, BYTE* pbBuf)
{
	int64 iVal64;
	float f;
	BYTE i,j;

	if (bOldDec == bNewDec) //���С��λ��ȱʡ����ͬ����ת��
		return;
    if (bItemLen == 0)
        return;
	for (i=0; i<bToltLen/bItemLen; i++)
	{
		if ( !IsBcdCode(pbBuf+i*bItemLen, bItemLen) ) //�зǷ�����
			continue;

		iVal64 = 0;
		for (j=0; j<bItemLen; j++)
		{
			iVal64 = iVal64+(((*(pbBuf+bItemLen*i+j))&0xf) + ((*(pbBuf+bItemLen*i+j))>>4)*10)*POW10N(2*j);  
		}	

		if( bOldDec >= bNewDec )
			iVal64 = iVal64*POW10N(bOldDec-bNewDec);
		else
			f = iVal64*POW10(bOldDec-bNewDec);

		if (bOldDec != bNewDec)
		{
			if (bOldDec >= bNewDec)
			{				
				iVal64 = Round(iVal64*10);
			}
			else
			{
				iVal64 = Round((DWORD)(f*10));				
			}
		}
		DWORDToBCD((DWORD)iVal64, pbBuf+bItemLen*i, bItemLen);	
	}
}

void CheckDecimalNew(BYTE bDstLen, BYTE bSrcLen, BYTE bDstDec, BYTE bSrcDec, BYTE* pbDstBuf, BYTE* pbSrcBuf)
{
	uint64 iVal64;
	float f;	
	BYTE j;

	if (bDstLen==bSrcLen && bDstDec==bSrcDec)
	{
		memcpy(pbDstBuf, pbSrcBuf, bSrcLen);
		return;
	}
	
	iVal64 = 0;
	for (j=0; j<bSrcLen; j++)
	{
		iVal64 = iVal64+(((*(pbSrcBuf+j))&0xf) + ((*(pbSrcBuf+j))>>4)*10)*POW10N(2*j);  
	}	

	if (bDstDec >= bSrcDec)
	{
		iVal64 = iVal64*POW10N(bDstDec-bSrcDec);
		iVal64 = Round(iVal64*10);	
	}
	else
	{
		f = iVal64*POW10(bDstDec-bSrcDec);
		iVal64 = Round((DWORD)(f*10));
	}	

	Uint64ToBCD(iVal64, pbDstBuf, bDstLen);		
}

//����:	���ַ������ҳ���Ӧ�ַ���λ��
//����:	@pStr		��Ҫ���ҵ�Ŀ���ַ���
//		@iStrLen	Ŀ���ַ����ĳ���
//		@c			��־�ַ�
//����: ����λ�� -1 ��ʾ����ʧ��
int SchStrPos(char* pStr, int iStrLen, char c)
{
	int i;
	for (i=0; i<iStrLen; i++)
	{
		if (pStr[i] == c)
			break;
	}

	if (i == iStrLen)
		return -1;
	else 
		return i;
}

//����������DLT645���ݸ�ʽ��ͨ�����ݸ�ʽ��ת��
//������@wID 		������ID��
//		@pbBuf 		ָ�����������ָ��, ��Ϊ���������Ҳ�����������
//      @wLen		����645��ʽ���ݵĳ���
//���أ�ת���ɹ��򷵻�ͨ�����ݵĳ���.���򷵻�0
WORD Data645ToComm(WORD wID, BYTE* pbBuf, WORD wLen)
{		
	WORD i;	
	WORD tLen = 0;//���ܳ�
	WORD oLen = 0;//ԭ��ID��
	WORD nLen = 0;//����ID��
	DWORD dwVal;

	int32 iVal32;
	uint64 iVal64;	
	BYTE bItemLen;

	BYTE mBuf[80];
	memset(mBuf, 0, 80);		
	
	switch (wID>>8)
	{
		case 0x90://�ɼ��ݷ������
		case 0x91:			
		case 0x94:
		case 0x95://����+2С��λ
		case 0x9a:			
		case 0x9b://�ն���ĵ���
				oLen = 4;
				nLen = oLen+1;
				for (i=0; i<wLen/oLen; i++)
				{					
					if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����	
					{
						memset(&mBuf[i*nLen], m_bInvdData, nLen);
						continue;
					}									
					CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 2, &mBuf[i*nLen], &pbBuf[i*oLen]);				
				}
				tLen = i*nLen;
				memcpy(pbBuf, mBuf, tLen);
				break;
		case 0x9c:	//�ն������������������ʱ��
			if ( IsDayFrzDemdId(wID) )			
				oLen = 3;			
			else if ( IsDayFrzDemdTime(wID) )			
				oLen = 4;			
			nLen = oLen;

			if ( IsDayFrzDemdId(wID) )	
			{
				for (i=0; i<wLen/oLen; i++)
				{					
					if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����						
						continue;									
					CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 4, &mBuf[i*nLen], &pbBuf[i*oLen]);				
				}
				tLen = i*nLen;
				memcpy(pbBuf, mBuf, tLen);				
			}

			tLen = wLen;
			break;	
		case 0xa0:
		case 0xa1:	
		case 0xa4:
		case 0xa5:		
				oLen = 3;
				nLen = oLen;
				for (i=0; i<wLen/oLen; i++)
				{					
					if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����						
						continue;									
					CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 4, &mBuf[i*nLen], &pbBuf[i*oLen]);				
				}
				tLen = i*nLen;
				memcpy(pbBuf, mBuf, tLen);
				tLen = wLen;
				break;
		case 0xb0:
		case 0xb1:	
		case 0xb4:
		case 0xb5:
				oLen = 4;
				nLen = oLen;
				tLen = wLen;
				break;
		case 0xb6:
				switch (wID>>4)
				{
					case 0xb61://��ѹ+1С��λ
							oLen = 2;	
							nLen = oLen;							
							for (i=0; i<wLen/oLen; i++)
							{					
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����						
									continue;												
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 1, 0, &mBuf[i*nLen], &pbBuf[i*oLen]);				
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);								
							break;
					case 0xb62://����+1����λ
					case 0xb66://��λ��+1����λ
							oLen = 2;
							nLen = oLen;
							for (i=0; i<wLen/oLen; i++)
							{
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����
									continue;
								if ((wID>>4) == 0xb62)
								{
									nLen = 3;
									CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 3, 2, &mBuf[i*nLen], &pbBuf[i*oLen]);		
								}
								else
									CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 2, 2, &mBuf[i*nLen], &pbBuf[i*oLen]);	
								*(mBuf+(i+1)*nLen-1) &= 0x7f;
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);
							break;
					case 0xb63://�й�����+1����λ
							oLen = 3;
							nLen = oLen;
							for (i=0; i<wLen/oLen; i++)
							{
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����
									continue;
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 4, &mBuf[i*nLen], &pbBuf[i*oLen]);		
								*(mBuf+(i+1)*nLen-1) &= 0x7f;
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);
							break;					
					case 0xb64://�޹�����+1����λ+2С��λ
							oLen = 2;	
							nLen = oLen+1;			
							for (i=0; i<wLen/oLen; i++)
							{	
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����	
								{
									memset(&mBuf[i*nLen], m_bInvdData, nLen);
									continue;
								}			
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 2, &mBuf[i*nLen], &pbBuf[i*oLen]);	
								*(mBuf+(i+1)*nLen-1) &= 0x7f;									
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);
							break;
					case 0xb65://��������-2��λ+1����λ
							oLen = 2;	
							nLen = oLen;
							for (i=0; i<wLen/oLen; i++)
							{						
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����
									continue;
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 3, 3, &mBuf[i*nLen], &pbBuf[i*oLen]);
								*(mBuf+(i+1)*nLen-1) &= 0x7f;
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);
							break;
					case 0xb67://���ڹ���
							oLen = 3;	
							nLen = oLen;
							for (i=0; i<wLen/oLen; i++)
							{
								if (IsAllAByte(pbBuf+i*oLen, m_bInvdData, oLen)) //��Ч����
									continue;
								CheckDecimalNew((BYTE)nLen, (BYTE)oLen, 4, 4, &mBuf[i*nLen], &pbBuf[i*oLen]);		
								*(mBuf+(i+1)*nLen-1) &= 0x7f;
							}
							tLen = i*nLen;
							memcpy(pbBuf, mBuf, tLen);							
							break;						
					default:
							tLen = wLen;		
							break;				
					}
					break;		
			default:
					tLen = wLen;		
					break;					
	}
	
	if (nLen!=0 && oLen!=0) //������ʽ��ҪתΪ16���Ƶ�����
	{	
		memcpy(mBuf, pbBuf, tLen);
		
		if ((wID&0xf000)==0x9000 && !IsDayFrzDemdId(wID) && !IsDayFrzDemdTime(wID)) //����
		{	
			bItemLen = sizeof(uint64);
			for (i=0; i<wLen/oLen; i++)
			{
				if (IsAllAByte(mBuf+i*nLen, m_bInvdData, nLen) || !IsBcdCode(mBuf+i*nLen, nLen)) //��Ч���� ���зǷ�����
					iVal64 = (DWORD)INVALID_VAL64;
				else
					iVal64 = BcdToUint64(mBuf+i*nLen, nLen);	
				memcpy(pbBuf+i*bItemLen, (BYTE*)&iVal64, bItemLen);
			}
			tLen = i*bItemLen;
		}
		else if ((wID&0xf000)==0xa000 ||  IsDayFrzDemdId(wID)) //����
		{
			bItemLen = sizeof(DWORD);
			for (i=0; i<wLen/oLen; i++)
			{
				if (IsAllAByte(mBuf+i*nLen, m_bInvdData, nLen) || !IsBcdCode(mBuf+i*nLen, nLen)) //��Ч���� ���зǷ�����
					dwVal = (DWORD)INVALID_VAL;
				else
					dwVal = BcdToDWORD(mBuf+i*nLen, nLen);	
				memcpy(pbBuf+i*bItemLen, (BYTE*)&dwVal, bItemLen);
			}
			tLen = i*bItemLen;
		}
		else if ( IsDemdTime(wID) ) //����ʱ��
		{
			for (i=0; i<wLen/oLen; i++)
			{
				if ( !IsBcdCode(mBuf+i*nLen, nLen)	//��Ч���� ���зǷ�����
					/*|| BcdToByte(mBuf[i*nLen])>59 
					|| BcdToByte(mBuf[i*nLen+1])>23		
					|| (BcdToByte(mBuf[i*nLen+2])<1 || BcdToByte(mBuf[i*nLen+2])>31) 
					|| (BcdToByte(mBuf[i*nLen+3])<1 || BcdToByte(mBuf[i*nLen+3])>12)*/ )
					memset(mBuf+i*nLen, m_bInvdData, nLen);

				memcpy(pbBuf+i*nLen, mBuf+i*nLen, nLen);		
			}
			tLen = wLen;
		}
		else if ((wID&0xff00) == 0xb600) //˲ʱ��(��ѹ/����/����/��������)
		{
			bItemLen = sizeof(int32);
			for (i=0; i<wLen/oLen; i++)
			{
				if (IsAllAByte(mBuf+i*nLen, m_bInvdData, nLen) || !IsBcdCode(mBuf+i*nLen, nLen)) //��Ч���� ���зǷ�����
					iVal32 = INVALID_VAL;
				else
					iVal32 = BcdToDWORD(mBuf+i*nLen, nLen);	//ʵ�����ֻ��3�ֽ�����,���ᳬ��ֵ��
				memcpy(pbBuf+i*bItemLen, (BYTE*)&iVal32, bItemLen);
			}
			tLen = i*bItemLen;
		}			
	}
	
	return tLen;
}

//����������2007��645Э���ȡ����չIDתΪ����Ӧ645ID���Լ���698�ն��ϵĶ�ȡ
//���أ�������Ӧ645ID
WORD Id645V07toDL645(WORD wExtId)
{
	WORD w645Id;

	switch(wExtId)
	{
		case 0xc860://�������״̬��1
		case 0xc86f://�������״̬�ֿ�
			w645Id = 0xc020;			
			break;
		case 0xc870://��ع���ʱ��
			w645Id = 0xb214;			
			break;
		case 0xc810://����ܴ���
			w645Id = 0xb212;	
			break;
		case 0xc811://���һ�α��ʱ��	
			w645Id = 0xb210;
			break;
		case 0xc830://���������ܴ���
			w645Id = 0xb213;		
			break;
		case 0xc831://���һ��������ʱ��
			w645Id = 0xb211;		
			break;
		case 0xc871://��һ�Զ�������
			w645Id = 0xc117;		
			break;
		default:
			w645Id = wExtId;			
			break;
	}

	return w645Id;
}

//��������97��645���ص�����תΪ2007��64�����ݸ�ʽ���Լ���698�ն��ϵĶ�ȡ
//���أ�2007��64�ĸ�ʽ���ݵĳ���
WORD Data645to645V07(WORD w645Id, BYTE* pbBuf, WORD wLen)
{
	WORD wExtLen;
	BYTE mBuf[20];
	memset(mBuf, 0, sizeof(mBuf));

	switch(w645Id)
	{
		case 0xc020: //��λ��һ�ֽ�0
		case 0xb214:
		case 0xb212:
		case 0xb213:
			pbBuf[wLen] = 0;
			wExtLen = wLen+1;
			break;
		case 0xb210://ʱ�䲹���뼰��
		case 0xb211:
			memcpy(mBuf+1, pbBuf, wLen);
			memcpy(pbBuf, mBuf, wLen+2);
			wExtLen = wLen+2;
		default:
			wExtLen = wLen;			
			break;
	}

	return wExtLen;
}
	
//�������Ӵ���ѭ����ȡ���ݲ�����Ƿ�����Ч֡,�˳���������:
//		1.����ʼ�ж������ݺ�,�����Ƿ�����Ч֡,��Ч�򷵻�TRUE
//		2.����ʼ�ж������ݺ�,Ϊ�쵽��Ч��������ݸ���Ϊ0����ڸ����Ļ��������ȷ���false
//		3.��һֱδ��������,����С��ʱʱ�䵽�򷵻�false
//		4.����������ʱʱ�䵽�򷵻�false
//������@dwDelayTime	�����ڵļ��˯��ʱ��
//		@bMaxSec		�������ڳ�ʱʱ��,��λ:��
//		@bMinSec		��С�����ڳ�ʱʱ��,��λ:��
//		@dwTimeOut		���ݵ������ں����Ķ�ȡ��ʱʱ��
//		@dwReadLength	ÿ�ζ�����Ҫ��ȡ���ֽ���
//		@dwBufSize		�����������ݵĻ������ĳ���,ȱʡΪ0,�������Ҫ�������ݲ��õ�
//		@pbBuf			ָ�򴫳��������ݵĻ�������ָ��,ȱʡΪ��,�������Ҫ�������ݲ��õ�
//		@dwFrmSize		������Ч֡ʱ���жϵ����ݵ���󳤶�,ȱʡΪ0,Ŀǰֻ��ABB,EDMIЭ�������Ч֡ʱ�õ�
//���أ�����true ��ʾ��⵽��Ч֡, false��ʾδ�쵽��Ч֡
bool ReadCommFrm(struct TMtrPro* pMtrPro, void* pTmpInf, DWORD dwDelayTime, DWORD dwMaxSec, DWORD dwMinSec, DWORD dwTimeOut, 
						  DWORD dwReadLength, DWORD dwBufSize, BYTE* pbBuf, DWORD dwFrmSize)
{
	bool fBegin = false;
	DWORD dwOldTick = GetTick();
	DWORD dwTmpClick;
	DWORD dwNewClick;	
	DWORD dwRxTick;
	DWORD dwLen = 0;
	DWORD dwPtr = 0;
	WORD i = 0, j = 0;
	BYTE bBuf[MTR_FRM_SIZE];
	BYTE bPrintPro;
	char szProName[20];
	DWORD wErrCnt = 1;
    static DWORD dwLastClick=0;
	CComm* pComm = pMtrPro->pMtrPara->pComm;

	pMtrPro->pfnGetProPrintType(&bPrintPro, szProName);		

	dwReadLength = ((dwReadLength > MTR_FRM_SIZE) ? MTR_FRM_SIZE:dwReadLength);
	if(dwBufSize>0 && pbBuf!=NULL)
		dwReadLength = ((dwReadLength > dwBufSize) ? dwBufSize:dwReadLength);

	dwTmpClick = GetClick();
	dwNewClick = dwTmpClick;
	dwRxTick = 0;
	while (dwNewClick-dwTmpClick < dwMaxSec)    //n�γ��Զ�ȡ����
	{
		i++;	
		if (dwDelayTime > 0)
		{
			Sleep(dwDelayTime);
		}	
		dwLen = pComm->Read(bBuf, dwReadLength, dwTimeOut);
	
		if (pbBuf != NULL) //�������Ҫ�򴫻����ݣ�����ֱ������ʱBUF����������OK�������ѷ�������Լ��Ľ��ջ�����
		{
			if ((dwLen+dwPtr) >= dwBufSize)
			{
				DTRACE(bPrintPro, ("CMeterPro:: MtrPro=%s ReadComm Buffer not enough!\r\n", szProName));
				break;
			}
			else
				memcpy(pbBuf+dwPtr, bBuf, dwLen);		
		}

		//DTRACE(bPrintPro, ("CMeterPro:: read com i=%d tick=%d dwLen=%d, dwPtr=%d\r\n", i, GetTick(), dwLen, dwPtr));

		if (dwLen > 0)
		{
            if (GetClick()-dwLastClick > 2)
            {
                dwLastClick = GetClick();
                //g_b485RxLed = 2;
//                SetLed(true, LED_DOWNR);  //����
            }    

			//dwRxTick = GetTick();
			j = 0;
			if ((dwLen+dwPtr) >= dwReadLength)
			{
				DTRACE(bPrintPro, ("CMeterPro:: MtrPro=%s ReadComm len over dwReadLength!\r\n", szProName));
				break;
			}
			dwPtr += dwLen;		
			fBegin = true;

			//if (RcvBlock(pbBuf, dwPtr, dwFrmSize)) //��⵽������֡���˳�������,��ʹ���ݶ�ȡ���̸���
			if (pMtrPro->pfnRcvBlock(pMtrPro, pTmpInf, bBuf, dwLen, dwFrmSize)) //��⵽������֡���˳�������,��ʹ���ݶ�ȡ���̸���
			{
				DTRACE(bPrintPro, ("CMeterPro:: read com OK, dwPtr=%ld, dwclick=%ld, RdCount=%ld, time=%ld, Pn=%d, MtrPro=%s\r\n", 
								   dwPtr, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick, 
								   pMtrPro->pMtrPara->wPn, szProName));
				
				//#ifdef SYS_WIN
				if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM))
				{
					char szHeader[32];
					sprintf(szHeader, "%s rx <--", szProName);
					TraceBuf(DB_645FRM, szHeader, bBuf, dwLen);
				}
				//#endif

				return true;
			}
			else
			{
		    	if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM))
				{
					DTRACE(bPrintPro, ("CMeterPro:: *****dwLen=%ld, dwclick=%ld, RdCount=%ld, time=%ld!\r\n", 
										dwLen, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick));
					//#ifdef SYS_WIN
					TraceBuf(DB_645FRM, "rx <--", bBuf, dwLen);
					//#endif
				}
			}				
		}
		else
		{
#ifdef SYS_LINUX
			if ((dwRxTick!=0 && GetTick()-dwRxTick>2200) || 		//�յ����ֽں�,�ȴ�2200���붼û�յ��ֽڵ��������˳� (fBegin && j>wErrCnt)  
				(dwRxTick==0 && dwNewClick-dwTmpClick>=dwMinSec))	//��û�յ����ֽ�,�ȴ�dwMinSec�˳�
			{
				break;
			}
#else
			j ++; //������ͨ�ż���			
			
			//20090817 ARMƽ̨�³�֡��Ҫ��3�β�������
			if (dwTimeOut <= 300) //������ʱ�Ͽ��Э�飬�ɶ��һ���Ա�֤��֡ʱ�����ݵĿɿ���
				wErrCnt = 2;
			
			if ((fBegin && j>wErrCnt) || (!fBegin && dwNewClick-dwTmpClick>=dwMinSec))
				break;	
#endif
		}

        //if (g_fDirRd[pMtrPro->bThrId] && !g_bDirRdStep)
		//	break;

		dwNewClick = GetClick();
	}	

	DTRACE(bPrintPro, ("CMeterPro:: read com Fail, fBegin=%d, dwPtr=%ld, dwclick=%ld, RdCount=%ld, time=%ld Pn=%d, MtrPro=%s\r\n", 
					   fBegin, 
					   dwPtr, dwNewClick-dwTmpClick, i, GetTick()-dwOldTick, 
					   pMtrPro->pMtrPara->wPn, szProName));

	if (IsDebugOn(bPrintPro) && IsDebugOn(DB_645FRM) && pbBuf!=NULL)
	{
		DTRACE(bPrintPro, ("CMeterPro:: MtrPro=%s !", szProName));
		
		//#ifdef SYS_WIN
		TraceBuf(DB_645FRM, "rx <--", pbBuf, dwPtr);
		//#endif
	}

	return false;
}

//�������Ƿ�97��645��֧�ֵ�����
bool Is645NotSuptId(WORD wID)
{
	if ( IsPhaseEngId(wID)	//�������
		 || ((wID>>8)==0xb6 && wID>0xb65f) //��λ�ǡ����ڹ��ʡ���������Ȳ�֧��	
		 || (wID>>8)==0xb7 //����97��645����չʧѹ���¼�ID
		 || wID>=0xc600 //����97��645��ID�б�֮���ID����07��645��ID����չID��
		 || wID<0x9010)	//����97��645��ID�б�֮���ID
	{
		return true;
	}
	return false;
}

