#include "syscfg.h"
#ifdef SYS_VDK
#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include "FaAPI.h"
#include "RemoteDown.h"
#include "ComAPI.h"

#ifdef SYS_VDK
#include "bios.h"
#include "st7529.h"
#include "CL790DMENU.h"
#include "Lzari.h"
#include <flash.h>
#endif



#define RETRYNUM   10
//#define ACKNUM     40
#define ACKNUM     40
//#define TFTPDEBUG

bool g_fUpdateFirmware = false;

#ifdef SYS_VDK       
extern void iFlash_One_Sector (WORD  *Src, WORD  *Dst, int iRetryNum);
extern WORD get_crc_16 (WORD start, BYTE *p, int n);


namespace
{
	const int  FrameReceiveLen =1024*2;
	int	rxnum;
	const char NETERR = 30;
}
#endif

CRemoteDown::CRemoteDown()	
{	     
}

CRemoteDown:: ~CRemoteDown()
{
    if (m_bFileBuf == NULL)
	{
	   delete m_bFileBuf;
	   m_bFileBuf = NULL;
	}
}

void CRemoteDown::Run()
{    
}

#ifdef SYS_VDK
int iFlash_Write(unsigned char *Src,unsigned int Size,unsigned long Dst)
{

    static WORD Buffer[2048];//2*2K 字对齐
    WORD offset,writesize,i,j,TempSize;
    unsigned char *ps,*pd;
    ps = Src;
	pd = (unsigned char *)(Dst+0x20000000);
    TempSize = Size;
 
  while(1)
	{
      memcpy(Buffer,(char *)((unsigned long)pd & 0xfffff000),sizeof(Buffer));
	  offset =  (0xfff & (unsigned long)pd);//第一个要写入的个数
	  writesize = 2048*2 - offset;
    // 
    if (TempSize <= writesize) 
		{ //余数写
		 memcpy((char *)((unsigned long)&Buffer+offset),
	 			(char *)ps,TempSize);
		 for (i=0;i<10;i++) 
		 {
		   iFlash_One_Sector(Buffer,(WORD *)((unsigned long)pd & 0xfffff000), i);
		   j=memcmp(Buffer,(char *)((unsigned long)pd & 0xfffff000),sizeof(Buffer));
		   if (j==0)return true;
		 }
		  return false;//10次写不成功返回
		}
     else 
	 {
		{ 
		 memcpy((char *)((unsigned long)&Buffer+offset),
	 			(char *)ps,writesize);
		 for (i=0;i<10;i++) 
		 {
		   iFlash_One_Sector(Buffer,(WORD *)((unsigned long)pd & 0xfffff000), i);
		   j=memcmp(Buffer,(WORD *)((unsigned long)pd & 0xfffff000),sizeof(Buffer));
		   if (j==0)break;
		   if (i==9)return false;
		 }
		}
		//写成功了
         TempSize -= writesize;
		 pd += writesize;
		 ps += writesize;
	  }
	} 

}
/*
bool iFlash_Write(unsigned char *Src,unsigned int Size,unsigned long Dst)
{
   static BYTE Buffer[4096];//2*2K 字对齐
   static BYTE Buffer1[4096];//2*2K 字对齐
    WORD writesize,i,j,TempSize;
    unsigned long *ps,*pd;
    ps = (unsigned long *)Src;
	pd = (unsigned long *)Dst;
    TempSize = Size;
 	while(TempSize > 0)
 	{
 		if (TempSize > 4096)
 			writesize = 4096;
 		else
 			writesize = TempSize;
		memcpy(Buffer, (char *)ps,writesize);
		for (i=0;i<10;i++)
		{
			WriteData((unsigned long)pd, writesize, 1, (int *)Buffer);
			ReadData((unsigned long)pd, writesize, 1, (int *)Buffer1);
			j=memcmp(Buffer, Buffer1, writesize);
			if (j==0)break;
			if (i==9)return false;
		}
		TempSize -= writesize;
		pd += writesize;
		ps += writesize;
 	}
 	return true;
}

*/

#endif

