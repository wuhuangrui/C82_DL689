#ifndef _ESAM_H_
#define _ESAM_H_
//#include "bios.h"
#include "sysarch.h"
//#include "sysapi.h"
//#include "apptypedef.h"

typedef unsigned char           BYTE;
typedef  unsigned short         WORD;
typedef  unsigned long          DWORD;
typedef struct SWTab
{
	BYTE bSW1;
	BYTE bSW2;
	char *str;//状态说明性文字
}TSWTab;

typedef enum {
	AllEsamInfo = 0x01,
	EsamSerialNum = 0x02,
	EsamVerison,
	KeyVersion,
	CerVersion,
	SessionMaxTime,
	SessionRemainTime,
	Counter,
	TermCerSerNum,
	MasterStationCerSerNum,
	TermCertificate,
	MasterStationCer,
} EsamInfoList;

#define ESAM_MAC_LEN 4


bool EsamInit();
void EsamClose();
bool EsamReset();
//BYTE memrcpy(BYTE *pbDst,BYTE *pbSrc,WORD wLen);
//void Swap(BYTE *pbBuf, WORD wLen);
int EsamRead(BYTE* pbBuf, WORD wBufSize);
int EsamTxRxCmd(const BYTE* pbTx, BYTE* pbRx, WORD wExpLen);
int EsamGetTermInfo(BYTE* pbRx);
int EsamNegotiateKey(BYTE* pbTx, WORD wTxLen, BYTE* pbRx, DWORD dwTimeout);
bool EsamUpdSymKey(BYTE bKeyNum, BYTE* pbKey, DWORD dwTimeout);
int EsamUpdCert(BYTE* pbRx, DWORD dwTimeout);
bool EsamUpdCA(BYTE* pbTx, WORD wTxLen, DWORD dwTimeout);
int EsamIntCert(BYTE* pbTx, WORD wTxLen, BYTE* pbRx, DWORD dwTimeout);
int EsamExtCert(BYTE* pbTx, WORD wTxLen, BYTE* pbRx, DWORD dwTimeout);
bool EsamSwitchState(BYTE bP1, BYTE* pbTx, WORD wTxLen, DWORD dwTimeout);
bool EsamSetOfflineCnt(BYTE* pbTx, WORD wTxLen, DWORD dwTimeout);
bool EsamTransEncrAuth(BYTE* pbTx, WORD wTxLen, DWORD dwTimeout);
int EsamVerifyMac(BYTE bAFN, BYTE* pbData, WORD wDataLen, BYTE* pbMac, BYTE* pbRx, DWORD dwTimeout);
bool EsamGrpBroadcastVerifyMac(BYTE bAFN, WORD wGrpAddr, BYTE* pbData, WORD wDataLen, BYTE* pbRx, DWORD dwTimeout);
int EsamGetRandom(BYTE* pbRx, BYTE bRandomLen, DWORD dwTimeout);
bool EsamGetMtrAuth(BYTE bP2, BYTE* pbMtrCiph, BYTE* pbMtrAddr, BYTE* pbTskData, BYTE* pbR1, BYTE* pbER1, DWORD dwTimeout);
int EsamGetAdjTmCiph(BYTE* pbTskFmt, BYTE* pbTskData, BYTE bTskLen, BYTE* pbMtrKeyCiph, BYTE* pbR2, BYTE* pbRx, DWORD dwTimeout);

//new fun
int Esam_GetEsamInfo(EsamInfoList tInfo, BYTE* pbRx, WORD wBufLen);
int Esam_InitSession(BYTE* pbOutSessionInit, BYTE bOutSessionInitLen, BYTE* pbOutSign, 
		BYTE bOutSignLen, BYTE* pbSessionData, BYTE* pbSessionDataLen, BYTE* pbSign, BYTE* pbSignLen);
int Esam_PlnDatResCalMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac);
int Esam_PlainDataCalRnMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac);
bool Esam_CheckResponseMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac);
int Esam_Broadcast(BYTE* pbSIDMAC, BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen);
int Esam_SIDDecode(BYTE* pbSID, BYTE* pbAttachData, WORD wAttachDataLen, 
		BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen);
int Esam_SIDMACDecode(BYTE* pbSID, BYTE* pbAttachData, WORD wAttachDataLen, BYTE* pbMAC,
		BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen);
int Esam_ResMakeEndata(BYTE* pbData, WORD wDataLen, BYTE* pbEnData, WORD wRxBufLen);
int Esam_ResMakeEndataMac(BYTE* pbData, WORD wDataLen, BYTE* pbEnData, WORD wRxBufLen);
int Esam_ResMakePlndataMac(BYTE* pbData, WORD wDataLen, BYTE* pbEnData, WORD wRxBufLen);
void Esam_ReflashSessionRemainTime(void);
void Esam_ReflashSessionMaxTime(void);

void Esam_ReflashCounter(void);
int EsamGetRandom(BYTE* pbRx);
int Esam_ReadMtrDataVerify(BYTE bOpType, BYTE *pMtrNum, WORD wMtrNumLen, BYTE *pRn,
						   WORD wRnLen, BYTE *pSrcData, WORD wSrcDataLen, BYTE *pMac, BYTE *pDstData);


// OI Method
int EsamResetMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamExeMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamDataReadMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamDataUpdateMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamConsultFailMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamWalletOpMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamKeyUpdateMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamCerUpdateMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamSetConsultTimeBarMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int EsamWalletInitMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);


#endif /* _ESAM_H_ */
