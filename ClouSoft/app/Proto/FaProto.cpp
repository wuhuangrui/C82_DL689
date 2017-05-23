/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FaProto.cpp
 * 摘    要：本文件主要实现终端面向对象协议通讯
 * 当前版本：1.0
 * 作    者：孔成波
 * 完成日期：2016年8月
 * 1. 终端连接过程：link---connect----Get/Set/Action/Proxy
 * 2. 终端分层处理，可分为链路层检帧，应用层检帧，应用层处理等
 * 3. 应用层APDU帧，必须是完整的可自解析的帧，而LPDU帧是APDU的分片(如果链路buf小于APDU大小)，不可解析，需要接受完所有的LPDU组合成一帧APDU来处理
 * 4. 
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaProto.h"
#include "FaAPI.h"
#include "sysfs.h"
#include "DCSample.h"
#include "ParaMgr.h"
#include "DbOIAPI.h"
#include "MtrCtrl.h"
#include "CctTaskMangerOob.h"
#include "CctAPI.h"
#include "ProPara.h"
#include "Esam.h"
#include "MeterAPI.h"
#ifndef SYS_WIN
#include "Sample.h"
#endif
extern DWORD TiToSecondes(TTimeInterv *pTI);

CFaProto::CFaProto()
{
	m_wRxPtr = 0;
	m_nRxCnt = 0;
	m_nRxStep = 0;	
	m_pSftpClient = new CSftp;

//new protocol part
	//link layer 
	memset( &m_LnkComm, 0, sizeof(TLnkLayerComm) );
	memset( m_bRxBuf, 0, sizeof(MAXFRMSIZE) );
	memset( m_bTxBuf, 0, sizeof(MAXFRMSIZE) );
	memset( m_bAutoBuf, 0, sizeof(MAXFRMSIZE) );
	m_nRxStep = 0;
	m_wRxPtr = 0;
	m_nRxCnt = 0;
	m_wTxPtr = 0;
	m_wAutoPtr = 0;
	memset( &m_RxLPduPool, 0, sizeof(TLPduPool) );
	memset( &m_TxLPduPool, 0, sizeof(TLPduPool) );

	//app layer
	memset( &m_AppComm, 0, sizeof(TAppLayerComm) );
	memset( &m_RxAPduPool, 0, sizeof(TAPduPool) );//To APP manage
	memset( &m_TxAPdu, 0, sizeof(m_TxAPdu) );//From APP manage
	memset( &m_RxAPdu, 0, sizeof(m_RxAPdu) );//To APP manage
	memset( &m_TxInsAPduPool, 0, sizeof(TInsertAPduPool) );//From APP manage
 
	//app data manager 
	memset( &m_RxDPool, 0, sizeof(TDataPool) );
	memset( &m_TxDPool, 0, sizeof(TDataPool) );

	//物理层的协议识别
	m_bNoComuSta = 0;
	m_dwClickFrmOk = GetClick();

	m_bBeatNum = 0;
	memset((BYTE*)&m_iK, 0, sizeof(m_iK));

	m_wCurTaskId = 0;
	m_wLastTaskId = 0;
	m_iStart = -1;
	m_iRsd10Pn = -1;
	m_fPwrOnRun = false;
}

CFaProto::~CFaProto()
{
}

bool CFaProto::Init(TFaProPara* pFaProPara)
{	
	BYTE bBuf[32];

	m_pFaProPara = pFaProPara;
	if (!CProto::Init(&m_pFaProPara->ProPara)) //基类的初始化
		return false;

	ResetLnkPara();	
	ClearVar( LNKST_NDM );
	//GPRS需要主动上报，红外不需要
	m_dwProRxClick = 0;
	m_dwBeatClick = 0;
	
	//初始化终端通讯地址
	if (OoReadAttr(0x4001, ATTR_2, bBuf, NULL, NULL) <= 0 || IsAllAByte(bBuf, 0x00, 2))
	{
		DTRACE(DB_FAPROTO,("AddrCheck: Read Addr failed !\r\n"));
		m_LnkComm.bSvrAddLen = 5;
		memset(m_LnkComm.bSvrAddr, 0x00, 6);
		m_LnkComm.bSvrAddr[0] = 0x01;
	}
	else
	{
		m_LnkComm.bSvrAddLen = bBuf[1];	//bBuf[0]为DT_OCT_STR，
		revcpy(m_LnkComm.bSvrAddr, &bBuf[2], bBuf[1]);
	}

	//协商空间实际尺寸
	InitCommSize(pFaProPara->wConnectType);
	//上报部分
	if ( m_pFaProPara->ProPara.fAutoSend)
		m_queEvt.Init(RPTWAITQUE_SIZE, MAXFRMSIZE);
	SetCnOAD();	
	WaitQueClear();
	m_AppComm.bMyPIID = 2;
	return true;
}

//帧校验
int CFaProto::RcvBlock(BYTE *pbBuf,int wLen)
{
	BYTE *pbBlock = pbBuf;

	for (int i=0; i<wLen; i++)
	{
        BYTE b = *pbBlock++;
		
        switch (m_nRxStep) 
		{
		case 0:
			if( b == 0x68 )
			{
				memset( m_bRxBuf, 0, MAXFRMSIZE );
				m_bRxBuf[0] = b;	
				m_wRxPtr = 1;			
				m_nRxStep = 1;	
				m_nRxCnt = 2;//之后是长度域(2BYTE,控制域1BYTE，地址长度)
			}
			else
			{
				m_wRxPtr = 0;			
				m_nRxStep = 0;	
			}
			break;
		case 1:
			m_bRxBuf[m_wRxPtr++] = b;
			m_nRxCnt--;
			if( m_nRxCnt == 0)
			{
				WORD wRxFrmLen = ((WORD)(m_bRxBuf[2]&0x3f)<<8) | m_bRxBuf[1];
				if (wRxFrmLen < MAXFRMSIZE)
				{
					m_nRxCnt = wRxFrmLen-1; //帧长度是去掉起始0x68和结束0x16之后的字节
					m_nRxStep = 2;
				}
			}
			break;
		case 2:
			m_bRxBuf[m_wRxPtr++] = b;
			m_nRxCnt--;
			if( m_nRxCnt == 0)
			{
				m_nRxStep = 0;
				if (m_bRxBuf[m_wRxPtr-1] == 0x16)	//控制域
				{
					//if (VeryFrm())
					{
						return i+1;
					}
				}
			}
			break;

		default:
			m_nRxStep = 0;
			break;
		} //switch (m_nRxStep) 
	}

	return -wLen;
}


int CFaProto::ProRecordToApduInfo(BYTE *pbApdu, TApduInfo *pApduInfo)
{
	BYTE *pbApdu0 = pbApdu;

	pApduInfo->wOADLen = 4;
	pApduInfo->pbOAD = pbApdu;
	pbApdu += pApduInfo->wOADLen;
	pApduInfo->pbRSD = pbApdu;
	pApduInfo->wRSDLen = ScanRSD(pbApdu, false);	
	pbApdu += pApduInfo->wRSDLen;
	pApduInfo->pbRCSD = pbApdu;
	pApduInfo->wRCSDLen = ScanRCSD(pbApdu, false);	
	pbApdu += pApduInfo->wRCSDLen;

	return pbApdu - pbApdu0;
}

//描述：计算地址十进制位数个数
BYTE CalDecDigNum(BYTE *pbAddr, BYTE bLen)
{
	BYTE bH, bL;
	BYTE bNum = 0;

	for (BYTE i=0; i<bLen; i++)
	{
		bH = (pbAddr[i]>>4) & 0x0f;
		bL = pbAddr[i] & 0x0f;

		if (bH != 0x0f)
			bNum++;
		if (bL != 0x0f)
			bNum++;
	}	

	return bNum;
}

bool CFaProto::AddrCheck()
{
	BYTE bCLDefAddr[] = {"\x66\x55\x44\x33\x22\x11"};	//科陆通用地址
	BYTE bSvrH, bSvrL;
	BYTE bCliH,	bCliL;
	BYTE bDecNum;
	char szSvrAddr[TSA_LEN]={0};
	char szCliAddr[TSA_LEN]={0};

	m_LnkComm.bAddrType = (m_bRxBuf[4]>>6)&0x03;	//地址类别标志
	m_LnkComm.bCliAddrLen = (m_bRxBuf[4]&0x0f) + 1;	//地址长度
	memcpy(m_LnkComm.bCliAddrs, &m_bRxBuf[5], m_LnkComm.bCliAddrLen);
	if (m_LnkComm.bAddrType==ADDR_TYPE_SINGLE || m_LnkComm.bAddrType==ADDR_TYPE_GROUP)
	{
		if (m_pFaProPara->ProPara.fLocal) //本地维护口
			if (memcmp(m_LnkComm.bCliAddrs, bCLDefAddr, sizeof(bCLDefAddr)) == 0)	
				return true;

		if (m_LnkComm.bCliAddrLen == m_LnkComm.bSvrAddLen)
		{
			bDecNum = CalDecDigNum(m_LnkComm.bCliAddrs, m_LnkComm.bCliAddrLen);
			if (bDecNum%2 == 1)	//10进制位数为奇数
			{
				if (memcmp(m_LnkComm.bCliAddrs, m_LnkComm.bSvrAddr, m_LnkComm.bSvrAddLen-1)==0)
					if (((m_LnkComm.bCliAddrs[m_LnkComm.bCliAddrLen-1]>>4)&0x0f) == ((m_LnkComm.bSvrAddr[m_LnkComm.bCliAddrLen-1]>>4)&0x0f))
						return true;
			}
			else
			{
				if(memcmp(m_LnkComm.bCliAddrs, m_LnkComm.bSvrAddr, m_LnkComm.bSvrAddLen) == 0)
					return true;
			}
		}
		DTRACE(DB_FAPROTO,("AddrCheck: Signal/Group-Addr check failed, bSvrAddr:%s, bCliAddr:%s !\r\n", \
			HexToStr(m_LnkComm.bSvrAddr, m_LnkComm.bSvrAddLen, szSvrAddr), \
			HexToStr(m_LnkComm.bCliAddrs, m_LnkComm.bCliAddrLen, szCliAddr)));
	}
	else if (m_LnkComm.bAddrType == ADDR_TYPE_UNIVERSAL)
	{
		if (m_LnkComm.bCliAddrLen == m_LnkComm.bSvrAddLen)
		{
			bDecNum = CalDecDigNum(m_LnkComm.bCliAddrs, m_LnkComm.bCliAddrLen);
			for(BYTE i=0; i<m_LnkComm.bCliAddrLen; i++)
			{
				bSvrH = (m_LnkComm.bSvrAddr[i]>>4) & 0x0f;
				bSvrL = m_LnkComm.bSvrAddr[i] & 0x0f;
				bCliH = (m_LnkComm.bCliAddrs[i]>>4) & 0x0f;
				bCliL = m_LnkComm.bCliAddrs[i] & 0x0f;

				if ((bCliH!=bSvrH && bCliH!=0x0A) && (bCliL!=bSvrL && bCliL!=0x0A))
				{
					if ((i==(m_LnkComm.bCliAddrLen-1))	//地址的最后一个字节
						&& (bDecNum%2 == 1)		//地址10进制位数为奇数
						&& (bCliH==0x0A))	//通配地址	
						return true;

					DTRACE(DB_FAPROTO,("AddrCheck: Universal-Addr check failed, bSvrAddr:%s, bCliAddr:%s !\r\n", \
						HexToStr(m_LnkComm.bSvrAddr, m_LnkComm.bSvrAddLen, szSvrAddr), \
						HexToStr(m_LnkComm.bCliAddrs, m_LnkComm.bCliAddrLen, szCliAddr)));
				}
			}
		}
	}
	else	//广播地址
	{
		if (IsAllAByte(m_LnkComm.bCliAddrs, 0xAA, m_LnkComm.bCliAddrLen))
			return true;
	}

	return false;
}

//描述：帧校验
//主要校验：服务器地址+HCS+FCS
int  CFaProto::VeryFrm()
{
	if (!AddrCheck())
		return -2;

	m_LnkComm.wRFrmLen = (WORD )m_bRxBuf[1] + ((WORD )(m_bRxBuf[2]&0x3f)<<8);	//总帧长度,除起始和结束符之外的帧字节数
	m_LnkComm.bFunCode = m_bRxBuf[3]&0x0f;				//帧控制码
	m_LnkComm.fIsSegSend = ((m_bRxBuf[3]&0x20) != 0);		//链路层是否有分帧
	m_LnkComm.bFrmHeaderLen = m_LnkComm.bCliAddrLen+6;	//0x68+Len(2byte)+C(1byte)+AF(1byte)+SA(SvrLen)+CA(1byte)
	
	//HCS检验	
	WORD wCrc = CheckCrc16(&m_bRxBuf[1], m_LnkComm.bFrmHeaderLen-1); //帧头校验是除起始字符和HCS外的所有字节的校验
	WORD wCrc1 = ((WORD)m_bRxBuf[m_LnkComm.bFrmHeaderLen+1]<<8) | m_bRxBuf[m_LnkComm.bFrmHeaderLen];
	if( wCrc != wCrc1 )
	{
		DTRACE(DB_FAPROTO,("VeryFrm: HCS 校验错, Term wCrc=0x%04x, Master wCrc=0x%04x!\r\n", wCrc, wCrc1));
		return -4;
	}
	
	//FCS校验
	wCrc = CheckCrc16(&m_bRxBuf[1], m_LnkComm.wRFrmLen-2); //帧尾校验是除起始字符和FCS外的所有字节的校验,帧长度是除起始结束字符外的字符
	wCrc1 = ((WORD)m_bRxBuf[m_LnkComm.wRFrmLen]<<8) | m_bRxBuf[m_LnkComm.wRFrmLen-1];
	if( wCrc != wCrc1 )
	{
		DTRACE(DB_FAPROTO,("VeryFrm: FCS 校验错, Term wCrc=0x%04x, Master wCrc=0x%04x!\r\n", wCrc, wCrc1));
		return -4;
	}

	m_LnkComm.bCliAddr = m_bRxBuf[m_LnkComm.bCliAddrLen+8-3];
	BYTE bOffLnkApp = m_LnkComm.bCliAddrLen+8;			//帧头部加HCS
	if (m_LnkComm.fIsSegSend)							//如果是分帧传输的帧，链路应用层开始两字节是分帧格式域
	{
		m_LnkComm.wRcvFrmSegNo = (WORD )m_bRxBuf[bOffLnkApp] +  (WORD )((m_bRxBuf[bOffLnkApp+1]&0x0f)<<8);		//帧序号
		m_LnkComm.FrmSegType = m_bRxBuf[bOffLnkApp+1]>>6;			//帧类型
		m_LnkComm.bAPDUoffset = bOffLnkApp+2;						//实际的APDU的偏移
		m_LnkComm.wAPDULen = m_LnkComm.wRFrmLen+4-m_LnkComm.bAPDUoffset-3;	//所有接收的帧，减去APDU偏移，减去FCS和结束符号
	}
	else
	{
		m_LnkComm.bAPDUoffset = bOffLnkApp;
		m_LnkComm.wAPDULen = m_LnkComm.wRFrmLen+2-bOffLnkApp-3;	//所有接收的帧，减去APDU偏移，减去FCS和结束符号
		memcpy(m_RxDPool.bBuf, &m_bRxBuf[bOffLnkApp], m_LnkComm.wAPDULen);
		if (m_LnkComm.wAPDULen == 2)//如果接收到的应用层只有2个字节，那就应该是APDU分帧确认帧，需要终端上送APDU的下一个分片APDU
		{
			if (((m_bRxBuf[bOffLnkApp+1]>>6)&0x03) == 0x02)
			{
				WORD wSegFrmNo = ((m_bRxBuf[bOffLnkApp+1]&0x0f)<<8) + m_bRxBuf[bOffLnkApp];		//帧序号
				if (wSegFrmNo != m_LnkComm.wSendFrmSegNo)
				{
					DTRACE(DB_FAPROTO, ("VeryFrm::LPDU frm seg err, wSegFrmNo:%d, correct-No:%d !!\r\n", wSegFrmNo, m_LnkComm.wSendFrmSegNo));
					return -4;
				}
			}
			else
			{
				DTRACE(DB_FAPROTO,("VeryFrm:Get a short frame, not segFrame due to wrong frame-type!!\r\n"));
				return -4;
			}
		}
	}

	return 1;
}

#define ERR_LINK_OK					0	//成功
#define ERR_LINK_REPEATED_ADDR		1	//地址重复
#define ERR_LINK_ILLEGAL_DEV		2	//非法设备
#define ERR_LINK_CAP_LESS			3	//容量不足
#define ERR_LINK_ELSE				4	//其他错误

//如果链路分帧的帧格式
#define LNK_SEGFRM_START	0	//链路起始帧
#define LNK_SEGFRM_MID		3	//链路中间帧
#define LNK_SEGFRM_FIN		1	//链路结束帧
#define LNK_SEGFRM_CFM		2	//链路确认帧

//终端链路层的分帧处理
//终端分帧处理方式为：通讯时接收到分帧标志，接收分帧，接收到最后一帧后组合成一帧APDU给应用处理
//分帧接收方式：客户机和服务器都有一个发送接收帧尺寸，
int CFaProto::LPduSegFrmHandle()
{
	int nRet = -1;

	if (m_LnkComm.wAPDULen > LPDUSIZE)
	{
		DTRACE(DB_FAPROTO, ("LPduSegFrmHandle::LPDU size:%d is too big !!\r\n", m_LnkComm.wAPDULen));
		return -1;
	}
		
	if (m_LnkComm.FrmSegType == LNK_SEGFRM_START)	//链路分帧第一帧
	{
		memset((BYTE*)&m_RxLPduPool, 0, sizeof(m_RxLPduPool));
		m_RxLPduPool.LPdu[0].wLen =  m_LnkComm.wAPDULen - 2;	//帧长度 -2 减去CS校验两个字节
		m_RxLPduPool.LPdu[0].wFrmNo = m_LnkComm.wRcvFrmSegNo;	//帧序号
		memcpy(m_RxLPduPool.LPdu[0].bBuf, &m_bRxBuf[m_LnkComm.bAPDUoffset], m_RxLPduPool.LPdu[0].wLen);	//帧数据
		m_RxLPduPool.bPduNum = 1;

		nRet =  MakeSegFrm(LNK_SEGFRM_CFM, m_LnkComm.wRcvFrmSegNo);
	}
	else if (m_LnkComm.FrmSegType == LNK_SEGFRM_MID || m_LnkComm.FrmSegType == LNK_SEGFRM_FIN) //链路分帧中间帧,需要校验帧序号吧
	{
		WORD wLastSegNo = m_RxLPduPool.LPdu[m_RxLPduPool.bPduNum-1].wFrmNo;	//上一帧的帧序号
		if (m_LnkComm.wRcvFrmSegNo != wLastSegNo+1)
		{
			DTRACE(DB_FAPROTO, ("LPduSegFrmHandle::LPDU frm seg err, wRcvFrmSegNo:%d, correct-No:%d !!\r\n", m_LnkComm.wRcvFrmSegNo, wLastSegNo+1));
			return -1;
		}

		if (m_RxLPduPool.bPduNum > MAXLPDUNUM)
		{
			DTRACE(DB_FAPROTO, ("LPduSegFrmHandle::LPDU error bPduNum=%d > MAXLPDUNUM=%d.\r\n", m_RxLPduPool.bPduNum, MAXLPDUNUM));
			return -1;
		}

		m_RxLPduPool.LPdu[m_RxLPduPool.bPduNum].wLen =  m_LnkComm.wAPDULen - 2;	//帧长度 -2 减去CS校验两个字节
		m_RxLPduPool.LPdu[m_RxLPduPool.bPduNum].wFrmNo = m_LnkComm.wRcvFrmSegNo;	//帧序号
		memcpy(m_RxLPduPool.LPdu[m_RxLPduPool.bPduNum].bBuf, &m_bRxBuf[m_LnkComm.bAPDUoffset], m_RxLPduPool.LPdu[m_RxLPduPool.bPduNum].wLen);	//帧数据
		m_RxLPduPool.bPduNum++;

		nRet = MakeSegFrm(LNK_SEGFRM_CFM, m_LnkComm.wRcvFrmSegNo);

		if (m_LnkComm.FrmSegType == LNK_SEGFRM_FIN)	//如果是最后一帧，需要组帧为一个完整APDU
		{
			if (CombinApduFrm(m_RxLPduPool, &m_RxDPool))
				return 0;
		}
	}
	
	if (nRet > 0)
		return 1;
	else
		return -1;
}

//描述：将接收到LPDU分帧组合成一帧完整的APDU
bool CFaProto::CombinApduFrm(TLPduPool RxLpduPool, TDataPool* ptRxApduPool)
{
	BYTE* ptr = ptRxApduPool->bBuf;
	BYTE* ptr0 = ptr;
	ptRxApduPool->fValid = true;
	for (int i=0; i<RxLpduPool.bPduNum; i++)
	{
		WORD wLpduLen = RxLpduPool.LPdu[i].wLen;
		if (wLpduLen == 0)
			continue;
		WORD wApduLen = ptr-ptr0;
		if ((wApduLen+wLpduLen) > MAXDATASIZE)
		{
			DTRACE(DB_FAPROTO, ("CombinApduFrm::LPDU size:%d overflow !!\r\n", wApduLen+wLpduLen));
			return false;
		}

		memcpy(ptr, RxLpduPool.LPdu[i].bBuf, RxLpduPool.LPdu[i].wLen);
		ptr += RxLpduPool.LPdu[i].wLen;
	}
	ptRxApduPool->wLen = ptr-ptr0;

	return true;
}

//描述：终端帧处理
bool  CFaProto::HandleFrm()
{
	int nRet;

	if( VeryFrm() < 0 )
	{
		DTRACE(DB_FAPROTO,("DB_FAPROTO:FrmType err!\r\n"));
		return false;
	}
	
	if (m_LnkComm.wAPDULen == 2)//如果应用层只有两个字节，应该是确认帧，需要终端发送下一个APDU片段
	{
		if (m_LnkComm.wRcvFrmSegNo == m_LnkComm.wSendFrmSegNo)
		{
			nRet = DoLPdu();
			return nRet;
		}
		else
		{
			DTRACE(DB_FAPROTO,("HandleFrm: Get a wrong SegNo, Send SegNo=%d, Receive SegNo:%d.\r\n", m_LnkComm.wSendFrmSegNo, m_LnkComm.wRcvFrmSegNo));
			return -1;
		}
	}
	else if (m_LnkComm.fIsSegSend)	//如果是分帧
	{
		nRet = LPduSegFrmHandle();
		if (nRet != 0)	//为0，表示链路分帧接收完毕，组合成一帧APDU，可以处理了
			return nRet;
	}

	//在应用层APDU之外，偏移包括：起始字符(1Byte)+长度2Byte+控制域1Byte+地址AF(1Byte)+服务器地址SA(bSvrAddLen)+客户地址CA(1Byte)+HCS(2Byte)
	BYTE *pRxApdu = m_RxDPool.bBuf;
	BYTE *pTxApdu = m_TxDPool.bBuf;

	//SECURITY layer
	if ((*pRxApdu==SECURITY_REQ) || (*pRxApdu==SECURITY_RES))
	{
		int iPos = 0;
		m_SecurityParam.fSecurityLayer = true;
		switch(*pRxApdu)
		{
		case SECURITY_REQ://SECURITY-Request
			iPos = SecurityRequest(pRxApdu, &m_LnkComm.wAPDULen);
			if (iPos < 0)
			{
				DTRACE(DB_FAPROTO,("HandleFrm: Handle SecurityRequest fail!!\r\n"));
				SetInfo(INFO_ESAM_AUTH_FAIL);
				MakeSecureErrFrm();
				return false; 
			}
			pRxApdu += iPos;
			break;

		case SECURITY_RES://SECURITY-Response
			iPos = SecurityResponse(pRxApdu, &m_LnkComm.wAPDULen);
			if (iPos < 0)
			{
				DTRACE(DB_FAPROTO,("HandleFrm: Handle SecurityResponse fail!!\r\n"));
				SetInfo(INFO_ESAM_AUTH_FAIL);
				return false; 
			}
			pRxApdu += iPos;
			break;
			
		default:
			break;
		}
	}
	else
	{
		m_SecurityParam.fSecurityLayer = false;
	}
	
	switch(*pRxApdu)
	{
		case LINK_REQ:
			Link_Request(pRxApdu, m_LnkComm.wAPDULen);
			return true;

		case LINK_RESPONSE:	//登录/心跳/断开连接的请求，预连接LINK，实际是终端(服务器)发起的，主站响应，故此处可以不处理
			Link_Responce(pRxApdu, m_LnkComm.wAPDULen);
			return true;

		case CONNECT_REQ://connect-request，本服务由客户机应用进程调用，终端回答
			Connect_response(pRxApdu);
			return true;

		case RELEASE_REQ://Release-request
			Release_response(pRxApdu);
			return true;

		case GET_REQ://Get-request
			Get_response(pRxApdu);
			return true;

		case SET_REQ://set-request
			Set_response(pRxApdu, m_LnkComm.wAPDULen);
			return true;

		case ACTION_REQ://Act-request
			Act_response(pRxApdu, m_LnkComm.wAPDULen);
			return true;

		case REPORT_RES://report-request
			Rpt_response(pRxApdu, m_LnkComm.wAPDULen);
			break;
		case PROXY_REQ://Proxy-request
			ProxyResponse();
			break;

	}

	return false;
}

//描述：链路分帧上报
//参数：bType：分帧类型
//		wSeg：分帧序号
int CFaProto::MakeSegFrm(BYTE bType, WORD wSeg)
{
	BYTE bBuf[6];
	bBuf[0] = (BYTE )wSeg;
	bBuf[1] = (BYTE)(wSeg>>8) + (bType<<6);
	
	return MakeFrm(bBuf, 2);
}

int CFaProto::MakeLinkFrm(BYTE bLinkSvr)
{
	BYTE bApdu[16];
	BYTE bLen;

	bApdu[0] = 0x01;		//link-request
	bApdu[1] = 0x00;		//PIID-ACD
	bApdu[2] = bLinkSvr;	//建立连接
	
	//心跳间隔
	BYTE bModuleType;
	//ReadItemEx(BANK2, PN0, 0x10d3, &bModuleType);
	ReadItemEx(BN2, PN0, 0x2050, &bModuleType);
	WORD wBeat = 0;
	if (bModuleType == MODULE_SOCKET)
		wBeat = GetEthBeat();
	else
		wBeat = GetGprsBeat();

	if (wBeat == 0)
		wBeat = 300;	//默认300s
	bApdu[3] = (BYTE )(wBeat>>8);
	bApdu[4] = (BYTE )wBeat;

	//请求时间
	TMillTime now;
	GetCurMillTime(&now);

	bApdu[5] = now.nYear/256;
	bApdu[6] = now.nYear%256;
	bApdu[7] = now.nMonth;
	bApdu[8] = now.nDay;
	bApdu[9] = now.nDayOfWeek;
	bApdu[10] = now.nHour;
	bApdu[11] = now.nMinute;
	bApdu[12] = now.nSecond;
	bApdu[13] = now.nMilliseconds/256;
	bApdu[14] = now.nMilliseconds%256;

	return MakeFrm(bApdu, 15);
}

//描述：终端登录主站
//发送帧：68 1D 00 81 05 07 09 19 05 16 20 00 CS CS 01 00 00 00 B4 20 16 05 19 08 05 00 00 A4 CS CS 16
//接受帧：68 2D 00 01 05 07 09 19 05 16 20 10 CS CS 81 00 80 20 16 05 19 08 05 00 00 89 20 16 05 19 08 05 01 02 5F 20 16 05 19 08 05 02 02 DA CS CS 16
bool CFaProto::Login()
{
	BYTE bApdu[12], bBuf[12];
	BYTE bLen;

	MakeLinkFrm(SVR_LINK);
	Sleep(200);
		
	DWORD dwOldClick = GetClick();
	DTRACE(DB_FAPROTO, ("CFaProto::Login : at click %ld\n", dwOldClick));

	//在应用层APDU之外，偏移包括：起始字符(1Byte)+长度2Byte+控制域1Byte+地址AF(1Byte)+服务器地址SA(bSvrAddLen)+客户地址CA(1Byte)+HCS(2Byte)
	//BYTE bAPDUOffset = 8+m_LnkComm.bSvrAddLen; 
	do
	{
		if (RcvFrm())
		{
			if (m_bRxBuf[m_LnkComm.bAPDUoffset] == LINK_RESPONSE && 
				(m_bRxBuf[m_LnkComm.bAPDUoffset+2]&0x03) == 0x00)
			{
				DTRACE(DB_FAPROTO, ("CFaProto::Login : login ok\n"));
				if (m_LnkComm.bCommStep < 1)
					m_LnkComm.bCommStep = 1;

				Sleep(1000);
				return true;
			}
		}
		Sleep(100);
	} while (GetClick()-dwOldClick < 30);//m_pGbPara->wConfirmDelayTime);   //12秒后重发

	DTRACE(DB_FAPROTO, ("CFaProto::Login : login fail\n"));
	return false;
}

//描述：终端心跳
bool CFaProto::Beat()
{
	MakeLinkFrm(SVR_BEAT);
	return true;
}

