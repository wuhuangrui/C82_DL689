#include "stdafx.h"
#include "SearchMeter.h"
#include "ComAPI.h"
#include "FaAPI.h"
#include "ComAPI.h"
#include "StdReader.h"
#include "DL69845.h"
#include "MtrCtrl.h"


TMtrSchInf g_tMtrRdSchInf[MTR_PORT_NUM];
const WORD g_dwTestID[] = {0x901f, 0x9010};
BYTE IsMetOrNot(BYTE bPort, BYTE bMetType, BYTE bAddrlen = 6);
int Make698FrmSend(WORD wPort, DWORD dwOAD, DWORD dwBaudRate, BYTE* pbMtrAddr, BYTE* pbRxFrm, BYTE bAddrLen);
BYTE SearchMeter(BYTE bPort, BYTE bMetType, BYTE bAddrLen = 6);

const TMeterPro g_tMeterPro[] = 
{//---Baud-------Proto---
    {CBR_1200, CCT_MTRPRO_97},
    {CBR_2400, CCT_MTRPRO_07},  
//	{CBR_1200, CCT_MTRPRO_T188},
	{CBR_9600, CCT_MTRPRO_69845},
};

int MtrT188RcvBlock(BYTE* pbBlock, DWORD dwLen, BYTE* pbRxBuf, BYTE* pbRxLen, DWORD dwBufSize,BYTE bMtrType)
{
	BYTE bRxPtr = 0;
	BYTE bRxCnt = 0;
	BYTE bRxStep = 0;
	short sFrmHead = -1;
	for (WORD i=0; i<dwLen; i++)
	{
		BYTE b = *pbBlock++;

		switch (bRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				pbRxBuf[0] = 0x68;
				bRxPtr = 1;
				bRxCnt = 11;       
				bRxStep = 1;
				sFrmHead = i;//��֮ǰ�����ݶ�����Ч��
			}
			break;
		case 1:    //������ǰ������
			pbRxBuf[bRxPtr++] = b;
			bRxCnt --;
			if (bRxCnt == 0)   //�����꣬����У��
			{
				if (pbRxBuf[1] == bMtrType)//�������ƥ��
				{
					bRxCnt = pbRxBuf[10] + 2;  //0xfe+2
					sFrmHead++;
					if (bRxCnt+11>dwBufSize || pbRxBuf[10]>=dwBufSize)   //��֡�Ļ���������
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

				if (pbRxBuf[bRxPtr-1]==0x16 && pbRxBuf[bRxPtr-2]==CheckSum(pbRxBuf, pbRxBuf[10]+11))
				{
					*pbRxLen = pbRxBuf[10] + 13;
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
	if 	(sFrmHead != -1)
		return -sFrmHead;

	return -(int)dwLen;

}



//�������Ӵ��ڻ�������������645�ı���
//������@pbBlock - ���ܵĻ���
//		@dwLen - ���յĳ���
//		@pbRxBuf - �����������֡�Ļ���
//		@pbRxLen - ��������֡���ĳ���
//		@dwBufSize - ���ջ������ĳ���
//���أ�0-�����ݣ�����-���յ�������֡���ȣ�����-��Ч���ݳ���
int Mtr645RcvBlock(BYTE* pbBlock, DWORD dwLen, BYTE* pbRxBuf, WORD* pbRxLen, DWORD dwBufSize)
{
	BYTE bRxPtr = 0;
	BYTE bRxCnt = 0;
	BYTE bRxStep = 0;
	short sFrmHead = -1;

	for (WORD i=0; i<dwLen; i++)
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
					if (/*bRxCnt+10>dwBufSize ||*/ pbRxBuf[9]>=dwBufSize)   //��֡�Ļ���������
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
					*pbRxLen = pbRxBuf[9] + 12;
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

	if 	(sFrmHead != -1)
		return -sFrmHead;

	return -(int)dwLen;
}	



//������������֤
//����>0: ���ܵĵ���ȷ֡
//    <0: 
int DL69845RcvBlock(BYTE* pbBlock, DWORD dwLen, BYTE* pbRxBuf, WORD* pwRxLen, DWORD dwBufSize)
{ //BYTE* pbBlock, DWORD dwLen, BYTE* pbRxBuf, BYTE* pbRxLen, DWORD dwBufSize
	WORD i, wCrc;
	BYTE b; 
	T698Tmp Tmp698;
	short sFrmHead = -1;
//	BYTE* pbTxBuf = pMtrPro->pbTxBuf; 
	memset(&Tmp698, 0, sizeof(Tmp698));

	for (WORD wLen = 0; wLen < dwLen; wLen++)
	{
		b = *pbBlock++;

		switch (Tmp698.nRxStep) 
		{
		case 0:   //0x68
			if (b == 0x68)
			{
				memset(pbRxBuf, 0, dwBufSize);
				pbRxBuf[0] = 0x68;
				Tmp698.wRxPtr = 1;
				Tmp698.wRxCnt = 4;
				Tmp698.nRxStep = 1;
				sFrmHead = wLen;//��֮ǰ�����ݶ�����Ч��
			}
			break;
		case 1:    //��ַ��ǰ������
			pbRxBuf[Tmp698.wRxPtr++] = b;
			Tmp698.wRxCnt --;
			if (Tmp698.wRxCnt == 0)   //�����꣬����У��
			{
				if ((pbRxBuf[DL69845_CTL_POS]&0x07) == ((DL698_CTL_DIR_CLI | DL698_CTL_PRM_CLI | DL698_CTL_AFN_USERDATA)&0x07))
				{
					memcpy(&Tmp698.wRxDataLen, &pbRxBuf[DL69845_LEN_POS], 2);
					Tmp698.wRxCnt = (pbRxBuf[DL69845_TSA_POS]&0x0f) + 4;
					Tmp698.nRxStep = 2;
					if (Tmp698.wRxDataLen+2 > dwBufSize)
					{
						Tmp698.nRxStep = 0;
						break;
					}
				}
				else
				{
					Tmp698.nRxStep = 0;
				}
			}
			break;
		case 2:    //������ǰ������
			pbRxBuf[Tmp698.wRxPtr++] = b;
			Tmp698.wRxCnt --;
			if (Tmp698.wRxCnt == 0)   //�����꣬����У��
			{
				wCrc = CheckCrc16(pbRxBuf+1, Tmp698.wRxPtr - 3);
				if (/*memcmp(&pbTxBuf[DL69845_TSA_POS], &pbRxBuf[DL69845_TSA_POS], pTmp698.wRxPtr - 8)==0 && */(pbRxBuf[Tmp698.wRxPtr-2]==(wCrc&0xff)) && (pbRxBuf[Tmp698.wRxPtr-1]==((wCrc>>8)&0xff)))
				{
					Tmp698.wRxCnt = Tmp698.wRxDataLen + 2 - Tmp698.wRxPtr;
					Tmp698.wRxAPDUPos = Tmp698.wRxPtr;
					Tmp698.nRxStep = 3;
				}
				else
				{
					Tmp698.nRxStep = 0;
					sFrmHead++;
				}
			}
			break;
		case 3:     //���� + ������ + ������
			pbRxBuf[Tmp698.wRxPtr++] = b;
			Tmp698.wRxCnt -- ;
			if (Tmp698.wRxCnt == 0)   //�����꣬����У��
			{
				Tmp698.nRxStep = 0;

				wCrc = CheckCrc16(pbRxBuf+1, Tmp698.wRxPtr - 4);
				if (pbRxBuf[Tmp698.wRxPtr-1]==0x16 && (pbRxBuf[Tmp698.wRxPtr-3]==(wCrc&0xff)) && (pbRxBuf[Tmp698.wRxPtr-2]==((wCrc>>8)&0xff)) /*&& (pbRxBuf[Tmp698.wRxAPDUPos]&0x7f)==pbTxBuf[Tmp698.wRxAPDUPos]*/)
				{
					Tmp698.wRxAPDULen = Tmp698.wRxPtr - Tmp698.wRxAPDUPos - 3;
					*pwRxLen = Tmp698.wRxDataLen+2;
					return dwLen;    //���յ�������һ֡
				}
				else
				{
					sFrmHead++;
				}
			}
			break;
		default:
			Tmp698.nRxStep = 0;
			break;
		} //switch (pTmp698.nRxStep)
	}

	if 	(sFrmHead != -1)
		return -sFrmHead;

	return -(int)dwLen;
}

static void InitStack(TStack *ptStack)
{
    ptStack->iTop = 0;                   //��ѹջ���ƶ������ƶ����ջ��
}

static BYTE IsStackEmpty(TStack *ptStack)
{
    if (ptStack->iTop == 0)
        return 1;             //��
    return 0;
}

static BYTE PushStack(TStack *ptStack, BYTE bData)
{
    if ((ptStack->iTop+1) == STACK_SIZE+EXT_STACK_SIZE)          //ջ���� ������ջ
    {
        return 0;
    }
    else
    {
        ptStack->bData[ptStack->iTop] = bData;
        ptStack->iTop++;
        return 1;
    }
}

static BYTE PopStack(TStack *ptStack, BYTE *pbData)
{
    if (IsStackEmpty(ptStack))           //��ջ
    {
        return 0;
    }
    else
    {
        ptStack->iTop--;
        *pbData = ptStack->bData[ptStack->iTop];
        return 1;
    }
}

static int DepthStack(TStack *ptStack)
{
    return ptStack->iTop;
}

//ȡ��ջ��Ԫ�ص����ǵ���
static BYTE GetStackTop(TStack *ptStack, BYTE *pbData)
{
    if (IsStackEmpty(ptStack))           //��ջ
    {
        return 0;
    }
    else
    {
        *pbData = ptStack->bData[ptStack->iTop-1];
        return 1;
    }
}

void InitSearch(BYTE bPort, BYTE bStartSer)
{    
	if (bPort >= MTR_PORT_NUM)
		return;

    //BYTE bBuf[2];
    //memset(g_tMtrRdSchInf[bPort].tMeterAddrTab, 0, sizeof(g_tMtrRdSchInf[bPort].tMeterAddrTab));
    memset(g_tMtrRdSchInf[bPort].bAddrPatten, 0xAA, sizeof(g_tMtrRdSchInf[bPort].bAddrPatten));
    //ʹ��������ܣ�
    //ReadPara(0x00000025, 0, bBuf, sizeof(bBuf)); 
    //if (*bBuf == 1)  //ʹ���ѱ���
    //    g_tMtrRdSchInf[bPort].bSearchState = PRO07METORNOT; 
    //else
    {
        if (bStartSer)
            g_tMtrRdSchInf[bPort].bSearchState = PRO07METORNOT;
        else
            g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;                  //��ʼ����һ���ѱ�
    }
    InitStack(&g_tMtrRdSchInf[bPort].tStack);
    //g_tEventRept.bMtrAddrNum = 0;                  //Ҫ�����ϱ�
    //g_tEventRept.bMtrAddrCnt = 0;
    
    g_tMtrRdSchInf[bPort].bFinish = 0;
    g_tMtrRdSchInf[bPort].bCurTry = 0;
    g_tMtrRdSchInf[bPort].bCurTryLevel = 0;
}

void ReinitSearch(BYTE bPort)
{
	if (bPort >= MTR_PORT_NUM)
		return;

    memset(g_tMtrRdSchInf[bPort].bAddrPatten, 0xAA, sizeof(g_tMtrRdSchInf[bPort].bAddrPatten));
    InitStack(&g_tMtrRdSchInf[bPort].tStack);
    g_tMtrRdSchInf[bPort].bFinish = 0;
    g_tMtrRdSchInf[bPort].bCurTry = 0;
    g_tMtrRdSchInf[bPort].bCurTryLevel = 0;
}



//����±��ַ�Ƿ���Ч
//����1-��Ч��0-��Ч
BYTE CheckMetAddr(BYTE *pbMAC)
{
    BYTE i;
    for (i=0; i<6; i++)
    {
        if (pbMAC[i] > 0x99)
            return 0;
    }
    return 1;
}


void AddMetToTab(TMtrSchInf* ptMtrSch, BYTE *pbMAC, BYTE bAddrLen, BYTE bPro, BYTE bBaud)
{
    BYTE i, bGbPro;
	TTime tmNow;

	GetCurTime(&tmNow);
    if ((bPro==CCT_MTRPRO_97) || (bPro==CCT_MTRPRO_07))
    {
        if (!CheckMetAddr(pbMAC))
            return;
    }

	if (bPro == CCT_MTRPRO_97)
		bGbPro = PROTOCOLNO_DLT645;

	if (bPro == CCT_MTRPRO_07)
		bGbPro = PROTOCOLNO_DLT645_V07;

	if (bPro == CCT_MTRPRO_69845)
		bGbPro = PROTOCOLNO_DLT69845;
	
	SaveSearchPnToDb(pbMAC, bAddrLen, bGbPro, tmNow, bBaud);	
}

//��ַ��������
BYTE g_bAddrLen[] = {6, 7, 5, 8, 9, 10, 11, 12, 4, 3};


//void SaveSearchPnToDb(BYTE* pbMtrAddr, BYTE bPro, TTime tmNow, BYTE bPort);

//68 AA AA AA AA AA AA 68 01 02 43 C3 D5 16 //�Թ㲥��ַ��43 C3         9010
//68 AA AA AA AA AA AA 68 81 06 43 C3 94 A5 35 33 FA 16 //��Щ97�ı�صĻ���ͨ���ַ
//���ֱ����º����Ѳ������ַ
//bSubMetType ��������ͣ������������Ϊ�����T188Э����ı�����Ͳ���������Э�鲻�øò��� 
//���� 0-û�б�
//     1- 1����
//     2- �����
BYTE IsMetOrNot(BYTE bPort, BYTE bMetType, BYTE bAddrLen)
{    
    BYTE bMAC[16]; 
    BYTE bRxBuf[256];
	BYTE bRxFrm[100];
	WORD wLen;   
    BYTE bMetNum = 0;
    short sRet = -1;
	int	iLen;

	if (bMetType >= sizeof(g_tMeterPro)/sizeof(TMeterPro))
		return 0;   

	WORD wPortNum, wPortMin, wPortMax;
	GetLogicPortNum(&wPortNum, &wPortMin, &wPortMax);
   
    if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_69845)
	{
		memset(bMAC+1, 0xaa, bAddrLen);
	}
	else
		memset(bMAC, 0xaa, bAddrLen);

	for (BYTE i=0; i<sizeof(g_dwTestID)/sizeof(WORD); i++)  //�������Ե�ID���ID
	{
	    for (BYTE j=0; j<2; j++)           //���������α������
	    {         
			memset(bRxBuf, 0, sizeof(bRxBuf));
			memset(bRxFrm, 0, sizeof(bRxFrm));
			if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_69845)
			{
			//	iLen = DoMtrFwdFunc(bPort+wPortMin, g_dwTestID[i], g_tMeterPro[bMetType].bProto, bMAC, bRxBuf);
				//bMAC[0] |= (0x10|((bAddrLen&0x0f)-1));
				bMAC[0] = bAddrLen;
				//bMAC[bAddrLen] = 0;//ca
				ReverBuff(&bMAC[1], bAddrLen);
				iLen = Make698FrmSend(bPort+wPortMin, 0x40000200, CBR_9600, bMAC, bRxBuf, bAddrLen);
				//return 0;
			}
			else
			{
				iLen = DoMtrFwdFunc(bPort+wPortMin, g_dwTestID[i], g_tMeterPro[bMetType].bProto, bMAC, bRxBuf);
			}
		

			if (iLen == -1) //�˿ڲ�Ϊ�����
				break;
			else if (iLen <= 0) //��������   
				continue;

			BYTE bPos = 0;

			if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_69845)
			{
				//sRet = MtrT188RcvBlock(bRxBuf, DWORD(iLen), bRxFrm, &bLen, 100, bSubMetType);//Ĭ����ˮ��10������
				sRet = DL69845RcvBlock(bRxBuf, DWORD(iLen), bRxFrm, &wLen, 100);
				bPos = 5;
			}
			else
			{
				sRet =  Mtr645RcvBlock(bRxBuf, DWORD(iLen), bRxFrm, &wLen, 100);
				bPos = 1;
			}
	        if (sRet > 0)
	        {       
				if (sRet > wLen+4) //�����Ϊ֡ͷ���4���ֽ�  //��һ�³��ȣ���ѭ����������֡���滹��û�����ݣ�������
	            {
	                bMetNum |= 2;
	            }
	            else   //���ʱ���ﲻ��ӣ���Ϊ��ỹ����ң�������ظ�
	            {
	                AddMetToTab(&g_tMtrRdSchInf[bPort], bRxFrm+bPos, bAddrLen, g_tMeterPro[bMetType].bProto, ((bPort+wPortMin)&0x1f)+ (BaudrateToGbNum(g_tMeterPro[bMetType].wBaud)<<5));//todo:ע��һ����Ȼأ�����������ص����
	                bMetNum |= 1;
	            }
	        }
	        else if (sRet < 0)//û����֡             ֡ͷ����
	        {   
	            bMetNum |= 2;
	        }	        	        	
	        //sRet == 0  û�б�       
	    }
	    
	    if (bMetNum != 0) //��ǰID����Ӧ,��������һ��ID        
	     	break;		
	}

    if (bMetNum >= 2) //����
        return 2;
        
    return bMetNum;
}


//�ѱ�ӵ�ַ���ֽ���߳��ԣ���һ�����Եĵ�ַΪAA AA AA AA AA 00�������������Ӧ�ý�00ѹ��ջ�У�
//Ȼ����AA AA AA AA 00 00��AA AA AA AA 01 00��...
//6���ֽڵĵ�ַ��5��������ֽڽ�0������0����ʼ���ԡ�ÿ���ֽڵĵ�ֵַֻ����0-99.
//ջ�յ�ʱ���Ե�0����ջ���Ϊ1ʱ���Ե�Ϊ1�������ջ����ȿ���֪����ǰӦ�ó�����һ����
//ջ��ȼ�1ʱ����ֵַӦ��������0��ʼֱ��99.����99��ʾ�ü���������Ե���һ��ջ��Ԫ�أ�ջ��Ԫ�صĴ�С��1����
//�ü�Ӧ�ÿ�ʼ���Եĵ�ֵַ����ʼֵ��
//��������
//g_tMtrRdSchInf[bPort].bCurTry����ǰ���Լ��ĵ�ֵַ���仯��Χ��0-99
//g_tMtrRdSchInf[bPort].bCurTryLevel����¼���𡣱仯��Χ0-5�������ж��Ƿ���ѹջ
//g_tMtrRdSchInf[bPort].bAddrPatten����Ҫ���Եı�ַ����ã�����g_tMtrRdSchInf[bPort].bAddrPatten�С�����ջ����2���ֽ�02��01��Ln��ʾջ�Ķ�Ӧλ��û��Ԫ�أ� L7 L6 L5 L4 L3 L2 02 01��
//��ô��ǰ����λΪL2��L2�ǵ�ֵ��0��ʼ��99���ԡ�g_tMtrRdSchInf[bPort].bAddrPatten��ΪֵΪ��AA AA AA g_tMtrRdSchInf[bPort].bCurTry 02 01
//��ȡһ�����Գ����ĵ���ַ�����ص�ǰ������һ�㣨Ҳ���ǵ���ַ�ĵڼ����ֽڣ�����
BYTE GetTryAddr(TMtrSchInf* pMtrSch, BYTE *pbMtrAddr)
{        
    BYTE bNode;
    BYTE bLevel = 0;
    if (IsStackEmpty(&pMtrSch->tStack))
    {
    	memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel); 
        pMtrSch->bAddrPatten[0] = ByteToBcd(pMtrSch->bCurTry++);
        if (pMtrSch->bCurTry > 100)//////     ���б��ַ�������ꡣ
            pMtrSch->bFinish = 1; 
    }
    else
    {
        GetStackTop(&pMtrSch->tStack, &bNode);     
        bLevel = (BYTE)DepthStack(&pMtrSch->tStack);	
        if (bLevel > pMtrSch->bCurTryLevel)      //ѹ��ջ               //bCurTryLevel
        {       //BYTE SearchMeter()ѹ�����µ��ֽڣ�pMtrSch->bCurTryLevelǰ��һ����������
            if (bLevel >= 6)//˵���õ�ַ��������������ַһģһ��
            {
                //���õ�ַ��ջ��ȡ��������Ҫ�ƶ�ջ           todo:�������ø澯�¼�
                PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry);     //�����ñ�
                bLevel--;
                pMtrSch->bCurTry++;     
                while (pMtrSch->bCurTry > 99)
                {
                    if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//����,      ȫ99Ϊ�㲥��ַ��������
                    {
                        pMtrSch->bFinish = 1; 
                        break;
                    }
                    pMtrSch->bCurTry++;        //�ڵ�������ֵ�����ϼ�1������
                    pMtrSch->bCurTryLevel--;
                    bLevel--;
                }                     
            }
            else
            {
                pMtrSch->bCurTry = 0;
                pMtrSch->bAddrPatten[bLevel-1] = ByteToBcd(bNode); //ջָ��Ϊ1��ʱ��ջ��ֻ��һ��Ԫ��
                pMtrSch->bCurTryLevel++;
            }
        }
        else if (bLevel == pMtrSch->bCurTryLevel)    //��ǰ���Լ�����ֽڣ�0~99�ص���
        {
            while (pMtrSch->bCurTry > 99)  //������ֽ��ϣ�û��ͻ�ı�����Ѿ��ѳ����ˣ�����֮ǰ�г�ͻ�Ķ��Ѿ�ѹջ������
            {
                if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//����,      ȫ99Ϊ�㲥��ַ��������
                {
                    pMtrSch->bFinish = 1; 
                    break;
                }
                pMtrSch->bCurTry++;        //�ڵ�������ֵ�����ϼ�1������
                pMtrSch->bCurTryLevel--;
                bLevel--;
            }        
        }
        else         //
        {            
        }

        //���룺��ǰ���Լ�����ֽ�(0~99)��������Ĺ㲥�ֽ�
        memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel);         //��ջʱҪ���˳���λ����0xAA
        pMtrSch->bAddrPatten[bLevel] = ByteToBcd(pMtrSch->bCurTry++);        
    }    
    memcpy(pbMtrAddr, pMtrSch->bAddrPatten, 6);   
    return bLevel;
}

