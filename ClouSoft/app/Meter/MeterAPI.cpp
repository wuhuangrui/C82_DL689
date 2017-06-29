/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֳ���Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��9��
 *********************************************************************************************************/
#include "stdafx.h"
#include "MeterAPI.h"
#include "DbAPI.h"
#include "FaAPI.h"
#include "MtrCtrl.h"
#include "bios.h"
#include "DbOIAPI.h"
#include "CctAPI.h"
#include "Mem.h"

DWORD GbValToBaudrate(BYTE val)
{
	static DWORD dwBaudrate[8] = {0, CBR_600, CBR_1200, CBR_2400, 
								  CBR_4800, CBR_4800, CBR_9600, CBR_19200};
	if (val <= 7)
		return dwBaudrate[val];
	else
		return CBR_1200;
}

BYTE GbValToParity(BYTE val)
{
	static BYTE bParityTab[] = {NOPARITY, EVENPARITY, EVENPARITY}; 

	if (val < 3)
		return bParityTab[val];
	else	
		return NOPARITY;
}


BYTE GbValToStopBits(BYTE val)
{
	static BYTE bStopBitsTab[] = {ONESTOPBIT, TWOSTOPBITS, ONE5STOPBITS};
	if (val>0 && val<=3)
		return bStopBitsTab[val-1];
	else
		return ONESTOPBIT;
}

BYTE GbValToByteSize(BYTE val)
{
	if (val>=5 && val<=8)
		return val;
	else
		return 8;
}

//��������ʼ�����������
void Init485MtrMask()
{
	BYTE b485MtrMask[PN_MASK_SIZE] = {0};

	for (WORD wPn = 1; wPn < POINT_NUM; wPn++)
	{
		if (IsMtrPn(wPn))
		{
			b485MtrMask[wPn/8] |= 1<<(wPn%8);
		}
	}

	WriteItemEx(BANK17, PN0, 0x6002, b485MtrMask);
}

//����: DLMSЭ��ĵ�������ʼ��
//����: @wPn �������
//		@pMtrPara ָ���ŵ���������ĵ������ṹָ��
//����: �ɹ��򷵻�true		
bool GetMeterPara(WORD wPn, TMtrPara* pMtrPara)
{	
	int iPort = 0;
	DWORD dwPort = 0;
	BYTE bBuf[PNPARA_LEN];	
	
	memset(bBuf, 0,sizeof(bBuf));	
	if (ReadItemEx(BN0, wPn, 0x6000, bBuf) <= 0)
		return false;

	memset(pMtrPara, 0, sizeof(TMtrPara));

	//��������
	pMtrPara->wPn = wPn;
	//BYTE bAddL = bBuf[9];
	BYTE bAddL = bBuf[10]+1;
	pMtrPara->bAddr[0] = bAddL;	//��ַ����
	memcpy(&pMtrPara->bAddr[1], &bBuf[11], bAddL);	  //��ַ����

	pMtrPara->bProId = bBuf[14+bAddL];				  //��Լ����
#ifdef EN_SBJC_V2
	pMtrPara->bSubProId = GetMeterSubPro(wPn);
    if (pMtrPara->bProId == PROTOCOLNO_SBJC)
    {
        memcpy(pMtrPara->bMtrAddr, &bBuf[12], 7);	//6	
       // pMtrPara->bMtrAddr[6] = GetMeterType(wPn);//tll
    }
#endif
	dwPort = OoOadToDWord(&bBuf[16+bAddL]);
	if ((dwPort&0xFFFFFF00) == 0xF2090200)	//�ز��˿�
	{
		//DTRACE(DB_METER, ("GetMeterPara : plc port.\n"));
		iPort = PORT_CCT_PLC;
	}
	else if ((dwPort&0xFFFFFF00) == 0xF2010200)	//485�˿�
	{
		iPort = MeterPortToPhy(bBuf[19+bAddL]); // ������߼��˿ڵ�����˿ڵ�ӳ��
		if (iPort < 0)
		{
			DTRACE(DB_METER, ("GetMeterPara : fail to map port %d to physic\n", bBuf[19+bAddL]));
			return false;
		}
	}
	else
	{
		//DTRACE(DB_METER, ("GetMeterPara : port oad = 0x%08x invalid!\n", dwPort));
		return false;
	}

	pMtrPara->CommPara.wPort = (WORD )iPort;
	pMtrPara->CommPara.dwBaudRate = GbValToBaudrate(bBuf[12+bAddL]); //42	
	pMtrPara->CommPara.bByteSize = 8; //GbValToByteSize(bBuf[11]);  //44
	pMtrPara->CommPara.bStopBits = ONESTOPBIT; //GbValToStopBits(bBuf[13]);  //45
	pMtrPara->CommPara.bParity = EVENPARITY; //GbValToParity(bBuf[14]);	  //43

	return true;
}

//����:	��ȡ����ַ,
//����:	@wPn �������
//		@pbAddr �������ص�ַ
//����:	����ɹ��򷵻�true,���򷵻�false
//��ע:	�����ز���, �ز������ַ�����ַһ��,
//		���ڲɼ���ģ��,����ȡĿ�ĵ���ַ.
BYTE GetMeterAddr(WORD wPn, BYTE* pbAddr)
{
	BYTE bBuf[PNPARA_LEN];

	if (ReadItemEx(BN0, wPn, 0x6000, bBuf)<=0)
		return 0;

	BYTE bAddL = bBuf[9] + 1;
	memcpy(pbAddr, &bBuf[9], bAddL);
	return bAddL;
}

//����:	��ȡ������ͨ����
//����:	@wPn �������
BYTE GetPnCn(WORD wPn)
{
	BYTE bBuf[PNPARA_LEN];

	if (ReadItemEx(BN0, wPn, 0x6000, bBuf)<=0)
		return 0;

	BYTE bAddL = bBuf[9] + 1;

	return bBuf[18+bAddL];
}

//����:ȡ������
WORD GetMeterInterv()
{
	BYTE bMeterInterv = 0;
	
	if (ReadItemEx(BN0, PN0, 0x6708, &bMeterInterv) > 0)	//�ն�����
	{	
		//���û������,ϵͳ��Ĭ��ֵΪ0,������Զ�ȡĬ��ֵ
	}
	else
	{
		bMeterInterv = 15;
	}

	if (bMeterInterv==0 || bMeterInterv>60)
		bMeterInterv = 15;

	return bMeterInterv;
}

