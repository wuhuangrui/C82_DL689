/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DataManager.cpp
 * 摘    要：本文件主要实现系统数据库的数据项读写存储管理
 * 当前版本：见本文VER_STR
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 * 备    注：$测量点动态映射实现说明
			  1.总体规则:
				a.跟测量点相关的参数,数据,扩展参数,临时数据等,都要用统一的方案进行映射管理.
				b.有可能同一个测量点的数据存在多种映射方案,比如测量点参数(国标F10)按照1024块表来映射,
				  测量点数据分不同的属性的测量点来映射,负控表映射64块,载波表映射1024块;对于这种情况,
				  我们把表参数和载波表数据固定分配1024块,不进行动态映射,只是负控表的数据映射出64块;
				  即对于名义上支持很多个测量点实际上只支持少数几个的数据,才进行映射
				c.需要映射的测量点的创建和删除要明确的申请和释放映射的资源
				d.对于测量点动态映射,整个系统数据库只支持有限的几套方案,不同的方案
			      把名义上支持的测量点号映射成实际支持的测量点数,具体配置见TPnMapCtrl
				e.涉及到的测量点数据管理方式有:测量点数量直接配置在数据项中,测量点数据分测量点存放,
				  镜像数据三种,他们都统一根据整个系统的映射方案来管理
				f.测量点到存储号的映射表要保存到文件系统,上电时要从文件系统恢复
				g.测量点到存储号的映射表使用二分法来检索,同时使用存储空间分配表来加快空白空间的查找
				h.对正常读写流程的影响就是在int WriteItem(WORD wImg, WORD wPn, ...)和
				  int ReadItem(WORD wImg, WORD wPn,...)两个函数中,遇到进行动态映射的数据项,
				  都要先先使用SearchPnMap()进行测量点到实际存储号(映射号)的转换
			  2.映射数据上电时从文件系统恢复到RAM的说明:
				a.对于数据分测量点存放的SECT,如测量点数据,存储文件名还是point%d.dat,
				  其中的测量点号指的是存储号,从文件系统恢复时直接根据存储号恢复到对应的内存中,
				  真正的测量点号要根据从文件系统中恢复的测量点到存储号的映射表
				b.对于测量点数量混杂在数据项ID后配置的情况,恢复时也是直接把文件内容拷贝到RAM中,
				  数据对应的测量点也是根据测量点到存储号的映射表
			  3.测量点重新映射后,数据的清除及同步问题?
  				测量点的映射都会伴随着测量点的变更而重新进行映射的,所以测量点映射操作本身不做测量点
				数据的清除,依靠外部函数在新配置测量点且完成映射后,把新测量点的空间清除一遍
			  4.测量点映射发生改变的时候发生测量点数据访问的问题:
---------------------------------------------------------------------------------------------------------
* 版本信息:
 ---2009-2-9:---V1.1.01----岑坚宇---
 	1.增加测量点动态映射的功能
 	2.系统库数据库进一步与数据项配置分离成两个文件,系统库使用参数来初始化,系统库操作不再是以前固定
	  定义好的数据结构
 ---2009-2-9:---V1.1.05----杨进---
 	1.去除IsMeterPn()中对PN_NUM的引用
 	2.编译时把外部函数IsSectHaveSampleData()改为非inline函数
 ---2009-4-16:---V1.1.06----岑坚宇---
 	1.无效节定义改为GetDbInvalidData(),替代INVALID_DATA宏定义,方便不同的版本定义不同的无效字节
 ---2009-4-17:---V1.1.07----岑坚宇---
	1.修正系统库按值读数据项接口,在时间不符合时,如果数据项格式带描述串,没有按照格式串把每个子项赋值为无效数据的BUG
 ---2009-4-17:---V1.1.08----岑坚宇---
	1.版本控制支持开始没有配置支持版本控制,后来又配置支持，文件中旧长度等于0,也认为旧文件有效
 ---2009-4-17:---V1.1.09----岑坚宇---
 	1.只有在一个BANK或SECT的文件个数小于等于8时,才打印文件找不到的信息,避免文件多打印过多
 ---2009-7-30:---V1.1.10----岑坚宇---
 	1.触发保存在执行DoTrigerSaveBank()时,对触发标志进行备份,马上清原标志,再进行文件保存操作,
 	  避免保存完再清标志,导致后面新触发保存的文件由于新标志被清,实际上没保存上
 ---2009-8-13:---V1.1.11----岑坚宇---
	1.计算数据项长度函数GetItemLen(),对于组合ID,长度的计算用子ID的长度和,而不是以前算的是组合ID在系统库
	  中的配置长度,这样的好处是组合ID在系统库中的长度配置为1即可,不用浪费存储资源
 ---2009-9-24:---V1.1.12----岑坚宇---
	1.增加接口int GetItemsLen(TBankItem* pBankItem, WORD wNum);
	2.TItemDesc的wRW字段,配置时支持DI_NTS,表示该数据项不支持时标,
	  主要用在BANK1后面的BANK,数据有带时标和不带时标的,但是又不能分开SECT的情况,
	  可以整个BANK配置成支持时标访问,但个别数据项配置成不支持时标
 ---2009-10-12:---V1.1.13----岑坚宇---
	1.改正使用测量点动态映射的块ID中的子ID,偏移计算不对的问题,块ID的偏移使用新的计算方法
	2.对系统库配置表中的一些错误进行自动修正,比如:
	  a.块ID中的子ID的测量点个数强制为1;
	  b.用到测量点动态映射的,测量点个数跟映射方案相同
 *********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "sysfs.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "DbHook.h"
#include "Trace.h"
#include "sysapi.h"

#define VER_STR	"Ver1.1.40"

extern TSem   g_semDataRW;
extern TTime g_tmAccessDenied;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//CDataManager

CDataManager::CDataManager()
{
	m_pDbCtrl = NULL;
	m_pImgCtrl = NULL;
}

CDataManager::~CDataManager()
{
}

int CDataManager::Save(bool fSaveAll)
{
	int iRet = 0;
	//先保存参数
	iRet += SavePara();
	
	//再保存数据
	iRet += SaveData(fSaveAll);
	
    return iRet;
}

		
//描述:保存参数,后缀名为.cfg的文件
int CDataManager::SavePara()
{
	WORD i;
	int iLen;
	int iRet = 0;
	TBankCtrl* pBankCtrl;
	
	for (i=0; i<m_wSectNum; i++)
	{
		pBankCtrl = &m_pBank0Ctrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL)   //该BANK不需要被保存
		{
			if ((iLen=strlen(pBankCtrl->pszPathName)) > 0)
			{
				if (strcmp(&pBankCtrl->pszPathName[iLen-4], ".cfg") == 0)
				{
					if (SaveBank(pBankCtrl) < 0)
						iRet--;
				}
			}
		}
	}

	for (i=0; i<m_wBankNum; i++)
	{
		pBankCtrl = &m_pBankCtrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL)   //该BANK不需要被保存
		{
			if ((iLen=strlen(pBankCtrl->pszPathName)) > 0)
			{
				if (strcmp(&pBankCtrl->pszPathName[iLen-4], ".cfg") == 0)
				{
					if (SaveBank(pBankCtrl) < 0)
						iRet--;
				}
			}
		}
	}
	
	return iRet;
}

//描述:保存数据,后缀名不是.cfg的文件
int CDataManager::SaveData(bool fSaveAll)
{
	WORD i;
	int iLen;
	int iRet = 0;
	TBankCtrl* pBankCtrl;
	
	for (i=0; i<m_wSectNum; i++)
	{
		pBankCtrl = &m_pBank0Ctrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL)   //该BANK不需要被保存
		{
			if (!fSaveAll && pBankCtrl->wSaveInterv!=0)	//不是整体保存 && 本BANK数据按照单独的间隔进行保存
				continue;

			if ((iLen=strlen(pBankCtrl->pszPathName)) > 0)
			{
				if (strcmp(&pBankCtrl->pszPathName[iLen-4], ".cfg") != 0)
				{
					if (SaveBank(pBankCtrl) < 0)
						iRet--;
				}
			}
		}
	}

	for (i=1; i<m_wBankNum; i++)
	{
		pBankCtrl = &m_pBankCtrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL)   //该BANK不需要被保存
		{
			if (!fSaveAll && pBankCtrl->wSaveInterv!=0)	//不是整体保存 && 本BANK数据按照单独的间隔进行保存
				continue;

			if ((iLen=strlen(pBankCtrl->pszPathName)) > 0)
			{
				if (strcmp(&pBankCtrl->pszPathName[iLen-4], ".cfg") != 0)
				{
					if (SaveBank(pBankCtrl) < 0)
						iRet--;
				}
			}
		}
	}
	
	return iRet;
}

int CDataManager::SaveBank(TBankCtrl* pBankCtrl, int iFile)
{
	if (pBankCtrl->pItemDesc==NULL || pBankCtrl->pszPathName==NULL)   //该BANK不需要被保存
		return 0;

	int iRet = 0;
	WORD wStartFile = 0;
	if (iFile >= 0)
		wStartFile = (WORD )iFile;

	for (WORD i=wStartFile; i<pBankCtrl->wFileNum; i++)
	{
		//数据文件的修改标志,按照测量点
		WORD wByte = i >> 3; //除8
		BYTE bMask = 1 << (i & 0x07);
		if ((pBankCtrl->bModified[wByte] & bMask) == 0)
		{
			if (iFile >= 0)	//只保存一个文件
				break;
				
			continue;
		}

		DWORD dwIndexNum;
		char szPathName[128];
		char szTimeFileName[128];
		sprintf(szPathName, pBankCtrl->pszPathName, i);

		WaitSemaphore(pBankCtrl->semBankRW);
		
		if (WriteFile(szPathName,
						pBankCtrl->pbBankData+pBankCtrl->dwFileSize*i, 
						pBankCtrl->dwFileSize))
		{
			if (pBankCtrl->pszBakPathName!=NULL && i==0)	//备份文件的路径文件名，为空表示不备份，
			{												//备份文件不支持多测量点多文件的备份，只支持单文件，不支持时标的备份
				char szBakPathName[128];	//备份文件的名称
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				WriteFile(szBakPathName,
							pBankCtrl->pbBankData+pBankCtrl->dwFileSize*i, 
							pBankCtrl->dwFileSize);
			}

			DTRACE(DB_DB, ("CDataManager::SaveBank: save %s ok, dwFileSize=%ld, Click=%ld\n", 
				   			szPathName, pBankCtrl->dwFileSize, GetClick()));

			if (pBankCtrl->fUpdTime) //本BANK数据是否需要更新时间
			{
				if (pBankCtrl->wImgNum > 1) //多个镜像多个测量点
					dwIndexNum = pBankCtrl->dwIndexNum * pBankCtrl->wPnNum;
				else //if (pBankCtrl->wPnNum > 1) //多个测量点
					dwIndexNum = pBankCtrl->dwIndexNum;
				
				sprintf(szTimeFileName, "%s.tm", szPathName);
				if (WriteFile(szTimeFileName, 
								(BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i], 
								dwIndexNum*sizeof(DWORD)))
				{
					pBankCtrl->bModified[wByte] &= ~bMask;
				}
				else
				{
					iRet--;
				}
			}
			else
			{
				pBankCtrl->bModified[wByte] &= ~bMask;
			}
		}
		else
		{
			iRet--;
		}
		
		SignalSemaphore(pBankCtrl->semBankRW);
		
		if (iFile >= 0)	//只保存一个文件
			break;
	}

	pBankCtrl->dwSaveClick = GetClick();

	return iRet;
}

