/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DL645.cpp
 * 摘    要：本文件给出97版645抄表协议的功能实现
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年11月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "DL645.h"
#include "DbAPI.h"
#include "MtrCtrl.h"
#include "ComAPI.h"

#define DL645_CMD          8 
#define DL645_LEN          9
#define DL645_DATA         10

//#define DL645_CMD_RESERVE        0x00
#define DL645_CMD_ASK_DATA       0x01
#define DL645_CMD_ASK_NEXT       0x02
//#define DL645_CMD_REASK          0x03
//#define DL645_CMD_WRITE_DATA     0x04
#define DL645_CMD_BC_TIME        0x08
//#define DL645_CMD_WRITE_ADDR     0x0a
//#define DL645_CMD_CHG_BR         0x0c
//#define DL645_CMD_CHG_PSW        0x0f
//#define DL645_CMD_DMD_RESET      0x10

//#define DL645_CMD_MAX         DL645_CMD_DMD_RESET
#define DL645_CMD_GET   0x1f

//645协议自用函数
void FillAddrBuf(BYTE bAddrByte, BYTE* pbAddr);
WORD DL645MakeFrm(BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen);
int DL645TxRx(struct TMtrPro* pMtrPro, T645Tmp* pTmp645, WORD wID, WORD wLen, BYTE bReTryTimes);
int AskItemBID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);
int AskItem1BID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf, BYTE bReTryTimes);
int AskItemSID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);
int AskItem1SID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf, BYTE bReTryTimes);
int DL645AskItem1(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf);

//描述:645协议初始化函数
bool Mtr645Init(struct TMtrPro* pMtrPro, BYTE bThrId)
{
	pMtrPro->bThrId = bThrId;

	pMtrPro->pfnAskItem = DL645AskItemEx;	
	pMtrPro->pfnDirAskItem = DL645DirAskItemEx;
	pMtrPro->pfnRcvBlock = DL645RcvBlock;
	pMtrPro->pfnGetProPrintType = DL645GetProPrintType;
	pMtrPro->pfnWriteItem = DL645pfnWriteItem;	

	pMtrPro->pbTxBuf = &m_MtrTxBuf[bThrId][0];
	pMtrPro->pbRxBuf = &m_MtrRxBuf[bThrId][0];
	memset(pMtrPro->pbTxBuf, 0, MTR_FRM_SIZE); 
	memset(pMtrPro->pbRxBuf, 0, MTR_FRM_SIZE); 

	return true;
}

