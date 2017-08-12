
#include "stdafx.h"
#include "VirtualComm.h"






CVirtualComm::CVirtualComm()
{

}

CVirtualComm::~CVirtualComm()
{

}

BOOL CVirtualComm::Open(LPCTSTR szPort, DWORD dwBaudRate, BYTE bByteSize,
			         BYTE bStopBits, BYTE bParity)
{
	m_strPort = szPort;
	return true;
}

int CVirtualComm::Write(LPCVOID lpBuf, DWORD dwLength)
{
	return dwLength;
}

int CVirtualComm::Read(LPVOID buf, DWORD dwLength)
{
	int iLen = 0;

	return iLen;
}