///////////////////////////////////////////////////////////////////////////////////////
bool InitMeter()
{
	//Init485MtrMask();
	MtrCtrlInit();
	InitMtrCacheCtrl();
	return true;
}

void NewMeterThread()
{
	NewThread(MtrRdThread, (void * )0, 8192, THREAD_PRIORITY_NORMAL);
	NewThread(MtrRdThread, (void * )1, 8192, THREAD_PRIORITY_NORMAL);
}

///////////////////////////////////////////////////////////////////////////////////////
//��ʼ���������ƽṹ
void InitMtrCacheCtrl()
{
	BYTE bLen, bTsa[17];
	BYTE bCacheCnt = 0;

	memset(g_MtrCacheCtrl, 0, sizeof(g_MtrCacheCtrl));

	for (WORD wPn=1; wPn<POINT_NUM; wPn++)
	{
		if (IsMtrPn(wPn))
		{
			bLen = GetMeterAddr(wPn, bTsa);
			g_MtrCacheCtrl[bCacheCnt].bStatus = CACHE_STATUS_IDLE;
			g_MtrCacheCtrl[bCacheCnt].wPn = wPn;
			g_MtrCacheCtrl[bCacheCnt].dwCacheTime = GetCurTime();
			g_MtrCacheCtrl[bCacheCnt].dwLastAcessTime = GetCurTime();
			g_MtrCacheCtrl[bCacheCnt].fDirty = false;
			g_MtrCacheCtrl[bCacheCnt].fTrigerSave = false;
			InitMtrRdCtrl(wPn, bTsa, &g_MtrCacheCtrl[bCacheCnt].mtrRdCtrl);
			bCacheCnt++;
			if (bCacheCnt == MTR_CACHE_NUM)
				break;
		}
	}
}

//���µ������ƽṹ
void RefreshMtrCacheCtrl()
{
	char szName[64];

	for (WORD wPn=1; wPn<POINT_NUM; wPn++)
	{
		sprintf(szName, USER_DATA_PATH"MtrRdCtrl_Pn%d.dat", wPn);
		DeleteFile(szName);
	}

	InitMtrCacheCtrl();
}

//��������ʼ����������ƽṹ
//������@wPn
//		@pbTsa
//		@pMtrRdCtrl
//���أ���
void InitMtrRdCtrl(WORD wPn, BYTE* pbTsa, TMtrRdCtrl* pMtrRdCtrl)
{
	TTaskCfg tTaskCfg; 
	int iSchCfgLen;
	BYTE bType;
	BYTE* pbMs, *pbArry, *pbSch;
	WORD wLen, wFmtLen, wNum;
	BYTE *pFmt, *pbCollMode;
	WORD* pwLen;
	DWORD* pdwOAD, dwOAD;
	TTimeInterv tiExe;	 //�ɼ�����
	BYTE bPnMask[PN_MASK_SIZE];

	memset(pMtrRdCtrl, 0, sizeof(TMtrRdCtrl));

	pdwOAD = MtrGetFixedItems(&wNum);
	pwLen = MtrGetFixedLen();
	InitMtrTmpData(&pMtrRdCtrl->mtrTmpData, pdwOAD, pwLen, wNum);

	pMtrRdCtrl->bTaskSN = GetTaskCfgSn();
	memcpy(pMtrRdCtrl->bTsa, pbTsa, pbTsa[0]+1);
	for (WORD wIndex= 0; wIndex<TASK_NUM; wIndex++)	//�����������ñ�
	{
		memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
		if (GetTaskCfg(wIndex, &tTaskCfg))
		{
			pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
			if (pbSch!=NULL)
			{
				switch (tTaskCfg.bSchType)
				{
				case SCH_TYPE_COMM:
					pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
					pbMs = OoGetField(pbSch, pFmt, wFmtLen, 4, &wLen, &bType);	//MS
					if (pbMs == NULL)
						break;
					memset(bPnMask, 0, sizeof(bPnMask));
					ParserMsParam(pbMs, bPnMask, sizeof(bPnMask));
					if ((bPnMask[wPn/8]&(1<<(wPn%8))) != 0)
					{
						pbArry = OoGetField(pbSch, pFmt, wFmtLen, 3, &wLen, &bType);	//array CSD
						int iLen = OoGetDataLen(bType, pbArry+1);
						if (iLen > 0)
						{
							AllocTmpRec(pMtrRdCtrl, MEM_TYPE_TASK, &tTaskCfg, *(pbArry+1), iLen);
						}
					
						dwOAD = OoOadToDWord(pbArry+4);
						if (dwOAD == 0x50020200) //������������
						{
							pbCollMode = OoGetField(pbSch, pFmt, wFmtLen, 2, &wLen, &bType);	//�ɼ���ʽ
							bType = pbCollMode[3];
							tiExe.bUnit = pbCollMode[5];
							tiExe.wVal = OoLongUnsignedToWord(pbCollMode+6);
							if (bType==3 /*&& tiExe.bUnit==1*/) //��TI����ɼ�
							{
								DWORD dwTiMin = TiToSecondes((TTimeInterv*)&tiExe)/60;
								if (dwTiMin == 0)
									dwTiMin = 15;
								wLen = (TiToSecondes((TTimeInterv*)&tTaskCfg.tiExe)/60/dwTiMin + 7)/8; //ÿ���ռ1����
								if (wLen > 0)
									AllocMem(pMtrRdCtrl->bGlobal, pMtrRdCtrl->allocTab, MTR_TAB_NUM, MEM_TYPE_CURVE_FLG, tTaskCfg.bTaskId, wLen);
							}
						}
					}
					break;

				case SCH_TYPE_EVENT:
					pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
					pbMs = OoGetField(pbSch, pFmt, wFmtLen, 2, &wLen, &bType);	//MS
					if (pbMs == NULL)
						break;
					memset(bPnMask, 0, sizeof(bPnMask));
					ParserMsParam(pbMs, bPnMask, sizeof(bPnMask));
					if ((bPnMask[wPn/8]&(1<<(wPn%8))) != 0)
					{
						pbArry = OoGetField(pbSch, pFmt, wFmtLen, 1, &wLen, &bType);	//array CSD
						pbArry += 4;
						int iLen = OoGetDataLen(*pbArry, pbArry+1);
						if (iLen > 0)
						{
							AllocTmpRec(pMtrRdCtrl, MEM_TYPE_EVT_ACQ, &tTaskCfg, *(pbArry+1), iLen);
						}
					}
					break;

				//case SCH_TYPE_TRANS: (͸�������������ﴦ��)
				//	break;

				case SCH_TYPE_REPORT:
					break;

				case SCH_TYPE_SCRIPT:
					break;

				case SCH_TYPE_REAL:
					break;
				}
			}
		}
	}

	//����Ϊ�����¼���ʼ��
	InitMtrExcCtrl(wPn, &pMtrRdCtrl->mtrExcTmp);	//��ʼ�������¼����ƽṹ

	AllocateMtrExcMem(pMtrRdCtrl->bGlobal, pMtrRdCtrl->allocTab, MTR_TAB_NUM);	//���䳭���¼���ʱ�ڴ�ռ�
}