//��698.45�е�ַ��ָ���������ΪbSetData���԰���Ϊ��λ,bSetData���ܴ���0x0f
//bOffset�ֽ�ƫ�ƣ�Ĭ���Ǵ��ֽڵĸ�λ��ʼ������Ҳ�д��ֽڵĵ�λ��ʼ�ġ�
//bOffset : ȡֵ0����1
/**************************************************
________________________________________________
|    1  |    2     |     3      |      4      |
|_______|__________|____________|_____________|
|    ��һ���ֽ�    |       �ڶ����ֽ�         | 

1 ��1��ʼ��� ��������ż�������ֽڳ���
2 ��2��ʼ��� ��������ż�������ֽڳ���
****************************************************/
void Clear698Addr(BYTE *bAddr, BYTE bOffset, BYTE bSetData, BYTE bLen)
{
	if ((bAddr != NULL) && (bSetData <= 0x0f) && (bOffset < 2))
	{
		if (bOffset > 0)//����ͼ��1��ʼ���
		{
			bAddr[0] = (bAddr[0]&0xf0) | (bSetData);
		}
		if (bLen%2)//������������ֽڳ���
		{
			memset(bAddr+bOffset, (bSetData<<4)|bSetData, bLen/2);
			if (bOffset == 0)
			{
				bAddr[bLen/2+bOffset] = (bSetData<<4) | (bAddr[bLen/2] &0x0f);//
			}



		}
		else//���ż�������ֽڳ���
		{
			memset(bAddr+bOffset, bSetData<<4|bSetData, bLen/2);
			if (bOffset > 0)
			{
				bAddr[bLen/2+bOffset] = (bSetData<<4) | (bAddr[bLen/2] &0x0f);//
			}
		}
	}
}

