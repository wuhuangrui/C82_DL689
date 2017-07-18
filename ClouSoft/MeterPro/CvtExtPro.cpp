/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CvtExtPro.c
 * ժ    Ҫ�����ļ������˽ӿ�ת������չЭ�����ض���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2016��03��
 * ��    ע��
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaCfg.h"
#include "CvtExtPro.h"
#include "DbAPI.h"
#include "MtrCtrl.h"
#include "FaAPI.h"
#include <stddef.h>

#define	CVTEXT_CMD	      8	
#define	CVTEXT_LEN	      9	
#define	CVTEXT_EXTCMD	  10	
#define	CVTEXT_DATA	      11

#define	CVTEXT_CMD_T188   0x09
#define	CVT_CMD_GET	      0x1f
#define STEP_SIZE		  27	
#define PRO_POS			  5
#define MTR_ADDR_POS	  6
#define MTR_ADDR_LAST_BYTE 19
#define FIRST_PN_POS	  2
#define MTR_TYPE_POS	  26
#define PORT_POS		  4
#define CVT_ADDR_POS	  20
#define RX_FRM_BUF_LEN	  110
#define TX_BUF_LEN		  520
#define RX_BUF_LEN		  110
#define DATA_BUF_LEN	  520
#define ADD_BUF_LEN		  520
#define DEL_BUF_LEN		  520


//����:��ӿ�ת������չЭ�鷢��֡
int MakeCvtExtFrm(BYTE* pbCvtAddr, const BYTE* pbDataArea, BYTE bDataAreaLen, BYTE* pbFrm)
{
	int i;
    BYTE bCS = 0;
    BYTE bDataLen = 0;
    //BYTE bOffset = 0;
    BYTE *pbFrmCur = pbFrm;    

    //memset(pbFrmCur, 0xFE, bOffset);
    //pbFrmCur += bOffset;

    *pbFrmCur++ = 0x68;                 //68
    memcpy(pbFrmCur, pbCvtAddr, 6);        //��ַ
    pbFrmCur += 6;           
    *pbFrmCur++ = 0x68;                 //68
    *pbFrmCur++ = 0x1F;                 //������,�Զ���
    *pbFrmCur++ = bDataAreaLen+1;     //����   ��չ������+������
    *pbFrmCur++ = CVTEXT_CMD_T188;             //��չ������   
    memcpy(pbFrmCur, pbDataArea, bDataAreaLen);//������
    pbFrmCur += bDataAreaLen;
   
    //bDataLen = pbFrmCur - pbFrm - bOffset;
	 bDataLen = pbFrmCur - pbFrm;
    for (i = 0; i < bDataLen; i++)
    {
        //bCS += pbFrm[i + bOffset];
		bCS += pbFrm[i];
    }
    *pbFrmCur++ = bCS;
    *pbFrmCur++ = 0x16;
    //return bDataLen + 2 + bOffset;
	return bDataLen + 2;
}

//������485͸���ӿ�ת������չЭ��֡
//���أ�Ӧ��֡����
int DoCvtFwdFunc(BYTE bPort, DWORD dwCvtID, BYTE* pbCvtAddr, BYTE* pbRxFrm, DWORD dwRxBufSize, BYTE* pbTxBuf, WORD wTxBufLen)
{
	int iPort;
	BYTE bFrmLen;
	TCommPara CommPara;
	CComm *pComm = &g_commRs485[1];
	BYTE bData[86], mBuf[105];
	DWORD dwTmpClick, dwLen = 0;
	BYTE IDLen = sizeof(dwCvtID);
	BYTE bCvtID[4], i = 0;
	WORD wErrCnt = 10;
	bool fBegin = false;
	DWORD dwRxLen = 0;

	memset(bCvtID, 0, IDLen);
	DWordToByte(dwCvtID, bCvtID);
	memset(bData, 0, sizeof(bData));
	memcpy(bData, bCvtID, IDLen);
	memcpy(bData+IDLen, pbTxBuf, wTxBufLen);
	memset(pbTxBuf, 0, wTxBufLen);
	wTxBufLen += IDLen;
	
	iPort = MeterPortToPhy(bPort);
	if (iPort < 0)
		return -1;
	CommPara.wPort = (WORD)iPort;
	CommPara.dwBaudRate = CBR_2400;	
	CommPara.bParity =  EVENPARITY;
	CommPara.bByteSize = 8;
	CommPara.bStopBits = ONESTOPBIT;
	
	if ( !MtrProOpenComm(pComm, &CommPara) )
	{
		return -1;
	}

	pComm->Read(mBuf, sizeof(mBuf), 200); //�����һ�´���

	bFrmLen = MakeCvtExtFrm(pbCvtAddr, bData, wTxBufLen, pbTxBuf);

	memset(mBuf, 0, sizeof(mBuf));
	memset(mBuf, 0xfe, 4);
	memcpy(&mBuf[4], pbTxBuf, bFrmLen);
	bFrmLen += 4;
	if (pComm->Write(mBuf, bFrmLen) != bFrmLen)
	{
		DTRACE(DB_CVTEXT, ("DoCvtFwdFunc : fail to write comm!\r\n")); 
		return -1;
	}
	DTRACE(DB_CVTEXT, ("DoCvtFwdFunc: port=%d\r\n", iPort));
	TraceBuf(DB_CVTEXT, "DoCvtFwdFunc: tx -->", mBuf, bFrmLen);

	dwTmpClick = GetTick();	
	memset(mBuf, 0, sizeof(mBuf));
	while (GetTick()-dwTmpClick < 4000)    //n�γ��Զ�ȡ����
	{
		dwLen = pComm->Read(mBuf, sizeof(mBuf), 300);
	
		if ((dwRxLen+dwLen) > dwRxBufSize)
		{
			DTRACE(DB_CVTEXT, ("DoCvtFwdFunc: CommRead Buffer not enough!\r\n"));
			memcpy(pbRxFrm+dwRxLen, mBuf, dwRxLen+dwLen-dwRxBufSize-1);
			dwRxLen = dwRxBufSize;
			break;
		}
		else
			memcpy(pbRxFrm+dwRxLen, mBuf, dwLen);

		if (dwLen > 0)
		{
			i = 0;
			dwRxLen += dwLen;
			fBegin = true;
			if (CvtRcvBlock(pbRxFrm, dwRxLen, mBuf, &dwLen, sizeof(mBuf)) > 0)
			{
				DTRACE(DB_CVTEXT, ("DoCvtFwdFunc: port=%d\r\n", iPort));
				TraceBuf(DB_CVTEXT, "DoCvtFwdFunc: rx <--", pbRxFrm, dwRxLen);					
				return dwRxLen; //�յ�����֡����ǰ����
			}
		}
		else
		{
			i++; //������ͨ�ż���

			if ((fBegin && i>wErrCnt) || (!fBegin && (GetTick()-dwTmpClick>3000)))
				break;
		}					
	}

	DTRACE(DB_CVTEXT, ("DoCvtFwdFunc: port=%d\r\n", iPort));
	TraceBuf(DB_CVTEXT, "DoCvtFwdFunc: rx <--", pbRxFrm, dwRxLen);	
	return dwRxLen;
}