void DoMangerMtrCacheCtrl()
{
	WaitSemaphore(g_semMtrCtrl);
	//����������ƽṹ
	for (BYTE bIndex=0; bIndex<MTR_CACHE_NUM; bIndex++)	
	{
		//����ṹ��Ч && Ϊ���� && ����ʱ�䳬��10����
		if (g_MtrCacheCtrl[bIndex].bStatus==CACHE_STATUS_IDLE 
			&& (g_MtrCacheCtrl[bIndex].fTrigerSave || (g_MtrCacheCtrl[bIndex].fDirty
			&& (abs(GetCurTime()-g_MtrCacheCtrl[bIndex].dwCacheTime)>10*60))))
		{
			SaveMtrRdCtrl(g_MtrCacheCtrl[bIndex].wPn, &g_MtrCacheCtrl[bIndex].mtrRdCtrl);
			//memset((BYTE*)&g_MtrCacheCtrl[bIndex], 0, sizeof(TMtrCacheCtrl));
			g_MtrCacheCtrl[bIndex].dwCacheTime = GetCurTime();
			DTRACE(DB_METER, ("DoMangerMtrCacheCtrl: wPn=%d, bIndex=%d.\n", g_MtrCacheCtrl[bIndex].wPn, bIndex));
		}
	}
	SignalSemaphore(g_semMtrCtrl);
}

//////////////////////////////////////////////////////////////////////////////////////
//��������ƽṹ�ӿں�������
//˵����
//1�����浽�ļ�ϵͳ�е��ļ���MtrRdCtrl_Pn%d.dat������
//2��g_MtrCacheCtrl[]ƥ��ĳ�����Ŀ��ƽṹ�Ƿ񻺴����ڴ�ʱ��ֻ��ҪwPnƥ�伴�ɣ�bTsa��Ҫ�����Ƚ��ļ�����ĵ���ַ�Ƿ���ȣ�
//	 ������ȣ���˵���ò�����Ĳ��������ı䣬�����������Ч
//3��GetMtrRdCtrl()���ȴ�g_MtrCacheCtr[]���ң�����õ��Ŀ��ƽṹ�Ѿ��������ڴ�����ֱ�ӷ��أ��������һ���¿ռ䣺
//	����CACHE_STATUS_FREE�Ŀռ䣬���û������Ҫ�����ϵ�CACHE_STATUS_IDLE�ṹ�������ļ�ϵͳ���ͷų�һ���ռ䣬
//	�ٴ��ļ�ϵͳ�е���ñ�Ľṹ���շ���Ŀռ��С�
//4��TMtrRdCtrl*ָ���g_MtrCacheCtrl[]��ȡ���ͷŻأ���Ҫ�޸�bStatus״̬������dwLastAcessTime

