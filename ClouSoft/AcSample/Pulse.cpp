/*********************************************************************************************************
 * Copyright (c) 2005,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Pulse.cpp
 * ժ    Ҫ��
 * ��ǰ�汾��1.0
 * ��    �ߣ��� ǿ
 * ������ڣ�2008��7��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
*********************************************************************************************************/
#include "Pulse.h"
#include "Energy.h"
#include "Demand2.h"		
#include "AcFmt.h"
#include "ComAPI.h"
#include "math.h"
#include "Sample.h"
#include "FaAPI.h"
#include "AcHook.h"
#include "LibAcConst.h"
#include "DbOIAPI.h"
#include "OoFmt.h"

//BYTE	g_bPulseFlag = 0xff;    	//�㽭�����Ϊ0x00
//static TTime g_tmPwrCalcu;
TPulseInData  g_PulseInData[MAX_YMNUM];

CPulseManager g_PulseManager;	//���������

//����:�ڽ���ִ���¶���ʱ�Ļص�����,ֻ�ṩ���ɲ�����,Ĭ�ϲ������ɱ���������
//	   �費��ҪҲͬʱִ��
//����:@wPn ���ɵĲ�����,���֧��˫������,��wPnָ��Ӧ���Ƿ�Ĭ�ϲ�����,
//			�����ⲿû�����ý��ɲ������ʱ��,wPnҲ���ܱ���ΪĬ�ϲ�����(��PN0)
//			��֧��˫������������,��������жϵ�wPnΪ��Ĭ�ϲ�����,��ִ��
//			wPn����Ӧ������ͬʱ,ҲӦ��ΪĬ�ϲ�����ִ����Ӧ����
//	   @time �ն����ʱ��,ֻʹ���ꡢ�¡��ա�ʱ�����ֶ�	
/*void PulseOnMonthFrz(WORD wIndex, const TTime& time, BYTE bFrzIdx)
{
	if ((wIndex >= 4) || (bFrzIdx >= AUTO_DATE_NUM))
		return;

	DWORD dwTmp = MonthFrom2000(time);
	WriteItemEx(BN18, wIndex, 0x0614+bFrzIdx, (BYTE *)&dwTmp); 	//���һ���¶���ʱ��
	TrigerSaveBank(BN18, 0, -1); 
}*/

//����:���ɿ������ж�ĳ���¶����Ƿ��Ѿ�����Ļص�����,ֻ�ṩ���ɲ�����,Ĭ�ϲ������ɱ���������
//	   �費��ҪҲͬʱִ��
//����:@wPn ���ɵĲ�����,���֧��˫������,��wPnָ��Ӧ���Ƿ�Ĭ�ϲ�����,
//			�����ⲿû�����ý��ɲ������ʱ��,wPnҲ���ܱ���ΪĬ�ϲ�����(��PN0)
//			��֧��˫������������,��������жϵ�wPnΪ��Ĭ�ϲ�����,��ִ��
//			wPn����Ӧ������ͬʱ,ҲӦ��ΪĬ�ϲ�����ִ����Ӧ����
//	   @time �ն����ʱ��,ֻʹ���ꡢ�¡��ա�ʱ�����ֶ�	
/*bool PulseIsMonthFrozen(WORD wIndex, const TTime& time, BYTE bFrzIdx)
{
	if ((wIndex >= 4) || (bFrzIdx >= AUTO_DATE_NUM))
		return false;
	
	DWORD dwTmp = 0;
	ReadItemEx(BN18, wIndex, 0x0614+bFrzIdx, (BYTE *)&dwTmp);	//���һ���¶���ʱ��
	return (dwTmp == MonthFrom2000(time));
}*/


CPulse::CPulse()
{
	m_fValid = false;
	m_fStopSaveLog = false;
	//m_bPortIdx = 0;
	memset(&m_PulseCfg, 0, sizeof(TPulseCfg));
}

CPulse::~CPulse()
{
	
}


bool CPulse::Init(TPulseCfg* pPulseCfg)
{    
    m_wPn = pPulseCfg->wPn;
	if (!PulseLoadPara(pPulseCfg, &m_PulsePara))
		return false;

	ReadItemEx(BN1, PN0, 0x2040+pPulseCfg->bPortNo, (BYTE* )&m_PulseCfg);	//��ȡ��·����ɵĲ�������

	if (memcmp(&m_PulseCfg, pPulseCfg, sizeof(TPulseCfg)) != 0)	//��1���ϵ��ʼ���Ҹ�·�������øı�
	{
	    DTRACE(DB_FA, ("CPulse::Init Cfg Change: Old Cfg wPn=%d,bPortNo=%d,bType=%d,iConst=%u.\r\n", m_PulseCfg.wPn, m_PulseCfg.bPortNo, m_PulseCfg.bType, m_PulseCfg.i64Const));
	    DTRACE(DB_FA, ("CPulse::Init Cfg Change: New Cfg wPn=%d,bPortNo=%d,bType=%d,iConst=%u.\r\nClearPulseLog.\r\n", pPulseCfg->wPn, pPulseCfg->bPortNo, pPulseCfg->bType, pPulseCfg->i64Const));	    

	    m_PulseCfg = *pPulseCfg;
	    WriteItemEx(BN1, PN0, 0x2040+pPulseCfg->bPortNo, (BYTE* )pPulseCfg);
	    TrigerSaveBank(BN1, 0, -1);

    	ClearLogBlock(pPulseCfg->bPortNo);//�����·���ڿ����־����
	}


	m_wRate = GetRate(m_wPn);
	m_PulsePara.EnergyPara.wRate = m_wRate;
	//m_PulsePara.DemandPara.wRate = m_wRate;
	
	//memset(m_dwDayEnergy, 0, sizeof(m_dwDayEnergy));
	//memset(m_dwMonthEnergy, 0, sizeof(m_dwMonthEnergy));

	m_Energy.Init(&m_PulsePara.EnergyPara);   //����ʵ���
	//m_Demand.Init(&m_PulsePara.DemandPara);   //����

#ifdef ACLOG_ENABLE	
	m_Energy.ResetLog();	//���³�ʼ����־
	//m_Demand.ResetLog();	//���³�ʼ����־
#endif
	
	memset(m_dwPulse, 0, sizeof(m_dwPulse));
	memset(m_dwLastPulse, 0 , sizeof(m_dwLastPulse));  //�ϴμ���ʱ�ĵ���������
	m_fTrigerSave = false;//�ڷ��ʡ��·ݻ򳭱��շ����л���ʱ�򣬴������ݿ�ȥ���汾�����������
	//m_fClrDemand = false; //��������־
	m_bEnergyMinute = 0;	
	m_fValid = true;
	m_wPowerPtr = 0;
	m_fStopSaveLog = false;	//����������־������ע����ñ�־���������粻��������	

	for (WORD i = 0; i < AUTO_DATE_NUM; i++)
		m_dwLastDateMin[i] = GetCurMinute();	//�ϴγ�����ִ�еķ���

	memset(m_fDateAdjBackward, 0, sizeof(m_fDateAdjBackward));//������ִ��ʱ����ǰ����	

	return true;
}