//描述：应用链接请求(先简单处理，后续在针对不同客户端增加不同的心跳控制)
bool CFaProto::Link_Request(BYTE *pApdu, WORD wApduLen)
{
	TMillTime tMillTime;
	BYTE bTxApdu[128];
	BYTE *pbTx = bTxApdu;

	*pbTx++ = LINK_RESPONSE;
	pApdu++;
	*pbTx++ = *pApdu++;
	*pbTx++ = 0x80;
	pApdu += 3;	//跳过请求类型（1BYTE）+ 心跳周期（2BYTE）
	memcpy(pbTx, pApdu, 10);	//请求时间
	pbTx += 10;
	GetCurMillTime(&tMillTime);	//收到时间
	*pbTx++ = tMillTime.nYear/256;
	*pbTx++ = tMillTime.nYear%256;
	*pbTx++ = tMillTime.nMonth;
	*pbTx++ = tMillTime.nDay;
	*pbTx++ = tMillTime.nDayOfWeek;
	*pbTx++ = tMillTime.nHour;
	*pbTx++ = tMillTime.nMinute;
	*pbTx++ = tMillTime.nSecond;
	*pbTx++ = tMillTime.nMilliseconds/256;
	*pbTx++ = tMillTime.nMilliseconds%256;
	GetCurMillTime(&tMillTime);	//响应时间
	*pbTx++ = tMillTime.nYear/256;
	*pbTx++ = tMillTime.nYear%256;
	*pbTx++ = tMillTime.nMonth;
	*pbTx++ = tMillTime.nDay;
	*pbTx++ = tMillTime.nDayOfWeek;
	*pbTx++ = tMillTime.nHour;
	*pbTx++ = tMillTime.nMinute;
	*pbTx++ = tMillTime.nSecond;
	*pbTx++ = tMillTime.nMilliseconds/256;
	*pbTx++ = tMillTime.nMilliseconds%256;

	return MakeFrm(bTxApdu, pbTx - bTxApdu);
}

typedef struct {
	BYTE bBeatCnt;		//最近心跳时间总个数
	BYTE bMaxDelCnt;	//最大值剔除个数
	BYTE bMinDelCnt;	//最小值剔除个数
	BYTE bCommDlyRate;	//通讯延时阈值
	BYTE bMinValidCnt;	//最少有效个数
}TPreAdjPara;	//精准校时参数

//描述：预链接响应(实现精准对时功能)
bool CFaProto::Link_Responce(BYTE *pApdu, WORD wApduLen)
{
	TPreAdjPara tPreAdjPara;
	TMillTime tMillTime;
	TTime tTime;
	char szBuf[32]= {0};
	char szTermReqTimeBuf[32];
	char szTermCurTimeBuf[32];
	char szMastRcvTimeBuf[32];
	char szMastRespTimeBuf[32];
	BYTE bBuf[32];
	BYTE bTimeFlg;
	int T1;	//终端发送时间
	int T2;	//主站接收时间
	int T3;	//主站响应时间
	int T4;	//终端接收时间
	int U;
	int V;
	int K;

	pApdu++;	//LINK_Responce
	pApdu++;	//PIID
	bTimeFlg = *pApdu++;	//时间可信标识

	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BANK0, PN0, 0x4520, bBuf);
	if (bBuf[0]==DT_ENUM && bBuf[1]==0x01)	//终端精确校时
	{
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BANK0, PN0, 0x4521, bBuf);
		if (bBuf[0]==DT_STRUCT && bBuf[1]==0x05)
		{
			m_bBeatNum++;
			if ((bTimeFlg & 0x80) == 0x80)	//时间可信?
			{
				memset((BYTE*)&tPreAdjPara, 0, sizeof(tPreAdjPara));
				tPreAdjPara.bBeatCnt = bBuf[3];
				tPreAdjPara.bMaxDelCnt = bBuf[5];
				tPreAdjPara.bMinDelCnt = bBuf[7];
				tPreAdjPara.bCommDlyRate = bBuf[9];
				tPreAdjPara.bMinValidCnt = bBuf[11];

				memset((BYTE*)&tMillTime, 0, sizeof(tMillTime));
				OoDateTimeToMillTime(pApdu, &tMillTime);
				T1 = MilTimeToSeconds(tMillTime);
				MillTimeToStr(tMillTime, szTermReqTimeBuf);
				pApdu += 10;

				memset((BYTE*)&tMillTime, 0, sizeof(tMillTime));
				OoDateTimeToMillTime(pApdu, &tMillTime);
				T2 = MilTimeToSeconds(tMillTime);
				MillTimeToStr(tMillTime, szMastRcvTimeBuf);
				pApdu += 10;

				memset((BYTE*)&tMillTime, 0, sizeof(tMillTime));
				OoDateTimeToMillTime(pApdu, &tMillTime);
				T3 = MilTimeToSeconds(tMillTime);
				MillTimeToStr(tMillTime, szMastRespTimeBuf);
				pApdu += 10;

				GetCurMillTime(&tMillTime);
				T4 = MilTimeToSeconds(tMillTime);
				MillTimeToStr(tMillTime, szTermCurTimeBuf);

				U = T2 - T1;
				V = T4 - T3;
				K = (U-V)/2;

				m_iK[m_bValidNum++] = K;

				if (!m_fPwrOnRun)	//首次运行，简单对时
				{
					m_fPwrOnRun = true;
					if (abs(T4-T3) > 300)	//300s=5min
					{
						TTime tm;
						GetCurTime(&tm);
						g_AdjTermTime.bClock[0] = DT_DATE_TIME_S;
						OoTimeToDateTimeS(&tm, &g_AdjTermTime.bClock[1]);	//记录校时前时间

						SecondsToTime(T3, &tTime);
						if (SetSysTime(tTime))
						{
							//SetInfo(INFO_ADJ_TERM_TIME);
							DealSpecTrigerEvt(TERM_CLOCKPRG);	//HYL 直接存储终端对时事件
							DTRACE(DB_FAPROTO, ("Simple adjust time successful, Master time:%s, Term time=%s.\n", szMastRespTimeBuf, szTermCurTimeBuf));
						}
						else
							DTRACE(DB_FAPROTO, ("Simple adjust time fail, Master time:%s, Term time=%s.\n", szMastRespTimeBuf, szTermCurTimeBuf));
						
						m_bBeatNum = 0;	//设置为0，开始精确对时
					}
				}

				if (m_bBeatNum >= tPreAdjPara.bBeatCnt)
				{
					int iAdjSec = 0;

					//可信心跳排序
					IntSort(m_iK, m_bValidNum);

					//最小值剔除个数
					if (m_bValidNum > tPreAdjPara.bMinDelCnt)
					{
						for (WORD i=0; i<m_bValidNum; i++)
						{
							if (i >= tPreAdjPara.bMinDelCnt)
								m_iK[i-tPreAdjPara.bMinDelCnt] = m_iK[i];
						}
						m_bValidNum -= tPreAdjPara.bMinDelCnt;

						//最大值剔除个数
						if (m_bValidNum>tPreAdjPara.bMaxDelCnt && m_bValidNum!=0)
						{
							//余下心跳个数是否大于“最少有效个数”
							if (m_bValidNum > tPreAdjPara.bMinValidCnt)
							{
								DWORD dwTime = GetCurTime();

								for (WORD i=0; i<m_bValidNum; i++)
								{
									iAdjSec += m_iK[i];	
									DTRACE(DB_FAPROTO, ("Precise adjust time0, iAdjSec=%d, m_iK[%d]=%d.\n", iAdjSec, i, m_iK[i]));
								}
								
								DTRACE(DB_FAPROTO, ("Precise adjust time1, iAdjSec=%d, m_bValidNum=%d, bComDlyTime=%d.\n", iAdjSec, m_bValidNum, tPreAdjPara.bCommDlyRate));
								iAdjSec /= m_bValidNum;
								DTRACE(DB_FAPROTO, ("Precise adjust time2, iAdjSec=%d, m_bValidNum=%d, bComDlyTime=%d.\n", iAdjSec, m_bValidNum, tPreAdjPara.bCommDlyRate));

								if (abs(iAdjSec) > tPreAdjPara.bCommDlyRate)
								{
									TTime tm;
									GetCurTime(&tm);
									g_AdjTermTime.bClock[0] = DT_DATE_TIME_S;
									OoTimeToDateTimeS(&tm, &g_AdjTermTime.bClock[1]);	//记录校时前时间

									dwTime += iAdjSec;
									SecondsToTime(dwTime, &tTime);
									if (SetSysTime(tTime))
									{
										//SetInfo(INFO_ADJ_TERM_TIME);
										DealSpecTrigerEvt(TERM_CLOCKPRG);	//HYL 直接存储终端对时事件
										DTRACE(DB_FAPROTO, ("Precise adjust time successful, Time:%s.\n", TimeToStr(tTime, szBuf)));
									}
									else
										DTRACE(DB_FAPROTO, ("Precise adjust time fail, Time:%s.\n", TimeToStr(tTime, szBuf)));
								}
								else
									DTRACE(DB_FAPROTO, ("Precise adjust time fail, iAdjSec=%d, bComDlyTime=%d.\n", iAdjSec, tPreAdjPara.bCommDlyRate));
							}
							else
								DTRACE(DB_FAPROTO, ("Precise adjust time fail, m_bValidNum=%d, bMinValidNum=%d.\n", m_bValidNum, tPreAdjPara.bMinValidCnt));
						}
						else
							DTRACE(DB_FAPROTO, ("Precise adjust time fail, m_bValidNum=%d, bMaxDelNum=%d.\n", m_bValidNum, tPreAdjPara.bMaxDelCnt));
					}
					else
						DTRACE(DB_FAPROTO, ("Precise adjust time fail, m_bValidNum=%d, bMinDelNum=%d.\n", m_bValidNum, tPreAdjPara.bMinDelCnt));

					m_bBeatNum = 0;
					m_bValidNum = 0;
					memset((BYTE*)&m_iK, 0, sizeof(m_iK));
				}

				DTRACE(DB_FAPROTO, ("Precise adjust time: bMaxBeatNum=%d, m_bBeatNum=%d, m_bValidNum=%d, T1:%s, T2:%s, T3:%s, T4:%s.\n", \
					tPreAdjPara.bBeatCnt, m_bBeatNum, m_bValidNum, szTermReqTimeBuf, szMastRcvTimeBuf, szMastRespTimeBuf, szTermCurTimeBuf));
			}
			else
				DTRACE(DB_FAPROTO, ("Precise adjust time: Time trust indclock error.\n"));
		}
	}

	return true;
}

/*
MakeSignatureSecurity

*/
BYTE MakeSignatureSecurity(BYTE *pApdu, BYTE *pSecurityData, BYTE *pSecurityDataLen)
{
	BYTE bOutSessionInit[128];
	BYTE bOutSign[128];
	BYTE bSessionData[128];
	BYTE bSign[128];
	DWORD dwOutSessionInitLen, dwOutSignLen;
	BYTE bSessionDataLen = sizeof(bSessionData);
	BYTE bSignLen = sizeof(bSign);
	int iRet;
	int iPos;
	BYTE bSignResult;
	BYTE *p1 = pSecurityData;
	*pSecurityDataLen = 0;


	//获取密文2与客户机签名2
	//密文2		 octet-string
	iPos = DecodeLength(pApdu, &dwOutSessionInitLen);
	if (dwOutSessionInitLen > sizeof(bOutSessionInit))
	{
		DTRACE(DB_FAPROTO, ("MakeSignatureSecurity: OutSessionInit too long (%d) !!\r\n", dwOutSessionInitLen));
		bSignResult = 3; //非对称解密错误       （3），
		return bSignResult;
	}
	pApdu += iPos;
	memcpy(bOutSessionInit, pApdu, dwOutSessionInitLen);
	pApdu += dwOutSessionInitLen;
	
	//客户机签名2  octet-string
	iPos = DecodeLength(pApdu, &dwOutSignLen);
	if (dwOutSignLen > sizeof(bOutSign))
	{
		DTRACE(DB_FAPROTO, ("MakeSignatureSecurity: OutSign too long (%d) !!\r\n", dwOutSignLen));
		bSignResult = 4; //签名错误             （4），
		return bSignResult;
	}
	pApdu += iPos;
	memcpy(bOutSign, pApdu, dwOutSignLen);
	//pApdu += dwOutSignLen;

	//ESAM模块验证
	iRet = Esam_InitSession(bOutSessionInit, (BYTE)dwOutSessionInitLen, bOutSign, (BYTE)dwOutSignLen,
		bSessionData, &bSessionDataLen, bSign, &bSignLen);
	if (iRet < 0)
	{
		DTRACE(DB_FAPROTO, ("MakeSignatureSecurity: Esam_InitSession fail !!\r\n"));
		bSignResult = 4; //签名错误             （4），
		return bSignResult;
	}

	//刷新计数器
	Esam_ReflashCounter();

	//编码APDU
	iPos = EncodeLength(bSessionDataLen, p1);
	p1 += iPos;
	memcpy(p1, bSessionData, bSessionDataLen);
	p1 += bSessionDataLen;

	iPos = EncodeLength(bSignLen, p1);
	p1 += iPos;
	memcpy(p1, bSign, bSignLen);
	p1 += bSignLen;

	*pSecurityDataLen = p1 - pSecurityData;

	bSignResult = 0; //允许建立应用连接     （0），
	return bSignResult;
	
}

//描述：终端连接回应
//参数：pApdu：接收到的应用部分数据
//事例帧：02 00 00 10 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 04 00 04 00 01 04 00 00 00 00 64 00 00
void CFaProto::Connect_response(BYTE* pApdu)
{
	BYTE bBuf[256], bPtr=0;
	BYTE bConnectResult = 0; //允许建立应用连接     （0），
	BYTE bSecurityData[128];
	BYTE bSecurityDataLen = 0;

	pApdu += 2;	//connect-request(1 byte) + PIID(1 byte) 
	memcpy(m_LnkComm.tRxTrsPara.tConnPara.bProtoVer, pApdu, 2);
	pApdu += 2;	//proto-version

	memcpy(m_LnkComm.tRxTrsPara.tConnPara.bProConformance, pApdu, 8);
	pApdu += 8;//protocol-conformance

	memcpy(m_LnkComm.tRxTrsPara.tConnPara.bFunConformance, pApdu, 16);
	pApdu += 16;//function-conformance

	m_LnkComm.tRxTrsPara.tConnPara.wRcvFrmMaxLen = OIBinToVal(pApdu, 2);
	pApdu += 2;	//cli-send-max-size

	m_LnkComm.tRxTrsPara.tConnPara.wSenFrmMaxLen = OIBinToVal(pApdu, 2);
	pApdu += 2;	//cli-rcv-max-size

	m_LnkComm.tRxTrsPara.tConnPara.bRcvWindows = *pApdu++;

	m_LnkComm.tRxTrsPara.tConnPara.wHandleApduSize = OIBinToVal(pApdu, 2);
	pApdu += 2;	//cli-max-handle-apdu-size

	m_LnkComm.tRxTrsPara.tConnPara.dwConnectTimeOut = OIBinToVal(pApdu, 4);
	pApdu += 4;	//connect-timeout

	//memcpy(m_LnkComm.tRxTrsPara.bMechanismInfo, pApdu, 2);
	//bMechanismType = *pApdu;
	switch(*pApdu) //认证请求对象 ConnectMechanismInfo
	{
	case NullSecurity:
		DTRACE(DB_FAPROTO, ("Connect_response: NullSecurity !!\r\n"));
		break;
	case PasswordSecurity:
		bConnectResult = 1; //密码错误             （1），
		DTRACE(DB_FAPROTO, ("PasswordSecurity not support !!\r\n"));
		break;
	case SymmetrySecurity:
		bConnectResult = 2; //对称解密错误         （2）
		DTRACE(DB_FAPROTO, ("SymmetrySecurity not support !!\r\n"));
		break;
	case SignatureSecurity:
		DTRACE(DB_FAPROTO, ("Connect_response: SignatureSecurity !!\r\n"));
		pApdu++;
		bConnectResult = MakeSignatureSecurity(pApdu, bSecurityData, &bSecurityDataLen);
		break;
	default:
		break;
	}

//终端回应
	BYTE *p = bBuf;
	memcpy(p, &g_bTermSoftVer[4], g_bTermSoftVer[3]);
	p += 4;
	memcpy(p, &g_bTermSoftVer[10], g_bTermSoftVer[9]);
	p += 4;
	memcpy(p, &g_bTermSoftVer[16], g_bTermSoftVer[15]);
	p += 6;
	memcpy(p, &g_bTermSoftVer[24], g_bTermSoftVer[23]);
	p += 4;
	memcpy(p, &g_bTermSoftVer[30], g_bTermSoftVer[29]);
	p += 6;
	memcpy(p, &g_bTermSoftVer[38], g_bTermSoftVer[37]);
	p += 8;
	memcpy((BYTE *)&m_LnkComm.tTxTrsPara.tTermnInfo, bBuf, p-bBuf);	//厂商版本信息
	memcpy(m_LnkComm.tTxTrsPara.tConnPara.bProtoVer, m_LnkComm.tRxTrsPara.tConnPara.bProtoVer, sizeof(m_LnkComm.tRxTrsPara.tConnPara.bProtoVer)); //协议版本

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[bPtr++] = 0x82;
	bBuf[bPtr++] = m_AppComm.bPIID;
	memcpy(&bBuf[bPtr], (BYTE *)&m_LnkComm.tTxTrsPara.tTermnInfo, sizeof(TFacVersion));	//厂家信息
	bPtr += sizeof(TFacVersion);
	memcpy(&bBuf[bPtr], (BYTE *)&m_LnkComm.tTxTrsPara.tConnPara.bProtoVer, 2);	//协议版本
	bPtr += 2;
	memcpy(&bBuf[bPtr], (BYTE *)m_LnkComm.tTxTrsPara.tConnPara.bProConformance, sizeof(m_LnkComm.tTxTrsPara.tConnPara.bProConformance));	//协议一致性
	bPtr += sizeof(m_LnkComm.tTxTrsPara.tConnPara.bProConformance);

	memcpy(&bBuf[bPtr], (BYTE *)m_LnkComm.tTxTrsPara.tConnPara.bFunConformance, sizeof(m_LnkComm.tTxTrsPara.tConnPara.bFunConformance));	//功能一致性
	bPtr += sizeof(m_LnkComm.tTxTrsPara.tConnPara.bFunConformance);

	OIValToBin(m_LnkComm.tTxTrsPara.tConnPara.wSenFrmMaxLen, &bBuf[bPtr], 2);
	bPtr += 2;
	OIValToBin(m_LnkComm.tTxTrsPara.tConnPara.wRcvFrmMaxLen, &bBuf[bPtr], 2);
	bPtr += 2;
	bBuf[bPtr++] = m_LnkComm.tTxTrsPara.tConnPara.bRcvWindows;
	OIValToBin(m_LnkComm.tTxTrsPara.tConnPara.wHandleApduSize, &bBuf[bPtr], 2);
	bPtr += 2;
	OIValToBin(m_LnkComm.tTxTrsPara.tConnPara.dwConnectTimeOut, &bBuf[bPtr], 4);
	bPtr += 4;

	//connectResponseinfo 
	DTRACE(DB_FAPROTO, ("Connect_response: connectResponseinfo pos %d.\r\n", bPtr));
	bBuf[bPtr++] = bConnectResult;
	bBuf[bPtr++] = (bSecurityDataLen>0)?1:0; //data-optional
	memcpy(&bBuf[bPtr], bSecurityData, bSecurityDataLen);
	bPtr += bSecurityDataLen;
	bBuf[bPtr++] = 0x00; //时间标签-optional

	if (MakeFrm(bBuf, bPtr) == bPtr)
		m_LnkComm.bCommStep = 2;
}

//描述：断开应用连接
void CFaProto::Release_response(BYTE* pApdu)
{
	BYTE bBuf[12],bPtr=0;

	m_AppComm.bPIID = pApdu[1];
	bBuf[bPtr++] = RELEASE_RES;
	bBuf[bPtr++] = m_AppComm.bPIID;
	bBuf[bPtr++] = 0x00;
	bBuf[bPtr++] = GetTimeFlg();
	bBuf[bPtr++] = GetRptFlg();

	if (MakeFrm(bBuf, bPtr) == bPtr)
		m_LnkComm.bCommStep = 1;
}

//描述：读取终端数据
int CFaProto::Get_response(BYTE* pApdu)
{
	int iRet;
	WORD wBlkNo;
	WORD wLen;
	BYTE bGetCmd = *pApdu++;

	m_AppComm.bAnsCmdMod = *pApdu++;
	m_AppComm.bPIID = *pApdu++;

	switch(m_AppComm.bAnsCmdMod)
	{
		case GET_NORMAL://get-request-normal
			NewAppServer();
			m_AppComm.bServerMod = GET_NORMAL;
			m_AppComm.pbAskStart = pApdu;
			m_AppComm.pbAskEnd = pApdu + m_LnkComm.wAPDULen-4;//4; 只要OAD+扩展协议数据
			
			wLen = m_AppComm.pbAskEnd - m_AppComm.pbAskStart;
			memcpy(m_AppComm.bAskBuf, m_AppComm.pbAskStart, wLen);
			m_AppComm.pbAskStart = m_AppComm.bAskBuf;
			m_AppComm.pbAskEnd = m_AppComm.bAskBuf+wLen;
			return Get_request_normal();

		case GET_NORMAL_LIST://get-request-normal-list
			NewAppServer();
			m_AppComm.bServerMod = GET_NORMAL_LIST;
			m_AppComm.bAskItemNum = *pApdu++;
			m_AppComm.pbAskStart = pApdu;
			m_AppComm.pbAskEnd = pApdu + (m_AppComm.bAskItemNum * 4);

			wLen = m_AppComm.pbAskEnd - m_AppComm.pbAskStart;
			memcpy(m_AppComm.bAskBuf, m_AppComm.pbAskStart, wLen);
			m_AppComm.pbAskStart = m_AppComm.bAskBuf;
			m_AppComm.pbAskEnd = m_AppComm.bAskBuf+wLen;
			return Get_request_normal_list();

		case GET_RECORD://get-request-record
			NewAppServer();
			m_AppComm.bServerMod = GET_RECORD;
			m_AppComm.pbAskStart = pApdu;
			//OAD
			pApdu += 4;
			//RSD
			iRet = ScanRSD(pApdu, false);
			if (iRet < 0)
			{
				DTRACE(DB_FAPROTO, ("Get_RECORD:ScanRSD error, iRet=%d.\n", iRet));
				return -1;
			}
			pApdu += iRet;
			//RCSD
			iRet = ScanRCSD(pApdu, false);	
			if (iRet < 0)
			{
				DTRACE(DB_FAPROTO, ("Get_RECORD:ScanRCSD error, iRet=%d.\n", iRet));
				return -1;
			}
			pApdu += iRet;
			m_AppComm.pbAskEnd = pApdu;

			wLen = m_AppComm.pbAskEnd - m_AppComm.pbAskStart;
			memcpy(m_AppComm.bAskBuf, m_AppComm.pbAskStart, wLen);
			m_AppComm.pbAskStart = m_AppComm.bAskBuf;
			m_AppComm.pbAskEnd = m_AppComm.bAskBuf+wLen;
			return Get_request_record();

		case GET_RECORD_LIST://get-request-record-list
			NewAppServer();
			m_AppComm.bServerMod = GET_RECORD_LIST;
			m_AppComm.bAskRecItemNum = *pApdu++;
			m_AppComm.pbAskStart = pApdu;
			for (int i=0; i<m_AppComm.bAskRecItemNum; i++)
			{
				//OAD
				pApdu += 4;
				//RSD
				iRet = ScanRSD(pApdu, false);
				if (iRet < 0)
				{
					DTRACE(DB_FAPROTO, ("Get_RECORD_LIST:ScanRSD error, i=%d, iRet=%d.\n", i, iRet));
					return -1;
				}
				pApdu += iRet;
				//RCSD
				iRet = ScanRCSD(pApdu, false);	
				if (iRet < 0)
				{
					DTRACE(DB_FAPROTO, ("Get_RECORD_LIST:ScanRCSD error, i=%d, iRet=%d.\n", i, iRet));
					return -1;
				}
				pApdu += iRet;
			}
			m_AppComm.pbAskEnd = pApdu;

			wLen = m_AppComm.pbAskEnd - m_AppComm.pbAskStart;
			memcpy(m_AppComm.bAskBuf, m_AppComm.pbAskStart, wLen);
			m_AppComm.pbAskStart = m_AppComm.bAskBuf;
			m_AppComm.pbAskEnd = m_AppComm.bAskBuf+wLen;
			return Get_request_record_list();

		case GET_NEXT://get-request-next
			wBlkNo = (WORD )(pApdu[0]<<8) + pApdu[1];
			if (m_AppComm.wBlkNo != (wBlkNo+1))	//首先确认块序号是否正确
			{
				return GetResErr(GET_NEXT, DR_DataBlockErr);
			}
			else
			{
				if (m_TxAPdu.fFinal)	//如果上帧已经是最后一帧，再确认请求就是错误
					return GetResErr( GET_NEXT, DR_DataBlockErr);
			}
			return Get_request_next();

		default:
			break;
	}

	return -1;
}

//描述：处理终端接收到的Get_request_normal命令
int CFaProto::Get_request_normal()
{
	TApduInfo tApduInfo;
	BYTE *pTxApdu = m_TxAPdu.bBuf;
	BYTE *pRxPtr;
	BYTE *pbAskStart = m_AppComm.pbAskStart;
	WORD wMaxRxLen,wLen;
	int nRet;

	memset((BYTE*)&tApduInfo, 0, sizeof(tApduInfo));
	tApduInfo.wOI = OoOiToWord(pbAskStart);
	pbAskStart += 2;
	tApduInfo.bAttr = *pbAskStart++;
	tApduInfo.bIndex = *pbAskStart++;

	pRxPtr = pTxApdu + 16;
	wMaxRxLen = sizeof(m_TxAPdu.bBuf) - (pRxPtr-pTxApdu);
	wLen = m_AppComm.pbAskEnd - m_AppComm.pbAskStart-4;//去掉OAD
	
	*pTxApdu++ = GET_RES;	//Get-response-normal
	
	if(tApduInfo.wOI==0xFFF0)
	{//科陆内部扩展帧，需特殊处理，lzx20170222
		nRet = ZJUserDef(pbAskStart, wLen, pRxPtr);
	}
	else
	{
		nRet = OoProReadAttr(tApduInfo.wOI, tApduInfo.bAttr, tApduInfo.bIndex, pRxPtr, wMaxRxLen, &m_AppComm.iStep);
	}
	if (nRet <= 0)
	{
		*pTxApdu++ = GET_NORMAL;
		*pTxApdu++ = m_AppComm.bPIID;
		*pTxApdu++ = (BYTE)(tApduInfo.wOI>>8);
		*pTxApdu++ = (BYTE)tApduInfo.wOI;
		*pTxApdu++ = tApduInfo.bAttr;
		*pTxApdu++ = tApduInfo.bIndex;

		*pTxApdu++ = 0x00;				//DAR
		*pTxApdu++ = GetErrOfGet(nRet);	//对象不存在
	}
	else if (m_AppComm.fNewServer)	//开始读取
	{
		m_AppComm.fNewServer = false;
		if (m_AppComm.iStep < 0)	//一帧就读取完成了
		{
			*pTxApdu++ = GET_NORMAL;
			*pTxApdu++ = m_AppComm.bPIID;
		}
		else
		{
			*pTxApdu++ = GET_NEXT;
			*pTxApdu++ = m_AppComm.bPIID;
			*pTxApdu++ = 0x00;	//末帧标志
			*pTxApdu++ = (BYTE )(m_AppComm.wBlkNo>>8);	//分帧序号
			*pTxApdu++ = (BYTE )m_AppComm.wBlkNo;
			m_AppComm.wBlkNo++;
			//Choice 分帧响应
			*pTxApdu++ = 0x01;		
			*pTxApdu++ = 0x01;		
			m_TxAPdu.fFinal = false;
		}
		*pTxApdu++ = (BYTE)(tApduInfo.wOI>>8);
		*pTxApdu++ = (BYTE)tApduInfo.wOI;
		*pTxApdu++ = tApduInfo.bAttr;
		*pTxApdu++ = tApduInfo.bIndex;

		*pTxApdu++ = 0x01;	//Data
		memcpy(pTxApdu, pRxPtr, nRet);
		pTxApdu += nRet;
	}
	else
	{
		*pTxApdu++ = GET_NEXT;			//get-response-next
		*pTxApdu++ = m_AppComm.bPIID;
		if (m_AppComm.iStep < 0)		//读取完成了
		{
			*pTxApdu++ = 0x01;			//末帧标志
			m_TxAPdu.fFinal = true;
		}
		else
		{
			*pTxApdu++ = 0x00;			//末帧标志
			m_TxAPdu.fFinal = false;
		}

		*pTxApdu++ = (BYTE)(m_AppComm.wBlkNo>>8);	//分帧序号
		*pTxApdu++ = (BYTE)(m_AppComm.wBlkNo);
		m_AppComm.wBlkNo++;
		//Choice 分帧响应
		*pTxApdu++ = 0x01;		
		*pTxApdu++ = 0x01;		
		*pTxApdu++ = (BYTE)(tApduInfo.wOI>>8);
		*pTxApdu++ = (BYTE)tApduInfo.wOI;
		*pTxApdu++ = tApduInfo.bAttr;
		*pTxApdu++ = tApduInfo.bIndex;

		*pTxApdu++ = 0x01;	//Data
		memcpy(pTxApdu, pRxPtr, nRet);	//如果是电表档案，在数据开始部分要有01 **，(**代表电表档案的个数)
		pTxApdu += nRet;		
	}

	*pTxApdu++ = 0x00;	//表示没有上报信息
	*pTxApdu++ = 0x00;	//没有时间标签
	
	m_TxAPdu.wLen = pTxApdu - m_TxAPdu.bBuf;
	return ToSecurityLayer(); //ToLnkLayer();
}

