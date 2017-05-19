
#include "ComConst.h"
#include "ComStruct.h"
#include "apptypedef.h"
#include "Info.h"
#include <string.h>


#ifndef FILETRAN_H
#define FILETRAN_H

#define BLK_STATUS_SIZE		1024

typedef struct{
	BYTE		bSrcFile[128];		//源文件    visible-string,文件路径及名称
	BYTE 	bDstFile[128];		//目标文件  visible-string,文件路径及名称
	DWORD  	dwTotalLen;			//文件总长度
	BYTE    	bFileAttr;          //文件的属性
	BYTE  	bFileVer[64];    	//文件版本信息
	BYTE		bFileType;		
	//文件类型是enum型0-终端文件，1-本地通信模块文件，2-远程通信模块文件，3-采集器文件，4-从节点通信模块文件

	WORD    	wBlkSize;		//传输块大小
	BYTE		bChkType;		//校验类型
	BYTE 	bChkVal[32];	//校验值  octet-string,(按长度的编码方式来)

	BYTE		bBlkStatus[BLK_STATUS_SIZE];	//传输块状态字
	WORD 	wTotalBlks;					//总块数,值为n-1,
	WORD    	wEndBlkSize;					//最后一个块的数据大小
	DWORD    dwTranLen;					//记录已传输的文件长度
}TFileBlkTrans;          //文件分块传输控制结构




int FileBlkTransMethod7(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int FileBlkTransMethod8(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int FileBlkTransMethod9(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int FileBlkTransMethod10(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int FileExtTransmit(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int UpdFile();
int GetFileTransAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wBufSize, int* piStart);
int SetFileTransAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbPara);


//=====================以下是王的写的，目前没有用上
#if 0

/* 属性2
文件信息∷=structure
{
源文件    visible-string，
目标文件  visible-string，
文件大小  double-long-unsigned，
文件属性  bit-string(SIZE(3))，
文件版本  visible-string
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
参数∷=structure
{
校验类型  enum
{
    CRC校验（默认）	（0），
md5校验			（1），
SHA1校验			（2），
其他				（255）
}，
  校验值  octet-string
}
*/
typedef struct{
	BYTE bCheckType;
	BYTE bCheckValue[256];
}TCheckInfo;

/*
参数∷=structure
{
文件信息    structure，
传输块大小  long-unsigned，
校验        structure
}
*/
typedef struct{
	TFileInfo stFileInfo;
	WORD wBlockSize;
	TCheckInfo stCheckInfo;
}TFileTranStartPara;

/*
方法8：写文件（参数）
参数∷=structure
{
   块序号  long-unsigned，
   块数据  octet-string
}
*/
typedef struct{
    WORD wBlockSN;
	BYTE *pbBlockData;
}TBlockInfo;
#endif
	
#endif

