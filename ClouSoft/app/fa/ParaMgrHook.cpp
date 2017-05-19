#include "stdafx.h"
#include "ParaMgrHook.h"
#include "DbAPI.h"

bool OnWriteSpecialPara(WORD wBn, WORD wPn, WORD wID, BYTE* pbBuf, WORD wLen)
{
	WORD wClass, wFN, wPN;
	BYTE* p = pbBuf;
	if (wID == 0xffff)
    {
        memcpy(&wClass, p+sizeof(WORD)*2, sizeof(WORD));
        memcpy(&wFN, p+sizeof(WORD)*3, sizeof(WORD));
        memcpy(&wPN, p+sizeof(WORD)*4, sizeof(WORD));
		//GBWriteItem(wClass, wFN, wPN, p+sizeof(WORD)*5);
		return true;			
    }	
	return false;
}