int CFaProto::Get_request_normal_list()
{
	TApduInfo tApduInfo;
	int iBakStep;
	BYTE *pbApdu0 = m_TxAPdu.bBuf;
	BYTE *pbApdu;
	BYTE *pbApdu1;
	BYTE *pbApdu2;
	BYTE *pbRxPtr;
	WORD wRxMaxLen;
	BYTE *pbAskStart;

	pbApdu = pbApdu0;
	pbApdu1 = pbApdu0 + 10;	//预留10个字节空间用来存储应用分帧信息
	pbApdu2 = pbApdu1;

	BYTE bGetNum=0;
	while(m_AppComm.pbAskStart < m_AppComm.pbAskEnd)
	{
		if (m_AppComm.fNewServer)
		{
			m_AppComm.fNewServer = false;
			m_AppComm.iStep = -1;
			m_AppComm.pbAskMid = m_AppComm.pbAskStart;
		}
		else if(m_AppComm.pbAskStart != m_AppComm.pbAskMid)//请求的对象变化，抄读起始序号要置位
		{
			m_AppComm.iStep = -1;
			m_AppComm.pbAskMid = m_AppComm.pbAskStart;
		}

		memset((BYTE*)&tApduInfo, 0, sizeof(tApduInfo));
		pbAskStart = m_AppComm.pbAskStart;
		tApduInfo.wOI = OoOiToWord(pbAskStart);
		pbAskStart += 2;
		tApduInfo.bAttr = *pbAskStart++;
		tApduInfo.bIndex = *pbAskStart++;

		iBakStep = m_AppComm.iStep;
		pbRxPtr = pbApdu2 + 16;	//预留16个字节的空间用来存储一个OAD的OI\ATTR\OI相应的信息
		wRxMaxLen = sizeof(m_TxAPdu.bBuf) - (pbApdu2 - pbApdu0);
		int nRet = OoProReadAttr(tApduInfo.wOI, tApduInfo.bAttr, tApduInfo.bIndex, pbRxPtr, wRxMaxLen, &m_AppComm.iStep);
		if (nRet < 0)	//数据抄读失败了
		{
			if (int(sizeof(m_TxAPdu.bBuf) - (pbApdu2 - pbApdu0)) < 6)	//数据是否溢出
			{
				if (m_AppComm.iStep >= 0)
					m_AppComm.iStep = iBakStep;
				break;
			}
			else
			{
				*pbApdu2++ = tApduInfo.wOI>>8;
				*pbApdu2++ = tApduInfo.wOI;
				*pbApdu2++ = tApduInfo.bAttr;
				*pbApdu2++ = tApduInfo.bIndex;
				*pbApdu2++ = 0x00;	//Dar
				*pbApdu2++ = GetErrOfGet(nRet);
				bGetNum++;
			}
		}
		else
		{
			if (int(sizeof(m_TxAPdu.bBuf) - (pbApdu2 - pbApdu0) - nRet) < 6)	//数据是否溢出
			{
				if (m_AppComm.iStep >= 0)
					m_AppComm.iStep = iBakStep;
				break;
			}
			else
			{
				*pbApdu2++ = tApduInfo.wOI>>8;
				*pbApdu2++ = tApduInfo.wOI;
				*pbApdu2++ = tApduInfo.bAttr;
				*pbApdu2++ = tApduInfo.bIndex;
				*pbApdu2++ = 0x01;	//Data
				memcpy(pbApdu2, pbRxPtr, nRet);
				pbApdu2 += nRet;
				bGetNum++;
			}
		}
		if (m_AppComm.iStep == -1)
			m_AppComm.pbAskStart += 4;
	}

	if (m_AppComm.wBlkNo==1 && m_AppComm.pbAskStart==m_AppComm.pbAskEnd)//一帧就直接抄读完成了
	{
		*pbApdu++ = GET_RESPONSE;
		*pbApdu++ = GET_NORMAL_LIST;
		*pbApdu++ = m_AppComm.bPIID;
		*pbApdu++ = bGetNum;
		memcpy(pbApdu, pbApdu1, pbApdu2-pbApdu1);
		pbApdu += (pbApdu2-pbApdu1);
	}
	else// if (pApdu - m_TxAPdu.bBuf > sizeof(bBuf))	//分帧传输
	{
		*pbApdu++ = GET_RESPONSE;
		*pbApdu++ = GET_NEXT;
		*pbApdu++ = m_AppComm.bPIID;
		if (m_AppComm.pbAskStart == m_AppComm.pbAskEnd)	//抄读完成，最后一帧
		{
			*pbApdu++ = 0x01;		//最后一帧
			m_TxAPdu.fFinal = true;
		}
		else
		{
			*pbApdu++ = 0x00;		//中间帧
			m_TxAPdu.fFinal = false;
		}

		*pbApdu++ = m_AppComm.wBlkNo>>8;
		*pbApdu++ = m_AppComm.wBlkNo;
		m_AppComm.wBlkNo++;
		*pbApdu++ = 0x01;	//SEQUENCE OF A-ResultNormal，
		*pbApdu++ = bGetNum;
		memcpy(pbApdu, pbApdu1, pbApdu2-pbApdu1);
		pbApdu += (pbApdu2-pbApdu1);
	}

	*pbApdu++ = GetRptFlg();
	*pbApdu++ = GetTimeFlg();
	m_TxAPdu.wLen = pbApdu - pbApdu0;

	return ToSecurityLayer(); //ToLnkLayer();
}


#define DEBUG_TEST_RECORD_SEGMENT	//台体测试回帧字节数为4096时异常，先用宏控制
int CFaProto::Get_request_record()
{
	TApduInfo tApduInfo;
	WORD wRetNum=0;
	WORD wRxMaxLen;
	BYTE *pbRxPtr;
	WORD wBufSize;
#ifdef DEBUG_TEST_RECORD_SEGMENT
	BYTE bBuf[1800+256];
	BYTE *pApdu = bBuf;
	wBufSize = sizeof(bBuf);
#else
	BYTE *pApdu = m_TxAPdu.bBuf;
	wBufSize = sizeof(m_TxAPdu.bBuf);
#endif
	BYTE *pbAskStart;

	pbAskStart = m_AppComm.pbAskStart;

	//提取APDU信息
	memset((BYTE*)&tApduInfo, 0, sizeof(tApduInfo));
	pbAskStart += ProRecordToApduInfo(pbAskStart, &tApduInfo);

	pbRxPtr = pApdu + (tApduInfo.wRCSDLen + tApduInfo.wOADLen + 256);
	wRxMaxLen = wBufSize - (tApduInfo.wRCSDLen+tApduInfo.wOADLen + 256);
	*pApdu++ = GET_RES;	//Get-response-normal

	//-----------------------------------------------------
	TOobMtrInfo tMtrInfo;
	BYTE bRsdBuf[1024] = {0};
	BYTE bMtrMask[PN_MASK_SIZE] = {0};
	BYTE *pRSD, *pbMsd;
	BYTE bMethod;
	WORD wPn;
	int nRet = -1;
	memcpy(bRsdBuf, tApduInfo.pbRSD, tApduInfo.wRSDLen);
	pRSD = bRsdBuf;
	bMethod = *pRSD;
	if (bMethod == 10)
	{
		BYTE *pbRsd10 = pbRxPtr;
		WORD wRsd10Len = wRxMaxLen - (pbRxPtr-pbRsd10);;
		WORD wRsd10RetNum = 0;
		int iRet = 0;
		int iBakPn = -1;

GOTO_RSD10:	//方法10比较特殊
		if (m_AppComm.iStep == -1)
			m_iRsd10Pn = 0;
		if (m_iRsd10Pn != 0)
			m_AppComm.iStep = -1;
		pbMsd = pRSD + 2;	//跳过“上n条记录  unsigned”
		ParserMsParam(pbMsd, bMtrMask, sizeof(bMtrMask));
		m_iRsd10Pn = SearchNextPnFromMask(bMtrMask, m_iRsd10Pn);
		if (m_iRsd10Pn > 0)
		{
			GetMeterInfo(m_iRsd10Pn, &tMtrInfo);
			*pbMsd++ = 0x04;	//MS“一组配置序号”
			*pbMsd++ = 0x01;	//1个表序号
			pbMsd += OoWordToOi(tMtrInfo.wMtrSn, pbMsd);
			nRet = ReadRecord(tApduInfo.pbOAD, bRsdBuf, tApduInfo.pbRCSD, &m_AppComm.iTabIdx, &m_AppComm.iStep, pbRsd10, wRsd10Len, &wRsd10RetNum);
			if (m_AppComm.iStep == -1)
			{
				iBakPn = m_iRsd10Pn;	//先备份下，先用SearchNextPnFromMask会自动切到下一个测量点
				iBakPn = SearchNextPnFromMask(bMtrMask, iBakPn);
				if (iBakPn < 0)
					m_AppComm.iStep = -1;
				else
					m_AppComm.iStep = 0;
			}

			if (nRet > 0)
			{
				iRet += nRet;
				pbRsd10 += nRet;
				wRetNum += wRsd10RetNum;
			}

			wRsd10Len = wRxMaxLen - (pbRsd10-pbRxPtr);
			if (iBakPn>0 && wRsd10Len>250)	//预留250字节空间，可调整
				goto GOTO_RSD10;
		}
		else
		{
			m_AppComm.iStep = -1;
		}
		nRet = iRet;
	}
	else
	{
		nRet = ReadRecord(tApduInfo.pbOAD, tApduInfo.pbRSD, tApduInfo.pbRCSD, &m_AppComm.iTabIdx, &m_AppComm.iStep, pbRxPtr, wRxMaxLen, &wRetNum);
	}

	if (nRet <= 0)
	{
		*pApdu++ = GET_RECORD;
		*pApdu++ = m_AppComm.bPIID;
		memcpy(pApdu, tApduInfo.pbOAD, tApduInfo.wOADLen);	
		pApdu += tApduInfo.wOADLen;
		if (tApduInfo.wRCSDLen == 1)	//表是RCSD里有0个CSD,即表示全部
		{
			if (tApduInfo.pbRCSD[0] != 0) //表示已填充了RCSD中的所有CSD
			{
				int iRcsdLen;
				//RCSD
				iRcsdLen = ScanRCSD(tApduInfo.pbRCSD, false);	
				memcpy(pApdu, tApduInfo.pbRCSD, iRcsdLen);	
				pApdu += iRcsdLen;
				//*pApdu++ = 0x00;	//Data
				//*pApdu++ = GetErrOfGet(nRet);	//对象不存在
				*pApdu++ = 0x01;	//Data
				*pApdu++ = 0x00;	//对象不存在
			}
			else
			{
				DWORD dwOAD;
				*pApdu++ = 0x01;	//1个RCSD
				*pApdu++ = 0x00;	//CSD里选择OAD
				dwOAD = OoOadToDWord(tApduInfo.pbOAD);
				dwOAD = dwOAD + 0x00010000;	//如0x60140200 + 0x00010000 = 0x60150200
				pApdu += OoDWordToOad(dwOAD, pApdu);
				//*pApdu++ = 0x00;	//DAR
				//*pApdu++ = GetErrOfGet(nRet);	//对象不存在
				*pApdu++ = 0x01;	//Data
				*pApdu++ = 0x00;	//对象不存在
			}
		}
		else
		{
			memcpy(pApdu, tApduInfo.pbRCSD, tApduInfo.wRCSDLen);	
			pApdu += tApduInfo.wRCSDLen;
			//*pApdu++ = 0x00;				//DAR
			//*pApdu++ = GetErrOfGet(nRet);	//对象不存在
			*pApdu++ = 0x01;	//Data
			*pApdu++ = 0x00;	//对象不存在
		}

	}
	else if (m_AppComm.fNewServer)	//开始读取
	{
		m_AppComm.fNewServer = false;
		if (m_AppComm.iStep < 0)	//一帧就读取完成了
		{
			*pApdu++ = GET_RECORD;
			*pApdu++ = m_AppComm.bPIID;
		}
		else
		{
			*pApdu++ = GET_NEXT;
			*pApdu++ = m_AppComm.bPIID;
			*pApdu++ = 0x00;	//末帧标志
			*pApdu++ = (BYTE)m_AppComm.wBlkNo>>8;	//分帧序号
			*pApdu++ = (BYTE)m_AppComm.wBlkNo;
			m_AppComm.wBlkNo++;
			//Choice 分帧响应
			*pApdu++ = 0x02;		//本函数只是针对读取数据库数据，Get-record-data 这个回答是0x02，在另外的接口函数中
			*pApdu++ = 0x01;		//seq num,此处为get-request，如果是get-request-list，此处不为1
			m_TxAPdu.fFinal = false;
		}
		memcpy(pApdu, tApduInfo.pbOAD, tApduInfo.wOADLen);	
		pApdu += tApduInfo.wOADLen;

		if (tApduInfo.wRCSDLen == 1)	//表是RCSD里有0个CSD,即表示全部
		{
			if (tApduInfo.pbRCSD[0] != 0) //表示已填充了RCSD中的所有CSD
			{
				int iRcsdLen;
				//RCSD
				iRcsdLen = ScanRCSD(tApduInfo.pbRCSD, false);	
				memcpy(pApdu, tApduInfo.pbRCSD, iRcsdLen);	
				pApdu += iRcsdLen;
				*pApdu++ = 0x01;	//Data
			}
			else
			{
				DWORD dwOAD;
				*pApdu++ = 0x01;	//1个RCSD
				*pApdu++ = 0x00;	//CSD里选择OAD
				dwOAD = OoOadToDWord(tApduInfo.pbOAD);
				dwOAD = dwOAD + 0x00010000;	//如0x60140200 + 0x00010000 = 0x60150200
				pApdu += OoDWordToOad(dwOAD, pApdu);
				*pApdu++ = 0x01;	//DAR
			}
		}
		else
		{
			memcpy(pApdu, tApduInfo.pbRCSD, tApduInfo.wRCSDLen);	
			pApdu += tApduInfo.wRCSDLen;
			*pApdu++ = 0x01;	//Data
		}

		if (wRetNum != 0)
			*pApdu++ = (BYTE)wRetNum;
		memcpy(pApdu, pbRxPtr, nRet);
		pApdu += nRet;
	}
	else
	{
		*pApdu++ = GET_NEXT;			//get-response-next
		*pApdu++ = m_AppComm.bPIID;
		if (m_AppComm.iStep < 0)		//读取完成了
		{
			*pApdu++ = 0x01;			//末帧标志
			m_TxAPdu.fFinal = true;
		}
		else
		{
			*pApdu++ = 0x00;			//末帧标志
			m_TxAPdu.fFinal = false;
		}

		*pApdu++ = (BYTE )m_AppComm.wBlkNo>>8;	//分帧序号
		*pApdu++ = (BYTE )m_AppComm.wBlkNo;
		m_AppComm.wBlkNo++;
		//Choice 分帧响应
		*pApdu++ = 0x02;		//本函数只是针对读取数据库数据，Get-record-data 这个回答是0x02，在另外的接口函数中
		*pApdu++ = 0x01;		//seq num,此处为get-request，如果是get-request-list，此处不为1
		memcpy(pApdu, tApduInfo.pbOAD, tApduInfo.wOADLen);	
		pApdu += tApduInfo.wOADLen;

		if (tApduInfo.wRCSDLen == 1)	//表是RCSD里有0个CSD,即表示全部
		{
			if (tApduInfo.pbRCSD[0] != 0) //表示已填充了RCSD中的所有CSD
			{
				int iRcsdLen;
				//RCSD
				iRcsdLen = ScanRCSD(tApduInfo.pbRCSD, false);	
				memcpy(pApdu, tApduInfo.pbRCSD, iRcsdLen);	
				pApdu += iRcsdLen;
				*pApdu++ = 0x01;	//Data
			}
			else
			{
				DWORD dwOAD;
				*pApdu++ = 0x01;	//1个RCSD
				*pApdu++ = 0x00;	//CSD里选择OAD
				dwOAD = OoOadToDWord(tApduInfo.pbOAD);
				dwOAD = dwOAD + 0x00010000;	//如0x60140200 + 0x00010000 = 0x60150200
				pApdu += OoDWordToOad(dwOAD, pApdu);
				*pApdu++ = 0x01;	//DAR
			}
		}
		else
		{
			memcpy(pApdu, tApduInfo.pbRCSD, tApduInfo.wRCSDLen);	
			pApdu += tApduInfo.wRCSDLen;
			*pApdu++ = 0x01;	//Data
		}

		if (wRetNum != 0)
			*pApdu++ = (BYTE)wRetNum;
		memcpy(pApdu, pbRxPtr, nRet);	//如果是电表档案，在数据开始部分要有01 **，(**代表电表档案的个数)
		pApdu += nRet;
	}
	*pApdu++ = GetRptFlg();
	*pApdu++ = GetTimeFlg();

#ifdef DEBUG_TEST_RECORD_SEGMENT
	m_TxAPdu.wLen = pApdu - bBuf;
	memcpy(m_TxAPdu.bBuf, bBuf, m_TxAPdu.wLen);
#else
	m_TxAPdu.wLen = pApdu - m_TxAPdu.bBuf;
#endif

	return ToSecurityLayer(); //ToLnkLayer();
}

int CFaProto::Get_request_record_list()
{
	static const int iApdu1Offset = 128;
	static const int iApdu2Offset = 128;
	TApduInfo tApduInfo;
	int iBakStep;
	WORD wRetNum;
	WORD wRxMaxLen;
	WORD wReqApduLen, wRspApduLen;
	BYTE bGetNum=0;
	BYTE *pbAskStart;
	BYTE *pApdu0 = m_TxAPdu.bBuf;
	BYTE *pApdu1;
	BYTE *pApdu2;
	BYTE *pbRxPtr;

	pApdu1 = pApdu0;
	pApdu2 = pApdu1 + iApdu1Offset;	//+16:
	while(m_AppComm.pbAskStart < m_AppComm.pbAskEnd)
	{
		if (m_AppComm.fNewServer)
		{
			m_AppComm.fNewServer = false;
			m_AppComm.iStep = -1;
			m_AppComm.pbAskMid = m_AppComm.pbAskStart;
		}
		else if( m_AppComm.pbAskStart != m_AppComm.pbAskMid)//请求的对象变化，抄读起始序号要置位
		{
			m_AppComm.iStep = -1;
			m_AppComm.pbAskMid = m_AppComm.pbAskStart;
		}

		//提取APDU信息
		pbAskStart = m_AppComm.pbAskStart;
		memset((BYTE*)&tApduInfo, 0, sizeof(tApduInfo));
		pbAskStart += ProRecordToApduInfo(pbAskStart, &tApduInfo);

		//从任务库中提取RCSD数据
		wReqApduLen = tApduInfo.wOADLen + tApduInfo.wRSDLen + tApduInfo.wRCSDLen;	//APDU的请求信息长度（OAD\RSD\RCSD）
		wRspApduLen = tApduInfo.wOADLen + tApduInfo.wRCSDLen;	//APDU的响应信息长度（OAD\RCSD）
		pbRxPtr = pApdu2+iApdu2Offset;
		wRxMaxLen = sizeof(m_TxAPdu.bBuf) - (pApdu2-pApdu0) - wRspApduLen;	//10
		iBakStep = m_AppComm.iStep;
		wRetNum = 0;
		int nRet = ReadRecord(tApduInfo.pbOAD, tApduInfo.pbRSD, tApduInfo.pbRCSD, &m_AppComm.iTabIdx, &m_AppComm.iStep, pbRxPtr, wRxMaxLen, &wRetNum);
		if (nRet <= 0)
		{
			if (int(sizeof(m_TxAPdu.bBuf) - (pApdu2-pApdu0) - wRspApduLen) < 16)	//数据是否溢出
			{
				if (m_AppComm.iStep >= 0)
					m_AppComm.iStep = iBakStep;	//还原上一次的状态
				break;
			}
			else
			{
				memcpy(pApdu2, tApduInfo.pbOAD, tApduInfo.wOADLen);	
				pApdu2 += tApduInfo.wOADLen;

				if (tApduInfo.wRCSDLen == 1)	//表是RCSD里有0个CSD,即表示全部
				{
					if (tApduInfo.pbRCSD[0] != 0) //表示已填充了RCSD中的所有CSD
					{
						int iRcsdLen;
						//RCSD
						iRcsdLen = ScanRCSD(tApduInfo.pbRCSD, false);	
						memcpy(pApdu2, tApduInfo.pbRCSD, iRcsdLen);	
						pApdu2 += iRcsdLen;
						//*pApdu++ = 0x00;	//Data
						//*pApdu++ = GetErrOfGet(nRet);	//对象不存在
						*pApdu2++ = 0x01;	//Data
						*pApdu2++ = 0x00;	//对象不存在
					}
					else
					{
						DWORD dwOAD;
						*pApdu2++ = 0x01;	//1个RCSD
						*pApdu2++ = 0x00;	//CSD里选择OAD
						dwOAD = OoOadToDWord(tApduInfo.pbOAD);
						dwOAD = dwOAD + 0x00010000;	//如0x60140200 + 0x00010000 = 0x60150200
						pApdu2 += OoDWordToOad(dwOAD, pApdu2);
						//*pApdu++ = 0x00;	//DAR
						//*pApdu++ = GetErrOfGet(nRet);	//对象不存在
						*pApdu2++ = 0x01;	//Data
						*pApdu2++ = 0x00;	//对象不存在
					}
				}
				else
				{
					memcpy(pApdu2, tApduInfo.pbRCSD, tApduInfo.wRCSDLen);	
					pApdu2 += tApduInfo.wRCSDLen;
					//*pApdu++ = 0x00;				//DAR
					//*pApdu++ = GetErrOfGet(nRet);	//对象不存在
					*pApdu2++ = 0x01;	//Data
					*pApdu2++ = 0x00;	//对象不存在
				}
				bGetNum++;
			}
		}
		else 
		{
			if (int(sizeof(m_TxAPdu.bBuf) - (pApdu2-pApdu0) - wRspApduLen) < nRet) //数据长度超出了
			{
				if (m_AppComm.iStep >= 0)
					m_AppComm.iStep = iBakStep;
				break;
			}
			else
			{
				memcpy(pApdu2, tApduInfo.pbOAD, tApduInfo.wOADLen);	
				pApdu2 += tApduInfo.wOADLen;
				
				if (tApduInfo.wRCSDLen == 1)	//表是RCSD里有0个CSD,即表示全部
				{
					if (tApduInfo.pbRCSD[0] != 0) //表示已填充了RCSD中的所有CSD
					{
						int iRcsdLen;
						//RCSD
						iRcsdLen = ScanRCSD(tApduInfo.pbRCSD, false);	
						memcpy(pApdu2, tApduInfo.pbRCSD, iRcsdLen);	
						pApdu2 += iRcsdLen;
						//*pApdu++ = 0x00;	//Data
						//*pApdu++ = GetErrOfGet(nRet);	//对象不存在
						*pApdu2++ = 0x01;	//Data
						*pApdu2++ = 0x00;	//对象不存在
					}
					else
					{
						DWORD dwOAD;
						*pApdu2++ = 0x01;	//1个RCSD
						*pApdu2++ = 0x00;	//CSD里选择OAD
						dwOAD = OoOadToDWord(tApduInfo.pbOAD);
						dwOAD = dwOAD + 0x00010000;	//如0x60140200 + 0x00010000 = 0x60150200
						pApdu2 += OoDWordToOad(dwOAD, pApdu2);
					}
				}
				else
				{
					memcpy(pApdu2, tApduInfo.pbRCSD, tApduInfo.wRCSDLen);	
					pApdu2 += tApduInfo.wRCSDLen;
				}

				*pApdu2++ = 0x01;	//Data
				if (wRetNum != 0)
					*pApdu2++ = (BYTE)wRetNum;
				memcpy(pApdu2, pbRxPtr, nRet);
				pApdu2 += nRet;
				bGetNum++;
			}
		}

		if (m_AppComm.iStep == -1)
			m_AppComm.pbAskStart += wReqApduLen;
	}

	if (m_AppComm.wBlkNo==1 && m_AppComm.pbAskStart==m_AppComm.pbAskEnd)	//一帧就直接抄读完成了
	{
		*pApdu1++ = GET_RESPONSE;
		*pApdu1++ = GET_RECORD_LIST;
		*pApdu1++ = m_AppComm.bPIID;
		*pApdu1++ = bGetNum;
		memcpy(pApdu1, pApdu0+iApdu1Offset, pApdu2-(pApdu0+iApdu1Offset));
		pApdu1 += (pApdu2-(pApdu0+iApdu1Offset));
	}
	else	//分帧传输
	{
		*pApdu1++ = GET_RESPONSE;
		*pApdu1++ = GET_NEXT;
		*pApdu1++ = m_AppComm.bPIID;
		if ( m_AppComm.pbAskStart==m_AppComm.pbAskEnd)	//抄读完成，最后一帧
		{
			*pApdu1++ = 0x01;		//最后一帧
			m_TxAPdu.fFinal = true;
		}
		else
		{
			*pApdu1++ = 0x00;		//中间帧
			m_TxAPdu.fFinal = false;
		}

		*pApdu1++ = m_AppComm.wBlkNo>>8;
		*pApdu1++ = m_AppComm.wBlkNo;
		m_AppComm.wBlkNo++;
		*pApdu1++ = 0x02;	//SEQUENCE OF A-ResultRecord
		*pApdu1++ = bGetNum;
		memcpy(pApdu1, pApdu0+iApdu1Offset, pApdu2-(pApdu0+iApdu1Offset));
		pApdu1 += (pApdu2-(pApdu0+iApdu1Offset));
	}

	*pApdu1++ = GetRptFlg();
	*pApdu1++ = GetTimeFlg();

	m_TxAPdu.wLen = pApdu1 - pApdu0;

	return ToSecurityLayer(); //ToLnkLayer();
}

//描述：应用层请求下一帧
//      其中涉及get-request-normal,list,record.record-list不同类型的请求下一帧
int CFaProto::Get_request_next()
{
	int nRet = -1;

	if (m_AppComm.bServerMod == GET_NORMAL)
		nRet = Get_request_normal();
	else if (m_AppComm.bServerMod == GET_NORMAL_LIST)
		nRet = Get_request_normal_list();
	else if (m_AppComm.bServerMod == GET_RECORD)
		nRet = Get_request_record();
	else if (m_AppComm.bServerMod == GET_RECORD_LIST)
		nRet = Get_request_record_list();
	
	return nRet;
}


//描述：设置终端参数，暂时假定每次设置的参数都不涉及到应用分帧的情况
int CFaProto::Set_response(BYTE* pApdu, WORD wApduLen)
{
	WORD wBlkNo,wIdx,wOI;
	BYTE* pApdu0 = pApdu;
	BYTE bGetCmd = *pApdu++;
	m_AppComm.bAnsCmdMod = *pApdu++;
	m_AppComm.bPIID = *pApdu++;
	BYTE bAttr, bIdx;
	int nRet;
	const ToaMap* pOI;
	DWORD dwOAD;
	WORD wDataLen;

	m_AppComm.bServer = SET_REQ;
	//NewAppServer();//启动1个新的服务

	switch(m_AppComm.bAnsCmdMod)
	{
		case SET_NORMAL:
			m_AppComm.bServerMod = SET_NORMAL;
			m_AppComm.pbAskStart = pApdu;
			m_AppComm.pbAskEnd = pApdu + wApduLen;
			return Set_Request_Normal();

		case SET_NORMAL_LIST:
			m_AppComm.bServerMod = SET_NORMAL_LIST;
			m_AppComm.bAskItemNum = *pApdu++;
			m_AppComm.pbAskStart = pApdu;
			m_AppComm.pbAskEnd = pApdu + wApduLen;
			return Set_Request_Normal_List();
			
		case SET_GET_NORMAL_LIST:
			m_AppComm.bServerMod = SET_GET_NORMAL_LIST;
			return Set_Then_Get_Request_Normal_List(pApdu, wApduLen);
	}

}