void CDataManager::DoTrigerSaveBank()
{
	WORD i, j;
	WORD wByte;
	BYTE bMask;

	SavePnMap(); //只要测量点映射发生了改变,就应该马上保存到文件系统中去,避免映射发生错乱

	if (!m_fTrigerSaveBank)
		return;
		
	WaitSemaphore(g_semDataRW);
	m_fTrigerSaveBank = false;
	memcpy(m_bTmpSectSaveFlg, m_bSectSaveFlg, sizeof(m_bSectSaveFlg));
	memcpy(m_bTmpBankSaveFlg, m_bBankSaveFlg, sizeof(m_bBankSaveFlg));
	
	memset(m_bSectSaveFlg, 0, sizeof(m_bSectSaveFlg));
	memset(m_bBankSaveFlg, 0, sizeof(m_bBankSaveFlg));
	SignalSemaphore(g_semDataRW);
	
	DTRACE(DB_DB, ("CDataManager::DoTrigerSaveBank: start at click %ld\n", GetClick()));
	
	TBankCtrl* pBankCtrl;
	
	for (i=0; i<m_wSectNum; i++)
	{
		pBankCtrl = &m_pBank0Ctrl[i];
		if (pBankCtrl->pItemDesc == NULL)
			continue;
		
		for (j=0; j<pBankCtrl->wFileNum; j++)
		{
			wByte = j >> 3; //除8
			bMask = 1 << (j & 0x07);
			if (m_bTmpSectSaveFlg[i][wByte] & bMask)
			{
				SaveBank(pBankCtrl, j);
			}
		}
	}

	for (i=1; i<m_wBankNum; i++)
	{
		pBankCtrl = &m_pBankCtrl[i];
		if (pBankCtrl->pItemDesc == NULL)
			continue;
		
		for (j=0; j<pBankCtrl->wFileNum; j++)
		{
			wByte = j >> 3; //除8
			bMask = 1 << (j & 0x07);
			if (m_bTmpBankSaveFlg[i][wByte] & bMask)
			{
				SaveBank(pBankCtrl, j);
			}
		}
	}
	
	DTRACE(DB_DB, ("CDataManager::DoTrigerSaveBank: done at click %ld\n", GetClick()));
}

//描述:按照自有间隔保存
void CDataManager::DoSelfIntervSave()
{
	WORD i;
	TBankCtrl* pBankCtrl;

	DWORD dwClick = GetClick();
	for (i=0; i<m_wSectNum; i++)
	{
		pBankCtrl = &m_pBank0Ctrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL && pBankCtrl->wSaveInterv!=0)   
		{														//本BANK数据按照自有的间隔进行保存
			if (pBankCtrl->wSaveInterv>m_pDbCtrl->wSaveInterv && dwClick<pBankCtrl->wSaveInterv*60)
			{	//本BANK的保存间隔比系统库整体保存间隔大 && 系统启动以来时间还没达到本BANK的保存间隔
				if (dwClick-pBankCtrl->dwSaveClick > m_pDbCtrl->wSaveInterv*60)	//先按照系统库整体间隔保存
					SaveBank(pBankCtrl);
			}
			else if (dwClick-pBankCtrl->dwSaveClick > pBankCtrl->wSaveInterv*60)
			{		//系统启动以来时间已经达到本BANK的保存间隔,则按照本BANK的间隔保存
				SaveBank(pBankCtrl);
			}
		}
	}

	for (i=1; i<m_wBankNum; i++)
	{
		pBankCtrl = &m_pBankCtrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL && pBankCtrl->wSaveInterv!=0)
		{														//本BANK数据按照自有的间隔进行保存
			if (pBankCtrl->wSaveInterv>m_pDbCtrl->wSaveInterv && dwClick<pBankCtrl->wSaveInterv*60)
			{	//本BANK的保存间隔比系统库整体保存间隔大 && 系统启动以来时间还没达到本BANK的保存间隔
				if (dwClick-pBankCtrl->dwSaveClick > m_pDbCtrl->wSaveInterv*60)	//先按照系统库整体间隔保存
					SaveBank(pBankCtrl);
			}
			else if (dwClick-pBankCtrl->dwSaveClick > pBankCtrl->wSaveInterv*60)
			{		//系统启动以来时间已经达到本BANK的保存间隔,则按照本BANK的间隔保存
				SaveBank(pBankCtrl);
			}
		}
	}
}

//描述:清除一个BANK的数据，并保存到文件系统中去
//参数:@wBank BANK号
//	   @wSect 如果是BANK0,表示BANK0内的段号
//	   @iFile 文件号,分别对应测量点号或者镜像号,小于0时表示全部文件
//备注:目前不支持指定清某个IMG的数据,因为IMG是动态生成的,要清就该SECT的所有镜像数据都清
void CDataManager::TrigerSaveBank(WORD wBank, WORD wSect, int iFile)
{
	WORD i;
	WORD wByte;
	BYTE bMask;
	int iPn;

	DTRACE(DB_DB, ("CDataManager::TrigerSaveBank: wBank=%d, wSect=%d, iFile=%ld\n", wBank, wSect, iFile));
	
	if (wBank>=m_wBankNum || wSect>=m_wSectNum)   //该BANK需要被保存
		return;

	TBankCtrl* pBankCtrl;
	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	if (pBankCtrl->pItemDesc == NULL) //空的BANK
		return;

	//如果该BANK使用测量点动态映射且按照测量点来分文件保存,则把文件号iFile调整为相应测量点的映射号
	if (iFile>=0	//指定保存某个文件
		&& pBankCtrl->bPnMapSch>0 && pBankCtrl->bPnMapSch<=m_wPnMapNum //方案号应该在1~m_wPnMapNum间
		&& pBankCtrl->wPnNum>1 && pBankCtrl->wImgNum==1)	//按照测量点来分文件保存
	{	
		iPn = SearchPnMap(pBankCtrl->bPnMapSch, (WORD )iFile);	//此时iFile指定的是测量点号
		if (iPn < 0)
			return;

		iFile = iPn;
	}

	if (iFile >= pBankCtrl->wFileNum)
		return;
		
	WaitSemaphore(g_semDataRW);
	
	if (wBank == BANK0)
	{
		if (iFile < 0)	//清除所有文件(每个测量点或镜像)
		{
			for (i=0; i<pBankCtrl->wFileNum; i++)
			{
				wByte = i >> 3; //除8
				bMask = 1 << (i & 0x07);
				m_bSectSaveFlg[wSect][wByte] |= bMask;
			}
		}
		else
		{
			wByte = (WORD )(iFile >> 3); //除8
			bMask = 1 << (iFile & 0x07);
			m_bSectSaveFlg[wSect][wByte] |= bMask;
		}	
	}
	else
	{
		if (iFile < 0)	//清除所有文件(每个测量点或镜像)
		{
			for (i=0; i<pBankCtrl->wFileNum; i++)
			{
				wByte = i >> 3; //除8
				bMask = 1 << (i & 0x07);
				m_bBankSaveFlg[wBank][wByte] |= bMask;
			}
		}
		else
		{
			wByte = (WORD )(iFile >> 3); //除8
			bMask = 1 << (iFile & 0x07);
			m_bBankSaveFlg[wBank][wByte] |= bMask;
		}	
	}	
	
	m_fTrigerSaveBank = true;
		
	SignalSemaphore(g_semDataRW);
}

//描述:保存测量点动态映射表
int CDataManager::SavePnMap()
{
	WORD iRet = 0;
	if (m_dwPnMapFileFlg != 0) //有测量点映射文件发生了修改
	{
		DWORD dwFlg = 1;
		char szPathName[128];
		
		WaitSemaphore(m_semPnMap);
		
		for (WORD i=0; i<m_wPnMapNum; i++, dwFlg=dwFlg<<1)
		{
			if (m_dwPnMapFileFlg & dwFlg)
			{
				m_dwPnMapFileFlg &= ~dwFlg; //在保存前先清标志,以便在保存的过程中可以反应新的变化
				//从文件系统把测量点号到存储号的映射表恢复到pwPnToMemMap
				sprintf(szPathName, "%sPNMAP%d.cfg", m_pDbCtrl->pszDbPath, i); 
				if (!WriteFile(szPathName, (BYTE* )m_pPnMapCtrl[i].pwPnToMemMap, m_pPnMapCtrl[i].dwFileSize))
				{
					DTRACE(DB_DB, ("CDataManager::SavePnMap: fail to save %s\n", szPathName));
					iRet--;
				}
			}
		}
		
		SignalSemaphore(m_semPnMap);
	}

	return iRet;
}


void CDataManager::TrigerSaveAll()
{
	m_fTrigerSaveAll = true;
}

void CDataManager::TrigerSavePara()
{
	m_fTrigerSavePara = true;
}


void CDataManager::DoSave()
{
	bool fTrigerSaveAll;
	DWORD dwClick = GetClick();
	if (dwClick > 10)  //在启动后10秒触发一次保存数据,主要避免在刚上电电源不稳定的时候保存数据
	{				   
		if (m_fTrigerSaveAll || dwClick-m_dwSaveClick>m_pDbCtrl->wSaveInterv*60) //保存间隔,单位分钟
		{	
			DTRACE(DB_DB, ("DoSave: @@@@@@@@@@@@ save start at click=%d.\r\n", dwClick));
			fTrigerSaveAll = m_fTrigerSaveAll;	//m_fTrigerSaveAll要清零,先临时保存
			m_fTrigerSaveAll = false;	//NOTICE:标志先清除再进行实际保存,避免连续配置参数的时候,后面的标志刚置上就被一起清掉,导致实际没保存
			m_fTrigerSavePara = false;
			Save(fTrigerSaveAll);
			m_dwSaveClick = dwClick;
			DTRACE(DB_DB, ("DoSave: @@@@@@@@@@@@ save done at click=%d.\r\n", GetClick()));
		}

		if (m_fTrigerSavePara)
		{
			DTRACE(DB_DB, ("DoSave: @@@@@@@@@@@@ save para start at click=%d.\r\n", dwClick));
			m_fTrigerSavePara = false; //NOTICE:标志先清除再进行实际保存,避免连续配置参数的时候,后面的标志刚置上就被一起清掉,导致实际没保存
			SavePara();
			DTRACE(DB_DB, ("DoSave: @@@@@@@@@@@@ save para done at click=%d.\r\n", GetClick()));
		}
	}
	
	DoTrigerSaveBank();
	DoSelfIntervSave();
}