//����������Ӧ��֡����ȡ����������
//���أ��ɹ������������򳤶ȣ����������ݱ�ʶ���ȣ�ʧ�ܣ�����-1
//������@dwCvtID,���ݱ�ʶ�������Ϊ���� @pbRxFrm,����֡ @pbTxFrm,����֡ @pbDataBuf,����֡����������   
int CvtExtHandleFrm(DWORD dwCvtID, BYTE* pbRxFrm, BYTE* pbTxFrm, BYTE* pbDataBuf, DWORD dwRxSize)
{
	BYTE *pbRxBuf = pbRxFrm;
	BYTE *pbTxBuf = pbTxFrm;
	BYTE bCvtID[4];
	BYTE bDataLen, IDLen;
	IDLen = sizeof(dwCvtID);
	bDataLen = pbRxBuf[CVTEXT_LEN];
	memset(bCvtID, 0, IDLen);

	DWordToByte(dwCvtID, bCvtID);

	if ((pbRxBuf[CVTEXT_CMD]&CVT_CMD_GET) == (pbTxBuf[CVTEXT_CMD]&CVT_CMD_GET))
	{		
		if ((pbRxBuf[CVTEXT_CMD]-pbTxBuf[CVTEXT_CMD]) == 0x80)   //������У����ȷ������Ӧ��
		{	               
			if (pbRxBuf[CVTEXT_EXTCMD] == 0x09)   //��չ������У����ȷ
			{
				if (memcmp(&pbRxBuf[CVTEXT_DATA], &pbTxBuf[CVTEXT_DATA], IDLen) == 0)
				{
					if (bDataLen-IDLen-1 > dwRxSize)
					{					
						memcpy(pbDataBuf, &pbRxBuf[CVTEXT_DATA+IDLen], dwRxSize);  //ֻ�����������ݣ���Ҫ��չ�������ID
						return dwRxSize;
					}
					else
					{					
						memcpy(pbDataBuf, &pbRxBuf[CVTEXT_DATA+IDLen], bDataLen-IDLen-1);  //ֻ�����������ݣ���Ҫ��չ�������ID
						return (bDataLen-IDLen-1);
					}
				}
				else
				{
					DTRACE(DB_CVTEXT, ("CvtExtHandleFrm : Cmd code not match!\r\n"));
					return -1;    //���ݱ�ʶ��һ��
				}
			}
			else if (pbRxBuf[CVTEXT_EXTCMD] == 0xc9)   //�쳣Ӧ��
			{   
				DTRACE(DB_CVTEXT, ("CvtExtHandleFrm : Abnormal response!\r\n"));
				return -1;       
			}              
		}
		else
		{
			DTRACE(DB_CVTEXT, ("CvtExtHandleFrm : Extend control code was wrong.\r\n")); 
			return -1;
		}
	}

	return -1;
}

void InitTCvtInfo(TCvtInfo *tpCvtInfo)
{
	memset(tpCvtInfo->bCvtAddr, 0, 6);
	tpCvtInfo->bPort = 0;
	memset(tpCvtInfo->bMtrType, 0, PN_NUM_IN_CVT);
	memset(tpCvtInfo->bMtrAddr, 0, PN_NUM_IN_CVT*7);
	tpCvtInfo->bPnNum = 0;
	tpCvtInfo->wCrc = 0;
}

