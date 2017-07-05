

#include "stdafx.h"
#include "syscfg.h"
#include "FileTran.h"
#include "FaAPI.h"
#include <string.h>
#include <stdio.h>
#include "sysfs.h"
#include "DbOIAPI.h"

//extern int OoScanData(BYTE* pbSrc, BYTE* pbFmt, WORD wFmtLen, bool fRevOrder, int iIndex, WORD* pwLen, BYTE* pbType, BYTE** ppFieldFmt=NULL, WORD* pwFieldFmtLen=NULL);

//TFileBlkTrans g_FileBlkTrans;
TFileBlkTrans g_FileBlkTrans;         //文件分块传输控制结构

#define KANWU_170303		1//勘误20170303

//启动传输
#ifdef KANWU_170303
static BYTE g_bStartTransFmt[] = {
	DT_STRUCT,	//struct
	3,	//最大个数
		DT_STRUCT,	//struct
		6,	
			DT_VIS_STR, 100, RLV,
			DT_VIS_STR, 100, RLV,
			DT_DB_LONG_U,
			DT_BIT_STR, 3, RLF,
			DT_VIS_STR, 100, RLV,
			DT_ENUM,
		DT_LONG_U,	
		2,
		2,
			DT_ENUM,	//CSD
			DT_OCT_STR, 100, RLV,
};
#else
//勘误前的启动传输
static BYTE g_bStartTransFmt[] = {
	DT_STRUCT,	//struct
	3,	//最大个数
		DT_STRUCT,	//struct
		5,	
			DT_VIS_STR, 100, RLV,
			DT_VIS_STR, 100, RLV,
			DT_DB_LONG_U,
			DT_BIT_STR, 3, RLF,
			DT_VIS_STR, 100, RLV,
		DT_LONG_U,	
		2,
		2,
			DT_ENUM,	//CSD
			DT_OCT_STR, 100, RLV,
};
#endif

//文件下载
static BYTE g_bFileTransFmt[] = {
	DT_STRUCT,	//struct
	2,	//最大个数
		DT_LONG_U,	
		DT_OCT_STR, 100, RLV,
};
//文件信息
static BYTE g_bFileInfoFmt[] = {
	DT_STRUCT,	//struct
	5,	
		DT_VIS_STR, 100, RLV,
		DT_VIS_STR, 100, RLV,
		DT_DB_LONG_U,
		DT_BIT_STR, 3, RLF,
		DT_VIS_STR, 100, RLV,
};

bool IsAllABit(const BYTE* p, WORD len)
{
	for (WORD i=0; i<=len; i++)
	{
		if ((p[i/8] & (0x80 >> (i%8))) == 0)
			return false;
	}
	return true;
}

int GetFileTransAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wBufSize, int* piStart)
{
	BYTE *p = pbBuf;
	WORD i;
	int iLenArea = 0;
	DWORD dwDataLen = 0;
	
	if (bAttr == 0x02)//((wOI == 0xF000)||(wOI == 0xF001)) && 
	{
		if (!PartReadFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE *)&g_FileBlkTrans, sizeof(TFileBlkTrans)))
			return -1;
		*p ++ = DT_STRUCT;
#ifdef KANWU_170303
		*p ++ = 0x06;
#else
		*p ++ = 0x05;
#endif
		*p ++ = g_FileBlkTrans.bSrcFile[0];
		iLenArea = DecodeLength(&g_FileBlkTrans.bSrcFile[1], &dwDataLen);	
		memcpy(p, &g_FileBlkTrans.bSrcFile[1], dwDataLen+iLenArea);
		p += dwDataLen+iLenArea;
		dwDataLen = 0;
		iLenArea = 0;
		
		*p ++ = g_FileBlkTrans.bDstFile[0];
		iLenArea = DecodeLength(&g_FileBlkTrans.bDstFile[1], &dwDataLen);	
		memcpy(p, &g_FileBlkTrans.bDstFile[1], dwDataLen+iLenArea);
		p += dwDataLen+iLenArea;
		dwDataLen = 0;
		iLenArea = 0;

		*p ++ = DT_DB_LONG_U;
		OoDWordToDoubleLongUnsigned(g_FileBlkTrans.dwTotalLen, p);
		p += 4;
		
		*p ++ = DT_BIT_STR;
		*p ++ = 0x03;
		*p ++ = g_FileBlkTrans.bFileAttr;

		*p ++ = g_FileBlkTrans.bFileVer[0];
		iLenArea = DecodeLength(&g_FileBlkTrans.bFileVer[1], &dwDataLen);	
		memcpy(p, &g_FileBlkTrans.bFileVer[1], dwDataLen+iLenArea);
		p += dwDataLen+iLenArea;
		dwDataLen = 0;
		iLenArea = 0;