//描述：设置终端参数
int CFaProto::Set_Request_Normal()
{
	TApduInfo tApduInfo;
	int ret;
	BYTE *pApdu = m_TxAPdu.bBuf;
	BYTE *pbAskStart = m_AppComm.pbAskStart;
	BYTE *pbAskEnd = m_AppComm.pbAskEnd;
	WORD wLen;
	BYTE *pRxPtr;

	memset((BYTE*)&tApduInfo, 0, sizeof(tApduInfo));
	tApduInfo.wOI = OoOiToWord(pbAskStart);
	tApduInfo.pbOAD = pbAskStart;
	pbAskStart += 4;
	tApduInfo.pbSetAttr = pbAskStart;
	tApduInfo.wSetAttrLen = pbAskEnd - pbAskStart;

	*pApdu++ = SET_RES;
	*pApdu++ = SET_NORMAL;
	*pApdu++ = m_AppComm.bPIID;
	memcpy(pApdu, tApduInfo.pbOAD, 4);
	pApdu += 4;

	pRxPtr = pApdu+16;
	wLen = m_AppComm.pbAskEnd - pbAskStart-4;//去掉OAD
	if(tApduInfo.wOI==0xFFF0)
	{//科陆内部扩展帧，需特殊处理，lzx20170222
		ret = ZJUserDef(pbAskStart, wLen, pRxPtr);
	}
	else
	{
		ret = OoProWriteAttr(OoOiToWord(tApduInfo.pbOAD), tApduInfo.pbOAD[2], tApduInfo.pbOAD[3], tApduInfo.pbSetAttr, tApduInfo.wSetAttrLen, m_SecurityParam.fSecurityLayer);
	}

	if( ret <= 0 )
		*pApdu++ = GetErrOfSet(ret);
	else
	{	
		GetTermPrgOAD(OoOiToWord(tApduInfo.pbOAD), tApduInfo.pbOAD[2], tApduInfo.pbOAD[3]);
		DealSpecTrigerEvt(TERM_TERMPRG);
		//SetInfo(INFO_TERM_PROG);
		
		if (OoOadToDWord(tApduInfo.pbOAD) == 0x42040300)
		{
			SetInfo(INFO_ONE_BRAODCAST_ARG_485);
			SetInfo(INFO_ONE_BRAODCAST_ARG_CCT);
		}
		if (OoOadToDWord(tApduInfo.pbOAD) == 0x42040200)
		{
			SetInfo(INFO_MTR_BRAODCAST_ARG_CCT);
			SetInfo(INFO_MTR_BRAODCAST_ARG_485);
		}
		
		if(tApduInfo.wOI==0xFFF0)
		{//科陆内部扩展帧，需特殊处理，lzx20170222
			*pApdu++ = DR_Other;
			memcpy(pApdu, pRxPtr, ret);
			pApdu += ret;
		}
		else
		{
			*pApdu++ = DR_ERROK;
		}
	}

	*pApdu++ = 0x00;	//FollowReport
	*pApdu++ = 0x00;	//FollowReport

	m_TxAPdu.wLen = pApdu - m_TxAPdu.bBuf;

	return ToSecurityLayer(); //ToLnkLayer();
}

//描述：设置多个对象属性
int CFaProto::Set_Request_Normal_List()
{
	int iRet, iDataLen;
	WORD wOI, wNum;
	BYTE *pApdu = m_TxAPdu.bBuf;
	BYTE bAttr, bIdx;
	BYTE *pbAskStart;
	BYTE *pbAskEnd;
	bool fSetPrgInfo = false;

	*pApdu++ = SET_RES;
	*pApdu++ = SET_NORMAL_LIST;
	*pApdu++ = m_AppComm.bPIID;
	*pApdu++ = m_AppComm.bAskItemNum;

	wNum = 0;
	pbAskStart = m_AppComm.pbAskStart;
	pbAskEnd = m_AppComm.pbAskEnd;
	while (wNum < m_AppComm.bAskItemNum)
	{
		iDataLen = OoGetDataLen(DT_OAD, pbAskStart);
		if (iDataLen < 0)	//数据长度出错，后面的OAD不再设置，直接退出，防止异常
		{
			DTRACE(DB_FAPROTO, ("Set_Request_Normal_List:: OoGetDataLen() error, OAD=0x%08x.\n"));
			break;
		}
		else
		{
			wNum++;
			wOI = OoOiToWord(pbAskStart);
			pbAskStart += 2;
			bAttr = *pbAskStart++;
			bIdx = *pbAskStart++;
			
			pApdu += OoWordToOi(wOI, pApdu);
			*pApdu++ = bAttr;
			*pApdu++ = bIdx;

			iRet = OoProWriteAttr(wOI, bAttr, bIdx, pbAskStart, iDataLen, m_SecurityParam.fSecurityLayer);
			if (iRet < 0)	//设置失败，后面的OAD不再设置，后面OAD的计算需要本次OAD的长度才能计算，所以这里直接退出，防止异常
			{
				*pApdu++ = GetErrOfSet(iRet);
				break;
			}
			else
			{	
				GetTermPrgOAD(wOI, bAttr, bIdx);
				fSetPrgInfo = true;
				*pApdu++ = DR_ERROK;
			}

			pbAskStart += iRet;
		}
	}

	if (fSetPrgInfo)
	//	SetInfo(INFO_TERM_PROG);
		DealSpecTrigerEvt(TERM_TERMPRG);


	m_TxAPdu.bBuf[3] = wNum;
	*pApdu++ = 0x00;	//FollowReport optional
	*pApdu++ = 0x00;	//NULL

	m_TxAPdu.wLen = pApdu - m_TxAPdu.bBuf;

	return ToSecurityLayer(); //ToLnkLayer();
}

//描述：设置参数之后，按照设定间隔读取参数, 这里暂不考虑分帧的情况
int CFaProto::Set_Then_Get_Request_Normal_List(BYTE *pApdu, WORD wApduLen)
{
	int iRet, iDataLen, piStart = -1;
	WORD wIdx;
	WORD wOI;
	BYTE bAttr, bIndex;
	BYTE bAskItemNum = 0;
	BYTE *pTxApdu = m_TxAPdu.bBuf;
	WORD wLen = 0;
	BYTE bRdDlyTime;
	bool fSetPrgInfo = false;

	*pTxApdu++ = SET_RES;	//Set-response
	*pTxApdu++ = SET_GET_NORMAL_LIST;
	*pTxApdu++ = m_AppComm.bPIID;

	m_AppComm.bAskItemNum = *pApdu++;
	*pTxApdu++ = m_AppComm.bAskItemNum;
	if (m_AppComm.bAskItemNum > MAXATTRI)
	{
		DTRACE(DB_FAPROTO, ("SET_GET_NORMAL_LIST:bAskItemNum:%d overflow.\n", m_AppComm.bAskItemNum));
		return -1;	
	}

	for (wIdx=0; wIdx<m_AppComm.bAskItemNum; wIdx++)
	{
		wOI = (pApdu[0]<<8) + pApdu[1];
		bAttr = pApdu[2];
		bIndex = pApdu[3];

		//设置OAD
		memcpy(pTxApdu, pApdu, 4);	//OAD
		pTxApdu += 4;
		iDataLen = OoGetDataLen(DT_OAD, pApdu);
		pApdu += 4;	//OAD
		if (iDataLen < 0)
		{
			*pTxApdu++ = GetErrOfGet(iDataLen);
			break;	//无法获取长度，防止后续数据异常，直接退出
		}
		else
		{
			iRet = OoProWriteAttr(wOI, bAttr, bIndex, pApdu, iDataLen, m_SecurityParam.fSecurityLayer);
			if (iRet < 0)
			{
				DTRACE(DB_FAPROTO, ("Set_Then_Get_Request_Normal_List(): OoProWriteAttr OAD=0x%08x error.\n", (wOI<<16)|(bAttr<<8)|(bIndex)));
				*pTxApdu++ = GetErrOfGet(iDataLen);
				break;
			}
			else
			{
				GetTermPrgOAD(wOI, bAttr, bIndex);
				*pTxApdu++ = DR_ERROK;
			}
		}
		pApdu += iRet;	//DATA

		//读取OAD
		wOI = (pApdu[0]<<8) + pApdu[1];
		bAttr = pApdu[2];
		bIndex = pApdu[3];
		memcpy(pTxApdu, pApdu, 4);	//OAD
		pTxApdu += 4;
		pApdu += 4;
		bRdDlyTime = *pApdu++;
		if (bRdDlyTime != 0)
		{
			DWORD dwLstClik = GetClick();
			while (GetClick()-dwLstClik < bRdDlyTime)
				Sleep(500);
		}
		iRet = OoProReadAttr(wOI, bAttr, bIndex, pTxApdu+1, APDUSIZE - (pTxApdu - 1 - m_TxAPdu.bBuf), &piStart);
		if (iRet < 0)
		{
			*pTxApdu++ = 0;
			*pTxApdu++ = GetErrOfGet(iRet);
		}
		else
		{
			*pTxApdu++ = 1;
			pTxApdu += iRet;
		}

		if (pTxApdu - m_TxAPdu.bBuf > (APDUSIZE-20))
		{
			DTRACE(DB_FAPROTO, ("SET_GET_NORMAL_LIST:bAskItemNum:%d, Buffer overflow.\n", m_AppComm.bAskItemNum));
			break;
		}
	}

	if (fSetPrgInfo)
	//	SetInfo(INFO_TERM_PROG);
		DealSpecTrigerEvt(TERM_TERMPRG);


	*pTxApdu++ = GetRptFlg();
	*pTxApdu++ = GetTimeFlg();
	m_TxAPdu.wLen = pTxApdu - m_TxAPdu.bBuf;

	return ToSecurityLayer(); //ToLnkLayer();
}

//描述：操作请求终端应答
int CFaProto::Act_response(BYTE* pApdu, WORD wApduLen)
{
	BYTE b;	
	int i,nRet,cnt=0,inx;
	BYTE *pOpt=NULL,*pApdu0=pApdu,*pTail=pApdu+wApduLen;
	TOIMethodDesc *pMethod;
	const TOmMap* pOmMap;
	BYTE *pTmp;
	DWORD dwOMD;

	BYTE bActCmd = *pApdu++;		//ACT
	m_AppComm.bCmdMod = *pApdu++;	//ACT-mode:request,request-list,then-request-normal
	m_AppComm.bPIID = *pApdu++;		//PIID

	switch(m_AppComm.bCmdMod)
	{
		case ACT_NORMAL:
			NewAppServer();//启动1个新的服务
			m_AppComm.pbAskStart = pApdu;
			m_AppComm.pbAskEnd = pApdu + wApduLen;
			return Act_Response_Normal();

		case ACT_NORMAL_LIST:
			NewAppServer();//启动1个新的服务
			m_AppComm.bAskMethodNum = *pApdu++;
			m_AppComm.pbAskStart = pApdu;
			m_AppComm.pbAskEnd = pApdu + wApduLen;
			return Act_Response_List();

		case ACT_GET_NORMAL_LIST:
			NewAppServer();//启动1个新的服务
			m_AppComm.bAskMethodNum = *pApdu++;
			m_AppComm.pbAskStart = pApdu;
			m_AppComm.pbAskEnd = pApdu + wApduLen;
			return Act_Then_Rd_List();

		default:
			break;
	}

	return -1;
}

//描述：Act命令-normal模式的响应
int CFaProto::Act_Response_Normal()
{
	const TOmMap* pOmMap = NULL;
	int iRet, iRetParaLen;
	DWORD dwOMD=0;
	BYTE *pApdu0 = m_TxAPdu.bBuf;
	BYTE *pApdu = pApdu0;
	BYTE *pbRxBuf;
	BYTE *pbAskStart;
	BYTE *pbAskEnd;



	//操作信息设置
	pbAskStart = m_AppComm.pbAskStart;
	pbAskEnd = m_AppComm.pbAskEnd;
	dwOMD = OoOadToDWord(pbAskStart);
	pbAskStart += 4;

	//相应信息提取
	*pApdu++ = ACTION_RES;	//Get-response-normal
	*pApdu++ = ACT_NORMAL;
	*pApdu++ = m_AppComm.bPIID;
	pApdu += OoDWordToOad(dwOMD, pApdu);

	pOmMap = GetOmMap(dwOMD);
	if (pOmMap != NULL)
	{
		pbRxBuf = pApdu0 + 16;
		iRet = DoObjMethod(pOmMap->dwOM>>16, pOmMap->dwOM>>8, pOmMap->dwOM, pbAskStart, &iRetParaLen, NULL, pbRxBuf);
		if (iRet>=0 && m_AppComm.fNewServer)
		{
			*pApdu++ = DR_ERROK;	//DAR--ok
			memcpy(pApdu, pbRxBuf, iRet);
			pApdu += iRet;
		}
		else
		{
			*pApdu++ = DR_RWDenied;
			DTRACE(DB_FAPROTO, ("Act_Response_Normal failed, dwOMD=0x%08x !\r\n", dwOMD));
			
		}
	}
	else
	{
		*pApdu++ = DR_Other;
		DTRACE(DB_FAPROTO, ("Act_Response_Normal failed, dwOMD=0x%08x !\r\n", dwOMD));
	}

	*pApdu++ = 0x00;	//Data optional
	*pApdu++ = 0x00;	//FollowReport
	*pApdu++ = 0x00;	//表示没有时间标签

	m_TxAPdu.wLen = pApdu - pApdu0;

	return ToSecurityLayer(); //ToLnkLayer();
}

//描述：Act命令-list模式的响应
int CFaProto::Act_Response_List()
{
	const TOmMap* pOmMap = NULL;
	int iRet, iRetParaLen;
	DWORD dwOMD;
	WORD wNum, wSetLen;
	BYTE *pApdu = m_TxAPdu.bBuf;
	BYTE *pbAskStart;
	BYTE *pbAskEnd;
	BYTE *pbRxBuf;

	//提取操作返回相应信息
	*pApdu++ = ACTION_RES;
	*pApdu++ = ACT_NORMAL_LIST;
	*pApdu++ = m_AppComm.bPIID;
	*pApdu++ = m_AppComm.bAskMethodNum;

	//提取操作设置的相应信息
	pbAskStart = m_AppComm.pbAskStart;
	pbAskEnd = m_AppComm.pbAskEnd;
	wNum = 0;
	while (wNum < m_AppComm.bAskMethodNum)
	{
		dwOMD = OoOadToDWord(pbAskStart);
		memcpy(pApdu, pbAskStart, 4);
		pApdu += 4;
		pbAskStart += 4;
		pOmMap = GetOmMap(dwOMD);
		if (pOmMap != NULL)
		{
			pbRxBuf = pApdu + 16;
			iRet = DoObjMethod(pOmMap->dwOM>>16, pOmMap->dwOM>>8, pOmMap->dwOM, pbAskStart, &iRetParaLen, NULL, pbRxBuf);
			if (iRet >= 0)
			{
				*pApdu++ = DR_ERROK;	//DAR--ok
				*pApdu++ = 0;
				memcpy(pApdu, pbRxBuf, iRet);
				pApdu += iRet;
			}
			else
			{
				*pApdu++ = GetErrOfGet(iRet);	//对象不存在
				*pApdu++ = 0;
				DTRACE(DB_FAPROTO, ("Act_response-Act-list failed, dwOMD:0x%08x DoObjMethod failed !\r\n", dwOMD));
				//break;
			}
		}
		else
		{
			*pApdu++ = GetErrOfGet(iRet);	//对象不存在
			DTRACE(DB_FAPROTO, ("Act_response-Act-list failed, dwOMD:0x%08x GetOmMap failed !\r\n", dwOMD));
			break;
		}

		wNum++;
		pbAskStart += iRetParaLen;
	}

	m_TxAPdu.bBuf[3] = wNum;
	*pApdu++ = 0x00;	//FollowReport optional
	*pApdu++ = 0x00;	//NULL
	m_TxAPdu.wLen = pApdu - m_TxAPdu.bBuf;

	return ToSecurityLayer(); //ToLnkLayer();
}

//描述：Act命令-list-read模式的响应
int CFaProto::Act_Then_Rd_List()
{
	DWORD dwSetOMD, dwGetOAD;
	BYTE bDlyTime;
	const TOmMap* pOmMap = NULL;
	int iRet, iRetParaLen;
	WORD wNum, wSetLen;
	BYTE *pApdu0 = m_TxAPdu.bBuf;
	BYTE *pApdu = pApdu0;
	BYTE *pbAskStart;
	BYTE *pbAskEnd;
	BYTE *pbRxBuf;
	WORD wMaxRxLen;

	//提取操作返回相应信息
	*pApdu++ = ACTION_RES;
	*pApdu++ = ACT_GET_NORMAL_LIST;
	*pApdu++ = m_AppComm.bPIID;
	*pApdu++ = m_AppComm.bAskMethodNum;

	//提取操作设置的相应信息
	pbAskStart = m_AppComm.pbAskStart;
	pbAskEnd = m_AppComm.pbAskEnd;
	wNum = 0;
	while (wNum < m_AppComm.bAskItemNum)
	{
		dwSetOMD = OoOadToDWord(pbAskStart);
		pbAskStart += 4;
		pOmMap = GetOmMap(dwSetOMD);
		if (pOmMap != NULL)
		{
			pbRxBuf = pApdu + 16;
			iRet = DoObjMethod(pOmMap->dwOM>>16, pOmMap->dwOM>>8, pOmMap->dwOM, pbAskStart, &iRetParaLen, NULL, pbRxBuf);
			if (iRet > 0)
			{
				pApdu += OoDWordToOad(dwSetOMD, pApdu);
				*pApdu++ = DR_ERROK;	//DAR--ok
				*pApdu++ = 0x01;	//data
				memcpy(pApdu, pbRxBuf, iRet);
				pApdu += iRet;
			}
			else
			{
				pApdu += OoDWordToOad(dwSetOMD, pApdu);
				*pApdu++ = GetErrOfGet(iRet);	//对象不存在
				DTRACE(DB_FAPROTO, ("Act_Then_Rd_List failed, dwOMD:0x%08x DoObjMethod failed !\r\n", dwSetOMD));
				break;
			}
		}
		else
		{
			pApdu += OoDWordToOad(dwSetOMD, pApdu);
			*pApdu++ = GetErrOfGet(iRet);	//对象不存在
			DTRACE(DB_FAPROTO, ("Act_Then_Rd_List failed, dwOMD:0x%08x GetOmMap failed !\r\n", dwSetOMD));
			break;
		}
		
		pbAskStart += iRetParaLen;
		dwGetOAD = OoOadToDWord(pbAskStart);
		pbAskStart += 4;

		bDlyTime = *pbAskStart++;
		Sleep(bDlyTime*1000);
	
		pbRxBuf = pApdu + 16;
		wMaxRxLen = sizeof(m_TxAPdu.bBuf) - (pbRxBuf-pApdu0);

		iRet = OoProReadAttr(dwGetOAD>>16, dwGetOAD>>8, dwGetOAD, pbRxBuf, wMaxRxLen, &m_AppComm.iStep);
		if (iRet <= 0)
		{
			pApdu += OoDWordToOad(dwGetOAD, pApdu);
			*pApdu++ = 0x00;				//DAR
			*pApdu++ = GetErrOfGet(iRet);	//对象不存在
			break;
		}
		else
		{
			pApdu += OoDWordToOad(dwGetOAD, pApdu);
			*pApdu++ = 0x01;	//Data
			memcpy(pApdu, pbRxBuf, iRet);
			pApdu += iRet;
		}
		wNum++;
	}

	m_TxAPdu.bBuf[3] = wNum;
	*pApdu++ = 0x00;	//FollowReport optional
	*pApdu++ = 0x00;	//NULL

	m_TxAPdu.wLen = pApdu - pApdu0;

	return ToSecurityLayer(); //ToLnkLayer();
}

int CFaProto::GetErrOfGet(int iRetVal)
{
	switch( iRetVal )
	{
	case -1: return DR_RWDenied;
	case -2: return DR_ObjUndefined;
	case -3: return DR_ObjIFInValid;
	case -4: return DR_Other;
	}
	return DR_Other;
}

int CFaProto::GetErrOfSet(int iRetVal)
{
	switch( iRetVal )
	{
	case -1: return DR_RWDenied;
	case -2: return DR_ObjUndefined;
	case -3: return DR_ObjIFInValid;
	case -4: return DR_Other;
	}
	return DR_Other;
}

int CFaProto::GetErrOfAct(int iRetVal)
{
	switch( iRetVal )
	{
	case -1: return AR_RWDenied;			//读写操作拒绝
	case -2: return AR_ObjUndefined;		//对象未定义
	case -3: return AR_ObjClsInconsistent;	//类和对象不一致
	case -4: return AR_Other;				//其它错误
	case -5: return 0xff;//pan 确认ACT协议OK，但获取数据返回错误，2008-1-15
	case -6: return AR_ScopeViolated;		//操作无效，2009-9-2
	}
	return AR_Other;
}

//描述：代理请求
int CFaProto::ProxyResponse()
{
	BYTE bRequestType = m_RxDPool.bBuf[1];
	m_AppComm.bPIID = m_RxDPool.bBuf[2];

	switch(bRequestType)
	{
	case PROXY_GET_REQ_LIST: //代理读取若干个服务器的若干个对象属性请求    
		return ProxyGetRequestList();

	case PROXY_GET_REQ_RECORD://代理读取一个服务器的一个记录型对象属性请求
		return ProxyGetRequestRecord();

	case PROXY_SET_REQ_LIST: //代理设置若干个服务器的若干个对象属性请求
		return ProxySetRequestList();

	case PROXY_SET_THEN_GET_REQ_LIST://代理设置后读取若干个服务器的若干个对象属性请求
		return ProxySetThenGetRequestList();

	case PROXY_ACT_REQ_LIST: //代理操作若干个服务器的若干个对象方法请求   
		return ProxyActionRequestList();

	case PROXY_ACT_THEN_GET_REQ_LIST://代理操作后读取若干个服务器的若干个对象方法和属性请求		
		return ProxyActionThenGetRequestList();

	case PROXY_TRANS_CMD_REQ://代理透明转发命令请求
		return ProxyTransCommandRequest();

	default:
		break;
	}

	return -1;
}

int CFaProto::ToSecurityLayer(void)
{
	TAPdu *pApdu = &m_TxAPdu;
	BYTE *pbMakeSecuFrm;
	BYTE* pApdu0;
	BYTE bBuf[APDUSIZE];
	WORD wDataLen;
	int iLengthLen;
	int iRet;
	
	if (m_SecurityParam.fSecurityLayer == false)
	{
		return ToLnkLayer();
	}

	if (m_LnkComm.bAddrType == ADDR_TYPE_BROADCAST)	//广播地址无需回确认帧
		return 1;

	//原APDU先放到buf里面
	wDataLen = pApdu->wLen;
	memcpy(bBuf, pApdu->bBuf, wDataLen);
	//pbMakeSecuFrm指针指向pApdu->bBuf
	pbMakeSecuFrm = pApdu->bBuf;
	pApdu0 = pApdu->bBuf;
	*pbMakeSecuFrm++ = SECURITY_RES; //安全响应   		[144]	SECURITY-Response
	
	if (m_SecurityParam.bAppDataUnit == SecureData_Plaintext) //明文数据单元
	{
		BYTE bEncodeBuf[APDUSIZE];
		switch (m_SecurityParam.bDataAuthInfo)
		{
		case AuthType_SIDMAC: //数据验证码      [0]  SID_MAC，
			if (*bBuf == SET_RES) //如果是安全模式设置
			{
				iRet = Esam_ResMakeEndataMac(bBuf, wDataLen, bEncodeBuf, sizeof(bEncodeBuf));
				if (iRet > 0)
				{
					int iDataUnitLen = iRet - ESAM_MAC_LEN; //ESAM返回数据包括密文+MAC
					*pbMakeSecuFrm++ = SecureData_Ciphertext; // 密文应用数据单元
					iLengthLen = EncodeLength(iDataUnitLen, pbMakeSecuFrm);
					pbMakeSecuFrm += iLengthLen;
					memcpy(pbMakeSecuFrm, bEncodeBuf, iDataUnitLen);
					pbMakeSecuFrm += iDataUnitLen;
					
					*pbMakeSecuFrm++ = 1; //  OPTIONAL 有数据验证信息
					*pbMakeSecuFrm++ = 0; // 00 数据MAC         [0]  MAC
					*pbMakeSecuFrm++ = ESAM_MAC_LEN; // MAC的长度
					memcpy(pbMakeSecuFrm, bEncodeBuf+iDataUnitLen, ESAM_MAC_LEN);
					pbMakeSecuFrm += ESAM_MAC_LEN;
					
					pApdu->wLen = pbMakeSecuFrm - pApdu0;
				}
				else
				{
					*pbMakeSecuFrm++ = SecureData_ErrorDAR;
					*pbMakeSecuFrm++ = DAR_ESAM_CERT_FAIL;
					*pbMakeSecuFrm++ = 0; // OPTIONAL 无数据验证信息
					pApdu->wLen = pbMakeSecuFrm - pApdu0;
				}
			}
			else
			{
				iRet = Esam_ResMakePlndataMac(bBuf, wDataLen, bEncodeBuf, sizeof(bEncodeBuf));
				if (iRet > 0)
				{
					int iDataUnitLen = iRet - ESAM_MAC_LEN; //ESAM返回数据包括明文+MAC
					*pbMakeSecuFrm++ = SecureData_Plaintext; // 明文应用数据单元
					iLengthLen = EncodeLength(iDataUnitLen, pbMakeSecuFrm);
					pbMakeSecuFrm += iLengthLen;
					memcpy(pbMakeSecuFrm, bEncodeBuf, iDataUnitLen);
					pbMakeSecuFrm += iDataUnitLen;
					
					*pbMakeSecuFrm++ = 1; //  OPTIONAL 有数据验证信息
					*pbMakeSecuFrm++ = 0; // 00 数据MAC         [0]  MAC
					*pbMakeSecuFrm++ = ESAM_MAC_LEN; // MAC的长度
					memcpy(pbMakeSecuFrm, bEncodeBuf+iDataUnitLen, ESAM_MAC_LEN);
					pbMakeSecuFrm += ESAM_MAC_LEN;
					
					pApdu->wLen = pbMakeSecuFrm - pApdu0;
				}
				else
				{
					*pbMakeSecuFrm++ = SecureData_ErrorDAR;
					*pbMakeSecuFrm++ = DAR_ESAM_CERT_FAIL;
					*pbMakeSecuFrm++ = 0; // OPTIONAL 无数据验证信息
					pApdu->wLen = pbMakeSecuFrm - pApdu0;
				}

				/*
				*pbMakeSecuFrm++ = SecureData_Plaintext; // 明文应用数据单元   [0]  octet-string，
				iLengthLen = EncodeLength(wDataLen, pbMakeSecuFrm);
				pbMakeSecuFrm += iLengthLen;
				memcpy(pbMakeSecuFrm, bBuf, wDataLen);
				pbMakeSecuFrm += wDataLen;

				*pbMakeSecuFrm++ = 0; //  OPTIONAL 无数据验证信息
				
				pApdu->wLen = pbMakeSecuFrm - pApdu0;
				*/
			}

			
			break;
		case AuthType_RN: //随机数          [1]  RN，
			if (Esam_PlnDatResCalMac(bBuf, wDataLen, m_SecurityParam.bRn, m_SecurityParam.bMac)>0)
			{
				*pbMakeSecuFrm++ = SecureData_Plaintext; // 明文应用数据单元   [0]  octet-string，
				iLengthLen = EncodeLength(wDataLen, pbMakeSecuFrm);
				pbMakeSecuFrm += iLengthLen;
				memcpy(pbMakeSecuFrm, bBuf, wDataLen);
				pbMakeSecuFrm += wDataLen;
				
				*pbMakeSecuFrm++ = 1; //  OPTIONAL 有数据验证信息
				*pbMakeSecuFrm++ = 0; // 00 数据MAC         [0]  MAC
				*pbMakeSecuFrm++ = ESAM_MAC_LEN; // MAC的长度
				memcpy(pbMakeSecuFrm, m_SecurityParam.bMac, ESAM_MAC_LEN);
				pbMakeSecuFrm += ESAM_MAC_LEN;
				
				pApdu->wLen = pbMakeSecuFrm - pApdu0;
			}
			else
			{
				*pbMakeSecuFrm++ = SecureData_ErrorDAR;
				*pbMakeSecuFrm++ = DAR_ESAM_CERT_FAIL;
				*pbMakeSecuFrm++ = 0; // OPTIONAL 无数据验证信息
				pApdu->wLen = pbMakeSecuFrm - pApdu0;
			}
			
			break;
		case AuthType_RNMAC: //随机数+数据MAC  [2]  RN_MAC，
			break;
		case AuthType_SID: //安全标识        [3]  SID
			break;
		default:
			break;
		}
	}
	else if (m_SecurityParam.bAppDataUnit == SecureData_Ciphertext)//密文数据单元
	{
		BYTE bEncodeBuf[APDUSIZE];
		switch (m_SecurityParam.bDataAuthInfo)
		{
		case AuthType_SIDMAC: //数据验证码	  [0]  SID_MAC，
			iRet = Esam_ResMakeEndataMac(bBuf, wDataLen, bEncodeBuf, sizeof(bEncodeBuf));
			if (iRet > 0)
			{
				int iDataUnitLen = iRet - ESAM_MAC_LEN; //ESAM返回数据包括密文+MAC
				*pbMakeSecuFrm++ = SecureData_Ciphertext; // 密文应用数据单元
				iLengthLen = EncodeLength(iDataUnitLen, pbMakeSecuFrm);
				pbMakeSecuFrm += iLengthLen;
				memcpy(pbMakeSecuFrm, bEncodeBuf, iDataUnitLen);
				pbMakeSecuFrm += iDataUnitLen;
				
				*pbMakeSecuFrm++ = 1; //  OPTIONAL 有数据验证信息
				*pbMakeSecuFrm++ = 0; // 00 数据MAC         [0]  MAC
				*pbMakeSecuFrm++ = ESAM_MAC_LEN; // MAC的长度
				memcpy(pbMakeSecuFrm, bEncodeBuf+iDataUnitLen, ESAM_MAC_LEN);
				pbMakeSecuFrm += ESAM_MAC_LEN;
				
				pApdu->wLen = pbMakeSecuFrm - pApdu0;
			}
			else
			{
				*pbMakeSecuFrm++ = SecureData_ErrorDAR;
				*pbMakeSecuFrm++ = DAR_ESAM_CERT_FAIL;
				*pbMakeSecuFrm++ = 0; // OPTIONAL 无数据验证信息
				pApdu->wLen = pbMakeSecuFrm - pApdu0;
			}
			break;
		case AuthType_RN: //随机数		  [1]  RN，
			break;
		case AuthType_RNMAC: //随机数+数据MAC  [2]  RN_MAC，
			break;
		case AuthType_SID: //安全标识		  [3]  SID
			iRet = Esam_ResMakeEndata(bBuf, wDataLen, bEncodeBuf, sizeof(bEncodeBuf));
			if (iRet > 0)
			{
				*pbMakeSecuFrm++ = SecureData_Ciphertext; // 密文应用数据单元
				iLengthLen = EncodeLength(iRet, pbMakeSecuFrm);
				pbMakeSecuFrm += iLengthLen;
				memcpy(pbMakeSecuFrm, bEncodeBuf, iRet);
				pbMakeSecuFrm += iRet;
				*pbMakeSecuFrm++ = 0; //  OPTIONAL 无数据验证信息

				pApdu->wLen = pbMakeSecuFrm - pApdu0;
			}
			else
			{
				*pbMakeSecuFrm++ = SecureData_ErrorDAR;
				*pbMakeSecuFrm++ = DAR_ESAM_CERT_FAIL;
				*pbMakeSecuFrm++ = 0; // OPTIONAL 无数据验证信息
				pApdu->wLen = pbMakeSecuFrm - pApdu0;
			}
			break;
		default:
			break;
		}
	}
	else
		return -1;

	m_SecurityParam.fSecurityLayer = false;
	return ToLnkLayer();
	
	
}

