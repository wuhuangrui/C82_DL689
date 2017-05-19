#include "syscfg.h"
#ifdef SYS_VDK
#ifndef REMOTEDOWN_H
#define REMOTEDOWN_H

#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5
#define TFTP_ECHO	6
#define TFTP_ERR_NODEFINE	0
#define TFTP_ERR_NOFILE		1
#define TFTP_ERR_DESTORY	2
#define TFTP_ERR_NOSPACE	3
#define TFTP_ERR_FFCZ		4
#define TFTP_ERR_NOPORT		5
#define TFTP_ERR_FILEEX		6
#define TFTP_ERR_NOUSER		7
#define FILE_SIZE 			1024000
#define RX_SIZE				1024
#define TX_SIZE				128
#define PATH_MAX_SIZE    512

#include "Comm.h"
#include "stdio.h"


class CRemoteDown
{

public:
	CRemoteDown();		
    virtual ~CRemoteDown();
    
	short requestTry;			
	void Run();
	DWORD	m_dwFileSize;
	BYTE*  	m_bFileBuf;
	char  	m_szPathName[PATH_MAX_SIZE+1];	
	void    DoUpdate();	
	
};

extern int iFlash_Write(unsigned char *Src,unsigned int Size,unsigned long Dst);
extern long LzariUncompress(BYTE *src, long uzLen,unsigned long flashOffset);

extern int UpdateThread(void* pvPara);
extern CRemoteDown g_RemoteDown;
extern bool InitFaProto();
extern bool g_fUpdateFirmware;
#endif //REMOTEDOWN_H
#endif
