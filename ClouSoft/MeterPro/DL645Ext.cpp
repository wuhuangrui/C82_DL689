/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DL645Ext.c
 * 摘    要：本文件给出07版645抄表协议的功能实现
 * 当前版本：1.0
 * 作    者：郭树海
 * 完成日期：2015年10月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaCfg.h"
#include "DL645Ext.h"
#include "DbAPI.h"
#include "MtrCtrl.h"
#include "Esam.h"

#if 0
#define	DL645EXT_CMD	      8	
#define	DL645EXT_LEN	      9	
#define	DL645EXT_EXTCMD	      10	
#define	DL645Ext_DATA	      11


#define	DL645EXT_CMD_T188 	0x08
#define	DL645EXT_CMD_OTHER 	0x0B
//#define	DL645V07_CMD_RESERVE	0x00	
#define	DL645V07_CMD_BC_TIME	0x08
#define	DL645V07_CMD_ASK_DATA	0x11	
#define	DL645V07_CMD_ASK_NEXT	0x12	

//#define	DL645V07_CMD_ASK_ADDR	0x13	
#define	DL645V07_CMD_WRITE_DATA	0x14	
//#define	DL645V07_CMD_WRITE_ADDR	0x15	
//#define	DL645V07_CMD_FRZ		0x16	
//#define	DL645V07_CMD_CHG_BR		0x17	
//#define	DL645V07_CMD_CHG_PSW	0x18	
//#define	DL645V07_CMD_DMD_RESET	0x19	
//#define	DL645V07_CMD_ENG_RESET	0x1A
//#define	DL645V07_CMD_EVENT_RESET 0x1B
#define	DL645V07_CMD_CTRL		0x1C	//跳合闸、报警、保电

//#define	DL645V07CMD_MAX			DL645V07CMD_EVENT_RESET
#define	DL645V07_CMD_GET	0x1f

#endif

/*******************************************************
********************************************************/ 

typedef struct sT188
{  
    BYTE Head1;      // 68
    BYTE bMtrType;   // 仪表类型T
    BYTE bAddrs[7];  // 地址A0~A6
    BYTE bTCode;     // T188控制码C
    BYTE bLen;       // 长度
    BYTE bData[];    // 数据域
}sT188_Type;


static TItemListExt g_IdInTo188Cfg[] = //ID转换描述
{
    { 0xfa00,  0x901f,   43,    43,   },  //实时数据	
    { 0xfa01,  0xD120,   5,     5,    },  //结算日数据    
    { 0,       0,        0,     0,    }   //----保持本项在最后----
};
#define  IDINTO188_NUM   (sizeof(g_IdInTo188Cfg) / sizeof(TItemListExt) - 1)
//07版645协议自用函数

WORD DL645ExtMakeFrm(T645ExtTmp* pTmpV07, BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen);
BYTE Make188AskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm);
BYTE MakeJsJdAskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm);
BYTE MakeJsLxAskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm);
int DL645ExtTxRx(struct TMtrPro* pMtrPro, T645ExtTmp* pTmpV07, WORD wLen);
int DL645ExtAskItem1(struct TMtrPro* pMtrPro, T645ExtTmp* pTmpV07, DWORD dwOad, WORD wID, BYTE* pbBuf);
extern int MtrReadFrzExt(struct TMtrPro* pMtrPro, T645ExtTmp* pTmpV07, WORD wID, BYTE* pbBuf, WORD wSubIdx);
int MakeExt645Frm(BYTE* pbAddr, BYTE bExtCode, const BYTE* pbDataArea, BYTE bDataAreaLen, BYTE* pbFrm);

bool Mtr645ExtInit(struct TMtrPro* pMtrPro, BYTE bThrId)
{
	pMtrPro->bThrId = bThrId;

	pMtrPro->pfnAskItem = DL645ExtAskItemEx;	
	pMtrPro->pfnDirAskItem = DL645ExtDirAskItemEx;
	pMtrPro->pfnRcvBlock = DL645ExtRcvBlock;
	pMtrPro->pfnGetProPrintType = DL645ExtGetProPrintType;
	pMtrPro->pfnWriteItem = DL645ExtpfnWriteItem;	
	
	pMtrPro->pbTxBuf = &m_MtrTxBuf[bThrId][0];
	pMtrPro->pbRxBuf = &m_MtrRxBuf[bThrId][0];
	memset(pMtrPro->pbRxBuf, 0, MTR_FRM_SIZE); 
	memset(pMtrPro->pbRxBuf, 0, MTR_FRM_SIZE); 	
	return true;
}