//描述:抄读接口数据对费率块ID以及C86f块ID的特殊处理
int DL645AskItem(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{	
	int iRet = 0;		
	WORD wID1 = wID;	
	BYTE bNum = 0;
	BYTE bLen = 0;	

	if (wID == 0xc86f) //如果是块数据，其他的状态字都读不到,
	{		
		bNum = 7;
		bLen = 2;
		wID1 = Id645V07toDL645(wID);				
		iRet = DL645AskItem1(pMtrPro, wID1, pbBuf);			

		if (iRet > 0) //把获得的645数据转成2007版645数据	
		{
			iRet = Data645to645V07(wID1, pbBuf, iRet);	
			memset(pbBuf+iRet, 0, bNum*bLen-iRet);
			iRet = bNum*bLen;						
		}		
	}		
	else//其他块ID以及单ID请求
	{	
		wID1 = Id645V07toDL645(wID); 		
		iRet = DL645AskItem1(pMtrPro, wID1, pbBuf);		

		if (iRet>0 && wID!=wID1) //把获得的645数据转成2007版645数据					
			iRet = Data645to645V07(wID1, pbBuf, iRet);		
	}

	return iRet;
}

//描述：读取电表事件数据的接口
int DL645DirAskItem(struct TMtrPro* pMtrPro, DWORD dwID, BYTE* pbBuf)
{	
	int iRet = 0;		

	return iRet;
}

BYTE bSchSpecAddr(BYTE* pbAddr)
{
	BYTE i, n=0;
	for (i=0; i<6; i++)
	{
		if (pbAddr[6-i-1]==0xAA || pbAddr[6-i-1]==0x99)
			n ++;
		else 
			break;
	}
	return n;
}

//描述:645接收验帧函数
bool DL645RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize)
{
    BYTE n;
	WORD i;
	//T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;
	T645Tmp* pTmp645 = (T645Tmp* )pTmpInf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf; 
    BYTE* pbTxBuf = pMtrPro->pbTxBuf;

	for ( ; dwLen; dwLen--)
	{
		BYTE b = *pbBlock++;

		switch (pTmp645->nRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				pbRxBuf[0] = 0x68;
				pTmp645->wRxPtr = 1;
				pTmp645->wRxCnt = 9;       
				pTmp645->nRxStep = 1;
			}
			break;
		case 1:    //数据域前的数据
			pbRxBuf[pTmp645->wRxPtr++] = b;
			pTmp645->wRxCnt --;
			if (pTmp645->wRxCnt == 0)   //接收完，进行校验
			{
                n = bSchSpecAddr(&pbTxBuf[1]); //避开广播地址的返回
				if (memcmp(&pbRxBuf[1], &pbTxBuf[1], 6-n)==0 && pbRxBuf[7]==0x68)
				{
					pTmp645->wRxDataLen = pbRxBuf[9];
					pTmp645->wRxCnt = pTmp645->wRxDataLen + 2;
					pTmp645->nRxStep = 2;
				}
				else
				{
					pTmp645->nRxStep = 0;
				}
			}
			break;
		case 2:     //数据 + 检验码 + 结束码
			pbRxBuf[pTmp645->wRxPtr++] = b;
			pTmp645->wRxCnt --;
			if (pTmp645->wRxCnt == 0)   //接收完，进行校验
			{
				pTmp645->nRxStep = 0;

				if (pbRxBuf[pTmp645->wRxPtr-1]==0x16 && pbRxBuf[pTmp645->wRxPtr-2]==CheckSum(pbRxBuf, pTmp645->wRxDataLen+10))
				{
					for (i=10; i<10+pTmp645->wRxDataLen; i++)
						pbRxBuf[i] -= 0x33;

					return true;    //接收到完整的一帧
				}
			}
			break;
		default:
			pTmp645->nRxStep = 0;
			break;
		} //switch (m_nRxStep) 
	}

	return false;
}

//描述:获取协议打印信息
void DL645GetProPrintType(BYTE* pbPrintPro, char* pszProName)
{
	*pbPrintPro = DB_645;
	memcpy(pszProName, "DL645", sizeof("DL645"));	
}

/////////////////////////////////////////////////////
//以下为645协议内部使用的函数定义

//描述:根据填充标志对地址进行填充
void FillAddrBuf(BYTE bAddrByte, BYTE* pbAddr)
{
	BYTE i;

	for (i=0; i<6; i++)
	{
		if (pbAddr[6-i-1] != 0)
			break;				
	}

	if (bAddrByte == 2)
	{
		memset(pbAddr+6-i, 0xaa, i);
	}	
	else
	{
		memset(pbAddr+6-i, 0, i);
	}
}

WORD DL645MakeFrm(BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen)
{	
	WORD i;	
	pbTxBuf[0] = 0x68;

	memrcpy(&pbTxBuf[1], pbAddr, 6);

	if (bCmd != DL645_CMD_BC_TIME)
		FillAddrBuf(0, &pbTxBuf[1]);

	pbTxBuf[7] = 0x68;
	pbTxBuf[8] = bCmd;
	pbTxBuf[9] = bLen;

    //+0x33
    for (i=10; i<(WORD)bLen+10; i++)
	{
  	    pbTxBuf[i] += 0x33;
	}	 
	
	pbTxBuf[10+(WORD)bLen] = CheckSum(pbTxBuf, (WORD)bLen+10);
	pbTxBuf[11+(WORD)bLen] = 0x16;

	return bLen+12;
}    


