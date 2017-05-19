/*********************************************************************************************************
* Copyright (c) 2010,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：GetIpAPI.cpp
* 摘    要：本文件主要实现在自动IP模式下获取系统默认网关，在手动设置模式下可以直接读取参数
* 当前版本：1.0
* 原作者：杨进
*  标准化：陈鹏举

* 完成日期：2010年9月


* 当前版本：2.0
* 作者：陈鹏举
* 新增内容：通过套接字使用ICMP协议实现ping功能  
* 完成日期：2010年12月
********************************************************************************************************/
#include "GetIpAPI.h"using namespace std;
#include "apptypedef.h"
#include "LibDbAPI.h"

int INET_rresolve(char *name, size_t len, struct sockaddr_in *s_in,
				  int numeric, unsigned int netmask)
{
	struct hostent *ent;
	struct netent *np;
	struct addr *pn;
	unsigned long ad, host_ad;
	int host = 0;

	/* Grmpf. -FvK */
	if (s_in->sin_family != AF_INET) {
#ifdef DEBUG
		printf("rresolve: unsupport address family %d !",
				  s_in->sin_family);
#endif
		errno = EAFNOSUPPORT;
		return (-1);
	}
	ad = (unsigned long) s_in->sin_addr.s_addr;
#ifdef DEBUG
	printf("rresolve: %08lx, mask %08x, num %08x", ad, netmask, numeric);
#endif
	if (ad == INADDR_ANY) {
		if ((numeric & 0x0FFF) == 0) {
			if (numeric & 0x8000)
				strncpy(name, INET_default, len);
			else
				strncpy(name, "*", len);
			return (0);
		}
	}
	if (numeric & 0x0FFF) {
		strncpy(name, inet_ntoa(s_in->sin_addr), len);
		return (0);
	}

	if ((ad & (~netmask)) != 0 || (numeric & 0x4000))
		host = 1;

	pn = INET_nn;
	while (pn != NULL) {
		if (pn->addr.sin_addr.s_addr == ad && pn->host == host) {
			strncpy(name, pn->name, len);
			return (0);
		}
		pn = pn->next;
	}

	host_ad = ntohl(ad);
	np = NULL;
	ent = NULL;
	if (host) {
		ent = gethostbyaddr((char *) &ad, 4, AF_INET);
		if (ent != NULL) {
			strncpy(name, ent->h_name, len);
		}
	} else {
		np = getnetbyaddr(host_ad, AF_INET);
		if (np != NULL) {
			strncpy(name, np->n_name, len);
		}
	}
	if ((ent == NULL) && (np == NULL)) {
		strncpy(name, inet_ntoa(s_in->sin_addr), len);
	}
	pn = (struct addr *) malloc(sizeof(struct addr));
	pn->addr = *s_in;
	pn->next = INET_nn;
	pn->host = host;
	pn->name = strdup(name);
	INET_nn = pn;

	return (0);
}

void set_flags(char *flagstr, int flags)
{
	int i;

	*flagstr++ = 'U';

	for (i=0 ; (*flagstr = flagchars[i]) != 0 ; i++) {
		if (flags & flagvals[i]) {
			++flagstr;
		}
	}
}