bool CPulse::LoadPara(void)		//����ˢ��
{
	PulseLoadPara(&m_PulseCfg, &m_PulsePara);
	m_Energy.ReInit();   //����ʵ���
	//m_Demand.ReInit();   //����
	m_wRate = GetRate(m_wPn);
	m_PulsePara.EnergyPara.wRate = m_wRate;
	//m_PulsePara.DemandPara.wRate = m_wRate;
	
	return true;
}


//����:�������幦��,ÿ���ӵ��ñ�����һ��
void CPulse::CalPower(void)
{
    WORD i;
	BYTE bBuf[8];

    //if (!m_fValid)
    //	continue;

    BYTE bPortIndex = m_PulseCfg.bPortNo - 1;
	WORD wWritePtr = g_PulseInData[bPortIndex].WritePtr;
	if (wWritePtr != m_wPowerPtr)
	{
		DWORD dwPulseNum;
		if (wWritePtr >= m_wPowerPtr)
			dwPulseNum = wWritePtr - m_wPowerPtr;
		else
			dwPulseNum = wWritePtr + TICK_BUF_LENGTH - m_wPowerPtr;
	
		dwPulseNum--;
		if (dwPulseNum == 0)	//����3�����壬����ֻ�ܲ��2������
		{
			m_i64Power = 0;
			return;
		}

		WORD wHeadPtr;
		if (wWritePtr == 0)
			wHeadPtr = TICK_BUF_LENGTH - 1;
		else
			wHeadPtr = wWritePtr - 1;  
			
		int64 tmp = m_PulseCfg.i64Const * (g_PulseInData[bPortIndex].TickBuf[wHeadPtr] - g_PulseInData[bPortIndex].TickBuf[m_wPowerPtr]);
		m_i64Power = (int64 ) 3600 * dwPulseNum * 1000000 / tmp * 10;		//FMT9Ϊ4λС��λ���ʷŴ�10��
		if (m_PulseCfg.bType > EQ_POS)	//������
			m_i64Power = -m_i64Power;

		m_wPowerPtr = wWritePtr;	//����
	}
	else
	{
		//���·����Ϊ0
		m_i64Power = 0;
	}
}


void CPulse::Run(bool fCalcuPwr)
{
	BYTE bBuf[4];
	if (!m_fValid)
	{
		//���·����Ϊ0
		m_i64Power = 0;
		return;
	}
		
	if (fCalcuPwr)
	{							
		CalPower();
	}
	
	RunMeter();
}