#ifdef KANWU_170303
		*p ++ = DT_ENUM;
		*p ++ = g_FileBlkTrans.bFileType;
#endif		
		if ((p - pbBuf) <= wBufSize)
			return p - pbBuf;
		else
			return -1;
	}
	
	if (wOI == 0xF000)
	{
		return -1;
	}
	else if (wOI == 0xF001 && bAttr == 0x04)
	{
		if (!PartReadFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE *)&g_FileBlkTrans, sizeof(TFileBlkTrans)))
			return -1;
		*p++ = DT_BIT_STR;
		BYTE y = EncodeLength(g_FileBlkTrans.wTotalBlks+1, p);
		p += y;
		WORD wBLen = (g_FileBlkTrans.wTotalBlks+1)/8;
		if ((g_FileBlkTrans.wTotalBlks+1)%8 != 0)
			wBLen ++;
		for (i=0; i<wBLen; i++)
			*p++ = g_FileBlkTrans.bBlkStatus[i];
		if ((p - pbBuf) <= wBufSize)
			return p - pbBuf;
		else
			return -1;
	}
	return -1;
}

int SetFileTransAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbPara)
{
	BYTE *p = pbPara;
	WORD i;
	int iLenArea = 0;
	DWORD dwDataLen = 0;
	int iScanLen = -1;

	if (bAttr == 0x02)//((wOI == 0xF000)||(wOI == 0xF001)) && 
	{
		iScanLen = OoScanData(pbPara, g_bFileInfoFmt, sizeof(g_bFileInfoFmt), false, -1, NULL, NULL);	
		if (iScanLen < 0)
			return -1;
		if (!PartReadFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE *)&g_FileBlkTrans, sizeof(TFileBlkTrans)))
			return -1;
		*p ++;
		*p ++;
		g_FileBlkTrans.bSrcFile[0] = *p ++;
		iLenArea = DecodeLength(p, &dwDataLen);	
		memcpy(&g_FileBlkTrans.bSrcFile[1], p, dwDataLen+iLenArea);
		p += dwDataLen+iLenArea;
		dwDataLen = 0;
		iLenArea = 0;
		
		g_FileBlkTrans.bDstFile[0] = *p ++;
		iLenArea = DecodeLength(p, &dwDataLen);	
		memcpy(&g_FileBlkTrans.bDstFile[1], p, dwDataLen+iLenArea);
		p += dwDataLen+iLenArea;
		dwDataLen = 0;
		iLenArea = 0;

		*p ++;
		g_FileBlkTrans.dwTotalLen = OoDoubleLongUnsignedToDWord(p);
		p += 4;
		
		*p ++;
		*p ++;
		g_FileBlkTrans.bFileAttr = *p ++;

		*p ++;
		g_FileBlkTrans.bFileVer[0] = *p ++;
		iLenArea = DecodeLength(p, &dwDataLen);	
		memcpy(&g_FileBlkTrans.bFileVer[1], p, dwDataLen+iLenArea);
		p += dwDataLen+iLenArea;
		//*p ++;
#ifdef KANWU_170303
		*p ++;
		g_FileBlkTrans.bFileType = *p ++;
#endif		
		dwDataLen = 0;
		iLenArea = 0;
		return p - pbPara;
	}
	return -1;
}


//文件分块传输管理方法的处理

