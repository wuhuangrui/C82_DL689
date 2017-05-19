
#include "ComConst.h"
#include "ComStruct.h"
#include "apptypedef.h"
#include "Info.h"
#include <string.h>


#ifndef FILETRAN_H
#define FILETRAN_H

#define BLK_STATUS_SIZE		1024

typedef struct{
	BYTE		bSrcFile[128];		//Դ�ļ�    visible-string,�ļ�·��������
	BYTE 	bDstFile[128];		//Ŀ���ļ�  visible-string,�ļ�·��������
	DWORD  	dwTotalLen;			//�ļ��ܳ���
	BYTE    	bFileAttr;          //�ļ�������
	BYTE  	bFileVer[64];    	//�ļ��汾��Ϣ
	BYTE		bFileType;		
	//�ļ�������enum��0-�ն��ļ���1-����ͨ��ģ���ļ���2-Զ��ͨ��ģ���ļ���3-�ɼ����ļ���4-�ӽڵ�ͨ��ģ���ļ�

	WORD    	wBlkSize;		//������С
	BYTE		bChkType;		//У������
	BYTE 	bChkVal[32];	//У��ֵ  octet-string,(�����ȵı��뷽ʽ��)

	BYTE		bBlkStatus[BLK_STATUS_SIZE];	//�����״̬��
	WORD 	wTotalBlks;					//�ܿ���,ֵΪn-1,
	WORD    	wEndBlkSize;					//���һ��������ݴ�С
	DWORD    dwTranLen;					//��¼�Ѵ�����ļ�����
}TFileBlkTrans;          //�ļ��ֿ鴫����ƽṹ




int FileBlkTransMethod7(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int FileBlkTransMethod8(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int FileBlkTransMethod9(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int FileBlkTransMethod10(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int FileExtTransmit(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int UpdFile();
int GetFileTransAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wBufSize, int* piStart);
int SetFileTransAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbPara);


//=====================����������д�ģ�Ŀǰû������
#if 0

/* ����2
�ļ���Ϣ��=structure
{
Դ�ļ�    visible-string��
Ŀ���ļ�  visible-string��
�ļ���С  double-long-unsigned��
�ļ�����  bit-string(SIZE(3))��
�ļ��汾  visible-string
}
*/
typedef struct{  
	char cPathNameSrc[256];  
	char cPathNameDst[256];  
	DWORD dwFileSize;  
	BYTE bFileAtr;
	char cFileVersion[256];
}TFileInfo;

/*
������=structure
{
У������  enum
{
    CRCУ�飨Ĭ�ϣ�	��0����
md5У��			��1����
SHA1У��			��2����
����				��255��
}��
  У��ֵ  octet-string
}
*/
typedef struct{
	BYTE bCheckType;
	BYTE bCheckValue[256];
}TCheckInfo;

/*
������=structure
{
�ļ���Ϣ    structure��
������С  long-unsigned��
У��        structure
}
*/
typedef struct{
	TFileInfo stFileInfo;
	WORD wBlockSize;
	TCheckInfo stCheckInfo;
}TFileTranStartPara;

/*
����8��д�ļ���������
������=structure
{
   �����  long-unsigned��
   ������  octet-string
}
*/
typedef struct{
    WORD wBlockSN;
	BYTE *pbBlockData;
}TBlockInfo;
#endif
	
#endif