//描述:装入1个文件的默认数据,
//		1.当描述表中存在多个测量点的描述,其实装入的是整个BANK的多个测量点的默认值,
//		  即装入的是该描述标所描述的多个测量点的默认数据;
//		2.当描述表中不存在多个测量点的描述时,则装入的是该描述标所描述的1个测量点的默认数据
//		  (可能是TBankCtrl中的多个测量点中的一个)
//参数:@pBankCtrl BANK控制结构
//	   @wFile 文件号,分别对应测量点号或者镜像号 
//	   @dwOffset 一个文件里的偏移,主要针对那些版本发生了改变的BANK,
//				后面又扩展了新的数据项,在装入文件内容的时候已经装载了
//				前面原有的部分,后面给新加的部分装入默认置
//				对于那些版本没有发生改变的BANK,把本参数置为0
void CDataManager::LoadOneFileDefault(TBankCtrl* pBankCtrl, WORD wFile, DWORD dwOffset)
{
	if (pBankCtrl->pbDefault == NULL)	//本BANK没有默认值,直接取0
	{
		memset(pBankCtrl->pbBankData+pBankCtrl->dwFileSize*wFile+dwOffset, 
				0, pBankCtrl->dwFileSize-dwOffset);
		return;
	}

	if (!pBankCtrl->fMutiPnInDesc)  //描述表中不存在多个测量点的描述
	{
		memcpy(pBankCtrl->pbBankData+pBankCtrl->dwFileSize*wFile+dwOffset, 
			   pBankCtrl->pbDefault+dwOffset,
			   pBankCtrl->dwFileSize-dwOffset);

		return;
	}

	TItemDesc* pItemDesc = pBankCtrl->pItemDesc;
	DWORD num = pBankCtrl->dwItemNum;
	BYTE* pbDst = pBankCtrl->pbBankData + pBankCtrl->dwFileSize*wFile;
	BYTE* pbDst0 = pbDst;
	BYTE* pbSrc = pBankCtrl->pbDefault;
	for (DWORD i=0; i<num; i++)
    {
		if (pItemDesc[i].bSelfItem)		 //本数据项能自成独立数据项
		{
			for (WORD j=0; j<pItemDesc[i].wPnNum; j++)
			{
				if ((DWORD )(pbDst-pbDst0) >= dwOffset)
					memcpy(pbDst, pbSrc, pItemDesc[i].wLen);
					
				pbDst += pItemDesc[i].wLen;
			}

			pbSrc += pItemDesc[i].wLen;
		}
	}
}


//描述:装入1个BANK的默认数据,
//参数:@pBankCtrl BANK控制结构
//	   @iFile 文件号,分别对应测量点号或者镜像号,小于0时表示装入整BANK默认数据 
//	   @dwOffset 一个文件里的偏移,主要针对那些版本发生了改变的BANK,
//				后面又扩展了新的数据项,在装入文件内容的时候已经装载了
//				前面原有的部分,后面给新加的部分装入默认置
//				对于那些版本没有发生改变的BANK,把本参数置为0
void CDataManager::LoadBankDefault(TBankCtrl* pBankCtrl, int iFile, DWORD dwOffset)
{
	if (iFile < 0)  //装入整个BANK的默认值
	{
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
			LoadOneFileDefault(pBankCtrl, i, dwOffset);
	}
	else
	{
		LoadOneFileDefault(pBankCtrl, iFile, dwOffset);
	}
}

//描述:初始化一个Bank数据库
bool CDataManager::InitBank(TBankCtrl* pBankCtrl)
{
	pBankCtrl->dwMemUsage = 0; 	//内存使用量,单位字节,包括数据和时标存储空间
	pBankCtrl->dwSaveClick = 0; //本BANK数据保存的时标

	if (pBankCtrl->pItemDesc == NULL)  //无效的数据库的组描述
		return true;	
	
	if (!InitItemDesc(pBankCtrl)) //初始化数据项描述表
		return false;

	if (pBankCtrl->pbBankData)
		delete []  pBankCtrl->pbBankData;

	pBankCtrl->pbBankData = NULL;
	if (pBankCtrl->wPnNum==0 && pBankCtrl->wImgNum==0)
	{	//本BANK的只作为数据项描述用,真正的数据访问要靠相应的读写函数
		DTRACE(DB_DB, ("CDataManager::InitBank: <%s> ---just for item desc--- init ok, dwItemNum=%ld, dwBankSize=%ld, wPnNum=%d, wImgNum=%d\n", 
				   		pBankCtrl->pszBankName,
				   		pBankCtrl->dwItemNum,
				   		pBankCtrl->dwBankSize,
				   		pBankCtrl->wPnNum, pBankCtrl->wImgNum));

		return true;
	}	

	//如果该BANK使用测量点动态映射,则pBankCtrl->wPnNum调整为映射方案中配置的实际支持的测量点数
	if (pBankCtrl->bPnMapSch>0 && pBankCtrl->bPnMapSch<=m_wPnMapNum) //方案号应该在1~m_wPnMapNum间
	{	
		pBankCtrl->wPnNum = m_pPnMapCtrl[pBankCtrl->bPnMapSch-1].wRealNum;
	}

	pBankCtrl->dwTotalSize = pBankCtrl->dwBankSize * pBankCtrl->wPnNum * pBankCtrl->wImgNum;
	pBankCtrl->pbBankData = new BYTE[pBankCtrl->dwTotalSize];
						  //如果有多个测量点,则只申请一个容纳所有测量点的大缓冲,
						  //pbBankData指向总的起始地址

	if (pBankCtrl->pbBankData == NULL)
	{
		DTRACE(1, ("CDataManager::InitBank : critical error : sys out of memory.\r\n"));
		return false;
	}

	pBankCtrl->dwMemUsage += pBankCtrl->dwTotalSize; //内存使用量,单位字节,包括数据和时标存储空间
	
	pBankCtrl->semBankRW = NewSemaphore(1);	//TODO:信号量是否创建成功

	if (pBankCtrl->wImgNum > 1)
	{
		pBankCtrl->wFileNum = pBankCtrl->wImgNum; //一共分成多少个文件
		pBankCtrl->dwFileSize = pBankCtrl->dwBankSize*pBankCtrl->wPnNum; //每个文件的大小
	}
	else
	{
		pBankCtrl->wFileNum = pBankCtrl->wPnNum;
		pBankCtrl->dwFileSize = pBankCtrl->dwBankSize;
	}

	if (pBankCtrl->wFileNum > BANK_FILE_MAX)
	{
		DTRACE(DB_DB, ("CDataManager::InitBank : bank<%s>'s file num=%d can't over %d\n", 
					   pBankCtrl->pszBankName, pBankCtrl->wFileNum, BANK_FILE_MAX));
		return false;
	}

	DWORD dwIndexNum = pBankCtrl->dwIndexNum;
	if (pBankCtrl->fUpdTime)
	{
		DWORD dwTmIdxNum = pBankCtrl->dwIndexNum*pBankCtrl->wPnNum*pBankCtrl->wImgNum;
		pBankCtrl->pdwUpdTime = new DWORD[dwTmIdxNum];
		if (pBankCtrl->pdwUpdTime == NULL)
		{
			DTRACE(1, ("CDataManager::InitBank : critical error : sys out of memory.\r\n"));
			return false;
		}
		
		pBankCtrl->dwMemUsage += dwTmIdxNum*sizeof(DWORD); //内存使用量,单位字节,包括数据和时标存储空间
		
		memset(pBankCtrl->pdwUpdTime, 0, sizeof(DWORD)*dwTmIdxNum);
		//TODO:更新时间由文件中读入
		
		if (pBankCtrl->wImgNum > 1) //多个镜像多个测量点
			dwIndexNum = pBankCtrl->dwIndexNum * pBankCtrl->wPnNum;
		//else //if (pBankCtrl->wPnNum > 1) //多个测量点
		//	dwIndexNum = pBankCtrl->dwIndexNum;
	}
	
	int len;
	BYTE bVer;
	DWORD dwVerLen, dwVerItemNum;
	BYTE* pbFileBuf;
	pBankCtrl->fOldFileExist = false;
	if (pBankCtrl->pszPathName != NULL)   //该BANK需要被保存
	{
		bool fLoadOk;
		char szPathName[128];
		char szTimeFileName[128];
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
		{
			fLoadOk = true;
			sprintf(szPathName, pBankCtrl->pszPathName, i);
			
			//初始化Bank3数据
			pbFileBuf = pBankCtrl->pbBankData + pBankCtrl->dwFileSize*i;
			len = readfile(szPathName, pbFileBuf, pBankCtrl->dwFileSize);
						//预留最大长度读取文件,文件长度不符合的时候不删除文件
						//在这里读到长度不符合的文件暂时不删除也没关系,
						//因为WriteFile()判断到文件长度不符合的时候会在写前先删除文件


			if (len<=0 && pBankCtrl->pszBakPathName!=NULL && i==0)	//备份文件的路径文件名，为空表示不备份，
			{														//备份文件不支持多测量点多文件的备份，只支持单文件，不支持时标的备份
				char szBakPathName[128];	//备份文件的名称
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				len = readfile(szBakPathName, pbFileBuf, pBankCtrl->dwFileSize);
				if (len > 0)
				{
					DTRACE(DB_DB, ("CDataManager::InitBank: use bak file %s\n", szBakPathName));
				}
			}

			if (len == (int )pBankCtrl->dwFileSize)	//文件长度刚好相等,版本没有发生改变
			{
				pBankCtrl->fOldFileExist = true;	 //旧版本文件存在,长度必须完全符合
				if (pBankCtrl->fUpdTime) //本BANK数据是否需要更新时间
				{
					sprintf(szTimeFileName, "%s.tm", szPathName);
					if (!ReadFile(szTimeFileName, 
								  (BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i], 
								  dwIndexNum*sizeof(DWORD)))
					{
						if (pBankCtrl->pszBakPathName!=NULL && i==0) //为了增加可靠性，备份文件可以不考虑时标，主要针对测量点0交采数据
						{
							memset((BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i], 0, dwIndexNum*sizeof(DWORD));
						}
						else
						{
							fLoadOk = false;
							DTRACE(DB_DB, ("CDataManager::InitBank: fail to read time file %s, clr all data\n", szTimeFileName, i));
						}
					}
				}
			}
			else if (len>0 && len<(int )pBankCtrl->dwFileSize && pBankCtrl->bVer!=0)
			{		//版本发生了改变	&& 本BANK支持版本控制
				bVer = pbFileBuf[0];
				memcpy(&dwVerLen, pbFileBuf+1, sizeof(DWORD));
				memcpy(&dwVerItemNum, pbFileBuf+5, sizeof(DWORD));

				if (bVer<pBankCtrl->bVer && (((DWORD )len==dwVerLen) || (dwVerLen==0)))//(DWORD )len==dwVerLen)
				{	//版本发生了递增 && (读出长度等于旧的长度||或者刚开始没有版本控制时，旧长度等于0)
					DTRACE(DB_DB, ("CDataManager::InitBank: <%s> version change, fix old data with new default, ver old=%d new=%d, file len=%ld dwVerLen=%ld\n", 
									pBankCtrl->pszBankName, 
									bVer, pBankCtrl->bVer, len, dwVerLen));
					LoadBankDefault(pBankCtrl, i, len);

					if (pBankCtrl->fUpdTime) //本BANK数据是否需要更新时间
					{
						sprintf(szTimeFileName, "%s.tm", szPathName);
						BYTE* pbUpdTime = (BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i];
						int iTimeLen = readfile(szTimeFileName, 
												pbUpdTime, 
												dwIndexNum*sizeof(DWORD)); //按预留最大长度读取文件
						if (iTimeLen==dwVerItemNum*sizeof(DWORD) 
							&& (DWORD )iTimeLen<dwIndexNum*sizeof(DWORD))	//旧的时间文件合法
						{
							memset(pbUpdTime+iTimeLen, 0, dwIndexNum*sizeof(DWORD)-iTimeLen);
										//新增部分的时间清0
						}
						else
						{
							fLoadOk = false;
							DTRACE(DB_DB, ("CDataManager::InitBank: fail to read time file %s, clr all data\n", szTimeFileName));
						}
					}
				}
				else
				{
					DTRACE(DB_DB, ("CDataManager::InitBank: <%s> version change, but verinfo mismatch, ver old=%d new=%d, file len=%ld dwVerLen=%ld\n", 
									pBankCtrl->pszBankName, 
									bVer, pBankCtrl->bVer, len, dwVerLen));
					fLoadOk = false;
				}
			}
			else
			{
				fLoadOk = false;
				if (pBankCtrl->wFileNum <= 8) //避免文件多打印过多
					DTRACE(DB_DB, ("CDataManager::InitBank: fail to read %s, use default\n", szPathName, i));
			}
			
			if (!fLoadOk)
			{
				LoadBankDefault(pBankCtrl, i, 0);
				if (pBankCtrl->fUpdTime)
					memset((BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i], 0, dwIndexNum*sizeof(DWORD));
			}

			if (pBankCtrl->bVer != 0) //本BANK支持版本控制,则把最新的版本信息更新到数据库
			{
				memset(pbFileBuf, 0, BN_VER_LEN);
				pbFileBuf[0] = pBankCtrl->bVer;	//版本
				memcpy(pbFileBuf+1, &pBankCtrl->dwFileSize, sizeof(DWORD));	//文件长度
				memcpy(pbFileBuf+5, &dwIndexNum, sizeof(DWORD));	//文件长度
			}
		} //for (i=0; i<pBankCtrl->wFileNum; i++)
	}
	else
	{
		LoadBankDefault(pBankCtrl, -1, 0); //-1 小于0时表示装入整BANK默认数据 

		//不需要保存文件的BANK不需要版本控制
	}

	memset(pBankCtrl->bModified, 0, sizeof(pBankCtrl->bModified));
	
	DTRACE(DB_DB, ("CDataManager::InitBank: <%s> init ok, dwItemNum=%ld, dwIndexNum=%ld, dwDefaultSize=%ld, dwBankSize=%ld, wPnNum=%d, wImgNum=%d, dwTotalSize=%ld, wFileNum=%d, dwFileSize=%ld, dwMemUsage=%ld\n", 
				   pBankCtrl->pszBankName,
				   pBankCtrl->dwItemNum,
				   pBankCtrl->dwIndexNum,
				   pBankCtrl->dwDefaultSize, pBankCtrl->dwBankSize,
				   pBankCtrl->wPnNum, pBankCtrl->wImgNum,
				   pBankCtrl->dwTotalSize, pBankCtrl->wFileNum, pBankCtrl->dwFileSize,
				   pBankCtrl->dwMemUsage));

	return true;
}

