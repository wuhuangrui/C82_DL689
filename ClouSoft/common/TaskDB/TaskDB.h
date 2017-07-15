/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TaskDB.h
 * ժ    Ҫ���������ݿ�ʵ���ࣨ�������ݱ���������¼��������������
 *
 * ��ǰ�汾��1.1.23
 * ��    �ߣ����
 * ������ڣ�2007-12-28
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
 * ��    ע��1��������������Ŀǰֻ֧��2������
 *           2��������������֧�ֵ�8��
 *           3�����ڳ���2�������ֶε�Ŀǰ��֧�ּ��й����ֶ�����˽���ֶεı�
 *           4�����ڳ���2�������ֶε���������ȡ��¼��ȡʱֻ֧��ǰ2������������ȡ
 *
************************************************************************************************************/
#pragma once
#ifndef _TASKDB_H
#define _TASKDB_H
#include "sysarch.h"
#include "syscfg.h"

#define TDB_PATH			USER_DATA_PATH
#define TDB_TAB_INDEX		"tdbindex.idx"

//#define TDB_SCH_MODE        1   //���ű�������ʱ����ɵü�¼��ʼ���ң�ע�����д����µļ�¼��ʼ����
//���¶����û�������Ը���ʵ��������е���
////////////////////////////////////////////////////////////////////////////////////////
//#define TDB_FIELD_MAX_LEN	256	//������ʱʹ�ã�һ�ʼ�¼�����ֶ�����ܳ���
//#define TDB_MAX_PATH		255 //���ݱ��ļ�����󳤶�
//#define TDB_MAX_TAB_NUM		255	//�������ı�����
//#define TDB_MAX_OPENED_TAB	32	//ϵͳͬһʱ���������Ĵ򿪵ı�
//#define TDB_MAX_BUF_SIZE	0x80000		//��໺��(1MB)�����ݣ�������ô�����ݾͲ����ڴ��л��棬
										//���Ǵ浽�ļ���, ��TDB_BUF_BLOCK_SIZE������
//#define TDB_BUF_BLOCK_SIZE	0x20000		//ÿ����������ʹ�õ���󻺴�(64KB)
//#define TDB_MAX_CONCUR_SCH	5			//��������������
#define TDB_MAX_SCH_FIELD	16			//һ����������������������
#define TDB_SCH_VAL_LEN    255			//�ؼ����ֶε���󳤶�
#define TDB_MAX_OPNUM		10			//һ���ֶ��������Ƚϵķ�����
//#define TDB_RDBLOCK_SIZE	1024		//��ȡ�����С(ϵͳ��һ�ʼ�¼����ܳ���)
//#define TDB_PERI_CACHENUM	10			//��Ҫ��ʱ�λ������ݵ�ʱ����ʱ������໺���¼����
////////////////////////////////////////////////////////////////////////////////////////

#define TDB_INVALID_DATA	0xFF	//��Ч����
////////////////////////////////////////��������
#define TDB_ERR_UNKNOWN			-1	//����δ֪����
#define TDB_ERR_OK				 0	//û�д���
#define TDB_ERR_TBEXIST			-1	//���Ѵ���
#define	TDB_ERR_TBOVERNUM		-2	//����Ŀ��ϵͳ������������
#define TDB_ERR_TBNOEXIST		-3	//������
#define TDB_ERR_TBLOCKED		-4	//��ǰ������(��ĳ���̱߳༭һ�����ʱ�򣬱�������ֹ������༭)
#define TDB_ERR_TBDATA			-5	//���ݴ���Ӧ�������ݿ�����¼ʱ�������¼���ȳ������ݿ�����ĳ��ȷ��ظô���
#define TDB_ERR_INVALID_HANDLE	-6	//����ľ��
#define TDB_ERR_OVER_CONCUR		-7	//������������������Ŀ
#define TDB_ERR_INVALID_FIELD	-8  //������ֶ�
#define TDB_ERR_OVER_BUFFER		-9  //û�л�����
#define TDB_ERR_TBSCHRULENUM	-10 //�������������
#define TDB_ERR_DBLOCKED		-11 //���ݿⱻ����
#define TDB_ERR_UNINIT		    -12 //���ݿ���δ��ʼ��
#define TDB_ERR_NOMEMORY	    -13 //�ڴ治��
////////////////////////////////////////

//�������ݿ�Ŀǰ֧����������
#define TDB_TIME	1	//ʱ������ ������ʱ��	 5	�Ӹ��ֽڵ����ֽڱȽϣ�֧��>��>=��<��<=��=
#define TDB_STR		2	//�����β���ַ���	����	ʹ��strcmp��>��<��=
#define TDB_BCD		3	//BCD����		����	�Ӹ��ֽڵ����ֽڱȽϣ�֧��>��>=��<��<=��=
#define TDB_BYTE	4	//BYTE����		����	�Ӹ��ֽڵ����ֽڱȽϣ�֧��>��>=��<��<=��=
#define TDB_INT16	5	//16λ����������			��ֵ�Ƚϣ�֧��>��>=��<��<=��=
#define TDB_INT32	6	//32λ����������			��ֵ�Ƚϣ�֧��>��>=��<��<=��=
#define	TDB_INT64	7	//64λ����������			��ֵ�Ƚϣ�֧��>��>=��<��<=��=