/*
函数名:SecurityRequest
入参:BYTE* pApdu
出参:WORD *pwAPDULen(解码完成的应用数据单元APDU报文有多长)
返回值:int ，返回的应用数据单元APDU相对原APDU指针的偏移量
*/
int CFaProto::SecurityRequest(BYTE* pApdu, WORD *pwAPDULen)
{
	BYTE *pApdu0 = pApdu;
	BYTE *pbDataUnit;
	DWORD dwDataUnitLen;
	int iLengthLen;
	int iPos = -1;
	BYTE bBuf[APDUSIZE];
	int iRet;
	BYTE bErrInfo;
	BYTE bAttachData[64];
	DWORD dwAttachLen;
	
	BYTE bSercureCmd = *pApdu++; //0x10 SecurityRequest
	
	m_SecurityParam.bAppDataUnit = *pApdu++; //应用数据单元类型
	
	iLengthLen = DecodeLength(pApdu, &dwDataUnitLen); //应用数据单元长度
	pApdu += iLengthLen;
	
	pbDataUnit = pApdu; //应用数据单元
	pApdu += dwDataUnitLen;

	m_SecurityParam.bDataAuthInfo = *pApdu++; //数据验证信息

	if (m_SecurityParam.bAppDataUnit == SecureData_Plaintext) //明文数据单元
	{
		switch (m_SecurityParam.bDataAuthInfo)
		{
		case AuthType_SIDMAC: //数据验证码      [0]  SID_MAC，
#if 0
			//广播
			if ((m_LnkComm.bAddrType==ADDR_TYPE_GROUP) || (m_LnkComm.bAddrType==ADDR_TYPE_BROADCAST))
			{
				DWORD dwTemp;
				BYTE bBroadcastSID[4] = {0x80,0x16,0x48,0x00};
				DTRACE(DB_FAPROTO, ("SecurityRequest: Broadcast !!\r\n"));

				bBroadcastSID[3] += m_LnkComm.bCliAddrs[0]&0x0F; //判断广播地址最后一位有效位与SID最后一位是否相等
				if (bBroadcastSID[3] == 0x00)	
					bBroadcastSID[3] = 0x0A;  //为0时X=A；

				memcpy(bBuf, pbDataUnit, dwDataUnitLen);
				memcpy(m_SecurityParam.bSID, pApdu, 4);
				pApdu += 4;

				if (memcmp(bBroadcastSID, m_SecurityParam.bSID, 4)==0) //收到广播帧，判断安全标识是否正确
				{
					
					iLengthLen = DecodeLength(pApdu, &dwAttachLen);
					if (dwAttachLen > sizeof(bAttachData))
					{
						DTRACE(DB_FAPROTO, ("SecurityRequest: AttachData too long !!\r\n"));
						dwAttachLen = sizeof(bAttachData);
					}
					pApdu += iLengthLen;
					memcpy(bAttachData, pApdu, dwAttachLen);
					pApdu += dwAttachLen;
					iLengthLen = DecodeLength(pApdu, &dwTemp);
					pApdu += iLengthLen;
					if(dwTemp == ESAM_MAC_LEN) //MAC长度为4
					{
						memcpy(m_SecurityParam.bMac, pApdu, ESAM_MAC_LEN);
					}
					iRet = Esam_SIDMACDecode(m_SecurityParam.bSID, bAttachData,
							(WORD)dwAttachLen, m_SecurityParam.bMac, bBuf, dwDataUnitLen,
							pApdu0, MAXDATASIZE);
					if (iRet > 0)
					{
						//广播解码成功，刷新计数器
						Esam_ReflashCounter();
						
						iPos = 0;
						*pwAPDULen = (WORD)iRet;
					}
					else
					{
						bErrInfo = DAR_ESAM_CERT_FAIL;
						DTRACE(DB_FAPROTO, ("SecurityRequest: Esam_SIDMACDecode error !!\r\n"));
						return -1;
					}
				}
				else
				{
					DTRACE(DB_FAPROTO, ("SecurityRequest: Broadcast, SID error(%02x%02x%02x%02x) !!\r\n",
						bBroadcastSID[0],bBroadcastSID[1],bBroadcastSID[2],bBroadcastSID[3]));
					return -1;
				}
			}
#else
			DWORD dwTemp;
			memcpy(bBuf, pbDataUnit, dwDataUnitLen);
			memcpy(m_SecurityParam.bSID, pApdu, 4);
			pApdu += 4;
			iLengthLen = DecodeLength(pApdu, &dwAttachLen);
			if (dwAttachLen > sizeof(bAttachData))
			{
				DTRACE(DB_FAPROTO, ("SecurityRequest: AttachData too long !!\r\n"));
				dwAttachLen = sizeof(bAttachData);
			}
			pApdu += iLengthLen;
			memcpy(bAttachData, pApdu, dwAttachLen);
			pApdu += dwAttachLen;
			iLengthLen = DecodeLength(pApdu, &dwTemp);
			pApdu += iLengthLen;
			if(dwTemp == ESAM_MAC_LEN) //MAC长度为4
			{
				memcpy(m_SecurityParam.bMac, pApdu, ESAM_MAC_LEN);
			}
			iRet = Esam_SIDMACDecode(m_SecurityParam.bSID, bAttachData,
					(WORD)dwAttachLen, m_SecurityParam.bMac, bBuf, dwDataUnitLen,
					pApdu0, MAXDATASIZE);
			if (iRet > 0)
			{
				//广播解码成功，刷新计数器
				Esam_ReflashCounter();
				
				iPos = 0;
				*pwAPDULen = (WORD)iRet;
			}
			else
			{
				bErrInfo = DAR_ESAM_CERT_FAIL;
				DTRACE(DB_FAPROTO, ("SecurityRequest: Esam_SIDMACDecode error !!\r\n"));
				return -1;
			}

#endif
			break;
		case AuthType_RN: //随机数          [1]  RN，
			//明文+MAC读取
			iLengthLen = DecodeLength(pApdu, &m_SecurityParam.dwRnLen);
			pApdu += iLengthLen;
			memcpy(m_SecurityParam.bRn, pApdu, m_SecurityParam.dwRnLen);
			*pwAPDULen = (WORD)dwDataUnitLen;
			iPos = pbDataUnit - pApdu0;
			break;
		case AuthType_RNMAC: //随机数+数据MAC  [2]  RN_MAC，
			//无用
			break;
		case AuthType_SID: //安全标识        [3]  SID
			//无用
			break;
		default:
			break;
		}
	}
	else if (m_SecurityParam.bAppDataUnit == SecureData_Ciphertext)//密文数据单元
	{
		switch (m_SecurityParam.bDataAuthInfo)
		{
		case AuthType_SIDMAC: //数据验证码	  [0]  SID_MAC，
			DWORD dwTemp;
			memcpy(bBuf, pbDataUnit, dwDataUnitLen);
			memcpy(m_SecurityParam.bSID, pApdu, 4);
			pApdu += 4;
			iLengthLen = DecodeLength(pApdu, &dwAttachLen);
			if (dwAttachLen > sizeof(bAttachData))
			{
				DTRACE(DB_FAPROTO, ("SecurityRequest: AttachData too long !!\r\n"));
				dwAttachLen = sizeof(bAttachData);
			}
			pApdu += iLengthLen;
			memcpy(bAttachData, pApdu, dwAttachLen);
			pApdu += dwAttachLen;
			iLengthLen = DecodeLength(pApdu, &dwTemp);
			pApdu += iLengthLen;
			if(dwTemp == ESAM_MAC_LEN) //MAC长度为4
			{
				memcpy(m_SecurityParam.bMac, pApdu, ESAM_MAC_LEN);
			}
			iRet = Esam_SIDMACDecode(m_SecurityParam.bSID, bAttachData,
					(WORD)dwAttachLen, m_SecurityParam.bMac, bBuf, dwDataUnitLen,
					pApdu0, MAXDATASIZE);
			if (iRet > 0)
			{
				iPos = 0;
				*pwAPDULen = (WORD)iRet;
			}
			else
			{
				bErrInfo = DAR_ESAM_CERT_FAIL;
				DTRACE(DB_FAPROTO, ("SecurityRequest: Esam_SIDMACDecode error !!\r\n"));
				return -1;
			}
			break;
		case AuthType_RN: //随机数		  [1]  RN，
			//无用
			break;
		case AuthType_RNMAC: //随机数+数据MAC  [2]  RN_MAC，
			//无用
			break;
		case AuthType_SID: //安全标识		  [3]  SID
			memcpy(bBuf, pbDataUnit, dwDataUnitLen);
			memcpy(m_SecurityParam.bSID, pApdu, 4);
			pApdu += 4;
			iLengthLen = DecodeLength(pApdu, &dwAttachLen);
			if (dwAttachLen > sizeof(bAttachData))
			{
				DTRACE(DB_FAPROTO, ("SecurityRequest: AttachData too long !!\r\n"));
				dwAttachLen = sizeof(bAttachData);
			}
			pApdu += iLengthLen;
			memcpy(bAttachData, pApdu, dwAttachLen);
			iRet = Esam_SIDDecode(m_SecurityParam.bSID, bAttachData,
					(WORD)dwAttachLen, bBuf, dwDataUnitLen, pApdu0, MAXDATASIZE);
			if (iRet >0)
			{
				iPos = 0;
				*pwAPDULen = iRet;
			}
			else
			{
				bErrInfo = DAR_ESAM_CERT_FAIL;
				DTRACE(DB_FAPROTO, ("SecurityRequest: Esam_SIDDecode error !!\r\n"));
				return -1;
			}
			break;
		default:
			break;
		}
	}
	else
		return -1;

	return iPos;

}

int CFaProto::SecurityResponse(BYTE* pApdu, WORD *pwAPDULen)
{
	BYTE *pApdu0 = pApdu;
	BYTE *pbDataUnit;
	DWORD dwDataUnitLen;
	int iLengthLen;
	int iPos = -1;
	int iRet;
	BYTE bErrInfo;
	BYTE bAppDataUnit, bDataAuthInfo;
	BYTE bMac[ESAM_MAC_LEN];
	BYTE bOption;
	DWORD dwTemp;

	BYTE bSercureCmd = *pApdu++; //0x90 SecurityResponse
	
	bAppDataUnit = *pApdu++; //应用数据单元类型
	
	iLengthLen = DecodeLength(pApdu, &dwDataUnitLen); //应用数据单元长度
	pApdu += iLengthLen;
	
	pbDataUnit = pApdu; //应用数据单元
	pApdu += dwDataUnitLen;

	bOption = *pApdu++;

	if (bAppDataUnit == SecureData_Plaintext) //明文数据单元
	{
		if (bOption == 1)
		{
			bDataAuthInfo = *pApdu++; //数据验证信息
			if (bDataAuthInfo == 0) //MAC
			{
				iLengthLen = DecodeLength(pApdu, &dwTemp);
				pApdu += iLengthLen;
				if(dwTemp == ESAM_MAC_LEN) //MAC长度为4
				{
					memcpy(bMac, pApdu, ESAM_MAC_LEN);
					if (Esam_CheckResponseMac(pbDataUnit, dwDataUnitLen, m_RptSecureParam.bRn, bMac)==true)
					{
						*pwAPDULen = (WORD)dwDataUnitLen;
						iPos = pbDataUnit - pApdu0;
					}
				}
			}
			else
			{
				return -1;
			}


		}
		else
		{
			*pwAPDULen = (WORD)dwDataUnitLen;
			iPos = pbDataUnit - pApdu0;
		}
	}
	else
	{
		return -1;
	}

	return iPos;
}

int CFaProto::MakeSecureErrFrm(void)
{
	BYTE* pApdu = m_TxAPdu.bBuf;
	BYTE* pApdu0 = pApdu;
	
	*pApdu++ = SECURITY_RES;
	*pApdu++ = SecureData_ErrorDAR;
	*pApdu++ = DAR_NONE_SMT_DCP_ERR;
	*pApdu++ = 0x00; //数据验证信息  OPTIONAL

	m_TxAPdu.wLen = pApdu - pApdu0;
	return ToLnkLayer();
}



int CFaProto::ProxyGetRequestList()
{
	return ProxyResponse(PROXY_GET_REQ_LIST);
}

int CFaProto::ProxyGetRequestRecord()
{
	BYTE *pTxApdu = m_TxAPdu.bBuf;
	BYTE bAddrL, bTsa[17];
	BYTE *pApdu = &m_RxDPool.bBuf[3];
	WORD wTimeOut, wOneSeqLen;

	wTimeOut = (WORD )(pApdu[0]<<8) + pApdu[1];
	pApdu += 2;

	*pTxApdu++ = 0x89;//Proxy
	*pTxApdu++ = PROXY_GET_REQ_RECORD;//GetRequestRecord
	*pTxApdu++ = m_AppComm.bPIID;

	*pTxApdu++ = *pApdu++;	//跳过TSA数据长度
	*pTxApdu++ = *pApdu;	//地址长度
	bAddrL = *pApdu++ + 1;	//地址长度+1
	memcpy(pTxApdu, pApdu, bAddrL);
	memcpy(bTsa, pApdu, bAddrL);
	pApdu += bAddrL;
	pTxApdu += bAddrL;

	wOneSeqLen = m_LnkComm.wAPDULen - (pApdu-&m_RxDPool.bBuf[3]) - 1;	//-1:时间标签
	int iRet = DirAskMtrData(5, 3, bTsa, bAddrL, pApdu, wOneSeqLen, wTimeOut, pTxApdu);
	pApdu += wOneSeqLen;
	pTxApdu += iRet;

	*pTxApdu++ = 0;	//时间标签
	*pTxApdu++ = 0;	//上报标识

	m_TxAPdu.wLen = pTxApdu - m_TxAPdu.bBuf;
	return ToSecurityLayer(); //ToLnkLayer();
}

int CFaProto::ProxySetRequestList()
{
	return ProxyResponse(PROXY_SET_REQ_LIST);
}

int CFaProto::ProxySetThenGetRequestList()
{
	return ProxyResponse(PROXY_SET_THEN_GET_REQ_LIST);
}

int CFaProto::ProxyActionRequestList()
{
	return ProxyResponse(PROXY_ACT_REQ_LIST);
}

int CFaProto::ProxyActionThenGetRequestList()
{
	return ProxyResponse(PROXY_ACT_THEN_GET_REQ_LIST);
}

int CFaProto::ProxyTransCommandRequest()
{
	int iLen;
	BYTE bSopBitArry[] = {ONESTOPBIT, TWOSTOPBITS};
	BYTE *pTxApdu = m_TxAPdu.bBuf;
	BYTE bTsa[TSA_LEN] = {0};
	BYTE bTsaLen;
	DWORD dwOAD;
	TCommPara CommPara;
	BYTE *pApdu = &m_RxDPool.bBuf[3];
	WORD wFrmTimeOut, wByteTimeOut;

	*pTxApdu++ = 0x89;//Proxy
	*pTxApdu++ = 0x07;//TransCommandResponse
	*pTxApdu++ = m_AppComm.bPIID;
	memcpy(pTxApdu, pApdu, 4);
	pTxApdu += 4;

	revcpy((BYTE* )&dwOAD, pApdu, 4);
	pApdu += 4;
	if ((dwOAD&0xffffff00) == 0xf2010200)
	{
		BYTE bPort = (dwOAD & 0x000f);
		if (bPort>=LOGIC_PORT_MIN && bPort<=LOGIC_PORT_MAX)
			CommPara.wPort = bPort;
	}
	else 
	{
		CommPara.wPort = PORT_CCT_PLC;
	}

	CommPara.dwBaudRate = GbValToBaudrate(*pApdu++);
	CommPara.bParity = *pApdu++;
	CommPara.bByteSize = *pApdu++;
	CommPara.bStopBits = bSopBitArry[*pApdu++ - 1];
	pApdu++; //流控
	wFrmTimeOut = (WORD )(pApdu[0]<<8) + pApdu[1];
	pApdu += 2;
	wByteTimeOut = (WORD )(pApdu[0]<<8) + pApdu[1];
	pApdu += 2;

	 DWORD dwSendLen;
	iLen = DecodeLength(pApdu, &dwSendLen);
	pApdu += iLen;
	static const BYTE bOffsetLen = 8;
	BYTE *pTmpTxApdu = pTxApdu+bOffsetLen;
	int iRet = MtrDoFwd(CommPara, pApdu, dwSendLen, pTmpTxApdu, sizeof(m_TxAPdu.bBuf)-(pTmpTxApdu-m_TxAPdu.bBuf)-bOffsetLen, wFrmTimeOut, wByteTimeOut);
	if (iRet > 0)
	{
		BYTE bTmpBuf[8] = {0};
		*pTxApdu++ = 1;
		iLen = EncodeLength(iRet, bTmpBuf);
		memcpy(pTxApdu, bTmpBuf, iLen);
		pTxApdu += iLen;
		memcpy(pTxApdu, pTmpTxApdu, iRet);
		pTxApdu += iRet;
	}
	else
	{
		*pTxApdu++ = 0;
		*pTxApdu++ = 0xff;
	}

	*pTxApdu++ = 0x00;	//FollowReport 上报信息
	*pTxApdu++ = 0x00;	//时间标签

	m_TxAPdu.wLen = pTxApdu - m_TxAPdu.bBuf;
	return ToSecurityLayer(); //ToLnkLayer();
}

int CFaProto::ProxyResponse(BYTE bPoxyType)
{
	BYTE *pTxApdu = m_TxAPdu.bBuf;
	BYTE bAddrL, bTsa[TSA_LEN];
	BYTE bApdu[128];
	BYTE *pbApdu;
	//BYTE bSequence;
	BYTE bSerObNum, bObDscNum;
	WORD wPtr = 0, wOneSeqLen, wTmpLen;
	BYTE *pbRx = &m_RxDPool.bBuf[3];
	WORD wTimeOut, wOneTimeOut;
	DWORD dwLastTime;
	int iTypeLen;
	int iRet;

	dwLastTime = GetCurTime();
	wTimeOut = OoOiToWord(pbRx);
	pbRx += 2;
	bSerObNum = *pbRx++;
	if (bSerObNum == 0)
		return -1;
	wOneSeqLen = (m_LnkComm.wAPDULen - 7)/bSerObNum;

	memset(m_TxAPdu.bBuf, 0, sizeof(m_TxAPdu.bBuf));
	*pTxApdu++ = 0x89;//Proxy
	*pTxApdu++ = bPoxyType;//PROXY-Response∷=CHOICE
	*pTxApdu++ = m_AppComm.bPIID;
	*pTxApdu++ = bSerObNum;

	for (BYTE i=0; i<bSerObNum; i++)
	{
		*pTxApdu++ = *pbRx++;	//TSA数据长度
		*pTxApdu++ = *pbRx;	//地址长度
		bAddrL = *pbRx++ + 1;	//地址长度+1
		memcpy(pTxApdu, pbRx, bAddrL);
		memcpy(bTsa, pbRx, bAddrL);
		pTxApdu += bAddrL;
		pbRx += bAddrL;
		wOneTimeOut = OoOiToWord(pbRx);
		if (wOneTimeOut == 0)
			wOneTimeOut = wTimeOut;
		pbRx += 2;
		bObDscNum = *pbRx++;
		switch(bPoxyType)
		{
		case PROXY_GET_REQ_LIST:
			*pTxApdu++ = bObDscNum;
			for (BYTE k=0; k<bObDscNum; k++)
			{
				DTRACE(DB_FAPROTO, ("FaProto::ProxyResponse: bObDscNum=%d\r\n", bObDscNum));
				memset(bApdu, 0, sizeof(bApdu));
				pbApdu = bApdu;
				memcpy(pbApdu, pbRx, 4);
				pbApdu += 4;
				//*pbApdu++ = 0x00;	//时间标签
				iRet = DirAskMtrData(5, 1, bTsa, bAddrL, bApdu, pbApdu-bApdu, wOneTimeOut, pTxApdu);
				if (iRet <= 0)
				{
					memcpy(pTxApdu, pbRx, 4);
					pTxApdu += 4;
					*pTxApdu++ = DAR;	//错误信息
					*pTxApdu++ = DAR_REQ_TIMEOUT;
				}
				else
				{
					pTxApdu += iRet;	
				}
				pbRx += 4;

				if (GetCurTime() - dwLastTime > wTimeOut)
				{
					DTRACE(DB_FAPROTO, ("FaProto::PROXY_GET_REQ_LIST timeout here\r\n"));
					goto ProxyResponse_ret;
				}
			}
			break;
		case PROXY_SET_REQ_LIST:
		case PROXY_ACT_REQ_LIST:
			//*pbTx++ = bObDscNum;
			memset(bApdu, 0, sizeof(bApdu));
			pbApdu = bApdu;
			*pbApdu++ = bObDscNum;
			for (BYTE k=0; k<bObDscNum; k++)
			{
				int iLen;
				WORD wLen;
				BYTE bType;

				memcpy(pbApdu, pbRx, 4);
				const ToaMap* pOI = GetOIMap(OoOadToDWord(pbRx));
				if (pOI == NULL)
				{
					DTRACE(DB_FAPROTO, ("ProxyResponse(): PROXY_ACT_REQ_LIST Can`t support, dwOAD=%08x.\n", OoOadToDWord(pbRx)));
					return -1;
				}
				iLen = OoScanData(pbRx+4, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType);
				if (iLen < 0)
				{
					DTRACE(DB_FAPROTO, ("ProxyResponse(): PROXY_ACT_REQ_LIST Can`t support, dwOAD=%08x.\n", OoOadToDWord(pbRx)));
					return -1;
				}
				pbApdu += 4;
				pbRx += 4;
				memcpy(pbApdu, pbRx, iLen);
				pbApdu += iLen;
				pbRx += iLen;
			}

			*pbApdu++ = 0x00;	//时间标签
			if (bPoxyType == PROXY_SET_REQ_LIST)
				iRet = DirAskMtrData(6, 2, bTsa, bAddrL, bApdu, pbApdu-bApdu, wOneTimeOut, pTxApdu);
			else
				iRet = DirAskMtrData(7, 2, bTsa, bAddrL, bApdu, pbApdu-bApdu, wOneTimeOut, pTxApdu);

			if (iRet <= 0)
			{
				memcpy(pTxApdu, bApdu, 4);
				pTxApdu += 4;
				*pTxApdu++ = DAR;
				*pTxApdu++ = DAR_OTHER;
			}
			else
			{
				//pbTx += (iRet-2);	//由于抄读时，数据返回(2字节)： FllowReport + 时间标签
				pTxApdu += iRet;
			}

			if (GetCurTime() - dwLastTime > wTimeOut)
				goto ProxyResponse_ret;
			break;
		case PROXY_SET_THEN_GET_REQ_LIST:
		case PROXY_ACT_THEN_GET_REQ_LIST:
			int iLen;
			WORD wLen;
			BYTE bType;

			//*pbTx++ = bObDscNum;
			memset(bApdu, 0, sizeof(bApdu));
			pbApdu = bApdu;
			*pbApdu++ = bObDscNum;

			for (BYTE k=0; k<bObDscNum; k++)
			{
				//设置的对象属性描述符
				memcpy(pbApdu, pbRx, 4);
				const ToaMap* pOI = GetOIMap(OoOadToDWord(pbRx));
				if (pOI == NULL)
				{
					DTRACE(DB_FAPROTO, ("ProxyResponse(): PROXY_ACT_REQ_LIST Can`t support, dwOAD=%08x.\n", OoOadToDWord(pbRx)));
					return -1;
				}
				iLen = OoScanData(pbRx+4, pOI->pFmt, pOI->wFmtLen, false, -1, &wLen, &bType);
				if (iLen < 0)
				{
					DTRACE(DB_FAPROTO, ("ProxyResponse()1: PROXY_SET_THEN_GET_REQ_LIST Can`t support, dwOAD=%08x.\n", OoOadToDWord(pbRx)));
					return -1;
				}
				pbApdu += 4;
				pbRx += 4;
				memcpy(pbApdu, pbRx, iLen);
				pbApdu += iLen;
				pbRx += iLen;

				//读取的对象属性描述符
				memcpy(pbApdu, pbRx, 4);	
				pbApdu += 4;
				pbRx += 4;

				//及其延时读取时间
				*pbApdu++ = *pbRx++;
			}

			//时间标签
			*pbApdu++ = 0x00;	
			if (bPoxyType == PROXY_SET_THEN_GET_REQ_LIST)
				iRet = DirAskMtrData(6, 3, bTsa, bAddrL, bApdu, pbApdu-bApdu, wOneTimeOut, pTxApdu+1);
			else
				iRet = DirAskMtrData(7, 3, bTsa, bAddrL, bApdu, pbApdu-bApdu, wOneTimeOut, pTxApdu+1);

			if (iRet <= 0)
			{
				memcpy(pTxApdu, bApdu, 4);
				pTxApdu += 4;
				*pTxApdu++ = DAR;
				*pTxApdu++ = DAR_OTHER;
				memcpy(pTxApdu, &bApdu[4+iTypeLen], 4);
				*pTxApdu++ = DAR;
				*pTxApdu++ = DAR_OTHER;
			}
			else
			{
				*pTxApdu++ = 0x01;
				pTxApdu += iRet;
			}
			if (GetCurTime() - dwLastTime > wTimeOut)
				goto ProxyResponse_ret;

			break;
		}
	}

ProxyResponse_ret:

	*pTxApdu++ = 0x00;	//FollowReport 上报信息
	*pTxApdu++ = 0x00;	//时间标签

	m_TxAPdu.wLen = pTxApdu - m_TxAPdu.bBuf;
	return ToSecurityLayer(); //ToLnkLayer();
}

void CFaProto::DoProRelated()	//做一些协议相关的非标准的事情
{
	if (m_pIf == NULL)
		return;

	if (m_pIf->GetState() <= IF_STATE_CONNECT)
		m_dwProRxClick = 0;

	DoNoComuTimeout();
}

void CFaProto::SetWaitTime(BYTE bWaitMin)		//记录回帧等待超时
{
	m_LnkComm.m_bWaitMin = bWaitMin;
}

void CFaProto::StartWaitTimer(void)
{
	if (m_pIf->GetIfType() != IF_COMM)
	{
		m_dwProTxClick = GetClick();
		DTRACE(DB_FAPROTO, ("%s : start wait timer at click %d\n", __FUNCTION__, m_dwProTxClick));
	}
}

void CFaProto::ResetWaitTimer(void)
{
	//if (m_pIf->GetIfType() != IF_COMM)
	{
		m_dwProTxClick = 0;
		m_LnkComm.m_bLinkDetectCount = 0;
		DTRACE(DB_FAPROTO, ("%s : reset wait timer at click %d\n", __FUNCTION__, GetClick()));
	}
}

//缺省为黑龙江参数
void CFaProto::InitCommSize(BYTE bType)
{
	m_NegoSizeDef.bType = bType;
	switch( bType )
	{
	case CONNECTTYPE_GPRS:
		m_LnkComm.tTxTrsPara.tConnPara.wSenFrmMaxLen = LPDUSIZE;//1024;升级时单数据字节数就是1024了
		m_LnkComm.tTxTrsPara.tConnPara.wRcvFrmMaxLen = LPDUSIZE;//1024;
		m_LnkComm.tTxTrsPara.tConnPara.wHandleApduSize = APDUSIZE;
		m_LnkComm.tTxTrsPara.tConnPara.bRcvWindows = MAXLPDUNUM;
		m_LnkComm.tTxTrsPara.tConnPara.dwConnectTimeOut = 30;
		break;

	case CONNECTTYPE_LOCAL:
		m_LnkComm.tTxTrsPara.tConnPara.wSenFrmMaxLen = LPDUSIZE;
		m_LnkComm.tTxTrsPara.tConnPara.wRcvFrmMaxLen = LPDUSIZE;
		m_LnkComm.tTxTrsPara.tConnPara.wHandleApduSize = APDUSIZE;
		m_LnkComm.tTxTrsPara.tConnPara.bRcvWindows = MAXLPDUNUM;
		m_LnkComm.tTxTrsPara.tConnPara.dwConnectTimeOut = 30;
		break;

	default:
		m_LnkComm.tTxTrsPara.tConnPara.wSenFrmMaxLen = LPDUSIZE;//1024;
		m_LnkComm.tTxTrsPara.tConnPara.wRcvFrmMaxLen = LPDUSIZE;//1024;
		m_LnkComm.tTxTrsPara.tConnPara.wHandleApduSize = APDUSIZE;
		m_LnkComm.tTxTrsPara.tConnPara.bRcvWindows = MAXLPDUNUM;
		m_LnkComm.tTxTrsPara.tConnPara.dwConnectTimeOut = 30;
		break;
	}
}