//������ʵ��ͬ��ˮ���ȱ�����ת���������Ȳ�ѯ��ת�����ﵵ��������CRC������������ת������Ľ���Ƚϣ������ͬ����Ҫ���£�
//����ת����������е������뼯������ĵ����Ƚϣ��ҳ����죬���ݲ������ӻ�ɾ���������ٴαȽϵ���������CRC�������ͬ����³ɹ���
//��ͬ��ɾ����ת��������е���
bool SyncDocsToCvt(TAllCvtInfo *ptAllCvt)
{
	int i;
	BYTE bRxFrmBuf[RX_FRM_BUF_LEN];
	BYTE bRxBuf[RX_BUF_LEN];
	BYTE bDataBuf[DATA_BUF_LEN];
	BYTE bAddBuf[ADD_BUF_LEN];
	BYTE bDelBuf[DEL_BUF_LEN];
	int iRtnLen = -1;
	BYTE bDocNum = 0;

	WORD wAddBufLen;
	WORD wDelBufLen;
	int iRtnVal = 0;
	BYTE bSychNum = 0;
	BYTE bClearCnt = 0;
	TCvtInfo tCvt;
	TAllCvtInfo tBackCvt;
	TAllCvtInfo tClearCvt;
	BYTE bCvtClearNum = 0;
	BYTE bCvtNum = ptAllCvt->bCvtNum;
	BYTE bAddrOffSet = offsetof(TAllCvtInfo, bAllCvtAddr);
	BYTE bPortOffSet = offsetof(TAllCvtInfo, bAllPort);

	memset(&tBackCvt, 0, sizeof(TAllCvtInfo));
	memset(&tClearCvt, 0, sizeof(TAllCvtInfo));

	for (i=0; i<bCvtNum; i++)
	{
		InitTCvtInfo(&tCvt);
		GetCvtInfo(&tCvt, ptAllCvt->bAllCvtAddr[i]);

		wAddBufLen = 0;
		wDelBufLen = 0;

		memset(bRxFrmBuf, 0, sizeof(bRxFrmBuf));
		memset(bRxBuf, 0, sizeof(bRxBuf));
		memset(bDataBuf, 0, sizeof(bDataBuf));
		memset(bAddBuf, 0, sizeof(bAddBuf));
		memset(bDelBuf, 0, sizeof(bDelBuf));

		//��鵵��������CRC
		if ((iRtnVal = CheckNumAndCrc(&tCvt, &bDocNum)) < 0)
		{
			if (bDocNum == 0) //ת������û�е���������������ĵ���ȫ���·���SelectDocs()Ҳ�ܴ�����������ʱ�临�ӶȻ�����
			{
				GetAllDocs(&tCvt, bAddBuf);
				if (AddDocs(tCvt, bRxFrmBuf, bRxBuf, bAddBuf, bDataBuf, RX_FRM_BUF_LEN) < 0)
				{
					continue;
				}
			}
			else //�������������һ�����ҳ�Ҫ���ӵĺ�Ҫɾ���ĵ���
			{   //��ѯ�����е���
				if ((iRtnLen = QueryDocs(&tCvt, bDocNum, bRxFrmBuf, bRxBuf, bAddBuf, bDataBuf, RX_FRM_BUF_LEN)) < 0)
				{
					continue;
				}
				//�ҵ�ת������Ҫ���ӵĺ�Ҫɾ���ĵ���
				SelectDocs(&tCvt, bDataBuf, iRtnLen, bAddBuf, bDelBuf, &wAddBufLen, &wDelBufLen);
				//��Ҫ����bAddBuf[0]������
				if (bAddBuf[0] > 0) 
				{
					if (AddDocs(tCvt, bRxFrmBuf, bRxBuf, bAddBuf, bDataBuf, RX_FRM_BUF_LEN) < 0)
					{
						continue;
					}
				}
				//��Ҫɾ��bDelBuf[0]������
				if (bDelBuf[0] > 0) 
				{
					if (DelDocs(tCvt, bRxFrmBuf, bRxBuf, bDelBuf, bDataBuf, RX_FRM_BUF_LEN) < 0)
					{
						continue;
					}
				}
			}
			Sleep(2000);  //���ӡ�ɾ������ת�����ĵ�������ʱ���ת����ȥ��CRC
			//�ٴμ�鵵��������CRC
			if ((iRtnVal = CheckNumAndCrc(&tCvt, &bDocNum)) < 0)
			{
				TraceBuf(DB_CVTEXT, "CheckNumAndCrc: Synchronize failed, doc number or crc error,convertor address is:", tCvt.bCvtAddr, 6);
				continue;
			}
			else if (iRtnVal == 1)
			{
				TraceBuf(DB_CVTEXT, "CheckNumAndCrc: Synchronize successfully,convertor address is:", tCvt.bCvtAddr, 6);
				bSychNum++;
				continue;
			}
		}
		else if (iRtnVal == 1)
		{
			TraceBuf(DB_CVTEXT, "CheckNumAndCrc: no need to Synchronize,convertor address is:", tCvt.bCvtAddr, 6);
			bSychNum++;
			continue;
		}
	}

	BYTE bCvtinfo[1024];
	memset(bCvtinfo, 0x00, sizeof(bCvtinfo));
	if (readfile(USER_PARA_PATH"CvtinfoBack.data", bCvtinfo, sizeof(bCvtinfo)))
	{													//ͬʱ����־λ��Ϊ0x55ʱҲ�������ĳ��ת�����Ķ����ˣ���Ϊת��������
		tBackCvt.bCvtNum = bCvtinfo[0];
		memcpy(tBackCvt.bAllCvtAddr, &bCvtinfo[bAddrOffSet], CVT_MAX_NUM*CVT_ADDR_LEN);
		memcpy(tBackCvt.bAllPort, &bCvtinfo[bPortOffSet], CVT_MAX_NUM);

		bCvtClearNum = FindCvtAddrToClear(&tBackCvt, tBackCvt.bCvtNum, ptAllCvt, bCvtNum, &tClearCvt);

		for (i=0; i<bCvtClearNum; i++)
		{
			if (0 == ClearDocs(tClearCvt.bAllPort[i], tClearCvt.bAllCvtAddr[i], bRxFrmBuf, bRxBuf, bAddBuf, bDataBuf, RX_FRM_BUF_LEN))
			{
				bClearCnt++;
			}
		}
	}

	memset(bCvtinfo, 0x00, sizeof(bCvtinfo));
	bCvtinfo[0] = bCvtNum;
	memcpy(&bCvtinfo[bAddrOffSet], ptAllCvt->bAllCvtAddr, CVT_MAX_NUM*CVT_ADDR_LEN);
	memcpy(&bCvtinfo[bPortOffSet], ptAllCvt->bAllPort, CVT_MAX_NUM);
	if (!WriteFile(USER_PARA_PATH"CvtinfoBack.data", bCvtinfo, sizeof(bCvtinfo)))
	{
		return false;
	}

	if ((bSychNum==bCvtNum) && (bClearCnt==bCvtClearNum)) 
	{
		return true;    //ȫ��ͬ���ɹ�
	}
	else
	{
		return false;   
	}
}