//描述:删除一个Bank所申请的资源
void CDataManager::DeleteBank(TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->pbBankData)	//本BANK数据库的数据,
	{	
		FreeSemaphore(pBankCtrl->semBankRW);	//BANK数据的读写保护

		delete [] pBankCtrl->pbBankData;
		pBankCtrl->pbBankData = NULL;	
	}

	if (pBankCtrl->pdwUpdTime)	  //本BANK数据的更新时间
	{	
		delete [] pBankCtrl->pdwUpdTime;
		pBankCtrl->pdwUpdTime = NULL;	
	}
}

//描述:删除一个Bank的文件
void CDataManager::DeleteBankFile(TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->pszPathName != NULL)   //该BANK需要被保存
	{
		char szPathName[128];
		char szTimeFileName[128];
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
		{
			sprintf(szPathName, pBankCtrl->pszPathName, i);
			DTRACE(DB_DB, ("CDataManager::DeleteBankFile: delete %s\r\n", szPathName));
			unlink(szPathName);

			if (pBankCtrl->pszBakPathName!=NULL && i==0)	//备份文件的路径文件名，为空表示不备份，
			{												//备份文件不支持多测量点多文件的备份，只支持单文件，不支持时标的备份
				char szBakPathName[128];	//备份文件的名称
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				unlink(szBakPathName);
			}

			if (pBankCtrl->fUpdTime) //本BANK数据是否需要更新时间
			{
				sprintf(szTimeFileName, "%s.tm", szPathName);
				DTRACE(DB_DB, ("CDataManager::DeleteBankFile: delete %s\r\n", szTimeFileName));
				unlink(szTimeFileName);
			}
		}
	}
}

//描述:清除一个BANK的数据，并保存到文件系统中去
//参数:@wBank BANK号
//	   @wSect 如果是BANK0,表示BANK0内的段号
//	   @iPn 测量点号,小于0时表示全部文件
//备注:目前支持清IMG中的某个测量点的数据,要到所有镜像中清除某个测量点的数据
bool CDataManager::ClearBankData(WORD wBank, WORD wSect, int iPn)
{
	char szPathName[128];
	char szTimeFileName[128];
	if (wBank>=m_wBankNum || wSect>=m_wSectNum)   //该BANK需要被保存
		return false;

	TBankCtrl* pBankCtrl;
	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	if (pBankCtrl->pItemDesc == NULL) //空的BANK
		return true;

	if (iPn >= pBankCtrl->wPnNum)
		return false;
		
	WaitSemaphore(pBankCtrl->semBankRW);

	if (iPn < 0)	//清除所有文件(每个测量点或镜像)
	{
		LoadBankDefault(pBankCtrl, -1, 0); //-1 小于0时表示装入整BANK默认数据   //memset(pBankCtrl->pbBankData, 0, pBankCtrl->dwTotalSize);

		if (pBankCtrl->fUpdTime)
			memset(pBankCtrl->pdwUpdTime, 0, pBankCtrl->dwIndexNum*pBankCtrl->wPnNum*pBankCtrl->wImgNum*sizeof(DWORD));
	
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
		{
			//删除数据文件
			sprintf(szPathName, pBankCtrl->pszPathName, i);
			unlink(szPathName);
			
			if (pBankCtrl->pszBakPathName!=NULL && i==0)	//备份文件的路径文件名，为空表示不备份，
			{												//备份文件不支持多测量点多文件的备份，只支持单文件，不支持时标的备份
				char szBakPathName[128];	//备份文件的名称
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				unlink(szBakPathName);
			}

			//删除时标文件
			if (pBankCtrl->fUpdTime) //本BANK数据是否需要更新时间
			{	
				sprintf(szTimeFileName, "%s.tm", szPathName);
				unlink(szTimeFileName);
			}
			
			pBankCtrl->bModified[i/8] &= ~(1<<(i%8));
		}
	}
	else	//清除某个文件(或某个测量点)
	{
		DWORD dwIndexNum = pBankCtrl->dwIndexNum;
		if (pBankCtrl->wImgNum > 1) //多个镜像多个测量点
		{	
			//NOTICE:
			//到镜像中清除某个测量点的数据及时标的时候,保持其它测量点在系统库内存中的数据不变,
			//但是数据及时标保存的文件不分测量点,全部清除,这样主要是为了简化操作,避免过长的保存时间
			//如果在清理完后不复位,则系统库在内存中的数据是正确的,即使程序在清完后复位
			//数据产生丢失,但不会产生错误的数据
			
			for (DWORD dwImg=0; dwImg<pBankCtrl->wImgNum; dwImg++) //到每个镜像中删除每个测量点的数据及时标
			{
				//删除数据
				BYTE* pbPnAddr = pBankCtrl->pbBankData + pBankCtrl->dwBankSize*(pBankCtrl->wPnNum*dwImg + (DWORD )iPn);
				memset(pbPnAddr, 0, pBankCtrl->dwBankSize);
				sprintf(szPathName, pBankCtrl->pszPathName, dwImg);	
				unlink(szPathName);	//文件不分测量点,全部清除

				//删除时标
				DWORD dwTimeIndex = dwIndexNum * (pBankCtrl->wPnNum*dwImg + (DWORD )iPn);
				if (pBankCtrl->fUpdTime)
				{
					memset(pBankCtrl->pdwUpdTime+dwTimeIndex, 0, dwIndexNum*sizeof(DWORD));
					sprintf(szTimeFileName, "%s.tm", szPathName);
					unlink(szTimeFileName);	//文件不分测量点,全部清除
				}

				pBankCtrl->bModified[iPn/8] |= (1<<(iPn%8));	//下轮自动保存的时候进行写文件
			}
		}
		else //以测量点来分文件
		{
			LoadBankDefault(pBankCtrl, iPn, 0); //-1 小于0时表示装入整BANK默认数据   //memset(pBankCtrl->pbBankData, 0, pBankCtrl->dwTotalSize);

			if (pBankCtrl->fUpdTime)
				memset(pBankCtrl->pdwUpdTime+dwIndexNum*(DWORD )iPn, 0, dwIndexNum*sizeof(DWORD));
			
			//删除数据文件
			sprintf(szPathName, pBankCtrl->pszPathName, iPn);	
			unlink(szPathName);
			if (pBankCtrl->pszBakPathName!=NULL && iPn==0)	//备份文件的路径文件名，为空表示不备份，
			{												//备份文件不支持多测量点多文件的备份，只支持单文件，不支持时标的备份
				char szBakPathName[128];	//备份文件的名称
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, iPn);
				unlink(szBakPathName);
			}
			
						
			//删除时标文件
			if (pBankCtrl->fUpdTime) //本BANK数据是否需要更新时间
			{	
				sprintf(szTimeFileName, "%s.tm", szPathName);
				unlink(szTimeFileName);
			}
	
			pBankCtrl->bModified[iPn/8] &= ~(1<<(iPn%8));
		}
	}
	
	SignalSemaphore(pBankCtrl->semBankRW);

	//if (pBankCtrl->pszPathName != NULL)
	//	TrigerSave();

	return true;
}