//描述：读取97版645ID数据标识的数据的接口
//备注：有些块ID需拆分处理
int DL645ExtAskItem(struct TMtrPro* pMtrPro, DWORD dwOad, WORD wID, BYTE* pbBuf)
{
	int iRv;    
 
	//收帧解析的临时变量,每次处理重新开始
	T645ExtTmp tTmpV07;
	memset(&tTmpV07, 0, sizeof(T645ExtTmp));	

	iRv = DL645ExtAskItem1(pMtrPro, &tTmpV07, dwOad, wID, pbBuf);

    return iRv;		
}

//描述：读取电表事件数据的接口
int DL645ExtDirAskItem(struct TMtrPro* pMtrPro, DWORD dwID, BYTE* pbBuf)
{
	return -1;
}

//描述：接收验证
bool DL645ExtRcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize)
{
	BYTE b; 

	//TV07Priv* pMtrV07 = (TV07Priv* )pMtrPro->pvMtrPro;
	T645ExtTmp* pTmpV07 = (T645ExtTmp* )pTmpInf;
	BYTE* pbRxBuf = pMtrPro->pbRxBuf; 

#ifdef FRM_SEG_FLG
	memset((BYTE*)pTmpV07, 0, sizeof(T645ExtTmp));
#endif

	for ( ; dwLen; dwLen--)
	{
		b = *pbBlock++;

		switch (pTmpV07->nRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				pbRxBuf[0] = 0x68;
				pTmpV07->wRxPtr = 1;
				pTmpV07->wRxCnt = 9;       
				pTmpV07->nRxStep = 1;
			}
			break;
		case 1:    //数据域前的数据
			pbRxBuf[pTmpV07->wRxPtr++] = b;
			pTmpV07->wRxCnt --;
			if (pTmpV07->wRxCnt == 0)   //接收完，进行校验
			{
				if (/*memcmp(&pbRxBuf[1],&pbTxBuf[1],6)==0 &&*/ pbRxBuf[7]==0x68) //不去判断转换器地址是否正确，如果水汽热表挂在当前
				{                                                                 //转换器的话，不管是否正确配置了转换器地址，转换器都能够去
					pTmpV07->wRxDataLen = pbRxBuf[DL645EXT_LEN];                  //抄表，而且转换器的应答帧会带上自己的地址，这里要正确的判断应答帧
					pTmpV07->wRxCnt = pTmpV07->wRxDataLen + 2;
					pTmpV07->nRxStep = 2;
				}
				else
				{
					pTmpV07->nRxStep = 0;
				}
			}
			break;
		case 2:     //数据 + 检验码 + 结束码
			pbRxBuf[pTmpV07->wRxPtr++] = b;
			pTmpV07->wRxCnt -- ;
			if (pTmpV07->wRxCnt == 0)   //接收完，进行校验
			{
				pTmpV07->nRxStep = 0;

				if (pbRxBuf[pTmpV07->wRxPtr-1]==0x16 && pbRxBuf[pTmpV07->wRxPtr-2]==CheckSum(pbRxBuf, pTmpV07->wRxDataLen+10))
				{
				//	for (i=10; i<10+pTmpV07->wRxDataLen; i++)
				//		pbRxBuf[i] -= 0x33;

					return true;    //接收到完整的一帧
				}
			}
			break;
		default:
			pTmpV07->nRxStep = 0;
			break;
		} //switch (pMtrV07->nRxStep) 
	}

	return false;
}	


void DL645ExtGetProPrintType(BYTE* pbPrintPro, char* pszProName)
{
	*pbPrintPro = DB_Ext645;
	memcpy(pszProName, "DL645Ext", sizeof("DL645Ext"));
}