//����:������ĵ��ܺ�����,ÿ�����һ��
void CPulse::RunMeter()
{
    BYTE bPortIndex, bType;
	WORD i, wTypeNum, wInnerID;
	int iDiffPulse[ENERGY_NUM_MAX];  //ǰ�����ε����������Ĳ�
	DWORD dwDemandPulse[AC_DEMAND_NUM];
	//DWORD dwDemandTick[ENERGY_NUM_MAX];
	
	memset(iDiffPulse, 0, sizeof(iDiffPulse));
	memset(dwDemandPulse, 0, sizeof(dwDemandPulse));
	
	bType = m_PulseCfg.bType;
	bPortIndex = m_PulseCfg.bPortNo - 1;
	wTypeNum = m_PulsePara.EnergyPara.wTypeNum;
	wInnerID = m_PulsePara.EnergyPara.wInnerID[0]; //�ڲ�����ĵ���ID

	m_dwPulse[wInnerID] = g_PulseInData[bPortIndex].Pulse;
	iDiffPulse[0] = m_dwPulse[wInnerID] - m_dwLastPulse[wInnerID];

	m_Energy.AddPulse(iDiffPulse);   //����ʵ���
	
	/*if (m_fClrDemand)	//��������־,����߳�����,�����߳�ȥִ��
	{
		m_Energy.TransferMonth();
		//m_Demand.TransferMonth();
		m_fClrDemand = false;
		m_fTrigerSave = true; //�������ݿ�ȥ���汾�����������
	}*/

	TTime now;
	GetCurTime(&now);
	DWORD dwTick = GetTick();
	
	if (now.nMinute != m_bEnergyMinute)   //ÿ����ִ��һ��
	{  //Ҫ�ڷ��ӵĿ�ͷִ��

		DWORD dwCurMin = TimeToMinutes(now);	//�ϴγ�����ִ�еķ���
		DWORD dwDate = (DWORD )now.nMonth*0x10000 + (DWORD )now.nDay*0x100 + now.nHour;

		for (i=0; i<AUTO_DATE_NUM; i++)
		{
			if (dwCurMin < m_dwLastDateMin[i])
				m_fDateAdjBackward[i] = true;			//������ִ��ʱ����ǰ����
			m_dwLastDateMin[i] = dwCurMin;

			/*if ((dwDate&0xffff)==m_PulsePara.wAutoDate[i]) 	//��ǰʱ�䴦�ڳ�����
			{	
				if ((dwDate!=m_dwLastAutoDate[i] && !PulseIsMonthFrozen(bPortIndex, now, i)) || m_fDateAdjBackward[i])
				{		//�����������ϴ�����ʱ��ͬ && ����û����� || ������ִ��ʱ����ǰ����
					DWORD dwTmp = 0;
					ReadItemEx(BN18, bPortIndex, 0x0614, (BYTE *)&dwTmp);	//���һ���¶���ʱ��
					DTRACE(DB_FA, ("CPulse::RunMeter: transfer month, bPortIndex=%d, dwTmp=%ld\r\n", 
								   bPortIndex, dwTmp));
					
					PulseOnMonthFrz(bPortIndex, now, i);
					m_Energy.TransferMonth();
					//m_Demand.TransferMonth();
					
					m_fDateAdjBackward[i] = false;
					m_dwLastAutoDate[i] = dwDate; //�ڸ�λ������պ�Сʱû��,��������ת��һ��
					m_fTrigerSave = true; //�ڷ��ʡ��·ݻ򳭱��շ����л���ʱ�򣬴������ݿ�ȥ���汾�����������
				}
			}*/
		}

		m_wRate = GetRate(m_wPn);
		m_PulsePara.EnergyPara.wRate = m_wRate;
		//m_PulsePara.DemandPara.wRate = m_wRate;
		m_bEnergyMinute = now.nMinute;
	}

	/*WORD wWritePtr = g_PulseInData[bPortIndex].WritePtr;
	WORD wHeadPtr;
	if (wWritePtr == 0)
		wHeadPtr = TICK_BUF_LENGTH - 1;
	else
		wHeadPtr = wWritePtr - 1;

	DWORD dwDemandTick = 0;
	wTypeNum = m_PulsePara.DemandPara.wTypeNum;
	DWORD dwDemandTick0 = g_PulseInData[bPortIndex].TickBuf[wHeadPtr];
	
	for (i=0; i<wTypeNum; i++)
	{
		wInnerID = m_PulsePara.DemandPara.wInnerID[i]; //�ڲ�����ĵ���ID
		dwDemandPulse[i] = m_dwPulse[wInnerID];		
		dwDemandTick = GetTick();
		if (dwDemandTick - dwDemandTick0 > 60*1000)
			dwDemandTick = GetTick();
		else
			dwDemandTick = dwDemandTick0;			
	}		
	
	//m_Demand.CalcuDemand(dwDemandPulse, &dwDemandTick);*/
	
	/*if (m_fTrigerSave) //�ڷ��ʡ��·ݻ򳭱��շ����л���ʱ�򣬴������ݿ�ȥ���汾�����������
	{	  				
		TrigerSaveBank(BN0, SECT_PN_DATA, m_wPn);	//�������� //TrigerSavePoint(m_wPn);
		m_fTrigerSave = false;
	}*/
	
	memcpy(m_dwLastPulse, m_dwPulse, sizeof(m_dwPulse));
}


void CPulse::SaveLog()
{
#ifdef ACLOG_ENABLE
		
	if (m_fStopSaveLog)
		return;	

	m_Energy.SaveLog();
	//m_Demand.SaveLog();
	
#endif //ACLOG_ENABLE
	
}

//�����־����
bool CPulse::ClearLog()
{
#ifdef ACLOG_ENABLE

	m_fStopSaveLog = true;
	
	m_Energy.ClearLog();
	//m_Demand.ClearLog();
	
#endif //ACLOG_ENABLE
	
	return true;
}


//�����־����
bool CPulse::ClearLogBlock(BYTE bPortNo)
{
#ifdef ACLOG_ENABLE

	m_fStopSaveLog = true;
	
	CDataLog PulseLog;
	WORD wEnergyID = LOG_PULSE_ENERGY1 + bPortNo - 1;
	//WORD wDemandID = LOG_PULSE_DEMAND1 + bPortNo - 1;

	PulseLog.ClearBlock(wEnergyID);
	//PulseLog.ClearBlock(wDemandID);
	
#endif //ACLOG_ENABLE
	
	return true;
}



//���ա����µ���ͳ�����ݼ���ǰ����ʾֵ��0
bool CPulse::ResetData()
{
	int iLen;
	WORD i, wOI;
	BYTE bAttr;
	BYTE bBuf[80];

	wOI = OI_PULSE_BASE + m_wPn;
	for (bAttr=ATTR7; bAttr<=ATTR18; bAttr++)	//��������7~18 ������0
	{		
		memset(bBuf, 0, sizeof(bBuf));
		iLen = OoReadAttr(wOI, bAttr, bBuf, NULL, NULL);
		if (iLen > 0)
		{
			for (i=0; i<TOTAL_RATE_NUM; i++)
			{
				memset(bBuf+9*i+3, 0, 8);	//8 �߾��ȵ�������=long64-unsigned 
			}

			OoWriteAttr(wOI, bAttr, bBuf);
		}
	}

	TrigerSaveBank(BN0, SECT2, -1);

	return true;
}



CPulseManager::CPulseManager()
{
    m_bPulseNum = 0;
    m_bPulsePnNum = 0;
    m_dwLastTick = 0;
    m_bYMFlag = 0xff;	//����ռ��ң�ű�־λ
}

CPulseManager::~CPulseManager()
{

}

