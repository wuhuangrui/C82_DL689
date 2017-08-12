

class CVirtualComm
{

public:

	CString  m_strPort;
	WORD	 m_wPort; 
	DWORD	 m_dwBaudRate;
	BYTE	 m_bByteSize;
	BYTE	 m_bStopBits;
	BYTE	 m_bParity;
	BOOL	 m_OpenFlag;

   CVirtualComm();
   ~CVirtualComm();
   BOOL Open(LPCTSTR szPort, DWORD dwBaudRate, BYTE bByteSize,
		         BYTE bStopBits, BYTE bParity);
   BOOL Open(int szPort, DWORD dwBaudRate, BYTE bByteSize,
			         BYTE bStopBits, BYTE bParity);
   int Write(LPCVOID lpBuf, DWORD dwLength);
   int Read(LPVOID buf, DWORD dwLength);
   BOOL  IsOpen();

private:
   BYTE m_sendBuf[1024];
   BYTE m_RecBuf[1024];
   void RunThread(void);
   CWinThread* m_pThread;



};