//启动传输
//pbPara指向参数structure的格式字符,返回0为执行正常，-1为异常
int FileBlkTransMethod7(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iLenArea = 0;
	DWORD dwDataLen = 0;
	DWORD dwOff = 0;
	int iScanLen;
	WORD wLen;
	BYTE bType;
	DWORD dwOAD = 0xF0010700;

	if (pbPara == NULL)
		return -1;
	
	iScanLen = OoScanData(pbPara, g_bStartTransFmt, sizeof(g_bStartTransFmt), false, -1, &wLen, &bType);	
	if (iScanLen < 0)
		return -1;
	
	memset(&g_FileBlkTrans, 0, sizeof(TFileBlkTrans));
	//PartReadFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE *)&g_FileBlkTrans, sizeof(TFileBlkTrans));//test
	DeleteFile(USER_DATA_PATH"FileTrans.tmp");
	DeleteFile(USER_DATA_PATH"downinfo.dat");
#ifdef SYS_WIN
	DeleteFile(USER_DATA_PATH"clou.tgz");
#endif
	dwOff = 4;//指向源文件的格式字符，
	g_FileBlkTrans.bSrcFile[0] = pbPara[dwOff];//格式字
	dwOff ++;
	iLenArea = DecodeLength(pbPara+dwOff, &dwDataLen);	
	memcpy(&g_FileBlkTrans.bSrcFile[1], pbPara+dwOff, dwDataLen+iLenArea);
	
	dwOff += iLenArea+dwDataLen;
	iLenArea = 0;
	dwDataLen = 0;
	g_FileBlkTrans.bDstFile[0] = pbPara[dwOff];
	dwOff ++;
	iLenArea = DecodeLength(pbPara+dwOff, &dwDataLen);	
	memcpy(&g_FileBlkTrans.bDstFile[1], pbPara+dwOff, dwDataLen+iLenArea);

	dwOff += iLenArea+dwDataLen+1;
	iLenArea = 0;
	dwDataLen = 0;
	g_FileBlkTrans.dwTotalLen = OoDoubleLongUnsignedToDWord(pbPara+dwOff);

	dwOff += 4+1+1;
	g_FileBlkTrans.bFileAttr = pbPara[dwOff];
		
	dwOff += 1;
	g_FileBlkTrans.bFileVer[0] = pbPara[dwOff];
	dwOff ++;
	iLenArea = DecodeLength(pbPara+dwOff, &dwDataLen);	
	memcpy(&g_FileBlkTrans.bFileVer[1], pbPara+dwOff, dwDataLen+iLenArea);
	
	dwOff += 1+iLenArea+dwDataLen;
#ifdef KANWU_170303
	g_FileBlkTrans.bFileType =  pbPara[dwOff];

	dwOff += 1+1;
#endif	
	iLenArea = 0;
	dwDataLen = 0;
	g_FileBlkTrans.wBlkSize = OoLongUnsignedToWord(pbPara+dwOff);//传输块大小

	g_FileBlkTrans.wTotalBlks = g_FileBlkTrans.dwTotalLen/g_FileBlkTrans.wBlkSize;//算总块数,块序号从0~n-1
	if (g_FileBlkTrans.wTotalBlks >= 1)
		g_FileBlkTrans.wTotalBlks--;
	if ((g_FileBlkTrans.dwTotalLen%g_FileBlkTrans.wBlkSize) != 0)
		g_FileBlkTrans.wTotalBlks ++;

	//文件校验
	dwOff += 2 + 2+1;
	g_FileBlkTrans.bChkType = pbPara[dwOff];
	dwOff += 1 + 1;
	memcpy((BYTE *)&(g_FileBlkTrans.bChkVal), pbPara+dwOff, 2);
	PartWriteFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE*)&g_FileBlkTrans, sizeof(TFileBlkTrans));
	return 0;
		

}
//下载文件
int FileBlkTransMethod8(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	DWORD dwOff = 2;//指向块序号
	int iLenArea = 0;
	DWORD dwDataLen = 0;

	if (pbPara == NULL)
		return -1;
//OoScanData()里对于例如09的长度超过128进行了编码的情况没有考虑	
	//iScanLen = OoScanData(pbPara, g_bFileTransFmt, sizeof(g_bFileTransFmt), false, -1, &wLen, &bType);	
	//if (iScanLen < 0)
	//	return -1;
	
	if (pbPara[dwOff] != 0x12)
		return -1;//格式字不对

	dwOff++;
	WORD wCurBlockNum = OoOiToWord(pbPara+dwOff);//块序号从0~n-1
	
	//memset(&g_FileBlkTrans, 0, sizeof(TFileBlkTrans));
	PartReadFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE *)&g_FileBlkTrans, sizeof(TFileBlkTrans));
	if (wCurBlockNum > g_FileBlkTrans.wTotalBlks)
	{
		return -1;//块序号大于了总块数
	}
	dwOff += 2;
	if (pbPara[dwOff] != 0x09)
		return -1;//格式字不对
		
	dwOff++;
	if (pbPara[dwOff] != 0)//数据块长度
	{
		//g_FileBlkTrans.wLastSubBlock = wCurBlockNum;
		iLenArea = DecodeLength(pbPara+dwOff, &dwDataLen);	
		dwOff += iLenArea;
		if (dwDataLen > g_FileBlkTrans.wBlkSize)
			return -1;//下载的块长度大于了启动时约定的块长度
		//把数据写入文件
		PartWriteFile(USER_DATA_PATH"FileTrans.tmp", wCurBlockNum*g_FileBlkTrans.wBlkSize, pbPara+dwOff, dwDataLen);
		g_FileBlkTrans.dwTranLen += dwDataLen;

		//g_FileBlkTrans.bBlkStatus[wCurBlockNum/8] |= 0x01<<(wCurBlockNum%8);
		//DTRACE(DB_FAPROTO, ("SCN=%d\r\n",wCurBlockNum));
		g_FileBlkTrans.bBlkStatus[wCurBlockNum/8] |= 0x80>>(wCurBlockNum%8);
		if (IsAllABit(g_FileBlkTrans.bBlkStatus, g_FileBlkTrans.wTotalBlks))//状态位的各bit都为1则认为传输完成
		{
			DTRACE(DB_FAPROTO, ("FileTrans over!\r\n"));
			g_FileBlkTrans.wEndBlkSize  = dwDataLen;
			//g_FileBlkTrans.wEndBlockNum = wCurBlockNum;
			//DWORD dwTmpTotalLenthg = wCurBlockNum*g_FileBlkTrans.wBlkSize+dwDataLen;
			if (g_FileBlkTrans.dwTotalLen != g_FileBlkTrans.dwTranLen)//终端接收到的文件长度与启动时下发的总长度不一致
			{
				PartWriteFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE*)&g_FileBlkTrans, sizeof(TFileBlkTrans));
				DTRACE(DB_FAPROTO, ("TransFile TotalLen error dwTotalLen=%d  dwTranLen=%d\r\n",g_FileBlkTrans.dwTotalLen, g_FileBlkTrans.dwTranLen));
				return-1;
			}
			DTRACE(DB_FAPROTO, ("TransFile TotalLen OK\r\n"));
			PartWriteFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE*)&g_FileBlkTrans, sizeof(TFileBlkTrans));
			if(UpdFile()>0)
			{
				DTRACE(DB_FAPROTO, ("TransFile:UpdFile OK !!!\r\n"));
				//g_dwExtCmdClick = GetClick();
				//g_dwExtCmdFlg = FLG_HARD_RST;
			}
			
		}
	}
	PartWriteFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE*)&g_FileBlkTrans, sizeof(TFileBlkTrans));//tll 为了调试，先加上这一句
	return 0;
}
//读取文件(参数)
//目前只做支持一次读取一块数据，即使读取多块也只回一块的数据
int FileBlkTransMethod9(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD dwOff = 2;//指向块序号
	BYTE iLenArea = 0;
	int fDataLen = 0;
	BYTE bBuf[2048];
	WORD wDtaLen = 0;
	BYTE bLength;
	BYTE* pbRes0 = pbRes;
	
	if (pbPara[dwOff] != 0x12)
		return -1;//格式字不对
	dwOff++;
	PartReadFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE *)&g_FileBlkTrans, sizeof(TFileBlkTrans));
	WORD wCurBlockNum = OoOiToWord(pbPara+dwOff);//块序号从0~n-1
	if (wCurBlockNum > g_FileBlkTrans.wTotalBlks)
	{
		return -1;//读取的块不存在
	}
	*pbRes ++ = 0x02;
	*pbRes ++ = 0x01;
	*pbRes ++ = 0x09;
	if (wCurBlockNum == g_FileBlkTrans.wTotalBlks)
		fDataLen = g_FileBlkTrans.wEndBlkSize;
	else
		fDataLen = g_FileBlkTrans.wBlkSize;
	
	if (PartReadFile(USER_DATA_PATH"FileTrans.tmp", wCurBlockNum*g_FileBlkTrans.wBlkSize,bBuf, fDataLen) == false)
	{
		return -1;//读取的文件不存在
	}
	bLength = EncodeLength(fDataLen, pbRes );
	pbRes += bLength;
	memcpy(pbRes, bBuf, fDataLen);
	pbRes += fDataLen;

	fDataLen = pbRes-pbRes0;
	return fDataLen;
}