//bAddrLen: ��ַ����
//bMtrType: ���Э������
BYTE GetTryAddr(TMtrSchInf* pMtrSch, BYTE *pbMtrAddr, BYTE bAddrLen, BYTE bMtrType)
{
	BYTE bNode;
	BYTE bLevel = 0;
	if ((bMtrType != CCT_MTRPRO_69845) || (bAddrLen > 12) || (bAddrLen < 3))//�ٶ���Ϊ����3���ֽڵĵ�ַ�����ڣ�����12���ֽڵĵ�ַ������
	{
		return 0;
	}
	if (IsStackEmpty(&pMtrSch->tStack))
	{
		Clear698Addr(&pMtrSch->bAddrPatten[bLevel/2], bLevel%2, 0x0a, bAddrLen*2-bLevel);
		Clear698Addr(&pMtrSch->bAddrPatten[0], 0, pMtrSch->bCurTry++, 1);
		if (pMtrSch->bCurTry > 10)//////     ���б��ַ�������ꡣ
			pMtrSch->bFinish = 1; 
	}
	else
	{
		GetStackTop(&pMtrSch->tStack, &bNode);     
		bLevel = (BYTE)DepthStack(&pMtrSch->tStack);	
		if (bLevel > pMtrSch->bCurTryLevel)      //ѹ��ջ               //bCurTryLevel
		{       //BYTE SearchMeter()ѹ�����µ��ֽڣ�pMtrSch->bCurTryLevelǰ��һ����������
			if (bLevel >= bAddrLen * 2)//˵���õ�ַ��������������ַһģһ��
			{
				//���õ�ַ��ջ��ȡ��������Ҫ�ƶ�ջ           todo:�������ø澯�¼�
				PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry);     //�����ñ�
				bLevel--;
				pMtrSch->bCurTry++;     
				while (pMtrSch->bCurTry > 9)
				{
					if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//����,      ȫ99Ϊ�㲥��ַ��������
					{
						pMtrSch->bFinish = 1; 
						break;
					}
					pMtrSch->bCurTry++;        //�ڵ�������ֵ�����ϼ�1������
					pMtrSch->bCurTryLevel--;
					bLevel--;
				}                     
			}
			else
			{
				pMtrSch->bCurTry = 0;
			//	pMtrSch->bAddrPatten[bLevel-1] = ByteToBcd(bNode); //ջָ��Ϊ1��ʱ��ջ��ֻ��һ��Ԫ��
				Clear698Addr(&pMtrSch->bAddrPatten[(bLevel-1)/2], (bLevel-1)%2, ByteToBcd(bNode), 1);
				pMtrSch->bCurTryLevel++;
			}
		}
		else if (bLevel == pMtrSch->bCurTryLevel)    //��ǰ���Լ�����ֽڣ�0~99�ص���
		{
			while (pMtrSch->bCurTry > 9)  //������ֽ��ϣ�û��ͻ�ı�����Ѿ��ѳ����ˣ�����֮ǰ�г�ͻ�Ķ��Ѿ�ѹջ������
			{
				if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//����,      ȫ99Ϊ�㲥��ַ��������
				{
					pMtrSch->bFinish = 1; 
					break;
				}
				pMtrSch->bCurTry++;        //�ڵ�������ֵ�����ϼ�1������
				pMtrSch->bCurTryLevel--;
				bLevel--;
			}        
		}
		else         //
		{            
		}

		//���룺��ǰ���Լ�����ֽ�(0~99)��������Ĺ㲥�ֽ�
	//	memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel);         //��ջʱҪ���˳���λ����0xAA
		Clear698Addr(&pMtrSch->bAddrPatten[bLevel/2], bLevel%2, 0xa, 2*bAddrLen-bLevel);
		Clear698Addr(&pMtrSch->bAddrPatten[bLevel/2], bLevel%2, ByteToBcd(pMtrSch->bCurTry++), 1);
		///pMtrSch->bAddrPatten[bLevel] = ByteToBcd(pMtrSch->bCurTry++);        
	}    
	memcpy(pbMtrAddr, pMtrSch->bAddrPatten, bAddrLen);   
	return bLevel;
}