//�������Ƚϼ�������ת�����ĵ������ҵ�ת������Ҫ���ӵĺ�Ҫɾ���ĵ���
int SelectDocs(TCvtInfo *tpCvtInfo, BYTE *bpBuf, WORD wBufLen, BYTE *bpAddBuf, BYTE *bpDelBuf, WORD *wpAddBufLen, WORD *wpDelBufLen)
{
	int i, j;
	BYTE bCnt = 0;
	BYTE bDocNum1 = tpCvtInfo->bPnNum; //��������ĵ�����
	BYTE bDocNum2 = bpBuf[0]; //ת������ĵ�����

	for (i=0; i<bDocNum1; i++)  //�ҵ�Ҫ���ӵĵ���
	{
		for (j=0; j<bDocNum2; j++)
		{
			if ((memcmp(tpCvtInfo->bMtrAddr[i], &bpBuf[2+j*8], 7) == 0) && (tpCvtInfo->bMtrType[i] == bpBuf[1+j*8]))
				break;
		}

		if (j == bDocNum2)
		{
			bpAddBuf[1+8*bCnt] = tpCvtInfo->bMtrType[i]; //����������
			memcpy(&bpAddBuf[2+8*bCnt], tpCvtInfo->bMtrAddr[i], 7); //�������ַ
			bCnt++;
		}
	}
	bpAddBuf[0] = bCnt; //Ҫ���ӵĵ�������
	*wpAddBufLen = 1 + (1 + 7) * bCnt; //bpAddBuf���ǡ�����-������-���ַ-������-���ַ...��
	bCnt = 0;

	for (i=0; i<bDocNum2; i++) //�ҵ�Ҫɾ���ĵ���
	{
		for (j=0; j<bDocNum1; j++)
		{
			if ((memcmp(&bpBuf[2+i*8], tpCvtInfo->bMtrAddr[j], 7) == 0) && (bpBuf[1+i*8] == tpCvtInfo->bMtrType[j]))
				break;
		}

		if (j == bDocNum1)
		{
			memcpy(&bpDelBuf[1+8*bCnt], &bpBuf[1+i*8], 1); //����������
			memcpy(&bpDelBuf[2+8*bCnt], &bpBuf[2+i*8], 7); //�������ַ
			bCnt++;
		}
	}
	bpDelBuf[0] = bCnt; 
	*wpDelBufLen = 1 + (1 + 7) * bCnt; //Ҫɾ���ĵ�����������"����-������-���ַ..."

	return 0;
}

//������������е��������
int GetAllDocs(TCvtInfo *tpCvtInfo, BYTE *pbTxBuf)
{
	int i;
	WORD wLen = 0;

	for (i=0; i<tpCvtInfo->bPnNum; i++)  //�����ݴ���ɡ�����-������-���ַ-������-���ַ...����ʽ
	{
		pbTxBuf[1+8*i] = tpCvtInfo->bMtrType[i];
		memcpy(&pbTxBuf[2+8*i], tpCvtInfo->bMtrAddr[i], 7);
	}

	pbTxBuf[0] = tpCvtInfo->bPnNum;
	wLen = 1 + (1 + 7) * tpCvtInfo->bPnNum;

	return wLen;
}

void DoSyncDocs()
{
	int i;
	TAllCvtInfo tAllCvt;

	memset(&tAllCvt, 0, sizeof(TAllCvtInfo));
	GetCvtNums(&tAllCvt);

	for (i=0; i<3; i++) //һ�����ͬ��3��
	{
		if (SyncDocsToCvt( &tAllCvt ))  //ͬ���ɹ��ˣ��Ͳ���Ҫ��ͬ����
		{	
			break;
		}
	}
}