//����:ȡ��Ӧ���ַ�ĵ�������ƽṹ
//������@wPn Ҫȡ�Ĳ������
//		@pbTsa���ַ����Ҫ����У��������Ƿ����ı�
//����:����ɹ��򷵻ض�Ӧ���ַ�ĵ�������ƽṹ��ָ�룬���򷵻�NULL
TMtrRdCtrl* GetMtrRdCtrl(WORD wPn, BYTE*pbTsa)
{
	int i;
	int iLastInx = -1;
	BYTE bAddL = pbTsa[0]+1;
	DWORD dwLastAcessTime = 0xffffffff;

	//���ڴ��в���
	for (i=0; i<MTR_CACHE_NUM; i++)
	{
		if (g_MtrCacheCtrl[i].wPn == wPn)
		{
			if (memcmp(g_MtrCacheCtrl[i].mtrRdCtrl.bTsa, pbTsa, bAddL)==0 && g_MtrCacheCtrl[i].mtrRdCtrl.bTaskSN==GetTaskCfgSn()) //��ַδ�ı䣬������Ч
			{
				if (g_MtrCacheCtrl[i].bStatus == CACHE_STATUS_IDLE)
				{
					g_MtrCacheCtrl[i].bStatus = CACHE_STATUS_INUSE;
					g_MtrCacheCtrl[i].dwLastAcessTime = GetCurTime();
					return &g_MtrCacheCtrl[i].mtrRdCtrl;
				}
				else
				{
					return NULL;
				}
			}
			else
			{
				memset((BYTE *)&g_MtrCacheCtrl[i], 0, sizeof(TMtrCacheCtrl));
				DTRACE(DB_METER, ("GetMtrRdCtrl: run here1.\n"));
				g_MtrCacheCtrl[i].bStatus = CACHE_STATUS_INUSE;
				g_MtrCacheCtrl[i].wPn = wPn;
				g_MtrCacheCtrl[i].dwCacheTime = GetCurTime();
				g_MtrCacheCtrl[i].dwLastAcessTime = GetCurTime();
				g_MtrCacheCtrl[i].fDirty = false;
				g_MtrCacheCtrl[i].fTrigerSave = false;
				InitMtrRdCtrl(wPn, pbTsa, &g_MtrCacheCtrl[i].mtrRdCtrl);
				return &g_MtrCacheCtrl[i].mtrRdCtrl;
			}
		}
		if (g_MtrCacheCtrl[i].dwLastAcessTime<dwLastAcessTime && g_MtrCacheCtrl[i].bStatus==CACHE_STATUS_IDLE)
		{
			dwLastAcessTime = g_MtrCacheCtrl[i].dwLastAcessTime;
			iLastInx = i;
		}
	}

	//�ڴ���û�У���һ��δ�����
	for (i=0; i<MTR_CACHE_NUM; i++)
	{
		if (g_MtrCacheCtrl[i].bStatus == CACHE_STATUS_FREE)
		{
			if (LoadMtrRdCtrl(wPn, pbTsa, &g_MtrCacheCtrl[i].mtrRdCtrl) && g_MtrCacheCtrl[i].mtrRdCtrl.bTaskSN==GetTaskCfgSn())
			{
				g_MtrCacheCtrl[i].bStatus = CACHE_STATUS_INUSE;
				g_MtrCacheCtrl[i].wPn = wPn;
				g_MtrCacheCtrl[i].dwCacheTime = GetCurTime();
				g_MtrCacheCtrl[i].dwLastAcessTime = GetCurTime();
				g_MtrCacheCtrl[i].fDirty = false;
				g_MtrCacheCtrl[i].fTrigerSave = false;
				return &g_MtrCacheCtrl[i].mtrRdCtrl;
			}
			else
			{
				memset((BYTE *)&g_MtrCacheCtrl[i], 0, sizeof(TMtrCacheCtrl));
				g_MtrCacheCtrl[i].bStatus = CACHE_STATUS_INUSE;
				g_MtrCacheCtrl[i].wPn = wPn;
				g_MtrCacheCtrl[i].dwCacheTime = GetCurTime();
				g_MtrCacheCtrl[i].dwLastAcessTime = GetCurTime();
				g_MtrCacheCtrl[i].fDirty = false;
				g_MtrCacheCtrl[i].fTrigerSave = false;
				InitMtrRdCtrl(wPn, pbTsa, &g_MtrCacheCtrl[i].mtrRdCtrl);
				return &g_MtrCacheCtrl[i].mtrRdCtrl;
			}
		}
	}

	//û������Ҫ�����ϵ�CACHE_STATUS_IDLE�ṹ�������ļ�ϵͳ���ͷų�һ���ռ�
	if (iLastInx >= 0)
	{
		SaveMtrRdCtrl(g_MtrCacheCtrl[iLastInx].wPn, &g_MtrCacheCtrl[iLastInx].mtrRdCtrl);
		memset((BYTE *)&g_MtrCacheCtrl[iLastInx], 0, sizeof(TMtrCacheCtrl));
		if (LoadMtrRdCtrl(wPn, pbTsa, &g_MtrCacheCtrl[iLastInx].mtrRdCtrl) && g_MtrCacheCtrl[iLastInx].mtrRdCtrl.bTaskSN==GetTaskCfgSn())
		{
			g_MtrCacheCtrl[iLastInx].bStatus = CACHE_STATUS_INUSE;
			g_MtrCacheCtrl[iLastInx].wPn = wPn;
			g_MtrCacheCtrl[iLastInx].dwCacheTime = GetCurTime();
			g_MtrCacheCtrl[iLastInx].dwLastAcessTime = GetCurTime();
			g_MtrCacheCtrl[iLastInx].fDirty = false;
			g_MtrCacheCtrl[iLastInx].fTrigerSave = false;
			return &g_MtrCacheCtrl[iLastInx].mtrRdCtrl;
		}
		else
		{
			//memset((BYTE *)&g_MtrCacheCtrl[iLastInx], 0, sizeof(TMtrCacheCtrl));
			g_MtrCacheCtrl[iLastInx].bStatus = CACHE_STATUS_INUSE;
			g_MtrCacheCtrl[iLastInx].wPn = wPn;
			g_MtrCacheCtrl[iLastInx].dwCacheTime = GetCurTime();
			g_MtrCacheCtrl[iLastInx].dwLastAcessTime = GetCurTime();
			g_MtrCacheCtrl[iLastInx].fDirty = false;
			g_MtrCacheCtrl[iLastInx].fTrigerSave = false;
			InitMtrRdCtrl(wPn, pbTsa, &g_MtrCacheCtrl[iLastInx].mtrRdCtrl);
			return &g_MtrCacheCtrl[iLastInx].mtrRdCtrl;
		}
	}

	return NULL;
}

//����:�Ѷ�Ӧ���ַ�ĵ�������ƽṹ�Żص�g_MtrCacheCtrl[]
//������@wPn Ҫ�ŻصĲ������
//		@pbTsa���ַ
//		@pMtrRdCtrl ��������ƽṹ
//		@fModify�Ƿ��޸��ˣ�����޸��ˣ����������fDirty��־
//����:��
void PutMtrRdCtrl(WORD wPn, BYTE* pbTsa, bool fModify)
{
	int i;
	BYTE bAddL = pbTsa[0]+1;

	//���ڴ��в���
	for (i=0; i<MTR_CACHE_NUM; i++)
	{
		if (g_MtrCacheCtrl[i].wPn == wPn)
		{
			if (memcmp(g_MtrCacheCtrl[i].mtrRdCtrl.bTsa, pbTsa, bAddL) == 0)
			{
				g_MtrCacheCtrl[i].bStatus = CACHE_STATUS_IDLE;
				g_MtrCacheCtrl[i].dwLastAcessTime = GetCurTime();
				if (fModify)
				{
					g_MtrCacheCtrl[i].fDirty = true;
				}
			}
		}
	}
}

