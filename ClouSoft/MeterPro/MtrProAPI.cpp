/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterPro.h
 * ժ    Ҫ�����ļ���Ҫ��������Э��Ļ���API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2011��3��
 * ��    ע��
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaAPI.h"
#include "MtrProAPI.h"
#include "FaConst.h"
#include "CctAPI.h"
#include "MtrCtrl.h"


struct TMtrPro g_MtrPro[3];
extern TMtrClkPrg g_MtrClkPrg;
TTmRunMode g_TmRunModeI;
TTmRunMode g_TmRunModeII;
TTmRunMode g_TmRunModeIII;

//����:ͨ���������ȡ����Ӧ������Ĳ����ͱ������
//����:	@wPn:�������
//		@pMtrPara:���ڷ��ظõ�������ָ��
bool LoadMtrPara(WORD wPn, TMtrPara* pMtrPara)
{
	if (pMtrPara == NULL)
		return false;

	///////////////////////
	return true;
}

//����:ͨ���������д���Ӧ�����㱣�����
//����:	@wPn:�������
//		@pSaveInf:���ڴ���õ���������ָ��
//		@pbUnsupIdFlg:���ڴ���õ�����ID�Ƿ�֧�ֱ�־�Ļ����ָ��
bool SaveMtrInfo(WORD wPn, TMtrSaveInf* pSaveInf)
{
	if (pSaveInf == NULL)
		return false;

	///////////////////////
	return true;
}

//����:���������Э��������
//����:	@wPn:�������
//		@pMtrPara:���������������ָ��
//		@pSaveInf:�����������������ָ��
//����ֵ:�ɹ��򷵻ؾ�����Э��ṹ��ָ��,ʧ���򷵻ؿ�ָ��
struct TMtrPro*  CreateMtrPro(WORD wPn, TMtrPara* pMtrPara, BYTE bThrId)
{
	g_MtrPro[bThrId].pMtrPara = pMtrPara;	

	switch (pMtrPara->bProId)
	{
	case PROTOCOLNO_DLT645:			
		g_MtrPro[bThrId].pMtrPara->pComm = &g_commRs485[bThrId];
		if ( Mtr645Init(&g_MtrPro[bThrId], bThrId) )
			return &g_MtrPro[bThrId];
		break;
	case PROTOCOLNO_DLT645_V07:
		g_MtrPro[bThrId].pMtrPara->pComm = &g_commRs485[bThrId];
		if ( Mtr645V07Init(&g_MtrPro[bThrId], bThrId) )
			return &g_MtrPro[bThrId];	
		break;		

	case PROTOCOLNO_DLT69845:
		g_MtrPro[bThrId].pMtrPara->pComm = &g_commRs485[bThrId];
		if ( Mtr69845Init(&g_MtrPro[bThrId], bThrId) )
			return &g_MtrPro[bThrId];
		break;

#ifdef EN_SBJC_V2
    case PROTOCOLNO_SBJC:
		g_MtrPro[bThrId].pMtrPara->pComm = &g_commRs485[bThrId];
        if ( Mtr645ExtInit(&g_MtrPro[bThrId], bThrId) )
            return &g_MtrPro[bThrId];	
        break;
#endif
	default:
		break;
	}

	return NULL;
}


//����:���������Э�����ݽӿ�
//����:	@pMtrPro: �������������Э��ṹ��ָ��
//		@wPn:�������
//		@wID:����ID
//		@pbBuf:���ڷ��س������ݵ�ָ��

int AskMtrItem(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	BYTE bPrintPro = pMtrPro->pMtrPara->bProId;
	char szProName[20];
	int iRet=0;

	pMtrPro->pfnGetProPrintType(&bPrintPro, szProName);	
	//DTRACE(bPrintPro, ("MtrPro=%s::Point=%d,read OAD=0x%08x.\r\n", szProName,pMtrPro->pMtrPara->wPn, dwOAD)); 

	iRet = pMtrPro->pfnAskItem(pMtrPro, bRespType, dwOAD, pbData, pbRSD, bLenRSD, pbRCSD, bLenRCSD);

	return  iRet;
}