//描述：组帧发送
WORD DL645ExtMakeFrm(T645ExtTmp* pTmpV07, BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen)
{
	WORD i;	
	
	pbTxBuf[0] = 0x68;
	memcpy(&pbTxBuf[1], pbAddr, 6);
	pbTxBuf[7] = 0x68;
	pbTxBuf[8] = bCmd;	

	if (bCmd == DL645V07_CMD_ASK_NEXT)
	{	
		bLen ++; //增加帧序号
		pTmpV07->bRdNextSeq ++;
		if (pTmpV07->bRdNextSeq == 0)
			pTmpV07->bRdNextSeq = 1;
		pbTxBuf[14] = pTmpV07->bRdNextSeq;
	}
	else	
	{
		pTmpV07->fRdNext = false;
		pTmpV07->bRdNextSeq = 0;		
	}
	
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

//描述：帧解析
//返回1：正确；0：通讯正常，无有效数据返回，-1：无任何数据返回，错包；
int DL645ExtTxRx(struct TMtrPro* pMtrPro, T645ExtTmp* pTmpV07, WORD wLen)
{	
    BYTE n, bPos;
	bool fReadSuccess;
	BYTE mBuf[50];
	TMtrPara* pMtrPara = pMtrPro->pMtrPara;
	//TV07Priv* pMtrV07 = (TV07Priv* )pMtrPro->pvMtrPro;	
	BYTE* pbRxBuf = pMtrPro->pbRxBuf;
	BYTE* pbTxBuf = pMtrPro->pbTxBuf;
	memset(mBuf, 0, sizeof(mBuf));
	memset(mBuf, 0xfe, 4);
	memcpy(&mBuf[4], pbTxBuf, wLen);
	wLen += 4; 

	if (MtrProSend(pMtrPro->pMtrPara->pComm, mBuf, wLen) != wLen)
	{
		DTRACE(DB_Ext645, ("TxRx : fail to write comm.\r\n")); 
		return 0;
	}

	pTmpV07->nRxStep = 0;

    fReadSuccess = ReadCommFrm(pMtrPro, (void*)pTmpV07, 200, 30, 15, 200, MTR_FRM_SIZE, 0, NULL, 0);
	if (fReadSuccess)	//接收到一个完整的帧
	{
		if ((pbRxBuf[DL645EXT_CMD]&DL645V07_CMD_GET) == (pbTxBuf[DL645EXT_CMD]&DL645V07_CMD_GET))
		{		
            pTmpV07->fRdNext = false;

			if ((pbRxBuf[DL645EXT_CMD]-pbTxBuf[DL645EXT_CMD]) == 0x80)   //帧校验正确、正常回答
			{	               
                if(pbRxBuf[DL645EXT_EXTCMD]==DL645EXT_CMD_T188 || pbRxBuf[DL645EXT_EXTCMD]==DL645EXT_CMD_OTHER)   //帧校验正确
                {
					switch (pMtrPara->bSubProId)
					{
					case PROTOCOLNO_JSJD_MBUS:
						for (n=0; n<pbRxBuf[DL645EXT_LEN]; n++)
						{
							if (pbRxBuf[DL645Ext_DATA+n]==0x5A && pbRxBuf[DL645Ext_DATA+n+1]==0xD9)
							{
								bPos = n + 3;
								break;
							}
						}
						if (n >= pbRxBuf[DL645EXT_LEN])
						{
							DTRACE(DB_Ext645, ("TxRx : rx bSubProId==1 CMD fail.\r\n"));
							return -1;
						}
						if (pbRxBuf[DL645Ext_DATA+bPos]!=pMtrPara->bMtrAddr[2] || pbRxBuf[DL645Ext_DATA+bPos+1]!=pMtrPara->bMtrAddr[1] || pbRxBuf[DL645Ext_DATA+bPos+2]!=pMtrPara->bMtrAddr[0])
						{
							DTRACE(DB_Ext645, ("TxRx : Mtr Addr Diff, Rx Addr=%02x%02x%02x.\r\n", pbRxBuf[DL645Ext_DATA+bPos], pbRxBuf[DL645Ext_DATA+bPos+1],pbRxBuf[DL645Ext_DATA+bPos+2]));
							DTRACE(DB_Ext645, ("TxRx : Mtr Para Addr=%02x%02x%02x.\r\n",pMtrPara->bMtrAddr[2], pMtrPara->bMtrAddr[1], pMtrPara->bMtrAddr[0]));

							DTRACE(DB_Ext645, ("TxRx : rx addr fail.\r\n"));
							return -1;
						}
						break;

					case PROTOCOLNO_JSLX_RS485:
						for (n=0; n<pbRxBuf[DL645EXT_LEN]; n++)
						{
							if (pbRxBuf[DL645Ext_DATA+n]==0x7e && pbRxBuf[DL645Ext_DATA+n+1]==0x83)
							{
								bPos = n + 2;
								break;
							}
						}
						if (n >= pbRxBuf[DL645EXT_LEN])
						{
							DTRACE(DB_Ext645, ("TxRx : rx bSubProId==2 CMD fail.\r\n"));
							return -1;
						}
						if (pbRxBuf[DL645Ext_DATA+bPos]!=pbTxBuf[DL645Ext_DATA+7] || pbRxBuf[DL645Ext_DATA+bPos+1]!=pbTxBuf[DL645Ext_DATA+8] || pbRxBuf[DL645Ext_DATA+bPos+2]!=pbTxBuf[DL645Ext_DATA+9])
						{
							DTRACE(DB_Ext645, ("TxRx : Mtr Addr Diff, Rx Addr=%02x%02x%02x.\r\n", pbRxBuf[DL645Ext_DATA+bPos], pbRxBuf[DL645Ext_DATA+bPos+1],pbRxBuf[DL645Ext_DATA+bPos+2]));
							DTRACE(DB_Ext645, ("TxRx : Mtr Para Addr=%02x%02x%02x.\r\n",pMtrPara->bMtrAddr[2], pMtrPara->bMtrAddr[1], pMtrPara->bMtrAddr[0]));

							DTRACE(DB_Ext645, ("TxRx : rx addr fail.\r\n"));
							return -1;
						}
						break;

					default:
						BYTE bddr[7];
						revcpy(bddr, &pbRxBuf[DL645Ext_DATA+2], 7);
						if (memcmp(bddr, pMtrPara->bMtrAddr, 7) != 0)
						{
							DTRACE(DB_Ext645, ("TxRx : Mtr Addr Diff, Rx Addr=%02x%02x%02x%02x%02x%02x%02x.\r\n", pbRxBuf[DL645Ext_DATA+8], pbRxBuf[DL645Ext_DATA+7], 
								pbRxBuf[DL645Ext_DATA+6], pbRxBuf[DL645Ext_DATA+5], pbRxBuf[DL645Ext_DATA+4], pbRxBuf[DL645Ext_DATA+3], pbRxBuf[DL645Ext_DATA+2]));
							DTRACE(DB_Ext645, ("TxRx : Mtr Para Addr=%02x%02x%02x%02x%02x%02x%02x.\r\n", pMtrPara->bMtrAddr[6], pMtrPara->bMtrAddr[5], 
								pMtrPara->bMtrAddr[4], pMtrPara->bMtrAddr[3], pMtrPara->bMtrAddr[2], pMtrPara->bMtrAddr[1], pMtrPara->bMtrAddr[0]));

							DTRACE(DB_Ext645, ("TxRx : rx addr fail.\r\n"));
							return -1;
						}
						break;
					}
					return 1;
                }
                else if(pbRxBuf[DL645EXT_EXTCMD] == 0xc8)   //帧校验不正确
                {                        
					return -1;                 
                }              
			}
		}
        DTRACE(DB_Ext645, ("TxRx : mbus is not ack.\r\n"));
        return 0;
	}

	DTRACE(DB_Ext645, ("TxRx : fail to rx frame.\r\n")); 
	return -1;
}

WORD GetDataValT188(struct TMtrPro* pMtrPro, DWORD dwOad, WORD wID, BYTE* pbDstBuf)
{ 	
    sT188_Type *pT188 = NULL;
    WORD wAskId = 0, wFrmId;
	BYTE n, bPos = 0;
	int iLen = 0;
	DWORD dwVal;
    TMtrPara *pMtrPara = pMtrPro->pMtrPara;
	BYTE ExtCode = pMtrPro->pbRxBuf[DL645EXT_EXTCMD]; 

	if (ExtCode == DL645EXT_CMD_T188)
	{
        pT188 = (sT188_Type *)&pMtrPro->pbRxBuf[DL645Ext_DATA];
       /* if (memcmp(pT188->bAddrs, pMtrPara->bMtrAddr, 7) != 0) //应该在收发函数判断地址
        {
            DTRACE(DB_Ext645, ("GetDataValExt :T188 Addr fail wPn=%d, TxAddr=%02x%02x%02x%02x%02x%02x%02x,RxAddr=%02x%02x%02x%02x%02x%02x%02x.\r\n", 
                pMtrPara->wPn, pMtrPara->bMtrAddr[6], pMtrPara->bMtrAddr[5], pMtrPara->bMtrAddr[4], pMtrPara->bMtrAddr[3], pMtrPara->bMtrAddr[2], pMtrPara->bMtrAddr[1], pMtrPara->bMtrAddr[0],
                pT188->bAddrs[6], pT188->bAddrs[5], pT188->bAddrs[4], pT188->bAddrs[3], pT188->bAddrs[2], pT188->bAddrs[1], pT188->bAddrs[0]));
            return 0;
        }*/

		wAskId = pT188->bData[0] * 256 + pT188->bData[1];
		wFrmId = ByteToDWORD(&pT188->bData[0], 2);
        if (wAskId==wID || wFrmId==wID) //0x901f || 0x1f90
        {
			if (pT188->bLen > 3)
				iLen = pT188->bLen - 3;
			else
				iLen = 0;
			if (wID == 0xD120) //0xfa01
				memset(pbDstBuf, 0xee, 5);
			else
				memset(pbDstBuf, 0xee, 43);
			if (iLen > 0)
				memcpy(pbDstBuf, &pT188->bData[3], iLen);

			//水表0x901f的数据处理为国网C1F188的格式(43字节)
			switch (pMtrPara->bSubProId)
			{
				case PROTOCOLNO_HUAXU_T188_MBUS:
					if (wID==0x901f && iLen==6)
					{
						if (dwOad == 0x25000200)
						{
							memcpy((BYTE* )&dwVal, &pT188->bData[3], 4);
							OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25060200)
						{
							memcpy(pbDstBuf, &pT188->bData[7], 2);
							pbDstBuf[1] = (pbDstBuf[0] >> 2);
							pbDstBuf[0] &= 0x03; 
							iLen = 2;
						}
						else
							iLen = 0;
					}
					break;
				case PROTOCOLNO_HUAXU_T188_RS485:
					if (wID==0x901f && iLen==19)
					{
						BYTE bStatus;
						DDWORD dwCurVal, dwLastVal;
						if (dwOad == 0x25000200)
						{
							dwCurVal = ByteToDWORD(pbDstBuf+1, 4);	//读回的数据是16进制,单位是10升(1L=0.001m3)
							dwLastVal = ByteToDWORD(pbDstBuf+6, 4);
							//OoDWordToDoubleLongUnsigned(dwCurVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25060200)//
						{
							bStatus = pbDstBuf[41];
							pbDstBuf[0] = bStatus;
							pbDstBuf[1] = (pbDstBuf[0] >> 2);
							pbDstBuf[0] &= 0x03; 
							iLen = 2;
						}
						else
							iLen = 0;
					}
					break;
				case PROTOCOLNO_ZHENGTAI_T188:
					if (wID==0x901f && iLen==7)
					{
						if (dwOad == 0x25000200)
						{
							memcpy((BYTE* )&dwVal, pbDstBuf, 4);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25060200)
						{
							pbDstBuf[0] = pbDstBuf[5];
							pbDstBuf[1] = (pbDstBuf[0] >> 2);
							pbDstBuf[0] &= 0x03; 
							iLen = 2;
						}
						else
							iLen = 0;
					}
					break;
				default:	//PROTOCOLNO_STD_T188
					if (wID==0x901f)
					{
						if (dwOad==0x25000200 || dwOad==0x25010200) //水表和气表
						{
							memcpy((BYTE* )&dwVal, pbDstBuf, 4);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);//在OIFmtDataExt（）里有处理
							iLen = 4;
						}
						else if (dwOad == 0x25020200) //热表
						{
							memcpy((BYTE* )&dwVal, &pbDstBuf[5], 4);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25030200) //热表
						{
							memcpy((BYTE* )&dwVal, &pbDstBuf[10], 4);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25040200) //热表
						{
							dwVal = 0;
							memcpy((BYTE* )&dwVal, &pbDstBuf[31], 3);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25050200) //热表
						{
							dwVal = 0;
							memcpy((BYTE* )&dwVal, &pbDstBuf[25], 3);
							OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							dwVal = 0;
							memcpy((BYTE* )&dwVal, &pbDstBuf[28], 3);
							OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf+4);
							iLen = 8;
						}
						else if (dwOad == 0x25060200)
						{
							if ((pT188->bMtrType&0xf0) == 0x20) //热表//要注意与OIFmtDataExt（）里的处理别重复，后续再跟踪
							{
								pbDstBuf[0] = pbDstBuf[41];
								pbDstBuf[1] = (pbDstBuf[0] >> 2);
								pbDstBuf[0] &= 0x03; 
							}
							else
							{
								pbDstBuf[0] = pbDstBuf[19];
								pbDstBuf[1] = (pbDstBuf[0] >> 2);
								pbDstBuf[0] &= 0x03;
							}
							iLen = 2;
						}
						else
							iLen = 0;
					}
					break;
			}
        }
	}
	else if (ExtCode == DL645EXT_CMD_OTHER)
	{
		switch (pMtrPara->bSubProId)
		{
			case PROTOCOLNO_JSJD_MBUS:
				for (n=0; n<pMtrPro->pbRxBuf[DL645EXT_LEN]; n++)
				{
					if (pMtrPro->pbRxBuf[DL645Ext_DATA+n]==0x5A && pMtrPro->pbRxBuf[DL645Ext_DATA+n+1]==0xD9)
					{
						bPos = n + 6;
						break;
					}
				}
				if (wID == 0x901f) //0xfa00
				{
					BYTE bTmpBuf[3];
					memset(pbDstBuf, 0xee, 43);
					revcpy(bTmpBuf, &pMtrPro->pbRxBuf[DL645Ext_DATA+bPos], 2);
					wAskId = ByteToWord(bTmpBuf);
					if (dwOad == 0x25000200)
					{
						OoDWordToDoubleLongUnsigned(wAskId*100, pbDstBuf);
						iLen = 4;
					}
					else
						iLen = 0;
				}
				break;

			case PROTOCOLNO_JSLX_RS485:
				for (n=0; n<pMtrPro->pbRxBuf[DL645EXT_LEN]; n++)
				{
					if (pMtrPro->pbRxBuf[DL645Ext_DATA+n]==0x7e && pMtrPro->pbRxBuf[DL645Ext_DATA+n+1]==0x83)
					{
						bPos = n + 9;
						break;
					}
				}
				if (wID == 0x901f) //0xfa00
				{
					BYTE bTmpBuf[3];
					memset(pbDstBuf, 0xee, 43);
					revcpy(bTmpBuf, &pMtrPro->pbRxBuf[DL645Ext_DATA+bPos], 3);
					dwVal = ByteToDWORD(bTmpBuf, 3) / 1000;
					if (dwOad == 0x25000200)
					{
						OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
						iLen = 4;
					}
					else
						iLen = 0;
				}
				break;

			default:
				break;
		}
	}
	return iLen;
}

