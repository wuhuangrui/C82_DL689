#ifndef _IEC7816_H_
#define _IEC7816_H_
//#include "bios.h"
#include "sysarch.h"
//#include "sysapi.h"
//#include "apptypedef.h"

//������
#define NOERROR                                         0//�ɹ�
#define SENDTIMEOUT                                    -0xfe//���ͳ�ʱ
#define NOSW                                           -0xff//������Ӧ����
#define LENWRONG                                       -1//���ݳ��ȴ���
#define	RANDOMNOGET                                    -2//û�п��������
#define PUBKEYFILEWRITEWRONG                           -3//д��Կ�ļ�����
#define	COMMANDWRONG                                   -4//ָ��ṹ����
#define SM1KEYWRONG                                    -5//SM1��Կ����  
#define CHECKFILETYPENOMATCH                           -6//��ǩ�ļ����Ͳ�ƥ��
#define CHECKFILENOFIND                                -7//��ǩ�ļ�δ�ҵ�
#define PRIVATEKEYFILENOFIND                           -8//����RSA��Կ��ʱ˽Կ�ļ�δ�ҵ�
#define PUBLICKEYFILEFORCIPHERNOMATCH                  -9//�������ܵĹ�Կ�ļ���ƥ��
#define PUBLICKEYFILEFORCIPHERNOFIND                   -10//�������ܵĹ�Կ�ļ�û�ҵ�
#define PUBLICKEYFORDECIPHERNOMATCH                    -11//�������ܵĹ�Կ�ļ���ƥ��
#define PUBLICKEYFILEFORDECIPHERNOFIND                 -12//�������ܵĹ�Կ�ļ�û�ҵ�
#define RSACIPHERWRONG                                 -13//RSA���ܴ���
#define RSADECIPHERWRONG                               -14//RSA���ܴ���
#define RSACHECKSIGNWRONG                              -15//RSA��ǩ����
#define RSAKEYPAIRWRONG                                -16//RSA������Կ�Դ���
#define RSASIGNWRONG                                   -17//RSAǩ������
#define SM1DECIPHERDATAWRONG                           -18//SM1�������ݴ���
#define LINEPROTECTIONWRONG                            -19//SM1�������ݴ���
#define NOTHINGTOGET                                   -20//���������ݿɷ���
#define CLANOVALID                                     -21//��Ч��CLA
#define STATUSNOVALID                                  -22//��Ч��״̬
#define P1NOPUBFILE                                    -23 //P1��P2��ָ�ı�ʶ��������Ӧ�Ĺ�Կ�ļ�/��֧�ִ˹���
#define RIGHTSNOMEET                                   -24//���ӻ��޸�Ȩ�޲�����
#define KEYLOCKED                                      -25//��Կ������
#define KEYFILESPACEFULL                               -26//KEY�ļ��ռ�����
#define KEYNOFIND                                      -27//��Կδ�ҵ�
#define KEYFILENOFIND                                  -28//KEY�ļ�δ�ҵ�

typedef unsigned char           BYTE;
typedef  unsigned short         WORD;
typedef  unsigned long          DWORD;

struct TIEC7816_TPDUHead//����ͷ
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
	char *str;//״̬˵��������
}TSWTable;