void ReverBuff(BYTE *pbBuf, WORD bLen)
{

	BYTE bBuff = 0;
	BYTE *pBuf0 = pbBuf;
	BYTE *pBuf1 = pbBuf+bLen-1;
	if (bLen < 2)
	{
		return;
	}
	while(pBuf0 < pBuf1)
	{
		bBuff = *pBuf0;
		*pBuf0 = *pBuf1;
		*pBuf1 = bBuff;
		pBuf0++;
		pBuf1--;

	}
	return;
}

int Make698FrmSend(WORD wPort, DWORD dwOAD, DWORD dwBaudRate, BYTE* pbMtrAddr, BYTE* pbRxFrm, BYTE bAddrLen)
{
	//DWORD dwID;
	BYTE bCnt = 0;
	BYTE bFrmLen;
	int iLen = 0, iPort;
	BYTE bCmdFrm[256];
	BYTE bBuf[128];
	TCommPara tCommPara;

	if (GetLogicPortFun(wPort) != PORT_FUN_RDMTR) //-1
	{
		DTRACE(DB_METER, ("DoMtrFwdFunc: wPort=%d is not PORT_FUN_RDMTR!\r\n", wPort));
		return -1;	
	}

	iPort = wPort;
	tCommPara.dwBaudRate = dwBaudRate;
	tCommPara.wPort = wPort;
	tCommPara.bByteSize = 8;
	tCommPara.bStopBits = ONESTOPBIT;
	tCommPara.bParity = EVENPARITY;
	//0x40000200
	WORD wAPDULen = GetRequestNormal(dwOAD, bCmdFrm+8+bAddrLen);
	WORD wFrmLen = DL69845MakeFrm(0, pbMtrAddr, bCmdFrm, bCmdFrm+8+bAddrLen, wAPDULen);
	iLen = MtrDoFwd(tCommPara, bCmdFrm, wFrmLen, bBuf, sizeof(bBuf), 900, 10);
	if (iLen <= 0)
		return -13;
	memcpy(pbRxFrm, bBuf, iLen);
	return iLen;
}