//�������Ӵ��ڻ�������������ת������չЭ��ı���
//������@pbBlock - ���ܵĻ���
//		@dwLen - ���յĳ���
//		@pbRxBuf - �����������֡�Ļ���
//		@pwRxLen - ��������֡���ĳ���
//		@dwBufSize - ���ջ������ĳ���
//���أ�0-�����ݣ�����-���յ�������֡���ȣ�����-��Ч���ݳ���
int CvtRcvBlock(BYTE* pbBlock, DWORD dwLen, BYTE* pbRxBuf, DWORD* pRxLen, DWORD dwBufSize)
{	
	WORD i;
	BYTE bRxPtr = 0;
	BYTE bRxCnt = 0;
	BYTE bRxStep = 0;
	short sFrmHead = -1;

	for (i=0; i<dwLen; i++)
	{
		BYTE b = *pbBlock++;

		switch (bRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				pbRxBuf[0] = 0x68;
				bRxPtr = 1;
				bRxCnt = 9;       
				bRxStep = 1;
				sFrmHead = i;//��֮ǰ�����ݶ�����Ч��
			}
			break;
		case 1:    //������ǰ������
			pbRxBuf[bRxPtr++] = b;
			bRxCnt --;
			if (bRxCnt == 0)   //�����꣬����У��
			{
				if (pbRxBuf[7] == 0x68) // && (p[FAPDL645_CMD]&FAPDL645_CMD_DIR)==FAPDL645_CMD_DOWN //��ֹ���յ����ⷵ�ص��Լ�����ȥ��֡
				{
					bRxCnt = pbRxBuf[9] + 2;  //0xfe+2
					sFrmHead++;
					if (bRxCnt+10>dwBufSize || pbRxBuf[9]>=dwBufSize)   //��֡�Ļ���������
					{
						bRxStep = 0;					
						break;
						//return -sFrmHead;                 //���ﷵ��0������������Զ�޷��ͷ�
					}	
					bRxStep = 2;
				}
				else
				{					
					bRxStep = 0;
					sFrmHead++;
				}		
			}
			break;
		case 2:     //���� + ������ + ������
			pbRxBuf[bRxPtr++] = b;
			//DTRACE(DB_DL645V07, ("CDL645V07::RcvBlock : m_wRxCnt=%d.m_wRxPtr=%d,m_bRxBuf=%x\r\n", m_wRxCnt, m_wRxPtr, m_bRxBuf[m_wRxPtr-1])); 	
			bRxCnt -- ;
			if (bRxCnt == 0)   //�����꣬����У��
			{
				bRxStep = 0;

				if (pbRxBuf[bRxPtr-1]==0x16 && pbRxBuf[bRxPtr-2]==CheckSum(pbRxBuf, pbRxBuf[9]+10))
				{
					*pRxLen = pbRxBuf[9] + 12;
					//return i+1;//���յ�������һ֡		
					return dwLen;//���յ�������һ֡	����ȫ�� �Ա�ʾ��������ȫ���������
				}
				else
				{
					sFrmHead++;
				}
			}
			break;
		default:
			bRxStep = 0;
			break;
		} //switch (m_nRxStep) 
	}

	if (sFrmHead != -1)
		return -sFrmHead;

	return -(int)dwLen;
}

int ClearDocs(BYTE bPort, BYTE *pbCvtAddr, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbTxBuf, BYTE *pbDataBuf, DWORD dwRxBufSize)
{
	DWORD dwCvtID = 0x04101601;
	int iDataLen = 0;
	WORD wTxBufLen = 0;
	DWORD dwLen = 0;
	DWORD dwRxLen = 0;
	BYTE bReExcCnt = 3;

	while((0 < bReExcCnt--))
	{
		memset(pbRxFrmBuf, 0, RX_FRM_BUF_LEN);
		memset(pbTxBuf, 0, TX_BUF_LEN);
		memset(pbDataBuf, 0, DATA_BUF_LEN);
		memset(pbRxBuf, 0, RX_BUF_LEN);

		TraceBuf(DB_CVTEXT, "ClearDocs: preparing to clear docs from convertor: ", pbCvtAddr, 6);
		dwLen = DoCvtFwdFunc(bPort, dwCvtID, pbCvtAddr, pbRxFrmBuf, dwRxBufSize, pbTxBuf, wTxBufLen);

		if (!IsAllAByte(pbRxFrmBuf, 0, RX_FRM_BUF_LEN))
		{
			dwRxLen = 0;
			memset(pbRxBuf, 0, RX_BUF_LEN);
			if (CvtRcvBlock(pbRxFrmBuf, dwLen, pbRxBuf, &dwRxLen, RX_BUF_LEN) > 0)
			{
				iDataLen = CvtExtHandleFrm(dwCvtID, pbRxBuf, pbTxBuf, pbDataBuf, DATA_BUF_LEN); 
			}
			else
			{
				continue;
			}

			if (iDataLen < 0)
			{
				continue;
			}

			if (pbDataBuf[0] == 1) //��ճɹ�
			{
				DTRACE(DB_CVTEXT, ("ClearDocs: clear docs successfully!\r\n"));
				return 0;
			}
			else if (pbDataBuf[0] == 2) //���ʧ��
			{ 
				DTRACE(DB_CVTEXT, ("ClearDocs: failed to clear docs!\r\n"));
				return -1;
			}
		}
	}
	
	DTRACE(DB_CVTEXT, ("ClearDocs: the received frame is wrong!\r\n"));
	return -1;
}

