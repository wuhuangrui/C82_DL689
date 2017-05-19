#ifndef _PARAMGR_H
#define _PARAMGR_H

#include "apptypedef.h"

class CParaMgr
{
public:
    CParaMgr();
    virtual ~CParaMgr();
    int LoadPara(char* szFileName);
    bool Parse(void);
    
private:
	BYTE* m_pbParaData;
	int   m_iParaLen;
	
	void Clean(void);
};
extern CParaMgr g_pmParaMgr;
#endif
