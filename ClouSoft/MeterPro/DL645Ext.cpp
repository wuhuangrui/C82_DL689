/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DL645Ext.c
 * ժ    Ҫ�����ļ�����07��645����Э��Ĺ���ʵ��
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2015��10��
 * ��    ע��
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
#define	DL645V07_CMD_CTRL		0x1C	//����բ������������

//#define	DL645V07CMD_MAX			DL645V07CMD_EVENT_RESET
#define	DL645V07_CMD_GET	0x1f

#endif

/*******************************************************
********************************************************/ 

typedef struct sT188
{  
    BYTE Head1;      // 68
    BYTE bMtrType;   // �Ǳ�����T
    BYTE bAddrs[7];  // ��ַA0~A6
    BYTE bTCode;     // T188������C
    BYTE bLen;       // ����
    BYTE bData[];    // ������
}sT188_Type;


static TItemListExt g_IdInTo188Cfg[] = //IDת������
{
    { 0xfa00,  0x901f,   43,    43,   },  //ʵʱ����	
    { 0xfa01,  0xD120,   5,     5,    },  //����������    
    { 0,       0,        0,     0,    }   //----���ֱ��������----
};
#define  IDINTO188_NUM   (sizeof(g_IdInTo188Cfg) / sizeof(TItemListExt) - 1)
//07��645Э�����ú���

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

//��������ȡ97��645ID���ݱ�ʶ�����ݵĽӿ�
//��ע����Щ��ID���ִ���
int DL645ExtAskItem(struct TMtrPro* pMtrPro, DWORD dwOad, WORD wID, BYTE* pbBuf)
{
	int iRv;    
 
	//��֡��������ʱ����,ÿ�δ������¿�ʼ
	T645ExtTmp tTmpV07;
	memset(&tTmpV07, 0, sizeof(T645ExtTmp));	

	iRv = DL645ExtAskItem1(pMtrPro, &tTmpV07, dwOad, wID, pbBuf);

    return iRv;		
}

//��������ȡ����¼����ݵĽӿ�
int DL645ExtDirAskItem(struct TMtrPro* pMtrPro, DWORD dwID, BYTE* pbBuf)
{
	return -1;
}