int DL645TxRx(struct TMtrPro* pMtrPro, T645Tmp* pTmp645, WORD wID, WORD wLen, BYTE bReTryTimes)
{	
	BYTE n;
	BYTE mBuf[50];
	bool fReadSuccess = false;

	//T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	
	CComm* pComm = pMtrPro->pMtrPara->pComm;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	memset(mBuf, 0, sizeof(mBuf));
	memset(mBuf, 0xfe, 4);
	memcpy(&mBuf[4], pbTxBuf, wLen);
	wLen += 4;

	for (n=0; n<bReTryTimes; n++)
	{
		if(n > 0) //重发的时候先清除一下串口
		{			
			pComm->Read(mBuf, sizeof(mBuf), 200);
		}

        //if (g_fDirRd[pMtrPro->bThrId] && !g_bDirRdStep)
        //    break;

		if (MtrProSend(pComm, mBuf, wLen) != wLen)
		{
			DTRACE(DB_645, ("CDL645::TxRx : fail to write comm.\r\n")); 
			continue;//return 0;
		}
		
		pTmp645->nRxStep = 0;		

		fReadSuccess = ReadCommFrm(pMtrPro, (void*)pTmp645, 0, 4, 2, 200, MTR_FRM_SIZE, 0, NULL, 0);

		if (fReadSuccess)	//接收到一个完整的帧
		{	
			if ((pbRxBuf[DL645_CMD]&DL645_CMD_GET) == (pbTxBuf[DL645_CMD]&DL645_CMD_GET))
			{
				if ((pbRxBuf[DL645_CMD]&0xc0) == 0x80)   //帧校验正确
				{
					WORD wRxID = pbRxBuf[DL645_DATA] + (WORD)pbRxBuf[DL645_DATA+1]*0x100;
					if (wRxID != wID)
					{							
						DTRACE(DB_645, ("CDL645:: Tx_ID:%x != Rx_ID:%x\r\n", wID, wRxID)); 									
					}
					else
					{
		 	 			return 1;
					}
				}
				else if ((pbRxBuf[DL645_CMD]&0xc0) == 0xc0)   //帧校验正确
				{
					DTRACE(DB_645, ("CDL645::TxRx : rx = not surport data.\r\n")); 
		 			return -1;
				}
			}
		}
 		DTRACE(DB_645, ("CDL645::TxRx : fail to rx frame.\r\n")); 
	}

	return 0;
}

//描述:记录抄读901F成功的相关信息
void SetRead901fSuccess(struct TMtrPro* pMtrPro, T645Tmp* pTmp645)
{
		WORD wLen = 0;
		BYTE* pbRxBuf = pMtrPro->pbRxBuf;
		TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
		//T645Priv* pMtr645 = (T645Priv* )pMtrPro->pvMtrPro;	

		//pMtr645->fRd901f = true;
	
		if (pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xaa || pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xdd)//科陆表实测									
				wLen = pTmp645->wRxDataLen-3;				
		else							
				wLen = pTmp645->wRxDataLen-2;
				
		if ((wLen%4)==0 && (wLen/4==3 || wLen/4==4))
		{	
			//浙江需求,分时表返回长度是2费率或3费率,则自适应改变费率顺序				
			pMtrPara->bRateTab[0] = 2;//费率顺序	
			pMtrPara->bRateTab[1] = 3;			
			pMtrPara->bRateTab[2] = 4;			
			pMtrPara->bRateTab[3] = 0;
			DTRACE(DB_645, ("CDL645::AskItem1 : Adjust RateNum By Read 0x901f.\r\n")); 
		}				
}

//描述：请求数据打包发送并接收
int DL645MakeAskData(struct TMtrPro* pMtrPro, T645Tmp* pTmp645, WORD wID, BYTE bReTryTimes)
{
	WORD wFrmLen;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
    TMtrPara* pMtrPara = pMtrPro->pMtrPara;	

	pbTxBuf[DL645_DATA] = (BYTE)wID;
	pbTxBuf[DL645_DATA+1] = (BYTE)(wID>>8);		
	wFrmLen = DL645MakeFrm(pbTxBuf, &pMtrPara->bAddr[1], DL645_CMD_ASK_DATA, 2);

	return DL645TxRx(pMtrPro, pTmp645, wID, wFrmLen, bReTryTimes);
}

