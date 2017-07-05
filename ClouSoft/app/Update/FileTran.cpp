

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
TFileBlkTrans g_FileBlkTrans;         //�ļ��ֿ鴫����ƽṹ

#define KANWU_170303		1//����20170303

//��������
#ifdef KANWU_170303
static BYTE g_bStartTransFmt[] = {
	DT_STRUCT,	//struct
	3,	//������
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
//����ǰ����������
static BYTE g_bStartTransFmt[] = {
	DT_STRUCT,	//struct
	3,	//������
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

//�ļ�����
static BYTE g_bFileTransFmt[] = {
	DT_STRUCT,	//struct
	2,	//������
		DT_LONG_U,	
		DT_OCT_STR, 100, RLV,
};
//�ļ���Ϣ
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


//�ļ��ֿ鴫��������Ĵ���

//��������
//pbParaָ�����structure�ĸ�ʽ�ַ�,����0Ϊִ��������-1Ϊ�쳣
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
	dwOff = 4;//ָ��Դ�ļ��ĸ�ʽ�ַ���
	g_FileBlkTrans.bSrcFile[0] = pbPara[dwOff];//��ʽ��
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
	g_FileBlkTrans.wBlkSize = OoLongUnsignedToWord(pbPara+dwOff);//������С

	g_FileBlkTrans.wTotalBlks = g_FileBlkTrans.dwTotalLen/g_FileBlkTrans.wBlkSize;//���ܿ���,����Ŵ�0~n-1
	if (g_FileBlkTrans.wTotalBlks >= 1)
		g_FileBlkTrans.wTotalBlks--;
	if ((g_FileBlkTrans.dwTotalLen%g_FileBlkTrans.wBlkSize) != 0)
		g_FileBlkTrans.wTotalBlks ++;

	//�ļ�У��
	dwOff += 2 + 2+1;
	g_FileBlkTrans.bChkType = pbPara[dwOff];
	dwOff += 1 + 1;
	memcpy((BYTE *)&(g_FileBlkTrans.bChkVal), pbPara+dwOff, 2);
	PartWriteFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE*)&g_FileBlkTrans, sizeof(TFileBlkTrans));
	return 0;
		

}
//�����ļ�
int FileBlkTransMethod8(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	DWORD dwOff = 2;//ָ������
	int iLenArea = 0;
	DWORD dwDataLen = 0;

	if (pbPara == NULL)
		return -1;
//OoScanData()���������09�ĳ��ȳ���128�����˱�������û�п���	
	//iScanLen = OoScanData(pbPara, g_bFileTransFmt, sizeof(g_bFileTransFmt), false, -1, &wLen, &bType);	
	//if (iScanLen < 0)
	//	return -1;
	
	if (pbPara[dwOff] != 0x12)
		return -1;//��ʽ�ֲ���

	dwOff++;
	WORD wCurBlockNum = OoOiToWord(pbPara+dwOff);//����Ŵ�0~n-1
	
	//memset(&g_FileBlkTrans, 0, sizeof(TFileBlkTrans));
	PartReadFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE *)&g_FileBlkTrans, sizeof(TFileBlkTrans));
	if (wCurBlockNum > g_FileBlkTrans.wTotalBlks)
	{
		return -1;//����Ŵ������ܿ���
	}
	dwOff += 2;
	if (pbPara[dwOff] != 0x09)
		return -1;//��ʽ�ֲ���
		
	dwOff++;
	if (pbPara[dwOff] != 0)//���ݿ鳤��
	{
		//g_FileBlkTrans.wLastSubBlock = wCurBlockNum;
		iLenArea = DecodeLength(pbPara+dwOff, &dwDataLen);	
		dwOff += iLenArea;
		if (dwDataLen > g_FileBlkTrans.wBlkSize)
			return -1;//���صĿ鳤�ȴ���������ʱԼ���Ŀ鳤��
		//������д���ļ�
		PartWriteFile(USER_DATA_PATH"FileTrans.tmp", wCurBlockNum*g_FileBlkTrans.wBlkSize, pbPara+dwOff, dwDataLen);
		g_FileBlkTrans.dwTranLen += dwDataLen;

		//g_FileBlkTrans.bBlkStatus[wCurBlockNum/8] |= 0x01<<(wCurBlockNum%8);
		//DTRACE(DB_FAPROTO, ("SCN=%d\r\n",wCurBlockNum));
		g_FileBlkTrans.bBlkStatus[wCurBlockNum/8] |= 0x80>>(wCurBlockNum%8);
		if (IsAllABit(g_FileBlkTrans.bBlkStatus, g_FileBlkTrans.wTotalBlks))//״̬λ�ĸ�bit��Ϊ1����Ϊ�������
		{
			DTRACE(DB_FAPROTO, ("FileTrans over!\r\n"));
			g_FileBlkTrans.wEndBlkSize  = dwDataLen;
			//g_FileBlkTrans.wEndBlockNum = wCurBlockNum;
			//DWORD dwTmpTotalLenthg = wCurBlockNum*g_FileBlkTrans.wBlkSize+dwDataLen;
			if (g_FileBlkTrans.dwTotalLen != g_FileBlkTrans.dwTranLen)//�ն˽��յ����ļ�����������ʱ�·����ܳ��Ȳ�һ��
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
	PartWriteFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE*)&g_FileBlkTrans, sizeof(TFileBlkTrans));//tll Ϊ�˵��ԣ��ȼ�����һ��
	return 0;
}
//��ȡ�ļ�(����)
//Ŀǰֻ��֧��һ�ζ�ȡһ�����ݣ���ʹ��ȡ���Ҳֻ��һ�������
int FileBlkTransMethod9(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD dwOff = 2;//ָ������
	BYTE iLenArea = 0;
	int fDataLen = 0;
	BYTE bBuf[2048];
	WORD wDtaLen = 0;
	BYTE bLength;
	BYTE* pbRes0 = pbRes;
	
	if (pbPara[dwOff] != 0x12)
		return -1;//��ʽ�ֲ���
	dwOff++;
	PartReadFile(USER_DATA_PATH"downinfo.dat", 0, (BYTE *)&g_FileBlkTrans, sizeof(TFileBlkTrans));
	WORD wCurBlockNum = OoOiToWord(pbPara+dwOff);//����Ŵ�0~n-1
	if (wCurBlockNum > g_FileBlkTrans.wTotalBlks)
	{
		return -1;//��ȡ�Ŀ鲻����
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
		return -1;//��ȡ���ļ�������
	}
	bLength = EncodeLength(fDataLen, pbRes );
	pbRes += bLength;
	memcpy(pbRes, bBuf, fDataLen);
	pbRes += fDataLen;

	fDataLen = pbRes-pbRes0;
	return fDataLen;
}