//������������֤
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
		case 1:    //������ǰ������
			pbRxBuf[pTmpV07->wRxPtr++] = b;
			pTmpV07->wRxCnt --;
			if (pTmpV07->wRxCnt == 0)   //�����꣬����У��
			{
				if (/*memcmp(&pbRxBuf[1],&pbTxBuf[1],6)==0 &&*/ pbRxBuf[7]==0x68) //��ȥ�ж�ת������ַ�Ƿ���ȷ�����ˮ���ȱ���ڵ�ǰ
				{                                                                 //ת�����Ļ��������Ƿ���ȷ������ת������ַ��ת�������ܹ�ȥ
					pTmpV07->wRxDataLen = pbRxBuf[DL645EXT_LEN];                  //��������ת������Ӧ��֡������Լ��ĵ�ַ������Ҫ��ȷ���ж�Ӧ��֡
					pTmpV07->wRxCnt = pTmpV07->wRxDataLen + 2;
					pTmpV07->nRxStep = 2;
				}
				else
				{
					pTmpV07->nRxStep = 0;
				}
			}
			break;
		case 2:     //���� + ������ + ������
			pbRxBuf[pTmpV07->wRxPtr++] = b;
			pTmpV07->wRxCnt -- ;
			if (pTmpV07->wRxCnt == 0)   //�����꣬����У��
			{
				pTmpV07->nRxStep = 0;

				if (pbRxBuf[pTmpV07->wRxPtr-1]==0x16 && pbRxBuf[pTmpV07->wRxPtr-2]==CheckSum(pbRxBuf, pTmpV07->wRxDataLen+10))
				{
				//	for (i=10; i<10+pTmpV07->wRxDataLen; i++)
				//		pbRxBuf[i] -= 0x33;

					return true;    //���յ�������һ֡
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

//��������֡����
WORD DL645ExtMakeFrm(T645ExtTmp* pTmpV07, BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen)
{
	WORD i;	
	
	pbTxBuf[0] = 0x68;
	memcpy(&pbTxBuf[1], pbAddr, 6);
	pbTxBuf[7] = 0x68;
	pbTxBuf[8] = bCmd;	

	if (bCmd == DL645V07_CMD_ASK_NEXT)
	{	
		bLen ++; //����֡���
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

//������֡����
//����1����ȷ��0��ͨѶ����������Ч���ݷ��أ�-1�����κ����ݷ��أ������
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
	if (fReadSuccess)	//���յ�һ��������֡
	{
		if ((pbRxBuf[DL645EXT_CMD]&DL645V07_CMD_GET) == (pbTxBuf[DL645EXT_CMD]&DL645V07_CMD_GET))
		{		
            pTmpV07->fRdNext = false;

			if ((pbRxBuf[DL645EXT_CMD]-pbTxBuf[DL645EXT_CMD]) == 0x80)   //֡У����ȷ�������ش�
			{	               
                if(pbRxBuf[DL645EXT_EXTCMD]==DL645EXT_CMD_T188 || pbRxBuf[DL645EXT_EXTCMD]==DL645EXT_CMD_OTHER)   //֡У����ȷ
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
                else if(pbRxBuf[DL645EXT_EXTCMD] == 0xc8)   //֡У�鲻��ȷ
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
       /* if (memcmp(pT188->bAddrs, pMtrPara->bMtrAddr, 7) != 0) //Ӧ�����շ������жϵ�ַ
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

			//ˮ��0x901f�����ݴ���Ϊ����C1F188�ĸ�ʽ(43�ֽ�)
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
							dwCurVal = ByteToDWORD(pbDstBuf+1, 4);	//���ص�������16����,��λ��10��(1L=0.001m3)
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
						if (dwOad==0x25000200 || dwOad==0x25010200) //ˮ�������
						{
							memcpy((BYTE* )&dwVal, pbDstBuf, 4);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);//��OIFmtDataExt�������д���
							iLen = 4;
						}
						else if (dwOad == 0x25020200) //�ȱ�
						{
							memcpy((BYTE* )&dwVal, &pbDstBuf[5], 4);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25030200) //�ȱ�
						{
							memcpy((BYTE* )&dwVal, &pbDstBuf[10], 4);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25040200) //�ȱ�
						{
							dwVal = 0;
							memcpy((BYTE* )&dwVal, &pbDstBuf[31], 3);
							//OoDWordToDoubleLongUnsigned(dwVal, pbDstBuf);
							iLen = 4;
						}
						else if (dwOad == 0x25050200) //�ȱ�
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
							if ((pT188->bMtrType&0xf0) == 0x20) //�ȱ�//Ҫע����OIFmtDataExt������Ĵ�����ظ��������ٸ���
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

//��������ȡ2007��645֧�ֵ����ݱ�ʶ������
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
	
	for (n=0; n<TXRX_RETRYNUM; n++) //��Ϊ����֡�������ط���ĳ���֡�ط�
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

		//����1����ȷ��0��ͨѶ����������Ч���ݷ��أ�-1�����κ����ݷ��أ������
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

//�����ݽӿ�
int DL645ExtAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD)
{
	int iRet;
	//BYTE bBuf[1024];
	Toad645Map* pOad645Map = GetOad645ExtMap(dwOAD);

	if (pOad645Map == NULL) //��֧�ֵ�������
		return -2;

	if (bRespType == 1) //��ʵʱ����
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

//����:��չ645��� 
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
    memrcpy(pbFrmCur, pbAddr, 6);        //��ַ
    pbFrmCur += 6;           
    *pbFrmCur++ = 0x68;                 //68
    *pbFrmCur++ = 0x1F;                 //������,�Զ���
    *pbFrmCur++ = bDataAreaLen + 1;     //����
    *pbFrmCur++ = bExtCode;             //��չ������   
    memcpy(pbFrmCur, pbDataArea, bDataAreaLen);//������
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
    memrcpy(pbFrmCur, pbAddr, 7);      // �Ǳ��ַ
    pbFrmCur += 7;   

    *pbFrmCur++ = 0x01;  // ������
    if (dwID == 0xD23E)
    {
        *pbFrmCur++ = 0x09;  // ���ݳ���
    }
    else
    {
        *pbFrmCur++ = 0x03;  // ���ݳ���
    }

    *pbFrmCur++ = ((dwID >> 8) & 0xff);  // 901f �ȷ�90 �ٷ�1f
    *pbFrmCur++ = (dwID & 0xff);

  //   memcpy(pbFrmCur, &dwID, 2);
  //   pbFrmCur += 2;

    *pbFrmCur++ = bSER;

    if (dwID == 0xD23E) // ֻ��0xD23E���ID����������Ҫ���ʱ�䣬������188Э���ﰴ��׼����
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

//�齭�վ���ˮ����֡
BYTE MakeJsJdAskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm)
{	
	BYTE bCS = 0;
	BYTE bOffset = 2;
	BYTE bDataLen;
	BYTE *pbFrmCur = pbFrm;    

	memset(pbFrmCur, 0xFE, bOffset);
	pbFrmCur += bOffset;

	*pbFrmCur++ = 0xA5;  
	*pbFrmCur++ = 0xD9; // ������
	*pbFrmCur++ = 0x04; //���ݳ���
	revcpy(pbFrmCur, pbAddr, 3);      // �Ǳ��ַ
	pbFrmCur += 3;   
	*pbFrmCur++ = 0x00;  

	//*pbFrmCur++ = ((dwID >> 8) & 0xff);  // 901f �ȷ�90 �ٷ�1f
	//*pbFrmCur++ = (dwID & 0xff);

	bDataLen = pbFrmCur - pbFrm - bOffset;
	bCS = CRC8_Tab(&pbFrm[bOffset], bDataLen);
	*pbFrmCur++ = bCS;

	return bDataLen + 1 + bOffset;
}

//�齭������ˮ����֡
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

	*pbFrmCur++ = 0x03; // ������
	memset(pbFrmCur, 0x00, 3);
	pbFrmCur += 3;
	dwAddr = BcdToDWORD(pbAddr, 3);
	revcpy(pbFrmCur, (BYTE* )&dwAddr, 3);      // �Ǳ��ַ
	pbFrmCur += 3;
	*pbFrmCur++ = 0x00; //���ݳ���

	bDataLen = pbFrmCur - pbFrm - bOffset;
	for (i = 0; i < bDataLen; i++)
	{
		bCS += pbFrm[i + bOffset];
	}
	*pbFrmCur++ = ~bCS + 1;
	*pbFrmCur++ = 0x0d;

	return bDataLen + 2 + bOffset;
}

// ��ʧ���Ƿ񳬹�3��
bool IsReadDataItem(WORD wPn)
{
	BYTE Num = 0;
	BYTE Flag = 0;   

	UpdateReadDataState();

	ReadItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);
	if (Flag == 1)    // �ܱ�־�Ѿ���������������ȥ������
	{
		return false;
	}

	//�����㳭��ʧ�ܼ�����
	ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);  
	if (Num >= 3)                        
	{
		return false;
	} 

	return true;
}

// ��ʧ�ܼ���
void ReadDataFailCnt(WORD wPn)
{
	BYTE Num = 0;   

	//�����㳭��ʧ�ܼ�����
	ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
	if (Num >= 3)
	{
		return;
	}

	Num++;
	WriteItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);

	return;
}

// �����ܳ���״̬
void UpdateReadDataState()
{
	static DWORD msLastClick = 0;

	BYTE Num = 0;
	BYTE Flag = 0;
	WORD wPn = 0;   

	//����30sִ��һ��   
	if (GetClick() < msLastClick + 30) //30sҲ����һ��
	{
		return;
	}
	msLastClick = GetClick();


	ReadItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);
	if (Flag == 1)  // �ܱ�־�Ѿ���������������ȥ������
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

		//�����㳭��ʧ�ܼ�����
		ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
		if (Num < 2) //С��2�Σ��򷵻ء�
		{
			return;
		}
	}  

	// ���ܱ�־������ʱ��γ���ʧ���س������Ѿ����꣬�������ܱ�־����������ʾΪ�����������
	Flag = 1; 
	WriteItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);
	DTRACE(DB_Ext645, ("Pn all read end !!\r\n")); 
     
}

// �������������־ 
void CleanReadMeterFlag()
{  
	WORD wPn = 0;
	BYTE Num = 0;
	DWORD  Flag = 0;//tll BYTE Flag = 0;
	TTime now;

	GetCurTime(&now);
	if (now.nHour%2 != 0) //��ʱ2��Сʱ����һ��
		return;

	// ����ܱ�־λ
	ReadItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);
	if (Flag != 0) //��ֹƵ��д
	{
		Flag = 0;
		WriteItemEx(BN0, PN0, 0x400f, (BYTE*)(&Flag));
	}

	// ������������־λ
	for (wPn = 0; wPn < POINT_NUM; wPn++)
	{
		if(!IsPnValid(wPn))
		{
			continue;
		}

		ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
		if (Num != 0) //��ֹƵ��д
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
	DWORD Flag = 0;//tll û�ҵ�0x400f��dbcfg�еĶ��壬Ҳ��֪�����Ǽ����ֽڵ�   BYTE Flag = 0;
	
	// ����ܱ�־λ
	ReadItemEx(BN0, PN0, 0x400f, (BYTE*)&Flag);

	if (Flag != 0) //��ֹƵ��д
	{
		Flag = 0;
		WriteItemEx(BN0, PN0, 0x400f, (BYTE*)(&Flag));
	}

	// ������������־λ
	for (wPn = 0; wPn < POINT_NUM; wPn++)
	{
		if(!IsPnValid(wPn))
		{
			continue;
		}

		ReadItemEx(BN0, wPn, 0x0c0f, (BYTE*)&Num);
		if (Num != 0) //��ֹƵ��д
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