class CIEC7816
{
private:
	int m_ifd;
	BYTE m_bSeris[8];//���к� 
	BYTE m_bRandomNumber[8];//��ǰ����� 
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

//��ȡ�ն����к�
//�ɹ� �������кŵĳ��� ʧ�ܴ�����
inline int GetESAMSeris(BYTE *pbBuf,DWORD dwTimeout)
{
	return g_ESAM.GetSeris(pbBuf,dwTimeout);
}

//��ȡ�ն˵�ǰ�����  
//�ɹ� ����������ĳ��� ʧ��-1
inline int GetESAMRandomNumber(BYTE *pbBuf,DWORD dwTimeout)
{
	return g_ESAM.GetESAMRandomNumber(pbBuf,dwTimeout);
}

//��ȡ�ն��µ������  ���·�������������
//bLen ���������������
// pbBuf ESAM���ص�������
//�ɹ� ����������ĳ��� ʧ��-1
inline int GetChallenge(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
    return g_ESAM.GetChallenge(bLen, pbBuf,dwTimeout);
}

//��ȡ�ն˵���Ӧ����
//bLen ��������Ӧ���ݳ���
// pbBuf ESAM���ص���Ӧ����
// �ɹ�  ������Ӧ���ݵĳ��� ���򷵻���Ӧ�Ĵ�����
inline int GetResponse(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
	return g_ESAM.GetResponse(bLen,pbBuf,dwTimeout);
}


//MACУ��  pbBuf--�����������ǩ����Ϣ  ������ǩ���
//bAFN--������
// bA3--���ַ��־
// bGrpAddrInx--���ַ���
// pbData--�ն��յ��Ĵ�AFN��ʼ���������ݣ�����ʱ���ǩ��
// pbMacData--�ն��յ���MAC��Ϣ
// wDataLen--�ն��������ݳ��ȣ�����ʱ���ǩ��
// �ɹ�  ����MAC�ĳ��� ���򷵻���Ӧ�Ĵ����븺��
inline int VerifyMac(BYTE bAFN,BYTE bA3,BYTE bGrpAddrInx,BYTE *pbData,BYTE *pbMacData,WORD wDataLen,DWORD dwTimeout)
{
	return g_ESAM.VerifyMac(bAFN,bA3,bGrpAddrInx,pbData,pbMacData,wDataLen,dwTimeout);
}
//��Կ��֤
//pbRandom ��վ�����8
// bP2 Ҫ��֤��Կ�ļ����ļ���ʶ
// pbSign ����ǩ��128
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout)
{
	return  g_ESAM.VerifyPublicKey(bP2,pbSign,pbMastRandom,dwTimeout);
}

//���ع�Կ����
// ���ع�Կ�ͳ��� pbKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int UpdateESAMKey(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return g_ESAM.UpdateESAMKey(pbKey,pbTermRandom,pbSign,dwTimeout);
}

//��վ��Կ���ظ���
// ��վ��Կ�ͳ��� pbKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// pbRandom �ն������
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return g_ESAM.UpdateMKeyLocal(pbKey,pbTermRandom,pbSign,dwTimeout);
}

//��վ��ԿԶ�̸���
// pbSessionKey  �Ự��Կ 128
// ��վ��Կ�ͳ��� pbMKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// pbRandom �ն������
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return g_ESAM.UpdateMKeyFar(pbSessionKey,pbMKey,pbTermRandom,pbSign,dwTimeout);
}

//�Գ���Կ�ĸ���  ����0--ע��ɹ� ����--������
//bNum:������Կ����
// pbSign:����ǩ��128
// pbSessionKey �Ự��Կ128
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return g_ESAM.UpdateSymKey(bNum,pbSessionKey,pbTermKey,pbTermRandom,pbSign,dwTimeout);
}

//�ǶԳ���Կ��ע��  ����0--ע��ɹ� ����--������
//bP1:01/02 
// pbRandom: ��վ�����
// bRamdomLen: ��վ��������� 8�ֽ�
// pbSign:����ǩ��
// bSignLen:����ǩ������ 128�ֽ�
// pbBuf:ע��ɹ��󷵻صķǶԳ���Կ��Կ 256�ֽ�
// �ɹ�  ������Կ�ĳ��� ���򷵻���Ӧ�Ĵ�����
inline int RegisterNonSymKey(BYTE bP1,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf)
{
	return g_ESAM.RegisterNonSymKey(bP1,pbMastRandom,pbTermRandom,pbSign,dwTimeout,pbBuf);
}

//�ǶԳ���Կ�ĸ���  ����0--ע��ɹ� ����--������
//bP1:01/02
// pbRandom: ��վ�����8
// pbSign:����ǩ��128
// pbSessionKey �Ự��Կ128
// pbBuf:ע��ɹ��󷵻صķǶԳ���Կ��Կ 256�ֽ�
// �ɹ�  ������Կ�ĳ��� ���򷵻���Ӧ�Ĵ�����
inline int UpdateNonSymKey(BYTE bP1,BYTE *pbSessionKey,BYTE *pbMastRandom,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout,BYTE *pbBuf)
{
	return g_ESAM.UpdateNonSymKey(bP1,pbSessionKey,pbMastRandom,pbTermRandom,pbSign,dwTimeout,pbBuf);
}

#endif /* _IEC7816_H_ */
