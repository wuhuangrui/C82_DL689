/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�LibDbStruct.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֱ�׼ϵͳ�������ݽṹ����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 *********************************************************************************************************/
#ifndef LIBDBSTRUCT_H
#define LIBDBSTRUCT_H
#include "sysarch.h"
#include "LibDbConst.h"

#define DI_READ      0x01
#define DI_WRITE     0x02
#define DI_NTS     	 0x04		//No TimeStamp 	
								//��һ����ʱ���BANK��,ĳ�����������ϱ���־,�ͱ�ʾ�������֧��ʱ��
								//��Ҫ����BANK1�����BANK,�����д�ʱ��Ͳ���ʱ���,�����ֲ��ֿܷ�SECT�����
								//��������BANK���ó�֧��ʱ�����,���������������óɲ�֧��ʱ��
#define DI_CMB     	 0x08		//���ID,���ϱ���־�������������Ϊ0,�򲻷���洢�ռ��ʱ��ռ�

#define DI_LOW_PERM    0x00
#define DI_HIGH_PERM   0x11
//#define DI_HIGH_SUPER  0x02   //��������ʹ��
#define DI_LAB_PERM    0x20		//ʵ����״̬
#define DI_PRG_PERM    0x40		//��̿��رպ�

//�����������־λ
#define DI_FLG_BLK		0x01	//��ID

typedef struct{
    WORD  wID;			//�������ʶ
    WORD  wLen;			//�������,�����������������������б�������Ϊ0
    WORD  wPerm;		//Ȩ��
    WORD  wRW;			//��д����
    WORD  wOffset;		//����������ݿ鿪ʼ��ƫ��,�ϵ�ʱ����
						//���ͬʱ����f��ff���ݿ�,��wOffset��ff���ݿ��ƫ��
    WORD  wWrOp;		//д��������ʱ�����ݿ�ִ�е��Զ�����,��дҪ���͵���Ϣ��
    WORD  wFormat;		//�������ʽ
    WORD  wPnNum;		//������ĸ���,ֻ�п���������������������������ʵ�ĸ���,�������������1
						//������������ff���ݿ�,��f���ݿ�Ҳ��1.
						//Ϊ�����ڷ����ڳ�ʼ���׶μ����������ƫ��
	const BYTE* pbFmtStr;//��ʽ������
	BYTE  bPnMapSch;	//��������ѡ�õĶ�̬�����㷽����,0x00��ʾ��֧�ֶ�̬������

	//�����ֶ����ϵ�ʱ����,�������������в�����д
	DWORD dwBlockStart;  //�����ݵĿ�ʼ����������
	
	WORD  wBlockLen; 	 //�����ݵĳ���
	DWORD dwBlockOffset; //�����ݵ�����ڱ����ݱ�ͷ��ƫ��
						 //���ͬʱ����f��ff���ݿ�,��dwBlockOffset��ff���ݿ��ƫ��
	//DWORD dwItemOffset;  //������ƫ�ƣ���ʱʹ��
	
	DWORD dwBlkIndex;	 //�������ݿ��е���ʼ����,���ղ�����չ����
	BYTE  bBlkIndexNum;	 //�����������������������,���Ϊ2����
	BYTE  bBlkIdIndexNum;//��ID�Լ����е���������ĸ���	
	BYTE  bInnerIndex;	 //�����������ڿ��ڵ����,���ղ�����չ�����˳��

	//WORD wDefaultOffset; //Ĭ��ֵ��ƫ��,ÿ��������ֻ��һ��
	BYTE bSelfItem;			//���������Ƿ����Գɶ���������
	BYTE bItemFlg;			//�����������־λ
}TItemDesc;         //����������

//һ���������ƫ��:dwBlockOffset+������*wBlockLen+wOffset
//������Ŀ�������:wLen
//���ղ�����չ����һ�������������:dwBlkIndex+������*bBlkIndexNum+bInnerIndex

typedef struct{
	WORD wLen;		//����
	WORD wSect;		//��
	WORD wPnNum;	//���������
}TItemInfo;		//���������Ϣ


typedef struct{
	WORD wID;     	//BANK0����645ID
	WORD wPn;  		//�������
}TBank0Item;		//��Ҫ���������������ʱ���ѯ,
					//Ŀǰֻ֧��BANK0,�������֧�ֵ����BANK,���ƿɸ�ΪTBankItem,
					//�Ҽ���bBank�ֶ�

typedef struct{
	WORD wBn;  		//BANK��
	WORD wPn;  		//�������
	WORD wID;     	//������ID
}TBankItem;			//��Ҫ���������������ʱ���ѯ


typedef struct{
	char*	   	pszBankName;	//��BANK������
    char*      	pszPathName;   	//��BANK���ݱ����·���ļ���
	char*      	pszBakPathName; //�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
    TItemDesc* 	pItemDesc;     //��BANK������������
    DWORD      	dwItemNum;     //��BANK����������������������
    BYTE*  	   	pbDefault;     //��BANK���ݿ��Ĭ��ֵ
    DWORD  	   	dwDefaultSize; //��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	BYTE	   	bVer;		  //��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	WORD	   	wPnNum;		  //��BANK����ʵ��֧�ֵĲ��������,
							  //��������BANK��֧�ֲ�����,��������BANK�������������������֧�ֲ�����
							  //����ֻ�ܾ���һ	
	WORD	   	wImgNum;		  //����ĸ���
					//���wPnNum��wImgNumͬʱ����Ϊ0, 
					//��ʾ��BANK��ֻ��Ϊ������������,��ʼ��ʱ������洢�ռ�
					//���������ݷ���Ҫ����Ӧ�Ķ�д����
	bool	   	fUpdTime;      //��BANK�����Ƿ���Ҫ����ʱ��
	BYTE	   	bPnMapSch;	  //��SECTION����ѡ�õĶ�̬�����㷽����,0x00��ʾ��֧������BANK��֧�ֶ�̬������
	WORD		wSaveInterv;	//������,��λ����,Ϊ0��ʾ����ϵͳ��ͳһ�ļ�����б���

	//�����ֶ����ϵ�ʱ����,�������в�����д
	TSem   	   	semBankRW;	  //BANK���ݵĶ�д����
    BYTE*  	   	pbBankData;	  //��BANK���ݿ������,
							  //����ж��������,��ֻ����һ���������в�����Ĵ󻺳�,
							  //pbBankDataָ���ܵ���ʼ��ַ
	DWORD      	dwBankSize;    //��BANK���ݵĴ�С
	DWORD*	   	pdwUpdTime;	  //��BANK���ݵĸ���ʱ��
	bool	   	fMutiPnInDesc; //�������д��ڶ�������������
	DWORD	   	dwTotalSize;	  //��BANK�������ݵĴ�С,��pbBankData����ռ�Ĵ�С
	DWORD      	dwIndexNum;    //�����������ĸ���,��dwItemNum���ղ��������չ����ĸ���
							  //Ŀǰֻͳ�Ʒ���ռ�(��ʱ��)��������ĸ���,���ID������
	WORD	   	wFileNum;	  //һ���ֳɶ��ٸ��ļ�
	DWORD	   	dwFileSize;	  //ÿ���ļ��Ĵ�С
	bool	   	fOldFileExist; //�ɰ汾�ļ�����,���ȱ�����ȫ����
	BYTE	   	bModified[BANK_FILE_FLG_SIZE];   //��BANK���ݵ��޸ı�־,ÿ1λ��ʾһ��������
	
	DWORD		dwMemUsage;	  //�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�
	DWORD		dwSaveClick;  //��BANK���ݱ����ʱ��
}TBankCtrl;                   //���ݿ������ƿ�


typedef struct{
	WORD	wMaxPn;		//���������֧�ֵĲ������
	WORD	wRealNum;	//ʵ��֧�ֵĲ�������
	bool	fGenerateMapWhenNoneExist;	//��û��ӳ����ʱ���Զ�����һһ��Ӧ��ӳ�����Ҫ��Ӧ�԰汾����

	//һ���ֶ��ϵ���ʼ��
	DWORD	dwFileSize;		//ӳ�䱣����ļ���С
	WORD	wAllocSize;		//�洢�ռ�����Ĵ�С						 
	WORD*	pwPnToMemMap;	//������ŵ��洢�ŵ�ӳ���(��Ҫ���浽�ļ�ϵͳ)
							//ǰ������WORD�������������Ϣ,���е�һ��WORD���Ѿ�ӳ��ĸ���,�ڶ�������
	BYTE*	pbAllocTab;		//�洢�ռ�����(�����浽�ļ�ϵͳ,��̬����)
}TPnMapCtrl;       	//�����㶯̬ӳ����ƽṹ