bool CPulseManager::Init()
{
    int iLen = 0;
	DWORD dwOA;
    bool fPulsePnValid;
    int64 i64Const;
	WORD i, j, wPn, wID, wOI, wFmtLen=0;
	BYTE bIndex, bProp, bPulsePnIndex, bYMFlag, bCfgNum=0, bType = 0;	
	TPulseCfg PulseCfg;
    TPulseCfg* pPulseCfg = &PulseCfg;
    BYTE bBuf[PULSE_CFG_ID_LEN];
	BYTE bNum = 1, bPortNo = 1;
	BYTE* pbFmt = NULL;
	const WORD wRateNum = RATE_NUM+1;
	
	m_bPulseNum = 0;
    m_bPulsePnNum = 0;	//�����
    m_dwLastTick = 0;
	m_dwLastStatClick = 0;
    
    memset(&g_PulseInData, 0, sizeof(g_PulseInData));
    memset(m_PulsePnDesc, 0, sizeof(m_PulsePnDesc));
	
	for (bType=0; bType<MAX_PULSE_TYPE; bType++)
	{
		wID = PULSE_HI_POSE_ID + bType;
		ReadItemEx(BN0, wPn, wID, bBuf);
		AcFmtToEng(wID, m_i64LastE[bType], bBuf, false, false, 0, wRateNum);	//��ǰ����ʾֵ
	}

	ReadItemEx(BN10, PN0, 0xa1bd, &bNum);
	if(bNum==0 || bNum>8)
		bNum = 1;

    for (i=0; i<MAX_YMNUM; i++)
    {
    	for (j=0; j<MAX_PULSE_TYPE; j++)
			m_PulsePnDesc[i].bIndex[j] = m_PulsePnDesc[i].bPortNo[j] = 0xff;	//����Ϊ��Ч0xff

		m_Pulse[i].SetValid(false);
    }
    
    bIndex = 0;	//��������
    bPulsePnIndex = 0;	//�������������
    bYMFlag = 0;
    m_bYMFlag = 0xff;	//��ʼ��Ϊȫ��ռ��
    
	//DTRACE(DB_FA, ("CPulseManager::Init: bNum=%d\r\n", bNum));
    for (wPn=PN0; wPn<PULSE_PN_NUM; wPn++)		//����������<-->������� ӳ��
    {
        fPulsePnValid = false;
		wOI = OI_PULSE_BASE + wPn;
		DTRACE(DB_FA, ("CPulseManager::Init: wOI=0x%04x\r\n", wOI));
		iLen = OoReadAttr(wOI, ATTR4, bBuf, &pbFmt, &wFmtLen);	//�������ò���

		#if 0
		BYTE g_bPulseCfgBuf[] = {0x01, 0x02, 0x02, 0x03, 0x51, 0x00, 0x10, 0x02, 0x00, 0x16, 0x00, 0x12, 0x00, 0x19, 
											 0x02, 0x03, 0x51, 0x00, 0x10, 0x02, 0x00, 0x16, 0x00, 0x12, 0x00, 0x19,
		};
		#endif

		bCfgNum = bBuf[1];	//����ʵ������·��
		if (iLen>0 && bCfgNum>0 && bCfgNum<=MAX_PULSE_TYPE)		//��������ʱ�豣֤ÿ�����������4�����͵���������
        {
			DTRACE(DB_FA, ("CPulseManager::Init: bCfgNum=%d.\r\n", bCfgNum));
            memset(&PulseCfg, 0, sizeof(TPulseCfg));
    		for (j=0; j<bCfgNum; j++)
    		{
				dwOA = OoOadToDWord(bBuf+j*PULSE_CFG_LEN+5);	//OAD
				DTRACE(DB_FA, ("CPulseManager::Init: dwOA=0x%08x\r\n", dwOA));
				bPortNo = dwOA & 0xff;		//��ȡ����˿ں�
				if ((dwOA>>16) != OI_PULSE_INPUT)	//�жϲ����Ƿ���Ч
					continue;

    		    pPulseCfg->wPn = wPn;
    			pPulseCfg->bPortNo = bPortNo+bNum-1;	//����˿ں�
    			pPulseCfg->bType = bBuf[j*PULSE_CFG_LEN+10];		//��������
    			pPulseCfg->i64Const = OoLongUnsignedToWord(bBuf+j*PULSE_CFG_LEN+12);		//�����
				DTRACE(DB_FA, ("CPulseManager::Init: wPn=%d, bPortNo=%d, bType=%d, i64Const=%ld.\r\n", wPn, pPulseCfg->bPortNo, pPulseCfg->bType, pPulseCfg->i64Const));
    			if (pPulseCfg->bPortNo>0 && pPulseCfg->bPortNo<=MAX_YMNUM && pPulseCfg->i64Const>0 && pPulseCfg->bType<MAX_PULSE_TYPE)
    			{
					DTRACE(DB_FA, ("CPulseManager::Init2: wPn=%d, bPortNo=%d, bType=%d, i64Const=%ld.\r\n", wPn, pPulseCfg->bPortNo, pPulseCfg->bType, pPulseCfg->i64Const));
					if (m_Pulse[bIndex].Init(pPulseCfg))
					{
					    m_PulsePnDesc[bPulsePnIndex].wPn = wPn;
					    m_PulsePnDesc[bPulsePnIndex].bPortNo[pPulseCfg->bType] = pPulseCfg->bPortNo;
					    m_PulsePnDesc[bPulsePnIndex].bIndex[pPulseCfg->bType] = bIndex;
						DTRACE(DB_FA, ("CPulseManager::Init3: wPn=%d, bPortNo=%d, bType=%d, i64Const=%ld.\r\n", wPn, pPulseCfg->bPortNo, pPulseCfg->bType, pPulseCfg->i64Const));

					    fPulsePnValid = true;
					    bYMFlag |= (1<<(pPulseCfg->bPortNo-1));	//����ռ�ñ�־
						bIndex++;
						if (bIndex >= MAX_YMNUM)
				    		break;
					}
    			}
    		}

			if (fPulsePnValid)
			{
	    		InitPulseValToDb(bPulsePnIndex);
				bPulsePnIndex++;
				if (bPulsePnIndex >= PULSE_PN_NUM)
    				break;
			}
        }
    }

    m_bYMFlag = bYMFlag;	//����ռ��ң�ű�־λ
	m_bPulseNum = bIndex;	//������Ч������·��
	m_bPulsePnNum = bPulsePnIndex;	//������������
	if (m_bPulseNum == 0)	//F9���������ú���Ҫ�Ѹ�·�������������Ҳ���
	{
		iLen = ReadItemEx(BN1, PN0, 0x2041, bBuf);
	    if (iLen>0 && bBuf[0]!=0)
	    {
			memset(bBuf, 0, sizeof(bBuf));
			for (i=0; i<MAX_YMNUM; i++)
				WriteItemEx(BN1, PN0, 0x2041+i, bBuf);

			TrigerSaveBank(BN1, 0, -1);
	    }
	}

	DTRACE(DB_FA, ("CPulseManager::Init: m_bPulseNum=%d\r\n", m_bPulseNum));

	return true;
}