BYTE SearchMeter(BYTE bPort, BYTE bMetType, BYTE bAddrLen)
{
	BYTE bMtrAddr[17]; 
	BYTE bRxBuf[256];
	BYTE bRxFrm[100];
	WORD wLen;	
	BYTE bMutiMet = 0;
	short sRet = -1;
	int	iLen;     

	WORD wPortNum, wPortMin, wPortMax;
	GetLogicPortNum(&wPortNum, &wPortMin, &wPortMax);

	if (bMetType >= sizeof(g_tMeterPro)/sizeof(TMeterPro))
		return SEARCH_UNDOEN;
  

	if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_69845)
	{
		GetTryAddr(&g_tMtrRdSchInf[bPort], bMtrAddr+1, bAddrLen, CCT_MTRPRO_69845);
	}
	else
		GetTryAddr(&g_tMtrRdSchInf[bPort], bMtrAddr);            //todotodo:��Э��   
	if (g_tMtrRdSchInf[bPort].bFinish)
        return SEARCH_OVER; 

 
	for (BYTE i=0; i<1; i++)  //�ظ�����
	{
		for (BYTE j=0; j<1; j++)  //�ظ�����
		{
			memset(bRxBuf, 0, sizeof(bRxBuf));
			memset(bRxFrm, 0, sizeof(bRxFrm));

			if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_69845)
			{
				//	iLen = DoMtrFwdFunc(bPort+wPortMin, g_dwTestID[i], g_tMeterPro[bMetType].bProto, bMAC, bRxBuf);
				bMtrAddr[0] = bAddrLen;
				ReverBuff(&bMtrAddr[1], bAddrLen);
				iLen = Make698FrmSend(bPort+wPortMin, 0x40000200, CBR_9600, bMtrAddr, bRxBuf, bAddrLen);
				//return 0;
			}
			else
			{
				iLen = DoMtrFwdFunc(bPort+wPortMin, g_dwTestID[i], g_tMeterPro[bMetType].bProto, bMtrAddr, bRxBuf);
			}			
			if (iLen == -1) //�˿ڲ�Ϊ�����
				return SEARCH_UNDOEN;//break;
			else if (iLen <= 0) //��������   
				continue;
			BYTE bPos = 1;

			if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_69845)
			{
				//sRet = MtrT188RcvBlock(bRxBuf, DWORD(iLen), bRxFrm, &bLen, 100, bSubMetType);//Ĭ����ˮ��10������
				sRet = DL69845RcvBlock(bRxBuf, DWORD(iLen), bRxFrm,  &wLen, 100);
				bPos = 5;
			}else
			{
				sRet =  Mtr645RcvBlock(bRxBuf, DWORD(iLen), bRxFrm, &wLen, 100);
				bPos = 1;
			}


			if (sRet > 0)
			{       
				if (sRet > wLen+4) //�����Ϊ֡ͷ���4���ֽ�  //��һ�³��ȣ���ѭ����������֡���滹��û�����ݣ�������
				{			
					if (bMutiMet == 0)   //ͬһ���ַ�ظ����β���ÿ�ζ�ѹջ����ֻ��ѹһ�Ρ�
					{
						if (!PushStack(&g_tMtrRdSchInf[bPort].tStack, g_tMtrRdSchInf[bPort].bCurTry-1))          //���г�ͻ��ѹ��ջ��
							return SEARCH_ERROR;
						bMutiMet = 1;
					}
				}			
				AddMetToTab(&g_tMtrRdSchInf[bPort], bRxFrm+bPos, bAddrLen, g_tMeterPro[bMetType].bProto, ((bPort+wPortMin)&0x1f)+ (BaudrateToGbNum(g_tMeterPro[bMetType].wBaud)<<5));//todo:ע��һ����Ȼأ�����������ص����
				//if (GetMtrNumByPort(bPort+wPortMin) >= DEF_MTR_NUM_PER_PORT)      //�Ѿ��ҵ�32�������Ѱ�ҡ�
				//	return SEARCH_OVER;
			}      
			else if (sRet < 0)//û����֡             ֡ͷ����
			{            
				if (bMutiMet == 0)
				{
					if (!PushStack(&g_tMtrRdSchInf[bPort].tStack, g_tMtrRdSchInf[bPort].bCurTry-1))          //���г�ͻ��ѹ��ջ��
						return SEARCH_ERROR;
					bMutiMet = 1;
				}
			} 
			//sRet == 0  û�б�
		}

		if (bMutiMet != 0) //��ǰID����Ӧ,��������һ��ID        
			break;		
	}
    return SEARCH_UNDOEN;
}

