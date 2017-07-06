
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
#ifndef LZARI_H
#define LZARI_H
#include "syscfg.h"

#ifdef 0
#include "bios.h"
/********** Bit I/O **********/
//#ifndef _FILE_H_COMPRESSION_LZARI_
//#define  _FILE_H_COMPRESSION_LZARI_
//#pragma warning(disable:4786)
//#include <VECTOR>

//#define _OUTPUT_STATUS

#define LZARI_N		 4096	/* size of ring buffer */
#define LZARI_F		   60	/* upper limit for match_length */
#define LZARI_THRESHOLD	2   /* encode string into position and length
						   if match_length is greater than this */
#define LZARI_NIL		LZARI_N	/* index for root of binary search trees */
/********** Arithmetic Compression **********/

/*  If you are not familiar with arithmetic compression, you should read
		I. E. Witten, R. M. Neal, and J. G. Cleary,
			Communications of the ACM, Vol. 30, pp. 520-540 (1987),
	from which much have been borrowed.  */

#define LZARI_M   15

/*	Q1 (= 2 to the M) must be sufficiently large, but not so
	large as the unsigned long 4 * Q1 * (Q1 - 1) overflows.  */

#define LZARI_Q1  (1UL << LZARI_M)
#define LZARI_Q2  (2 * LZARI_Q1)
#define LZARI_Q3  (3 * LZARI_Q1)
#define LZARI_Q4  (4 * LZARI_Q1)
#define LZARI_MAX_CUM (LZARI_Q1 - 1)

#define LZARI_N_CHAR  (256 - LZARI_THRESHOLD + LZARI_F)

class LZARI 
{
public:	
	LZARI();
	virtual ~LZARI();
protected:

	unsigned long textsize;
	unsigned long codesize;
	unsigned long printcount;
	unsigned char  text_buf[LZARI_N + LZARI_F - 1];	/* ring buffer of size N,with extra F-1 bytes to facilitate string comparison */
	int	match_position;
	int	match_length;  /* of longest match.  These areset by the InsertNode() procedure. */
	int	lson[LZARI_N + 1];
	int rson[LZARI_N + 257];
	int dad[LZARI_N + 1];  /* left & right children &parents -- These constitute binary search trees. */

	/* character code = 0, 1, ..., N_CHAR - 1 */

	unsigned long low;
	unsigned long high;
	unsigned long value;
	int  shifts;  /* counts for magnifying low and high around Q2 */
	int  char_to_sym[LZARI_N_CHAR];
	int sym_to_char[LZARI_N_CHAR + 1];
	unsigned int sym_freq[LZARI_N_CHAR + 1];  /* frequency for symbols */
	unsigned int sym_cum[LZARI_N_CHAR + 1];   /* cumulative freq for symbols */
	unsigned int position_cum[LZARI_N + 1];   /* cumulative freq for positions */

	// Compress in memory;
	bool m_bMem;

	//std::vector<BYTE> m_OutBuffer;
	BYTE *m_pOutBuffer;
	int m_nOutLength;
	int m_nOutCur;

	const BYTE *m_pInBuffer;
	int m_nInLength;
	int m_nInCur;

	unsigned int  buffer_putbit, mask_putbit;
	unsigned int  buffer_getbit, mask_getbit;

private:
	void Error(char *message);
	void PutBit(int bit);  /* Output one bit (bit = 0,1) */
	void FlushBitBuffer(void);  /* Send remaining bits */
	int GetBit(void);  /* Get one bit (0 or 1) */

/********** LZSS with multiple binary trees **********/

	void InitTree(void);  /* Initialize trees */
//	void InsertNode(int r);
//	void DeleteNode(int p);  /* Delete node p from tree */
	void StartModel(void); /* Initialize model */
	void UpdateModel(int sym);
//	void Output(int bit);  /* Output 1 bit, followed by its complements */
//	void EncodeChar(int ch);
//	void EncodePosition(int position);
//	void EncodeEnd(void);
	int BinarySearchSym(unsigned int x);
	int BinarySearchPos(unsigned int x);
	void StartDecode(void);
	int DecodeChar(void);
	int DecodePosition(void);

//	void Encode(void);
	void Decode(void);

public:
	int Alloc(int nLen);
//	void Compress(const char *lpszInfile,const char *lpszOutfile);
//	void UnCompress(const char *lpszInfile,const char *lpszOutfile);

//	void Compress(const BYTE *pInBuffer,int nInLength,const BYTE * &pOutBuffer ,int &nOutLength);
	void UnCompress(const BYTE *pInBuffer,int nInLength, BYTE * pOutBuffer,int &nOutLength);
	
	void Release();
};

#endif
#endif