//描述:645协议抄读接口函数
int DL645AskItem1(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{
	int iRet= -1;

	iRet = AskItemBID(pMtrPro, wID, pbBuf);

	//if (iRet > 0)	
	//	iRet = Data645ToComm(wID, pbBuf, (WORD)iRet);	

	return iRet;
}


//描述：读取那些DL645有此数据标识的数据,有块读块,没块拆分
int AskItemBID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{
	BYTE i, j;
	BYTE bNum, bItemLen;
	BYTE bTemp[100];
	bool bfind=false;
	WORD wSubID=0; 	
	int iLen=0, iRv;
	
	if ((wID&0x000F) == 0xF)
	{
		iLen = AskItem1BID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);
		if(iLen > 0)
			return iLen;

		wSubID = wID&0xFF00;
		if (IsRateId(wID) || wSubID==0xB600 || wSubID==0xB300)
		{
			bNum = GetBlockIdNum(wID);
			bItemLen = Get645TypeLength(wID);
			memset(pbBuf, m_bInvdData,  bNum*bItemLen);		
			
			iLen = 0;
			for (i=0; i<bNum; i++)
			{
				memset(bTemp, 0, sizeof(bTemp));
				if (wID==0xB61F || wID==0xB62F)
					j = i+1;			
				else
					j = i;
				iRv = AskItem1BID(pMtrPro, (wID&0xFFF0)+j, bTemp, TXRX_RETRYNUM);
				if (iRv > 0)
				{
					bfind = true;
					memcpy(pbBuf+iLen, bTemp, iRv);
					iLen += iRv;
				}
				else
				{	
					if (iRv < 0) //如果ID表示不支持（含通信返回不支持以及非通信检测不支持的情况），也算通信OK的，上次不予以补抄
						bfind = true;

					if (bNum == 5) //电量、需量、需量时间之类的数据
					{
						if (iRv<0 && i==0) //如果第一个子ID就不支持，则块ID视作不支持
							return -1;
						else if (iRv == -2) //非通信检测不支持的情况(费率调整或是不支持抄读的ID)
							iLen += bItemLen;
						else //if (iRv == -1) //注意需是通信返回的不支持,再停止抄后续子ID,否则单ID依据参数进行费率调整的时候,也会有不支持的,但要继续请求支持的子ID
						{
							iLen = bNum*bItemLen;
							break;
						}
					}
					else //瞬时量或断相
					{
						if (j == 0)	//总量不予判断
							iLen += bItemLen;
						else	//以A相为依据返回,后面的子ID不抄了
						{
							if (iRv<0 && j==1) //如果A相就不支持，则块ID视作不支持
							{
								if ( bfind && wID!=0xb67f ) //表示总已经回OK了
									iLen = bNum*bItemLen;
								else
									return -1;
							}
							else
								iLen = bNum*bItemLen;
							break;						
						}						
					}
				}			
			}

			if ( !bfind ) //子ID一个都没回答
			{
				return -1;
			}
		}	
	}
	else
	{
		iLen = AskItem1BID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);
	}

	if (iLen == -2) //转化配置不支持的ID的返回值
		return -1;

	return iLen;	
}

