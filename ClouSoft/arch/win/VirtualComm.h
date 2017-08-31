

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
   int SetRecBuf(LPCVOID lpBuf, DWORD dwLength);

   int SetAutoRecBuf(LPCVOID lpBuf, DWORD dwLength);
   int ClearAutoRecBuf(void);

private:
   BYTE m_sendBuf[1024];
   BYTE m_RecBuf[1024];
   BYTE m_autoBuf[1024];
   int  m_iAutoLen;
   void RunThread(void);
   CWinThread* m_pThread;



};