//描述：读取2007版645支持的数据标识的数据
int DL645ExtAskItem1(struct TMtrPro* pMtrPro, T645ExtTmp* pTmpV07, DWORD dwOad, WORD wID, BYTE* pbBuf)
{		
	int iRet, iLen;	
	BYTE eLen, n;	
	WORD wFrmLen, wTmpID = wID;
    BYTE bData[200];
    BYTE bMtrType;
    BYTE bSER = 0;

	TMtrPara* pMtrPara = pMtrPro->pMtrPara;	
	CComm* pComm = pMtrPro->pMtrPara->pComm;
	//TV07Priv* pMtrV07 = (TV07Priv* )pMtrPro->pvMtrPro;	
	BYTE* pbTxBuf = pMtrPro->pbTxBuf; 
	
	if ( !MtrProOpenComm(pMtrPro->pMtrPara->pComm, &pMtrPro->pMtrPara->CommPara) )
		return 0;
	
	for (n=0; n<TXRX_RETRYNUM; n++) //因为有续帧，所以重发需改成整帧重发
	{		
        //if (g_fDirRd[pMtrPro->bThrId] && !g_bDirRdStep)//tll
		//	return 0;

		pTmpV07->fRdNext = false;		

        bMtrType = GetMeterType(pMtrPara->wPn);
		switch (pMtrPara->bSubProId)
		{
			case PROTOCOLNO_HUAXU_T188_MBUS:
			case PROTOCOLNO_HUAXU_T188_RS485:
			case PROTOCOLNO_DENENG_T188:
			case PROTOCOLNO_BEILIN_T188:
				eLen = Make188AskItemFrm(bMtrType, pMtrPara->bMtrAddr, wID, bSER++, bData);
				wFrmLen = MakeExt645Frm(&pMtrPara->bAddr[1], DL645EXT_CMD_T188, bData, eLen, pbTxBuf);
				break;

			case PROTOCOLNO_JSJD_MBUS:
				eLen = MakeJsJdAskItemFrm(bMtrType, pMtrPara->bMtrAddr, wID, bSER++, bData);
				wFrmLen = MakeExt645Frm(&pMtrPara->bAddr[1], DL645EXT_CMD_OTHER, bData, eLen, pbTxBuf);
				break;

			case PROTOCOLNO_JSLX_RS485:
				eLen = MakeJsLxAskItemFrm(bMtrType, pMtrPara->bMtrAddr, wID, bSER++, bData);
				wFrmLen = MakeExt645Frm(&pMtrPara->bAddr[1], DL645EXT_CMD_OTHER, bData, eLen, pbTxBuf);
				break;

			default:
				wTmpID = ((wID>>8) | (wID<<8)); 
				eLen = Make188AskItemFrm(bMtrType, pMtrPara->bMtrAddr, wTmpID, bSER++, bData);
				wFrmLen = MakeExt645Frm(&pMtrPara->bAddr[1], DL645EXT_CMD_T188, bData, eLen, pbTxBuf);
				break;
		}

		//返回1：正确；0：通讯正常，无有效数据返回，-1：无任何数据返回，错包；
		iRet = DL645ExtTxRx(pMtrPro, pTmpV07, wFrmLen);

        iLen = pTmpV07->wRxDataLen;				
        if (iLen<0 || iLen >= MTR_FRM_SIZE)
        {
            DTRACE(DB_Ext645, ("AskItem1 : Reading break for wPn=%d, iLen=%d .\r\n", pMtrPara->wPn, iLen));            
            return 0;
        }

        if (iRet == 1)
        {
			memset(bData, 0, sizeof(bData));
			iRet = GetDataValT188(pMtrPro, dwOad, wID, bData);
			if (iRet > 0)
			{
				const ToaMap* pOI = GetOIMap(dwOad);
				if (pOI == NULL)
					return -1;
				
				return OIFmtDataExt(bData, iRet, pbBuf, pOI->pFmt, pOI->wFmtLen, dwOad);
			}
        }
        else if (iRet == 0)
	{
		continue; 
	}        
        else
        {
            DTRACE(DB_Ext645, ("DL645ExtAskItem1: Read data fail sleep 2s wPn=%d.\r\n", pMtrPara->wPn));  
            Sleep(2000);
		    pComm->Read(bData, sizeof(bData), 200);
            continue;         
        }		
	}
    return 0;
}

