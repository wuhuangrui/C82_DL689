 /*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DrvAPI.cpp
 * 摘    要：本文件主要实现本系统下设备驱动程序的封装
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009年7月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "DrvAPI.h"
#include "syscfg.h"
#include "sysdebug.h"
#include "bios.h"
#include "FaAPI.h"
#include <string>
#include "ProPara.h"
//#include "DbGbAPI.h"
#ifdef  SYS_LINUX
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
typedef unsigned char           u8;
typedef  unsigned short         u16;
typedef  unsigned long          u32;
typedef  unsigned long long     u64;
#include <linux/ethtool.h>
#include <linux/sockios.h>

#endif

using namespace std;
CLcd* g_pLcd = NULL;
RouteCMD RouteCmd;
DefaultGWPara  DefGWPara;
#ifdef SYS_LINUX

string cmd_system(const char* vCmd)
{
	string tRet = "";
	FILE *fpRead;
	fpRead = popen(vCmd, "r");
	if (fpRead == NULL)
		return tRet;


	char str[1024] = {0};
	while (fgets(str, 1024 - 1, fpRead) != NULL)
	{
		tRet += str;
		memset(str, 0, sizeof(str));
	}
	if(fpRead != NULL)
		pclose(fpRead);

	return tRet;
}

//ping默认网关
//超时返回-1
int Ping()
{

	return -1;

	int iCnt=-1,iResult=-1;
	/*BYTE bBuf[128];
	char szTmp[128];
	memset(bBuf,0,sizeof(bBuf));
	memset(szTmp,0,sizeof(szTmp));
	ReadItemEx(0, 0, 0x007f, bBuf);//读默认网关*/

	if(GetNetStat()==1)
		DTRACE(SOCKETS_DEBUG,("---%s----\r\n",RouteCmd.PingCmd));
	else
	{
		DTRACE(SOCKETS_DEBUG,("-----Ping fail due to Ethernet is not alive!!!!---"));
		return -2;
	}
	//sprintf(szTmp, "ping -c 1 %d.%d.%d.%d", bBuf[8], bBuf[9], bBuf[10], bBuf[11]);
	//string tStrRet = cmd_system("ping -c 1 10.98.96.7");//返回分区信息
	//string tStrRet = cmd_system(RouteCmd.PingCmd);//返回分区信息
	//i = tStrRet.find("bytes from");

	for(int i=0; i<4; i++)
	{
		string tStrRet = cmd_system(RouteCmd.PingCmd);//返回分区信息
		iResult = tStrRet.find("bytes from");
		if(iResult>=0)
			iCnt++;
	}

	//printf("iCnt  = %d\r\n", iCnt );
	if (iCnt >= 2)//ping4次至少通3次
	{
		DTRACE(SOCKETS_DEBUG,("host is alive\r\n"));
	}
	else
	{
		DTRACE(SOCKETS_DEBUG,("host is not alive\r\n"));
		iCnt = -1 ;
	}
	return iCnt ;
}

int IfReadListProc(char *target, BYTE *pBuf)
{
	struct ifreq ifr;
	int fd;
	int err;
	unsigned long ip;
	unsigned long rip;
	unsigned long netmask;
	strcpy(ifr.ifr_name, target);

	WaitSemaphore(g_semGetIP);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) //获取IP
	{
		//perror("ioctl");
		close(fd);

		SignalSemaphore(g_semGetIP);

		return 0;
	}
	memcpy(&ip, ifr.ifr_addr.sa_data+2, 4);

	strcpy(ifr.ifr_name, target);
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)  //获取子网掩码
	{
		close(fd);

		SignalSemaphore(g_semGetIP);

		return 0;
	}    
	memcpy(&netmask, ifr.ifr_netmask.sa_data+2, 4);


	strcpy(ifr.ifr_name, target);
	if (ioctl(fd, SIOCGIFDSTADDR, &ifr) < 0) //获取网关，在自动获取的时候获取到的网关
	{
		close(fd);

	    SignalSemaphore(g_semGetIP);

		return 0;
	}    
	memcpy(&rip, ifr.ifr_dstaddr.sa_data+2, 4);

	close(fd);

	SignalSemaphore(g_semGetIP);

	if (ip == 0 || 0==rip || 0==netmask|| 0xffffffff==netmask) 
		return 0;

	DTRACE(DB_CRITICAL, ("local ip is %d.%d.%d.%d, remote ip is %d.%d.%d.%d.\n", \
		ip&0xff, (ip>>8)&0xff, (ip>>16)&0xff, (ip>>24)&0xff, \
		rip&0xff, (rip>>8)&0xff, (rip>>16)&0xff, (rip>>24)&0xff));

     *pBuf++ = (BYTE)ip&0xff;
	 *pBuf++ = (BYTE)(ip>>8)&0xff ;
	 *pBuf++ = (BYTE)(ip>>16)&0xff ;
	 *pBuf++ = (BYTE)(ip>>24)&0xff ;

	 *pBuf++ = (BYTE)netmask&0xff;
	 *pBuf++ = (BYTE)(netmask>>8)&0xff ;
	 *pBuf++ = (BYTE)(netmask>>16)&0xff ;
	 *pBuf++ = (BYTE)(netmask>>24)&0xff ;
	
	/*if (ip == rip)
	{
	DTRACE(DB_CRITICAL, ("local ip is the same as remote ip.\n"));
	return 
	-1;
	}*/
	return ip; //直接返回网关IP
}

