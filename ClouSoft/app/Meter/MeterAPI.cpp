/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterAPI.cpp
 * 摘    要：本文件主要实现抄表的公共接口
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年9月
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

//描述：初始化电表屏蔽字
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

//描述: DLMS协议的电表参数初始化
//参数: @wPn 测量点号
//		@pMtrPara 指向存放电表参数数组的电表参数结构指针
//返回: 成功则返回true		
bool GetMeterPara(WORD wPn, TMtrPara* pMtrPara)
{	
	int iPort = 0;
	DWORD dwPort = 0;
	BYTE bBuf[PNPARA_LEN];	
	
	memset(bBuf, 0,sizeof(bBuf));	
	if (ReadItemEx(BN0, wPn, 0x6000, bBuf) <= 0)
		return false;

	memset(pMtrPara, 0, sizeof(TMtrPara));

	//拷贝参数
	pMtrPara->wPn = wPn;
	//BYTE bAddL = bBuf[9];
	BYTE bAddL = bBuf[10]+1;
	pMtrPara->bAddr[0] = bAddL;	//地址长度
	memcpy(&pMtrPara->bAddr[1], &bBuf[11], bAddL);	  //地址内容

	pMtrPara->bProId = bBuf[14+bAddL];				  //规约类型
#ifdef EN_SBJC_V2
	pMtrPara->bSubProId = GetMeterSubPro(wPn);
    if (pMtrPara->bProId == PROTOCOLNO_SBJC)
    {
        memcpy(pMtrPara->bMtrAddr, &bBuf[12], 7);	//6	
       // pMtrPara->bMtrAddr[6] = GetMeterType(wPn);//tll
    }
#endif
	dwPort = OoOadToDWord(&bBuf[16+bAddL]);
	if ((dwPort&0xFFFFFF00) == 0xF2090200)	//载波端口
	{
		//DTRACE(DB_METER, ("GetMeterPara : plc port.\n"));
		iPort = PORT_CCT_PLC;
	}
	else if ((dwPort&0xFFFFFF00) == 0xF2010200)	//485端口
	{
		iPort = MeterPortToPhy(bBuf[19+bAddL]); // 抄表的逻辑端口到物理端口的映射
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

//描述:	获取电表地址,
//参数:	@wPn 测量点号
//		@pbAddr 用来返回地址
//返回:	如果成功则返回true,否则返回false
//备注:	对于载波表, 载波物理地址与电表地址一致,
//		对于采集器模型,这里取目的电表地址.
BYTE GetMeterAddr(WORD wPn, BYTE* pbAddr)
{
	BYTE bBuf[PNPARA_LEN];

	if (ReadItemEx(BN0, wPn, 0x6000, bBuf)<=0)
		return 0;

	BYTE bAddL = bBuf[9] + 1;
	memcpy(pbAddr, &bBuf[9], bAddL);
	return bAddL;
}

//描述:	获取测量点通道号
//参数:	@wPn 测量点号
BYTE GetPnCn(WORD wPn)
{
	BYTE bBuf[PNPARA_LEN];

	if (ReadItemEx(BN0, wPn, 0x6000, bBuf)<=0)
		return 0;

	BYTE bAddL = bBuf[9] + 1;

	return bBuf[18+bAddL];
}

//描述:取抄表间隔
WORD GetMeterInterv()
{
	BYTE bMeterInterv = 0;
	
	if (ReadItemEx(BN0, PN0, 0x6708, &bMeterInterv) > 0)	//终端配置
	{	
		//如果没有设置,系统库默认值为0,下面会自动取默认值
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
//初始化电表缓存控制结构
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

//更新电表缓存控制结构
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

//描述：初始化电表抄读控制结构
//参数：@wPn
//		@pbTsa
//		@pMtrRdCtrl
//返回：无
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
	TTimeInterv tiExe;	 //采集周期
	BYTE bPnMask[PN_MASK_SIZE];

	memset(pMtrRdCtrl, 0, sizeof(TMtrRdCtrl));

	pdwOAD = MtrGetFixedItems(&wNum);
	pwLen = MtrGetFixedLen();
	InitMtrTmpData(&pMtrRdCtrl->mtrTmpData, pdwOAD, pwLen, wNum);

	pMtrRdCtrl->bTaskSN = GetTaskCfgSn();
	memcpy(pMtrRdCtrl->bTsa, pbTsa, pbTsa[0]+1);
	for (WORD wIndex= 0; wIndex<TASK_NUM; wIndex++)	//遍历任务配置表
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
						if (dwOAD == 0x50020200) //分钟曲线数据
						{
							pbCollMode = OoGetField(pbSch, pFmt, wFmtLen, 2, &wLen, &bType);	//采集方式
							bType = pbCollMode[3];
							tiExe.bUnit = pbCollMode[5];
							tiExe.wVal = OoLongUnsignedToWord(pbCollMode+6);
							if (bType==3 /*&& tiExe.bUnit==1*/) //按TI间隔采集
							{
								DWORD dwTiMin = TiToSecondes((TTimeInterv*)&tiExe)/60;
								if (dwTiMin == 0)
									dwTiMin = 15;
								wLen = (TiToSecondes((TTimeInterv*)&tTaskCfg.tiExe)/60/dwTiMin + 7)/8; //每间隔占1比特
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

				//case SCH_TYPE_TRANS: (透明方案不在这里处理)
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

	//以下为抄表事件初始化
	InitMtrExcCtrl(wPn, &pMtrRdCtrl->mtrExcTmp);	//初始化抄表事件控制结构

	AllocateMtrExcMem(pMtrRdCtrl->bGlobal, pMtrRdCtrl->allocTab, MTR_TAB_NUM);	//分配抄表事件临时内存空间
}

void DoMangerMtrCacheCtrl()
{
	WaitSemaphore(g_semMtrCtrl);
	//遍历抄表控制结构
	for (BYTE bIndex=0; bIndex<MTR_CACHE_NUM; bIndex++)	
	{
		//抄表结构有效 && 为脏区 && 缓存时间超过10分钟
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
//电表抄读控制结构接口函数定义
//说明：
//1、保存到文件系统中的文件以MtrRdCtrl_Pn%d.dat来命名
//2、g_MtrCacheCtrl[]匹配某个电表的控制结构是否缓存在内存时，只需要wPn匹配即可，bTsa主要用来比较文件导入的电表地址是否相等，
//	 如果不等，则说明该测量点的参数发生改变，导入的数据无效
//3、GetMtrRdCtrl()首先从g_MtrCacheCtr[]中找，如果该电表的控制结构已经缓存在内存中则直接返回；否则分配一个新空间：
//	先找CACHE_STATUS_FREE的空间，如果没有则需要把最老的CACHE_STATUS_IDLE结构导出到文件系统以释放出一个空间，
//	再从文件系统中导入该表的结构到刚分配的空间中。
//4、TMtrRdCtrl*指针从g_MtrCacheCtrl[]中取出和放回，需要修改bStatus状态，更新dwLastAcessTime

//描述:取对应表地址的电表抄读控制结构
//参数：@wPn 要取的测量点号
//		@pbTsa表地址，主要用来校验电表参数是否发生改变
//返回:如果成功则返回对应表地址的电表抄读控制结构的指针，否则返回NULL
TMtrRdCtrl* GetMtrRdCtrl(WORD wPn, BYTE*pbTsa)
{
	int i;
	int iLastInx = -1;
	BYTE bAddL = pbTsa[0]+1;
	DWORD dwLastAcessTime = 0xffffffff;

	//先内存中查找
	for (i=0; i<MTR_CACHE_NUM; i++)
	{
		if (g_MtrCacheCtrl[i].wPn == wPn)
		{
			if (memcmp(g_MtrCacheCtrl[i].mtrRdCtrl.bTsa, pbTsa, bAddL)==0 && g_MtrCacheCtrl[i].mtrRdCtrl.bTaskSN==GetTaskCfgSn()) //地址未改变，数据有效
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

	//内存中没有，找一个未分配的
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

	//没有则需要把最老的CACHE_STATUS_IDLE结构导出到文件系统以释放出一个空间
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

//描述:把对应表地址的电表抄读控制结构放回到g_MtrCacheCtrl[]
//参数：@wPn 要放回的测量点号
//		@pbTsa表地址
//		@pMtrRdCtrl 电表抄读控制结构
//		@fModify是否修改了，如果修改了，函数会更新fDirty标志
//返回:无
void PutMtrRdCtrl(WORD wPn, BYTE* pbTsa, bool fModify)
{
	int i;
	BYTE bAddL = pbTsa[0]+1;

	//先内存中查找
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

//下面两个函数电表缓存管理内部使用
//描述:从文件系统装载对应表地址的电表抄读控制结构
//参数：@wPn 要取的测量点号
//		@pbTsa表地址，主要用来校验电表参数是否发生改变
//		@pMtrRdCtrl 用来接收文件中读取到的电表抄读控制结构
//返回:如果成功则返回true，否则返回false
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

//描述:从把电表抄读控制结构保存到文件系统中
//参数：@wPn 要写的测量点号
//		@pMtrRdCtrl 用来接收文件中读取到的电表抄读控制结构
//返回:如果成功则返回true，否则返回false
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

//描述：删除电表控制结构文件
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
//电表临时缓存访问接口定义：
//描述:保存一个数据项到电表临时数据结构
//参数：@pMtrTmpData电表临时数据结构
//	@dwOAD数据标识
//	@pbData数据内容
//	@bLen数据长度
//返回:如果有空间保存则返回true，否则返回false
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
		//未找到，已经到未分配区
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

//描述:从电表临时数据结构读出一个数据项
//参数：@pMtrTmpData电表临时数据结构
//	@dwOAD数据标识
//	@pbData数据内容
//	@bLen数据长度
//返回:如果找到该数据项则返回数据长度，否则返回-1
int GetMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD dwOAD, BYTE* pbData)
{
	for (int i=0; i<MTR_TMP_ITEM_NUM; i++)
	{
		if (pMtrTmpData->item[i].bValid==1 && dwOAD==pMtrTmpData->item[i].dwOAD)
		{
			memcpy(pbData, pMtrTmpData->bBuf+pMtrTmpData->item[i].wOffset, pMtrTmpData->item[i].bLen);

			return pMtrTmpData->item[i].bLen;
		}

		//未找到，已经到未分配区
		if (pMtrTmpData->item[i].bValid==0 && pMtrTmpData->item[i].dwOAD==0 && pMtrTmpData->item[i].bLen==0)
			break;
	}

	return -1;
}

static DWORD g_dwFixRDOad[] = {0x00100200,//正向有功电量
								0x00200200,//反向有功电量
								0x00300200,//组合无功1
								0x00400200,//组合无功2
								0x20000200,//电压
								0x20010200,//电流
								0x20040200,//有功功率
								0x20050200,//无功功率
};

static WORD g_wFixRDInID[] = {0xa010,//正向有功电量
								0xa020,//反向有功电量
								0xa030,//组合无功1
								0xa040,//组合无功2
								0xa050,//电压
								0xa051,//电流
								0xa052,//有功功率
								0xa053,//无功功率
};

static WORD g_wFixDataLen[] = {27,//正向有功电量
								27,//反向有功电量
								27,//组合无功1
								27,//组合无功2
								11,//电压
								22,//电流
								22,//有功功率
								22,//无功功率
};

//描述:取固定抄读列表及数据项个数
//返回:抄读的OAD列表
DWORD* MtrGetFixedItems(WORD* pwItemNum)
{
	*pwItemNum = sizeof(g_dwFixRDOad)/sizeof(DWORD);
	return g_dwFixRDOad;
}

//描述:取固定抄读数据项长度
//返回:抄读的OAD列表数据长度
WORD* MtrGetFixedLen()
{
	return g_wFixDataLen;
}

//描述:取固定抄读数据项内部映射ID
//返回:数据项内部映射ID表
WORD* MtrGetFixedInItems()
{
	return g_wFixRDInID;
}

///////////////////////////////////////////////////////////////////////////////////////
//描述:刷新内部测量点数据
//参数:
//	@wPn测量点号
//	@dwOAD数据标识
//	@pbData数据内容
//返回:如果有保存则返回true，否则返回false
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

//描述:在一个间隔切换后，重新初始化电表临时数据结构
//	把pMtrTmpData整个数据结构清零,预先为pdwFixOAD分配空间，但是bValid==0
//参数：@pMtrTmpData电表临时数据结构
//	@pdwFixOAD 固定缓存OAD数组
//	@pwDataLen固定缓存OAD的数据长度，预先初始化好，
//					不要每回调用的时候重新初始化
//	@wNum固定缓存OAD数组的元素个数
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

//描述:查询电表临时数据结构中，是否缓存有所需要的OAD数组，且bValid==1
//参数：@pMtrTmpData电表临时数据结构
//	@pdwOAD 需要查询的OAD数组
//	@wNum OAD数组的元素个数
//返回:如果全部抄到则返回true,否则返回false
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
//TMtrRdCtrl中临时记录的操作接口
//分配临时记录空间
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
		//未找到，已经到未分配区
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

//释放临时记录空间
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

//读取临时记录
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

//写临时记录
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

//往临时记录里写某个CSD数据
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

//清零临时记录
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
//描述：保存采集到的电表数据
//参数：@pMtrRdCtrl电表抄读控制
//	    @bRespType电表帧的返回类型1:GetResponseNormal; 3:GetResponseRecord。
//                   如果是LIST方式，保存的时候转换成单个再保存
//      @pbCSD抄读的数据项标识，需要根据返回帧的类型，把OAD转换为CSD
//      @pbData表返回的数据
bool SaveMtrData(TMtrRdCtrl* pMtrRdCtrl, BYTE bRespType, BYTE* pbCSD, BYTE* pbData, WORD wDataLen)
{
	//遍历任务及采集方案的配置，电表帧的返回类型bRespType和采集方案的采集方式的采集类型相匹配
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

	if (bRespType == 1) //OAD类型存一下电表数据临时缓存
	{
		dwOAD = OoOadToDWord(pbCSD+1);
		wCSDLen = OoGetDataLen(DT_CSD, pbCSD);
		SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, pbData, wCSDLen);
		//SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp);		//zqq add ----20170412 hyl 屏蔽掉，抄表事件用数据按周期不按任务。这里做存储会使存储的终端时间与读电表数据时刻不一致。建议后续抄表事件按任务来做。。。。。
	}

	for (WORD wIndex=0; wIndex<TASK_NUM; wIndex++)	//遍历任务配置表
	{
		memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
		if (GetTaskCfg(wIndex, &tTaskCfg))
		{
			pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
			if (pbSch!=NULL)
			{
				for (BYTE bTaskId=0; bTaskId<MTR_TASK_NUM; bTaskId++) ///遍历电表结构中的32个任务ID
				{
					if (tTaskCfg.bTaskId==pMtrRdCtrl->taskSucFlg[bTaskId].bTaskId)
					{
						if (pMtrRdCtrl->taskSucFlg[bTaskId].fRecSaved)	//日冻结与小时冻结存在相同的OAD，小时更新时防止日冻结存储
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

									//避免反复计算偏移，只有在找到的情况下才计算
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
										DWORD dwCurRecIndex = OoDoubleLongUnsignedToDWord(pbData+3);	//事件记录序号 高字节在前传输
										if ((pbData[0]==0x00) && (pbData[1]==0x21))	//载波抄读失败 超时：0x00 0x21
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
												int iLen = OoReadAttr(0x4300, ATTR10, bTmpBuf, NULL, NULL);	//读取配置参数
												if (iLen<=0 || bTmpBuf[0]!=DT_ARRAY)
													continue;

												BYTE bChnNum = (bTmpBuf[1]>CN_RPT_NUM) ? CN_RPT_NUM : bTmpBuf[1];
												for (BYTE i=0; i<bChnNum; i++)
												{
													dwChnOAD = OoOadToDWord(&bTmpBuf[5*i+3]);	//通道OAD
													dwOAD = OoOadToDWord(pbTaskCSD);
													SendEvtMsg(dwChnOAD, dwOAD, iRecIdx, EVT_STAGE_TASK, tTaskCfg.bSchNo, bCSDIndex, pbTaskCSD, wRecOffset);
												}
											}
										}
									}
								}
							}
							break;

						//case SCH_TYPE_TRANS: (透明方案不在这里处理)
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


//DWORD dwItemRdTime[ITEM_RD_TIME_NUM];	//依次是正有、反有、时钟 在SaveMtrDataHook()中更新本成员
//在SaveMtrDataHook()中更新本成员
//hyl 20170412 过台子，3105事件只能按任务抄判断数据，添加bType特殊处理
void SaveMtrDataHook(DWORD dwOAD, TMtrExcTmp* pMtrExcTmp, BYTE bType)
{
	DWORD dwCurSec = GetCurTime();
	dwOAD &= OAD_FEAT_MASK;	//去除属性特征

	if (dwOAD==0x00100201 || dwOAD==0x00100200)
		pMtrExcTmp->dwItemRdTime[0] = dwCurSec;
	else if (dwOAD==0x00200201 || dwOAD==0x00200200)
		pMtrExcTmp->dwItemRdTime[1] = dwCurSec;
	else if (dwOAD==0x40000200)
	{	
		if(bType) pMtrExcTmp->dwItemRdTime[2] = dwCurSec;
	}
}