/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：SFTP.cpp
 * 摘    要：本文件主要实现了简单文件传输协议SFTP，本协议作为数据区嵌入到其它
 *       		 通讯协议中，它本身不控制接收发送，只提供接口函数供其它通信协议在
 *			 帧处理时调用
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年8月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#include "stdafx.h"
#include	"sftp.h"
#include	<stdio.h>
#include	<string.h>
//#include	<sys/types.h>
//#include	<unistd.h>
//#include	<sys/stat.h>
//#include	<fcntl.h>
//#include	<sys/types.h>
#include "ComAPI.h"
#include "sysfs.h"
#include "Trace.h"
#include "FaCfg.h"
#include "sysarch.h"
#include "FaConst.h"
#define	SFTP_CMD_DIR			1	//目录
#define	SFTP_CMD_RDF0 		2	//读文件启动帧
#define	SFTP_CMD_RDFN   		3	//读文件后续帧
#define	SFTP_CMD_WRF0   		4	//写文件启动帧
#define	SFTP_CMD_WRFN   		5	//写文件后续帧
#define	SFTP_CMD_FINISH 		6	//传输结束帧
#define	SFTP_CMD_CANCEL 		7	//取消传输帧

#define 	SFTP_ERR_OK         		0	//正确
#define 	SFTP_ERR_PATHNAME 	    1	//文件路径名错误
#define 	SFTP_ERR_FILE       		2	//打开文件失败
#define 	SFTP_ERR_READ       		3	//文件读错误
#define 	SFTP_ERR_WRITE      		4	//文件写错误
#define 	SFTP_ERR_FILEID     		5	//错误的文件标识
#define 	SFTP_ERR_OFFSET		6	//文件偏移错误
#define 	SFTP_ERR_FILESIZE		7	//文件大小错误
#define 	SFTP_ERR_TRANSTYPE	8	//文件传输类型错误
#define 	SFTP_ERR_CRC			9	//CRC Check Error
#define	  SFTP_ERR_CMDINVALID	10	//无效命令
#define	  SFTP_ERR_MEMORY		11

#define 	TYPE_DIR				1	//目录
#define	  TYPE_READ			2	//读文件
#define	  TYPE_WRITE			3	//写文件

#define 	CMD_UP           		0x80
#define 	CMD_DOWN         	0x00
#define 	CMD_UP_ERR       	0xC0

#define 	SFTP_BLOCKSIZE   	256

#define	DEBUG_WriteFirst
//#define	DEBUG_WriteNext

/////////////////////////////////////////////////////////////////////////////////////////////////////

CSftp::CSftp()
{
	m_IsFinish = true;   	
	m_wTxLen = 0;		 	
	m_wRxLen = 0;		 	
	m_dwFileID = 0;		   	
	m_dwFileSize = 0;	    	
	m_wBlockSize = 0;		  	
	m_wPermission = 0;	     	
	m_bTransType = 0;		 	
	memset(m_bDir, 0, sizeof(m_bDir));
	memset(m_szPathName, 0, sizeof(m_szPathName));
	m_pbDataBuf = NULL;
	memset(&m_tmLastRecv, 0, sizeof(m_tmLastRecv));
}


CSftp::~CSftp()
{
	
}