#define TDB_FIELD_MAX   32  //Ŀǰֻ֧��ÿ�ʼ�¼���32���ֶ�,������һ��DWORD��Ϊ��ȡ��¼ʱȡ�ֶε�����

#define TDB_INVALID_FIELD  0xffff
#define TDB_ALL_FIELD	   0xffffffff
#define TDB_OP_EQ   0  //����
#define TDB_OP_GE   1  //���ڵ���
#define TDB_OP_GT   2  //����
#define TDB_OP_LT   3  //С��
#define TDB_OP_LE   4  //С�ڵ���
#define TDB_OP_MIN  5  //������Сֵ
#define TDB_OP_MAX  6  //�������ֵ

#define TDB_IDX_ANY     0
#define TDB_IDX_SOLE    1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�������ݿ�Ķ���
//���ݱ����ṹ
typedef struct{
	BYTE bVer;										//���ݱ����ṹ�汾
	WORD wTabNum;									//��ǰ����Ŀ
	BYTE* bValidTab;		//BYTE bValidTab[TDB_MAX_TAB_NUM/8+1];			//
	BYTE** bTabName;	//BYTE bTabName[TDB_MAX_TAB_NUM][TDB_MAX_PATH];	//	
}TTabMgr;

//#define TDB_MGR_HEADLEN		((unsigned int)&((TTabMgr*)0)->bTabName)
#define TDB_MGR_SAVE_ALL	0xFFFFFFFF
#define TDB_MGR_SAVE_INDEX	0xFFFFFFFE
//����ƽṹ

typedef struct{
	TTime tmStart;
	TTime tmEnd;
}TTdbPeriTime;//�����ʱ����д��ļ�¼������д��Flash�����Ǵ��뻺�棬���û�����������ʱ��ʱ����

typedef struct{
	char* cTabName;	//char cTabName[TDB_MAX_PATH]; //����
	int iIndex;//��ǰ�����¼����������ָ����һ�������λ��
	int iNum;//��ǰ���������еļ�¼����
	BYTE *pbBuf;
}TTdbPeriCache;

//�й����ֶ�ʱ�ܼ�¼��������wMaxRecPublicNum, 
//ÿ�ʼ�¼����1һ�ʺ������ֶμ�wMaxRecPrivateNum�ʺ����ֶεļ�¼
//û�й����ֶ�ʱ�ܼ�¼��������wMaxRecPrivateNum, 
typedef struct{
	BYTE bVer;		 //����ƽṹ�İ汾
	BYTE bPublicNum; //�����ֶεĸ���
	BYTE bPrivateNum;  //���ֶθ��� 
	DWORD dwMaxRecPublicNum;   //�ɼ�¼��������
	DWORD dwMaxRecPrivateNum;   //�ɼ�¼�Ӹ���
	DWORD dwCurRecNum;    //��ǰ��¼����
	DWORD dwCurRecOffset; //��ǰ��¼
	WORD wField[TDB_FIELD_MAX][3];  //Ԫ��0--�ֶ�����;Ԫ��1--����;Ԫ��2--����
	BYTE bNoCache;
	BYTE bReserve[255];  //Ԥ���ռ�
	BYTE bChkSum;   	//���������ֶε�У���
}TTabCtrl;				//����ƽṹ

typedef struct{
	int			iHandle;	//���ݱ���
	char*		cTabName;	//char		cTabName[TDB_MAX_PATH]; //����
	int			iOpenedCnt;
	DWORD		dwRecLen;	//���ʼ�¼��С
	TSem		tSemaphore;	//��ֹͬһ����ͬʱ���д��		
	TTabCtrl	tTabCtrl;	//�������Ϣ	
}TTdbActivedTabInfo;//�״̬�ı���Ϣ

typedef struct{
	WORD wField;  //�ֶ����
	WORD wOpNum;  //�ȽϷ�������
	WORD wOp[TDB_MAX_OPNUM];    //�ȽϷ�����TDB_OP_EQ/TDB_OP_GE/TDB_OP_GT/TDB_OP_LT/TDB_OP_LE
	BYTE bVal[TDB_MAX_OPNUM][TDB_SCH_VAL_LEN];  //�Ƚ�ֵ
}TTdbSchRule;			//�����¼������һ������Ŀǰֻ֧�ֶ����������

typedef struct{
	WORD	wID;	//����ID���������ֲ�ͬ��Ӧ���ύ������
	int		iTabIndex;	//�������ı��
	//��ѯ�ñ��ռ�õĻ���λ��
	TTdbSchRule rTdbSchRule[TDB_MAX_SCH_FIELD];
	BYTE*	pbBufIdxTab;//BYTE	dwBufIdxTab[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE];		
	char*	pcFileName;	//char	cFileName[TDB_MAX_PATH];	//���ɷ���������Ļ��治���õ�ʱ����ʹ���ļ�����
									//��Ϊ�����ļ���·��(�����ļ���)
}TTdbSchInfo;