//����ַ�㲥Уʱ����
int OneAddrBroadcast_485(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	BYTE bBuf1[100];
	BYTE bBuf2[50];
	int iRet = 0;
	int iStep = -1;
	TTime tmMtr,tmTm;
	BYTE bProId = pMtrPro->pMtrPara->bProId;
	BYTE btProId;
	char szProName[20];
	
	//BYTE bBuf3[50];
	memset(bBuf1, 0, sizeof(bBuf1));
	if (OoProReadAttr(0x4204, 3, 0, bBuf1, sizeof(bBuf1), &iStep) > 0)
	{
		if (bBuf1[9] == 1)//����
		{
			memset(bBuf2, 0, sizeof(bBuf2));
			GetCurTime(&tmTm);
			tmMtr.nYear = OoLongUnsignedToWord(&pbData[1]);
			tmMtr.nMonth = pbData[3];
			tmMtr.nDay= pbData[4];
			tmMtr.nHour= pbData[5];
			tmMtr.nMinute= pbData[6];
			tmMtr.nSecond= pbData[7];//��ʱ�ȹ̶���1���ͨ����ʱ
			if (IsInvalidTime(tmMtr))
				return -1;

			if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[5]*3600+bBuf1[6]*60+bBuf1[7]))
				return -1;
			
			DWORD dwSecM = TimeToSeconds(tmMtr);
			DWORD dwSecT = TimeToSeconds(tmTm);
			DWORD dwSecDiff = 0;
			//	return -1;
            if(dwSecM >= dwSecT)
            {
                dwSecDiff = dwSecM -dwSecT;
            }
            else
            {
                dwSecDiff = dwSecT - dwSecM;
            }
			//if (dwSecM > (dwSecT+bBuf1[3]) ||dwSecT > (dwSecM+bBuf1[3]))//���ʱ�䳬��
			if(dwSecDiff > abs((char)bBuf1[3]))
			{
				TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
				g_MtrClkPrg.bEvtSrcTSA[0] = DT_TSA;
				g_MtrClkPrg.bEvtSrcTSA[1] = pMtrPara->bAddr[0]+1;
				g_MtrClkPrg.bEvtSrcTSA[2] = pMtrPara->bAddr[0]-1;
				//g_MtrClkPrg.bEvtSrcTSA[1] = bTsaLen;
				memcpy(&g_MtrClkPrg.bEvtSrcTSA[3], &pMtrPara->bAddr[1], pMtrPara->bAddr[0]);
				memcpy(&g_MtrClkPrg.bClock[0], pbData, 8);
				g_MtrClkPrg.bClkErr[0] = DT_INT;
				pMtrPro->pfnGetProPrintType(&btProId, szProName);

				
				if (bProId == PROTOCOLNO_DLT645_V07 || bProId == PROTOCOLNO_DLT645)
				{
					BYTE bBufY[18]={0x68, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x08, 0x06, 0x01, 0x02, 0x03, 0x07, 0x03, 0x17, 0x5E, 0x16};

					revcpy(&bBufY[1], &pMtrPara->bAddr[1],6);
					bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
					bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
					bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
					bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
					bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
					bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
					bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);
					memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
					DTRACE(DB_METER, ("OneAddrBroadcast_485 MtrPro=%s::Point=%d\r\n", szProName,pMtrPara->wPn)); 
					iRet = pMtrPro->pfnWriteItem(pMtrPro, dwOAD, 0x04000101, sizeof(bBufY));
					//if (iRet < 0)
					//	return -1;
				}
				else if (bProId == PROTOCOLNO_DLT69845)
				{
					bBuf1[0] = 7;//ACTION_REQ
					bBuf1[1] = DL69845_APPSVR_GETREQUEST_NORMAL;	//GET-Request-NORMAL
					bBuf1[2] = 0; //PIID
					bBuf1[3] = 0x40;
					bBuf1[4] = 0x00;
					bBuf1[5] = 0x7f;
					bBuf1[6] = 0x00;
					bBuf1[7] = 0x1c;
					bBuf1[8] = tmTm.nYear>>8;
					bBuf1[9] = tmTm.nYear & 0x00ff;
					bBuf1[10] = tmTm.nMonth;
					bBuf1[11] = tmTm.nDay;
					bBuf1[12] = tmTm.nHour;
					bBuf1[13] = tmTm.nMinute;
					bBuf1[14] = tmTm.nSecond;
					bBuf1[15] = 0x00;	//ʱ���ǩ
					DTRACE(DB_METER, ("OneAddrBroadcast_485 MtrPro=%s::Point=%d,write id=0x40000200.\r\n", szProName,pMtrPara->wPn)); 
			
					iStep = Make69845Frm_485(&pMtrPara->bAddr[1], pMtrPara->bAddr[0], 0x43, 0, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);
					iRet = pMtrPro->pfnWriteItem(pMtrPro, dwOAD, 0x40000200, iStep);
					//if (iRet < 0)
					//	return -1;
					
				}
				memset(bBuf1, 0, sizeof(bBuf1));
				iRet = AskMtrItem(pMtrPro, bRespType, dwOAD, bBuf1, pbRSD, bLenRSD, pbRCSD, bLenRCSD);
				if (iRet > 0)
				{
					tmMtr.nYear = OoLongUnsignedToWord(&bBuf1[1]);
					tmMtr.nMonth = bBuf1[3];
					tmMtr.nDay= bBuf1[4];
					tmMtr.nHour= bBuf1[5];
					tmMtr.nMinute= bBuf1[6];
					tmMtr.nSecond= bBuf1[7];

					GetCurTime(&tmTm);
					dwSecM = TimeToSeconds(tmMtr);
					dwSecT = TimeToSeconds(tmTm);
					DWORD dwDel;
					BYTE bDel;
					if (dwSecT > dwSecM)
					{
						dwDel = dwSecT - dwSecM;
						if (dwDel > 127)
							bDel = 127;
						else
							bDel = dwDel;
						g_MtrClkPrg.bClkErr[1] =(bDel^0xff) + 1;//�����ĸ����Ǹ�������λȡ���ټ�1
					}
					else
					{
						dwDel = dwSecM - dwSecT;
						if (dwDel > 127)
							bDel = 127;
						else
							bDel = dwDel;
						g_MtrClkPrg.bClkErr[1] = bDel;//����Ϊ��
					}
				}
				else
					g_MtrClkPrg.bClkErr[1] = 1;//���������ʱ��͸���1��2�����ҵ����ֵ

				SetInfo(INFO_TERM_MTRCLKPRG);

				return 1;
			}
				
		}
	}
	return -1;
}
extern WORD CheckCRC16(BYTE *pInBuf, int iInLen);

