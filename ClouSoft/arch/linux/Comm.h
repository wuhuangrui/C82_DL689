// Comm.h: interface for the CComm class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __CCOMM_H__
#define __CCOMM_H__
//#include "Uart.h"

#include <termios.h>
#include <unistd.h>
#include "sysarch.h"
#include "apptypedef.h"

#define BR_NUM      12
#define PARITY_NUM  3

//extern char* g_pszBaudRateStrTab[BR_NUM];
//extern char* g_pszParityStrTab[PARITY_NUM];
#define CBR_300		  B300
#define CBR_600       B600
#define CBR_1200      B1200
#define CBR_2400      B2400
#define CBR_4800      B4800
#define CBR_9600      B9600
//#define CBR_14400     B14400 
#define CBR_19200     B19200  	
#define CBR_38400     B38400   
#define CBR_56000     B56000 
#define CBR_57600     B57600 
#define CBR_115200    B115200 

#define NOPARITY     0
#define ODDPARITY    1
#define EVENPARITY   2

#define ONESTOPBIT     1
#define TWOSTOPBITS    2
#define	ONE5STOPBITS   TWOSTOPBITS 

typedef struct {	
	WORD wPort; 
	DWORD dwBaudRate; 
	BYTE bByteSize; 
	BYTE bStopBits; 
	BYTE bParity;
}TCommPara; //串口配置 

class CComm
{
public:
	CComm();
	virtual ~CComm();

private:
	WORD  	m_wPort;		//串口号//CSIO* m_pCSIO;
	DWORD 	m_dwTimeouts;
	DWORD 	m_dwBaudRate; 
	BYTE  	m_bByteSize;
	BYTE  	m_bStopBits;
	BYTE  	m_bParity;
	bool  	m_fOpen;
	
	int    Comm_fd;		//打开串口的文件描述符
	struct termios TTYstate;
	TSem   g_semRead;
	//TSem   g_semWrite; //test mask

	int  OpenDev(char *Dev);
public:
	void Config(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);
	void Config(TCommPara &CommPara);
	bool GetCommPara(TCommPara* pCommPara);
	TSem   g_semWrite; //test add
	bool Open(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,	BYTE bStopBits, BYTE bParity);
  	bool Open(TCommPara& CommPara) { return Open(CommPara.wPort, CommPara.dwBaudRate, CommPara.bByteSize, CommPara.bStopBits, CommPara.bParity); };	
  	bool Open();
  	
  	bool Close();
  	bool SetComm(DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);
	bool SetBaudRate(DWORD dwBaudRate);
	int GetCommFd();
	//void clear_serial_buffer();
	
  	bool  IsOpen() { return m_fOpen; };	
	DWORD ReadFunc(unsigned char *buf, INT16U len, DWORD dwTimeouts);
	DWORD ReadComm(void* pvBuf, DWORD dwLength) { return ReadFunc((INT8U *)pvBuf,(INT16U )dwLength, m_dwTimeouts); };
	DWORD ReadComm(void* pvBuf, DWORD dwLength, DWORD dwTimeouts) {return ReadFunc((INT8U *)pvBuf,(INT16U )dwLength, dwTimeouts); };
	
	DWORD Read(void* pvBuf, DWORD dwLength) { return ReadComm(pvBuf, dwLength);};
	DWORD Read(void* pvBuf, DWORD dwLength, DWORD dwTimeouts) { return ReadComm(pvBuf, dwLength, dwTimeouts);};
	
	DWORD WriteFunc(unsigned char *buf, int Txlen);
	DWORD Write(void* pvBuf, WORD wLength) { return WriteFunc((INT8U *)pvBuf, wLength); };
											//{ return m_pCSIO->Write((INT8U *)pvBuf, wLength, 1000); };
	
	bool SetDTR(){  return true;};
	bool ClrDTR(){  return true;};
	bool GetRI(void) { return false;};
	//bool GetDCD(void) { return ((COutUART* )m_pCSIO)->GetCDHolding();};
	void SetTimeouts(DWORD dwTimeouts) { m_dwTimeouts = dwTimeouts;};
	
	void CreateSemaphore(void);
	void DeleteSemaphore(void);
	
};


#endif // CCOMM_H


