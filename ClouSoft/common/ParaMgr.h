#ifndef _PARAMGR_H
#define _PARAMGR_H

#include "apptypedef.h"
#include "DbOIAPI.h"


#define DATA_TYPE_OAD	0x06
#define DATA_TYPE_OMD	0x07
#define DATA_TYPE_EXT_ID	0xff

typedef int (* TPfnFmtTrans)(BYTE* pBuf);


typedef struct{
	DWORD dwOA;		//object attrib, 高WORD帮OI，低WORD放属性值
	WORD wID;     	//数据项ID,
					//如果为块ID,则wInnerID配为第一个ID的索引,wSubNum为子ID的个数
	WORD wBn;  		//BANK号
	WORD  wPn;		//测量点号
//		BYTE* pFmt;		//格式描述串
//		WORD  wFmtLen;	//格式描述串长度
	TPfnFmtTrans pfnFmtTrans;	//格式转换函数
}TOobFmtMap;

typedef struct{
	WORD wID;     	//数据项ID,
					//如果为块ID,则wInnerID配为第一个ID的索引,wSubNum为子ID的个数
	WORD wBn;  		//BANK号
	WORD  wPn;		//测量点号
	TPfnFmtTrans pfnFmtTrans;	//格式转换函数
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
	int OobParaTrans(char* pUserDir, BYTE opt, BYTE* pbPara);//转存关键参数到面向对象协议DFT文件
    bool SwitchToOob(BYTE* pbPara);
    bool SwitchTo13761(BYTE* pbPara);
private:
	BYTE* m_pbParaData;
	int   m_iParaLen;
	
	void Clean(void);
	bool ParseOAD(BYTE *pbData, DWORD dwLen);
	bool ParseOMD(BYTE *pbData, DWORD dwLen);
	bool ParseExtID(BYTE *pbData, DWORD dwLen);

	//专门增加dft文件写函数，dft文件OAD写是增加，通过通信协议OAD写，是先清除空属性，
	//为了去分这两种情况，增加这三个函数，而不调用原来的函数  whr -- 20170419
	int OoProWriteAttr_dft(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wLen, bool fIsSecurityLayer);
	int OIWrite_Spec_dft(const ToaMap* pOI, BYTE* pbBuf);
	bool IsNeedWrSpec_dft(const ToaMap* pOI);
	int special_write_math(DWORD dwOMD, BYTE *pbData);
	DWORD Search_OMD_from_OAD(DWORD dwOAD);
};
extern CParaMgr g_pmParaMgr;
#endif
