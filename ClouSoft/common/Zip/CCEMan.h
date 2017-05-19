#ifndef _CCEMAN_H_
#define _CCEMAN_H_
#if defined(__cplusplus)
extern "C"{
#endif

#define MAXSBUFLEN 1024

#define EXE_COMPRESS_NEW	0x01//ÐÂµÄÑ¹Ëõ 

#include "CompressFunNew.h"

short EnData(BYTE * DataBuf, short DataLen, unsigned char Oper);
short DeData(BYTE * DataBuf, short DataLen);

unsigned char SendBuf[MAXSBUFLEN];
unsigned char RecvBuf[MAXSBUFLEN];

short FormFrame(unsigned char Oper, unsigned char * buf,short buflen);

#if defined(__cplusplus)
}
#endif

#endif //(_CCEMAN_H_)