typedef struct{
	//-------------------Ӧ�ø��������ݿ�Ĳ�������-----------------------------------------------
	//��¼�������
	WORD wSortNum;		//����������
	WORD wSortFild[TDB_MAX_SCH_FIELD];    //�ֶ�ID��TDB_INVALID_FIELD��ʾ���ֶ���Ч
	WORD wSortOp[TDB_MAX_SCH_FIELD];      //�ȽϷ�����TDB_OP_GE/TDB_OP_LE:����������߽���Ĺ�������,�����м�ֵ�ظ����������
				      	  //		  TDB_OP_GT/TDB_OP_LT:����������߽���Ĺ�������,�������м�ֵ�ظ����������
	
	WORD wRecsToSch;	//�ܹ�Ҫ�����ļ�¼��������������������������ݿ��ֹͣ����
	//---------------------�������ݿⷵ�ظ�Ӧ�õ���Ϣ-------------------------------------------
	WORD wRecSize;			//ÿ�ʵļ�¼��С
	WORD wRecsFound;		//�ܹ�����Ҫ��ļ�¼����,��TdbOpenSch()�г�ʼ��
	BYTE bValFirstRec[TDB_MAX_SCH_FIELD][TDB_SCH_VAL_LEN];  //��һ�ʼ�¼�������ֶε�ֵ
							//����Ҫ��ĵ�һ�ʼ�¼��ʱ��,Ӧ���ڰ��ռ������ȡ����ʱ,
							//���Ը��ݷ��ص����ʱ���������Ժ��ÿ�ʼ�¼��ʱ��
	//-----------------------�������ݿ��������������Ĳ���---------------------------------------
	int iPublicRecStart;    //�ļ��п�ʼ��������ʼ��¼�������ʼ��Ӧ���趨Ϊ-1��ʾ��һ�ε��ã�
							//������TdbInitSch()�趨Ϊ����һ�ؿ�ʼ��������ʼλ��, �����ֶ�
	int iPrivateRecStart;	//���ֶ�
					
//	WORD wIndexID;	//ʹ�����������ݿ�������������ı�ʶ	
}TTdbSchCtrl;	    //�������ƽṹ,֧�ֶ�������

typedef struct{
	//-------------------Ӧ�ø��������ݿ�Ĳ�������-----------------------------------------------
	DWORD dwFiledNeed;      //��Ҫ�������ֶ�,0~31λ����Ӧһ���ֶ�,0xffffffff��ʾȫ����
							//�����޶�ÿ�ʼ�¼���32���ֶ�,��Щ�ֶ����û��Ҫ��������Ժϲ�Ϊһ��BYTE_STR,���ӳ��ȼ���
							//��Щʱ����Ҫȡ��¼��ʱ����Ҫʱ����߱��ַ,����ʹ�ñ��ֶ�������
							//ͬһ�ɼ�����Ķ������ݺϳ�һ��������ִ��ʱ, ����ɼ��������е�ң�����ŵ���һ��������,
							//����ͨ�����ֶ�����ȡ��Ҫ��������	
	
	//-----------------------�������ݿ��������������Ĳ���---------------------------------------
	int iRecStart;			//������������ʼλ��,�����������ʼ��Ӧ�ó�ʼ��Ϊ-1;
							//��������TdbReadRec()�ĵ��ö��ı�
} TTdbReadCtrl;   //����¼���ƽṹ

class CTaskDB
{
public:
	CTaskDB(void);
public:
	~CTaskDB(void);
private:
    bool        m_fInit;
	TTabMgr		m_TTabMgr;			//���ݱ����ṹ
	TTdbActivedTabInfo*	m_ptatHandle;//TTdbActivedTabInfo	m_tatHandle[TDB_MAX_OPENED_TAB];		//��ǰ���򿪵ı�������
	TSem		m_tsTabMgrSemaphore;
	TSem		m_tsTabSemaphore;
	BYTE*		m_pbSchBuf;//BYTE		m_pbSchBuf[TDB_MAX_BUF_SIZE];		//�������������������, ʹ�ù�������黺�汻�ֳɺܶ�С��
													//���������������
	BYTE*		m_pbBufIdxTab;//BYTE		m_dwBufIdxTab[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE];
	TTdbSchInfo*	m_ptsiConcurSch;//TTdbSchInfo	m_tsiConcurSch[TDB_MAX_CONCUR_SCH];	//����
	BYTE*		m_pbReadBuffer;//BYTE		m_bReadBuffer[TDB_FIELD_MAX_LEN];	//��������/��������л���һ�ʼ�¼
	BYTE*       m_pbRecBuffer;//BYTE        m_bRecBuffer[TDB_MAX_CONCUR_SCH*TDB_RDBLOCK_SIZE];   //��ȡ��¼���棬�������ٶ�ȡ��¼�Ĵ���������ٶ�
	BYTE*       m_pbDelBuffer;//BYTE        m_pbDelBuffer[TDB_MAX_CONCUR_SCH*TDB_RDBLOCK_SIZE];   //ɾ����¼���棬��������д��¼�Ĵ���������ٶ�
	bool		m_fLocked;	//���ݱ��Ƿ�����

	//���ò���
	WORD		m_wMaxPath;		//���ݱ��ļ�����󳤶�
	WORD		m_wMaxTabNum;	//�������ı�����
	DWORD		m_dwMaxFieldLen; //һ�ʼ�¼����ܳ���
	WORD		m_wMaxOpenedTabNum;//ϵͳͬһʱ���������Ĵ򿪵ı������
	DWORD		m_dwMaxBufSize;	//����������û����С��Ϊm_dwBufBlkSize����������[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE]
	DWORD		m_dwBufBlkSize;//�����ʹ�õĻ�����С���С��һ������Ϊ����Ϊ�ؼ��ֵ�����ֶγ���[���Գ���1��]+8��*n��nΪ����¼������
	DWORD		m_dwMaxCCSchNum;//�����������ĸ���
	DWORD		m_dwMaxRecLen;//��ȡ�����С(һ�ʼ�¼��ĳ���)
	//DWORD		m_dwMaxSchField;//һ�������������Ĳ��������Ĺؼ��ָ���
	//DWORD		m_dwKeyLen;//��Ĺؼ����ֶγ���
	DWORD		m_dwPeriCacheNum;//��Ҫ��ʱ�λ������ݵ�ʱ����ʱ������໺���¼����
	WORD		m_wSchMode;//0-�����µļ�¼��ʼ���ң�1-����ɵü�¼��ʼ����
	char		m_szTdbDataPath[255];//�������ݿ����ݴ��λ��

private:
	int			GetTableIndex(const char* pszName);//ָ��������λ��
	int			GetEmptyIndex();//�����ݱ����ṹ���ҳ�һ����λ��
	int			SaveMgrData(DWORD dwOffset);//�������ݱ����ṹ��Ϣ 0xFFFFFFFF ֻ��������
	BYTE		CheckSum(const BYTE* bBuf, const WORD wLen);
	inline int	GetActiveTabInfo(int fd);	//fdΪ-1ʱ������һ���յ�λ��
	int			AllocBuf();		//Ϊ�������仺��
	int			AllocSchID();				//����һ������ID
	int			ReadOneRec(int fd, int iSchId, BYTE* pbBuf, DWORD dwOffset, DWORD dwLen);
	int         DeleteOneRec(int fd, int iSchId, DWORD dwOffset, DWORD dwLen);
	bool		Compare(BYTE *pbSrcBuf, BYTE *pbDesBuf, DWORD dwLen, WORD wOp, WORD wFieldType);
				//���й����ֶΣ�����������ֻ�й����ֶ�
	int			OpenPublicSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl &rTdbSchCtrl, BYTE *p);
				//���й����ֶΣ�����������ֻ�����ֶ�
	int			OpenPrivateSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl &rTdbSchCtrl, BYTE *p);
				//���й����ֶΣ����������м��й����ֶ��������ֶ�
	int			OpenAllSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl &rTdbSchCtrl, BYTE *p);
				//�������й����ֶ�
	int			OpenNoPublicSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl &rTdbSchCtrl, BYTE *p);
	int			DeleteNoPublicSch(int iIndex, int iSchId, TTdbSchRule *pTdbSchRule, const WORD wRuleNum);
	void		SortRec(int iSchId, TTdbSchCtrl &rTdbSchCtrl, const WORD wRuleNum, BYTE *p, int iIndex);//�������������
	bool		IsAllByte(BYTE bDesByte, WORD wLen, BYTE *p);
	int 		GetBuffer(int iSchId, BYTE *pbBuf, DWORD dwOffset, DWORD dwLen);
	int         SetBuffer(int iSchId, DWORD dwOffset, DWORD dwLen);
	int 		FreeBuffer(int iSchId);
	bool		IsLocked() { return m_fLocked; };