int Make69845Frm_485(BYTE *pbMtr, BYTE bMtrLen, BYTE bCtrl, BYTE bAFType, BYTE bCA, WORD wFramFmt, BYTE *pbAPDU, WORD wAPDULen, BYTE *pbRespBuf)
{
	WORD wLen;
	BYTE *p = pbRespBuf;
	BYTE *pHeadCs;	//֡ͷУ��	

	//--------------֡ͷ--------------
	*p++ = 0x68;
	p += 0x02;
	*p++ = bCtrl;
	*p++ = ((bMtrLen-1)&0x0f) | (bAFType<<6);	
	revcpy(p, pbMtr, bMtrLen);
	//memcpy(p, pbMtr, bMtrLen);
	p += bMtrLen;
	*p++ = bCA;
	pHeadCs = p;
	p += 0x02;

	//-----------��·�û�����-----------
	if (bCtrl & 0x20)	//������־λPRM
	{
		WordToByte(wFramFmt, p);
		p += 2;
	}
	memcpy(p, pbAPDU, wAPDULen);
	p += wAPDULen;

	//-----------�������ݳ���------------
	wLen = (p - pbRespBuf) + 0x02 - 0x01;
	WordToByte(wLen, &pbRespBuf[1]);

	//-----------֡ͷУ��------------
	WordToByte(CheckCRC16(pbRespBuf+0x01, pHeadCs-pbRespBuf-0x01), pHeadCs);

	//-----------֡У��------------
	WordToByte(CheckCRC16(pbRespBuf+0x01, p-pbRespBuf-0x01), p);
	p += 2;
	*p++ = 0x16;

	return p - pbRespBuf;
}

