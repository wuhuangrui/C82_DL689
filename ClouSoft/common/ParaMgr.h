#ifndef _PARAMGR_H
#define _PARAMGR_H

#include "apptypedef.h"
#include "DbOIAPI.h"


#define DATA_TYPE_OAD	0x06
#define DATA_TYPE_OMD	0x07
#define DATA_TYPE_EXT_ID	0xff

typedef int (* TPfnFmtTrans)(BYTE* pBuf);


typedef struct{
	DWORD dwOA;		//object attrib, ��WORD��OI����WORD������ֵ
	WORD wID;     	//������ID,
					//���Ϊ��ID,��wInnerID��Ϊ��һ��ID������,wSubNumΪ��ID�ĸ���
	WORD wBn;  		//BANK��
	WORD  wPn;		//�������
//		BYTE* pFmt;		//��ʽ������
//		WORD  wFmtLen;	//��ʽ����������
	TPfnFmtTrans pfnFmtTrans;	//��ʽת������
}TOobFmtMap;

typedef struct{
	WORD wID;     	//������ID,
					//���Ϊ��ID,��wInnerID��Ϊ��һ��ID������,wSubNumΪ��ID�ĸ���
	WORD wBn;  		//BANK��
	WORD  wPn;		//�������
	TPfnFmtTrans pfnFmtTrans;	//��ʽת������
}T13761FmtMap;


class CParaMgr
{
public:
    CParaMgr();
    virtual ~CParaMgr();
    int LoadPara(char* szFileName);
    bool Parse(void);
	int ReSetParamKeepSaveFile(BYTE* pbPara);
	int ReSetParamKeepReadFile();
	int OobParaTrans(char* pUserDir, BYTE opt, BYTE* pbPara);//ת��ؼ��������������Э��DFT�ļ�
    bool SwitchToOob(BYTE* pbPara);
    bool SwitchTo13761(BYTE* pbPara);
private:
	BYTE* m_pbParaData;
	int   m_iParaLen;
	
	void Clean(void);
	bool ParseOAD(BYTE *pbData, DWORD dwLen);
	bool ParseOMD(BYTE *pbData, DWORD dwLen);
	bool ParseExtID(BYTE *pbData, DWORD dwLen);

	//ר������dft�ļ�д������dft�ļ�OADд�����ӣ�ͨ��ͨ��Э��OADд��������������ԣ�
	//Ϊ��ȥ�������������������������������������ԭ���ĺ���  whr -- 20170419
	int OoProWriteAttr_dft(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wLen, bool fIsSecurityLayer);
	int OIWrite_Spec_dft(const ToaMap* pOI, BYTE* pbBuf);
	bool IsNeedWrSpec_dft(const ToaMap* pOI);
	int special_write_math(DWORD dwOMD, BYTE *pbData);
	DWORD Search_OMD_from_OAD(DWORD dwOAD);
};
extern CParaMgr g_pmParaMgr;
#endif
