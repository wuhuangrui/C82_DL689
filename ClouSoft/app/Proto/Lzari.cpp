/**************************************************************
	LZARI.C -- A Data Compression Program
	(tab = 4 spaces)
***************************************************************
	4/7/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************/

/********************************************************************
	lzari.cpp -- A Data Compression Class
	created:	2004/10/04
	created:	4:10:2004   16:44
	file base:	lzari
	file ext:	cpp
	author:	阙荣文	(querw@sina.com)
	
	purpose: 如上所述,lzari.c提供了lzari压缩算法的实现,基于lzari.c我把它
			 做成了一个c++类方便使用
*********************************************************************/
#include "stdafx.h"
#include "syscfg.h"

#ifdef SYS_VDK
//#include "StdAfx.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
#include "Lzari.h"
#include "const.h"

LZARI::LZARI()
{


	textsize = 0;
	codesize = 0;
	printcount = 0;

	low = 0;
	high = LZARI_Q4;
	value = 0;
	shifts = 0;/* counts for magnifying low and high around Q2 */
	
	m_bMem = true;

	m_pInBuffer = NULL;
	m_nInLength = 0;
	m_nInCur = 0;

	//m_pOutBuffer = NULL;
	m_nOutLength = 0;
	m_nOutCur = 0;

	buffer_putbit = 0;
	mask_putbit = 128;

	buffer_getbit = 0;
	mask_getbit = 0;

}

LZARI::~LZARI()
{
	Release();
}

void LZARI::Error(char *message)
{

}

void LZARI::PutBit(int bit)  /* Output one bit (bit = 0,1) */
{
	if (bit) buffer_putbit |= mask_putbit;
	if ((mask_putbit >>= 1) == 0) 
	{
		if (!m_bMem)
		{
			//if (putc(buffer_putbit, outfile) == EOF) Error("Write Error");
		}
		else
		{
			if (m_nOutCur == m_nOutLength) Error("Write Error");
			m_pOutBuffer[m_nOutCur++] = buffer_putbit;
		//	m_OutBuffer.push_back(buffer_putbit);
			//m_nOutCur++;
		}
		buffer_putbit = 0;  
		mask_putbit = 128;  
		codesize++;
	}
}

void LZARI::FlushBitBuffer(void)  /* Send remaining bits */
{
	int  i;
	
	for (i = 0; i < 7; i++) PutBit(0);
}

int LZARI::GetBit(void)  /* Get one bit (0 or 1) */
{	
	if ((mask_getbit >>= 1) == 0) 
	{
		if (!m_bMem)
			{
				//buffer_getbit = getc(infile);
			}
		else{
			buffer_getbit = m_pInBuffer[m_nInCur++];
		}
		mask_getbit = 128;
	}
	return ((buffer_getbit & mask_getbit) != 0);
}

/********** LZSS with multiple binary trees **********/

void LZARI::InitTree(void)  /* Initialize trees */
{
	int  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = LZARI_N + 1; i <= LZARI_N + 256; i++) rson[i] = LZARI_NIL;	/* root */
	for (i = 0; i < LZARI_N; i++) dad[i] = LZARI_NIL;	/* node */
}




/********** Arithmetic Compression **********/

/*  If you are not familiar with arithmetic compression, you should read
		I. E. Witten, R. M. Neal, and J. G. Cleary,
			Communications of the ACM, Vol. 30, pp. 520-540 (1987),
	from which much have been borrowed.  */

	/* character code = 0, 1, ..., N_CHAR - 1 */


void LZARI::StartModel(void)  /* Initialize model */
{
	int ch, sym, i;
	
	sym_cum[LZARI_N_CHAR] = 0;
	for (sym = LZARI_N_CHAR; sym >= 1; sym--) 
	{
		ch = sym - 1;
		char_to_sym[ch] = sym;  sym_to_char[sym] = ch;
		sym_freq[sym] = 1;
		sym_cum[sym - 1] = sym_cum[sym] + sym_freq[sym];
	}
	sym_freq[0] = 0;  /* sentinel (!= sym_freq[1]) */
	position_cum[LZARI_N] = 0;
	for (i = LZARI_N; i >= 1; i--)
		position_cum[i - 1] = position_cum[i] + 10000 / (i + 200);
			/* empirical distribution function (quite tentative) */
			/* Please devise a better mechanism! */
}

