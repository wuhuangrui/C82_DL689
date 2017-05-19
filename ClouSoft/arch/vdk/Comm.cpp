// Comm.cpp: implementation of the CComm class.
//
//////////////////////////////////////////////////////////////////////

#include "Comm.h"
#include "Uart.h"
#define COMM_INVALID_PORT	0xff

CComm::CComm()
{
	m_pCSIO = NULL;
	//m_pCSport = NULL;
	m_dwTimeouts = 1000;
	m_dwBaudRate = CBR_9600;
	m_bByteSize = 8;
	m_bStopBits = ONESTOPBIT;
	m_bParity = NOPARITY;
	m_fOpen = false;
	m_wPort = COMM_INVALID_PORT;
}


CComm::~CComm()
{
	 
}

void CComm::Config(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
{
	m_wPort = wPort;
	m_dwBaudRate = dwBaudRate;
	m_bByteSize = bByteSize;
	m_bStopBits = bStopBits;
	m_bParity = bParity;
}

void CComm::Config(TCommPara& CommPara)
{
	m_wPort = CommPara.wPort;
	m_dwBaudRate = CommPara.dwBaudRate;
	m_bByteSize = CommPara.bByteSize;
	m_bStopBits = CommPara.bStopBits;
	m_bParity = CommPara.bParity;
}

bool CComm::GetCommPara(TCommPara* pCommPara)
{
	if (IsOpen())
	{
		pCommPara->wPort = m_wPort; 
		pCommPara->dwBaudRate = m_dwBaudRate;
		pCommPara->bByteSize = m_bByteSize;
		pCommPara->bStopBits = m_bStopBits;
		pCommPara->bParity = m_bParity;
		return true;
	}
	else
	{
		return false;
	}
}

//备注：应保证在成功打开即占有资源后才更改相应变量
bool CComm::Open(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
{
	m_wPort = wPort;
	m_dwBaudRate = dwBaudRate;
	m_bByteSize = bByteSize;
	m_bStopBits = bStopBits;
	m_bParity = bParity;
	
	return Open();
}


//备注：应保证在成功打开即占有资源后才更改相应变量
bool CComm::Open()
{
	if (m_wPort == COMM_INVALID_PORT)	//串口没有配置过
		return false;
	
	if (m_wPort == 0)
	{
		m_fOpen = Uart0.Open(m_dwBaudRate, m_bByteSize, m_bStopBits, m_bParity);
		if (m_fOpen)
			m_pCSIO = &Uart0;
	}
	else if (m_wPort == 1)
	{
		m_fOpen = Uart1.Open(m_dwBaudRate, m_bByteSize, m_bStopBits, m_bParity);
		if (m_fOpen)
			m_pCSIO = &Uart1;
	}
	else if (m_wPort == 2)
	{
		m_fOpen = Uart2.Open(m_dwBaudRate, m_bByteSize, m_bStopBits, m_bParity);
		if (m_fOpen)
			m_pCSIO = &Uart2;
	}
	else if (m_wPort == 3)
	{
		m_fOpen = Uart3.Open(m_dwBaudRate, m_bByteSize, m_bStopBits, m_bParity);
		if (m_fOpen)
			m_pCSIO = &Uart3;
	}
	else if (m_wPort == 4)
	{
		m_fOpen = Uart4.Open(m_dwBaudRate, m_bByteSize, m_bStopBits, m_bParity);
		if (m_fOpen)
			m_pCSIO = &Uart4;
	}
	else
	{
		return false;
	}
	
	return m_fOpen;
	
}


bool CComm::Close()
{
	m_fOpen = false;
	
	if (m_pCSIO != NULL)
		return m_pCSIO->Close();
	//else if (m_pCSport != NULL)
	//	return m_pCSport->Close();
	else
		return false;
}


bool CComm::SetComm(DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity)
{
    if (m_dwBaudRate==dwBaudRate && m_bByteSize==bByteSize 
    	&& m_bStopBits==bStopBits && m_bParity==bParity)
			return true;

    m_dwBaudRate = dwBaudRate;
	m_bByteSize = bByteSize;
	m_bStopBits = bStopBits;
	m_bParity = bParity;
	
	if (m_pCSIO != NULL)
		return m_pCSIO->SetUart(dwBaudRate, bByteSize, bStopBits, bParity);
	//else if (m_pCSport != NULL)		
	//	return m_pCSport->Open(dwBaudRate, bByteSize, bStopBits, bParity);	
	else
		return false;	
}


DWORD CComm::ReadComm(void* pvBuf, DWORD dwLength)
{
	if (m_pCSIO != NULL)
		return m_pCSIO->Read((INT8U *)pvBuf,(INT16U )dwLength, m_dwTimeouts);
	//else if (m_pCSport != NULL)	
	//	return m_pCSport->Read((INT8U *)pvBuf,(INT16U )dwLength, m_dwTimeouts);
	else
		return 0;
}

DWORD CComm::ReadComm(void* pvBuf, DWORD dwLength, DWORD dwTimeouts)
{
	if (m_pCSIO != NULL)
	 	return m_pCSIO->Read((INT8U *)pvBuf,(INT16U )dwLength, dwTimeouts);		
	//else if (m_pCSport != NULL)	
	//	return m_pCSport->Read((INT8U *)pvBuf,(INT16U )dwLength, dwTimeouts);
	else
		return 0;
}

DWORD CComm::Write(void* pvBuf, WORD wLength)
{
	if (m_pCSIO != NULL)
		return m_pCSIO->Write((INT8U *)pvBuf, wLength, 1000);	
	//else if (m_pCSport != NULL)	
	//	return m_pCSport->Write((INT8U *)pvBuf, wLength, 1000);			
	else	
		return 0;	
}


//描述:供串口打开后,重新设置串口波特率
bool CComm::SetBaudRate(DWORD dwBaudRate)
{
	if (m_fOpen == false)
		return false;
	if (dwBaudRate == m_dwBaudRate)
		return true;

    	m_dwBaudRate = dwBaudRate; 
	
	if (m_pCSIO != NULL)
		return m_pCSIO->SetUart(dwBaudRate, m_bByteSize, m_bStopBits, m_bParity);
	else 
		return false;
}

bool CComm::SetDTR()
{
	if (m_pCSIO != NULL)
	{
		((COutUART* )m_pCSIO)->SetDTREnable(true);		
		return true;
	}
	else
	{
		return false;
	}
}

bool CComm::ClrDTR()
{
	if (m_pCSIO != NULL)
	{
		((COutUART* )m_pCSIO)->SetDTREnable(false);
		return true;
	}
	else
	{
		return false;
	}
}

bool CComm::GetRI(void)
{
	if (m_pCSIO != NULL)
		return ((COutUART* )m_pCSIO)->GetRIHolding();
	else
		return false;
}

bool CComm::GetDCD(void)
{
	if (m_pCSIO != NULL)
		return ((COutUART* )m_pCSIO)->GetCDHolding();
	else
		return false;	
}