public:
	void		SetMaxPath(const WORD wMaxPath) { m_wMaxPath = wMaxPath; };		//���ݱ��ļ�����󳤶�
	void		SetMaxTabNum(const WORD wMaxTabNum) { m_wMaxTabNum = wMaxTabNum; };	//�������ı�����
	void		SetMaxFieldLen(const DWORD dwMaxFieldLen) { m_dwMaxFieldLen = dwMaxFieldLen; }; //һ�ʼ�¼����ܳ���
	void		SetMaxOpenedTabNum(const WORD wMaxOpenedTabNum) { m_wMaxOpenedTabNum = wMaxOpenedTabNum; };//ϵͳͬһʱ���������Ĵ򿪵ı������
	void		SetMaxBufSize(const DWORD dwMaxBufSize) { m_dwMaxBufSize = dwMaxBufSize; };	//����������û����С��Ϊm_dwBufBlkSize����������[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE]
	void		SetBufBlkSize(const DWORD dwBufBlkSize) { m_dwBufBlkSize = dwBufBlkSize; };//�����ʹ�õĻ�����С���С��һ������Ϊ����Ϊ�ؼ��ֵ�����ֶγ���[���Գ���1��]+8��*n��nΪ����¼������
	void		SetMaxSchNum(const DWORD dwMaxCCSchNum) { m_dwMaxCCSchNum = dwMaxCCSchNum; };//�����������ĸ���
	void		SetMaxRecLen(const DWORD dwMaxRecLen) { m_dwMaxRecLen = dwMaxRecLen; };//��ȡ�����С(һ�ʼ�¼��ĳ���)
//	void		SetMaxSchField(const DWORD dwMaxSchField) { m_dwMaxSchField = dwMaxSchField; };//һ�������������Ĳ��������Ĺؼ��ָ���
//	void		SetKeyWordLen(const DWORD dwKeyLen) { m_dwKeyLen = dwKeyLen; };//��Ĺؼ����ֶγ���
	void		SetPeriCacheNum(const DWORD dwPeriCacheNum) { m_dwPeriCacheNum = dwPeriCacheNum; };//��Ҫ��ʱ�λ������ݵ�ʱ����ʱ������໺���¼����
	void		SetSchMode(const WORD wMode) { m_wSchMode = wMode; };//0-�����µļ�¼��ʼ���ң�1-����ɵü�¼��ʼ����
	void		SetTdbDataPath(char* pszPath) { if (strlen(pszPath) < sizeof (m_szTdbDataPath)) strcpy(m_szTdbDataPath, pszPath); }

	WORD		GetMaxPath() { return m_wMaxPath; };		//���ݱ��ļ�����󳤶�
	WORD		GetMaxTabNum() { return m_wMaxTabNum; };	//�������ı�����
	DWORD		GetMaxFieldLen() { return m_dwMaxFieldLen; }; //һ�ʼ�¼����ܳ���
	WORD		GetMaxOpenedTabNum() { return m_wMaxOpenedTabNum; };//ϵͳͬһʱ���������Ĵ򿪵ı������
	DWORD		GetMaxBufSize() { return m_dwMaxBufSize; };	//����������û����С��Ϊm_dwBufBlkSize����������[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE]
	DWORD		GetBufBlkSize() { return m_dwBufBlkSize; };//�����ʹ�õĻ�����С���С��һ������Ϊ����Ϊ�ؼ��ֵ�����ֶγ���[���Գ���1��]+8��*n��nΪ����¼������
	DWORD		GetMaxSchNum() { return m_dwMaxCCSchNum; };//�����������ĸ���
	DWORD		GetMaxRecLen() { return m_dwMaxRecLen; };//��ȡ�����С(һ�ʼ�¼��ĳ���)
//	DWORD		GetMaxSchField() { return m_dwMaxSchField; };//һ�������������Ĳ��������Ĺؼ��ָ���
//	DWORD		GetKeyWordLen() { return m_dwKeyLen; };//��Ĺؼ����ֶγ���
	DWORD		GetPeriCacheNum() { return m_dwPeriCacheNum; };//��Ҫ��ʱ�λ������ݵ�ʱ����ʱ������໺���¼����
	WORD		GetSchMode() { return m_wSchMode; };//0-�����µļ�¼��ʼ���ң�1-����ɵü�¼��ʼ����
	char*		GetTdbDataPath() { return m_szTdbDataPath;};

	bool		Init(char* pszPath=NULL);	//��ʼ�����ݱ������Ϣ
	int			CreateTable(const char* pszName, TTabCtrl& rTabCtrl);//����ָ����
	int			ClearRec(const char* pszName);//���ָ����ļ�¼
	int         DeleteSchRec(int fd, TTdbSchRule *pTdbSchRule, WORD wRuleNum); //ɾ���������������ļ�¼
	int			DeleteTable(const char* pszName);//ɾ��ָ����
	int			OpenTable(const char* pszName, int iOpenFlag);//��ָ����
	int			CloseTable(const int fd);//�ر�ָ����
	int			AppendRec(const int fd, int index, const BYTE* pbRec); //���ָ��λ��һ�ʼ�¼
	int			AppendRec(const int fd, const BYTE* pbRec, bool fUseCache=true);//���һ����¼
	int			ReadRec(const int iSchId, TTdbSchRule* pTdbSchRule, const WORD wRuleNum, TTdbReadCtrl& rTdbReadCtrl, BYTE* pbBuf);
	int         ReadRec(const int fd, int index, BYTE* pbBuf, int iLen);
	int			ReadRec(const int fd, int* piStart, BYTE* pbBuf, WORD wBufSize, bool fFromEnd, WORD* pwRetNum);
	int         GetRecNum(const int fd);
	int			GetMaxRecNum(const int fd);
	int         GetRecPtr(const int fd);
				//������ؼ�¼
	int			OpenSch(const int fd, TTdbSchRule* pTdbSchRule, const WORD wRuleNum, TTdbSchCtrl& rTdbSchCtrl);
				//����������ؼ�¼
	int			CloseSch(const int iSchId);//��������	
	inline bool IsInit() { return m_fInit; };
	void		Lock();
	void		UnLock();
};