//��������ѯ����鵵��������CRC
int CheckNumAndCrc(TCvtInfo *tpCvtInfo, BYTE *pbDocNum)
{
	DWORD dwCvtID = 0x04101602;
	int iDataLen = 0;
	WORD wTxBufLen = 0;
	DWORD dwLen = 0;
	DWORD dwRxLen = 0;
	WORD wCrc = 0;
	BYTE bReExcCnt = 3;
	BYTE bRxFrmBuf[64];
	BYTE bRxBuf[32];
	BYTE bTxBuf[32];
	BYTE bDataBuf[4];

	while(0 < bReExcCnt--)
	{
		memset(bRxFrmBuf, 0, sizeof(bRxFrmBuf));
		memset(bTxBuf, 0, sizeof(bTxBuf));

		dwLen = DoCvtFwdFunc(tpCvtInfo->bPort, dwCvtID, tpCvtInfo->bCvtAddr, bRxFrmBuf, sizeof(bRxFrmBuf), bTxBuf, wTxBufLen);//��ѯ����������CRC
		if (!IsAllAByte(bRxFrmBuf, 0, sizeof(bRxFrmBuf))) //���յ��ӿ�ת������Ӧ��֡�Ŵ���
		{
			dwRxLen = 0;
			memset(bRxBuf, 0, sizeof(bRxBuf));
			memset(bDataBuf, 0, sizeof(bDataBuf));
			if (CvtRcvBlock(bRxFrmBuf, dwLen, bRxBuf, &dwRxLen, sizeof(bRxBuf)) > 0) //��������ת������չЭ��֡�Ŵ���
			{
				iDataLen = CvtExtHandleFrm(dwCvtID, bRxBuf, bTxBuf, bDataBuf, sizeof(bDataBuf));
			}
			else
			{
				continue;
			}

			if (iDataLen < 0)
			{
				continue;
			}

			if (iDataLen == 3)
			{
				*pbDocNum = bDataBuf[0];
				wCrc = ByteToWord(&bDataBuf[1]);

				if ((*pbDocNum == tpCvtInfo->bPnNum) && (wCrc == tpCvtInfo->wCrc)) //������ȷ�������һ��ת����
				{
					return 1;
				}
				else
				{
					return -1;
				}
			}
		}
	}

	DTRACE(DB_CVTEXT, ("CheckNumAndCrc: the received frame is wrong!\r\n"));
	return 0;
}

//��������ѯ��ת����������е���
int QueryDocs(TCvtInfo *tpCvtInfo, BYTE bDocNum, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbTxBuf, BYTE *pbDataBuf, DWORD dwRxBufSize)
{
	DWORD dwCvtID = 0x04101603;
	int iDataLen = 0;
	WORD wTxBufLen = 0;
	DWORD dwLen = 0;
	DWORD dwRxLen = 0;
	BYTE bQueryNum = 0;
	BYTE bQueryTimes = 0;
	BYTE bLastTimesNum = 0;
	BYTE bReExcCnt = 3;
	BYTE i;
	BYTE bTmpBuf[90];
	WORD wIndex = 1;

	memset(pbDataBuf, 0, DATA_BUF_LEN);

	if (bDocNum%10 > 0)
	{
		bQueryTimes = bDocNum/10 + 1;
		bLastTimesNum = bDocNum%10;
	}
	else
	{
		bQueryTimes = bDocNum/10;
		bLastTimesNum = 0;
	}

	for (i=0; i<bQueryTimes; i++)
	{
		bReExcCnt = 3;

		if ((i == bQueryTimes - 1) && (bDocNum%10 > 0))
		{
			bQueryNum = bLastTimesNum;
		}
		else
		{
			bQueryNum = 10; //ÿ������ѯ10������
		}
		wTxBufLen = 2;
			
		while(0 < bReExcCnt)
		{
			memset(pbRxFrmBuf, 0, RX_FRM_BUF_LEN);			
			memset(pbRxBuf, 0, RX_BUF_LEN);
			memset(bTmpBuf, 0, sizeof(bTmpBuf));
			memset(pbTxBuf, 0, TX_BUF_LEN);

			pbTxBuf[0] = i*10; //��ʼ���
			pbTxBuf[1] = bQueryNum; //��ѯ����

			dwLen = DoCvtFwdFunc(tpCvtInfo->bPort, dwCvtID, tpCvtInfo->bCvtAddr, pbRxFrmBuf, dwRxBufSize, pbTxBuf, wTxBufLen); //��ȡת����������е���
			if (!IsAllAByte(pbRxFrmBuf, 0, RX_FRM_BUF_LEN)) //���յ��ӿ�ת������Ӧ��֡�Ŵ���
			{
				dwRxLen = 0;
				if (CvtRcvBlock(pbRxFrmBuf, dwLen, pbRxBuf, &dwRxLen, RX_BUF_LEN) > 0)
				{
					iDataLen = CvtExtHandleFrm(dwCvtID, pbRxBuf, pbTxBuf, bTmpBuf, sizeof(bTmpBuf)); //Ӧ������Ϊ��Ӧ������+������+���ַ...��
					if (iDataLen < 0)
					{
						DTRACE(DB_CVTEXT, ("QueryDocs: the received frame is wrong!\r\n"));
						continue;
					}
					pbDataBuf[0] += bTmpBuf[0]; //��������
					memcpy(pbDataBuf+wIndex, bTmpBuf+1, iDataLen-1);
					wIndex += (iDataLen - 1);
					break;
				}
			}
			bReExcCnt--;
		}
		if (bReExcCnt == 0) //�ط����ν���֡��Ȼ����ȷ
		{
			DTRACE(DB_CVTEXT, ("QueryDocs: the received frame is wrong!\r\n"));
			return -1;
		}
	}

	return wIndex;
}