void LZARI::UpdateModel(int sym)
{
	int i, c, ch_i, ch_sym;
	
	if (sym_cum[0] >= LZARI_MAX_CUM) 
	{
		c = 0;
		for (i = LZARI_N_CHAR; i > 0; i--) 
		{
			sym_cum[i] = c;
			c += (sym_freq[i] = (sym_freq[i] + 1) >> 1);
		}
		sym_cum[0] = c;
	}
	for (i = sym; sym_freq[i] == sym_freq[i - 1]; i--) ;
	if (i < sym) 
	{
		ch_i = sym_to_char[i];    ch_sym = sym_to_char[sym];
		sym_to_char[i] = ch_sym;  sym_to_char[sym] = ch_i;
		char_to_sym[ch_i] = sym;  char_to_sym[ch_sym] = i;
	}
	sym_freq[i]++;
	while (--i >= 0) sym_cum[i]++;
}


int LZARI::BinarySearchSym(unsigned int x)
	/* 1      if x >= sym_cum[1],
	   N_CHAR if sym_cum[N_CHAR] > x,
	   i such that sym_cum[i - 1] > x >= sym_cum[i] otherwise */
{
	int i, j, k;
	
	i = 1;  j = LZARI_N_CHAR;
	while (i < j) 
	{
		k = (i + j) / 2;
		if (sym_cum[k] > x) i = k + 1;  else j = k;
	}
	return i;
}

int LZARI::BinarySearchPos(unsigned int x)
	/* 0 if x >= position_cum[1],
	   N - 1 if position_cum[N] > x,
	   i such that position_cum[i] > x >= position_cum[i + 1] otherwise */
{
	int i, j, k;
	
	i = 1;  j = LZARI_N;
	while (i < j)
	{
		k = (i + j) / 2;
		if (position_cum[k] > x) i = k + 1;  else j = k;
	}
	return i - 1;
}

void LZARI::StartDecode(void)
{
	int i;

	for (i = 0; i < LZARI_M + 2; i++)
		value = 2 * value + GetBit();
}

int LZARI::DecodeChar(void)
{
	int	 sym, ch;
	unsigned long int  range;
	
	range = high - low;
	sym = BinarySearchSym((unsigned int)
		(((value - low + 1) * sym_cum[0] - 1) / range));
	high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
	low +=       (range * sym_cum[sym    ]) / sym_cum[0];
	for ( ; ; ) {
		if (low >= LZARI_Q2) {
			value -= LZARI_Q2;  low -= LZARI_Q2;  high -= LZARI_Q2;
		} else if (low >= LZARI_Q1 && high <= LZARI_Q3) {
			value -= LZARI_Q1;  low -= LZARI_Q1;  high -= LZARI_Q1;
		} else if (high > LZARI_Q2) break;
		low += low;  high += high;
		value = 2 * value + GetBit();
	}
	ch = sym_to_char[sym];
	UpdateModel(sym);
	return ch;
}

int LZARI::DecodePosition(void)
{
	int position;
	unsigned long int  range;
	
	range = high - low;
	position = BinarySearchPos((unsigned int)
		(((value - low + 1) * position_cum[0] - 1) / range));
	high = low + (range * position_cum[position    ]) / position_cum[0];
	low +=       (range * position_cum[position + 1]) / position_cum[0];
	for ( ; ; ) {
		if (low >= LZARI_Q2) {
			value -= LZARI_Q2;  low -= LZARI_Q2;  high -= LZARI_Q2;
		} else if (low >= LZARI_Q1 && high <= LZARI_Q3) {
			value -= LZARI_Q1;  low -= LZARI_Q1;  high -= LZARI_Q1;
		} else if (high > LZARI_Q2) break;
		low += low;  high += high;
		value = 2 * value + GetBit();
	}
	return position;
}