//���������������������ڲ�ʹ��
//����:���ļ�ϵͳװ�ض�Ӧ���ַ�ĵ�������ƽṹ
//������@wPn Ҫȡ�Ĳ������
//		@pbTsa���ַ����Ҫ����У��������Ƿ����ı�
//		@pMtrRdCtrl ���������ļ��ж�ȡ���ĵ�������ƽṹ
//����:����ɹ��򷵻�true�����򷵻�false
bool LoadMtrRdCtrl(WORD wPn, BYTE* pbTsa, TMtrRdCtrl* pMtrRdCtrl)
{
	char szName[64];
	bool fRet = false;
	WORD wLen = sizeof(TMtrRdCtrl);
	int iRet;

	sprintf(szName, USER_DATA_PATH"MtrRdCtrl_Pn%d.dat", wPn);
	if (ReadFile(szName, (BYTE *)pMtrRdCtrl, wLen))
	{
		if (pMtrRdCtrl->bChkSum == CheckSum((BYTE *)&pMtrRdCtrl->bTsa[0], wLen-1))
		{
			if (memcmp(pbTsa, pMtrRdCtrl->bTsa, pbTsa[0]) == 0)
				fRet = true;
			else
				DTRACE(DB_METER, ("LoadMtrRdCtrl : wPn=%d pbTsa chg !\n", wPn));
		}
		else
		{
			DTRACE(DB_METER, ("LoadMtrRdCtrl : wPn=%d chk	error !\n", wPn));
		}
	}
	else
	{
		DTRACE(DB_METER, ("LoadMtrRdCtrl : wPn=%d read	len error !\n", wPn));
	}

	return fRet;
}

//����:�Ӱѵ�������ƽṹ���浽�ļ�ϵͳ��
//������@wPn Ҫд�Ĳ������
//		@pMtrRdCtrl ���������ļ��ж�ȡ���ĵ�������ƽṹ
//����:����ɹ��򷵻�true�����򷵻�false
bool SaveMtrRdCtrl(WORD wPn, TMtrRdCtrl* pMtrRdCtrl)
{
	char szName[64];
	bool fRet = true;
	WORD wLen = sizeof(TMtrRdCtrl);

	sprintf(szName, USER_DATA_PATH"MtrRdCtrl_Pn%d.dat", wPn);

	pMtrRdCtrl->bChkSum = CheckSum((BYTE *)&pMtrRdCtrl->bTsa[0], wLen-1);
	if (!WriteFile(szName, (BYTE *)pMtrRdCtrl, wLen))
	{
		DTRACE(DB_METER, ("SaveMtrRdCtrl : write file error !\n"));
		fRet = false;
	}

	return fRet;
}

//������ɾ�������ƽṹ�ļ�
void DeleteMtrRdCtrl()
{
	char szName[64];

	DTRACE(DB_TASK,("DeleteMtrRdCtrl....\n"));
	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		sprintf(szName, USER_DATA_PATH"MtrRdCtrl_Pn%d.dat", wPn);	
		DeleteFile(szName);
	}

	memset(dwTaskLastUpdataTime, 0, sizeof(dwTaskLastUpdataTime));
}


///////////////////////////////////////////////////////////////////////////////////////
//�����ʱ������ʽӿڶ��壺
//����:����һ������������ʱ���ݽṹ
//������@pMtrTmpData�����ʱ���ݽṹ
//	@dwOAD���ݱ�ʶ
//	@pbData��������
//	@bLen���ݳ���
//����:����пռ䱣���򷵻�true�����򷵻�false
bool SaveMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD dwOAD, BYTE* pbData, BYTE bLen)
{
	for (int i=0; i<MTR_TMP_ITEM_NUM; i++)
	{
		if (dwOAD==pMtrTmpData->item[i].dwOAD && bLen==pMtrTmpData->item[i].bLen)
		{
			memcpy(pMtrTmpData->bBuf+pMtrTmpData->item[i].wOffset, pbData, bLen);
			pMtrTmpData->item[i].bValid = 1;

			return true;
		}
	}

	for (int i=0; i<MTR_TMP_ITEM_NUM; i++)
	{
		//δ�ҵ����Ѿ���δ������
		if (pMtrTmpData->item[i].dwOAD==0 && pMtrTmpData->item[i].bLen==0)
		{
			pMtrTmpData->item[i].dwOAD = dwOAD;		//zqq modify 161111
			if (i > 0)
			{
				pMtrTmpData->item[i].wOffset = pMtrTmpData->item[i-1].wOffset + pMtrTmpData->item[i-1].bLen;
				memcpy(pMtrTmpData->bBuf+pMtrTmpData->item[i].wOffset, pbData, bLen);
				pMtrTmpData->item[i].bLen = bLen;
				pMtrTmpData->item[i].bValid = 1;
			}
			else
			{
				pMtrTmpData->item[i].wOffset = 0;
				memcpy(pMtrTmpData->bBuf, pbData, bLen);
				pMtrTmpData->item[i].bLen = bLen;
				pMtrTmpData->item[i].bValid = 1;
			}

			return true;
		}
	}
	return false;
}

//����:�ӵ����ʱ���ݽṹ����һ��������
//������@pMtrTmpData�����ʱ���ݽṹ
//	@dwOAD���ݱ�ʶ
//	@pbData��������
//	@bLen���ݳ���
//����:����ҵ����������򷵻����ݳ��ȣ����򷵻�-1
int GetMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD dwOAD, BYTE* pbData)
{
	for (int i=0; i<MTR_TMP_ITEM_NUM; i++)
	{
		if (pMtrTmpData->item[i].bValid==1 && dwOAD==pMtrTmpData->item[i].dwOAD)
		{
			memcpy(pbData, pMtrTmpData->bBuf+pMtrTmpData->item[i].wOffset, pMtrTmpData->item[i].bLen);

			return pMtrTmpData->item[i].bLen;
		}

		//δ�ҵ����Ѿ���δ������
		if (pMtrTmpData->item[i].bValid==0 && pMtrTmpData->item[i].dwOAD==0 && pMtrTmpData->item[i].bLen==0)
			break;
	}

	return -1;
}

static DWORD g_dwFixRDOad[] = {0x00100200,//�����й�����
								0x00200200,//�����й�����
								0x00300200,//����޹�1
								0x00400200,//����޹�2
								0x20000200,//��ѹ
								0x20010200,//����
								0x20040200,//�й�����
								0x20050200,//�޹�����
};

static WORD g_wFixRDInID[] = {0xa010,//�����й�����
								0xa020,//�����й�����
								0xa030,//����޹�1
								0xa040,//����޹�2
								0xa050,//��ѹ
								0xa051,//����
								0xa052,//�й�����
								0xa053,//�޹�����
};

