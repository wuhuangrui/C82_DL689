// CrypFun.h: interface for the CCrypFun class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRYPFUN_H_
#define _CRYPFUN_H_

/*
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/

//
#ifndef DATAZIP
	typedef unsigned char BYTE;
#endif


typedef unsigned char	word8;
typedef unsigned short  word16;
typedef unsigned long	word32;

typedef struct
{
	word8 *x;
	word32 length;
}DATA;

#endif // !defined(AFX_CRYPFUN_H__1EC6029B_0E82_49BF_A978_9F0CBDDAE293__INCLUDED_)

