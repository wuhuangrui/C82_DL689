#ifndef DCPROC_H
#define DCPROC_H
#include "apptypedef.h"

class CDCProc
{
private:

public:
    int Init(void);
    int DoDCProc(void);
    WORD GetDcSamplePn(BYTE bPnIndex);
    bool InitDcValToDb(BYTE bPnIndex);	
	void ValToDb(BYTE bPnIndex, int iVal);	
protected:
		
	double m_dDcParaKp[DC_CHN_MAX];
	double m_dDcParaB[DC_CHN_MAX];
};

#endif