//描述:清除指定BANK/SECT,测量点数量配置为wPnNum的,指定测量点的数据
//备注:测量点数据的长度不能超过256个字节
bool CDataManager::ClrPnData(WORD wBank, WORD wSect, WORD wPnNum, WORD wPn)
{
	BYTE bBuf[256];
	if (wBank>=m_wBankNum || wSect>=m_wSectNum)
		return false;

	TBankCtrl* pBankCtrl;
	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	if (pBankCtrl->pItemDesc == NULL) //空的BANK
		return true;

	if (pBankCtrl->wPnNum > 1)	//本函数不支持按整个BANK配置测量点数量的BANK
		return false;
	
	memset(bBuf, 0, sizeof(bBuf));	//目前假定测量点数据都不会太长

	for (DWORD i=0; i<pBankCtrl->dwItemNum; i++)
	{
		if (pBankCtrl->pItemDesc[i].wPnNum == wPnNum)	//测量点数量配置为wPnNum
		{
			::WriteItemEx(wBank, wPn, pBankCtrl->pItemDesc[i].wID, bBuf, (DWORD )0);	//清数据清时间
		}
	}

	return true;
}

//描述:初始化系统的TPnMapCtrl结构,并从文件系统把测量点号到存储号的映射表恢复到pwPnToMemMap
bool CDataManager::InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum)
{
	WORD i, j;
	WORD wMapNum;	//已经映射的个数
	WORD wMN;		//存储号
	char szPathName[128];
	char szHeader[160];
	for (i=0; i<wNum; i++)
	{
		//初始化TPnMapCtrl结构
		pPnMapCtrl[i].dwFileSize = (pPnMapCtrl[i].wRealNum*2 + 2)*sizeof(WORD);	//映射保存的文件大小,
																	//前面两个WORD用来保存控制信息,其中第一个WORD是已经映射的个数,第二个保留
		pPnMapCtrl[i].wAllocSize = (pPnMapCtrl[i].wRealNum+7)/8;	//存储空间分配表的大小

		pPnMapCtrl[i].pwPnToMemMap = new WORD[pPnMapCtrl[i].dwFileSize]; //测量点号到存储号的映射表(需要保存到文件系统)
		pPnMapCtrl[i].pbAllocTab = new BYTE[pPnMapCtrl[i].wAllocSize];	 //存储空间分配表(不保存到文件系统,动态更新)
		if (pPnMapCtrl[i].pwPnToMemMap==NULL || pPnMapCtrl[i].pbAllocTab==NULL)
		{
			DTRACE(DB_DB, ("CDataManager::InitBank : critical error : sys out of memory.\r\n"));
			return false;
		}

		memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
		memset(pPnMapCtrl[i].pbAllocTab, 0, pPnMapCtrl[i].wAllocSize);

		//从文件系统把测量点号到存储号的映射表恢复到pwPnToMemMap
		sprintf(szPathName, "%sPNMAP%d.cfg", m_pDbCtrl->pszDbPath, i);
		if (!ReadFile(szPathName, (BYTE* )pPnMapCtrl[i].pwPnToMemMap, pPnMapCtrl[i].dwFileSize))
		{
			DTRACE(DB_DB, ("CDataManager::InitPnMap: fail to read %s\n", szPathName));
			memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);

			if (pPnMapCtrl[i].fGenerateMapWhenNoneExist)	//当没有映射表的时候，自动生成一一对应的映射表，主要是应对版本升级
			{
				wMapNum = pPnMapCtrl[i].pwPnToMemMap[0] = pPnMapCtrl[i].wRealNum; //已经映射的个数
				for (j=0; j<wMapNum; j++)
				{
					wMN = pPnMapCtrl[i].pwPnToMemMap[2+j*2+1] = j; //存储号 wMN<pPnMapCtrl[i].wRealNum
					pPnMapCtrl[i].pwPnToMemMap[2+j*2] = j;		//测量点号<pPnMapCtrl[i].wMaxPn
					pPnMapCtrl[i].pbAllocTab[wMN/8] |= 1<<(wMN%8);	//表示该存储空间已经分配
				}
			}
		}
		else
		{
			sprintf(szHeader, "%s :", szPathName);
			TraceBuf(DB_DB, szHeader, (BYTE* )pPnMapCtrl[i].pwPnToMemMap, pPnMapCtrl[i].dwFileSize);

			//根据测量点号到存储号的映射表,初始化存储空间分配表
			wMapNum = pPnMapCtrl[i].pwPnToMemMap[0]; //已经映射的个数
			if (wMapNum > pPnMapCtrl[i].wRealNum)	//映射文件信息有误
			{
				DTRACE(DB_DB, ("CDataManager::InitPnMap: err! wMapNum(%d)>wRealNum(%d) in %s\n", 
							   wMapNum, pPnMapCtrl[i].wRealNum, szPathName));
				memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
			}
			else
			{
				for (j=0; j<wMapNum; j++)
				{
					wMN = pPnMapCtrl[i].pwPnToMemMap[2+j*2+1]; //存储号
					if (pPnMapCtrl[i].pwPnToMemMap[2+j*2]<pPnMapCtrl[i].wMaxPn 	//测量点号
						&& wMN<pPnMapCtrl[i].wRealNum) 	//存储号
					{
						pPnMapCtrl[i].pbAllocTab[wMN/8] |= 1<<(wMN%8);	//表示该存储空间已经分配
					}
					else	//映射文件信息有误
					{
						DTRACE(DB_DB, ("CDataManager::InitPnMap: err! wMN=%d, Pn=%d, wMaxPn=%d, wRealNum=%d in %s\n", 
									   wMN, pPnMapCtrl[i].pwPnToMemMap[2+j*2], pPnMapCtrl[i].wMaxPn,
									   pPnMapCtrl[i].wRealNum, szPathName));

						memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
						memset(pPnMapCtrl[i].pbAllocTab, 0, pPnMapCtrl[i].wAllocSize);
						break;
					}
				}
			}
		}
	}

	return true;
}

//描述:数据库的初始化
//参数:@pDbCtrl 外界对数据库进行参数配置的数据库控制结构
//返回:如果成功则返回true,否则返回false
bool CDataManager::Init(TDbCtrl* pDbCtrl)
{
	m_pDbCtrl = pDbCtrl; //外界对数据库进行参数配置的数据库控制结构

	//为了访问方便,参数m_pDbCtrl中的部分变量拷贝出来直接使用
	m_wSectNum = m_pDbCtrl->wSectNum;		//BANK0中的SECT数目
	m_pBank0Ctrl = m_pDbCtrl->pBank0Ctrl;
	m_wBankNum = m_pDbCtrl->wBankNum;		//支持的BANK数目
	m_pBankCtrl = m_pDbCtrl->pBankCtrl;
	m_iSectImg = m_pDbCtrl->iSectImg;		//485抄表数据镜像段,如果没有则配成-1
	m_wImgNum = m_pDbCtrl->wImgNum;			//485抄表数据镜像个数
	m_wPnMapNum = m_pDbCtrl->wPnMapNum;  	//支持的映射方案数目,整个数据库不支持测量点动态映射则设为0
	m_pPnMapCtrl = m_pDbCtrl->pPnMapCtrl; 	//整个数据库不支持测量点动态映射则设为NULL
	m_pDbUpgCtrl = m_pDbCtrl->pDbUpgCtrl;

	if (m_wSectNum>SECT_MAX || m_wBankNum>BANK_MAX || m_wPnMapNum>PNMAP_MAX)
	{
		DTRACE(DB_DB, ("CDataManager::Init: the following var over max, wSectNum=%d(%d), wBankNum=%d(%d), m_wPnMapNum=%d(%d)\r\n",
					   m_wSectNum, SECT_MAX, 
					   m_wBankNum, BANK_MAX,
					   m_wPnMapNum, PNMAP_MAX));
		return false;
	}
	
	if (m_pDbCtrl->wSaveInterv == 0) //保存间隔,单位分钟
		m_pDbCtrl->wSaveInterv = 15;	

	m_semPnMap = NewSemaphore(1);

	WORD i;
	m_fTrigerSaveBank = false;
	memset(m_bSectSaveFlg, 0, sizeof(m_bSectSaveFlg));
	memset(m_bBankSaveFlg, 0, sizeof(m_bBankSaveFlg));
	m_dwPnMapFileFlg = 0;

	m_fDbUpg = InitUpgrade(m_pDbUpgCtrl);

	m_dwMemUsage = 0;	  //内存使用量,单位字节,包括数据和时标存储空间
	
	for (i=0; i<m_wSectNum; i++) 
	{
		if (InitBank(&m_pBank0Ctrl[i]) == false)
			return false;
		
		m_dwMemUsage += m_pBank0Ctrl[i].dwMemUsage;
	}

	for (i=0; i<m_wBankNum; i++)
	{
		if (InitBank(&m_pBankCtrl[i]) == false)
			return false;
	
		m_dwMemUsage += m_pBankCtrl[i].dwMemUsage;
	}
	
	if (m_wImgNum > 0)
	{	
		m_pImgCtrl = new TImgCtrl[m_wImgNum];
		if (m_pImgCtrl == NULL)
			return false;

		memset(m_pImgCtrl, 0, sizeof(TImgCtrl)*m_wImgNum);
	}

	if (!InitPnMap(m_pPnMapCtrl, m_wPnMapNum))
		return false;

	m_pbMeterPnMask = new BYTE[m_pDbCtrl->wPnMaskSize];
	if (m_pbMeterPnMask == NULL)
		return false;

	memset(m_pbMeterPnMask, 0, sizeof(m_pbMeterPnMask));

	DWORD dwTime = GetCurTime();
	TimeAdjBackward(dwTime); //把需要保存时间的数据项的时间全部校验一遍

	if (m_fDbUpg)
	{
		DoUpgrade(m_pDbUpgCtrl);
		m_fDbUpg = false;
	}

	m_dwSaveClick = 0;
	m_fTrigerSaveAll = false;
	m_fTrigerSavePara = false;

	SetEmptyTime(&g_tmAccessDenied);
	DTRACE(DB_CRITICAL, ("CDataManager::Init: "VER_STR" init ok, MemUsage=%ld.\r\n", m_dwMemUsage));
	return true;
}


