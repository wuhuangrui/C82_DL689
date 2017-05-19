#ifndef GETIPAPI_H
#define GETIPAPI_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <setjmp.h>
#include <arpa/inet.h>
#include <assert.h>
#include <iostream>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/route.h>
#include <net/if.h>
#include <netdb.h>
#include <errno.h>
#include "FaAPI.h"

#define PACKET_SIZE     4096
#define PING_ERROR           0
#define SUCCESS         1

using namespace std;
static const unsigned int flagvals[] = { /* Must agree with flagchars[]. */
	RTF_GATEWAY,
	RTF_HOST,
	RTF_REINSTATE,
	RTF_DYNAMIC,
	RTF_MODIFIED,
};

static const char flagchars[] =		/* Must agree with flagvals[]. */
	"GHRDM"
;

/* cache */
struct addr {
	struct sockaddr_in addr;
	char *name;
	int host;
	struct addr *next;
};

static struct addr *INET_nn = NULL;	/* addr-to-name cache           */
#define IPV4_MASK (RTF_GATEWAY|RTF_HOST|RTF_REINSTATE|RTF_DYNAMIC|RTF_MODIFIED)

const char INET_default[] = "default";
//string cmd_system(const char* vCmd);
int INET_rresolve(char *name, size_t len, struct sockaddr_in *s_in,
				  int numeric, unsigned int netmask);
void set_flags(char *flagstr, int flags);
void displayroutes(int noresolve, int netstatfmt, char* pSgw, char *name);
unsigned long GetDefaultGateWay();
int PingByICMP(unsigned long Ip);
#endif

