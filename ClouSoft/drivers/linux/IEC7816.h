#ifndef _IEC7816_H_
#define _IEC7816_H_
//#include "bios.h"
#include "sysarch.h"
//#include "sysapi.h"
//#include "apptypedef.h"

//错误码
#define NOERROR                                         0//成功
#define SENDTIMEOUT                                    -0xfe//发送超时
#define NOSW                                           -0xff//不明响应数据
#define LENWRONG                                       -1//数据长度错误
#define	RANDOMNOGET                                    -2//没有可用随机数
#define PUBKEYFILEWRITEWRONG                           -3//写公钥文件错误
#define	COMMANDWRONG                                   -4//指令结构错误
#define SM1KEYWRONG                                    -5//SM1密钥错误  
#define CHECKFILETYPENOMATCH                           -6//验签文件类型不匹配
#define CHECKFILENOFIND                                -7//验签文件未找到
#define PRIVATEKEYFILENOFIND                           -8//产生RSA密钥对时私钥文件未找到
#define PUBLICKEYFILEFORCIPHERNOMATCH                  -9//用来加密的公钥文件不匹配
#define PUBLICKEYFILEFORCIPHERNOFIND                   -10//用来加密的公钥文件没找到
#define PUBLICKEYFORDECIPHERNOMATCH                    -11//用来解密的公钥文件不匹配
#define PUBLICKEYFILEFORDECIPHERNOFIND                 -12//用来解密的公钥文件没找到
#define RSACIPHERWRONG                                 -13//RSA加密错误
#define RSADECIPHERWRONG                               -14//RSA解密错误
#define RSACHECKSIGNWRONG                              -15//RSA验签错误
#define RSAKEYPAIRWRONG                                -16//RSA产生密钥对错误
#define RSASIGNWRONG                                   -17//RSA签名错误
#define SM1DECIPHERDATAWRONG                           -18//SM1解密数据错误
#define LINEPROTECTIONWRONG                            -19//SM1解密数据错误
#define NOTHINGTOGET                                   -20//卡中无数据可返回
#define CLANOVALID                                     -21//无效的CLA
#define STATUSNOVALID                                  -22//无效的状态
#define P1NOPUBFILE                                    -23 //P1、P2所指的标识符不是响应的公钥文件/不支持此功能
#define RIGHTSNOMEET                                   -24//增加或修改权限不满足
#define KEYLOCKED                                      -25//密钥被锁死
#define KEYFILESPACEFULL                               -26//KEY文件空间已满
#define KEYNOFIND                                      -27//密钥未找到
#define KEYFILENOFIND                                  -28//KEY文件未找到

typedef unsigned char           BYTE;
typedef  unsigned short         WORD;
typedef  unsigned long          DWORD;

struct TIEC7816_TPDUHead//命令头
{
	BYTE CLA;
	BYTE INS;
	BYTE P1;
	BYTE P2;
	BYTE P3;
};

struct TIEC7816_TAPDU
{
	BYTE CLA;
	BYTE INS;
	BYTE P1;
	BYTE P2;
	BYTE Lc;
	BYTE Le;
	BYTE *pData;
};

struct TIEC7816_RAPDU
{
	union
	{
		WORD word;
		BYTE byte[2];
	} SW;
	BYTE *pResponse;
	BYTE Le;
};

typedef struct SWTable
{
	BYTE bSW1;
	BYTE bSW2;
	char *str;//状态说明性文字
}TSWTable;

class CIEC7816
{
private:
	int m_ifd;
	BYTE m_bSeris[8];//序列号 
	BYTE m_bRandomNumber[8];//当前随机数 
	bool m_fReseted;
	bool m_fGetChallenge;
	TSem  m_semEsam;

//  struct TIEC7816_TAPDU m_TApdu;
//  struct TIEC7816_RAPDU m_RApdu;

private:
//  BYTE SendChar(BYTE bCharToSend);
//  BYTE GetChar(BYTE *pCharToReceive);

	WORD Read(BYTE *pbBuf, WORD wBufSize);
//  void GetATR(BYTE* pAtr, BYTE* pLength);
	bool DecodeATR(BYTE* pbBuf,int iCnt);
//  BYTE APDU(const struct TIEC7816_TAPDU *pCAPDU, struct TIEC7816_RAPDU *pRAPDU);
//  BYTE TPDU(const unsigned char *pAPDU, unsigned char *pMessage, unsigned short wLength );
    int CheckSW(int iCnt,BYTE *pbBuf,BYTE bSW1,BYTE bSW2);
	int CheckSW(int iCnt,BYTE *pbBuf);
	void PrintEsamBuf(char *str,int iLen,BYTE *pbBuf);
	void CheckStatus();

public:
	CIEC7816();
	~CIEC7816();
	bool Init();
	bool Reset();
	void Close(void);
    int GetSeris(BYTE *pbBuf,DWORD dwTimeout);
	int GetESAMRandomNumber(BYTE *pbBuf,DWORD dwTimeout);
    int GetChallenge(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout);
    int UpdateESAMKey(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
	int GetResponse(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout);
    int VerifyMac(BYTE bAFN,BYTE bA3,BYTE bGrpAddr,BYTE *pbData,BYTE *pbMacData,WORD wDataLen,DWORD dwTimeout);
	int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout);
	int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
	int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
	int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
	int RegisterNonSymKey(BYTE bP1,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf);
	int UpdateNonSymKey(BYTE bP1,BYTE *pbSessionKey,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf);
};