//描述:初始化数据库版本升级
bool CDataManager::InitUpgrade(TDbUpgCtrl* pDbUpgCtrl)
{
	if (pDbUpgCtrl == NULL)
		return false;

	WORD i;
	char szLog[UPG_LOG_LEN];
	char szFileBuf[UPG_LOG_LEN];
	char szPathName[128];

	//比较旧的升级记录,看当前版本有没升级过,有升级过就不升了
	sprintf(szPathName, "%sUpgLog.txt", m_pDbCtrl->pszDbPath);
	int len = readfile(szPathName, (BYTE* )szFileBuf, UPG_LOG_LEN); //预留最大长度读取文件,文件长度不符合的时候不删除文件
	if (len > 0)
	{
		szFileBuf[UPG_LOG_LEN-1] = '\0';
		sprintf(szLog, "DbUpgVer%d", pDbUpgCtrl->bSchVer);
		if (strcmp(szLog, szFileBuf) == 0)	//当前版本升级已经做过
		{
			DTRACE(DB_CRITICAL, ("CDataManager::InitUpgrade: upgrade undo due to %s already exist!\r\n", szLog));
			return false;
		}
	}

	for (i=0; i<pDbUpgCtrl->wBankNum; i++)
	{
		if (InitBank(&pDbUpgCtrl->pBankCtrl[i]) == false)
			break;

		if (!pDbUpgCtrl->pBankCtrl[i].fOldFileExist && 	 //旧版本文件不存在
			pDbUpgCtrl->pBankCtrl[i].pbBankData!=NULL)	//且分配了资源
		{
			DeleteBank(&pDbUpgCtrl->pBankCtrl[i]);
		}


		//备注:
		//如果全部BANK都没找到旧文件或没分配到资源,也返回true,让DoUpgrade()执行一遍,
		//把升级日志记下,下回就不用了再执行升级流程了
	}

	if (i < pDbUpgCtrl->wBankNum)	//没完全初始化正确
	{
		for (i=0; i<pDbUpgCtrl->wBankNum; i++)
		{
			if (pDbUpgCtrl->pBankCtrl[i].pbBankData != NULL)	//已经分配资源
				DeleteBank(&pDbUpgCtrl->pBankCtrl[i]);
		}

		DTRACE(DB_CRITICAL, ("CDataManager::InitUpgrade: fail to init bank.\r\n"));
		return false;
	}
	else
	{
		return true;
	}
}

void CDataManager::DoUpgrade(TDbUpgCtrl* pDbUpgCtrl)
{
	DWORD i;
	if (pDbUpgCtrl == NULL)
		return;

	//旧版本ID导到新版本ID
	DTRACE(DB_DB, ("CDataManager::DoUpgrade: ID->ID upgrade!\r\n"));
	TIdUpgMap* pIdUpgMap = pDbUpgCtrl->pIdUpgMap;	//ID切换映射表
	for (i=0; i<pDbUpgCtrl->dwIdNum; i++,pIdUpgMap++)
	{
		if (pIdUpgMap->pfnUpgFun != NULL)
		{	
			pIdUpgMap->pfnUpgFun(pIdUpgMap->wFrmBn, pIdUpgMap->wFrmId, 
								 pIdUpgMap->wToBn, pIdUpgMap->wToId,
								 GetItemPnNum(pIdUpgMap->wToBn, pIdUpgMap->wToId));
		}
		else
		{
			DefaultUpgFun(pIdUpgMap->wFrmBn, pIdUpgMap->wFrmId, 
						  pIdUpgMap->wToBn, pIdUpgMap->wToId,
						  GetItemPnNum(pIdUpgMap->wToBn, pIdUpgMap->wToId));
		}
	}

	//删除旧版本文件
	if (pDbUpgCtrl->fDelFile)
	{
		DTRACE(DB_DB, ("CDataManager::DoUpgrade: del files...\r\n"));
		for (i=0; i<pDbUpgCtrl->wBankNum; i++)
		{
			if (pDbUpgCtrl->pBankCtrl[i].pbBankData)	//资源还没释放
			{
				DeleteBankFile(&pDbUpgCtrl->pBankCtrl[i]);
			}
		}
	}

	//保存新版本文件
	DTRACE(DB_DB, ("CDataManager::DoUpgrade: save files...\r\n"));
	Save();

	//释放旧版本资源
	DTRACE(DB_DB, ("CDataManager::DoUpgrade: free resources...\r\n"));
	for (i=0; i<pDbUpgCtrl->wBankNum; i++)
	{
		if (pDbUpgCtrl->pBankCtrl[i].pbBankData)	//资源还没释放
		{
			DeleteBank(&pDbUpgCtrl->pBankCtrl[i]);
		}
	}

	//保存升级日志
	char szLog[UPG_LOG_LEN];
	char szPathName[128];
	DTRACE(DB_DB, ("CDataManager::DoUpgrade: save log...\r\n"));
	sprintf(szPathName, "%sUpgLog.txt", m_pDbCtrl->pszDbPath);

	memset(szLog, 0, sizeof(szLog));
	sprintf(szLog, "DbUpgVer%d", pDbUpgCtrl->bSchVer);
	WriteFile(szPathName, (BYTE* )szLog, UPG_LOG_LEN);

	//复位终端
	if (pDbUpgCtrl->fRstCPU)
	{
		DTRACE(DB_DB, ("CDataManager::DoUpgrade: reset cpu...\r\n"));
		Sleep(3000);
		ResetApp();
	}
}

//描述:新创建一个镜像,由抄表管理器在每分钟向抄表线程提交抄表需求的时候,收集所提交数据的起始时间和间隔,
//	   并为它们创建镜像.数据库可能已经存在该数据项的镜像,比如日月切换的时候可能滞后1~2分钟提交
//     要抄0点数据,但0点镜像可能在00:00已经创建.如果存在不同的起始时间,则必须为它们创建不同的镜像,
//	   起始时间相同的,创建一个镜像就行了,间隔取最大值
//参数:@dwStartTime 镜像的起始时间,单位S
//	   @wInterval 镜像的间隔,单位分
void CDataManager::NewImg(DWORD dwStartTime, WORD wInterval)
{
	if (m_wImgNum==0 || m_iSectImg<0)
		return;

	WORD i;
	int iEmptyImg = -1;
	WORD wOldImg = 0;
	DWORD dwEndTime = 0;

	WaitSemaphore(m_pBank0Ctrl[m_iSectImg].semBankRW);

	//找出结束时间最前的一个
	for (i=0; i<m_wImgNum; i++)
	{
		if (m_pImgCtrl[i].dwStartTime == dwStartTime) //找到了时间相同的镜像
			break;
		
		if (iEmptyImg < 0)  //只有在没找到了空镜像的情况下,才找结束时间最前的镜像
		{
			if (m_pImgCtrl[i].dwStartTime == 0) //找到了空的镜像
			{	
				iEmptyImg = i;
			}
			else if (dwEndTime==0 || m_pImgCtrl[i].dwEndTime<dwEndTime)
			{
				wOldImg = i;
				dwEndTime = m_pImgCtrl[i].dwEndTime;
			}
		}
	}
	
	//算新镜像的超时时间
	dwEndTime = (wInterval>>1) * 60;	//镜像的结束时间,单位S,取最大超时为1/2间隔,
	if (dwEndTime > IMG_MAX_TIMEOUTS)   //大于5分钟的取5分钟
		dwEndTime = IMG_MAX_TIMEOUTS;
	else if (dwEndTime == 0)			//最小也要取1分钟
		dwEndTime = 60;

	dwEndTime += dwStartTime + wInterval*60;

	if (i < m_wImgNum)  //找到了时间相同的镜像
	{
		if (dwEndTime > m_pImgCtrl[i].dwEndTime)
			m_pImgCtrl[i].dwEndTime = dwEndTime;

		DTRACE(DB_DB, ("CDataManager::NewImg: get same img for %ld~%ldS\n", dwStartTime, m_pImgCtrl[i].dwEndTime));
	}
	else if (iEmptyImg >= 0) //找到了空的镜像
	{
		m_pImgCtrl[iEmptyImg].dwStartTime = dwStartTime;
		m_pImgCtrl[iEmptyImg].dwEndTime = dwEndTime;

		DTRACE(DB_DB, ("CDataManager::NewImg: get empty img for %ld~%ldS\n", dwStartTime, dwEndTime));
	}
	else  //使用结束时间最前的镜像
	{
		DTRACE(DB_DB, ("CDataManager::NewImg: replace %ld~%ldS' img with %ld~%ldS\n", 
						m_pImgCtrl[wOldImg].dwStartTime, m_pImgCtrl[wOldImg].dwEndTime,
						dwStartTime, dwEndTime));

		m_pImgCtrl[wOldImg].dwStartTime = dwStartTime;
		m_pImgCtrl[wOldImg].dwEndTime = dwEndTime;
	}

	SignalSemaphore(m_pBank0Ctrl[m_iSectImg].semBankRW);
}

//描述:当时间往前调整到dwTime(秒),数据库相应作出的调整
void CDataManager::TimeAdjBackward(DWORD dwTime)
{
	DWORD i;
	TBankCtrl* pBankCtrl;

	if (m_wImgNum > 0)
	{
		WaitSemaphore(m_pBank0Ctrl[m_iSectImg].semBankRW);
		
		//调整时刻后的镜像都删除
		for (i=0; i<m_wImgNum; i++)
		{
			if (m_pImgCtrl[i].dwStartTime >= dwTime) 
			{
				DTRACE(DB_DB, ("CDataManager::TimeAdjBackward: delet img %ld~%ldS at new time %ldS\n", 
							   m_pImgCtrl[i].dwStartTime, m_pImgCtrl[i].dwEndTime, dwTime));
		
				m_pImgCtrl[i].dwStartTime = 0;
				m_pImgCtrl[i].dwEndTime = 0;
				
				memset(&m_pBank0Ctrl[m_iSectImg].pdwUpdTime[m_pBank0Ctrl[m_iSectImg].dwItemNum*m_pBank0Ctrl[m_iSectImg].wPnNum*i], 
					   0, m_pBank0Ctrl[m_iSectImg].dwItemNum*m_pBank0Ctrl[m_iSectImg].wPnNum); 
			}
		}
		
		SignalSemaphore(m_pBank0Ctrl[m_iSectImg].semBankRW);
	}

	//数据项时标的调整
	for (WORD wSect=0; wSect<m_wSectNum; wSect++) //终端参数和
	{
		if (m_pBank0Ctrl[wSect].fUpdTime && m_pBank0Ctrl[wSect].wImgNum<=1)
		{ //需要更新时间				&& 不是镜像数据
			//WaitSemaphore(m_pBank0Ctrl[wSect].semBankRW); 
			//由于pBankCtrl->pdwUpdTime[i] = 0;是单指令操作,所以不用信号量保护
			 
			pBankCtrl = &m_pBank0Ctrl[wSect];
			DWORD dwTimeNum = pBankCtrl->dwIndexNum* pBankCtrl->wPnNum * pBankCtrl->wImgNum;
			for (i=0; i<dwTimeNum; i++)
			{
				if (pBankCtrl->pdwUpdTime[i] >= dwTime)
				{
					pBankCtrl->pdwUpdTime[i] = 0;
				}
			}

			//SignalSemaphore(m_pBank0Ctrl[wSect].semBankRW);
		}
	}

	for (WORD wBank=1; wBank<m_wBankNum; wBank++) //终端参数和
	{
		if (m_pBankCtrl[wBank].fUpdTime && m_pBankCtrl[wBank].wImgNum<=1)
		{ //需要更新时间				&& 不是镜像数据
			//WaitSemaphore(m_pBankCtrl[wBank].semBankRW);
			//由于pBankCtrl->pdwUpdTime[i] = 0;是单指令操作,所以不用信号量保护
			 
			pBankCtrl = &m_pBankCtrl[wBank];
			DWORD dwTimeNum = pBankCtrl->dwIndexNum* pBankCtrl->wPnNum * pBankCtrl->wImgNum;
			for (i=0; i<dwTimeNum; i++)
			{
				if (pBankCtrl->pdwUpdTime[i] >= dwTime)
				{
					pBankCtrl->pdwUpdTime[i] = 0;
				}
			}

			//SignalSemaphore(m_pBankCtrl[wBank].semBankRW);
		}
	}
}