//检查网线是否已插好
//正常返回1，没插网线返回－1
int GetNetStat()  
{
		int skfd;
		struct ifreq ifr;
		struct ethtool_value edata;

		edata.cmd = ETHTOOL_GLINK;
		edata.data = 0;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name) - 1);
		ifr.ifr_data = (char *) &edata;

		if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
			return -1;

		if(ioctl( skfd, SIOCETHTOOL, &ifr ) == -1)
		{
			close(skfd);
			return -1;
		}

		close(skfd);
		return edata.data;
	
	

	/*
	char    buffer[BUFSIZ];
	FILE    *read_fp = NULL;
	int        chars_read;
	int        iret;

	memset( buffer, 0, BUFSIZ );

	WaitSemaphore(g_semNetStat);  //申请信号量

	read_fp = popen("ifconfig eth0 | grep RUNNING", "r");
	if ( read_fp != NULL ) 
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);
		if (chars_read > 0) 
		{
			iret = 1;
		}
		else
		{
			iret = -1;
		}
		pclose(read_fp);
	}
	else
	{
		iret = -1;
	}

	SignalSemaphore(g_semNetStat);

	return iret;
	*/
}

//检查驱动的版本
//正常返回1，没插网线返回－1
int GetNetMacbStat()  
{
		int skfd;
		struct ifreq ifr;
		struct ethtool_drvinfo drvinfo;

		memset(&drvinfo, 0, sizeof(drvinfo));
		drvinfo.cmd = ETHTOOL_GDRVINFO;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name) - 1);
		ifr.ifr_data = (char *) &drvinfo;

		if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
			return -1;

		if(ioctl( skfd, SIOCETHTOOL, &ifr ) == -1)
		{
			close(skfd);
			return -1;
		}
		
		char	version[32];	/* driver version string */
		memset(version, 0, sizeof(version));
		strcpy(version, drvinfo.version);
		if (strstr(version, "error") != NULL)
		{
			DTRACE(0, ("macb_drvinfo_version: %s\r\n", version));
			return 1;
		}
		close(skfd);
		return 0;
}


//描述：检查udhcpc命令是否已执行完毕
//返回：-1执行完毕，大于0还在执行
int CheckDhcpStat()
{
	char    buffer[1000];//BUFSIZ
	FILE    *read_fp;
	int        chars_read;
	int        iret;
	int   iCnt = 1;
	string str ="";
	memset( buffer, 0, 1000 );

	read_fp = popen("ps -ef | grep udhcpc", "r");
	if ( read_fp != NULL ) 
	{
		chars_read = fread(buffer, sizeof(char), 1000-1, read_fp);
		if (chars_read > 0) 
		{
			buffer[999]='\0';
			str.append(buffer);
			printf("CheckDhcpStat grep udhcpc1: %s .\r\n", str.c_str());
			iret =  str.find("grep udhcpc");

			while(iret>0)
			{
				iCnt++;
				str.erase(iret,11);
				iret =  str.find("grep udhcpc");
				printf("CheckDhcpStat grep udhcpc%d: %s.\r\n", iCnt, str.c_str());
			}

			iret = 0;
			printf("CheckDhcpStat grep udhcpc%d: %s.\r\n", iCnt, str.c_str());
			iret =  str.find("udhcpc -n");
			printf("CheckDhcpStat grep udhcpc%d: %d.\r\n", iCnt, iret); 
		}
		else
		{
			iret = -1;
		}
		pclose(read_fp);
	}
	else
	{
		iret = -1;
	}

	return iret;
}