//�жϱ�����������Ƿ���Ч������˿�ȫΪ0����Ч��
bool CPulseManager::IsPulsePnInvalid(TPulsePnDesc* pPnDesc)
{
	BYTE bType;
	for (bType=0; bType<MAX_PULSE_TYPE; bType++)
	{
		if (pPnDesc->bPortNo[bType] != 0)
			return false;
	}

	return true;
}

void CPulseManager::CalcPwr()
{
	WORD wPnIndex;
	int64 i64PosPower, i64NegPower, i64PosNPower, i64NegNPower;
    BYTE bPosEpIndex, bNegEpIndex, bPosEqIndex, bNegEqIndex;
	int iVal[PULSE_VAL_NUM];
	
	for (wPnIndex=0; wPnIndex<m_bPulsePnNum; wPnIndex++)	//������������
	{
	    TPulsePnDesc* pPnDesc = &m_PulsePnDesc[wPnIndex];
		if (IsPulsePnInvalid(pPnDesc))
			continue;

		i64PosPower = 0;
		i64NegPower = 0;

   		bPosEpIndex = pPnDesc->bIndex[EP_POS];
   		bNegEpIndex = pPnDesc->bIndex[EP_NEG];
   		
   		if (bPosEpIndex < MAX_YMNUM)
   			i64PosPower = m_Pulse[bPosEpIndex].CurPower();	//��·���й�����
   		if (bNegEpIndex < MAX_YMNUM)
   			i64NegPower = m_Pulse[bNegEpIndex].CurPower();	//��·���й�����

   		if (i64PosPower != 0)
   			iVal[0] = i64PosPower;
   		else if (i64NegPower != 0)
   			iVal[0] = i64NegPower;
   		else
   			iVal[0] = 0;
   			
   		bPosEqIndex = pPnDesc->bIndex[EQ_POS];
   		bNegEqIndex = pPnDesc->bIndex[EQ_NEG];

   		i64PosNPower = 0;
   		i64NegNPower = 0;

   		if (bPosEqIndex < MAX_YMNUM)
   			i64PosNPower = m_Pulse[bPosEqIndex].CurPower();	//��·���й�����
   		if (bNegEqIndex < MAX_YMNUM)
   			i64NegNPower = m_Pulse[bNegEqIndex].CurPower();	//��·���й�����

   		if (i64PosNPower != 0)
   			iVal[1] = i64PosNPower;
   		else if (i64NegNPower != 0)
   			iVal[1] = i64NegNPower;
	    else
	    	iVal[1] = 0;

	    PulseValToDb(wPnIndex, iVal);
	}
}