//////////////////////////////////////////////////////////////////////////////////////
//ȫ�ֶ���
extern CTaskDB* GetTaskDB();

//��������ʼ���������ݿ�
//������@ pszPath ����Ᵽ��·��
inline bool TdbInit(char* pszPath=NULL)
{
	return GetTaskDB()->Init(pszPath);
}

//�����������ݿ����봴��һ�ű�
//������@pszName �������,��һ������·�����ļ���,���ݿ���Զ����ļ����浽���ݿ��Ŀ¼��
//      @wRecSize ÿ�ʼ�¼�Ĵ�С
//      @wRecNum  ��¼�ĸ���
//���أ������ȷ�򷵻�true�����򷵻�false
inline int TdbCreateTable(char* pszName, TTabCtrl& rTabCtrl)
{
	return GetTaskDB()->CreateTable(pszName, rTabCtrl);
}

//���������ָ����ļ�¼�����ǲ�ɾ����
//������@pszName �������
//���أ��޴��󷵻�TDB_ERR_OK ,����������ͼ�ͷ�ļ���"��������"
inline int TdbClearRec(char* pszName)
{
	return GetTaskDB()->ClearRec(pszName);
}

//�����������ݿ�����ɾ��һ�ű�
//������@pszName �������
//���أ������ȷ�򷵻�true�����򷵻�false
inline int TdbDeleteTable(char* pszName)
{
	return GetTaskDB()->DeleteTable(pszName);
}

//�����������ݿ������һ�ű�
//������@pszName �������
//	   @ iOpenFlag ����ͬ�ļ�ϵͳ�д��ļ�����open�е�flag��ͬ
//		 (����������O_RDONLY/O_WRONLY/O_RDWR)
//���أ������ȷ�򷵻ر�ı�ʶ�������ڵ���0�����򷵻ظ���
inline int TdbOpenTable(char* pszName, int iOpenFlag)
{
	return GetTaskDB()->OpenTable(pszName, iOpenFlag);
}

//�����������ݿ�����ر�һ�ű�
//������@pszName �������
//���أ������ȷ�򷵻�true�����򷵻�false
inline int TdbCloseTable(int fd)
{
	return GetTaskDB()->CloseTable(fd);
}

//�����������ݿ�ı�fd���һ�ʼ�¼
//������@fd ���ݿ��ı�ʶ��
//      @pbRec ָ���¼�Ļ�����
//���أ������ȷ�򷵻�true�����򷵻�false
inline int TdbAppendRec(int fd, BYTE* pbRec)
{
	return GetTaskDB()->AppendRec(fd, pbRec);
}

//�����������ݿ�ı�fd���ָ��λ�õ�һ�ʼ�¼
//������@fd ���ݿ��ı�ʶ��
//      @index ��¼λ��
//	    @pbRec ָ���¼�Ļ�����
//���أ������ȷ�򷵻ؼ�¼�ĳ���,С�ڱ�ʾ����,0��ʾ�޴�λ�õļ�¼
inline int TdbAppendRec(const int fd, int index, const BYTE* pbRec)
{
	return GetTaskDB()->AppendRec(fd, index, pbRec);
}

//������ɾ���������ݿ���������ļ�¼
//     @fd ���ݿ��ı�ʶ��
//     @pTdbSchRule ָ�������¼�������������ָ��
//     @wRuleNum ���������
//���أ������ȷ�򷵻ر���ɾ�����������򷵻ظ���
inline int TdbDeleteSchRec(int fd, TTdbSchRule *pTdbSchRule, WORD wRuleNum)
{
	return GetTaskDB()->DeleteSchRec(fd, pTdbSchRule, wRuleNum);
}

//��������ʼ����������
//     @fd ���ݿ��ı�ʶ��
//     @pTdbSchRule ָ�������¼�������������ָ��
//     @wRuleNum ���������
//     @rTdbSchCtrl �������ƽṹ
//���أ������ȷ�򷵻�������id>=0�����򷵻ظ���
inline int TdbOpenSch(int fd, TTdbSchRule* pTdbSchRule, WORD wRuleNum, TTdbSchCtrl& rTdbSchCtrl)
{
	return GetTaskDB()->OpenSch(fd, pTdbSchRule, wRuleNum, rTdbSchCtrl);
}

//�������ر���������
//������@id ������id
//���أ������ȷ�򷵻�true�����򷵻�false
inline int TdbCloseSch(int id)
{
	return GetTaskDB()->CloseSch(id);
}