void displayroutes(int noresolve, int netstatfmt, char* pSgw, char *name)
{
	char devname[64], flags[16], sdest[16], sgw[16];
	unsigned long int d, g, m;
	int flgs, ref, use, metric, mtu, win, ir;
	struct sockaddr_in s_addr;
	struct in_addr mask;

	FILE *fp = fopen("/proc/net/route", "r");
	if (fp == NULL)
		return ;
/*
	printf("Kernel IP routing table\n"
			  "Destination     Gateway         Genmask"
			  "         Flags %s Iface\n",
			  netstatfmt ? "  MSS Window  irtt" : "Metric Ref    Use");
*/
	if (fscanf(fp, "%*[^\n]\n") < 0) { /* Skip the first line. */
		goto ERROR;		   /* Empty or missing line, or read error. */
	}
	while (1) {
		int r;
		r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
				   devname, &d, &g, &flgs, &ref, &use, &metric, &m,
				   &mtu, &win, &ir);
		if (r != 11) {
			if ((r < 0) && feof(fp)) { /* EOF with no (nonspace) chars read. */
				break;
			}
		ERROR:
			printf("fscanf");
		}

		if (!(flgs & RTF_UP)) { /* Skip interfaces that are down. */
			continue;
		}

		set_flags(flags, (flgs & IPV4_MASK));
		if (flgs & RTF_REJECT) {
			flags[0] = '!';
		}

		memset(&s_addr, 0, sizeof(struct sockaddr_in));
		s_addr.sin_family = AF_INET;
		s_addr.sin_addr.s_addr = d;
		INET_rresolve(sdest, sizeof(sdest), &s_addr,
					  (noresolve | 0x8000), m);	/* Default instead of *. */

		s_addr.sin_addr.s_addr = g;
		INET_rresolve(sgw, sizeof(sgw), &s_addr,
					  (noresolve | 0x4000), m);	/* Host instead of net. */

		mask.s_addr = m;
		//printf("%-16s%-16s%-16s%-6s", sdest, sgw, inet_ntoa(mask), flags);
     	       
      if(devname[0]==name[0] && devname[1]==name[1] && devname[2]==name[2] && devname[3]==name[3])
      { 
      	memcpy(pSgw,sgw,sizeof(sgw));
		if (pSgw[0] != '*')
      		break;
      }
/*
		if (netstatfmt) {
			printf("%5d %-5d %6d %s\n", mtu, win, ir, devname);
		} else {
			printf("%-6d %-2d %7d %s\n", metric, ref, use, devname);
		}
*/
	}
	if(fp != NULL)
	   fclose(fp);
}
//获取默认以太网关
//返回网关IP
unsigned long GetDefaultGateWay()
{
   unsigned long  iIp= 0;
   char cGateWay[16] = {0};
   memset(cGateWay,0x00,sizeof(cGateWay));
	BYTE bNetContTye = 0;
	ReadItemEx(BN10, PN0, 0xa1b6, &bNetContTye);
	WaitSemaphore(g_semGateWay);
	if (bNetContTye != 0)
		displayroutes(0, 0,cGateWay, "ppp0");
	else
		displayroutes(0, 0,cGateWay, "eth0");
	SignalSemaphore(g_semGateWay);
  
  printf("-------------displayroutes return char :%s--------------\r\n",cGateWay);
   
   if('*'== cGateWay[0]) //
      return 0;
   iIp = inet_addr(cGateWay);
   
	return iIp;
}

/*int main()
{        char pSgw[16];
	displayroutes(0, 0,pSgw);
	return 0;
}*/