//描述：组帧函数
//参数：pbBuf：终端应用数据(APDU链路层数据)
//      wLen:链路层应用数据长度
//返回：帧长度
int CFaProto::MakeFrm(BYTE *pbBuf, WORD wLen)
{
	WORD wFrmLen = wLen + m_LnkComm.bSvrAddLen - 1 + 10;;//APDU之外的长度：长度2Byte+控制域1Byte+地址AF(1Byte)+客户地址CA(1Byte)+HCS(2Byte)+FCS(2Byte)

	m_bTxBuf[0] = 0x68;
	m_bTxBuf[1] = (BYTE )wFrmLen;
	m_bTxBuf[2] = (BYTE )(wFrmLen>>8);

	BYTE bApduType = pbBuf[0];	//APDU 第一个字节为服务控制码
	if (bApduType == 1 )//Link请求
		m_bTxBuf[3] = 0x81;
	else if (bApduType == 0x88)//上报
		m_bTxBuf[3] = 0x83;
	else if ((bApduType) == 0x10 && (pbBuf[3]==0x88))//带加密上报
		m_bTxBuf[3] = 0x83;
	else
		m_bTxBuf[3] = 0xC3;

	if (m_LnkComm.fIsSegSend)
		m_bTxBuf[3] |= 0x20;

	//无论主站是否使用通配请求终端数据，终端上报的地址都是自己的地址
	m_bTxBuf[4] = m_LnkComm.bSvrAddLen-1;
	memcpy(&m_bTxBuf[5], m_LnkComm.bSvrAddr, m_LnkComm.bSvrAddLen);
	if (bApduType == 0x88)//上报
		m_bTxBuf[5+m_LnkComm.bSvrAddLen] = 0;
	else
		m_bTxBuf[5+m_LnkComm.bSvrAddLen] = m_LnkComm.bCliAddr;

	//0x68+Len(2byte)+C(1byte)+AF(1byte)+SA(SvrLen+1)+CA(1byte)
	WORD wHCS = CheckCrc16(&m_bTxBuf[1], m_LnkComm.bSvrAddLen+5); //帧头校验是除起始字符和HCS外的所有字节的校验
	m_bTxBuf[6+m_LnkComm.bSvrAddLen] = (BYTE )wHCS;
	m_bTxBuf[7+m_LnkComm.bSvrAddLen] = (BYTE )(wHCS>>8);

	memcpy(&m_bTxBuf[8+m_LnkComm.bSvrAddLen], pbBuf, wLen);

	//帧尾校验是除起始字符和FCS外的所有字节的校验,帧长度是除起始结束字符外的字符
	WORD wFCS = CheckCrc16(&m_bTxBuf[1], wFrmLen-2);
	m_bTxBuf[9+m_LnkComm.bSvrAddLen+wLen-1] = (BYTE )wFCS;
	m_bTxBuf[10+m_LnkComm.bSvrAddLen+wLen-1] = (BYTE )(wFCS>>8);
	m_bTxBuf[11+m_LnkComm.bSvrAddLen+wLen-1] = 0x16;
	
	m_wTxPtr = 12+m_LnkComm.bSvrAddLen+wLen-1;
	Send( m_bTxBuf, m_wTxPtr);//send函数返回的是bool
	return m_wTxPtr;//本函数要求返回的是发送的帧长度
}

int CFaProto::MakeAutoFrm(bool fFinal)
{
	WORD wFrmLen;	
	wFrmLen = (m_wAutoPtr>GetTxHeaderLen())?m_wAutoPtr + 1:m_wAutoPtr - 1;//m_wAutoPtr此时指到HCS后面的第１个字节或是FCS第１个字节

	m_bAutoBuf[0] = 0x7e;
	m_bAutoBuf[1] = 0xa0|((wFrmLen>>8)&0x7);	
	m_bAutoBuf[2] = wFrmLen&0xff;
	
	m_bAutoBuf[3] = 0x3;//客户端管理地址
	m_bAutoBuf[4] = 0x0;
	m_bAutoBuf[5] = 0x2;//服务器管理逻辑设备
//	m_bAutoBuf[6] = ((m_LnkComm.wDevAddr>>7)&0x7f)<<1;
//	m_bAutoBuf[7] = ((m_LnkComm.wDevAddr&0x7f)<<1)|1;

	m_bAutoBuf[8] = 0x03;
	if( fFinal == true )
		m_bAutoBuf[8] |= 0x10;

	WORD fcs;
	fcs = CheckCrc16( m_bAutoBuf+1, 8 );
	m_bAutoBuf[9] = fcs&0xff;
	m_bAutoBuf[10] = fcs>>8;

	if( m_wAutoPtr > GetTxHeaderLen() )
	{
		fcs = CheckCrc16( m_bAutoBuf+1, wFrmLen-2 );
		m_bAutoBuf[m_wAutoPtr++] = fcs&0xff;
		m_bAutoBuf[m_wAutoPtr++] = fcs>>8;
		m_bAutoBuf[m_wAutoPtr++] = 0x7e;
	}
	else
	{
		m_bAutoBuf[m_wAutoPtr++] = 0x7e;
	}

	int iRet = Send( m_bAutoBuf, m_wAutoPtr );
	memset( m_bAutoBuf, 0, MAXFRMSIZE );
	m_wAutoPtr = 0;   
	return iRet;
}


void CFaProto::ResetLnkPara()
{
	//读取IEC HDLC设置
	BYTE bBuf[32];
	int ret;

	memset(bBuf, 0x00, sizeof(bBuf));
	memset(bBuf, 0xff, 4);
	memcpy(m_LnkComm.tTxTrsPara.tConnPara.bProConformance, bBuf, 8);
	memcpy(m_LnkComm.tTxTrsPara.tConnPara.bFunConformance, bBuf, 16);
	m_LnkComm.tTxTrsPara.tConnPara.bRcvWindows = 10;
	m_LnkComm.tTxTrsPara.tConnPara.wHandleApduSize = 1024;
	m_LnkComm.tTxTrsPara.tConnPara.wRcvFrmMaxLen = 1024;
	m_LnkComm.tTxTrsPara.tConnPara.wSenFrmMaxLen = 1024;
	m_LnkComm.tTxTrsPara.tConnPara.dwConnectTimeOut = 100;
	m_LnkComm.bCommStep = 0;

	m_LnkComm.m_bWaitMin = 1;	//等待回帧超时默认为2分钟
}

void CFaProto::ClearVar(BYTE bConcentSta)//清零变量
{
	ResetWaitTimer();
//	ClearEventBit(EVENT_LINK_FAULT);

	ClsLPduPool( &m_RxLPduPool );
	ClsLPduPool( &m_TxLPduPool );
	ClsAPduPool( &m_RxAPduPool );
//	ClsAPduPool( &m_TxAPduPool );
	ClsDataPool( &m_RxDPool );
	ClsDataPool( &m_TxDPool );
}

int CFaProto::ToLnkLayer()
{
	BYTE i;
	TLPduPool *pLPool = &m_TxLPduPool;
	TAPdu *pTxApdu = &m_TxAPdu;
	BYTE *pbApdu = pTxApdu->bBuf;

	if (m_LnkComm.bAddrType == ADDR_TYPE_BROADCAST)	//广播地址无需回确认帧
		return 1;

	WORD wSigFrmSize = m_LnkComm.tTxTrsPara.tConnPara.wSenFrmMaxLen;
	if (pTxApdu->wLen <= wSigFrmSize)	//如果在链路一帧就可以发送完，就无需分帧了
	{
		m_LnkComm.fIsSegSend = false;
		int ret = MakeFrm(pTxApdu->bBuf, pTxApdu->wLen);
		return ret;
	}

	//如果空的则将取1个APDU分成帧
	if( pLPool->fValid == false )
	{
		WORD wTxApduLen;
		WORD wTxTotalLen = 0;
		WORD wFrmSegNo = 0;
		BYTE *p;

		ClsLPduPool(pLPool);
		
		for( i=0; i<MAXLPDUNUM; i++ )
		{
			p = pLPool->LPdu[i].bBuf;

			if(i == 0)	//首帧
			{
				wTxApduLen = wSigFrmSize - 2;
				*p++ = wFrmSegNo;	
				*p++ = (wFrmSegNo>>8) | FIRST_FRM;
			}
			else if ((pTxApdu->wLen - wTxTotalLen + 2) <= wSigFrmSize)	//结束帧
			{
				wTxApduLen = pTxApdu->wLen - wTxTotalLen;
				*p++ = wFrmSegNo;	
				*p++ = (wFrmSegNo>>8) | LAST_FRM;
			}
			else	//中间帧
			{
				wTxApduLen = wSigFrmSize - 2;
				*p++ = wFrmSegNo;	
				*p++ = (wFrmSegNo>>8) | MID_FRM;
			}

			memcpy(p, pbApdu, wTxApduLen);	
			p += wTxApduLen;	
			pbApdu += wTxApduLen;
			wTxTotalLen += wTxApduLen;
			
			pLPool->LPdu[i].fOk = false;
			//pLPool->LPdu[i].wLen = wTxOneLen;
			pLPool->LPdu[i].wLen = p - pLPool->LPdu[i].bBuf;
			pLPool->LPdu[i].fFinal = false;
			pLPool->LPdu[i].wFrmNo = wFrmSegNo++;
			if(wTxTotalLen >= pTxApdu->wLen)//判断是否最后一段(相对于1个块而言)
				pLPool->LPdu[i].fFinal = true;

			pLPool->bPduNum++;
			if(wTxTotalLen >= pTxApdu->wLen)
				break;
		}
		if( pLPool->bPduNum > 0)
			pLPool->fValid = true;
	}
	
	//发送1个LPDU
	pLPool->bStart = 0;	//假定每次分帧发送，分帧序号都从0开始
	return DoLPdu();
}

//检出一帧发送
int CFaProto::DoLPdu()
{
	int ret;
	BYTE bBuf[LPDUSIZE];
	WORD wLen;
	if( m_TxLPduPool.fValid == true )
	{
		//分帧格式域
		memcpy(bBuf, m_TxLPduPool.LPdu[m_TxLPduPool.bStart].bBuf, m_TxLPduPool.LPdu[m_TxLPduPool.bStart].wLen);
		wLen = m_TxLPduPool.LPdu[m_TxLPduPool.bStart].wLen;
// 		if( m_TxLPduPool.LPdu[m_TxLPduPool.bStart].fFinal == true )
// 			m_LnkComm.fIsSegSend = false;
// 		else
			m_LnkComm.fIsSegSend = true;
		
		ret = MakeFrm(bBuf, wLen);
		if (ret > 0)
		{
			m_LnkComm.wSendFrmSegNo = m_TxLPduPool.LPdu[m_TxLPduPool.bStart].wFrmNo;
			if ((m_TxLPduPool.bStart+1) == m_TxLPduPool.bPduNum)	//帧发送完毕
			{
				ClsLPduPool( &m_TxLPduPool);
				DTRACE(DB_FAPROTO, ("DoLPdu : Send all the LPDU-seg-frm at click %d\n", GetClick()));
			}

			m_TxLPduPool.bStart++;

			return ret;
		}
	}

	return -1;
}

int CFaProto::DoInsertLPdu()//处理一个插入的LPdu的组帧和发送（此时LPdu等于插入的APdu）
{
	if( m_TxInsAPduPool.fValid == false )
		return -1;

	m_wTxPtr = GetTxHeaderLen();
//	LLC_MakeFrmHead();
	memcpy( m_bTxBuf+m_wTxPtr, m_TxInsAPduPool.bBuf, m_TxInsAPduPool.wLen );
	m_wTxPtr += m_TxInsAPduPool.wLen;
	m_LnkComm.fIsSegSend = false;
	
	m_TxInsAPduPool.fValid = false;
	return 0;//MakeFrm( DL_FRM_I, true );
}

void CFaProto::ClsLPduPool(TLPduPool *p)
{
	memset(p, 0x00, sizeof(TLPduPool));
}

////////////////////////////////////////////////APP layer //////////////////////////////////////////////////////

void CFaProto::ClsAPduPool(TAPduPool *p)
{ 
	memset(p,0,sizeof(TAPduPool)); 
/*	for( DWORD i = 0; i < MAXAPDUNUM; i++ )
	{
		p->APdu[i].dwBlkNo = 0xffffffff;
	}*/
}

void CFaProto::NewAppServer()
{
	//m_AppComm
	m_AppComm.fNewServer = true;
	m_AppComm.bServer = m_AppComm.bCmd;//记录一次服务aarq/get/set/action,即第一个命令的服务值
	m_AppComm.bServerMod = m_AppComm.bCmdMod;//记录一次服务aarq/get/set/action的模式,即第一个命令模式的值
	m_AppComm.fMoreBlk = false;//根据bServer+bServerMod判别是否有多块
	m_AppComm.wBlkNo = 1;//当前块号  备注：组态是从1开始
	m_AppComm.iStep = -1;//操作库函数需要的参数,用来分布获取回复数据的
	m_AppComm.iTabIdx = 0;
	memset(m_AppComm.bAskBuf, 0, sizeof(m_AppComm.bAskBuf));

	m_AppComm.bAskItemNum = 0;//请求的属性数量


	m_AppComm.bAskMethodNum = 0;//请求的方法数量

	m_AppComm.bAnsServer = 0;		//记录一次服务响应的服务值
	m_AppComm.bAnsServerMod = 0;	//记录一次服务响应命令模式的值
	m_AppComm.bAnsCmdMod = 0;		//记录每一块响应命令模式的值
	m_AppComm.fAnsMoreBlk = false;	//根据响应数据判别是否有多块
	m_AppComm.dwAnsBlkNo = 1;		//响应数据当前块号

//	ClsDataPool( &m_RxDPool );
	ClsDataPool( &m_TxDPool );

	//m_RxAPduPool
	ClsAPduPool( &m_RxAPduPool );
//	ClsAPduPool( &m_TxAPduPool );

	sizeof(m_TxAPdu, 0x00, sizeof(m_TxAPdu));
	m_TxAPdu.fFinal = true;

	//m_TxLPduPool
	ClsLPduPool( &m_TxLPduPool );
}

//描述：处理get-request-normal,或者get-request-list（但只有一个OI）错误说明
int CFaProto::GetResErr(BYTE bGetMod,BYTE bErr)
{
	BYTE* pApdu = m_TxAPdu.bBuf;	//使用m_TxInsAPduPool，主要防止终端LPDU缓存有数据，直接插入会冲掉
	BYTE* pApdu0 = pApdu;

	*pApdu++ = GET_RES;
	*pApdu++ = bGetMod;
	*pApdu++ = m_AppComm.bPIID;
	memcpy(pApdu, m_AppComm.pbAskStart, 4);

	if( bGetMod == GET_NORMAL_LIST )//1个
	{	
		*pApdu++ = 0x01;	//num of seq
		*pApdu++ = 0x01;	//result
		*pApdu++ = bErr;	//DAR
	}
	else
	{
		*pApdu++ = 0x01;	//result
		*pApdu++ = bErr;	//DAR
	}
	m_TxAPdu.wLen = pApdu-pApdu0;
/*
	m_TxInsAPduPool.bBuf[ptr++] = GETRES;
	m_TxInsAPduPool.bBuf[ptr++] = bGetMod;
	m_TxInsAPduPool.bBuf[ptr++] = m_AppComm.bPIID;

	if( bGetMod == GETRES_BLK )
	{
//		dwBlkNo = m_TxAPduPool.APdu[m_TxAPduPool.bStart].dwBlkNo;

#ifdef DLMS_863_VER
		m_TxInsAPduPool.bBuf[ptr++] = (m_TxAPduPool.APdu[m_TxAPduPool.bStart].fFinal)?1:0;
#else
		m_TxInsAPduPool.bBuf[ptr++] = 0x03;
		m_TxInsAPduPool.bBuf[ptr++] = (m_TxAPduPool.APdu[m_TxAPduPool.bStart].fFinal)?1:0;
		m_TxInsAPduPool.bBuf[ptr++] = 0x06;
#endif
		m_TxInsAPduPool.bBuf[ptr++] = (BYTE)(dwBlkNo>>24);
		m_TxInsAPduPool.bBuf[ptr++] = (BYTE)(dwBlkNo>>16);
		m_TxInsAPduPool.bBuf[ptr++] = (BYTE)(dwBlkNo>>8);
		m_TxInsAPduPool.bBuf[ptr++] = (BYTE)dwBlkNo;
		m_TxInsAPduPool.bBuf[ptr++] = 1;//err tag
		m_TxInsAPduPool.bBuf[ptr++] = bErr;//err value
	}
	else if( bGetMod == GETRES_LIST )//1个
	{		
		m_TxInsAPduPool.bBuf[ptr++] = 1;
		m_TxInsAPduPool.bBuf[ptr++] = 1;//err tag
		m_TxInsAPduPool.bBuf[ptr++] = bErr;//err value
	}
	else
	{		
		m_TxInsAPduPool.bBuf[ptr++] = 1;//err tag
		m_TxInsAPduPool.bBuf[ptr++] = bErr;//err value
	}
	m_TxInsAPduPool.wLen = ptr;
	m_TxInsAPduPool.fValid = true;
*/
	//发给链路层
	return ToSecurityLayer(); //ToLnkLayer();
}
/*
int CFaProto::SetRes(BYTE bSetMod)
{
	BYTE* pApdu = m_TxInsAPduPool.bBuf;	//使用m_TxInsAPduPool，主要防止终端LPDU缓存有数据，直接插入会冲掉
	BYTE* pApdu0 = pApdu;

	*pApdu++ = SET_RES;
	*pApdu++ = bSetMod;
	*pApdu++ = m_AppComm.bPIID;

	switch( bSetMod )
	{
	case SETRES_NORMAL://1
		*pApdu++ = (BYTE )m_AppComm.AskItem[0].wOI>>8;
		*pApdu++ = (BYTE )m_AppComm.AskItem[0].wOI;
		*pApdu++ = (BYTE )m_AppComm.AskItem[0].bAttr;
		*pApdu++ = (BYTE )m_AppComm.AskItem[0].bIndex;

		*pApdu++ = bAskOIResult[0];
		break;

	case SETRES_LIST://5 n个
		*pApdu++ = m_AppComm.bAskItemNum;
		for( i = 0; i < m_AppComm.bAskItemNum; i++ )
		{
			*pApdu++ = (BYTE )m_AppComm.AskItem[i].wOI>>8;
			*pApdu++ = (BYTE )m_AppComm.AskItem[i].wOI;
			*pApdu++ = (BYTE )m_AppComm.AskItem[i].bAttr;
			*pApdu++ = (BYTE )m_AppComm.AskItem[i].bIndex;

			*pApdu++ = bAskOIResult[i];//err value
		}
		break;
	}

	m_TxInsAPduPool.wLen = pApdu-pApdu0;
	m_TxInsAPduPool.fValid = true;
	//发给链路层
	return ToLnkLayer();
}

//除了模式4时bErr_BlkNo=BlkNo，其它bErr_BlkNo=bErr
int CFaProto::ActRes(BYTE bActMod, DWORD bErr_BlkNo)
{
	BYTE* pApdu = m_TxInsAPduPool.bBuf;	//使用m_TxInsAPduPool，主要防止终端LPDU缓存有数据，直接插入会冲掉
	BYTE* pApdu0 = pApdu;

	*pApdu++ = ACTION_RES;
	*pApdu++ = bSetMod;
	*pApdu++ = m_AppComm.bPIID;

	switch( bActMod )
	{
	case ACTRES_NORMAL://1
		*pApdu++ = (BYTE )m_AppComm.AskItem[i].wOI>>8;
		*pApdu++ = (BYTE )m_AppComm.AskItem[i].wOI;
		*pApdu++ = (BYTE )m_AppComm.AskItem[i].bAttr;
		*pApdu++ = (BYTE )m_AppComm.AskItem[i].bIndex;

		*pApdu++ = bActResult[0];

		if (bErr == 0xff)// pan 2008-1-15 ACT正确接收但取数据失败的情况
		{
			m_TxInsAPduPool.bBuf[ptr++] = 0;//act ok
			m_TxInsAPduPool.bBuf[ptr++] = 1;//opt data
			m_TxInsAPduPool.bBuf[ptr++] = 1;//data err tag
			m_TxInsAPduPool.bBuf[ptr++] = AR_Other;//err value
		}	
		else //ACT接收错误
		{
			m_TxInsAPduPool.bBuf[ptr++] = bErr;
			m_TxInsAPduPool.bBuf[ptr++] = 0;
		}
		break;
		
	case ACTRES_LIST://3
		*pApdu++ = m_AppComm.bAskMethodNum;
		for( i = 0; i < m_AppComm.bAskMethodNum; i++ )
		{
			*pApdu++ = (BYTE )m_AppComm.dwActOMD[i]>>24;
			*pApdu++ = (BYTE )m_AppComm.dwActOMD[i]>>16;
			*pApdu++ = (BYTE )m_AppComm.dwActOMD[i]>>8;
			*pApdu++ = (BYTE )m_AppComm.dwActOMD[i];

			*pApdu++ = bActResult[i];//err value
		}
		break;
	}

	m_TxInsAPduPool.wLen = pApdu-pApdu0;
	m_TxInsAPduPool.fValid = true;
	//发给链路层
	return ToLnkLayer();
}*/

int CFaProto::DecodeChoice(BYTE *pbBuf)
{
	int ret;
	DWORD num;
	switch( *pbBuf++ )
	{
	case 15: 
	case DT_UNSIGN:
		return 2;
	case 16: 
	case DT_LONG_U: 
		return 3;
	case 5: 
	case 6: 
	case DT_FLOAT32:
	case DT_TIME:
		return 5;
	case DT_DATE:
		return 6;
	case DT_LONG64:
	case DT_LONG64_U:
	case DT_FLOAT64:
		return 9;
	case DT_DATE_TIME:
		return 13;	
	case 9:
	case 10:
		ret = DecodeLength( pbBuf, &num );
		if( ret > 0 )
			return (ret+num+1);
		break;
	}
	return -1;
}

//取得上报方式
BYTE CFaProto::GetEvtRptFlg()
{
	BYTE bBuf[25];
	memset(bBuf, 0, sizeof(bBuf));

	ReadItemEx(BN0, PN0, 0x40d0, bBuf);

	if ((bBuf[19]&0x01) == 0) //不允许上报
		return NOT_RPT;

	if ((bBuf[19]&0x09) == 0x09) //立即上报，优先级最高
		return NOW_RPT;
	else if ((bBuf[19]&0x05) == 0x05) //心跳上报告警位
		return BEAT_RPT;
	else if ((bBuf[19]&0x03) == 0x03) //尾随上报
		return FOLLOW_RPT;
	
	return NOT_RPT;
}


bool CFaProto::GetEventWritePtr(BYTE& bWrPtr)
{
/*	int fd = TdbOpenTable(EventFileName, O_RDWR|O_BINARY);
	if(fd >= 0)
	{
		m_AppComm.bEvtWPtr = TdbGetRecPtr(fd); 
		TdbCloseTable(fd);
		return true;
	}
	else*/
	{
//		DTRACE(DB_METER, ("TdbGetRecPtr():: fail to open table %s\n", EventFileName));
		return false;
	}

}

bool CFaProto::IsNeedReport()
{
	if ( !GetEventWritePtr(m_AppComm.bEvtWPtr) )
		return false;		

	//重要事件上报
	if( m_pFaProPara->ProPara.fLocal == false ) //CONNECTTYPE_GPRS 
		ReadItemEx( BANK0, 0, 0x5502, &m_AppComm.bEvtRPtr );
 	else
		ReadItemEx( BANK0, 0, 0x5503, &m_AppComm.bEvtRPtr );
	if (m_AppComm.bEvtWPtr!=m_AppComm.bEvtRPtr)
	{
		DTRACE(DB_FAPROTO, ("TaskThread : write write ptr=%d  read ptr=%d!\r\n",m_AppComm.bEvtWPtr,m_AppComm.bEvtRPtr));
	}
	if( m_AppComm.bEvtRPtr != m_AppComm.bEvtWPtr )
		return true;
	
	return false;
}

//A 主动上报1条事件:用事件对象
//B 主动上报10条事件:用事件曲线
int CFaProto::GetEvent(BYTE *pb)
{
	BYTE *pEvtBuf = pb;
	int num=0,iStep=-1,ret;
	TTime t;

//1.整理数目
	if( m_AppComm.bEvtWPtr >= m_AppComm.bEvtRPtr )
		num = m_AppComm.bEvtWPtr - m_AppComm.bEvtRPtr;
	else
		num = (m_AppComm.bEvtWPtr+256)-m_AppComm.bEvtRPtr;
	if( !num )
		return 0;	

	num = 1;//A=1
	//if( num > 10 )num = 10;//B=10

//2.组织报文
	BYTE bEvtCapt[9]={0x00,0x01, 0x00, 0x00, 0x60, 0x51, 0x03, 0xff,  0x02};
	BYTE bObisEvt[9]={0x00,0x07, 0x00, 0x00, 0x63, 0x62, 0x00, 0xff,  0x02};
	BYTE bOptBuf[20]={ 0x02, 0x02, 0x04, 
						0x06, 0x00, 0x00, 0x00, 0xff,
						0x06, 0x00, 0x00, 0x00, 0xff,
						0x12, 0x00, 0x01, 
						0x12, 0x00, 0x00, 
	};
	bOptBuf[7] = m_AppComm.bEvtRPtr;
	bOptBuf[12] = m_AppComm.bEvtRPtr+num-1;//不对，回头了没处理？？？


	*pEvtBuf++ = EVTNOTE;//event notification cmd

	*pEvtBuf++ = 0x01;//带时间标签	
	GetCurTime( &t );
	*pEvtBuf++ = (BYTE)(t.nYear>>8);
	*pEvtBuf++ = (BYTE)t.nYear;
	*pEvtBuf++ = t.nMonth;
	*pEvtBuf++ = t.nDay;
	*pEvtBuf++ = t.nWeek;
	*pEvtBuf++ = t.nHour;
	*pEvtBuf++ = t.nMinute;
	*pEvtBuf++ = t.nSecond;
	*pEvtBuf++ = 0xff;
	*pEvtBuf++ = 0x80;
	*pEvtBuf++ = 0x00;
	*pEvtBuf++ = 0x00;

	//A
	memcpy( pEvtBuf, bEvtCapt, 9 );
	//B
	//memcpy( pEvtBuf, bObisEvt, 9 );

	pEvtBuf += 9;

	//B
	//*pEvtBuf++ = 0x01;
	//*pEvtBuf++ = num;
	
/*
//3.读取数据	
	//if( (ret=DlmsReadObj(7,bObisEvt,2,pEvtBuf,bOptBuf,iStep)) > 0 ) 
	//pan 2007-12-29 起始记录号的意义与内部数据库的意义不一样，前者是FIFO,后者是循环缓冲区
	//另外如果多条从曲线读取，则没地方给出选择性访问参数，应该是全部BUF
	//应该是用事件对象按FIFO的顺序将未传的一条条连续传出		
	if( (ret=GetOneEvent(m_AppComm.bEvtRPtr, pEvtBuf)) > 0 ) 
	{
		pEvtBuf += ret;	
		m_AppComm.bEvtRPtr = (m_AppComm.bEvtRPtr+1)%MAXNUM_EVENT;
		if( m_pFaProPara->ProPara.fLocal == false )//CONNECTTYPE_GPRS 
			WriteItemEx( BANK0, 0, 0x5502, &m_AppComm.bEvtRPtr );
		else
			WriteItemEx( BANK0, 0, 0x5503, &m_AppComm.bEvtRPtr );
		TrigerSaveBank(BN0, SECT_DLMS_TERMN_DATA, 0);
		return (pEvtBuf-pb);
	}*/
	return 0;
}