//�����������ݿ��и������������ȡ��¼,����Ƕ�����¼�ı�,�򷵻صļ�¼�ǹ����ֶμ����Ӽ�¼
//������@fd ���ݿ��ı�ʶ��
//      @pTdbSchRule ָ�������¼�������������ָ��
//      @wRuleNum ���������
//      @rTdbSchCtrl �������ƽṹ
//	    @pbBuf �������������¼�Ļ�����
//���أ������ȷ�򷵻ػ������ڼ�¼�ĳ���,С�ڱ�ʾ����,0��ʾ������
inline int TdbReadRec(int iSchId, 
			   TTdbSchRule* pTdbSchRule, WORD wRuleNum, TTdbReadCtrl& rTdbReadCtrl, BYTE* pbBuf)
{
	return GetTaskDB()->ReadRec(iSchId, pTdbSchRule, wRuleNum, rTdbReadCtrl, pbBuf);
}

//�����������ݿ��ж�ȡָ��λ�õ�һ�ʼ�¼
//������@fd ���ݿ��ı�ʶ��
//      @index ��¼λ��
//	    @pbBuf �������������¼�Ļ�����
//���أ������ȷ�򷵻ػ������ڼ�¼�ĳ���,С�ڱ�ʾ����,0��ʾ������
inline int TdbReadRec(const int fd, int index, BYTE* pbBuf, int iLen)
{
	return GetTaskDB()->ReadRec(fd, index, pbBuf, iLen);
}

//���������������ָ������������ȡ��ʼ�¼
//������@fd���ݿ��ı�ʶ��
//	   @piStart�������뼰���ؼ�¼��������ʼλ��,��һ�ε��ô���-1��
//				���������߲��ܸı�,��������ʱ����������������Ϊ-2
//@pbBuf�������������¼�Ļ�����
//     @wBufSize�������Ĵ�С
//     @fFromEnd�Ƿ�����һ�ʼ�¼��ʼ��������������ӵ�һ�ʼ�¼˳������
//   @pwRetNum���ζ�ȡ���ļ�¼������ֻ�����ļ�����ʱ�Ż�û�ﵽҪ�����
//���أ������ȷ�򷵻ػ������ڼ�¼�ĳ���,С��0��ʾ����,0��ʾ������
inline int TdbReadRec(const int fd, int* piStart, BYTE* pbBuf, WORD wBufSize, bool fFromEnd, WORD* pwRetNum)
{
	return GetTaskDB()->ReadRec(fd, piStart, pbBuf, wBufSize, fFromEnd, pwRetNum);
}

//��������ǰ���¼��
//������@fd ���ݿ��ľ��
//���أ���ǰ��ļ�¼��,С�ڱ�ʾ����
inline int TdbGetRecNum(const int fd)
{
    return GetTaskDB()->GetRecNum(fd);
}



//��������ǰ�����ɼ�¼����
//������@fd ���ݿ��ľ��
//���أ���ǰ������ɼ�¼����,С�ڱ�ʾ����
inline int TdbGetMaxRecNum(const int fd)
{
	//return GetTaskDB()->GetMaxRecNum(fd);
	return 1200;   //for test whr 20170715
}


//��������ǰ���¼ָ��
//������@fd ���ݿ��ľ��
//���أ���ǰ��ļ�¼ָ��,С�ڱ�ʾ����
inline int TdbGetRecPtr(const int fd)
{
    return GetTaskDB()->GetRecPtr(fd);
}

//�������ݿ⣬��ɾ������ʱʱ����ܱȽϳ�������Ҫ���������ݿ⡣
inline void TdbLock()
{
    GetTaskDB()->Lock();
}

inline void TdbUnLock()
{
    GetTaskDB()->UnLock();
}

//������Ƿ��ѳ�ʼ��
inline bool TdbIsInit() 
{ 
    return GetTaskDB()->IsInit(); 
}

//�������ݱ��ļ�����󳤶�(Ĭ��ֵ:255)
inline void TdbSetMaxPath(const WORD wMaxPath)
{
	GetTaskDB()->SetMaxPath(wMaxPath);
}

//�����������ı�����(Ĭ��ֵ:255)
inline void TdbSetMaxTabNum(const WORD wMaxTabNum)
{
	GetTaskDB()->SetMaxTabNum(wMaxTabNum);
}

//����һ�ʼ�¼����ܳ���(Ĭ��ֵ:256)
inline void TdbSetMaxFieldLen(const DWORD dwMaxFieldLen)
{
	GetTaskDB()->SetMaxFieldLen(dwMaxFieldLen);
}

//����ϵͳͬһʱ���������Ĵ򿪵ı������(Ĭ��ֵ:32)
inline void TdbSetMaxOpenedTabNum(const WORD wMaxOpenedTabNum)
{
	GetTaskDB()->SetMaxOpenedTabNum(wMaxOpenedTabNum);
}

