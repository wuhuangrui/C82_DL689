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
				sFrmHead = i;//这之前的数据都是无效的
			}
			break;
		case 1:    //数据域前的数据
			pbRxBuf[bRxPtr++] = b;
			bRxCnt --;
			if (bRxCnt == 0)   //接收完，进行校验
			{
				if (pbRxBuf[1] == bMtrType)//表计类型匹配
				{
					bRxCnt = pbRxBuf[10] + 2;  //0xfe+2
					sFrmHead++;
					if (bRxCnt+11>dwBufSize || pbRxBuf[10]>=dwBufSize)   //剪帧的缓存区不够
					{
						bRxStep = 0;					
						break;
						//return -sFrmHead;                 //这里返回0，缓存区将永远无法释放
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
		case 2:     //数据 + 检验码 + 结束码
			pbRxBuf[bRxPtr++] = b;
			//DTRACE(DB_DL645V07, ("CDL645V07::RcvBlock : m_wRxCnt=%d.m_wRxPtr=%d,m_bRxBuf=%x\r\n", m_wRxCnt, m_wRxPtr, m_bRxBuf[m_wRxPtr-1])); 	
			bRxCnt -- ;
			if (bRxCnt == 0)   //接收完，进行校验
			{
				bRxStep = 0;

				if (pbRxBuf[bRxPtr-1]==0x16 && pbRxBuf[bRxPtr-2]==CheckSum(pbRxBuf, pbRxBuf[10]+11))
				{
					*pbRxLen = pbRxBuf[10] + 13;
					//return i+1;//接收到完整的一帧		
					return dwLen;//接收到完整的一帧	返回全长 以表示本轮收码全部处理完毕
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



//描述：从串口缓冲区中找满足645的报文
//参数：@pbBlock - 接受的缓存
//		@dwLen - 接收的长度
//		@pbRxBuf - 存放完整数据帧的缓存
//		@pbRxLen - 完整数据帧报文长度
//		@dwBufSize - 接收缓冲区的长度
//返回：0-无数据，正数-接收到的数据帧长度，负数-无效数据长度
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
				sFrmHead = i;//这之前的数据都是无效的
			}
			break;
		case 1:    //数据域前的数据
			pbRxBuf[bRxPtr++] = b;
			bRxCnt --;
			if (bRxCnt == 0)   //接收完，进行校验
			{
				if (pbRxBuf[7] == 0x68) // && (p[FAPDL645_CMD]&FAPDL645_CMD_DIR)==FAPDL645_CMD_DOWN //防止接收到红外返回的自己发出去的帧
				{
					bRxCnt = pbRxBuf[9] + 2;  //0xfe+2
					sFrmHead++;
					if (/*bRxCnt+10>dwBufSize ||*/ pbRxBuf[9]>=dwBufSize)   //剪帧的缓存区不够
					{
						bRxStep = 0;					
						break;
						//return -sFrmHead;                 //这里返回0，缓存区将永远无法释放
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
		case 2:     //数据 + 检验码 + 结束码
			pbRxBuf[bRxPtr++] = b;
			//DTRACE(DB_DL645V07, ("CDL645V07::RcvBlock : m_wRxCnt=%d.m_wRxPtr=%d,m_bRxBuf=%x\r\n", m_wRxCnt, m_wRxPtr, m_bRxBuf[m_wRxPtr-1])); 	
			bRxCnt -- ;
			if (bRxCnt == 0)   //接收完，进行校验
			{
				bRxStep = 0;

				if (pbRxBuf[bRxPtr-1]==0x16 && pbRxBuf[bRxPtr-2]==CheckSum(pbRxBuf, pbRxBuf[9]+10))
				{
					*pbRxLen = pbRxBuf[9] + 12;
					//return i+1;//接收到完整的一帧		
					return dwLen;//接收到完整的一帧	返回全长 以表示本轮收码全部处理完毕
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



//描述：接收验证
//返回>0: 接受的到正确帧
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
				sFrmHead = wLen;//这之前的数据都是无效的
			}
			break;
		case 1:    //地址域前的数据
			pbRxBuf[Tmp698.wRxPtr++] = b;
			Tmp698.wRxCnt --;
			if (Tmp698.wRxCnt == 0)   //接收完，进行校验
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
		case 2:    //数据域前的数据
			pbRxBuf[Tmp698.wRxPtr++] = b;
			Tmp698.wRxCnt --;
			if (Tmp698.wRxCnt == 0)   //接收完，进行校验
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
		case 3:     //数据 + 检验码 + 结束码
			pbRxBuf[Tmp698.wRxPtr++] = b;
			Tmp698.wRxCnt -- ;
			if (Tmp698.wRxCnt == 0)   //接收完，进行校验
			{
				Tmp698.nRxStep = 0;

				wCrc = CheckCrc16(pbRxBuf+1, Tmp698.wRxPtr - 4);
				if (pbRxBuf[Tmp698.wRxPtr-1]==0x16 && (pbRxBuf[Tmp698.wRxPtr-3]==(wCrc&0xff)) && (pbRxBuf[Tmp698.wRxPtr-2]==((wCrc>>8)&0xff)) /*&& (pbRxBuf[Tmp698.wRxAPDUPos]&0x7f)==pbTxBuf[Tmp698.wRxAPDUPos]*/)
				{
					Tmp698.wRxAPDULen = Tmp698.wRxPtr - Tmp698.wRxAPDUPos - 3;
					*pwRxLen = Tmp698.wRxDataLen+2;
					return dwLen;    //接收到完整的一帧
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
    ptStack->iTop = 0;                   //先压栈后移动，先移动后出栈。
}

static BYTE IsStackEmpty(TStack *ptStack)
{
    if (ptStack->iTop == 0)
        return 1;             //空
    return 0;
}

static BYTE PushStack(TStack *ptStack, BYTE bData)
{
    if ((ptStack->iTop+1) == STACK_SIZE+EXT_STACK_SIZE)          //栈满， 不能入栈
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
    if (IsStackEmpty(ptStack))           //空栈
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

//取得栈顶元素但不是弹出
static BYTE GetStackTop(TStack *ptStack, BYTE *pbData)
{
    if (IsStackEmpty(ptStack))           //空栈
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
    //使用这个功能：
    //ReadPara(0x00000025, 0, bBuf, sizeof(bBuf)); 
    //if (*bBuf == 1)  //使用搜表功能
    //    g_tMtrRdSchInf[bPort].bSearchState = PRO07METORNOT; 
    //else
    {
        if (bStartSer)
            g_tMtrRdSchInf[bPort].bSearchState = PRO07METORNOT;
        else
            g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;                  //初始化起动一次搜表
    }
    InitStack(&g_tMtrRdSchInf[bPort].tStack);
    //g_tEventRept.bMtrAddrNum = 0;                  //要重新上报
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



//检查下表地址是否有效
//返回1-有效，0-无效
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

//地址长度数组
BYTE g_bAddrLen[] = {6, 7, 5, 8, 9, 10, 11, 12, 4, 3};


//void SaveSearchPnToDb(BYTE* pbMtrAddr, BYTE bPro, TTime tmNow, BYTE bPort);

//68 AA AA AA AA AA AA 68 01 02 43 C3 D5 16 //以广播地址读43 C3         9010
//68 AA AA AA AA AA AA 68 81 06 43 C3 94 A5 35 33 FA 16 //有些97的表回的还是通配地址
//这种表以下函数搜不出表地址
//bSubMetType 表计子类型，这个参数就是为了针对T188协议里的表计类型参数，其他协议不用该参数 
//返回 0-没有表，
//     1- 1块电表，
//     2- 多块电表，
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

	for (BYTE i=0; i<sizeof(g_dwTestID)/sizeof(WORD); i++)  //连续尝试单ID与块ID
	{
	    for (BYTE j=0; j<2; j++)           //连续发二次避免出错
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
		

			if (iLen == -1) //端口不为抄表口
				break;
			else if (iLen <= 0) //其他错误   
				continue;

			BYTE bPos = 0;

			if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_69845)
			{
				//sRet = MtrT188RcvBlock(bRxBuf, DWORD(iLen), bRxFrm, &bLen, 100, bSubMetType);//默认是水表10冷类型
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
				if (sRet > wLen+4) //多表，因为帧头最多4个字节  //查一下长度，看循环缓存区的帧后面还有没有数据，有则多表
	            {
	                bMetNum |= 2;
	            }
	            else   //多表时这里不添加，因为后会还会查找，否则会重复
	            {
	                AddMetToTab(&g_tMtrRdSchInf[bPort], bRxFrm+bPos, bAddrLen, g_tMeterPro[bMetType].bProto, ((bPort+wPortMin)&0x1f)+ (BaudrateToGbNum(g_tMeterPro[bMetType].wBaud)<<5));//todo:注意一块表先回，其它几块表后回的情况
	                bMetNum |= 1;
	            }
	        }
	        else if (sRet < 0)//没剪到帧             帧头处理
	        {   
	            bMetNum |= 2;
	        }	        	        	
	        //sRet == 0  没有表       
	    }
	    
	    if (bMetNum != 0) //当前ID有响应,不用试下一个ID        
	     	break;		
	}

    if (bMetNum >= 2) //多块表
        return 2;
        
    return bMetNum;
}


//搜表从地址低字节向高尝试，第一个尝试的地址为AA AA AA AA AA 00，如果碰到多块表应该将00压入栈中，
//然后尝试AA AA AA AA 00 00，AA AA AA AA 01 00，...
//6个字节的地址叫5级，最低字节叫0级，从0级开始尝试。每个字节的地址值只能是0-99.
//栈空的时候尝试的0级，栈深度为1时尝试的为1级，因此栈的深度可以知道当前应该尝试哪一级。
//栈深度加1时，地址值应该重新由0开始直到99.到了99表示该级尝试完可以弹出一个栈顶元素，栈顶元素的大小加1就是
//该级应该开始尝试的地址值的起始值。
//变量解释
//g_tMtrRdSchInf[bPort].bCurTry：当前尝试级的地址值，变化范围是0-99
//g_tMtrRdSchInf[bPort].bCurTryLevel：记录级别。变化范围0-5，用于判断是否有压栈
//g_tMtrRdSchInf[bPort].bAddrPatten：将要尝试的表址计算好，放入g_tMtrRdSchInf[bPort].bAddrPatten中。比如栈里有2个字节02，01。Ln表示栈的对应位但没有元素， L7 L6 L5 L4 L3 L2 02 01，
//那么当前尝试位为L2，L2是的值从0开始到99来试。g_tMtrRdSchInf[bPort].bAddrPatten中为值为：AA AA AA g_tMtrRdSchInf[bPort].bCurTry 02 01
//获取一个尝试抄读的电表地址，返回当前是在哪一层（也就是电表地址的第几个字节）尝试
BYTE GetTryAddr(TMtrSchInf* pMtrSch, BYTE *pbMtrAddr)
{        
    BYTE bNode;
    BYTE bLevel = 0;
    if (IsStackEmpty(&pMtrSch->tStack))
    {
    	memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel); 
        pMtrSch->bAddrPatten[0] = ByteToBcd(pMtrSch->bCurTry++);
        if (pMtrSch->bCurTry > 100)//////     所有表地址都已找完。
            pMtrSch->bFinish = 1; 
    }
    else
    {
        GetStackTop(&pMtrSch->tStack, &bNode);     
        bLevel = (BYTE)DepthStack(&pMtrSch->tStack);	
        if (bLevel > pMtrSch->bCurTryLevel)      //压过栈               //bCurTryLevel
        {       //BYTE SearchMeter()压入了新的字节，pMtrSch->bCurTryLevel前进一级进行搜索
            if (bLevel >= 6)//说明该地址，至少有两块表地址一模一样
            {
                //将该地址从栈中取出，但不要移动栈           todo:可以设置告警事件
                PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry);     //跳过该表。
                bLevel--;
                pMtrSch->bCurTry++;     
                while (pMtrSch->bCurTry > 99)
                {
                    if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//棧空,      全99为广播地址，电表不会回
                    {
                        pMtrSch->bFinish = 1; 
                        break;
                    }
                    pMtrSch->bCurTry++;        //在弹出来的值基础上加1继续试
                    pMtrSch->bCurTryLevel--;
                    bLevel--;
                }                     
            }
            else
            {
                pMtrSch->bCurTry = 0;
                pMtrSch->bAddrPatten[bLevel-1] = ByteToBcd(bNode); //栈指针为1的时候，栈中只有一个元素
                pMtrSch->bCurTryLevel++;
            }
        }
        else if (bLevel == pMtrSch->bCurTryLevel)    //当前尝试级别的字节，0~99地递增
        {
            while (pMtrSch->bCurTry > 99)  //在这个字节上，没冲突的表可能已经搜出来了，后者之前有冲突的都已经压栈且搜完
            {
                if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//棧空,      全99为广播地址，电表不会回
                {
                    pMtrSch->bFinish = 1; 
                    break;
                }
                pMtrSch->bCurTry++;        //在弹出来的值基础上加1继续试
                pMtrSch->bCurTryLevel--;
                bLevel--;
            }        
        }
        else         //
        {            
        }

        //填入：当前尝试级别的字节(0~99)、及后面的广播字节
        memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel);         //退栈时要将退出的位补上0xAA
        pMtrSch->bAddrPatten[bLevel] = ByteToBcd(pMtrSch->bCurTry++);        
    }    
    memcpy(pbMtrAddr, pMtrSch->bAddrPatten, 6);   
    return bLevel;
}

//将698.45中地址的指定长度清空为bSetData，以半字为单位,bSetData不能大于0x0f
//bOffset字节偏移，默认是从字节的高位开始，但是也有从字节的低位开始的。
//bOffset : 取值0或者1
/**************************************************
________________________________________________
|    1  |    2     |     3      |      4      |
|_______|__________|____________|_____________|
|    第一个字节    |       第二个字节         | 

1 从1开始清除 奇数或者偶数个半字节长度
2 从2开始清除 奇数或者偶数个半字节长度
****************************************************/
void Clear698Addr(BYTE *bAddr, BYTE bOffset, BYTE bSetData, BYTE bLen)
{
	if ((bAddr != NULL) && (bSetData <= 0x0f) && (bOffset < 2))
	{
		if (bOffset > 0)//从上图中1开始清除
		{
			bAddr[0] = (bAddr[0]&0xf0) | (bSetData);
		}
		if (bLen%2)//清除奇数个半字节长度
		{
			memset(bAddr+bOffset, (bSetData<<4)|bSetData, bLen/2);
			if (bOffset == 0)
			{
				bAddr[bLen/2+bOffset] = (bSetData<<4) | (bAddr[bLen/2] &0x0f);//
			}



		}
		else//清除偶数个半字节长度
		{
			memset(bAddr+bOffset, bSetData<<4|bSetData, bLen/2);
			if (bOffset > 0)
			{
				bAddr[bLen/2+bOffset] = (bSetData<<4) | (bAddr[bLen/2] &0x0f);//
			}
		}
	}
}

//bAddrLen: 地址长度
//bMtrType: 表计协议类型
BYTE GetTryAddr(TMtrSchInf* pMtrSch, BYTE *pbMtrAddr, BYTE bAddrLen, BYTE bMtrType)
{
	BYTE bNode;
	BYTE bLevel = 0;
	if ((bMtrType != CCT_MTRPRO_69845) || (bAddrLen > 12) || (bAddrLen < 3))//假定认为短语3个字节的地址不存在，长于12个字节的地址不存在
	{
		return 0;
	}
	if (IsStackEmpty(&pMtrSch->tStack))
	{
		Clear698Addr(&pMtrSch->bAddrPatten[bLevel/2], bLevel%2, 0x0a, bAddrLen*2-bLevel);
		Clear698Addr(&pMtrSch->bAddrPatten[0], 0, pMtrSch->bCurTry++, 1);
		if (pMtrSch->bCurTry > 10)//////     所有表地址都已找完。
			pMtrSch->bFinish = 1; 
	}
	else
	{
		GetStackTop(&pMtrSch->tStack, &bNode);     
		bLevel = (BYTE)DepthStack(&pMtrSch->tStack);	
		if (bLevel > pMtrSch->bCurTryLevel)      //压过栈               //bCurTryLevel
		{       //BYTE SearchMeter()压入了新的字节，pMtrSch->bCurTryLevel前进一级进行搜索
			if (bLevel >= bAddrLen * 2)//说明该地址，至少有两块表地址一模一样
			{
				//将该地址从栈中取出，但不要移动栈           todo:可以设置告警事件
				PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry);     //跳过该表。
				bLevel--;
				pMtrSch->bCurTry++;     
				while (pMtrSch->bCurTry > 9)
				{
					if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//棧空,      全99为广播地址，电表不会回
					{
						pMtrSch->bFinish = 1; 
						break;
					}
					pMtrSch->bCurTry++;        //在弹出来的值基础上加1继续试
					pMtrSch->bCurTryLevel--;
					bLevel--;
				}                     
			}
			else
			{
				pMtrSch->bCurTry = 0;
			//	pMtrSch->bAddrPatten[bLevel-1] = ByteToBcd(bNode); //栈指针为1的时候，栈中只有一个元素
				Clear698Addr(&pMtrSch->bAddrPatten[(bLevel-1)/2], (bLevel-1)%2, ByteToBcd(bNode), 1);
				pMtrSch->bCurTryLevel++;
			}
		}
		else if (bLevel == pMtrSch->bCurTryLevel)    //当前尝试级别的字节，0~99地递增
		{
			while (pMtrSch->bCurTry > 9)  //在这个字节上，没冲突的表可能已经搜出来了，后者之前有冲突的都已经压栈且搜完
			{
				if (!PopStack(&pMtrSch->tStack, &pMtrSch->bCurTry))//棧空,      全99为广播地址，电表不会回
				{
					pMtrSch->bFinish = 1; 
					break;
				}
				pMtrSch->bCurTry++;        //在弹出来的值基础上加1继续试
				pMtrSch->bCurTryLevel--;
				bLevel--;
			}        
		}
		else         //
		{            
		}

		//填入：当前尝试级别的字节(0~99)、及后面的广播字节
	//	memset(&pMtrSch->bAddrPatten[bLevel], 0xaa, 6-bLevel);         //退栈时要将退出的位补上0xAA
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
		GetTryAddr(&g_tMtrRdSchInf[bPort], bMtrAddr);            //todotodo:分协议   
	if (g_tMtrRdSchInf[bPort].bFinish)
        return SEARCH_OVER; 

 
	for (BYTE i=0; i<1; i++)  //重复三次
	{
		for (BYTE j=0; j<1; j++)  //重复三次
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
			if (iLen == -1) //端口不为抄表口
				return SEARCH_UNDOEN;//break;
			else if (iLen <= 0) //其他错误   
				continue;
			BYTE bPos = 1;

			if (g_tMeterPro[bMetType].bProto == CCT_MTRPRO_69845)
			{
				//sRet = MtrT188RcvBlock(bRxBuf, DWORD(iLen), bRxFrm, &bLen, 100, bSubMetType);//默认是水表10冷类型
				sRet = DL69845RcvBlock(bRxBuf, DWORD(iLen), bRxFrm,  &wLen, 100);
				bPos = 5;
			}else
			{
				sRet =  Mtr645RcvBlock(bRxBuf, DWORD(iLen), bRxFrm, &wLen, 100);
				bPos = 1;
			}


			if (sRet > 0)
			{       
				if (sRet > wLen+4) //多表，因为帧头最多4个字节  //查一下长度，看循环缓存区的帧后面还有没有数据，有则多表
				{			
					if (bMutiMet == 0)   //同一表地址重复三次不能每次都压栈，而只能压一次。
					{
						if (!PushStack(&g_tMtrRdSchInf[bPort].tStack, g_tMtrRdSchInf[bPort].bCurTry-1))          //将有冲突的压入栈中
							return SEARCH_ERROR;
						bMutiMet = 1;
					}
				}			
				AddMetToTab(&g_tMtrRdSchInf[bPort], bRxFrm+bPos, bAddrLen, g_tMeterPro[bMetType].bProto, ((bPort+wPortMin)&0x1f)+ (BaudrateToGbNum(g_tMeterPro[bMetType].wBaud)<<5));//todo:注意一块表先回，其它几块表后回的情况
				//if (GetMtrNumByPort(bPort+wPortMin) >= DEF_MTR_NUM_PER_PORT)      //已经找到32块表。则不再寻找。
				//	return SEARCH_OVER;
			}      
			else if (sRet < 0)//没剪到帧             帧头处理
			{            
				if (bMutiMet == 0)
				{
					if (!PushStack(&g_tMtrRdSchInf[bPort].tStack, g_tMtrRdSchInf[bPort].bCurTry-1))          //将有冲突的压入栈中
						return SEARCH_ERROR;
					bMutiMet = 1;
				}
			} 
			//sRet == 0  没有表
		}

		if (bMutiMet != 0) //当前ID有响应,不用试下一个ID        
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
        if (IsMetOrNot(bPort, 1) >= 2)//是否有多块07的表
            g_tMtrRdSchInf[bPort].bSearchState = PRO07;
        else
            g_tMtrRdSchInf[bPort].bSearchState = PRO97METORNOT;
        break;
    case PRO07:              //07协议搜表
        bSerState = SearchMeter(bPort, 1);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//07搜完
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = PRO97METORNOT;
            }
            else //超过32块表
                g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
        }        
        break;
    case PRO97METORNOT:        
        if (IsMetOrNot(bPort, 0) >= 2)//是否有多块97的表
            g_tMtrRdSchInf[bPort].bSearchState = PRO97;
        else 
            g_tMtrRdSchInf[bPort].bSearchState = PRO69845ORNOT;  //liyan
        break;
    case PRO97:              //97协议搜表 
        bSerState = SearchMeter(bPort, 0);
        if (bSerState == SEARCH_OVER) 
        {
            if (g_tMtrRdSchInf[bPort].bFinish)//97搜完
            {
                ReinitSearch(bPort);
                g_tMtrRdSchInf[bPort].bSearchState = PRO69845ORNOT;
            }
        }
        break;
	case PRO69845ORNOT:        
		if (IsMetOrNot(bPort, 2, g_bAddrLen[bCnt[bPort]]) >= 2)//是否有多块97的表
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
		if (IsMetOrNot(bPort, 2) >= 2)//水表
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
			else //超过32块表
				g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		}        
		break;
//------------------------------------------
	case PROT188GASORNOT:        
		if (IsMetOrNot(bPort, 2) >= 2)//气表
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
			else //超过32块表
				g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		}        
		break;
//------------------------------------------------------------
	case PROT188HEATORNOT:        
		if (IsMetOrNot(bPort, 2) >= 2)//热表
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
			else //超过32块表
				g_tMtrRdSchInf[bPort].bSearchState = SEARCHOVER;
		}        
		break;
#endif
    case SEARCHOVER:
        //准备地址上报,将包数算出; 
        //g_tEventRept.bMtrAddrNum = MetNum(bPort);
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    case SEARCHWAIT:                //搜表结束       //由初始化来起动一次重新搜表。        
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    default:
        g_tMtrRdSchInf[bPort].bSearchState = SEARCHWAIT;
        break;
    }    
}