//数据优先帧
int CFaProto::Tx_PriorFrm(bool fFinal)
{
	int iRet = -1;
	BYTE bBuf[500];
	BYTE bTmpTx[500];
	WORD wFrmLen, wTxPtr = 0;	

	BYTE* pbTmpTx =  bTmpTx;

	wTxPtr = 0;
	pbTmpTx[0] = 0x7e;
	//m_bTxBuf[1] = 0xC0|((wFrmLen>>8)&0x7);	
	//m_bTxBuf[2] = wFrmLen&0xff;
	
	wTxPtr += 3;
	ReadItemEx( BN0, 0, 0x4037, bBuf);//倒序
	revcpy(&pbTmpTx[3], bBuf, 2);
	wTxPtr += 2;

	if (IsFkTermn())
		pbTmpTx[wTxPtr++] = 0x03;	//负控终端事件
	else
		pbTmpTx[wTxPtr++] = 0x02;	//集中器事件

	pbTmpTx[wTxPtr++] = 0x02;
	pbTmpTx[wTxPtr++] = 0x04;
	pbTmpTx[wTxPtr++] = 0x11;
	pbTmpTx[wTxPtr++] = m_AppComm.bEvtRPtr;	//循环事件编号

/*		//应该是用事件对象按FIFO的顺序将未传的一条条连续传出		
	if( (iRet=GetOneEvent(m_AppComm.bEvtRPtr, bBuf)) > 0 )  //02 03 要去掉
	{
		memcpy(pbTmpTx+wTxPtr, bBuf+2, iRet-2) ; //拷贝datatime+enum+ERC等的内容
		wTxPtr += iRet-2;

		m_AppComm.bEvtRPtr = (m_AppComm.bEvtRPtr+1)%MAXNUM_EVENT;
		if( m_pFaProPara->ProPara.fLocal == false )//CONNECTTYPE_GPRS 
			WriteItemEx( BANK0, 0, 0x5502, &m_AppComm.bEvtRPtr );
		else
			WriteItemEx( BANK0, 0, 0x5503, &m_AppComm.bEvtRPtr );
		TrigerSaveBank(BN0, SECT_DLMS_TERMN_DATA, 0);		
	}
	else */
		return -1;

	wFrmLen = wTxPtr+2-1; //算上校验的长度 去掉帧头
	pbTmpTx[1] = 0xC0|((wFrmLen>>8)&0x7);	//数据优先帧控制字
	pbTmpTx[2] = wFrmLen&0xff;

	WORD fcs = CheckCrc16( pbTmpTx+1, wTxPtr-1 );
	pbTmpTx[wTxPtr++] = fcs&0xff;
	pbTmpTx[wTxPtr++] = fcs>>8;
	pbTmpTx[wTxPtr++] = 0x7e;
	
	iRet = Send(pbTmpTx, wTxPtr);
	return iRet;
}

//物理连接断开时调用
void CFaProto::OnBroken()
{
	m_nRxStep = 0;
//	if (GetEventFlag()&EVENT_LINK_FAULT)	//断点续传状态，应用连接仍然保持
//		m_AppComm.bConnectSta = APPST_OK;
//	else
		m_AppComm.bConnectSta = APPST_NOK;
}


void CFaProto::OnConnectOK()
{ 
	CProto::OnConnectOK();
}



///////////////////////////////////兼容浙江版的扩展协议帧定义////////////////////////////////////////////////////////////
int CFaProto::ZJHandleFrm(BYTE* pbRxBuf, BYTE* pbTxBuf)
{
	int iResult = -1;
	WORD  wRxDataLen = (WORD )pbRxBuf[FAP_LEN+1]*0x100 + pbRxBuf[FAP_LEN];

	switch (pbRxBuf[FAP_CMD] & FAP_CMD_GET)
	{
#if 0
		case 0x01:    //读当前数据
			iResult = ZJReadData(pbRxBuf, wRxDataLen, pbTxBuf);
			break;
			
	    case 0x08:    //写对象参数
            iResult = ZJWriteData(pbRxBuf, wRxDataLen, pbTxBuf);
			break;
#endif
		case FAP_CMD_USERDEF:
			iResult = ZJUserDef(pbRxBuf, wRxDataLen, pbTxBuf);
			break;
			
		default:
			break;

	} //switch (pbRxBuf[FAP_CMD] & FAP_CMD_GET)

	return iResult;
}

int CFaProto::ZJUserDef(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	if (pbRxBuf[FAP_DATA+1]!=0x3d || pbRxBuf[FAP_DATA+2]!=0x5a)
		return -1;

	switch (pbRxBuf[FAP_DATA+3])    //扩展控制码
	{
		case 0x01:
			return ZJReadDataEx(pbRxBuf, wRxDataLen, pbTxBuf, false);

		case 0x02:
			return ZJWriteDataEx(pbRxBuf, wRxDataLen, pbTxBuf, false);

		case 0x03:					//格式化硬盘
			if (!PswCheck(DI_HIGH_PERM, &pbRxBuf[FAP_DATA_EX]))
				return ZJReExtCmd(ERR_PERM, pbRxBuf, pbTxBuf);

			g_dwExtCmdClick = GetClick();
			g_dwExtCmdFlg = FLG_FORMAT_DISK;
			return ZJReExtCmd(ERR_OK, pbRxBuf, pbTxBuf);

		case 0x07:
			g_dwExtCmdClick = GetClick();
			g_dwExtCmdFlg = FLG_REMOTE_DOWN;
			g_PowerOffTmp.bRemoteDownIP[0] = pbRxBuf[FAP_DATA_EX+3]; 
			g_PowerOffTmp.bRemoteDownIP[1] = pbRxBuf[FAP_DATA_EX+4];
			g_PowerOffTmp.bRemoteDownIP[2] = pbRxBuf[FAP_DATA_EX+5];
			g_PowerOffTmp.bRemoteDownIP[3] = pbRxBuf[FAP_DATA_EX+6];			
			g_PowerOffTmp.bRemoteDownIP[4] = pbRxBuf[FAP_DATA_EX+7];
			g_PowerOffTmp.bRemoteDownIP[5] = pbRxBuf[FAP_DATA_EX+8];
			g_PowerOffTmp.bRemoteDownIP[6] = pbRxBuf[FAP_DATA_EX+9];
			g_PowerOffTmp.bRemoteDownIP[7] = pbRxBuf[FAP_DATA_EX+10];
			return ZJReExtCmd(ERR_OK, pbRxBuf, pbTxBuf);

		case 0x08:			//自动校准
			return ZJTrigerAdj(pbRxBuf, wRxDataLen, pbTxBuf);
		case 0x17:			//att7022自动校准
			return ZJTrigerAdj(pbRxBuf, wRxDataLen, pbTxBuf);

		case 0x0E:
			return ZJSftpDataEx(pbRxBuf,wRxDataLen, pbTxBuf);//下载

		case 0x11:
			return ZJReadDataEx(pbRxBuf, wRxDataLen, pbTxBuf, true);
		case 0x12:
			return ZJWriteDataEx(pbRxBuf, wRxDataLen, pbTxBuf, true);
			
		case 0x13:
		  	ZJRunCmd(pbRxBuf, wRxDataLen, pbTxBuf);
			return ZJReExtCmd(ERR_OK, pbRxBuf, pbTxBuf);
		
		case 0x15:	//集中器扩展命令
			if (!PswCheck(DI_HIGH_PERM, &pbRxBuf[FAP_DATA_EX]))
				return ZJReExtCmd(ERR_PERM, pbRxBuf, pbTxBuf);
			
			if (pbRxBuf[FAP_DATA_EX+3] == 0x01) //01H清路由
			{
				SetInfo(INFO_PLC_CLRRT);//?????
				return ZJReExtCmd(ERR_OK, pbRxBuf, pbTxBuf);
			}
			else
			{
				return ZJReExtCmd(ERR_ITEM, pbRxBuf, pbTxBuf);
			}
			break;
		case 0x10:
			return ZJLoadParaFile(pbRxBuf,wRxDataLen, pbTxBuf);//下载		

		case 0x20://模拟量校准
			//		BYTE bBuf[48] = { 0 };
			//		bBuf[20] = 0x10;
			//		bBuf[21] = 0x27;
			//		g_dcSample.TrigerAdj(bBuf);
#ifndef SYS_WIN		
			DCTrigerAdj((BYTE *)&pbRxBuf[FAP_DATA+3+4]);
			return ZJReExtCmd(ERR_OK, pbRxBuf, pbTxBuf);
#endif
			break;
#ifdef SYS_LINUX
		case 0x19://U盘升级
			if (!PswCheck(DI_HIGH_PERM, &pbRxBuf[FAP_DATA_EX]))
				return ZJReExtCmd(ERR_PERM, pbRxBuf, pbTxBuf); 

			if (UsbUpdate((char *)&pbRxBuf[FAP_DATA_EX+3]))
			{
				ZJReExtCmd(ERR_OK, pbRxBuf, pbTxBuf); 
				SetBeep();
				Sleep(500);
				ResetBeep();
				Sleep(500);
				SetBeep();
				Sleep(500);
				ResetBeep();
				ResetCPU();
				return true;
			}
			else
			{
				SetBeep();
				Sleep(2000);
				ResetBeep();
				return ZJReExtCmd(ERR_ITEM, pbRxBuf, pbTxBuf); 
				return true;
			}
			break;
#endif //SYS_LINUX
	default:
		return -1;
	}

	return -1;
}

//描述：通信帧的数据已经放好在p指向的通信缓冲区，本函数给它加上其余的通信字段
//返回：通信帧的长度
int CFaProto::ZJMakeFrm(WORD wDataLen, BYTE* pbRxBuf, BYTE* pbTxBuf, bool fErr)
{
	pbTxBuf[FAP_CMD] = (pbRxBuf[FAP_CMD] & 0x3f) | FAP_CMD_UP;
	if (fErr)
		pbTxBuf[FAP_CMD] |= 0x40;

	pbTxBuf[FAP_LEN] = wDataLen & 0xff;
	pbTxBuf[FAP_LEN+1] = (wDataLen >> 8) & 0xff;

	return wDataLen + FAP_FIX_LEN;
}	


int CFaProto::ZJReplyErr(BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf)
{
	pbTxBuf[FAP_DATA] = bErrCode;
    return ZJMakeFrm(1, pbRxBuf, pbTxBuf, true);
}

int CFaProto::ZJReExtCmd(BYTE bErrCode, BYTE* pbRxBuf, BYTE* pbTxBuf)
{
	BYTE* pbRx = &pbRxBuf[FAP_DATA];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];

	*pbTx++ = *pbRx++;  //厂商编号
	*pbTx++ = *pbRx++;  //科陆识别码
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //扩展控制码
	*pbTx++ = bErrCode;
	
    return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA), pbRxBuf, pbTxBuf, false);
}

int CFaProto::ZJReadDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf, bool fWordPn)
{
	BYTE* pbRx = &pbRxBuf[FAP_DATA];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];

	*pbTx++ = *pbRx++;  //厂商编号
	*pbTx++ = *pbRx++;  //科陆识别码
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //扩展控制码

	WORD wPn;
	if (fWordPn)
	{
		wPn = ByteToWord(pbRx);
		*pbTx++ = *pbRx++;
		*pbTx++ = *pbRx++;
	}
	else
	{
		wPn = *pbRx;
		*pbTx++ = *pbRx++;
	}

	while (pbRx < &pbRxBuf[wRxDataLen])//FAP_DATA+
	{
		BYTE bBank = *pbRx;
		WORD wID = ByteToWord(pbRx+1);

		*pbTx++ = *pbRx++;   //Bank
		*pbTx++ = *pbRx++;   //数据项编号
		*pbTx++ = *pbRx++;
				   
        int nRead = ReadItemEx(bBank, wPn, wID, pbTx);
		if (nRead < 0)
		{
//				pbTx += ZJReplyErr(-nRead, pbRxBuf, pbTxBuf);
//				return (WORD)(pbTx-pbTxBuf);
			return ZJReplyErr(-nRead, pbRxBuf, pbTxBuf);//只要有ID出错则整个报错
		}
		else   //正常
		{
			pbTx += nRead;
		}
	}

	if (pbTx == &pbTxBuf[FAP_DATA+5])    //根本就没读到数据
	{
//			pbTx += ZJReplyErr(ERR_ITEM, pbRxBuf, pbTx);
//			return (WORD)(pbTx-pbTxBuf);
		return ZJReplyErr(ERR_ITEM, pbRxBuf, pbTxBuf);
	}
//	    return (WORD)(pbTx-pbTxBuf);//ZJMakeFrm((WORD)(pbTx-pbTxBuf), pbRxBuf, pbTxBuf, false);
	return ZJMakeFrm((WORD)(pbTx-&pbTxBuf[FAP_DATA]), pbRxBuf, pbTxBuf, false);
}

//描述:写扩展数据
int CFaProto::ZJWriteDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf, bool fWordPn)
{
	BYTE* pbRx = &pbRxBuf[FAP_DATA];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];

	*pbTx++ = *pbRx++;  //厂商编号
	*pbTx++ = *pbRx++;  //科陆识别码
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //扩展控制码

	WORD wPn;
	WORD wPswOffset;
	if (fWordPn)
	{
		wPn = ByteToWord(pbRx);
		*pbTx++ = *pbRx++;
		*pbTx++ = *pbRx++;
		wPswOffset = 7;
	}
	else
	{
		wPn = *pbRx;
		*pbTx++ = *pbRx++;
		wPswOffset = 6;
	}

	BYTE bPerm = *pbRx++;
	pbRx += 3;   //跳过密码

	int nWritten;
	while (pbRx < &pbRxBuf[wRxDataLen])
	{
		BYTE bBank = *pbRx;
		WORD wID = ByteToWord(pbRx+1);

		*pbTx++ = *pbRx++;   //Bank
		*pbTx++ = *pbRx++;   //数据项编号
		*pbTx++ = *pbRx++;

		nWritten = WriteItemEx(bBank, wPn, wID, pbRx, bPerm, &pbRxBuf[FAP_DATA+wPswOffset]);
		if (nWritten < 0)
		{
			nWritten = -nWritten;
			if (nWritten == ERR_ITEM)   //本系统不支持的数据项，数据长度不清楚，所以下面的数据没法设置
			{
				*pbTx++ = (BYTE )nWritten;  //数据项设置结果
				break;
			}
			else   //系统有的数据项，只是权限等没通过
			{
				*pbTx++ = (BYTE )(nWritten & 0xff);  //数据项设置结果
				nWritten = nWritten >> 8;
			}
		}
		else   //设置成功
		{
			*pbTx++ = ERR_OK;              //数据项设置结果
		}

		pbRx += nWritten;   //跳过数据
	}

    TrigerSavePara();
//	    return (WORD)(pbTx-pbTxBuf);//ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA), pbRxBuf, pbTxBuf, false);
	return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA), pbRxBuf, pbTxBuf, false);

}

int CFaProto::ZJSftpDataEx(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	BYTE* pbRx = &pbRxBuf[FAP_DATA];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];

	*pbTx++ = *pbRx++;  //厂商编号
	*pbTx++ = *pbRx++;  //科陆识别码
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //扩展控制码

	pbRx += 3;   //跳过密码

	memset(m_bSftpBuf, 0, sizeof(m_bSftpBuf));

	if(m_pSftpClient->HandleFrm(pbRx, m_bSftpBuf))
	{
		memcpy(pbTx, m_bSftpBuf, m_pSftpClient->m_wTxLen);
		return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA+m_pSftpClient->m_wTxLen), pbRxBuf, pbTxBuf, false);
	}
	else
	{
		return -1;
	}
}

int CFaProto::ZJLoadParaFile(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	BYTE* pbRx = &pbRxBuf[FAP_LEN];
	BYTE* pbTx = &pbTxBuf[FAP_DATA];
	char  szPathName[PATHNAME_LEN+1];
	BYTE bRet = 0;
	WORD wLen = 0;
	WORD wNameLen = 0;
	
	wLen = ByteToWord(pbRx);  
	
	pbRx += 2;   
	*pbTx++ = *pbRx++;  //厂商编号
	*pbTx++ = *pbRx++;  //科陆识别码
	*pbTx++ = *pbRx++;
	*pbTx++ = *pbRx++;  //扩展控制码

	pbRx += 3;   //跳过密码
	
	wNameLen = wLen-FAP_DATA_EX;  	//文件名长度
	
	if (wNameLen > PATHNAME_LEN)
	{
		return 2;
	}
	memcpy(szPathName, pbRx, wNameLen);
	bRet = g_pmParaMgr.LoadPara(szPathName);
	if (bRet == 0)
	{
		if (!g_pmParaMgr.Parse())
			bRet = -1;
		else
            TrigerSave();
	}
	
	memcpy(pbTx, &bRet, sizeof(bRet));
	return ZJMakeFrm((WORD)(pbTx-pbTxBuf-FAP_DATA+sizeof(bRet)), pbRxBuf, pbTxBuf, false);
}

bool CFaProto::ZJRunCmd(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	memset(m_szCmdLine, '\0', 100);
	WORD wCmdLen = ByteToWord(&pbRxBuf[FAP_DATA_EX+3]);
		DTRACE(DB_FAPROTO, ("CFaProto::ZJRunCmd cmdLen=%x\n", wCmdLen));
	
  	memcpy(m_szCmdLine, &pbRxBuf[FAP_DATA_EX+5], wCmdLen);
		DTRACE(DB_FAPROTO, ("CFaProto::ZJRunCmd RunCmd::%s\n", m_szCmdLine));

#ifdef SYS_LINUX  
	system(m_szCmdLine);
#endif    
	return true;
}

int CFaProto::ZJTrigerAdj(BYTE* pbRxBuf, WORD wRxDataLen, BYTE* pbTxBuf)
{
	bool fRet = false;	

#ifdef EN_AC
	fRet = AcTrigerAdj(&pbRxBuf[FAP_DATA_EX+3]);
	if (pbRxBuf[FAP_DATA_EX+3] == 0)
	{
		BYTE bBuf[128];
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BN25, PN0, 0x5005, bBuf);  //0x5005 72 ATT7022校正参数
	}
#endif
	if (fRet)
	{
		TrigerSavePara();
		
		return ZJReExtCmd(ERR_OK, pbRxBuf, pbTxBuf);
//			ZJReplyErr(ERR_OK, pbRx, pbTx++);
	}
	else
	{
		return ZJReExtCmd(ERR_ITEM, pbRxBuf, pbTxBuf);
//			ZJReplyErr(ERR_ITEM, pbRx, pbTx++);
	}
//		return (WORD)(pbTx-pbTxBuf);//
}

void CFaProto::DoNoComuTimeout()
{
	if (g_GbProPara.wConnectType == CONNECTTYPE_GPRS)
	{
		DWORD dwClick = GetClick();

		BYTE bBuf[100];
		ReadItemEx(BN0, PN0, 0x40c0, bBuf);
		if (bBuf[21] != 0)
		{
			if (dwClick-m_dwClickFrmOk > (DWORD)bBuf[21]*3600)
			{
				if (m_bNoComuSta == 0)
				{
					m_bNoComuSta = 1;
					WriteItemEx(BN2, PN0, 0x2021, &m_bNoComuSta);
				}
			}
			else
			{
				if (m_bNoComuSta == 1)
				{
					m_bNoComuSta = 0;
					WriteItemEx(BN2, PN0, 0x2021, &m_bNoComuSta);
				}
			}
		}
	}
}

//描述：主动上送
bool CFaProto::AutoSend()
{
	BYTE bNSend = 0;
	BYTE bBuf[10];
	DWORD dwTick = GetClick();

	if ( !m_pFaProPara->ProPara.fAutoSend)
		return false;
	
	/*
	TTime TmpTime;
	GetCurTime(&TmpTime);
	if ((TmpTime.nHour==23) && (TmpTime.nMinute>=56))//最后4分钟不报了.
		return false;
	if ((TmpTime.nHour==0) && (TmpTime.nMinute<2))//跨日时等待2分钟再报.
		return false;
	*/
	
//总开头判断，即是否允许上报
	//是取43000800
	memset(bBuf, 0, sizeof(bBuf));
	ReadItem(PN0, 0x4308, bBuf);
	if (bBuf[1] != 0x01)
		return false;

//对于跟随上报应该怎么做呢? (这个功能先放一下)

	//处理待确认区的是还没有需要再上报的
	ReRpt(&bNSend);
	//处理事件上报
	EventRpt(&bNSend);
	//处理任务上报

	//if ((dwOldTick+2) <= dwTick)
	if (m_dwOldTick!= dwTick)
	{
		TaskRpt(&bNSend);
		m_dwOldTick = dwTick;
	}
	
	return true;
}