//���������ӵ���
int AddDocs(TCvtInfo tCvt, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbAddBuf, BYTE *pbDataBuf, DWORD dwRxBufSize)
{
	DWORD dwCvtID = 0x04101604;
	int iDataLen = 0;
	DWORD dwLen = 0;
	DWORD dwRxLen = 0;
	BYTE bReExcCnt = 3;
	BYTE bAddTimes = 0;
	BYTE bLeftNum = 0;
	BYTE bLastTimesNum = 0;
	BYTE i;
	BYTE bTmpBuf[110];
	WORD wIndex = 1;
	BYTE bAddNum = pbAddBuf[0];

	if (bAddNum%10 > 0)
	{
		bAddTimes = bAddNum/10 + 1;
		bLastTimesNum = bAddNum%10;
	}
	else
	{
		bAddTimes = bAddNum/10;
		bLastTimesNum = 0;
	}

	for (i=0; i<bAddTimes; i++)
	{
		bReExcCnt = 3;

		if ((i == bAddTimes - 1) && (bAddNum%10 > 0))
		{
			bLeftNum = bLastTimesNum;
		}
		else
		{
			bLeftNum = 10; //ÿ���������10������
		}

		while(0 < bReExcCnt)
		{
			memset(pbRxFrmBuf, 0, RX_FRM_BUF_LEN);			
			memset(pbRxBuf, 0, RX_BUF_LEN);
			memset(pbDataBuf, 0, DATA_BUF_LEN);
			memset(bTmpBuf, 0, sizeof(bTmpBuf));
			
			bTmpBuf[0] = bLeftNum;
			memcpy(bTmpBuf+1, pbAddBuf+wIndex, bLeftNum*8);
			
			dwLen = DoCvtFwdFunc(tCvt.bPort, dwCvtID, tCvt.bCvtAddr, pbRxFrmBuf, dwRxBufSize, bTmpBuf, bLeftNum*8+1); //��ȡת����������е���
			if (!IsAllAByte(pbRxFrmBuf, 0, RX_FRM_BUF_LEN)) //���յ��ӿ�ת������Ӧ��֡�Ŵ���
			{
				dwRxLen = 0;
				if (CvtRcvBlock(pbRxFrmBuf, dwLen, pbRxBuf, &dwRxLen, RX_BUF_LEN) > 0)
				{
					iDataLen = CvtExtHandleFrm(dwCvtID, pbRxBuf, bTmpBuf, pbDataBuf, DATA_BUF_LEN); 
					if (iDataLen != -1)
					{
						break;  //����֡��ȷ
					}
				}
			}
			bReExcCnt--;
		}
		if (bReExcCnt == 0) //�ط����ν���֡��Ȼ����ȷ
		{
			DTRACE(DB_CVTEXT, ("AddDocs: the received frame is wrong!\r\n"));
			return -1;
		}

		wIndex += bLeftNum*8;
		Sleep(5000);   //����һ֡����ʱ���ת��������ת������ʱ��洢�����ӵĵ���
	}

	return 0;
}

//������ɾ������
int DelDocs(TCvtInfo tCvt, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbDelBuf, BYTE *pbDataBuf, DWORD dwRxBufSize)
{
	DWORD dwCvtID = 0x04101605;
	int iDataLen = 0;
	DWORD dwLen = 0;
	DWORD dwRxLen = 0;
	BYTE bReExcCnt = 3;
	BYTE bDelTimes = 0;
	BYTE bLeftNum = 0;
	BYTE bLastTimesNum = 0;
	BYTE i;
	BYTE bTmpBuf[110];
	WORD wIndex = 1;
	BYTE bDelNum = pbDelBuf[0];

	if (bDelNum%10 > 0)
	{
		bDelTimes = bDelNum/10 + 1;
		bLastTimesNum = bDelNum%10;
	}
	else
	{
		bDelTimes = bDelNum/10;
		bLastTimesNum = 0;
	}

	for (i=0; i<bDelTimes; i++)
	{
		bReExcCnt = 3;

		if ((i == bDelTimes - 1) && (bDelNum%10 > 0))
		{
			bLeftNum = bLastTimesNum;
		}
		else
		{
			bLeftNum = 10; //ÿ�����ɾ��10������
		}

		while(0 < bReExcCnt)
		{
			memset(pbRxFrmBuf, 0, RX_FRM_BUF_LEN);			
			memset(pbRxBuf, 0, RX_BUF_LEN);
			memset(pbDataBuf, 0, DATA_BUF_LEN);
			memset(bTmpBuf, 0, sizeof(bTmpBuf));

			bTmpBuf[0] = bLeftNum;
			memcpy(bTmpBuf+1, pbDelBuf+wIndex, bLeftNum*8);

			dwLen = DoCvtFwdFunc(tCvt.bPort, dwCvtID, tCvt.bCvtAddr, pbRxFrmBuf, dwRxBufSize, bTmpBuf, bLeftNum*8+1); //��ȡת����������е���
			if (!IsAllAByte(pbRxFrmBuf, 0, RX_FRM_BUF_LEN)) //���յ��ӿ�ת������Ӧ��֡�Ŵ���
			{
				dwRxLen = 0;
				if (CvtRcvBlock(pbRxFrmBuf, dwLen, pbRxBuf, &dwRxLen, RX_BUF_LEN) > 0)
				{
					iDataLen = CvtExtHandleFrm(dwCvtID, pbRxBuf, bTmpBuf, pbDataBuf, DATA_BUF_LEN);
					if (iDataLen != -1)
					{
						break; //����֡��ȷ
					}
				}
			}
			bReExcCnt--;
		}
		if (bReExcCnt == 0) //�ط����ν���֡��Ȼ����ȷ
		{
			DTRACE(DB_CVTEXT, ("DelDocs: the received frame is wrong!\r\n"));
			return -1;
		}

		wIndex += bLeftNum*8;
		Sleep(5000);  //����һ֡����ʱ���ת��������ת������ʱ��ȥɾ��Ҫɾ���ĵ���
	}

	return 0;
}

