#ifndef _IEC7816_H_
#define _IEC7816_H_
//#include "bios.h"
//#include "sysarch.h"
//#include "sysapi.h"
//#include "apptypedef.h"

//������

#define IEC7816_OK					0x00	// ��ȷִ��
#define IEC7816_ERR_RecvOvertime			0x01	// ���ճ�ʱ
#define IEC7816_ERR_RecvOvertime			0x01	// ���ճ�ʱ
#define IEC7816_ERR_ParityChk				0x02	// ���������շ�У�����
#define IEC7816_ERR_PARA					0x03	// ��������

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

class CIEC7816
{
private:
	int m_ifd;
	BYTE m_bSeris[8];//���к� 
	BYTE m_bRandomNumber[8];//��ǰ����� 
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
	BYTE GetRandomNumber(BYTE *pbBuf,BYTE bLen);//ȡ��ǰ�����
	BYTE VerifyMac(BYTE *pbBuf);
	BYTE VerifyPublicKey(BYTE *pbBuf);//��Կ��֤
	BYTE UpdateKey(BYTE *pbBuf);//��Կ����
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

//��ȡ�ն����к�
//�ɹ� �������кŵĳ��� ʧ�ܴ�����
inline int GetESAMSeris(BYTE *pbBuf,DWORD dwTimeout)
{
	return -1;
}

//��ȡ�ն˵�ǰ�����  
//�ɹ� ����������ĳ��� ʧ��-1
inline int GetESAMRandomNumber(BYTE *pbBuf,DWORD dwTimeout)
{
	return -1;
}

//��ȡ�ն��µ������  ���·�������������
//bLen ���������������
// pbBuf ESAM���ص�������
//�ɹ� ����������ĳ��� ʧ��-1
inline int GetChallenge(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
    return -1;
}

//��ȡ�ն˵���Ӧ����
//bLen ��������Ӧ���ݳ���
// pbBuf ESAM���ص���Ӧ����
// �ɹ�  ������Ӧ���ݵĳ��� ���򷵻���Ӧ�Ĵ�����
inline int GetResponse(BYTE bLen,BYTE *pbBuf,DWORD dwTimeout)
{
	return -1;
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
	return -1;
}
//��Կ��֤
//pbRandom ��վ�����8
// bP2 Ҫ��֤��Կ�ļ����ļ���ʶ
// pbSign ����ǩ��128
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int VerifyPublicKey(BYTE bP2,BYTE *pbSign,BYTE *pbMastRandom,DWORD dwTimeout)
{
	return  -1;
}

//���ع�Կ����
// ���ع�Կ�ͳ��� pbKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int UpdateESAMKey(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return -1;
}

//��վ��Կ���ظ���
// ��վ��Կ�ͳ��� pbKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// pbRandom �ն������
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int UpdateMKeyLocal(BYTE *pbKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return -1;
}

//��վ��ԿԶ�̸���
// pbSessionKey  �Ự��Կ 128
// ��վ��Կ�ͳ��� pbMKey bKeyLen 144�ֽ�
// ����ǩ���ͳ���pbSign bSignLen 128�ֽ�
// pbRandom �ն������
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int UpdateMKeyFar(BYTE *pbSessionKey,BYTE *pbMKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return -1;
}

//�Գ���Կ�ĸ���  ����0--ע��ɹ� ����--������
//bNum:������Կ����
// pbSign:����ǩ��128
// pbSessionKey �Ự��Կ128
// �ɹ�  ����0 ���򷵻���Ӧ�Ĵ�����
inline int UpdateSymKey(BYTE bNum,BYTE *pbSessionKey,BYTE *pbTermKey,BYTE *pbTermRandom,BYTE *pbSign,DWORD dwTimeout)
{
	return -1;
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
	return -1;
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
	return -1;
}

extern int CodeToStr(int iErrCode,char *str);

#endif /* _IEC7816_H_ */