//任务上报处理
void CFaProto::TaskRpt(BYTE * pbNSend)
{
	DWORD dwSecsNow = GetCurTime();
	BYTE i;	
	bool fIsRestC3TxCnt = false;
	TFapRptMsg *pMsg;
	TTaskCfg tTaskCfg;
	BYTE *pbSchCfg;
	DWORD dwOMD;
	const TOmMap *p;
	WORD wLen;
	WORD wOI;
	BYTE bType, bNum;
	BYTE *pbTaskCSD = NULL, *pbRSD = NULL;
	int nRet;
	BYTE bBuf[20];	//读取单个OAD,最大允许长度为512字节
	BYTE* pApdu = m_TxAPdu.bBuf;
	BYTE* pApdu0 = pApdu;
	BYTE *pRecBuf;
	int ret;
	BYTE bRcdNum = 0;//RecordData时的成员过个数
	BYTE bOAD[4];
	BYTE * pRCSD = NULL;
	BYTE * pRSD = NULL;
	int iTabIdx = 0;
	WORD wRetNum;
	DWORD dwCurSec, dwStartSec = 0, dwEndSec = 0, dwRptSec = 0;
	DWORD dwPerStartMin, dwPerEndMin;
	DWORD dwCycTime;
	BYTE bTxCnt;
	WORD wID;
	BYTE * pTmp = NULL;
	BYTE bMethod;

	if (*pbNSend >= RPTMAXFRM_EVERYSEND)
		return;

	//如果待确认区满了，则不在处理常规队列，以防还有待确认帧无处可存
	if (WaitQueGetNum() >= WaitQueGetSize())
		return;
	
	if ( m_pFaProPara->ProPara.fAutoSend == false)//本通道不具备上报功能测退出
		return;

	if (m_wCurTaskId >= TASK_ID_NUM)
	{
		m_wCurTaskId = 0;
		m_iStart = -1;
	}

	for (; m_wCurTaskId<TASK_ID_NUM; m_wCurTaskId++)
	{
		if (m_wCurTaskId != m_wLastTaskId)
		{
			m_wLastTaskId = m_wCurTaskId;
			m_iStart = -1;
			m_iRdPn = 0;
		}

		if (GetTaskCfg(m_wCurTaskId, &tTaskCfg) && (tTaskCfg.bSchType == SCH_TYPE_REPORT))
		{
			if (GetTaskCurExeTime(&tTaskCfg, &dwCurSec, &dwStartSec, &dwEndSec) != 0)
				continue;//不在执行时间内则跳过

			wID = 0x6002+m_pIf->GetIfType();
			ReadBankId(BANK16, m_wCurTaskId, wID, (BYTE*)&dwRptSec);//取得上一次的上报时间sec
			if ((dwRptSec==dwCurSec) && (dwRptSec!=0))
			{
				m_iStart = -1;
				continue;//本间隔里已上报过了
			}
			
			ret = 0;
			pbSchCfg = GetSchCfg(&tTaskCfg, &ret);
			if ((ret > 0) && (pbSchCfg != NULL))
			{
				WORD pwFmtLen = 0; 
				dwOMD = 0x601C7F00;
				//p = GetSchFmt(SCH_TYPE_REPORT, &pwFmtLen);//岑总说要把下边的p = GetOmMap(dwOMD)换成这种方式
				p = GetOmMap(dwOMD);//要改成上边的，记得!!
				if (p ==NULL)
				{
					m_iStart = -1;
					continue;
				}

				if ((pbTaskCSD=OoGetField(pbSchCfg, p->pFmt+2, p->wFmtLen-2, 1, &wLen, &bType)) != NULL)//取上报通道,OoGetField()返回的偏移没有跳过了格式字符位置
				{
					//判断方案通道里是否包括了自身线程通道，是则往下执行
					BYTE n;
					for (n=0; n<pbTaskCSD[1]; n++)
					{
						DWORD dwCn = OoOadToDWord(&pbTaskCSD[3+n*5]);
						//if (dwCn == m_dwCnOAD)
#ifndef SYS_WIN
						if ((dwCn&0xfff00000) == m_dwCnOAD)
							break;
#else
						break;
#endif
					}
					if (n >= pbTaskCSD[1])
					{
						m_iStart = -1;
						continue;
					}
				}
					
				if ((pbTaskCSD=OoGetField(pbSchCfg, p->pFmt+2, p->wFmtLen-2, 4, &wLen, &bType)) != NULL)//取上报内容项
				{
					pApdu = m_TxAPdu.bBuf;
					pApdu0 = pApdu;
					if(pbTaskCSD[3] == 0)//上报类型是OAD
					{
						 wOI = OoOiToWord(&pbTaskCSD[5]);
						*pApdu++ = REPORT_NOTI;	//上报
						*pApdu++ = GET_NORMAL;
						pTmp = pApdu;//先记录PIID的位置，组帧发送时再获取PIID
						*pApdu++ = 0;//PIID;
						*pApdu++ = 1;//SEQUENCE OF OAD 个数=1,暂不考虑分帧
						memcpy(pApdu, &pbTaskCSD[5], 4);
						pApdu += 4;
						nRet = OoProReadAttr(wOI, pbTaskCSD[7], pbTaskCSD[8], pApdu, 900, &m_iStart);//APDUSIZE,ESAM那边给的空间有限
						//发送后直接放到待确认区
						//先不考虑分帧,到时参看Get_request_normal()里的做法再来做分帧
						if (nRet < 0)	//没取到数据
						{
							*pApdu++ = 0x00;	//为NULL
						}
						else
							pApdu += nRet;
						*pApdu++ = 0x00;	//无上报信息
						*pApdu++ = 0x00;	//无时标
						*pTmp = GetMyPIID();					
						m_TxAPdu.wLen = pApdu - pApdu0;
					
						//return ToLnkLayer();
						//WORD wSigFrmSize = m_LnkComm.tTxTrsPara.tConnPara.wSenFrmMaxLen;
						//if (pApdu->wLen <= wSigFrmSize)	//如果在链路一帧就可以发送完，就无需分帧了
						{
							m_LnkComm.fIsSegSend = false;
							//加密处理
							BYTE bSecBuf[APDUSIZE];
							int wSecLen;
							WORD wApduLen = 0;
							BYTE * pSec = NULL;
							
							memset(bSecBuf, 0, sizeof(bSecBuf));
							if (m_TxAPdu.wLen >= (APDUSIZE-32))
								m_TxAPdu.wLen = APDUSIZE-32;
							wSecLen = Rpt_SecureLayer(pApdu0,  m_TxAPdu.wLen, bSecBuf);
							if (wSecLen < 0)
								return;
							else if (wSecLen > 0)
							{
								wApduLen = wSecLen;
								pSec = bSecBuf;
							}
							else
							{
								wApduLen = m_TxAPdu.wLen;
								pSec = pApdu0;
							}
							ret = MakeFrm(pSec, wApduLen);
							//ret = MakeFrm(pApdu0, m_TxAPdu.wLen);
							DTRACE(DB_FAPROTO, ("---TaskRpt ResultNormal bOAD=0x%08x  nRet=%d.---\n", OoOadToDWord(&pbTaskCSD[5]), ret));
							if (ret > 0)
							{
								if (m_iStart == -1)
								{
									memset(bBuf, 0, sizeof(bBuf));
									DWordToByte(dwCurSec, bBuf);//dwCurSec
									WriteItemEx(BANK16, m_wCurTaskId, wID, bBuf);//记录本次的上报时间sec
								}

								//发送成功，把该帧放入确认区
								//注意，如果上报的信息不需要确认的就不用走下边的流程了
								if ((pbTaskCSD=OoGetField(pbSchCfg, p->pFmt+2, p->wFmtLen-2, 3, &wLen, &bType)) != NULL)
									bTxCnt = pbTaskCSD[1];//取得最大上报次数
								else
									bTxCnt = 1;
								
								if ((pbTaskCSD=OoGetField(pbSchCfg, p->pFmt+2, p->wFmtLen-2, 2, &wLen, &bType)) != NULL)
								{
									pbTaskCSD ++;
									TTimeInterv Interv;
									Interv.bUnit = pbTaskCSD[0];
									Interv.wVal = OoOiToWord(&pbTaskCSD[1]);
									dwCycTime = TiToSecondes(&Interv);//,取得上报响应超时时间
								}
								else
								{
									dwCycTime = 0;
								}

								WaitQue(GB_DATACLASS1, bTxCnt, dwCycTime, m_wCurTaskId, m_bTxBuf, ret);
								
								*pbNSend = *pbNSend + 1;
								if (*pbNSend >= RPTMAXFRM_EVERYSEND)
									return;
							}
						}
					}
					else//上报类型是RecordData上报记录型对象属性
					{
						TOobMtrInfo tMtrInfo;
						TTime tTime;
						int iRCSDLen, iDayFrzState;
						WORD wPn;
						BYTE bTaskCsdBuf[1024] = {0};
						BYTE bMtrMask[PN_MASK_SIZE] = {0};
						BYTE *pbMsd, *pTime, *pSechFrzOad;
						
						
						
						if (wLen > sizeof(bTaskCsdBuf))
						{
							DTRACE(DB_FAPROTO, ("TaskRpt error, wLen=%d > bTaskCSD=%d.\n", wLen, sizeof(bTaskCsdBuf)));
							return;
						}
						memcpy(bTaskCsdBuf, pbTaskCSD, wLen);
						memset(bOAD, 0, sizeof(bOAD));
						memcpy(bOAD, bTaskCsdBuf+7, 4);//取得主对象描述符
						pRCSD = bTaskCsdBuf+12;//取得bRCSD个数位置
						pRSD = pRCSD;
						pRSD += ScanRCSD(pRCSD, false);
						pRSD++;//把RSD的第一个格式字符去掉了

						bMethod = *pRSD;
						switch (bMethod)
						{
						case 0:
							pRSD[0] = 10;	//RSD=10
							pRSD[1] = 1;	//上1条记录
							pbMsd = pRSD+2;
							pbMsd[0] = DT_MS;
							pbMsd[1] = 0x01; //所有表
							break;
						case 4:
						case 5:
							pbMsd = pRSD + (1+7);	//1+7:RSD类型+格式date_time_s 
							ParserMsParam(pbMsd, bMtrMask, sizeof(bMtrMask));
							if (m_iStart < 0)
							{
								if ((m_iRdPn=SearchNextPnFromMask(bMtrMask, m_iRdPn)) < 0)
								{
									WriteItemEx(BANK16, m_wCurTaskId, wID, (BYTE*)&dwCurSec);//记录本次的上报时间sec
									goto NEXT_TASK;
								}	
							}
							GetMeterInfo(m_iRdPn, &tMtrInfo);
							*pbMsd++ = 0x04;	//MS“一组配置序号”
							*pbMsd++ = 0x01;	//1个表序号
							pbMsd += OoWordToOi(tMtrInfo.wMtrSn, pbMsd);
							DTRACE(DB_FAPROTO, ("###TaskRpt bMethod=%d, wTaskId=%d, wSn=%d.\n", bMethod, m_wCurTaskId, tMtrInfo.wMtrSn));
							break;
						case 6:
						case 7:
							pbMsd = pRSD + (1+7+7+3);//1+7+7+3: RSD类型+date_time_s+date_time_s+TI, 
							pTime = pRSD+1;
							if (IsAllAByte(pTime, 0xff, 7))	//起始时间
							{
								dwStartSec -= TiToSecondes(&(tTaskCfg.tiExe));
								SecondsToTime(dwStartSec, &tTime);
								OoTimeToDateTimeS(&tTime, pTime);
							}
							if (IsAllAByte(pTime+7, 0xff, 7))	//结束时间
							{
								dwEndSec -= TiToSecondes(&(tTaskCfg.tiExe));
								dwEndSec -= 60;
								SecondsToTime(dwEndSec, &tTime);
								OoTimeToDateTimeS(&tTime, pTime+7);
							}
							ParserMsParam(pbMsd, bMtrMask, sizeof(bMtrMask));
							if (m_iStart < 0)
							{
								if ((m_iRdPn=SearchNextPnFromMask(bMtrMask, m_iRdPn)) < 0)
								{
									WriteItemEx(BANK16, m_wCurTaskId, wID, (BYTE*)&dwCurSec);//记录本次的上报时间sec
									goto NEXT_TASK;
								}	
							}
							GetMeterInfo(m_iRdPn, &tMtrInfo);
							*pbMsd++ = 0x04;	//MS“一组配置序号”
							*pbMsd++ = 0x01;	//1个表序号
							pbMsd += OoWordToOi(tMtrInfo.wMtrSn, pbMsd);
							DTRACE(DB_FAPROTO, ("###TaskRpt bMethod=%d, wTaskId=%d, wSn=%d.\n", bMethod, m_wCurTaskId, tMtrInfo.wMtrSn));
							break;
						case 8:
							pbMsd = pRSD+(1+7+7+3);//1+7+7+3: RSD类型+date_time_s+date_time_s+TI, 
							pTime = pRSD+1;

							char szRptStartTime[32];
							char szRptEndTime[32];
							memset(szRptStartTime, 0, sizeof(szRptStartTime));
							memset(szRptEndTime, 0, sizeof(szRptEndTime));

							if (IsAllAByte(pTime, 0xff, 7))	//起始时间
							{
								dwStartSec -= 2*TiToSecondes(&(tTaskCfg.tiExe));
								SecondsToTime(dwStartSec, &tTime);
								OoTimeToDateTimeS(&tTime, pTime);
								TimeToStr(tTime, szRptStartTime);
							}
							if (IsAllAByte(pTime+7, 0xff, 7))	//结束时间
							{
								dwEndSec -= 2*TiToSecondes(&(tTaskCfg.tiExe));
								SecondsToTime(dwEndSec, &tTime);
								OoTimeToDateTimeS(&tTime, pTime+7);
								TimeToStr(tTime, szRptEndTime);
							}

							DTRACE(DB_FAPROTO, ("------TaskRpt bMethod=%d, wTaskId=%d, RptStartTime:%s, RptEndTime:%s.-------\n", bMethod, m_wCurTaskId, szRptStartTime, szRptEndTime));
							break;
						case 10:
							pbMsd = pRSD + 2;	//1+2: 上n条记录  unsigned”
							ParserMsParam(pbMsd, bMtrMask, sizeof(bMtrMask));
							if (m_iStart < 0)
							{
								if ((m_iRdPn=SearchNextPnFromMask(bMtrMask, m_iRdPn)) < 0)
								{
									WriteItemEx(BANK16, m_wCurTaskId, wID, (BYTE*)&dwCurSec);//记录本次的上报时间sec
									goto NEXT_TASK;
								}	
							}
							GetMeterInfo(m_iRdPn, &tMtrInfo);
							*pbMsd++ = 0x04;	//MS“一组配置序号”
							*pbMsd++ = 0x01;	//1个表序号
							pbMsd += OoWordToOi(tMtrInfo.wMtrSn, pbMsd);
							DTRACE(DB_FAPROTO, ("###TaskRpt bMethod=%d, wTaskId=%d, wSn=%d.\n", bMethod, m_wCurTaskId, tMtrInfo.wMtrSn));
							break;
						default:
							DTRACE(DB_FAPROTO, ("###TaskRpt wTaskId=%d, Nonsupport bMethod=%d.\n", m_wCurTaskId, bMethod));
							WriteItemEx(BANK16, m_wCurTaskId, wID, (BYTE*)&dwCurSec);//记录本次的上报时间sec
							goto NEXT_TASK;
						}

						*pApdu++ = REPORT_NOTI;	//上报
						*pApdu++ = GET_NORMAL_LIST;
						pTmp = pApdu;//先记录PIID的位置，组帧发送时再获取PIID
						*pApdu++ = 0;//PIID;
						*pApdu++ = 1;//SEQUENCE OF OAD 个数=1,暂不考虑分帧
						memcpy(pApdu, bOAD, 4);
						pApdu += 4;

						iRCSDLen = ScanRCSD(pRCSD, false);	
						if (iRCSDLen != 1)	//RCSDLen=1表示无RCSD
						{
							memcpy(pApdu, pRCSD, iRCSDLen);	
							pApdu += iRCSDLen;
						}
						*pApdu++ = 0x01;	//Data
						nRet = ReadRecord(bOAD, pRSD, pRCSD, &iTabIdx, &m_iStart, pApdu+1, 2500, &wRetNum);//APDUSIZE,ESAM那边给的空间有限
						pRecBuf = pApdu+1;
						switch (bMethod)
						{
						case 0:
							if (m_iStart <= -1)
							{
								WriteItemEx(BANK16, m_wCurTaskId, wID, (BYTE*)&dwCurSec);//记录本次的上报时间sec
								if (nRet <= 0)
									goto NEXT_TASK;
							}
							break;
						case 4:
						case 5:
						case 6:
						case 7:
						case 10:
							if (m_iStart <= -1)	//表示所有的表地址都上报完成了，切到下一个任务
							{
								int iTmpPn = m_iRdPn;
								iTmpPn = SearchNextPnFromMask(bMtrMask, iTmpPn);	
								if (iTmpPn < 0)
								{
									WriteItemEx(BANK16, m_wCurTaskId, wID, (BYTE*)&dwCurSec);//记录本次的上报时间sec
									if (nRet < 0)
										goto NEXT_TASK;
								}
								if (nRet <= 0)
									goto NEXT_PN;
							}

							break;
						case 8:
							if (m_iStart <= -1)
							{
								WriteItemEx(BANK16, m_wCurTaskId, wID, (BYTE*)&dwCurSec);//记录本次的上报时间sec
								if (nRet < 0)
									goto NEXT_TASK;
							}
							break;
						default:
							DTRACE(DB_FAPROTO, ("###TaskRpt read task-db wTaskId=%d, Error: bMethod=%d unsupport.\n", m_wCurTaskId, bMethod));
							WriteItemEx(BANK16, m_wCurTaskId, wID, (BYTE*)&dwCurSec);//记录本次的上报时间sec
							goto NEXT_TASK;
						}

						//浙江要求数据上报需判断冻结时间，时间不对直接回NULL
						iDayFrzState = DayFrzTimeMatch(pRecBuf, pRCSD);
						if (iDayFrzState >= 1)	
						{
							*pApdu = wRetNum;
							pApdu += nRet+1;
							*pApdu++ = 0x00;	//无上报信息
							*pApdu++ = 0x00;	//无时标		
							*pTmp = GetMyPIID();					
							m_TxAPdu.wLen = pApdu - pApdu0;
						}
						else if (iDayFrzState == 0)
						{
							*pApdu++ = 0;
							*pApdu++ = 0x00;	//无上报信息
							*pApdu++ = 0x00;	//无时标		
							*pTmp = GetMyPIID();					
							m_TxAPdu.wLen = pApdu - pApdu0;
						}
						else
						{
							*pApdu = wRetNum;
							pApdu += nRet+1;
							*pApdu++ = 0x00;	//无上报信息
							*pApdu++ = 0x00;	//无时标		
							*pTmp = GetMyPIID();					
							m_TxAPdu.wLen = pApdu - pApdu0;
						}
						DTRACE(DB_FAPROTO, ("TaskRpt Record bOAD=0x%08x  nRet=%d  wRetNum=%d\r\n", OoOadToDWord(bOAD), nRet,wRetNum));	

					
						//return ToLnkLayer();
						//WORD wSigFrmSize = m_LnkComm.tTxTrsPara.tConnPara.wSenFrmMaxLen;
						//if (pApdu->wLen <= wSigFrmSize)	//如果在链路一帧就可以发送完，就无需分帧了
						{
							m_LnkComm.fIsSegSend = false;
							//加密处理
							BYTE bSecBuf[APDUSIZE];
							int wSecLen;
							WORD wApduLen = 0;
							BYTE * pSec = NULL;
							
							memset(bSecBuf, 0, sizeof(bSecBuf));
							if (m_TxAPdu.wLen >= APDUSIZE-32)
								m_TxAPdu.wLen = APDUSIZE-32;
							wSecLen = Rpt_SecureLayer(pApdu0,  m_TxAPdu.wLen, bSecBuf);
							if (wSecLen < 0)
								return;
							else if (wSecLen > 0)
							{
								wApduLen = wSecLen;
								pSec = bSecBuf;
							}
							else
							{
								wApduLen = m_TxAPdu.wLen;
								pSec = pApdu0;
							}
							ret = MakeFrm(pSec, wApduLen);
							//ret = MakeFrm(pApdu0, m_TxAPdu.wLen);
							if (ret > 0)//m_bTxBuf
							{
								//发送成功，把该帧放入确认区
								//注意，如果上报的信息不需要确认的就不用走下边的流程了
								if ((pbTaskCSD=OoGetField(pbSchCfg, p->pFmt+2, p->wFmtLen-2, 3, &wLen, &bType)) != NULL)
									bTxCnt = pbTaskCSD[1];//取得最大上报次数
								else
									bTxCnt = 1;
								
								if ((pbTaskCSD=OoGetField(pbSchCfg, p->pFmt+2, p->wFmtLen-2, 2, &wLen, &bType)) != NULL)
								{
									pbTaskCSD ++;
									TTimeInterv Interv;
									Interv.bUnit = pbTaskCSD[0];
									Interv.wVal = OoOiToWord(&pbTaskCSD[1]);
									dwCycTime = TiToSecondes(&Interv);//,取得上报响应超时时间
								}
								else
								{
									dwCycTime = 0;
								}

								WaitQue(GB_DATACLASS2, bTxCnt, dwCycTime, m_wCurTaskId, m_bTxBuf, ret);
								
								*pbNSend = *pbNSend + 1;
								if (*pbNSend >= RPTMAXFRM_EVERYSEND)
									return;
								return;
							}							
						}

NEXT_PN:
						break;
NEXT_TASK:
						;
					}
				}
			}
		}
	}
}

//描述：暂存待确认的上报帧
//参数：@bClass  上报类型
//		@dwOAD事件OAD
//		@dwCycTime 等待响应的最大超时时间
//		@bNum对于任务上报则为任务号，对于事件上报则为事件表的索引
//		@bStage 记录事件的发生阶段,0不关注；1发生；2结束
//返回：正确返回true，否则返回false
void CFaProto::WaitQue(BYTE bClass, BYTE bTxCn, DWORD dwCycTime, WORD bNum, BYTE *pBuf, WORD wLen, BYTE  bStage)
{
	TFapRptMsg pMsg;
	DWORD dwSecsNow = GetCurTime();


	pMsg.bClass = bClass;
	pMsg.bTask = bNum;//对于事件就记录好事件列表的序号,任务上报的就不关心
	pMsg.bNeedConfirm = true;//m_bConfirmedFlg;
	if (bTxCn == 0)
		bTxCn = 3;
	if (bTxCn >3)
		bTxCn = 3;

	if (bClass == GB_DATACLASS3)
	{
		pMsg.bSeq = bStage;//用bSeq来记录事件的发生阶段
	}
	if (bTxCn > 20)
		bTxCn = 20;
	pMsg.bTxCnt = bTxCn;//取得最大上报次数
	
	pMsg.dwCycTime = dwCycTime;//取得上报响应超时时间
	
	if (pMsg.dwCycTime == 0)
		pMsg.dwCycTime = 60;//默认为60秒
		
	memset(pMsg.bRptInfBuf, 0, 16);
	pMsg.bRptInfBuf[0] = 0;//bFn;
	pMsg.bRptInfBuf[1] = 0;//bEc;
	memcpy(pMsg.bBuf, pBuf, wLen);
	pMsg.wLen = wLen;
	//发送完毕后的处理
	if (pMsg.bNeedConfirm)//对于需要确认的先放到待确认区
	{
		pMsg.dwTrigCnt = 1;//后续每重发一次就加1
		pMsg.dwSendTime = dwSecsNow;
		if (!WaitQueInsert(&pMsg) )
		{
			DTRACE(DB_FAPROTO, ("CProMngRpt::DoMngRptSend: failed to Insert a NeedConfirm msg\n"));
		}
	}
}


//处理待确认区的是还没有需要再上报的
void CFaProto::ReRpt(BYTE * pbNSend)
{
	TFapRptMsg pMsg;
	DWORD dwSecsNow = GetCurTime();
	BYTE i;

//总开头判断，即是否允许上报
	//等陈工那边,是取43000800/43000700
	
	for(i=0; i<RPTWAITQUE_SIZE; i++)//WaitQueGetSize()
	{
		if (WaitQueGetMsg(i, &pMsg) == false)
			continue;
		//重发处理：超时再发，超次停止
		if (pMsg.dwSendTime + pMsg.dwCycTime < dwSecsNow)
		{
			if (pMsg.dwTrigCnt >= pMsg.bTxCnt)//超时又超次删除
			{
				//if( (pMsg->bClass!=GB_DATACLASSGRADEERC && pMsg->bClass!=GB_DATACLASS3) || ((GetClick()- pMsg->dwPostTime)>10*60) )
				{
					//EchoSend(pMsg, true);//如果重发失败后不推进记录给false!
					WaitQueDelete(i);
				}				
			}
			else//超时不超次重发
			{
				Send(pMsg.bBuf,pMsg.wLen); //tll ????XXX
				pMsg.dwSendTime = dwSecsNow;
				pMsg.dwTrigCnt ++;
				memcpy(&m_pRptMsg[i], &pMsg, sizeof(TFapRptMsg));
				
				*pbNSend = *pbNSend + 1;
				if (*pbNSend >= RPTMAXFRM_EVERYSEND)
					return;
			}
		}		
	}	
}

//事件上报处理
void CFaProto::EventRpt(BYTE * pbNSend)
{
	int nRet = 0;
	BYTE bBuf[200] = {0};
	WORD wLen = 0;
	TEvtMsg tEvnMsg;
	int *ptr = NULL;//读取任务库的当前记录指针
	BYTE* pApdu = m_TxAPdu.bBuf;
	BYTE* pApdu0 = pApdu;
	DWORD dwCycTime;
	BYTE bTxCnt;
	BYTE * pTmp = NULL;

	if (m_queEvt.GetMsgNum() <= 0)
		return;
	if (m_queEvt.Remove(bBuf, 2) >= 7)
	{
		memcpy((BYTE *)&tEvnMsg, bBuf, sizeof(TEvtMsg));
		//pEvnMsg = (TEvtMsg *)bBuf;
		
		DTRACE(DB_FAPROTO, ("EventRpt---dwOAD=%08x---bBuf=%02x%02x%02x%02x--7! \r\n",tEvnMsg.dwOAD,bBuf[0],bBuf[1],bBuf[2],bBuf[3]));	
		pApdu = m_TxAPdu.bBuf;
		pApdu0 = pApdu;
		*pApdu++ = REPORT_NOTI;	//上报
		*pApdu++ = GET_NORMAL_LIST;//事件给的是OAD，所以上报事件时这里应该是01
		pTmp = pApdu;//先记录PIID的位置，组帧发送时再获取PIID
		*pApdu++ = 0;//PIID;
		*pApdu++ = 1;//SEQUENCE OF OAD 个数=1,暂不考虑分帧
		//revcpy(pApdu, bBuf, 4);//注意，如果读事件接口返回的数据里带了这个OAD,则这里就不用再拷了
		//OoDWordToOad(pEvnMsg->dwOAD, pApdu);
		//pApdu += 4;
		
		//*pApdu++ = 0x01;	//Data
		if (tEvnMsg.bStage == EVT_STAGE_TASK)
		{
			memcpy(pApdu, tEvnMsg.bRcsd, tEvnMsg.wRcsdLen);
			pApdu += tEvnMsg.wRcsdLen;
			*pApdu++ = 1; //数据
			*pApdu++ = 1; //条数
		}
		nRet = GetEvtRec(&tEvnMsg, pApdu, 1024, 0);//黄工那边可能会修改Msg里的dwOAD,
		if (nRet > 0)
		{
			DTRACE(DB_FAPROTO, ("AppendEvtMsg--dwOAD=0x%08x-wRecIdx=%04x-bStage=%02x-nRet=%d---6! \r\n",tEvnMsg.dwOAD, tEvnMsg.wRecIdx, tEvnMsg.bStage,nRet));	
			BYTE btStage = 0;
			
			if (tEvnMsg.bStage == 1)
				btStage = 1;
			else if (tEvnMsg.bStage == 2)
				btStage = 4;
			if (tEvnMsg.bStage != EVT_STAGE_TASK)
				UpdateEvtRptState(m_dwCnOAD, &tEvnMsg, btStage);
			//让事件那边分析一个读回时回复的事件帧看，我还不知上报事件的具体格式
			pApdu += nRet;
			//下边的两个信息字节如果组帧那里有处理这里就不用加了,待确认
			*pApdu++ = 0x00;	//无上报信息
			*pApdu++ = 0x00;	//无时标
									
			*pTmp = 	GetMyPIID();					
			m_TxAPdu.wLen = pApdu - pApdu0;
			m_LnkComm.fIsSegSend = false;
			//加密处理
			BYTE bSecBuf[APDUSIZE];
			int wSecLen;
			WORD wApduLen = 0;
			BYTE * pSec = NULL;

			memset(bSecBuf, 0, sizeof(bSecBuf));
			if (m_TxAPdu.wLen >= APDUSIZE-32)
				m_TxAPdu.wLen = APDUSIZE-32;
			wSecLen = Rpt_SecureLayer(pApdu0,  m_TxAPdu.wLen, bSecBuf);
			if (wSecLen < 0)
				return;
			else if (wSecLen > 0)
			{
				wApduLen = wSecLen;
				pSec = bSecBuf;
			}
			else
			{
				wApduLen = m_TxAPdu.wLen;
				pSec = pApdu0;
			}
			nRet = MakeFrm(pSec, wApduLen);
				
			if (nRet > 0)
			{
				//发送成功，把该帧放入确认区
				//注意，如果上报的信息不需要确认的就不用走下边的流程了
				//事件的最大上报次数和上报响应超时时间从哪里得知，如果默认应该为什么值合理?
				bTxCnt = 3;//最大上报次数
				dwCycTime = 60;//上报响应超时时间
				if (tEvnMsg.bStage != EVT_STAGE_TASK)
					WaitQue(GB_DATACLASS3, bTxCnt, dwCycTime, tEvnMsg.wRecIdx, m_bTxBuf, nRet, tEvnMsg.bStage);
				else
					WaitQue(GB_DATACLASS2, bTxCnt, dwCycTime, tEvnMsg.wRecIdx, m_bTxBuf, nRet, tEvnMsg.bStage);
			}
			
		}
	}
	
}

//清待确认区
void CFaProto::WaitQueClear()
{
	memset(&m_pRptMsg, 0, sizeof(TFapRptMsg)*RPTWAITQUE_SIZE);
	m_bRptMsgNum = 0;
}
//删除待确认区的一条消息
bool CFaProto::WaitQueDelete(BYTE bIdx)
{
	if (bIdx >= RPTWAITQUE_SIZE)
		return false;
	if (!m_bRptMsgNum)// || bIdx>=m_bRptMsgNum)
		return false;

	memset(&m_pRptMsg[bIdx], 0, sizeof(TFapRptMsg));

	if (m_bRptMsgNum > 0)
		m_bRptMsgNum--;
	return true;
}

bool CFaProto::WaitQueInsert(TFapRptMsg *pMsg)
{
	if (m_bRptMsgNum >= RPTWAITQUE_SIZE)
		return false;

	for (BYTE i=0; i<RPTWAITQUE_SIZE; i++)
	{
		if (m_pRptMsg[i].wLen == 0)
		{
			memcpy(&m_pRptMsg[i], pMsg, sizeof(TFapRptMsg));
			break;
		}
	}
	m_bRptMsgNum++;
	return true;
}

bool CFaProto::WaitQueGetMsg(BYTE bIdx,TFapRptMsg*pMsg )
{
	if (bIdx >= RPTWAITQUE_SIZE)//m_bRptMsgNum
		return false;
	
	//if (m_bRptMsgNum > RPTWAITQUE_SIZE)
	//{
	//	m_bRptMsgNum = 0;
	//	return false;
	//}
	if (m_pRptMsg[bIdx].wLen == 0)
		return false;
	
	memcpy(pMsg, &m_pRptMsg[bIdx], sizeof(TFapRptMsg));
	return true;
}

//上报确认帧处理
int CFaProto::Rpt_response(BYTE* pApdu, WORD wApduLen)
{
	TFapRptMsg pMsg;
	BYTE i;
	DWORD dwDataLen = 0;
	int iLenArea = 0;
	WORD wOffset;
	BYTE *pApdu0 = pApdu;


	//再判断PIID和OAD是否与我待确认区里的某帧相同，相同就认为该帧被确认，
	//被确认后的任务型上报帧就直接删除，对于事件上报帧还得找到全局变量PowerOffTmp里对应的
	//事件OAD且把该事件的上报状态置好，同时把它的存储标识置好。
	//if (WaitQueGetNum() == 0)
	//	return -1;
	//DTRACE(DB_FAPROTO, ("Recive Rpt_response  \r\n"));
	for(i=0; i<RPTWAITQUE_SIZE; i++)
	{
		pApdu = pApdu0;

		memset(&pMsg, 0, sizeof(TFapRptMsg));
		if (WaitQueGetMsg(i, &pMsg) == false)
			continue;
		wOffset = 9+pMsg.bBuf[4]+1;
		//DTRACE(DB_FAPROTO, ("Rpt Msg Num=%d  RevFrmwOffset=%d\r\n",i, wOffset));
		if (pMsg.bBuf[wOffset-1] == 0x10)//安全传输
		{
			//DTRACE(DB_FAPROTO, ("Rpt_response  0 \r\n"));
			iLenArea = DecodeLength(&pMsg.bBuf[wOffset+1], &dwDataLen);
			wOffset += 2+iLenArea;
		}
// 		DTRACE(DB_FAPROTO, ("上报确认帧iLenArea=%d  iLenArea=%d  wOffset=%d \r\n",iLenArea,iLenArea,wOffset));
// 		TraceBuf(DB_FAPROTO, "Apdu<--", pApdu, wApduLen);
// 		TraceBuf(DB_FAPROTO, "send-->", pMsg.bBuf, pMsg.wLen);

		BYTE bRecType = pApdu[1];
		BYTE bSendType = pMsg.bBuf[wOffset];
		
		DTRACE(DB_FAPROTO, ("Rpt_response: Compare type bRecType=%d, bSendType=%d.\n", bRecType, bSendType));
		if ((bRecType==bSendType))//类型是否相同
			//&&(pApdu[2] == pMsg.bBuf[wOffset+1]))//判断PIID是否相等
		{
			pApdu += 4;

			DWORD dwSendOAD, dwRecOAD;
			dwRecOAD = OoOadToDWord(pApdu);
			dwSendOAD = OoOadToDWord(&pMsg.bBuf[wOffset+3]);
			DTRACE(DB_FAPROTO, ("Rpt_response: Compare OAD dwRecOAD=0x%08x, dwSendOAD=0x%08x.\n", dwRecOAD, dwSendOAD));
			if (dwRecOAD == dwSendOAD)//判断OAD是否相等
			{
				DTRACE(DB_FAPROTO, ("Rpt_response  OK! \r\n"));
				//如果是事件测还要处理标志给黄与张那边
				if (pMsg.bClass == GB_DATACLASS3)
				{
					DWORD dwOAD;
					dwOAD = OoOadToDWord(&(pMsg.bBuf[wOffset+3]));
					DTRACE(DB_FAPROTO, ("Rpt_response  Event OK! \r\n"));
					
					/*
					pMsg.bSeq//事件发生阶段
					pMsg.bTask//该事件在表中的索引*/
					TEvtMsg EvnMsg;
					BYTE btStage = 0;
					//bit0-发生上报,bit1-发生确认,bit2-结束上报,bit3-结束确认
					if (pMsg.bSeq == 1)
						btStage = 2;
					else if (pMsg.bSeq == 2)
						btStage = 8;
					
					EvnMsg.bStage = pMsg.bSeq;
					EvnMsg.dwOAD = dwOAD;
					EvnMsg.wRecIdx = pMsg.bTask;
					UpdateEvtRptState(m_dwCnOAD, &EvnMsg, btStage);
					//根据上边三个参数来完成对事件那边的确认标识，具体接口要事件
				}
				WaitQueDelete(i);
				return 0;
			}
		}
	}
	return 0;
}

//上报安全传输处理
/* 把原来的上报数据帧组织为安全上报数据帧
 * 入参1:BYTE *pApdu 原来的上报APDU
 * 入参2:WORD wApduLen 原来的APDU长度
 * 出参1:BYTE *pSecureApdu 生成的安全传输APDU报文，需要一个比原APDU大32个字节的buf
 * 返回值: 为安全传输APDU报文的长度，
 *         若返回值<0，表示失败；
 *         若返回值=0，代表无须加密，直接使用原APDU。
 */
int CFaProto::Rpt_SecureLayer(BYTE *pApdu, WORD wApduLen, BYTE *pSecureApdu)
{
	int iDataLen;
	BYTE bBuf[16];
	BYTE *p;
	int iPos;
	
	if ((pApdu==NULL) || (pSecureApdu==NULL))
		return -1;
	if (*pApdu != REPORT_NOTI)
		return -1;

	iDataLen = ReadItemEx(BN0, PN0, 0xF112, bBuf);
	if ((iDataLen>0) && (bBuf[1]==0)) //不启用安全模式参数
	{
		return 0;
	}
	
	p = pSecureApdu;

	*p++ = SECURITY_REQ;
	*p++ = SecureData_Plaintext;

	iPos = EncodeLength(wApduLen, p);
	p += iPos;

	memcpy(p, pApdu, wApduLen);
	p += wApduLen;

	*p++ = AuthType_RNMAC;
	iDataLen = Esam_PlainDataCalRnMac(pApdu, wApduLen, m_RptSecureParam.bRn, bBuf);
	if (iDataLen < 0)
		return 0;

	iPos = EncodeLength(12, p);
	p += iPos;
	memcpy(p, m_RptSecureParam.bRn, 12);
	p += 12;

	iPos = EncodeLength(ESAM_MAC_LEN, p);
	p += iPos;
	memcpy(p, bBuf, ESAM_MAC_LEN);
	p += ESAM_MAC_LEN;

	return p - pSecureApdu;
	
}


int CFaProto::AppendEvtMsg(TEvtMsg* pEvtMsg)
{

	if ( m_pFaProPara->ProPara.fAutoSend == false)//本通道不具备上报功能测退出
		return -1;
	
	m_queEvt.Append((BYTE *)(pEvtMsg), sizeof(TEvtMsg), 1);
	return sizeof(TEvtMsg);
}

//描述：设置通道OAD
void CFaProto::SetCnOAD()
{
	if ( m_pFaProPara->wConnectType == CONNECTTYPE_ETH)
		m_dwCnOAD = 0x45100000;
	else if ( m_pFaProPara->wConnectType == CONNECTTYPE_GPRS)
		m_dwCnOAD = 0x45000000;
	else if ( m_pFaProPara->wConnectType == CONNECTTYPE_LOCAL)
		m_dwCnOAD = 0xF2020000;
	else //if (memcmp(pszName, "Test", 4) == 0)
		m_dwCnOAD = 0xF2010000;
	//else
	//	m_dwCnOAD = 0;
}