TTime tLastTime[31];//���ڼ�¼���˿ڹ㲥Уʱ�͵���ַУʱ��ִ��ʱ��
//tLastTime[30]-��¼�ز�����ַ
//tLastTime[15]-��¼�ز��㲥
//tLastTime[port1],tLastTime[port[port2],485 I��485-II�ĵ���ַУʱ
//tLastTime[16+port1],tLastTime[port[16+port2],485 I��485-II�Ĺ㲥Уʱ

extern TMtrPara g_MtrPara[LOGIC_PORT_NUM];
extern const BYTE* Get485PnMask( );
extern bool GetMeterPara(WORD wPn, TMtrPara* pMtrPara);
extern WORD SearchPnFromMask(const BYTE* pbPnMask, WORD wStartPn);
//485����ַ�㲥Уʱ�����ܽӿ�
int MtrBroadcast_485(BYTE bThrId)
{
	TRdItem tRdItem;
	int m_iPn = 0;
	int iRet;
	BYTE bRespData[512];
	TTime tmTm;
	BYTE bBuf1[100];
	int iStep = -1;
	BYTE bPort;
	
	if (GetInfo(INFO_ONE_BRAODCAST_ARG_485))
	{
		DTRACE(DB_METER, ("INFO_ONE_BRAODCAST_ARG_485 \r\n")); 
		memset((BYTE *)&tLastTime[0], 0, sizeof(TTime));
		memset((BYTE *)&tLastTime[1], 0, sizeof(TTime));
		memset((BYTE *)&tLastTime[2], 0, sizeof(TTime));
		//SetInfo(INFO_ONE_BRAODCAST_ARG_CCT);
	}
	bPort = LOGIC_PORT_MIN + bThrId;
	if (OoProReadAttr(0x4204, 3, 0, bBuf1, sizeof(bBuf1), &iStep) > 0)
	{
		if (bBuf1[9] == 1)//����
		{
			GetCurTime(&tmTm);
			if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[5]*3600+bBuf1[6]*60+bBuf1[7]))
				return -1;

			if (tmTm.nYear == tLastTime[bThrId].nYear && tmTm.nMonth== tLastTime[bThrId].nMonth && tmTm.nDay== tLastTime[bThrId].nDay)//һ��ִֻ��һ��
				return -1;
			DTRACE(DB_METER, ("star 485 mtrBroadcast \r\n")); 
			tLastTime[bThrId] = tmTm;
			memset((BYTE*)&tRdItem, 0, sizeof(tRdItem));
			tRdItem.bReqType = 1;

			const BYTE* pbPnMask = Get485PnMask();	//ȡ�÷��ز���485����λ.
			while (1)
			{
				if (pbPnMask == NULL)
				{
					Sleep(500);
					break;
				}
				m_iPn = SearchPnFromMask(pbPnMask, m_iPn);	//�����ѳ��Ĳ����㶼��485��
				if (m_iPn >= POINT_NUM)
				{
					//Sleep(500);
					break;
				}
				if (GetPnPort(m_iPn) != bPort) //���Ǳ��˿ڵĵ��ܱ�
				{
					m_iPn++;
					continue;
				}

				GetMeterPara(m_iPn, &g_MtrPara[bThrId]);
				TMtrPro * pMtrPro = CreateMtrPro(m_iPn, &g_MtrPara[bThrId], bThrId);

				tRdItem.dwOAD = 0x40000200;
				tRdItem.wRsdLen = 0;
				tRdItem.wRcsdLen = 0;
				tRdItem.bReqType = 1;
				memset(tRdItem.bRSD, 0, 50);
				memset(tRdItem.bRCSD, 0, 50);
                WaitSemaphore(g_semRdMtr[bThrId]);
				iRet = AskMtrItem(pMtrPro, tRdItem.bReqType, tRdItem.dwOAD, bRespData, tRdItem.bRSD, tRdItem.wRsdLen, tRdItem.bRCSD, tRdItem.wRcsdLen);
				if (iRet > 0)
				{
					OneAddrBroadcast_485(pMtrPro, tRdItem.bReqType, tRdItem.dwOAD, bRespData, tRdItem.bRSD, tRdItem.wRsdLen, tRdItem.bRCSD, tRdItem.wRcsdLen);
				}
                SignalSemaphore(g_semRdMtr[bThrId]);
                Sleep(10);
				m_iPn++;
				if (m_iPn >= POINT_NUM)
				{
					Sleep(500);
				}

			}
		}
	}

	return 0;
}
extern DWORD GbValToBaudrate(BYTE val);
int BroadcastAdjustTime_485(BYTE bThrId)
{
	TRdItem tRdItem;
	int m_iPn = 0;
	int iRet;
	TTime tmTm;
	BYTE bBuf1[100];
	int iStep = -1;
	BYTE bPort;
    BYTE bMtrPort;
	bool fIsNeedBCTm645 = false;
    bool fIsNeedBCTm698 = false;
	BYTE bMtrPro = 0;
	TMtrPro * pMtrPro = NULL;
	
	if (GetInfo(INFO_MTR_BRAODCAST_ARG_485))
	{
		DTRACE(DB_METER, ("INFO_MTR_BRAODCAST_ARG_485 \r\n")); 
		memset((BYTE *)&tLastTime[16], 0, sizeof(TTime));//����ĸ�485�߳��յ��˾Ͱ�485��ʱ�䶼����
		memset((BYTE *)&tLastTime[16+bThrId], 0, sizeof(TTime));
		//memset((BYTE *)&tLastTime[16+bThrId+1], 0, sizeof(TTime));
		//memset((BYTE *)&tLastTime[16+bThrId+2], 0, sizeof(TTime));
	}
	
	const BYTE* pbPnMask = Get485PnMask();	//ȡ�÷��ز���485����λ.
	if (pbPnMask == NULL)
	{
		return -1;
	}

    if(IsAllAByte(pbPnMask, 0x00, PN_MASK_SIZE))
	{
		return -1;
	}
	
	bPort = LOGIC_PORT_MIN + bThrId;
	//���ڹ㲥����ֻ���ǰ�645��ķ�һ���ٰ������ķ�һ��
	memset(bBuf1, 0, sizeof(bBuf1));
	tRdItem.bReqType = 1;
	tRdItem.dwOAD = 0x40000200;
	BYTE bBufY[18]={0x68, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x68, 0x08, 0x06, 0x01, 0x02, 0x03, 0x07, 0x03, 0x17, 0x5E, 0x16};
	if (OoProReadAttr(0x4204, 2, 0, bBuf1, sizeof(bBuf1), &iStep) > 0)
	{
		if (bBuf1[7] == 1)//����
		{
			GetCurTime(&tmTm);
			if ((tmTm.nHour*3600+tmTm.nMinute*60+tmTm.nSecond) < (bBuf1[3]*3600+bBuf1[4]*60+bBuf1[5]))
				return -1;

			if (tmTm.nYear == tLastTime[16+bThrId].nYear && tmTm.nMonth== tLastTime[16+bThrId].nMonth && tmTm.nDay== tLastTime[16+bThrId].nDay)//һ��ִֻ��һ��
				return -1;
			
			memcpy( (BYTE *)&tLastTime[16+bThrId].nYear,(BYTE *)&tmTm, sizeof(TTime));
            while(m_iPn < POINT_NUM)
            {
                m_iPn = SearchPnFromMask(pbPnMask, m_iPn);  //�����ѳ��Ĳ����㶼��485��
                if(m_iPn >= POINT_NUM)
                {
                    break;
                }

                bMtrPort = GetPnPort(m_iPn);
                if(bMtrPort != bPort)
                {
                    m_iPn++;
                    continue;
                }
                bMtrPro = GetMeterPro(m_iPn);
                if(bMtrPro == PROTOCOLNO_DLT69845)
                {
                    if(!fIsNeedBCTm698)
                    {
                        fIsNeedBCTm698 = true;
                    }
                }
                else if(bMtrPro == PROTOCOLNO_DLT645 || bMtrPro == PROTOCOLNO_DLT645_V07)
                {
                    if(!fIsNeedBCTm645)
                    {
                        fIsNeedBCTm645 = true;
                    }                    
                }
                if(fIsNeedBCTm645 && fIsNeedBCTm698)
                {
                    break;
                }
                m_iPn++;
            }
            if (m_iPn >= POINT_NUM && !fIsNeedBCTm645 && !fIsNeedBCTm698 )
            {
                return -1;
            }            
			TMtrPara tMtrPara;
			tMtrPara.CommPara.wPort = MeterPortToPhy(bPort);
            tMtrPara.CommPara.bByteSize = 8;
            tMtrPara.CommPara.bStopBits = ONESTOPBIT;
		    tMtrPara.CommPara.bParity = EVENPARITY;
            
            if(fIsNeedBCTm645)
            {
                memset(tMtrPara.bAddr, 0x99, 6);
                tMtrPara.bProId = PROTOCOLNO_DLT645;//�㲥Уʱ97��07һ����,�������ʲ�һ����
                
				tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(2);//1200;				
    			pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
	            if(pMtrPro == NULL)
				{
	            	return -1;
	            }

                WaitSemaphore(g_semRdMtr[bThrId]); 
	    	    GetCurTime(&tmTm);
				bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
				bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
				bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
				bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
				bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
				bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
				bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);
				memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
				DTRACE(DB_CCT, ("bPort %d Broadcast 645-1200 \r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x04000101, sizeof(bBufY));
                SignalSemaphore(g_semRdMtr[bThrId]); 
                
				tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(3);//2400;
				pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
                if(pMtrPro==NULL)
                {
                    return -1;
                }
                WaitSemaphore(g_semRdMtr[bThrId]); 
                GetCurTime(&tmTm);
				bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
				bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
				bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
				bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
				bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
				bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
				bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);				
				memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
    			DTRACE(DB_CCT, ("bPort %d Broadcast 645-2400 \r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x04000101, sizeof(bBufY));
                SignalSemaphore(g_semRdMtr[bThrId]); 
                
                tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(4);//4800;
                pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
                if(pMtrPro==NULL)
                {
                    return -1;
                }
                WaitSemaphore(g_semRdMtr[bThrId]); 
                GetCurTime(&tmTm);				
				bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
				bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
				bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
				bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
				bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
				bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
				bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);				
				memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
    			DTRACE(DB_CCT, ("bPort %d Broadcast 645-4800 \r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x04000101, sizeof(bBufY));
                SignalSemaphore(g_semRdMtr[bThrId]); 
                
                tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(6);//9600;
                pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
                if(pMtrPro==NULL)
                {
                    return -1;
                }
                WaitSemaphore(g_semRdMtr[bThrId]); 
                GetCurTime(&tmTm);				
				bBufY[10] = ByteToBcd(tmTm.nSecond)+0x33;
				bBufY[11] = ByteToBcd(tmTm.nMinute)+0x33;
				bBufY[12] = ByteToBcd(tmTm.nHour%100)+0x33;
				bBufY[13] = ByteToBcd(tmTm.nDay)+0x33;
				bBufY[14] = ByteToBcd(tmTm.nMonth)+0x33;
				bBufY[15] = ByteToBcd(tmTm.nYear%100)+0x33;
				bBufY[16] = CheckSum(bBufY, sizeof(bBufY)-2);				
				memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
    			DTRACE(DB_CCT, ("bPort %d Broadcast 645-9600 \r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x04000101, sizeof(bBufY));
                SignalSemaphore(g_semRdMtr[bThrId]); 
			}
			//if (bProId == PROTOCOLNO_DLT69845)//�����Ĺ㲥Уʱ����ַ����������???????
			if(fIsNeedBCTm698)
			{
				bBuf1[0] = 7;//ACTION_REQ
				bBuf1[1] = DL69845_APPSVR_GETREQUEST_NORMAL;	//GET-Request-NORMAL
				bBuf1[2] = 0; //PIID
				bBuf1[3] = 0x40;
				bBuf1[4] = 0x00;
				bBuf1[5] = 0x7f;// ����127
				bBuf1[6] = 0x00;
				bBuf1[7] = 0x1c;
				bBuf1[8] = tmTm.nYear>>8;
				bBuf1[9] = tmTm.nYear & 0x00ff;
				bBuf1[10] = tmTm.nMonth;
				bBuf1[11] = tmTm.nDay;
				bBuf1[12] = tmTm.nHour;
				bBuf1[13] = tmTm.nMinute;
				bBuf1[14] = tmTm.nSecond;
				bBuf1[15] = 0x00;	//ʱ���ǩ
			
				bBufY[0] = 0xC1;
				bBufY[1] = 0xAA;
				tMtrPara.bProId = PROTOCOLNO_DLT69845;

                tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(2);
				pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
                if(pMtrPro == NULL)
                {
                    return -1;
                }
                WaitSemaphore(g_semRdMtr[bThrId]);
				GetCurTime(&tmTm);
				bBuf1[8] = tmTm.nYear>>8;
			    bBuf1[9] = tmTm.nYear & 0x00ff;
				bBuf1[10] = tmTm.nMonth;
				bBuf1[11] = tmTm.nDay;
				bBuf1[12] = tmTm.nHour;
				bBuf1[13] = tmTm.nMinute;
				bBuf1[14] = tmTm.nSecond;			
				iStep = Make69845Frm_485(bBufY+1, 1, 0x43, 3, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);//��˼�ǹ㲥ʱ�ĵ�ַ��Ϊ��  C1 AA
				DTRACE(DB_CCT, ("bPort %d Broadcast 69845-1200\r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);
                SignalSemaphore(g_semRdMtr[bThrId]); 

                tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(3);
				pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
                if(pMtrPro == NULL)
                {
                    return -1;
                }
                WaitSemaphore(g_semRdMtr[bThrId]);
				GetCurTime(&tmTm);
				bBuf1[8] = tmTm.nYear>>8;
			    bBuf1[9] = tmTm.nYear & 0x00ff;
				bBuf1[10] = tmTm.nMonth;
				bBuf1[11] = tmTm.nDay;
				bBuf1[12] = tmTm.nHour;
				bBuf1[13] = tmTm.nMinute;
				bBuf1[14] = tmTm.nSecond;				
				iStep = Make69845Frm_485(bBufY+1, 1, 0x43, 3, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);
				DTRACE(DB_CCT, ("bPort %d Broadcast 69845-2400\r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);
                SignalSemaphore(g_semRdMtr[bThrId]); 

                tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(4);
                pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
                if(pMtrPro == NULL)
                {
                    return -1;
                }
                WaitSemaphore(g_semRdMtr[bThrId]); 
				GetCurTime(&tmTm);
				bBuf1[8] = tmTm.nYear>>8;
			    bBuf1[9] = tmTm.nYear & 0x00ff;
				bBuf1[10] = tmTm.nMonth;
				bBuf1[11] = tmTm.nDay;
				bBuf1[12] = tmTm.nHour;
				bBuf1[13] = tmTm.nMinute;
				bBuf1[14] = tmTm.nSecond;				
				iStep = Make69845Frm_485(bBufY+1, 1, 0x43, 3, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);
				DTRACE(DB_CCT, ("bPort %d Broadcast 69845-4800\r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);
                SignalSemaphore(g_semRdMtr[bThrId]);
                
				tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(6);
				pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
                if(pMtrPro == NULL)
                {
                    return -1;
                }
                WaitSemaphore(g_semRdMtr[bThrId]); 
				GetCurTime(&tmTm);
				bBuf1[8] = tmTm.nYear>>8;
			    bBuf1[9] = tmTm.nYear & 0x00ff;
				bBuf1[10] = tmTm.nMonth;
				bBuf1[11] = tmTm.nDay;
				bBuf1[12] = tmTm.nHour;
				bBuf1[13] = tmTm.nMinute;
				bBuf1[14] = tmTm.nSecond;
				iStep = Make69845Frm_485(bBufY+1, 1, 0x43, 3, 0, 0, bBuf1, 16, pMtrPro->pbTxBuf);
				DTRACE(DB_CCT, ("bPort %d Broadcast 69845-9600\r\n",bPort)); 
				iRet = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);
                SignalSemaphore(g_semRdMtr[bThrId]); 
			}
		}
	}
	return 0;
}

void IsInTestMode(BYTE bThrId)
{
	TRdItem tRdItem;
	int iStep = -1;

	BYTE bPort;
	WORD wPnSum = 0;
	BYTE bBuf[10];
	TTmRunMode tmpmode;
	wPnSum = GetPnNum();
	int iRet1, iRet2;
	//if (bThrId != 0)//ֻ��485 I  �����˳���
	//	return;
	if (bThrId == 2)//��III·Ϊ���е��Կ�
		return;
	
	bPort = LOGIC_PORT_MIN + bThrId;
	if (ReadBankId(BANK1, PN0, 0x2111, bBuf) <0 )
		bBuf[0] = 8;
	if (g_TmRunModeI.bTmRunMode == 1 ||g_TmRunModeII.bTmRunMode==1 ||g_TmRunModeIII.bTmRunMode==1)//�κ�һ����⵽�˾Ͳ��ټ���
		return;
	if (bThrId == 0)
		tmpmode = g_TmRunModeI;
	else if (bThrId == 1)
		tmpmode = g_TmRunModeII;
	else
		tmpmode = g_TmRunModeII;
	
	if (wPnSum > bBuf[0])//�϶�Ϊ�ֳ�ģʽ����ȥ���Զ�ģ�����
	{
			tmpmode.bTmRunMode = 0;
	}
	else
	{
		TTime now;
		GetCurTime(&now);
		if (tmpmode.bTmRunMode == 1)
		{
			DWORD dwCurHour = TimeToMinutes(now)/60;
			DWORD dwRdHour = TimeToMinutes(tmpmode.tTimeRd)/60;
			if (dwCurHour == dwRdHour)//�ڲ���ģʽ�����Ҳ1Сʱȥ��ȷ��һ��
				return;
		}
		else
		{
			DWORD dwCurMin = TimeToMinutes(now);
			DWORD dwRdMin = TimeToMinutes(tmpmode.tTimeRd);
			if (dwCurMin == dwRdMin)//���ڲ���ģʽʱ1���ӳ���һ��
				return;
		}
		BYTE bBufY[25]={0x68, 0x17, 0x00, 0x43, 0x05, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x00, 0x9E, 0x50, 0x05, 0x01, 0x02, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00, 0x17, 0x16};
		BYTE bBufX[25]={0x68, 0x17, 0x00, 0x43, 0x05, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x00, 0xAC, 0xEF, 0x05, 0x01, 0x03, 0x40, 0x00, 0x02, 0x00, 0x00, 0x2B, 0x13, 0x16};

		TMtrPara tMtrPara;
		WORD m_iPn = 1;
		tMtrPara.CommPara.wPort = MeterPortToPhy(bPort);
		tMtrPara.CommPara.dwBaudRate = GbValToBaudrate(3);//2400;
		tMtrPara.CommPara.bByteSize = 8;
		tMtrPara.CommPara.bStopBits = ONESTOPBIT;
		tMtrPara.CommPara.bParity = EVENPARITY;

		tMtrPara.bProId = PROTOCOLNO_DLT69845;
		TMtrPro * pMtrPro = CreateMtrPro(m_iPn, &tMtrPara, bThrId);
		DTRACE(DB_METER, ("bPort %d test read-2400\r\n",bPort)); 
		memcpy(pMtrPro->pbTxBuf, bBufY, sizeof(bBufY));
		iStep = sizeof(bBufY);
		
		tRdItem.bReqType = 1;
		tRdItem.dwOAD = 0x40000200;
		iRet1 = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);
		if ((iRet1 >= 0))
		{
			memcpy(pMtrPro->pbTxBuf, bBufX, sizeof(bBufX));
			iStep = sizeof(bBufX);
			iRet2 = pMtrPro->pfnWriteItem(pMtrPro, tRdItem.dwOAD, 0x40000200, iStep);
			if (iRet2 >= 0)
			{
				tmpmode.bTmRunMode = 1;//˵��Ϊ����ģʽ
				DTRACE(DB_METER, ("bPort %d is test mode\r\n",bPort)); 
			}
			else
				tmpmode.bTmRunMode = 0;
		}
		else
			tmpmode.bTmRunMode = 0;
		
		memcpy(&tmpmode.tTimeRd, &now, sizeof(TTime));

	}

	if (bThrId == 0)
		g_TmRunModeI = tmpmode;
	else if (bThrId == 1)
		g_TmRunModeII = tmpmode;
	else
		g_TmRunModeII = tmpmode;

}


