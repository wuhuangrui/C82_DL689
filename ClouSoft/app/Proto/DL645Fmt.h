#ifndef DL645FMT_H_
#define DL645FMT_H_

#include "apptypedef.h"
#include "FaCfg.h"
#include "FaAPI.h"
#include "sysfs.h"

#define		BN_645_DATA			BN0
#define		BN_645_PARA			BN4
#define		BN_645_DATAEX		BN4

#define		MTRPN				PN0		//�ڱ�������

//#define		TDB_MAP				2

//#define		ENG_DAYFRZ_LEN		160	//ZQQ DEBUG TEST
//#define		DMD_DAYFRZ_LEN		160

#define		FEE_RATE_NUM		4		//������
#define		TOTAL_FEE_RATE		(FEE_RATE_NUM+1)	//��+������

//#define		ERR_OVER_RECNUM		-20


typedef struct{
	//BYTE	bMap;		//ӳ�䷽ʽ:0--ϵͳ��һһ��Ӧ; 1--ϵͳ���������������ʱ��; 2---�����
	BYTE	bBank;		//645����IDΪBN4, ����IDΪBN0
	DWORD	dw07Id;	
	WORD	w97Id;		//ϵͳ���Ӧ��97ID;���߶��������ӳ�䷽ʽ,Ϊ������97����ID,������ƫ��,һ������Ϊ��ID
	char*	psTdbName;	//��Ӧ����������
}T07To97IDCfg;


extern int Read97Id(WORD wBn, WORD wID, BYTE* pbTx);
extern int Read07Id(DWORD dw07Id, BYTE* pbTx);
extern bool Map07To97ID(DWORD dw07Id, T07To97IDCfg* pIdCfg);

extern int  FmtGBTo645(WORD wBn, WORD* pwID, WORD wNum, BYTE* pbIn, BYTE* pbOut);
extern WORD FmtGBTo645(WORD wBn, WORD wID, BYTE* pbIn, BYTE* pbOut);
extern int GetRecIdx(const int fd, int iRecNo);
extern bool HaveProgPermit(WORD wID, BYTE bPswPerm);

extern WORD Map97ToBank(DWORD dw97Id);
extern int ReadLastNRec(char* szTbName, BYTE bPtr, BYTE* pbBuf, int iLen);
int GetRecPhyIdx(char* szTbName, BYTE bPtr);
#endif
