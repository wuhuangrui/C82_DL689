// Comm.h: interface for the CComm class.
//
//////////////////////////////////////////////////////////////////////
#ifndef CCOMM_H
#define CCOMM_H
#include "Uart.h"

#define BR_NUM      12
#define PARITY_NUM  3

//extern char* g_pszBaudRateStrTab[BR_NUM];
//extern char* g_pszParityStrTab[PARITY_NUM];
#define CBR_300		  300
#define CBR_600       600
#define CBR_1200      1200
#define CBR_2400      2400
#define CBR_4800      4800
#define CBR_9600      9600
#define CBR_14400     14400 
#define CBR_19200     19200  	
#define CBR_38400     38400   
#define CBR_56000     56000 
#define CBR_57600     57600 
#define CBR_115200    115200 

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
}TCommPara; //¥Æø⁄≈‰÷√ 

class CComm
{
public:
	CComm();
	virtual ~CComm();

private:
	CSIO* m_pCSIO;
	//CSport* m_pCSport;
	WORD  m_wPort;
	DWORD m_dwBaudRate; 
	BYTE  m_bByteSize;
	BYTE  m_bStopBits;
	BYTE  m_bParity;
	bool  m_fOpen;
	DWORD m_dwTimeouts;

public:
	void Config(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);
	void Config(TCommPara &CommPara);
	bool GetCommPara(TCommPara* pCommPara);
	
	bool Open(WORD wPort, DWORD dwBaudRate, BYTE bByteSize,	BYTE bStopBits, BYTE bParity);
  	bool Open(TCommPara& CommPara) { return Open(CommPara.wPort, CommPara.dwBaudRate, CommPara.bByteSize, CommPara.bStopBits, CommPara.bParity); };	
  	bool Open();

    bool Close();
    bool SetComm(DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity);
	bool SetBaudRate(DWORD dwBaudRate);
	
    bool IsOpen() { return m_fOpen; };
	DWORD ReadComm(void* pvBuf, DWORD dwLength);
	DWORD ReadComm(void* pvBuf, DWORD dwLength, DWORD dwTimeouts);
	
	DWORD Read(void* pvBuf, DWORD dwLength) { return ReadComm(pvBuf, dwLength);};
	DWORD Read(void* pvBuf, DWORD dwLength, DWORD dwTimeouts) {return  ReadComm(pvBuf, dwLength, dwTimeouts);};
	DWORD Write(void* pvBuf, WORD wLength);
	
	bool SetDTR();
	bool ClrDTR();
	bool GetRI(void);
	bool GetDCD(void);
	void SetTimeouts(DWORD dwTimeouts) { m_dwTimeouts = dwTimeouts; };
};


#endif // CCOMM_H