void CPulseManager::CalcPnStatEnergy(WORD wPnIndex, bool fClrDayEnergy, bool fClrMonthEnergy)
{	
	BYTE bType = 0;
	WORD i, wPn, wID;
	int64 i64E[MAX_PULSE_TYPE][RATE_NUM+1]; //�����ݿ��Ӧ�ĵ���
	int64 i64DeltaE[MAX_PULSE_TYPE][RATE_NUM+1];
	WORD wEDayID[] = { 0x2406, 0x2410, 0x2408, 0x2412 };
	WORD wEMonthID[] = { 0x2407, 0x2411, 0x2409, 0x2413 };
	WORD wStartDayID[] = {0x0d01, 0x0d05, 0x0d03, 0x0d07 };
	WORD wStartMonthID[] = {0x0d02, 0x0d06, 0x0d04, 0x0d08 };

	BYTE bBuf[80];
	int64 i64LastDayE[MAX_PULSE_TYPE][RATE_NUM+1];		//����������
	int64 i64LastMonthE[MAX_PULSE_TYPE][RATE_NUM+1];	//����������
	const WORD wRateNum = RATE_NUM+1;
	TPulsePnDesc* pPnDesc = NULL;
	TTime tmNow, tmLastStat;

	GetCurTime(&tmNow);
	pPnDesc = &m_PulsePnDesc[wPnIndex];
	if (IsPulsePnInvalid(pPnDesc))
	{
		DTRACE(DB_FA, ("CPulseManager::CalcPnStatEnergy: wPnIndex=%d invalid, return\r\n", wPnIndex));
		return;
	}

	wPn = pPnDesc->wPn;
	ReadItemEx(BN11, wPn, 0x0d00, bBuf);
	Fmt15ToTime(bBuf, tmLastStat);
	if (IsDiffDay(tmLastStat, tmNow))
	{
		DTRACE(DB_FA, ("CPulseManager::CalcPnStatEnergy: wPn=%d day chg.\r\n", wPn));
		for (bType=0; bType<MAX_PULSE_TYPE; bType++)
		{
			wID = PULSE_HI_POSE_ID + bType;
			ReadItemEx(BN0, wPn, wID, bBuf);

			WriteItemEx(BN11, wPn, wStartDayID[bType], bBuf);		//���������ֵ
		}

		fClrDayEnergy = true;

		if (!IsSameMon(tmLastStat, tmNow))
		{
			for (bType=0; bType<MAX_PULSE_TYPE; bType++)
			{
				wID = PULSE_HI_POSE_ID + bType;
				ReadItemEx(BN0, wPn, wID, bBuf);
				WriteItemEx(BN11, wPn, wStartMonthID[bType], bBuf);		//���������ֵ
			}

			fClrMonthEnergy = true;
		}

		tmLastStat = tmNow;
		TimeToFmt15(tmLastStat, bBuf);
		WriteItemEx(BN11, wPn, 0x0d00, bBuf);	//�������һ��ͳ��ʱ��

		TrigerSaveBank(BN11, 0, -1);
	}

	//��ȡ����/�������ֵ
	for (bType=0; bType<MAX_PULSE_TYPE; bType++)
	{	
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN11, wPn, wStartDayID[bType], bBuf);
		AcFmtToEng(wStartDayID[bType], i64LastDayE[bType], bBuf, false, false, 0, wRateNum);	//����������ʾֵ
		if (bType == 0)
			DTRACE(DB_FA, ("wStartDayID[bType]=%d, i64LastDayE[bType][0]=%lld, i64LastDayE[bType][1]=%lld, i64LastDayE[bType][2]=%lld, , i64LastDayE[bType][3]=%lld.\r\n",  wStartDayID[bType], i64LastDayE[bType][0], i64LastDayE[bType][1], i64LastDayE[bType][2], i64LastDayE[bType][3]));

		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN11, wPn, wStartMonthID[bType], bBuf);
		AcFmtToEng(wStartMonthID[bType], i64LastMonthE[bType], bBuf, false, false, 0, wRateNum);	//����������ʾֵ		
	}

	//��ȡ��ǰֵ�����õ�ǰֵ-���ֵ���������ֵ
	for (bType=0; bType<MAX_PULSE_TYPE; bType++)
	{
		wID = PULSE_HI_POSE_ID + bType;
		ReadItemEx(BN0, wPn, wID, bBuf);
		AcFmtToEng(wID, i64E[bType], bBuf, false, false, 0, wRateNum);	//��ǰ����ʾֵ
		if (bType == 0)
			DTRACE(DB_FA, ("wCurID = %04x , i64E[bType][0]=%lld, i64E[bType][1]=%lld, i64E[bType][2]=%lld, i64E[bType][3]=%lld.\r\n",  wID, i64E[bType][0], i64E[bType][1], i64E[bType][2], i64E[bType][3]));

		if (memcmp(i64E[bType], m_i64LastE[bType], wRateNum*sizeof(int64))!=0 || fClrDayEnergy || fClrMonthEnergy)		//����/�����б仯
		{
			DTRACE(DB_FA, ("step000, bType=%d, fClrDayEnergy=%d, fClrMonthEnergy=%d.\r\n",  bType, fClrDayEnergy, fClrMonthEnergy));
			//���յ�������
			if (fClrDayEnergy)
			{
				memset(i64DeltaE[bType], 0, wRateNum*sizeof(int64));
			}
			else
			{
				for (i=0; i<wRateNum; i++)
				{					
					if (i64E[bType][i] > i64LastDayE[bType][i])
						i64DeltaE[bType][i] = i64E[bType][i]-i64LastDayE[bType][i];
					else
						i64DeltaE[bType][i] = 0;

					DTRACE(DB_FA, ("bType=%d i64E[%d][i]=%lld, i64LastDayE=%lld, i64DeltaE=%lld.\r\n",  bType, bType, i, i64E[bType][i], i64LastDayE[bType][i], i64DeltaE[bType][i]));
				}
			}

			memset(bBuf, 0, sizeof(bBuf));
			AcEngToFmt(wEDayID[bType], i64DeltaE[bType], bBuf, false, false, wRateNum);
			DTRACE(DB_FA, ("step111 wPn=%d, wEDayID[%d]=%04x, i64DeltaE[0]=%lld, i64DeltaE[1]=%lld, i64DeltaE[2]=%lld, i64DeltaE[3]=%lld.\r\n", wPn, bType, wEDayID[bType], i64DeltaE[bType][0], i64DeltaE[bType][1], i64DeltaE[bType][2], i64DeltaE[bType][3]));
			//DTRACE(DB_FA, ("step222 wPn=%d, bBuf[0]=%lld, i64DeltaE[1]=%lld, i64DeltaE[2]=%lld, i64DeltaE[3]=%lld.\r\n", wPn, i64DeltaE[bType][0], i64DeltaE[bType][1], i64DeltaE[bType][2], i64DeltaE[bType][3]));
			//TraceBuf(DB_FA, "step333-> ", (BYTE*)bBuf, 27);
			WriteItemEx(BN0, wPn, wEDayID[bType], bBuf);

			//���µ�������
			if (fClrMonthEnergy)
			{
				memset(i64DeltaE[bType], 0, wRateNum*sizeof(int64));
			}
			else
			{
				for (i=0; i<wRateNum; i++)
				{
					if (i64E[bType][i] > i64LastMonthE[bType][i])
						i64DeltaE[bType][i] = i64E[bType][i]-i64LastMonthE[bType][i];
					else
						i64DeltaE[bType][i] = 0;
				}
			}

			memset(bBuf, 0, sizeof(bBuf));
			AcEngToFmt(wEMonthID[bType], i64DeltaE[bType], bBuf, false, false, wRateNum);
			WriteItemEx(BN0, wPn, wEMonthID[bType], bBuf);

			memcpy(m_i64LastE[bType], i64E[bType], wRateNum*sizeof(int64));
		}
	}
}

