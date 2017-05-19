#ifndef DL645FMT_H_
#define DL645FMT_H_

#include "apptypedef.h"
#include "FaCfg.h"
#include "FaAPI.h"
#include "sysfs.h"

#define		BN_645_DATA			BN0
#define		BN_645_PARA			BN4
#define		BN_645_DATAEX		BN4

#define		MTRPN				PN0		//内表测量点号

//#define		TDB_MAP				2

//#define		ENG_DAYFRZ_LEN		160	//ZQQ DEBUG TEST
//#define		DMD_DAYFRZ_LEN		160

#define		FEE_RATE_NUM		4		//费率数
#define		TOTAL_FEE_RATE		(FEE_RATE_NUM+1)	//总+费率数

//#define		ERR_OVER_RECNUM		-20


typedef struct{
	//BYTE	bMap;		//映射方式:0--系统库一一对应; 1--系统库最大需量及发生时间; 2---任务库
	BYTE	bBank;		//645参数ID为BN4, 其它ID为BN0
	DWORD	dw07Id;	
	WORD	w97Id;		//系统库对应的97ID;或者对于任务库映射方式,为任务库的97冻结ID,用来求偏移,一般配置为块ID
	char*	psTdbName;	//对应的任务库表名
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