//描述：执行udhcpc命令获取IP
int DoUdhcpc()
{
	if(CheckDhcpStat()>0)
		DTRACE(SOCKETS_DEBUG,("------DoUdhcpc:udhcpc is Running!------\r\n"));
	else
	{
		DTRACE(SOCKETS_DEBUG,("------DoUdhcpc:udhcpc Start ------\r\n"));
		system("udhcpc -q -n&");  
	}

	return 0;
}

bool GetNetPara(BYTE bType,  BYTE* pbBuf)
{
	BYTE bBuf[128];
	memset(bBuf,0,sizeof(bBuf));
	unsigned long UGetWay = 0;
	unsigned long UMask = 0;
	unsigned long UIp = 0;
	BYTE bNetContTye = 0;
	//ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //以太网连接方式
	bNetContTye = GetEthConfigType();


	if (bNetContTye == ETH_IP_CFG_PPPoE)
	{
		IfReadListProc("ppp0", bBuf);//获取ip和掩码
		UGetWay = GetDefaultGateWay(); //获取默认网关
		bBuf[8]= UGetWay&0xff;
		bBuf[9]= (UGetWay>>8)&0xff;
		bBuf[10]=(UGetWay>>16)&0xff;
		bBuf[11]=(UGetWay>>24)&0xff;	
	}
	else if(bNetContTye == ETH_IP_CFG_STATIC)
	{
	#ifdef PRO_698
		ReadItemEx(BN0, PN0, 0x007f, bBuf);//读默认网关
	#else
		ReadItemEx(BN10, PN0, 0xa15f, bBuf);//读默认网关
		memcpy(bBuf, bBuf+6, 12);
	#endif
	}
	else
	{
		IfReadListProc("eth0", bBuf);//获取ip和掩码
		UGetWay = GetDefaultGateWay(); //获取默认网关
		bBuf[8]= UGetWay&0xff;
		bBuf[9]= (UGetWay>>8)&0xff;
		bBuf[10]=(UGetWay>>16)&0xff;
		bBuf[11]=(UGetWay>>24)&0xff;
	}

	switch(bType)
	{
	case 1: //返回IP
		memcpy(pbBuf, bBuf, 4);
		break;

	case 2: //返回子网掩码
		memcpy(pbBuf, bBuf+4, 4);
		break;

	case 3: //返回默认网关
		memcpy(pbBuf, bBuf+8, 4);
		break;

	case 4: //以上3项
		memcpy(pbBuf, bBuf, 12);
		break;

	default:
		break;
	}

	return true;
}

#else
unsigned long GetDefaultGateWay()
{
	return inet_addr("127.0.0.1");
}

int Ping()
{
	return 1;
}

int PingByICMP(unsigned long Ip) 
{
	return 1;
}

int GetNetStat()
{
	return 1;
}

int CheckDhcpStat()
{
	return 1;
}

int DoUdhcpc()
{
	return 1;
}

bool GetNetPara(BYTE bType, BYTE* pbBuf)
{
	return true;
}
#endif
extern unsigned long GetDefaultGateWay();