//����&���µ���ͳ��
void CPulseManager::CalcStatEnergy()
{	
	WORD wPnIndex = 0;	
	bool fClrDayEnergy = false, fClrMonthEnergy = false;

	for (wPnIndex=0; wPnIndex<m_bPulsePnNum; wPnIndex++)	//������������
		CalcPnStatEnergy(wPnIndex, fClrDayEnergy, fClrMonthEnergy);
}



void CPulseManager::Run()
{
	WORD i;	

	if (GetInfo(INFO_PULSE))	//�������ò���8903����
	{
	    Init();
	    for (i=0; i<m_bPulseNum; i++)
			m_Pulse[i].LoadPara();

		SetInfo(INFO_YX_PARA);
	}

	if (GetInfo(INFO_PULSEDATA_RESET))	//�յ���λ��������
	{
		ResetPulseData();
		SetInfo(INFO_PULSE);	//���³�ʼ��
		return;
	}

	TTime now;
	GetCurTime(&now);
	bool fCalcuPwr = false;		//�����й��ʵĸ��¶�ͬ��
	if (GetTick()-m_dwLastTick >= 60*1000)
	{ //�ϵ糬��30�������
		m_dwLastTick = GetTick();
		fCalcuPwr = true;
	}

	for (i=0; i<m_bPulseNum; i++)
	{
		m_Pulse[i].Run(fCalcuPwr);
	}
	
	if (fCalcuPwr)
		CalcPwr();	//�������

	if (GetClick()-m_dwLastStatClick >= 3)
	{
		CalcStatEnergy();
		m_dwLastStatClick = GetClick();
	}
}


//����洢�������������
void CPulseManager::SaveLog()
{
#ifdef ACLOG_ENABLE
	WORD i;

    for (i=0; i<m_bPulseNum; i++)
	{
		m_Pulse[i].SaveLog();
	}
#endif //ACLOG_ENABLE
}


void CPulseManager::ClearLog()
{
#ifdef ACLOG_ENABLE
	WORD i;

    for (i=0; i<m_bPulseNum; i++)
	{
		m_Pulse[i].ClearLog();
	}
#endif //ACLOG_ENABLE
}


bool  CPulseManager::ResetPulseData()
{
	WORD i, wPn, wPnIndex;
	bool fTrigSave = false;
	BYTE bBuf[10];

	for (wPn=PN0; wPn<PULSE_PN_NUM; wPn++)		//����������<-->������� ӳ��
	{
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN11, wPn, 0x0b14, bBuf);
		if (bBuf[0]==wPn && bBuf[1]==0xA5)		//��λ������Ч
		{
			for (i=0; i<m_bPulseNum; i++)
			{
				if (m_Pulse[i].IsValid() && m_Pulse[i].GetPn()==wPn)	//�ҳ��ò������µ���Ч�������
				{
					m_Pulse[i].ClearLog();		//��������־
					m_Pulse[i].ResetData();		//��ͳ���м�����

					m_Pulse[i].SetValid(false);		//��ֹ��0������ݱ�����
				}
			}

			for (wPnIndex=0; wPnIndex<m_bPulsePnNum; wPnIndex++)	//������������
			{
				TPulsePnDesc* pPnDesc = &m_PulsePnDesc[wPnIndex];
				if (pPnDesc->wPn == wPn)
				{
					DTRACE(DB_FA, ("CPulseManager::ResetPulseData: clr wPn=%d stat data.\r\n", wPn));
					CalcPnStatEnergy(wPnIndex, true, true);
				}
			}

			memset(bBuf, 0, sizeof(bBuf));
			WriteItemEx(BN11, wPn, 0x0b14, bBuf);	//�������
			fTrigSave = true;
		}
	}

	if (fTrigSave)
		TrigerSaveBank(BN11, 0, -1); //��������

	return true;
}


//����1����λ
int OnResePulseCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bPn;
	BYTE bBuf[10];

	if (GetOiClass(wOI)!=IC12 || bMethod!=OMD_PULSE_RESET)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}
	
	bPn = bBuf[0] = (wOI - OI_PULSE_BASE);
	if (bPn >= PULSE_PN_NUM)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	bBuf[1] = 0xA5;	//��Ч��־
	WriteItemEx(BN11, bPn, 0x0b14, bBuf);
	TrigerSaveBank(BN11, 0, -1); //��������

	SetInfo(INFO_PULSEDATA_RESET);
	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}

//����2��ִ��
//�պ���
int OnRunPulseCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	if (GetOiClass(wOI)!=IC12 || bMethod!=OMD_PULSE_RUN)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}
	
	//nothing to do

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}