void LZARI::Decode(void)
{
	int  i, j, k, r, c;
	unsigned long int  count;

	if (!m_bMem)
	{
	//	if (fread(&textsize, sizeof textsize, 1, infile) < 1)
	//		Error("Read Error");  /* read size of text */
	}
	else
	{
		if(m_nInLength < sizeof textsize)
			Error("Read Error");
		memcpy(&textsize,m_pInBuffer + m_nInCur,sizeof textsize);
		
		//m_OutBuffer.reserve(textsize);
		m_nOutLength = textsize;
		//m_nOutCur = 0;
		
		m_nInCur += sizeof textsize;
	}
	
	if (textsize == 0) return;
	
	StartDecode();
	StartModel();
	
	for (i = 0; i < LZARI_N - LZARI_F; i++) text_buf[i] = ' ';
	r = LZARI_N - LZARI_F;
	for (count = 0; count < textsize; ) 
	{
		c = DecodeChar();
		if (c < 256) 
		{
			if(!m_bMem){
			//	putc(c, outfile);
			}
			else
			{
				m_pOutBuffer[m_nOutCur++] = c;
				//m_OutBuffer.push_back(c);
				//m_nOutCur++;
			}
			text_buf[r++] = c;
			r &= (LZARI_N - 1);
			count++;
		}
		else
		{
			i = (r - DecodePosition() - 1) & (LZARI_N - 1);
			j = c - 255 + LZARI_THRESHOLD;
			for (k = 0; k < j; k++) 
			{
				c = text_buf[(i + k) & (LZARI_N - 1)];
				if(!m_bMem){
				//	putc(c, outfile);
				}
				else
				{
					m_pOutBuffer[m_nOutCur++] = c;
				//	m_OutBuffer.push_back(c);
					//m_nOutCur ++;
				}
				text_buf[r++] = c;
				r &= (LZARI_N - 1);
				count++;
			}
		}
		if (count > printcount) 
		{
#ifdef _OUTPUT_STATUS
            DTRACE(DB_FAPROTO, ("%12lu\r", count));
#endif			
			printcount += 1024;
			ClearWDG();
		}
	}

#ifdef _OUTPUT_STATUS
    DTRACE(DB_FAPROTO, ("%12lu\n", count));
#endif
}
void LZARI::UnCompress(const BYTE *pInBuffer,int nInLength, BYTE *pOutBuffer ,int &nOutLength)
{
	if(pOutBuffer==NULL) return;
	m_pInBuffer = pInBuffer;
	m_nInLength = nInLength;
	m_nInCur = 0;
	m_nOutCur = 0;
	m_pOutBuffer = pOutBuffer;
	m_bMem = true;
	Decode();
//	pOutBuffer = &m_OutBuffer[0];
//	pOutBuffer = m_pOutBuffer;
//	nOutLength = m_OutBuffer.size();
	nOutLength = m_nOutCur;
//	m_OutBuffer.push_back(0);
}

void LZARI::Release()
{
//	if(!m_OutBuffer.empty())
	if(m_nOutCur>0)
	{

		
		textsize = 0;
		codesize = 0;
		printcount = 0;
		
		low = 0;
		high = LZARI_Q4;
		value = 0;
		shifts = 0;
		
		m_bMem = FALSE;
		
		m_pInBuffer = NULL;
		m_nInLength = 0;
		m_nInCur = 0;
		m_nOutCur = 0;
//		m_OutBuffer.clear();
//		if(m_pOutBuffer!=NULL)
//		{	delete[] m_pOutBuffer;
//			m_pOutBuffer = NULL;
//		}
		m_pOutBuffer = NULL;
		m_nOutLength = 0;

		buffer_putbit = 0;
		mask_putbit = 128;
		
		buffer_getbit = 0;
		mask_getbit = 0;
	}
}

int LZARI::Alloc(int nLen)
{
	m_pOutBuffer = new BYTE[nLen];
	m_nOutCur=0;
	m_nInCur =0;
	if(m_pOutBuffer!=NULL)
		return 1;
	else return -1;
}
#endif