bool CSftp::HandleFrm(BYTE* pbRx, BYTE* pbTx)
{
	switch (*pbRx)  //命令
	{
		case SFTP_CMD_DIR:
			//m_wTxLen = Dir( pbRx, wRxLen, pbTx );
			return true;
		
		case SFTP_CMD_RDF0:
			m_wTxLen = ReadFirst( pbRx, pbTx );
			return true;
		
		case SFTP_CMD_RDFN:
			m_wTxLen = ReadNext( pbRx, pbTx );
			return true;
		
		case SFTP_CMD_WRF0:
			m_wTxLen = WriteFirst( pbRx, pbTx );
			return true;
		
		case SFTP_CMD_WRFN:
			m_wTxLen = WriteNext( pbRx, pbTx );
			return true;
		
		case SFTP_CMD_FINISH:
			m_wTxLen = TransferFinish( pbRx, pbTx );
			return true;
		
		case SFTP_CMD_CANCEL:
			m_wTxLen = TransferCancel( pbRx, pbTx );
			return true;
		
		default:
			DTRACE(DB_FAPROTO, ("unknow cmd = %x\n",*pbRx));
			break;
	}	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSftp::Clear()
{
	m_IsFinish = true;   	
	m_wTxLen = 0;		 	
	m_wRxLen = 0;		 	
	m_dwFileID = 0;		   	
	m_dwFileSize = 0;	    	
	m_wBlockSize = 0;		  	
	m_wPermission = 0;	     	
	m_bTransType = 0;		 	
	memset(m_bDir, 0, sizeof(m_bDir));
	memset(m_szPathName, 0, sizeof(m_szPathName));
	m_pbDataBuf = NULL;
	memset(&m_tmLastRecv, 0, sizeof(m_tmLastRecv));
}

//描述：SFTP协议的DIR命令
/*
int CSftp::Dir(BYTE* pbRx, WORD wRxLen, BYTE* pbTx)
{
	BYTE* pbTx0 = pbTx;
	
	pbRx++;   							//命令
	
	BYTE bBlkNO = *pbRx++;					//序号
	
	m_wBlockSize = ByteToWord(pbRx);		//上载快大小
	
	pbRx += 2;
	
	*pbTx++ = SFTP_CMD_DIR;
	
	if (bBlkNO == 0)
	{
		m_iDirLen =  GetDir(m_bDir);
		if (m_iDirLen < 0)
		{
			*pbTx++ = 0xff;   				//帧序号,错误
			return pbTx - pbTx0;
		}
	}
	
	if (m_iDirLen<=0 || bBlkNO>=(m_iDirLen+m_wBlockSize)/m_wBlockSize)
	{
		*pbTx++ = 0xff;   					//帧序号,错误
		return pbTx - pbTx0;
	}
	
	pbTx += WordToByte(m_iDirLen, pbTx); 		//目录结构的总长度
	
	WORD wLen = m_wBlockSize;
	
	if (m_iDirLen < wLen)
		wLen = m_iDirLen;
	
	pbTx += WordToByte(wLen, pbTx); 			//实际上载长度
	
	memcpy(pbTx, &m_bDir[(DWORD )m_wBlockSize*bBlkNO], wLen);
	
	pbTx += wLen;
	
	return pbTx - pbTx0;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//向服务器上传文件启动帧
int CSftp::ReadFirst(BYTE* pbRx, BYTE* pbTx)
{
	BYTE* pbTx0 = pbTx;
	
	if (m_IsFinish)
	{
		pbRx++;   							//命令
		m_wBlockSize = ByteToWord(pbRx);  		//上载块大小
		pbRx += 2;
		
		WORD wNameLen = ByteToWord(pbRx);  	//文件名长度
		
		pbRx += 2;
		
		if (wNameLen > PATHNAME_LEN)
		{
			*pbTx0++ = SFTP_CMD_RDF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_PATHNAME;
			return 2;
		}
		
		memcpy(m_szPathName, pbRx, wNameLen);
		
		m_szPathName[wNameLen] = '\0';

		DTRACE(DB_FAPROTO, ("CSftp::ReadFirst: upload file %s\n", m_szPathName));

		int f = open(m_szPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
		if (f < 0)
		{
			DTRACE(DB_FAPROTO, ("CSftp::ReadFirst: fail to open file %s\n", m_szPathName));

			*pbTx0++ = SFTP_CMD_RDF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_FILE;
			return 2;
		}
		
		m_dwFileID++;
		if (m_dwFileID == 0)
		{
			m_dwFileID = 1;
		}
		
		int nFileSize = lseek(f, 0, SEEK_END);
		m_dwFileSize = nFileSize;
		
		lseek(f, 0, SEEK_SET);
		
		WORD wUpLen = m_wBlockSize;
		
		if (nFileSize < wUpLen)
		wUpLen = nFileSize;
		
		m_bTransType = TYPE_READ;
		
		//组正确的帧
		*pbTx++ = SFTP_CMD_RDF0 + CMD_UP;       	//cmd		
		DWordToByte(m_dwFileID, pbTx);  	//文件ID
		pbTx += 4;		
		DWordToByte(nFileSize, pbTx);   	//文件长度
		pbTx += 4;		
		pbTx += 2;							//CRC Check		
		WordToByte(wUpLen, pbTx);       	//上载长度
		pbTx += 2;		

		if (read(f, pbTx, wUpLen) != wUpLen)
		{
			*pbTx0++ = SFTP_CMD_RDF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_READ;
			close(f);
			return 2;
		}
		
		//WORD wMycrc = get_crc_16(0, pbTx, wUpLen);
		WORD wMycrc = CheckCrc16(pbTx, wUpLen);
		//printf("SFTP::FirstRead CRC =%d\n",wMycrc);
		pbTx -=4;
		
		WordToByte(wMycrc, pbTx);
		pbTx += 2;
		
		pbTx += wUpLen+2;
		
		close(f);
		
		return pbTx - pbTx0;
	}
	else
	{
		*pbTx0++ = SFTP_CMD_RDF0 + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_CMDINVALID;
		return 2;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//向服务器上传文件后续帧
int CSftp::ReadNext(BYTE* pbRx, BYTE* pbTx)
{
	BYTE* pbTx0 = pbTx;
	pbRx++;   							//命令
	DWORD dwFileID = ByteToDWORD(pbRx, 4);  	//文件ID
	pbRx += 4;
	if (dwFileID != m_dwFileID)
	{
		*pbTx0++ = SFTP_CMD_RDFN + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_FILEID;
		return 2;
	}

	WORD wOffsetNum = ByteToWord(pbRx);  	//偏移号
	
	//偏移地址=偏移号*256
	DWORD dwOffset = (DWORD )wOffsetNum * SFTP_BLOCKSIZE;
	
	pbRx += 2;
	WORD wUpLen = ByteToWord(pbRx);   		//上载长度
	
	pbRx += 2;

	int f = open(m_szPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
	if (f < 0)
	{
		*pbTx0++ = SFTP_CMD_RDFN + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_FILE;
		return 2;
	}

	int nFileSize = lseek(f, 0, SEEK_END);
	if (dwOffset > nFileSize)		//>=
	{
		*pbTx0++ = SFTP_CMD_RDFN + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_OFFSET;
		close(f);
		return 2;
	}
	
	if (dwOffset+wUpLen > nFileSize)
		wUpLen = nFileSize - dwOffset;
	
	//组正确的帧
	*pbTx++ = SFTP_CMD_RDFN + CMD_UP;       	//cmd
	
	DWordToByte(m_dwFileID, pbTx);  	//文件ID
	pbTx += 4;
	
	WordToByte(wOffsetNum, pbTx);   	//偏移号
	pbTx += 2;
	
	pbTx += 2;							//CRC Check
	
	WordToByte(wUpLen, pbTx);       	//实际上载长度
	pbTx += 2;

	lseek(f, dwOffset, SEEK_SET);
	if (read(f, pbTx, wUpLen) != wUpLen)
	{
		*pbTx0++ = SFTP_CMD_RDFN + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_READ;
		close(f);
		return 2;
	}
	
	//WORD wMycrc = get_crc_16(0, pbTx, wUpLen);
	WORD wMycrc = CheckCrc16(pbTx, wUpLen);
	//printf("SFTP::ReadNext CRC =%d\n",wMycrc);
	pbTx -=4;
	
	WordToByte(wMycrc, pbTx);	
	pbTx += 2;
	
	pbTx += wUpLen+2;
	
	close(f);
	return pbTx - pbTx0;
}

bool CSftp::IsFwUpdate()
{
    return strstr(m_szPathName, ".ldr") != NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//从服务器下载文件启动帧
int CSftp::WriteFirst(BYTE* pbRx, BYTE* pbTx)
{
    int f = -1;
	BYTE* pbTx0 = pbTx;	
	if (m_IsFinish)
	{	
		pbRx++;   							//命令
/////////////////////////	
		WORD wNameLen = ByteToWord(pbRx);  	//文件名长度
		
		if (wNameLen > PATHNAME_LEN)
		{
			*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_PATHNAME;
			return 2;
		}
////////////////////////	
		pbRx += 2;
		memcpy(m_szPathName, pbRx, wNameLen);	//文件名
		
		m_szPathName[wNameLen] = '\0';

		if (!IsFwUpdate())
		{//下载终端程序直接放到内存里即可
			f = open(m_szPathName, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, S_IREAD|S_IWRITE);
			if (f < 0)
			{
				DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : fail to open %s\n", m_szPathName));
				*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
				*pbTx0++ = SFTP_ERR_FILE;
				return 2;
			}
		}

//////////////////////	
		pbRx += wNameLen;
		int nFileSize = ByteToDWORD(pbRx, 4);		//文件长度
		m_dwFileSize= nFileSize;
//////////////////////	
		pbRx += 4;
		m_wPermission = ByteToWord(pbRx);		//文件属性
//////////////////////
		pbRx += 2;
		WORD wRcrc = ByteToWord(pbRx);			//crc check
//////////////////////
		pbRx += 2;
		WORD wDownLen = ByteToDWORD(pbRx, 4);	//下载长度
		
		if( wDownLen > nFileSize )
		{
			DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : File len error\n"));
			*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_FILE;
			return 2;//???
		}
//////////////////////		
		m_dwFileID++;							//文件ID
		if (m_dwFileID == 0)
		{
			m_dwFileID = 1;
		}
		
		pbRx += 2;	
		//WORD wMycrc = get_crc_16(0, pbRx, wDownLen);
		WORD wMycrc = CheckCrc16(pbRx, wDownLen);
		if(wMycrc != wRcrc)
		{
			DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : CRC Check Error\n"));
			DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : Recv CRC = %d\n",wRcrc));
			DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : My CRC = %d\n",wMycrc));
			*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_CRC;
			return 2;
		}
		
#ifdef	DEBUG_WriteFirst
		DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : download from server: %s %d\n",m_szPathName, wNameLen));
		DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : Filesize = %d\n", nFileSize));
		DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : File Permission = %o\n", m_wPermission));
		DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : download len = %d\n",wDownLen));
		DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : CRC Check = %d\n", wMycrc));
#endif	
		
		if (!IsFwUpdate())
		{
			if (write(f, pbRx, wDownLen) != wDownLen)
			{
				DTRACE(DB_FAPROTO, ("CSftp::WriteFirst : File write error\n"));
				*pbTx0++ = SFTP_CMD_WRF0 + CMD_UP_ERR;
				*pbTx0++ = SFTP_ERR_WRITE;
				close(f);
				return 2;
			}
		}
		else
		{		
			if (m_pbDataBuf != NULL)
			{
				delete []m_pbDataBuf;
				m_pbDataBuf = NULL;
			}
			m_pbDataBuf = new BYTE[nFileSize];
			memcpy(m_pbDataBuf, pbRx, wDownLen);
		}
		
		GetCurTime(&m_tmLastRecv);
		
		WORD wOffsetNum = 0;
		
		m_bTransType = TYPE_WRITE;
		
		//组正确的帧
		*pbTx++ = SFTP_CMD_WRF0 + CMD_UP;       	//cmd		
		DWordToByte(m_dwFileID, pbTx);  	//文件ID	
		pbTx += 4;		
		WordToByte(wOffsetNum, pbTx);   	//偏移号
		pbTx += 2;				
		*pbTx++ = 0x00;			       			//执行结果, 0: OK; FF: ERROR
		if (!IsFwUpdate())
			close(f);
		return pbTx - pbTx0;
	}
	else
	{
		*pbTx0++ = SFTP_CMD_RDF0 + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_CMDINVALID;
		return 2;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//从服务器下载文件后续帧
int CSftp::WriteNext(BYTE* pbRx, BYTE* pbTx)
{
    int f = -1;
	/*
#ifdef	DEBUG
	for(int i=0; i<9; i++)
	{
		printf("recv[%d]=%x\n",i,pbRx[i]);
	}
#endif
*/
	BYTE* pbTx0 = pbTx;
	pbRx++;   							//命令
	
	DWORD dwFileID = ByteToDWORD(pbRx, 4);  	//文件ID
	
	if (dwFileID != m_dwFileID)
	{
		DTRACE(DB_FAPROTO, ("CSftp::WriteNext : WriteNext FileID error\n"));
		*pbTx0++ = SFTP_CMD_WRFN + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_FILEID;
		return 2;
	}
	
	pbRx += 4;
	WORD wOffsetNum = ByteToWord(pbRx);  	//偏移号
	
	pbRx += 2;
	WORD wRcrc = ByteToWord(pbRx);  		//CRC Check	
	
	//偏移地址=偏移号*256
	DWORD dwOffset = (DWORD )wOffsetNum * SFTP_BLOCKSIZE;
	
	pbRx += 2;
	WORD wDownLen = ByteToWord(pbRx);   		//下载长度
	/*
#ifdef	DEBUG_WriteNext
	printf("SFTP::OffsetNum = %d\n", wOffsetNum);
	printf("SFTP::downlen = %d\n", wDownLen);
#endif
*/
	if (!IsFwUpdate())
	{
		f = open(m_szPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
		if (f < 0)
		{
			DTRACE(DB_FAPROTO, ("CSftp::WriteNext : Open error\n"));
			*pbTx++ = SFTP_CMD_WRFN + CMD_UP_ERR;
			*pbTx++ = SFTP_ERR_FILE;
			return 2;
		}
	}
	else
	{
		if (m_pbDataBuf == NULL)
		{
			DTRACE(DB_FAPROTO, ("CSftp::WriteNext : Open error\n"));
			*pbTx0++ = SFTP_CMD_WRFN + CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_FILE;
			return 2;
		}
	}	

	pbRx += 2;
	
	//WORD wMycrc = get_crc_16(0, pbRx, wDownLen);
	WORD wMycrc = CheckCrc16(pbRx, wDownLen);
	if(wMycrc != wRcrc)
	{
		DTRACE(DB_FAPROTO, ("CSftp::WriteNext : CRC Check Error\n"));
		DTRACE(DB_FAPROTO, ("CSftp::WriteNext : Recv CRC = %d\n",wRcrc));
		DTRACE(DB_FAPROTO, ("CSftp::WriteNext : My CRC = %d\n",wMycrc));
		*pbTx0++ = SFTP_CMD_WRFN + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_CRC;
		close(f);
		return 2;
	}	

	if (!IsFwUpdate())
	{
		lseek(f, dwOffset, SEEK_SET);
		if (write(f, pbRx, wDownLen) != wDownLen)
		{
			*pbTx++ = SFTP_CMD_WRFN + CMD_UP_ERR;
			*pbTx++ = SFTP_ERR_WRITE;
			close(f);
			return 2;
		}
	}
	else	
	{
		memcpy(m_pbDataBuf+dwOffset, pbRx, wDownLen);
	}
	
	GetCurTime(&m_tmLastRecv);
	
	//组正确的帧
	*pbTx++ = SFTP_CMD_WRFN + CMD_UP;       	//cmd	
	DWordToByte(m_dwFileID, pbTx);  	//文件ID	
	pbTx += 4;	
	WordToByte(wOffsetNum, pbTx);   	//偏移号	
	pbTx += 2;	
	*pbTx++ = 0x00;      					//执行结果
	if (!IsFwUpdate())
		close(f);
	return pbTx - pbTx0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//传输结束帧
///////////////////////////////////////////////////////////////////////////
int CSftp::TransferFinish(BYTE* pbRx, BYTE* pbTx)
{	
    int f = -1;
	BYTE* pbTx0 = pbTx;
	pbRx++;   							//命令	
	BYTE bType = *pbRx;					//传输类型
	DWORD dwFileID = 0;
	pbRx++;   
	dwFileID = ByteToDWORD(pbRx, 4);  	//文件ID
	
	if( bType != m_bTransType)
	{
		DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : TransType error\n"));
		*pbTx0++ = SFTP_CMD_FINISH + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_TRANSTYPE;
		return 2;
	}	
	if (dwFileID != m_dwFileID)
	{
		DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : FileID error\n"));
		*pbTx0++ = SFTP_CMD_FINISH + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_FILEID;
		return 2;
	}
	pbRx +=4;
	WORD wRcrc = ByteToWord(pbRx);

	if (!IsFwUpdate())
	{
		f = open(m_szPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
		if (f < 0)
		{
			DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : Fail to open file %s\n", m_szPathName));
			*pbTx++ = SFTP_CMD_FINISH+ CMD_UP_ERR;
			*pbTx++ = SFTP_ERR_FILE;
			return 2;
		}

		int nFileSize = lseek(f, 0, SEEK_END);
		lseek(f, 0, SEEK_SET);
	
		if( nFileSize != m_dwFileSize )
		{
			DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : File size mismatchm, rx=%d, my=%d\n", m_dwFileSize, nFileSize));
			*pbTx++ = SFTP_CMD_FINISH + CMD_UP_ERR;
			*pbTx++ = SFTP_ERR_FILESIZE;
			close(f);
			return 2;
		}
	}
	else
	{
		if (m_pbDataBuf == NULL)
		{	    
			DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : Open error\n"));
			*pbTx0++ = SFTP_CMD_FINISH+ CMD_UP_ERR;
			*pbTx0++ = SFTP_ERR_FILE;
			return 2;
		}
	}

	BYTE *pbbuf;
	if(m_dwFileSize > 512)
		pbbuf = new BYTE[512];
	else
		pbbuf = new BYTE[m_dwFileSize];
	
	if(pbbuf == NULL)
	{
		DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : Memeoy not enough!\n"));
		*pbTx0++ = SFTP_CMD_FINISH + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_MEMORY;
		close(f);
		return 2;
	}
	
	WORD wMycrc = PPPINITFCS16;
	
/////////////////////////////////////		
	for(int i=0; i<m_dwFileSize/512; i++)
	{
	    if (!IsFwUpdate())
	    {
	    	if (read(f, pbbuf, 512) != 512)
			{
				DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : Fail to read file, i=%d\n", i));
				*pbTx++ = SFTP_CMD_FINISH + CMD_UP_ERR;
				*pbTx++ = SFTP_ERR_READ;
				close(f);
				return 2;
			}
	    }
	    else
	    {
			memcpy(pbbuf, m_pbDataBuf+i*512, 512);
	    }
		//wMycrc = get_crc_16(wMycrc, pbbuf, 512);
		wMycrc = pppfcs16(wMycrc, pbbuf, 512);
		//printf("SFTP:: %d crc = %x\n",i,wMycrc);
	}
	
	if(m_dwFileSize%512 > 0)
	{
	    if (!IsFwUpdate())
	    	read(f, pbbuf, m_dwFileSize%512);
	    else
			memcpy(pbbuf, m_pbDataBuf+m_dwFileSize-m_dwFileSize%512, m_dwFileSize%512);
		//wMycrc = get_crc_16(wMycrc, pbbuf, m_dwFileSize%512);
		wMycrc = pppfcs16(wMycrc, pbbuf, m_dwFileSize%512);
	}
	wMycrc ^= 0xffff; 
	delete[] pbbuf;
////////////////////////////////////
	
	if(wMycrc != wRcrc)
	{
		DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : CRC Check Error\n"));
		DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : Recv CRC = %x\n",wRcrc));
		DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : My CRC = %x\n",wMycrc));
		*pbTx0++ = SFTP_CMD_FINISH + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_CRC;
		close(f);
		return 2;
	}

	if(bType == TYPE_WRITE)
	{
		if (!IsFwUpdate())
			chmod(m_szPathName, m_wPermission);		//设置文件属性
	}

	//组正确的帧
   	*pbTx++ = SFTP_CMD_FINISH + CMD_UP;       	//cmd	
    *pbTx++ = bType;						//传输类型	
    DWordToByte(m_dwFileID, pbTx);  	//文件ID	
    pbTx += 4;
    WordToByte(wMycrc, pbTx);
    pbTx += 2;
    *pbTx++ = 0x00;      					//保留

    if (IsFwUpdate())
    {	
    }
	close(f);
	DTRACE(DB_FAPROTO, ("CSftp::TransferFinish : TransferFinished\n"));	
	return pbTx - pbTx0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//取消传输帧
int CSftp::TransferCancel(BYTE* pbRx, BYTE* pbTx)
{
	BYTE* pbTx0 = pbTx;
	pbRx++;   							//命令
	
	BYTE bType = *pbRx;					//传输类型
	if( bType != m_bTransType)
	{
		*pbTx0++ = SFTP_CMD_CANCEL + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_TRANSTYPE;
		return 2;
	}
	pbRx++;   
	
	DWORD dwFileID = ByteToDWORD(pbRx, 4);  	//文件ID
	
	if (dwFileID != m_dwFileID)
	{
		*pbTx0++ = SFTP_CMD_CANCEL + CMD_UP_ERR;
		*pbTx0++ = SFTP_ERR_FILEID;
		return 2;
	}
	
	if( m_bTransType == TYPE_WRITE)
	{
		unlink(m_szPathName);
	}
	
	//组正确的帧
	*pbTx++ = SFTP_CMD_CANCEL + CMD_UP;       	//cmd
	
	*pbTx++ = m_bTransType;				//传输类型
	
	DWordToByte(m_dwFileID, pbTx);  	//文件ID
	
	pbTx += 4;
	
	*pbTx++ = 0x00;      					//保留

	if (IsFwUpdate()) 
	{
		if (m_pbDataBuf != NULL)
		{
			delete []m_pbDataBuf;
			m_pbDataBuf = NULL;
		}
		m_pbDataBuf = NULL;
	}
	Clear();
	
	return pbTx - pbTx0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