void DoSearch(BYTE bPort)
{
   
    BYTE bSerState;  
	static BYTE bCnt[MTR_PORT_NUM] = {0};

	if (bPort >= MTR_PORT_NUM)
		return;
   
    switch(g_tMtrRdSchInf[bPort].bSearchState)
    {
    case PRO07METORNOT:        
        if (IsMetOrNot(bPort, 1) >= 2)//�Ƿ��ж��07�ı�
            g_tMtrRdSchInf[bPort].bSearchState = PRO07;
        else
            g_tMtrRdSchInf[bPort].bSearchState = PRO97METORNOT;
        break;
    case PRO07:              //07Э���ѱ�
        bSerState = SearchMeter(bPort, 1);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//07����
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = PRO97METORNOT;
            }
            else //����32���
                g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
        }        
        break;
    case PRO97METORNOT:        
        if (IsMetOrNot(bPort, 0) >= 2)//�Ƿ��ж��97�ı�
            g_tMtrRdSchInf[bPort].bSearchState = PRO97;
        else 
            g_tMtrRdSchInf[bPort].bSearchState = PRO69845ORNOT;  //liyan
        break;
    case PRO97:              //97Э���ѱ� 
        bSerState = SearchMeter(bPort, 0);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//97����
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = PRO69845ORNOT;
            }
        }
        break;
	case PRO69845ORNOT:        
		if (IsMetOrNot(bPort, 2, g_bAddrLen[bCnt[bPort]]) >= 2)//�Ƿ��ж��97�ı�
			g_tMtrRdSchInf[bPort].bSearchState = PRO69845;
		else 
		{
			if (++bCnt[bPort] >= sizeof(g_bAddrLen)/sizeof(BYTE))
			{
				g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
				bCnt[bPort] = 0;

			}
		}

		break;
	case PRO69845:              
		bSerState = SearchMeter(bPort, 2, g_bAddrLen[0]);//bCnt[bPort]
		if (bSerState == SEARCH_OVER) 
		{
			if (g_tMtrRdSchInf[bPort].bFinish)
			{
				ReinitSearch(bPort);
				g_tMtrRdSchInf[bPort].bSearchState = PRO69845ORNOT;
			//	if (++bCnt[bPort] >= sizeof(g_bAddrLen)/sizeof(BYTE))
			//	{
					g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
			//		bCnt[bPort] = 0;

			//	}
			}
		}
		break;
