 /*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DrvAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֱ�ϵͳ���豸��������ķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009��7��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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

//pingĬ������
//��ʱ����-1
int Ping()
{

	return -1;

	int iCnt=-1,iResult=-1;
	/*BYTE bBuf[128];
	char szTmp[128];
	memset(bBuf,0,sizeof(bBuf));
	memset(szTmp,0,sizeof(szTmp));
	ReadItemEx(0, 0, 0x007f, bBuf);//��Ĭ������*/

	if(GetNetStat()==1)
		DTRACE(SOCKETS_DEBUG,("---%s----\r\n",RouteCmd.PingCmd));
	else
	{
		DTRACE(SOCKETS_DEBUG,("-----Ping fail due to Ethernet is not alive!!!!---"));
		return -2;
	}
	//sprintf(szTmp, "ping -c 1 %d.%d.%d.%d", bBuf[8], bBuf[9], bBuf[10], bBuf[11]);
	//string tStrRet = cmd_system("ping -c 1 10.98.96.7");//���ط�����Ϣ
	//string tStrRet = cmd_system(RouteCmd.PingCmd);//���ط�����Ϣ
	//i = tStrRet.find("bytes from");

	for(int i=0; i<4; i++)
	{
		string tStrRet = cmd_system(RouteCmd.PingCmd);//���ط�����Ϣ
		iResult = tStrRet.find("bytes from");
		if(iResult>=0)
			iCnt++;
	}

	//printf("iCnt  = %d\r\n", iCnt );
	if (iCnt >= 2)//ping4������ͨ3��
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
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) //��ȡIP
	{
		//perror("ioctl");
		close(fd);

		SignalSemaphore(g_semGetIP);

		return 0;
	}
	memcpy(&ip, ifr.ifr_addr.sa_data+2, 4);

	strcpy(ifr.ifr_name, target);
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)  //��ȡ��������
	{
		close(fd);

		SignalSemaphore(g_semGetIP);

		return 0;
	}    
	memcpy(&netmask, ifr.ifr_netmask.sa_data+2, 4);


	strcpy(ifr.ifr_name, target);
	if (ioctl(fd, SIOCGIFDSTADDR, &ifr) < 0) //��ȡ���أ����Զ���ȡ��ʱ���ȡ��������
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
	return ip; //ֱ�ӷ�������IP
}

//��������Ƿ��Ѳ��
//��������1��û�����߷��أ�1
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

	WaitSemaphore(g_semNetStat);  //�����ź���

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

//��������İ汾
//��������1��û�����߷��أ�1
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


//���������udhcpc�����Ƿ���ִ�����
//���أ�-1ִ����ϣ�����0����ִ��
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