static WORD g_wFixDataLen[] = {27,//�����й�����
								27,//�����й�����
								27,//����޹�1
								27,//����޹�2
								11,//��ѹ
								22,//����
								22,//�й�����
								22,//�޹�����
};

//����:ȡ�̶������б����������
//����:������OAD�б�
DWORD* MtrGetFixedItems(WORD* pwItemNum)
{
	*pwItemNum = sizeof(g_dwFixRDOad)/sizeof(DWORD);
	return g_dwFixRDOad;
}

//����:ȡ�̶������������
//����:������OAD�б����ݳ���
WORD* MtrGetFixedLen()
{
	return g_wFixDataLen;
}

//����:ȡ�̶������������ڲ�ӳ��ID
//����:�������ڲ�ӳ��ID��
WORD* MtrGetFixedInItems()
{
	return g_wFixRDInID;
}

///////////////////////////////////////////////////////////////////////////////////////
//����:ˢ���ڲ�����������
//����:
//	@wPn�������
//	@dwOAD���ݱ�ʶ
//	@pbData��������
//����:����б����򷵻�true�����򷵻�false
bool SaveMtrInItemMem(WORD wPn, DWORD dwOAD, BYTE* pbData)
{
	for (int i=0; i<sizeof(g_dwFixRDOad)/sizeof(DWORD); i++)
	{
		if (g_dwFixRDOad[i] == dwOAD)
		{
			WriteItemEx(BN0, wPn, g_wFixRDInID[i], pbData, GetCurTime());
			return true;
		}	
	}

	return false;
}

//����:��һ������л������³�ʼ�������ʱ���ݽṹ
//	��pMtrTmpData�������ݽṹ����,Ԥ��ΪpdwFixOAD����ռ䣬����bValid==0
//������@pMtrTmpData�����ʱ���ݽṹ
//	@pdwFixOAD �̶�����OAD����
//	@pwDataLen�̶�����OAD�����ݳ��ȣ�Ԥ�ȳ�ʼ���ã�
//					��Ҫÿ�ص��õ�ʱ�����³�ʼ��
//	@wNum�̶�����OAD�����Ԫ�ظ���
void InitMtrTmpData(TMtrTmpData* pMtrTmpData, DWORD* pdwFixOAD, WORD* pwDataLen, WORD wNum)
{
	memset(pMtrTmpData, 0, sizeof(TMtrTmpData));
	for (WORD i=0; i<wNum; i++)
	{
		if (pMtrTmpData->item[i].dwOAD==0 && pMtrTmpData->item[i].bLen==0)
		{
			if (i > 0)
			{
				pMtrTmpData->item[i].wOffset = pMtrTmpData->item[i-1].wOffset + pMtrTmpData->item[i-1].bLen;
				pMtrTmpData->item[i].dwOAD = pdwFixOAD[i];
				pMtrTmpData->item[i].bLen = pwDataLen[i];
			}
			else
			{
				pMtrTmpData->item[i].wOffset = 0;
				pMtrTmpData->item[i].dwOAD = pdwFixOAD[i];
				pMtrTmpData->item[i].bLen = pwDataLen[i];
			}
		}
	}
}

//����:��ѯ�����ʱ���ݽṹ�У��Ƿ񻺴�������Ҫ��OAD���飬��bValid==1
//������@pMtrTmpData�����ʱ���ݽṹ
//	@pdwOAD ��Ҫ��ѯ��OAD����
//	@wNum OAD�����Ԫ�ظ���
//����:���ȫ�������򷵻�true,���򷵻�false
bool QueryMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD* pdwOAD, WORD wNum)
{
	WORD i, j;

	for (i=0; i<wNum; i++)
	{
		for (j=0; j<MTR_TMP_ITEM_NUM; j++)
		{
			if (pdwOAD[i] == pMtrTmpData->item[j].dwOAD)
			{
				if (pMtrTmpData->item[j].bValid == 0)
					return false;
				else
					break;
			}
		}
		if (j == MTR_TMP_ITEM_NUM)
			return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//TMtrRdCtrl����ʱ��¼�Ĳ����ӿ�
//������ʱ��¼�ռ�
bool AllocTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, TTaskCfg *pTaskCfg, BYTE bCSDNum, WORD wRecLen)
{
	int i;
	TTaskCfg tTaskCfg;
	DWORD dwStartTime, dwEndTime;

	for (i=0; i<MTR_TASK_NUM; i++)
	{
		if (pMtrRdCtrl->taskSucFlg[i].bValid==1 && pTaskCfg->bTaskId==pMtrRdCtrl->taskSucFlg[i].bTaskId)
		{
			if (wRecLen == GetMemLen(pMtrRdCtrl->allocTab, MTR_TAB_NUM, bType, pTaskCfg->bTaskId))
			{
				return true;
			}
			else
			{
				if (FreeTmpRec(pMtrRdCtrl, bType, pTaskCfg->bTaskId))
					break;
			}
		}
	}

	for (i=0; i<MTR_TASK_NUM; i++)
	{
		//δ�ҵ����Ѿ���δ������
		if (pMtrRdCtrl->taskSucFlg[i].bValid == 0)
		{
			if (bType == MEM_TYPE_TASK)
			{
				WORD wNeedBlk = (wRecLen+MTR_TMP_ITEM_NUM-1) / MTR_TMP_ITEM_NUM;
				if (AllocMem(pMtrRdCtrl->bGlobal, pMtrRdCtrl->allocTab, MTR_TAB_NUM, bType, pTaskCfg->bTaskId, wRecLen))
				{
					pMtrRdCtrl->taskSucFlg[i].bValid = 1;
					pMtrRdCtrl->taskSucFlg[i].bTaskId = pTaskCfg->bTaskId;
					pMtrRdCtrl->taskSucFlg[i].bCSDItemNum = bCSDNum;
					memset(pMtrRdCtrl->taskSucFlg[i].bSucFlg, 0, TASK_SUC_FLG_LEN);

					return true;
				}
				else
					return false;
			}
			else
			{
				pMtrRdCtrl->taskSucFlg[i].bValid = 1;
				pMtrRdCtrl->taskSucFlg[i].bTaskId = pTaskCfg->bTaskId;
				pMtrRdCtrl->taskSucFlg[i].bCSDItemNum = bCSDNum;
				memset(pMtrRdCtrl->taskSucFlg[i].bSucFlg, 0, TASK_SUC_FLG_LEN);

				return true;

			}
		}
	}
	return false;
}

//�ͷ���ʱ��¼�ռ�
bool FreeTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId)
{
	for (int i=0; i<MTR_TASK_NUM; i++)
	{
		if (pMtrRdCtrl->taskSucFlg[i].bValid==1 && bId==pMtrRdCtrl->taskSucFlg[i].bTaskId)
		{
			memset(&pMtrRdCtrl->taskSucFlg[i], 0, sizeof(pMtrRdCtrl->taskSucFlg[i]));
			FreeMem(pMtrRdCtrl->bGlobal, pMtrRdCtrl->allocTab, MTR_TAB_NUM, bType, bId);
			FreeMem(pMtrRdCtrl->bGlobal, pMtrRdCtrl->allocTab, MTR_TAB_NUM, MEM_TYPE_CURVE_FLG, bId);
			return true;
		}
	}

	return false;
}