//��������������û����С��ΪSetBufBlkSize����������(Ĭ��ֵ:0x80000)
inline void TdbSetMaxBufSize(const DWORD dwMaxBufSize)
{
	GetTaskDB()->SetMaxBufSize(dwMaxBufSize);
}

//���������ʹ�õĻ�����С���С��һ������Ϊ����Ϊ�ؼ��ֵ����
//�ֶγ���[���Գ���1��]+8��*n��nΪ����¼������(Ĭ��ֵ:0x20000)
inline void TdbSetBufBlkSize(const DWORD dwBufBlkSize)
{
	GetTaskDB()->SetBufBlkSize(dwBufBlkSize);
}

//���������������ĸ���(Ĭ��ֵ:5)
inline void TdbSetMaxSchNum(const DWORD dwMaxCCSchNum)
{
	GetTaskDB()->SetMaxSchNum(dwMaxCCSchNum);
}

//���ö�ȡ�����С(һ�ʼ�¼��ĳ���)(Ĭ��ֵ:1024)
inline void TdbSetMaxRecLen(const DWORD dwMaxRecLen)
{
	GetTaskDB()->SetMaxRecLen(dwMaxRecLen);
}

/*
//����һ�������������Ĳ��������Ĺؼ��ָ���(Ĭ��ֵ:8)
inline void TdbSetMaxSchField(const DWORD dwMaxSchField)
{
	GetTaskDB()->SetMaxSchField(dwMaxSchField);
}

//������Ĺؼ����ֶγ���(Ĭ��ֵ:255)
inline void TdbSetKeyWordLen(const DWORD dwKeyLen)
{
	GetTaskDB()->SetKeyWordLen(dwKeyLen);
}
*/

//������Ҫ��ʱ�λ������ݵ�ʱ����ʱ������໺���¼����(Ĭ��ֵ:10)
inline void	TdbSetPeriCacheNum(const DWORD dwPeriCacheNum)
{
	GetTaskDB()->SetPeriCacheNum(dwPeriCacheNum);
}

//0-�����µļ�¼��ʼ���ң�1-����ɵü�¼��ʼ����
inline void TdbSetSchMode(const WORD wMode)
{
	GetTaskDB()->SetSchMode(wMode);
}

//���ݱ��ļ�����󳤶�
inline WORD	TdbGetMaxPath() 
{ 
	return GetTaskDB()->GetMaxPath();
}

//�������ı�����
inline WORD	TdbGetMaxTabNum() 
{ 
	return GetTaskDB()->GetMaxTabNum(); 
}

//һ�ʼ�¼����ܳ���
inline DWORD TdbGetMaxFieldLen()
{ 
	return GetTaskDB()->GetMaxFieldLen(); 
}

//ϵͳͬһʱ���������Ĵ򿪵ı������
inline WORD TdbGetMaxOpenedTabNum() 
{ 
	return GetTaskDB()->GetMaxOpenedTabNum(); 
}

//����������û����С��Ϊm_dwBufBlkSize����������[TDB_MAX_BUF_SIZE/TDB_BUF_BLOCK_SIZE]
inline DWORD TdbGetMaxBufSize() 
{ 
	return GetTaskDB()->GetMaxBufSize();
}

//�����ʹ�õĻ�����С���С��һ������Ϊ����Ϊ�ؼ��ֵ�����ֶγ���[���Գ���1��]+8��*n��nΪ����¼������
inline DWORD TdbGetBufBlkSize() 
{ 
	return GetTaskDB()->GetBufBlkSize(); 
}

//�����������ĸ���
inline DWORD TdbGetMaxSchNum() 
{ 
	return GetTaskDB()->GetMaxSchNum();
}

//��ȡ�����С(һ�ʼ�¼��ĳ���)
inline DWORD TdbGetMaxRecLen() 
{ 
	return GetTaskDB()->GetMaxRecLen(); 
}

/*
//һ�������������Ĳ��������Ĺؼ��ָ���
inline DWORD TdbGetMaxSchField() 
{ 
	return GetTaskDB()->GetMaxSchField(); 
}

//��Ĺؼ����ֶγ���
inline DWORD TdbGetKeyWordLen() 
{ 
	return GetTaskDB()->GetKeyWordLen(); 
}
*/

//��Ҫ��ʱ�λ������ݵ�ʱ����ʱ������໺���¼����
inline DWORD TdbGetPeriCacheNum() 
{ 
	return GetTaskDB()->GetPeriCacheNum(); 
}

//0-�����µļ�¼��ʼ���ң�1-����ɵü�¼��ʼ����
inline WORD TdbGetSchMode()
{
	return GetTaskDB()->GetSchMode();
}

//�����������ݿ����ݴ�ŵĵط�
inline void SetTdbDataPath(char* pszPath)
{
	GetTaskDB()->SetTdbDataPath(pszPath);
}

//ȡ�õ�ǰ�������ݿ����ݴ�ŵĵط�
inline char* TdbGetTdbDataPath()
{
	return GetTaskDB()->GetTdbDataPath();
}
#endif