//BYTE bCmpBuf[] = {0x02, 0x03, 0x51, 0x00, 0x10, 0x02, 0x00, 0x16, 0x00, 0x12, 0x00, 0x19, 
//����3������������뵥Ԫ
//������=�������뵥Ԫ
int OnAddPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, j;
	int iLen;
	bool fTypeAlreadyExist = false;
	BYTE bCfgNum;
	BYTE bBuf[PULSE_CFG_ID_LEN];
		
	if (GetOiClass(wOI)!=IC12 || bMethod!=OMD_PULSE_ADDCFG || pbPara[0]!=DT_STRUCT || pbPara[1]!=3)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}	

	// ��ȡ�������Ա�
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//��ȡ��������
	if (iLen <= 0)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}
	
	bCfgNum = bBuf[1];	
	for (i=0; i<bCfgNum; i++)	//�����Ƿ��Ѿ����� bBuf+j*PULSE_CFG_LEN+5
	{
		if (FieldCmp(DT_PULSE_CFG, &bBuf[i*PULSE_CFG_LEN + 3], DT_PULSE_CFG, pbPara+1) == 0)	//��ȫ��ͬ������Ϊ����Ч����,��֤OADΨһ��
		{
			*pbRes = 3;	//�ܾ���д ��3��
			return -1;
		}
		else if (FieldCmp(DT_PULSE_CFG, &bBuf[i*PULSE_CFG_LEN + 3], DT_OAD, pbPara+OFFSET_PULSE_PORT) == 0)	//OAD��ͬ�����������Ի����峣����ͬ,�޸�
		{
			fTypeAlreadyExist = false;
			for (j=0; j<bCfgNum; j++)	//�����Ƿ��Ѿ����� bBuf+j*PULSE_CFG_LEN+5
			{
				if (j == i)
					continue;

				if (bBuf[j*PULSE_CFG_LEN + 10] == pbPara[OFFSET_PULSE_TYPE])	//�������Ѿ����� +10Ϊ���Ե�ƫ��
				{
					fTypeAlreadyExist = true;
					*pbRes = 3;	//�ܾ���д ��3��
					return -1;
				}
			}

			if (!fTypeAlreadyExist)	//����������û���ù�
			{
				memcpy(&bBuf[i*PULSE_CFG_LEN + 2], pbPara, PULSE_CFG_LEN);	//�޸Ĺ����������
				break;
			}
		}
	}

	// �����һ����������
	if (i == bCfgNum)
	{
		if (bCfgNum >= MAX_PULSE_TYPE)		//�Ѿ������� ����ʧ��
		{
			*pbRes = 3;	//�ܾ���д ��3��
			return -1;
		}

		fTypeAlreadyExist = false;
		for (j=0; j<bCfgNum; j++)	//�����Ƿ��Ѿ����� bBuf+j*PULSE_CFG_LEN+5
		{
			if (bBuf[j*PULSE_CFG_LEN + 10] == pbPara[OFFSET_PULSE_TYPE])
			{
				fTypeAlreadyExist = true;	//�������Ѿ�����
				*pbRes = 3;	//�ܾ���д ��3��
				return -1;
			}
		}

		if (!fTypeAlreadyExist)	//����������û���ù�
		{
			memcpy(&bBuf[i*PULSE_CFG_LEN + 2], pbPara, PULSE_CFG_LEN);
			bCfgNum++;
			bBuf[1] = bCfgNum;	//����Ԫ�ظ���
		}		
	}

	if (OoWriteAttr(wOI, ATTR4, bBuf) <= 0)		//������������
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}
	else
	{		
		*pbRes = 0;	//�ɹ�  ��0��// ���ؽ��
		return 1;
	}
}
int BatAddPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD wLen;
	const ToaMap *pOadMap;
	BYTE *pbFeildFmt;
	WORD wFeildLen;
	BYTE bPnNum, bType;
	int iTmpLen = iParaLen;
	BYTE* pbTmp = pbPara;
	DWORD dwOAD;
	pbTmp++; //��������
	bPnNum = *pbTmp++; //Ԫ�ظ���
	iTmpLen -= 2;
	dwOAD = wOI;
	dwOAD = (dwOAD<<16) + 0x0400;
	pOadMap = GetOIMap(dwOAD);
	for (BYTE i=0; i<bPnNum; i++)
	{
		pbTmp = OoGetField(pbPara, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType, &pbFeildFmt, &wFeildLen);
		if (OnAddPulseCfgCmd(wOI, bMethod, bOpMode, pbTmp, wLen, pvAddon, pbFeildFmt, wFeildLen, pbRes) == 1)
		{
			iTmpLen -= wLen;
		}
		else
		{
			return -1;
		}
	}
	if (iTmpLen == 0)
		return 0;
	else
		return -1;
}

//����4��ɾ���������뵥Ԫ
//������=��������˿ں�OAD
int OnDelPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, j;
	int iLen;
	BYTE bCfgNum;
	BYTE bBuf[PULSE_CFG_ID_LEN];
		
	if (GetOiClass(wOI)!=IC12 || bMethod!=OMD_PULSE_DELCFG || pbPara[0]!=DT_OAD)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//��ȡ��������
	if (iLen<=0 || bBuf[1]==0)	//�յ�
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	if (bBuf[1] > MAX_PULSE_TYPE)
		bCfgNum = MAX_PULSE_TYPE;
	else
		bCfgNum = bBuf[1];

	for (i=0; i<bCfgNum; i++)	//�����Ƿ��Ѿ�����
	{
		if (FieldCmp(DT_PULSE_CFG, &bBuf[i*DT_FRZRELA_LEN + 3], DT_OAD, pbPara+1) == 0)	//�ҵ�Ŀ��
		{
			memset(&bBuf[i*PULSE_CFG_LEN + 2], 0, PULSE_CFG_LEN);
			bBuf[1]--;	//����Ԫ�ظ���
			break;
		}
	}
	
	if (i == bCfgNum)	//û�ҵ�
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	for (j=i; j<bCfgNum-1; j++)		//����Ĳ�����ǰŲ
	{
		memcpy(&bBuf[j*PULSE_CFG_LEN + 2], &bBuf[(j+1)*PULSE_CFG_LEN + 2], PULSE_CFG_LEN);
	}

	// ˢ�¹������Ա�
	if (OoWriteAttr(wOI, ATTR4, bBuf) <= 0)		//������������
	{
		*pbRes = 3;			//дʧ�� ��3��
		return -1;
	}
	else
	{
		*pbRes = 0;		//�ɹ�  ��0�� ���ؽ��
		return 1;
	}
}