//软件比对（参数）
int FileBlkTransMethod10(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return 0;
}



int UpdFile()
{

	BYTE bFileHead[2048];
	WORD i = 0, j = 0, wCrc = 0;//配合2048最大可以达到127M大小
	memset(bFileHead, 0x00, sizeof(bFileHead));
	DTRACE(DB_FAPROTO, ("TransFile:Be in the Area!!!\r\n"));
#ifndef SYS_WIN		
	//if(g_fUpdateFirmware)//正在更新中
	//	return 0;
#endif			
	
	//PartReadFile(USER_DATA_PATH"clou.tgz", 0, bFileHead, sizeof(bFileHead));

#ifdef KANWU_170303
	if (g_FileBlkTrans.bFileType == 0)
#endif		
	{
		char szTmp[128];
		sprintf(szTmp, "cp %s /mnt/data/clmain.tar.xz", USER_DATA_PATH"FileTrans.tmp");
		system(szTmp);
#if 0
		j = g_FileBlkTrans.dwTotalLen / sizeof(bFileHead);
		for(i=0; i<j; i++)
		{
			memset(bFileHead, 0x00, sizeof(bFileHead));
			PartReadFile(USER_DATA_PATH"FileTrans.tmp", i*sizeof(bFileHead), bFileHead, sizeof(bFileHead));
			//PartWriteFile(USER_DATA_PATH"clou.tgz", i*sizeof(bFileHead), bFileHead, sizeof(bFileHead));
			PartWriteFile("/mnt/data/clmain.tgz", i*sizeof(bFileHead), bFileHead, sizeof(bFileHead));
			if (g_FileBlkTrans.bChkType == 0)//默认为0,即CRC16校验
				wCrc = get_crc_16( wCrc, bFileHead, sizeof(bFileHead));
		}
		j = g_FileBlkTrans.dwTotalLen % sizeof(bFileHead);
		if(j > 0)
		{
			memset(bFileHead, 0x00, sizeof(bFileHead));
			PartReadFile(USER_DATA_PATH"FileTrans.tmp", i*sizeof(bFileHead), bFileHead, j);
			//PartWriteFile(USER_DATA_PATH"clou.tgz", i*sizeof(bFileHead), bFileHead, sizeof(bFileHead));
			PartWriteFile("/mnt/data/clmain.tgz", i*sizeof(bFileHead), bFileHead, sizeof(bFileHead));
			if (g_FileBlkTrans.bChkType == 0)//默认为0,即CRC16校验
				wCrc = get_crc_16( wCrc, bFileHead, j);
		}
#endif
		if (g_FileBlkTrans.bChkVal[0] != 0)
		{
			WORD wCrcOld = OoOiToWord(&(g_FileBlkTrans.bChkVal[1]));
			if(wCrcOld != wCrc)
			{
				DTRACE(DB_FAPROTO, ("CFaProto::RxCmd_TransFile CRC Error\r\n"));
				//DeleteFile(g_FileBlkTrans->szFileTitle);
				//g_fUpdateFirmware = false; 
				return -1;			
			}
		}
		DTRACE(DB_FAPROTO, ("TransFile:CRC OK !!!\r\n"));
#ifndef SYS_WIN		
#if 1
//升级文件改为升clmain和\或驱动文件，再加上update脚本一起压缩的包

		sprintf(szTmp, "rm -fr /mnt/data/clmain/*");
		system(szTmp);

		//sprintf(szTmp, "tar zxvf /mnt/data/clmain.tgz -C /mnt/data/");//K32改用下边的新压缩方式
		sprintf(szTmp, "xz -d /mnt/data/clmain.tar.xz");
		system(szTmp);
		sprintf(szTmp, "tar xvf /mnt/data/clmain.tar -C /mnt/data/");
		system(szTmp);
		sprintf(szTmp, "source /mnt/data/clmain/update&");
		system(szTmp);
		return 1;
#else
		//if (g_RemoteDown.m_dwFileSize>=0)//其它文件
		//{
			
			//if((g_FileBlkTrans->bFileValid == 0x01) && strstr(g_FileBlkTrans->szFileTitle, ".tgz")!= NULL)	//Linux下升级，更新编程次数和最后一次编程时间
			{
				//g_fUpdateFirmware = true;//xzz add.
				Sleep(1000);
				/*if(strstr(g_FileBlkTrans->szFileTitle, "clmain.tgz")!= NULL)
				{
					
					DTRACE(DB_FAPROTO,  ("Linux Updated : started!Updated Parameter clmain\n"));
					char szTmp[128];

					sprintf(szTmp, "tar zxvf %s -C /", g_FileBlkTrans->szFileTitle);
					system(szTmp);

					memset(szTmp, 0x00, sizeof(szTmp));
					sprintf(szTmp, "tar zcvf /mnt/data/clou.tgz /clou");
					system(szTmp);

					memset(szTmp, 0x00, sizeof(szTmp));
					sprintf(szTmp, "cp  /mnt/data/clou.tgz /mnt/app/clou.tgz");
					system(szTmp);

					DeleteFile(g_FileBlkTrans->szFileTitle);
					//DeleteFile(USER_DATA_PATH"downinfo.dat");


				}
				else*/
				{
					DTRACE(DB_FAPROTO,  ("Linux Updated : started!Updated Parameter\n"));
					DTRACE(DB_FAPROTO,  ("Linux Updated : End!Updated Parameter\n"));
					char szTmp[128];
					sprintf(szTmp, "cp %s /mnt/app/clou.tgz", USER_DATA_PATH"clou.tgz");
					system(szTmp);
					DeleteFile(USER_DATA_PATH"clou.tgz");
					//DeleteFile(USER_DATA_PATH"downinfo.dat");//这个文件先不删，可能主站还会有读取
					return 1;
					//还要处理终端复位等动作
				}
			}
			/*else
			{
				DTRACE(DB_FAPROTO,  ("Linux Updated : It isn't Updated file\n"));
				DeleteFile(USER_DATA_PATH"downinfo.dat");
				g_fUpdateFirmware = false; 
				return 0;

			}*/
			
		//}	
#endif		
#else
		//DeleteFile(USER_DATA_PATH"clou.tgz");
		return 1;
#endif
	}
	//else if(g_FileBlkTrans.bFileType == 3)

}