//������ִ��udhcpc�����ȡIP
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
	//ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //��̫�����ӷ�ʽ
	bNetContTye = GetEthConfigType();


	if (bNetContTye == ETH_IP_CFG_PPPoE)
	{
		IfReadListProc("ppp0", bBuf);//��ȡip������
		UGetWay = GetDefaultGateWay(); //��ȡĬ������
		bBuf[8]= UGetWay&0xff;
		bBuf[9]= (UGetWay>>8)&0xff;
		bBuf[10]=(UGetWay>>16)&0xff;
		bBuf[11]=(UGetWay>>24)&0xff;	
	}
	else if(bNetContTye == ETH_IP_CFG_STATIC)
	{
	#ifdef PRO_698
		ReadItemEx(BN0, PN0, 0x007f, bBuf);//��Ĭ������
	#else
		ReadItemEx(BN10, PN0, 0xa15f, bBuf);//��Ĭ������
		memcpy(bBuf, bBuf+6, 12);
	#endif
	}
	else
	{
		IfReadListProc("eth0", bBuf);//��ȡip������
		UGetWay = GetDefaultGateWay(); //��ȡĬ������
		bBuf[8]= UGetWay&0xff;
		bBuf[9]= (UGetWay>>8)&0xff;
		bBuf[10]=(UGetWay>>16)&0xff;
		bBuf[11]=(UGetWay>>24)&0xff;
	}

	switch(bType)
	{
	case 1: //����IP
		memcpy(pbBuf, bBuf, 4);
		break;

	case 2: //������������
		memcpy(pbBuf, bBuf+4, 4);
		break;

	case 3: //����Ĭ������
		memcpy(pbBuf, bBuf+8, 4);
		break;

	case 4: //����3��
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

//��ʼ��route����
void InitRouteCmd()
{
	BYTE bBuf[8];
	BYTE bNetContTye = 0;
	unsigned long rip; //Ĭ������

	memset(&RouteCmd,0,sizeof(RouteCMD));
	memset(bBuf,0,sizeof(bBuf));

	bNetContTye = GetEthConfigType();	//��̫�����ӷ�ʽ

	GetEthGataway(bBuf);	//��Ĭ������

	if (bNetContTye == ETH_IP_CFG_PPPoE)
	{     
		rip = GetDefaultGateWay();  //�Զ���ȡipģʽȡʵ������
		DTRACE(SOCKETS_DEBUG, ("InitRouteCmd  GetDefaultGateWay() :0x%8x \r\n",rip)); //pingĬ������

		if(rip>0 && rip!=0xffffffff) //255.255.255.255����Ч��ַ
		{              
			bBuf[0]= rip&0xff;
			bBuf[1]= (rip>>8)&0xff;
			bBuf[2]=(rip>>16)&0xff;
			bBuf[3]=(rip>>24)&0xff;
		} 
		else
			memset(&bBuf[0], 0x01, 4);
	}
	else if(bNetContTye == ETH_IP_CFG_DHCP)  //�Ƿ��Զ���ȡ
	{     
		rip = GetDefaultGateWay();  //�Զ���ȡipģʽȡʵ������
		DTRACE(SOCKETS_DEBUG, ("InitRouteCmd  GetDefaultGateWay() :0x%8x \r\n",rip)); //pingĬ������

		if(rip>0 && rip!=0xffffffff) //255.255.255.255����Ч��ַ
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
			memset(&bBuf[0], 0x01, 4); //Ĭ�����ز���ȫΪ0
	}

	sprintf(RouteCmd.PingCmd, "ping -c 1 %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //pingĬ������
	sprintf(RouteCmd.AddRoute, "route add default gw %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //�����Ĭ��̫����
	sprintf(RouteCmd.DelRoute, "route del default gw %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //ɾ����Ĭ��̫����
}

//��ʼ��ping����
void InitPingCMD()
{
	memset(&(RouteCmd.PingCmd),0,sizeof(RouteCmd.PingCmd));
	unsigned long iRip = GetDefaultGateWay();
	if (iRip==0 || iRip==0xffffffff)
	{
		BYTE bGw[15];
		BYTE bNetContTye = 0;
		//ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye); //��̫�����ӷ�ʽ
		bNetContTye = GetEthConfigType();

		if (bNetContTye==ETH_IP_CFG_DHCP)
		{
			GetNetPara(3, bGw);
			memcpy((BYTE*)&iRip, bGw, 4);
		}
	}
	BYTE bBuf[4]={1};//����ȫΪ0
	DTRACE(SOCKETS_DEBUG, ("In InitPingCMD GetDefaultGateWay return:0x%4x \r\n",iRip)); //pingĬ������
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

	if(IsAllAByte(bBuf,0,sizeof(bBuf))) //Ĭ�����ز���Ϊ0
		memset(bBuf,1,sizeof(bBuf));

	sprintf(RouteCmd.PingCmd, "ping -c 1 %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //pingĬ������
	sprintf(RouteCmd.AddRoute, "route add default gw %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //�����Ĭ��̫����
	sprintf(RouteCmd.DelRoute, "route del default gw %d.%d.%d.%d", bBuf[0], bBuf[1], bBuf[2], bBuf[3]); //ɾ����Ĭ��̫����
}

//�Ƿ��Զ���ȡIPģʽ
bool IsAutoGetIP() 
{
	BYTE bBuf[128];
	memset(bBuf,0,sizeof(bBuf));

#ifdef PRO_698
	ReadItemEx(BN0, PN0, 0x007f, bBuf);//��Ĭ������
#else
	ReadItemEx(BN10, PN0, 0xa15f, bBuf);//��Ĭ������
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

	ReadItemEx(BN2, PN0, 0x2052, &bPingRet); //���ĵ�½����Ϊ��̫��	
	if(bPingRet>0)
		return bPingRet;
	else
		return -1;
}

//���������pppoe�����Ƿ���ִ�����
//���أ�-1ִ����ϣ�1����ִ��
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

	system(RouteCmd.AddRoute);//ɾ����Ĭ��̫����
}

void DelDefaultGateWay()
{
	DTRACE(SOCKETS_DEBUG, ("DelDefaultGateWay: %s\n", RouteCmd.DelRoute));

	system(RouteCmd.DelRoute);//ɾ����Ĭ��̫����
	system("route del default netmask 0.0.0.0 eth0"); //ɾ��Ĭ�����أ�����GPRS��������
	system("route del default netmask 255.255.255.0 eth0"); //ɾ��Ĭ�����أ�����GPRS��������
}

bool g_fExecPingCmd = false;
//��λping����ִ�й�
void SetExecPingCMD( bool fPing)
{
	g_fExecPingCmd = fPing;
}

//ping�����Ƿ�ִ�й�
bool IsExecPingCMD()
{
	return g_fExecPingCmd;
}

BYTE g_bDownConnMode = 0; //0:��Ч1:GPRS 2:��̫��
//���õ�ǰ����ͨ����ʽ
void SetDownConnMode(BYTE bConnMode)
{
	if (IsDownSoft())
	{
		if (g_bDownConnMode == 0)  //�״ν�����¼�·�ʽ
			g_bDownConnMode = bConnMode;
	}
}

//��ȡ��ǰ����ͨ����ʽ
BYTE GetDownConnMode()
{
	return g_bDownConnMode;
}