//读数据接口
int DL645ExtAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	int iRet;
	//BYTE bBuf[1024];
	Toad645Map* pOad645Map = GetOad645ExtMap(dwOAD);

	if (pOad645Map == NULL) //不支持的数据项
		return -2;

	if (bRespType == 1) //读实时数据
	{
		return DL645ExtAskItem(pMtrPro, dwOAD, pOad645Map->wID, pbData);

		//if (iRet > 0)
		//{
		//	return OIFmtData(bBuf, pbData, pOad645Map->pFmt, pOad645Map->wFmtLen);
		//}
	}
	else
	{
	}

	return -1;
}

int DL645ExtDirAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData)
{
	return -1;
}

//描述:扩展645组包 
int MakeExt645Frm(BYTE* pbAddr, BYTE bExtCode, const BYTE* pbDataArea, BYTE bDataAreaLen, BYTE* pbFrm)
{	
    int i;
    BYTE bCS = 0;
    BYTE bDataLen = 0;
    BYTE bOffset = 0;
    BYTE *pbFrmCur = pbFrm;    

    memset(pbFrmCur, 0xFE, bOffset);
    pbFrmCur += bOffset;

    *pbFrmCur++ = 0x68;                 //68
    memrcpy(pbFrmCur, pbAddr, 6);        //地址
    pbFrmCur += 6;           
    *pbFrmCur++ = 0x68;                 //68
    *pbFrmCur++ = 0x1F;                 //控制码,自定义
    *pbFrmCur++ = bDataAreaLen + 1;     //长度
    *pbFrmCur++ = bExtCode;             //扩展控制码   
    memcpy(pbFrmCur, pbDataArea, bDataAreaLen);//数据域
    pbFrmCur += bDataAreaLen;
   
    bDataLen = pbFrmCur - pbFrm - bOffset;
    for (i = 0; i < bDataLen; i++)
    {
        bCS += pbFrm[i + bOffset];
    }
    *pbFrmCur++ = bCS;
    *pbFrmCur++ = 0x16;
    return bDataLen + 2 + bOffset;
}	