//描述:取数据项的地址,但要注意不要直接用该地址访问数据,而是要使用数据库提供的函数ReadItem()和WriteItem()
TDataItem CDataManager::GetItem(WORD wPoint, WORD wID)
{
	TDataItem di;
	memset(&di, 0, sizeof(di));
	
	/*TItemDesc* pItemDesc = BinarySearchItem(g_TermnParaDesc, sizeof(g_TermnParaDesc)/sizeof(TItemDesc), wID);
	if (pItemDesc != NULL)
	{
		di.pbAddr = m_pbTermnPara + pItemDesc->wOffset;
		di.wLen = pItemDesc->wLen;
		di.pfModified = &m_fParaModified;
		return di;
	}*/

	return di;
}

//描述:测量点是否需要镜像
bool CDataManager::IsMeterPn(WORD wPn)
{
	WORD wByte = wPn >> 3; //除8
	BYTE bMask = 1 << (wPn & 0x07);
	
	if (wByte >= m_pDbCtrl->wPnMaskSize)
	    return false;
	
	if (m_pbMeterPnMask[wByte] & bMask)
		return true;
	else
		return false;
}

//描述:设置镜像测量点屏蔽位
void CDataManager::SetMeterPnMask(BYTE* pbMeterPnMask)
{
	memcpy(m_pbMeterPnMask, pbMeterPnMask, m_pDbCtrl->wPnMaskSize);
}


bool CDataManager::IsImgItem(WORD wBank, WORD wPn, WORD wID)
{
	if (wBank == BANK0)
	{
		int iIndex;
		if (m_wImgNum>0 && m_iSectImg>=0 && IsMeterPn(wPn) && 
			(iIndex=BinarySearchIndex(m_pBank0Ctrl[m_iSectImg].pItemDesc, m_pBank0Ctrl[m_iSectImg].dwItemNum, wID))>=0)
		{
			return true;
		}
	}

	return false;
}

//备注:$关于什么时候需要镜像
//		1.主要是针对抄表的测量点,如果同时存在不同时间间隔的抄表任务,比如15分钟
//		  和60分钟的,如果不使用镜像,则导致60分钟任务当中,和15分钟共有的数据项
//		  不断地被优先级高的15分钟任务刷新,导致60分钟数据的一致性不太好;
//		  如果使用了镜像,比如两个任务都同时要采901f,则对于60分钟的任务来说,
//		  它采集901f的优先级高了,保证了901f的采集都在每个小时的前15分钟内采集
//		2.对于交采/脉冲测量点,普通任务应该按照测量点分开,这样交采和脉冲测量点
//		  的采集不至于收到抄表测量点的影响,可以做得很准时,所以就没有必要使用镜像
//		3.对于同时存在抄表测量点和交采/脉冲测量点的计算任务,比如总加组的计算
//		  目前的并不要求参与计算的量都是在同一时刻采集到的,反而要求电表的量
//		  可以在一个间隔内保持不变,交采/脉冲使用最新的量不断地刷新总加功率,
//		  所以抄表测量点使用镜像,而交采/脉冲测量点不使用镜像,对于目前的使用
//		  情况来说是适合的
int CDataManager::ReadItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess& rItemAcess, DWORD dwStartTime, DWORD dwEndTime)
{
	int iRet;

	if (wBank == BANK0)
	{
		int iIndex;
		if (m_wImgNum>0 && m_iSectImg>=0 && dwStartTime!=0 && IsMeterPn(wPn) 
			&& (iIndex=BinarySearchIndex(m_pBank0Ctrl[m_iSectImg].pItemDesc, m_pBank0Ctrl[m_iSectImg].dwItemNum, wID))>=0)
		{	//带时刻的读且数据项是镜像中的数据项
			
			//NOTE:什么数据项需要从镜像中读
			//1.交采和脉冲数据不属于镜像测量点,IsMeterPn()返回false,所有数据都到当前库中读取
			//2.交采和脉冲的当前数据在写的时候不带时标,在WriteItemEx()中,写入到当前库
			//3.交采和脉冲的数据跟电表的数据一样都提交查询需求,但都都返回时间符合
			//4.直抄的数据都通过当前库,不通过镜像,为了避免到镜像中查找,
			//  查询的时标都要避免整分,即不传递一个按照分钟规整的查询时间

			//如果时间是应用提交需求的某个规整时刻,则一定要到到镜像中取
			//这个应用肯定是个按间隔执行的任务,而不是直抄
			for (WORD wImg=0; wImg<m_wImgNum; wImg++)
			{
				if (dwStartTime == m_pImgCtrl[wImg].dwStartTime) //有该时刻的镜像
				{		
					return ::ReadItem(wImg, wPn, wID, rItemAcess, dwStartTime, dwEndTime, &m_pBank0Ctrl[m_iSectImg]);
				}
			}
			
			//如果数据项是镜像中的数据项,但不存在该时刻的镜像,一般是时刻没规整
			//则应该是直接抄读,去当前库中找
			//直抄的相应函数会保证下来的查询时间dwTime不按分钟规整,
			//如果碰巧抄读的时刻就是镜像时标,会减一秒,保证数据的查询到当前库中查
			return ::ReadItem(IMG0, wPn, wID, rItemAcess, 
							  dwStartTime, dwEndTime, 
							  &m_pBank0Ctrl[m_pDbCtrl->wSectPnData]);
		}
		else
		{
			//再到当前库中找
			for (WORD wSect=0; wSect<m_wSectNum; wSect++) //终端参数和
			{
				if (wSect != m_iSectImg)
				{
					
					//只有在终端有交采,需要抄表的情况下才进行限制
					
					//NOTE:关于何时把查询的时间自动置为0
					//交采和脉冲测量点的数据,时标都不进行判断,认为当前库中的数据
					//就是最新的,就相当于电表抄到了表数据;
					//应用程序在编写的时候不能依赖于数据的更新时间,比如读月数据的
					//时候,就不行判断数据的更新时间来判断数据进行了月转存,
					//而是通过规定执行的起始时间来规避这个问题
					
					DWORD dwStartTm = dwStartTime;
					DWORD dwEndTm = dwEndTime;
					if (IsSectHaveSampleData(wSect) && !IsMeterPn(wPn)) 
					{	//段含有交采脉冲测量点的数据   && 交采脉冲测量点
						dwStartTm = dwEndTm= 0;
					}
						
					iRet = ::ReadItem(IMG0, wPn, wID, rItemAcess, dwStartTm, dwEndTm, &m_pBank0Ctrl[wSect]);
					if (iRet != -ERR_ITEM)
					{
						if (rItemAcess.bType == DI_ACESS_INFO)	//取数据项信息(长度和段)
							rItemAcess.pItemInfo->wSect = wSect;
						else if (rItemAcess.pdwTime!=0 && IsSectHaveSampleData(wSect) && !IsMeterPn(wPn) && IsPnValid(wPn)) 
							*rItemAcess.pdwTime = GetCurTime();

						return iRet;
					}
				}
			}
		}
	}
	else if (wBank<m_wBankNum && m_pBankCtrl[wBank].pItemDesc!=NULL)
	{
		return ::ReadItem(IMG0, wPn, wID, rItemAcess, dwStartTime, dwEndTime, &m_pBankCtrl[wBank]);
	}
	
	return -ERR_ITEM;
}


int CDataManager::WriteItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess& rItemAcess, BYTE bPerm, BYTE* pbPassword, DWORD dwTime)
{
	int iRet;
	
	if (IsDbLocked())
		return -ERR_ITEM;
		
	if (wBank == BANK0)
	{
		int iIndex;
		if (m_wImgNum>0 && m_iSectImg>=0 && dwTime!=0 && IsMeterPn(wPn) 
			&& (iIndex=BinarySearchIndex(m_pBank0Ctrl[m_iSectImg].pItemDesc, m_pBank0Ctrl[m_iSectImg].dwItemNum, wID))>=0)
		{	//带时刻的写且数据项是镜像中的数据项,需要写到镜像
			
			WORD wImg;
			for (wImg=0; wImg<m_wImgNum; wImg++)
			{
				if (dwTime == m_pImgCtrl[wImg].dwStartTime) //有该时刻的镜像
					break;
			}
			
			//对于镜像中的数据项,入镜像的同时也入当前库的SECT,当前库的时标就用当前的时标就行了,
			//因为当前库的时标保持跟最新数据抄读的时间一致,如果是镜像数据的话,有可能这个时标被规整过
			//在这里统一用当前时间入当前库
			DWORD dwCurTime = GetCurTime();	
			iRet = ::WriteItem(IMG0, wPn, wID, rItemAcess, 
							   bPerm, pbPassword, dwCurTime, 
							   &m_pBank0Ctrl[m_pDbCtrl->wSectPnData]);
														//同时把数据写到当前库
			if (wImg < m_wImgNum) //找到该时间的镜像
			{
				return ::WriteItem(wImg, wPn, wID, rItemAcess, bPerm, pbPassword, dwTime, &m_pBank0Ctrl[m_iSectImg]);
			}
			else
			{
				return iRet;
			}
		}
		else //写到当前库
		{
			//再到当前库中找
			for (WORD wSect=0; wSect<m_wSectNum; wSect++)
			{
				if (wSect != m_iSectImg)
				{
					iRet = ::WriteItem(IMG0, wPn, wID, rItemAcess, bPerm, pbPassword, dwTime, &m_pBank0Ctrl[wSect]);
					if (iRet != -ERR_ITEM)
						return iRet;
				}
			}
		}
	}
	else if (wBank<m_wBankNum && m_pBankCtrl[wBank].pItemDesc!=NULL)
	{
		return ::WriteItem(IMG0, wPn, wID, rItemAcess, bPerm, pbPassword, dwTime, &m_pBankCtrl[wBank]);
	}

	return -ERR_ITEM;
}