//����ȶԣ�������
int FileBlkTransMethod10(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return 0;
}



int UpdFile()
{

	BYTE bFileHead[2048];
	WORD i = 0, j = 0, wCrc = 0;//���2048�����Դﵽ127M��С
	memset(bFileHead, 0x00, sizeof(bFileHead));
	DTRACE(DB_FAPROTO, ("TransFile:Be in the Area!!!\r\n"));
#ifndef SYS_WIN		
	//if(g_fUpdateFirmware)//���ڸ�����
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
			if (g_FileBlkTrans.bChkType == 0)//Ĭ��Ϊ0,��CRC16У��
				wCrc = get_crc_16( wCrc, bFileHead, sizeof(bFileHead));
		}
		j = g_FileBlkTrans.dwTotalLen % sizeof(bFileHead);
		if(j > 0)
		{
			memset(bFileHead, 0x00, sizeof(bFileHead));
			PartReadFile(USER_DATA_PATH"FileTrans.tmp", i*sizeof(bFileHead), bFileHead, j);
			//PartWriteFile(USER_DATA_PATH"clou.tgz", i*sizeof(bFileHead), bFileHead, sizeof(bFileHead));
			PartWriteFile("/mnt/data/clmain.tgz", i*sizeof(bFileHead), bFileHead, sizeof(bFileHead));
			if (g_FileBlkTrans.bChkType == 0)//Ĭ��Ϊ0,��CRC16У��
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
//�����ļ���Ϊ��clmain��\�������ļ����ټ���update�ű�һ��ѹ���İ�

		sprintf(szTmp, "rm -fr /mnt/data/clmain/*");
		system(szTmp);

		//sprintf(szTmp, "tar zxvf /mnt/data/clmain.tgz -C /mnt/data/");//K32�����±ߵ���ѹ����ʽ
		sprintf(szTmp, "xz -d /mnt/data/clmain.tar.xz");
		system(szTmp);
		sprintf(szTmp, "tar xvf /mnt/data/clmain.tar -C /mnt/data/");
		system(szTmp);
		sprintf(szTmp, "source /mnt/data/clmain/update&");
		system(szTmp);
		return 1;
#else
		//if (g_RemoteDown.m_dwFileSize>=0)//�����ļ�
		//{
			
			//if((g_FileBlkTrans->bFileValid == 0x01) && strstr(g_FileBlkTrans->szFileTitle, ".tgz")!= NULL)	//Linux�����������±�̴��������һ�α��ʱ��
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
					//DeleteFile(USER_DATA_PATH"downinfo.dat");//����ļ��Ȳ�ɾ��������վ�����ж�ȡ
					return 1;
					//��Ҫ�����ն˸�λ�ȶ���
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




//�ļ���չ����������Ĵ���
int FileExtTransmit(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{

	if (bMethod == 0x07)//�ӷ��������أ��ļ���Ϣ��Э�����ͣ�
	{
	}
	else if (bMethod == 0x08)//�ϴ������������ļ���Ϣ��Э�����ͣ�
	{
	}
	return 0;
}



//==========================���´����ǻƹ���

#if 0
bool GetFileInfoFromAtr2(TFileInfo *stFileInfo)
{
    
    // ��ϵͳ��ȡ����
    // ����stFileInfoFromAt2 ��ȡ��������Ϣ
    TFileInfo stFileInfoFromAt2;    

    // ��ȡ��Ϣ��stFileInfoFromAt2

    #if 0
    DWORD dwDataLen = 0;
    BYTE *pdData = NULL;
    //��ȡԴ�ļ�·��
    pdData = GetDataBuffInfo(dwDataLen, stFileInfoFromAt2.cPathNameSrc);
    memcpy(stFileInfo->cPathNameSrc, pdData, dwDataLen);
    stFileInfo->cPathNameSrc[dwDataLen] = '\0';

    //��ȡĿ���ļ�·��
    pdData = GetDataBuffInfo(dwDataLen, stFileInfoFromAt2.cPathNameDst);
    memcpy(stFileInfo->cPathNameDst, pdData, dwDataLen);
    stFileInfo->cPathNameDst[dwDataLen] = '\0';

    // ��ȡ�ļ���С    
    stFileInfo->dwFileSize =  stFileInfoFromAt2.dwFileSize;

    //��ȡ�ļ�����
    stFileInfo->bFileAtr = stFileInfoFromAt2.bFileAtr;

    // ��ȡ�ļ��汾
    pdData = GetDataBuffInfo(dwDataLen, stFileInfoFromAt2.cFileVersion);
    memcpy(stFileInfo->cFileVersion, pdData, dwDataLen);
    stFileInfo->cFileVersion[dwDataLen] = '\0';   
    #endif
    *stFileInfo = stFileInfoFromAt2;
    return true;
    
}

/*
*�ļ����� -- ����7 �����ֿ鴫��
*@ stFileTranStartPara 
������=structure
{
  �ļ���Ϣ    structure��
  ������С  long-unsigned��
  У��        structure
}
*/
bool OoFileTranBlockStart(TFileTranStartPara stFileTranStartPara)
{
    // stTFileTranStartPara Ҫ����⴦��

    // Ҫ������ܿ���
    DWORD dwBlockTotal;
    if(stFileTranStartPara.stFileInfo.dwFileSize%stFileTranStartPara.wBlkSize!=0)
    {
        dwBlockTotal = stFileTranStartPara.stFileInfo.dwFileSize/stFileTranStartPara.wBlkSize+1;
    }
    else{
        dwBlockTotal = stFileTranStartPara.stFileInfo.dwFileSize/stFileTranStartPara.wBlkSize;
    }
    // �崫����¼״̬��
    /*
    ��ȡ
    ���� 
    д��
    */
    // ��������ļ�
}

/*
����8��д�ļ�
������=structure
{
   �����  long-unsigned��
   ������  octet-string
}
*/
bool OoFileTranBlockWrite(TBlockInfo stBlockInfo)
{
    DWORD dwOffset = 0;
    DWORD dwBlockTotal;// ��ȡ�ܿ�����ֵ
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
        //��������4 �����״̬��
        //��ȡ
        //����   buf[stBlockInfo.wBlockSN>>3] = buf[stBlockInfo.wBlockSN>>3]|(1<<(stBlockInfo.wBlockSN&0x07));        
        //д��
        // ��⴫����Ƿ����
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
    DWORD dwBlockTotal;// ��ȡ�ܿ�����ֵ
    DWORD dwBlkSize;
    int iRet;
    bool fIsSuccess;
    DWORD dwDataLen=0;
    TFileTranStartPara stFileTranStartPara;
    TBlockInfo *stBlockInfo; // ������Ϊʱ�β�
    BYTE bDataHeadNum = 0;

    {
        // ͨ���ӿ�ȡ��ز���
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