//描述：读取那些DL645有此数据标识的数据.尝试块与单
int AskItem1BID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf, BYTE bReTryTimes)
{
	BYTE find = 0, i;
	int iRet = -1, iLen;
	BYTE mBuf[80], bRxLen, bItemLen;

	TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
	//BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;

	//收帧解析的临时变量,每次处理重新开始
	T645Tmp tTmp645; 
	T645Tmp* pTmp645 = &tTmp645;
	memset(&tTmp645, 0, sizeof(T645Tmp));	

	if ( !MtrProOpenComm(pMtrPro->pMtrPara->pComm, &pMtrPro->pMtrPara->CommPara) )
		return 0;

	DTRACE(DB_645, ("CDL645::AskItem1 : wID = %x.\r\n", wID)); 

	if ( Is645NotSuptId(wID) ) //分相电能及相位角、视在功率、零序电流等不支持	
		return -2;	

	//根据费率的参数,调整读取的费率的单ID
	//费率参数的意义,是将对应电表上来的费率顺序,依次对应到终端的费率位置去,比如参数i0=2;i1=3;i2=4;i3=0,表示电表回来的第一个费率数据(抄表ID9011),是对应终端的峰费率9012的位置
	//则需注意,读取峰费率,读取ID是9012,实际的抄表ID是9011,返回的数据认做9012
	if ( IsRateId(wID) )
	{
		if ((wID&0x000f)!=0x000f && (wID&0x000f)!=0x0000) //分费率转换
		{
			for (i=0; i<4; i++)
			{
				if (pMtrPara->bRateTab[i] == (wID&0x000f))
				{					
					DTRACE(DB_645, ("CDL645::AskItem1BID : Pn=%d, wID = %x.adjust rate to read =%x\r\n",pMtrPara ->wPn, wID, (wID&0xfff0)+i+1)); 
					wID = (wID&0xfff0)+i+1;
					find = 1;
					break;
				}
			}
			if ( !find )	
			{
				DTRACE(DB_645, ("CDL645::AskItem1BID : Pn=%d, wID = %x.not find adjust rate.\r\n",pMtrPara ->wPn, wID)); 
				return -2;		
			}
		}
	}

	if ((iRet=DL645MakeAskData(pMtrPro, pTmp645, wID, bReTryTimes)) <= 0) 	
		return iRet;	

	//读取成功的ID再进行解析
	iLen = 0;
	if ((wID&0x000f)==0x000f 
		&& (pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xaa || pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xdd))//科陆表实测
	{
		memcpy(pbBuf, &pbRxBuf[DL645_DATA+2], pTmp645->wRxDataLen-3);
		iLen = pTmp645->wRxDataLen-3;
	}
	else
	{
		memcpy(pbBuf, &pbRxBuf[DL645_DATA+2], pTmp645->wRxDataLen-2);
		iLen = pTmp645->wRxDataLen-2;
	}
	bRxLen = iLen;
	bItemLen = Get645TypeLength(wID);
	memset(mBuf, m_bInvdData, 80);
	memcpy(mBuf, pbBuf, iLen);
	if ((wID&0xf) == 0xf) //费率转换
	{
		if (wID>=0xc32f && wID<=0xc3af)//费率
		{
			if (iLen != 42)//长度正常的	
			{
				iLen = 42;
				memcpy(pbBuf, mBuf, 42);
			}
		}
		else if (wID == 0xb63f)	
		{
			if (iLen == 16)//去掉后面的上下限ID	
			{
				iLen = 12;
				memcpy(pbBuf, mBuf, 12);
			}
			else if (iLen != 12)//长度不对需拆分
				return -1;
		}
		else if (wID==0xb61f || wID==0xb62f)
		{
			if (iLen != 6)//长度不对需拆分
				return -1;
		}
		else if (wID==0xb64f || wID==0xb65f)
		{
			if (iLen != 8)//长度不对需拆分
				return -1;
			if (wID == 0xb64f)//修正与07协议的长度一致
			{
				iLen = 12;
				for (i=0; i<4; i++)
				{
					memcpy(&pbBuf[i*3], &mBuf[i*2], 2);
					pbBuf[i*3+2] = 0;
				}
			}
		}
	}
	else
	{
		memset(mBuf, 0, 80);
		memcpy(mBuf, pbBuf, iLen);
		iLen = bItemLen;
		memcpy(pbBuf, mBuf, iLen);
	}

	return iLen;
}

//描述：读取那些DL645有此数据标识的数据,只采用单ID,对块进行拆分
int AskItemSID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf)
{
	BYTE i, j;
	BYTE bNum, bItemLen;
	BYTE bTemp[100];
	bool bfind=false;
	WORD wSubID=0; 	
	int iLen=0, iRv;

	if ((wID&0x000F) == 0xF)
	{
		wSubID = wID&0xFF00;
		if (IsRateId(wID) || wSubID==0xB600 || wSubID==0xB300)
		{
			bNum = GetBlockIdNum(wID);
			bItemLen = Get645TypeLength(wID);
			memset(pbBuf, m_bInvdData,  bNum*bItemLen);		
			
			iLen = 0;
			for (i=0; i<bNum; i++)
			{
				memset(bTemp, 0, sizeof(bTemp));
				if (wID==0xB61F || wID==0xB62F)
					j = i+1;			
				else
					j = i;
				iRv = AskItem1SID(pMtrPro, (wID&0xFFF0)+j, bTemp, TXRX_RETRYNUM);
				if (iRv > 0)
				{
					bfind = true;
					memcpy(pbBuf+iLen, bTemp, iRv);
					iLen += iRv;
				}
				else
				{	
					if (iRv < 0) //如果ID表示不支持（含通信返回不支持以及非通信检测不支持的情况），也算通信OK的，上次不予以补抄
						bfind = true;

					if (bNum == 5) //电量、需量、需量时间之类的数据
					{
						if (iRv<0 && i==0) //如果第一个子ID就不支持，则块ID视作不支持
							return -1;
						else if (iRv == -2) //非通信检测不支持的情况(费率调整或是不支持抄读的ID)
							iLen += bItemLen;
						else //if (iRv == -1) //注意需是通信返回的不支持,再停止抄后续子ID,否则单ID依据参数进行费率调整的时候,也会有不支持的,但要继续请求支持的子ID
						{
							iLen = bNum*bItemLen;
							break;
						}
					}
					else //瞬时量或断相
					{
						if (j == 0)	//总量不予判断
							iLen += bItemLen;
						else	//以A相为依据返回,后面的子ID不抄了
						{
							if (iRv<0 && j==1) //如果A相就不支持，则块ID视作不支持
							{
								if ( bfind ) //表示总已经回OK了
									iLen = bNum*bItemLen;
								else
									return -1;
							}
							else
								iLen = bNum*bItemLen;
							break;						
						}						
					}
				}			
			}

			if ( !bfind ) //子ID一个都没回答
			{
				return -1;
			}
		}	
	}
	else
	{
		iLen = AskItem1SID(pMtrPro, wID, pbBuf, TXRX_RETRYNUM);
	}

	if (iLen == -2) //转化配置不支持的ID的返回值
		return -1;

	return iLen;	
}