//��ȡ��ʱ��¼
int ReadTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec)
{
	WORD wIndex;

	for (int i=0; i<MTR_TASK_NUM; i++)
	{
		if (pMtrRdCtrl->taskSucFlg[i].bValid==1 && bId==pMtrRdCtrl->taskSucFlg[i].bTaskId)
		{
			return ReadMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, bType, bId, pbRec);
		}
	}

	return -1;
}

//д��ʱ��¼
int WriteTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec)
{
	for (int i=0; i<MTR_TASK_NUM; i++)
	{
		if (pMtrRdCtrl->taskSucFlg[i].bValid==1 && bId==pMtrRdCtrl->taskSucFlg[i].bTaskId)
		{
			return WriteMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, bType, bId, pbRec);
		}
	}

	return -1;
}

//����ʱ��¼��дĳ��CSD����
int WriteTmpRecItem(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec, BYTE bTaskSucFlgIndex, WORD wOffset, WORD wLen)
{
	BYTE bTmpBuf[1024];

	for (int i=0; i<MTR_TASK_NUM; i++)
	{
		if (pMtrRdCtrl->taskSucFlg[i].bValid==1 && bId==pMtrRdCtrl->taskSucFlg[i].bTaskId)
		{
			if (ReadTmpRec(pMtrRdCtrl, bType, bId, bTmpBuf) > 0)
			{
				memcpy(&bTmpBuf[wOffset], pbRec, wLen);
				WriteTmpRec(pMtrRdCtrl, bType, bId, bTmpBuf);
				pMtrRdCtrl->taskSucFlg[i].bSucFlg[bTaskSucFlgIndex/8] |= (1<<bTaskSucFlgIndex%8);
				return wLen;
			}
			break;
		}
	}

	return -1;
}