#define DI_ACESS_BUF		0	//���ջ�������д
#define DI_ACESS_INT32		1	//��������32λ��д
#define DI_ACESS_INT64		2	//��������64λ��д
#define DI_ACESS_QRY		3	//��ѯ�������Ƿ����
#define DI_ACESS_UPD		4	//����������״̬
#define DI_ACESS_GI			5	//GetItem
#define DI_ACESS_INFO		6	//ȡ��������Ϣ(���ȺͶ�)
#define DI_ACESS_RDUNMAP	7	//���շ�ӳ��ķ�ʽ��
#define DI_ACESS_WRUNMAP	8	//���շ�ӳ��ķ�ʽд

typedef struct{
	BYTE	bType;			//DI_ACESS_BUF,DI_ACESS_INT32,DI_ACESS_INT64
	WORD	wValNum;		//��ֵ��дʱ��ֵ�ĸ���
	WORD	wValidNum;		//������ʱ���صĺϷ���������ĸ���
	DWORD*	pdwTime;		//����ȡ�������ʱ��,ֻ����DI_ACESS_BUF/DI_ACESS_INT32/DI_ACESS_INT64ʱ����ʹ��
	union
	{
		BYTE*	pbBuf;			//���ջ�������д
		int*	piVal32;		//��������32λ��д
		int64*  piVal64;		//��������64λ��д	
		DWORD	dwVal;			//����״̬��
		TItemInfo* pItemInfo;   //���������Ϣ
	};
}TItemAcess;

typedef struct{
	BYTE*  pbAddr;
	WORD   wLen;        //�������
	BYTE*  pbModified;
	BYTE   bModifiedMask;
	TBankCtrl* pBankCtrl;
}TDataItem;

typedef struct{
	WORD wFrmBn;	//�ɰ汾BANK
	WORD wFrmId;	//�ɰ汾ID
	WORD wToBn;		//�°汾BANK
	WORD wToId;		//�°汾ID
	void (*pfnUpgFun)(WORD wFrmBn, WORD wFrmId, WORD wToBn, WORD wToId, int iPnNum);	//ת������,ΪNULLʱʹ��Ĭ�ϵ�ת������
}TIdUpgMap;	//ID�л�ӳ���ϵ


typedef struct{
	BYTE 		bSchVer;		//���������İ汾��ʶ
	TBankCtrl* 	pBankCtrl;		//�����õ�����ʱBANK���ƽṹ
	WORD 		wBankNum;		//֧�ֵ�BANK��Ŀ

	TIdUpgMap*	pIdUpgMap;		//ID�л�ӳ���
	DWORD 		dwIdNum;		//ID�л�ӳ������Ŀ��
								 
	bool		fDelFile;		//�������Ƿ�ɾ�����ļ�
	bool		fRstCPU;		//�������Ƿ�λ�ն�
}TDbUpgCtrl;	//���ݿ�汾��������

typedef struct{
	//BANK0�Ŀ����ֶ�
	WORD 		wSectNum;	//BANK0�е�SECT��Ŀ
	TBankCtrl* 	pBank0Ctrl;

	//BANK�����ֶ�
	WORD 		wBankNum;	//֧�ֵ�BANK��Ŀ
	TBankCtrl* 	pBankCtrl;

	int			iSectImg;		//485�������ݾ����,���û�������-1
	WORD		wImgNum;		//485�������ݾ������
	WORD		wSectPnData;	//����485����������,��Ҫ�в�����������֮��Ӧ,���򱾲������ó�0����
								 
	//�����㶯̬ӳ������ֶ�
	WORD 		wPnMapNum;	//֧�ֵ�ӳ�䷽����Ŀ,�������ݿⲻ֧�ֲ����㶯̬ӳ������Ϊ0
	TPnMapCtrl*	pPnMapCtrl;	//�������ݿⲻ֧�ֲ����㶯̬ӳ������ΪNULL

	//����һЩ��������
	WORD 		wPnMaskSize; //����������λ�Ĵ�С,�������������������λ�ռ�
	char* 		pszDbPath;	 //ϵͳ��һЩ�����ļ��Ĵ��Ŀ¼,һ������ΪUSER_PARA_PATH

	//���ݿ�汾��������upgrade
	TDbUpgCtrl* pDbUpgCtrl;		//���ݿ�汾��������,����ΪNULL��ʾû����������
	
	WORD		wSaveInterv;	//������,��λ����
}TDbCtrl;	//�������ݿ���в������õ����ݿ���ƽṹ

#endif //LIBDBSTRUCT_H