//初始化route命令
void InitRouteCmd()
{
	BYTE bBuf[8];
	BYTE bNetContTye = 0;
	unsigned long rip; //默认网关

	memset(&RouteCmd,0,sizeof(RouteCMD));
	memset(bBuf,0,sizeof(bBuf));

	bNetContTye = GetEthConfigType();	//以太网连接方式

	GetEthGataway(bBuf);	//读默认网关

	if (bNetContTye == ETH_IP_CFG_PPPoE)
	{     
		rip = GetDefaultGateWay();  //自动获取ip模式取实际网关
		DTRACE(SOCKETS_DEBUG, ("InitRouteCmd  GetDefaultGateWay() :0x%8x \r\n",rip)); //ping默认网关

		if(rip>0 && rip!=0xffffffff) //255.255.255.255当无效地址
		{              
			bBuf[0]= rip&0xff;
			bBuf[1]= (rip>>8)&0xff;
			bBuf[2]=(rip>>16)&0xff;
			bBuf[3]=(rip>>24)&0xff;
		} 
		else
			memset(&bBuf[0], 0x01, 4);
	}
	else if(bNetContTye == ETH_IP_CFG_DHCP)  //是否自动获取
	{     
		rip = GetDefaultGateWay();  //自动获取ip模式取实际网关
		DTRACE(SOCKETS_DEBUG, ("InitRouteCmd  GetDefaultGateWay() :0x%8x \r\n",rip)); //ping默认网关

		if(rip>0 && rip!=0xffffffff) //255.255.255.255当无效地址
		{              
			bBuf[0]= rip&0xff;
			bBuf[1]= (rip>>8)&0xff;
			bBuf[2]=(rip>>16)&0xff;
			bBuf[3]=(rip>>24)&0xff;
		} 
		else
			memset(&bBuf[0], 0x01, 4);
	}
	else
	{
		if (IsAllAByte(bBuf, 0x00, sizeof(bBuf)))
			memset(&bBuf[0], 0x01, 4); //默认网关不能全为0
	}

	sprintf(RouteCmd.PingCmd, "ping -c 1 %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //ping默认网关
	sprintf(RouteCmd.AddRoute, "route add default gw %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //添加以默认太网关
	sprintf(RouteCmd.DelRoute, "route del default gw %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //删除以默认太网关
}

//初始化ping命令
void InitPingCMD()
{
	memset(&(RouteCmd.PingCmd),0,sizeof(RouteCmd.PingCmd));
	unsigned long iRip = GetDefaultGateWay();
	if (iRip==0 || iRip==0xffffffff)
	{
		BYTE bGw[15];
		BYTE bNetContTye = 0;
		//ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //以太网连接方式
		bNetContTye = GetEthConfigType();

		if (bNetContTye==ETH_IP_CFG_DHCP)
		{
			GetNetPara(3, bGw);
			memcpy((BYTE*)&iRip, bGw, 4);
		}
	}
	BYTE bBuf[4]={1};//不能全为0
	DTRACE(SOCKETS_DEBUG, ("In InitPingCMD GetDefaultGateWay return:0x%4x \r\n",iRip)); //ping默认网关
	if(iRip>0 && iRip!=0xffffffff)
	{
		bBuf[0]= iRip&0xff;
		bBuf[1]= (iRip>>8)&0xff;
		bBuf[2]=(iRip>>16)&0xff;
		bBuf[3]=(iRip>>24)&0xff;
		memcpy(DefGWPara.bGateWay, bBuf, sizeof(bBuf));
	}
	else
		memset(&bBuf, 0x01, 4);

	if(IsAllAByte(bBuf,0,sizeof(bBuf))) //默认网关不能为0
		memset(bBuf,1,sizeof(bBuf));

	sprintf(RouteCmd.PingCmd, "ping -c 1 %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //ping默认网关
	sprintf(RouteCmd.AddRoute, "route add default gw %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //添加以默认太网关
	sprintf(RouteCmd.DelRoute, "route del default gw %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //删除以默认太网关
}

//是否自动获取IP模式
bool IsAutoGetIP() 
{
	BYTE bBuf[128];
	memset(bBuf,0,sizeof(bBuf));

#ifdef PRO_698
	ReadItemEx(BN0, PN0, 0x007f, bBuf);//读默认网关
#else
	ReadItemEx(BN10, PN0, 0xa15f, bBuf);//读默认网关
	memcpy(bBuf, bBuf+6, 12);
#endif



	if (IsAllAByte(bBuf, 0x00, 12)) 
		return true;
	else
		return false;
}

int GetEthernetStat()
{
	BYTE bPingRet = 0;

	ReadItemEx(BN2, PN0, 0x2052, &bPingRet); //更改登陆类型为以太网	
	if(bPingRet>0)
		return bPingRet;
	else
		return -1;
}

//描述：检查pppoe命令是否已执行完毕
//返回：-1执行完毕，1还在执行
int CheckPppoeStat()
{
	char    buffer[1000];//BUFSIZ
	int        iret = -1;
	string str ="";
	memset( buffer, 0, 1000 );

#ifdef SYS_LINUX
	read_fp = popen("ps -ef | grep pppoe -I eth0", "r");
	if ( read_fp != NULL ) 
	{
		chars_read = fread(buffer, sizeof(char), 1000-1, read_fp);
		if (chars_read > 0) 
		{
			buffer[999]='\0';
			str.append(buffer);
			printf("CheckPppoeStat grep pppoe1: %s .\r\n", str.c_str());
			iret =  str.find("grep pppoe -I eth0");

			while(iret>0)
			{
				iCnt++;
				str.erase(iret,18);
				iret =  str.find("grep pppoe -I eth0");
				printf("CheckPppoeStat grep pppoe%d: %s.\r\n", iCnt, str.c_str());
			}

			iret = 0;
			printf("CheckPppoeStat grep pppoe%d: %s.\r\n", iCnt, str.c_str());
			iret =  str.find("pppoe -I eth0");
			printf("CheckPppoeStat grep pppoe%d: %d.\r\n", iCnt, iret); 
		}
		else
		{
			iret = -1;
		}
		pclose(read_fp);
	}
	else
	{
		iret = -1;
	}
#endif

	return iret;
}

bool OpenPppoe()
{		
#ifdef SYS_LINUX
	if(CheckPppoeStat() >= 0)
	{
		DTRACE(SOCKETS_DEBUG,("------OpenPppoe: is Running! ------\r\n"));
		return false;
	}
	else
		DTRACE(SOCKETS_DEBUG,("------OpenPppoe : Start------\r\n"));

	int nCnt = 0;
	WORD wConnectWait = 30;
	char command[256], szPppoeUser[64], szPppoePsw[64];

	memset(szPppoeUser, 0, sizeof(szPppoeUser));
	memset(szPppoePsw, 0, sizeof(szPppoePsw));
	GetEthPPPoEUserName(szPppoeUser);
	GetEthPPPoEUserPwd(szPppoePsw);

	if (szPppoeUser[0]=='\0')
	{
		strcpy(szPppoeUser, "CARD");	
	}

	if (szPppoePsw[0]=='\0')
	{
		strcpy(szPppoePsw, "CARD");
	}

  	memset(command, 0, sizeof(command));
  	sprintf(command, "/clou/ppp/script/pppoe-on %s %s", szPppoeUser, szPppoePsw);
  	DTRACE(DB_FAPROTO, ("OpenPppoe : command : %s.\r\n", command));

  	wConnectWait += 10;
  	for (WORD i=0; i<2; i++)
  	{
  		DTRACE(DB_FAPROTO, ("OpenPppoe: connect try %d\n", i));
  		system(command);
  		for (WORD j=0; j<wConnectWait; j++)
  		{
  			Sleep(1000);
  			if (IfReadListProc("ppp0") > 0)
  			{
  				Sleep(5000);
  				return true;
  			}
  		}
  	}
#endif //SYS_LINUX

	return false;	
}

void AddDefaultGateWay()
{
	DTRACE(SOCKETS_DEBUG, ("AddDefaultGateWay: %s\n", RouteCmd.AddRoute));

	system(RouteCmd.AddRoute);//删除以默认太网关
}

void DelDefaultGateWay()
{
	DTRACE(SOCKETS_DEBUG, ("DelDefaultGateWay: %s\n", RouteCmd.DelRoute));

	system(RouteCmd.DelRoute);//删除以默认太网关
	system("route del default netmask 0.0.0.0 eth0"); //删除默认网关，否则GPRS不能上线
	system("route del default netmask 255.255.255.0 eth0"); //删除默认网关，否则GPRS不能上线
}

bool g_fExecPingCmd = false;
//置位ping命令执行过
void SetExecPingCMD( bool fPing)
{
	g_fExecPingCmd = fPing;
}

//ping命令是否执行过
bool IsExecPingCMD()
{
	return g_fExecPingCmd;
}

BYTE g_bDownConnMode = 0; //0:无效1:GPRS 2:以太网
//设置当前下载通道方式
void SetDownConnMode(BYTE bConnMode)
{
	if (IsDownSoft())
	{
		if (g_bDownConnMode == 0)  //首次进来记录下方式
			g_bDownConnMode = bConnMode;
	}
}

//获取当前下载通道方式
BYTE GetDownConnMode()
{
	return g_bDownConnMode;
}