#if 0
//---------------------------------------------------

	case PROT188WATERORNOT:        
		if (IsMetOrNot(bPort, 2) >= 2)//ˮ��
			g_tMtrRdSchInf[bPort].bSearchState = PROT188WATER;
		else
			g_tMtrRdSchInf[bPort].bSearchState = PROT188GASORNOT;
		break;
	case PROT188WATER:              //
		bSerState = SearchMeter(bPort, 2);
		if (bSerState == SEARCH_OVER) 
		{
			if (g_tMtrRdSchInf[bPort].bFinish)//
			{
				ReinitSearch(bPort);
				g_tMtrRdSchInf[bPort].bSearchState = PROT188GASORNOT;
			}
			else //����32���
				g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		}        
		break;
//------------------------------------------
	case PROT188GASORNOT:        
		if (IsMetOrNot(bPort, 2) >= 2)//����
			g_tMtrRdSchInf[bPort].bSearchState = PROT188GAS;
		else
			g_tMtrRdSchInf[bPort].bSearchState = PROT188HEATORNOT;
		break;
	case PROT188GAS:              //
		bSerState = SearchMeter(bPort, 2);
		if (bSerState == SEARCH_OVER) 
		{
			if (g_tMtrRdSchInf[bPort].bFinish)//
			{
				ReinitSearch(bPort);
				g_tMtrRdSchInf[bPort].bSearchState = PROT188HEATORNOT;
			}
			else //����32���
				g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		}        
		break;
//------------------------------------------------------------
	case PROT188HEATORNOT:        
		if (IsMetOrNot(bPort, 2) >= 2)//�ȱ�
			g_tMtrRdSchInf[bPort].bSearchState = PROT188HEAT;
		else
			g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		break;
	case PROT188HEAT:              //
		bSerState = SearchMeter(bPort, 2);
		if (bSerState == SEARCH_OVER) 
		{
			if (g_tMtrRdSchInf[bPort].bFinish)//
			{
				ReinitSearch(bPort);
				g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
			}
			else //����32���
				g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		}        
		break;
#endif
    case SEARCHOVER:
        //׼����ַ�ϱ�,���������; 
        //g_tEventRept.bMtrAddrNum = MetNum(bPort);
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    case SEARCHWAIT:                //�ѱ����       //�ɳ�ʼ������һ�������ѱ�        
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    default:
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    }    
}