BYTE Make188AskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm)
{	
    WORD i;
    BYTE bCS = 0;
    BYTE bOffset = 0;
    BYTE bDataLen;
    BYTE *pbFrmCur = pbFrm;    

    memset(pbFrmCur, 0xFE, bOffset);
    pbFrmCur += bOffset;

    *pbFrmCur++ = 0x68;  
    *pbFrmCur++ = bMtrType;   
    memrcpy(pbFrmCur, pbAddr, 7);      // 仪表地址
    pbFrmCur += 7;   

    *pbFrmCur++ = 0x01;  // 控制码
    if (dwID == 0xD23E)
    {
        *pbFrmCur++ = 0x09;  // 数据长度
    }
    else
    {
        *pbFrmCur++ = 0x03;  // 数据长度
    }

    *pbFrmCur++ = ((dwID >> 8) & 0xff);  // 901f 先发90 再发1f
    *pbFrmCur++ = (dwID & 0xff);

  //   memcpy(pbFrmCur, &dwID, 2);
  //   pbFrmCur += 2;

    *pbFrmCur++ = bSER;

    if (dwID == 0xD23E) // 只有0xD23E这个ID的数据域需要添加时间，其他的188协议里按标准的来
    {
        TTime now;
        GetCurTime(&now);
        *pbFrmCur++ = ByteToBcd(now.nSecond);
        *pbFrmCur++ = ByteToBcd(now.nMinute);
        *pbFrmCur++ = ByteToBcd(now.nHour);

        *pbFrmCur++ = ByteToBcd(now.nDay);
        *pbFrmCur++ = ByteToBcd(now.nMonth);
        *pbFrmCur++ = ByteToBcd(now.nYear - 2000);
    }

    bDataLen = pbFrmCur - pbFrm - bOffset;
    for (i = 0; i < bDataLen; i++)
    {
        bCS += pbFrm[i + bOffset];
    }
    *pbFrmCur++ = bCS;
    *pbFrmCur++ = 0x16;
    return bDataLen + 2 + bOffset;
}