//������ʱ��¼
int ClrTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId)
{
	BYTE bBuf[1024];
	memset(bBuf, 0, sizeof(bBuf));

	for (int i=0; i<MTR_TASK_NUM; i++)
	{
		if (pMtrRdCtrl->taskSucFlg[i].bValid==1 && bId==pMtrRdCtrl->taskSucFlg[i].bTaskId)
		{
			if (GetMemLen(pMtrRdCtrl->allocTab, MTR_TAB_NUM, bType, bId) <= sizeof(bBuf))
				return WriteMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, bType, bId, bBuf);
			else
				break;
		}
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//����������ɼ����ĵ������
//������@pMtrRdCtrl���������
//	    @bRespType���֡�ķ�������1:GetResponseNormal; 3:GetResponseRecord��
//                   �����LIST��ʽ�������ʱ��ת���ɵ����ٱ���
//      @pbCSD�������������ʶ����Ҫ���ݷ���֡�����ͣ���OADת��ΪCSD
//      @pbData���ص�����
bool SaveMtrData(TMtrRdCtrl* pMtrRdCtrl, BYTE bRespType, BYTE* pbCSD, BYTE* pbData, WORD wDataLen)
{
	//�������񼰲ɼ����������ã����֡�ķ�������bRespType�Ͳɼ������Ĳɼ���ʽ�Ĳɼ�������ƥ��
	TTaskCfg tTaskCfg; 
	BYTE *pbSch;
	int iSchCfgLen;
	BYTE bType, bCSDType;
	WORD wLen, wCSDLen, wNum, wFmtLen;
	BYTE bCSDIndex, *pbTaskCSD, bFmtType;
	BYTE *pFmt;
	DWORD dwOAD, wRecOffset;
	bool fIsSaveFlg = false;
	bool fSave = false;
	int iTaskNum = GetTaskNum();

	if (bRespType == 1) //OAD���ʹ�һ�µ��������ʱ����
	{
		dwOAD = OoOadToDWord(pbCSD+1);
		wCSDLen = OoGetDataLen(DT_CSD, pbCSD);
		SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, pbData, wCSDLen);
		//SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp);		//zqq add ----20170412 hyl ���ε��������¼������ݰ����ڲ��������������洢��ʹ�洢���ն�ʱ������������ʱ�̲�һ�¡�������������¼���������������������
	}

	for (WORD wIndex=0; wIndex<TASK_NUM; wIndex++)	//�����������ñ�
	{
		memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
		if (GetTaskCfg(wIndex, &tTaskCfg))
		{
			pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
			if (pbSch!=NULL)
			{
				for (BYTE bTaskId=0; bTaskId<MTR_TASK_NUM; bTaskId++) ///�������ṹ�е�32������ID
				{
					if (tTaskCfg.bTaskId==pMtrRdCtrl->taskSucFlg[bTaskId].bTaskId)
					{
						if (pMtrRdCtrl->taskSucFlg[bTaskId].fRecSaved)	//�ն�����Сʱ���������ͬ��OAD��Сʱ����ʱ��ֹ�ն���洢
							continue;

						switch (tTaskCfg.bSchType)
						{
						case SCH_TYPE_COMM:
							pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
							if ((pbTaskCSD=OoGetField(pbSch, pFmt, wFmtLen, 3, &wLen, &bType)) != NULL)
							{
								if (*pbTaskCSD++ == DT_ARRAY)
								{
									fIsSaveFlg = false;
									wNum = *pbTaskCSD++;
									for (bCSDIndex=0; bCSDIndex<wNum; bCSDIndex++)
									{
										bFmtType = *pbTaskCSD++;
										if (FieldCmp(DT_CSD, pbCSD, bFmtType, pbTaskCSD) == 0)
										{
											fIsSaveFlg = true;
											break;
										}
										pbTaskCSD += ScanCSD(pbTaskCSD, false);
									}

									//���ⷴ������ƫ�ƣ�ֻ�����ҵ�������²ż���
									if (fIsSaveFlg)
									{
										wRecOffset = 0;
										if ((pbTaskCSD=OoGetField(pbSch, pFmt, wFmtLen, 3, &wLen, &bType)) != NULL)
										{
											if (*pbTaskCSD++ == DT_ARRAY)
											{
												wNum = *pbTaskCSD++;
												for (BYTE bIndex=0; bIndex<bCSDIndex+1; bIndex++)
												{
													bFmtType = *pbTaskCSD++;
													if (bIndex == bCSDIndex)
													{
														wCSDLen = OoGetDataLen(bFmtType, pbTaskCSD);
														break;
													}
													else
													{
														wRecOffset += OoGetDataLen(bFmtType, pbTaskCSD);
													}
													pbTaskCSD += ScanCSD(pbTaskCSD, false);
												}
											}
										}
										WriteTmpRecItem(pMtrRdCtrl, MEM_TYPE_TASK, tTaskCfg.bTaskId, pbData, bCSDIndex, wRecOffset, wCSDLen);
										fSave = true;
									}
								}
							}
							break;

						case SCH_TYPE_EVENT:
							pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
							if ((pbTaskCSD=OoGetField(pbSch, pFmt, wFmtLen, 1, &wLen, &bType)) != NULL)
							{
								pbTaskCSD += 4;
								if (*pbTaskCSD++ == DT_ARRAY)
								{
									fIsSaveFlg = false;
									wNum = *pbTaskCSD++;
									for (bCSDIndex=0; bCSDIndex<wNum; bCSDIndex++)
									{
										bFmtType = *pbTaskCSD++;
										if (FieldCmp(DT_ROAD, pbCSD, bFmtType, pbTaskCSD) == 0)
										{
											fIsSaveFlg = true;
											break;
										}
										pbTaskCSD += ScanROAD(pbTaskCSD, false);
									}

									if (fIsSaveFlg)
									{
										pMtrRdCtrl->taskSucFlg[bTaskId].bSucFlg[bCSDIndex/8] |= (1<<bCSDIndex%8);
										wRecOffset = ScanROAD(pbTaskCSD, false);
										DWORD dwLastRecIndex = GetEvtTaskRecLastSerialNumber(pMtrRdCtrl->bTsa, pMtrRdCtrl->bTsa[0], pbTaskCSD, wRecOffset);
										DWORD dwCurRecIndex = OoDoubleLongUnsignedToDWord(pbData+3);	//�¼���¼��� ���ֽ���ǰ����
										if ((pbData[0]==0x00) && (pbData[1]==0x21))	//�ز�����ʧ�� ��ʱ��0x00 0x21
											break;
										if (dwCurRecIndex==0 || dwLastRecIndex<dwCurRecIndex)
										{
											SaveTaskDataToDB(pMtrRdCtrl, MEM_TYPE_NONE, pMtrRdCtrl->taskSucFlg[bTaskId], pbData, wDataLen, bCSDIndex);
											fSave = true;

											char szTableName[32];
											memset(szTableName, 0, sizeof(szTableName));
											GetEvtTaskTableName(tTaskCfg.bSchNo, bCSDIndex, szTableName);
											int iRecIdx = GetRecPhyIdx(szTableName, 1);
											if (iRecIdx >= 0)
											{
												BYTE bTmpBuf[60];
												DWORD dwChnOAD;
												memset(bTmpBuf, 0, sizeof(bTmpBuf));
												int iLen = OoReadAttr(0x4300, ATTR10, bTmpBuf, NULL, NULL);	//��ȡ���ò���
												if (iLen<=0 || bTmpBuf[0]!=DT_ARRAY)
													continue;

												BYTE bChnNum = (bTmpBuf[1]>CN_RPT_NUM) ? CN_RPT_NUM : bTmpBuf[1];
												for (BYTE i=0; i<bChnNum; i++)
												{
													dwChnOAD = OoOadToDWord(&bTmpBuf[5*i+3]);	//ͨ��OAD
													dwOAD = OoOadToDWord(pbTaskCSD);
													SendEvtMsg(dwChnOAD, dwOAD, iRecIdx, EVT_STAGE_TASK, tTaskCfg.bSchNo, bCSDIndex, pbTaskCSD, wRecOffset);
												}
											}
										}
									}
								}
							}
							break;

						//case SCH_TYPE_TRANS: (͸�������������ﴦ��)
						//	break;

						case SCH_TYPE_REPORT:
							break;

						case SCH_TYPE_SCRIPT:
							break;

						case SCH_TYPE_REAL:
							break;
						}
					}
				}
			}
		}
	}

	return fSave;
}


//DWORD dwItemRdTime[ITEM_RD_TIME_NUM];	//���������С����С�ʱ�� ��SaveMtrDataHook()�и��±���Ա
//��SaveMtrDataHook()�и��±���Ա
//hyl 20170412 ��̨�ӣ�3105�¼�ֻ�ܰ������ж����ݣ����bType���⴦��
void SaveMtrDataHook(DWORD dwOAD, TMtrExcTmp* pMtrExcTmp, BYTE bType)
{
	DWORD dwCurSec = GetCurTime();
	dwOAD &= OAD_FEAT_MASK;	//ȥ����������

	if (dwOAD==0x00100201 || dwOAD==0x00100200)
		pMtrExcTmp->dwItemRdTime[0] = dwCurSec;
	else if (dwOAD==0x00200201 || dwOAD==0x00200200)
		pMtrExcTmp->dwItemRdTime[1] = dwCurSec;
	else if (dwOAD==0x40000200)
	{	
		if(bType) pMtrExcTmp->dwItemRdTime[2] = dwCurSec;
	}
}