//描述：读取那些DL645有此数据标识的数据,只尝试单ID
int AskItem1SID(struct TMtrPro* pMtrPro, WORD wID, BYTE* pbBuf, BYTE bReTryTimes)
{
	BYTE find = 0, i;
	int iRet = -1, iLen;


	TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
	//BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;

	//收帧解析的临时变量,每次处理重新开始
	T645Tmp tTmp645; 
	T645Tmp* pTmp645 = &tTmp645;
	memset(&tTmp645, 0, sizeof(T645Tmp));

	if ( !MtrProOpenComm(pMtrPro->pMtrPara->pComm, &pMtrPro->pMtrPara->CommPara) )
		return 0;

	DTRACE(DB_645, ("CDL645::AskItem1 : wID = %x.\r\n", wID)); 

	if ( Is645NotSuptId(wID) ) //分相电能及相位角、视在功率、零序电流等不支持	
		return -2;	

	//根据费率的参数,调整读取的费率的单ID
	//费率参数的意义,是将对应电表上来的费率顺序,依次对应到终端的费率位置去,比如参数i0=2;i1=3;i2=4;i3=0,表示电表回来的第一个费率数据(抄表ID9011),是对应终端的峰费率9012的位置
	//则需注意,读取峰费率,读取ID是9012,实际的抄表ID是9011,返回的数据认做9012
	if ( IsRateId(wID) )
	{
		if ((wID&0x000f)!=0x000f && (wID&0x000f)!=0x0000) //分费率转换
		{
			for (i=0; i<4; i++)
			{
				if (pMtrPara->bRateTab[i] == (wID&0x000f))
				{					
					wID = (wID&0xfff0)+i+1;
					find = 1;
					break;
				}
			}
			if (!find )	return -2;		
		}
	}

	if ((iRet=DL645MakeAskData(pMtrPro, pTmp645, wID, bReTryTimes)) <= 0) 	
		return iRet;	

	//读取成功的ID再进行解析
	iLen = 0;
	if ((wID&0x000f)==0x000f 
		&& (pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xaa || pbRxBuf[DL645_DATA+pTmp645->wRxDataLen-1]==0xdd))//科陆表实测
	{
		memcpy(pbBuf, &pbRxBuf[DL645_DATA+2], pTmp645->wRxDataLen-3);
		iLen = pTmp645->wRxDataLen-3;
	}
	else
	{
		memcpy(pbBuf, &pbRxBuf[DL645_DATA+2], pTmp645->wRxDataLen-2);
		iLen = pTmp645->wRxDataLen-2;
	}

	return iLen;
}