//文件扩展传输管理方法的处理
int FileExtTransmit(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{

	if (bMethod == 0x07)//从服务器下载（文件信息，协议类型）
	{
	}
	else if (bMethod == 0x08)//上传到服务器（文件信息，协议类型）
	{
	}
	return 0;
}



//==========================以下代码是黄工的

#if 0
bool GetFileInfoFromAtr2(TFileInfo *stFileInfo)
{
    
    // 从系统库取数据
    // 假设stFileInfoFromAt2 是取出来的信息
    TFileInfo stFileInfoFromAt2;    

    // 获取信息到stFileInfoFromAt2

    #if 0
    DWORD dwDataLen = 0;
    BYTE *pdData = NULL;
    //获取源文件路径
    pdData = GetDataBuffInfo(dwDataLen, stFileInfoFromAt2.cPathNameSrc);
    memcpy(stFileInfo->cPathNameSrc, pdData, dwDataLen);
    stFileInfo->cPathNameSrc[dwDataLen] = '\0';

    //获取目标文件路径
    pdData = GetDataBuffInfo(dwDataLen, stFileInfoFromAt2.cPathNameDst);
    memcpy(stFileInfo->cPathNameDst, pdData, dwDataLen);
    stFileInfo->cPathNameDst[dwDataLen] = '\0';

    // 获取文件大小    
    stFileInfo->dwFileSize =  stFileInfoFromAt2.dwFileSize;

    //获取文件属性
    stFileInfo->bFileAtr = stFileInfoFromAt2.bFileAtr;

    // 获取文件版本
    pdData = GetDataBuffInfo(dwDataLen, stFileInfoFromAt2.cFileVersion);
    memcpy(stFileInfo->cFileVersion, pdData, dwDataLen);
    stFileInfo->cFileVersion[dwDataLen] = '\0';   
    #endif
    *stFileInfo = stFileInfoFromAt2;
    return true;
    
}

/*
*文件传输 -- 方法7 启动分块传输
*@ stFileTranStartPara 
参数∷=structure
{
  文件信息    structure，
  传输块大小  long-unsigned，
  校验        structure
}
*/
bool OoFileTranBlockStart(TFileTranStartPara stFileTranStartPara)
{
    // stTFileTranStartPara 要做入库处理

    // 要传输的总块数
    DWORD dwBlockTotal;
    if(stFileTranStartPara.stFileInfo.dwFileSize%stFileTranStartPara.wBlkSize!=0)
    {
        dwBlockTotal = stFileTranStartPara.stFileInfo.dwFileSize/stFileTranStartPara.wBlkSize+1;
    }
    else{
        dwBlockTotal = stFileTranStartPara.stFileInfo.dwFileSize/stFileTranStartPara.wBlkSize;
    }
    // 清传输块记录状态字
    /*
    读取
    清零 
    写入
    */
    // 清除缓存文件
}

/*
方法8：写文件
参数∷=structure
{
   块序号  long-unsigned，
   块数据  octet-string
}
*/
bool OoFileTranBlockWrite(TBlockInfo stBlockInfo)
{
    DWORD dwOffset = 0;
    DWORD dwBlockTotal;// 获取总块数的值
    DWORD dwBlkSize;
    int iRet;
    bool fIsSuccess;
    DWORD dwDataLen=0;
    TFileTranStartPara stFileTranStartPara;

    dwOffset = dwBlkSize*stBlockInfo.wBlockSN;
    if(stBlockInfo.wBlockSN <dwBlockTotal-1)
    {        
        dwDataLen = dwBlkSize;
    }
    else  if(stBlockInfo.wBlockSN <dwBlockTotal-1)
    {
        dwDataLen = stFileTranStartPara.stFileInfo.dwFileSize%stFileTranStartPara.wBlkSize;
    }

    iRet = WriteFile(stFileTranStartPara.stFileInfo.cPathNameDst, dwOffset, stBlockInfo.pbBlockData);
    if(fIsSuccess)
    {
        //更新属性4 传输块状态字
        //读取
        //跟新   buf[stBlockInfo.wBlockSN>>3] = buf[stBlockInfo.wBlockSN>>3]|(1<<(stBlockInfo.wBlockSN&0x07));        
        //写入
        // 检测传输块是否完成
        return true;
    }
    else{
        return false;
    }    
}


//bool OoFileTranBlockRead(TBlockInfo *pstBlockInfo);

bool OoFileTranBlockRead(WORD wBlockSN)
{
    DWORD dwOffset = 0;
    DWORD dwBlockTotal;// 获取总块数的值
    DWORD dwBlkSize;
    int iRet;
    bool fIsSuccess;
    DWORD dwDataLen=0;
    TFileTranStartPara stFileTranStartPara;
    TBlockInfo *stBlockInfo; // 暂且认为时形参
    BYTE bDataHeadNum = 0;

    {
        // 通过接口取相关参数
    }
    dwOffset = dwBlkSize*wBlockSN;
    if(wBlockSN <dwBlockTotal-1)
    {        
        dwDataLen = dwBlkSize;
    }
    else  if(wBlockSN == dwBlockTotal-1)
    {
        dwDataLen = stFileTranStartPara.stFileInfo.dwFileSize%stFileTranStartPara.wBlkSize;
    }

    stBlockInfo->pbBlockData[0] = 0x82;
    stBlockInfo->pbBlockData[1] = (BYTE)((dwDataLen>>8)&0xff);
    stBlockInfo->pbBlockData[2] = (BYTE)(dwDataLen&0xff); 

    fIsSuccess = ReadFile(stFileTranStartPara.stFileInfo.cPathNameDst, dwOffset,stBlockInfo.pbBlockData+3, dwDataLen);

    if(fIsSuccess)
    {
        return true;
    }
    else{
        return false;
    }    
}
#endif