//组江苏君达水表抄读帧
BYTE MakeJsJdAskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm)
{	
	BYTE bCS = 0;
	BYTE bOffset = 2;
	BYTE bDataLen;
	BYTE *pbFrmCur = pbFrm;    

	memset(pbFrmCur, 0xFE, bOffset);
	pbFrmCur += bOffset;

	*pbFrmCur++ = 0xA5;  
	*pbFrmCur++ = 0xD9; // 命令码
	*pbFrmCur++ = 0x04; //数据长度
	revcpy(pbFrmCur, pbAddr, 3);      // 仪表地址
	pbFrmCur += 3;   
	*pbFrmCur++ = 0x00;  

	//*pbFrmCur++ = ((dwID >> 8) & 0xff);  // 901f 先发90 再发1f
	//*pbFrmCur++ = (dwID & 0xff);

	bDataLen = pbFrmCur - pbFrm - bOffset;
	bCS = CRC8_Tab(&pbFrm[bOffset], bDataLen);
	*pbFrmCur++ = bCS;

	return bDataLen + 1 + bOffset;
}

//组江苏立信水表抄读帧
BYTE MakeJsLxAskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm)
{
	WORD i;
	DWORD dwAddr;
	BYTE bCS = 0;
	BYTE bOffset = 3;
	BYTE bDataLen;
	BYTE *pbFrmCur = pbFrm;

	memset(pbFrmCur, 0x7e, bOffset);
	pbFrmCur += bOffset;

	*pbFrmCur++ = 0x03; // 命令码
	memset(pbFrmCur, 0x00, 3);
	pbFrmCur += 3;
	dwAddr = BcdToDWORD(pbAddr, 3);
	revcpy(pbFrmCur, (BYTE* )&dwAddr, 3);      // 仪表地址
	pbFrmCur += 3;
	*pbFrmCur++ = 0x00; //数据长度

	bDataLen = pbFrmCur - pbFrm - bOffset;
	for (i = 0; i < bDataLen; i++)
	{
		bCS += pbFrm[i + bOffset];
	}
	*pbFrmCur++ = ~bCS + 1;
	*pbFrmCur++ = 0x0d;

	return bDataLen + 2 + bOffset;
}