//读数据接口
int DL645AskItemRecord(struct TMtrPro* pMtrPro, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	int iRet;
	DWORD dwRelaOAD;
	BYTE* pbTmp = pbData;
	BYTE bCollMode, bRcsdNum;
	//收帧解析的临时变量,每次处理重新开始
	T645Tmp tTmp645;
	memset(&tTmp645, 0, sizeof(T645Tmp));

	bCollMode = *pbRSD;
	switch (dwOAD)
	{
	case 0x50020200: //分钟冻结
	case 0x50030200: //小时冻结
		//if (bCollMode == 2)
		{
			bRcsdNum = *pbRCSD++;

			for (BYTE i=0; i<bRcsdNum; i++)
			{
				pbRCSD++;
				memrcpy((BYTE* )&dwRelaOAD, pbRCSD, 4);
				if (dwRelaOAD != 0x20210200) //非时标
				{
					iRet = DL645AskItemEx(pMtrPro, 1, dwRelaOAD,  pbTmp);
				}
				else
				{
					iRet = 7;
					*pbTmp++ = DT_DATE_TIME_S;
					memcpy(pbTmp, pbRSD+6, iRet);
				}
				pbRCSD += 4;
				if (iRet > 0)
					pbTmp += iRet;
				else
					return iRet;
			}
		}
		//else
		//	return -1;
		break;

	case 0x50040200: //日冻结
	case 0x50050200: //结算日冻结
	case 0x50060200: //月冻结	
		//if (bCollMode == 9)
		{
			bRcsdNum = *pbRCSD++;
			for (BYTE i=0; i<bRcsdNum; i++)
			{
				pbRCSD++;
				memrcpy((BYTE* )&dwRelaOAD, pbRCSD, 4);
				if (dwRelaOAD != 0x20210200) //非时标
				{
					iRet = DL645AskItemEx(pMtrPro, 1, dwRelaOAD,  pbTmp);
				}
				else
				{
					*pbTmp++ = DT_DATE_TIME_S;
					TTime tm;
					GetCurTime(&tm);
					iRet = OoTimeToDateTimeS(&tm, pbTmp);
				}
				pbRCSD += 4;
				if (iRet > 0)
					pbTmp += iRet;
				else
					return iRet;
			}
		}
		//else
		//	return -1;
		break;

	default:
		return -1;
	}

	return pbTmp - pbData;
}

//读数据接口
int DL645AskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	int iRet;
	BYTE bBuf[1024];
	Toad645Map* pOad645Map = GetOad645Map(dwOAD);

	if (pOad645Map == NULL) //不支持的数据项
		return -2;

	if (bRespType == 1) //读实时数据
	{
		iRet = DL645AskItem(pMtrPro, pOad645Map->wID, bBuf);

		if (pOad645Map->wID == 0xc010)
		{
			BYTE bTmpBuf[10];
			if (iRet > 0)
				memcpy(&bTmpBuf[3], bBuf+1, iRet-1);
			iRet += DL645AskItem(pMtrPro, 0xc011, bTmpBuf);
			TTime tm;
			Fmt1ToTime(bTmpBuf, tm);
			pbData[0] = DT_DATE_TIME_S;
			iRet = 1;
			iRet += OoTimeToDateTimeS(&tm, pbData+1);
			return iRet;
		}

		if (iRet > 0)
		{
			return OIFmtDataExt(bBuf, iRet, pbData, pOad645Map->pFmt, pOad645Map->wFmtLen, dwOAD);
		}
	}
	else
	{
		return DL645AskItemRecord(pMtrPro, dwOAD, pbData, pbRSD, bLenRSD, pbRCSD, bLenRCSD);
	}

	return -1;
}

int DL645DirAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData)
{
	return -1;
}

int DL645pfnWriteItem(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen)
{
	BYTE mBuf[100];
	bool fReadSuccess = false;
	T645Tmp tTmp645; 
	T645Tmp* pTmp645 = &tTmp645;

	CComm* pComm = pMtrPro->pMtrPara->pComm;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	memset(mBuf, 0, sizeof(mBuf));
	memset(mBuf, 0xfe, 4);
	memcpy(&mBuf[4], pbTxBuf, wLen);
	wLen += 4;

	if ( !MtrProOpenComm(pMtrPro->pMtrPara->pComm, &pMtrPro->pMtrPara->CommPara) )
		return -1;

		if (MtrProSend(pComm, mBuf, wLen) != wLen)
		{
			DTRACE(DB_645, ("CDL645::TxRx : fail to write comm.\r\n")); 
			return -1;
		}
		
		pTmp645->nRxStep = 0;		

		fReadSuccess = ReadCommFrm(pMtrPro, (void*)pTmp645, 0, 4, 2, 200, MTR_FRM_SIZE, 0, NULL, 0);

		if (fReadSuccess)	//接收到一个完整的帧
		{	
			if ((pbRxBuf[DL645_CMD]&DL645_CMD_GET) == (pbTxBuf[DL645_CMD]&DL645_CMD_GET))
			{
				if ((pbRxBuf[DL645_CMD]&0xc0) == 0x80)   //帧校验正确
				return 0;
			}
		}
	return -1;

}
	