extern CIEC7816 g_ESAM;
/////////////////////////////////

inline bool InitESAM()
{
	return g_ESAM.Init();
}

inline bool ResetESAM()
{
	return g_ESAM.Reset();
}

inline void CloseESAM()
{
	g_ESAM.Close();
}

//读取终端序列号
//成功 返回序列号的长度 失败错误码
inline int GetESAMSeris(BYTE *pbBuf,DWORD dwTimeout)
{
	return g_ESAM.GetSeris(pbBuf,dwTimeout);
}

//读取终端当前随机数  
//成功 返回随机数的长度 失败-1
inline int GetESAMRandomNumber(BYTE *pbBuf,DWORD dwTimeout)
{
	return g_ESAM.GetESAMRandomNumber(pbBuf,dwTimeout);
}

//读取终端新的随机数  重新发起读随机数命令
//bLen 期望的随机数长度
// pbBuf ESAM返回的数据数
//成功 返回随机数的长度 失败-1
inline int GetChallenge(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
    return g_ESAM.GetChallenge(bLen, pbBuf,dwTimeout);
}

//读取终端的响应数据
//bLen 期望的响应数据长度
// pbBuf ESAM返回的响应数据
// 成功  返回响应数据的长度 否则返回相应的错误码
inline int GetResponse(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
	return g_ESAM.GetResponse(bLen,pbBuf,dwTimeout);
}


//MAC校验  pbBuf--随机数和数据签名信息  返回验签结果
//bAFN--功能码
// bA3--组地址标志
// bGrpAddrInx--组地址序号
// pbData--终端收到的从AFN开始的明文数据（不含时间标签）
// pbMacData--终端收到的MAC信息
// wDataLen--终端明文数据长度（不含时间标签）
// 成功  返回MAC的长度 否则返回相应的错误码负数
inline int VerifyMac(BYTE bAFN,BYTE bA3,BYTE bGrpAddrInx,BYTE *pbData,BYTE *pbMacData,WORD wDataLen,DWORD dwTimeout)
{
	return g_ESAM.VerifyMac(bAFN,bA3,bGrpAddrInx,pbData,pbMacData,wDataLen,dwTimeout);
}
//公钥验证
//pbRandom 主站随机数8
// bP2 要验证公钥文件的文件标识
// pbSign 数据签名128
// 成功  返回0 否则返回相应的错误码
inline int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout)
{
	return  g_ESAM.VerifyPublicKey(bP2,pbSign,pbMastRandom,dwTimeout);
}

//主控公钥更新
// 主控公钥和长度 pbKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// 成功  返回0 否则返回相应的错误码
inline int UpdateESAMKey(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return g_ESAM.UpdateESAMKey(pbKey,pbTermRandom,pbSign,dwTimeout);
}

//主站公钥本地更新
// 主站公钥和长度 pbKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// pbRandom 终端随机数
// 成功  返回0 否则返回相应的错误码
inline int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return g_ESAM.UpdateMKeyLocal(pbKey,pbTermRandom,pbSign,dwTimeout);
}

//主站公钥远程更新
// pbSessionKey  会话密钥 128
// 主站公钥和长度 pbMKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// pbRandom 终端随机数
// 成功  返回0 否则返回相应的错误码
inline int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return g_ESAM.UpdateMKeyFar(pbSessionKey,pbMKey,pbTermRandom,pbSign,dwTimeout);
}

//对称密钥的更新  返回0--注册成功 其他--错误码
//bNum:更新密钥条数
// pbSign:数据签名128
// pbSessionKey 会话密钥128
// 成功  返回0 否则返回相应的错误码
inline int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return g_ESAM.UpdateSymKey(bNum,pbSessionKey,pbTermKey,pbTermRandom,pbSign,dwTimeout);
}

//非对称密钥的注册  返回0--注册成功 其他--错误码
//bP1:01/02 
// pbRandom: 主站随机数
// bRamdomLen: 主站随机数长度 8字节
// pbSign:数据签名
// bSignLen:数据签名长度 128字节
// pbBuf:注册成功后返回的非对称密钥公钥 256字节
// 成功  返回密钥的长度 否则返回相应的错误码
inline int RegisterNonSymKey(BYTE bP1,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf)
{
	return g_ESAM.RegisterNonSymKey(bP1,pbMastRandom,pbTermRandom,pbSign,dwTimeout,pbBuf);
}

//非对称密钥的更新  返回0--注册成功 其他--错误码
//bP1:01/02
// pbRandom: 主站随机数8
// pbSign:数据签名128
// pbSessionKey 会话密钥128
// pbBuf:注册成功后返回的非对称密钥公钥 256字节
// 成功  返回密钥的长度 否则返回相应的错误码
inline int UpdateNonSymKey(BYTE bP1,BYTE *pbSessionKey,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf)
{
	return g_ESAM.UpdateNonSymKey(bP1,pbSessionKey,pbMastRandom,pbTermRandom,pbSign,dwTimeout,pbBuf);
}

#endif /* _IEC7816_H_ */