#ifdef SYS_VDK
long LzariUncompress(BYTE *src, long uzLen,unsigned long flashOffset)
{
       
//		DTRACE(DB_FA, ("CTftp::LzariUncompress: Begin uncompress to flash %d\r\n",flashOffset));
		LZARI Lzari;
		
//		if(Lzari.Alloc(uzLen)<=0)
//		{	DTRACE(DB_FA, ("aTftp::LzariUncompress: Alloc Error\r\n"));			
//			return 0;
//		}
		DWORD dwOutSize = 0;
		memcpy(&dwOutSize, src, 4);
		BYTE *lpszOut = new BYTE[dwOutSize];
//		BYTE *lpszOut = new BYTE[uzLen];
		int nOutSize = 0;
		int result=0;
		int blockno = 0;
		Lzari.UnCompress(src, uzLen, lpszOut, nOutSize);
		if(nOutSize <= 0){
//			DTRACE(DB_FA, ("bTftp::LzariUncompress: UnCompress Error\r\n"));	
			return 0;
		}
		result = nOutSize;
	//	WriteToUart1(lpszOut,nOutSize);
		while(nOutSize>0)					
		{			
			int bWriteFlash = iFlash_Write(lpszOut+blockno*FrameReceiveLen,FrameReceiveLen,flashOffset+blockno * FrameReceiveLen);
     	//	VDK::Sleep(10*TICKPERMS);//
     	//	DTRACE(DB_FA,("CTftp::LzariUncompress: Write flash %d block\r\n",blockno));
	     	ClearWDG();
	   		if(!bWriteFlash)
	   		{	
//	   			DTRACE(DB_FA,("CTftp::LzariUncompress: Write flash %d block fail\r\n",blockno));
	   			lcd.Print("Write flash fail!\n\n",0,1);
//	   			Sleep(30*1000);  			   			
	   			return -1;
	   		}
	   		blockno++;	   		
	   		nOutSize=nOutSize-FrameReceiveLen;		
		}
		Lzari.Release();
		if(lpszOut!=NULL)
			delete [] lpszOut;
		return result;

}
#endif

void CRemoteDown::DoUpdate()
{
#ifdef SYS_VDK 			
    g_fUpdateFirmware = true;   
    
    /*for (int i=0; i<2*60*10; i++)
    {
        ClearWDG();
        lcd.Clear();
	    lcd.Print("\n\n\r等待所有线程退出\n\n",0,1,false,true);
	    lcd.Refresh();
        VDK::Sleep(100);   		
    }*/

    if (m_bFileBuf!=NULL && m_dwFileSize>0 && strstr(m_szPathName, ".ldr")!= NULL)//升级程序
	{
		PushUnscheduledRegion();

		lcd.Clear();
		lcd.Print("\r更新程序中...\n\n",0,1,false,true);
		lcd.Print("\r请不要复位终端!\n\n",0,2,false,true);
		lcd.Refresh();
		int nTotalLen=0;
		int nOutLen = 0;
		SetFlashBankToProgram0();
	
		nOutLen = LzariUncompress(m_bFileBuf, m_dwFileSize, nTotalLen);
		nTotalLen+=nOutLen;
		if(nOutLen==-1) 
		{			
			lcd.Clear();
			lcd.Print("\r更新程序失败\n\n",0,1,false,true);
			lcd.Print("\r终端再过几秒复位",0,2,false,true);       
			lcd.Refresh();     
			Sleep(5000);
			ResetCPU();
		}
		lcd.Clear();
		lcd.Print("\r更新程序成功\n\n",0,1,false,true);
		lcd.Print("\r终端再过几秒复位",0,2,false,true);   
		lcd.Refresh();         
		Sleep(5000);
		ResetCPU();	
	}
	else if (m_bFileBuf!=NULL && m_dwFileSize>=0)//其它文件
	{
		int f = open(m_szPathName, O_RDWR, S_IREAD|S_IWRITE);
		if (f < 0)
		{
			lcd.Clear();
			lcd.Print("\r复制文件出错\n\n",0,1,false,true);
			lcd.Print("\r终端再过几秒复位",0,2,false,true);    
			lcd.Refresh();        
			//Sleep(5000);
			//ResetCPU();
			return;
		}
		lseek(f, 0, SEEK_SET);
		if (write(f, m_bFileBuf, m_dwFileSize) != m_dwFileSize)
		{
			lcd.Clear();
			lcd.Print("\r写文件出错\n\n",0,1,false,true);
			lcd.Print("\r终端再过几秒复位",0,2,false,true); 
			lcd.Refresh();           
			//Sleep(5000);
			//ResetCPU();	
		}
		close(f);
	}
	/*
	lcd.Clear();
	lcd.Print("\r写文件成功\n\n",0,1,false,true);
	lcd.Print("\r终端再过几秒复位",0,2,false,true);    
	lcd.Refresh();        
	Sleep(5000);
	ResetCPU();	*/
#endif
}

CRemoteDown g_RemoteDown;
int UpdateThread(void* pvPara)
{
	DTRACE(DB_FAPROTO, ("UpdateThread : started!\n"));
	g_RemoteDown.DoUpdate();	
	return 1;
}
#endif