// 抄失败是否超过3次
bool IsReadDataItem(WORD wPn)
{
	BYTE Num = 0;
	BYTE Flag = 0;   

	UpdateReadDataState();

	ReadItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);
	if (Flag == 1)    // 总标志已经抄读结束，不再去抄读；
	{
		return false;
	}

	//测量点抄读失败计数；
	ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);  
	if (Num >= 3)                        
	{
		return false;
	} 

	return true;
}

// 抄失败计数
void ReadDataFailCnt(WORD wPn)
{
	BYTE Num = 0;   

	//测量点抄读失败计数；
	ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
	if (Num >= 3)
	{
		return;
	}

	Num++;
	WriteItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);

	return;
}

// 更新总抄读状态
void UpdateReadDataState()
{
	static DWORD msLastClick = 0;

	BYTE Num = 0;
	BYTE Flag = 0;
	WORD wPn = 0;   

	//大于30s执行一次   
	if (GetClick() < msLastClick + 30) //30s也更新一下
	{
		return;
	}
	msLastClick = GetClick();


	ReadItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);
	if (Flag == 1)  // 总标志已经抄读结束，不再去抄读；
	{
		return;
	}  

	for (wPn = 0; wPn < POINT_NUM; wPn++)
	{       
		if(!IsPnValid(wPn))
		{
			continue;
		}

		if (wPn == POINT_NUM) //
		{
			break;
		}

		//测量点抄读失败计数；
		ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
		if (Num < 2) //小于2次，则返回。
		{
			return;
		}
	}  

	// 置总标志，本次时间段抄表失败重抄次数已经用完，至抄读总标志，并更新显示为“抄表结束”
	Flag = 1; 
	WriteItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);
	DTRACE(DB_Ext645, ("Pn all read end !!\r\n")); 
     
}

// 清除补抄抄读标志 
void CleanReadMeterFlag()
{  
	WORD wPn = 0;
	BYTE Num = 0;
	DWORD  Flag = 0;//tll BYTE Flag = 0;
	TTime now;

	GetCurTime(&now);
	if (now.nHour%2 != 0) //暂时2个小时补抄一次
		return;

	// 清除总标志位
	ReadItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);
	if (Flag != 0) //防止频繁写
	{
		Flag = 0;
		WriteItemEx(BN0, PN0, 0x400f, (BYTE*)(&Flag));
	}

	// 清除各测量点标志位
	for (wPn = 0; wPn < POINT_NUM; wPn++)
	{
		if(!IsPnValid(wPn))
		{
			continue;
		}

		ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
		if (Num != 0) //防止频繁写
		{
			Num = 0;
			WriteItemEx(BN0, wPn, 0x0c0f, (BYTE*)(&Num));
		}
	}
	DTRACE(DB_Ext645, ("clear all read flag ok!!\r\n")); 
    
}

void InitReadMeterFlg()
{
	WORD wPn = 0;
	BYTE Num = 0;
	DWORD Flag = 0;//tll 没找到0x400f在dbcfg中的定义，也不知到底是几个字节的   BYTE Flag = 0;
	
	// 清除总标志位
	ReadItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);

	if (Flag != 0) //防止频繁写
	{
		Flag = 0;
		WriteItemEx(BN0, PN0, 0x400f, (BYTE*)(&Flag));
	}

	// 清除各测量点标志位
	for (wPn = 0; wPn < POINT_NUM; wPn++)
	{
		if(!IsPnValid(wPn))
		{
			continue;
		}

		ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
		if (Num != 0) //防止频繁写
		{
			Num = 0;
			WriteItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
		}
	}
	DTRACE(DB_Ext645, ("Init all read flag ok!!\r\n"));
}
int DL645ExtpfnWriteItem(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen)
{
	return -1;

}