// 效验算法
unsigned short cal_chksum(unsigned short *addr, int len)
{
	int nleft=len;
	int sum=0;
	unsigned short *w=addr;
	unsigned short answer=0;

	while(nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	if( nleft == 1)
	{       
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	return answer;
}

//add by CPJ at 2010-12-15
//描述：Linux系统中通过标准ICMP协议实现ping功能
//参数：4字节IP地址
//成功返回1，失败返回－1
int PingByICMP(unsigned long Ip)
{
	int timeout = 1500; //毫秒
	char *ips = NULL;
	unsigned long inaddr = Ip; 
	struct timeval timeo;
	int sockfd;
	struct sockaddr_in addr;
	struct sockaddr_in from;

	struct timeval *tval;
	struct ip *iph;
	struct icmp *icmp;

	char sendpacket[PACKET_SIZE];
	char recvpacket[PACKET_SIZE];

	int n;
	pid_t pid;
	int maxfds = 0;
	fd_set readfds;

	// 设定Ip信息
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	memcpy(&addr.sin_addr, &inaddr, sizeof(inaddr));
	ips =  inet_ntoa(addr.sin_addr);

	if(ips==NULL)
		return PING_ERROR;

	DTRACE(SOCKETS_DEBUG, ("--------PingByICMP:  Ping -->:%s Start !.---------\r\n", ips));	

	// 取得socket
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) 
	{
		//printf("~~~ socket  Error\n");
		DTRACE(SOCKETS_DEBUG, ("--------PingByICMP:  socket open failed.---------\r\n"));	
		return PING_ERROR;
	}

	// 设定TimeOut时间
	timeo.tv_sec = timeout / 1000;
	timeo.tv_usec = timeout % 1000;

	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo)) == -1)
	{
		// printf("~~~ setsockopt  Error\n");
		DTRACE(SOCKETS_DEBUG, ("--------PingByICMP:  setsockopt  failed.---------\r\n"));

		// 关闭socket
		close(sockfd);
		return PING_ERROR;
	}

	//设定Ping包 
	memset(&sendpacket[0], 0x00, PACKET_SIZE);

	// 取得PID，作为Ping的Sequence ID
	pid=getpid();
	int i,packsize, loop=0, iSucCnt=0;
	icmp=(struct icmp*)sendpacket;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_seq=0;
	icmp->icmp_id = pid;
	packsize = 8+56;
	
	//发5个包，成功3此即认为网络OK
	for(loop; loop<5; loop++)
	{
		icmp->icmp_seq = loop;
		tval= (struct timeval *)icmp->icmp_data;
		gettimeofday(tval,NULL);
		icmp->icmp_cksum=cal_chksum((unsigned short *)icmp,packsize);

		// 发包
		n = sendto(sockfd, (char *)&sendpacket, packsize, 0, (struct sockaddr *)&addr, sizeof(addr));
		if (n < 1)
		{
			DTRACE(SOCKETS_DEBUG, ("--------PingByICMP:  sendto  ERROR.---------\r\n"));	
			// 关闭socket
			close(sockfd);
			return PING_ERROR;
		}

		// 接受
		// 由于可能接受到其他Ping的应答消息，所以这里要用循环
		while(1)
		{
			// 设定TimeOut时间，这次才是真正起作用的
			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);
			maxfds = sockfd + 1;
			n = select(maxfds, &readfds, NULL, NULL, &timeo);
			if (n <= 0)
			{
				DTRACE(SOCKETS_DEBUG, ("--------PingByICMP:  select  ERROR.---------\r\n"));	
				//close(sockfd);
				//return PING_ERROR;
				break;		
			}

			// 接收
			memset(recvpacket, 0, sizeof(recvpacket));
			int fromlen = sizeof(from);
			n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from,  (socklen_t* )&fromlen);
			if (n < 1) 
			{
				DTRACE(SOCKETS_DEBUG, ("--------PingByICMP:  recvfrom  ERROR.---------\r\n"));	
				break;
			}

			// 判断是否是自己Ping的回复
			char *from_ip = (char *)inet_ntoa(from.sin_addr);

			if(from_ip==NULL)
				break;

			if (strcmp(from_ip,ips) != 0)
			{
				DTRACE(SOCKETS_DEBUG, ("--------PingByICMP:: RX ip :%s, but local ip: %s,    ERROR--------\r\n" ,from_ip, ips));	 
				break;
			}

			iph = (struct ip *)recvpacket;
			icmp=(struct icmp *)(recvpacket + (iph->ip_hl<<2));

			// 判断Ping回复包的状态
			if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_id == pid)
			{
				// 正常就跳出当前循环
                iSucCnt++;
				DTRACE(SOCKETS_DEBUG, ("--------PingByICMP:: Ping -->: %s OK  %d  times---------\r\n" ,ips,iSucCnt));	
				break;
			} 
			else
			{
				// 否则继续等
				continue;
			}
		}
	}
	// 关闭socket
	close(sockfd);

	if(iSucCnt>=3)
     	return SUCCESS;
    else 
		return PING_ERROR;
}