//�������ڳ����߳�1ͬ������ʱ��ʹ���ź��������Ƴ����߳�1�ͳ����߳�0�����߳�1��ͬ������ʱ��
//�������߳�0�������߳�0�ڳ���ʱ���������߳�1ͬ���������������ͬ�������ͳ���һ��ִ��ʱ���½���֡���ҵ�����
bool PortInit()
{
	//g_semPort = NewSemaphore(1,1);

	return true;
}

int GetCvtNums(TAllCvtInfo *ptAllCvt)
{
	WORD wPn;
	BYTE bTmpbuf[F10_LEN_PER_PN];
	BYTE bCvtAddr[CVT_ADDR_LEN];
	BYTE bCvtNum = 0;
	int i;

	for (wPn=MTR_START_PN; wPn<POINT_NUM; wPn++)
	{
		memset(bTmpbuf, 0, F10_LEN_PER_PN);
		memset(bCvtAddr, 0, CVT_ADDR_LEN);

		if (IsPnValid(wPn) && GetPnMtrPro(wPn) == PROTOCOLNO_SBJC) //�����ˮ���Ȳ�����
		{
			ReadItemEx(BN0, wPn, 0x8902, bTmpbuf); //��ȡ���������
			memcpy(bCvtAddr, &bTmpbuf[CVT_ADDR_POS], CVT_ADDR_LEN);
			if (IsAllAByte(bCvtAddr, 0, CVT_ADDR_LEN))
			{
				continue;
			}

			for (i=0; i<bCvtNum; i++)  
			{
				if (memcmp(ptAllCvt->bAllCvtAddr[i], bCvtAddr, CVT_ADDR_LEN) == 0)  //������Ѿ����ڵ�ת������ַ
				{
					break;
				}
			}

			if (i == bCvtNum)  //������µ�ת������ַ
			{
				memcpy(ptAllCvt->bAllCvtAddr[bCvtNum], bCvtAddr, CVT_ADDR_LEN);
				ptAllCvt->bAllPort[bCvtNum] = bTmpbuf[PORT_POS]&0x01f;
				bCvtNum++;  //ת������������
			}

			if (bCvtNum >= CVT_MAX_NUM) //�����������
				break;
		}
	}

	ptAllCvt->bCvtNum = bCvtNum;

	return bCvtNum;
}

void GetCvtInfo(TCvtInfo *ptCvtInfo, BYTE *pbTmpbuf)
{
	WORD wPn, wCrc;
	BYTE bCurNum = 0;
	BYTE bCvtAddr[CVT_ADDR_LEN];
	BYTE bTmpbuf[F10_LEN_PER_PN];

	memset(bCvtAddr, 0, CVT_ADDR_LEN);
	memset(bTmpbuf, 0, F10_LEN_PER_PN);

	memcpy(ptCvtInfo->bCvtAddr, pbTmpbuf, CVT_ADDR_LEN);

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		if (IsPnValid(wPn) && GetPnMtrPro(wPn) == PROTOCOLNO_SBJC) //�����ˮ���Ȳ�����
		{
			ReadItemEx(BN0, wPn, 0x8902, bTmpbuf); //��ȡ���������
			memcpy(bCvtAddr, &bTmpbuf[CVT_ADDR_POS], CVT_ADDR_LEN);
			if (IsAllAByte(bCvtAddr, 0, CVT_ADDR_LEN))
			{
				continue;
			}

			if(memcmp(pbTmpbuf, bCvtAddr, CVT_ADDR_LEN) == 0)
			{
				bCurNum = ptCvtInfo->bPnNum;  //��ǰת���������õĲ���������
				ptCvtInfo->bPort = bTmpbuf[PORT_POS]&0x01f;
				ptCvtInfo->bMtrType[bCurNum] = bTmpbuf[MTR_TYPE_POS];
				memcpy(ptCvtInfo->bMtrAddr[bCurNum], bTmpbuf+MTR_ADDR_POS, 6);
				ptCvtInfo->bMtrAddr[bCurNum][6] = bTmpbuf[MTR_ADDR_LAST_BYTE];  //ˮ���ȱ�ĵ�ַ�����λ�Ƿ������й�����ʾֵ����λ��С��λ������λ��
				ptCvtInfo->bPnNum += 1;
				wCrc = get_crc_16(0, &(ptCvtInfo->bMtrType[bCurNum]), 1);//�����ת���������в���������͵�У���
				wCrc = get_crc_16(wCrc, ptCvtInfo->bMtrAddr[bCurNum], 7);//�����ת���������в������ַ��У���
				ptCvtInfo->wCrc += wCrc;
			}

			if (ptCvtInfo->bPnNum >= PN_NUM_IN_CVT)
				break;
		}
	}
}

int FindCvtAddrToClear(TAllCvtInfo *tBackCvt, const BYTE bBackNum, TAllCvtInfo *ptAllCvt, const BYTE bCmpNum, TAllCvtInfo *tClearCvt)
{
	int i, j;
	BYTE bCount = 0;

	for (i=0; i<bBackNum; i++)
	{
		for (j=0; j<bCmpNum; j++)
		{
			if (memcmp(tBackCvt->bAllCvtAddr[i], &ptAllCvt->bAllCvtAddr[j], CVT_ADDR_LEN) == 0)
			{
				break;
			}
		}

		if(j == bCmpNum)
		{
			memcpy(tClearCvt->bAllCvtAddr[bCount], &tBackCvt->bAllCvtAddr[i], CVT_ADDR_LEN);
			tClearCvt->bAllPort[bCount] = tBackCvt->bAllPort[bCount];
			bCount++;
		}
	}

	return bCount;
}