//描述:供版本升级用的,读数据同时取数据库中的时标
int CDataManager::UpgReadItem(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime)
{
	if (m_pDbUpgCtrl==NULL || !m_fDbUpg)
		return -ERR_ITEM;

	if (wBank<m_pDbUpgCtrl->wBankNum && 	//该BANK的旧版本文件存在
		m_pDbUpgCtrl->pBankCtrl[wBank].pbBankData!=NULL)
	{
		TItemAcess ItemAcess;
		memset(&ItemAcess, 0, sizeof(TItemAcess));
		ItemAcess.bType = DI_ACESS_BUF;
		ItemAcess.pbBuf = pbBuf;
		ItemAcess.pdwTime = pdwTime;

		return ::ReadItem(IMG0, wPn, wID, ItemAcess, INVALID_TIME, INVALID_TIME, &m_pDbUpgCtrl->pBankCtrl[wBank]);
	}

	return -ERR_ITEM;
}

//描述:取数据项的地址,但要注意不要直接用该地址访问数据,
//     而是要使用数据库提供的函数ReadItem()和WriteItem()
TDataItem CDataManager::GetItemEx(WORD wBank, WORD wPn, WORD wID)
{
	int iRet;
	TDataItem di;
	memset(&di, 0, sizeof(di));
	
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(ItemAcess));
	ItemAcess.bType = DI_ACESS_GI;
	ItemAcess.pbBuf = (BYTE* )&di;
	
	if (wBank == BANK0)
	{
		//再到当前库中找
		for (WORD wSect=0; wSect<m_wSectNum; wSect++) //终端参数和
		{
			if (wSect != m_iSectImg)
			{
				iRet = ::ReadItem(IMG0, wPn, wID, ItemAcess, 
								  INVALID_TIME, INVALID_TIME, 
								  &m_pBank0Ctrl[wSect]);
								  
				if (iRet != -ERR_ITEM)
					break;
			}
		}
	}
	else if (wBank < m_wBankNum) // && wBank!=BANK0
	{
		iRet = ReadItem(IMG0, wPn, wID, ItemAcess, 
						INVALID_TIME, INVALID_TIME, 
						&m_pBankCtrl[wBank]);
	} 

	return di;
}

void CDataManager::ClearBank(TBankCtrl* pBankCtrl)
{	
	if (pBankCtrl->pszPathName != NULL)   //该BANK需要被保存
	{
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
		{
			char szPathName[128];
			sprintf(szPathName, pBankCtrl->pszPathName, i);
			unlink(szPathName);   //删除

			if (pBankCtrl->pszBakPathName!=NULL && i==0)	//备份文件的路径文件名，为空表示不备份，
			{												//备份文件不支持多测量点多文件的备份，只支持单文件，不支持时标的备份
				char szBakPathName[128];	//备份文件的名称
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				unlink(szBakPathName);
			}

		} //for (i=0; i<pBankCtrl->wFileNum; i++)
	}
}

void CDataManager::ClearData()
{
}

void CDataManager::ClearPara()
{
}

void CDataManager::SaveVolatile()
{
}

//描述:查找测量点对应的映射号
//返回:如果找到则返回对应的映射号,否则返回-1
int CDataManager::SearchPnMap(BYTE bSch, WORD wPn)
{
	if (bSch==0 || bSch>m_wPnMapNum)	//方案号应该在1~PNMAP_NUM间
		return -1;

	TPnMapCtrl* pPnMapCtrl = &m_pPnMapCtrl[bSch-1];
	if (wPn >= pPnMapCtrl->wMaxPn)
		return -1;

	WaitSemaphore(m_semPnMap);	
	WORD wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //已经映射的个数
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	int little, big, mid;
	little = 0;
	big = (int )wMapNum-1;
	while (little <= big)
	{                               
		mid = (little + big) / 2;  //二分

		if (wPn == pPnMapCtrl->pwPnToMemMap[2+mid*2])
		{
			SignalSemaphore(m_semPnMap);
			return pPnMapCtrl->pwPnToMemMap[2+mid*2+1];
		}
		else if (wPn > pPnMapCtrl->pwPnToMemMap[2+mid*2])
		{
			little = mid + 1;
		} 
		else  //if (wPn < pPnMapCtrl.pwPnToMemMap[2+mid*2])
		{
			big = mid - 1;
		}

		mid = (little + big) / 2;
	}

	SignalSemaphore(m_semPnMap);
	return -1;
}

//描述:查找映射号对应的测量点
//返回:如果找到则返回对应的测量点,否则返回-1
int CDataManager::MapToPn(BYTE bSch, WORD wMn)
{
	if (bSch==0 || bSch>m_wPnMapNum)	//方案号应该在1~PNMAP_NUM间
		return -1;

	TPnMapCtrl* pPnMapCtrl = &m_pPnMapCtrl[bSch-1];

	WaitSemaphore(m_semPnMap);	
	WORD wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //已经映射的个数
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	int iPn = -1;
	for (WORD i=0; i<wMapNum; i++)
	{                               
		if (wMn == pPnMapCtrl->pwPnToMemMap[2+i*2+1])
		{
			iPn = pPnMapCtrl->pwPnToMemMap[2+i*2];
			break;
		}
	}

	SignalSemaphore(m_semPnMap);
	return iPn;
}

//描述:申请动态映射测量点
//参数:@bSch 映射方案
// 	   @wPn 需要映射的测量点号
//返回:如果正确返回映射号,否则返回-1
int CDataManager::NewPnMap(BYTE bSch, WORD wPn)
{
	WORD i, j;
	int iMN = SearchPnMap(bSch, wPn);
	if (iMN >= 0)	//已经存在该测量点的映射了
		return iMN;

	if (bSch==0 || bSch>m_wPnMapNum)	//方案号应该在1~PNMAP_NUM间
		return -1;

	bSch--;	//转换成g_PnMapCtrl数组中对应的索引
	TPnMapCtrl* pPnMapCtrl = &m_pPnMapCtrl[bSch];
	if (wPn >= pPnMapCtrl->wMaxPn)
	{
		DTRACE(DB_DB, ("CDataManager::NewPnMap: err, wPn(%d)>=wMaxPn(%d)\n", 
					   wPn, pPnMapCtrl->wMaxPn));
		return -1;
	}

	WaitSemaphore(m_semPnMap);	//SearchPnMap()也会申请释放m_semPnMap,不能在它前面等信号量
	WORD wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //已经映射的个数
	if (wMapNum >= pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		DTRACE(DB_DB, ("CDataManager::NewPnMap: there is no room for pn=%d\n", wPn));
		return -1;
	}

	//分配存储号(或者叫映射号)
	for (i=0; i<pPnMapCtrl->wAllocSize; i++)
	{
		if (pPnMapCtrl->pbAllocTab[i] != 0xff)	//有没占用的空间
			break;
	}

	if (i >= pPnMapCtrl->wAllocSize)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	for (j=0; j<8; j++)
	{
		if ((pPnMapCtrl->pbAllocTab[i]&(1<<j)) == 0)
			break;
	}

	if (j >= 8)	//应该不会出现这样的错误,以防万一
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	WORD wMN = i*8 + j;
	pPnMapCtrl->pbAllocTab[i] |= 1<<j;	//标志该存储空间已经被分配

	//按测量点号从小到大,确定新测量点在映射表中的位置
	for (i=0; i<wMapNum; i++)
	{
		if (wPn < pPnMapCtrl->pwPnToMemMap[2+i*2])
			break;
	}

	//挪出一个放新映射测量点的空间
	for (j=wMapNum; j>i; j--)
	{
		pPnMapCtrl->pwPnToMemMap[2+j*2] = pPnMapCtrl->pwPnToMemMap[2+(j-1)*2];
		pPnMapCtrl->pwPnToMemMap[2+j*2+1] = pPnMapCtrl->pwPnToMemMap[2+(j-1)*2+1];
	}

	pPnMapCtrl->pwPnToMemMap[2+i*2] = wPn;
	pPnMapCtrl->pwPnToMemMap[2+i*2+1] = wMN;
	pPnMapCtrl->pwPnToMemMap[0]++;

	m_dwPnMapFileFlg |= (DWORD )1<<bSch;
	SignalSemaphore(m_semPnMap);

	DTRACE(DB_DB, ("CDataManager::NewPnMap: new map=%d for pn=%d\n", wMN, wPn));
	return wMN;
}

//描述:删除映射测量点
//参数:@bSch 映射方案
// 	   @wPn 已经映射的测量点号
//返回:如果正确返回true,否则返回false
bool CDataManager::DeletePnMap(BYTE bSch, WORD wPn)
{
	WORD i;
	if (bSch==0 || bSch>m_wPnMapNum)	//方案号应该在1~PNMAP_NUM间
		return false;

	bSch--;	//转换成g_PnMapCtrl数组中对应的索引
	TPnMapCtrl* pPnMapCtrl = &m_pPnMapCtrl[bSch];
	if (wPn >= pPnMapCtrl->wMaxPn)
		return false;

	WaitSemaphore(m_semPnMap);
	WORD wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //已经映射的个数
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{	
		SignalSemaphore(m_semPnMap);
		return false;
	}
	
	//按测量点号从小到大,确定新测量点在映射表中的位置
	for (i=0; i<wMapNum; i++)
	{
		if (wPn == pPnMapCtrl->pwPnToMemMap[2+i*2])
			break;
	}

	if (i >= wMapNum)
	{
		SignalSemaphore(m_semPnMap);
		return false;
	}

	//删除对映射资源的占用
	WORD wMN = pPnMapCtrl->pwPnToMemMap[2+i*2+1];
	pPnMapCtrl->pbAllocTab[wMN/8] &= ~(1<<(wMN%8));	//标志该存储空间已经被分配

	//后面的映射整体往前移动,占用被删除测量点的空间
	for (; i<wMapNum-1; i++)
	{
		pPnMapCtrl->pwPnToMemMap[2+i*2] = pPnMapCtrl->pwPnToMemMap[2+(i+1)*2];
		pPnMapCtrl->pwPnToMemMap[2+i*2+1] = pPnMapCtrl->pwPnToMemMap[2+(i+1)*2+1];
	}

	pPnMapCtrl->pwPnToMemMap[0]--;
	m_dwPnMapFileFlg |= (DWORD )1<<bSch;
	SignalSemaphore(m_semPnMap);
	return true;
}

//如果该BANK使用测量点动态映射,则pBankCtrl->wPnNum调整为映射方案中配置的实际支持的测量点数
int CDataManager::GetPnMapRealNum(BYTE bSch)
{
	if (bSch>0 && bSch<=m_wPnMapNum) //方案号应该在1~m_wPnMapNum间
		return m_pPnMapCtrl[bSch-1].wRealNum;
	else
		return -1;
}


