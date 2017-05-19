#ifndef _COMPRESS_NEW_
#define _COMPRESS_NEW_


#define NN       2048    /* buffer size */
#define FF       60  /* lookahead buffer size */
#define THRESHOLD   2
#define NIL     NN   /* leaf of tree */

#define N_CHAR      (256 - THRESHOLD + FF)
                /* kinds of characters (character code = 0..N_CHAR-1) */
#define TT       (N_CHAR * 2 - 1)    /* size of table */
#define RR       (TT - 1)         /* position of root */
#define MAX_FREQ    0x8000      /* updates tree when the */


#include "CrypFun.h"

short Expand (DATA * buffer);
short Compress (DATA * buffer);
		
DATA inbuffer, outbuffer;
unsigned long textsize, codesize, printcount;
	
short Encode ();
short Decode ();
void InitVar ();

unsigned char text_buf[NN + FF - 1];
short match_position, match_length, lson[NN + 1], rson[NN + 257], dad[NN + 1];
unsigned short freq[TT + 1];		/* frequency table */
short prnt[TT + N_CHAR];		/* pointers to parent nodes, except for the */
	    /* elements [TT..TT + N_CHAR - 1] which are used to get */
            /* the positions of leaves corresponding to the codes. */

short son[TT];			/* pointers to child nodes (son[], son[] + 1) */

unsigned short getbuf;
unsigned char getlen;
unsigned long incount, outcount;
unsigned short putbuf;
unsigned char putlen;
unsigned short code0, len;

void Error (char *message);
void freememory ();
void InitTree ();
void InsertNode (short r);
void DeleteNode (short p);
short GetBit ();
short GetByte ();
void Putcode (short l, unsigned short c);
void StartHuff ();
void reconst ();
void update (short c);
void EncodeChar (unsigned short c);
void EncodePosition (unsigned short c);
void EncodeEnd ();
short DecodeChar ();
short DecodePosition ();
#endif
