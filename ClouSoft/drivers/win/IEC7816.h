#ifndef _IEC7816_H_
#define _IEC7816_H_
//#include "bios.h"
//#include "sysarch.h"
//#include "sysapi.h"
//#include "apptypedef.h"

//错误码

#define IEC7816_OK					0x00	// 正确执行
#define IEC7816_ERR_RecvOvertime			0x01	// 接收超时
#define IEC7816_ERR_RecvOvertime			0x01	// 接收超时
#define IEC7816_ERR_ParityChk				0x02	// 连续三次收发校验错误
#define IEC7816_ERR_PARA					0x03	// 参数错误

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

class CIEC7816
{
private:
	int m_ifd;
	BYTE m_bSeris[8];//序列号 
	BYTE m_bRandomNumber[8];//当前随机数 
	bool m_fTest;

    struct TIEC7816_TAPDU m_TApdu;
    struct TIEC7816_RAPDU m_RApdu;

private:
	BYTE SendChar(BYTE bCharToSend);
	BYTE GetChar(BYTE *pCharToReceive);

	void GetATR(BYTE* pAtr, BYTE* pLength);
	void Decode_ATR(BYTE* pAtr);
	BYTE APDU(const struct TIEC7816_TAPDU *pCAPDU, struct TIEC7816_RAPDU *pRAPDU);
	BYTE TPDU(const unsigned char *pAPDU, unsigned char *pMessage, unsigned short wLength );
    int CheckErr(int iCnt,BYTE *pbBuf,BYTE bSW1,BYTE bSW2);

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
    int VerifyMac(BYTE bAFN,BYTE bA3,BYTE *pbTermAddr,BYTE *pbGrpAddr,BYTE *pbData,BYTE *pbMacData,WORD wDataLen,DWORD dwTimeout);
	int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout);
	int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
	int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
	int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout);
	int RegisterNonSymKey(BYTE bP1,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf);
	int UpdateNonSymKey(BYTE bP1,BYTE *pbSessionKey,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf);
	void SetTest(bool fTest);
	BYTE GetRandomNumber(BYTE *pbBuf,BYTE bLen);//取当前随机数
	BYTE VerifyMac(BYTE *pbBuf);
	BYTE VerifyPublicKey(BYTE *pbBuf);//公钥验证
	BYTE UpdateKey(BYTE *pbBuf);//密钥更新
	void DoTest();
};

extern CIEC7816 g_ESAM;
/////////////////////////////////

inline bool InitESAM()
{
	return true;
}

inline bool ResetESAM()
{
	return true;
}

inline void CloseESAM()
{
	
}

//读取终端序列号
//成功 返回序列号的长度 失败错误码
inline int GetESAMSeris(BYTE *pbBuf,DWORD dwTimeout)
{
	return -1;
}

//读取终端当前随机数  
//成功 返回随机数的长度 失败-1
inline int GetESAMRandomNumber(BYTE *pbBuf,DWORD dwTimeout)
{
	return -1;
}

//读取终端新的随机数  重新发起读随机数命令
//bLen 期望的随机数长度
// pbBuf ESAM返回的数据数
//成功 返回随机数的长度 失败-1
inline int GetChallenge(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
    return -1;
}

//读取终端的响应数据
//bLen 期望的响应数据长度
// pbBuf ESAM返回的响应数据
// 成功  返回响应数据的长度 否则返回相应的错误码
inline int GetResponse(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
	return -1;
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
	return -1;
}
//公钥验证
//pbRandom 主站随机数8
// bP2 要验证公钥文件的文件标识
// pbSign 数据签名128
// 成功  返回0 否则返回相应的错误码
inline int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout)
{
	return  -1;
}

//主控公钥更新
// 主控公钥和长度 pbKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// 成功  返回0 否则返回相应的错误码
inline int UpdateESAMKey(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return -1;
}

//主站公钥本地更新
// 主站公钥和长度 pbKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// pbRandom 终端随机数
// 成功  返回0 否则返回相应的错误码
inline int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return -1;
}

//主站公钥远程更新
// pbSessionKey  会话密钥 128
// 主站公钥和长度 pbMKey bKeyLen 144字节
// 数据签名和长度pbSign bSignLen 128字节
// pbRandom 终端随机数
// 成功  返回0 否则返回相应的错误码
inline int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return -1;
}

//对称密钥的更新  返回0--注册成功 其他--错误码
//bNum:更新密钥条数
// pbSign:数据签名128
// pbSessionKey 会话密钥128
// 成功  返回0 否则返回相应的错误码
inline int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return -1;
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
	return -1;
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
	return -1;
}

extern int CodeToStr(int iErrCode,char *str);

#endif /* _IEC7816_H_ */
