#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "GuiAPI.h"
#include "ComAPI.h"
#include "apptypedef.h"
#include "sysarch.h"
#include "sysapi.h"
#include "sysfs.h"
#include "GUI.H"
#include "LCD1.H"
#include "GuiBmp.h"
#include "Key.h"
#include "FaAPI.h"
#include "LibDbAPI.h"
#include "math.h"
#include "ProPara.h"

//#include "DbGbAPI.h"
#include "DbOIAPI.h"

//#include "LibCctAPI.h"

//#include "DbCctAPI.h"
//#include "CctHook.h"
#include "MeterAPI.h"
//#include "DbSgAPI.h"
//#include "CctAPI.h"
#include "MtrHook.h"

#include "DrvAPI.h"
#include "Key.h"

//#include "FrzTask.h"
#include "TaskManager.h"
#include "LoadCtrl.h"
#include "ParaMgr.h"
#include "AcConst.h"
#include "CctAPI.h"
#include "ProHook.h"
#include "MtrCtrl.h"

#define MAX_VALID_LINE_PER_SCREEN  8

#define EN_DRAW_TOPSPLITLINE
#define EN_DRAW_BOTTOMSPLITLINE
#define AcSampleMpNum 1

#ifndef BIG_LCD
	#define CHAR_NUM_LINE			20	//

#else

	#define  CHAR_NUM_LINE          16

#endif
//#define TERM_TYPE_CCT	1	//����

#define MENU_ONELEVEL_HAVE_NO	0xFF
#define MENU_TWOLEVEL_HAVE_NO   0xFF
#define MENU_THREELEVEL_HAVE_NO	0xFF

#define CCT_USB_PATH		"/mnt/usb/RtUpdate/"
//#define  USER_CFG_PATH     "/mnt/para/cfg"
#define STORE_FILE_LEN		64	
#define STORE_MAX_COUNT		32	

#ifndef SYS_WIN
#define ENABLE_PASSWORD_TIME	(15*60) //������Чʱ��15����
#else
#define ENABLE_PASSWORD_TIME     0
#endif


extern TSem g_SemGUI;
extern DWORD g_dwStateTaskClick;
extern bool g_fLcdRefresh;
CQueue g_MsgQueue;

//���߹���ͨѶ�ӿ����ò���

//extern BYTE g_bGprsCommCfg[26]; 
/*= {DT_STRUCT, 0x0C, 
								DT_ENUM, 		//����ģʽ
								DT_ENUM, 	//���߷�ʽ
								DT_ENUM, 	//���ӷ�ʽ
								DT_ENUM, 	//����Ӧ�÷�ʽ
								DT_ARRAY, 8, //֡���˿��б�
								DT_LONG_U,
								DT_VIS_STR, 32, RLV,  //APN,���32�ֽ�
								DT_VIS_STR, 32, RLV, //�û���,���32�ֽ�
								DT_VIS_STR, 32, RLV, //����,���32�ֽ�
								DT_OCT_STR, 4,	RLF,//�����������ַ
								DT_LONG_U, 		//����˿�
								DT_BIT_STR, 8, RLF,  //��ʱʱ�估�ط�����
								DT_LONG_U, 		//��������
};*/

//extern BYTE GetMeterTsa(WORD wMtrSn, BYTE *pbTsa);

//��������ȡ��Լ����
//extern BYTE GetMeterPro(WORD wMtrSn);

//�������жϱ�����Ƿ���Ч
//������&wMtrSn�����
//���أ���Ч����true����֮
//extern bool IsMtrSnValid(WORD wMtrSn);

//��������ȡ�����Ϣ
//������@wMtrSn ������
//		@tTMtrInfo ������õ�Ԫ��Ϣ
//extern bool GetMeterInfo(WORD wMtrSn, TOobMtrInfo *pTMtrInfo);

//���������õ����Ϣ
//������@wMtrSn ������
//		@tTMtrInfo ������õ�Ԫ��Ϣ
//extern bool SetMeterInfo(WORD wMtrSn, TOobMtrInfo tTMtrInfo);

//BYTE GetMeterTsa(WORD wMtrSn, BYTE *pbTsa, bool fRev=false);


#ifndef SYS_WIN
extern CKey  g_Key;	
#endif

#define DEBUG_DISP
#undef DEBUG_DISP
#define SECT_PULSE_PARA		SECT2
#ifdef EN_CTRL
	#define FK_TERM
#endif

static bool g_fStopState = false;
//static bool g_fPwdOk = false;
//����ID����ʱ��¼
//static BYTE g_bTmpLoopIdBuff[10] = {0,};
bool fNeedDisplay = true;
DWORD g_PastClick = 0;

static int g_iTermiType = 0;

int DrawStateTask();
int StatDir(char* dirList, int iBufSize);//�г�U�̸�Ŀ¼�µ�Ŀ¼
int UpdateList(const char* pcList, const int iCnt);
int SetMpPara(void *arg);
int SetTermiPwd(void *arg);
int SetMySysTime(void *arg);

int Dummy(void *var)
{
	return -1;
}

//����:ͨ������¼�ķ�ʽ��ȡ���ݿ����һ��ʵʱ����
//����: @wMtrSn �����
//		@dwOAD ����������
//		@pbBuf ���ص����ݻ�����
//		@wBufSize ��������С
//����:�����ȷ�򷵻����ݵĳ���,���򷵻ظ���
int ReadClass1Data(WORD wMtrSn, DWORD dwOAD, BYTE *pbBuf)
{
	int iStart = -1;
	int iTabIdx = 0;
	WORD wRetNum = 0;
	BYTE bRSD[30] = {0};                                      //��¼ѡ��������
	BYTE bRCSD[6] = {0x01, //01 ���� RCSD��SEQUENCE OF CSD����=1
		0x00, //OAD
		0x00, 0x00, 0x00, 0x00 //���� OAD��
	};  //��¼��ѡ��������                               
	BYTE *ptr = NULL;
	WORD wPn = 0;
	BYTE bTsa[17] = {0};
	BYTE bTsaLen = 0;
	BYTE bOAD[4] = {0x60, 0x12, 0x03, 0x00};  //�������ñ� ��¼��Ԫ
	int nRet;
	BYTE bBuf[512] = {0};
	WORD wBufSize = sizeof(bBuf);

	wPn = MtrSnToPn(wMtrSn);
	bTsaLen = GetMeterTsa(wPn, bTsa);
	OoDWordToOad(dwOAD, bRCSD+2);

	ptr = bRSD;
	*ptr++ = 10;       //���� RSD��ѡ�񷽷�10
	*ptr++ = 1;        //���� ��һ����¼
	*ptr++ = 3;        //���� ���ܱ���MS  һ���û���ַ [3]
	*ptr++ = 1;        //����ַ����=1
	*ptr++ = bTsaLen;
	memcpy(ptr, bTsa, bTsaLen);   //����ַ
	ptr += bTsaLen;

	nRet = ReadRecord(bOAD, bRSD, bRCSD, &iTabIdx, &iStart, bBuf, wBufSize, &wRetNum);

	if (nRet > 0)
		memcpy(pbBuf, bBuf, nRet);

	return nRet;
}

int ProtectState(void *arg)
{
	char menuitem[2][32];
	//BYTE pbBuf[64];
	BYTE bState[2] = {0};
	BYTE bField;
	WORD wFmtLen = NULL;
	BYTE *pbFmt = NULL;
	int i = 0;
	int iStart = -1;
	//char *ProState[] = {"����Ͷ��","������"};
	char *ProState[] = {"������","����Ͷ��","�Զ�����"};
	struct ListBoxExItem tmp[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	//ReadItemEx(BN0, PN0, 0X104F, pbBuf);//
	if (0 > OoProReadAttr(0x8001, 0x02, 0x00, bState, sizeof(bState), &iStart))
	{
		MessageBox("��ȡ���ݿ����!", KEY_ESC, 6000);
		return -1;
	}

	sprintf(menuitem[i],"����: ");

	if (bState[1] == 1)
	{
		strcat(menuitem[i],ProState[1]);
	}
	else if (bState[1] == 2)
	{
		strcat(menuitem[i],ProState[2]);
	}
	else
	{
		strcat(menuitem[i], ProState[0]);
	}
	
	CListBoxEx listbox;
	listbox.Show(0, "����״̬", tmp, KEY_ESC, 60000,false);
	if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
	{
		return -1;
	}
	return -1;
}

//��������ѡ��������ļ��躬��update�ļ��У����ļ�������clou.tgz
//������
//���أ�
void TermiUpdate2()
{
	char str[64];
	char command[64];
	char szList[STORE_MAX_COUNT][STORE_FILE_LEN];
	
//	char szupdateList[STORE_MAX_COUNT][STORE_FILE_LEN];
	int  iCnt = 0;

//	MessageBox("���U����,���Ժ�!", KEY_ESC,500);
/*
	if ( !IsExistUsb() )
	{
		MessageBox("����U��ʧ��,��������˳�!", KEY_ESC,2000);
		return ;
	}
*/
	memset(szList, 0, sizeof(szList));
	iCnt = StatDir((char *)&szList[0], STORE_MAX_COUNT);
	if (iCnt <= 0)
	{
		MessageBox("U���²�����Ŀ¼,��������˳�!", KEY_ESC, 2000);
		return ;
	}

	int iIndex = UpdateList((char *)szList, iCnt);
	if (iIndex < 0 || iIndex >= iCnt)
		return;

	MessageBox("������,��ȴ�!", KEY_ESC,500);
	MessageBox("�벻Ҫ�γ�U��!", KEY_ESC,500);

	sprintf(str, "/mnt/usb/%s", szList[iIndex]);
	//DTRACE(0, ("select file ---%s", szList[iIndex]));
	sprintf(command, "cp -rf %s/update /mnt/app", str);
	
	//DTRACE(0, ("copy command ---%s", command));
	//sprintf(command, "cp -rf %s/clou.tgz /mnt/app", str);
	//sprintf(command, "cp -f %s /mnt/app", str);
	system(command);

	system("chmod +x /mnt/app/update");
	
	//sprintf(command, "source /mnt/app/update %s", str);
	//system(command);
	
	DWORD dwClick = GetClick();
	while(GetClick()-dwClick < 5)
		MessageBox("�벻Ҫ�γ�U��!", KEY_ESC,100);
	system("umount /mnt/usb");

	//StatDir2((char *)&szupdateList[0], 1);
	//DTRACE(0, ("the file is %s", szupdateList[0]));
	strcpy(str, "/mnt/app/update/clou.tgz");
//	int f = open(str, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
	int f = open(str, O_RDWR , S_IREAD|S_IWRITE);
	if (f >= 0)
	{
		close(f);
		system("cp -rf /mnt/app/update/clou.tgz /mnt/app/");
		system("rm -Rf /mnt/app/update");
		MessageBox("�������!", KEY_ESC,1500);
	}
	else
	{
		MessageBox("����ʧ��!", KEY_ESC,1500);
	}
	dwClick = GetClick();
	SetInfo(INFO_HARDWARE_INIT);//��֤ͣ��ͳ�Ʋ���ͳ�ƣ������������ʱ��
	while(GetClick()-dwClick < 5)
		MessageBox("�ȴ�5���ն˸�λ!",KEY_ESC,100);

#ifdef SYS_LINUX
	system("/clou/ppp/script/ppp-off");
	Sleep(2000);
#endif
	ResetCPU();
}


//������u�̸�Ŀ¼����update�ļ��У����ļ�����clou.tgz
//������
//���أ�
void quickUpdate()
{
	int  iCnt = 0;
	char str[64];
	char command[64];
	char szList[STORE_MAX_COUNT][STORE_FILE_LEN];

	MessageBox("����������......", KEY_ESC,500);

	memset(command, 0, 64);
	sprintf(command, "cp -rf /mnt/usb/update /mnt/app ");
	system(command);

	system("chmod +x /mnt/app/update");

	DWORD dwClick = GetClick();
	while(GetClick()-dwClick < 5)
		MessageBox("�벻Ҫ�γ�U��!", KEY_ESC, 100);
	system("umount /mnt/usb");
	strcpy(str, "/mnt/app/update/clou.tgz");
	int f = open(str, O_RDWR, S_IREAD|S_IWRITE);//|O_BINARY
	if (f >= 0)
	{
		close(f);
		system("cp -rf /mnt/app/update/clou.tgz /mnt/app/");
		system("rm -Rf /mnt/app/update");
		MessageBox("�������!", KEY_ESC, 1500);
	}
	else
	{
		MessageBox("����ʧ��!", KEY_ESC, 1500);
	}
	dwClick = GetClick();
	SetInfo(INFO_HARDWARE_INIT);//��֤ͣ��ͳ�Ʋ���ͳ�ƣ������������ʱ��
	while(GetClick()-dwClick < 5)
		MessageBox("�ȴ�5���ն˸�λ!", KEY_ESC, 100);

#ifdef SYS_LINUX
	system("/clou/ppp/script/ppp-off");
	Sleep(2000);
#endif
	ResetCPU();

}

int CopyData()
{
	int  iCnt = 0, iRet = 0;
	char str[64];
	char command[64];
	char szList[STORE_MAX_COUNT][STORE_FILE_LEN];		
	char menuitem[6][32];
	char title[10] = "USB����";

	int i = 0;
	struct ListBoxExItem items[] = {
		{(char *)"��������", MENU_TWOLEVEL_HAVE_NO, Dummy,(void*)1},
		{(char *)"��������", MENU_TWOLEVEL_HAVE_NO, Dummy,(void*)2},
		{(char *)"��������", MENU_TWOLEVEL_HAVE_NO, Dummy,(void*)3},
		{(char *)"������־", MENU_TWOLEVEL_HAVE_NO, Dummy,(void*)4},
		{(char *)"����ȫ��", MENU_TWOLEVEL_HAVE_NO, Dummy,(void*)5},
		{(char *)"�˳�", MENU_TWOLEVEL_HAVE_NO, Dummy,(void*)6},
		{NULL, MENU_TWOLEVEL_HAVE_NO, Dummy, (void*)7},
	};

	CListBoxEx listbox;	
	listbox.Show(0, title, items, KEY_ESC | (KEY_RIGHT << 8) | (KEY_LEFT << 16) | (KEY_OK<<24), 60000);
	if(listbox.key == KEY_OK)
	{
		if(listbox.item == 0)
		{//�ն˳�������
			MessageBox("���ڿ�������", KEY_ESC, 500);
			memset(command, 0, 64);
			sprintf(command, "cp -rf %s /mnt/usb/", USER_PARA_PATH);
			system(command);
			Sleep(10*1000);
			system("umount /mnt/usb");//
			MessageBox("�������ݳɹ�", KEY_ESC, 1500);
		}
		else if(listbox.item == 1)
		{
			MessageBox("���ڿ�������", KEY_ESC, 500);
			memset(command, 0, 64);
			sprintf(command, "cp -rf %s /mnt/usb/", USER_DATA_PATH);
			system(command);
			Sleep(10*1000);
			system("umount /mnt/usb");//
			MessageBox("�������ݳɹ�", KEY_ESC, 1500);
		}
		else if(listbox.item == 2)
		{//��������
			MessageBox("���ڿ�������", KEY_ESC, 500);
			memset(command, 0, 64);
			sprintf(command, "cp -rf %s /mnt/usb/", USER_CFG_PATH);
			system(command);
			Sleep(10*1000);
			system("umount /mnt/usb");//
			MessageBox("�������ݳɹ�", KEY_ESC, 1500);	
		}
		else if(listbox.item == 3)
		{//������־
			MessageBox("���ڿ�����־", KEY_ESC, 500);
			memset(command, 0, 64);
			sprintf(command, "cp -rf %slog /mnt/usb/", USER_PATH);
			system(command);
			Sleep(10*1000);
			system("umount /mnt/usb");//
			MessageBox("�������ݳɹ�", KEY_ESC, 1500);			
		}
		else if(listbox.item == 4)
		{//����ȫ��
			MessageBox("���ڿ���ȫ��", KEY_ESC, 500);
			memset(command, 0, 64);
			sprintf(command, "cp -rf %s /mnt/usb/", USER_PATH);
			system(command);
			Sleep(10*1000);
			system("umount /mnt/usb");//
			MessageBox("�������ݳɹ�", KEY_ESC, 1500);			
		}
		else if(listbox.item == 5)
		{//�˳�

		}
		iRet = 0;
	}
	else
		iRet = -1;

	return iRet;
}

//add by qiaojun.chen
void TermiUpdate(BYTE);
bool InputPwd(void);
int UsbUpdate2(void *arg)
{
	//ClearScreenRec(0, 16, 160, 144);
	//	if(!InputPwd2())
	//	return -1;
	if(!InputPwd())
		return -1;
	char menuitem[6][32];
	char title[] = "USB���������ݿ���";

	int i = 0;
	struct ListBoxExItem items[] = {
		{menuitem[i++],MENU_TWOLEVEL_HAVE_NO,Dummy,(void*)1},
		{menuitem[i++],MENU_TWOLEVEL_HAVE_NO,Dummy,(void*)2},
		{menuitem[i++],MENU_TWOLEVEL_HAVE_NO,Dummy,(void*)3},
		{menuitem[i++],MENU_TWOLEVEL_HAVE_NO,Dummy,(void*)4},
		{menuitem[i++],MENU_TWOLEVEL_HAVE_NO,Dummy,(void*)5},
	};

	memset(menuitem,0,sizeof(menuitem));
	i = 0;
	sprintf(menuitem[i++],"%s","�ն˳�������");
	sprintf(menuitem[i++],"%s","�ն����ݿ���");
	//sprintf(menuitem[i++],"%s","��������");
	sprintf(menuitem[i++],"%s","�˳�");
	items[i].text = NULL;
	CListBoxEx listbox;

	while(1)
	{	
		listbox.Show(0, title ,items, KEY_ESC|(KEY_OK<<8), 60000);
		
		if(!IsInUsbProcess())
		{
			return 100;
		}

		if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			return 100;
		}
		if(listbox.key == KEY_OK)
		{
			if(listbox.item == 0)
			{//�ն˳�������
				TermiUpdate(1);

			}
			else if(listbox.item == 1)
			{//��������
				CopyData();//
			}
			/*
			else if(listbox.item == 2)
			{//��������
				quickUpdate();
			}
			*/
			else if(listbox.item == 2)
			{//�˳�
				system("umount /mnt/usb");
				return 100;
			}
		}
	}

	return -1;
}

int PowEnergCtrlState(void *arg)
{
	char menuitem[9][32];
	BYTE bBuf[75] = {0};
	int i = 0;
	int j = 0;
	char *CtrState[] = {"δͶ��", "Ͷ��"};
	char *cp[6] = {NULL};
	struct ListBoxExItem tmp[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 1 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	char cInput[20] = {0};
	int iGroupNo = 0;
	typedef struct {
		BYTE bStructType;
		BYTE bParserNum;
		BYTE bParser1Type;
		BYTE bOI[2];
		BYTE bParser2Type;
		BYTE bCtrlState;
	}TCtrlState;
	TCtrlState tBuyCtrlState, tTmpCtrlState, tMonthCtrlState, tPeriodCtrlState, tResetCtrlState, tShutOutCtrlStata; 
	BYTE bLen = sizeof(TCtrlState);
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	BYTE bOffSet = 0;
	int iRet[6] = {0};
	if(EditSpecBox(2,"�������ܼ���(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
	{
		sscanf(cInput,"%d",&iGroupNo);
		if(iGroupNo < 9 && iGroupNo > 0)
		{
			/*
			**��ȡ�����ص�ص�����3--����Ͷ��״̬
			*/
			bOffSet = 2 + (iGroupNo - 1) * bLen;
			iRet[0] = OoReadAttr(0x8103, 0x03, bBuf, &pbFmt, &wFmtLen); //ʱ�ι���	
			memcpy(&tPeriodCtrlState, bBuf+bOffSet, bLen);
			iRet[1] = OoReadAttr(0x8104, 0x03, bBuf, &pbFmt, &wFmtLen); //���ݿ�
			memcpy(&tResetCtrlState, bBuf+bOffSet, bLen);
			iRet[2] = OoReadAttr(0x8105, 0x03, bBuf, &pbFmt, &wFmtLen); //Ӫҵ��ͣ��
			memcpy(&tShutOutCtrlStata, bBuf+bOffSet, bLen);
			iRet[3] = OoReadAttr(0x8106, 0x03, bBuf, &pbFmt, &wFmtLen); //�¸���
			memcpy(&tTmpCtrlState, bBuf+bOffSet, bLen);
			iRet[4] = OoReadAttr(0x8107, 0x03, bBuf, &pbFmt, &wFmtLen); //�����
			memcpy(&tBuyCtrlState, bBuf+bOffSet, bLen);
			iRet[5] = OoReadAttr(0x8108, 0x03, bBuf, &pbFmt, &wFmtLen); //�µ��
			memcpy(&tMonthCtrlState, bBuf+bOffSet, bLen);

			for (j=0; j<6; j++)
			{
				if (iRet[j] < 0)
			{
					MessageBox("��ȡ���ݿ����!", KEY_ESC, 6000);
					return -1;
			}
			}

			cp[0] = CtrState[(tPeriodCtrlState.bCtrlState)&0x01]; //ʱ�ι���	
			cp[1] = CtrState[(tResetCtrlState.bCtrlState)&0x01];  //���ݿ�
			cp[2] = CtrState[(tShutOutCtrlStata.bCtrlState)&0x01];//Ӫҵ��ͣ��
			cp[3] = CtrState[(tTmpCtrlState.bCtrlState)&0x01];    //�¸���
			cp[4] = CtrState[(tBuyCtrlState.bCtrlState)&0x01];    //�����
			cp[5] = CtrState[(tMonthCtrlState.bCtrlState)&0x01];  //�µ��

			sprintf(menuitem[i++],"�ܼ���:%d",iGroupNo);
			sprintf(menuitem[i++],"ʱ�οأ�%s",cp[0]);
			sprintf(menuitem[i++],"���ݿأ�%s",cp[1]);
			sprintf(menuitem[i++],"��ͣ�أ�%s",cp[2]);
			sprintf(menuitem[i++],"�¸��أ�%s",cp[3]);
			sprintf(menuitem[i++],"����أ�%s",cp[4]);
			sprintf(menuitem[i++],"�µ�أ�%s",cp[5]);
			tmp[i].text = NULL;

			CListBoxEx listbox;
			listbox.Show(0, "�ܼ������״̬", tmp, KEY_ESC, 60000,false);
			if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			{
				return -1;
			}
		}
		else
		{
			MessageBox("�����ܼ������",KEY_ESC,6000);
		}
	}

	return -1;
}

//����״̬
int  CtrlState(void *arg)
{
	struct ListBoxExItem tmp[] = { 
		{ "����״̬", MENU_TWOLEVEL_HAVE_NO, ProtectState, (void *) 1 }, //
		{ "���ؼ����״̬", MENU_TWOLEVEL_HAVE_NO, PowEnergCtrlState, (void *) 2 }, //

		{ NULL, 0xFF, NULL, NULL }, //
	};


#if 0
	i = 0;
	memset(menuitem,0,sizeof(menuitem));

	if (g_LoadCtrl.IsBuyCtrlOn())
		fBuyCtrl = true;
	else
		fBuyCtrl = false;

	if (g_LoadCtrl.IsMonthCtrlOn())
		fMonthCtrl = true;
	else
		fMonthCtrl = false;

	if (g_LoadCtrl.IsPowerCtrlOn())
		fTimeCtrl = true;
	else
		fTimeCtrl = false;

	if (g_LoadCtrl.IsTmpCtrlOn())
		fTempCtrl = true;
	else
		fTempCtrl = false;

	if (g_LoadCtrl.IsYkCtrlOn())   //ң��
		fYkCtrl = true;
	else
		fYkCtrl = false;

		
	sprintf(menuitem[i++], "%s", "��ǰ���Ʒ���:");

	if (fTimeCtrl || fBuyCtrl || fMonthCtrl || fTempCtrl || fYkCtrl)
	{
		if (fYkCtrl)
			strcat(szStr, "/ң��\n\n");
		if (fTimeCtrl)
			strcat(szStr, "/ʱ�ο�\n\n");
		if (fBuyCtrl)
			strcat(szStr, "/�����\n\n");
		if (fMonthCtrl)
			strcat(szStr, "/�µ��\n\n");
		if (fTempCtrl)
			strcat(szStr, "/��ʱ�޵��\n\n");

		sprintf(menuitem[i++], "%s", szStr);

	}
	else
	{
		sprintf(menuitem[i++], "%s", "��");
	}

	if (g_LoadCtrl.IsGuarantee())
		sprintf(menuitem[i++], "%s", "����:Ͷ��");
	else
		sprintf(menuitem[i++], "%s", "����:�˳�");


	sprintf(menuitem[i++], "%s", "����ң��״̬:");

	iLen = ReadItem(POINT0, 0x8860, bBuf);
	
	sprintf(menuitem[i++], "%s", "�ִ�: 1 2 3 4");

	strcpy(szStr, "״̬: ");
	if ((bBuf[1]&0x08) == 0x08)
		strcat(szStr, "��");
	else
		strcat(szStr, "��");

	if ((bBuf[1]&0x10) == 0x10)
		strcat(szStr, "��");
	else
		strcat(szStr, "��");

	
	if ((bBuf[1]&0x20) == 0x20)
		strcat(szStr, "��");
	else
		strcat(szStr, "��");

	if ((bBuf[1]&0x40) == 0x40)
		strcat(szStr, "��");
	else
		strcat(szStr, "��");	
	

	sprintf(menuitem[i++], "%s", szStr);
#endif


	CListBoxEx listbox;
	char menuitem[8][32];

	while(1)
	{
		listbox.Show(0, "����״̬", tmp, KEY_ESC, 6000);

		if( listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			break;
	}
	return -1;
}


//#ifdef DISPLAY
//Ƶ��
bool Fmt1ToStr(BYTE* pbBuf, char* str)//20140306-1
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 2))
		strcpy(str, "xx.xx");
	else
	{
		double dEng = (double)BcdToDDWORD(pbBuf, 2);
		sprintf(str, "%.2f", dEng/100.0);
	}
	return true;  
}

//��������
bool Fmt5ToStr(BYTE* pbBuf, char* str)
{
	BYTE bTmpBuf[5];
	memcpy(bTmpBuf, pbBuf, 2);
	if (IsAllAByte(bTmpBuf, INVALID_DATA, 2))
		strcpy(str, "xxx.x");
	else
	{
		if ((bTmpBuf[1]&0x80) != 0)
		{
			bTmpBuf[1] &= 0X7f;
			double dEng = (double)BcdToDDWORD(bTmpBuf, 2);
			sprintf(str, "-%.1f", dEng/10.0);
		}
		else
		{
			double dEng = (double)BcdToDDWORD(bTmpBuf, 2);
			sprintf(str, "%.1f", dEng/10.0);
		}
	}
	return true;  
}

//��ѹ/���
bool Fmt7ToStr(BYTE* pbBuf, char* str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 2))
		strcpy(str, "xxx.x");
	else
	{
		DWORD dEng = BcdToDDWORD(pbBuf, 2);
		sprintf(str, "%.1f", dEng/10.0);
	}
	return true;  
}


bool Fmt6ToStr(BYTE *pbBuf, char*str)
{
	BYTE TmpBuf[2];
	memcpy(TmpBuf,pbBuf,2);
	if (IsAllAByte(TmpBuf, INVALID_DATA, 2))
	{
		strcpy(str, "xx.xx");
	}
	else
	{
		if (TmpBuf[1]>>7)
		{
			TmpBuf[1] &= 0x7F;
			DWORD Volt = BcdToDDWORD(TmpBuf,2);
			sprintf(str,"-%.2f",Volt/100.0);
		}
		else
		{
			TmpBuf[1] &= 0x7F;
			DWORD Volt = BcdToDDWORD(TmpBuf,2);
			sprintf(str,"%.2f",Volt/100.0);
		}
	}
	return true;
}

bool Fmt18ToStr(BYTE *pbBuf,char *str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 3))
		strcpy(str, "xx-xx:xx");
	else
	{
		sprintf(str,"%02d-%d:%d",BcdToByte(*(pbBuf+2)),BcdToByte(*(pbBuf+1)),BcdToByte(*(pbBuf)));

	}
	return true;  
}

bool Fmt22ToStr(BYTE* pbBuf, char* str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 1))
		strcpy(str, "x.x");
	else
	{
		WORD dEng = (WORD)BcdToByte(*pbBuf);
		sprintf(str, "%.1f", dEng/10.0);
	}
	return true;  
}

//����
bool Fmt25ToStr(BYTE* pbBuf, char* str)
{
	BYTE bTmpBuf[5];
	memcpy(bTmpBuf, pbBuf, 3);
	if (IsAllAByte(bTmpBuf, INVALID_DATA, 3))
		strcpy(str, "xxx.xxx");
	else
	{
		if ((bTmpBuf[2]&0x80) != 0)
		{
			bTmpBuf[2] &= 0x7f;
			double dEng = (double)BcdToDDWORD(bTmpBuf, 3);
			sprintf(str, "-%.3f", dEng/1000.0);
		}
		else
		{
			double dEng = (double)BcdToDDWORD(bTmpBuf, 3);
			sprintf(str, "%.3f", dEng/1000.0);
		}
	}
	return true;  
}
//#endif
//����
bool Fmt9ToStr(BYTE* pbBuf, char* str)
{
	BYTE bTmpBuf[5];
	memcpy(bTmpBuf, pbBuf, 3);
	if (IsAllAByte(bTmpBuf, INVALID_DATA, 3))
		strcpy(str, "xx.xxxx");
	else
	{
		if ((bTmpBuf[2]&0x80) != 0)
		{
			bTmpBuf[2] &= 0X7f;
			double dEng = (double)BcdToDDWORD(bTmpBuf, 3);
			sprintf(str, "-%.4f", dEng/10000.0);
		}
		else
		{
			double dEng = (double)BcdToDDWORD(bTmpBuf, 3);
			sprintf(str, "%.4f", dEng/10000.0);
		}
	}
	return true;  
}

//������ʾ��ֵʱ����������ʾΪ0����֮��Ȼ
bool Fmt9ExtToStr(BYTE* pbBuf, char* str, bool fNeg)
{
	BYTE bTmpBuf[5];
	memcpy(bTmpBuf, pbBuf, 3);
	if (IsAllAByte(bTmpBuf, INVALID_DATA, 3))
		strcpy(str, "xx.xxxx");
	else
	{
		if (fNeg)
		{
			if ((bTmpBuf[2]&0x80) != 0)
			{
				bTmpBuf[2] &= 0X7f;
				double dEng = (double)BcdToDDWORD(bTmpBuf, 3);
				sprintf(str, "%.4f", dEng/10000.0);
			}
			else
			{
				double dEng = 0;
				sprintf(str, "%.4f", dEng/10000.0);
			}
		}
		else
		{
			if ((bTmpBuf[2]&0x80) != 0)
			{
				double dEng = 0;
				sprintf(str, "%.4f", dEng/10000.0);
			}
			else
			{
				double dEng = (double)BcdToDDWORD(bTmpBuf, 3);
				sprintf(str, "%.4f", dEng/10000.0);
			}
		}
	}
	return true;  
}

bool Fmt23ToStr(BYTE* pbBuf, char* str)
{
	BYTE bTmpBuf[5];
	memcpy(bTmpBuf, pbBuf, 3);
	if (IsAllAByte(bTmpBuf, INVALID_DATA, 3))
		strcpy(str, "xx.xxxx");
	else
	{
		double dEng = (double)BcdToDDWORD(bTmpBuf, 3);
		sprintf(str, "%.4f", dEng/10000.0);
	}
	return true; 
}

bool Fmt4ToStr(BYTE *pbBuf, char* str)
{
	BYTE bTmpBuf[5];
	memcpy(bTmpBuf, pbBuf, 3);
	if (IsAllAByte(bTmpBuf, INVALID_DATA, 1))
		strcpy(str, "xx%");
	else
	{
		BYTE Factor = 0;
		if (bTmpBuf[0] & 0x80)
		{
			bTmpBuf[0] &= 0x7F;
			Factor= BcdToByte(bTmpBuf[0]);
			sprintf(str, "�¸� %d", Factor);
		}
		else
		{
			Factor= BcdToByte(bTmpBuf[0]);
			sprintf(str, "�ϸ� %d", Factor);
		}
	}
	return true;
}

double Fmt02ToDouble(BYTE* pbBuf)
{
	double power;
	BYTE   bFat;
	power = BcdToByte(pbBuf[0]);
	power += BcdToByte(pbBuf[1]&0x0f) * 100;
	power *= 10000;

	bFat = pbBuf[1]>>5;			//G3G2G1

	for (BYTE i=0; i<bFat; i++)
		power /= 10.0;

	if((pbBuf[1]>>4) & 0x01)	//S
		power = -power;
	return power;
}	

long Fmt09ToLong(BYTE* pbBuf)
{
	BYTE b = pbBuf[2];
	pbBuf[2] &= 0x7f;
	long lVal = BcdToDWORD(pbBuf, 3);
	if (b & 0x80)
		lVal = -lVal;
	return lVal;
}

//������ �ø�ʽת�������ṩ���ܼ��鹦����ʾʹ��
bool Fmt2ToStr(BYTE *pbBuf, char* str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 2))
	{
		strcpy(str, "xxx.xxx");
	}
	else
	{
		double fFact = (double)((double)Fmt2ToVal64(pbBuf, 2) / 1000.0);
		sprintf(str, "%.3f", fFact);
	}
	return true;
}

bool Fmt19ToStr(BYTE* pbBuf, char* str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 2))
		strcpy(str, "xx:xx");
	else
	{
		BYTE hour = BcdToByte(pbBuf[1]);
		BYTE min = BcdToByte(pbBuf[0]);
		sprintf(str, "%02dʱ%02d��", hour,min);
	}
	return true;
}

bool Fmt20ToStr(BYTE* pbBuf, char* str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 3))
		strcpy(str, "xx-xx:xx");
	else
	{
		WORD year = 2000 + BcdToByte(pbBuf[2]);
		BYTE month = BcdToByte(pbBuf[1]);
		BYTE day = BcdToByte(pbBuf[0]);
		
		sprintf(str, "%02d��%02d��%02d��", year,month,day);
	}
	return true;
}

//�޹�����ʾֵ
bool Fmt11ToStr(BYTE* pbBuf, char* str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 4))
		strcpy(str, "xxxxxx.xx");
	else
	{
		double dEng = (double)BcdToDDWORD(pbBuf, 4);
		sprintf(str, "%.2f", dEng/100.0);
	}
	return true;  
}

//�й�
bool Fmt14ToStr(BYTE* pbBuf, char* str) //
{
	if (IsAllAByte(pbBuf, INVALID_DATA,5))
	{
		strcpy(str, "xxxx.xxxx");
	}
	else
	{
		double dEng = (double)BcdToDDWORD(pbBuf, 5);
		sprintf(str, "%.4f", dEng / 10000.0);
	}
	return true;
}


void Fmt03ToStr(BYTE* pbBuf, char* str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 4))
		strcpy(str, "xxxxxxxxkWh");
	else
	{
		BYTE bUnit;
		char *szUnit[2] = {"kWh","MWh"};

		BYTE b = pbBuf[3];
		if ((b & 0x40))
			bUnit = 1;	//
		else
			bUnit = 0;	//

		pbBuf[3] &= 0x0f;
		DWORD lEnergy = BcdToDWORD(pbBuf, 4);
		if (b & 0x10)	//
			lEnergy = -lEnergy;

		sprintf(str, "%d", lEnergy);
		strcat(str, szUnit[bUnit]);
	}
}

void Fmt03ToVarStr(BYTE* pbBuf, char* str)
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 4))
		strcpy(str, "xxxxxxxxkvarh");
	else
	{
		BYTE bUnit;
		char *szUnit[2] = {"kvarh","Mvarh"};

		BYTE b = pbBuf[3];
		if ((b & 0x40))
			bUnit = 1;	//
		else
			bUnit = 0;	//

		pbBuf[3] &= 0x0f;
		DWORD lEnergy = BcdToDWORD(pbBuf, 4);
		if (b & 0x10)	//
			lEnergy = -lEnergy;
			
		sprintf(str, "%d", lEnergy);
		strcat(str, szUnit[bUnit]);
	}
}


int Fmt17ToStr(BYTE* pbBuf,char* str)	//��ʱ����
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 4))
		strcpy(str, "xx-xx xx:xx");
	else
		sprintf(str, "%02d-%02d %02d:%02d", BcdToByte(pbBuf[3]), BcdToByte(pbBuf[2]), BcdToByte(pbBuf[1]), BcdToByte(pbBuf[0]));
	return true;  
}

bool Fmt13ToStr(BYTE* pbBuf, char* str) //�й�����ʹ��
{
	if (IsAllAByte(pbBuf, INVALID_DATA,4))
	{
		strcpy(str, "xxxx.xxxx");
	}
	else
	{
		double dEng = (double)BcdToDDWORD(pbBuf, 4);
		sprintf(str, "%.4f", dEng / 10000.0);
	}
	return true;
}

bool Fmt15ToStr(BYTE* pbBuf,char* str)	//��ʱ������
{
	if (IsAllAByte(pbBuf, INVALID_DATA, 5))
		strcpy(str, "xx-xx-xx xx:xx");
	else
		sprintf(str, "%02d-%02d-%02d %02d:%02d", BcdToByte(pbBuf[4]), BcdToByte(pbBuf[3]), BcdToByte(pbBuf[2]), BcdToByte(pbBuf[1]), BcdToByte(pbBuf[0]));
	return true;  
}


bool InputPwd()
{
	//static DWORD dwPwdClick = 0;
	//�Ѿ����������,������Чʱ��1Сʱ
	//if(g_fPwdOk && (dwPwdClick == 0 || (GetClick()-dwPwdClick)<ENABLE_PASSWORD_TIME))
	//{
	//	return g_fPwdOk;
	//}
	//else
	//{
	//	g_fPwdOk = false;

		char szPwd[16]={0};
		char szOldPwd[16]={0};
		BYTE bBuf[16]={0};
	//	ReadItemEx(BN1,PN0,0x2110,bBuf);
		ReadItemEx(BN10, PN0, 0xa045, bBuf);
		memcpy(szOldPwd,bBuf,6);
		sprintf(szPwd,"%s","000000");
	//	memcpy(szPwd, szOldPwd, 6);
// 		memset(szPwd, 0x00, sizeof(szPwd));
		int iCnt = 3;
		while(iCnt>0)
		{
			ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
			//if(EditTextBox(2,"����������:", szPwd, 30000, 6, DATA_ASC) >= 0)
			if(getSoftKey("����������:",szPwd,60000,6,DATA_ASC)>= 0)
			{
				memset(bBuf,0,sizeof(bBuf));
				int i = 0;
				while(szPwd[i]!=' ' && i < 6)
				{
					bBuf[i] = szPwd[i];
					i++;
				}
				memcpy(szPwd,bBuf,6);

				if(!strcmp(szPwd,szOldPwd))
				{
					//g_fPwdOk = true;
					//dwPwdClick = GetClick();
					return true;
				}
				else
				{
					MessageBox("�������!",KEY_ESC,2000);
				}

				iCnt--;
				ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
			}
			else
			{
				return false;
			}
		}
		
	//}
	return false;
}


int CurrntPower(void *arg)
{
		BYTE i = 0;
		struct ListBoxExItem tmpM[] = {
#ifdef FK_TERM
			{ "�ܼ��鹦��", 0xFF, Dummy, (void *) 1 }, //
#endif
			{ "�����㹦�ʼ���������", 0xFF, Dummy, (void *) 2 }, //
			{  NULL, 0xFF, Dummy, (void *) 3 }, //
		};
		CListBoxEx listbox;

flag:	while(1)
		{
			listbox.Show(0,"��ǰ����",tmpM, KEY_ESC | KEY_OK<<8, 60000);
			if (listbox.key == KEY_OK)
			{
				break;
			}
			else if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			{
				return -1;
			}
		}
		char menuitem[8][32];
#ifdef FK_TERM
		char *title[] = {"�ܼ��鹦��","�����㹦�ʼ���������"};
#else
		char *title[] = {"�����㹦�ʼ���������"};
#endif
		DWORD dwPowerID[] = {0x20040200, 0x20050200, 0x20060200, 0x200A0200};  //�й�����,�޹�����,���ڹ���,��������
		memset(menuitem, 0, sizeof(menuitem));

		struct ListBoxExItem tmpS[] = { 
			{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
			{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
			{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
			{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
			{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
			{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
			{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
			{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
			{ NULL, 0xFF, NULL, NULL }, //
		};

		BYTE bTmpBuff[110] = {0};

		BYTE *cp = NULL;
		int MenuPage = 0;
		int iGroupNo = 0;
		BYTE *pbFmt = NULL;
		WORD wFmtLen = 0;
		//BYTE bTsa[17] = {0};
		CListBoxEx listboxSub;
#ifdef FK_TERM		
		if (listbox.item == 0)
		{
			char cInput[5] = {0};
			if(EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
			{
				memset(bTmpBuff, 0, sizeof(bTmpBuff));
				sscanf(cInput,"%d",&iGroupNo);

				if(iGroupNo < 9 && iGroupNo > 0/* && IsGrpValid(iGroupNo)*/)
				{

					while(1)
					{
						i = 0;
						
						OoReadAttr(0x2300+iGroupNo, 0x03, bTmpBuff, &pbFmt, &wFmtLen);
						cp = bTmpBuff + 1;

						sprintf(menuitem[i++], "�ܼ���%d�й�����:",iGroupNo);
						sprintf(menuitem[i++], "  %.1fW", (float)OoLong64ToInt64(cp)*1e-1);

						OoReadAttr(0x2300+iGroupNo, 0x04, bTmpBuff+10, &pbFmt, &wFmtLen);

						sprintf(menuitem[i++], "�ܼ���%d�޹�����:",iGroupNo);
						cp += 10;
						sprintf(menuitem[i++],"  %.1fvar", (float)OoLong64ToInt64(cp)*1e-1);
						tmpS[i+2].text = NULL;

						listboxSub.Show(0, title[listbox.item], tmpS, KEY_ESC, 60000, false );

						if (KEY_ESC == listboxSub.key || KEY_NULL == listboxSub.key)
						{
							goto flag;
						}
					}	
				}
				else
				{
					MessageBox("�����ܼ������",KEY_ESC,10000);
					goto flag;
				}
			}
			else
			{
				goto flag;
			}
			
		}
		else if (listbox.item == 1)
#else
		if (listbox.item == 0)
#endif
		{
			char cInput[5] = {0};
			if(EditSpecBox(2,"������������:",cInput,60000,2,DATA_DEC)>= 0)
			{	
				/*WORD wPn = 0;
				TMtrRdCtrl *ptMtrRdCtrl = NULL;*/
				sscanf(cInput,"%d",&iGroupNo);

				if(iGroupNo < POINT_NUM && iGroupNo >= 1 /*&& IsPnValid(iGroupNo)*/)
				{
					TOobMtrInfo tTMtrInfo;
					memset(&tTMtrInfo, 0, sizeof(TOobMtrInfo));
					WORD wPn = MtrSnToPn(iGroupNo);
					if (!GetMeterInfo(wPn, &tTMtrInfo))
					{
						MessageBox("�õ��δ����!", KEY_ESC, 3000);
						goto flag;
					}
				}
				else
				{
					MessageBox("�������Ŵ���",KEY_ESC,10000);
					goto flag;
				}

				while(1)
				{
					i = 0;
					memset(menuitem, 0, sizeof(menuitem));
					memset(bTmpBuff, 0, sizeof(bTmpBuff));

					/*if (ptMtrRdCtrl != NULL)
					{
						GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x20040200, bTmpBuff);//�й�����
						GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x20050200, bTmpBuff+30);//�޹�����
						GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x20060200, bTmpBuff+60);//���ڹ���
						GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x200A0200, bTmpBuff+90);//��������
					}*/

					ReadClass1Data(iGroupNo, dwPowerID[MenuPage], bTmpBuff);

					if (MenuPage == 0)
					{
						cp = bTmpBuff+2+1;
						sprintf(menuitem[i++], "������%d:",iGroupNo);
						sprintf(menuitem[i++], "���й����ʣ�%.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

						cp += 5;
						sprintf(menuitem[i++], "A��: %.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

						if (bTmpBuff[1] != 2) //�����
						{
							cp += 5;
							sprintf(menuitem[i++], "B��: %.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

							cp += 5;
							sprintf(menuitem[i++], "C��: %.1fW", (float)OoDoubleLongToInt(cp)*1e-1);
						}
						
						//tmpS[i].text = NULL;
					}
					else if (MenuPage == 1)
					{
						//cp = bTmpBuff+30;
						cp = bTmpBuff+2+1;
						sprintf(menuitem[i++], "������%d:", iGroupNo);

						//cp += 3;
						sprintf(menuitem[i++], "���޹����ʣ�%.1fvar", (float)OoDoubleLongToInt(cp)*1e-1);

						cp += 5;
						sprintf(menuitem[i++], "A��: %.1fvar", (float)OoDoubleLongToInt(cp)*1e-1);

						if (bTmpBuff[1] != 2) //�����
						{
							cp += 5;
							sprintf(menuitem[i++], "B��: %.1fvar", (float)OoDoubleLongToInt(cp)*1e-1);

							cp += 5;
							sprintf(menuitem[i++], "C��: %.1fvar", (float)OoDoubleLongToInt(cp)*1e-1);
						}
						
						//tmpS[i].text = NULL;
					}
					else if (MenuPage == 2)
					{
						//cp = bTmpBuff+60;
						cp = bTmpBuff+2+1;
						sprintf(menuitem[i++], "������%d:", iGroupNo);

						//cp += 5;
						sprintf(menuitem[i++], "���ڹ���: %.1fVA", (float)OoDoubleLongToInt(cp)*1e-1);

						cp += 5;
						sprintf(menuitem[i++], "A��: %.1fVA", (float)OoDoubleLongToInt(cp)*1e-1);

						if (bTmpBuff[1] != 2)
						{
							cp += 5;
							sprintf(menuitem[i++], "B��: %.1fVA", (float)OoDoubleLongToInt(cp)*1e-1);

							cp += 5;
							sprintf(menuitem[i++], "C��: %.1fVA", (float)OoDoubleLongToInt(cp)*1e-1);
						}
					}
					else if (MenuPage == 3)
					{
						//cp = bTmpBuff+90;
						cp = bTmpBuff+2+1;
						sprintf(menuitem[i++], "������%d:", iGroupNo);

						//cp += 3;
						sprintf(menuitem[i++], "�������أ�%.3f", (float)OoLongToInt16(cp)*1e-3);

						cp += 3;
						sprintf(menuitem[i++], "A�ࣺ%.3f", (float)OoLongToInt16(cp)*1e-3);

						if (bTmpBuff[1] != 2)
						{
							cp += 3;
							sprintf(menuitem[i++], "B�ࣺ%.3f", (float)OoLongToInt16(cp)*1e-3);

							cp += 3;
							sprintf(menuitem[i++], "C�ࣺ%.3f", (float)OoLongToInt16(cp)*1e-3);
						}
						//tmpS[i].text = NULL;
					}
					listboxSub.Show(0, title[listbox.item], tmpS, KEY_ESC | KEY_UP<< 8 | KEY_DOWN<<16, 60000, false );

					if (KEY_ESC == listboxSub.key || KEY_NULL == listboxSub.key)
					{
						goto flag;
					}
					if (KEY_UP == listboxSub.key )
					{
						MenuPage--;
						if (MenuPage < 0)
						{
							MenuPage = 3;
						}
					}
					else if (KEY_DOWN == listboxSub.key)
					{
						MenuPage++;
						if (MenuPage > 3)
						{
							MenuPage = 0;
						}
					}
				}	
			}
			else
			{
				goto flag;
			}
		}									

		return -1;
}

int CurrntEnage(void *arg)
{
	BYTE i = 0;
	struct ListBoxExItem tmpM[] = {
#ifdef FK_TERM
		{ "�ܼ������", 0xFF, Dummy, (void *) 1 }, //
#endif
		{ "������", 0xFF, Dummy, (void *) 2 }, //
		{  NULL, 0xFF, Dummy, (void *) 3 }, //
	};
	CListBoxEx listbox;

flag:	while(1)
		{
			listbox.Show(0,"��ǰ������",tmpM, KEY_ESC | KEY_OK<<8, 60000);
			if (listbox.key == KEY_OK)
			{
				break;
			}
			else if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			{
				return -1;
			}
		}
		char menuitem[9][32];
		/*char *title[14] = {"�ܼ��鹦��","�����㹦��","�����ܼ����й�������","�����ܼ����޹�������","�����ܼ����й�������","�����ܼ����޹�������",
		"���������й�������","���շ����й�������","���������޹�������", "���շ����޹�������", "���������й�������","���·����й�������","���������޹�������","���·����޹�������"};
*/
		char *title[] = {"�ܼ����й�������","�ܼ����޹�������","�ܼ����й�������","�ܼ����޹�������",
			"�����й�����ʾֵ","�����й�����ʾֵ","�����޹�����ʾֵ", "�����޹�����ʾֵ", "һ�����޹�����ʾֵ","�������޹�����ʾֵ","�������޹�����ʾֵ","�������޹�����ʾֵ"};
		const DWORD bDeltaAddGrpIDAttr[] = {0x07, 0x08, 0x09, 0x0a};//�ܼ���ӿ�������7~10
		const DWORD dwGrpOI[] = {0x2301, 0x2302, 0x2303, 0x2304, 0x2305, 0x2306, 0x2307, 0x2308};//�ܼ���1~8
		const DWORD dwDeltaPnID[] = { 0x00100200, 0x00200200, 0x00300200, 0x00400200, 0x00500200, 0x00600200, 0x00700200, 0x00800200};//�����й�����ʾֵ~�������޹�����ʾֵ

		memset(menuitem, 0, sizeof(menuitem));

		struct ListBoxExItem tmpS[] = { 
			{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
			{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
			{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
			{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
			{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
			{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
			{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
			{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
			{ menuitem[8], 0xFE, Dummy, (void *) 8 }, //
			{ NULL, 0xFF, NULL, NULL }, //
		};
#ifdef DEBUG_DISP
		BYTE bTestBuf[80] = {DT_ARRAY, 5, 
										DT_LONG64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
										DT_LONG64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
										DT_LONG64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
										DT_LONG64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
										DT_LONG64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};
		BYTE bTmpBuff[80] = {DT_ARRAY, 5, 
										DT_DB_LONG_U, 0x00, 0x00, 0xFF, 0xFF,
										DT_DB_LONG_U, 0x00, 0x00, 0xFF, 0xFF,
										DT_DB_LONG_U, 0x00, 0x00, 0xFF, 0xFF,
										DT_DB_LONG_U, 0x00, 0x00, 0xFF, 0xFF,
										DT_DB_LONG_U, 0x00, 0x00, 0xFF, 0xFF};
#else
		BYTE bTmpBuff[80] = {0};
#endif
		BYTE *cp = NULL;
		char str[30] = {0};
		int MenuPage = 0;
		int iGroupNo = 0;
		BYTE bFmt[5] = {0};
		BYTE *pbFmt = bFmt;
		WORD wFmtLen = 0;

		CListBoxEx listboxSub;
		while(1)
		{
#ifdef FK_TERM
			if (listbox.item == 0)
			{
				char cInput[5] = {0};
				MenuPage = 0;
				if(EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
				{
					memset(str, 0, sizeof(str));
					sscanf(cInput,"%d",&iGroupNo);
					
					while(1)
					{
						i = 0;
						if(iGroupNo<9 && iGroupNo>0 /*&& IsGrpValid(iGroupNo)*/)
						{
#ifdef DEBUG_DISP
							cp = bTestBuf + 2 + 1;
#else
							memset(bTmpBuff, 0, sizeof(bTmpBuff));
							OoReadAttr(dwGrpOI[iGroupNo-1], bDeltaAddGrpIDAttr[MenuPage], bTmpBuff, &pbFmt, &wFmtLen);
							cp = bTmpBuff+2+1;
#endif	
							//memcpy(str, cp, 8);
	
							sprintf(menuitem[i++], "�ܼ���%d:",iGroupNo);
							switch(MenuPage)
							{
							case 0:
							case 2:
								sprintf(menuitem[i++],"�ܵ���: %.4fkwh", (float)OoLong64ToInt64(cp)*1.0e-4);
								cp += 9;
								//memcpy(str, cp, 8);
								sprintf(menuitem[i++], "��: %.4fkwh", (float)OoLong64ToInt64(cp)*1.0e-4);
								cp += 9;
								//memcpy(str, cp, 8);
								sprintf(menuitem[i++], "��: %.4fkwh", (float)OoLong64ToInt64(cp)*1.0e-4);
								cp += 9;
								//memcpy(str, cp, 8);
								sprintf(menuitem[i++], "ƽ: %.4fkwh", (float)OoLong64ToInt64(cp)*1.0e-4);
								cp += 9;
								//memcpy(str, cp, 8);
								sprintf(menuitem[i++], "��: %.4fkwh", (float)OoLong64ToInt64(cp)*1.0e-4);
								break;
							case 1:
							case 3:
								sprintf(menuitem[i++], "�ܵ���: %.4fkvarh", (float)OoLong64ToInt64(cp)*1.0e-4);
								cp += 9;
								//memcpy(str, cp, 8);
								sprintf(menuitem[i++], "��: %.4fkvarh", (float)OoLong64ToInt64(cp)*1.0e-4);
								cp += 9;
								//memcpy(str, cp, 8);
								sprintf(menuitem[i++], "��: %.4fkvarh", (float)OoLong64ToInt64(cp)*1.0e-4);
								cp += 9;
								//memcpy(str, cp, 8);
								sprintf(menuitem[i++], "ƽ: %.4fkvarh", (float)OoLong64ToInt64(cp)*1.0e-4);
								cp += 9;
								//memcpy(str, cp, 8);
								sprintf(menuitem[i++], "��: %.4fkvarh", (float)OoLong64ToInt64(cp)*1.0e-4);
								break;
							}
							
							tmpS[i].text = NULL;
						}
						else
						{
							MessageBox("�����ܼ������",KEY_ESC,10000);
							goto flag;
						}
						listboxSub.Show(0, title[MenuPage], tmpS, KEY_ESC | KEY_UP<<8 | KEY_DOWN<<16, 60000, false );

						if (KEY_ESC == listboxSub.key || KEY_NULL == listboxSub.key)
						{
							goto flag;
						}
						if (KEY_UP == listboxSub.key )
						{
							MenuPage--;
							if (MenuPage < 0)
							{
								MenuPage = 3;
							}
						}
						else if (KEY_DOWN == listboxSub.key)
						{
							MenuPage++;
							if (MenuPage > 3)
							{
								MenuPage = 0;
							}
						}
					}
				}
				else
				{
					goto flag;
				}
			}
			else if (listbox.item == 1)
#else
			if (listbox.item == 0)
#endif
			{
				char cInput[5] = {0};
				BYTE bTsa[17] = {0};
				/*TMtrRdCtrl *ptMtrRdCtrl = NULL;*/
				//WORD wPn = 0;
				MenuPage = 4;
				if(EditSpecBox(2,"����������:",cInput,60000,2,DATA_DEC)>= 0)
				{
					sscanf(cInput,"%d",&iGroupNo);
					if (iGroupNo<POINT_NUM && iGroupNo>=1 /*&& IsPnValid(iGroupNo)*/)
					{
						TOobMtrInfo tTMtrInfo;
						memset(&tTMtrInfo, 0, sizeof(TOobMtrInfo));
						WORD wPn = MtrSnToPn(iGroupNo);
						if (!GetMeterInfo(wPn, &tTMtrInfo))
						{
							MessageBox("�õ��δ����!", KEY_ESC, 3000);
							goto flag;
						}
					}
					else
					{
						MessageBox("�������Ŵ���",KEY_ESC,10000);
						goto flag;
					}
				}
				else
				{
					goto flag;
				}
				while(1)
				{
					i = 0;
#ifndef DEBUG_DISP
					memset(bTmpBuff, 0, sizeof(bTmpBuff));
					ReadClass1Data(iGroupNo, dwDeltaPnID[MenuPage-4], bTmpBuff);
					/*if (ptMtrRdCtrl != NULL)
					{
						GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, dwDeltaPnID[MenuPage-4], bTmpBuff);
						DTRACE(DB_FA, ("############# bValid=%d, bTaskId=%d, fRecSaved=%d, bCSDItemNum=%d, bSucFlg[0]=%d", ptMtrRdCtrl->taskSucFlg[0].bValid, ptMtrRdCtrl->taskSucFlg[0].bTaskId, ptMtrRdCtrl->taskSucFlg[0].fRecSaved, ptMtrRdCtrl->taskSucFlg[0].bCSDItemNum, ptMtrRdCtrl->taskSucFlg[0].bSucFlg[0]));
						DTRACE(DB_FA, ("############# dwOAD = %08x\n", dwDeltaPnID[MenuPage-4]));
						TraceBuf(DB_FA, "############# bBuf-->", ptMtrRdCtrl->mtrTmpData.bBuf, 100);
					}*/
#endif							
					cp = bTmpBuff+2+1;
					//memcpy(str, cp, 4);
					sprintf(menuitem[i++], "������%d:",iGroupNo);
					switch(MenuPage)
					{
					case 4:
					case 5:
						sprintf(menuitem[i++],"�ܵ���: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						cp += 5;
						sprintf(menuitem[i++],"��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						cp += 5;
						sprintf(menuitem[i++],"��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						cp += 5;
						sprintf(menuitem[i++],"ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						cp += 5;
						sprintf(menuitem[i++],"��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						break;
					case 6:
					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
						sprintf(menuitem[i++],"�ܵ���: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						cp += 5;
						sprintf(menuitem[i++],"��: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						cp += 5;
						sprintf(menuitem[i++],"��: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						cp += 5;
						sprintf(menuitem[i++],"ƽ: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						cp += 5;
						sprintf(menuitem[i++],"��: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
						break;
					}

					tmpS[i].text = NULL;
						
					listboxSub.Show(0, title[MenuPage], tmpS, KEY_ESC | KEY_UP<< 8 | KEY_DOWN << 16, 60000, false );

					if (KEY_ESC == listboxSub.key || KEY_NULL == listboxSub.key)
					{
						goto flag;
					}
					if (KEY_UP == listboxSub.key )
					{
						MenuPage--;
						if (MenuPage < 4)
						{
							MenuPage = 11;
						}
					}
					else if (KEY_DOWN == listboxSub.key)
					{
						MenuPage++;
						if (MenuPage > 11)
						{
							MenuPage = 4;
						}
					}
				}
			}
		}
		return -1;
}

int CurrntDemand(void *arg)
{
	int i;
	char menuitem[4][32];
	memset(menuitem, 0, sizeof(menuitem));
	CListBoxEx listbox;

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE bTmpBuff[20] = {0};

	int iGroupNo = 0;
	//BYTE bTsa[17] = {0};
	BYTE *cp = NULL;

	char cInput[5] = {0};
	if(EditSpecBox(2,"����������:",cInput, 60000, 2, DATA_DEC)>= 0)
	{
		WORD wPn = 0;
		TMtrRdCtrl *ptMtrRdCtrl = NULL;
		sscanf(cInput,"%d",&iGroupNo);
		if(iGroupNo<POINT_NUM && iGroupNo>=1 /*&& IsPnValid(iGroupNo)*/)
		{
			TOobMtrInfo tTMtrInfo;
			memset(&tTMtrInfo, 0, sizeof(TOobMtrInfo));
			WORD wPn = MtrSnToPn(iGroupNo);
			if (!GetMeterInfo(wPn, &tTMtrInfo))
			{
				MessageBox("�õ��δ����!", KEY_ESC, 3000);
				return -1;
			}
			/*wPn = MtrSnToPn(iGroupNo);
			GetMeterTsa(wPn, bTsa);
			ptMtrRdCtrl = GetMtrRdCtrl(wPn, bTsa);*/
		}
		else
		{
			MessageBox("�������Ŵ���",KEY_ESC,10000);
			return -1;
		}

		while(1)
		{
			i = 0;
			memset(bTmpBuff, 0, sizeof(bTmpBuff));
		
			/*if (ptMtrRdCtrl != NULL)
			{
				GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x20170200, bTmpBuff);
				GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x20180200, bTmpBuff+5);
				GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x20190200, bTmpBuff+10);
			}*/
			ReadClass1Data(iGroupNo, 0x20170200, bTmpBuff);
			ReadClass1Data(iGroupNo, 0x20180200, bTmpBuff+5);
			//ReadClass1Data(iGroupNo, 0x20190200, bTmpBuff+10);

			sprintf(menuitem[i++], "������%d:",iGroupNo);
			cp = bTmpBuff + 1;
			sprintf(menuitem[i++],"�й�����: %.4fkW", (float)OoDoubleLongToInt(cp)*1e-4);
			cp += 5;
			sprintf(menuitem[i++],"�޹�����: %.4fkvar", (float)OoDoubleLongToInt(cp)*1e-4);
			//cp += 5;
			//sprintf(menuitem[i++],"��������: %.4fkVA", (float)OoDoubleLongToInt(cp)*1e-4);

			tmpS[i].text = NULL;
			
			listbox.Show(0, "��ǰ����", tmpS, KEY_ESC | KEY_UP<< 8 | KEY_DOWN << 16, 60000, false );

			if (KEY_ESC == listbox.key || KEY_NULL == listbox.key)
			{
				return -1;
			}
		}
	}
	
	return -1;

}

int VoltAndCurrnt(void *arg)
{
	int i;
	char menuitem[5][32];
	memset(menuitem, 0, sizeof(menuitem));
	CListBoxEx listbox;

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 4 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	BYTE bTmpBuff[20] = {0};
	int iGroupNo = 0;
	//BYTE bTsa[17] = {0};
	BYTE *cp = NULL;
	int MenuPage = 0;
	char *title[] = {"��ѹ", "����"};
	BYTE bBuf[30] = {0};
	char cInput[5] = {0};
	DWORD dwPnID[] = {0x20000200, 0x20010200}; //��ѹ������

	if(EditSpecBox(2,"����������:",cInput, 60000, 2, DATA_DEC)>= 0)
	{
		//WORD wPn = 0;
		//TMtrRdCtrl *ptMtrRdCtrl = NULL;
		sscanf(cInput,"%d",&iGroupNo);

		if(iGroupNo<POINT_NUM && iGroupNo>=1 /*&& IsPnValid(iGroupNo)*/)
		{
			TOobMtrInfo tTMtrInfo;
			memset(&tTMtrInfo, 0, sizeof(TOobMtrInfo));
			WORD wPn = MtrSnToPn(iGroupNo);
			if (!GetMeterInfo(wPn, &tTMtrInfo))
			{
				MessageBox("�õ��δ����!", KEY_ESC, 3000);
				return -1;
			}
			/*wPn = MtrSnToPn(iGroupNo);
			GetMeterTsa(wPn, bTsa);
			ptMtrRdCtrl = GetMtrRdCtrl(wPn, bTsa);*/
		}
		else
		{
			MessageBox("�������Ŵ���",KEY_ESC,10000);
			return -1;
		}

		while(1)
		{
			i = 0;
			memset(menuitem, 0, sizeof(menuitem));
				
			/*if (ptMtrRdCtrl != NULL)
			{
				GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x20000200, bVolt);
				GetMtrItemMem(&ptMtrRdCtrl->mtrTmpData, 0x20010200, bCurrnt);
			}*/
			ReadClass1Data(iGroupNo, dwPnID[MenuPage], bBuf);

			if (MenuPage == 0)
			{
				sprintf(menuitem[i++], "������%d:", iGroupNo);

				if (bBuf[1] == 1) //����
				{
					cp = bBuf+3;
					sprintf(menuitem[i++],"����: %.1fV", OoLongUnsignedToWord(cp)*1e-1);
				}
				else //����
				{
					cp = bBuf+3;
					sprintf(menuitem[i++],"A��: %.1fV", OoLongUnsignedToWord(cp)*1e-1);
					cp += 3;
					sprintf(menuitem[i++],"B��: %.1fV", OoLongUnsignedToWord(cp)*1e-1);
					cp += 3;
					sprintf(menuitem[i++],"C��: %.1fV", OoLongUnsignedToWord(cp)*1e-1);
				}	
			}
			else
			{
				sprintf(menuitem[i++], "������%d:", iGroupNo);

				if (bBuf[1] == 2) //����,N��
				{
					cp = bBuf+3;
					sprintf(menuitem[i++],"����: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);
					cp += 5;
					sprintf(menuitem[i++],"N��: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);
				}
				else //����,N��
				{
					cp = bBuf+3;
					sprintf(menuitem[i++],"A��: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);
					cp += 5;
					sprintf(menuitem[i++],"B��: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);
					cp += 5;
					sprintf(menuitem[i++],"C��: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);
					cp += 5;
					sprintf(menuitem[i++],"N��: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);
				}
				
				tmpS[i].text = NULL;
			}
			listbox.Show(0, title[MenuPage], tmpS, KEY_ESC | KEY_UP<< 8 | KEY_DOWN<<16, 60000, false );

			if (KEY_ESC == listbox.key || KEY_NULL == listbox.key)
			{
				return -1;
			}
			if (KEY_UP == listbox.key )
			{
				MenuPage--;
				if (MenuPage < 0)
				{
					MenuPage = 1;
				}
			}
			else if (KEY_DOWN == listbox.key)
			{
				MenuPage++;
				if (MenuPage > 1)
				{
					MenuPage = 0;
				}
			}
		}
	}

	return -1;
}

int RemoteSignal(void * arg)
{
	CListBoxEx listBox;
#if 1
	char menuitem[8][32];
#else
	char menuitem[3][32];
#endif
	memset(menuitem, 0, sizeof(menuitem));
	char *State[MAX_SW_PORT_NUM] = {NULL};
	char *StateChange[MAX_SW_PORT_NUM] = {NULL};
	BYTE i = 0, j = 0;
	BYTE bBuf[50] = {0};
#if 1
	struct ListBoxExItem tmpM[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
#else
	struct ListBoxExItem tmpM[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
#endif
	typedef struct {
		BYTE bStructType;
		BYTE bParserNum;
		BYTE bParser1Type;
		BYTE bStatus;
		BYTE bParser2Type;
		BYTE bChange;
	}TSwitchUnit;

	TSwitchUnit tSwitchUnit;
	BYTE bSwitchUnitLen = sizeof(TSwitchUnit);
	memset(&tSwitchUnit, 0, bSwitchUnitLen);
	BYTE bOffSet = 0;
	int iStart = -1;
	BYTE bValidNum = 0;
	DWORD dwOIAtt = 0xF2030200;
	const ToaMap* pOI = GetOIMap(dwOIAtt);

	//BYTE bTestBuf[] = {};
	DWORD dwSec = GetClick();

	while(1)
	{
		i = 0;
		memset(bBuf, 0, sizeof(bBuf));
		if (0 > OIRead_Spec((ToaMap *)pOI, bBuf, sizeof(bBuf), &iStart))
		{
			MessageBox("��ȡ���ݿ����!", KEY_ESC, 6000);
			return -1;
		}

		bValidNum = bBuf[1];
		if (bValidNum > MAX_SW_PORT_NUM)
			bValidNum = MAX_SW_PORT_NUM;

		for (j=0; j<bValidNum; j++)
		{
			bOffSet = 2 + j * bSwitchUnitLen;
			memcpy(&tSwitchUnit, bBuf+bOffSet, sizeof(TSwitchUnit));
			if (tSwitchUnit.bStatus)
				State[j] = "��";
			else
				State[j] = "��";
			if (tSwitchUnit.bChange)
				StateChange[j] = "1 ";
			else
				StateChange[j] = "0 ";
		}
#if 1
		sprintf(menuitem[i++], "���غ�: 1   2   3   4");
		if (bValidNum == 8)
			sprintf(menuitem[i++], "        5   6   7   8");

		sprintf(menuitem[i++], "״̬:   %s  %s  %s  %s", State[0], State[1], State[2], State[3]);
		if (bValidNum == 8)
			sprintf(menuitem[i++], "        %s  %s  %s  %s", State[4], State[5], State[6], State[7]);

		sprintf(menuitem[i++], "��λ:   %s  %s  %s  %s", StateChange[0], StateChange[1], StateChange[2], StateChange[3]);
		if (bValidNum == 8)
			sprintf(menuitem[i++], "        %s  %s  %s  %s", StateChange[4], StateChange[5], StateChange[6], StateChange[7]);
#else
		sprintf(menuitem[i++], "���غ�: 1   2");
		sprintf(menuitem[i++], "״̬:   %s  %s", State[0], State[1]);
		sprintf(menuitem[i++], "��λ:   %s  %s", StateChange[0], StateChange[1]);
#endif
		tmpM[i].text = NULL;

		listBox.Show(0, "����״̬",tmpM, KEY_ESC << 8, 1000, false);
		BlightOn(true);

		if (listBox.key == KEY_ESC)
		{
			break;
		}

		if( (GetClick()-dwSec) > 60)
		{
			break;
		}
	}

	return -1;
}

/*
int PowerCtrlInfo(void *arg)
{
	CListBoxEx listBox;
	char menuitem[12][32];
	char str[20] = {0};
	char szTmp[10] = {0};
	BYTE i = 0, j = 0;
	BYTE pbBuf[140];
	BYTE *cp = NULL;
	BYTE bLen = 0;
	int iPage = 0;
	char title[20];
	bool isfind = false;
	char *CtrlName[] = {"ʱ�ο� ","���ݿ� ","Ӫҵ��ͣ�� ","�����¸��� "};
	struct ListBoxExItem tmpM[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 6 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	//int iret = ReadItemEx(BN0,PN0, 0x0006,pbBuf);//todo luom
#define RECORE_NUM 5
	BYTE RetRecNum = ReadClass3Item(6,RECORE_NUM,pbBuf,bLen);//��ȡ�����¼���¼(���2��)
	if (RetRecNum <= 0 || RetRecNum > RECORE_NUM)
	{
		MessageBox("�޸澯��¼,�뷵��!",KEY_ESC,2000);
		return -1;
	}
	while(1)
	{
		i = 0;
		//switch (iPage)
		//{
		//case 0:
		//	cp = pbBuf;
		//	break;
		//case 1:
		//case 2:
		//case 3:
		//case 4:
		//	cp = pbBuf + iPage * bLen;
		//	break;

		//}
		cp = pbBuf + iPage * bLen;
		cp += 1;
		Fmt15ToStr(cp+2,str);

		sprintf(menuitem[i++],"��բʱ��:");

		sprintf(menuitem[i++]," %s",str);

		sprintf(menuitem[i++],"�ܼ����: %d",*(cp + 7));

		sprintf(menuitem[i++],"��բ�ִ�:");

		memset(menuitem[i],0,sizeof(menuitem[0]));
		for ( j = 0; j < 8; j++)
		{
			if (*(cp + 8) & (1<<j))
			{
				sprintf(szTmp,"%d ",j+1);
				strcat(menuitem[i],szTmp);
			}
		}

		i++;
		sprintf(menuitem[i++],"�������:");
		memset(menuitem[i],0,sizeof(menuitem[0]));

		for ( j = 0; j < 4; j++)
		{
			if ((*(cp + 9) >> j) & 0x01)
			{
				strcat(menuitem[i], CtrlName[j]);
				isfind = true;
			}
		}
		if (!isfind)
		{
			strcat(menuitem[i],"δ֪");
		}
		i++;
		Fmt2ToStr(cp + 10,str);
		sprintf(menuitem[i++],"��ǰ����:%s",str);

		memset(str,0,sizeof(str));
		Fmt2ToStr(cp + 12,str);
		sprintf(menuitem[i++],"������:%s",str);

		memset(str,0,sizeof(str));
		Fmt2ToStr(cp + 14,str);
		sprintf(menuitem[i++],"��ʱ��ֵ:%s",str);

		tmpM[i].text = NULL;
		memset(title,0,sizeof(title));
		sprintf(title,"������բ��¼%d",iPage+1);
		listBox.Show(0, title,tmpM, KEY_ESC | KEY_LEFT << 8|KEY_RIGHT << 16, 60000);
		if (listBox.key == KEY_ESC || listBox.key == KEY_NULL)
		{
			return -1;
		}
		if (listBox.key == KEY_LEFT)
		{
			iPage--;
			if (iPage < 0)
			{
				iPage = RetRecNum-1;
			}
		}
		else if (listBox.key == KEY_RIGHT)
		{
			iPage++;
			if (iPage > RetRecNum-1)
			{
				iPage = 0;
			}
		}
	}
	return -1;
}*/

/*
int EnergCtrlInfo(void *arg)
{
	CListBoxEx listBox;
	char menuitem[12][32];
	char str[20] = {0};
	char szTmp[10] = {0};
	BYTE i = 0, j = 0;
	BYTE pbBuf[140];
	int iPage = 0;
	BYTE bLen = 0;
	BYTE *cp = NULL;
	char title[20];
	bool isfind = false;
	char *CtrlName[] = {"�µ�� ","����� ","�������� ","����ѿ� "};
	struct ListBoxExItem tmpM[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[13], 0xFE, Dummy, (void *) 6 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	//int iret = ReadItemEx(BN0,PN0, 0x0007,pbBuf);//todo luom

	BYTE RetRecNum = ReadClass3Item(7,RECORE_NUM,pbBuf,bLen);//��ȡ����¼���¼(���2��)
	if (RetRecNum <= 0 || RetRecNum > RECORE_NUM)
	{
		MessageBox("�޸澯��¼,�뷵��!",KEY_ESC,2000);
		return -1;
	}
	while (1)
	{
		i = 0;
		//switch (iPage)
		//{
		//case 0:
		//	cp = pbBuf;
		//	break;
		//case 1:
		//	cp = pbBuf + bLen;
		//	break;

		//}
		cp = pbBuf + iPage * bLen;
		cp += 1;
		Fmt15ToStr(cp+2,str);
		//DTRACE(DB_FAPROTO, ("-----eg---------- pbBuf[0] is %d, pbBuf[1] is %d, pbBuf[2] is %d,bLen is : %d !\n",pbBuf[0],pbBuf[1],pbBuf[2],bLen));

		sprintf(menuitem[i++],"��բʱ��:");

		sprintf(menuitem[i++]," %s",str);

		sprintf(menuitem[i++],"�ܼ����: %d",*(cp + 7));

		sprintf(menuitem[i++],"��բ�ִ�:");

		memset(menuitem[i],0,sizeof(menuitem[0]));
		for ( j = 0; j < 8; j++)
		{
			if (*(cp + 8) & (1<<j))
			{
				sprintf(szTmp,"%d ",j+1);
				strcat(menuitem[i],szTmp);
			}
		}

		i++;
		sprintf(menuitem[i++],"������:");
		memset(menuitem[i],0,sizeof(menuitem[0]));

		for ( j = 0; j < 2; j++)
		{
			if ((*(cp + 9) >> j) & 0x01)
			{
				if (j == 0)
				{
					strcat(menuitem[i], CtrlName[j]);
				}
				else if (j == 1)
				{
	#ifdef EN_CTRL
					//if (IsEnergyFee()) //todo luom
					{
					//	strcat(menuitem[i], CtrlName[j+2]);
					}
					//else
					{
						strcat(menuitem[i], CtrlName[j+1]);
					}

	#else
					strcat(menuitem[i],CtrlName[i]);
	#endif
				}
				
				isfind = true;
			}
		}
		if (!isfind)
		{
			strcat(menuitem[i],"δ֪");
		}
		i++;
		Fmt03ToStr(cp + 10,str);
		sprintf(menuitem[i++],"��բʱ����/��:");
		sprintf(menuitem[i++],"%s",str);

		memset(str,0,sizeof(str));
		Fmt03ToStr(cp + 14,str);
		sprintf(menuitem[i++],"��բʱ��ֵ:");
		sprintf(menuitem[i++],"%s",str);

		tmpM[i].text = NULL;
		memset(title,0,sizeof(title));
		sprintf(title,"�����բ��¼%d",iPage+1);
		listBox.Show(0, title, tmpM, KEY_ESC | KEY_LEFT << 8 | KEY_RIGHT << 16, 60000);
		if (listBox.key == KEY_ESC || listBox.key == KEY_NULL)
		{
			return -1;
		}
		if (listBox.key == KEY_LEFT)
		{
			iPage--;
			if (iPage < 0)
			{
				iPage = RetRecNum-1;
			}
		}
		else if (listBox.key == KEY_RIGHT)
		{
			iPage++;
			if (iPage > RetRecNum-1)
			{
				iPage = 0;
			}
		}
	}

	return -1;
}*/

/*
int RemoteCtrlInfo(void *arg)
{
	CListBoxEx listBox;
	char menuitem[10][32];
	char str[20] = {0};
	BYTE bLen = 0;
	char szTmp[10] = {0};
	BYTE i = 0, j = 0;
	int iPage = 0;
	BYTE *cp = NULL;
	char title[20];
	BYTE pbBuf[120];
	bool isfind = false;
//	char *CtrlName[] = {"�µ�� ","����� ","�������� ","����ѿ� "};
	struct ListBoxExItem tmpM[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 6 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	//int iret = ReadItemEx(BN0,PN0, 0x0007,pbBuf);//todo luom
	BYTE RetRecNum = ReadClass3Item(5,RECORE_NUM,pbBuf,bLen);//��ȡң���¼���¼(���2��)
	if (RetRecNum <= 0 || RetRecNum > RECORE_NUM)
	{
		MessageBox("�޸澯��¼,�뷵��!",KEY_ESC,2000);
		return -1;
	}
	while (1)
	{
		i = 0;
		//switch (iPage)
		//{
		//case 0:
		//	cp = pbBuf;
		//	break;
		//case 1:
		//	cp = pbBuf + bLen;
		//	break;

		//}
		cp = pbBuf + iPage * bLen;
		cp += 1;
		Fmt15ToStr(cp+2,str);

		sprintf(menuitem[i++],"��բʱ��:");

		sprintf(menuitem[i++]," %s",str);

	//	sprintf(menuitem[i++],"�ܼ����: %d",pbBuf[7]);

		sprintf(menuitem[i++],"��բ�ִ�:");

		memset(menuitem[i],0,sizeof(menuitem[0]));
		for ( j = 0; j < 8; j++)
		{
			if (*(cp+7) & (1<<j))
			{
				sprintf(szTmp,"%d ",j+1);
				strcat(menuitem[i],szTmp);
			}
		}

		i++;

		Fmt2ToStr(cp+8,str);
		sprintf(menuitem[i++],"��բʱ����:%skW",str);

		memset(str,0,sizeof(str));
		Fmt2ToStr(cp+10,str);
		sprintf(menuitem[i++],"��բ����:%skW",str);

		tmpM[i].text = NULL;
		memset(title,0,sizeof(title));
		sprintf(title,"ң����բ��¼%d",iPage+1);
		listBox.Show(0, title, tmpM, KEY_ESC | KEY_LEFT<< 8 | KEY_RIGHT << 16, 60000);
		if (listBox.key == KEY_ESC || listBox.key == KEY_NULL)
		{
			return -1;
		}
		if (listBox.key == KEY_LEFT)
		{
			iPage--;
			if (iPage < 0)
			{
				iPage = RetRecNum-1;
			}

		}
		else if (listBox.key == KEY_RIGHT)
		{
			iPage++;
			if (iPage > RetRecNum-1)
			{
				iPage = 0;
			}
		}
	}

	return -1;
}*/

/*
int PowerFailerInfo(void *arg)
{
	CListBoxEx listBox;
	char menuitem[9][32];
	char str[20] = {0};
	char szTmp[10] = {0};
	BYTE i = 0, j = 0;
	BYTE pbBuf[180];
	BYTE bLen = 0;
	int iPage = 0;
	BYTE *cp = NULL;
	char title[20];
	bool isfind = false;
	struct ListBoxExItem tmpM[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 6 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	//int iret = ReadItemEx(BN0,PN0, 0x000e,pbBuf);//todo luom
	BYTE RetRecNum = ReadClass3Item(14,RECORE_NUM,pbBuf,bLen);//��ȡң���¼���¼(���2��)

	if (RetRecNum <= 0 || RetRecNum > RECORE_NUM)
	{
		MessageBox("�޸澯��¼,�뷵��!",KEY_ESC,2000);
		return -1;
	}
	while (1)
	{
		i = 0;
		switch (iPage)
		{
		case 0:
			//cp = pbBuf;
			//break;
		case 1:
		case 2:
		case 3:
		case 4:
			cp = pbBuf + iPage * bLen;
			break;
		}
		cp += 1;
		Fmt15ToStr(cp+3,str);
		DTRACE(DB_FAPROTO, ("bLen is : %d !\n",bLen));
		TraceFrm("------------------",pbBuf, bLen);
		sprintf(menuitem[i++],"ͣ�緢��ʱ��:");

		sprintf(menuitem[i++]," %s",str);

		Fmt15ToStr(cp+8,str);
		sprintf(menuitem[i++],"�ϵ緢��ʱ��:");
		sprintf(menuitem[i++]," %s",str);

		tmpM[i].text = NULL;
		memset(title,0,sizeof(title));
		sprintf(title,"ʧ���¼%d",iPage+1);
		listBox.Show(0, title,tmpM, KEY_ESC | KEY_LEFT<< 8 | KEY_RIGHT << 16, 60000);
		if (listBox.key == KEY_ESC || listBox.key == KEY_NULL)
		{
			return -1;
		}
		if (listBox.key == KEY_LEFT)
		{
			iPage--;
			if (iPage < 0)
			{
				iPage = RetRecNum-1;
			}
		}
		else if (listBox.key == KEY_RIGHT)
		{
			iPage++;
			if (iPage > RetRecNum-1)
			{
				iPage = 0;
			}
		}
	}

	return -1;
}*/

int CurrntAc(void *arg)
{
	CListBoxEx listBox;
	char *title[] = {"���ɵ�ѹ","���ɵ���","�����й�����","�����޹�����","���ɹ�������","���ɵ�ѹ���","���ɵ������"};
	char menuitem[5][32];
	memset(menuitem, 0, sizeof(menuitem));
	int iPageNum = 0;
	//char str[20] = {0};
	BYTE *cp = NULL;
	BYTE i = 0,j = 0;
#ifdef DEBUG_DISP
	BYTE bVlot[] = {DT_ARRAY, 3, DT_DB_LONG_U, 0x08, 0x98,
								 DT_DB_LONG_U, 0x08, 0x98,
	                             DT_DB_LONG_U, 0x08, 0x98};

	BYTE bCurrent[] = {DT_ARRAY, 4, DT_DB_LONG, 0x00, 0x00, 0x05, 0xDC,
								    DT_DB_LONG, 0x00, 0x00, 0x05, 0xDC,
									DT_DB_LONG, 0x00, 0x00, 0x05, 0xDC,
									DT_DB_LONG, 0x00, 0x00, 0x05, 0xDC};

	BYTE bActivePower[] = {DT_ARRAY, 4, DT_DB_LONG, 0x00, 0x00, 0x07, 0x08,
										DT_DB_LONG, 0x00, 0x00, 0x02, 0x58,
										DT_DB_LONG, 0x00, 0x00, 0x02, 0x58,
										DT_DB_LONG, 0x00, 0x00, 0x02, 0x58};

	BYTE bReActivePower[] = {DT_ARRAY, 4, DT_DB_LONG, 0x00, 0x00, 0x07, 0x08,
										  DT_DB_LONG, 0x00, 0x00, 0x02, 0x58,
										  DT_DB_LONG, 0x00, 0x00, 0x02, 0x58,
										  DT_DB_LONG, 0x00, 0x00, 0x02, 0x58};

	BYTE bPowerFator[] = {DT_ARRAY, 4, DT_LONG, 0x00, 0xFF,
									   DT_LONG, 0x00, 0xFF,
									   DT_LONG, 0x00, 0xFF,
									   DT_LONG, 0x00, 0xFF};

	BYTE bVoltPhaseAngle[] = {DT_ARRAY, 3, 
										    DT_LONG_U, 0x04, 0xB0,
											DT_LONG_U, 0x04, 0xB0,
											DT_LONG_U, 0x04, 0xB0};

	BYTE bCurrentPhaseAngle[] = {DT_ARRAY, 3,
											  DT_LONG_U, 0x04, 0xB0,
											  DT_LONG_U, 0x04, 0xB0,
											  DT_LONG_U, 0x04, 0xB0};
#endif
	BYTE pbBuf[30] = {0};

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	while(1)
	{
		j = i;
		i = 0;
		//sprintf(menuitem[i++],"�������: %d",GetAcPn());
		//ReadItemEx(BN0,GetAcPn(), 0x8902,pbBuf);//todo luom
		//sprintf(menuitem[i++],"���ַ:%02X%02X%02X%02X%02X%02X",pbBuf[10],pbBuf[9],pbBuf[8],pbBuf[7],pbBuf[6],pbBuf[5]);

		tmpS[j].text = menuitem[j];
		memset(pbBuf, 0, sizeof(pbBuf));
		memset(menuitem, 0, sizeof(menuitem));
		switch (iPageNum)
		{
		case 0:
			ReadItemEx(BN0, PN0, 0x2000, pbBuf);  //��ȡ�ڲ�ID
			//DTRACE(DB_FAPROTO, ("-----eg---------- pbBuf[0] is %d, pbBuf[1] is %d, pbBuf[2] is %d ,pbBuf[3] is %d, pbBuf[4] is %d, pbBuf[5] is %d!\n",pbBuf[0],pbBuf[1],pbBuf[2],pbBuf[3],pbBuf[4],pbBuf[5]));
			//TraceFrm("------",pbBuf,20);
#ifdef DEBUG_DISP
			cp = bVlot + 2 + 1;
#else
			cp = pbBuf + 2 + 1;
#endif
			sprintf(menuitem[i++], "Ua: %.1fV", (float)OoLongUnsignedToWord(cp)*1e-1);
			
			cp += 3;
			sprintf(menuitem[i++], "Ub: %.1fV", (float)OoLongUnsignedToWord(cp)*1e-1);

			cp += 3;
			sprintf(menuitem[i++], "Uc: %.1fV", (float)OoLongUnsignedToWord(cp)*1e-1);
			break;
		case 1:
			ReadItemEx(BN0, PN0, 0x2001, pbBuf);
#ifdef DEBUG_DISP
			cp = bCurrent + 2 + 1;
#else
			cp = pbBuf + 2 + 1;
#endif
			sprintf(menuitem[i++], "Ia: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);

			cp += 5;
			sprintf(menuitem[i++], "Ib: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);

			cp += 5;
			sprintf(menuitem[i++], "Ic: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);
			
			cp += 5;
			sprintf(menuitem[i++], "�������: %.3fA", (float)OoDoubleLongToInt(cp)*1e-3);
			break;
		case 2:
			ReadItemEx(BN0, PN0, 0x2004, pbBuf);
#ifdef DEBUG_DISP
			cp = bActivePower + 2 + 1;
#else
			cp = pbBuf + 2 + 1;
#endif
			sprintf(menuitem[i++], "P: %.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

			cp += 5;
			sprintf(menuitem[i++],"Pa: %.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

			cp += 5;
			sprintf(menuitem[i++],"Pb: %.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

			cp += 5;
			sprintf(menuitem[i++],"Pc: %.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

			break;
		case 3:
			ReadItemEx(BN0, PN0, 0x2005, pbBuf);
#ifdef DEBUG_DISP
			cp = bReActivePower + 2 + 1;
#else
			cp = pbBuf + 2 + 1;
#endif
			sprintf(menuitem[i++], "Q: %.1fvar", (float)OoDoubleLongToInt(cp)*1e-1);

			cp += 5;
			sprintf(menuitem[i++], "Qa: %.1fvar", (float)OoDoubleLongToInt(cp)*1e-1);

			cp += 5;
			sprintf(menuitem[i++], "Qb: %.1fvar", (float)OoDoubleLongToInt(cp)*1e-1);

			cp += 5;
			sprintf(menuitem[i++], "Qc: %.1fvar", (float)OoDoubleLongToInt(cp)*1e-1);
			break;
		case 4:
			ReadItemEx(BN0, PN0, 0x200A, pbBuf);
#ifdef DEBUG_DISP
			cp = bPowerFator + 2 + 1;
#else
			cp = pbBuf + 2 + 1;
#endif
			sprintf(menuitem[i++], "cos��: %.3f", (float)OoLongToInt16(cp)*1e-3);

			cp += 3;
			sprintf(menuitem[i++], "cos��A: %.3f", (float)OoLongToInt16(cp)*1e-3);

			cp += 3;
			sprintf(menuitem[i++], "cos��B: %.3f", (float)OoLongToInt16(cp)*1e-3);

			cp += 3;
			sprintf(menuitem[i++], "cos��C: %.3f", (float)OoLongToInt16(cp)*1e-3);
			break;
		case 5:
			ReadItemEx(BN0, PN0, 0x2002, pbBuf);
#ifdef DEBUG_DISP
			cp = bVoltPhaseAngle + 2 + 1;
#else
			cp = pbBuf + 2 + 1;
#endif
			sprintf(menuitem[i++], "Ua���: %.1f ��", (float)OoLongUnsignedToWord(cp)*1e-1);

			cp += 3;
			sprintf(menuitem[i++], "Ub���: %.1f ��", (float)OoLongUnsignedToWord(cp)*1e-1);

			cp += 3;
			sprintf(menuitem[i++], "Uc���: %.1f ��", (float)OoLongUnsignedToWord(cp)*1e-1);
			break;
		case 6:
			ReadItemEx(BN0, PN0, 0x2003, pbBuf);
#ifdef DEBUG_DISP
			cp = bCurrentPhaseAngle + 2 + 1;
#else
			cp = pbBuf + 2 + 1;
#endif
			sprintf(menuitem[i++], "Ia���: %.1f ��", (float)OoLongUnsignedToWord(cp)*1e-1);

			cp += 3;
			sprintf(menuitem[i++], "Ib���: %.1f ��", (float)OoLongUnsignedToWord(cp)*1e-1);

			cp += 3;
			sprintf(menuitem[i++], "Ic���: %.1f ��", (float)OoLongUnsignedToWord(cp)*1e-1);
			break;
		default:
			break;
		}
		tmpS[i].text = NULL;

		listBox.Show(0, title[iPageNum],tmpS, KEY_ESC | KEY_DOWN<<8 | KEY_UP<<16, 60000, false);
		if (listBox.key == KEY_ESC || listBox.key == KEY_NULL)
		{
			return -1;
		}
		if (listBox.key == KEY_UP)
		{
			iPageNum--;
			if (iPageNum < 0)
			{
				iPageNum = 6;
			}
		}
		else if (listBox.key == KEY_DOWN)
		{
			iPageNum++;
			if (iPageNum > 6)
			{
				iPageNum = 0;
			}
		}
	}

	return -1;
}

int CurrntEnergAc(void *arg)
{
	CListBoxEx listBox;
	char *title[] = {"�����й�����ʾֵ", "�����й�����ʾֵ", "�����޹�����ʾֵ", "�����޹�����ʾֵ", "�����޹�����ʾֵ", "�����޹�����ʾֵ", "�����޹�����ʾֵ", "�����޹�����ʾֵ",};
	char menuitem[6][32];
	int iPageNum = 0;
	BYTE i = 0,j = 0;
	BYTE pbBuf[50] = {0};
	BYTE *cp = NULL;
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	while(1)
	{
		j = i;
		i = 0;
		
		tmpS[j].text = menuitem[j];
		memset(menuitem, 0, sizeof(menuitem));
		switch (iPageNum)
		{
		case 0:
			memset(pbBuf, 0, sizeof(pbBuf));
			ReadItemEx(BN0, PN0, 0x0010, pbBuf);
			cp = pbBuf + 2 + 1;
			sprintf(menuitem[i++], "������: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			break;
		case 1:
			memset(pbBuf, 0, sizeof(pbBuf));
			ReadItemEx(BN0, PN0, 0x0020, pbBuf);
			cp = pbBuf + 2 + 1;
			sprintf(menuitem[i++], "������: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			break;
		case 2:
			memset(pbBuf, 0, sizeof(pbBuf));
			ReadItemEx(BN0, PN0, 0x0030, pbBuf);
			cp = pbBuf + 2 + 1;
			sprintf(menuitem[i++], "������: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "ƽ: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			break;
		case 3:
			memset(pbBuf, 0, sizeof(pbBuf));
			ReadItemEx(BN0, PN0, 0x0040, pbBuf);
			cp = pbBuf + 2 + 1;
			sprintf(menuitem[i++], "������: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "ƽ: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkvarh", (float)OoDoubleLongToInt(cp)*1e-2);
			break;
		case 4:
			memset(pbBuf, 0, sizeof(pbBuf));
			ReadItemEx(BN0, PN0, 0x0050, pbBuf);
			cp = pbBuf + 2 + 1;
			sprintf(menuitem[i++], "һ�����޹�: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			break;
		case 5:
			memset(pbBuf, 0, sizeof(pbBuf));
			ReadItemEx(BN0, PN0, 0x0060, pbBuf);
			cp = pbBuf + 2 + 1;
			sprintf(menuitem[i++], "�������޹�: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			break;
		case 6:
			memset(pbBuf, 0, sizeof(pbBuf));
			ReadItemEx(BN0, PN0, 0x0070, pbBuf);
			cp = pbBuf + 2 + 1;
			sprintf(menuitem[i++], "�������޹�: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			break;
		case 7:
			memset(pbBuf, 0, sizeof(pbBuf));
			ReadItemEx(BN0, PN0, 0x0080, pbBuf);
			cp = pbBuf + 2 + 1;
			sprintf(menuitem[i++], "�������޹�: %.2fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			cp += 5;
			sprintf(menuitem[i++], "��: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
			break;
		default:
			break;
		}
		tmpS[i].text = NULL;

		listBox.Show(0, title[iPageNum], tmpS, KEY_ESC | KEY_DOWN<<8 | KEY_UP<<16, 60000, false);
		if (listBox.key == KEY_ESC || listBox.key == KEY_NULL)
		{
			return -1;
		}
		if (listBox.key == KEY_UP)
		{
			iPageNum--;
			if (iPageNum < 0)
			{
				iPageNum = 7;
			}
		}
		else if (listBox.key == KEY_DOWN)
		{
			iPageNum++;
			if (iPageNum > 7)
			{
				iPageNum = 0;
			}
		}
	}
	return -1;
}

int AcSampleInfo(void *arg)
{
	CListBoxEx listbox;
	char menuitem[3][32]; 
	struct ListBoxExItem tmp[] = { 
		{ "����˲ʱ��", 0xFE, CurrntAc, (void *) 1 }, //
		{ "���ɵ�ǰ����", 0xFE, CurrntEnergAc, (void *) 2 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	while(1)
	{
		listbox.Show(0 , "������Ϣ", tmp, KEY_ESC << 8, 60000);
		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

int PulseMeteringInfo(void *arg)
{
	CListBoxEx listBox;
	char menuitem[7][32];
	BYTE i = 0;
	BYTE pbBuf[50] = {0};
	BYTE *cp = NULL;
	int iPurseNum = 0;
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	char cInput[5] = {0};
	if(EditSpecBox(2,"������������(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
	{
		sscanf(cInput, "%d", &iPurseNum);
		
		while(1)
		{
			i = 0;
			if(iPurseNum<9 && iPurseNum>0 /*&& IsGrpValid(iGroupNo)*/)
			{
				sprintf(menuitem[i++], "�������:%d", iPurseNum);

				memset(pbBuf, 0, sizeof(pbBuf));
				ReadItemEx(BN0, iPurseNum-1, 0x2404, pbBuf);  //�й����ʣ���ȡ�ڲ�ID
				cp = pbBuf + 1;
				sprintf(menuitem[i++], "�й�����:%.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

				memset(pbBuf, 0, sizeof(pbBuf));
				ReadItemEx(BN0, iPurseNum-1, 0x2405, pbBuf);  //�޹����ʣ���ȡ�ڲ�ID
				cp = pbBuf + 1;
				sprintf(menuitem[i++], "�޹�����:%.1fW", (float)OoDoubleLongToInt(cp)*1e-1);

				memset(pbBuf, 0, sizeof(pbBuf));
				ReadItemEx(BN0, iPurseNum-1, 0x2414, pbBuf);  //�����й�����ʾֵ����ȡ�ڲ�ID
				cp = pbBuf + 2 + 1;
				sprintf(menuitem[i++], "���й�ʾֵ:%.4fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);

				memset(pbBuf, 0, sizeof(pbBuf));
				ReadItemEx(BN0, iPurseNum-1, 0x2416, pbBuf);  //�����й�����ʾֵ����ȡ�ڲ�ID
				cp = pbBuf + 2 + 1;
				sprintf(menuitem[i++], "���й�ʾֵ:%.4fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);

				memset(pbBuf, 0, sizeof(pbBuf));
				ReadItemEx(BN0, iPurseNum-1, 0x2415, pbBuf);  //�����޹�����ʾֵ����ȡ�ڲ�ID
				cp = pbBuf + 2 + 1;
				sprintf(menuitem[i++], "���޹�ʾֵ:%.4fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);

				memset(pbBuf, 0, sizeof(pbBuf));
				ReadItemEx(BN0, iPurseNum-1, 0x2417, pbBuf);  //�����޹�����ʾֵ����ȡ�ڲ�ID
				cp = pbBuf + 2 + 1;
				sprintf(menuitem[i++], "���޹�ʾֵ:%.4fkvarh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);

				tmpS[i].text = NULL;
			}
			else
			{
				MessageBox("���������Ŵ���", KEY_ESC, 10000);
				return -1;
			}

			listBox.Show(0, "���������Ϣ", tmpS, KEY_ESC | KEY_DOWN<<8 | KEY_UP<<16, 60000, false);
			if (listBox.key == KEY_ESC || listBox.key == KEY_NULL)
			{
				return -1;
			}
		}
		
	}

	return -1;
}

#define  CURVE_POINTS 96
#define  STATE_BAR_NUM 2//����״̬��

/*
int DrawPowerCurve(void *arg)
{
	int iType = (int)arg;
	int iGroupNo=1;
	int iPage = 0;
	BYTE bBuf[500];
	BYTE bPointNum = 0;
	WORD wPn=0;
	BYTE bFn=0;
	WORD wLen;
	BYTE *pbBuf = NULL;
	BYTE bLen = 2, k = 0;
	DWORD dwMaxValue = 0;
	bool fDataValid[CURVE_POINTS];
	DWORD dwCurve[CURVE_POINTS];
	char cInput[20] = {0};
	BYTE bPointx,bPointy;
	BYTE bReduceY;
	char title[22];
	struct KeyState keyState;

	if (iType == 0)
	{
		bLen = 2;
		memset(cInput,0,sizeof(cInput));
		if (EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
		{
flag0:		memset(title,0,sizeof(title));
			sscanf(cInput,"%d",&iGroupNo);
			if (iGroupNo > 0 && iGroupNo < 9 && IsGrpValid(iGroupNo))
			{
				sprintf(title,"�ܼ���%d ",iGroupNo);

				if (iPage == 0)
				{
					bFn = 73;
					strcat(title,"�й���������");
				}
				else if (iPage == 1)
				{
					bFn = 74;
					strcat(title,"�޹���������");
				}
			}
			else
			{
				MessageBox("�����ܼ������",KEY_ESC,2000);
				//break;
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else if (iType == 1)
	{
		bLen = 3;
		memset(cInput,0,sizeof(cInput));
		if (EditSpecBox(2,"������������(1-64):",cInput, 60000, 2,DATA_DEC)>= 0)
		{
flag1:		memset(title,0,sizeof(title));
			sscanf(cInput,"%d",&iGroupNo);
			if (iGroupNo>=GB_MAXOFF && iGroupNo<GB_MAXMETER && IsPnValid(iGroupNo))
			{
				BYTE bTmpFn[] = {81,85,82,86,83,87,84,88};
				char *szTip[] = {"P����","Q����","PA����","QA����","PB����","QB����","PC����","QC����"};
				bFn = bTmpFn[iPage];
				sprintf(title,"������%d ",iGroupNo);
				strcat(title,szTip[iPage]);
			}
			else
			{
				MessageBox("������������",KEY_ESC,2000);
				//break;
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	if (iType == 1 && GetPnProp(iGroupNo) == PN_PROP_PULSE && iPage >= 2)
	{
		MessageBox("�����޸����๦������",KEY_ESC,2000);
		return -1;
	}
	ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());//��һ����Ļ
	DrawStringHCenterAtLock(title, LCD_GET_XSIZE() / 2, GetFontHeight(), GUI_WHITE, GUI_BLACK);
	TTime now;
	//bLen = 3;
	GetCurTime(&now);
	bPointNum=(now.nHour)*4 + (now.nMinute)/15;
	bBuf[0] = 0;							//min
	bBuf[1] = 0;							//hour
	bBuf[2] = ByteToBcd(now.nDay);			//day
	bBuf[3] = ByteToBcd(now.nMonth);		//month
	bBuf[4] = ByteToBcd(now.nYear - 2000);	//year
	bBuf[5] = 1;							//�ܶ�,1:������ʱ��Ϊ15����
	bBuf[6] = bPointNum;					//����,ÿ15����һ����

	int iLen = GBReadItemEx(GB_DATACLASS2, bFn, iGroupNo, bBuf, &wLen);	//��������
	pbBuf = &bBuf[7];
	for (k=0; k<bPointNum; k++)
	{	//ͳ�����е������й�����,���ó����������й�����
		if (*pbBuf == INVALID_DATA)
		{
			fDataValid[k] = false;
			dwCurve[k] = 0;
		}
		else
		{
			fDataValid[k] = true;	
			if (iType == 0)		//�ܼ��鹦������
				dwCurve[k] = (DWORD)(ABS(Fmt02ToDouble(pbBuf)));
			else if (iType == 1)	//�����㹦������
			{
				dwCurve[k] =(DWORD)(ABS(Fmt09ToLong(pbBuf)));
			}
				
			if (dwMaxValue < dwCurve[k]) 
			{
				if (dwCurve[k] < (dwMaxValue * 100) || dwMaxValue == 0)//����100�� ���Ե�
				{
					dwMaxValue = dwCurve[k];
				}
			}
		}
		pbBuf += bLen;
	}

	bReduceY = (STATE_BAR_NUM - 1) * 16;

	//��ͼ
	for (k=0; k<bPointNum; k++)	
	{
		if (fDataValid[k] == true)		//����Ч�Ż���,�õ����
		{			
			bPointx = 24 + k;
			if (dwMaxValue == 0)
				bPointy = 152 - bReduceY;
			else
				bPointy = 152 - bReduceY - dwCurve[k]*100/dwMaxValue;
			break;
		}
	}
	for (k=0; k<bPointNum; k++)	
	{
		if (fDataValid[k] == true)		//����Ч�Ż���
		{		
			if((dwCurve[k] > (dwMaxValue * 100)) && dwMaxValue != 0)
			{
				continue;
			}
			if (dwMaxValue == 0)
				GUI_DrawLine(bPointx, bPointy, 24+k, 152-bReduceY);//��ֱ��
			else
				GUI_DrawLine(bPointx, bPointy, 24+k, 152-bReduceY-dwCurve[k]*100/dwMaxValue);
			bPointx = 24 + k;
			if (dwMaxValue == 0)
				bPointy = 152 - bReduceY;
			else
				bPointy = 152 - bReduceY - dwCurve[k]*100/dwMaxValue;
		}
	}

	DrawBmp8x8(arrowR0,142,152-bReduceY - 3);//�����ͷ
	DrawBmp8x8(arrowU,20,58);//�����ͷ
	DrawRectLock(0,  152-bReduceY, 145, 152-bReduceY);//����
	DrawRectLock(24, 58,  24,  159-bReduceY);//����
	LcdRefresh();
	BYTE Recnt = 0;

	while(1)
	{
		Sleep(100);
		Recnt++;
		if (Recnt == 5)
		{
			DrawStateTask();//����״̬��
			LcdRefresh();
			Recnt = 0;
		}

		keyState = GetKey();
		if (keyState.idle > 60000 / 1000 || keyState.key == KEY_ESC)
		{
			break;
		}
		if (keyState.key == KEY_UP)
		{
			iPage++;
			if (iType == 0)
			{
				if (iPage > 1)
				{
					iPage = 0;
				}
				goto flag0;
			}
			else
			{
				if (iPage > 7)
				{
					iPage = 0;
				}
				goto flag1;
			}
		}
		else if (keyState.key == KEY_DOWN)
		{
			iPage--;
			if (iType == 0)
			{
				if (iPage < 0)
				{
					iPage = 1;
				}
				goto flag0;
			}
			else
			{
				if (iPage < 0)
				{
					iPage = 7;
				}
				goto flag1;
			}
		}
	}

	return -1;
}
*/


int ShowCurve(void *arg)
{
	CListBoxEx listbox;
	char menuitem[2][32]; 

	struct ListBoxExItem tmp[] = { 
		//{ "�����ܼ��鹦������", 0xFE, DrawPowerCurve, (void *) 0 }, //
		//{ "���ղ����㹦������", 0xFE, DrawPowerCurve, (void *) 1 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	while(1)
	{
		listbox.Show(0 , "��������", tmp, KEY_ESC , 60000);
		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}
	
	return -1;
}


int QueryRealUIP(void *arg)
{
	CListBoxEx listbox;
	//char menuitem[10][32]; 

	int i = 0;
	/*struct ListBoxExItem tmp[] = { 
		//{ "��ǰ����", 0xFF, CurrntPower, (void *) 1 }, //
		{ "��ǰ����", 0xFF, CurrntEnage, (void *) 2 }, //
		//{ "��������", 0xFF, ShowCurve, (void *) 3 }, //
		//{ "����״̬", 0xFF, RemoteSignal, (void *) 4 }, //
		//{ "���ؼ�¼", 0xFF, PowerCtrlInfo, (void *) 0 }, //
		//{ "��ؼ�¼", 0xFF, EnergCtrlInfo, (void *) 1 }, //
		//{ "ң�ؼ�¼", 0xFF, RemoteCtrlInfo, (void *) 2 }, //
		//{ "ʧ���¼", 0xFF, PowerFailerInfo, (void *) 8 }, //
		//{ "����������Ϣ", 0xFF, AcSampleInfo , (void *) 8 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
*/

	struct ListBoxExItem tmp[] = { 
		{ "��ǰ������", 0xFF, CurrntEnage, (void *) 1 }, 
		{ "��ǰ���ʼ���������", 0xFF, CurrntPower, (void *) 2 }, 
		{ "��ǰ����", 0xFF, CurrntDemand, (void *) 3 },
		{ "��ѹ����", 0xFF, VoltAndCurrnt, (void *) 4 },
		{ "����������Ϣ", 0xFF, AcSampleInfo, (void *) 5 },
		{ "���������Ϣ", 0xFF, PulseMeteringInfo, (void *) 6 },
		{ NULL, 0xFF, NULL, NULL }, //
	};

	while(1)
	{
		listbox.Show(0 , "ʵʱ����", tmp, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

int QueryDataOfDay(void *arg)
{
	int iStart = -1;
	int iTabIdx = 0;
	WORD wRetNum = 0;
	TTime tTime;
	DWORD dwCurSec;

	char menuitem[7][32];
	memset(menuitem, 0, sizeof(menuitem));
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{NULL, 0xFF, NULL, NULL },
	};
	//char *page[] = {"�����й�����ʾֵ", "�����й�����ʾֵ", "�����й��������", "�����й��������"};
	CListBoxEx listbox;
	char cInput[12] = {0};
	BYTE i = 0, j = 0;
	//char szInput[33];
	int iPageNum = 0;
	int iMtrNo = 0;
	BYTE bTime[7] = {0};
	BYTE bPower1[27] = {0};
	BYTE bPower2[27] = {0};
	BYTE bMaxDemand1[80] = {0};
	BYTE bMaxDemand2[80] = {0};
	BYTE *cp = NULL;

	int nRet;
#ifdef DEBUG_DISP
	BYTE bTmpBuf[] = {DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x10, 0x1E, 0x00, //�ն���ʱ��
					  DT_ARRAY, 5, DT_DB_LONG_U, 0x00, 0x05, 0x7E, 0x40, 
					               DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
							       DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,        //�����й��ܼ�����
					  DT_ARRAY, 5, DT_DB_LONG_U, 0x00, 0x05, 0x7E, 0x40, 
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,        //�����й��ܼ�����
					  DT_ARRAY, 5, DT_STRUCT, 2, 
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
											    DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
											    DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,     //�����й��������
					  DT_ARRAY, 5, DT_STRUCT, 2, 
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
						           DT_STRUCT, 2,
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00     //�����й��������
	};
#else
	BYTE bTmpBuf[500] = {0};
#endif
	BYTE bOAD[4] = {0x60, 0x12, 0x03, 0x00};  //�������ñ� ��¼��Ԫ		   
	BYTE bRSD[30] = {0};                                      //��¼ѡ��������
	BYTE bRCSD[30] = {0x02, //02 ���� RCSD��SEQUENCE OF CSD����=2
					  0x00, //OAD
					  0x60, 0x41, 0x02, 0x00, //���� OAD1���ɼ��ɹ�ʱ��
					  0x01, // ���� ROAD					 
					  0x50, 0x04, 0x02, 0x00, //���� �ն���
					  0x04, //���� ������������������  SEQUENCE OF����=4
					  0x00, 0x10, 0x02, 0x00, //���� �����й��ܼ�����
					  0x00, 0x20, 0x02, 0x00, //���� �����й��ܼ�����
					  0x10, 0x10, 0x02, 0x00, //���� �����й��������
					  0x10, 0x20, 0x02, 0x00, //���� �����й��������
	};  //��¼��ѡ��������                               
	BYTE *ptr = NULL;
	
	if(EditSpecBox(2,"����������:",cInput,60000,2,DATA_DEC)>= 0)
	{
		sscanf(cInput, "%d", &iMtrNo);
		if (iMtrNo<POINT_NUM && iMtrNo>=1)
		{
			TOobMtrInfo tTMtrInfo;
			memset(&tTMtrInfo, 0, sizeof(TOobMtrInfo));
			WORD wPn = MtrSnToPn(iMtrNo);
			if (!GetMeterInfo(wPn, &tTMtrInfo))
			{
				MessageBox("�õ��δ����!", KEY_ESC, 3000);
				goto flag;
			}
			//memset(cInput, 0, sizeof(cInput));
			GetCurTime(&tTime);
			dwCurSec = TimeToSeconds(tTime) - 24*60*60; //ʹ�õ�ǰ���ڵ���һ����ΪĬ�ϲ�ѯ����
			SecondsToTime(dwCurSec, &tTime); 

			sprintf(cInput, "%04d-%02d-%02d", tTime.nYear, tTime.nMonth, tTime.nDay);
			if (EditTextBox(2, "��ѡ���ѯ����", cInput, 60000, 10, DATA_DEC)>= 0)
			{
				if (cInput[4]=='-' && cInput[7]=='-')
				{
					WORD wYear;
					BYTE bMonth;
					BYTE bDay;

					wYear = (cInput[0]-'0')*1000 + (cInput[1]-'0')*100 + (cInput[2]-'0')*10 + (cInput[3]-'0');
					bMonth = (cInput[5]-'0')*10 + (cInput[6]-'0');
					bDay = (cInput[8]-'0')*10 + (cInput[9]-'0');
					OoWordToLongUnsigned(wYear, bTime);
					bTime[2] = bMonth;
					bTime[3] = bDay;  

					/*
					**��¼ѡ��������RSD����
					*/
					ptr = bRSD;
					*ptr = 5;       //���� RSD��ѡ�񷽷�5
					ptr++;          //���� �ɼ��洢ʱ��
					memcpy(ptr, bTime, sizeof(bTime));
					ptr += sizeof(bTime);  
					*ptr = 4;       //���� ���ܱ���MS һ���������  	[4] 	SEQUENCE OF long-unsigned
					ptr++;          
					*ptr = 1;        //���������Ÿ���=1
					ptr++;
					*ptr = iMtrNo;   //������
#ifndef DEBUG_DISP
					nRet = ReadRecord(bOAD, bRSD, bRCSD, &iTabIdx, &iStart, bTmpBuf, sizeof(bTmpBuf), &wRetNum);
					if (nRet < 0)
					{
						MessageBox("��ѯʧ��",KEY_ESC,10000);
						goto flag;
					}
#endif
					memset(bTime, 0, sizeof(bTime));
					ptr = bTmpBuf;          //OAD1���ɼ��ɹ�ʱ��
					memcpy(bTime, ptr, 8);  //date_time_s ��-��-�� ʱ:��:�� 8bytes
					ptr += 8;               //�����й��ܼ�����
					memcpy(bPower1, ptr, 27);
					ptr += 27;              //�����й��ܼ�����
					memcpy(bPower2, ptr, 27);
					ptr += 27;              //�����й��������
					memcpy(bMaxDemand1, ptr, 77);
					ptr += 77;              //�����й��������
					memcpy(bMaxDemand2, ptr, 77);
					
					while(1)
					{
						i = 0;
						memset(menuitem, 0, sizeof(menuitem));
						switch(iPageNum)
						{
						case 0:
							cp = bPower1+3;
							sprintf(menuitem[i++], "������: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���м�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���з�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "����ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���й�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							break;
						case 1:
							cp = bPower2+3;
							sprintf(menuitem[i++], "������: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���м�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���з�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "����ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���й�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							break;
						case 2:
							cp = bMaxDemand1+5;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "��������ƽ: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							break;
						case 3:
							cp = bMaxDemand2+5;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "��������ƽ: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							break;
						}
						sprintf(menuitem[i++], " ");
						sprintf(menuitem[i++], "����ʱ�� %d/%d/%d %d:%d", OoLongUnsignedToWord(bTime+1)%100, bTime[3], bTime[4], bTime[5], bTime[6]);
						tmpS[i].text = NULL;

						listbox.Show(0, "������", tmpS, KEY_ESC | (KEY_OK << 8) | (KEY_UP << 16) | (KEY_DOWN << 24), 60000, false);
						if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
						{
							goto flag;
						}
						if (listbox.key == KEY_UP)
						{
							iPageNum--;
							if (iPageNum < 0)
							{
								iPageNum = 3;
							}
						}
						else if (listbox.key == KEY_DOWN)
						{
							iPageNum++;
							if (iPageNum > 3)
							{
								iPageNum = 0;
							}
						}
					}	
				}
				else
				{
					MessageBox("����ʱ�����",KEY_ESC,10000);
					goto flag;
				}
			}
		}
		else
		{
			MessageBox("�������Ŵ���",KEY_ESC,10000);
			goto flag;
		}
	}

flag:
	return -1;
}



int QueryDataOfMonth(void *arg)
{
	BYTE * pRCSD = NULL;
	BYTE * pRSD = NULL;
	int iStart = -1;
	int iTabIdx = 0;
	WORD wRetNum = 0;
	TTime tTime;

	char menuitem[7][32];
	memset(menuitem, 0, sizeof(menuitem));
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{NULL, 0xFF, NULL, NULL },
	};
	//char *page[] = {"�����й�����ʾֵ", "�����й�����ʾֵ", "�����й��������", "�����й��������"};
	CListBoxEx listbox;
	char cInput[12] = {0};
	BYTE i = 0, j = 0;
	//char szInput[33];
	int iPageNum = 0;
	int iMtrNo = 0;
	BYTE bTime[7] = {0};
	BYTE bPower1[27] = {0};
	BYTE bPower2[27] = {0};
	BYTE bMaxDemand1[80] = {0};
	BYTE bMaxDemand2[80] = {0};
	BYTE *cp = NULL;

	int nRet;
#ifdef DEBUG_DISP
	BYTE bTmpBuf[] = {DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x10, 0x1E, 0x00, //�ն���ʱ��
					  DT_ARRAY, 5, DT_DB_LONG_U, 0x00, 0x05, 0x7E, 0x40, 
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,        //�����й��ܼ�����
					  DT_ARRAY, 5, DT_DB_LONG_U, 0x00, 0x05, 0x7E, 0x40, 
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,
								   DT_DB_LONG_U, 0x00, 0x01, 0x5F, 0x90,        //�����й��ܼ�����
					  DT_ARRAY, 5, DT_STRUCT, 2, 
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
							       DT_STRUCT, 2,
											     DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,     //�����й��������
					  DT_ARRAY, 5, DT_STRUCT, 2, 
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00,
								   DT_STRUCT, 2,
												 DT_DB_LONG_U, 0x00, 0x0F, 0x42, 0x40,
												 DT_DATE_TIME_S, 0x07, 0xE0, 0x0B, 0x0A, 0x00, 0x00, 0x00     //�����й��������
	};
#else
	BYTE bTmpBuf[500] = {0};
#endif
	BYTE bOAD[4] = {0x50,0x06,0x02,0x00};		              //�����������������¶���
	BYTE bRSD[30] = {0};                                      //��¼ѡ��������
	BYTE bRCSD[30] = {0x02, //02 ���� RCSD��SEQUENCE OF CSD����=2
					  0x00, //OAD
					  0x60, 0x41, 0x02, 0x00, //���� OAD1���ɼ��ɹ�ʱ��
					  0x01, // ���� ROAD					 
					  0x50, 0x06, 0x02, 0x00, //���� �¶���
					  0x04, //���� ������������������  SEQUENCE OF����=4
					  0x00, 0x10, 0x02, 0x00, //���� �����й��ܼ�����
					  0x00, 0x20, 0x02, 0x00, //���� �����й��ܼ�����
					  0x10, 0x10, 0x02, 0x00, //���� �����й��������
					  0x10, 0x20, 0x02, 0x00, //���� �����й��������
	};  //��¼��ѡ��������                               
	BYTE *ptr = NULL;
	
	if(EditSpecBox(2,"����������:",cInput,60000,2,DATA_DEC)>= 0)
	{
		sscanf(cInput, "%d", &iMtrNo);
		if (iMtrNo<GB_MAXMETER && iMtrNo>=GB_MAXOFF)
		{
			TOobMtrInfo tTMtrInfo;
			memset(&tTMtrInfo, 0, sizeof(TOobMtrInfo));
			WORD wPn = MtrSnToPn(iMtrNo);
			if (!GetMeterInfo(wPn, &tTMtrInfo))
			{
				MessageBox("�õ��δ����!", KEY_ESC, 3000);
				goto flag;
			}
			//memset(cInput, 0, sizeof(cInput));
			GetCurTime(&tTime);
			sprintf(cInput, "%04d-%02d", tTime.nYear, tTime.nMonth);
			if (EditTextBox(2, "��ѡ���ѯ����", cInput, 60000, 7, DATA_DEC)>= 0)
			{
				if (cInput[4]=='-')
				{
					WORD wYear;
					BYTE bMonth; 

					AsciiToByte((BYTE *)cInput, 4, bTime);    //��
					AsciiToByte((BYTE *)(cInput+5), 2, bTime+2);//��
					wYear = (cInput[0]-'0')*1000 + (cInput[1]-'0')*100 + (cInput[2]-'0')*10 + (cInput[3]-'0');
					bMonth = (cInput[5]-'0')*10 + (cInput[6]-'0');
					OoWordToLongUnsigned(wYear, bTime);
					bTime[2] = bMonth;
					bTime[3] = 1;//��
					/*
					**��¼ѡ��������RSD����
					*/
					ptr = bRSD;
					*ptr = 5;       //���� RSD��ѡ�񷽷�5
					ptr++;          //���� �ɼ��洢ʱ��
					memcpy(ptr, bTime, sizeof(bTime));
					ptr += sizeof(bTime);  
					*ptr = 4;       //���� ���ܱ���MS һ���������  	[4] 	SEQUENCE OF long-unsigned
					ptr++;          
					*ptr = 1;        //���������Ÿ���=1
					ptr++;
					*ptr = iMtrNo;   //������
#ifndef DEBUG_DISP
					nRet = ReadRecord(bOAD, bRSD, bRCSD, &iTabIdx, &iStart, bTmpBuf, sizeof(bTmpBuf), &wRetNum);
					if (nRet < 0)
					{
						MessageBox("��ѯʧ��",KEY_ESC,10000);
						goto flag;
					}
#endif
					memset(bTime, 0, sizeof(bTime));
					ptr = bTmpBuf;          //OAD1���ɼ��ɹ�ʱ��
					memcpy(bTime, ptr, 8);  //date_time_s ��-��-�� ʱ:��:�� 8bytes
					ptr += 8;               //�����й��ܼ�����
					memcpy(bPower1, ptr, 27);
					ptr += 27;              //�����й��ܼ�����
					memcpy(bPower2, ptr, 27);
					ptr += 27;              //�����й��������
					memcpy(bMaxDemand1, ptr, 77);
					ptr += 77;              //�����й��������
					memcpy(bMaxDemand2, ptr, 77);
					
					while(1)
					{
						i = 0;
						memset(menuitem, 0, sizeof(menuitem));
						switch(iPageNum)
						{
						case 0:
							cp = bPower1+3;
							sprintf(menuitem[i++], "������: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���м�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���з�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "����ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���й�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							break;
						case 1:
							cp = bPower2+3;
							sprintf(menuitem[i++], "������: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���м�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���з�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "����ƽ: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							cp += 5;
							sprintf(menuitem[i++], "���й�: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
							break;
						case 2:
							cp = bMaxDemand1+5;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "��������ƽ: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							break;
						case 3:
							cp = bMaxDemand2+5;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "��������ƽ: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							cp += 15;
							sprintf(menuitem[i++], "����������: %.4fkW", (float)OoDoubleLongUnsignedToDWord(cp)*1e-4);
							break;
						}
						sprintf(menuitem[i++], " ");
						sprintf(menuitem[i++], "����ʱ�� %d/%d/%d %d:%d", OoLongUnsignedToWord(bTime+1)%100, bTime[3], bTime[4], bTime[5], bTime[6]);
						tmpS[i].text = NULL;

						listbox.Show(0, "������", tmpS, KEY_ESC | (KEY_OK << 8) | (KEY_UP << 16) | (KEY_DOWN <<24), 60000, false);
						if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
						{
							goto flag;
						}
						if (listbox.key == KEY_UP)
						{
							iPageNum--;
							if (iPageNum < 0)
							{
								iPageNum = 3;
							}
						}
						else if (listbox.key == KEY_DOWN)
						{
							iPageNum++;
							if (iPageNum > 3)
							{
								iPageNum = 0;
							}
						}
					}	
				}
				else
				{
					MessageBox("����ʱ�����",KEY_ESC,10000);
					goto flag;
				}
			}
		}
		else
		{
			MessageBox("�������Ŵ���",KEY_ESC,10000);
			goto flag;
		}
	}

flag:
	return -1;
}

int SetGprsCommunicationPara(void *arg)
{
	if (!InputPwd())
	{
		return -1;
	}

	char menuitem[26][32];
	memset(menuitem, 0, sizeof(menuitem));
	char portlistitem[6][16];
	memset(portlistitem, 0, sizeof(portlistitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 9 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 10 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 11 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 12 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 13 }, //
		{ menuitem[13], 0xFE, Dummy, (void *) 14 }, //
		{ menuitem[14], 0xFE, Dummy, (void *) 15 }, //
		{ menuitem[15], 0xFE, Dummy, (void *) 16 }, //
		{ menuitem[16], 0xFE, Dummy, (void *) 17 }, //
		{ menuitem[17], 0xFE, Dummy, (void *) 18 }, //
		{ menuitem[18], 0xFE, Dummy, (void *) 19 }, //
		{ menuitem[19], 0xFE, Dummy, (void *) 20 }, //
		{ menuitem[20], 0xFE, Dummy, (void *) 21 }, //
		{ menuitem[21], 0xFE, Dummy, (void *) 22 }, //
		{ menuitem[22], 0xFE, Dummy, (void *) 23 }, //
		{ menuitem[23], 0xFE, Dummy, (void *) 24 }, //
		{ menuitem[24], 0xFE, Dummy, (void *) 25 }, //
		{ menuitem[25], 0xFE, Dummy, (void *) 26 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0, j = 0;
	char szInput[33];
	CListBoxEx listbox;
	char *WorkMode[] = {"���ģʽ", "�ͻ���ģʽ", "������ģʽ"};
	char *ActiveMode[] = {"��������","��������"};
	char *LinkMode[] = {"TCP", "UDP"};
	char *LinkAppMode[] = {"����ģʽ", "������ģʽ"};

	BYTE bGprsBuf[160] = {0};
	BYTE bActiveMode[2] = {0}, bWorkMode[2] = {0}, bLinkMode[2] = {0}, bLinkAppMode[2] = {0};
	BYTE bGprsReSendTime;
	WORD wHeartBeat;
	//TFieldParser tGprsParser = {bGprsBuf};
	TFieldParser tGprsParser;
	memset(&tGprsParser, 0, sizeof(TFieldParser));

	BYTE bType = 0;
	WORD wItemOffSet = 0;
	WORD wItemLen = 0;
	WORD wPortList[5] = {0};
	BYTE bAPN[35] = {0};
	BYTE bUserName[35] = {0};
	BYTE bUserPwd[35] = {0};
	BYTE bBuf[20] = {0};
	BYTE bServAddr[7] = {0};
	//BYTE bTmpBuf[33] = {0};
	WORD wPort = 0;
	BYTE bTimeOut[3] = {0};
	BYTE bResendTime = 0;
	BYTE bMyBuf1[26] = {0};
	BYTE bMyBuf2[10] = {0};
	BYTE bMyBuf3[10] = {0};
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE *pbFmt = NULL;

	char cInput[5] = {0};
	int iModuleNum = 0;
	if(EditSpecBox(2,"������ģ���(1-2):",cInput, 60000, 1,DATA_DEC8)>= 0)//�������߹���ģ���
	{
		sscanf(cInput,"%d",&iModuleNum);
		if(iModuleNum<3 && iModuleNum>0)
		{
			dwOAD = 0x45000200 + (iModuleNum-1)*0x00010000;	//ͨ������
			if ((tGprsParser.wCfgLen=OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf, &pbFmt, &wFmtLen)) <= 0)
			{
				MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
				return -1;
			}
			tGprsParser.pbCfg = bGprsBuf;
			if (OoParseField(&tGprsParser, pbFmt, wFmtLen, false))
			{
				ReadParserField(&tGprsParser, 0x00, bWorkMode, &bType, &wItemOffSet, &wItemLen);   //����ģʽ
				ReadParserField(&tGprsParser, 0x01, bActiveMode, &bType, &wItemOffSet, &wItemLen); //����ģʽ
				ReadParserField(&tGprsParser, 0x02, bLinkMode, &bType, &wItemOffSet, &wItemLen);   //���ӷ�ʽ
				ReadParserField(&tGprsParser, 0x03, bLinkAppMode, &bType, &wItemOffSet, &wItemLen);//����Ӧ�÷�ʽ
				ReadParserField(&tGprsParser, 0x04, bMyBuf1, &bType, &wItemOffSet, &wItemLen);      //�����˿��б�
				wPortList[0] = OoLongUnsignedToWord(bMyBuf1+2+3*0+1);
				wPortList[1] = OoLongUnsignedToWord(bMyBuf1+2+3*1+1);
				wPortList[2] = OoLongUnsignedToWord(bMyBuf1+2+3*2+1);
				wPortList[3] = OoLongUnsignedToWord(bMyBuf1+2+3*3+1);
				wPortList[4] = OoLongUnsignedToWord(bMyBuf1+2+3*4+1);
				ReadParserField(&tGprsParser, 0x05, bAPN, &bType, &wItemOffSet, &wItemLen);         //APN
				ReadParserField(&tGprsParser, 0x06, bUserName, &bType, &wItemOffSet, &wItemLen);    //�û���
				ReadParserField(&tGprsParser, 0x07, bUserPwd, &bType, &wItemOffSet, &wItemLen);     //����
				ReadParserField(&tGprsParser, 0x08, bServAddr, &bType, &wItemOffSet, &wItemLen);    //�����������ַ
				ReadParserField(&tGprsParser, 0x09, bMyBuf2, &bType, &wItemOffSet, &wItemLen);      //����������˿�
				wPort = OoLongUnsignedToWord(bMyBuf2+1);
				ReadParserField(&tGprsParser, 0x0A, bTimeOut, &bType, &wItemOffSet, &wItemLen);    //��ʱʱ��,�ط�����
				ReadParserField(&tGprsParser, 0x0B, bMyBuf3, &bType, &wItemOffSet, &wItemLen);      //��������
				wHeartBeat = OoLongUnsignedToWord(bMyBuf3+1);	
			}	
		}
		else
		{
			MessageBox("����ģ��Ŵ���",KEY_ESC,10000);
			return -1;
		}
	}
	else
	{
		return -1;
	}

	while(1)
	{
		i = 0;

		sprintf(menuitem[i++],"����ģʽ:%s",WorkMode[bWorkMode[1]&0x03]);

		sprintf(menuitem[i++],"����ģʽ:%s",ActiveMode[bActiveMode[1]&0x03]);

		sprintf(menuitem[i++],"���ӷ�ʽ:%s", LinkMode[bLinkMode[1]&0x03]);

		sprintf(menuitem[i++],"����Ӧ�÷�ʽ:%s", LinkAppMode[bLinkAppMode[1]&0x03]);

		sprintf(menuitem[i++], "�����˿��б�:");
		sprintf(menuitem[i++], "1. %d",  wPortList[0]);
		sprintf(menuitem[i++], "2. %d",  wPortList[1]);
		sprintf(menuitem[i++], "3. %d",  wPortList[2]);
		sprintf(menuitem[i++], "4. %d",  wPortList[3]);
		sprintf(menuitem[i++], "5. %d",  wPortList[4]);

		memset(bBuf, 0, sizeof(bBuf));
		sprintf(menuitem[i++], "APN:");
		memcpy(bBuf, &bAPN[2], 16);
		sprintf(menuitem[i++],"%s", (char *)bBuf);
		memcpy(bBuf, &bAPN[18], 16);
		if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
			sprintf(menuitem[i++], " "); //����û���
		else
			sprintf(menuitem[i++],"%s",(char *)bBuf);

		memset(bBuf, 0, sizeof(bBuf));
		sprintf(menuitem[i++],"�û���:");
		memcpy(bBuf, &bUserName[2], 16);
		sprintf(menuitem[i++],"%s",(char *)bBuf);
		memcpy(bBuf, &bUserName[18], 16);
		if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
			sprintf(menuitem[i++], " "); //����û���
		else
			sprintf(menuitem[i++],"%s",(char *)bBuf);

		memset(bBuf, 0, sizeof(bBuf));
		sprintf(menuitem[i++],"����:");
		memcpy(bBuf, &bUserPwd[2], 16);
		sprintf(menuitem[i++],"%s",(char *)bBuf);
		memcpy(bBuf, &bUserPwd[18], 16);
		if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
			sprintf(menuitem[i++], " "); //����û���
		else
			sprintf(menuitem[i++],"%s",(char *)bBuf);

		memset(bBuf, 0, sizeof(bBuf));
		
		sprintf(menuitem[i++],"�����������ַ:");
		sprintf(menuitem[i++], "%d.%d.%d.%d", bServAddr[2], bServAddr[3], bServAddr[4], bServAddr[5]);

		sprintf(menuitem[i++],"����������˿�:%d", wPort);

		sprintf(menuitem[i++],"��ʱʱ��:%d", bTimeOut[1]>>2);

		bResendTime = bTimeOut[1]&0x03;
		sprintf(menuitem[i++],"�ط�����:%d", bResendTime);

		sprintf(menuitem[i++],"��������:%d", wHeartBeat);

		sprintf(menuitem[i++], "��������");
		tmpS[i].text = NULL;

		listbox.Show(0,"���߹���ͨ������",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item == 0)//����ģʽ
			{
				CListBoxEx listboxTmp0;
				struct ListBoxExItem tmp0[] = { 
					{ WorkMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ WorkMode[1], 0xFF, Dummy, (void *) 0x01 },//
					{ WorkMode[2], 0xFF, Dummy, (void *) 0x02 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp0.Show(0, "����ģʽ", tmp0, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp0.key == KEY_OK)
				{
					bWorkMode[1] = (BYTE)((int)tmp0[listboxTmp0.item].arg);
				}
			}
			else if (listbox.item == 1)//����ģʽ������Ϊ����
			{
				CListBoxEx listboxTmp1;
				struct ListBoxExItem tmp1[] = { 
					{ ActiveMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ ActiveMode[1], 0xFF, Dummy, (void *) 0x01 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp1.Show(0, "����ģʽ", tmp1, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp1.key == KEY_OK)
				{
					bActiveMode[1] = (BYTE)((int)tmp1[listboxTmp1.item].arg);
				}

			}
			else if (listbox.item == 2)//���ӷ�ʽ������Ϊ����
			{
				CListBoxEx listboxTmp2;
				struct ListBoxExItem tmp2[] = { 
					{ LinkMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ LinkMode[1], 0xFF, Dummy, (void *) 0x01 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp2.Show(0, "���ӷ�ʽ", tmp2, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp2.key == KEY_OK)
				{
					bLinkMode[1] = (BYTE)((int)tmp2[listboxTmp2.item].arg);
				}	
			}
			else if (listbox.item == 3)//����Ӧ�÷�ʽ������Ϊ����
			{
				CListBoxEx listboxTmp3;
				struct ListBoxExItem tmp3[] = { 
					{ LinkAppMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ LinkAppMode[1], 0xFF, Dummy, (void *) 0x01 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp3.Show(0, "����Ӧ�÷�ʽ", tmp3, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp3.key == KEY_OK)
				{
					bLinkAppMode[1] = (BYTE)((int)tmp3[listboxTmp3.item].arg);
				}
			}
			else if (listbox.item >= 5 && listbox.item <= 9)//�����˿��б�����Ϊ����
			{
				CListBoxEx listboxTmp5;
				struct ListBoxExItem tmp5[] = { 
					{ portlistitem[0], 0xFF, Dummy, (void *) 0x00 },//
					{ portlistitem[1], 0xFF, Dummy, (void *) 0x01 },//
					{ portlistitem[2], 0xFF, Dummy, (void *) 0x02 },//
					{ portlistitem[3], 0xFF, Dummy, (void *) 0x03 },//
					{ portlistitem[4], 0xFF, Dummy, (void *) 0x04 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				while(1)
				{
					sprintf(portlistitem[0], "�˿�һ:%d", wPortList[0]);
					sprintf(portlistitem[1], "�˿ڶ�:%d", wPortList[1]);
					sprintf(portlistitem[2], "�˿���:%d", wPortList[2]);
					sprintf(portlistitem[3], "�˿���:%d", wPortList[3]);
					sprintf(portlistitem[4], "�˿���:%d", wPortList[4]);

					listboxTmp5.Show(0, "�����˿��б�", tmp5, KEY_ESC | (KEY_OK << 8), 60000);
					if (listboxTmp5.key == KEY_ESC || listboxTmp5.key == KEY_NULL)
					{
						break;
					}
					if (listboxTmp5.key == KEY_OK)
					{
						BYTE bcurItem = listboxTmp5.item;
						memset(szInput, 0, sizeof(szInput));
						if (EditTextBox(2, "�����˿ں�",szInput,60000,5,DATA_DEC)>=0)
						{
							int iPort = 0;
							WORD wTmpPort = 0;
							sscanf(szInput, "%d", &iPort);
							wTmpPort = iPort;
							wPortList[bcurItem] = wTmpPort;
						}
					}
				}
			}
			else if (listbox.item==11 || listbox.item==12)//APN������Ϊ�䳤
			{
				char *pTail = NULL;
				if (listbox.item==11)
				{
					memset(szInput, 0, sizeof(szInput));
					memcpy(szInput, &bAPN[2], 16);
					if (getSoftKey("APN", szInput, 60000, 16, DATA_ASC)>=0)
					{
						memset(&bAPN[2], 0, sizeof(bAPN)-2);
						memcpy(&bAPN[2], szInput, 16);
					}
					pTail = strstr((char *)(bAPN+2), " ");
					if (pTail)
					{
						memset(pTail, 0x00, &bAPN[17]-(BYTE *)pTail+1);
						memset(&bAPN[17], 0, 16);
					}
				}
				else if (listbox.item==12)
				{
					memset(szInput, 0, sizeof(szInput));
					memcpy(szInput, &bAPN[18], 16);
					if (bAPN[17] != 0)
					{
						if(getSoftKey("APN", szInput, 60000, 16, DATA_ASC) >= 0)
						{
							memcpy(&bAPN[18], szInput, 16);
						}
						pTail = strstr((char*)&bAPN[18]," ");
						if(pTail)
						{
							memset(pTail, 0x00, &bAPN[33]-(BYTE*)pTail+1);
						}
					}
				}
				//pTail = strstr((char*)&bAPN[2], " ");
				//tGprsParser.bLen[5] = pTail - (char*)&bAPN[2];
				bAPN[1] = strlen((char*)&bAPN[2]);          //�������ݳ���
				tGprsParser.wLen[5] = strlen((char*)bAPN);  //������ʽ���Ⱥ���������
				if (tGprsParser.wLen[5] < 2)
					tGprsParser.wLen[5] = 2;
			}
			else if (listbox.item==14 || listbox.item==15)//�û���������Ϊ�䳤
			{
				char *pTail = NULL; 
				if (listbox.item == 14)
				{
					memset(szInput, 0, sizeof(szInput));
					memcpy(szInput, &bUserName[2], 16);
					if(getSoftKey("�û���", szInput, 60000, 16, DATA_ASC) >= 0)
					{
						memcpy(&bUserName[2], szInput, 16);
					}
					pTail = strstr((char*)&bUserName[2]," ");
					if(pTail)
					{
						memset(pTail, 0x00, &bUserName[17]-(BYTE*)pTail+1);
						memset(&bUserName[18], 0, 16);
					}
				}
				else if (listbox.item == 15)
				{
					memset(szInput, 0, sizeof(szInput));
					memcpy(szInput, &bUserName[18], 16);
					if (bUserName[17] != 0)
					{
						if(getSoftKey("�û���", szInput, 60000, 16, DATA_ASC) >= 0)
						{
							memcpy(&bUserName[18], szInput, 16);
						}
						pTail = strstr((char*)&bUserName[18]," ");
						if(pTail)
						{
							memset(pTail, 0x00, &bUserName[33]-(BYTE*)pTail+1);
						}
					}
				}
				//pTail = strstr((char*)&bUserName[2], " ");
				//tGprsParser.bLen[6] = pTail - (char*)&bUserName[2];
				bUserName[1] =  strlen((char*)&bUserName[2]);   //�������ݳ���
				tGprsParser.wLen[6] = strlen((char*)bUserName); //������ʽ���Ⱥ���������
				if (tGprsParser.wLen[6] < 2)
					tGprsParser.wLen[6] = 2;
			}
			else if (listbox.item==17 || listbox.item==18)//���룬����Ϊ�䳤
			{
				char *pTail = NULL;
				if (listbox.item == 17)
				{
					memset(szInput,0,sizeof(szInput));
					memcpy(szInput, &bUserPwd[2], 16);
					if(getSoftKey("����", szInput, 60000, 16, DATA_ASC) >= 0)
					{
						memcpy(&bUserPwd[2],szInput,16);
					}
					pTail = strstr((char*)&bUserPwd[2]," ");
					if(pTail)
					{
						memset(pTail, 0x00, &bUserPwd[17]-(BYTE*)pTail+1);
						memset(&bUserPwd[18], 0, 16);
					}
				}
				else if (listbox.item == 18)
				{
					memset(szInput,0,sizeof(szInput));
					memcpy(szInput, &bUserPwd[18], 16);
					if (bUserPwd[17] != 0)
					{
						if(getSoftKey("����", szInput, 60000, 16, DATA_ASC) >= 0)
						{
							memcpy(&bUserPwd[18],szInput,16);
						}
						pTail = strstr((char*)&bUserPwd[18]," ");
						if(pTail)
						{
							memset(pTail, 0x00, &bUserPwd[33]-(BYTE*)pTail+1);
						}
					}
				}
				//pTail = strstr((char*)&bUserPwd[2], " ");
				//tGprsParser.bLen[7] = pTail - (char*)&bUserPwd[2];
				bUserPwd[1] = strlen((char*)&bUserPwd[2]);     //�������ݳ���
				tGprsParser.wLen[7] = strlen((char*)bUserPwd); //������ʽ���Ⱥ���������
				if (tGprsParser.wLen[7] < 2)
					tGprsParser.wLen[7] = 2;
			}
			else if (listbox.item==20)//�����������ַ������Ϊ����
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput,"%03d.%03d.%03d.%03d", bServAddr[2], bServAddr[3], bServAddr[4], bServAddr[5]);	 //spintf()���Զ���szInput��ĩβ����\0������szInputҪ�࿪1�ֽڣ���ֹ���

				if(EditTextBox(2,"���÷�������ַ",szInput,60000,15,DATA_DEC)>=0)
				{
					if (szInput[3]=='.' && szInput[7]=='.' && szInput[11]=='.')
					{
						int iAddr1,iAddr2,iAddr3,iAddr4;
						if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
						{
							if (iAddr1<=255 && iAddr2<=255 && iAddr3<=255 && iAddr4<=255)
							{
								bServAddr[2] = iAddr1;
								bServAddr[3] = iAddr2;
								bServAddr[4] = iAddr3;
								bServAddr[5] = iAddr4;
							}
						}	
					}
					else
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
				}
			}
			else if (listbox.item == 21)//����������˿ڣ�����Ϊ����
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%d", wPort);
				if(EditTextBox(2,"���÷������˿�",szInput,60000,5,DATA_DEC)>=0)
				{
					wPort = (WORD)atoi(szInput);
				}
			}
			else if (listbox.item == 22)//��ʱʱ�䣬����Ϊ����
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%d", bTimeOut[1]-(bTimeOut[1]&0x03));
				if (EditTextBox(2, "��ʱʱ��(0~63)", szInput, 60000, 2, DATA_DEC) >= 0)
				{
					BYTE bTimeOutSec = (WORD)atoi(szInput);
					if (bTimeOutSec >= 64)
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
					else
					{
						bTimeOut[1] &= 0x03;
						bTimeOut[1] |= (bTimeOutSec<<2);
					}
				}
			}
			else if (listbox.item == 23)//�ط�����������Ϊ����
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%d", bTimeOut[1]&0x03);
				if (EditTextBox(2, "�ط�����(0~3)", szInput, 60000, 1, DATA_DEC) >= 0)
				{
					BYTE bMyResendTime = (BYTE)atoi(szInput);
					if (bMyResendTime > 3)
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
					else
					{
						bTimeOut[1] &= 0xfc;
						bTimeOut[1] |= bMyResendTime;
					}
				}
			}
			else if (listbox.item == 24)//�������ڣ�����Ϊ����
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%d", wHeartBeat);
				if (EditTextBox(2, "��������(0~65535)", szInput, 60000, 5, DATA_DEC) >= 0)
				{
					int tmp = 0;
					sscanf(szInput, "%d", &tmp);
					if (tmp > 65535)
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
					else
					{
						wHeartBeat = tmp;
					}
				}
			}
			else if (listbox.item == 25)//��������
			{
				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					/*
					**�ӵ�һ���䳤�ֶ�(APN)�󣬺����ֶε�ƫ�ƶ�Ҫ���¼���
					*/
					tGprsParser.wPos[6] =  tGprsParser.wPos[5] + tGprsParser.wLen[5]; //�û���
					tGprsParser.wPos[7] = tGprsParser.wPos[6] + tGprsParser.wLen[6];  //����
					tGprsParser.wPos[8] = tGprsParser.wPos[7] + tGprsParser.wLen[7];  //�����������ַ
					tGprsParser.wPos[9] = tGprsParser.wPos[8] + tGprsParser.wLen[8];  //����˿�
					tGprsParser.wPos[10] = tGprsParser.wPos[9] + tGprsParser.wLen[9]; //��ʱʱ�估�ط�����
					tGprsParser.wPos[11] = tGprsParser.wPos[10] + tGprsParser.wLen[10];//��������

					OoWordToLongUnsigned(wPortList[0], bMyBuf1+2+3*0+1);//�����˿�1
					OoWordToLongUnsigned(wPortList[1], bMyBuf1+2+3*1+1);//�����˿�2
					OoWordToLongUnsigned(wPortList[2], bMyBuf1+2+3*2+1);//�����˿�3
					OoWordToLongUnsigned(wPortList[3], bMyBuf1+2+3*3+1);//�����˿�4
					OoWordToLongUnsigned(wPortList[4], bMyBuf1+2+3*4+1);//�����˿�5

					OoWordToLongUnsigned(wPort, bMyBuf2+1);//����������˿�
					OoWordToLongUnsigned(wHeartBeat, bMyBuf3+1);//��������
					memcpy(bGprsBuf+tGprsParser.wPos[0], bWorkMode, tGprsParser.wLen[0]);
					memcpy(bGprsBuf+tGprsParser.wPos[1], bActiveMode, tGprsParser.wLen[1]);
					memcpy(bGprsBuf+tGprsParser.wPos[2], bLinkMode, tGprsParser.wLen[2]);
					memcpy(bGprsBuf+tGprsParser.wPos[3], bLinkAppMode, tGprsParser.wLen[3]);
					memcpy(bGprsBuf+tGprsParser.wPos[4], bMyBuf1, tGprsParser.wLen[4]);
					memcpy(bGprsBuf+tGprsParser.wPos[5], bAPN, tGprsParser.wLen[5]);
					memcpy(bGprsBuf+tGprsParser.wPos[6], bUserName, tGprsParser.wLen[6]);
					memcpy(bGprsBuf+tGprsParser.wPos[7], bUserPwd, tGprsParser.wLen[7]);
					memcpy(bGprsBuf+tGprsParser.wPos[8], bServAddr, tGprsParser.wLen[8]);
					memcpy(bGprsBuf+tGprsParser.wPos[9], bMyBuf2, tGprsParser.wLen[9]);
					memcpy(bGprsBuf+tGprsParser.wPos[10], bTimeOut, tGprsParser.wLen[10]);
					memcpy(bGprsBuf+tGprsParser.wPos[11], bMyBuf3, tGprsParser.wLen[11]);
				
					//if (WriteItemEx(BANK0, iGprsModuleNum-1, 0x4500, bGprsBuf) >= 0)
					if (OoWriteAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf) >= 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT4, -1);
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}
				
			}
		}
		
	}

	return -1;
}

/*
int SetGprsMessagePara(void *arg)
{
	//if (!InputPwd())
	//{
	//	return -1;
	//}
	char menuitem[15][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 9 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 10 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 11 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 12 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 13 }, //
		{ menuitem[13], 0xFE, Dummy, (void *) 14 }, //
		{ menuitem[14], 0xFE, Dummy, (void *) 15 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0, j = 0;
	//char szInput[33];
	CListBoxEx listbox;
	
	BYTE bGprsBuf[120] = {0};
	BYTE bStaionSmsBuf[MAX_SMS_MAIN_NUM][10] = {0};
	BYTE bInformSmsBuf[MAX_SMS_MAIN_NUM][10] = {0};
	TFieldParser tGprsParser = {bGprsBuf};
	TFieldParser tStationParser = {bStaionSmsBuf[0]};
	TFieldParser tInformParser = {bInformSmsBuf[0]};
	BYTE bType = 0;
	WORD wItemOffSet = 0;
	WORD wItemLen = 0;
	char cInput[5] = {0};
	int iModuleNum = 0;
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE bFmt[20] = {0};
	BYTE *pbFmt = bFmt;
	char szPhone[32];
	BYTE bCenterSms[10] = {0};                    //�������ĺ��� visible-string(SIZE(16))
	BYTE bStationSms[2+MAX_SMS_MAIN_NUM*10] = {0};  //��վ����  array visible-string(SIZE(16))
	BYTE bInformSms[2+MAX_SMS_SEND_NUM*10] = {0};   //����֪ͨĿ�ĺ��� array visible-string(SIZE(16))
	
	BYTE bSmsFmt[] = {DT_ARRAY, MAX_SMS_MAIN_NUM,
							 DT_VIS_STR, 8, RLV};
	WORD wSmsFmtLen = sizeof(bSmsFmt);
	WORD wLen = 0;

	if(EditSpecBox(2,"������ģ���(1-2):",cInput, 60000, 1,DATA_DEC8)>= 0)//�������߹���ģ���
	{
		sscanf(cInput,"%d",&iModuleNum);
		if(iModuleNum<3 && iModuleNum>0)
		{
			dwOAD = 0x45000400 + (iModuleNum-1)*0x00010000;	//����ͨ�Ų���
			if ((tGprsParser.wCfgLen=OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf, &pbFmt, &wFmtLen)) <= 0)
			{
				MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
				return -1;
			}
			tGprsParser.pbCfg = bGprsBuf;
			if (OoParseField(&tGprsParser, pbFmt, wFmtLen, false))
			{
				ReadParserField(&tGprsParser, 0x00, bCenterSms, &bType, &wItemOffSet, &wItemLen);   //�������ĺ���
				ReadParserField(&tGprsParser, 0x01, bStationSms, &bType, &wItemOffSet, &wItemLen);  //��վ����
				ReadParserField(&tGprsParser, 0x02, bInformSms, &bType, &wItemOffSet, &wItemLen);  //����֪ͨĿ�ĺ���
				tStationParser.pbCfg = bGprsBuf+tGprsParser.wPos[1];
				tInformParser.pbCfg = bGprsBuf+tGprsParser.wPos[2];
				tStationParser.wCfgLen = tGprsParser.bLen[1];
				tInformParser.wCfgLen = tGprsParser.bLen[2];

				if (OoParseField(&tStationParser, bSmsFmt, wSmsFmtLen, false) && OoParseField(&tInformParser, bSmsFmt, wSmsFmtLen, false))
				{
					for (i=0; i<MAX_SMS_SEND_NUM; i++)
					{
						ReadParserField(&tStationParser, i, &bStaionSmsBuf[i][0], &bType, &wItemOffSet, &wItemLen);  //��վ����
						ReadParserField(&tInformParser, i, &bInformSmsBuf[i][0], &bType, &wItemOffSet, &wItemLen);  //����֪ͨĿ�ĺ���
					}	
				}
				
			}	
		}
		else
		{
			MessageBox("����ģ��Ŵ���",KEY_ESC,10000);
			return -1;
		}
	}
	else
	{
		return -1;
	}

	while(1)
	{
		i = 0;
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(bCenterSms+2, 8, szPhone);
		sprintf(menuitem[i++],"�������ĺ���:");
		sprintf(menuitem[i++],"%s", szPhone);

		memset(szPhone, 0, sizeof(szPhone));
		sprintf(menuitem[i++],"��վ����:");
		PhoneToStr(&bStaionSmsBuf[0][2], 8, szPhone);
		sprintf(menuitem[i++],"��վ1��%s", szPhone);
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bStaionSmsBuf[1][2], 8, szPhone);
		sprintf(menuitem[i++],"��վ2��%s", szPhone);
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bStaionSmsBuf[2][2], 8, szPhone);
		sprintf(menuitem[i++],"��վ3��%s", szPhone);
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bStaionSmsBuf[3][2], 8, szPhone);
		sprintf(menuitem[i++],"��վ4��%s", szPhone);
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bStaionSmsBuf[4][2], 8, szPhone);
		sprintf(menuitem[i++],"��վ5��%s", szPhone);

		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bInformSmsBuf[0][2], 8, szPhone);
		sprintf(menuitem[i++],"����֪ͨĿ�ĺ���:");
		sprintf(menuitem[i++],"Ŀ��1:%s", szPhone);
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bInformSmsBuf[1][2], 8, szPhone);
		sprintf(menuitem[i++],"Ŀ��2:%s", szPhone);
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bInformSmsBuf[2][2], 8, szPhone);
		sprintf(menuitem[i++],"Ŀ��3:%s", szPhone);
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bInformSmsBuf[3][2], 8, szPhone);
		sprintf(menuitem[i++],"Ŀ��4:%s", szPhone);
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(&bInformSmsBuf[4][2], 8, szPhone);
		sprintf(menuitem[i++],"Ŀ��5:%s", szPhone);

		sprintf(menuitem[i++],"��������");
		tmpS[i].text = NULL;

		listbox.Show(0,"���߹�������ͨ�Ų���",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			char *pszPhone = NULL; 
			char szInput[20];
			BYTE bOffSet = 0;

			memset(szInput, 0x00, sizeof(szInput));
			if (listbox.item == 1)
			{
				memset(szPhone, 0, sizeof(szPhone));
				PhoneToStr(bCenterSms+2, 8, szPhone);
				pszPhone = szPhone;
				strcpy(szInput,pszPhone);
				if(getSoftKey("�������ĺ���",szInput,60000,16,DATA_ASC)>=0)
				{
					for(j=0; j<16; j++)
					{
						if (szInput[j] < '0' || szInput[j] > '9')
							szInput[j] = 'f';	//��������f												
					}

					memcpy(pszPhone,szInput,16);
					AsciiToByte((BYTE*)pszPhone, 16, bCenterSms+2);
					tGprsParser.bLen[0] = strlen(pszPhone)/2 + 2; //�������ĺ����ֶ��ܳ���
					bCenterSms[1] = strlen(pszPhone)/2;           //�������ĺ����ֶ��������ݳ���
					bCenterSms[0] = DT_VIS_STR;
				}
			}
			else if (listbox.item==3 || listbox.item==4 || listbox.item==5 || listbox.item==6 || listbox.item==7)
			{
				bOffSet = listbox.item - 3;
				memset(szPhone, 0, sizeof(szPhone));
				PhoneToStr(&bStaionSmsBuf[bOffSet][2], 8, szPhone);
				pszPhone = szPhone;
				strcpy(szInput,pszPhone);
				if (getSoftKey("�������ĺ���", szInput, 60000, 16, DATA_ASC)>=0)
				{
					for(j=0; j<16; j++)
					{
						if (szInput[j] < '0' || szInput[j] > '9')
							szInput[j] = 'f';	//��������f												
					}

					memcpy(pszPhone,szInput,16);
					AsciiToByte((BYTE*)pszPhone, 16, &bStaionSmsBuf[bOffSet][2]);
					tStationParser.bLen[bOffSet] = strlen(pszPhone)/2 + 2;
					bStaionSmsBuf[bOffSet][1] = strlen(pszPhone)/2;
					bStaionSmsBuf[bOffSet][0] = DT_VIS_STR;
				}
			}
			else if (listbox.item==9 || listbox.item==10 || listbox.item==11 || listbox.item==12 || listbox.item==13)
			{
				bOffSet = listbox.item - 9;
				memset(szPhone, 0, sizeof(szPhone));
				PhoneToStr(&bInformSmsBuf[bOffSet][2], 8, szPhone);
				pszPhone = szPhone;
				strcpy(szInput,pszPhone);
				if (getSoftKey("����֪ͨĿ�ĺ���", szInput, 60000, 16, DATA_ASC)>=0)
				{
					for(j=0; j<16; j++)
					{
						if (szInput[j] < '0' || szInput[j] > '9')
							szInput[j] = 'f';	//��������f												
					}

					memcpy(pszPhone,szInput,16);
					AsciiToByte((BYTE*)pszPhone, 16, &bInformSmsBuf[bOffSet][2]);
					tInformParser.bLen[bOffSet] = strlen(pszPhone)/2 + 2;   //�޸��ֶγ���
					bInformSmsBuf[bOffSet][1] = strlen(pszPhone)/2; //�޸��������ݳ���
					bInformSmsBuf[bOffSet][0] = DT_VIS_STR;
				}
			}
			
			else if (listbox.item==14)
			{
				BYTE bTmpLen1 = 0; 
				BYTE bTmpLen2 = 0;

				if(MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					BYTE *ptr1 = bStationSms+2;
					BYTE *ptr2 = bInformSms+2;
					for (i=0; i<MAX_SMS_SEND_NUM; i++)
					{
						memcpy(ptr1, &bStaionSmsBuf[i][0], tStationParser.bLen[i]);
						memcpy(ptr2, &bInformSmsBuf[i][0], tInformParser.bLen[i]);
						ptr1 += tStationParser.bLen[i];
						ptr2 += tInformParser.bLen[i];
						
							bTmpLen1 += tStationParser.bLen[i]; 
							bTmpLen2 += tInformParser.bLen[i];
						}
					tGprsParser.bLen[1] = 2 + bTmpLen1; //��վ�����ֶε��ܳ���
					tGprsParser.bLen[2] = 2 + bTmpLen2; //����֪ͨĿ�ĺ�����ܳ���

					if (OoWriteField(bGprsBuf, tGprsParser.wCfgLen, pbFmt, wFmtLen, 0x00, bCenterSms, tGprsParser.bLen[0])>=0 &&
						OoWriteField(bGprsBuf, tGprsParser.wCfgLen, pbFmt, wFmtLen, 0x01, bStationSms, tGprsParser.bLen[1])>=0 &&
						OoWriteField(bGprsBuf, tGprsParser.wCfgLen, pbFmt, wFmtLen, 0x02, bInformSms, tGprsParser.bLen[2])>=0)
					{
						if (OoWriteAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf) >= 0)
						{
							MessageBox("���óɹ�",KEY_ESC,3000);
							TrigerSaveBank(BANK0, SECT4, -1);
							DoTrigerSaveBank();
						}
						else
						{
							goto flag;
						}
					}
					else
					{
flag:
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}
				
			}
		}
		
	}

	return -1;
}*/

int SetGprsMessagePara(void *arg)
{
	char menuitem[7][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0, j = 0;
	CListBoxEx listbox;

	BYTE bGprsBuf[40] = {0};
	BYTE bCenterSms[10] = {0};
	BYTE bStaionSms[10] = {0};
	BYTE bInformSms[10] = {0};

	char cInput[5] = {0};
	int iModuleNum = 0;
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE *pbFmt = NULL;
	char szPhone[32];
	BYTE *pbPos1 = NULL;
	BYTE *pbPos2 = NULL;
	BYTE *pbPos3 = NULL;
	BYTE bLen1, bLen2, bLen3;


	if(EditSpecBox(2,"������ģ���(1-2):",cInput, 60000, 1,DATA_DEC8)>= 0)//�������߹���ģ���
	{
		sscanf(cInput,"%d",&iModuleNum);
		if(iModuleNum<3 && iModuleNum>0)
		{
			dwOAD = 0x45000400 + (iModuleNum-1)*0x00010000;	//����ͨ�Ų���
			if (0 > OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf, &pbFmt, &wFmtLen))
			{
				MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
				return -1;
			}
			pbPos1 = bGprsBuf + 2;      //����1����λ��
			bLen1 = *(pbPos1+1);        //����1���ݳ��ȣ�������ʽ
			pbPos2 = pbPos1 + 2 + bLen1;//����2����λ��
			bLen2 = *(pbPos2+3);		//����2���ݳ��ȣ�������ʽ
			pbPos3 = pbPos2 + 2 + 2 + bLen2;//����3����λ��
			bLen3 = *(pbPos3+3);		//����3���ݳ��ȣ�������ʽ
			
			memcpy(bCenterSms, pbPos1, bLen1+2);
			memcpy(bStaionSms, pbPos2+2, bLen2+2);
			memcpy(bInformSms, pbPos3+2, bLen3+2);
		}
		else
		{
			MessageBox("����ģ��Ŵ���",KEY_ESC,10000);
			return -1;
		}
	}
	else
	{
		return -1;
	}

	while(1)
	{
		i = 0;
		
		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(bCenterSms+2, 8, szPhone);
		sprintf(menuitem[i++],"�������ĺ���:");
		sprintf(menuitem[i++],"%s", szPhone);

		memset(szPhone, 0, sizeof(szPhone));
		sprintf(menuitem[i++],"��վ����:");
		PhoneToStr(bStaionSms+2, 8, szPhone);
		sprintf(menuitem[i++],"%s", szPhone);

		memset(szPhone, 0, sizeof(szPhone));
		PhoneToStr(bInformSms+2, 8, szPhone);
		sprintf(menuitem[i++],"����֪ͨĿ�ĺ���:");
		sprintf(menuitem[i++],"%s", szPhone);
		
		sprintf(menuitem[i++],"��������");
		tmpS[i].text = NULL;

		listbox.Show(0,"���߹�������ͨ�Ų���",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			char *pszPhone = NULL; 
			char szInput[20];
			BYTE bOffSet = 0;

			memset(szInput, 0x00, sizeof(szInput));
			if (listbox.item == 1)
			{
				memset(szPhone, 0, sizeof(szPhone));
				PhoneToStr(bCenterSms+2, 8, szPhone);
				pszPhone = szPhone;
				strcpy(szInput,pszPhone);
				if(getSoftKey("�������ĺ���",szInput,60000,16,DATA_ASC)>=0)
				{
					for(j=0; j<16; j++)
					{
						if (szInput[j] < '0' || szInput[j] > '9')
							szInput[j] = 'f';	//��������f												
					}

					memcpy(pszPhone,szInput,16);
					AsciiToByte((BYTE*)pszPhone, 16, bCenterSms+2);
					bLen1 = strlen(pszPhone)/2;           //�������ĺ����ֶ��������ݳ���
					bCenterSms[1] = strlen(pszPhone)/2;      //�������ĺ����ֶ��������ݳ���
					bCenterSms[0] = DT_VIS_STR;
				}
			}
			else if (listbox.item == 3)
			{
				memset(szPhone, 0, sizeof(szPhone));
				PhoneToStr(bStaionSms+2, 8, szPhone);
				pszPhone = szPhone;
				strcpy(szInput,pszPhone);
				if (getSoftKey("�������ĺ���", szInput, 60000, 16, DATA_ASC)>=0)
				{
					for(j=0; j<16; j++)
					{
						if (szInput[j] < '0' || szInput[j] > '9')
							szInput[j] = 'f';	//��������f												
					}

					memcpy(pszPhone,szInput,16);
					AsciiToByte((BYTE*)pszPhone, 16, bStaionSms+2);
					bLen2 = strlen(pszPhone)/2;
					bStaionSms[1] = strlen(pszPhone)/2;
					bStaionSms[0] = DT_VIS_STR;
				}
			}
			else if (listbox.item == 5)
			{
				memset(szPhone, 0, sizeof(szPhone));
				PhoneToStr(bInformSms+2, 8, szPhone);
				pszPhone = szPhone;
				strcpy(szInput,pszPhone);
				if (getSoftKey("����֪ͨĿ�ĺ���", szInput, 60000, 16, DATA_ASC)>=0)
				{
					for(j=0; j<16; j++)
					{
						if (szInput[j] < '0' || szInput[j] > '9')
							szInput[j] = 'f';	//��������f												
					}

					memcpy(pszPhone,szInput,16);
					AsciiToByte((BYTE*)pszPhone, 16, bInformSms+2);
					bLen3 = strlen(pszPhone)/2;   //�޸��ֶγ���
					bInformSms[1] = strlen(pszPhone)/2; //�޸��������ݳ���
					bInformSms[0] = DT_VIS_STR;
				}
			}
			else if (listbox.item == 6)
			{
				pbPos1 = bGprsBuf + 2;      //����1����λ��
				//bLen1 = *(pbPos1+1);        //����1���ݳ��ȣ�������ʽ
				pbPos2 = pbPos1 + 2 + bLen1;//����2����λ��
				//bLen2 = *(pbPos2+1);		//����2���ݳ��ȣ�������ʽ
				pbPos3 = pbPos2 + 2 + 2 + bLen2;//����3����λ��
				//bLen3 = *(pbPos3+1);		//����3���ݳ��ȣ�������ʽ
				memcpy(pbPos1, bCenterSms, bLen1+2);
				memcpy(pbPos2+2, bStaionSms, bLen2+2);
				memcpy(pbPos3+2, bInformSms, bLen3+2);
				pbPos2[0] = DT_ARRAY;
				pbPos2[1] = 1;
				pbPos3[0] = DT_ARRAY;
				pbPos3[1] = 1;

				if (OoWriteAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf) >= 0)
				{
					MessageBox("���óɹ�",KEY_ESC,3000);
					TrigerSaveBank(BANK0, SECT4, -1);
					DoTrigerSaveBank();
				}
				else
				{
					MessageBox("����ʧ��",KEY_ESC,2000);
				}
			}
		}
	}
	
	return -1;
}

int SetGprsMasterPara(void *arg)
{
	if (!InputPwd())
	{
		return -1;
	}

	char menuitem[5][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0;
	char szInput[20];
	CListBoxEx listbox;
	
	BYTE bGprsBuf[24] = {0};
	TFieldParser tGprsParser = {bGprsBuf};
	BYTE bType = 0;
	WORD wItemOffSet = 0;
	WORD wItemLen = 0;
	char cInput[5] = {0};
	int iModuleNum = 0;
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE *pbFmt;
	typedef struct{
		BYTE bStructType;
		BYTE bFiledNum;
		BYTE bStringType;
		BYTE bLen;
		BYTE bIP[4];
		BYTE bLongUnsighedType;
		BYTE bPort[2];
	}TMasterPara; 
	TMasterPara tMaster1;
	TMasterPara tMaster2;
	memset(&tMaster1, 0, sizeof(TMasterPara));
	memset(&tMaster2, 0, sizeof(TMasterPara));


	if(EditSpecBox(2,"������ģ���(1-2):",cInput, 60000, 1,DATA_DEC8)>= 0)//�������߹���ģ���
	{
		sscanf(cInput,"%d",&iModuleNum);
		if(iModuleNum<3 && iModuleNum>0)
		{
			dwOAD = 0x45000300 + (iModuleNum-1)*0x00010000;	//��վͨ�Ų���
			if ((tGprsParser.wCfgLen=OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf, &pbFmt, &wFmtLen)) <= 0)
			{
				MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
				return -1;
			}
			tGprsParser.pbCfg = bGprsBuf;
			if (OoParseField(&tGprsParser, pbFmt, wFmtLen, false))
			{
				ReadParserField(&tGprsParser, 0x00, (BYTE *)&tMaster1, &bType, &wItemOffSet, &wItemLen);   //��վ1
				ReadParserField(&tGprsParser, 0x01, (BYTE *)&tMaster2, &bType, &wItemOffSet, &wItemLen);  //��վ2
			}	
		}
		else
		{
			MessageBox("����ģ��Ŵ���",KEY_ESC,10000);
			return -1;
		}
	}
	else
	{
		return -1;
	}

	while(1)
	{
		i = 0;
		sprintf(menuitem[i++],"��վ1IP:%d.%d.%d.%d", tMaster1.bIP[0], tMaster1.bIP[1], tMaster1.bIP[2], tMaster1.bIP[3]);
		sprintf(menuitem[i++],"��վ1�˿�:%d", OoLongUnsignedToWord(tMaster1.bPort));
		sprintf(menuitem[i++],"��վ2IP:%d.%d.%d.%d", tMaster2.bIP[0], tMaster2.bIP[1], tMaster2.bIP[2], tMaster2.bIP[3]);
		sprintf(menuitem[i++],"��վ2�˿�:%d", OoLongUnsignedToWord(tMaster2.bPort));
		sprintf(menuitem[i++],"��������");
		tmpS[i].text = NULL;

		listbox.Show(0,"��վͨ�Ų�����",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item==0 || listbox.item==2)
			{
				memset(szInput, 0, sizeof(szInput));
				if (listbox.item==0)
					sprintf(szInput,"%03d.%03d.%03d.%03d", tMaster1.bIP[0], tMaster1.bIP[1], tMaster1.bIP[2], tMaster1.bIP[3]);	 //spintf()���Զ���szInput��ĩβ����\0������szInputҪ�࿪1�ֽڣ���ֹ���
				else
					sprintf(szInput,"%03d.%03d.%03d.%03d", tMaster2.bIP[0], tMaster2.bIP[1], tMaster2.bIP[2], tMaster2.bIP[3]);

				if(EditTextBox(2,"������վ��ַ",szInput,60000,15,DATA_DEC)>=0)
				{
					if (szInput[3]=='.' && szInput[7]=='.' && szInput[11]=='.')
					{
						int iAddr1,iAddr2,iAddr3,iAddr4;
						if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
						{
							if (iAddr1<=255 && iAddr2<=255 && iAddr3<=255 && iAddr4<=255)
							{
								if (listbox.item==0)//��վ1
								{
									tMaster1.bIP[0] = iAddr1;
									tMaster1.bIP[1] = iAddr2;
									tMaster1.bIP[2] = iAddr3;
									tMaster1.bIP[3] = iAddr4;
								}
								else//��վ2
								{
									tMaster2.bIP[0] = iAddr1;
									tMaster2.bIP[1] = iAddr2;
									tMaster2.bIP[2] = iAddr3;
									tMaster2.bIP[3] = iAddr4;
								}
							}
						}	
					}
					else
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
				}	
			}
			else if (listbox.item==1 || listbox.item==3)
			{
				memset(szInput, 0, sizeof(szInput));
				if (listbox.item == 1)
					sprintf(szInput, "%d", OoLongUnsignedToWord(tMaster1.bPort));
				else
					sprintf(szInput, "%d", OoLongUnsignedToWord(tMaster2.bPort));

				if (EditTextBox(2, "������վ�˿�",szInput,60000,5,DATA_DEC)>=0)
				{
					int iport = 0;
					WORD wTmpPort = 0;
					sscanf(szInput, "%d", &iport);
					wTmpPort = iport;
					if (listbox.item == 1)
					{
						OoWordToLongUnsigned(wTmpPort, tMaster1.bPort);
					}
					else
					{
						OoWordToLongUnsigned(wTmpPort, tMaster2.bPort);
					}	
				}
			}
			else if (listbox.item == 4)
			{
				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					memcpy(bGprsBuf+tGprsParser.wPos[0], (BYTE *)&tMaster1, tGprsParser.wLen[0]);
					memcpy(bGprsBuf+tGprsParser.wPos[1], (BYTE *)&tMaster2, tGprsParser.wLen[1]);
					if (OoWriteAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf) >= 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT4, -1);
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}
				
			}
		}
	}
	
	return -1;
}

int SetEthCommunicationPara(void *arg)
{
	if (!InputPwd())
	{
		return -1;
	}

	char menuitem[16][32];
	memset(menuitem, 0, sizeof(menuitem));
	char portlistitem[6][16];
	memset(portlistitem, 0, sizeof(portlistitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 9 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 10 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 11 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 12 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 13 }, //
		{ menuitem[13], 0xFE, Dummy, (void *) 14 }, //
		{ menuitem[14], 0xFE, Dummy, (void *) 15 }, //
		{ menuitem[15], 0xFE, Dummy, (void *) 16 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0, j = 0;
	char szInput[33];
	CListBoxEx listbox;
	char *WorkMode[] = {"���ģʽ", "�ͻ���ģʽ", "������ģʽ"};
	char *LinkMode[] = {"TCP", "UDP"};
	char *LinkAppMode[] = {"����ģʽ", "������ģʽ"};

	BYTE bEthBuf[50] = {0};
	BYTE bWorkMode[2] = {0}, bLinkMode[2] = {0}, bLinkAppMode[2] = {0};
	BYTE bEthReSendTime;
	WORD wHeartBeat;
	TFieldParser tEthParser = {bEthBuf};
	BYTE bType = 0;
	WORD wItemOffSet = 0;
	WORD wItemLen = 0;
	WORD wPortList[5] = {0};
	BYTE bServAddr[6] = {0};
	BYTE bTmpBuf[33] = {0};
	WORD wPort = 0;
	BYTE bTimeOut[3] = {0};
	BYTE bResendTime = 0;
	BYTE bMyBuf1[26] = {0};
	BYTE bMyBuf2[10] = {0};
	BYTE bMyBuf3[10] = {0};
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE *pbFmt;

	char cInput[5] = {0};
	int iModuleNum = 0;
	if(EditSpecBox(2,"������ģ���(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)//������̫��ģ���
	{
		sscanf(cInput,"%d",&iModuleNum);
		if(iModuleNum<9 && iModuleNum>0)
		{
			dwOAD = 0x45100200 + (iModuleNum-1)*0x00010000;	//ͨ������
			if ((tEthParser.wCfgLen=OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bEthBuf, &pbFmt, &wFmtLen)) <= 0)
			{
				MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
				return -1;
			}
			tEthParser.pbCfg = bEthBuf;
			if (OoParseField(&tEthParser, pbFmt, wFmtLen, false))
			{
				ReadParserField(&tEthParser, 0x00, bWorkMode, &bType, &wItemOffSet, &wItemLen);   //����ģʽ
				ReadParserField(&tEthParser, 0x01, bLinkMode, &bType, &wItemOffSet, &wItemLen);   //���ӷ�ʽ
				ReadParserField(&tEthParser, 0x02, bLinkAppMode, &bType, &wItemOffSet, &wItemLen);//����Ӧ�÷�ʽ
				ReadParserField(&tEthParser, 0x03, bMyBuf1, &bType, &wItemOffSet, &wItemLen);      //�����˿��б�
				wPortList[0] = OoLongUnsignedToWord(bMyBuf1+2+3*0+1);
				wPortList[1] = OoLongUnsignedToWord(bMyBuf1+2+3*1+1);
				wPortList[2] = OoLongUnsignedToWord(bMyBuf1+2+3*2+1);
				wPortList[3] = OoLongUnsignedToWord(bMyBuf1+2+3*3+1);
				wPortList[4] = OoLongUnsignedToWord(bMyBuf1+2+3*4+1);
				ReadParserField(&tEthParser, 0x04, bServAddr, &bType, &wItemOffSet, &wItemLen);    //�����������ַ
				ReadParserField(&tEthParser, 0x05, bMyBuf2, &bType, &wItemOffSet, &wItemLen);      //����������˿�
				wPort = OoLongUnsignedToWord(bMyBuf2+1);
				ReadParserField(&tEthParser, 0x06, bTimeOut, &bType, &wItemOffSet, &wItemLen);    //��ʱʱ��,�ط�����
				ReadParserField(&tEthParser, 0x07, bMyBuf3, &bType, &wItemOffSet, &wItemLen);      //��������
				wHeartBeat = OoLongUnsignedToWord(bMyBuf3+1);	
			}	
		}
		else
		{
			MessageBox("����ģ��Ŵ���",KEY_ESC,10000);
			return -1;
		}
	}
	else
	{
		return -1;
	}

	while(1)
	{
		i = 0;

		sprintf(menuitem[i++],"����ģʽ:%s",WorkMode[bWorkMode[1]&0x03]);

		sprintf(menuitem[i++],"���ӷ�ʽ:%s", LinkMode[bLinkMode[1]&0x03]);

		sprintf(menuitem[i++],"����Ӧ�÷�ʽ:%s", LinkAppMode[bLinkAppMode[1]&0x03]);

		sprintf(menuitem[i++], "�����˿��б�:");
		sprintf(menuitem[i++], "1. %d",  wPortList[0]);
		sprintf(menuitem[i++], "2. %d",  wPortList[1]);
		sprintf(menuitem[i++], "3. %d",  wPortList[2]);
		sprintf(menuitem[i++], "4. %d",  wPortList[3]);
		sprintf(menuitem[i++], "5. %d",  wPortList[4]);

		sprintf(menuitem[i++],"�����������ַ:");
		sprintf(menuitem[i++], "%d.%d.%d.%d", bServAddr[2], bServAddr[3], bServAddr[4], bServAddr[5]);

		sprintf(menuitem[i++],"����������˿�:%d", wPort);

		sprintf(menuitem[i++],"��ʱʱ��:%d", bTimeOut[1]>>2);

		bResendTime = bTimeOut[1]&0x03;
		sprintf(menuitem[i++],"�ط�����:%d", bResendTime);

		sprintf(menuitem[i++],"��������:%d", wHeartBeat);

		sprintf(menuitem[i++], "��������");
		tmpS[i].text = NULL;

		listbox.Show(0,"��̫��ͨ������",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item == 0)//����ģʽ
			{
				CListBoxEx listboxTmp0;
				struct ListBoxExItem tmp0[] = { 
					{ WorkMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ WorkMode[1], 0xFF, Dummy, (void *) 0x01 },//
					{ WorkMode[2], 0xFF, Dummy, (void *) 0x02 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp0.Show(0, "����ģʽ", tmp0, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp0.key == KEY_OK)
				{
					bWorkMode[1] = (BYTE)((int)tmp0[listboxTmp0.item].arg);
				}	
			}
			else if (listbox.item == 1)//���ӷ�ʽ
			{
				CListBoxEx listboxTmp1;
				struct ListBoxExItem tmp1[] = { 
					{ LinkMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ LinkMode[1], 0xFF, Dummy, (void *) 0x01 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp1.Show(0, "���ӷ�ʽ", tmp1, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp1.key == KEY_OK)
				{
					bLinkMode[1] = (BYTE)((int)tmp1[listboxTmp1.item].arg);
				}	
			}
			else if (listbox.item == 2)//����Ӧ�÷�ʽ
			{
				CListBoxEx listboxTmp2;
				struct ListBoxExItem tmp2[] = { 
					{ LinkAppMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ LinkAppMode[1], 0xFF, Dummy, (void *) 0x01 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp2.Show(0, "����Ӧ�÷�ʽ", tmp2, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp2.key == KEY_OK)
				{
					bLinkAppMode[1] = (BYTE)((int)tmp2[listboxTmp2.item].arg);
				}	
			}
			else if (listbox.item >= 4 && listbox.item <= 8)//�����˿��б�
			{
				CListBoxEx listboxTmp4;
				struct ListBoxExItem tmp4[] = { 
					{ portlistitem[0], 0xFF, Dummy, (void *) 0x00 },//
					{ portlistitem[1], 0xFF, Dummy, (void *) 0x01 },//
					{ portlistitem[2], 0xFF, Dummy, (void *) 0x02 },//
					{ portlistitem[3], 0xFF, Dummy, (void *) 0x03 },//
					{ portlistitem[4], 0xFF, Dummy, (void *) 0x04 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				while(1)
				{
					sprintf(portlistitem[0], "�˿�һ:%d", wPortList[0]);
					sprintf(portlistitem[1], "�˿ڶ�:%d", wPortList[1]);
					sprintf(portlistitem[2], "�˿���:%d", wPortList[2]);
					sprintf(portlistitem[3], "�˿���:%d", wPortList[3]);
					sprintf(portlistitem[4], "�˿���:%d", wPortList[4]);

					listboxTmp4.Show(0, "�����˿��б�", tmp4, KEY_ESC | (KEY_OK << 8), 60000);
					if (listboxTmp4.key == KEY_ESC || listboxTmp4.key == KEY_NULL)
					{
						break;
					}
					if (listboxTmp4.key == KEY_OK)
					{
						BYTE bcurItem = listboxTmp4.item;
						memset(szInput, 0, sizeof(szInput));
						if (EditTextBox(2, "�����˿ں�",szInput,60000,5,DATA_DEC)>=0)
						{
							int iPort = 0;
							WORD wTmpPort = 0;
							sscanf(szInput, "%d", &iPort);
							wTmpPort = iPort;
							wPortList[bcurItem] = wTmpPort;
						}
					}
				}
			}
			else if (listbox.item==10)//�����������ַ
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput,"%03d.%03d.%03d.%03d", bServAddr[2], bServAddr[3], bServAddr[4], bServAddr[5]);	 //spintf()���Զ���szInput��ĩβ����\0������szInputҪ�࿪1�ֽڣ���ֹ���

				if(EditTextBox(2,"���÷�������ַ",szInput,60000,15,DATA_DEC)>=0)
				{
					if (szInput[3]=='.' && szInput[7]=='.' && szInput[11]=='.')
					{
						int iAddr1,iAddr2,iAddr3,iAddr4;
						if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
						{
							if (iAddr1<=255 && iAddr2<=255 && iAddr3<=255 && iAddr4<=255)
							{
								bServAddr[2] = iAddr1;
								bServAddr[3] = iAddr2;
								bServAddr[4] = iAddr3;
								bServAddr[5] = iAddr4;
							}
						}	
					}
					else
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
				}	
			}
			else if (listbox.item == 11)//����������˿�
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%d", wPort);
				if(EditTextBox(2,"���÷������˿�",szInput,60000,5,DATA_DEC)>=0)
				{
					wPort = (WORD)atoi(szInput);
				}
			}
			else if (listbox.item == 12)//��ʱʱ��
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%d", bTimeOut[1]>>2);
				if (EditTextBox(2, "��ʱʱ��(0~63)", szInput, 60000, 2, DATA_DEC) >= 0)
				{
					BYTE bTimeOutSec = (WORD)atoi(szInput);
					if (bTimeOutSec >= 64)
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
					else
					{
						bTimeOut[1] &= 0x03;
						bTimeOut[1] |= (bTimeOutSec<<2);
					}
				}
			}
			else if (listbox.item == 13)//�ط�����
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%d", bTimeOut[1]&0x03);
				if (EditTextBox(2, "�ط�����(0~3)", szInput, 60000, 1, DATA_DEC) >= 0)
				{
					BYTE bMyResendTime = (BYTE)atoi(szInput);
					if (bMyResendTime > 3)
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
					else
					{
						bTimeOut[1] &= 0xfc;
						bTimeOut[1] |= bMyResendTime;
					}
				}
			}
			else if (listbox.item == 14)//��������
			{
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%d", wHeartBeat);
				if (EditTextBox(2, "��������(0~65535)", szInput, 60000, 5, DATA_DEC) >= 0)
				{
					int tmp = 0;
					sscanf(szInput, "%d", &tmp);
					if (tmp > 65535)
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
					else
					{
						wHeartBeat = tmp;
					}
				}
			}
			else if (listbox.item == 15)//��������
			{
				OoWordToLongUnsigned(wPortList[0], bMyBuf1+2+3*0+1);//�����˿�1
				OoWordToLongUnsigned(wPortList[1], bMyBuf1+2+3*1+1);//�����˿�2
				OoWordToLongUnsigned(wPortList[2], bMyBuf1+2+3*2+1);//�����˿�3
				OoWordToLongUnsigned(wPortList[3], bMyBuf1+2+3*3+1);//�����˿�4
				OoWordToLongUnsigned(wPortList[4], bMyBuf1+2+3*4+1);//�����˿�5
				OoWordToLongUnsigned(wPort, bMyBuf2+1);//����������˿�
				OoWordToLongUnsigned(wHeartBeat, bMyBuf3+1);//��������
				
				memcpy(bEthBuf+tEthParser.wPos[0], bWorkMode, tEthParser.wLen[0]);
				memcpy(bEthBuf+tEthParser.wPos[1], bLinkMode, tEthParser.wLen[1]);
				memcpy(bEthBuf+tEthParser.wPos[2], bLinkAppMode, tEthParser.wLen[2]);
				memcpy(bEthBuf+tEthParser.wPos[3], bMyBuf1, tEthParser.wLen[3]);
				memcpy(bEthBuf+tEthParser.wPos[4], bServAddr, tEthParser.wLen[4]);
				memcpy(bEthBuf+tEthParser.wPos[5], bMyBuf2, tEthParser.wLen[5]);
				memcpy(bEthBuf+tEthParser.wPos[6], bTimeOut, tEthParser.wLen[6]);
				memcpy(bEthBuf+tEthParser.wPos[7], bMyBuf3, tEthParser.wLen[7]);
				
				if(MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					if (OoWriteAttr(dwOAD>>16, (dwOAD>>8)&0xff, bEthBuf) >= 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT4, -1);
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}		
			}
		}
	}

	return -1;
}

int SetEthNetPara(void *arg)
{
	char menuitem[14][32];
	memset(menuitem, 0, sizeof(menuitem));
	
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 9 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 10 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 11 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 12 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 13 }, //
		{ menuitem[13], 0xFE, Dummy, (void *) 14 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0, j = 0;
	char szInput[33];
	CListBoxEx listbox;
	char *IPCfgMode[] = {"DHCP", "��̬", "PPPoE"};
	
	BYTE bEthBuf[102] = {0};
	BYTE bIPCfgMode[2] = {0};
	TFieldParser tEthParser = {bEthBuf};
	BYTE bType = 0;
	WORD wItemOffSet = 0;
	WORD wItemLen = 0;
	//BYTE bTmpBuf[33] = {0};
	BYTE bBuf[20] = {0};
	BYTE bIPAddr[10] = {0};
	BYTE bSubNetMask[10] = {0};
	BYTE bGateWay[10] = {0};
	BYTE bPPPoEUserName[34] = {0};
	BYTE bPPPoEUserPws[34] = {0};
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE *pbFmt;

	char cInput[5] = {0};
	int iModuleNum = 0;
	if(EditSpecBox(2,"������ģ���(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)//������̫��ģ���
	{
		sscanf(cInput,"%d",&iModuleNum);
		if(iModuleNum<9 && iModuleNum>0)
		{
			dwOAD = 0x45100400 + (iModuleNum-1)*0x00010000;	//��������
			if ((tEthParser.wCfgLen=OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bEthBuf, &pbFmt, &wFmtLen)) <= 0)
			{
				MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
				return -1;
			}
			tEthParser.pbCfg = bEthBuf;
			if (OoParseField(&tEthParser, pbFmt, wFmtLen, false))
			{
				ReadParserField(&tEthParser, 0x00, bIPCfgMode, &bType, &wItemOffSet, &wItemLen);   //IP���÷�ʽ
				ReadParserField(&tEthParser, 0x01, bIPAddr, &bType, &wItemOffSet, &wItemLen);		//IP��ַ
				ReadParserField(&tEthParser, 0x02, bSubNetMask, &bType, &wItemOffSet, &wItemLen);	//��������
				ReadParserField(&tEthParser, 0x03, bGateWay, &bType, &wItemOffSet, &wItemLen);      //���ص�ַ
				ReadParserField(&tEthParser, 0x04, bPPPoEUserName, &bType, &wItemOffSet, &wItemLen);//PPPoE�û���
				ReadParserField(&tEthParser, 0x05, bPPPoEUserPws, &bType, &wItemOffSet, &wItemLen); //PPPoE����
			}	
		}
		else
		{
			MessageBox("����ģ��Ŵ���",KEY_ESC,10000);
			return -1;
		}
	}
	else
	{
		return -1;
	}

	while(1)
	{
		i = 0;

		sprintf(menuitem[i++],"IP���÷�ʽ:%s",IPCfgMode[bIPCfgMode[1]&0x03]);

		sprintf(menuitem[i++],"IP��ַ:");
		sprintf(menuitem[i++],"%d.%d.%d.%d", bIPAddr[2], bIPAddr[3], bIPAddr[4], bIPAddr[5]);

		sprintf(menuitem[i++],"��������:");
		sprintf(menuitem[i++],"%d.%d.%d.%d", bSubNetMask[2], bSubNetMask[3], bSubNetMask[4], bSubNetMask[5]);

		sprintf(menuitem[i++],"���ص�ַ:");
		sprintf(menuitem[i++],"%d.%d.%d.%d", bGateWay[2], bGateWay[3], bGateWay[4], bGateWay[5]);

		
		memset(bBuf, 0, sizeof(bBuf));
		sprintf(menuitem[i++],"PPPoE�û���:");
		memcpy(bBuf, &bPPPoEUserName[2], 16);
		sprintf(menuitem[i++],"%s",(char *)bBuf);
		memcpy(bBuf, &bPPPoEUserName[18], 16);
		if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
			sprintf(menuitem[i++], " "); //����û���
		else
			sprintf(menuitem[i++],"%s",(char *)bBuf);
		
		memset(bBuf, 0, sizeof(bBuf));
		sprintf(menuitem[i++],"PPPoE����:");
		memcpy(bBuf, &bPPPoEUserPws[2], 16);
		sprintf(menuitem[i++],"%s",(char *)bBuf);
		memcpy(bBuf, &bPPPoEUserPws[18], 16);
		if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
			sprintf(menuitem[i++], " "); //����û���
		else
			sprintf(menuitem[i++],"%s",(char *)bBuf);

		sprintf(menuitem[i++], "��������");
		tmpS[i].text = NULL;

		listbox.Show(0,"��̫����������",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.item == 0)
		{
			CListBoxEx listboxTmp0;
			struct ListBoxExItem tmp0[] = { 
				{ IPCfgMode[0], 0xFF, Dummy, (void *) 0x00 },//
				{ IPCfgMode[1], 0xFF, Dummy, (void *) 0x01 },//
				{ IPCfgMode[2], 0xFF, Dummy, (void *) 0x02 },//
				{ NULL, 0xFF, NULL, NULL }, //
			};
			listboxTmp0.Show(0, "IP���÷�ʽ", tmp0, KEY_ESC | (KEY_OK << 8), 60000);
			if (listboxTmp0.key == KEY_OK)
			{
				bIPCfgMode[1] = (BYTE)((int)tmp0[listboxTmp0.item].arg);
			}	
		}
		else if (listbox.item == 2)
		{
			memset(szInput, 0, sizeof(szInput));
			sprintf(szInput,"%03d.%03d.%03d.%03d", bIPAddr[2], bIPAddr[3], bIPAddr[4], bIPAddr[5]);	 

			if(EditTextBox(2,"IP��ַ",szInput,60000,15,DATA_DEC)>=0)
			{
				if (szInput[3]=='.' && szInput[7]=='.' && szInput[11]=='.')
				{
					int iAddr1,iAddr2,iAddr3,iAddr4;
					if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
					{
						if (iAddr1<=255 && iAddr2<=255 && iAddr3<=255 && iAddr4<=255)
						{
							bIPAddr[2] = iAddr1;
							bIPAddr[3] = iAddr2;
							bIPAddr[4] = iAddr3;
							bIPAddr[5] = iAddr4;
							bIPAddr[1] = 4;     //��ʱ����IPv4��IP��ַ���ȣ�������չ��IPv6���޸�
							tEthParser.wLen[1] = bIPAddr[1] + 2;
						}
					}	
				}
				else
				{
					MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
				}
			}	
		}
		else if (listbox.item == 4)
		{
			memset(szInput, 0, sizeof(szInput));
			sprintf(szInput,"%03d.%03d.%03d.%03d", bSubNetMask[2], bSubNetMask[3], bSubNetMask[4], bSubNetMask[5]);	 

			if(EditTextBox(2,"��������",szInput,60000,15,DATA_DEC)>=0)
			{
				if (szInput[3]=='.' && szInput[7]=='.' && szInput[11]=='.')
				{
					int iAddr1,iAddr2,iAddr3,iAddr4;
					if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
					{
						if (iAddr1<=255 && iAddr2<=255 && iAddr3<=255 && iAddr4<=255)
						{
							bSubNetMask[2] = iAddr1;
							bSubNetMask[3] = iAddr2;
							bSubNetMask[4] = iAddr3;
							bSubNetMask[5] = iAddr4;
							bSubNetMask[1] = 4;    //��ʱ����IPv4��IP��ַ���ȣ�������չ��IPv6���޸�
							tEthParser.wLen[2] = bSubNetMask[1] + 2;
						}
					}	
				}
				else
				{
					MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
				}
			}		
		}
		else if (listbox.item == 6)
		{
			memset(szInput, 0, sizeof(szInput));
			sprintf(szInput,"%03d.%03d.%03d.%03d", bGateWay[2], bGateWay[3], bGateWay[4], bGateWay[5]);	 

			if(EditTextBox(2,"���ص�ַ",szInput,60000,15,DATA_DEC)>=0)
			{
				if (szInput[3]=='.' && szInput[7]=='.' && szInput[11]=='.')
				{
					int iAddr1,iAddr2,iAddr3,iAddr4;
					if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
					{
						if (iAddr1<=255 && iAddr2<=255 && iAddr3<=255 && iAddr4<=255)
						{
							bGateWay[2] = iAddr1;
							bGateWay[3] = iAddr2;
							bGateWay[4] = iAddr3;
							bGateWay[5] = iAddr4;
							bGateWay[1] = 4;     //��ʱ����IPv4��IP��ַ���ȣ�������չ��IPv6���޸�
							tEthParser.wLen[3] = bGateWay[1] + 2; 
						}
					}	
				}
				else
				{
					MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
				}
			}		
		}
		else if (listbox.item==8 || listbox.item==9)
		{
			if (listbox.item == 8)
			{
				memset(szInput, 0, sizeof(szInput));
				memcpy(szInput, &bPPPoEUserName[2], 16);
				if(getSoftKey("PPPoE�û���", szInput, 60000, 16, DATA_ASC) >= 0)
				{
					memcpy(&bPPPoEUserName[2], szInput, 16);
				}
				char *pTail = strstr((char*)&bPPPoEUserName[2]," ");
				if(pTail)
				{
					memset(pTail, 0x00, &bPPPoEUserName[17]-(BYTE*)pTail+1);
					memset(&bPPPoEUserName[18], 0, 16);
				}
			}
			else if (listbox.item == 9)
			{
				memset(szInput, 0, sizeof(szInput));
				memcpy(szInput, &bPPPoEUserName[18], 16);
				if (bPPPoEUserName[17] != 0)
				{
					if(getSoftKey("PPPoE�û���", szInput, 60000, 16, DATA_ASC) >= 0)
					{
						memcpy(&bPPPoEUserName[18], szInput, 16);
					}
					char *pTail = strstr((char*)&bPPPoEUserName[18]," ");
					if(pTail)
					{
						memset(pTail, 0x00, &bPPPoEUserName[33]-(BYTE*)pTail+1);
					}
				}
			}
			tEthParser.wLen[4] = strlen((char *)bPPPoEUserName);
			bPPPoEUserName[1] = tEthParser.wLen[4] - 2;
		}
		else if (listbox.item==11 || listbox.item==12)
		{
			if (listbox.item == 11)
			{
				memset(szInput, 0, sizeof(szInput));
				memcpy(szInput, &bPPPoEUserPws[2], 16);
				if(getSoftKey("PPPoE����", szInput, 60000, 16, DATA_ASC) >= 0)
				{
					memcpy(&bPPPoEUserPws[2], szInput, 16);
				}
				char *pTail = strstr((char*)&bPPPoEUserPws[2]," ");
				if(pTail)
				{
					memset(pTail, 0x00, &bPPPoEUserPws[17]-(BYTE*)pTail+1);
					memset(&bPPPoEUserPws[18], 0, 16);
				}
			}
			else if (listbox.item == 12)
			{
				memset(szInput, 0, sizeof(szInput));
				memcpy(szInput, &bPPPoEUserPws[18], 16);
				if (bPPPoEUserPws[17] != 0)
				{
					if(getSoftKey("PPPoE����", szInput, 60000, 16, DATA_ASC) >= 0)
					{
						memcpy(&bPPPoEUserPws[18], szInput, 16);
					}
					char *pTail = strstr((char*)&bPPPoEUserPws[18]," ");
					if(pTail)
					{
						memset(pTail, 0x00, &bPPPoEUserPws[33]-(BYTE*)pTail+1);
					}
				}
			}
			tEthParser.wLen[5] = strlen((char *)bPPPoEUserPws);
			bPPPoEUserPws[1] = tEthParser.wLen[5] - 2;
		}
		else if (listbox.item == 13)
		{
			if(MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
			{
				if (OoWriteField(bEthBuf, tEthParser.wCfgLen, pbFmt, wFmtLen, 0x00, bIPCfgMode, tEthParser.wLen[0])>=0 &&
					OoWriteField(bEthBuf, tEthParser.wCfgLen, pbFmt, wFmtLen, 0x01, bIPAddr, tEthParser.wLen[1])>=0 &&
					OoWriteField(bEthBuf, tEthParser.wCfgLen, pbFmt, wFmtLen, 0x02, bSubNetMask, tEthParser.wLen[2])>=0 &&
					OoWriteField(bEthBuf, tEthParser.wCfgLen, pbFmt, wFmtLen, 0x03, bGateWay, tEthParser.wLen[3])>=0 &&
					OoWriteField(bEthBuf, tEthParser.wCfgLen, pbFmt, wFmtLen, 0x04, bPPPoEUserName, tEthParser.wLen[4])>=0 &&
					OoWriteField(bEthBuf, tEthParser.wCfgLen, pbFmt, wFmtLen, 0x05, bPPPoEUserPws, tEthParser.wLen[5])>=0)
				{
					if (OoWriteAttr(dwOAD>>16, (dwOAD>>8)&0xff, bEthBuf) >= 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT4, -1);
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}
				else
				{
					MessageBox("����ʧ��",KEY_ESC,2000);
				}
			}	
		}
	}


	return -1;
}

int SetEthMasterPara(void *arg)
{
	if (!InputPwd())
	{
		return -1;
	}

	char menuitem[5][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0;
	char szInput[20];
	CListBoxEx listbox;
	
	BYTE bEthBuf[24] = {0};
	TFieldParser tEthParser = {bEthBuf};
	BYTE bType = 0;
	WORD wItemOffSet = 0;
	WORD wItemLen = 0;
	char cInput[5] = {0};
	int iModuleNum = 0;
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE *pbFmt;
	typedef struct{
		BYTE bStructType;
		BYTE bFiledNum;
		BYTE bStringType;
		BYTE bLen;
		BYTE bIP[4];
		BYTE bLongUnsighedType;
		BYTE bPort[2];
	}TMasterPara; 
	TMasterPara tMaster1;
	TMasterPara tMaster2;
	memset(&tMaster1, 0, sizeof(TMasterPara));
	memset(&tMaster2, 0, sizeof(TMasterPara));


	if(EditSpecBox(2,"������ģ���(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)//������̫��ģ���
	{
		sscanf(cInput,"%d",&iModuleNum);
		if(iModuleNum<9 && iModuleNum>0)
		{
			dwOAD = 0x45100300 + (iModuleNum-1)*0x00010000;	//��վͨ�Ų���
			if ((tEthParser.wCfgLen=OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bEthBuf, &pbFmt, &wFmtLen)) <= 0)
			{
				MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
				return -1;
			}
			tEthParser.pbCfg = bEthBuf;
			if (OoParseField(&tEthParser, pbFmt, wFmtLen, false))
			{
				ReadParserField(&tEthParser, 0x00, (BYTE *)&tMaster1, &bType, &wItemOffSet, &wItemLen);   //��վ1
				ReadParserField(&tEthParser, 0x01, (BYTE *)&tMaster2, &bType, &wItemOffSet, &wItemLen);  //��վ2
			}	
		}
		else
		{
			MessageBox("����ģ��Ŵ���",KEY_ESC,10000);
			return -1;
		}
	}
	else
	{
		return -1;
	}

	while(1)
	{
		i = 0;
		sprintf(menuitem[i++],"��վ1IP:%d.%d.%d.%d", tMaster1.bIP[0], tMaster1.bIP[1], tMaster1.bIP[2], tMaster1.bIP[3]);
		sprintf(menuitem[i++],"��վ1�˿�:%d", OoLongUnsignedToWord(tMaster1.bPort));
		sprintf(menuitem[i++],"��վ2IP:%d.%d.%d.%d", tMaster2.bIP[0], tMaster2.bIP[1], tMaster2.bIP[2], tMaster2.bIP[3]);
		sprintf(menuitem[i++],"��վ2�˿�:%d", OoLongUnsignedToWord(tMaster2.bPort));
		sprintf(menuitem[i++],"��������");
		tmpS[i].text = NULL;

		listbox.Show(0,"��վͨ�Ų���",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item==0 || listbox.item==2)
			{
				memset(szInput, 0, sizeof(szInput));
				if (listbox.item==0)
					sprintf(szInput,"%03d.%03d.%03d.%03d", tMaster1.bIP[0], tMaster1.bIP[1], tMaster1.bIP[2], tMaster1.bIP[3]);	 //spintf()���Զ���szInput��ĩβ����\0������szInputҪ�࿪1�ֽڣ���ֹ���
				else
					sprintf(szInput,"%03d.%03d.%03d.%03d", tMaster2.bIP[0], tMaster2.bIP[1], tMaster2.bIP[2], tMaster2.bIP[3]);

				if(EditTextBox(2,"������վ��ַ",szInput,60000,15,DATA_DEC)>=0)
				{
					if (szInput[3]=='.' && szInput[7]=='.' && szInput[11]=='.')
					{
						int iAddr1,iAddr2,iAddr3,iAddr4;
						if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
						{
							if (iAddr1<=255 && iAddr2<=255 && iAddr3<=255 && iAddr4<=255)
							{
								if (listbox.item==0)//��վ1
								{
									tMaster1.bIP[0] = iAddr1;
									tMaster1.bIP[1] = iAddr2;
									tMaster1.bIP[2] = iAddr3;
									tMaster1.bIP[3] = iAddr4;
								}
								else//��վ2
								{
									tMaster2.bIP[0] = iAddr1;
									tMaster2.bIP[1] = iAddr2;
									tMaster2.bIP[2] = iAddr3;
									tMaster2.bIP[3] = iAddr4;
								}
							}
						}	
					}
					else
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
				}	
			}
			else if (listbox.item==1 || listbox.item==3)
			{
				memset(szInput, 0, sizeof(szInput));
				if (listbox.item == 1)
					sprintf(szInput, "%d", OoLongUnsignedToWord(tMaster1.bPort));
				else
					sprintf(szInput, "%d", OoLongUnsignedToWord(tMaster2.bPort));
				if (EditTextBox(2, "������վ�˿�",szInput,60000,5,DATA_DEC)>=0)
				{
					int iPort = 0;
					WORD wTmpPort = 0;
					sscanf(szInput, "%d", &iPort);
					wTmpPort = iPort;
					if (listbox.item == 1)
					{
						OoWordToLongUnsigned(wTmpPort, tMaster1.bPort);
					}
					else
					{
						OoWordToLongUnsigned(wTmpPort, tMaster2.bPort);
					}	
				}
			}
			else if (listbox.item == 4)
			{
				memcpy(bEthBuf+tEthParser.wPos[0], (BYTE *)&tMaster1, tEthParser.wLen[0]);
				memcpy(bEthBuf+tEthParser.wPos[1], (BYTE *)&tMaster2, tEthParser.wLen[1]);
				if(MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					if (OoWriteAttr(dwOAD>>16, (dwOAD>>8)&0xff, bEthBuf) >= 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT4, -1);
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}	
			}
		}
	}

	return -1;
}

int SetEthMacAddr(void *arg)
{
	char menuitem[2][32] = {0};
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ NULL, 0xFE, NULL, NULL }, //
	};

	int i = 0;
	char szInput[20];
	CListBoxEx listbox;
	char cInput[5] = {0};
	int iModuleNum = 0;
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE *pbFmt = NULL;
	BYTE bMacBuf[8] = {0};
	
	if(EditSpecBox(2,"������ģ���(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)//������̫��ģ���
	{
		sscanf(cInput,"%d",&iModuleNum);
		if(iModuleNum<9 && iModuleNum>0)
		{
			dwOAD = 0x45100500 + (iModuleNum-1)*0x00010000;	//��վͨ�Ų���
			if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bMacBuf, &pbFmt, &wFmtLen) <= 0)
			{
				MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
				return -1;
			}
		}
		else
		{
			MessageBox("����ģ��Ŵ���",KEY_ESC,10000);
			return -1;
		}
	}
	else
	{
		return -1;
	}

	while(1)
	{	
		sprintf(menuitem[0], "MAC��ַ:");
		sprintf(menuitem[1], "%02X:%02X:%02X:%02X:%02X:%02X", bMacBuf[2], bMacBuf[3], bMacBuf[4], bMacBuf[5], bMacBuf[6], bMacBuf[7]);
		tmpS[2].text = NULL;

		listbox.Show(0, "MAC��ַ", tmpS, KEY_ESC | KEY_OK << 8, 60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK && listbox.item != 0)
		{
			int i;
			BYTE *p = (BYTE *)szInput;
			memset(szInput, 0, sizeof(szInput));
			for (i=0; i<bMacBuf[1]; i++)
			{
				ByteToASCII(bMacBuf[2+i], &p);
			}
			if (EditTextBox(2, "����MAC��ַ", szInput, 60000, 12, DATA_HEX) >= 0)
			{
				AsciiToByte((BYTE *)szInput, 12, bMacBuf+2);
				if (MessageBox("ȷ��Ҫ����?", KEY_ESC,10000) > 0)
				{
					if (OoWriteAttr(dwOAD>>16, (dwOAD>>8)&0xff, bMacBuf) >= 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT4, -1);
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}
			}
		}
	}

	return -1;
}

int SetGprsModule(void *arg)
{
	if(!InputPwd())
		return -1;

	CListBoxEx listbox;
	char menuitem[4][32];
	int i = 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[i++],0xFE,Dummy,(void*)1},
		{menuitem[i++],0xFE,Dummy,(void*)2},
		{menuitem[i++],0xFE,Dummy,(void*)3},
		{menuitem[i++],0xFE,Dummy,(void*)4},

	};
	i = 0;
	WORD wModuleType = 0;
	WORD wModuleTypeBak;
	ReadItemEx(BN1, PN0, 0x2012, (BYTE*)&wModuleType);
	sprintf(menuitem[i++],"ģ���ͺ�: %03d",wModuleType);
	sprintf(menuitem[i++],"%s","��������");
	items[i].text = NULL;
	while(1)
	{
		i = 0;
		sprintf(menuitem[i++],"��ģ���ͺ�: %03d",wModuleType);
		listbox.Show(0,"ģ������",items,KEY_ESC|(KEY_OK<<8),60000);

		if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			break;

		if(listbox.key == KEY_OK)
		{
			char szInput[16];
			if(listbox.item == 0)
			{
				memset(szInput, 0, sizeof(szInput));
				wModuleTypeBak = wModuleType;
				sprintf(szInput,"%03d",wModuleType);
				if(EditTextBox(2,"ģ������", szInput, 60000, 3, DATA_DEC) >= 0)
				{
					wModuleType = szInput[0] - 0x30;
					wModuleType *= 100;
					wModuleType += BcdToByte(((szInput[1]-0x30)<<4) + (szInput[2]-0x30));
					if (wModuleType > 255)
					{
						wModuleType = wModuleTypeBak;
						MessageBox("ģ�鷶Χ��1~255",KEY_ESC,10000);
					}
				}
			}
			else if(listbox.item == 1)
			{
				if(MessageBox("ȷ��Ҫ����ģ��?",KEY_ESC,10000) > 0)
				{
					if (WriteItemEx(BN1, PN0, 0x2012, (BYTE*)&wModuleType)>0)
					{
						FaSavePara();
						MessageBox("���óɹ�!",KEY_ESC,2000);
						TrigerSaveBank(BN1, 0, -1);	//��������һ��
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��!",KEY_ESC,2000);
					}
				}
			}
		}
	}
	return -1;
}

int SetGprsPara(void *arg)
{
	CListBoxEx listbox;

	struct ListBoxExItem items[] = {
		{ (char *) "ͨ������", MENU_ONELEVEL_HAVE_NO, SetGprsCommunicationPara, (void *)1 },
		{ (char *) "����ͨ�Ų���", MENU_ONELEVEL_HAVE_NO, SetGprsMessagePara, (void *)2 },
		{ (char *) "��վͨ�Ų���", MENU_ONELEVEL_HAVE_NO, SetGprsMasterPara, (void *)3 },
		{ NULL, 0xFF, NULL, NULL },
	};

	while(1)
	{
		listbox.Show(0 , "���߹�����ʽ", items, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

int SetEthPara(void *arg)
{
	CListBoxEx listbox;

	struct ListBoxExItem items[] = {
		{ (char *) "ͨ������", MENU_ONELEVEL_HAVE_NO, SetEthCommunicationPara, (void *)1 },
		{ (char *) "��������", MENU_ONELEVEL_HAVE_NO, SetEthNetPara, (void *)2 },
		{ (char *) "��վͨ�Ų���", MENU_ONELEVEL_HAVE_NO, SetEthMasterPara, (void *)3 },
		{ (char *) "MAC��ַ", MENU_ONELEVEL_HAVE_NO, SetEthMacAddr, (void *)4 },
		{ NULL, 0xFF, NULL, NULL },
	};

	while(1)
	{
		listbox.Show(0 , "��̫����ʽ", items, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

int  ChoseCommunicationMode(void *arg)
{
	CListBoxEx listbox;

	struct ListBoxExItem items[] = {
		{ (char *) "���߹�����ʽ", MENU_ONELEVEL_HAVE_NO, SetGprsPara, NULL },
		{ (char *) "��̫����ʽ", MENU_ONELEVEL_HAVE_NO, SetEthPara, NULL },
		{ NULL, 0xFF, NULL, NULL },
	};

	while(1)
	{
		listbox.Show(0 , "ͨѶ��������", items, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
	
}

int GetRealData(void *arg)
{
	CListBoxEx listbox;
	int iArg = (int)arg;
	char menuitem[10][32];
	char *title[2][4] = {{"��������Ϣ","�����й�����ʾֵ","�����й�����ʾֵ","�޹�����ʾֵ"},{"��������Ϣ","��ǰ�����ѹ","��ǰ�������","��ǰ��������"}};
	memset(menuitem, 0, sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)3},
		{menuitem[3],0xFE,Dummy,(void*)4},
		{menuitem[4],0xFE,Dummy,(void*)5},
		{menuitem[5],0xFE,Dummy,(void*)6},
		{menuitem[6],0xFE,Dummy,(void*)6},
		{ (char*) NULL, 0xFE, NULL, NULL }, //	
	};
	BYTE i  = 0;
	BYTE *cp = NULL;
	int iPageNum = 0;
	char szInput[20];
	memset(szInput, 0, sizeof(szInput));
	int iGroupNo = 0;
	BYTE pbBuf[80];
	char str[20];
	WORD wIDs[8] = {
		0x901f,
		0x902f,

	};
	TBankItem NewItemBank[] = {
		{BN0,PN0,0x9110},
		{BN0,PN0,0x9120},
		{BN0,PN0,0x9130},
		{BN0,PN0,0x9150},
		{BN0,PN0,0x9160},
		{BN0,PN0,0x9140},
	};
	
	if(EditSpecBox(2,"�����������",szInput,60000,4,DATA_DEC)>=0)
	{
		sscanf(szInput,"%d",&iGroupNo);
		
		if (iGroupNo<1 || iGroupNo>=POINT_NUM || !IsPnValid(iGroupNo))
		{
			MessageBox("��������Ŵ���!",KEY_ESC,10000);
			return -1;
		}

		while(1)
		{
			if (iArg == 1)
			{
				i = 0;
				memset(menuitem, 0, sizeof(menuitem));
				switch (iPageNum)
				{
				case 0:
					sprintf(menuitem[i++],"������:%d",iGroupNo);
					//ReadItemEx(BN0,iGroupNo,0x01df,pbBuf);
					//sprintf(menuitem[i++],"�ֱ��:%s",pbBuf);
					//ReadItemEx(BN0,iGroupNo,0x8902, pbBuf);
					GetMeterTsa((WORD)iGroupNo, pbBuf);
					sprintf(menuitem[i++],"���ַ:%02X%02X%02X%02X%02X%02X",pbBuf[5],pbBuf[4],pbBuf[3],pbBuf[2],pbBuf[1],pbBuf[0]);
					break;
				case 1:
				case 2:
					ReadItemEx(BN0,iGroupNo,wIDs[iPageNum-1],pbBuf);
					cp = pbBuf;
					Fmt14ToStr(cp, str);
					sprintf(menuitem[i++], "�ܣ�%skWh", str);

					cp += 5;
					Fmt14ToStr(cp,str);
					sprintf(menuitem[i++], "�⣺%skWh", str);

					cp += 5;
					Fmt14ToStr(cp,str);
					sprintf(menuitem[i++], "�壺%skWh", str);

					cp += 5;
					Fmt14ToStr(cp,str);
					sprintf(menuitem[i++], "ƽ��%skWh", str);

					cp += 5;
					Fmt14ToStr(cp,str);
					sprintf(menuitem[i++], "�ȣ�%skWh", str);

					memset(menuitem[i++], 0, sizeof(menuitem[0]));//���������Ƕ��룬����������

					break;
				case 3:
					for(BYTE j = 0;j < 6; j++)
					{
						NewItemBank[j].wPn = iGroupNo;
					}
					ReadItemEx(NewItemBank, 6, pbBuf);
					cp = pbBuf;
					Fmt11ToStr(cp, str);
					sprintf(menuitem[i++], "�����޹�:%skvarh", str);
	
					cp += 4;
					Fmt11ToStr(cp, str);
					sprintf(menuitem[i++], "�����޹�:%skvarh", str);

					cp += 4;
					Fmt11ToStr(cp, str);
					sprintf(menuitem[i++], "һ����:%skvarh", str);

					cp += 4;
					Fmt11ToStr(cp, str);
					sprintf(menuitem[i++], "������:%skvarh", str);

					cp += 4;
					Fmt11ToStr(cp, str);
					sprintf(menuitem[i++], "������:%skvarh", str);

					cp += 4;
					Fmt11ToStr(cp, str);
					sprintf(menuitem[i++], "������:%skvarh", str);

					break;
				}

				listbox.Show(0, title[0][iPageNum],items,KEY_ESC | KEY_UP<<8 | KEY_DOWN<<16, 60000, false);
				if (listbox.key == KEY_DOWN)
				{
					iPageNum++;
					if (iPageNum >= 4)
					{
						iPageNum = 0;
					}
				}
				else if (listbox.key == KEY_UP)
				{
					iPageNum--;
					if (iPageNum < 0)
					{
						iPageNum = 3;
					}
				}
				else if (listbox.key == KEY_NULL || listbox.key == KEY_ESC)
				{
					break;
				}
			}
			else if (iArg == 2)
			{
				i = 0;
				memset(menuitem, 0, sizeof(menuitem));
				switch (iPageNum)
				{
				case 0:
					sprintf(menuitem[i++],"������:%d",iGroupNo);
					//ReadItemEx(BN0,iGroupNo,0x01df,pbBuf);
					//sprintf(menuitem[i++],"�ֱ��:%s",pbBuf);
					//ReadItemEx(BN0,iGroupNo,0x8902, pbBuf);
					GetMeterTsa((WORD)iGroupNo, pbBuf);
					sprintf(menuitem[i++],"���ַ:%02X%02X%02X%02X%02X%02X",pbBuf[5],pbBuf[4],pbBuf[3],pbBuf[2],pbBuf[1],pbBuf[0]);
					break;
				case 1:
				
					ReadItemEx(BN0,iGroupNo,0xb61f,pbBuf);
					cp = pbBuf;
				//	Fmt14ToStr(pbBuf, str);
					Fmt7ToStr(cp,str);
					sprintf(menuitem[i++], "Ua��%sV", str);

					cp += 2;
					Fmt7ToStr(cp,str);
					sprintf(menuitem[i++], "Ub��%sV", str);

					cp += 2;
					Fmt7ToStr(cp,str);
					sprintf(menuitem[i++], "Uc��%sV", str);
					
					memset(menuitem[i++], 0, sizeof(menuitem[0]));//���������Ƕ��䣬����������

					break;
				case 2:
					ReadItemEx(BN0,iGroupNo,0xb62f,pbBuf);
					cp = pbBuf;
					Fmt25ToStr(cp, str);
					sprintf(menuitem[i++], "Ia:%sA", str);

					cp += 3;
					Fmt25ToStr(cp, str);
					sprintf(menuitem[i++], "Ib:%sA", str);

					cp += 3;
					Fmt25ToStr(cp, str);
					sprintf(menuitem[i++], "Ic:%sA", str);

					ReadItemEx(BN0,iGroupNo,0xb6a0,pbBuf);
				//	cp += 3;
					Fmt25ToStr(pbBuf, str);
					sprintf(menuitem[i++], "�������:%sA", str);

					break;
				case 3:
					ReadItemEx(BN0,iGroupNo,0xb65f,pbBuf);
					cp = pbBuf;
					Fmt5ToStr(cp, str);
					sprintf(menuitem[i++], "cos��:%s%%", str);

					cp += 2;
					Fmt5ToStr(cp, str);
					sprintf(menuitem[i++], "cos��a:%s%%", str);

					cp += 2;
					Fmt5ToStr(cp, str);
					sprintf(menuitem[i++], "cos��b:%s%%", str);

					cp += 2;
					Fmt5ToStr(cp, str);
					sprintf(menuitem[i++], "cos��c:%s%%", str);
					break;
				}

				listbox.Show(0, title[1][iPageNum],items,KEY_ESC | KEY_UP<<8 | KEY_DOWN<<16, 60000, false);
				if (listbox.key == KEY_DOWN)
				{
					iPageNum++;
					if (iPageNum >= 4)
					{
						iPageNum = 0;
					}
				}
				else if (listbox.key == KEY_UP)
				{
					iPageNum--;
					if (iPageNum < 0)
					{
						iPageNum = 3;
					}
				}
				else if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
				{
					break;
				}
			}
		}
	}
	return -1;
}


int GetDemandEng(void *arg)
{
	CListBoxEx listbox;
	char menuitem[7][32];
	int iPageNum = 0;
	BYTE i = 0;
	char *title[3] = {"��������Ϣ","��ǰ�й��������","��ǰ�޹��������"};//,"�����й��������","�����޹��������"};
	memset(menuitem, 0, sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)3},
		{menuitem[3],0xFE,Dummy,(void*)4},
		{menuitem[4],0xFE,Dummy,(void*)5},
		{menuitem[5],0xFE,Dummy,(void*)6},
		{menuitem[6],0xFE,Dummy,(void*)7},
		{ NULL, 0xFE, NULL, NULL }, //	
	};
	char szInput[20] = {0};
	BYTE *cp = NULL;
	int iGroupNo = 0;
	BYTE pbBuf[80];
	char str[20];
	if(EditSpecBox(2,"������������:",szInput,60000,2,DATA_DEC)>=0)
	{
		iGroupNo = atoi(szInput);
		
		while(1)
		{
			if (iGroupNo<1 || iGroupNo>=POINT_NUM || !IsPnValid(iGroupNo))
			{
				MessageBox("������������",KEY_ESC,10000);
				return -1;
			}
			i = 0;
			
			memset(menuitem, 0, sizeof(menuitem));
			switch(iPageNum)
			{
			case 0:
				sprintf(menuitem[i++],"�����㣺%d",iGroupNo);
				ReadItemEx(BN0,iGroupNo,0x01df,pbBuf);
				sprintf(menuitem[i++],"�ֱ��:%s",pbBuf);
				ReadItemEx(BN0,iGroupNo,0x8902, pbBuf);
				sprintf(menuitem[i++],"���ַ:%02X%02X%02X%02X%02X%02X",pbBuf[10],pbBuf[9],pbBuf[8],pbBuf[7],pbBuf[6],pbBuf[5]);
				break;
			case 1:
				ReadItemEx(BN0,iGroupNo,0xa010,pbBuf);
				Fmt23ToStr(pbBuf, str);
				sprintf(menuitem[i++],"��ǰ�����й��������:");
				sprintf(menuitem[i++], "  %skW", str);

				ReadItemEx(BN0,iGroupNo,0xb010,pbBuf);
				Fmt17ToStr(pbBuf,str);
				sprintf(menuitem[i++],"����ʱ��:%s",str); 

				ReadItemEx(BN0,iGroupNo,0xa020,pbBuf);
				Fmt23ToStr(pbBuf, str);

				sprintf(menuitem[i++],"��ǰ�����й��������:");
				sprintf(menuitem[i++], "  %skW", str);

				ReadItemEx(BN0,iGroupNo,0xb020,pbBuf);
				Fmt17ToStr(pbBuf,str);
				sprintf(menuitem[i++],"����ʱ��:%s",str); 
				break;
			case 2:
				ReadItemEx(BN0,iGroupNo,0xa110,pbBuf);
				Fmt23ToStr(pbBuf, str);
				sprintf(menuitem[i++],"��ǰ�����޹��������:");
				sprintf(menuitem[i++], "  %skvar", str);


				ReadItemEx(BN0,iGroupNo,0xb120,pbBuf);
				Fmt17ToStr(pbBuf,str);
				sprintf(menuitem[i++],"����ʱ��:%s",str);

				sprintf(menuitem[i++],"��ǰ�����޹��������:");
				Fmt23ToStr(pbBuf, str);
				sprintf(menuitem[i++], "  %skvar", str);

				ReadItemEx(BN0,iGroupNo,0xb120,pbBuf);
				Fmt17ToStr(pbBuf,str);
				sprintf(menuitem[i++],"����ʱ��:%s",str);
				break;
// 			case 2:
// 				ReadItemEx(BN0,iGroupNo,0xa120,pbBuf);
// 				sprintf(menuitem[i++],"���������й��������:");
// 				Fmt23ToStr(pbBuf, str);
// 				sprintf(menuitem[i++], "  %skW", str);
// 
// 
// 				ReadItemEx(BN0,iGroupNo,0xb410,pbBuf);
// 				Fmt17ToStr(pbBuf,str);
// 				sprintf(menuitem[i++],"����ʱ��:%s",str);
// 
// 				sprintf(menuitem[i++],"���·����й��������:");
// 				Fmt23ToStr(pbBuf, str);
// 				sprintf(menuitem[i++], "  %skW", str);
// 
// 
// 				ReadItemEx(BN0,iGroupNo,0xb420,pbBuf);
// 				Fmt17ToStr(pbBuf,str);
// 				sprintf(menuitem[i++],"����ʱ��:%s",str);
// 				break;
// 			case 3:
// 				ReadItemEx(BN0,iGroupNo,0xa510,pbBuf);
// 				sprintf(menuitem[i++],"���������޹��������:");
// 				Fmt23ToStr(pbBuf, str);
// 				sprintf(menuitem[i++], "  %skvar", str);
// 
// 
// 				ReadItemEx(BN0,iGroupNo,0xb510,pbBuf);
// 				Fmt17ToStr(pbBuf,str);
// 				sprintf(menuitem[i++],"����ʱ��:%s",str);
// 
// 				ReadItemEx(BN0,iGroupNo,0xa520,pbBuf);
// 				sprintf(menuitem[i++],"���·����޹��������:");
// 				Fmt23ToStr(pbBuf, str);
// 				sprintf(menuitem[i++], "  %skvar", str);
// 
// 
// 				ReadItemEx(BN0,iGroupNo,0xb520,pbBuf);
// 				Fmt17ToStr(pbBuf,str);
// 				sprintf(menuitem[i++],"����ʱ��:%s",str);
// 				break;
			}

			listbox.Show(0,title[iPageNum],items,KEY_ESC | KEY_UP<<8 | KEY_DOWN<<16,60000,false);
			if (listbox.key == KEY_UP)
			{
				iPageNum--;
				if (iPageNum < 0)
				{
					iPageNum = 2;
				}
			}
			if (listbox.key == KEY_DOWN)
			{
				iPageNum++;
				if (iPageNum > 2)
				{
					iPageNum = 0;
				}
			}
			if (listbox.key == KEY_ESC || listbox.key == NULL)
			{
				break;
			}
		}
	}

	return -1;
}
/*
int ReadMainMeterClass2(void *arg)
{
    BYTE buff[256];
	int iPage = 0;
	int iDay  = 0;
    int ipn    = 0;

	BYTE Fn = 0;
	BYTE bLen = 0;
	char *title = NULL;
	char szInput[20] = {0};
	bool (*pFunc)(BYTE *, char *) = NULL;
	static CListBoxEx ListBox(1);
    char menuBuff[17][24];
    BYTE cnt = 0;
    struct ListBoxExItem tmReadPowerMeter[] =
    {
        { menuBuff[cnt++], 0xFE, Dummy, (void*)1 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)2 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)3 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)4 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)5 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)6 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)7 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)8 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)9 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)10 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)11 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)12 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)13 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)14 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)15 },  //
        { menuBuff[cnt++], 0xFE, Dummy, (void*)16 },  //
        { NULL, 0xFF, NULL, NULL }, //
    };

	if(EditSpecBox(2,"������������:",szInput,60000,2,DATA_DEC) < 0)
	{
		return -1;
	}

    ipn = atoi(szInput);
    if (ipn>=POINT_NUM  ||  !IsPnValid(ipn))
    {
        MessageBox((char*)"��Ч������,�뷵��...", KEY_ESC, 10 * 1000);
        return -1;
    }

	if(EditSpecBox(2,"�����ѯ��?�������:",szInput,60000,2,DATA_DEC) < 0)
	{
		return -1;
	}

	iDay = atoi(szInput);
	if (iDay>10 || iDay<0)
	{
		MessageBox((char*)"��Ч��ѯ,�뷵��...", KEY_ESC, 10 * 1000);
		return -1;
	}

    while (1)
    {
        MessageBox((char*)"��ѯ��,���Ժ�...", KEY_ESC, 0);
        memset(menuBuff, 0, sizeof(menuBuff));
        WORD wLen;
        bool fValid = true;
        if ((int)arg >= 4 && (int)arg <= 7)
        {
            if ((int)arg == 4)
            {
                BYTE tmp;
                TTime tmTime;
				int iLen = 0;
				
				char *cpNameEng[] = {"��ʷ�������й�����","��ʷ�շ����й�����","��ʷ�������޹�����","��ʷ�շ����޹�����"};
				char *cpUnit[] = {"kWh","kvarh"};
				title = cpNameEng[iPage];
				
				GetCurTime(&tmTime);
				AddIntervs(tmTime, TIME_UNIT_DAY, -iDay);
				tmp = tmTime.nDay;
				buff[0] = ByteToBcd(tmp);
				tmp = tmTime.nMonth;
				buff[1] = ByteToBcd(tmp);
				tmp = tmTime.nYear % 100;
				buff[2] = ByteToBcd(tmp);
				cnt = 0;
				if (iPage == 0)
				{
					Fn = 161;
					bLen = 5;
					pFunc = Fmt14ToStr;
				}
				else if (iPage == 1)
				{
					Fn = 163;
					bLen = 5;
					pFunc = Fmt14ToStr;
				}
				else if (iPage == 2)
				{
					Fn = 162;
					bLen = 4;
					pFunc = Fmt11ToStr;
				}
				else if (iPage == 3)
				{
					Fn = 164;
					bLen = 4;
					pFunc = Fmt11ToStr;
				}
				iLen = GBReadItemEx(GB_DATACLASS2, Fn, ipn, buff, &wLen);
				if (iLen <= 0 || wLen == 0)
				{
					fValid = false;
				}
				else if (buff[0] == INVALID_DATA)
				{
					fValid = false;
				}
				else if (IsAllAByte(buff + 9, 0xEE, 5))
				{
					fValid = false;
				}
				if (!fValid)
				{
					memset(buff, INVALID_DATA, sizeof(buff));
				}

				{
					BYTE *cp = buff + 3;
					sprintf(menuBuff[cnt++], "������ţ�%d", ipn);
					char str[128];
					memset(str, 0, sizeof(str));
					sprintf(menuBuff[cnt], "����ʱ��");
					Fmt15ToStr(cp, str); // * /
					strcat(menuBuff[cnt++], str);
					cp += 6;
					memset(str, 0, sizeof(str));
					pFunc(cp, str); //
					sprintf(menuBuff[cnt++], "��: %s%s", str, cpUnit[iPage/2]);
					cp += bLen;
					memset(str, 0, sizeof(str));
					pFunc(cp, str); //
					sprintf(menuBuff[cnt++], "��: %s%s", str, cpUnit[iPage/2]);
					cp += bLen;
					memset(str, 0, sizeof(str));
					pFunc(cp, str); //
					sprintf(menuBuff[cnt++], "��: %s%s", str, cpUnit[iPage/2]);
					cp += bLen;
					memset(str, 0, sizeof(str));
					pFunc(cp, str); //
					sprintf(menuBuff[cnt++], "ƽ��%s%s", str, cpUnit[iPage/2]);
					cp += bLen;
					memset(str, 0, sizeof(str));
					pFunc(cp, str); //
					sprintf(menuBuff[cnt++], "�ȣ�%s%s", str, cpUnit[iPage/2]);
				}
				
                tmReadPowerMeter[cnt++].text = NULL;
            }
            else if ((int)arg == 5)
            {
                BYTE tmp;
                TTime tmTime;
				char *cpNameEngNo[] = {"��ʷ��һ�����޹�", "��ʷ���������޹�"};
				title = cpNameEngNo[iPage];
                GetCurTime(&tmTime);
                AddIntervs(tmTime, TIME_UNIT_DAY, -iDay);
                tmp = tmTime.nDay;
                buff[0] = ByteToBcd(tmp);
                tmp = tmTime.nMonth;
                buff[1] = ByteToBcd(tmp);
                tmp = tmTime.nYear % 100;
                buff[2] = ByteToBcd(tmp);
                cnt = 0;
				if (iPage == 0)
				{
					Fn = 165;
				}
				else if(iPage == 1)
				{
					Fn = 168;
				}
                int iLen = GBReadItemEx(GB_DATACLASS2, Fn, ipn, buff, &wLen);
                if (iLen <= 0 || wLen == 0)
                {
                    fValid = false;
                }
                else if (buff[0] == INVALID_DATA)
                {
                    fValid = false;
                }
                else if (IsAllAByte(buff + 9, 0xEE, 4))
                {
                    fValid = false;
                }
                if (!fValid)
                {
                    memset(buff, INVALID_DATA, sizeof(buff));
                }
            
                {
                    BYTE *cp = buff + 3;
					sprintf(menuBuff[cnt++], "������ţ�%d", ipn);
                    char str[128];
                    memset(str, 0, sizeof(str));
					sprintf(menuBuff[cnt], "����ʱ��");
                    Fmt15ToStr(cp, str); // * /
					strcat(menuBuff[cnt++], str);
                    cp += 6;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
                    sprintf(menuBuff[cnt++], "��: %skvarh", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
                    sprintf(menuBuff[cnt++], "��: %skvarh", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
                    sprintf(menuBuff[cnt++], "��: %skvarh", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
                    sprintf(menuBuff[cnt++], "ƽ��%skvarh", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
                    sprintf(menuBuff[cnt++], "�ȣ�%skvarh", str);
                }
                tmReadPowerMeter[cnt++].text = NULL;
            }
            else if ((int)arg == 6)
            {
                BYTE tmp;
                TTime tmTime;
				char *cpNameEngNo[] = {"��ʷ�ն������޹�", "��ʷ���������޹�"};
				title = cpNameEngNo[iPage];
                GetCurTime(&tmTime);
                AddIntervs(tmTime, TIME_UNIT_DAY, -iDay);
                tmp = tmTime.nDay;
                buff[0] = ByteToBcd(tmp);
                tmp = tmTime.nMonth;
                buff[1] = ByteToBcd(tmp);
                tmp = tmTime.nYear % 100;
                buff[2] = ByteToBcd(tmp);
                cnt = 0;
				if (iPage == 0)
				{
					Fn = 166;
				}
				else if (iPage == 1)
				{
					Fn = 167;
				}
                int iLen = GBReadItemEx(GB_DATACLASS2, Fn, ipn, buff, &wLen);
                if (iLen <= 0 || wLen == 0)
                {
                    fValid = false;
                }
                else if (buff[0] == INVALID_DATA)
                {
                    fValid = false;
                }
                else if (IsAllAByte(buff + 9, 0xEE, 4))
                {
                    fValid = false;
                }

                if (!fValid)
                {
                    memset(buff, INVALID_DATA, sizeof(buff));
                }

               
                {
                    BYTE *cp = buff + 3;
                    sprintf(menuBuff[cnt++], "������ţ�%d", ipn);
                    char str[128];
                    memset(str, 0, sizeof(str));
					sprintf(menuBuff[cnt], "����ʱ��");
                    Fmt15ToStr(cp, str); // * /
					strcat(menuBuff[cnt++], str);
                    cp += 6;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
                    sprintf(menuBuff[cnt++], "��: %skvarh", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
                    sprintf(menuBuff[cnt++], "��: %skvarh", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
                    sprintf(menuBuff[cnt++], "��: %skvarh", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
					sprintf(menuBuff[cnt++], "ƽ: %skvarh", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt11ToStr(cp, str); //
					sprintf(menuBuff[cnt++], "��: %skvarh", str);
                }

                tmReadPowerMeter[cnt++].text = NULL;
            }
            else if ((int)arg == 7)
            {
				BYTE bmenucnt = 0;
                BYTE tmp;
                TTime tmTime;
				char *cpName5[] = {"��ʷ�������й��������", "��ʷ�շ����й��������", "��ʷ�������޹��������", "��ʷ�շ����޹��������"};
				char *cpUnit5[] = {"kW", "kvar"};
				title = cpName5[iPage/2];
                GetCurTime(&tmTime);
                AddIntervs(tmTime, TIME_UNIT_DAY, -iDay);
                tmp = tmTime.nDay;
                buff[0] = ByteToBcd(tmp);
                tmp = tmTime.nMonth;
                buff[1] = ByteToBcd(tmp);
                tmp = tmTime.nYear % 100;
                buff[2] = ByteToBcd(tmp);
				if (iPage == 0 || iPage == 1)
				{
					Fn = 185;
				}
				else if (iPage == 2 || iPage == 3)
				{
					Fn = 187;
				}
				else if (iPage == 4 || iPage == 5)
				{
					Fn = 186;
				}
				else if (iPage == 6 || iPage == 7)
				{
					Fn = 188;
				}
                int iLen = GBReadItemEx(GB_DATACLASS2, Fn, ipn, buff, &wLen);
				sprintf(menuBuff[bmenucnt++], "�������: %d", ipn);
                if (iLen <= 0 || wLen == 0)
                {
                    fValid = false;
                }
                else if (buff[0] == INVALID_DATA)
                {
                    fValid = false;
                }
                else if (IsAllAByte(buff + 9, 0xEE, 4))
                {
                    fValid = false;
                }

                if (!fValid)
                {
                    memset(buff, INVALID_DATA, sizeof(buff));
                }

				BYTE *cp = NULL;
				char str[128];
                if (iPage%2 == 0)
                {
                    cp = buff + 9;
				    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "��: %s%s", str, cpUnit5[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "��: %s%s", str, cpUnit5[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
					sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "��: %s%s", str, cpUnit5[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
				}
				else if (iPage%2 == 1)
				{
					cp = buff + 9 + 21;
                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "ƽ��%s%s", str, cpUnit5[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
                    cp += 4;
                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "�ȣ�%s%s", str, cpUnit5[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
					sprintf(menuBuff[bmenucnt++], "      ");
					sprintf(menuBuff[bmenucnt++], "      ");
                }
                tmReadPowerMeter[bmenucnt++].text = NULL;//
            }
        }
        else
        {
			int		rxlen  = -1;
            DWORD	dwStartSecond = 1;
			DWORD   dwTime = 0;

            if ((int)arg == 0)
            {
				BYTE	*cp = NULL;
				BYTE	tmp = 0;
				TTime	tmTime = {0};
				char	str[128] = {0};
				char *cpName1[] = {"��ʷ�������й�����", "��ʷ�·����й�����", "��ʷ�������޹�����", "��ʷ�·����޹�����"};
				char *cpUnit[] = {"kWh", "kvarh"};
				title = cpName1[iPage];
				
				GetCurTime(&tmTime);
				AddIntervs(tmTime, TIME_UNIT_MONTH, -iDay);
				tmp = tmTime.nMonth;
				buff[0] = ByteToBcd(tmp);
				tmp = tmTime.nYear % 100;
				buff[1] = ByteToBcd(tmp);

                cnt = 0;
				if (iPage == 0)
				{
					Fn = 177;
					pFunc = Fmt14ToStr;
					bLen  = 5;
				}
				else if (iPage == 1)
				{
					Fn = 179;
					pFunc = Fmt14ToStr;
					bLen  = 5;
				}
				else if (iPage == 2)
				{
					Fn = 178;
					pFunc = Fmt11ToStr;
					bLen  = 4;
				}
				else if (iPage == 3)
				{
					Fn = 180;
					pFunc = Fmt11ToStr;
					bLen  = 4;
				}
                rxlen = GBReadItemEx(GB_DATACLASS2, Fn, ipn, buff, &wLen);

				if (rxlen <= 0 || wLen == 0)
                {
                    fValid = false;
                }
                else if (buff[0] == INVALID_DATA)
                {
                    fValid = false;
                }
                else if (IsAllAByte(buff + 9, 0xEE, 5))
                {
                    fValid = false;
                }

                if (!fValid)
                {
                    memset(buff, INVALID_DATA, sizeof(buff));
                }

				cp = buff+2+6;
				
				sprintf(menuBuff[cnt++], "������ţ�%d", ipn);
				memset(str, 0, sizeof(str));
				pFunc(cp, str); //
				sprintf(menuBuff[cnt++], "��: %s%s", str, cpUnit[iPage/2]);
				cp += bLen;
				memset(str, 0, sizeof(str));
				pFunc(cp, str); //
				sprintf(menuBuff[cnt++], "��: %s%s", str, cpUnit[iPage/2]);
				cp += bLen;
				memset(str, 0, sizeof(str));
				pFunc(cp, str); //
				sprintf(menuBuff[cnt++], "��: %s%s", str, cpUnit[iPage/2]);
				cp += bLen;
				memset(str, 0, sizeof(str));
				pFunc(cp, str); //
				sprintf(menuBuff[cnt++], "ƽ��%s%s", str, cpUnit[iPage/2]);
				cp += bLen;
				memset(str, 0, sizeof(str));
				pFunc(cp, str); //
				sprintf(menuBuff[cnt++], "�ȣ�%s%s", str, cpUnit[iPage/2]);   
				
				tmReadPowerMeter[cnt++].text = NULL;
            }
            else if ((int)arg == 1)
            {
                BYTE	bmenucnt = 0;
				BYTE	*cp = NULL;
				BYTE	tmp = 0;
				TTime	tmTime = {0};
				char	str[128] = {0};

				GetCurTime(&tmTime);
				AddIntervs(tmTime, TIME_UNIT_MONTH, -iDay);
				tmp = tmTime.nMonth;
				buff[0] = ByteToBcd(tmp);
				tmp = tmTime.nYear % 100;
				buff[1] = ByteToBcd(tmp);
				if(iPage == 0)
				{
					title = (char *)"��ʷ��һ�����޹�����";
					rxlen = GBReadItemEx(GB_DATACLASS2, 181, ipn, buff, &wLen);

					if (rxlen <= 0 || wLen == 0)
					{
						fValid = false;
					}
					else if (buff[0] == INVALID_DATA)
					{
						fValid = false;
					}
					else if (IsAllAByte(buff + 9, 0xEE, 5))
					{
						fValid = false;
					}

					if (!fValid)
					{
						memset(buff, INVALID_DATA, sizeof(buff));
					}

					cp = buff+2+6;
					
					sprintf(menuBuff[bmenucnt++], "������ţ�%d", ipn);
					memset(str, 0, sizeof(str));
					Fmt11ToStr(cp, str); //
					sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
					cp += 4;
					memset(str, 0, sizeof(str));
					Fmt11ToStr(cp, str); //
					sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
					cp += 4;
					memset(str, 0, sizeof(str));
					Fmt11ToStr(cp, str); //
					sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
					cp += 4;
					memset(str, 0, sizeof(str));
					Fmt11ToStr(cp, str); //
					sprintf(menuBuff[bmenucnt++], "ƽ��%skvarh", str);
					cp += 4;
					memset(str, 0, sizeof(str));
					Fmt11ToStr(cp, str); //
					sprintf(menuBuff[bmenucnt++], "�ȣ�%skvarh", str);
				}
				else if (iPage == 1)
				{
    				rxlen = GBReadItemEx(GB_DATACLASS2, 184, ipn, buff, &wLen);

					if (rxlen <= 0 || wLen == 0)
					{
						fValid = false;
					}
					else if (buff[0] == INVALID_DATA)
					{
						fValid = false;
					}
					else if (IsAllAByte(buff + 9, 0xEE, 5))
					{
						fValid = false;
					}

					if (!fValid)
					{
						memset(buff, INVALID_DATA, sizeof(buff));
					}
					title = (char *)"��ʷ���������޹�����";

					
					{
						cp = buff+2+6;
						
						sprintf(menuBuff[bmenucnt++], "������ţ�%d", ipn);
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "ƽ��%skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "�ȣ�%skvarh", str);
						tmReadPowerMeter[bmenucnt++].text = NULL;
					}
				}
            }
            else if ((int)arg == 2)
            {
				BYTE	bmenucnt = 0;
				BYTE	*cp = NULL;
				BYTE	tmp = 0;
				TTime	tmTime = {0};
				char	str[128] = {0};

				GetCurTime(&tmTime);
				AddIntervs(tmTime, TIME_UNIT_MONTH, -iDay);
				tmp = tmTime.nMonth;
				buff[0] = ByteToBcd(tmp);
				tmp = tmTime.nYear % 100;
				buff[1] = ByteToBcd(tmp);
				if(iPage == 0)
				{
					title = (char *)"��ʷ�¶������޹�����";
					rxlen = GBReadItemEx(GB_DATACLASS2, 182, ipn, buff, &wLen);

					if (rxlen <= 0 || wLen == 0)
					{
						fValid = false;
					}
					else if (buff[0] == INVALID_DATA)
					{
						fValid = false;
					}
					else if (IsAllAByte(buff + 9, 0xEE, 5))
					{
						fValid = false;
					}

					if (!fValid)
					{
						memset(buff, INVALID_DATA, sizeof(buff));
					}


					
					{
						cp = buff+2+6;
					
						sprintf(menuBuff[bmenucnt++], "������ţ�%d", ipn);
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "ƽ��%skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "�ȣ�%skvarh", str);
					}
				}
				else if (iPage == 1)
				{
					title = (char *)"��ʷ���������޹�����";
					rxlen = GBReadItemEx(GB_DATACLASS2, 183, ipn, buff, &wLen);

					if (rxlen <= 0 || wLen == 0)
					{
						fValid = false;
					}
					else if (buff[0] == INVALID_DATA)
					{
						fValid = false;
					}
					else if (IsAllAByte(buff + 9, 0xEE, 5))
					{
						fValid = false;
					}

					if (!fValid)
					{
						memset(buff, INVALID_DATA, sizeof(buff));
					}  
					
					{
						cp = buff+2+6;
						
						sprintf(menuBuff[bmenucnt++], "������ţ�%d", ipn);
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "��: %skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "ƽ��%skvarh", str);
						cp += 4;
						memset(str, 0, sizeof(str));
						Fmt11ToStr(cp, str); //
						sprintf(menuBuff[bmenucnt++], "�ȣ�%skvarh", str);
						cp += 4;
						tmReadPowerMeter[bmenucnt++].text = NULL;
					}
				}
            }
            else if ((int)arg == 3)
            {
				BYTE	bmenucnt = 0;
				BYTE	*cp = NULL;
				BYTE	*cptime = NULL;				
				BYTE	tmp = 0;
				TTime	tmTime = {0};
				char	str[128] = {0};
				char *cpName2[] = {"��ʷ�������й��������", "��ʷ�·����й��������", "��ʷ�������޹��������", "��ʷ�·����޹��������"};
				char *cpUnit[] = {"kW", "kvar"};
				title = cpName2[iPage/2];

				GetCurTime(&tmTime);
				AddIntervs(tmTime, TIME_UNIT_MONTH, -iDay);
				tmp = tmTime.nMonth;
				buff[0] = ByteToBcd(tmp);
				tmp = tmTime.nYear % 100;
				buff[1] = ByteToBcd(tmp);
				if (iPage == 0 || iPage == 1)
				{
					Fn = 193;
				}
				else if (iPage == 2 || iPage == 3)
				{
					Fn = 195;
				}
				else if (iPage == 4 || iPage == 5)
				{
					Fn = 194;
				}
				else if (iPage == 6 || iPage == 7)
				{
					Fn = 196;
				}

				rxlen = GBReadItemEx(GB_DATACLASS2, Fn, ipn, buff, &wLen);

				if (rxlen <= 0 || wLen == 0)
				{
					fValid = false;
				}
				else if (buff[0] == INVALID_DATA)
				{
					fValid = false;
				}
				else if (IsAllAByte(buff + 9, 0xEE, 5))
				{
					fValid = false;
				}

				if (!fValid)
				{
					memset(buff, INVALID_DATA, sizeof(buff));
				}
				sprintf(menuBuff[bmenucnt++], "�������: %d", ipn);

                if (iPage%2 == 0)
                {
                    cp = buff+2+6;
                   
                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); 
                    sprintf(menuBuff[bmenucnt++], "��: %s%s", str, cpUnit[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); 
					sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
					cp += 4;
                   
                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "��: %skW", str, cpUnit[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
                    cp += 4;

                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "��: %skW", str, cpUnit[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);


				}
				else if (iPage%2 == 1)
				{
					cp = buff+2+6+21;

                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "ƽ��%s%s", str, cpUnit[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
                    cp += 4;

                    memset(str, 0, sizeof(str));
                    Fmt23ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "�ȣ�%s%s", str, cpUnit[iPage/4]);
                    cp += 3;
                    memset(str, 0, sizeof(str));
                    Fmt17ToStr(cp, str); //
                    sprintf(menuBuff[bmenucnt++], "����ʱ��:%s", str);
					sprintf(menuBuff[bmenucnt++], "      ");
					sprintf(menuBuff[bmenucnt++], "      ");
                }
            }
        }
       
        ListBox.Show(0, title, tmReadPowerMeter, KEY_UP | KEY_DOWN << 8, 60000,false);
        if (ListBox.key == KEY_ESC || ListBox.key == KEY_NULL)
        {
            //TrigerSaveBank(BN21, 0, -1);
            break;
        }
        
		if (ListBox.key == KEY_UP)
		{
			if ((int)arg == 3 || (int)arg == 7)
			{
				iPage--;
				if (iPage < 0)
				{
					iPage = 7;
				}
			}
			else if ((int)arg == 0 || (int)arg == 4)
			{
				iPage--;
				if (iPage < 0)
				{
					iPage = 3;
				}
			}
			else
			{
				iPage--;
				if (iPage < 0)
				{
					iPage = 1;
				}
			}
		}
		if (ListBox.key == KEY_DOWN)
		{
			if ((int)arg == 3 || (int)arg == 7)
			{
				iPage++;
				if (iPage > 7)
				{
					iPage = 0;
				}
			}
			else if ((int)arg == 0 || (int)arg == 4)
			{
				iPage++;
				if (iPage > 3)
				{
					iPage = 0;
				}
			}
			else
			{
				iPage++;
				if (iPage > 1)
				{
					iPage = 0;
				}
			}
		}
    }
    return -1;
}*/



/*
int Mtr485DataClass2(void *arg)
{
	struct ListBoxExItem tmReadMpData[] =	{	{ "���й�/�޹�����", 0xFF, ReadMainMeterClass2, (void*)4 },    
												{ "��һ/�������޹�", 0xFF, ReadMainMeterClass2, (void*)5 },    
												{ "�ն�/�������޹�", 0xFF, ReadMainMeterClass2, (void*)6 },    
												{ "���й�/�޹�����", 0xFF, ReadMainMeterClass2, (void*)7 },    

												{ "���й�/�޹�����", 0xFF, ReadMainMeterClass2, (void*)0 },
												{ "��һ/�������޹�", 0xFF, ReadMainMeterClass2, (void*)1 },    
												{ "�¶�/�������޹�", 0xFF, ReadMainMeterClass2, (void*)2 },    
												{ "���й�/�޹�����", 0xFF, ReadMainMeterClass2, (void*)3 },   

												{ NULL, 0xFF, NULL, NULL }, 
											};
	CListBoxEx ListBox;

	while (1)
	{
		ListBox.Show(0, (char*)"��ʷ����", tmReadMpData, KEY_ESC, 60000);
		if (ListBox.key == KEY_NULL || ListBox.key == KEY_ESC)
		{
			return 0;
		}
	}
	return -1;
}*/

int EnergyStat(void *arg)
{
	struct ListBoxExItem tmp[] = { { (char*) "��ѹ��������������", MENU_TWOLEVEL_HAVE_NO, GetRealData, (void *) 2 },
	{ (char*) "��ǰ����ʾֵ", MENU_TWOLEVEL_HAVE_NO, GetRealData, (void *) 1 },//
	{ (char*) "�µ�ǰ���������ʱ��", MENU_TWOLEVEL_HAVE_NO, GetDemandEng, (void *) 1 },//
	//{ (char*) "��ʷ����", MENU_TWOLEVEL_HAVE_NO, Mtr485DataClass2, (void *) 1 },//
	{  NULL, 0xFF, NULL, NULL }, //
	};

	CListBoxEx listbox;
	listbox.Show(0, (char*) "����ʾֵ", tmp, KEY_ESC, 60000);

	return -1;
}


int DisplayChnMsg(void *arg)
{
	int tmpArg = (int)arg;
	char menuitem[16][32];
	//BYTE bChnMsgBuf[256];
	BYTE *pbChnMsg = NULL;
#ifdef DEBUG_DISP
	BYTE bBuf[1100] = {DT_ARRAY, 0x05, 
					   DT_STRUCT, 0x04, DT_UNSIGN, 0x01, DT_DATE_TIME_S, 0x20, 0x16, 0x11, 0x08, 0x19, 0x30, 0x00, DT_BOOL, 0x01, DT_VIS_STR, 0x01, 0x41, 
					   DT_STRUCT, 0x04, DT_UNSIGN, 0x01, DT_DATE_TIME_S, 0x20, 0x16, 0x11, 0x08, 0x19, 0x30, 0x00, DT_BOOL, 0x01, DT_VIS_STR, 0x01, 0x42,
					   DT_STRUCT, 0x04, DT_UNSIGN, 0x01, DT_DATE_TIME_S, 0x20, 0x16, 0x11, 0x08, 0x19, 0x30, 0x00, DT_BOOL, 0x01, DT_VIS_STR, 0x01, 0x43,
					   DT_STRUCT, 0x04, DT_UNSIGN, 0x01, DT_DATE_TIME_S, 0x20, 0x16, 0x11, 0x08, 0x19, 0x30, 0x00, DT_BOOL, 0x01, DT_VIS_STR, 0x01, 0x44,
					   DT_STRUCT, 0x04, DT_UNSIGN, 0x01, DT_DATE_TIME_S, 0x20, 0x16, 0x11, 0x08, 0x19, 0x30, 0x00, DT_BOOL, 0x01, DT_VIS_STR, 0x01, 0x45,
	};
	BYTE bFmtBuf[] = {DT_ARRAY, 0x05, 
		DT_STRUCT, 0x04,
		DT_UNSIGN, 
		DT_DATE_TIME_S, 
		DT_BOOL, 
		DT_VIS_STR, 200, RLV};
#else
	BYTE bBuf[1100] = {0};
#endif
	
	BYTE i = 0;
	BYTE CpyLength = 0;
	int iChnMsgNo = 0;
	char *title[2] = {"��%d����Ҫ��Ϣ","��%d����ͨ��Ϣ"};
	char MenuTitle[20] = {0};
	memset(menuitem,0,sizeof(menuitem));

	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)3},
		{menuitem[3],0xFE,Dummy,(void*)4},
		{menuitem[4],0xFE,Dummy,(void*)5},
		{menuitem[5],0xFE,Dummy,(void*)6},
		{menuitem[6],0xFE,Dummy,(void*)4},
		{menuitem[7],0xFE,Dummy,(void*)5},
		{menuitem[8],0xFE,Dummy,(void*)6},
		{menuitem[9],0xFE,Dummy,(void*)4},
		{menuitem[10],0xFE,Dummy,(void*)5},
		{menuitem[11],0xFE,Dummy,(void*)6},
		{menuitem[12],0xFE,Dummy,(void*)5},
		{menuitem[13],0xFE,Dummy,(void*)6},
		{menuitem[14],0xFE,Dummy,(void*)5},
		{menuitem[15],0xFE,Dummy,(void*)6},
		{ (char*) NULL, 0xFE, NULL, NULL }, //	
	};
	int strLength = 0;
	int LineNum = 0;
	BYTE *cp = NULL,*pbAscii =	NULL;
	BYTE bAscii = 0;
	int iRet = 0;
	WORD wFisrtPageLen = 0;
	BYTE j = 0;
	BYTE bCurDisPtr = 0;
	CListBoxEx listbox(2);
	typedef struct {
		BYTE bStructType;
		BYTE bParserNum;
		BYTE bParser1Type;
		BYTE bSerial;
		BYTE bParser2Type;
		BYTE bTime[7];
		BYTE bParser3Type;
		bool flag;
		BYTE bParser4Type;
		BYTE bTextLen;
		BYTE bText[200]; 
	}TChineseInfo;
	TChineseInfo tChnInfo;
	memset(tChnInfo.bText, 0, sizeof(tChnInfo.bText));
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	BYTE bType = 0;

	while(1)
	{
		bCurDisPtr = 0;
		
		//cp = bChnMsgBuf + 2;
		
		memset(menuitem, 0, sizeof(menuitem));
		//memset(bChnMsgBuf,0,sizeof(bChnMsgBuf));
		items[i].text = menuitem[i];
		i = 0;
		if (tmpArg == 1)
		{
#ifndef DEBUG_DISP
			if (0 > OoReadAttr(0x8004, 0x02, bBuf, &pbFmt, &wFmtLen))
			{
				MessageBox("     ����Ҫ��Ϣ!", KEY_ESC, 6000);
				return -1;
			}
			iRet = OoReadField(bBuf, pbFmt, wFmtLen, iChnMsgNo, (BYTE *)&tChnInfo, &bType);
#else
			iRet = OoReadField(bBuf, bFmtBuf, sizeof(bFmtBuf), iChnMsgNo, (BYTE *)&tChnInfo, &bType);
#endif
			pbChnMsg = tChnInfo.bText;
			strLength = strlen((char *)pbChnMsg);
			sprintf(MenuTitle,title[0],(WORD)(iChnMsgNo + 1));

			if (iRet<0 || tChnInfo.bSerial==0)
			{
				//MessageBox("����Ҫ������Ϣ!",KEY_ESC,10000);
				sprintf(menuitem[i++],"     ����Ҫ��Ϣ!");
				//return -1;
			}
		}
		else if(tmpArg == 2)
		{
#ifndef DEBUG_DISP
			if (0 > OoReadAttr(0x8003, 0x02, bBuf, &pbFmt, &wFmtLen))
			{
				MessageBox("     ����ͨ��Ϣ!", KEY_ESC, 6000);
				return -1;
			}
			iRet = OoReadField(bBuf, pbFmt, wFmtLen, iChnMsgNo, (BYTE *)&tChnInfo, &bType);
#else
			iRet = OoReadField(bBuf, bFmtBuf, sizeof(bFmtBuf), iChnMsgNo, (BYTE *)&tChnInfo, &bType);
#endif
			pbChnMsg = tChnInfo.bText;
			strLength = strlen((char *)pbChnMsg);

			sprintf(MenuTitle,title[1],(WORD)(iChnMsgNo + 1));

			if (iRet<0 || tChnInfo.bSerial==0)
			{
				//MessageBox("����ͨ������Ϣ!",KEY_ESC,10000);
				sprintf(menuitem[i++],"     ����ͨ��Ϣ!");
				//return -1;
			}
		}
		BYTE bCpyLine = 0;
		if (iRet>0 && tChnInfo.bSerial!=0)
		{
			cp = pbChnMsg;
			bCpyLine = LineNum = strLength/CHAR_NUM_LINE + 1;
			CpyLength = CHAR_NUM_LINE;
			if (LineNum > 7)
			{
				bCurDisPtr = 1;
				bCpyLine = 7;
			}
			cp +=  wFisrtPageLen * j;//CpyLength * ( 7 * j);
			BYTE *ptr = cp;
			
			for (BYTE k = 0; k < bCpyLine/*LineNum - (7 * j)*/; k++)
			{
				bAscii = 0;
				pbAscii = cp;
				CpyLength = CHAR_NUM_LINE;
				for (BYTE nu = 0; nu < CHAR_NUM_LINE; nu++)
				{
					if (*pbAscii++ < 0x80)//ascii �루�����������0x80��
					{
						bAscii++;
					}
				}
				if (bAscii%2 != 0)//������ascii��
				{
					CpyLength -= 1;
				}

				memcpy(menuitem[i],cp,CpyLength);

				if (strLength - (cp - pbChnMsg) <= 20)//���һ��
				{
					CpyLength = strLength - (cp - pbChnMsg);

					memcpy(menuitem[i],cp,CpyLength);//����ʵ�ʳ���
					if (bAscii%2 != 0 && (*cp) >= 0x80)//�������������ascci�룬�����һ���ַ�Ϊ���ֵ�һ�룬����ȥ��������ʾ��������
					{
						if (CpyLength > 0)
						{
							menuitem[i][CpyLength-1] = 0;
						}
					}
					menuitem[i++][CpyLength] = '\0';
					break;
				}
				
				cp += CpyLength;
				menuitem[i][CpyLength] = '\0';
				i++;
			}
			if (j == 0)//ֻͳ�Ƶ�һҳ��ʵ�ʳ���
			{
				wFisrtPageLen = cp - ptr;
			}
		}
		items[i].text = NULL;

		listbox.Show(0, MenuTitle, items, KEY_UP | KEY_LEFT << 8 | KEY_RIGHT << 16 |  KEY_DOWN << 24, 60000,false);
		if(listbox.key == KEY_LEFT || listbox.key == KEY_RIGHT)
		{
			if (listbox.key == KEY_RIGHT)
			{
				wFisrtPageLen = 0;
				j = 0;
				iChnMsgNo++;
				if (iChnMsgNo > 4)
				{
					iChnMsgNo = 0;
				}
			}
			else if (listbox.key == KEY_LEFT)
			{
				wFisrtPageLen = 0;
				j = 0;
				iChnMsgNo--;
				if (iChnMsgNo < 0)
				{
					iChnMsgNo = 4;
				}
			}
		}
		if (listbox.key == KEY_UP && bCurDisPtr == 1)
		{
			j = 0;
		}
		else if (listbox.key == KEY_DOWN && bCurDisPtr == 1)
		{
			j = 1;
		}
		if (listbox.key == NULL || listbox.key == KEY_ESC)
		{
			break;
		}
	}

	return -1;
}


int DisplayMssage(void *arg)
{
	struct ListBoxExItem tmp[] = {{	(char *) "���������Ҫ��Ϣ", 0xFF, DisplayChnMsg, (void *) 1},
	{ (char *) "�������������Ϣ", 0xFF, DisplayChnMsg,(void *) 2},
	{  NULL, 0xFF, NULL, NULL }, //
	};

	CListBoxEx listbox;
	listbox.Show(0,(char *)"������Ϣ",tmp,KEY_ESC,60000);

	return -1;
}
int SetMasteTransParam(void *arg);
int SetTermIPParam(void *arg);
int SetTermAddr(void *arg);
int SetTermSmsPara(void *arg);
int SetCommunicationStatePara(void *arg);
int SetVPNPara(void *arg);

/*
int MenuTransParam(void *arg)
{
//	BYTE i = 0;
	struct ListBoxExItem tmpM[] = {

		{(char *)"ͨ�ŷ�ʽ",			0xFF,	SetCommunicationStatePara,	(void*)1},
		{ (char *)"��վIP��ַ",			0xFF,	SetMasteTransParam,			(void*)0}, //
		{ (char *)"��̫������",			0xFF,	SetTermIPParam,				(void*)1}, //
		//{(char *) "�ն˵�ַ����������", 0xFF,	SetTermAddr,				(void*)2}, //
		{(char *) "��������ַ", 0xFF,	SetTermAddr,				(void*)2}, //
		{ (char *)"�������ĺ���",		0xFF,	SetTermSmsPara,				(void*)1}, //
		{(char *)"����ר������",		0xFF,	SetVPNPara,					(void*)3},
		{NULL,							0xFF,	Dummy,						(void*)3}, //
	};
	CListBoxEx listbox;

	while(1)
	{
		listbox.Show(0,"���ò���",tmpM, KEY_ESC, 60000);
		if (listbox.key == KEY_OK)
		{
			break;
		}
		else if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			return -1;
		}
	}
	
	return -1;
}*/


int SetTermAddr(void *arg)
{
	if (!InputPwd())
	{
		//MessageBox("�������",KEY_ESC,2000);
		return -1;
	}

	BYTE i = 0, j = 0;
	char menuitem[3][32];
	BYTE bBuf[32] = {0};
	BYTE bTmpBuf[32];
	char szServAddr[32] = {0};
	BYTE *p = (BYTE *)szServAddr;
	memset(menuitem, 0, sizeof(menuitem));
	CListBoxEx listbox;

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE bAddrType;
	BYTE bLogicAddr;
	BYTE bAddrLen;
	
	//��ʾ��������ַ���ն����ڷ�����
	 if (OoReadAttr(0x4001, 0x02, bBuf, NULL, NULL) < 0)
	 {
		MessageBox("��ȡ���ݿ����!", KEY_ESC, 3000);
		return -1;
	 }

	bAddrLen = bBuf[1];
	for(i; i<bAddrLen; i++)
	{
		ByteToASCII(bBuf[2+i], &p);
	}
	
	while(1)
	{
		i = 0;
		
		sprintf(menuitem[i++], "��������ַ��");  //��������ַ�����Ǳ䳤����Ҫ�����д���
		memset(bTmpBuf, 0, sizeof(bTmpBuf));
		memcpy(bTmpBuf, &szServAddr[0], 16);
		sprintf(menuitem[i++], "%s", (char *)bTmpBuf);
		memcpy(bTmpBuf, &szServAddr[16], 16);
		if (IsAllAByte(bTmpBuf, 0, sizeof(bTmpBuf)))
			sprintf(menuitem[i++], " "); //����û���
		else
			sprintf(menuitem[i++], "%s", (char *)bTmpBuf);
		tmpS[i].text = NULL;

		listbox.Show(0, "��������ַ����",tmpS,KEY_ESC | KEY_OK <<8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		
		if (listbox.key == KEY_OK && listbox.item != 0)
		{			
			char szInput[33];
			memset(szInput, 0, sizeof(szInput));
	
			sprintf(szInput, "%s", szServAddr);	 //spintf()���Զ���szInput��ĩβ����\0������szInputҪ�࿪1�ֽڣ���ֹ���

			if(EditTextBox(2, "���÷�������ַ", szInput, 60000, 32, DATA_HEX)>=0) //����ֻ����ĩβ��������Ϊ16���ƣ�������Ϊ0x0f��0x0a
			{	
				i = 0;
				j = 0;
				BYTE bLen = 0;
				memcpy(szServAddr, szInput, 32);  
				for (j=0; j<sizeof(szServAddr); j++)
				{
					if (szServAddr[j] != ' ')
					{
						bLen++;
					}
				}
					
				AsciiToByte((BYTE *)szServAddr, (WORD)bLen, (bBuf+2));
				bAddrLen = bLen/2;
				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					bBuf[0] = DT_OCT_STR;
					bBuf[1] = bAddrLen;
					if (OoWriteAttr(0x4001, 0x02, bBuf) > 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT4, -1);
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,3000);
					}
				}
			}
		}
	}

	return -1;
}

int SetTermSmsPara(void *arg)
{
	if (!InputPwd())
	{
		//MessageBox("�������",KEY_ESC,2000);
		return -1;
	}

	BYTE i = 0, j=0;
	char menuitem[5][32];
	BYTE bCenterSms[16] = {0};
	BYTE bStationSms[48] = {0};  //��վ����  array visible-string(SIZE(16)) ,��֧��3��
	char szPhone[32];
	char szSMSPhone[32];
	BYTE bBuf[400] = {0};
	BYTE *pbFmtBuf;
	WORD wFmtLen = 0;
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //

		{ NULL, 0xFF, NULL, NULL }, //
	};

	//GBReadItem(4, 4, 0, bSms, 0);
	if (OoReadAttr(0x4500, 0x04, bBuf, &pbFmtBuf, &wFmtLen) > 0)
	{	
		OoReadField(bBuf, pbFmtBuf, wFmtLen, 0x01, bCenterSms, NULL);
		OoReadField(bBuf, pbFmtBuf, wFmtLen, 0x02, bStationSms, NULL);
	}
	else
	{
		//return -1;
	}

	memset(szPhone, 0, sizeof(szPhone)) ;
	memset(szSMSPhone, 0, sizeof(szSMSPhone)) ;

	CListBoxEx listbox;
	while(1)
	{
		i = 0;
		PhoneToStr(bStationSms, 8, szPhone);
		//memcpy(szPhone, szStationSms, 16);
		sprintf(menuitem[i++], "��վ����:");
		sprintf(menuitem[i++], "%s", szPhone);

		
		PhoneToStr(bCenterSms, 8, szSMSPhone);
		//memcpy(szSMSPhone, szCenterSms, 16);
		sprintf(menuitem[i++], "���ĺ���:");
		sprintf(menuitem[i++], "%s", szSMSPhone);

		sprintf(menuitem[i++],"��������");

		tmpS[i].text = NULL;


		listbox.Show(0, "�������ĺ���",tmpS, KEY_ESC | KEY_OK <<8, 60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}

		if (listbox.key == KEY_OK)
		{
			char *pszPhone = szPhone; 
			char szInput[20];

			memset(szInput, 0x00, sizeof(szInput));
			if (listbox.item == 1)
			{
				pszPhone = szPhone;
				strcpy(szInput,pszPhone);
				if(getSoftKey("��վ����",szInput,60000,16,DATA_ASC)>=0)
				{
					for(j=0; j<16; j++)
					{
						if (szInput[j] < '0' || szInput[j] > '9')
							szInput[j] = 'f';	//��������f												
					}
					
					memcpy(pszPhone,szInput,16);
					AsciiToByte((BYTE*)pszPhone, 16, bStationSms);
					//memcpy(szStationSms, pszPhone, 16);
				}
			}
			else if (listbox.item == 3)
			{
				pszPhone = szSMSPhone;
				strcpy(szInput, pszPhone);
				if(getSoftKey("���ĺ���",szInput,60000,16,DATA_ASC)>=0)
				{
					for(j=0; j<16; j++)
					{
						if (szInput[j] < '0' || szInput[j] > '9')
							szInput[j] = 'f';	//��������f
					}

					memcpy(pszPhone, szInput, 16);	
					//memcpy(szCenterSms, pszPhone, 16);
					AsciiToByte((BYTE*)pszPhone, 16, bCenterSms);					
				}
			}
			if (listbox.item == 4)
			{
				/*if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					if (OoWriteField(bBuf, pbFmtBuf, wFmtLen, 0x01, bCenterSms)>0 &&
						OoWriteField(bBuf, pbFmtBuf, wFmtLen, 0x02, bStationSms)>0 &&
						OoWriteAttr(0x4500, 0x04, bBuf)>0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}*/
			}
		}
	}

	return -1;
}

int SetTermIPParam(void *arg)
{
	if (!InputPwd())
	{
		//MessageBox("�����������!",KEY_ESC,10000);
		return -1;
	}

	BYTE i = 0;
	CListBoxEx listboxSub;
	char menuitem[10][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 4 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	//BYTE bTermin[64];
	BYTE bTermin[30] = {0};
	//BYTE bMacAddr[6];
	BYTE bTerF8Para[8];
	char *pchConnectMode[] = {"TCP","UDP"};//���ӷ�ʽ
	char *pchWorkMode[] = {"���","�ͻ���","������"};//С��λ��
	BYTE bMode = 0;
	//ReadItemEx(BN2,PN0,0x2054,bTermin);
	//ReadItemEx(BN10,PN0,0xa150,bMacAddr);
	//ReadItemEx(BN0, PN0, 0x008f, bTerF8Para);
	BYTE bCommuCfgBuf[40];
	BYTE bTermIpInfoBuf[90];
	BYTE *pbAttribute2Fmt;
	BYTE *pbAttribute4Fmt;
	BYTE bTermIp[4] = {0};
	BYTE bTermIpMask[4] = {0};
	BYTE bTermIpGateway[4] = {0};
	BYTE bTermComm[10];
	BYTE bConnectWay;
	BYTE bWorkMode;
	WORD wFmt2Len = 0;
	WORD wFmt4Len = 0;

	if (OoReadAttr(0x4510, 0x02, bCommuCfgBuf, &pbAttribute2Fmt, &wFmt2Len)>0 &&
		OoReadAttr(0x4510, 0x04, bTermIpInfoBuf, &pbAttribute4Fmt, &wFmt4Len)>0)
	{
		OoReadField(bTermIpInfoBuf, pbAttribute4Fmt, wFmt4Len, 0x02, bTermin, NULL); //�ն�IP��ַ
		OoReadField(bTermIpInfoBuf, pbAttribute4Fmt, wFmt4Len, 0x03, bTermin+4, NULL); //��������
		OoReadField(bTermIpInfoBuf, pbAttribute4Fmt, wFmt4Len, 0x04, bTermin+8, NULL); //���ص�ַ
		OoReadField(bCommuCfgBuf, pbAttribute2Fmt, wFmt2Len, 0x04, bTermin+12, NULL); //�����˿��б�
		OoReadField(bCommuCfgBuf, pbAttribute2Fmt, wFmt2Len, 0x02, bTermin+22, NULL); //���ӷ�ʽ
		OoReadField(bCommuCfgBuf, pbAttribute2Fmt, wFmt2Len, 0x01, bTermin+23, NULL); //����ģʽ
	}
	else
	{
		//return -1;
	}
	while(1)
	{
		i = 0;
		sprintf(menuitem[i++], "�ն�IP��ַ��");
		sprintf(menuitem[i++],"   %03d.%03d.%03d.%03d",bTermin[0],bTermin[1],bTermin[2],bTermin[3]);
		sprintf(menuitem[i++], "���ص�ַ��");
		sprintf(menuitem[i++],"   %03d.%03d.%03d.%03d",bTermin[8],bTermin[9],bTermin[10],bTermin[11]);
		sprintf(menuitem[i++], "�������룺");
		sprintf(menuitem[i++],"   %03d.%03d.%03d.%03d",bTermin[4],bTermin[5],bTermin[6],bTermin[7]);
		//sprintf(menuitem[i++], "�ն�MAC��ַ��");
		//sprintf(menuitem[i++], "%02X:%02X:%02X:%02X:%02X:%02X",bMacAddr[0],bMacAddr[1],bMacAddr[2],bMacAddr[3],bMacAddr[4],bMacAddr[5]);
		sprintf(menuitem[i++], "�����˿�:%d", ByteToWord(bTermin+12));
		//bMode = (bTerF8Para[0]>>7)&0x01;
		bMode = bTermin[22];
		sprintf(menuitem[i++], "���ӷ�ʽ:%s", pchConnectMode[bMode]);
		//bMode = (bTerF8Para[0]>>4)&0x03;
		bMode = bTermin[23];
		if(bMode < 3)
		{
			sprintf(menuitem[i++], "����ģʽ:%s", pchWorkMode[bMode]);
		}
		else
		{
			sprintf(menuitem[i++], "����ģʽ:δ֪");
		}
		
		sprintf(menuitem[i++],"��������");
		tmpS[i].text = NULL;

		listboxSub.Show(0,"�ն�ͨ�ŵ�ַ",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listboxSub.key == KEY_NULL || listboxSub.key == KEY_ESC)
		{
			break;
		}

		if (listboxSub.key == KEY_OK)
		{
			BYTE *pbIpAddr = NULL; 
			if (listboxSub.item == 1)  //�ն�IP��ַ
			{
				pbIpAddr = bTermin;
			}
			else if (listboxSub.item == 3)  //���ص�ַ
			{
				pbIpAddr = &bTermin[8];
			}
			else if (listboxSub.item == 5)  //��������
			{
				pbIpAddr = &bTermin[4];
			}
			else if (listboxSub.item == 6)  //�����˿�
			{
				pbIpAddr = &bTermin[12];
			}
			else if (listboxSub.item ==7)  //���ӷ�ʽ
			{
				pbIpAddr = &bTermin[22];
			}
			else if (listboxSub.item == 8) //����ģʽ
			{
				pbIpAddr = &bTermin[23];
			}

			char szInput[32];
			if (listboxSub.item==1 || listboxSub.item==3 || listboxSub.item==5)
			{
				sprintf(szInput,"%03d.%03d.%03d.%03d",pbIpAddr[0],pbIpAddr[1],pbIpAddr[2],pbIpAddr[3]);	

				if(EditTextBoxIP(2,"����IP��ַ",szInput,60000,15,DATA_DEC)>=0)
				{
					int iAddr1,iAddr2,iAddr3,iAddr4;
					if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
					{
						*(pbIpAddr) = (BYTE)iAddr1;
						*(pbIpAddr+1) = (BYTE)iAddr2;
						*(pbIpAddr+2) = (BYTE)iAddr3;
						*(pbIpAddr+3) = (BYTE)iAddr4;
					}
				}
			}
			//else if (listboxSub.item == 6 || listboxSub.item == 7)
			//{
				
			//	sprintf(szInput,"%02X:%02X:%02X:%02X:%02X:%02X",pbIpAddr[0],pbIpAddr[1],pbIpAddr[2],pbIpAddr[3],pbIpAddr[4],pbIpAddr[5]);

			//	if(getSoftKeyMAC("����MAC��ַ",szInput,60000,18)>=0)
			//	{
			//		int iAddr1,iAddr2,iAddr3,iAddr4,iAddr5,iAddr6;
				
			//		if(sscanf(szInput,"%x:%x:%x:%x:%x:%x",&iAddr1,&iAddr2,&iAddr3,&iAddr4,&iAddr5,&iAddr6)==6)
			//		{
			//			*(pbIpAddr) = (BYTE)iAddr1;
			//			*(pbIpAddr+1) = (BYTE)iAddr2;
			//			*(pbIpAddr+2) = (BYTE)iAddr3;
			//			*(pbIpAddr+3) = (BYTE)iAddr4;
			//			*(pbIpAddr+4) = (BYTE)iAddr5;
			//			*(pbIpAddr+6) = (BYTE)iAddr6;
			//		}

			//	}

			//}
			else if (listboxSub.item == 6)
			{
				//���ö˿�
				sprintf(szInput,"%d",ByteToWord(pbIpAddr,2));
				if(EditTextBox(2,"���ö˿�",szInput,60000,5,DATA_DEC)>=0)
				{
					WORD wPort = (WORD)atoi(szInput);

					WordToByte(wPort,pbIpAddr);
				}
			}
			else if (listboxSub.item == 7)
			{
				CListBoxEx listboxTmp1;
				struct ListBoxExItem tmp1[] = { 
					{ pchConnectMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ pchConnectMode[1], 0xFF, Dummy, (void *) 0x01 },//	
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp1.Show(0, "���ӷ�ʽ", tmp1, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp1.key == KEY_OK)
				{
					bMode = (BYTE)((int)tmp1[listboxTmp1.item].arg);
					bTermin[40] = bMode;
					pbIpAddr = &bTermin[40];
				}	
			}
			else if (listboxSub.item == 8)
			{
				CListBoxEx listboxTmp2;
				struct ListBoxExItem tmp2[] = { 
					{ pchWorkMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ pchWorkMode[1], 0xFF, Dummy, (void *) 0x01 },//
					{ pchWorkMode[2], 0xFF, Dummy, (void *) 0x02 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp2.Show(0, "����ģʽ", tmp2, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp2.key == KEY_OK)
				{
					bMode = (BYTE)((int)tmp2[listboxTmp2.item].arg);
					bTermin[50] = bMode;
					pbIpAddr = &bTermin[50];
				}	
			}

			if (listboxSub.item == 9)	
			{
				/*if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					memcpy(bTermIp, bTermin, 4);
					memcpy(bTermIpMask, bTermin+4, 4);
					memcpy(bTermIpGateway, bTermin+8, 4);
					memcpy(bTermComm, bTermin+12, 10);
					bConnectWay = bTermin[22];
					bWorkMode = bTermin[23];
					if (OoWriteField(bTermIpInfoBuf, pbAttribute4Fmt, wFmt4Len, 0x02, bTermIp)>0 &&
						OoWriteField(bTermIpInfoBuf, pbAttribute4Fmt, wFmt4Len, 0x03, bTermIpMask)>0 && 
						OoWriteField(bTermIpInfoBuf, pbAttribute4Fmt, wFmt4Len, 0x04, bTermIpGateway)>0 &&
						OoWriteField(bCommuCfgBuf, pbAttribute2Fmt, wFmt2Len, 0x04, bTermComm)>0 &&
						OoWriteField(bCommuCfgBuf, pbAttribute2Fmt, wFmt2Len, 0x02, &bConnectWay)>0 &&
						OoWriteField(bCommuCfgBuf, pbAttribute2Fmt, wFmt2Len, 0x01, &bWorkMode)>0 &&
						OoWriteAttr(0x4510, 0x02, bCommuCfgBuf)>0 && OoWriteAttr(0x4510, 0x04, bTermIpInfoBuf)>0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}*/
			}
		}
	}
	return -1;
}

int SetMasteTransParam(void *arg)
{
	if (!InputPwd())
	{
		//MessageBox("�����������",KEY_ESC,3000);
		return -1;
	}

	BYTE i = 0;
	CListBoxEx listboxSub;
	char menuitem[11][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 9 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 10 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 10 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	
	//BYTE bMaster[64];
	WORD wGprsPort = 0;
	WORD wEthPort = 0;
	//memset(bMaster, 0, sizeof(bMaster));

	//ReadItemEx(BN0,PN0,0x003f,bMaster);
	BYTE bGprsBuf[140] = {0};
	BYTE bGprsMasterInfo[40] = {0};
	BYTE bGprsMasterIp[4] = {0};
	BYTE bGprsMasterComm[2];
	BYTE bAPN[16] = {0};
	BYTE bEthMasterInfo[40] = {0};
	BYTE bEthMasterIp[4] = {0};
	BYTE bEthMasterComm[2];
	BYTE *pbAttribute2Fmt;
	BYTE *pbGprsAttribute3Fmt;
	BYTE *pbEthAttribute3Fmt;
	WORD wGprsFmt2Len;
	WORD wGprsFmt3Len;
	WORD wEthFmt3Len;

	if (OoReadAttr(0x4500, 0x02, bGprsBuf, &pbAttribute2Fmt, &wGprsFmt2Len)>0 &&
		OoReadAttr(0x4510, 0x03, bEthMasterInfo, &pbEthAttribute3Fmt, &wEthFmt3Len)>0 &&
		OoReadAttr(0x4500, 0x03, bGprsMasterInfo, &pbGprsAttribute3Fmt, &wGprsFmt3Len)>0)
	{
		OoReadField(bGprsMasterInfo, pbGprsAttribute3Fmt, wGprsFmt3Len, 0x01, bGprsMasterIp, NULL); //���߹�����վIP��ַ
		OoReadField(bGprsMasterInfo, pbGprsAttribute3Fmt, wGprsFmt3Len, 0x02, bGprsMasterComm, NULL); //���߹�����վ�˿ں�
		OoReadField(bGprsBuf, pbAttribute2Fmt, wGprsFmt2Len, 0x06, bAPN, NULL); //���߹���APN
		OoReadField(bEthMasterInfo, pbEthAttribute3Fmt, wEthFmt3Len, 0x01, bEthMasterIp, NULL); //��̫����վIP��ַ
		OoReadField(bEthMasterInfo, pbEthAttribute3Fmt, wEthFmt3Len, 0x02, bEthMasterComm, NULL); //��̫����վ�˿�
	}
	else
	{
		//return -1;
	}

	while(1)
	{
		i = 0;
		wGprsPort = 0;
		wEthPort = 0;

		//wPort = bMaster[5]*256 + bMaster[4];
		wGprsPort = ByteToWord(bGprsMasterComm);
		sprintf(menuitem[i++],"    ���߹�����ʽ    ");
		sprintf(menuitem[i++], "��վIP��ַ��");
		sprintf(menuitem[i++],"   %d.%d.%d.%d",bGprsMasterIp[0],bGprsMasterIp[1],bGprsMasterIp[2],bGprsMasterIp[3]);
		sprintf(menuitem[i++],"��վ�˿�: %d",wGprsPort);
		//sprintf(menuitem[i++], "����IP��ַ��");
		//sprintf(menuitem[i++],"   %d.%d.%d.%d",bMaster[6],bMaster[7],bMaster[8],bMaster[9]);
		//wPort = bMaster[11]*256 + bMaster[10];
		//sprintf(menuitem[i++],"���ö˿�: %d",wPort);
		sprintf(menuitem[i++],"APN:");
		sprintf(menuitem[i++],"%s", bAPN);
		
		wEthPort = ByteToWord(bEthMasterComm);
		sprintf(menuitem[i++], "     ��̫����ʽ     ");
		sprintf(menuitem[i++], "��վIP��ַ��");
		sprintf(menuitem[i++],"   %d.%d.%d.%d",bEthMasterIp[0],bEthMasterIp[1],bEthMasterIp[2],bEthMasterIp[3]);
		sprintf(menuitem[i++],"��վ�˿�: %d",wEthPort);
		sprintf(menuitem[i++],"��������");
		tmpS[i].text = NULL;

		listboxSub.Show(0,"��վͨ�ŵ�ַ",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listboxSub.key == KEY_ESC || listboxSub.key == KEY_NULL)
		{
			break;
		}

		if (listboxSub.key == KEY_OK)
		{
			BYTE *pbIpAddr = NULL;
			if (listboxSub.item == 2)
			{
				pbIpAddr = bGprsMasterIp;
			}
			else if (listboxSub.item == 3)
			{
				pbIpAddr = bGprsMasterComm;
			}
			else if (listboxSub.item == 5)
			{
				pbIpAddr = bAPN;
			}
			else if (listboxSub.item == 8)
			{
				pbIpAddr = bEthMasterIp;
			}
			else if (listboxSub.item == 9)
			{
				pbIpAddr = bEthMasterComm;
			}
			//else if (listboxSub.item == 7)
			//{
				//pbIpAddr = &bMaster[12];
			//}
			char szInput[35];

			if (listboxSub.item==2 || listboxSub.item==8)
			{
				sprintf(szInput,"%03d.%03d.%03d.%03d",pbIpAddr[0],pbIpAddr[1],pbIpAddr[2],pbIpAddr[3]);	

				if(EditTextBoxIP(2,"����IP��ַ",szInput,60000,15,DATA_DEC)>=0)
				{
					int iAddr1,iAddr2,iAddr3,iAddr4;
					if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
					{
						*(pbIpAddr) = (BYTE)iAddr1;
						*(pbIpAddr+1) = (BYTE)iAddr2;
						*(pbIpAddr+2) = (BYTE)iAddr3;
						*(pbIpAddr+3) = (BYTE)iAddr4;
					}
				}
			}
			else if (listboxSub.item == 3 || listboxSub.item == 9)
			{
				//���ö˿�
				sprintf(szInput,"%d",ByteToWord(pbIpAddr,2));
				if(EditTextBox(2,"���ö˿�",szInput,60000,5,DATA_DEC)>=0)
				{
					WORD wPort = (WORD)atoi(szInput);
					WordToByte(wPort,pbIpAddr);
				}
			}
			else if (listboxSub.item == 5)
			{
				strcpy(szInput,(char*)pbIpAddr);
				if(getSoftKey("����APN",szInput,60000,15,DATA_ASC)>=0)
				{
					memcpy(pbIpAddr,szInput,15);
				}
				char *pTail = strstr((char*)pbIpAddr," ");
				if(pTail)
				{
					memset(pTail,0x00,&bAPN[15]-(BYTE*)pTail+1);
				}
			}
			if (listboxSub.item == 10)
			{
				/*if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					//if(WriteItemEx(BN0,PN0,0x003f,bMaster)> 0)
					if (OoWriteField(bGprsMasterInfo, pbGprsAttribute3Fmt, wGprsFmt3Len, 0x01, bGprsMasterIp)>0 &&\
						OoWriteField(bGprsMasterInfo, pbGprsAttribute3Fmt, wGprsFmt3Len, 0x02, bGprsMasterComm)>0 &&\
						OoWriteField(bGprsBuf, pbAttribute2Fmt, wGprsFmt2Len, 0x06, bAPN)>0 &&\
						OoWriteField(bEthMasterInfo, pbEthAttribute3Fmt, wEthFmt3Len, 0x01, bEthMasterIp)>0 &&\
						OoWriteField(bEthMasterInfo, pbEthAttribute3Fmt, wEthFmt3Len, 0x02, bEthMasterComm)>0 &&\
						OoWriteAttr(0x4500, 0x02, bGprsBuf)>0 && OoWriteAttr(0x4500, 0x03, bGprsMasterInfo)>0 &&\
						OoWriteAttr(0x4510, 0x03, bEthMasterInfo)>0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						/ *TrigerSaveBank(BN0, SECT_KEEP_PARA, -1);	//��������һ��
						DoTrigerSaveBank();* /
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}*/
			}
		}
	}

	return -1;
}


typedef struct
{
	DWORD beginTime;	//ʱ�ο�ʼʱ�䣨���ӣ�
	DWORD endTime;		//ʱ�ν���ʱ�䣨���ӣ�
}TTimeChartPara;

TTimeChartPara timeCtlField[9];

//ʱ�οز���
int PeriodCtrlPara(void *arg)
{
	/*char menuitem[16][32];
	char menuitemBk[30][32];
	BYTE i = 0;
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[13], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[14], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[15], 0xFE, Dummy, (void *) 5 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox(0);
	char cInput[20] = {0};
	int iGroupNo = 0;
	BYTE bLmtBuf[100];
	BYTE bEachNum[3] = {0};
	WORD wLen = 0;
	DWORD dwBeginTime = 0,dwEndTime = 0;
	int iPageNum = 0;
	BYTE bType;

	memset(menuitemBk, 0, sizeof(menuitemBk));

	if(EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
	{
		i = 0;
		BYTE j = 0;
		BYTE offset = 0;
		char str[20],jj = 0;
		int totalPageNum = 0;
		BYTE EachLen[3] = {0};
		sscanf(cInput,"%d",&iGroupNo);

		if (iGroupNo < 9 && iGroupNo > 0)
		{
			int iret = GBReadItemEx(GB_DATACLASS4, 41, iGroupNo, bLmtBuf, &wLen);	//F41--------------------------------for���� ѭ�� ��δ������ҳ���Ч��ʱ�οط���
			bType = bLmtBuf[0];
			for (BYTE m = 0; m < 3 && iret; m++)
			{
				if (!((bLmtBuf[0]>>m) & 1))
				{
					continue;
				}
				jj = 0;//ÿһ�׷�����ʱ��������������׷�����������
				sprintf(menuitemBk[i++],"��%d�׶�ֵ����:",m+1);
				for (j = 0; j < 8; j++)
				{
					if ((bLmtBuf[1 + offset]>>j)&1)//ʱ�κ�����ƫ��
					{
						Fmt2ToStr(&bLmtBuf[2 + offset + 2 * jj],str);
						sprintf(menuitemBk[i++], "ʱ��%d���ض�ֵ:%skW",j+1,str);
						jj++;
					}
					else
					{
						sprintf(menuitemBk[i++], "ʱ��%d���ض�ֵ:xxxxkW",j+1);
					}
				}
				offset +=  2 * jj + 1;
				EachLen[m] = jj;//����ÿһ�ַ�����ʱ����
				//if (jj > 6)
				//{
				//	bEachNum[0] = 2;
				//	totalPageNum += 2;
				//}
				//else if (jj > 0)
				//{
				//	bEachNum[0] = 1;
				//	totalPageNum += 1;
				//}
				totalPageNum++;//�����ҳ��
				
			}//------------------------------------------------------------------------------------------���� for ѭ����δ������ҳ���Ч��ʱ�οط���
			while(1)
			{
				i = 0;
				dwBeginTime = 0;
				dwEndTime = 0;

				sprintf(menuitem[i++],"�ܼ��飺%d", iGroupNo);
				memset(bLmtBuf, 0, sizeof(bLmtBuf));
				if(GBReadItemEx(GB_DATACLASS4, 18, PN0, bLmtBuf, &wLen) > 0 && bLmtBuf[0] != INVALID_DATA)		//F18 �ն˹���ʱ��
				{
					BYTE k = 0,bb = 0;

					BYTE tt = bLmtBuf[0] & 0x03;
					for(j=0; j<48; j++)
					{
						if (k < 8)	//
						{	//bb:
							bb = ( (bLmtBuf[j/4] >> ((j%4) * 2)) & 0x03);
							if (tt != bb)	//
							{
								dwEndTime = j*30;
								if ((tt==1) || (tt==2))
								{
									timeCtlField[k].beginTime = dwBeginTime;
									timeCtlField[k++].endTime = dwEndTime;
								}
								dwBeginTime = dwEndTime;
								tt = bb;
							}
						}
					}
					dwEndTime = j*30;
					if ((tt==1) || (tt==2))
					{
						timeCtlField[k].beginTime = dwBeginTime;
						timeCtlField[k++].endTime = dwEndTime;
					}
					tt = bb;
				}
				else
				{
					memset(timeCtlField, 0, sizeof(timeCtlField));
				}
				tmpS[1 + 3].text = menuitem[4];
				switch(iPageNum)
				{
					case  0:
					{
						memset(bLmtBuf, 0, sizeof(bLmtBuf));
						sprintf(menuitem[i++], "Ͷ���ִ�:");//�����ִ�
						int iLen = GBReadItemEx(GB_DATACLASS4, 45, iGroupNo, bLmtBuf, &wLen);
						if (iLen < 0 || wLen == 0 || (bLmtBuf[0] == 0))
						{
							sprintf(menuitem[i++],"x x x x x x x x");
						}
						else
						{
							char szTmp[20] = {0};
							memset(menuitem[i],0,sizeof(menuitem[i]));
							for (BYTE bi=0; bi<8; bi++)
							{
								if (bLmtBuf[0] & (1<<bi))
								{
									sprintf(szTmp, "%d ", bi+1);
									strcat(menuitem[i], szTmp);
								}
							}
							i++;
						}
					
						sprintf(menuitem[i++],"��ֵ����ϵ��:");//�ϸ� �¸�

						memset(bLmtBuf, 0, sizeof(bLmtBuf));

						if (GBReadItemEx(GB_DATACLASS4, 19, PN0, bLmtBuf, &wLen) >= 0 )	//
						{
							if( (bLmtBuf[0]&0x80) >> 7 )
								sprintf(menuitem[i++], "�¸� %d%%",((bLmtBuf[0]&0x70)>>4)*10+(bLmtBuf[0]&0x0F));
							else
								sprintf(menuitem[i++], "�ϸ� %d%%",((bLmtBuf[0]&0x70)>>4)*10+(bLmtBuf[0]&0x0F));
						}
						else
							sprintf(menuitem[i++], "xxx");

						for (j = 0; j < 2; j++)
						{
							if (timeCtlField[j].beginTime==0 && timeCtlField[j].endTime==0 )
							{
								sprintf(menuitem[i++], "ʱ��%d: xx:xx-xx:xx",j+1);
							}
							else
							{
								sprintf(menuitem[i++], "ʱ��%d: %02d:%02d-%02d:%02d", j+1,timeCtlField[j].beginTime/60, 
									timeCtlField[j].beginTime%60, timeCtlField[j].endTime/60, timeCtlField[j].endTime%60);
							}
						 }
						 tmpS[i].text = NULL;
					  }
					  //tmpS[2].text = menuitem[2];
					  //tmpS[EachLen[0] + 2].text = menuitem[EachLen[0] + 2];//��һ�� �ܼ��飬�ڶ��ж�ֵ����%d ����+2
					  //tmpS[EachLen[1] + 2].text = menuitem[EachLen[1] + 2];
					  //tmpS[EachLen[2] + 2].text = menuitem[EachLen[2] + 2];
					  break;

					case 1:
						i = 1;
						for (j = 2; j < 8; j++)
						{
							if (timeCtlField[j].beginTime==0 && timeCtlField[j].endTime==0 )
							{
								sprintf(menuitem[i++], "ʱ��%d: xx:xx-xx:xx",j+1);
							}
							else
							{
								sprintf(menuitem[i++], "ʱ��%d: %02d:%02d-%02d:%02d", j+1,timeCtlField[j].beginTime/60, 
									timeCtlField[j].beginTime%60, timeCtlField[j].endTime/60, timeCtlField[j].endTime%60);
							}
						}
						break;
					case 2:
						//if ((bType & 0x07) == 0x0)
						//{

						//	
						//	sprintf(menuitem[i++],"ʱ�οض�ֵ�������ޣ�");
						//	memset(menuitem[i], 0, sizeof(menuitem)-2*sizeof(menuitem[0]));
						//	tmpS[i].text = NULL; //���������ֿհ׵���
						//	
						//}
						
						memset(menuitem[1],0,sizeof(menuitem)-sizeof(menuitem[0]));
						memcpy(&menuitem[1], menuitemBk, 6 * sizeof(menuitemBk[0]));

					//	tmpS[EachLen[0] + 2].text = NULL; //���������ֿհ׵���
						
						break;
					case 3:
						memcpy(&menuitem[1], &menuitemBk[6], 3 * sizeof(menuitemBk[0]));
						tmpS[1 + 3].text = NULL;
						break;

					case 4:
						memset(menuitem[1],0,sizeof(menuitem)-sizeof(menuitem[0]));
						memcpy(&menuitem[1], &menuitemBk[9], 6 * sizeof(menuitemBk[0]));
					//	tmpS[EachLen[1] + 2].text = NULL;  //���������ֿհ׵���

						break;

					case 5:
						memcpy(&menuitem[1], &menuitemBk[15], 3 * sizeof(menuitemBk[0]));
						tmpS[1 + 3].text = NULL;
						break;

					case 6:
						memset(menuitem[1],0,sizeof(menuitem)-sizeof(menuitem[0]));
						memcpy(menuitem[1], menuitemBk[18], 6 * sizeof(menuitemBk[0]));
					//	tmpS[EachLen[2] + 2].text = NULL; //���������ֿհ׵���

						break;

					case 7:
						memcpy(&menuitem[1], &menuitemBk[24], 3 * sizeof(menuitemBk[0]));
						tmpS[1 + 3].text = NULL;
						break;
				 }

				listbox.Show(0, (char*)"ʱ�οز���", tmpS,  KEY_DOWN << 8 | KEY_UP << 16, 60000 );
				if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
				{
					break;
				}
				else if (listbox.key == KEY_DOWN)
				{
					iPageNum++;
					if (iPageNum > 2*totalPageNum + 1)
					{
						iPageNum = 0;
					}
				}
				else if (listbox.key == KEY_UP)
				{
					iPageNum--;
					if (iPageNum < 0)
					{
						iPageNum = 2*totalPageNum + 1;
					}
				}
			}
		}
		else 
		{
			MessageBox("�����ܼ������!",KEY_ESC,3000);
			return -1;
		}
	}*/

	return -1;
}

//���ݿز���
int RecessCtrlPara(void *arg)
{
	char menuitem[13][32];
	BYTE i = 0;
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 4 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;
	char cInput[20] = {0};
	int iGroupNo = 0;
	BYTE pbBuf[20];
	if (EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
	{
		sscanf(cInput,"%d",&iGroupNo);
		if (iGroupNo > 0 && iGroupNo < 9)
		{
			i = 0;
			BYTE *cp = pbBuf;
			BYTE LunCi;
			char str[20] = {0};
			int iret = ReadItemEx(BN0,iGroupNo,0x02af,pbBuf);
			
			sprintf(menuitem[i++],"�ܼ���: %d",iGroupNo);
			Fmt2ToStr(cp,str);
			cp += 2;
			sprintf(menuitem[i++],"���ݿض�ֵ:%skW",str);
			
			Fmt19ToStr(cp,str);
			cp += 2;
			sprintf(menuitem[i++],"��ʼʱ��:%s",str);
			
			sprintf(menuitem[i++],"����ʱ��:%.1fh",(*cp) * 0.5);
			cp += 1;

			iret = ReadItemEx(BN0,iGroupNo,0x02df,&LunCi);

			sprintf(menuitem[i++], "Ͷ���ִ�:");//�����ִ�
	//		int iLen = GBReadItemEx(GB_DATACLASS4, 45, iGroupNo, pbBuf, &wLen);
			if (iret < 0 || (LunCi == 0))
			{
				sprintf(menuitem[i++],"x x x x x x x x");
			}
			else
			{
				char szTmp[20] = {0};
				memset(menuitem[i],0,sizeof(menuitem[i]));
				for (BYTE bi=0; bi<8; bi++)
				{
					if (LunCi & (1<<bi))
					{
						sprintf(szTmp, "%d ", bi+1);
						strcat(menuitem[i], szTmp);
					}
				}
				i++;
			}
			
			char *Week[7] = {"һ","��","��","��","��","��","��"};
			memset(menuitem[i],0,sizeof(menuitem[0]));
			if((*cp) == 0)
			{
				sprintf(menuitem[i++],"������:  �ޣ�");
			}
			else
			{
				sprintf(menuitem[i++],"������:");
				sprintf(menuitem[i],"���� ");
				for (BYTE Bi = 0; Bi < 8; Bi++)
				{
					if(((*cp) >> (Bi +1)) & 0x01)
					{
						sprintf(str,"%s",Week[Bi]);
						strcat(menuitem[i],str);
					}
				}
				i++;
			}
			tmpS[i].text = NULL;

			listbox.Show(0,(char *)"���ݿز���",tmpS,KEY_ESC,60000);
			if (listbox.key == KEY_ESC || listbox.key == NULL)
			{
				return -1;
			}
		}
		else
		{
			MessageBox("�����ܼ������!",KEY_ESC,3000);
			return -1;
		}
	}
}

int StopCtrlPara(void *arg)
{
	char menuitem[11][32];

	BYTE i = 0;
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 7 }, //

		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;
	char cInput[20] = {0};
	int iGroupNo = 0;
	BYTE pbBuf[20];
	if (EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
	{
		sscanf(cInput,"%d",&iGroupNo);
		if (iGroupNo > 0 && iGroupNo < 9)
		{
			i = 0;
			BYTE *cp = pbBuf;
			BYTE LunCi;
			char str[20] = {0};
			int iret = ReadItemEx(BN0,iGroupNo,0x02cf,pbBuf);

			sprintf(menuitem[i++],"�ܼ���: %d",iGroupNo);

			Fmt20ToStr(cp,str);
			cp += 3;
			sprintf(menuitem[i++],"��ʼʱ��:%s",str);

			Fmt20ToStr(cp,str);
			cp += 3;
			sprintf(menuitem[i++],"����ʱ��:%s",str);

			Fmt2ToStr(cp,str);
			cp += 2;
			sprintf(menuitem[i++],"��ͣ�ض�ֵ:%skW",str);
			iret = ReadItemEx(BN0,iGroupNo,0x02df,&LunCi);

			sprintf(menuitem[i++], "Ͷ���ִ�:");//�����ִ�

			if (iret < 0 || (LunCi == 0))
			{
				sprintf(menuitem[i++],"x x x x x x x x");
			}
			else
			{
				char szTmp[20] = {0};
				memset(menuitem[i],0,sizeof(menuitem[i]));
				for (BYTE bi=0; bi<8; bi++)
				{
					if (LunCi & (1<<bi))
					{
						sprintf(szTmp, "%d ", bi+1);
						strcat(menuitem[i], szTmp);
					}
				}
				i++;
			}

			tmpS[i].text = NULL;

			listbox.Show(0,(char *)"Ӫҵ��ͣ�ز���",tmpS,KEY_ESC,60000);
			if (listbox.key == KEY_ESC || listbox.key == NULL)
			{
				return -1;
			}
		}
		else
		{
			MessageBox("�����ܼ������!",KEY_ESC,3000);
			return -1;
		}
	}

	return -1;
}

int PowerDecreCtrlPara(void *arg)
{
	char menuitem[11][32];

	BYTE i = 0;
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 7 }, //

		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;
	char cInput[20] = {1};
	int iGroupNo = 0;
	BYTE pbBuf[20];
	if (EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
	{
		sscanf(cInput,"%d",&iGroupNo);
		if (iGroupNo > 0 && iGroupNo < 9)
		{
			int iPageNum = 0,j = 0;
			BYTE *cp = pbBuf;
			BYTE LunCi;
			char str[20] = {0};
;
			while(1)
			{
				j = i;//��ʱ���±�����NULL�Ĳ˵���
				i = 0;
				int iret = ReadItemEx(BN0,iGroupNo,0x074f,pbBuf);
				tmpS[j].text = menuitem[j];//�ָ�������NULL�Ĳ˵���
				sprintf(menuitem[i++],"�ܼ���: %d",iGroupNo);
				switch (iPageNum)
				{
				case 0:
					cp = pbBuf;
					iret = ReadItemEx(BN0,iGroupNo,0x02df,&LunCi);

					sprintf(menuitem[i++], "Ͷ���ִ�:");//�����ִ�

					if (iret < 0 || (LunCi == 0))
					{
						sprintf(menuitem[i++],"x x x x x x x x");
					}
					else
					{
						char szTmp[20] = {0};
						memset(menuitem[i],0,sizeof(menuitem[i]));
						for (BYTE bi=0; bi<8; bi++)
						{
							if (LunCi & (1<<bi))
							{
								sprintf(szTmp, "%d ", bi+1);
								strcat(menuitem[i], szTmp);
							}
						}
						i++;
					}

					cp += 5;
					sprintf(menuitem[i++],"��һ�ָ澯ʱ��:%d min",*cp);

					cp++;
					sprintf(menuitem[i++],"�ڶ��ָ澯ʱ��:%d min",*cp);

					cp++;
					sprintf(menuitem[i++],"�����ָ澯ʱ��:%d min",*cp);

					cp++;
					sprintf(menuitem[i++],"�����ָ澯ʱ��:%d min",*cp);
					break;
				case 1:
					BYTE CurrentPower[4];
					BYTE LimitPower[4];
					cp = pbBuf;

					cp += 2;
					memset(str, 0,sizeof(str));
					Fmt4ToStr(cp, str);
					sprintf(menuitem[i++],"�¸��ظ���ϵ��:%s%%",str);

					cp += 2;
					sprintf(menuitem[i++],"����ʱ��:%.1fh",*cp * 0.5);

					iret = ReadItemEx(BN0,iGroupNo,0x084f,LimitPower);
					memset(str, 0,sizeof(str));
					Fmt2ToStr(LimitPower, str);
					sprintf(menuitem[i++],"�¸��ع��ʶ�ֵ:%skW",str);

					iret = ReadItemEx(BN0,iGroupNo,0x109f,CurrentPower);
					memset(str, 0,sizeof(str));
					Fmt2ToStr(CurrentPower, str);
					sprintf(menuitem[i++],"��ǰ����:%skW",str);
					break;
				}
				tmpS[i].text = NULL;

				listbox.Show(0,(char *)"��ǰ�����¸���",tmpS,KEY_ESC | KEY_DOWN<<8 | KEY_UP<<16,60000,false);
				if (listbox.key == KEY_ESC || listbox.key == NULL)
				{
					break;
					//return -1;
				}
				if (listbox.key == KEY_UP)
				{
					iPageNum--;
					if (iPageNum < 0)
					{
						iPageNum = 1;
					}
				}
				else if (listbox.key == KEY_DOWN)
				{
					iPageNum++;
					if (iPageNum > 1)
					{
						iPageNum = 0;
					}
				}
			}
		}
		else
		{
			MessageBox("�����ܼ������!",KEY_ESC,3000);
			return -1;
		}
	}
	return -1;
}


int MonEngCtrlPara(void *arg)
{
	char menuitem[11][32];

	BYTE i = 0;
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 7 }, //

		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;
	char cInput[20] = {0};
	int iGroupNo = 0;
	BYTE pbBuf[20];
	if (EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8)>= 0)
	{
		sscanf(cInput,"%d",&iGroupNo);
		if (iGroupNo > 0 && iGroupNo < 9)
		{
			int iPageNum = 0,j = 0;
			BYTE *cp = pbBuf;
			BYTE LunCi;
			char str[20] = {0};

			while(1)
			{
				j = i;//��ʱ���±�����NULL�Ĳ˵���
				i = 0;
				BYTE LimitEng[6];

				 ReadItemEx(BN0,iGroupNo,0x02ef,LimitEng);

				tmpS[j].text = menuitem[j];//�ָ�������NULL�Ĳ˵���
				sprintf(menuitem[i++],"�ܼ���: %d",iGroupNo);
				switch (iPageNum)
				{
				case 0:
					BYTE LimitEngFactor[2];
					BYTE TotalMonEng[21];
		
					ReadItemEx(BN0,iGroupNo,0x10df,TotalMonEng);
					sprintf(menuitem[i++],"�����ۼ��õ���:");
					Fmt03ToStr(&TotalMonEng[1],str);
					sprintf(menuitem[i++]," %s",str);

					memset(str,0,sizeof(str));
				    Fmt03ToStr(LimitEng,str);//need revise
					sprintf(menuitem[i++],"�µ�ص�����ֵ:");
					sprintf(menuitem[i++]," %s ",str);

					ReadItemEx(BN0,PN0,0x014f,LimitEngFactor);
					memset(str, 0,sizeof(str));
					Fmt4ToStr(LimitEngFactor, str);
					sprintf(menuitem[i++],"��ֵ����ϵ��:%s%%",str);

					break;
				case 1:
					
			//		BYTE LimitPower[4];

					memset(str, 0,sizeof(str));
					Fmt4ToStr(&LimitEng[4], str);
					sprintf(menuitem[i++],"��������ֵϵ��:%s%%",str);

					int iret = ReadItemEx(BN0,iGroupNo,0x030f,&LunCi);

					sprintf(menuitem[i++], "Ͷ���ִ�:");//�����ִ�

					if (iret < 0 || (LunCi == 0))
					{
						sprintf(menuitem[i++],"x x x x x x x x");
					}
					else
					{
						char szTmp[20] = {0};
						memset(menuitem[i],0,sizeof(menuitem[i]));
						for (BYTE bi=0; bi<8; bi++)
						{
							if (LunCi & (1<<bi))
							{
								sprintf(szTmp, "%d ", bi+1);
								strcat(menuitem[i], szTmp);
							}
						}
						i++;
					}

					break;
				}
				tmpS[i].text = NULL;

				listbox.Show(0,(char *)"�µ�ز���",tmpS,KEY_ESC | KEY_DOWN<<8 | KEY_UP<<16,60000,false);
				if (listbox.key == KEY_ESC || listbox.key == NULL)
				{
					break;
					//return -1;
				}
				if (listbox.key == KEY_UP)
				{
					iPageNum--;
					if (iPageNum < 0)
					{
						iPageNum = 1;
					}
				}
				else if (listbox.key == KEY_DOWN)
				{
					iPageNum++;
					if (iPageNum > 1)
					{
						iPageNum = 0;
					}
				}
			}
		}
		else
		{
			MessageBox("�����ܼ������!",KEY_ESC,3000);
			return -1;
		}
	}
	return -1;
}

int ShowKvipPara(void *arg)
{
	char menuitem[6][32];

	BYTE i = 0;
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;
	char cInput[20] = {0};
	int MpNum = 0;
	if (EditSpecBox(2, "������������:",cInput, 60000, 2,DATA_DEC)>= 0)
	{
		sscanf(cInput,"%d",&MpNum);
		if (MpNum >= 1 && MpNum < POINT_NUM)
		{
			while(1)
			{
				BYTE Kvip[12];
				ReadItemEx(BN0,MpNum,0x019f,Kvip);

				sprintf(menuitem[i++],"������: %d",MpNum);

				sprintf(menuitem[i++],"Kv: %d",ByteToDWORD(Kvip,2));

				sprintf(menuitem[i++],"Ki: %d",ByteToDWORD(Kvip+2,2));

				sprintf(menuitem[i++],"Kp: %d",ByteToDWORD(Kvip,2) * ByteToDWORD(Kvip,2));
				
				tmpS[i].text = NULL;

				listbox.Show(0,(char *)"��ѹ��������������",tmpS,KEY_ESC,60000,false);
				if (listbox.key == KEY_ESC || listbox.key == NULL)
				{
					break;
				}
			}
		}
		else
		{
			MessageBox("������������!",KEY_ESC,3000);
			return -1;
		}
	}
	return -1;
}

static const char CommPortType[][16] = { "��Ч", "����", "485I", "485II"};

//����ĵ������MpProto[]��Э�����ProtoNo[]��һһ��Ӧ��
char *MpProto[] = {"δ֪","DL/T645-1997","��������","����MK6","������IEC1107","������DLMS", \
"������","��ʢI��","DL/T645-2007","�������","����","��¡�ϱ�","A1700��","��ͨ��", \
"EMAIL��","�¹�Modbus��","������ZMC��"};
BYTE ProtoNo[] = {PROTOCOLNO_NULL, PROTOCOLNO_DLT645, /*0x02, PROTOCOLNO_EDMI, PROTOCOLNO_LANDIS, PROTOCOLNO_OSTAR, \
PROTOCOLNO_DLMS, PROTOCOLNO_WS, */PROTOCOLNO_DLT645_V07/*, PROTOCOLNO_HND, PROTOCOLNO_OSTAR, PROTOCOLNO_HL645, \
PROTOCOLNO_1107, PROTOCOLNO_HT3A, PROTOCOLNO_EMAIL, PROTOCOLNO_MODBUS, PROTOCOLNO_LANDIS_ZMC*/};

int GetMpPara(void *arg)
{
	char menuitem[5][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 4 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0,j = 0,k = 0;
	int iPageNum = 0;
	int iMpNo = 1;
	int iRead = 0;
	//	BYTE pbBuf[10];
	BYTE tmpBuf[64];
	char szInput[20];
	WORD wNum=1;
	CListBoxEx listbox;
	//TPORT_PARAM tTPORT_PARAM;
	BYTE bMtrPro;
	BYTE bTsa[17] = {0};

	char cInput[5] = {0};
	if(EditSpecBox(2, "������������:",cInput, 60000, 2,DATA_DEC)>= 0)
	{
		sscanf(cInput,"%d",&iMpNo);

		if (iMpNo >= 1 && iMpNo < POINT_NUM)
		{	
			//memcpy(tmpBuf, &wNum, sizeof(wNum));	//
			//memcpy(tmpBuf+2, &iMpNo, 2);	//
			//iRead = GBReadItemEx(GB_DATACLASS4, 10, PN0, tmpBuf, &wNum);
			//if (iRead < 0 || wNum <= 2)
			if (!IsMtrSnValid(iMpNo))
			{
				//memset(tmpBuf,0,sizeof(tmpBuf));
				MessageBox("�ò�����δ����",KEY_ESC,500);
				return -1;
			}
			//GetMeterPort(iMpNo, &tTPORT_PARAM);
			bMtrPro = GetMeterPro(iMpNo);
			GetMeterTsa(iMpNo, bTsa);
			
			while(1)
			{
				i = 0;
				for (k = 0; k < sizeof(ProtoNo); k ++)
				{
					if (ProtoNo[k] == bMtrPro)
						break;
				}
				if (k >=  sizeof(ProtoNo))
					k = 0;

				sprintf(menuitem[i++],"������:%d",iMpNo);
				i++;
				//sprintf(menuitem[i++], "�˿�: %s", CommPortType[tmpBuf[6] & 0x1F]);
				sprintf(menuitem[i++],"Э��:%s",MpProto[k]);
				sprintf(menuitem[i++],"���ַ:");
				for (k=0; k<(bTsa[0]&0x0f); k++)
				{
					sprintf(&menuitem[i][k++],"%02X", bTsa[1+k]);
				}
				i++;
				//sprintf(menuitem[i++],"���ַ:%02X%02X%02X%02X%02X%02X",tmpBuf[8+5],tmpBuf[8+4],tmpBuf[8+3],tmpBuf[8+2],tmpBuf[8+1],tmpBuf[8]);

				//memset(tmpBuf,0,sizeof(tmpBuf));
				//ReadItemEx(BN0,iMpNo,0x01df,tmpBuf);
				//sprintf(menuitem[1],"�ֱ��:%s",tmpBuf);

				tmpS[i].text = NULL;

				listbox.Show(0, (char*)"���ܱ����", tmpS, KEY_ESC, 60000);
				if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
				{
					break;
				}
			}
		}
	}

	return -1;
}


/*
int ParamReadWrite(void *arg)
{
	struct ListBoxExItem tmp[] = { 
		//{ (char*) "ʱ�οز���", MENU_ONELEVEL_HAVE_NO, PeriodCtrlPara, (void *) 1 },//
		//{ (char*) "���ݿز���", MENU_ONELEVEL_HAVE_NO, RecessCtrlPara, (void *) 1 },//
		//{ (char*) "��ͣ�ز���", MENU_ONELEVEL_HAVE_NO, StopCtrlPara, (void *) 1 },//
		//{ (char*) "�¸��ز���", MENU_ONELEVEL_HAVE_NO, PowerDecreCtrlPara, (void *) 1 },//���������ļ�
		//{ (char*) "�µ�ز���", MENU_ONELEVEL_HAVE_NO, MonEngCtrlPara, NULL }, //
		//{ (char*) "KvKiKp", MENU_ONELEVEL_HAVE_NO, ShowKvipPara, NULL }, //
		{ (char*) "���ܱ����", MENU_ONELEVEL_HAVE_NO, GetMpPara, NULL }, //
		{ (char*) "���ò���", MENU_ONELEVEL_HAVE_NO, MenuTransParam, NULL }, //

		{ NULL, MENU_ONELEVEL_HAVE_NO, NULL, NULL }, //
	};

	CListBoxEx listbox;
	while(1)
	{
		listbox.Show(0, (char*) "������ֵ", tmp, KEY_ESC, 60000);
		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}
	
	return -1;
}*/


#if 1

static bool IsMountedOK2(char *str)
{	
#ifdef SYS_LINUX

	DIR *d = opendir(str);//str====/mnt/usb
	if (d == NULL)
	{
		//DTRACE(DB_CRITICAL, ("%s no exit\n", str));
		return false;
	}
	char str2[64];
	sprintf(str2,"%s/..", str);	//str2: /mnt/usb/..
	struct stat s1;
	struct stat s2;
	lstat(str,&s1);	
	lstat(str2,&s2);
	if (s1.st_dev != s2.st_dev)
	{
		closedir(d);
		return true;
	}
	if (s1.st_ino == s2.st_ino)
	{
		closedir(d);
		return true;
	}

	closedir(d);
	return false;
#else
	return true;
#endif
}

#endif


// static bool IsMountedOK2(char *str)//str ��"/dev/sda"
// {	
// #ifdef SYS_LINUX
// 	struct stat s1;
// 	lstat(str,&s1);	
// 	if(S_ISBLK(s1.st_mode))
// 	{
// 		return true;
// 	}
// 	else
// 
// 	return false;
// #else
// 	return true;
// #endif
// }



bool IsExistUsb()
{

#ifdef SYS_LINUX
	int i;
	char str[64];
	strcpy(str, "/mnt/usb");	//strcpy(str, "/dev/mmcblk0p1"); //SD��
	for (i=0; i<10; i++)
	{	//�ȼ��U���Ƿ��Ѿ����سɹ� 
		DrawStateTask();
		if (IsMountedOK2(str))
		{
			DTRACE(DB_CRITICAL, ("IsExistUsb USB mounted OK\n"));
			break;
		}
		system("mkdir /mnt/usb");
		system("mount -t vfat /dev/sda /mnt/usb");
		system("mount -t vfat /dev/sda1 /mnt/usb");
		Sleep(1000);
		continue;
	}

	if (i == 10)
		return false;
	else
		return true;
#else
	return true;
#endif

}

bool IsExistUsb2()
{
#ifdef SYS_LINUX
	int i;
	char str[64];
	strcpy(str, "/mnt/usb");	//strcpy(str, "/dev/mmcblk0p1"); //SD��
	if (IsMountedOK2(str))
	{
		DTRACE(DB_CRITICAL, ("USB mounted OK\n"));
		return true;
	}
	return false;
#else
	return true;
#endif
}


int StatDir(char* dirList, int iBufSize)//�г�U�̸�Ŀ¼�µ�Ŀ¼
{	
	int iCnt = 0;
#ifdef SYS_LINUX

	DIR   *d;   
	struct dirent   *de; 
	struct stat s;
	const char *dname = "/mnt/usb";
	char  fdir[] = ".";
	char  gdir[] = "..";
	char  rdir[] = "recycler"; 

	char str[100];

	d = opendir(dname);		// "/mnt/usb", 
	if (d != NULL)
	{
		while((de = readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s", dname, de->d_name);
			lstat(str, &s);	
			if ((s.st_mode & S_IFMT) == S_IFDIR)
			{	//�ļ�����:�ļ���һ��Ŀ¼
				if ((memcmp(de->d_name, fdir, sizeof(fdir)) != 0) && (memcmp(de->d_name, gdir, sizeof(gdir)) != 0) && (memcmp(de->d_name, rdir, sizeof(rdir)) != 0))
				{
					strcpy(dirList+iCnt*64, de->d_name);
					iCnt++;
					if (iCnt >= iBufSize)
					{
						closedir(d);
						return iCnt;
					}
					continue;
				}
			}
		}
		closedir(d);
	}
#endif
	return iCnt;
}

int UpdateList(const char* pcList, const int iCnt)
{
	char szTemp[64];
	char szStr[128];
	WORD wLen;
	int i;
	int iNnm = iCnt;

	BYTE bMaxLine = 8;
	WORD wDirNum = iNnm/bMaxLine;

	if (iNnm%bMaxLine > 0)
		wDirNum++;

	int iSel = 0;

	char menuitem[12][64];
	struct ListBoxExItem tmp[] = { { menuitem[0], 0xFE, Dummy, (void *) 0 },//
	{ menuitem[1], 0xFE, Dummy, (void *) 1 },//
	{ menuitem[2], 0xFE, Dummy, (void *) 2 },//
	{ menuitem[3], 0xFE, Dummy, (void *) 3 },//
	{ menuitem[4], 0xFE, Dummy, (void *) 4 },//
	{ menuitem[5], 0xFE, Dummy, (void *) 5 },//
	{ menuitem[6], 0xFE, Dummy, (void *) 6 },//
	{ menuitem[7], 0xFE, Dummy, (void *) 7 },//
	{ (char*) NULL, 0xFF, NULL, NULL }, //
	};
	int curPos = 0;
	CListBoxEx listbox;
	while (1)
	{
		iSel = curPos*bMaxLine;
		if ((iSel/bMaxLine)*bMaxLine >= iNnm)
			iSel = 0;

		for (i=0; i<bMaxLine; i++)
		{
			tmp[i + 1].text = tmp[i].text = (char *) NULL;

			if (((iSel/bMaxLine)*bMaxLine+i) >= iNnm)
				break;

			memset(szTemp, 0, sizeof(szTemp));
			wLen = strlen((char *)(pcList+((iSel/bMaxLine)*bMaxLine+i)*64));

			if (wLen > 64)
				wLen = 64;
			memcpy(szTemp, (char *)(pcList+((iSel/bMaxLine)*bMaxLine+i)*64), wLen);
			sprintf(menuitem[i], "%s", szTemp);   
			tmp[i].text = menuitem[i];     
		}

		
		listbox.Show(0, NULL, tmp, KEY_ESC | (KEY_RIGHT << 8) | (KEY_LEFT << 16) | (KEY_OK<<24), 60000);

		if (listbox.key == KEY_LEFT)
		{
			curPos--;
			if (curPos < 0)
			{
				if (wDirNum > 1)
					curPos = wDirNum -1;
				else
				{
					curPos = 0;
				}
			}
		}
		else if (listbox.key == KEY_RIGHT)
		{
			curPos++;
			if (curPos >= wDirNum)
			{
				curPos = 0;
			}
		}
		else if (listbox.key == KEY_OK)
		{
			iSel = curPos*bMaxLine+listbox.item;
			char str[128];			
			sprintf(str, "Ŀ¼<%s>,�ǰ�[ȷ��],��֮[����]", menuitem[listbox.item]);			
			if (MessageBox((char *) str, KEY_ESC, 10000) > 0)
			{
				return iSel;
			}
		}
		else if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
	}

	return -1;
}


int FuncMenu(void *arg)
{
	char menuitem[2][32];
	struct ListBoxExItem tmp[] = { { "Ӧ��", 0xFE, Dummy, (void *) 55 },//
	{ "ɾ��", 0xFE, Dummy, (void *) 66 },//
	{ (char*) NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listboxs;

	listboxs.Show(0,"",tmp,KEY_ESC|KEY_OK<<8,60000);
	if (listboxs.key == KEY_ESC || KEY_NULL)
	{
		return 0;
	}
	else if (listboxs.key == KEY_OK)
	{
		return ((BYTE)((int)tmp[listboxs.item].arg));
	}

	return -1;
}


int UpdateDftList(char* pcList, int iCnt,int &iSel)
{
	char szTemp[64];
	char szStr[128];
	WORD wLen;
	int i;
	int iNnm = iCnt;

	BYTE bMaxLine = 8;
	WORD wDirNum = iNnm/bMaxLine;

	if (iNnm%bMaxLine > 0)
		wDirNum++;

	//int iSel = 0;

	char menuitem[8][64];
	struct ListBoxExItem tmp[] = { { menuitem[0], 0xFE, FuncMenu, (void *) 0 },//
	{ menuitem[1], 0xFE, FuncMenu, (void *) 1 },//
	{ menuitem[2], 0xFE, FuncMenu, (void *) 2 },//
	{ menuitem[3], 0xFE, FuncMenu, (void *) 3 },//
	{ menuitem[4], 0xFE, FuncMenu, (void *) 4 },//
	{ menuitem[5], 0xFE, FuncMenu, (void *) 5 },//
	{ menuitem[6], 0xFE, FuncMenu, (void *) 6 },//
	{ menuitem[7], 0xFE, FuncMenu, (void *) 7 },//
	{ (char*) NULL, 0xFF, NULL, NULL }, //
	};
	int curPos = 0;
	CListBoxEx listbox;
	while (1)
	{
		iSel = curPos*bMaxLine;
		if ((iSel/bMaxLine)*bMaxLine >= iNnm)
			iSel = 0;

		for (i=0; i<bMaxLine; i++)
		{
			tmp[i + 1].text = tmp[i].text = (char *) NULL;

			if (((iSel/bMaxLine)*bMaxLine+i) >= iNnm)
				break;

			memset(szTemp, 0, sizeof(szTemp));
			wLen = strlen((char *)(pcList+((iSel/bMaxLine)*bMaxLine+i)*64));

			if (wLen > 64)
				wLen = 64;
			memcpy(szTemp, (char *)(pcList+((iSel/bMaxLine)*bMaxLine+i)*64), wLen);
			sprintf(menuitem[i], "%s", szTemp);   
			tmp[i].text = menuitem[i];     
		}

		
		listbox.Show(0, NULL, tmp, KEY_ESC | (KEY_RIGHT << 8) | (KEY_LEFT << 16)|KEY_OK<<24, 60000);
		
		//return iSel;
		if (listbox.key == KEY_LEFT)
		{
			curPos--;
			if (curPos < 0)
			{
				if (wDirNum > 1)
					curPos = wDirNum -1;
				else
				{
					curPos = 0;
				}
			}
		}
		else if (listbox.key == KEY_RIGHT)
		{
			curPos++;
			if (curPos >= wDirNum)
			{
				curPos = 0;
			}
		}
		else if (listbox.key == KEY_OK)
		{
			iSel = curPos*bMaxLine+listbox.item;
			//char str[128];			
			//sprintf(str, "Ŀ¼<%s>,�ǰ�[ȷ��],��֮[����]", menuitem[listbox.item]);
			int iret = FuncMenu(0);
			if (iret == 55)
			{
				return  1;
			}
			else if (iret == 66)
			{
				return 0;
			}
			else 
				return -1;
			
			//if (MessageBox((char *) str, KEY_ESC, 10000) > 0)
			//{
			//	return iSel;
			//}
		}
		else if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
	}

	return -1;
}

int StatPostfixFile(const char *dname, char* pcList, int iBufSize, char* pcFilter)
{	
#ifdef SYS_WIN								
	return 0;
#else
	DIR   *d;
	struct dirent   *de; 
	struct stat s;

	int iCnt = 0;
	char str[100];

	d = opendir(dname);	
	if (d != NULL)
	{
		while((de = readdir(d)) != NULL)
		{
			sprintf(str, "%s/%s", dname, de->d_name);
			lstat(str, &s);				//��ȡһЩ�ļ���ص���Ϣ
			//printf("%s length %d mode %X ",de->d_name,s.st_size,s.st_mode);
			switch(s.st_mode & S_IFMT)	//S_IFMT:�ļ�����
			{	//s.st_mode:�ļ����ͺ�Ȩ����Ϣ
			case S_IFREG:			//S_IFREG:�ļ���һ����ͨ�ļ�
				if ((pcFilter == NULL) || 
					(pcFilter!=NULL && strstr(de->d_name, pcFilter)!=NULL))
				{
					strcpy(pcList+iCnt*STORE_FILE_LEN, de->d_name);
					iCnt++;
				}
				if (iCnt >= iBufSize)
				{
					closedir(d);
					return iCnt;
				}
				//printf("data file"); 
				break;
			case S_IFDIR: 
				//printf("directory"); 
				break;
			case S_IFLNK:  
				//printf("symlink -->");
				//if(readlink(str,str,100) < 0)
				//	printf("no alias");
				//else
				//	printf("\"%s\"",str);	 
				break;
			default: 
				//printf("unknown"); 
				break;
			}			
			//printf("\n");		
		}
		closedir(d);
	}

	return iCnt;
#endif
}

void CctUpdate()
{
	char str[64];
	char command[64];
	char szList[STORE_MAX_COUNT][STORE_FILE_LEN];
	int  iCnt = 0;

	MessageBox("���U����,���Ժ�!", KEY_ESC,500);

	if ( !IsExistUsb() )
	{
		MessageBox("����U��ʧ��,��������˳�!", KEY_ESC,2000);
		return ;
	}
	memset(szList, 0, sizeof(szList));
	iCnt = StatPostfixFile(CCT_USB_PATH,(char *)&szList[0], STORE_MAX_COUNT, NULL);
	if (iCnt <= 0)
	{
		MessageBox("U���²������ļ�,��������˳�!", KEY_ESC, 2000);
		return ;
	}

	int iIndex = UpdateList((char *)szList, iCnt);
	if (iIndex < 0 || iIndex >= iCnt)
		return;

	MessageBox("������,��ȴ�", KEY_ESC,500);
	MessageBox("�ز����򿽱���...", KEY_ESC,500);
	MessageBox("�벻Ҫ�γ�U��!", KEY_ESC,500);
	
	char szPathFile[80], szcommand[80];
	sprintf(szPathFile, CCT_USB_PATH"%s", szList[iIndex]);
	sprintf(szcommand, "cp -f %s /mnt/data/", szPathFile);
	system(szcommand);

	memset(szcommand, 0, sizeof(szcommand));
	memset(szPathFile, 0, sizeof(szPathFile));
	strcpy(szPathFile, "/mnt/data/");
	strcat(szPathFile, szList[iIndex]);
	sprintf(szcommand, "chmod +x %s", szPathFile);
	system(szcommand);
	Sleep(500);
	system("umount /mnt/usb");

	int f = open(szPathFile, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
	if (f >= 0)
	{
		close(f);
		WriteItemEx(BN23, PN0, 0x3022, (BYTE *)szList[iIndex]);
		MessageBox("�ز����򿽱����,������!", KEY_ESC,500);
		TrigerSavePara();				
	}
	else
		MessageBox("�ز����򿽱�ʧ��,������!", KEY_ESC,500);
}


void TermiUpdate(BYTE bType)
{

	char str[64];
	char command[64];
	char szList[STORE_MAX_COUNT][STORE_FILE_LEN];
	int  iCnt = 0;
	if (bType == 0)
	{
		MessageBox("���U����,���Ժ�!", KEY_ESC,500);

		if ( !IsExistUsb() )
		{
			MessageBox("����U��ʧ��,��������˳�!", KEY_ESC,2000);
			return ;
		}
	}
	memset(szList, 0, sizeof(szList));
	iCnt = StatDir((char *)&szList[0], STORE_MAX_COUNT);
	if (iCnt <= 0)
	{
		MessageBox("U���²�����Ŀ¼,��������˳�!", KEY_ESC, 2000);
		return ;
	}

	int iIndex = UpdateList((char *)szList, iCnt);
	if (iIndex < 0 || iIndex >= iCnt)
		return;

	MessageBox("������,��ȴ�!", KEY_ESC,500);
	MessageBox("�벻Ҫ�γ�U��!", KEY_ESC,500);

	sprintf(str, "/mnt/usb/%s", szList[iIndex]);
	memset(command, 0, sizeof(command));
	sprintf(command, "cp -f %s/update %s", str, USER_APP_PATH);
	system(command);

	memset(command, 0, sizeof(command));
	sprintf(command,"chmod +x %supdate", USER_APP_PATH);
	system(command);

	memset(command, 0 , sizeof(command));
	sprintf(command, "source %supdate %s", USER_APP_PATH, str);
	system(command);
	DWORD dwClick = GetClick();
	while(GetClick()-dwClick < 5)
		MessageBox("�벻Ҫ�γ�U��!", KEY_ESC,100);
	system("umount /mnt/usb");
	memset(command, 0, sizeof(command));
	sprintf(command, "%supdate", USER_APP_PATH);
	strcpy(str, command);
	int f = open(str, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
	if (f >= 0)
	{
		close(f);
		memset(command, 0, sizeof(command));
		sprintf(command, "rm -rf %supdate", USER_APP_PATH);
		system(command);
		MessageBox("�������!", KEY_ESC,1500);
	}
	else
	{
		MessageBox("����ʧ��!", KEY_ESC,1500);
	}
	dwClick = GetClick();
	SetInfo(INFO_HARDWARE_INIT);//��֤ͣ��ͳ�Ʋ���ͳ�ƣ������������ʱ��
	while(GetClick()-dwClick < 5)
		MessageBox("�ȴ�5���ն˸�λ!",KEY_ESC,100);

#ifdef SYS_LINUX
	system("/clou/ppp/script/ppp-off");
	Sleep(2000);
#endif
	ResetCPU();
}


void AppSysParaCfg(void)
{
	char szTemp[64];
	char command[64];
	char szList[STORE_MAX_COUNT][STORE_FILE_LEN];
	int  iCnt = 0;
	int iSel = 0;
	char *dname = USER_CFG_PATH;
	char *postfix = ".dft";
	memset(szList, 0, sizeof(szList));
	//iCnt = StatDir((char *)&szList[0], STORE_MAX_COUNT);
	iCnt = StatPostfixFile(dname, (char *)&szList[0], STORE_MAX_COUNT, postfix);
	if (iCnt <= 0)
	{
		MessageBox("�ļ�������!", KEY_ESC, 2000);
		return ;
	}

//	int iIndex = UpdateList((char *)szList, iCnt);
	int iIndex = UpdateDftList((char *)szList, iCnt, iSel);
	if (iIndex < 0 )
		return;

//	MessageBox("������,��ȴ�!", KEY_ESC,500);
//	MessageBox("�벻Ҫ�γ�U��!", KEY_ESC,500);

	int iRet = g_pmParaMgr.LoadPara(szList[iSel]);
	if (iIndex == 1)
	{
		if (iRet != 0)
		{
			if (iRet == -1)
			{
				MessageBox("�������ļ�ʧ��",KEY_ESC,1000);
			}
			else if (iRet == -2)
			{
				MessageBox("�����ļ���Ч",KEY_ESC,1000);
			}
			else if (iRet == -3)
			{
				MessageBox("�����ļ����Ȳ���",KEY_ESC,1000);
			}
			else if (iRet == -4)
			{
				MessageBox("�����ļ�̫��",KEY_ESC,1000);
			}
			else if (iRet == -5)
			{
				MessageBox("��ȡ�����ļ�����",KEY_ESC,1000);
			}
			else if (iRet == -6)
			{
				MessageBox("�����ļ�CRC��",KEY_ESC,1000);
			}
			else
			{
				MessageBox("���������ļ�����",KEY_ESC,1000);
			}
		}
		else
		{
			g_pmParaMgr.Parse();
			TrigerSave();
			MessageBox("Ӧ�ò����ɹ�",KEY_ESC,1000);
		}
	}
	else
	{
		sprintf(szTemp, USER_CFG_PATH"%s", szList[iSel]);
		unlink(szTemp);
		MessageBox("ɾ�������ɹ�",KEY_ESC,1000);
	}

	//sprintf(str, "/mnt/usb/%s", szList[iSel]);
	//sprintf(command, "cp -f %s/update /mnt/app", str);
	//system(command);

	//system("chmod +x /mnt/app/update");
	//sprintf(command, "source /mnt/app/update %s", str);
	//system(command);
	//DWORD dwClick = GetClick();
	//while(GetClick()-dwClick < 5)
	//	MessageBox("�벻Ҫ�γ�U��!", KEY_ESC,100);
	//system("umount /mnt/usb");
	//strcpy(str, "/mnt/app/update");
	//int f = open(str, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
	//if (f >= 0)
	//{
	//	close(f);
	//	system("rm -Rf /mnt/app/update");
	//	MessageBox("�������!", KEY_ESC,1500);
	//}
	//else
	//{
	//	MessageBox("����ʧ��!", KEY_ESC,1500);
	//}
	//dwClick = GetClick();
	//SetInfo(INFO_HARDWARE_INIT);//��֤ͣ��ͳ�Ʋ���ͳ�ƣ������������ʱ��
	//while(GetClick()-dwClick < 5)
	//	MessageBox("�ȴ�5���ն˸�λ!",KEY_ESC,100);

//#ifdef SYS_LINUX
//	system("/clou/ppp/script/ppp-off");
//	Sleep(2000);
//#endif
//	ResetCPU();
}

int UsbUpdate(void *arg)
{
	//if(!InputPwd())
	//	return -1;

	//char menuitem[1][32];
	//int i = 0;
	//memset(menuitem,0,sizeof(menuitem));
	//struct ListBoxExItem items[] = {
	//	{menuitem[i++],0xFE,Dummy,(void*)1},
	//	{ NULL, 0xFe, NULL, NULL }, //
	//};
	

	//CListBoxEx listbox;

	//while(1)
	//{
	//	i = 0;
	//	sprintf(menuitem[i++],"�ն˳�������");
	//	listbox.Show(0,"USB����", items, KEY_ESC | KEY_OK<<8, 60000);

	//	if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
	//	{
	//		break;
	//	}

	//	if(listbox.key == KEY_OK)		
	//	{
	//		if(listbox.item == 0)
	//		{
				//�ն˳�������
				TermiUpdate(0);
	//		}
			//else if(listbox.item == 1)
			//{
			//	//�ز�ģ������
			//	CctUpdate();
			//}
	//		break;
	//	}
	//}

	return -1;
}



int SetTermiCommPort(void *arg)
{
	char menuitem[8][32];
	BYTE i= 0;
	BYTE bTmpIndex = 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[i++],0xFE,Dummy,(void*)1},
		{menuitem[i++],0xFE,Dummy,(void*)2},
		{menuitem[i++],0xFE,Dummy,(void*)3},
		{menuitem[i++],0xFE,Dummy,(void*)4},
		{menuitem[i++],0xFE,Dummy,(void*)5},
		{menuitem[i++],0xFE,Dummy,(void*)6},
		{menuitem[i++],0xFE,Dummy,(void*)7},
		{menuitem[i++],0xFE,Dummy,(void*)8},
		{ NULL, 0xFF, NULL, NULL }, //
	};

	struct ListBoxExItem items0[] = {
		{"���������",0xFF,Dummy,(void*)1},
		{"232��1����",0xFF,Dummy,(void*)2},
		{"232��2����",0xFF,Dummy,(void*)3},
		{ NULL, 0xFF, NULL, NULL }, //
	};

	typedef struct {
		BYTE bStructType;
		BYTE bParseNum;
		BYTE bVisbleStringType;
		BYTE bStringLen;
		BYTE bCommDesc[16];
		BYTE bComdcbType;
		BYTE bBaudrate;
		BYTE bCheckBit;
		BYTE bDataBit;
		BYTE bStopBit;
		BYTE bFlowCtrl;
		BYTE bEnumType;
		BYTE bCommFun;
	}TRS232Type;

	typedef struct {
		BYTE bStructType;
		BYTE bParseNum;
		BYTE bVisbleStringType;
		BYTE bStringLen;
		BYTE bCommDesc[16];
		BYTE bComdcbType;
		BYTE bBaudrate;
		BYTE bCheckBit;
		BYTE bDataBit;
		BYTE bStopBit;
		BYTE bFlowCtrl;
	}TIRType;

	TIRType tIR;
	TRS232Type tRS232;
	CListBoxEx listbox;
	DWORD IRBaundrate;
	DWORD Baundrate232;
	char *pszBaudrate[] = { "300bps","600bps","1200bps",
						"2400bps","4800bps","7200bps",
						"9600bps","19200bps","38400bps",
						"57600bps","115200bps","����Ӧ"};
	char *pszCheckBit[] = {"��У��","��У��","żУ��",};
	char *pszDataBit[] = {"","","","","","5","6","7","8"};
	char *pszStopBit[] = {"","1","2"};
	char *pszFlowCtrl[] = {"��","Ӳ��","���"};
	char *pszCommFun[] = {"����ͨ��", "����", "����", "ͣ��"};

	while(1)
	{
		listbox.Show(0,"ά��������", items0, KEY_ESC | KEY_OK<<8, 60000);

		if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item == 0)
			{
				BYTE *pbFmt = NULL;
				WORD wFmtLen = 0;
				int j = 0;
				BYTE bTmpBaudrate = 0;
				CListBoxEx listbox1;
				memset(&tIR, 0, sizeof(TIRType));
				memset(menuitem, 0, sizeof(menuitem));
				OoReadAttr(0xF202, 0x02, (BYTE *)&tIR, &pbFmt, &wFmtLen);

				while(1)
				{
					j = 0;

					if (tIR.bBaudrate == 0xFF)
						bTmpBaudrate = 0x0B;
					else
						bTmpBaudrate = tIR.bBaudrate;

					sprintf(menuitem[j++],"����ڲ�����:%s",pszBaudrate[bTmpBaudrate]);
					sprintf(menuitem[j++],"�����У��λ:%s",pszCheckBit[tIR.bCheckBit]);
					sprintf(menuitem[j++],"���������λ:%s",pszDataBit[tIR.bDataBit]);
					sprintf(menuitem[j++],"�����ֹͣλ:%s",pszStopBit[tIR.bStopBit]);
					sprintf(menuitem[j++],"�����������:%s",pszFlowCtrl[tIR.bFlowCtrl]);
					sprintf(menuitem[j++],"��������");
					items[j].text = NULL;
					bTmpIndex = j;

					listbox1.Show(0,"���������", items, KEY_ESC | KEY_OK<<8, 60000);

					if (listbox1.key == KEY_ESC || listbox1.key == KEY_NULL)
					{	
						break;
					}
					if (listbox1.key == KEY_OK)
					{
						if (listbox1.item == 0)
						{
							CListBoxEx listboxTmp0;
							struct ListBoxExItem tmp0[] = { 
								{ pszBaudrate[0], 0xFF, Dummy, (void *) 0x00 },//
								{ pszBaudrate[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszBaudrate[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ pszBaudrate[3], 0xFF, Dummy, (void *) 0x03 },//
								{ pszBaudrate[4], 0xFF, Dummy, (void *) 0x04 },//
								{ pszBaudrate[5], 0xFF, Dummy, (void *) 0x05 },//	
								{ pszBaudrate[6], 0xFF, Dummy, (void *) 0x06 },//	
								{ pszBaudrate[7], 0xFF, Dummy, (void *) 0x07 },//
								{ pszBaudrate[8], 0xFF, Dummy, (void *) 0x08 },//
								{ pszBaudrate[9], 0xFF, Dummy, (void *) 0x09 },//
								{ pszBaudrate[10], 0xFF, Dummy, (void *) 0x0A },//
								{ pszBaudrate[11], 0xFF, Dummy, (void *) 0x0B },//
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp0.Show(0, "������", tmp0, KEY_ESC | (KEY_OK << 8), 60000);
							
							if (listboxTmp0.key == KEY_OK)
							{
								if ((BYTE)((int)tmp0[listboxTmp0.item].arg) == 0x0B)
									tIR.bBaudrate = 0xFF;
								else
									tIR.bBaudrate = (BYTE)((int)tmp0[listboxTmp0.item].arg);
							}
						}
						else if (listbox1.item == 1)
						{
							CListBoxEx listboxTmp1;
							struct ListBoxExItem tmp1[] = { 
								{ pszCheckBit[0], 0xFF, Dummy, (void *) 0x00 },//
								{ pszCheckBit[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszCheckBit[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp1.Show(0, "У��λ", tmp1, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp1.key == KEY_OK)
							{
								tIR.bCheckBit = (BYTE)((int)tmp1[listboxTmp1.item].arg);
							}
						}
						else if (listbox1.item == 2)
						{
							CListBoxEx listboxTmp2;
							struct ListBoxExItem tmp2[] = { 
								{ pszDataBit[5], 0xFF, Dummy, (void *) 0x05 },//
								{ pszDataBit[6], 0xFF, Dummy, (void *) 0x06 },//	
								{ pszDataBit[7], 0xFF, Dummy, (void *) 0x07 },//	
								{ pszDataBit[8], 0xFF, Dummy, (void *) 0x08 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp2.Show(0, "����λ", tmp2, KEY_ESC | (KEY_OK << 8), 60000);
							
							if (listboxTmp2.key == KEY_OK)
							{
								tIR.bDataBit = (BYTE)((int)tmp2[listboxTmp2.item].arg);
							}
						}
						else if (listbox1.item == 3)
						{
							CListBoxEx listboxTmp3;
							struct ListBoxExItem tmp3[] = { 
								{ pszStopBit[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszStopBit[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp3.Show(0, "ֹͣλ", tmp3, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp3.key == KEY_OK)
							{
								tIR.bStopBit = (BYTE)((int)tmp3[listboxTmp3.item].arg);
							}
						}
						else if (listbox1.item == 4)
						{
							CListBoxEx listboxTmp4;
							struct ListBoxExItem tmp4[] = { 
								{ pszFlowCtrl[0], 0xFF, Dummy, (void *) 0x00 },//
								{ pszFlowCtrl[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszFlowCtrl[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp4.Show(0, "������", tmp4, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp4.key == KEY_OK)
							{
								tIR.bFlowCtrl = (BYTE)((int)tmp4[listboxTmp4.item].arg);
							}
						}
						else if (listbox1.item == 5)
						{
							if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
							{
								if (OoWriteAttr(0xF202, 0x02, (BYTE *)&tIR) > 0)
								{
									MessageBox("���óɹ�",KEY_ESC,3000);
									TrigerSaveBank(BN0, SECT11, -1); //��������.
									DoTrigerSaveBank();
								}
								else
								{
									MessageBox("����ʧ��",KEY_ESC,2000);
								}
							}	
						}
					}
				}
			}
			else if (listbox.item == 1 || listbox.item == 2)
			{
				int j = 0;
				BYTE bPort;
				BYTE bTmpBaudrate = 0;
				CListBoxEx listbox2;
				memset(&tRS232, 0, sizeof(TRS232Type));
				memset(menuitem, 0, sizeof(menuitem));
				items[bTmpIndex].text = menuitem[bTmpIndex];
				if (listbox.item == 1)
				{
					bPort = 1;
					ReadItemEx(BANK0, PN0, 0xF200, (BYTE*)&tRS232);
				}
				else
				{	bPort = 2;
					ReadItemEx(BANK0, PN1, 0xF200, (BYTE*)&tRS232);
				}

				while(1)
				{
					j = 0;
					
					if (tRS232.bBaudrate == 0xFF)
						bTmpBaudrate = 0x0B;
					else
						bTmpBaudrate = tRS232.bBaudrate;

					sprintf(menuitem[j++],"232��%d������:%s", bPort, pszBaudrate[bTmpBaudrate]);
					sprintf(menuitem[j++],"232��%dУ��λ:%s", bPort, pszCheckBit[tRS232.bCheckBit]);
					sprintf(menuitem[j++],"232��%d����λ:%s", bPort, pszDataBit[tRS232.bDataBit]);
					sprintf(menuitem[j++],"232��%dֹͣλ:%s", bPort, pszStopBit[tRS232.bStopBit]);
					sprintf(menuitem[j++],"232��%d������:%s", bPort, pszFlowCtrl[tRS232.bFlowCtrl]);
					sprintf(menuitem[j++],"232��%d�˿ڹ���:%s", bPort, pszCommFun[tRS232.bCommFun]);
					sprintf(menuitem[j++],"��������");
					items[j].text = NULL;

					listbox2.Show(0,"232������", items, KEY_ESC | KEY_OK<<8, 60000);

					if (listbox2.key == KEY_ESC || listbox2.key == KEY_NULL)
					{	
						break;
					}
					if (listbox2.key == KEY_OK)
					{
						if (listbox2.item == 0)
						{
							CListBoxEx listboxTmp0;
							struct ListBoxExItem tmp0[] = { 
								{ pszBaudrate[0], 0xFF, Dummy, (void *) 0x00 },//
								{ pszBaudrate[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszBaudrate[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ pszBaudrate[3], 0xFF, Dummy, (void *) 0x03 },//
								{ pszBaudrate[4], 0xFF, Dummy, (void *) 0x04 },//
								{ pszBaudrate[5], 0xFF, Dummy, (void *) 0x05 },//	
								{ pszBaudrate[6], 0xFF, Dummy, (void *) 0x06 },//	
								{ pszBaudrate[7], 0xFF, Dummy, (void *) 0x07 },//
								{ pszBaudrate[8], 0xFF, Dummy, (void *) 0x08 },//
								{ pszBaudrate[9], 0xFF, Dummy, (void *) 0x09 },//
								{ pszBaudrate[10], 0xFF, Dummy, (void *) 0x0A },//
								{ pszBaudrate[11], 0xFF, Dummy, (void *) 0x0B },//
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp0.Show(0, "������", tmp0, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp0.key == KEY_OK)
							{
								if ((BYTE)((int)tmp0[listboxTmp0.item].arg) == 0x0B)
									tRS232.bBaudrate = 0xFF;
								else
									tRS232.bBaudrate = (BYTE)((int)tmp0[listboxTmp0.item].arg);
							}
						}
						else if (listbox2.item == 1)
						{
							CListBoxEx listboxTmp1;
							struct ListBoxExItem tmp1[] = { 
								{ pszCheckBit[0], 0xFF, Dummy, (void *) 0x00 },//
								{ pszCheckBit[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszCheckBit[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp1.Show(0, "У��λ", tmp1, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp1.key == KEY_OK)
							{
								tRS232.bCheckBit = (BYTE)((int)tmp1[listboxTmp1.item].arg);
							}
						}
						else if (listbox2.item == 2)
						{
							CListBoxEx listboxTmp2;
							struct ListBoxExItem tmp2[] = { 
								{ pszDataBit[5], 0xFF, Dummy, (void *) 0x05 },//
								{ pszDataBit[6], 0xFF, Dummy, (void *) 0x06 },//	
								{ pszDataBit[7], 0xFF, Dummy, (void *) 0x07 },//	
								{ pszDataBit[8], 0xFF, Dummy, (void *) 0x08 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp2.Show(0, "����λ", tmp2, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp2.key == KEY_OK)
							{
								tRS232.bDataBit = (BYTE)((int)tmp2[listboxTmp2.item].arg);
							}
						}
						else if (listbox2.item == 3)
						{
							CListBoxEx listboxTmp3;
							struct ListBoxExItem tmp3[] = { 
								{ pszStopBit[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszStopBit[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp3.Show(0, "ֹͣλ", tmp3, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp3.key == KEY_OK)
							{
								tRS232.bStopBit = (BYTE)((int)tmp3[listboxTmp3.item].arg);
							}
						}
						else if (listbox2.item == 4)
						{
							CListBoxEx listboxTmp4;
							struct ListBoxExItem tmp4[] = { 
								{ pszFlowCtrl[0], 0xFF, Dummy, (void *) 0x00 },//
								{ pszFlowCtrl[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszFlowCtrl[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp4.Show(0, "������", tmp4, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp4.key == KEY_OK)
							{
								tRS232.bFlowCtrl = (BYTE)((int)tmp4[listboxTmp4.item].arg);
							}
						}
						else if (listbox2.item == 5)
						{
							CListBoxEx listboxTmp5;
							struct ListBoxExItem tmp5[] = {
								{ pszCommFun[0], 0xFF, Dummy, (void *) 0x00 },//
								{ pszCommFun[1], 0xFF, Dummy, (void *) 0x01 },//	
								{ pszCommFun[2], 0xFF, Dummy, (void *) 0x02 },//	
								{ NULL, 0xFF, NULL, NULL }, //
							};
							listboxTmp5.Show(0, "�˿ڹ���", tmp5, KEY_ESC | (KEY_OK << 8), 60000);

							if (listboxTmp5.key == KEY_OK)
							{
								tRS232.bCommFun = (BYTE)((int)tmp5[listboxTmp5.item].arg);
							}
						}
						else if (listbox2.item == 6)
						{
							if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
							{
								int nRet;

								if (bPort == 1)
								{
									nRet = WriteItemEx(BANK0, PN0, 0xF200, (BYTE *)&tRS232);
								}
								else
								{
									nRet = WriteItemEx(BANK0, PN1, 0xF200, (BYTE *)&tRS232);
								}

								if (nRet > 0)
								{
									MessageBox("���óɹ�",KEY_ESC,3000);
									TrigerSaveBank(BN0, SECT11, -1); //��������.
									DoTrigerSaveBank();
								}
								else
								{
									MessageBox("����ʧ��",KEY_ESC,2000);
								}
							}	
						}
					}
				}

			}
		}
	}

	return -1;
}


int SetRelayOutput(void *arg)
{
	char menuitem[8][32];
	BYTE i= 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[i++],0xFE,Dummy,(void*)1},
		{menuitem[i++],0xFE,Dummy,(void*)2},
		{menuitem[i++],0xFE,Dummy,(void*)3},
		{menuitem[i++],0xFE,Dummy,(void*)4},
		{menuitem[i++],0xFE,Dummy,(void*)5},
		{menuitem[i++],0xFE,Dummy,(void*)6},
		{menuitem[i++],0xFE,Dummy,(void*)7},
		{ NULL, 0xFF, NULL, NULL }, //
	};

	CListBoxEx listbox;
	BYTE bRelayOutput;
	char *pszRelayOutput[] = {"��ƽ", "����"};
	ReadItemEx(BN1, PN0, 0x2022, (BYTE *)&bRelayOutput);
	if (bRelayOutput > 2)
		bRelayOutput = 2;

	while(1)
	{
		i = 0;
		if (bRelayOutput >= 2)
			sprintf(menuitem[i++],"�̵������: δ֪");
		else
			sprintf(menuitem[i++],"�̵������: %s", pszRelayOutput[bRelayOutput]);
		sprintf(menuitem[i++],"��������");
		items[i].text = NULL;

		listbox.Show(0,"�̵����������", items, KEY_ESC | KEY_OK<<8, 60000);
		if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item == 0)
			{
				CListBoxEx listboxTmp;
				struct ListBoxExItem tmp[] = { 
					{ pszRelayOutput[0], 0xFF, Dummy, (void *) 0x00 },//
					{ pszRelayOutput[1], 0xFF, Dummy, (void *) 0x01 },//	
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp.Show(0, "�̵��������ʽ", tmp, KEY_ESC | (KEY_OK << 8), 60000);

				if (listboxTmp.key == KEY_OK)
				{
					bRelayOutput = (BYTE)((int)tmp[listboxTmp.item].arg);
				}			
			}
			else if (listbox.item == 1)
			{
				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					if(WriteItemEx(BN1, PN0, 0x2022, (BYTE *)&bRelayOutput)> 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BN10, 0, -1); //��������.
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}
			}
		}
	}

	return -1;
}

int TermiManageOpt(void *arg)
{
	BYTE bType = (int)arg;
	CListBoxEx listbox;
	BYTE bBuf[256];
	char menuitem[8][32];
	int i = 0;
	memset(bBuf,0,sizeof(bBuf));
	memset(menuitem,0,sizeof(menuitem));
	if (bType != 4)
	{
		if(!InputPwd())
			return -1;
	}

	i = 0;
	struct ListBoxExItem items[] = {
		{menuitem[i++],0xFE,Dummy,(void*)1},
		{menuitem[i++],0xFE,Dummy,(void*)2},
		{menuitem[i++],0xFE,Dummy,(void*)3},
		{menuitem[i++],0xFE,Dummy,(void*)4},
		{menuitem[i++],0xFE,Dummy,(void*)5},
		{menuitem[i++],0xFE,Dummy,(void*)6},
		{menuitem[i++],0xFE,Dummy,(void*)7},
		{ NULL, 0xFE, NULL, NULL }, //
	};

	switch(bType)
	{
	case 0: //�����ն�
	case 1: //���ݳ�ʼ��
	case 2: //������ʼ��
	//case 3: //ȫ�������ʼ��
	//case 13://ȫ�������ʼ��
		{
			if(MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
			{
				if (bType == 0)
					SetInfo(INFO_CLASS19_METHOD_RST);
				else if(bType == 1)
					SetInfo(INFO_CLASS19_METHOD_DATA_INIT);
				else if(bType == 2)
					SetInfo(INFO_CLASS19_METHOD_RST_FACT_PARA);
				//else if(bType == 3)
					//SetInfo(INFO_RST_ALLPARA);
				//else if(bType == 17)
				//{
				//	g_dwExtCmdClick = GetClick();
				//	g_dwExtCmdFlg = FLG_FORMAT_DISK;
				//}

				MessageBox("���óɹ�,��ȴ�...",KEY_ESC,3000);
			}
		}
		break;
/*	case 2: //Һ������
		{
			SetLcdContrast();
		}
		break;
	case 3: //�ն˱��
		{
			//ʡ��������
			ReadItemEx(BN0,PN0,0x8020+0x0870,bBuf);
			//�ն˵�ַ
			ReadItemEx(BN0,PN0,0x8021+0x0870,bBuf+3);

			bool ProgramStatue = IsProgramming();
			while(1)
			{
				DWORD dwAdr = 0;
				int iDataLenth = 0;

				i = 0;
				iDataLenth = GetItemLen(BN0, 0x8021);
				if(iDataLenth<=0)
					break;
				dwAdr = ByteToDWORD(bBuf+3, iDataLenth);
				
				sprintf(menuitem[i++],"��������: %02X%02X%02X",bBuf[2],bBuf[1],bBuf[0]);
				
				sprintf(menuitem[i++],"�ն˵�ַ: ");
				sprintf(menuitem[i++],"ʮ�����Ƶ�ַ: %02X%02X%02X",bBuf[5],bBuf[4],bBuf[3]);
				sprintf(menuitem[i++],"ʮ���Ƶ�ַ: %08d",dwAdr);
				if(ProgramStatue)
					sprintf(menuitem[i++],"%s","      ��������      ");
				items[i].text = NULL;

				listbox.Show(0, "�ն˱��", items, KEY_ESC|(KEY_OK<<8), 30000);

				if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
					break;

				if(ProgramStatue && listbox.key == KEY_OK)
				{
					char szInput[32];
					BYTE* pbIpAddr = bBuf;

					if(listbox.item == 0)
					{
						//ʡ��������
						sprintf(szInput,"%02X%02X%02X",bBuf[2],bBuf[1],bBuf[0]);	

						if(getSoftKey(2,"����������",szInput,60000,6,DATA_DEC)>=0)
						{
							int iAddr1,iAddr2,iAddr3;
							if(sscanf(szInput,"%02X%02X%02X",&iAddr1,&iAddr2,&iAddr3))
							{
								*(pbIpAddr+0) = (BYTE)iAddr3;
								*(pbIpAddr+1) = (BYTE)iAddr2;
								*(pbIpAddr+2) = (BYTE)iAddr1;
							}

						}
					}
					else if(listbox.item == 2)
					{
						//�ն˵�ַ
						sprintf(szInput,"%02X%02X%02X",bBuf[5],bBuf[4],bBuf[3]);	

						if(getSoftKey(2,"�����ն˵�ַ(BIN)",szInput,60000,6,DATA_HEX)>=0)
						{
							int iAddr1,iAddr2,iAddr3;
							if(sscanf(szInput,"%02X%02X%02X",&iAddr1,&iAddr2,&iAddr3)==3)
							{
								*(pbIpAddr+3) = (BYTE)iAddr3;
								*(pbIpAddr+4) = (BYTE)iAddr2;
								*(pbIpAddr+5) = (BYTE)iAddr1;
							}

						}
					}
					else if(listbox.item == 3)
					{
						//�ն˵�ַ
						sprintf(szInput,"%08d",dwAdr);	

						if(getSoftKey(2,"�����ն˵�ַ(BCD)",szInput,60000,8,DATA_DEC)>=0)
						{
							if(sscanf(szInput,"%08d",&dwAdr)==1)
							{
								memcpy(pbIpAddr+3, &dwAdr, iDataLenth);
							}

						}
					}
					else if(listbox.item == 4)
					{
						//�������
						if(MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
						{
							if (WriteItemEx(BN0,PN0,0x8020+0x0870,bBuf)>0 &&
								WriteItemEx(BN0,PN0,0x8021+0x0870,bBuf+3)>0)
							{
								FaSavePara();
								MessageBox("���óɹ�!",KEY_ESC,3000);


							}
							else
							{
								MessageBox("����ʧ��!",KEY_ESC,2000);
							}
						}
					}
				}
			}
		}
		break;
	case 4: //�ն˰汾
		{
			int iDataLenth = 0;
			i = 0;
			memset(bBuf,0,sizeof(bBuf));

			iDataLenth = ReadItemEx(BN10, PN0, 0xa043, bBuf); //�豸���
			if(IsAllAByte(bBuf, 0, iDataLenth) || IsAllAByte(bBuf, '0', iDataLenth) || iDataLenth<=0)
				memcpy(bBuf,g_bSoftVer,8);
			sprintf(menuitem[i++],"�豸���: %s",bBuf);

			iDataLenth = ReadItemEx(BN10, PN0, 0xa042, bBuf); //���̴���
			if(IsAllAByte(bBuf, 0, iDataLenth) || iDataLenth<=0)
				memcpy(bBuf,g_bSoftVer+12,2);
			sprintf(menuitem[i++],"���̴���: %02X%02X",bBuf[1],bBuf[0]);

			sprintf(menuitem[i++],"����汾: %c%c%c%c",g_bSoftVer[8],g_bSoftVer[9],g_bSoftVer[10],g_bSoftVer[11]);
			sprintf(menuitem[i++],"��������: %02X-%02X-%02X",g_bSoftVer[17],g_bSoftVer[16],g_bSoftVer[15]);
			sprintf(menuitem[i++],"Ӳ���汾: %02X%02X",g_bSoftVer[21],g_bSoftVer[20]);
			items[i].text = NULL;

			listbox.Show(0, "�ն˰汾", items, KEY_ESC, 30000);
		}
		break;
	case 5: //�ն�ʱ������
		{
			SetTermiTime();
		}
		break;
	case 6:
		{
			SetTermiPwd();
		}
		break;
	case 8:
		{
			//�ź�ǿ�Ⱥ͵�ص���
			WORD wSigStren = GetGprsWorker()->SignStrength();
			if (wSigStren >= 100)	//TD��ģ���ź�ǿ�ȷ�ΧΪ100-199
			{
				wSigStren -= 100;
				if (wSigStren != 99)
				{
					wSigStren = (wSigStren)*31/98 + 10;
					if (wSigStren > 31)
						wSigStren = 31;
				}
			}
			if (wSigStren > 31)
				wSigStren = 0;

			//ReadItemEx(BN2, PN0, 0x1028, bBuf); //0x1028 2 ��ص�ѹ NN.NN V
			//float fVal = (float)BcdToDWORD(bBuf, 2)/100.0;
			BYTE bBatPersent = ReadBatPersent();
			
			if(!GetBatStat())
				bBatPersent = 0;
				//fVal = 0.0;
			
			i = 0;
			sprintf(menuitem[i++],"�ź�ǿ��: %d",wSigStren);
			sprintf(menuitem[i++],"��ص���: %3d%%",bBatPersent);//20140724-2
			if(IsProgramming())
			{
				ReadItemEx(BN2, PN0, 0x1028, bBuf); //0x1028 2 ��ص�ѹ NN.NN V
				float fVal = (float)BcdToDWORD(bBuf, 2)/100.0;
				sprintf(menuitem[i++],"��ص�ѹ: %.2f V",fVal);
			}
			//sprintf(menuitem[i++],"��ص���: %.2f",fVal);
			
			items[i].text = NULL;

			listbox.Show(0, "�ź�ǿ�Ⱥ͵�ص���", items, KEY_ESC, 30000);
		}
		break;*/
	case 4:
		{
			/*while(1)
			{
				char str[32];
				char str1[32];
				i = 0;
				memset(str, 0, sizeof(str));
				memset(str1, 0, sizeof(str1));
				sprintf(menuitem[i++],"��¼״̬:");

				//�ն˵�¼��ǰ״̬
				{
					WORD wState = g_pFaProtoIf->GetState();
					if (wState == IF_STATE_DORMAN)
					{	//����
						char sec[32];
						BYTE bTempBuf[16];
						DWORD dwWakeUpTime = g_pFaProtoIf->GetWakeUpTime();	//����ʱ��
						DWORD dwInterv = ByteToWord(&bTempBuf[1]); 			//����ʱ����(��), 0:��ֹ����ģʽ
						if (dwWakeUpTime > dwInterv)
							dwWakeUpTime = 0;

						strcat(str, "����,ʣ");
						sprintf(sec, "%d��",  dwWakeUpTime/60);
						strcat(str, sec);
						sprintf(sec, "%d��",  dwWakeUpTime%60);
						strcat(str, sec);
					}
					else
					{
						if (wState > 4)
							wState = 5;
						const char* cStep[] = {"����", "��λģ��", "������", 
							"��¼", "���ݴ���", "ģ���ʼ��"};
						strcat(str, cStep[wState]);
					}
					memcpy(menuitem[i++], str, 32);
				}

				WORD wSigStren = GetGprsWorker()->SignStrength();
				if (wSigStren >= 100)	//TD��ģ���ź�ǿ�ȷ�ΧΪ100-199
				{
					wSigStren -= 100;
					if (wSigStren != 99)
					{
						wSigStren = (wSigStren)*31/98 + 10;
						if (wSigStren > 31)
							wSigStren = 31;
					}
				}
				if (wSigStren > 31)
					wSigStren = 0;

				sprintf(menuitem[i++],"�ź�ǿ��: %d",wSigStren);

				sprintf(menuitem[i++],"��¼����״̬:");
				//�ն˵�¼������
				{
					int iErr = g_pFaProtoIf->GetLastErr();
					if (iErr > 4)
						iErr = 5;
					else if (iErr < 0)
						iErr = 5;
					//iErr = 6;
					const char* cStep1[]={"û�д���", "��λģ��ʧ��", "ע������ʧ��", 
						"����ʧ��", "������վʧ��", "û�д���", "δ֪"};
					strcat(str1, cStep1[iErr]);
					memcpy(menuitem[i++], str1, 32);
				}

				items[i].text = NULL;

				listbox.Show(0, "�ն˵�¼״̬", items, KEY_ESC | (KEY_OK << 8), 30000);
				if (KEY_ESC == listbox.key || NULL == listbox.key)
				{
					break;
				}
				else if (KEY_OK == listbox.key)
				{
					continue;
				}
				//ShowOnlineNode();	//С����״̬
			}*/
		}
		break;
/*	case 10:
		{
			StateMassege((void*)0);	
		}
		break;*/
	case 5:
		{
			//UsbUpdate((void *)0);
			TermiUpdate(0);
		}
		break;
	case 6:
		{
			SetTermiCommPort((void*)6);
		}
		break;
	case 7:
		{
			BYTE TmpBuff0[4];
			BYTE TmpBuff1[4];
			BYTE TmpBuff2[4];
			ReadItemEx(BN10,PN0,0xa166,TmpBuff0);
			ReadItemEx(BN10,PN0,0xa131,TmpBuff1);
			ReadItemEx(BN10,PN0,0xa132,TmpBuff2);
			while(1)
			{
				i = 0;
				char *PortSeial[] = {"�ҵ���","����"};
				char *PortFunc[] = {"�����","������","������","���޹�����װ��","�ɼ���","����485��","ά����"};
				sprintf(menuitem[i++],"485�����˳��");//0xa131

				sprintf(menuitem[i++],"%s",PortSeial[TmpBuff0[0]]);

				sprintf(menuitem[i++],"��485�ڹ���:");
				
				sprintf(menuitem[i++],"%s",PortFunc[TmpBuff1[0]]);
				sprintf(menuitem[i++],"��485�ڹ���:");

				
				sprintf(menuitem[i++],"%s",PortFunc[TmpBuff2[0]]);
				sprintf(menuitem[i++],"��������");
				listbox.Show(0, "485�ڹ�������", items, KEY_ESC | (KEY_OK << 8), 30000);

				if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
				{
					break;
				}
				if (listbox.key == KEY_OK)
				{
					if (listbox.item == 0 || listbox.item == 1)
					{
						/*CListBoxEx listboxTmp;
						struct ListBoxExItem tmp[] = { 
							{ "�ҵ���", 0xFF, Dummy, (void *) 0x00 },//
							{ "����", 0xFF, Dummy, (void *) 0x01 },//	
							{ NULL, 0xFF, NULL, NULL }, //
						};
						listboxTmp.Show(0, "485�����˳��", tmp, KEY_ESC | (KEY_OK << 8), 60000);
						if (listboxTmp.key == KEY_OK)
						{
							TmpBuff0[0] = (BYTE)((int)tmp[listboxTmp.item].arg);
						}*/
					}
					else if (listbox.item > 1 && listbox.item < 6)
					{
						CListBoxEx listboxTmp1;
						struct ListBoxExItem tmp[] = { 
							{ "�����", 0xFF, Dummy, (void *) 0x00 },//
							{ "������", 0xFF, Dummy, (void *) 0x01 },//	
							{ "������", 0xFF, Dummy, (void *) 0x02 },//
							{ "���޹�����װ��", 0xFF, Dummy, (void *) 0x03 },//	
							{ "�ɼ���", 0xFF, Dummy, (void *) 0x04 },//
							{ "����485��", 0xFF, Dummy, (void *) 0x05 },//
							{ "ά����", 0xFF, Dummy, (void *) 0x06 },//	
							{ NULL, 0xFF, NULL, NULL }, //
						};
						listboxTmp1.Show(0, "485�˿ڹ���", tmp, KEY_ESC | (KEY_OK << 8), 60000);
						if (listboxTmp1.key == KEY_ESC || listboxTmp1.key == KEY_NULL)
						{
							break;
						}
						if (listboxTmp1.key == KEY_OK)
						{
							if (listbox.item < 4)
							{
								TmpBuff1[0] = (BYTE)((int)tmp[listboxTmp1.item].arg);
							}
							else
							{
								TmpBuff2[0] = (BYTE)((int)tmp[listboxTmp1.item].arg);
							}
						}
					}
					else
					{
						if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
						{
							if(WriteItemEx(BN10,PN0,0xa166,TmpBuff0)> 0 && WriteItemEx(BN10,PN0,0xa131,TmpBuff1)> 0 \
								&& WriteItemEx(BN10,PN0,0xa132,TmpBuff2)> 0)//		
							{
								MessageBox("���óɹ�",KEY_ESC,3000);
								TrigerSaveBank(BN10, 0, -1);	//��������һ��100f
								DoTrigerSaveBank();
							}
							else
							{
								MessageBox("���óɹ�",KEY_ESC,2000);
							}
						}
					}
				}
			}
		}
		break;
	case 8:
		{
			CListBoxEx listboxTmp;
			struct ListBoxExItem tmp[] = { 
				{ "��Ч", 0xFF, Dummy, (void *) 0x00 },//
				{ "��Ч", 0xFF, Dummy, (void *) 0x01 },//	
				{ NULL, 0xFF, NULL, NULL }, //
			};
			listboxTmp.Show(0, "���Կڹ���", tmp, KEY_ESC | (KEY_OK << 8), 60000);
			bBuf[0] = 0x00;

			if (listboxTmp.key == KEY_OK)
			{
				if(listboxTmp.item == 0)
				{
					bBuf[0] = (BYTE)((int)tmp[listboxTmp.item].arg);
				}
				else if(listboxTmp.item == 1)
				{
					bBuf[0] = (BYTE)((int)tmp[listboxTmp.item].arg);
				}

				if(MessageBox("ȷ��Ҫ����?",KEY_ESC,3000) > 0)
				{
					WriteItemEx(BN1,PN0,0x2111,bBuf);

					FaSavePara();

					MessageBox("���óɹ�!",KEY_ESC,1000);
					//SetInfo(INFO_APP_RST);
				}
			}
		}
		break;
	case 9:
		{
			BYTE TmpBuf[3];
			ReadItemEx(BN2, PN0, 0x2110, TmpBuf);
			while(1)
			{
				i = 0;
				char *PortFunc[] = {"����ά��", "���ؼ���"};
				sprintf(menuitem[i++], "232�ڹ���: %s", PortFunc[TmpBuf[0]]);
				sprintf(menuitem[i++],"    ��������");
				items[i].text = NULL;
				listbox.Show(0, "232�ڹ�������", items, KEY_ESC | (KEY_OK << 8), 30000);

				if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
				{
					break;
				}
				if (listbox.key == KEY_OK)
				{
					if (listbox.item == 0)
					{
						CListBoxEx listboxTmp1;
						struct ListBoxExItem tmp[] = { 
							{ "����ά��", 0xFF, Dummy, (void *) 0x00 },//
							{ "���ؼ���", 0xFF, Dummy, (void *) 0x01 },//	
							{ NULL, 0xFF, NULL, NULL }, //
						};
						listboxTmp1.Show(0, "232�ڹ���", tmp, KEY_ESC | (KEY_OK << 8), 60000);
						if (listboxTmp1.key == KEY_ESC || listboxTmp1.key == KEY_NULL)
						{
							break;
						}
						if (listboxTmp1.key == KEY_OK)
						{
							TmpBuf[0] = (BYTE)((int)tmp[listboxTmp1.item].arg);
						}
					}
					else
					{
						if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
						{
							if(WriteItemEx(BN2, PN0, 0x2110, TmpBuf) > 0)	
							{
								MessageBox("���óɹ�",KEY_ESC,3000);
								TrigerSaveBank(BN2, 0, -1);	//��������һ��
								DoTrigerSaveBank();
							}
							else
							{
								MessageBox("���óɹ�",KEY_ESC,2000);
							}
						}
					}
				}
			}
		}
		break;
	case 12:
		AppSysParaCfg();
		break;
/*	case 18:
		{
			//�����������
			
			if(MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
			{
				SetInfo(INFO_CLR_DEMAND);

				MessageBox("���óɹ�,��ȴ�...",KEY_ESC,3000); 
			}
		}
		break;*/
	}
	return -1;
}

int TermCommpAndVersion(void *arg)
{
	CListBoxEx listbox;
	char menuitem[10][32];
	//BYTE TmpBuff[110];
	DWORD dwKeySec = 0;
	DWORD dwKeySec1 = 0;
	BYTE i = 0;
	char *title[] = {"�ն˰汾��Ϣ","�ն�ͨ����Ϣ"};
	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)3},
		{menuitem[3],0xFE,Dummy,(void*)4},
		{menuitem[4],0xFE,Dummy,(void*)5},
		{menuitem[5],0xFE,Dummy,(void*)6},
		{menuitem[6],0xFE,Dummy,(void*)7},
		{menuitem[7],0xFE,Dummy,(void*)8},
		{menuitem[8],0xFE,Dummy,(void*)9},
		{menuitem[9],0xFE,Dummy,(void*)10},
		{ NULL, 0xFE, NULL, NULL }, //
	};
	WORD wGprsHeartBeat;
	WORD wEthHeartBeat;
	BYTE bGprsHeartBeat[2];
	BYTE bEthHeartBeat[2];
	BYTE *pbGprsAttribute2Fmt;
	BYTE *pbEthAttribute2Fmt;
	BYTE bGprsBuf[140] = {0};
	BYTE bEthBuf[40] = {0};
	WORD wGprsFmt2Len;
	WORD wEthFmt2Len;

	//ReadItemEx(BN10, PN0, 0xa04f,TmpBuff);

	if ((int)arg == 0)
	{
		dwKeySec1 = GetClick();
		while(GetClick() - dwKeySec1 <= 5)
		{
			i = 0;
			sprintf(menuitem[i++],"���̴���: %C%C%C%C",g_bTermSoftVer[4],g_bTermSoftVer[5],g_bTermSoftVer[6],g_bTermSoftVer[7]);
			//sprintf(menuitem[i++],"�豸���: %s",&TmpBuff[8]);
			sprintf(menuitem[i++],"����汾: %c%c%c%c",g_bTermSoftVer[10],g_bTermSoftVer[11],g_bTermSoftVer[12],g_bTermSoftVer[13]);
			sprintf(menuitem[i++],"�������: %c%c-%c%c-%c%c",g_bTermSoftVer[16],g_bTermSoftVer[17],g_bTermSoftVer[18],g_bTermSoftVer[19],g_bTermSoftVer[20],g_bTermSoftVer[21]);
			sprintf(menuitem[i++],"Ӳ���汾: %c%c%c%c",g_bTermSoftVer[24],g_bTermSoftVer[25],g_bTermSoftVer[26],g_bTermSoftVer[27]);
			sprintf(menuitem[i++],"Ӳ������: %c%c-%c%c-%c%c",g_bTermSoftVer[30],g_bTermSoftVer[31],g_bTermSoftVer[32],g_bTermSoftVer[33],g_bTermSoftVer[34],g_bTermSoftVer[35]);
			items[i].text = NULL;

			listbox.Show(0,title[(int)arg], items, KEY_ESC|(KEY_OK << 8), 60000,false);
			if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			{
				break;
			}
			
			
			if (listbox.key == KEY_OK)
			{
				BYTE bSoftInfo[25] = {0};
				CListBoxEx listboxTmp1;
				struct ListBoxExItem tmp[] = { 
					{menuitem[0],0xFE,Dummy,(void*)1},
					{menuitem[1],0xFE,Dummy,(void*)2},
					{menuitem[2],0xFE,Dummy,(void*)3},
					{ NULL, 0xFF, NULL, NULL }, //
				};
			
				
				ReadItemEx(BN2, PN0, 0x2107, bSoftInfo);
				dwKeySec = GetClick();
				
				Sleep(1000); //��ʱһ�²�����ȷ��ȡ������
				
				while (GetKey().key == KEY_OK)
				//while (GetKey() == KEY_OK)
				{					
					Sleep(200);
					if (GetClick() - dwKeySec >= 2)	//ȷ�ϼ���ʱ2s������ʾ�ڲ��汾��Ϣ
					{
						sprintf(menuitem[0],"����: %s",bSoftInfo);
						sprintf(menuitem[1],"�汾��: %c%c%c%c",bSoftInfo[16],bSoftInfo[17],bSoftInfo[18],bSoftInfo[19]);
						sprintf(menuitem[2],"��������: %02X-%02X-%02X",bSoftInfo[22],bSoftInfo[21],bSoftInfo[20]);
						listboxTmp1.Show(0, "�ڲ��汾��Ϣ", tmp, KEY_ESC, 20000, false);
						break;
					}
				}
			}
		}
	}
	else if ((int)arg == 1)
	{
		//sprintf(menuitem[i++],"�������룺%02X%02X",TmpBuff[1],TmpBuff[0]);
		//sprintf(menuitem[i++],"�ն˵�ַ(10)��%d",TmpBuff[3]*256+TmpBuff[2]);
		//sprintf(menuitem[i++],"�ն˵�ַ(16)��%02x%02x",TmpBuff[3], TmpBuff[2]);
		//ReadItemEx(BN0, PN0, 0x001f,TmpBuff);
		if (OoReadAttr(0x4500, 0x02, bGprsBuf, &pbGprsAttribute2Fmt, &wGprsFmt2Len)>0 &&
			OoReadAttr(0x4510, 0x02, bEthBuf, &pbEthAttribute2Fmt, &wEthFmt2Len)>0)
		{
			OoReadField(bGprsBuf, pbGprsAttribute2Fmt, wGprsFmt2Len, 0x0C, bGprsHeartBeat, NULL);
			OoReadField(bEthBuf, pbEthAttribute2Fmt, wEthFmt2Len, 0x0C, bEthHeartBeat, NULL);
		}
		else
		{
			//return -1;
		}
		wGprsHeartBeat = ByteToWord(bGprsHeartBeat);
		wEthHeartBeat = ByteToWord(bEthHeartBeat);
		sprintf(menuitem[i++],"���߹����������ڣ�");
		sprintf(menuitem[i++],"%d����",wGprsHeartBeat);
		sprintf(menuitem[i++],"��̫���������ڣ�");
		sprintf(menuitem[i++],"%d����",wEthHeartBeat);
		//TmpBuff[3] = TmpBuff[3]&0x0f;
		//sprintf(menuitem[i++],"�����ӳ٣�%d ms",TmpBuff[0]*20);
		//ReadItemEx(BN0, 2, 0x021f, TmpBuff);
		//if (TmpBuff[9] == 0 || TmpBuff[9] > 60)
			//TmpBuff[9] = 15;

		//sprintf(menuitem[i++],"485-1��������:%d����",TmpBuff[9]);
		//ReadItemEx(BN0, 3, 0x021f, TmpBuff);
		//if (TmpBuff[9] == 0 || TmpBuff[9] > 60)
			//TmpBuff[9] = 15;

		//sprintf(menuitem[i++],"485-2��������:%d����",TmpBuff[9]);
		//GetCurTime(&tmNow);
		//sprintf(menuitem[i++],"�ն����ڣ�%02d-%02d-%02d",tmNow.nYear, tmNow.nMonth, tmNow.nDay);
		items[i].text = NULL;

		listbox.Show(0,title[(int)arg], items, KEY_ESC, 60000);
	}

	return -1;
}
int IPAddrInfo(void *arg)
{
	char menuitem[20][32];
	BYTE i = 0;


	WORD wGprsPort = 0;
	WORD wEthPort = 0;
	BYTE bGprsBuf[140] = {0};
	BYTE bGprsMasterInfo[40] = {0};
	BYTE bGprsMasterIp[4] = {0};
	BYTE bGprsMasterComm[2];
	BYTE bAPN[16] = {0};
	BYTE bGprsUserName[32] = {0};
	BYTE bGprsUserPwd[32] = {0};
	BYTE bEthMasterInfo[40] = {0};
	BYTE bEthMasterIp[4] = {0};
	BYTE bEthMasterComm[2];
	BYTE bBuf[18] = {0};
	BYTE bTmpBuf[35];
	char szServAddr[32] = {0}; 
	BYTE bAddrLen;
	BYTE *pbAttribute2Fmt;
	BYTE *pbGprsAttribute3Fmt;
	BYTE *pbEthAttribute3Fmt;
	WORD wGprsFmt2Len;
	WORD wGprsFmt3Len;
	WORD wEthFmt3Len;

	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)3},
		{menuitem[3],0xFE,Dummy,(void*)4},
		{menuitem[4],0xFE,Dummy,(void*)5},
		{menuitem[5],0xFE,Dummy,(void*)6},
		{menuitem[6],0xFE,Dummy,(void*)7},
		{menuitem[7],0xFE,Dummy,(void*)6},
		{menuitem[8],0xFE,Dummy,(void*)7},
		{menuitem[9],0xFE,Dummy,(void*)8},
		{menuitem[10],0xFE,Dummy,(void*)9},
		{menuitem[11],0xFE,Dummy,(void*)10},
		{menuitem[12],0xFE,Dummy,(void*)11},
		{menuitem[13],0xFE,Dummy,(void*)12},
		{menuitem[14],0xFE,Dummy,(void*)13},
		{menuitem[15],0xFE,Dummy,(void*)14},
		{menuitem[16],0xFE,Dummy,(void*)15},
		{menuitem[17],0xFE,Dummy,(void*)16},
		{menuitem[18],0xFE,Dummy,(void*)17},
		{menuitem[19],0xFE,Dummy,(void*)18},
		{ NULL, 0xFE, NULL, NULL }, //
	};
	
	if (OoReadAttr(0x4500, 0x02, bGprsBuf, &pbAttribute2Fmt, &wGprsFmt2Len)>0 &&
		OoReadAttr(0x4510, 0x03, bEthMasterInfo, &pbEthAttribute3Fmt, &wEthFmt3Len)>0 &&
		OoReadAttr(0x4500, 0x03, bGprsMasterInfo, &pbGprsAttribute3Fmt, &wGprsFmt3Len)>0 && 
		OoReadAttr(0x4001, 0x02, bBuf, NULL, NULL))
	{
		OoReadField(bGprsMasterInfo, pbGprsAttribute3Fmt, wGprsFmt3Len, 0x01, bGprsMasterIp, NULL); //���߹�����վIP��ַ
		OoReadField(bGprsMasterInfo, pbGprsAttribute3Fmt, wGprsFmt3Len, 0x02, bGprsMasterComm, NULL); //���߹�����վ�˿ں�
		OoReadField(bGprsBuf, pbAttribute2Fmt, wGprsFmt2Len, 0x06, bAPN, NULL); //���߹���APN
		OoReadField(bGprsBuf, pbAttribute2Fmt, wGprsFmt2Len, 0x07, bGprsUserName, NULL); //���߹����û���
		OoReadField(bGprsBuf, pbAttribute2Fmt, wGprsFmt2Len, 0x08, bGprsUserPwd, NULL); //���߹����û�������
		OoReadField(bEthMasterInfo, pbEthAttribute3Fmt, wEthFmt3Len, 0x01, bEthMasterIp, NULL); //��̫����վIP��ַ
		OoReadField(bEthMasterInfo, pbEthAttribute3Fmt, wEthFmt3Len, 0x02, bEthMasterComm, NULL); //��̫����վ�˿�
	}
	else
	{
		//return -1;
	}
				
	//memset(bMaster, 0, sizeof(bMaster));
	//ReadItemEx(BN0,PN0,0x003f,bMaster);
	//wPort = bMaster[5]*256 + bMaster[4];

	//��ʾ��������ַ���ն����ڷ�����
	//bAddrType = (bBuf[0]>>4)&0x0C;
	//bLogicAddr = (bBuf[0]>>4)&0x03;
	bAddrLen = bBuf[0]&0x0f + 1;
	while(i < bAddrLen)
	{
		szServAddr[i] = bBuf[1+i]&0xf0;
		i++;
		szServAddr[i] = bBuf[1+i]&0x0f;
		i++;
	}

	i = 0;
	wGprsPort = ByteToWord(bGprsMasterComm);
	sprintf(menuitem[i++],"���߹�����վIP:");
	sprintf(menuitem[i++],"  %d.%d.%d.%d", bGprsMasterIp[0],bGprsMasterIp[1],bGprsMasterIp[2],bGprsMasterIp[3]);

	sprintf(menuitem[i++],"���߹�����վ�˿�: %d",wGprsPort);
	//sprintf(menuitem[i++],"����IP:");
	//sprintf(menuitem[i++],"  %d.%d.%d.%d", bMaster[6],bMaster[7],bMaster[8],bMaster[9]);
	//wPort = bMaster[11]*256 + bMaster[10];
	//sprintf(menuitem[i++],"���ö˿�: %d",wPort);

	sprintf(menuitem[i++],"APN:");
	sprintf(menuitem[i++],"  %s", bAPN);
	//memset(bMaster, 0, sizeof(bMaster));

	//memset(bMaster, 0, sizeof(bMaster));
	memset(bBuf, 0, sizeof(bBuf));
	//ReadItemEx(BN0,PN0,0x010f,bMaster);
	sprintf(menuitem[i++],"����ר���û���:");
	memcpy(bBuf, bGprsUserName, 16);
	sprintf(menuitem[i++],"%s",(char *)bBuf);
	memcpy(bBuf, &bGprsUserName[16], 16);
	if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
		sprintf(menuitem[i++], " "); //����û���
	else
		sprintf(menuitem[i++],"%s",(char *)bBuf);

	sprintf(menuitem[i++],"����ר������:");
	memcpy(bBuf, bGprsUserPwd, 16);
	sprintf(menuitem[i++],"%s",(char *)bBuf);
	memcpy(bBuf, &bGprsUserPwd[16], 16);
	if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
		sprintf(menuitem[i++], " "); //����û���
	else
		sprintf(menuitem[i++],"%s",(char *)bBuf);

	//memset(bMaster, 0, sizeof(bMaster));
	//ReadItemEx(BN2,PN0,0x2054,bMaster);

	wEthPort = ByteToWord(bEthMasterComm);
	sprintf(menuitem[i++],"��̫����վIP:");
	sprintf(menuitem[i++],"  %d.%d.%d.%d", bEthMasterIp[0],bEthMasterIp[1],bEthMasterIp[2],bEthMasterIp[3]);

	sprintf(menuitem[i++],"��̫����վ�˿�: %d",wEthPort);

	sprintf(menuitem[i++],"��������ַ:");
	memset(bTmpBuf, 0, sizeof(bTmpBuf));
	memcpy(bTmpBuf, &szServAddr[0], 16);
	sprintf(menuitem[i++], "%s", (char *)bTmpBuf);
	memcpy(bTmpBuf, &szServAddr[16], 16);
	if (IsAllAByte(bTmpBuf, 0, sizeof(bTmpBuf)))
		sprintf(menuitem[i++], " "); //����û���
	else
		sprintf(menuitem[i++], "%s", (char *)bTmpBuf);

	//memset(bMaster, 0, sizeof(bMaster));
	//ReadItemEx(BN10, PN0, 0xa15f,bMaster);

	//sprintf(menuitem[i++],"�ն�MAC��ַ:");
	//if (!IsAllAByte(bMaster, 0xee, 6))//�����������ַ���˴�ֻ���mac��ַ��ֻ������mac��ַ
	//{
		//sprintf(menuitem[i++],"  %02x:%02x:%02x:%02x:%02x:%02x",bMaster[5],bMaster[4],bMaster[3],bMaster[2],bMaster[1],bMaster[0]);
	//}
	//else
		//sprintf(menuitem[i++],"  xx:xx:xx:xx:xx:xx");

	items[i].text = NULL;

	CListBoxEx listbox;
	listbox.Show(0, (char*) "��վͨ����Ϣ", items, KEY_ESC, 60000);

	return -1;
}

int BatteryInfo(void *arg)
{
	char menuitem[2][32];
	int iVal = 0;
	BYTE i = 0;
	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{ NULL, 0xFE, NULL, NULL }, //
	};
	
	OoReadVal(0x20110200, &iVal, 1);
	float fVal = (float)iVal/100.0;
	sprintf(menuitem[i++],"ʱ�ӵ�ص�ѹ:%.2f V",fVal);

	OoReadVal(0x20120200, &iVal, 1);
	fVal = (float)iVal/100.0;
	if (fVal > 6.45)
	{
		fVal = 0;
	}
	sprintf(menuitem[i++],"���õ�ص�ѹ:%.2f V",fVal);

	items[i].text = NULL;


	CListBoxEx listbox;
	listbox.Show(0,"��ص���", items, KEY_ESC, 60000,false);

	return -1;
}

int TermiManageHelp(void *arg)
{
	struct ListBoxExItem tmp[] ={ {(char*) "�ն˰汾��Ϣ", MENU_TWOLEVEL_HAVE_NO, TermCommpAndVersion, (void *) 0 },//
	//{ (char*) "��վͨ����Ϣ", MENU_TWOLEVEL_HAVE_NO, IPAddrInfo, (void *) 0 },//
	//{ (char*) "�ն�ͨ����Ϣ", MENU_TWOLEVEL_HAVE_NO, TermCommpAndVersion, (void *) 1 },//
	{ (char*) "��ص���", MENU_TWOLEVEL_HAVE_NO, BatteryInfo, (void *) 0 },//
	{ NULL, 0xFF, NULL, NULL }, //
	};

	CListBoxEx listbox;
	listbox.Show(0, (char*) "�ն���Ϣ", tmp, KEY_ESC, 60000);

	return -1;
}

BYTE GetModuleType();
int GetNetType();
void DrawSignal(void)
{
	static char flag = 0;

	WORD wSigStren = GetGprsWorker()->SignStrength();
	if (wSigStren >= 100)
	{
		wSigStren -= 100;
		if (wSigStren != 99)
		{
			wSigStren = (wSigStren)*31/98 + 10;
			if (wSigStren > 31)
				wSigStren = 31;
		}
	}
	if (wSigStren > 31)
		wSigStren = 0;

#if 0	
	int iNetType = GetNetType();
	if (iNetType > 0)
	{//���ӳɹ�
		if (iNetType == 4)
			DrawBmp16x16(iconSess[iNetType+6], 22, 0);
		else
			DrawBmp16x16(iconSessA[1], 22, 0);	//�㽭�ͼ�����ʾA	lx20170302
	}
	else
	{
		/*flag++;
		flag &= 0xff;
		iNetType = GetModuleType();
		if((flag&0x01)==0)			
			DrawBmp16x16(iconSess[iNetType], 22, 0);
		else*/
			DrawBmp16x16(iconSess[0], 22, 0); //ɽ�����ԺҪ��û�������ɹ�ʱ����ʾ��˸G
	}
#endif

	int rssiNo = 0;
#ifdef SYS_WIN
	wSigStren = 28;
#endif
	//�ź�ǿ��1���ַ�
	if(wSigStren>25)	
		rssiNo = 4;	
	else if(wSigStren>20)	
		rssiNo = 3;	
	else if(wSigStren>15)	
		rssiNo = 2;	
	else if(wSigStren>10)	
		rssiNo = 1;

	DrawBmp16x16(iconRssi[rssiNo], 7, 0);		
}

void DrawAlarmMap(bool isDraw)
{
	bool PreFlag = true;
	char cEvNum[4];
	static bool Flag = false;
	BYTE eventNum = 0;
	if (isDraw)
	{
		Flag = !Flag;
		ReadItemEx(BN2, PN0, 0x2031, &eventNum);
		if (eventNum == 0)
		{
			DrawBmp16x16(iconRssi[0], 38, 0);//
			return;
		}
		if (PreFlag != Flag)
		{
			DrawBmp16x16(iconAlarm, 38, 0);//
		}
		else
		{
			DrawBmp16x16(iconSess[0],38,0);
			sprintf(cEvNum,"%02d",eventNum);
			DrawStringAtLock(cEvNum, 38, 0, GUI_WHITE, GUI_BLACK);
		}
	}
	return ;
}

BYTE GetModuleType()
{
#ifdef SYS_WIN
	return 4;
#else
	BYTE bModule = MODULE_UNKNOWN, bBuf[2] = {0};
	BYTE bProType[15] = {0};
	//ReadItemEx(BN2, PN0, 0x10d3, &bModule);
	bModule = MODULE_ME590;
	switch(bModule)
	{
	case MODULE_ME590: //GPRS
		{
			GetSysInfo(bBuf);
			if (bBuf[0] == 3)	// 3G
				return 5;
			else if (bBuf[0] == 4)		// 4G
				return 6;
			else	// 2G
			{
				if (bBuf[1] == 3)	//����2G
					return 3;
				else
					return 1;
			}
			break;
		}
	case MODULE_CM180: //CDMA
		return 3;
	case MODULE_SOCKET://SOCKET
		return 4;
	default:
		return 1;
	}
	return 1;
#endif
}

int GetNetType()
{
	if(IsGprsConnected())
	{
		return GetModuleType();
	}
	return 0; 
}

//void DispBottomBar()
//{
//	WORD wTempID;
//	BYTE bBuf[25];
//	char szTip[25];
//	memset(szTip, 0, sizeof(szTip));
//	for (BYTE bI=0; bI<5; bI++)
//	{
//		wTempID = 0x2030 + bI;
//		if (wTempID == 0x2031)
//			continue;
//		memset(bBuf, 0, sizeof(bBuf));
//		ReadItemEx(BN2, PN0, wTempID, bBuf);
//		if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
//			strcat(szTip, "    ");
//		else
//			strcat(szTip, (char *)bBuf);
//		strcat(szTip, " ");
//	}
//		
//	szTip[CHAR_NUM_LINE] = '\0';
//	g_pLcd->Print(szTip, 0, MAX_LINE_NUM-1, false, false);
////	g_pLcd->DrawRange(0, (MAX_LINE_NUM-1)*16, CHAR_NUM_LINE*8-1, MAX_LINE_NUM*16-1, false, false);
//	
//}



int ReadBatVol(void)
{
	int iBatNo = 0;
	float fBaseVale = 0.0;
	float power=0.0;
	float fClose = 0.0;
	float delpower=0.0;
	BYTE bBuf[8];


	ReadItemEx(BN2, PN0, 0x1028, bBuf);
	power = (BcdToDWORD(bBuf, 2))/100.0;

	ReadItemEx(BANK10, POINT0, 0xa1d0, bBuf); //������޵�ѹ
	ReadItemEx(BANK10, POINT0, 0xa1d2, bBuf+1); //��׼��ѹ 
	if(bBuf[1]>bBuf[0])
		bBuf[1] = bBuf[0]; //�ó�����޵�ѹ����׼��ѹ
	fBaseVale = (float)bBuf[1]/10.0;
	fBaseVale = 5.0;
	memset(bBuf, 0x00, sizeof(bBuf));

	ReadItemEx(BANK10, POINT0, 0xa1d1, bBuf); //������޵�ѹ 
	if(bBuf[0] == 0 || bBuf[0] <= 43)
		bBuf[0] = 56;
	
	fClose = (float)bBuf[0]/10.0;
	fClose = 5.6;
	if(power > 6.0)
		power = 0.0;
		
	if(!GetBatStat())
		power = 0.0;

	if (power < fBaseVale)
	{
		iBatNo = 0;
	}
	else if (power < fClose)//else if (power < 5.2)
	{
		delpower = power - fBaseVale;
		iBatNo = (int)((4 * delpower)*10/((fClose - fBaseVale)*10));
		
		if (iBatNo >= 4)
		{
			iBatNo = 3;
		}
	}
	else
	{
		iBatNo = 4;
	}
	//iBatNo = (int)((4 * power)/5.2);

	if(iBatNo < 0)
		iBatNo = 0;
	if(iBatNo > 4)
		iBatNo = 4;

	//DTRACE(DB_DISPLAY,("ReadBatVol: power=%.1f,iBatNo=%d\r\n",power,iBatNo));

	return iBatNo;
}
extern bool g_fBatCharge;
int DrawStateTask()
{
	int iRet = 0;
	static int rssiNo = 4;
	static int sessNo = 2;
	
	static BYTE bNum = 0;
	TTime tmNow;

	BYTE bBuf[2] = {0};
	char *NetType[] = {"","","2G","3G","4G"};
	char *Svrinfo[] = {"","�й��ƶ�","�й���ͨ","�й�����"};
	BYTE bModuleType;
	
	//top
	DrawSignal();
#if 0

	if(GetBatStat())//---cqj---�е��
	{
		//batteryCharge = ReadBatVol();
		//�е�س��
		if (g_fBatCharge)//---------------�е��,���
		{
			//DrawBmp16x16(iconBat[CHARGEBMP], 82, 0);
			DrawBmp16x16(iconBat[bNum] , 38, 0);//���,���
			bNum++;
			if (bNum > 3)
			{
				bNum = ReadBatVol();
			}	
		}
		else//�е�أ������
		{
			DTRACE(0, ("-------the batvol is %d", ReadBatVol()));
			DrawBmp16x16(iconBat[ReadBatVol()], 38, 0);
		}
	}
	else//�޵��
	{
		DrawBmp16x16(iconBat[NOBATTERYBMP], 38, 0);//���,δ��磬���ݵ�����ͼ
	}

#endif
	

//	DrawEventNum();

	//if (!IsInProgState())//���״̬
	//{
	//	DrawStringAtLock("P", 107, 0, GUI_WHITE, GUI_BLACK);
	//} 
	//else
	//{
	//	DrawStringAtLock(" ", 107, 0, GUI_WHITE, GUI_BLACK);
	//}

	//DrawBmp16x16(iconBat[ReadBatVol()], 82, 0);//���

	//if (IsGuarantee())//����״̬
	//{
		//DrawBmp16x16(iconKeepSupply, 54, 0);//
	//} 
	//else
	{
		DrawBmp16x16(iconRssi[0], 54, 0);//
	}


	DrawBmp8x16(iconOnline, 0, 0);//�ź���
	DrawAlarmMap(true); //�¼�ͼ��

	GetCurTime(&tmNow);
	char tm[48];
	sprintf(tm,"%02d:%02d",tmNow.nHour,tmNow.nMinute);
	
	DrawStringAtLock(tm, 122, 0, GUI_WHITE, GUI_BLACK);
#ifdef EN_DRAW_TOPSPLITLINE
	ReadItemEx(BN2, PN0, 0x2050, &bModuleType);	
	if(bModuleType == MODULE_SOCKET)//��̫��
	{
		if(IsGprsConnected())//���ӳɹ�
			DrawBmp16x16(iconSess[10], 22, 0);
			
	}
	else//GPRS
	{
		GetSysInfo(bBuf);

		DrawStringAtLock(NetType[bBuf[0]], 22, 0, GUI_WHITE, GUI_BLACK);//��ʾ��ʽͼ��
		DrawStringAtLock(Svrinfo[bBuf[1]], 50, 0, GUI_WHITE, GUI_BLACK);//��ʾ��Ӫ��
	}

	DrawRectLock(0, GetFontHeight() - 2, LCD_SIM_WIDTH, GetFontHeight() - 2);
#if 0
	DrawRectLock(71, 0, 71, GetFontHeight() - 2);
	DrawRectLock(105, 0, 105, GetFontHeight() - 2);
	memset(mp,0,sizeof(mp));

	sprintf(mp,"%04d",GetAcPn());
	DrawStringAtLock(mp, 75, 0, GUI_WHITE, GUI_BLACK);//����״̬���ϵĲ�����
#endif

#endif
	//bottom
	if (!g_fStopState)
	{
		char szTip[100];
		memset(szTip, 0, sizeof(szTip));
		//for (BYTE bI=0; bI<5; bI++)
		//{
			//WORD wTempID = 0x2030 + bI;
			//if (wTempID == 0x2031)
			//{
			//	continue;
			//}

			BYTE bBuf[30];	
			memset(bBuf, 0, sizeof(bBuf));
			ReadItemEx(BN2, PN0, 0x2032, bBuf);
			if(IsAllAByte(bBuf, 0, sizeof(bBuf)))
			{
				ReadItemEx(BN2, PN0, 0x2033, bBuf);
			}
			
			if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
				strcat(szTip, "       ");
			else
				strcat(szTip, (char *)bBuf);
		//	strcat(szTip, "   ");
		//}


		DrawStringAtLock("                    ", 0, (GetMaxLinePerScreen() - 1) * GetFontHeight(),
			GUI_WHITE, GUI_BLACK);
		DrawStringAtLock(szTip, 2, (GetMaxLinePerScreen() - 1) * GetFontHeight() + 2, GUI_WHITE, GUI_BLACK);

#ifdef EN_DRAW_BOTTOMSPLITLINE
		DrawRectLock(0, (GetMaxLinePerScreen() - 1) * GetFontHeight(), LCD_SIM_WIDTH,
			(GetMaxLinePerScreen() - 1) * GetFontHeight());
#endif
	}
	else
	{
		/*char szTip[100];
		memset(szTip, 0, sizeof(szTip));


		DrawStringAtLock("                            ", 0, (GetMaxLinePerScreen() - 1) * GetFontHeight(),
			GUI_WHITE, GUI_BLACK);
		DrawStringAtLock(szTip, 2, (GetMaxLinePerScreen() - 1) * GetFontHeight() + 2, GUI_WHITE, GUI_BLACK);*/
#ifdef EN_DRAW_BOTTOMSPLITLINE
		DrawRectLock(0, (GetMaxLinePerScreen() - 1) * GetFontHeight(), LCD_SIM_WIDTH,
			(GetMaxLinePerScreen() - 1) * GetFontHeight());
#endif
	}

//#ifndef SYS_WIN
	if (IsMountUsb() && (!IsInUsbProcess()))	
	{
		SetUsbProcessState(1);
		BlightOn(true);
		iRet = UsbUpdate2(0);
		LcdRefresh();
	}
//#endif
		
	return iRet;
}



int InterStatusToMsgIndex()
{
	return -1;
}
int SendDisplayMsg(void *pvMsg)
{
	if(pvMsg != NULL)
		return g_MsgQueue.Append(pvMsg, 1);
	return -1;
}

void RemoveDisplayMsg()
{
	return g_MsgQueue.RemoveAll();
}

void* GetDisplayMsg()
{
	return g_MsgQueue.Remove(0);
}

int GetDisplayMsgNum()
{
	return g_MsgQueue.GetMsgNum();
}


int BuyEngInfo(void *arg)
{
	char menuitem[15][32];
	BYTE TmpBuff[32];
	char title[20] = {0};
	BYTE i = 0;
	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)3},
		{menuitem[3],0xFE,Dummy,(void*)4},
		{menuitem[4],0xFE,Dummy,(void*)5},
		{menuitem[5],0xFE,Dummy,(void*)6},
		{menuitem[6],0xFE,Dummy,(void*)7},
		{menuitem[7],0xFE,Dummy,(void*)8},
		{menuitem[8],0xFE,Dummy,(void*)9},
		{menuitem[9],0xFE,Dummy,(void*)10},
		{menuitem[10],0xFE,Dummy,(void*)11},
		{menuitem[11],0xFE,Dummy,(void*)12},
		{menuitem[12],0xFE,Dummy,(void*)13},
		{ NULL, 0xFE, NULL, NULL }, //
	};
	char cInput[5] = {0};
	int iGroupNum = 0;
	if (EditSpecBox(2,"�������ܼ����(1-8):",cInput, 60000, 1,DATA_DEC8) >= 0)
	{
		sscanf(cInput, "%d", &iGroupNum);
		if (iGroupNum < 8 && iGroupNum > 0)
		{
			i = 0;
			char str[30];
			DWORD dwSheetNo = 0;
			ReadItemEx(BN0, iGroupNum, 0x02ff,TmpBuff);
			//TraceFrm("goudiandan---------------------------",TmpBuff,10);
			dwSheetNo = ByteToDWORD(TmpBuff,4);
			sprintf(menuitem[i++],"���絥��: %ld",dwSheetNo);
			char *aa = NULL;
			if (TmpBuff[4] == 0x55)
			{
				aa = "׷��";
			}
			else if (TmpBuff[4] == 0xaa)
			{
				aa = "ˢ��";
			}
			else
				aa = "��Ч";
			sprintf(menuitem[i++],"׷��/ˢ�£� %s",aa);

			sprintf(menuitem[i++],"����ǰ������");
			memset(TmpBuff, 0, sizeof(TmpBuff));
			ReadItemEx(BN0, iGroupNum, 0x082f,TmpBuff);
			memset(str, 0, sizeof(str));
			Fmt03ToStr(TmpBuff, str);
			sprintf(menuitem[i++],"%s",str);

			sprintf(menuitem[i++],"����������");
			memset(TmpBuff,0,sizeof(TmpBuff));
			ReadItemEx(BN0, iGroupNum, 0x083f, TmpBuff);
			memset(str,0,sizeof(str));
			Fmt03ToStr(TmpBuff, str);
			sprintf(menuitem[i++],"%s",str);

			sprintf(menuitem[i++],"�������ޣ�");
			memset(TmpBuff,0,sizeof(TmpBuff));
			ReadItemEx(BN0, iGroupNum, 0x02ff,TmpBuff);
			memset(str,0,sizeof(str));
			Fmt03ToStr(&TmpBuff[9], str);
			sprintf(menuitem[i++],"%s",str);

			sprintf(menuitem[i++],"��բ���ޣ�");
			memset(str,0,sizeof(str));
			Fmt03ToStr(&TmpBuff[13], str);
			sprintf(menuitem[i++],"%s",str);
	
			sprintf(menuitem[i++],"ʣ�������");
			memset(TmpBuff,0,sizeof(TmpBuff));
			ReadItemEx(BN0, iGroupNum, 0x110f, TmpBuff);
			memset(str,0,sizeof(str));
			Fmt03ToStr(TmpBuff, str);
			sprintf(menuitem[i++],"%s",str);
			items[i].text =	NULL;
		}
		else
		{
			MessageBox("�����ܼ������",KEY_ESC,10000);
			return -1;
		}
		CListBoxEx listbox;
		sprintf(title, "�ܼ���%d������Ϣ",iGroupNum);
		listbox.Show(0,title, items, KEY_ESC, 60000);
	}
	return -1;
}

int SetMySysTime(void *arg)
{
	if(!InputPwd())
		return -1;

	CListBoxEx listbox;
	char menuitem[4][32];
	int i = 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[i++],0XFE,Dummy,(void*)1},
		{menuitem[i++],0XFE,Dummy,(void*)2},
		{menuitem[i++],0XFE,Dummy,(void*)3},
		{menuitem[i++],0XFE,Dummy,(void*)4},
	};
	i = 0;
	TTime tmNow;
	DWORD dwSeconds = 0;
	GetCurTime(&tmNow);
	char szDate[12];
	char szTime[12];
	sprintf(szDate,"%04d-%02d-%02d",tmNow.nYear,tmNow.nMonth,tmNow.nDay);
	sprintf(szTime,"%02d:%02d:%02d",tmNow.nHour,tmNow.nMinute,tmNow.nSecond);

	int iYear = 0;
	int iMonth = 0;
	int iDay = 0;
	int iHour = 0;
	int iMinute = 0;
	int iSecond = 0;

	while(1)
	{
		memset(menuitem,0,sizeof(menuitem));
		i = 0;
		sprintf(menuitem[i++],"�ն�����: %s",szDate);
		sprintf(menuitem[i++],"�ն�ʱ��: %s",szTime);
		sprintf(menuitem[i++],"%s","��������");
		items[i].text = NULL;

		listbox.Show(0,"ʱ������",items,KEY_ESC|(KEY_OK<<8),60000);

		if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			break;

		if(listbox.key == KEY_OK)
		{
			char szInput[16];
			if(listbox.item == 0)
			{
				//�޸�����
				strcpy(szInput,szDate);
				if(EditTextBox(2,"��������",szInput,60000,strlen(szInput),DATA_DEC) >= 0)
				{
					if(sscanf(szInput,"%d-%d-%d",&iYear,&iMonth,&iDay)==3)
					{
						tmNow.nDay = iDay;
						tmNow.nMonth = iMonth;
						tmNow.nYear = iYear;

						if(!IsInvalidTime(tmNow))
						{
							strcpy(szDate,szInput);
						}
					}
				}
			}
			else if(listbox.item == 1)
			{
				//�޸�ʱ��
				strcpy(szInput,szTime);
				if(EditTextBox(2,"ʱ������",szInput,60000,strlen(szInput),DATA_DEC) >= 0)
				{
					if(sscanf(szInput,"%d:%d:%d",&iHour,&iMinute,&iSecond)==3)
					{
						tmNow.nHour = iHour;
						tmNow.nMinute = iMinute;
						tmNow.nSecond = iSecond;

						if(!IsInvalidTime(tmNow))
						{
							strcpy(szTime,szInput);
						}
					}
				}
			}
			else if(listbox.item == 2)
			{
				//��������
				if(MessageBox("ȷ��Ҫ����ʱ��?",KEY_ESC,10000) > 0)
				{
					dwSeconds = TimeToSeconds(tmNow);
					dwSeconds += 2;
					SecondsToTime(dwSeconds, &tmNow);

					tmNow.nWeek = DayOfWeek(tmNow);

					DTRACE(DB_CRITICAL,("SetTermiTime:: set system time:%04d-%02d-%02d %02d:%02d:%02d\r\n",tmNow.nYear,tmNow.nMonth,tmNow.nDay,tmNow.nHour,tmNow.nMinute,tmNow.nSecond));

					if (SetSysTime(tmNow) == true)
					{
						MessageBox("���óɹ�!",KEY_ESC,5000);
					}
					else
					{
						MessageBox("����ʧ��!",KEY_ESC,5000);
					}
				}
			}
		}
	}

	return -1;
}

// int SetCommunicationPara(void *arg)
// {
// 	if (!InputPwd())
// 	{
// 		return -1;
// 	}
// 
// 	BYTE i = 0;
// 	int iPageNum = 0;
// 	char menuitem[9][32];
// 	memset(menuitem, 0, sizeof(menuitem));
// 
// 	struct ListBoxExItem tmpS[] = { 
// 		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
// 		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
// 		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
// 		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
// 		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
// 		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
// 		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
// 		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
// 		{ menuitem[8], 0xFE, Dummy, (void *) 8 }, //
// 		{ NULL, 0xFF, NULL, NULL }, //
// 	};
// 
// 	BYTE bMaster[64];
// 	BYTE bTermin[64];
// 	BYTE bAddr[64];
// 	WORD wPort = 0;
// 	BYTE j = 0;
// 	BYTE DecHex = 16,bDecHex;
// 	//char SIMCardNo[15];
// 	//char ModuleType[10];
// 	BYTE bMacAddr[10];
// 	ReadItemEx(BN0,PN0,0x003f,bMaster);
// 	ReadItemEx(BN10,PN0,0xA04F,bAddr);
// 	//	ReadItemEx(BN10,PN0,0xa046,SIMCardNo);
// 	ReadItemEx(BN10,PN0,0xA144,&bDecHex);
// 	ReadItemEx(BN10,PN0,0xa150,bMacAddr);
// 	if (bDecHex == 0)
// 	{
// 		DecHex = 10;
// 	}
// 	else if(bDecHex == 1)
// 	{
// 		DecHex = 16;
// 	}
// 	
// 
// 	while(1)
// 	{
// 
// 		j = i;//��¼�±�����NULL�Ĳ˵���
// 		tmpS[j].text = menuitem[j];//�ָ�ԭ��ֵ
// 		switch(iPageNum)
// 		{
// 
// 		case 0:
// 			
// 			i = 0;
// 			wPort = 0;
// 
// 
// 			wPort = bMaster[5]*256 + bMaster[4];
// 			sprintf(menuitem[i++], "��վIP��ַ��");
// 
// 			sprintf(menuitem[i++],"   %d.%d.%d.%d",bMaster[0],bMaster[1],bMaster[2],bMaster[3]);
// 			sprintf(menuitem[i++],"��վ�˿�: %d",wPort);
// 
// 			sprintf(menuitem[i++], "����IP��ַ��");
// 			sprintf(menuitem[i++],"   %d.%d.%d.%d",bMaster[6],bMaster[7],bMaster[8],bMaster[9]);
// 			wPort = bMaster[11]*256 + bMaster[10];
// 			sprintf(menuitem[i++],"���ö˿�: %d",wPort);
// 
// 			//	sprintf(menuitem[i++],"APN: %s",&bMaster[12]);
// 
// 			sprintf(menuitem[i++],"��������");
// 			j = i;
// 			break;
// 		case 1:
// 			i = 0;
// 			ReadItemEx(BN2,PN0,0x2054,bTermin);
// 			sprintf(menuitem[i++], "�ն�ip��ַ��");
// 			sprintf(menuitem[i++],"   %03d.%03d.%03d.%03d",bTermin[0],bTermin[1],bTermin[2],bTermin[3]);
// 			sprintf(menuitem[i++], "���ص�ַ��");
// 			sprintf(menuitem[i++],"   %03d.%03d.%03d.%03d",bTermin[4],bTermin[5],bTermin[6],bTermin[7]);
// 			sprintf(menuitem[i++], "�������룺");
// 			sprintf(menuitem[i++],"   %03d.%03d.%03d.%03d",bTermin[8],bTermin[9],bTermin[10],bTermin[11]);
// 			sprintf(menuitem[i++],"��������");
// 			j = i;
// 			break;
// 		case 2:
// 			i = 0;
// 		//sprintf(menuitem[i++], "�ն˵�ַ(10)��%d",bAddr[2] *256+bAddr[3]);
// 		//	sprintf(menuitem[i++], "�ն˵�ַ(16)��%02X%02X",bAddr[2],bAddr[3]);
// 			
// 
// 			sprintf(menuitem[i++],"APN: %s",&bMaster[12]);
// 			sprintf(menuitem[i++], "�������룺%02X%02X",bAddr[0],bAddr[1]);
// 			sprintf(menuitem[i++],"�ն˵�ַ��ʽ:%d����",DecHex);
// 		//	sprintf(menuitem[i++], "�ն˵�ַ��%d",bAddr[3] *256+bAddr[2]);
// 			if (DecHex == 16)
// 			{
// 				sprintf(menuitem[i++], "�ն˵�ַ��%02X%02X",bAddr[3],bAddr[2]);
// 			}
// 			else
// 			{
// 				sprintf(menuitem[i++], "�ն˵�ַ��%d",bAddr[3] *256+bAddr[2]);
// 			}
// 			
// 		//	sprintf(menuitem[i++],"SIM����:%c%c%c%c%c%c%c%c%c%c%c",SIMCardNo[0],SIMCardNo[1],SIMCardNo[2],\
// 				SIMCardNo[3],SIMCardNo[4],SIMCardNo[5],SIMCardNo[6],SIMCardNo[7],SIMCardNo[8],SIMCardNo[9],SIMCardNo[10]);
// 		//	sprintf(menuitem[i++],"ģ���ͺ�:%c");
// 
// 			sprintf(menuitem[i++], "�ն�MAC��ַ��");
// 			sprintf(menuitem[i++], "%02X:%02X:%02X:%02X:%02X:%02X",bMacAddr[0],bMacAddr[1],bMacAddr[2],bMacAddr[3],bMacAddr[4],bMacAddr[5]);
// 			sprintf(menuitem[i++],"��������");
// 			j = i;
// 			break;
// 		//case 3:
// 			//i = 0;
// 			//memset(bTermin, 0, sizeof(bTermin));
// 			//ReadItemEx(BN2,PN0,0x2054,bTermin);
// 			//sprintf(menuitem[i++],"�����������ַ");
// 			//sprintf(menuitem[i++],"   %03d.%03d.%03d.%03d",bTermin[13],bTermin[14],bTermin[15],bTermin[16]);
// 			///wPort = bTermin[18]*256 + bTermin[17];
// 
// 			//sprintf(menuitem[i++],"����˿�:");
// 
// 			//sprintf(menuitem[i++],"�����û���:");
// 
// 
// 			//sprintf(menuitem[i++],"��������:");
// 
// 			//sprintf(menuitem[i++],"�������ӷ�ʽ:");
// 
// 
// 			//sprintf(menuitem[i++],"��������");
// 			//j = i;
// 			//break;
// 		//case 4:
// 		//	break;
// 		//case 5:
// 		//	break;
// 		default:
// 			break;
// 		}
// 
// 		tmpS[i].text = NULL;
// 
// 		CListBoxEx listboxSub;
// 		listboxSub.Show(0,"�ն���Ϣ����",tmpS,KEY_ESC | KEY_OK << 8 | KEY_LEFT << 16 | KEY_RIGHT << 24,60000);
// 		if (listboxSub.key == KEY_OK && iPageNum == 0)
// 		{
// 			BYTE *pbIpAddr = NULL;
// 			if (listboxSub.item == 0 || listboxSub.item == 1)
// 			{
// 
// 				pbIpAddr = bMaster;
// 
// 			}
// 			else if (listboxSub.item == 3 || listboxSub.item == 4)
// 			{
// 				pbIpAddr = &bMaster[6];
// 			}
// 			else if (listboxSub.item == 2)
// 			{
// 				pbIpAddr = &bMaster[4];
// 			}
// 			else if (listboxSub.item == 5)
// 			{
// 				pbIpAddr = &bMaster[10];
// 			}
// 			char szInput[32];
// 
// 			if (listboxSub.item == 0 || listboxSub.item == 1 || listboxSub.item == 3 || listboxSub.item == 4)
// 			{
// 
// 				sprintf(szInput,"%03d.%03d.%03d.%03d",pbIpAddr[0],pbIpAddr[1],pbIpAddr[2],pbIpAddr[3]);	
// 
// 				if(EditTextBoxIP(2,"����IP��ַ",szInput,60000,15,DATA_DEC)>=0)
// 				{
// 					int iAddr1,iAddr2,iAddr3,iAddr4;
// 					if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
// 					{
// 						*(pbIpAddr) = (BYTE)iAddr1;
// 						*(pbIpAddr+1) = (BYTE)iAddr2;
// 						*(pbIpAddr+2) = (BYTE)iAddr3;
// 						*(pbIpAddr+3) = (BYTE)iAddr4;
// 					}
// 
// 				}
// 			}
// 			else if (listboxSub.item == 2 || listboxSub.item == 5)
// 			{
// 
// 				//���ö˿�
// 				sprintf(szInput,"%d",ByteToWord(pbIpAddr));
// 				if(EditTextBox(2,"���ö˿�",szInput,60000,5,DATA_DEC)>=0)
// 				{
// 					WORD wPort = (WORD)atoi(szInput);
// 
// 					WordToByte(wPort,pbIpAddr);
// 				}
// 			}
// 			if (listboxSub.item == 6)
// 			{
// 				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
// 				{
// 					if(WriteItemEx(BN0,PN0,0x003f,bMaster)> 0)
// 					{
// 						MessageBox("���óɹ�",KEY_ESC,3000);
// 						TrigerSaveBank(BN0, SECT_KEEP_PARA, 0);	//��������һ��100f
// 						DoTrigerSaveBank();
// 					}
// 					else
// 					{
// 						MessageBox("���óɹ�",KEY_ESC,2000);
// 					}
// 
// 				}
// 
// 			}
// 		}
// 		else if (listboxSub.key == KEY_OK && iPageNum == 1)
// 		{
// 			BYTE *pbIpAddr = NULL; 
// 			char *EditTitle[2] = {"����IP��ַ","������������"};
// 			BYTE k = 0;
// 			if (listboxSub.item == 0 || listboxSub.item == 1)
// 			{
// 				pbIpAddr = bTermin;
// 
// 			}
// 			else if (listboxSub.item == 2 || listboxSub.item == 3)
// 			{
// 				pbIpAddr = &bTermin[4];
// 			}
// 			else if (listboxSub.item == 4 || listboxSub.item == 5)
// 			{
// 				pbIpAddr = &bTermin[8];
// 			}
// 			char szInput[32];
// 			if (listboxSub.item < 6)
// 			{
// 				
// 				k = 0;
// 				if (listboxSub.item == 4 || listboxSub.item == 5)
// 				{
// 					k = 1;
// 				}
// 				sprintf(szInput,"%03d.%03d.%03d.%03d",pbIpAddr[0],pbIpAddr[1],pbIpAddr[2],pbIpAddr[3]);	
// 				if(EditTextBoxIP(2,EditTitle[k],szInput,60000,15,DATA_DEC)>=0)
// 				{
// 					int iAddr1,iAddr2,iAddr3,iAddr4;
// 					if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
// 					{
// 						*(pbIpAddr) = (BYTE)iAddr1;
// 						*(pbIpAddr+1) = (BYTE)iAddr2;
// 						*(pbIpAddr+2) = (BYTE)iAddr3;
// 						*(pbIpAddr+3) = (BYTE)iAddr4;
// 					}
// 
// 				}
// 			}
// 			if (listboxSub.item == 6)	
// 			{
// 				if (!InputPwd())
// 				{
// 					continue;
// 				}
// 				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
// 				{
// 					if(WriteItemEx(BN2,PN0,0x2054,bTermin)> 0/* && WriteItemEx(BN10,PN0,0xa150,bMacAddr)*/)
// 					{
// 						MessageBox("���óɹ�",KEY_ESC,3000);
// 						TrigerSaveBank(BN0, SECT_KEEP_PARA, 0);	//��������һ��100f
// 						DoTrigerSaveBank();
// 					}
// 					else
// 					{
// 						MessageBox("����ʧ��",KEY_ESC,2000);
// 					}
// 
// 				}
// 
// 			}
// 		}
// 		else if (listboxSub.key == KEY_OK && iPageNum == 2)
// 		{
// 			char szInput[32];
// 			BYTE *pbIpAddr = NULL; 
// 			if (listboxSub.item == 0)
// 			{
// 				pbIpAddr = &bMaster[12];
// 				strcpy(szInput,(char*)pbIpAddr);
// 				if(getSoftKey("����APN",szInput,60000,15,DATA_ASC)>=0)
// 				{
// 					memcpy(pbIpAddr,szInput,15);
// 				}
// 				char *pTail = strstr((char*)pbIpAddr," ");
// 				if(pTail)
// 				{
// 					memset(pTail,0x00,&bMaster[64]-(BYTE*)pTail+1);
// 				} 
// 
// 			}
// 			else if (listboxSub.item == 1)
// 			{
// 				pbIpAddr = bAddr;
// 				sprintf(szInput,"%02X%02X",pbIpAddr[0],pbIpAddr[1]);
// 				if(EditTextBox(2,"������������",szInput,60000,5,DATA_DEC)>=0)
// 				{
// 					int iAddr1;
// 
// 					if(sscanf(szInput,"%x",&iAddr1)==1)
// 					{
// 
// 						*(pbIpAddr + 1) = (BYTE)(iAddr1 & 0xff);
// 						*(pbIpAddr) = (BYTE)((iAddr1 >> 8) &0xff);
// 						
// 					}
// 
// 				}
// 			}
// 			else if (listboxSub.item == 2)
// 			{
// 
// 				CListBoxEx listboxTmp1;
// 				struct ListBoxExItem tmpS[] = { 
// 					{ "ʮ������", 0xFE, Dummy, (void *) 16 }, //
// 					{ "ʮ����", 0xFE, Dummy, (void *) 10 }, //
// 					{ NULL, 0xFF, NULL, NULL }, //
// 				};
// 				listboxTmp1.Show(0, "�ն˵�ַ��ʽ", tmpS, KEY_ESC | (KEY_OK << 8), 60000);
// 				if (listboxTmp1.key == KEY_OK)
// 				{
// 					DecHex = ((BYTE)((int)tmpS[listboxTmp1.item].arg));
// 
// 				}
// 				
// 			}
// 			else if (listboxSub.item == 3)
// 			{
// 				pbIpAddr = &bAddr[2];
// 				char szDecHex[18];
// 				if (DecHex == 16)
// 				{
// 					sprintf(szDecHex,"�����ն˵�ַ(16)");
// 					sprintf(szInput,"%02X%02X",pbIpAddr[1],pbIpAddr[0]);	
// 				}
// 				else
// 				{
// 					sprintf(szDecHex,"�����ն˵�ַ(10)");
// 					sprintf(szInput,"%d",pbIpAddr[1] *256 + pbIpAddr[0]);	
// 				}
// 				
// 
// 				if(EditTextBox(2,szDecHex,szInput,60000,15,DATA_DEC)>=0)
// 				{
// 					int iAddr1, iAddr2;
// 
// 					if (DecHex == 16 && sscanf(szInput,"%02X%02X",&iAddr1,&iAddr2)==2)
// 					{
// 						*(pbIpAddr) = (BYTE)iAddr2;
// 						*(pbIpAddr + 1) = (BYTE)iAddr1;
// 					}
// 					else if (DecHex == 10 && sscanf(szInput,"%d",&iAddr1)==1)
// 					{
// 						*(pbIpAddr) = (BYTE)(iAddr1 & 0xff);
// 						*(pbIpAddr + 1) = (BYTE)((iAddr1>>8) &0XFF);
// 
// 					}
// 				}
// 
// 			}else if (listboxSub.item == 4 || listboxSub.item == 5)
// 			{
// 				pbIpAddr = bMacAddr;
// 				sprintf(szInput,"%02X:%02X:%02X:%02X:%02X:%02X",pbIpAddr[0],pbIpAddr[1],pbIpAddr[2],pbIpAddr[3],pbIpAddr[4],pbIpAddr[5]);
// 
// 				if(getSoftKey("����MAC��ַ",szInput,60000,18,DATA_HEX)>=0)
// 				{
// 					int iAddr1,iAddr2,iAddr3,iAddr4,iAddr5,iAddr6;
// 
// 					if(sscanf(szInput,"%x:%x:%x:%x:%x:%x",&iAddr1,&iAddr2,&iAddr3,&iAddr4,&iAddr5,&iAddr6)==6)
// 					{
// 						*(pbIpAddr) = (BYTE)iAddr1;
// 						*(pbIpAddr+1) = (BYTE)iAddr2;
// 						*(pbIpAddr+2) = (BYTE)iAddr3;
// 						*(pbIpAddr+3) = (BYTE)iAddr4;
// 						*(pbIpAddr+4) = (BYTE)iAddr5;
// 						*(pbIpAddr+5) = (BYTE)iAddr6;
// 					}
// 				}
// 			}
// 			if (listboxSub.item == 6)	
// 			{
// 				
// 				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
// 				{
// 					if (DecHex == 16)
// 					{
// 						bDecHex = 1;
// 					}
// 					else if (DecHex == 10)
// 					{
// 						bDecHex = 0;
// 					}
// 					if(WriteItemEx(BN10,PN0,0xA04F,bAddr)> 0 && WriteItemEx(BN10,PN0,0xA144,&bDecHex)> 0 && WriteItemEx(BN10,PN0,0xa150,bMacAddr) && WriteItemEx(BN0,PN0,0x003f,bMaster))
// 					{
// 						MessageBox("���óɹ�",KEY_ESC,3000);
// 						TrigerSaveBank(BN0, SECT_KEEP_PARA, 0);	//��������һ��100f
// 						TrigerSaveBank(BN10, 0, -1);	//��������һ��100f
// 						DoTrigerSaveBank();
// 					}
// 					else
// 					{
// 						MessageBox("����ʧ��",KEY_ESC,2000);
// 					}
// 
// 				}
// 
// 			}
// 		}
// 		if (listboxSub.key == KEY_LEFT)
// 		{
// 			iPageNum--;
// 			if (iPageNum < 0)
// 			{
// 				iPageNum = 5;
// 			}
// 
// 		}
// 		if (listboxSub.key == KEY_RIGHT)
// 		{
// 			iPageNum++;
// 			if (iPageNum > 5)
// 			{
// 				iPageNum = 0;
// 			}
// 		}
// 		if (listboxSub.key == KEY_ESC || listboxSub.key == KEY_NULL)
// 		{
// 			break;
// 		}
// 	}
// 
// 	return -1;
// }


int SetCommunicationStatePara(void *arg)
{
	if (!InputPwd())
	{
		return -1;
	}

	char menuitem[8][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 8 }, //
		//{ menuitem[8], 0xFE, Dummy, (void *) 8 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0;
	char szInput[20];
	CListBoxEx listbox;
	//char *LinkType[] = {"TCP��ʽ","UDP��ʽ"};
	//char *ActiveMode[] = {"��������","��������","��������","ʱ������"};
	char *ActiveMode[] = {"��������","��������"};
	//ReadItemEx(BN0,PN0,0x008f,tmpBuf);
	//ReadItemEx(BN0,PN0,0x001f,pbBuf);

	BYTE bGprsBuf[140] = {0};
	BYTE bEthBuf[40] = {0};
	BYTE bActiveMode = 0;
	BYTE bGprsReSendTime;
	BYTE bEthReSendTime;
	WORD wGprsHeartBeat;
	WORD wEthHeartBeat;
	BYTE bGprsHeartBeat[2] = {0};
	BYTE bEthHeartBeat[2] = {0};
	BYTE *pbGprsAttribute2Fmt;
	BYTE *pbEthAttribute2Fmt;
	WORD wGprsFmt2Len;
	WORD wEthFmt2Len;
	TFieldParser tGprsParser;
	TFieldParser tEthParser;
	//bool fGprsParseItem;
	//bool fEthParseItem;

	if ((tGprsParser.wCfgLen = OoReadAttr(0x4500, 0x02, bGprsBuf, &pbGprsAttribute2Fmt, &wGprsFmt2Len))>0 &&
		(tEthParser.wCfgLen = OoReadAttr(0x4510, 0x02, bEthBuf, &pbEthAttribute2Fmt, &wEthFmt2Len))>0)
	{
		tGprsParser.pbCfg = bGprsBuf;
		tEthParser.pbCfg = bEthBuf;
		
		if (OoParseField(&tGprsParser, pbGprsAttribute2Fmt, wGprsFmt2Len, false) && 
			OoParseField(&tEthParser, pbEthAttribute2Fmt, wEthFmt2Len, false))
		{
			//int ReadParserField(TFieldParser* pParser, WORD wIndex, BYTE* pbBuf, BYTE* pbType, WORD*  pwItemOffset, WORD* pwItemLen);
			ReadParserField(&tGprsParser, 0x0B, &bGprsReSendTime, NULL, NULL, NULL);
			ReadParserField(&tEthParser, 0x07, &bEthReSendTime, NULL, NULL, NULL);
			ReadParserField(&tGprsParser, 0x03, &bActiveMode, NULL, NULL, NULL);
			ReadParserField(&tGprsParser, 0x0C, bGprsHeartBeat, NULL, NULL, NULL);
			ReadParserField(&tEthParser, 0x08, bEthHeartBeat, NULL, NULL, NULL);		
		}
		else
		{
			//return -1;
		}

	}
	else
	{
		//return -1;
	}


	while(1)
	{
		i = 0;
		sprintf(menuitem[i++],"    ���߹�����ʽ    ");
		//sprintf(menuitem[i++],"�ط�����:%d��",(pbBuf[3]&0x30)>>4);
		//bGprsReSendTime = bGprsReSendTime&0x03;
		sprintf(menuitem[i++],"�ط�����:%d��",bGprsReSendTime&0x03);
		//sprintf(menuitem[i++],"�ز�����:%d��",tmpBuf[3]);
		//sprintf(menuitem[i++],"�ز����:%ds",ByteToDWORD(&tmpBuf[1],2));
		sprintf(menuitem[i++],"����ģʽ:%s",ActiveMode[bActiveMode]);
		//sprintf(menuitem[i++],"��ͨѶʱ��:%d��",tmpBuf[4]);
		wGprsHeartBeat = ByteToWord(bGprsHeartBeat);
		sprintf(menuitem[i++],"����ʱ��:%d��", wGprsHeartBeat);

		sprintf(menuitem[i++],"     ��̫����ʽ     ");
		//bEthReSendTime = bEthReSendTime&0x03;
		sprintf(menuitem[i++],"�ط�����:%d��", bEthReSendTime&0x03);
		wEthHeartBeat = ByteToWord(bEthHeartBeat);
		sprintf(menuitem[i++],"����ʱ��:%d��", wEthHeartBeat);
		sprintf(menuitem[i++],"��������");
		tmpS[i].text = NULL;
	
		listbox.Show(0,"�ն�ͨ�ŷ�ʽ",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item==1 || listbox.item==5)
			{
				if (listbox.item == 1)
					sprintf(szInput,"%d",bGprsReSendTime);
				else
					sprintf(szInput,"%d",bEthReSendTime);
				if (getSoftKey("�ط�����",szInput,60000,4,DATA_DEC)>=0)
				{
					DWORD ReSendCount = 0;
					sscanf(szInput,"%d",&ReSendCount);
					if(ReSendCount > 3)
						MessageBox("�Ƿ����� ����3!",KEY_ESC,2000);
					else
					{
						if (listbox.item == 1)
							bGprsReSendTime = (bGprsReSendTime&0xfc) | ReSendCount;
						else
							bEthReSendTime = (bEthReSendTime&0xfc) | ReSendCount;
					}

				}
			}
			else if (listbox.item == 2)
			{
				CListBoxEx listboxTmp2;
				struct ListBoxExItem tmp2[] = { 
					{ ActiveMode[0], 0xFF, Dummy, (void *) 0x00 },//
					{ ActiveMode[1], 0xFF, Dummy, (void *) 0x01 },//	
				    //{ ActiveMode[3], 0xFF, Dummy, (void *) 0x03 },//
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp2.Show(0, "����ģʽ", tmp2, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp2.key == KEY_OK)
				{
					bActiveMode = (BYTE)((int)tmp2[listboxTmp2.item].arg);
				}	
			}
			/*else if (listbox.item == 2)
			{
				sprintf(szInput,"%d",ByteToDWORD(&tmpBuf[1],2));
				
				if (getSoftKey("�ز����",szInput,60000,5,DATA_DEC)>=0)
				{
					DWORD dwReDialPeriod;
					sscanf(szInput,"%d",&dwReDialPeriod);
					if (dwReDialPeriod > 65535)
					{
						MessageBox("�Ƿ����� ����65535!",KEY_ESC,2000);
					}
					else
					{
						tmpBuf[1] = dwReDialPeriod & 0xff;
						tmpBuf[2] = (dwReDialPeriod >>8) & 0xff;
					}
				}
			}
			else if (listbox.item == 1)
			{
				sprintf(szInput,"%d",tmpBuf[3]);
				if (getSoftKey("�ز�����",szInput,60000,4,DATA_DEC)>=0)
				{
					DWORD dwReDialCount = 0;
					sscanf(szInput,"%d",&dwReDialCount);
					if (dwReDialCount > 255)
						MessageBox("�Ƿ����� ����255!",KEY_ESC,2000);
					else
						tmpBuf[3] = (BYTE)dwReDialCount;
				}
			}
			else if (listbox.item == 4)
			{
				sprintf(szInput,"%d",tmpBuf[4]);
				if (getSoftKey("��ͨѶʱ��",szInput,60000,3,DATA_DEC)>=0)
				{
					DWORD dwOffLineTime = 0;
					sscanf(szInput,"%d",&dwOffLineTime);
					if (dwOffLineTime > 255)
						MessageBox("�Ƿ����� ����255!",KEY_ESC,2000);
					else
						tmpBuf[4] = (BYTE)dwOffLineTime;
				}
			}*/
			else if (listbox.item==3 || listbox.item==6)
			{
				if (listbox.item == 3)
					sprintf(szInput,"%d",wGprsHeartBeat);
				else
					sprintf(szInput,"%d",wEthHeartBeat);
				//sprintf(szInput,"%d",pbBuf[5]);
				if (getSoftKey("����ʱ��",szInput,60000,3,DATA_DEC)>=0)
				{
					WORD wBeatTime = 0;
					int iTmp = 0;
					sscanf(szInput,"%d",&iTmp);
					wBeatTime = iTmp;
					if (wBeatTime > 255)
						MessageBox("�Ƿ����� ����255!",KEY_ESC,2000);						
					else
					{
						if (listbox.item == 3)
							WordToByte(wBeatTime, bGprsHeartBeat);
						else
							WordToByte(wBeatTime, bEthHeartBeat);
					}
				}
			}
			else if (listbox.item == 7)
			{
				/*if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					if (OoWriteField(bGprsBuf, pbGprsAttribute2Fmt, wGprsFmt2Len, 0x0B, &bGprsReSendTime)>0 &&
						OoWriteField(bEthBuf, pbEthAttribute2Fmt, wEthFmt2Len, 0x07, &bEthReSendTime)>0 && 
						OoWriteField(bGprsBuf, pbGprsAttribute2Fmt, wGprsFmt2Len, 0x03, &bActiveMode)>0 &&
						OoWriteField(bGprsBuf, pbGprsAttribute2Fmt, wGprsFmt2Len, 0x0C, bGprsHeartBeat)>0 &&
						OoWriteField(bEthBuf, pbEthAttribute2Fmt, wEthFmt2Len, 0x08, bEthHeartBeat)>0 &&
						OoWriteAttr(0x4510, 0x02, bEthBuf)>0 && OoWriteAttr(0x4500, 0x02, bGprsBuf)>0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
					}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,2000);
					}
				}*/
			}
		}
	}
	return -1;
}

void UpdateC1F13(BYTE bFN)
{
	BYTE bBuf[32];
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, PN0, 0x102f, bBuf);
	bBuf[(bFN-1)>>3] |= 1<<((bFN-1)&0x7);
	WriteItemEx(BN0, PN0, 0x102f, bBuf);
}


int SetMeterPort(void)
{
    CListBoxEx ListBox(1);
	char menuBuff[8][32] = {0};

    for (int i = 0; i < sizeof(CommPortType) / sizeof(CommPortType[0]); i++)
    {
        sprintf(menuBuff[i], "%s", CommPortType[i]);
    }

    struct ListBoxExItem tmp[] = {	{ (char*)menuBuff[0], 0xFF, Dummy, (void*)0 },
                                    { (char*)menuBuff[1], 0xFF, Dummy, (void*)1 },
                                    { (char*)menuBuff[2], 0xFF, Dummy, (void*)2 },
                                    { (char*)menuBuff[3], 0xFF, Dummy, (void*)3 },
					//				{ (char*)menuBuff[4], 0xFF, Dummy, (void*)4 },
                                  /*{ (char*)menuBuff[5], 0xFF, Dummy, (void*)5 },
                                    { (char*)menuBuff[6], 0xFF, Dummy, (void*)6 },
                                    { (char*)menuBuff[7], 0xFF, Dummy, (void*)7 },
                                    { (char*)menuBuff[8], 0xFF, Dummy, (void*)8 },
                                  */
                                    { NULL, 0xFF, NULL, NULL }, //
								};
    ListBox.Show(0, (char*)"��ѡ��˿ں�", tmp, KEY_OK << 8 | KEY_ESC, 60000);
    
	if (ListBox.key == KEY_OK)
	{
		return ListBox.item;
	}   

    return -1;
}

int SetPulseMtrAddr(void *arg)
{
	BYTE i = 0;
#ifdef DEBUG_DISP
	BYTE bAddress[18] = {DT_TSA, 0x0F, 
									0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44};
#else
	BYTE bAddress[18] = {0};
#endif
	//BYTE *pbFmt2 = NULL;
	//WORD wFmt2Len = 0;
	char szInput[33] = {0};
	//BYTE *p = (BYTE *)szInput;
	CListBoxEx listbox;
	int iPulseNum = 0;
	int iStart = -1;

	struct ListBoxExItem tmpS[] = { 
		{ "���ñ��ַ", 0xFF, Dummy, (void *) 1 }, //
		{ "��������", 0xFF, Dummy, (void *) 2 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	if (EditSpecBox(2,"���������������(1-8)",szInput,60000,2,DATA_DEC)>=0)
	{
		sscanf(szInput,"%d",&iPulseNum);
		if (iPulseNum > 0 && iPulseNum < 9)
			{
#ifdef DEBUG_DISP

#else
			//if (0 > OoReadAttr(0x2400+iPulseNum, 0x02, bAddress, &pbFmt2, &wFmt2Len))
			if (0 > OoProReadAttr(0x2400+iPulseNum, 0x02, 0x00, bAddress, sizeof(bAddress), &iStart))
				{
				MessageBox("��ȡ���ݿ����!", KEY_ESC, 3000);
				return -1;
			}
#endif
				}
				}
				else
				{
		MessageBox("����������������!",KEY_ESC,3000);
		return -1;
				}

	while(1)
				{
		listbox.Show(0,"ͨ�ŵ�ַ����",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
					break;
				if (listbox.key == KEY_OK)
				{
			if (listbox.item == 0)
					{
				BYTE *p = (BYTE *)szInput;
				memset(szInput, 0, sizeof(szInput));
				for (i=0; i<bAddress[1]; i++)
						{
					ByteToASCII(bAddress[2+i], &p);
					}

				if (EditTextBox(2, "���������ַ", szInput, 60000, 32, DATA_DEC) >= 0)
						{
					BYTE bLen = 0;
					for (i=0; i<sizeof(szInput); i++)
							{
						if (szInput[i]!='\0' && szInput[i]!=' ')
							bLen++;
							}
					AsciiToByte((BYTE *)szInput, bLen, &bAddress[2]);
					bAddress[1] = bLen/2;
						}
					}
			else
					{
				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
						{
					/*if (bAddress[0] != DT_OCT_STR)
						bAddress[0] = DT_OCT_STR;  //ǿ���޸���������ȷ������ȷд��*/
					if (OoWriteAttr(0x2400+iPulseNum, 0x02, bAddress) >= 0)
					{
						MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT_PULSE_PARA, -1);
						DoTrigerSaveBank();
					}
					else
						{
						MessageBox("����ʧ��",KEY_ESC,3000);
						}	
					}
			}
		}
	}	

	return -1;
}

int SetPulsePTCT(void *arg)
{
	

	int i = 0;
	typedef struct 
					{
		BYTE bStructType;
		BYTE bParserNum;
		BYTE bParser1Type;
		BYTE bPT[2];
		BYTE bParser2Type;
		BYTE bCT[2];
	}TBeilv;
	TBeilv tBeilv;
#ifdef DEBUG_DISP
	BYTE bBuf[20] = {DT_STRUCT, 0x02, DT_LONG_U, 0x00, 0x01, DT_LONG_U, 0x00, 0x01};
#else
	BYTE bBuf[20] = {0};
#endif
	char szInput[32] = {0};
	int iPulseNum = 0;
	CListBoxEx listbox;
	char menuitem[3][32];
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFF, Dummy, (void *) 0 }, //
		{ menuitem[1], 0xFF, Dummy, (void *) 1 }, //
		{ menuitem[2], 0xFF, Dummy, (void *) 2 }, //
							{ NULL, 0xFF, NULL, NULL }, //
						};
	DWORD dwOAD;
	int iStart = -1;

	if (EditSpecBox(2,"���������������(1-8)",szInput,60000,2,DATA_DEC)>=0)
						{
		sscanf(szInput,"%d",&iPulseNum);
		if (iPulseNum > 0 && iPulseNum < 9)
		{
			/*if (0 > OoReadAttr(0x2400+iPulseNum, 0x02, (BYTE *)&tBeilv, &pbFmt3, &wFmt3Len))
					{
				MessageBox("��ȡ���ݿ����!", KEY_ESC, 3000);
				return -1;
			}*/
#ifdef DEBUG_DISP

#else
			dwOAD = 0x24010300 + (iPulseNum-1)*0x00010000;
			//if (0 > OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt3, &wFmt3Len))
			if (0 > OoProReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, 0x00, bBuf, sizeof(bBuf), &iStart))
						{
				MessageBox("��ȡ���ݿ����!", KEY_ESC, 3000);
				return -1;
						}
#endif
			memcpy(&tBeilv, bBuf, sizeof(TBeilv));
					}
	}
	else
					{
		MessageBox("����������������!",KEY_ESC,3000);
		return -1;
	}

	while(1)
						{
		i = 0;
		sprintf(menuitem[i++], "��ѹ������:%d", OoLongUnsignedToWord(tBeilv.bPT));
		sprintf(menuitem[i++], "����������:%d", OoLongUnsignedToWord(tBeilv.bCT));
		sprintf(menuitem[i++], "��������");
		tmpS[i].text = NULL;

		listbox.Show(0 , "��������������", tmpS, KEY_ESC | KEY_OK << 8, 60000);
		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
		if (listbox.key == KEY_OK)
					{
			int iData = 0;
			memset(szInput, 0, sizeof(szInput));
			if (listbox.item == 0)
						{
				sprintf(szInput, "%s", (char *)tBeilv.bPT);
				if (EditTextBox(2, "���õ�ѹ����������", szInput, 60000, 5, DATA_DEC) >= 0)
					{
					sscanf(szInput, "%d", &iData);
					if (iData>=0 && iData<=65535)
						{
						OoInt16ToLong(iData, tBeilv.bPT);
					}
					else
						{
						MessageBox("�������ݲ��Ϸ�!", KEY_ESC, 10000);
						break;
						}	
					}
			}
			else if (listbox.item == 1)
					{
				sprintf(szInput, "%s", (char *)tBeilv.bCT);
				if (EditTextBox(2, "���õ�������������", szInput, 60000, 5, DATA_DEC) >= 0)
						{
					sscanf(szInput, "%d", &iData);
					if (iData>=0 && iData<=65535)
					{
						OoInt16ToLong(iData, tBeilv.bCT);
					}	
					else
						{
						MessageBox("�������ݲ��Ϸ�!", KEY_ESC, 10000);
						break;
						}
					}
			}
			else
					{
						if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
						{
					/*tBeilv.bStructType = DT_STRUCT;
					tBeilv.bParserNum = 2;
					tBeilv.bParser1Type = DT_LONG_U;
					tBeilv.bParser2Type = DT_LONG_U;*/
					if (OoWriteAttr(0x2400+iPulseNum, 0x03, (BYTE *)&tBeilv) >= 0)
							{
								MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT_PULSE_PARA, -1);
								DoTrigerSaveBank();
							}
					else
					{
						MessageBox("����ʧ��",KEY_ESC,3000);
					}
				}
			}
		}
	}
	return -1;
}

int SetPulseCfgUnit(void *arg)
{
	int i = 0;
	//BYTE *pbFmt4 = NULL;
	//WORD wFmt4Len = 0;
#ifdef DEBUG_DISP
	BYTE bPulseCfg[50] = {DT_ARRAY, MAX_PULSE_TYPE, 
													DT_STRUCT, 3, DT_OAD, 0xF2, 0x0A, 0x01, 0x00, DT_ENUM, 0x00, DT_LONG_U, 0x00, 0xFF,
													DT_STRUCT, 3, DT_OAD, 0xF2, 0x0A, 0x02, 0x00, DT_ENUM, 0x01, DT_LONG_U, 0x00, 0xFF,
													DT_STRUCT, 3, DT_OAD, 0xF2, 0x0A, 0x03, 0x00, DT_ENUM, 0x02, DT_LONG_U, 0x00, 0xFF,
													DT_STRUCT, 3, DT_OAD, 0xF2, 0x0A, 0x04, 0x00, DT_ENUM, 0x03, DT_LONG_U, 0x00, 0xFF};
#else
	BYTE bPulseCfg[50] = {0};
#endif
	int iPulseNum = 0;
	char szInput[30] = {0};
	int iStart = -1;
	typedef struct 
	{
		BYTE bStructType;
		BYTE bParserNum;
		BYTE bParser1Type;
		BYTE bPort[4];
		BYTE bParser2Type;
		BYTE bAtrr;
		BYTE bParser3Type;
		BYTE bPulseConstant[2];
	}TPulseUnit;
	CListBoxEx listbox;
	char menuitem[17][32];
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 0 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 9 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 10 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 11 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 12 }, //
		{ menuitem[13], 0xFE, Dummy, (void *) 13 }, //
		{ menuitem[14], 0xFE, Dummy, (void *) 14 }, //
		{ menuitem[15], 0xFE, Dummy, (void *) 15 }, //
		{ menuitem[16], 0xFE, Dummy, (void *) 15 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	
	TPulseUnit tCfgUnit1, tCfgUnit2, tCfgUnit3, tCfgUnit4;
	BYTE bPulseUnitLen = sizeof(TPulseUnit);
	
	char *titles[] = {"�����й�", "�����޹�", "�����й�", "�����޹�"};

	if (EditSpecBox(2,"���������������(1-8)",szInput,60000,2,DATA_DEC)>=0)
	{
		sscanf(szInput,"%d",&iPulseNum);
		if (iPulseNum > 0 && iPulseNum < 9)
		{
#ifdef DEBUG_DISP

#else
			//if (0 > OoReadAttr(0x2400+iPulseNum, 0x04, bPulseCfg, &pbFmt4, &wFmt4Len))
			if (0 > OoProReadAttr(0x2400+iPulseNum, 0x04, 0x00, bPulseCfg, sizeof(bPulseCfg), &iStart))
			{
				MessageBox("��ȡ���ݿ����!", KEY_ESC, 3000);
				return -1;
			}
#endif
			memcpy(&tCfgUnit1, (TPulseUnit *)(bPulseCfg+2), bPulseUnitLen);
			memcpy(&tCfgUnit2, (TPulseUnit *)(bPulseCfg+2+bPulseUnitLen), bPulseUnitLen);
			memcpy(&tCfgUnit3, (TPulseUnit *)(bPulseCfg+2+bPulseUnitLen*2), bPulseUnitLen);
			memcpy(&tCfgUnit4, (TPulseUnit *)(bPulseCfg+2+bPulseUnitLen*3), bPulseUnitLen);
		}
	}
	else
	{
		MessageBox("����������������!",KEY_ESC,3000);
		return -1;
	}

		while(1)
		{
			i = 0;
			sprintf(menuitem[i++], "���嵥Ԫ1:");
			sprintf(menuitem[i++], "�˿�:%08X", OoOadToDWord(tCfgUnit1.bPort));
			sprintf(menuitem[i++], "����:%s", titles[tCfgUnit1.bAtrr]);
			sprintf(menuitem[i++], "����:%d", OoLongUnsignedToWord(tCfgUnit1.bPulseConstant));
			sprintf(menuitem[i++], "���嵥Ԫ2:");
			sprintf(menuitem[i++], "�˿�:%08X", OoOadToDWord(tCfgUnit2.bPort));
			sprintf(menuitem[i++], "����:%s", titles[tCfgUnit2.bAtrr]);
			sprintf(menuitem[i++], "����:%d", OoLongUnsignedToWord(tCfgUnit2.bPulseConstant));
			sprintf(menuitem[i++], "���嵥Ԫ3:");
			sprintf(menuitem[i++], "�˿�:%08X", OoOadToDWord(tCfgUnit3.bPort));
			sprintf(menuitem[i++], "����:%s", titles[tCfgUnit3.bAtrr]);
			sprintf(menuitem[i++], "����:%d", OoLongUnsignedToWord(tCfgUnit3.bPulseConstant));
			sprintf(menuitem[i++], "���嵥Ԫ4:");
			sprintf(menuitem[i++], "�˿�:%08X", OoOadToDWord(tCfgUnit4.bPort));
			sprintf(menuitem[i++], "����:%s", titles[tCfgUnit4.bAtrr]);
			sprintf(menuitem[i++], "����:%d", OoLongUnsignedToWord(tCfgUnit4.bPulseConstant));
			sprintf(menuitem[i++],"��������");
			tmpS[i].text = NULL;

		listbox.Show(0 , "���嵥Ԫ����", tmpS, KEY_ESC | KEY_OK << 8, 60000);
		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
					break;
				if (listbox.key == KEY_OK)
				{
			int iValue = 0;
			if (listbox.item==1 || listbox.item==5 || listbox.item==9 || listbox.item==13)
			{
					memset(szInput,0,sizeof(szInput));
				sprintf(szInput, "F20A0100");
				if (EditTextBox(2, "��������˿�", szInput, 60000, 8, DATA_HEX) >= 0)
					{
					DWORD dwOAD;
					sscanf(szInput, "%08X", &iValue);
					dwOAD = iValue;
					switch(listbox.item)
						{
					case 1:
						OoDWordToOad(dwOAD, tCfgUnit1.bPort);
						break;
					case 5:
						OoDWordToOad(dwOAD, tCfgUnit2.bPort);
						break;
					case 9:
						OoDWordToOad(dwOAD, tCfgUnit3.bPort);
						break;
					case 13:
						OoDWordToOad(dwOAD, tCfgUnit4.bPort);
						break;
					default: 
						break;
						}
					}
			}
			else if (listbox.item==2 || listbox.item==6 || listbox.item==10 || listbox.item==14)
					{
				i = 0;
				CListBoxEx listbox2;
				struct ListBoxExItem tmp2[] = {
					{ titles[0], 0xFF, Dummy, (void *) 0 }, //
					{ titles[1], 0xFF, Dummy, (void *) 1 }, //
					{ titles[2], 0xFF, Dummy, (void *) 2 }, //
					{ titles[3], 0xFF, Dummy, (void *) 3 }, //
							{ NULL, 0xFF, NULL, NULL }, //
						};

				while(1)
				{
					listbox2.Show(0 , "������������", tmp2, KEY_ESC | KEY_OK << 8, 60000);
					if ((KEY_NULL == listbox2.key)||(KEY_ESC == listbox2.key))
						break;
					if (listbox2.key == KEY_OK)
					{
						switch(listbox.item)
						{
						case 2:
							tCfgUnit1.bAtrr = (BYTE)((int)tmp2[listbox2.item].arg);
							break;
						case 6:
							tCfgUnit2.bAtrr = (BYTE)((int)tmp2[listbox2.item].arg);
							break;
						case 10:
							tCfgUnit3.bAtrr = (BYTE)((int)tmp2[listbox2.item].arg);
							break;
						case 14:
							tCfgUnit4.bAtrr = (BYTE)((int)tmp2[listbox2.item].arg);
							break;
						default:
							break;
						}	
						break;
					}
				}	
			}
			else if (listbox.item==3 || listbox.item==7 || listbox.item==11 || listbox.item==15)
					{
				memset(szInput, 0, sizeof(szInput));
				if (EditTextBox(2, "�����������", szInput, 60000, 5, DATA_DEC) >= 0)
				{
					WORD wPulseConst;
					sscanf(szInput, "%d", &iValue);
					if (iValue>=0 && iValue<=65535)
						wPulseConst = iValue;
					switch(listbox.item)
						{
					case 3:
						OoWordToLongUnsigned(wPulseConst, tCfgUnit1.bPulseConstant);
						break;
					case 7:
						OoWordToLongUnsigned(wPulseConst, tCfgUnit2.bPulseConstant);
						break;
					case 11:
						OoWordToLongUnsigned(wPulseConst, tCfgUnit3.bPulseConstant);
						break;
					case 15:
						OoWordToLongUnsigned(wPulseConst, tCfgUnit4.bPulseConstant);
						break;
					default:
						break;
						}
					}
			}
			else
					{
						if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
						{
					/*tCfgUnit1.bStructType = tCfgUnit2.bStructType = tCfgUnit3.bStructType = tCfgUnit4.bStructType = DT_STRUCT;
					tCfgUnit1.bParser1Type = tCfgUnit2.bParser1Type = tCfgUnit3.bParser1Type = tCfgUnit4.bParser1Type = DT_OAD;
					tCfgUnit1.bParserNum = tCfgUnit2.bParserNum = tCfgUnit3.bParserNum = tCfgUnit4.bParserNum = 3;
					tCfgUnit1.bParser2Type = tCfgUnit2.bParser2Type = tCfgUnit3.bParser2Type = tCfgUnit4.bParser2Type = DT_ENUM;
					tCfgUnit1.bParser3Type = tCfgUnit2.bParser3Type = tCfgUnit3.bParser3Type = tCfgUnit4.bParser3Type = DT_LONG_U;
					bPulseCfg[0] = DT_ARRAY;
					bPulseCfg[1] = MAX_PULSE_TYPE;*/
					memcpy(bPulseCfg+2, &tCfgUnit1, bPulseUnitLen);
					memcpy(bPulseCfg+2+bPulseUnitLen, &tCfgUnit2, bPulseUnitLen);
					memcpy(bPulseCfg+2+bPulseUnitLen*2, &tCfgUnit3, bPulseUnitLen);
					memcpy(bPulseCfg+2+bPulseUnitLen*3, &tCfgUnit4, bPulseUnitLen);
					if (OoWriteAttr(0x2400+iPulseNum, 0x04, bPulseCfg) >= 0)
							{
								MessageBox("���óɹ�",KEY_ESC,3000);
						TrigerSaveBank(BANK0, SECT2, -1);
								DoTrigerSaveBank();
							}
							else
							{
						MessageBox("����ʧ��",KEY_ESC,3000);
							}
						}
					}
				}
			}

	return -1;
		}

int SetPulsePara(void *arg)
		{
	if (!InputPwd())
	{
		return -1;
	}
	struct ListBoxExItem tmpS[] = {
		{"ͨ�ŵ�ַ", 0xFF, SetPulseMtrAddr, (void *)1},
		{"����������", 0xFF, SetPulsePTCT, (void *)2},
		{"��������", 0xFF, SetPulseCfgUnit, (void *)3},
		//{"��������", 0xFF, Dummy, (void *)4},
		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;

	while(1)
	{
		listbox.Show(0 , "�����������", tmpS, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;

}

int QueryTaskCfgUnit(void *arg)
{
	TTaskCfg tTaskCfg;
	int i = 0;
	char szInput[32] = {0};
	BYTE bIndex;
	char menuitem[16][32];
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 0 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 9 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 10 }, //
		{ menuitem[11], 0xFE, Dummy, (void *) 11 }, //
		{ menuitem[12], 0xFE, Dummy, (void *) 12 }, //
		{ menuitem[13], 0xFE, Dummy, (void *) 13 }, //
		{ menuitem[14], 0xFE, Dummy, (void *) 14 }, //
		{ menuitem[15], 0xFE, Dummy, (void *) 15 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	char *pszTimeUnit[] = {"��", "��", "ʱ", "��", "��", "��"};
	char *pszSchemeType[] = {"", "��ͨ�ɼ�", "�¼��ɼ�", "͸��", "�ϱ�", "�ű�", "ʵʱ���"};
	char *pszExePriority[] = {"", "��Ҫ", "��Ҫ", "��Ҫ", "����"};
	char *pszStatus[] = {"", "����", "ͣ��"};
	char *pszPeriodType[] = {"ǰ�պ�", "ǰ�����", "ǰ�պ��", "ǰ����"};
	CListBoxEx listbox;

	if (EditSpecBox(2,"���������õ�Ԫ����",szInput,60000,2,DATA_DEC) >= 0)
	{
		int iIndex = 0;
		sscanf(szInput,"%03d",&iIndex);
		if (iIndex>=0 && iIndex<TASK_ID_NUM)
		{
			bIndex = iIndex;
		}
		else
		{
			MessageBox("���õ�Ԫ��������!", KEY_ESC, 3000);
			return -1;
		}
	}

	if (!GetTaskCfg(bIndex, (TTaskCfg*)&tTaskCfg))
	{
		MessageBox("��ȡ���õ�Ԫ����!", KEY_ESC, 3000);
		return -1;
	}
	
	while(1)
	{
flag:	i = 0;

		sprintf(menuitem[i++], "����ID:%d", tTaskCfg.bTaskId);
		sprintf(menuitem[i++], "ִ��Ƶ��:%d%s", tTaskCfg.tiExe.wVal, pszTimeUnit[tTaskCfg.tiExe.bUnit]);
		sprintf(menuitem[i++], "��������:%s", pszSchemeType[tTaskCfg.bSchType]);
		sprintf(menuitem[i++], "�������:%d", tTaskCfg.bSchNo);
		sprintf(menuitem[i++], "��ʼʱ��:");
		sprintf(menuitem[i++], "%d-%d-%d %02d:%02d:%02d", tTaskCfg.tmStart.nYear, tTaskCfg.tmStart.nMonth, tTaskCfg.tmStart.nDay,
															tTaskCfg.tmStart.nHour, tTaskCfg.tmStart.nMinute, tTaskCfg.tmStart.nSecond);
		sprintf(menuitem[i++], "����ʱ��:");
		sprintf(menuitem[i++], "%d-%d-%d %02d:%02d:%02d", tTaskCfg.tmEnd.nYear, tTaskCfg.tmEnd.nMonth, tTaskCfg.tmEnd.nDay, 
															tTaskCfg.tmEnd.nHour, tTaskCfg.tmEnd.nMinute, tTaskCfg.tmEnd.nSecond);
		sprintf(menuitem[i++], "��ʱ:%d%s", tTaskCfg.tiDelay.wVal, pszTimeUnit[tTaskCfg.tiDelay.bUnit]);
		sprintf(menuitem[i++], "ִ�����ȼ�:%s", pszExePriority[tTaskCfg.bPrio]);
		sprintf(menuitem[i++], "״̬:%s", pszStatus[tTaskCfg.bState]);
		sprintf(menuitem[i++], "��ʼǰ�ű�ID:%d", tTaskCfg.wPreScript);
		sprintf(menuitem[i++], "��ɺ�ű�ID:%d", tTaskCfg.wPostScript);
		sprintf(menuitem[i++], "����ʱ��:(�����鿴)");
		//sprintf(menuitem[i++], "��������");
		tmpS[i].text = NULL;

		listbox.Show(0 , "�������õ�Ԫ����", tmpS, KEY_ESC | KEY_OK << 8, 60000);
		if (listbox.key==KEY_NULL || listbox.key==KEY_ESC)
			break;
		if (listbox.key == KEY_OK)
		{
			/*if (listbox.item == 0) //����ID
			{
				int iValue = 0;
				
				memset(szInput,0,sizeof(szInput));
				sprintf(szInput, "%d", tTaskCfg.bTaskId);
				if (EditTextBox(2, "��������ID", szInput, 60000, 3, DATA_DEC) >= 0)
				{
					sscanf(szInput, "%d", &iValue);
					if (iValue > 255)
					{
						MessageBox("���벻�Ϸ�!", KEY_ESC, 3000);
						goto flag;
					}
					tTaskCfg.bTaskId = (BYTE)iValue;
				}
			}
			else if (listbox.item == 1) //ִ��Ƶ��
			{
flag1:			CListBoxEx listbox1;
				struct ListBoxExItem tmp1[] = {
					{ pszTimeUnit[0], 0xFF, Dummy, (void *) 0 }, //
					{ pszTimeUnit[1], 0xFF, Dummy, (void *) 1 }, //
					{ pszTimeUnit[2], 0xFF, Dummy, (void *) 2 }, //
					{ pszTimeUnit[3], 0xFF, Dummy, (void *) 3 }, //
					{ pszTimeUnit[4], 0xFF, Dummy, (void *) 4 }, //
					{ pszTimeUnit[5], 0xFF, Dummy, (void *) 5 }, //
					{ NULL, 0xFF, NULL, NULL }, //
				};
				
				while(1)
				{
					listbox1.Show(0 , "ִ��Ƶ�ʼ����λ", tmp1, KEY_ESC | KEY_OK << 8, 60000);
					if (listbox1.key==KEY_NULL || listbox1.key==KEY_ESC)
						break;
					if (listbox1.key == KEY_OK)
					{
						int iValue = 0;

						memset(szInput,0,sizeof(szInput));
						if (EditTextBox(2, "ִ��Ƶ�ʼ��ֵ", szInput, 60000, 5, DATA_DEC) >= 0)
						{
							sscanf(szInput, "%d", &iValue);
							if (iValue > 65535)
							{
								MessageBox("���벻�Ϸ�!", KEY_ESC, 3000);
								goto flag1;
							}
							tTaskCfg.tiExe.bUnit = (BYTE)tmp1[listbox1.item].arg;
							tTaskCfg.tiExe.wVal = (WORD)iValue;
							break;
						}
					}
				}	
			}
			else if (listbox.item == 2) //��������
			{
				CListBoxEx listbox2;
				struct ListBoxExItem tmp2[] = {
					{ pszSchemeType[1], 0xFF, Dummy, (void *) 1 }, //
					{ pszSchemeType[2], 0xFF, Dummy, (void *) 2 }, //
					{ pszSchemeType[3], 0xFF, Dummy, (void *) 3 }, //
					{ pszSchemeType[4], 0xFF, Dummy, (void *) 4 }, //
					{ pszSchemeType[5], 0xFF, Dummy, (void *) 5 }, //
					{ pszSchemeType[6], 0xFF, Dummy, (void *) 6 }, //
					{ NULL, 0xFF, NULL, NULL }, //
				};
				while(1)
				{
					listbox2.Show(0 , "��������", tmp2, KEY_ESC | KEY_OK << 8, 60000);
					if (listbox2.key==KEY_NULL || listbox2.key==KEY_ESC)
						break;
					if (listbox2.key == KEY_OK)
					{
						tTaskCfg.bSchType = (BYTE)tmp2[listbox2.item].arg;
						break;
					}
				}	
			}
			else if (listbox.item == 3) //�������
			{
				int iValue = 0;

				memset(szInput,0,sizeof(szInput));
				sprintf(szInput, "%d", tTaskCfg.bSchNo);
				if (EditTextBox(2, "�������", szInput, 60000, 3, DATA_DEC) >= 0)
				{
					sscanf(szInput, "%d", &iValue);
					if (iValue > 255)
					{
						MessageBox("���벻�Ϸ�!", KEY_ESC, 3000);
						goto flag;
					}
					tTaskCfg.bSchNo = (BYTE)iValue;
				}
			}
			else if (listbox.item==4 || listbox.item==5) //��ʼʱ��
			{
				int iValue = 0;
				memset(Tmp, 0, sizeof(Tmp));
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%02d %02d %02d", (tTaskCfg.tmStart.nYear)%100, tTaskCfg.tmStart.nMonth, tTaskCfg.tmStart.nDay);
				sprintf(Tmp, "%02d %02d %02d", tTaskCfg.tmStart.nHour, tTaskCfg.tmStart.nMinute, tTaskCfg.tmStart.nSecond);
				if (EditTextBoxDate(2, "��ʼʱ��(������)", szInput, 60000, 8, DATA_DEC) >= 0)
				{
					if (EditTextBoxTime(2, "��ʼʱ��(ʱ����)", Tmp, 60000, 8, DATA_DEC) >= 0)
					{
						sscanf(szInput, "%d", &iValue);
						tTaskCfg.tmStart.nYear = 2000 + (BYTE)iValue;
						sscanf(szInput+3, "%d", &iValue);
						tTaskCfg.tmStart.nMonth = (BYTE)iValue;
						sscanf(szInput+6, "%d", &iValue);
						tTaskCfg.tmStart.nDay = (BYTE)iValue;
						sscanf(Tmp, "%d", &iValue);
						tTaskCfg.tmStart.nHour = (BYTE)iValue;
						sscanf(Tmp+3, "%d", &iValue);
						tTaskCfg.tmStart.nMinute = (BYTE)iValue;
						sscanf(Tmp+6, "%d", &iValue);
						tTaskCfg.tmStart.nSecond = (BYTE)iValue;
					}
				}		
			}
			else if (listbox.item==6 || listbox.item==7) //����ʱ��
			{
				int iValue = 0;
				memset(Tmp, 0, sizeof(Tmp));
				memset(szInput, 0, sizeof(szInput));
				sprintf(szInput, "%02d %02d %02d", (tTaskCfg.tmEnd.nYear)%100, tTaskCfg.tmEnd.nMonth, tTaskCfg.tmEnd.nDay);
				sprintf(Tmp, "%02d %02d %02d", tTaskCfg.tmEnd.nHour, tTaskCfg.tmEnd.nMinute, tTaskCfg.tmEnd.nSecond);
				if (EditTextBoxDate(2, "����ʱ��(������)", szInput, 60000, 8, DATA_DEC) >= 0)
				{
					if (EditTextBoxTime(2, "����ʱ��(ʱ����)", Tmp, 60000, 8, DATA_DEC) >= 0)
					{
						sscanf(szInput, "%d", &iValue);
						tTaskCfg.tmEnd.nYear = 2000 + (BYTE)iValue;
						sscanf(szInput+3, "%d", &iValue);
						tTaskCfg.tmEnd.nMonth = (BYTE)iValue;
						sscanf(szInput+6, "%d", &iValue);
						tTaskCfg.tmEnd.nDay = (BYTE)iValue;
						sscanf(Tmp, "%d", &iValue);
						tTaskCfg.tmEnd.nHour = (BYTE)iValue;
						sscanf(Tmp+3, "%d", &iValue);
						tTaskCfg.tmEnd.nMinute = (BYTE)iValue;
						sscanf(Tmp+6, "%d", &iValue);
						tTaskCfg.tmEnd.nSecond = (BYTE)iValue;
					}
				}		
			}
			else if (listbox.item == 8) //��ʱ
			{
flag2:			CListBoxEx listbox8;
				struct ListBoxExItem tmp8[] = {
					{ pszTimeUnit[0], 0xFF, Dummy, (void *) 0 }, //
					{ pszTimeUnit[1], 0xFF, Dummy, (void *) 1 }, //
					{ pszTimeUnit[2], 0xFF, Dummy, (void *) 2 }, //
					{ pszTimeUnit[3], 0xFF, Dummy, (void *) 3 }, //
					{ pszTimeUnit[4], 0xFF, Dummy, (void *) 4 }, //
					{ pszTimeUnit[5], 0xFF, Dummy, (void *) 5 }, //
					{ NULL, 0xFF, NULL, NULL }, //
				};

				while(1)
				{
					listbox8.Show(0 , "��ʱ�����λ", tmp8, KEY_ESC | KEY_OK << 8, 60000);
					if (listbox8.key==KEY_NULL || listbox8.key==KEY_ESC)
						break;
					if (listbox8.key == KEY_OK)
					{
						int iValue = 0;

						memset(szInput,0,sizeof(szInput));
						if (EditTextBox(2, "��ʱ���ֵ", szInput, 60000, 5, DATA_DEC) >= 0)
						{
							sscanf(szInput, "%d", &iValue);
							if (iValue > 65535)
							{
								MessageBox("���벻�Ϸ�!", KEY_ESC, 3000);
								goto flag2;
							}
							tTaskCfg.tiDelay.bUnit = (BYTE)tmp8[listbox8.item].arg;
							tTaskCfg.tiDelay.wVal = (WORD)iValue;
							break;
						}
					}
				}
			}
			else if (listbox.item == 9) //ִ�����ȼ�
			{
				CListBoxEx listbox9;
				struct ListBoxExItem tmp9[] = {
					{ pszExePriority[1], 0xFF, Dummy, (void *) 1 }, //
					{ pszExePriority[2], 0xFF, Dummy, (void *) 2 }, //
					{ pszExePriority[3], 0xFF, Dummy, (void *) 3 }, //
					{ pszExePriority[4], 0xFF, Dummy, (void *) 4 }, //
					{ NULL, 0xFF, NULL, NULL }, //
				};
				
				while(1)
				{
					listbox9.Show(0 , "ִ�����ȼ�", tmp9, KEY_ESC | KEY_OK << 8, 60000);
					if (listbox9.key==KEY_NULL || listbox9.key==KEY_ESC)
						break;
					if (listbox9.key == KEY_OK)
					{
						tTaskCfg.bPrio = (BYTE)tmp9[listbox9.item].arg;
						break;
					}
				}
			}
			else if (listbox.item == 10) //״̬
			{
				CListBoxEx listbox10;
				struct ListBoxExItem tmp10[] = {
					{ pszStatus[1], 0xFF, Dummy, (void *) 1 }, //
					{ pszStatus[2], 0xFF, Dummy, (void *) 2 }, //
					{ NULL, 0xFF, NULL, NULL }, //
				};
				
				while(1)
				{
					listbox10.Show(0 , "״̬", tmp10, KEY_ESC | KEY_OK << 8, 60000);
					if (listbox10.key==KEY_NULL || listbox10.key==KEY_ESC)
						break;
					if (listbox10.key == KEY_OK)
					{
						tTaskCfg.bState = (BYTE)tmp10[listbox10.item].arg;
						break;
					}
				}
			}
			else if (listbox.item==11 || listbox.item==12) //�ű�ID
			{
				int iValue = 0;
				char *title = NULL;
				memset(szInput, 0, sizeof(szInput));
				if (listbox.item == 11)
				{
					title = "��ʼǰ�ű�ID";
					sprintf(szInput, "%d", tTaskCfg.wPreScript);
				}
				else
				{
					title = "��ɺ�ű�ID";
					sprintf(szInput, "%d", tTaskCfg.wPostScript);
				}
				if (EditTextBox(2, title, szInput, 60000, 5, DATA_DEC) >= 0)
				{
					sscanf(szInput, "%d", &iValue);
					if (iValue > 65535)
					{
						MessageBox("���벻�Ϸ�!", KEY_ESC, 3000);
						goto flag;
					}
					if (listbox.item==11)
						tTaskCfg.wPreScript = (BYTE)iValue;
					else
						tTaskCfg.wPostScript = (BYTE)iValue;
				}
			}
			else if (listbox.item == 13) //��������ʱ�� */
			if (listbox.item == 13)
			{
				int  CurItem = 0;
				//int PeriodNum = tTaskCfg.bPeriodNum;
				char *title[] = {"ʱ��һ", "ʱ�ζ�", "ʱ����", "ʱ����"};
				struct ListBoxExItem tmp13[] = {
					{"ʱ��һ", 0xFF, Dummy, (void *)0},
					{"ʱ�ζ�", 0xFF, Dummy, (void *)1},
					{"ʱ����", 0xFF, Dummy, (void *)2},
					{"ʱ����", 0xFF, Dummy, (void *)3},
					{ NULL, 0xFF, NULL, NULL }, //
				};
				CListBoxEx listbox13;
				
				
				while(1)
				{
					listbox13.Show(0 , "��������ʱ��", tmp13, KEY_ESC | KEY_OK << 8, 60000);
					if (listbox13.key==KEY_NULL || listbox13.key==KEY_ESC)
						break;
					if (listbox13.key == KEY_OK)
					{
						int i;
						CurItem = listbox13.item;
						CListBoxEx listbox13Tmp;
						char menuTmp[2][32] = {0};
						struct ListBoxExItem tmpItem[] = {
							{menuTmp[0], 0xFE, Dummy, (void *)0},
							{menuTmp[1], 0xFE, Dummy, (void *)1},
							{ NULL, 0xFF, NULL, NULL }, //
						};

						while(1)
						{
							i = 0;
							sprintf(menuTmp[i++], "����:%s", pszPeriodType[tTaskCfg.bPeriodType]);
							sprintf(menuTmp[i++], "ʱ��:%02d:%02d~%02d:%02d", tTaskCfg.period[CurItem].bStarHour,
																			  tTaskCfg.period[CurItem].bStarMin,
																			  tTaskCfg.period[CurItem].bEndHour,
																			  tTaskCfg.period[CurItem].bEndMin);
							tmpItem[i].text = NULL;

							listbox13Tmp.Show(0, title[CurItem], tmpItem, KEY_ESC | KEY_OK << 8, 60000);
							if (listbox13Tmp.key==KEY_NULL || listbox13Tmp.key==KEY_ESC)
								break;
							/*if (listbox13Tmp.key == KEY_OK)
							{

								if (listbox13Tmp.item == 0)
								{
									CListBoxEx listbox13Tmp1;
									struct ListBoxExItem tmpItem1[] = {
										{pszPeriodType[0], 0xFF, Dummy, (void *)0},
										{pszPeriodType[1], 0xFF, Dummy, (void *)1},
										{pszPeriodType[2], 0xFF, Dummy, (void *)2},
										{pszPeriodType[3], 0xFF, Dummy, (void *)3},
										{ NULL, 0xFF, NULL, NULL }, //
									};

									while(1)
									{
										listbox13Tmp1.Show(0, "����", tmpItem1, KEY_ESC | KEY_OK << 8, 60000);
										if (listbox13Tmp1.key==KEY_NULL || listbox13Tmp1.key==KEY_ESC)
											break;
										if (listbox13Tmp1.key == KEY_OK)
										{
											tTaskCfg.bPeriodType = (BYTE)tmpItem1[listbox13Tmp1.item].arg;
											break;
										}
									}
								}
								else if (listbox13Tmp.item == 1)
								{
									int iStartHour, iStartMin, iEndHour, iEndMin;
									char szTmp[32];
									memset(szTmp, 0, sizeof(szTmp));
									memset(szInput, 0, sizeof(szInput));
									sprintf(szInput, "%02d %02d", tTaskCfg.period[CurItem].bStarHour, tTaskCfg.period[CurItem].bStarMin);
									if (EditTextBoxTime(2, "��ʼʱ��(ʱ��)", szInput, 60000, 5, DATA_DEC) >=0 )
									{
										sprintf(szTmp, "%02d %02d", tTaskCfg.period[CurItem].bEndHour, tTaskCfg.period[CurItem].bEndMin);
										if (EditTextBoxTime(2, "����ʱ��(ʱ��)", szTmp, 60000, 5, DATA_DEC) >= 0)
										{
											sscanf(szInput, "%d", &iStartHour);
											sscanf(&szInput[3], "%d", &iStartMin);
											sscanf(szTmp, "%d", &iEndHour);
											sscanf(&szTmp[3], "%d", &iEndMin);
											if (iStartHour<iEndHour && iStartMin<=60 && iEndMin<=60 && iStartHour<=24 && iEndHour<=24)
											{
												tTaskCfg.period[CurItem].bStarHour = (BYTE)iStartHour;
												tTaskCfg.period[CurItem].bStarMin = (BYTE)iStartMin;
												tTaskCfg.period[CurItem].bEndHour = (BYTE)iEndHour;
												tTaskCfg.period[CurItem].bEndMin = (BYTE)iEndMin;
											}
											else
											{
												MessageBox("������������!", KEY_ESC, 3000);
											}
										}
									}
								}
							}*/
						}
					}
				}
			}
			/*else if (listbox.item == 14) //��������
			{
				
			}*/
		}
	}

	return -1;
}

int QueryCommonSchCfg(void *arg)
{
	int i;
	TTaskCfg tTaskCfg;
	TCommAcqSchCfg tCommAcqSchCfg;
	BYTE bIndex;
	char szInput[32] = {0};
	CListBoxEx listbox;
	char menuitem[11][32] = {0};
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 0 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[7], 0xFE, Dummy, (void *) 7 }, //
		{ menuitem[8], 0xFE, Dummy, (void *) 8 }, //
		{ menuitem[9], 0xFE, Dummy, (void *) 9 }, //
		{ menuitem[10], 0xFE, Dummy, (void *) 10 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	char *pszSchType[] = {"�ɼ���ǰ����", "�ɼ��ϵ�N��", "������ʱ��ɼ�", "��ʱ�����ɼ�"};
	char *pszTimeUnit[] = {"��", "��", "ʱ", "��", "��", "��"};
	char *pszMS[] = {"�޵��ܱ�", "ȫ���û���ַ", "һ���û�����", "һ���û���ַ", 
					 "һ���������", "һ���û���������", "һ���û���ַ����", "һ�������������"};
	char *pszStoreTime[] = {"δ����", "����ʼʱ��", "��Ե���0��0��", "�������23��59��", 
							"�������0��0��", "��Ե���1��0��0��"};
	
	if (EditSpecBox(2,"�����뷽�����",szInput,60000,2,DATA_DEC) >= 0)
	{
		int iIndex = 0;
		sscanf(szInput,"%03d",&iIndex);
		if (iIndex>=0 && iIndex<TASK_ID_NUM)
		{
			bIndex = iIndex;
		}
		else
		{
			MessageBox("�����������!", KEY_ESC, 3000);
			return -1;
		}
	}

	if (!GetTaskCfg(bIndex, (TTaskCfg*)&tTaskCfg))
	{
		MessageBox("��ȡ���ó���!", KEY_ESC, 3000);
		return -1;
	}

	if (!GetCommonSchCfg((TTaskCfg*)&tTaskCfg, (TCommAcqSchCfg*)&tCommAcqSchCfg))
	{
		MessageBox("��ȡ���ó���!", KEY_ESC, 3000);
		return -1;
	}

	while(1)
	{
		i = 0;
		sprintf(menuitem[i++], "�������:%d", tCommAcqSchCfg.bSchNo);
		sprintf(menuitem[i++], "�洢���:%d", tCommAcqSchCfg.wStgCnt);
		sprintf(menuitem[i++], "�ɼ�����:");
		sprintf(menuitem[i++], "%s", pszSchType[tCommAcqSchCfg.tTAcqType.bAcqType]);
		//sprintf(menuitem[i++], "�ɼ�����:");
		switch(tCommAcqSchCfg.tTAcqType.bAcqType)
		{
		case 0://�ɼ���ǰ����
			sprintf(menuitem[i++], "�ɼ�����:��");
			break;
		case 1://�ɼ��ϵ�N��
			sprintf(menuitem[i++], "�ɼ�����:��%d��", tCommAcqSchCfg.tTAcqType.bAcqData[0]);
			break;
		case 2://������ʱ��ɼ�
			sprintf(menuitem[i++], "�ɼ�����:��");
			break;
		case 3://��ʱ�����ɼ�
			sprintf(menuitem[i++], "�ɼ�����:%d%s", OoLongUnsignedToWord(&tCommAcqSchCfg.tTAcqType.bAcqData[1]), 
													pszTimeUnit[tCommAcqSchCfg.tTAcqType.bAcqData[0]]);
			break;
		default:
			break;
		}
		sprintf(menuitem[i++], "��¼��ѡ��:(Enter)");
		sprintf(menuitem[i++], "���ܱ���:");
		sprintf(menuitem[i++], "%s", pszMS[tCommAcqSchCfg.bMsChoice]);
		sprintf(menuitem[i++], "�洢ʱ��ѡ��:");
		sprintf(menuitem[i++], "%s", pszStoreTime[tCommAcqSchCfg.bStgTimeScale]);
		tmpS[i].text = NULL;
	
		listbox.Show(0 , "��ͨ�����ɼ�", tmpS, KEY_ESC | KEY_OK << 8, 60000);
		if (listbox.key==KEY_NULL || listbox.key==KEY_ESC)
			break;
		if (listbox.key == KEY_OK)
		{
			if (listbox.item == 5)
			{
				int i;
				char menuTmp[20][16] = {0};
				CListBoxEx listbox6;
				struct ListBoxExItem tmpS6[] = { 
					{ menuTmp[0], 0xFF, Dummy, (void *) 0 }, //
					{ menuTmp[1], 0xFF, Dummy, (void *) 1 }, //
					{ menuTmp[2], 0xFF, Dummy, (void *) 2 }, //
					{ menuTmp[3], 0xFF, Dummy, (void *) 3 }, //
					{ menuTmp[4], 0xFF, Dummy, (void *) 4 }, //
					{ menuTmp[5], 0xFF, Dummy, (void *) 5 }, //
					{ menuTmp[6], 0xFF, Dummy, (void *) 6 }, //
					{ menuTmp[7], 0xFF, Dummy, (void *) 7 }, //
					{ menuTmp[8], 0xFF, Dummy, (void *) 8 }, //
					{ menuTmp[9], 0xFF, Dummy, (void *) 9 }, //
					{ menuTmp[10], 0xFF, Dummy, (void *) 10 }, //
					{ menuTmp[11], 0xFF, Dummy, (void *) 11 }, //
					{ menuTmp[12], 0xFF, Dummy, (void *) 12 }, //
					{ menuTmp[13], 0xFF, Dummy, (void *) 13 }, //
					{ menuTmp[14], 0xFF, Dummy, (void *) 14 }, //
					{ menuTmp[15], 0xFF, Dummy, (void *) 15 }, //
					{ menuTmp[16], 0xFF, Dummy, (void *) 16 }, //
					{ menuTmp[17], 0xFF, Dummy, (void *) 17 }, //
					{ menuTmp[18], 0xFF, Dummy, (void *) 18 }, //
					{ menuTmp[19], 0xFF, Dummy, (void *) 19 }, //
					{ NULL, 0xFE, NULL, NULL }, //
				};

				while(1)
				{
					for (i=0; i<tCommAcqSchCfg.bCSDNum; i++)
					{
						if (tCommAcqSchCfg.tTCSD[i].bChoice == 0)//OAD
						{
							sprintf(menuTmp[i], "OAD:%08X", tCommAcqSchCfg.tTCSD[i].dwOAD);
						}
						else//ROAD
						{
							sprintf(menuTmp[i], "ROAD:%08X", tCommAcqSchCfg.tTCSD[i].tTROAD.dwOAD);
						}
					}
					tmpS6[i].text = NULL;

					listbox6.Show(0, "��¼��ѡ��", tmpS6, KEY_ESC | KEY_OK << 8, 60000);
					if (listbox6.key==KEY_NULL || listbox6.key==KEY_ESC)
						break;
				}
			}
		}
	}

	return -1;
}

int QueryEvtSchCfg(void *arg)
{
	int i;
	TTaskCfg tTaskCfg;
	TEvtAcqSchCfg  tEvtAcqSchCfg;
	BYTE bIndex;
	char szInput[32] = {0};
	CListBoxEx listbox;
	char menuitem[6][32] = {0};
	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 0 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 5 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	char *pszMS[] = {"�޵��ܱ�", "ȫ���û���ַ", "һ���û�����", "һ���û���ַ", 
		"һ���������", "һ���û���������", "һ���û���ַ����", "һ�������������"};

	if (EditSpecBox(2,"�����뷽�����",szInput,60000,2,DATA_DEC) >= 0)
	{
		int iIndex = 0;
		sscanf(szInput,"%03d",&iIndex);
		if (iIndex>=0 && iIndex<TASK_ID_NUM)
		{
			bIndex = iIndex;
		}
		else
		{
			MessageBox("�����������!", KEY_ESC, 3000);
			return -1;
		}
	}

	if (!GetTaskCfg(bIndex, (TTaskCfg*)&tTaskCfg))
	{
		MessageBox("��ȡ���ó���!", KEY_ESC, 3000);
		return -1;
	}

	if (!GetEventSchCfg((TTaskCfg*)&tTaskCfg, (TEvtAcqSchCfg *)&tEvtAcqSchCfg))
	{
		MessageBox("��ȡ���ó���!", KEY_ESC, 3000);
		return -1;
	}

	while(1)
	{
		i = 0;
		sprintf(menuitem[i++], "�������:%d", tEvtAcqSchCfg.bSchNo);
		sprintf(menuitem[i++], "�ɼ����¼�����:(Enter)");
		sprintf(menuitem[i++], "���ܱ���:");
		sprintf(menuitem[i++], "%s", pszMS[tEvtAcqSchCfg.bMsChoice]);
		if (tEvtAcqSchCfg.fRptFlg)
			sprintf(menuitem[i++], "�ϱ���ʶ:�����ϱ�");
		else
			sprintf(menuitem[i++], "�ϱ���ʶ:���ϱ�");
		sprintf(menuitem[i++], "�洢���:%d", tEvtAcqSchCfg.wStgCnt);
		tmpS[i].text = NULL;

		listbox.Show(0 , "�¼������ɼ�", tmpS, KEY_ESC | KEY_OK << 8, 60000);
		if (listbox.key==KEY_NULL || listbox.key==KEY_ESC)
			break;
		if (listbox.key == KEY_OK)
		{
			if (listbox.item == 1)
			{
				int i;
				char menuTmp[32][16] = {0};
				CListBoxEx listbox1;
				struct ListBoxExItem tmpS1[] = { 
					{ menuTmp[0], 0xFF, Dummy, (void *) 0 }, //
					{ menuTmp[1], 0xFF, Dummy, (void *) 1 }, //
					{ menuTmp[2], 0xFF, Dummy, (void *) 2 }, //
					{ menuTmp[3], 0xFF, Dummy, (void *) 3 }, //
					{ menuTmp[4], 0xFF, Dummy, (void *) 4 }, //
					{ menuTmp[5], 0xFF, Dummy, (void *) 5 }, //
					{ menuTmp[6], 0xFF, Dummy, (void *) 6 }, //
					{ menuTmp[7], 0xFF, Dummy, (void *) 7 }, //
					{ menuTmp[8], 0xFF, Dummy, (void *) 8 }, //
					{ menuTmp[9], 0xFF, Dummy, (void *) 9 }, //
					{ menuTmp[10], 0xFF, Dummy, (void *) 10 }, //
					{ menuTmp[11], 0xFF, Dummy, (void *) 11 }, //
					{ menuTmp[12], 0xFF, Dummy, (void *) 12 }, //
					{ menuTmp[13], 0xFF, Dummy, (void *) 13 }, //
					{ menuTmp[14], 0xFF, Dummy, (void *) 14 }, //
					{ menuTmp[15], 0xFF, Dummy, (void *) 15 }, //
					{ menuTmp[16], 0xFF, Dummy, (void *) 16 }, //
					{ menuTmp[17], 0xFF, Dummy, (void *) 17 }, //
					{ menuTmp[18], 0xFF, Dummy, (void *) 18 }, //
					{ menuTmp[19], 0xFF, Dummy, (void *) 19 }, //
					{ menuTmp[20], 0xFF, Dummy, (void *) 20 }, //
					{ menuTmp[21], 0xFF, Dummy, (void *) 21 }, //
					{ menuTmp[22], 0xFF, Dummy, (void *) 22 }, //
					{ menuTmp[23], 0xFF, Dummy, (void *) 23 }, //
					{ menuTmp[24], 0xFF, Dummy, (void *) 24 }, //
					{ menuTmp[25], 0xFF, Dummy, (void *) 25 }, //
					{ menuTmp[26], 0xFF, Dummy, (void *) 26 }, //
					{ menuTmp[27], 0xFF, Dummy, (void *) 27 }, //
					{ menuTmp[28], 0xFF, Dummy, (void *) 28 }, //
					{ menuTmp[29], 0xFF, Dummy, (void *) 29 }, //
					{ menuTmp[30], 0xFF, Dummy, (void *) 30 }, //
					{ menuTmp[31], 0xFF, Dummy, (void *) 31 }, //
					{ NULL, 0xFE, NULL, NULL }, //
				};

				while(1)
				{
					for (i=0; i<tEvtAcqSchCfg.bROADNum; i++)
					{	
						sprintf(menuTmp[i], "ROAD:%08X", tEvtAcqSchCfg.tTROAD[i].dwOAD);
					}
					tmpS1[i].text = NULL;

					listbox1.Show(0, "�ɼ����¼�����", tmpS1, KEY_ESC | KEY_OK << 8, 60000);
					if (listbox1.key==KEY_NULL || listbox1.key==KEY_ESC)
						break;
				}
			}
		}
	}

	return -1;
}


/*
int SetAnalogPara(void *arg)
{
	/ *if (!InputPwd())
	{
		return -1;
	}* /

	char menuitem[7][32];
	BYTE i = 0;
	BYTE tmpBuf[20];
	char szInput[20] = {0};
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;
	int iAnalogNum;
	WORD wNum = 1;
	int iRead = 0;
	char *cAnalogAttrib[] = {"A���ѹ","B���ѹ","C���ѹ","A�����","B�����","C�����","��Ч","��Ч"};

	if (EditSpecBox(2, "������ģ�����˿�(1-64)",szInput,60000,2,DATA_DEC)>=0)
	{
		sscanf(szInput,"%d",&iAnalogNum);
		if (iAnalogNum > 0 && iAnalogNum < 65)
		{
			//ReadItemEx(BN0,PN0,0x00df,tmpBuf);
			tmpBuf[0] = 0x01;
			tmpBuf[1] = iAnalogNum & 0xff;	
			iRead = GBReadItemEx(GB_DATACLASS4, 13, PN0, tmpBuf, &wNum);
			if (iRead < 0 || wNum <= 2)
			{
				memset(tmpBuf,0,sizeof(tmpBuf));
				MessageBox("��ģ�����˿�δ����,�����",KEY_ESC,500);
			}
			while(1)
			{
				i = 0;
				sprintf(menuitem[i++],"ģ��������˿ں�:%d",iAnalogNum);
				sprintf(menuitem[i++],"�������(1-64):%d",tmpBuf[2]);
				
				sprintf(menuitem[i++],"ģ��������:%s",cAnalogAttrib[tmpBuf[3]&0x07]);
				sprintf(menuitem[i++],"��������");
				tmpS[i].text = NULL;

				listbox.Show(0,"ģ��������",tmpS,KEY_ESC | KEY_OK<<8,60000);
				if (listbox.key == KEY_ESC || listbox.key == NULL)
				{
					break;
				}
				if (listbox.key == KEY_OK)
				{
					memset(szInput,0,sizeof(szInput));
					if (listbox.item == 1)
					{
						sprintf(szInput,"%02d",tmpBuf[2]);
						if (EditSpecBox(2, "����������:",szInput,60000,2,DATA_DEC)>=0)
						{
							int Addr1;
							sscanf(szInput,"%02d",&Addr1);
							tmpBuf[2] = (BYTE)Addr1;
						}
					}
					else if (listbox.item == 2)
					{
						CListBoxEx listboxTmp1;
						struct ListBoxExItem tmp1[] = { 
							{ cAnalogAttrib[0], 0xFF, Dummy, (void *) 0x00 },//
							{ cAnalogAttrib[1], 0xFF, Dummy, (void *) 0x01 },//	
							{ cAnalogAttrib[2], 0xFF, Dummy, (void *) 0x02 },//
							{ cAnalogAttrib[3], 0xFF, Dummy, (void *) 0x03 },//
							{ cAnalogAttrib[4], 0xFF, Dummy, (void *) 0x04 },//
							{ cAnalogAttrib[5], 0xFF, Dummy, (void *) 0x05 },//
							{ NULL, 0xFF, NULL, NULL }, //
						};
						listboxTmp1.Show(0, "ģ��������", tmp1, KEY_ESC | (KEY_OK << 8), 60000);
						if (listboxTmp1.key == KEY_OK)
						{
							tmpBuf[3] &= 0xF8;
							tmpBuf[3] |= ((BYTE)((int)tmp1[listboxTmp1.item].arg));
						}	
					}
					else if (listbox.item == 3)
					{
						if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
						{
							tmpBuf[0] = 0x01;
							tmpBuf[1] = iAnalogNum & 0xff;
							if(GBWriteItemEx_Class4(13, PN0, tmpBuf)>0)
							{
								MessageBox("���óɹ�",KEY_ESC,3000);
								TrigerSaveBank(BN0, SECT_TERMN_PARA, -1);	//��������һ��
								DoTrigerSaveBank();
							}
							else
							{
								MessageBox("����ʧ��",KEY_ESC,2000);
							}
						}
					}
				}
			}
		}
		else
		{
			MessageBox("����˿��������!",KEY_ESC,3000);
		}
	}
	return -1;
}*/

int SetStatusPara(void *arg)
{
	if (!InputPwd())
	{
		return -1;
	}

	char menuitem[3][32];
	BYTE i = 0;
	
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;
	int iPageNum = 0;
	char *cStatusFlag[] = {"δ����","����"};
	char *cStatusAttrib[] = {"���մ���","��������"};
	
	BYTE *pbFmt4;
	WORD wFmt4Len;
	typedef struct {
		BYTE bStructType;
		BYTE bparserNum;
		BYTE bParser1Type;
		BYTE bParser1Len;
	BYTE bAccessSign;
		BYTE bParser2Type;
		BYTE bParser2Len;
	BYTE bAttributeSign;
	}TSwitchInfo;

	TSwitchInfo tSwitchInfo;
	memset(&tSwitchInfo, 0, sizeof(TSwitchInfo));

	if (0 > OoReadAttr(0xF203, 0x04, (BYTE *)&tSwitchInfo, &pbFmt4, &wFmt4Len))
	{
		MessageBox("��ȡ���ݿ����!", KEY_ESC, 6000);
		return -1;
	}

	while(1)
	{
		switch(iPageNum)
		{
		//case 0:
		case 0:
			i = 0;
			//sprintf(menuitem[i++],"%d·�����־:%s",iPageNum*4 +1,cStatusFlag[tSwitchInfo.bAccessSign>>(iPageNum*4+0) &0x01]);
			//sprintf(menuitem[i++],"%d·�����־:%s",iPageNum*4 +2,cStatusFlag[tSwitchInfo.bAccessSign>>(iPageNum*4+1) &0x01]);
			//sprintf(menuitem[i++],"%d·�����־:%s",iPageNum*4 +3,cStatusFlag[tSwitchInfo.bAccessSign>>(iPageNum*4+2) &0x01]);
			//sprintf(menuitem[i++],"%d·�����־:%s",iPageNum*4 +4,cStatusFlag[tSwitchInfo.bAccessSign>>(iPageNum*4+3) &0x01]);
			sprintf(menuitem[i++],"һ·�����־:%s",cStatusFlag[tSwitchInfo.bAccessSign&0x01]);
			sprintf(menuitem[i++],"��·�����־:%s",cStatusFlag[(tSwitchInfo.bAccessSign>>1)&0x01]);
			sprintf(menuitem[i++],"��������");
			tmpS[i].text = NULL;

			listbox.Show(0,"״̬����������",tmpS,KEY_ESC | KEY_OK<<8 | KEY_LEFT<<16 | KEY_RIGHT<<24,60000);
			if (listbox.key == KEY_ESC || listbox.key == NULL)
			{
				return -1;
			}
			if (listbox.key ==	KEY_LEFT)
			{
				iPageNum--;
				if (iPageNum < 0)
				{
					iPageNum = 1;
					//iPageNum = 3;
				}
			}
			else if (listbox.key == KEY_RIGHT)
			{
				iPageNum++;
				//if (iPageNum > 3)
				if (iPageNum > 1)
				{
					iPageNum = 0;
				}
			}
			else if (listbox.key == KEY_OK)
			{
				//if (listbox.item < 4)
				if (listbox.item < 2)
				{
					CListBoxEx listboxTmp1;
					struct ListBoxExItem tmp1[] = { 
						{ cStatusFlag[0], 0xFF, Dummy, (void *) 0x00 },//
						{ cStatusFlag[1], 0xFF, Dummy, (void *) 0x01 },//	
						{ NULL, 0xFF, NULL, NULL }, //
					};
					listboxTmp1.Show(0, "״̬�������־", tmp1, KEY_ESC | (KEY_OK << 8), 60000);
					if (listboxTmp1.key == KEY_OK)
					{
						//if (iPageNum == 0)
						{							
							if (listbox.item == 0)
							{
								tSwitchInfo.bAccessSign &= 0xfe;
								tSwitchInfo.bAccessSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg));
							}
							else if (listbox.item == 1)
							{
								tSwitchInfo.bAccessSign &= 0xfd;
								tSwitchInfo.bAccessSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<1;
							}
							/*else if (listbox.item == 2)
							{
								tSwitchInfo.bAccessSign &= 0xfb;
								tSwitchInfo.bAccessSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<2;
							}
							else if (listbox.item == 3)
							{
								tSwitchInfo.bAccessSign &= 0xf7;
								tSwitchInfo.bAccessSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<3;
							}*/
						}
						/*else if (iPageNum == 1)
						{
							if (listbox.item == 0)
							{
								tSwitchInfo.bAccessSign &= 0xef;
								tSwitchInfo.bAccessSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<4;

							}
							else if (listbox.item == 1)
							{
								tSwitchInfo.bAccessSign &= 0xdf;
								tSwitchInfo.bAccessSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<5;
							}
							else if (listbox.item == 2)
							{
								tSwitchInfo.bAccessSign &= 0xbf;
								tSwitchInfo.bAccessSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<6;
							}
							else if (listbox.item == 3)
							{
								tSwitchInfo.bAccessSign &= 0x7f;
								tSwitchInfo.bAccessSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<7;
						}
						}*/
					}	
				}
				//else if (listbox.item == 4)
				else if (listbox.item == 2)
				{
					if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
					{
						tSwitchInfo.bParser1Len = 2;   //Э��֧��8·��������Ŀǰ�ն�ֻ֧��2·������ǿ������2·��Ч
						tSwitchInfo.bParser2Len = 2;
						if (OoWriteAttr(0xF203, 0x04, (BYTE *)&tSwitchInfo) > 0)
						{
							MessageBox("���óɹ�",KEY_ESC,3000);
							TrigerSaveBank(BN0, SECT10, -1);	//��������һ��
							DoTrigerSaveBank();
						}
						else
						{
							MessageBox("����ʧ��",KEY_ESC,2000);
						}
					}
				}
			}
			break;
		//case 2:
		case 1:
			i = 0;
			//sprintf(menuitem[i++],"%d·���Ա�־:%s",(iPageNum-2)*4 +1,cStatusAttrib[tSwitchInfo.bAttributeSign>>((iPageNum-2)*4+0) &0x01]);
			//sprintf(menuitem[i++],"%d·���Ա�־:%s",(iPageNum-2)*4 +2,cStatusAttrib[tSwitchInfo.bAttributeSign>>((iPageNum-2)*4+1) &0x01]);
			//sprintf(menuitem[i++],"%d·���Ա�־:%s",(iPageNum-2)*4 +3,cStatusAttrib[tSwitchInfo.bAttributeSign>>((iPageNum-2)*4+2) &0x01]);
			//sprintf(menuitem[i++],"%d·���Ա�־:%s",(iPageNum-2)*4 +4,cStatusAttrib[tSwitchInfo.bAttributeSign>>((iPageNum-2)*4+3) &0x01]);
			sprintf(menuitem[i++],"һ·���Ա�־:%s", cStatusAttrib[tSwitchInfo.bAttributeSign&0x01]);
			sprintf(menuitem[i++],"��·���Ա�־:%s", cStatusAttrib[(tSwitchInfo.bAttributeSign>>1)&0x01]);
			sprintf(menuitem[i++],"��������");
			tmpS[i].text = NULL;
			listbox.Show(0,"״̬����������",tmpS,KEY_ESC | KEY_OK<<8 | KEY_LEFT<<16 | KEY_RIGHT<<24,60000);
			if (listbox.key == KEY_ESC || listbox.key == NULL)
			{
				return -1;
			}
			if (listbox.key ==	KEY_LEFT)
			{
				iPageNum--;
				if (iPageNum < 0)
				{
					iPageNum = 1;
					//iPageNum = 3;
				}
			}
			else if (listbox.key == KEY_RIGHT)
			{
				iPageNum++;
				//if (iPageNum > 3)
				if (iPageNum > 1)
				{
					iPageNum = 0;
				}
			}
			else if (listbox.key == KEY_OK)
			{
				//if (listbox.item < 4)
				if (listbox.item < 2)
				{
					CListBoxEx listboxTmp1;
					struct ListBoxExItem tmp1[] = { 
						{ cStatusAttrib[0], 0xFF, Dummy, (void *) 0x00 },//
						{ cStatusAttrib[1], 0xFF, Dummy, (void *) 0x01 },//	
						{ NULL, 0xFF, NULL, NULL }, //
					};
					listboxTmp1.Show(0, "״̬�����Ա�־", tmp1, KEY_ESC | (KEY_OK << 8), 60000);
					if (listboxTmp1.key == KEY_OK)
					{
						//if (iPageNum == 2)
						{
							if (listbox.item == 0)
							{
								tSwitchInfo.bAttributeSign &= 0xfe;
								tSwitchInfo.bAttributeSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg));
							}
							else if (listbox.item == 1)
							{
								tSwitchInfo.bAttributeSign &= 0xfd;
								tSwitchInfo.bAttributeSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<1;
							}
							/*else if (listbox.item == 2)
							{
								tSwitchInfo.bAttributeSign &= 0xfb;
								tSwitchInfo.bAttributeSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<2;
							}
							else if (listbox.item == 3)
							{
								tSwitchInfo.bAttributeSign &= 0xf7;
								tSwitchInfo.bAttributeSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<3;
							}*/
						}
						/*else if (iPageNum == 3)
						{
							if (listbox.item == 0)
							{
								tSwitchInfo.bAttributeSign &= 0xef;
								tSwitchInfo.bAttributeSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<4;
							}
							else if (listbox.item == 1)
							{
								tSwitchInfo.bAttributeSign &= 0xdf;
								tSwitchInfo.bAttributeSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<5;
							}
							else if (listbox.item == 2)
							{
								tSwitchInfo.bAttributeSign &= 0xbf;
								tSwitchInfo.bAttributeSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<6;
							}
							else if (listbox.item == 3)
							{
								tSwitchInfo.bAttributeSign &= 0x7f;
								tSwitchInfo.bAttributeSign |= ((BYTE)((int)tmp1[listboxTmp1.item].arg))<<7;
						}
						}*/
					}	
				}
				//else if (listbox.item == 4)
				else if (listbox.item == 2)
				{
					if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
					{
						tSwitchInfo.bParser1Len = 8;   //Э��֧��8·��������Ŀǰ�ն�ֻ֧��2·��������Ȼ��������8·����ֻ��2·��Ч
						tSwitchInfo.bParser2Len = 8;
						if (OoWriteAttr(0xF203, 0x04, (BYTE *)&tSwitchInfo) > 0)
						{
							MessageBox("���óɹ�",KEY_ESC,3000);
							TrigerSaveBank(BN0, SECT10, -1);	//��������һ��
							DoTrigerSaveBank();
						}
						else
						{
							MessageBox("����ʧ��",KEY_ESC,2000);
						}
					}
				}
			}
			break;
		 }
	}
	return -1;
}


int SetMPBasicPara(void *arg)
{
	if (!InputPwd())
	{
		return -1;
	}

	char menuitem[22][32];
	BYTE i = 0;
	char szInput[33] = {0};
	int iMpNo = 1;

	TOobMtrInfo tTMtrInfo;
	memset(&tTMtrInfo, 0, sizeof(TOobMtrInfo));
	char szTmp[33] = {0};
	BYTE bTmpBuf[33] = {0};
	WORD wPn;
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *)1}, //
		{ menuitem[1], 0xFE, Dummy, (void *)2}, //
		{ menuitem[2], 0xFE, Dummy, (void *)3}, //
		{ menuitem[3], 0xFE, Dummy, (void *)4}, //
		{ menuitem[4], 0xFE, Dummy, (void *)5}, //
		{ menuitem[5], 0xFE, Dummy, (void *)6}, //
		{ menuitem[6], 0xFE, Dummy, (void *)7}, //
		{ menuitem[7], 0xFE, Dummy, (void *)8}, //
		{ menuitem[8], 0xFE, Dummy, (void *)9}, //
		{ menuitem[9], 0xFE, Dummy, (void *)10}, //
		{ menuitem[10], 0xFE, Dummy, (void *)11}, //
		{ menuitem[11], 0xFE, Dummy, (void *)12}, //
		{ menuitem[12], 0xFE, Dummy, (void *)13}, //
		{ menuitem[13], 0xFE, Dummy, (void *)14}, //
		{ menuitem[14], 0xFE, Dummy, (void *)15}, //
		{ menuitem[15], 0xFE, Dummy, (void *)16}, //
		{ menuitem[16], 0xFE, Dummy, (void *)17}, //
		{ menuitem[17], 0xFE, Dummy, (void *)18}, //
		{ menuitem[18], 0xFE, Dummy, (void *)18}, //
		{ menuitem[19], 0xFE, Dummy, (void *)19}, //
		{ menuitem[20], 0xFE, Dummy, (void *)20}, //
		{ menuitem[21], 0xFE, Dummy, (void *)21}, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;
	char *ConnectType[] = {"δ֪","�����","��������","��������"};
	char *MtrProType[] = {"δ֪", "DL/T645-1997", "DL/T645-2007", "DL/T698.45", "CJ/T 188-2004"};
	char *Boundrate[] = {"300bps", "600bps", "1200bps", "2400bps", "4800bps", "7200bps", 
						 "9600bps", "19200bps", "38400bps", "57600bps", "115200bps", "����Ӧ"};
	
	if (EditSpecBox(2,"��������:",szInput,60000,2,DATA_DEC)>=0)
	{
		sscanf(szInput,"%d",&iMpNo);
		if (iMpNo >= 1 & iMpNo < POINT_NUM)
		{
			/*
			**��õ����Ϣ
			*/
			wPn = MtrSnToPn(iMpNo);
			if (!GetMeterInfo(wPn, &tTMtrInfo))
			{
				if(MessageBox("�õ��δ����!", KEY_ESC, 3000))
                {
                }
                else
                {
				    return -1;
                }
			}

			while(1)
			{
				i = 0;
				sprintf(menuitem[i++],"�����:%d",iMpNo);

				memset(bTmpBuf, 0, sizeof(bTmpBuf));
				memset(szTmp, 0, sizeof(szTmp));
				HexToASCII(tTMtrInfo.bTsa, (BYTE *)szTmp, (WORD)tTMtrInfo.bTsaLen);
				memcpy(bTmpBuf, &szTmp[0], 16);
				sprintf(menuitem[i++], "���ַ:");
				sprintf(menuitem[i++], "%s", (char *)bTmpBuf); //���ַ
				memcpy(bTmpBuf, &szTmp[16], 16);
				if (IsAllAByte(bTmpBuf, 0, sizeof(bTmpBuf)))
					sprintf(menuitem[i++], " "); //����û���
				else
					sprintf(menuitem[i++], "%s", (char *)bTmpBuf);

				sprintf(menuitem[i++], "������:%s", Boundrate[(tTMtrInfo.bBps)&0x0F]);

				sprintf(menuitem[i++], "��Լ����:%s", MtrProType[(tTMtrInfo.bProType)&0x07]);
				
				sprintf(menuitem[i++], "�˿�:%08X", tTMtrInfo.dwPortOAD);

				memset(bTmpBuf, 0, sizeof(bTmpBuf));
				memset(szTmp, 0, sizeof(szTmp));
				HexToASCII(tTMtrInfo.bCode, (BYTE *)szTmp, (WORD)tTMtrInfo.bCodeLen);
				sprintf(menuitem[i++], "ͨ������:");
				memcpy(bTmpBuf, &szTmp[0], 16);
				sprintf(menuitem[i++], "%s", (char *)bTmpBuf); //ͨ������
				memcpy(bTmpBuf, &szTmp[16], 16);
				if (IsAllAByte(bTmpBuf, 0, sizeof(bTmpBuf)))
					sprintf(menuitem[i++], " "); //����û���
				else
					sprintf(menuitem[i++], "%s", (char *)bTmpBuf);

				sprintf(menuitem[i++], "���ʸ���:%d", tTMtrInfo.bRate);

				sprintf(menuitem[i++], "�û�����:%d", tTMtrInfo.bUserType);

				sprintf(menuitem[i++],"���߷�ʽ:%s",ConnectType[(tTMtrInfo.bLine)&0x03]);

				memset(bTmpBuf, 0, sizeof(bTmpBuf));
				memset(szTmp, 0, sizeof(szTmp));
				HexToASCII(tTMtrInfo.bAcqTsa, (BYTE *)szTmp, (WORD)tTMtrInfo.bAcqTsaLen);
				memcpy(bTmpBuf, &szTmp[0], 16);
				sprintf(menuitem[i++], "�ɼ�����ַ:");
				sprintf(menuitem[i++], "%s", (char *)bTmpBuf); //�ɼ�����ַ
				memcpy(bTmpBuf, &szTmp[16], 16);
				if (IsAllAByte(bTmpBuf, 0, sizeof(bTmpBuf)))
					sprintf(menuitem[i++], " "); //����û���
				else
					sprintf(menuitem[i++], "%s", (char *)bTmpBuf);

				memset(bTmpBuf, 0, sizeof(bTmpBuf));
				memset(szTmp, 0, sizeof(szTmp));
				HexToASCII(tTMtrInfo.bAsset, (BYTE *)szTmp, (WORD)tTMtrInfo.bAssetLen);
				memcpy(bTmpBuf, &szTmp[0], 16);
				sprintf(menuitem[i++], "�ʲ���:");
				sprintf(menuitem[i++], "%s", (char *)bTmpBuf); //�ʲ���
				memcpy(bTmpBuf, &szTmp[16], 16);
				if (IsAllAByte(bTmpBuf, 0, sizeof(bTmpBuf)))
					sprintf(menuitem[i++], " "); //����û���
				else
					sprintf(menuitem[i++], "%s", (char *)bTmpBuf);

				sprintf(menuitem[i++],"��ѹ����������:%d",tTMtrInfo.wPT);
				sprintf(menuitem[i++],"��������������:%d",tTMtrInfo.wCT);

				sprintf(menuitem[i++],"��������");
				listbox.Show(0,"���ܱ��������",tmpS,KEY_ESC | KEY_OK << 8,60000);
				if (listbox.key == NULL || listbox.key ==  KEY_ESC)
				{
					break;
				}
				if (listbox.key == KEY_OK)
				{
					if (listbox.item==2 || listbox.item==3)
					{
						memset(szInput, 0, sizeof(szInput));
						HexToASCII(tTMtrInfo.bTsa, (BYTE *)szInput, (WORD)tTMtrInfo.bTsaLen);
						if (EditTextBox(2, "���ַ", szInput, 60000, 32, DATA_DEC) >= 0)
						{
							WORD wLen = 0;
							memcpy(szTmp, szInput, 32);

							char *pTail = strstr(szTmp, " ");
							if(pTail)
							{
								memset(pTail, 0x00, &(szTmp[31])-pTail+1);
							}
							wLen = strlen(szTmp);
							AsciiToByte((BYTE *)szTmp, wLen, tTMtrInfo.bTsa);
							tTMtrInfo.bTsaLen = wLen / 2;
						}
					}
					else if (listbox.item == 4)
					{
						CListBoxEx listboxTmp0;
						struct ListBoxExItem tmp0[] = {
							{Boundrate[0], 0xFF, Dummy, (void *)0x00},
							{Boundrate[1], 0xFF, Dummy, (void *)0x01},
							{Boundrate[2], 0xFF, Dummy, (void *)0x02},
							{Boundrate[3], 0xFF, Dummy, (void *)0x03},
							{Boundrate[4], 0xFF, Dummy, (void *)0x04},
							{Boundrate[5], 0xFF, Dummy, (void *)0x05},
							{Boundrate[6], 0xFF, Dummy, (void *)0x06},
							{Boundrate[7], 0xFF, Dummy, (void *)0x07},
							{Boundrate[8], 0xFF, Dummy, (void *)0x08},
							{Boundrate[9], 0xFF, Dummy, (void *)0x09},
							{Boundrate[10], 0xFF, Dummy, (void *)0x10},
							{Boundrate[11], 0xFF, Dummy, (void *)0x11},
							{ NULL, 0xFF, NULL, NULL },
						};
						listboxTmp0.Show(0, "������", tmp0, KEY_ESC | (KEY_OK << 8), 60000);
						if (listboxTmp0.key == KEY_OK)
						{
							tTMtrInfo.bBps = ((BYTE)((int)tmp0[listboxTmp0.item].arg));
						}
					}
					else if (listbox.item == 5)
					{
						CListBoxEx listboxTmp1;
						struct ListBoxExItem tmp1[] = {
							{MtrProType[1], 0xFF, Dummy, (void *)0x01},
							{MtrProType[2], 0xFF, Dummy, (void *)0x02},
							{MtrProType[3], 0xFF, Dummy, (void *)0x03},
							{MtrProType[4], 0xFF, Dummy, (void *)0x04},
							{ NULL, 0xFF, NULL, NULL },
						};
						listboxTmp1.Show(0, "��Լ����", tmp1, KEY_ESC | (KEY_OK << 8), 60000);
						if (listboxTmp1.key == KEY_OK)
						{
							tTMtrInfo.bProType = ((BYTE)((int)tmp1[listboxTmp1.item].arg));
						}
					}
					else if (listbox.item == 6)
					{
						memset(szInput, 0, sizeof(szInput));
						memset(szTmp, 0, sizeof(szTmp));
						//OoDWordToOad(tTMtrInfo.dwPortOAD, (BYTE *)szTmp);
						//HexToASCII((BYTE *)szTmp, (BYTE *)szInput, 4);
						sprintf(szInput, "%04X", tTMtrInfo.dwPortOAD);						
						if (EditTextBox(2, "�˿�", szInput, 60000, 8, DATA_HEX) >= 0)
						{
							memset(bTmpBuf, 0, sizeof(bTmpBuf));
							AsciiToByte((BYTE *)szInput, 8, bTmpBuf);
							tTMtrInfo.dwPortOAD = OoOadToDWord(bTmpBuf);
						}
					}
					else if (listbox.item==8 || listbox.item==9)
					{
						memset(szInput, 0, sizeof(szInput));
						HexToASCII(tTMtrInfo.bCode, (BYTE *)szInput, tTMtrInfo.bCodeLen);
						if (EditTextBox(2, "ͨ������", szInput, 60000, 32, DATA_DEC) >= 0)
						{
							WORD wLen = 0;
							memcpy(szTmp, szInput, 32);
							
							char *pTail = strstr(szTmp, " ");
							if(pTail)
							{
								memset(pTail, 0x00, &(szTmp[31])-pTail+1);
							}
							wLen = strlen(szTmp);
							AsciiToByte((BYTE *)szTmp, wLen, tTMtrInfo.bCode);
							tTMtrInfo.bCodeLen = wLen / 2;
						}
					}
					else if (listbox.item == 10)
					{
						memset(szInput, 0, sizeof(szInput));
						sprintf(szInput, "%d", tTMtrInfo.bRate);
						if (EditTextBox(2, "���ʸ���", szInput, 60000, 3, DATA_DEC) >= 0)
						{
							int FeilvNum = 0;
							sscanf(szInput, "%d", &FeilvNum);
							tTMtrInfo.bRate = FeilvNum;
						}
					}
					else if (listbox.item == 11)
					{
						memset(szInput, 0, sizeof(szInput));
						sprintf(szInput,"%d",tTMtrInfo.bUserType);
						if (EditTextBox(2, "�û�����", szInput, 60000, 3, DATA_DEC) >= 0)
						{
							int UserType = 0;
							sscanf(szInput, "%d", &UserType);
							tTMtrInfo.bUserType = UserType;
						}
					}
					else if (listbox.item == 12)
					{
						CListBoxEx listboxTmp1;
						struct ListBoxExItem tmp2[] = { 
							{ ConnectType[1], 0xFF, Dummy, (void *) 0x01 },//	
							{ ConnectType[2], 0xFF, Dummy, (void *) 0x02 },//
							{ ConnectType[3], 0xFF, Dummy, (void *) 0x03 },//
							{ NULL, 0xFF, NULL, NULL }, //
						};
						listboxTmp1.Show(0, "���߷�ʽ", tmp2, KEY_ESC | (KEY_OK << 8), 60000);
						if (listboxTmp1.key == KEY_OK)
						{
							tTMtrInfo.bLine = ((BYTE)((int)tmp2[listboxTmp1.item].arg));
						}
					}
					else if (listbox.item==14 || listbox.item==15)
					{
						memset(szInput, 0, sizeof(szInput));
						if (EditTextBox(2, "�ɼ�����ַ", szInput, 60000, 32, DATA_DEC) >= 0)
						{
							WORD wLen = 0;
							memcpy(szTmp, szInput, 32);

							char *pTail = strstr(szTmp, " ");
							if(pTail)
							{
								memset(pTail, 0x00, &(szTmp[31])-pTail+1);
							}
							wLen = strlen(szTmp);
							AsciiToByte((BYTE *)szTmp, wLen, tTMtrInfo.bAcqTsa);
							tTMtrInfo.bAcqTsaLen = wLen / 2;
						}
					}
					else if (listbox.item==17 || listbox.item==18)
					{
						memset(szInput, 0, sizeof(szInput));
						if (getSoftKey("�ʲ���", szInput, 60000, 32, DATA_DEC) >= 0)
						{
							WORD wLen = 0;
							memcpy(szTmp, szInput, 32);

							char *pTail = strstr(szTmp, " ");
							if(pTail)
							{
								memset(pTail, 0x00, &(szTmp[31])-pTail+1);
							}
							wLen = strlen(szTmp);
							AsciiToByte((BYTE *)szTmp, wLen, tTMtrInfo.bAsset);
							tTMtrInfo.bAssetLen = wLen / 2;
						}
					}
					else if (listbox.item == 19)
					{
						//sprintf(szInput,"%d",ByteToDWORD(&tmpBuf[0],2));
						memset(szInput, 0, sizeof(szInput));
						sprintf(szInput,"%d",tTMtrInfo.wPT);
						if (EditTextBox(2,"��ѹ����������",szInput,60000,5,DATA_DEC)>=0)
						{
							int BeiLvV = 0;
							sscanf(szInput,"%d",&BeiLvV); 
							tTMtrInfo.wPT = (WORD)BeiLvV;
						}
					}
					else if (listbox.item == 20)
					{
						memset(szInput, 0, sizeof(szInput));
						sprintf(szInput,"%d", tTMtrInfo.wCT);
						if(EditTextBox(2,"��������������",szInput,60000,5,DATA_DEC)>=0)
						{
							int BeiLvI = 0;
							sscanf(szInput,"%d",&BeiLvI);
							tTMtrInfo.wCT = BeiLvI;
						}
					}
					else if (listbox.item == 21)
					{
						if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
						{
							if (SetMeterInfo(wPn, tTMtrInfo))
							{
								MessageBox("���óɹ�",KEY_ESC,3000);
								TrigerSaveBank(BN0, SECT6, -1);	//��������һ��
								DoTrigerSaveBank();
							}
							else
							{
								MessageBox("����ʧ��",KEY_ESC,2000);
							}
						}
					}
				}
			}
		}
		else
		{
			MessageBox("�������Ŵ���!",KEY_ESC,3000);
		}
	}
	
	return -1;
}

	
/*
int SetGroupPara(void *arg)
{
	/ *if (!InputPwd())
	{
		return -1;
	}* /
	char menuitem[7][32];
	char szInput[20] = {0};
	int iGroupNo = 0;
	int iMpNum = 0;
	BYTE TmpBuff[70] = {0};
	BYTE i= 0;
	memset(menuitem, 0, sizeof(menuitem));
	char *GroupFlag[] = {"����","����"};
	char *CalcFlag[] = {"��","��"};

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ menuitem[5], 0xFE, Dummy, (void *) 6 }, //
		{ menuitem[6], 0xFE, Dummy, (void *) 7 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};
	CListBoxEx listbox;

	if (EditSpecBox(2, "�������ܼ����(1-8)",szInput,60000,1,DATA_DEC8) >= 0)
	{
		sscanf(szInput,"%d",&iGroupNo);
		if (iGroupNo >= GB_MAXOFF && iGroupNo < GB_MAXMETER)
		{
			//if (!IsGrpValid(iGroupNo))
			//{
				
				
			//}
			memset(szInput,0,sizeof(szInput));
			if(ReadItemEx(BN0,iGroupNo,0x8905,TmpBuff)>0)
			{
				if (TmpBuff[0] > 0)
				{
					sprintf(szInput,"%d",TmpBuff[0]);
				}
				else
				{
					MessageBox("���ܼ������δ����!",KEY_ESC,500);

					ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());//��һ����Ļ��������ѿ�
				}
			}
			
			if (EditSpecBox(2, "�������������(1-64)",szInput,60000,2,DATA_DEC) >= 0)
			{
				sscanf(szInput,"%d",&iMpNum);
				int MpNo = 0;
				if (iMpNum >= GB_MAXOFF && iMpNum < GB_MAXMETER)
				{
					ReadItemEx(BN0,iGroupNo,0x8905,TmpBuff);
					while(1)
					{
						i = 0;
						sprintf(menuitem[i++],"�ܼ����:%d",iGroupNo);
						sprintf(menuitem[i++],"����������:%d",iMpNum);
						sprintf(menuitem[i++],"��%d��",MpNo + 1);
						sprintf(menuitem[i++],"�������(1-64):%d",(TmpBuff[1 + MpNo] & 0x3f) + 1);
						sprintf(menuitem[i++],"�ܼӱ�־:%s",GroupFlag[(TmpBuff[1 + MpNo]>>6) & 0x01]);
						sprintf(menuitem[i++],"�������־:%s",CalcFlag[(TmpBuff[1 + MpNo]>>7) & 0x01]);
						if (MpNo+1 == iMpNum)	//���һҳ
							sprintf(menuitem[i++],"��������");

						listbox.Show(0,"�ܼ�������",tmpS,KEY_ESC | KEY_OK<<8 | KEY_LEFT<<16 | KEY_RIGHT<<24,60000);
						if (listbox.key == NULL || listbox.key == KEY_ESC)
						{
							return -1;
						}
						
						if(listbox.key == KEY_OK)
						{
							if (listbox.item == 3)
							{
								memset(szInput,0,sizeof(szInput));
								if(EditSpecBox(2, "�������",szInput,60000,2,DATA_DEC)>=0)
								{
									DWORD kk;
									sscanf(szInput,"%d",&kk);
									TmpBuff[1 + MpNo] &= 0xc0;
									TmpBuff[1 + MpNo] |= (BYTE)(kk & 0x3f) - 1;
								}
							}
							else if (listbox.item == 4)
							{
								CListBoxEx listboxTmp1;
								struct ListBoxExItem tmp1[] = { 
									{ GroupFlag[0], 0xFF, Dummy, (void *) 0x00 },//	
									{ GroupFlag[1], 0xFF, Dummy, (void *) 0x01 },//
									{ NULL, 0xFF, NULL, NULL }, //
								};
								listboxTmp1.Show(0, "�ܼӱ�־",tmp1,KEY_ESC | (KEY_OK << 8), 60000);
								if (listboxTmp1.key == KEY_OK)
								{
									TmpBuff[1 + MpNo] &= 0xbf;
									TmpBuff[1 + MpNo] |= ((BYTE)((int)tmp1[listboxTmp1.item].arg)) << 6;
								}
							}
							else if (listbox.item == 5)
							{
								CListBoxEx listboxTmp2;
								struct ListBoxExItem tmp2[] = { 
									{ CalcFlag[0], 0xFF, Dummy, (void *) 0x00 },//	
									{ CalcFlag[1], 0xFF, Dummy, (void *) 0x01 },//
									{ NULL, 0xFF, NULL, NULL }, //
								};
								listboxTmp2.Show(0, "�����־", tmp2, KEY_ESC | (KEY_OK << 8), 60000);
								if (listboxTmp2.key == KEY_OK)
								{
									TmpBuff[1 + MpNo] &= 0x7f;
									TmpBuff[1 + MpNo] |= ((BYTE)((int)tmp2[listboxTmp2.item].arg)) << 7;
								}
							}
							else if (listbox.item == 6)
							{								
								if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0 && MpNo+1 == iMpNum)
								{
									TmpBuff[0] = iMpNum;
									if(WriteItemEx(BN0,iGroupNo,0x8905,TmpBuff)> 0 )
									{
										MessageBox("���óɹ�",KEY_ESC,3000);
										TrigerSaveBank(BN0, SECT_TERMN_PARA, -1);	//��������һ��
										DoTrigerSaveBank();
									}
									else
									{
										MessageBox("����ʧ��",KEY_ESC,2000);
									}
								}
							}
						}
						else if (listbox.key == KEY_LEFT)
						{
							MpNo--;
							if (MpNo < 0)
							{
								MpNo = 0;
							}
						}
						else if (listbox.key == KEY_RIGHT)
						{
							MpNo++;
							if (MpNo >= iMpNum)
							{
								MpNo = iMpNum-1;
							}
						}
					}
				}
				else
				{
					MessageBox("���������������!",KEY_ESC,3000);
					return -1;
				}
			}
		}
		else
		{
			MessageBox("�����ܼ������!",KEY_ESC,3000);
			return -1;
		}
	}
}
*/


int SetPara(void *arg)
{
	struct ListBoxExItem items[] = {
	//	{(char *)"ͨ�Ų�������",0xFF,SetCommunicationStatePara,(void*)1},
	//	{(char *)"�ն���Ϣ����",0xFF,SetCommunicationPara,(void*)2},
		{(char *)"�ն�ʱ������",		0xFF,	SetMySysTime,				(void*)1},
	//	{(char *)"�������������",		0xFF,	SetMpPara,					(void*)2},
	//	{(char *)"��������չ����",0xFF,SetMpExtPara,(void*)5},
	//	{(char *)"�����������",0xFF,SetPulsePara,(void*)3},
	//	{(char *)"�ܼ�������",0xFF,SetGroupPara,(void*)4},
		{(char *)"״̬������",0xFF,SetStatusPara,(void*)5},
		{(char *)"�̵����������",0xFF,SetRelayOutput,(void*)6},
		{(char *)"�������������",0xFF,SetMPBasicPara,(void*)7},
	//	{(char *)"ģ��������",0xFF,SetAnalogPara,(void*)2}, //D82 û�У�������
	//	{(char *)"����ר������",0xFF,SetVPNPara,(void*)3},
	//	{(char *)"������������",0xFF,Dummy,(void*)4},
		{ NULL, 0xFE, NULL, NULL }, //
	};

	CListBoxEx listbox;
	listbox.Show(0,"��������",items,KEY_ESC,60000);
	if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
	{
		return -1;
	}

	return -1;

}

int SetTermiPwd(void *arg)
{
	if(!InputPwd())
		return -1;

	CListBoxEx listbox;
	char menuitem[4][32];
	int i = 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[i++],0xFE,Dummy,(void*)1},
		{menuitem[i++],0xFE,Dummy,(void*)2},
		{menuitem[i++],0xFE,Dummy,(void*)3},
		{menuitem[i++],0xFE,Dummy,(void*)4},

	};
	i = 0;
	char szNewPwd[16]={0};
	ReadItemEx(BN10,PN0,0xa045,(BYTE*)szNewPwd);
	sprintf(menuitem[i++],"������: %s",szNewPwd);
	sprintf(menuitem[i++],"%s","��������");
	items[i].text = NULL;
	while(1)
	{
		i = 0;
		sprintf(menuitem[i++],"������: %s",szNewPwd);
		listbox.Show(0,"��������",items,KEY_ESC|(KEY_OK<<8),60000);

		if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			break;

		if(listbox.key == KEY_OK)
		{
			char szInput[16];
			if(listbox.item == 0)
			{
				strcpy(szInput,szNewPwd);
				if(EditTextBox(2,"��������", szInput, 60000, 6, DATA_DEC) >= 0)
				{
					strcpy(szNewPwd,szInput);

					BYTE bBuf[6]={0};
					i = 0;
					while(szNewPwd[i]!=' ' && i < 6)
					{
						bBuf[i] = szNewPwd[i];
						i++;
					}
					memcpy(szNewPwd,bBuf,6);
				}
			}
			else if(listbox.item == 1)
			{
				if(MessageBox("ȷ��Ҫ��������?",KEY_ESC,10000) > 0)
				{


					if (WriteItemEx(BN10,PN0,0xa045,(BYTE*)szNewPwd)>0)
					{
						//g_fPwdOk = false;
						FaSavePara();
						MessageBox("���óɹ�!",KEY_ESC,2000);
						TrigerSaveBank(BN10, 0, -1);	//��������һ��
						DoTrigerSaveBank();
					}
					else
					{
						MessageBox("����ʧ��!",KEY_ESC,2000);
					}
				}
			}
		}
	}
	return -1;
}

int ExeMaintenanceCmd(void *arg)
{
	BYTE buff[512] = {0};
	WORD wPn;
	char szInput[33] = {0};
	int iMpNo = 1;
	TOobMtrInfo tTMtrInfo;
	BYTE bApdu[6] = {0x01, 0x00, 0x10, 0x02, 0x00, 0x00};
	int iRet;

	if (EditSpecBox(2,"��������:",szInput,60000,2,DATA_DEC)<0)
		return -1;
	
	sscanf(szInput,"%d",&iMpNo);
	if (iMpNo >= 1 & iMpNo < POINT_NUM)
	{
		wPn = MtrSnToPn(iMpNo);
		memset(&tTMtrInfo, 0, sizeof(TOobMtrInfo));
		if (!GetMeterInfo(wPn, &tTMtrInfo))
		{
			MessageBox("�õ��δ����!", KEY_ESC, 3000);
			return -1;
		}
		MessageBox("������,���Ժ�...", KEY_ESC, 0);
		
		iRet = DirAskMtrData(5, 2, tTMtrInfo.bTsa, tTMtrInfo.bTsaLen, bApdu, 6, 60, buff);
		if (iRet <= 0)
		{
			MessageBox("����ʧ��!", KEY_ESC, 3000);
			return -1;
		}

		char menuBuff[8][24] = {0};
		struct ListBoxExItem actmp[] ={
			{ menuBuff[0], 0xFE, Dummy, (void*)1 },
			{ menuBuff[1], 0xFE, Dummy, (void*)2 },
			{ menuBuff[2], 0xFE, Dummy, (void*)3 },
			{ menuBuff[3], 0xFE, Dummy, (void*)4 },
			{ menuBuff[4], 0xFE, Dummy, (void*)5 },
			{ menuBuff[5], 0xFE, Dummy, (void*)6 },
			{ menuBuff[6], 0xFE, Dummy, (void*)7 },
			{ NULL, 0xFF, NULL, NULL },
		};
		sprintf(menuBuff[0], "�����:%d", iMpNo);
		BYTE *cp = buff + 9;
		sprintf(menuBuff[1], "�����й�����ʾֵ");
		sprintf(menuBuff[2], "��:    %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
		cp += 5;
		sprintf(menuBuff[3], "����1: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
		cp += 5;
		sprintf(menuBuff[4], "����2: %.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
		cp += 5;
		sprintf(menuBuff[5], "����3��%.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
		cp += 5;
		sprintf(menuBuff[6], "����4��%.2fkWh", (float)OoDoubleLongUnsignedToDWord(cp)*1e-2);
		
		static CListBoxEx ListBox;
		ListBox.Show(0, NULL, actmp, KEY_ESC | (KEY_RIGHT << 8) | (KEY_LEFT << 16), 60000);
		return -1;
	}
	else
	{
		MessageBox("�õ��δ����!", KEY_ESC, 3000);
		return -1;
	}

	return -1;
}

int ManualRdMtr(void *arg)
{
	struct ListBoxExItem tmp[] ={	{ "ָ�������㳭��",	   0xFF, ExeMaintenanceCmd, (void*)1   },
									//{ (s8*)"ָ�����ַ����",	   0xFF, ExeMaintenanceCmd, (void*)100 },
									//{ (s8*)"�����б�ǰ�����й�", 0xFF, ExeMaintenanceCmd, (void*)4   },
									//{ (s8*)"�����е������Ϣ",   0xFF, ExeMaintenanceCmd, (void*)5   },
									//{ (s8*)"�����е��ʱ��",	   0xFF, ExeMaintenanceCmd, (void*)2   },
									//{ (s8*)"�����е��״̬��",	   0xFF, ExeMaintenanceCmd, (void*)3   },
									//{ (s8*)"��ѯ�ն����ܵ���",	   0xFF, ExeMaintenanceCmd, (void*)6   },
									//{ (s8*)"��ѯ�¶����ܵ���",     0xFF, ExeMaintenanceCmd, (void*)7   },
									//{ (s8*)"��ѯʧ�ܲ�����",       0xFF, ExeMaintenanceCmd, (void*)8   },
									{ NULL, 0xFF, NULL, NULL },
							   };
	static CListBoxEx ListBox;

	while (1)
	{
		ListBox.Show(0, "�ֶ�����", tmp, KEY_ESC, 60000);
		if (ListBox.key == KEY_ESC || ListBox.key == KEY_NULL)
		{
			break;
		}
	}
	return -1;
}


int ProtocolSwitch(void *arg)
{
	if (!InputPwd())
	{
		return -1;
	}

	char menuitem[5][32];
	memset(menuitem, 0, sizeof(menuitem));

	struct ListBoxExItem tmpS[] = { 
		{ menuitem[0], 0xFE, Dummy, (void *) 1 }, //
		{ menuitem[1], 0xFE, Dummy, (void *) 2 }, //
		{ menuitem[2], 0xFE, Dummy, (void *) 3 }, //
		{ menuitem[3], 0xFE, Dummy, (void *) 4 }, //
		{ menuitem[4], 0xFE, Dummy, (void *) 5 }, //
		{ NULL, 0xFF, NULL, NULL }, //
	};

	BYTE i = 0;
	char szInput[20];
	CListBoxEx listbox;
	
	BYTE bGprsBuf[24] = {0};
	TFieldParser tGprsParser = {bGprsBuf};
	BYTE bType = 0;
	WORD wItemOffSet = 0;
	WORD wItemLen = 0;
	DWORD dwOAD;
	WORD wFmtLen;
	BYTE *pbFmt;
	typedef struct{
		BYTE bStructType;
		BYTE bFiledNum;
		BYTE bStringType;
		BYTE bLen;
		BYTE bIP[4];
		BYTE bLongUnsighedType;
		BYTE bPort[2];
	}TMasterPara; 
	TMasterPara tMaster1;
	TMasterPara tMaster2;
	memset(&tMaster1, 0, sizeof(TMasterPara));
	memset(&tMaster2, 0, sizeof(TMasterPara));

	dwOAD = 0x45000300;	//��վͨ�Ų���
	if ((tGprsParser.wCfgLen=OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bGprsBuf, &pbFmt, &wFmtLen)) <= 0)
	{
		MessageBox("��ȡ���ݿ����",KEY_ESC,10000);
		return -1;
	}
	tGprsParser.pbCfg = bGprsBuf;
	if (OoParseField(&tGprsParser, pbFmt, wFmtLen, false))
	{
		ReadParserField(&tGprsParser, 0x00, (BYTE *)&tMaster1, &bType, &wItemOffSet, &wItemLen);   //��վ1
		ReadParserField(&tGprsParser, 0x01, (BYTE *)&tMaster2, &bType, &wItemOffSet, &wItemLen);  //��վ2
	}

	while(1)
	{
		i = 0;
		sprintf(menuitem[i++],"����IP:%d.%d.%d.%d", tMaster1.bIP[0], tMaster1.bIP[1], tMaster1.bIP[2], tMaster1.bIP[3]);
		sprintf(menuitem[i++],"���ö˿�:%d", OoLongUnsignedToWord(tMaster1.bPort));
		sprintf(menuitem[i++],"����IP:%d.%d.%d.%d", tMaster2.bIP[0], tMaster2.bIP[1], tMaster2.bIP[2], tMaster2.bIP[3]);
		sprintf(menuitem[i++],"���ö˿�:%d", OoLongUnsignedToWord(tMaster2.bPort));
		sprintf(menuitem[i++],"��������");
		tmpS[i].text = NULL;

		listbox.Show(0,"1376.1��վͨ�Ų���",tmpS,KEY_ESC | KEY_OK << 8,60000);
		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item==0 || listbox.item==2)
			{
				memset(szInput, 0, sizeof(szInput));
				if (listbox.item==0)
					sprintf(szInput,"%03d.%03d.%03d.%03d", tMaster1.bIP[0], tMaster1.bIP[1], tMaster1.bIP[2], tMaster1.bIP[3]);	 //spintf()���Զ���szInput��ĩβ����\0������szInputҪ�࿪1�ֽڣ���ֹ���
				else
					sprintf(szInput,"%03d.%03d.%03d.%03d", tMaster2.bIP[0], tMaster2.bIP[1], tMaster2.bIP[2], tMaster2.bIP[3]);

				if(EditTextBox(2,"������վIP��ַ",szInput,60000,15,DATA_DEC)>=0)
				{
					if (szInput[3]=='.' && szInput[7]=='.' && szInput[11]=='.')
					{
						int iAddr1,iAddr2,iAddr3,iAddr4;
						if(sscanf(szInput,"%d.%d.%d.%d",&iAddr1,&iAddr2,&iAddr3,&iAddr4)==4)
						{
							if (iAddr1<=255 && iAddr2<=255 && iAddr3<=255 && iAddr4<=255)
							{
								if (listbox.item==0)//��վ1
								{
									tMaster1.bIP[0] = iAddr1;
									tMaster1.bIP[1] = iAddr2;
									tMaster1.bIP[2] = iAddr3;
									tMaster1.bIP[3] = iAddr4;
								}
								else//��վ2
								{
									tMaster2.bIP[0] = iAddr1;
									tMaster2.bIP[1] = iAddr2;
									tMaster2.bIP[2] = iAddr3;
									tMaster2.bIP[3] = iAddr4;
								}
							}
						}	
					}
					else
					{
						MessageBox("����ֵ�Ƿ�!",KEY_ESC,2000);
					}
				}	
			}
			else if (listbox.item==1 || listbox.item==3)
			{
				memset(szInput, 0, sizeof(szInput));
				if (listbox.item == 1)
					sprintf(szInput, "%d", OoLongUnsignedToWord(tMaster1.bPort));
				else
					sprintf(szInput, "%d", OoLongUnsignedToWord(tMaster2.bPort));

				if (EditTextBox(2, "������վ�˿�",szInput,60000,5,DATA_DEC)>=0)
				{
					int iport = 0;
					WORD wTmpPort = 0;
					sscanf(szInput, "%d", &iport);
					wTmpPort = iport;
					if (listbox.item == 1)
					{
						OoWordToLongUnsigned(wTmpPort, tMaster1.bPort);
					}
					else
					{
						OoWordToLongUnsigned(wTmpPort, tMaster2.bPort);
					}	
				}
			}
			else if (listbox.item == 4)
			{
				if (MessageBox("ȷ��Ҫ�л���1376.1Э��?",KEY_ESC,10000) > 0)
				{
					memcpy(bGprsBuf+tGprsParser.wPos[0], (BYTE *)&tMaster1, tGprsParser.wLen[0]);
					memcpy(bGprsBuf+tGprsParser.wPos[1], (BYTE *)&tMaster2, tGprsParser.wLen[1]);
					if (g_pmParaMgr.SwitchTo13761(bGprsBuf))
					{
						MessageBox("Э���л��ɹ�!",KEY_ESC,3000);
					}
					else
					{
						MessageBox("Э���л�ʧ��!",KEY_ESC,2000);
					}
				}
			}
		}
	}
		
	return -1;
}

int ProtoCtrl(void *arg)
{
	if(!InputPwd())
		return -1;
	CListBoxEx listbox;
	BYTE TmpBuff[5];
	char menuitem[3][32];
	int i = 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{"�ϵ籣��ʱ��",0xFE,Dummy,(void*)1},
		{"ȡ���ϵ籣��",0xFE,Dummy,(void*)2},
		{NULL,0xFE,NULL,(void*)4},
	};

	while(1)
	{
		listbox.Show(0,"��������",items,KEY_ESC | KEY_OK << 8,60000);

		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.item == 0)
		{
			ReadItemEx(BN10,PN0,0xa120,(BYTE*)TmpBuff);
			CListBoxEx listboxs;
			while(1)
			{
				i = 0;
				struct ListBoxExItem tmpS[] = {
					{menuitem[0],0xFE,Dummy,(void*)1},
					{menuitem[1],0xFE,Dummy,(void*)2},//0xa120
					{menuitem[2],0xFE,Dummy,(void*)2},//0xa120
					{NULL,0xFE,NULL,(void*)4},
				};

				sprintf(menuitem[i++],"�ϵ籣��ʱ��(��)");
				sprintf(menuitem[i++]," %d ��",TmpBuff[0]);
				sprintf(menuitem[i++],"��������");
				listboxs.Show(0,"�ϵ籣��ʱ��",tmpS,KEY_ESC | KEY_OK << 8,60000);
				if (listboxs.key == NULL || listboxs.key == KEY_ESC)
				{
					break;
				}
				else if (listboxs.key == KEY_OK)
				{
					if (listboxs.item == 0 || listboxs.item == 1)
					{
						char szInput[20] = {0};
						sprintf(szInput,"%d",TmpBuff[0]);
						if (EditTextBox(2,"�ϵ籣��ʱ��",szInput,60000,3,DATA_DEC) >= 0)
						{
							int pTime;
							sscanf(szInput,"%d",&pTime);
							TmpBuff[0] = (BYTE)(pTime &0xff); 
						}
					}
					else if (listboxs.item == 2)
					{
						if(MessageBox("ȷ��Ҫ���棿",KEY_ESC,10000) > 0)
						{
							if (WriteItemEx(BN10,PN0,0xa120,(BYTE*)TmpBuff)>0)
							{
								FaSavePara();
								MessageBox("���óɹ�!",KEY_ESC,2000);
							}
							else
							{
								MessageBox("����ʧ��!",KEY_ESC,2000);
							}
						}
					}
				}
			}
		}
		else if (listbox.item == 1)
		{
			if(MessageBox("ȷ��ȡ���ϵ籣�磿",KEY_ESC,10000) > 0)
			{
				TmpBuff[0] = 0;

				if (WriteItemEx(BN10,PN0,0xa120,(BYTE*)TmpBuff)>0)
				{
					FaSavePara();
					MessageBox("ȡ���ɹ�!",KEY_ESC,2000);
				}
				else
				{
					MessageBox("ȡ��ʧ��!",KEY_ESC,2000);
				}
			}
		}
	}

	return -1;
}

/*
int ReadMtrHandly(void *arg)
{
	char menuitem[6][32];
	char szInput[20] = {0};
	BYTE TmpBuf[100];
	int i = 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)1},
		{menuitem[3],0xFE,Dummy,(void*)1},
		{menuitem[4],0xFE,Dummy,(void*)2},
		{menuitem[5],0xFE,Dummy,(void*)2},
		{NULL,0xFE,NULL,(void*)4},
	};

	CListBoxEx listbox;

	int iMpNum = 0;	
	if (EditSpecBox(2, "������������",szInput,60000,2,DATA_DEC) >= 0)
	{
		sscanf(szInput,"%d",&iMpNum);
		if (iMpNum < GB_MAXMETER && iMpNum >= GB_MAXOFF)
		{
			char str[20];
			WORD wValidNum = 0;
			DWORD dwStartSeconds = 0;
			DWORD dwStartTime,dwEndTime = 0;
			TBankItem NewBankItem;
			int iConfirmNum = 0;
			DWORD dwEndSeconds = GetCurTime();
			int iRdTntvS = GetMeterInterv()*60;
			dwStartSeconds = dwEndSeconds - iRdTntvS;
			dwStartSeconds = (dwStartSeconds/iRdTntvS) * iRdTntvS - 1;
			if (IsPnValid(iMpNum))
			{
				MessageBox("������,���Ժ�...",KEY_ESC,1000);
				sprintf(menuitem[i++],"������:%d",iMpNum);
				ReadItemEx(BN0, iMpNum, 0x8902, TmpBuf);
				sprintf(menuitem[i++],"���ַ:%02X%02X%02X%02X%02X%02X",TmpBuf[10],TmpBuf[9],TmpBuf[8],TmpBuf[7],TmpBuf[6],TmpBuf[5]);
				
				sprintf(menuitem[i++],"�����й�����ʾֵ:");
				int iret = ReadItemEx(BN0, iMpNum, 0x901f, TmpBuf,dwStartSeconds,0);
				
				if(iret <= 0)
				{
					NewBankItem.wBn = BN0;
					NewBankItem.wID = 0X901F;
					NewBankItem.wPn = iMpNum;
					dwStartTime = dwStartSeconds;
					DWORD  dwStartClick = GetClick();
					WORD wItemCnt = SubmitDirReq(&NewBankItem, 1);
					
					do 
					{
						iConfirmNum = QueryItemTime(dwStartTime, dwEndTime, &NewBankItem, 1, &wValidNum);
						if (iConfirmNum >= 1)
							break;
						Sleep(200);
					} while (GetClick()-dwStartClick < 6);
					
					
					//iRead = ReadItemEx(bBank, wPn, wID, m_bBuf, dwStartSeconds, 0);
				}
				Fmt14ToStr(TmpBuf,str);
				sprintf(menuitem[i++],"��: %skWh",str);
				items[i].text = NULL;

				listbox.Show(0,"�����й�����ʾֵ",items,KEY_ESC | KEY_OK << 8,60000);

				if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
				{
					return -1;
				}
			}
			else
			{
				MessageBox("��������Ч!",KEY_ESC,2000);
				return -1;
			}
		}
		else
		{
			MessageBox("������������!",KEY_ESC,2000);
			return -1;
		}
	}

	return -1;
}*/


int DeletePn(void *arg)
{
	if(!InputPwd())
		return -1;

	char menuitem[6][32];
	char szInput[20] = {0};
	BYTE TmpBuf[20];
	//int i = 0;
	memset(menuitem,0,sizeof(menuitem));


	CListBoxEx listbox;

	int iMpNum = 0;	
	if (EditSpecBox(2, "������������",szInput,60000,2,DATA_DEC) >= 0)
	{
		sscanf(szInput,"%d",&iMpNum);
		if (iMpNum < POINT_NUM && iMpNum >= 1)
		{
			//char str[20];
			//WORD wValidNum = 0;
			//DWORD dwStartSeconds = 0;
			//DWORD dwStartTime,dwEndTime = 0;
			//TBankItem NewBankItem;
			//int iConfirmNum = 0;
			//DWORD dwEndSeconds = GetCurTime();
			//int iRdTntvS = GetMeterInterv()*60;
			//dwStartSeconds = dwEndSeconds - iRdTntvS;
			//dwStartSeconds = (dwStartSeconds/iRdTntvS) * iRdTntvS - 1;
			if (IsPnValid(iMpNum))
			{
				if(GetPnProp(iMpNum)==PN_PROP_METER || GetPnProp(iMpNum)==PN_PROP_AC)
				{
					WORD wSnTmp; 
					if (( wSnTmp = MtrPnToSn(iMpNum) ) > 0)
					{
						DeletSN(wSnTmp);
						MessageBox("ɾ���ɹ�!",KEY_ESC,2000);
					}
				}
				else if(GetPnProp(iMpNum) == PN_PROP_PULSE)
				{
					memset(TmpBuf, 0x00, sizeof(TmpBuf));
					WriteItemEx(BN0, iMpNum, 0x8903, TmpBuf);
					WriteItemEx(BN0, iMpNum, 0x8901, TmpBuf);
					MessageBox("ɾ���ɹ�!",KEY_ESC,2000);
				}

				if(listbox.key == KEY_ESC || listbox.key == KEY_NULL)
				{
					return -1;
				}
			}
			else
			{
				MessageBox("��������Ч!",KEY_ESC,2000);
				return -1;
			}
		}
		else
		{
			MessageBox("������������!",KEY_ESC,2000);
			return -1;
		}
	}

	return -1;
}




int SetLoopDisplay(void *arg)
{
	if(!InputPwd())
		return -1;

	char menuitem[15][32];
	BYTE TmpBuf[10];
	BYTE bTemp = 0;
	BYTE i = 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)3},
		{menuitem[3],0xFE,Dummy,(void*)4},
		{menuitem[4],0xFE,Dummy,(void*)5},
		{menuitem[5],0xFE,Dummy,(void*)6},
		{menuitem[6],0xFE,Dummy,(void*)7},
		{menuitem[7],0xFE,Dummy,(void*)8},
		{menuitem[8],0xFE,Dummy,(void*)9},
		{menuitem[9],0xFE,Dummy,(void*)10},
		{menuitem[10],0xFE,Dummy,(void*)11},
		{menuitem[11],0xFE,Dummy,(void*)12},
		{NULL,0xFE,NULL,(void*)14},
	};

	CListBoxEx listbox;

	ReadItemEx(BN1, PN0, 0x2049, TmpBuf);

	char *PortSeial[] = {"��","�ر�"};
	while(1)
	{
		i=0;
		sprintf(menuitem[i++],"1.���е���:%s",PortSeial[TmpBuf[0]&0x01]);
		sprintf(menuitem[i++],"2.���е���:%s",PortSeial[(TmpBuf[0]>>1)&0x01]);
		sprintf(menuitem[i++],"3.���޵���:%s",PortSeial[(TmpBuf[0]>>2)&0x01]);
		sprintf(menuitem[i++],"4.��    ѹ:%s",PortSeial[(TmpBuf[0]>>3)&0x01]);
		sprintf(menuitem[i++],"5.��    ��:%s",PortSeial[(TmpBuf[0]>>4)&0x01]);
		sprintf(menuitem[i++],"6.�й�����:%s",PortSeial[(TmpBuf[0]>>5)&0x01]);
		sprintf(menuitem[i++],"7.�޹�����:%s",PortSeial[(TmpBuf[0]>>6)&0x01]);
		sprintf(menuitem[i++],"8.��������:%s",PortSeial[(TmpBuf[0]>>7)&0x01]);

		sprintf(menuitem[i++],"9.��ʣ�����:%s", PortSeial[TmpBuf[1]&0x01]);
		sprintf(menuitem[i++],"10.�ն���Ϣ:%s",PortSeial[(TmpBuf[1]>>1)&0x01]);
		sprintf(menuitem[i++],"11.�ն�ʱ��:%s",PortSeial[(TmpBuf[1]>>2)&0x01]);
		sprintf(menuitem[i++],"      ��������");

		listbox.Show(0, "���Կ�������", items, KEY_ESC | (KEY_OK << 8), 30000);

		if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
		{
			break;
		}
		if (listbox.key == KEY_OK)
		{
			if (listbox.item >= 0 && listbox.item < i-1)
			{
				CListBoxEx listboxTmp1;
				struct ListBoxExItem tmp[] = { 
					{ "��", 0xFF, Dummy, (void *) 0x00 },//
					{ "�ر�", 0xFF, Dummy, (void *) 0x01 },//	
					{ NULL, 0xFF, NULL, NULL }, //
				};
				listboxTmp1.Show(0, "���Կ�������", tmp, KEY_ESC | (KEY_OK << 8), 60000);
				if (listboxTmp1.key == KEY_ESC || listboxTmp1.key == KEY_NULL)
				{
					break;
				}
				if (listboxTmp1.key == KEY_OK)
				{
					bTemp = (BYTE)((int)tmp[listboxTmp1.item].arg);
					if(bTemp == 1)
					{
						TmpBuf[listbox.item/8] = TmpBuf[listbox.item/8] | (1<<(listbox.item%8));
					}
					else
						TmpBuf[listbox.item/8] = TmpBuf[listbox.item/8] & (~(1<<(listbox.item%8)));
				}
			}
			else
			{
				if (MessageBox("ȷ��Ҫ����?",KEY_ESC,10000) > 0)
				{
					if(WriteItemEx(BN1, PN0, 0x2049,TmpBuf)> 0)//		
					{
						TrigerSaveBank(BN1, 0, 0);	//��������һ��
						DoTrigerSaveBank();
						MessageBox("���óɹ�",KEY_ESC,3000);
					}
					else
					{
						MessageBox("���óɹ�",KEY_ESC,2000);
					}
				}
			}
		}
	}

	return -1;
}




int ManageTerm(void *arg)
{
//ReadMtrHandly
	struct ListBoxExItem items[] = {
		{(char *)"�޸Ľ�������",0xFF,SetTermiPwd,(void*)1},
		{(char *)"���������ļ�",0xFF,TermiManageOpt,(void*)12},
		{(char *)"ά��������",0xFF,TermiManageOpt,(void*)6},
		{(char *)"�����ն�",0xFF,TermiManageOpt,(void*)0},
		{(char *)"���ݳ�ʼ��",0xFF,TermiManageOpt,(void*)1},
		//{(char *)"����(��ͨ��)��ʼ��",0xFF,TermiManageOpt,(void*)2},
	    {(char *)"�ָ���������",0xFF,TermiManageOpt,(void*)2},
		//{(char *)"������ʼ��",0xFF,TermiManageOpt,(void*)3},
		//{(char *)"USB����",0xFF,TermiManageOpt,(void*)5},
		{(char *)"U�����������ݿ���",0xFF, UsbUpdate2,(void*)1},
		{(char *)"ͨ��ģ������",0xFF,SetGprsModule,(void*)5},
		{(char *)"�ֶ�����",0xFF, ManualRdMtr,(void*)1},
		{(char *)"�л���1376.1Э��",0xFF, ProtocolSwitch,(void*)1},
		{ NULL, 0xFE, NULL, NULL }, //
	};

	CListBoxEx listbox;
	listbox.Show(0,"�ն˹���",items,KEY_ESC,60000);
	if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
	{
		return -1;
	}

	return -1;
}


int TermRealState(void *arg)
{
	char menuitem[6][32];
//	char szInput[20] = {0};
//	BYTE TmpBuf[100];
	int i = 0;
	BYTE StateWord = 0;
	memset(menuitem,0,sizeof(menuitem));
	struct ListBoxExItem items[] = {
		{menuitem[0],0xFE,Dummy,(void*)1},
		{menuitem[1],0xFE,Dummy,(void*)2},
		{menuitem[2],0xFE,Dummy,(void*)1},
		{menuitem[3],0xFE,Dummy,(void*)1},
		{menuitem[4],0xFE,Dummy,(void*)2},
		{menuitem[5],0xFE,Dummy,(void*)2},
		{NULL,0xFE,NULL,(void*)4},
	};

	CListBoxEx listbox;

	ReadItemEx(BN2, PN0, 0x1120, &StateWord);//��ʼ��������
	char *JudgeChar[] = {"��","��"};

	sprintf(menuitem[i++],"��ѹ������:%s",JudgeChar[StateWord &0x01]);
	sprintf(menuitem[i++],"����������:%s",JudgeChar[(StateWord >> 1) &0x01]);
	sprintf(menuitem[i++],"Ia������:%s",JudgeChar[(StateWord >> 2) &0x01]);
	sprintf(menuitem[i++],"Ib������:%s",JudgeChar[(StateWord >> 3) &0x01]);
	sprintf(menuitem[i++],"Ic������:%s",JudgeChar[(StateWord >> 4) &0x01]);
	items[i].text = NULL;


	listbox.Show(0,"�ն˵�ǰ״̬",items,KEY_ESC,60000);
	if (listbox.key == KEY_ESC || KEY_NULL)
	{
		return -1;
	}

	return -1;
}


int DebugInfo(void *arg)
{
	struct ListBoxExItem items[] = {
		//{(char *)"�ն˵�¼״̬",0xFF,TermiManageOpt,(void*)4},
		{(char *)"�ն˵�ǰ״̬",0xFF,TermRealState,(void*)2},
		{(char *)"485������",0xFF,TermiManageOpt,(void*)7},
		{(char *)"232�ڹ���", 0xFF, TermiManageOpt, (void*)9},
	//	{(char *)"ɾ��������",0xFF,DeletePn,(void*)7},
		{(char *)"���Կ���",0xFF,SetLoopDisplay,(void*)8},
	//	{(char *)"�ֶ�����",0xFF,ReadMtrHandly,(void*)4},
	//	{(char *)"ʣ��洢�ռ�",0xFF,Dummy,(void*)5},
		{ NULL, 0xFE, NULL, NULL }, //
	};

	CListBoxEx listbox;
	listbox.Show(0,"������Ϣ",items,KEY_ESC,60000);
	if (listbox.key == KEY_ESC || listbox.key == KEY_NULL)
	{
		return -1;
	}

	return -1;
}

static BYTE g_bCurAlrPt=0, g_bAlrCnt=0;
static bool g_fDispAlrInf=false;


bool IsAlertExist()
{
	BYTE bAlrBuf[25];
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	int iRet = ReadItemEx(BN0, PN0, 0x0900, bAlrBuf);

	if ((iRet <= 0)||(bAlrBuf[0] == 0))
	{
		g_bAlrCnt = 0;
		g_bCurAlrPt = 0;
		//g_bCurAlrPage = 0;
		g_fDispAlrInf = false;
		return false;
	}
	else
	{
		g_fDispAlrInf = true;
		return true;
	}
}

bool IsAlertChange()
{
	BYTE bAlrBuf[25];
	bool fAlrChange = false;
	static BYTE bPreAlrType = 0xee;
	static BYTE bPreYKTurn = 0xee;

	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	int iRet = ReadItemEx(BN0, PN0, 0x0900, bAlrBuf);

	if ( (iRet <= 0) || (bAlrBuf[0] == 0) )
	{
		g_bAlrCnt = 0;
		g_bCurAlrPt = 0;
		//g_bCurAlrPage = 0;
		g_fDispAlrInf = false;
	}
	else
	{
		if ( bAlrBuf[0] > g_bAlrCnt)
			fAlrChange = true;
		else if ( bPreAlrType != bAlrBuf[1] )
			fAlrChange = true;
		else if ( (bPreAlrType==bAlrBuf[1]) &&
			((bAlrBuf[1]==CTL_YkCtrl) || (bAlrBuf[1]==CTL_YkCtrl_CLOSE)) &&
			(bPreYKTurn!=bAlrBuf[2]) )
			fAlrChange = true;	

		g_bAlrCnt = bAlrBuf[0];
		bPreAlrType = bAlrBuf[1];
		if ( (bAlrBuf[1]==CTL_YkCtrl) || (bAlrBuf[1]==CTL_YkCtrl_CLOSE))
			bPreYKTurn = bAlrBuf[2];

		if ( fAlrChange )
		{
			g_bCurAlrPt = 0;
			//g_bCurAlrPage = 0;
			g_fDispAlrInf = true;
		}
	}

	return fAlrChange;
}

void CtrlAlertDisp(void)
{
	BYTE bBuf[16];
	BYTE bAlrBuf[25];
	char title[20];
	static unsigned long lDispAlrInfTm = 0;
	static BYTE bCount = 0;
	if (g_bAlrCnt == 0)
		return;
		
  if ( (GetTick()-lDispAlrInfTm) < 200 )	//200mS
		return;
	
	lDispAlrInfTm = GetTick();

	char menuitem[8][32];
	memset(menuitem,0,sizeof(menuitem));
	memset(title,0,sizeof(title));
	int i = 0;
	struct ListBoxExItem tmp[] = { 
		{ menuitem[i++], 0xfe, Dummy, (void *) 1 },
		{ menuitem[i++], 0xfe, Dummy, (void *) 2 },
		{ menuitem[i++], 0xfe, Dummy, (void *) 3 },
		{ menuitem[i++], 0xfe, Dummy, (void *) 4 },
		{ menuitem[i++], 0xfe, Dummy, (void *) 5 },
		{ menuitem[i++], 0xfe, Dummy, (void *) 6 },
		{ menuitem[i++], 0xfe, Dummy, (void *) 7 },
		{ menuitem[i++], 0xfe, Dummy, (void *) 8 },
		{ NULL, 0xFF, NULL, NULL },
	};

	int iRet = ReadItemEx(BN0, PN0, 0x0900, bAlrBuf);
	if ( (iRet <= 0) || (bAlrBuf[0] == 0) )
		return;

	char cTemp1[30], cTemp2[30];
	BYTE bTmpBuf[2];
	memset(cTemp1,  0, sizeof(cTemp1));
	memset(cTemp2,  0, sizeof(cTemp2));
	memcpy(bTmpBuf, bAlrBuf+1+g_bCurAlrPt*2, 2);

	//if ( (bTmpBuf[0]>=CTL_YkCtrl) && (bTmpBuf[0]<=CTL_PWR_PERIOD_ALLCLOSE) )
	{
	//	DTRACE(DB_FAPROTO, ("MainDisplay BlightOn --------############-----------> !\n"));
		//BlightOn(true);

	}

	int   iLeaveSecond;
	WORD  wSecondTimeLen=0;
	DWORD dwStartSecond=0;
	DWORD dwCurSecond = GetCurTime();
	bool fBuyCtl = false;
	i=0;

	switch (bTmpBuf[0])
	{
	case CTL_YkCtrl:
		iRet = ReadItemEx(BN0, bTmpBuf[1], 0x0910, bAlrBuf);
		sprintf(title,"ң�ظ澯");
		if (iRet <= 0)
		{
			MessageBox("��ң�ظ澯ʧ��!",KEY_ESC,2000);
			return ;
		}
		else
		{
			sprintf(menuitem[i++],"�ִ�%d����ʱ",bTmpBuf[1]);
			memcpy((BYTE *)&dwStartSecond, &bAlrBuf[0], 4);	
			memcpy((BYTE *)&wSecondTimeLen, &bAlrBuf[4], 2);
			iLeaveSecond = dwStartSecond + wSecondTimeLen - dwCurSecond;
			if ((iLeaveSecond<=0) || (iLeaveSecond>wSecondTimeLen))
				strcpy(menuitem[i++], "  00:00");
			else
				sprintf(menuitem[i++], "  %d:%02d", iLeaveSecond/60, iLeaveSecond%60);
		}
		break;
	case CTL_PWR_TMP:
	case CTL_PWR_SHUTOUT:
	case CTL_PWR_REST:
	case CTL_PWR_PERIOD:
	case CTL_PWR_TMP_ALLCLOSE:
	case CTL_PWR_SHUTOUT_ALLCLOSE:
	case CTL_PWR_REST_ALLCLOSE:
	case CTL_PWR_PERIOD_ALLCLOSE:
		iRet= ReadItemEx(BN0, PN0, 0x0920, bAlrBuf);
		if (iRet <= 0)
		{
			MessageBox("�����ظ澯ʧ��!",KEY_ESC,2000);
			return ;
		}
		else
		{
			if (bTmpBuf[0] == CTL_PWR_TMP)
			{//title
				sprintf(title,"��ʱ�¸��ظ澯");

			}
			else if (bTmpBuf[0] == CTL_PWR_SHUTOUT)
			{
				sprintf(title,"Ӫҵ��ͣ�ظ澯");
			}
			else if (bTmpBuf[0] == CTL_PWR_REST)
			{
				sprintf(title,"���ݿظ澯");
			}
			else if (bTmpBuf[0] == CTL_PWR_PERIOD)
			{
				sprintf(title,"ʱ�οؿظ澯");
			}
			else if (bTmpBuf[0] == CTL_PWR_TMP_ALLCLOSE)
			{
				sprintf(title,"��ʱ�¸��غ�բ�澯");
			}
			else if (bTmpBuf[0] == CTL_PWR_SHUTOUT_ALLCLOSE)
			{
				sprintf(title,"Ӫҵ��ͣ�غ�բ�澯");
			}
			else if (bTmpBuf[0] == CTL_PWR_REST_ALLCLOSE)
			{
				sprintf(title,"���ݿغ�բ�澯");
			}
			else if (bTmpBuf[0] == CTL_PWR_PERIOD_ALLCLOSE)
			{
				sprintf(title,"ʱ�οغ�բ�澯");
			}
		}
		
		memset(cTemp1,0,sizeof(cTemp1));
		Fmt2ToStr(&bAlrBuf[6],cTemp1);
		sprintf(menuitem[i++],"���ʶ�ֵ:%skW",cTemp1);
		memset(cTemp1,0,sizeof(cTemp1));
		Fmt2ToStr(&bAlrBuf[4],cTemp1);
		sprintf(menuitem[i++],"��ǰ����:%skW",cTemp1);
		memset(cTemp2,0,sizeof(cTemp2));

	//	sprintf(cTemp2," %d",bAlrBuf[1]);
		sprintf(menuitem[i++],"���ظ澯�ִ� %d",bAlrBuf[1]);
		break;

	case CTL_ENG_MONTH:
	case CTL_ENG_BUY:
		WORD wID;

		if (bTmpBuf[0] == CTL_ENG_MONTH)
		{
			wID = 0x0930;
			fBuyCtl = false;
		}
		else
		{
			wID = 0x0940;
			fBuyCtl = true;
		}
		iRet= ReadItemEx(BN0, PN0, wID, bAlrBuf);
		if (iRet <= 0)
		{
			if (bTmpBuf[0] == CTL_ENG_MONTH)
			{
				MessageBox("���µ�ظ澯ʧ��!",KEY_ESC,2000);
			}
			else
				MessageBox("������ظ澯ʧ��!",KEY_ESC,2000);
			
			return ;
		}
		else
		{
			if(bTmpBuf[0] == CTL_ENG_MONTH)
			{
				sprintf(title,"�µ�ظ澯");
			}
			else
			{
				sprintf(title,"����ظ澯");
			}
		    i=0;
			if (bTmpBuf[0] == CTL_ENG_MONTH)
			{
				sprintf(menuitem[i++],"�������õ�����");
				memset(cTemp1,0,sizeof(cTemp1));
				Fmt03ToStr(&bAlrBuf[1],cTemp1);
				sprintf(menuitem[i++]," %s",cTemp1);
				sprintf(menuitem[i++],"��������:");
				memset(cTemp1,0,sizeof(cTemp1));
				Fmt03ToStr(&bAlrBuf[5],cTemp1);
				sprintf(menuitem[i++]," %s",cTemp1);
				sprintf(menuitem[i++],"��բ��ֵ:");
				memset(cTemp1,0,sizeof(cTemp1));
				Fmt03ToStr(&bAlrBuf[9],cTemp1);
				sprintf(menuitem[i++]," %s",cTemp1);
			//	TraceFrm("the tiaozha buf",bAlrBuf,13);
			}
			else
			{
				sprintf(menuitem[i++],"ʣ�๺����/�ѣ�");
				memset(cTemp1,0,sizeof(cTemp1));
				Fmt03ToStr(&bAlrBuf[1],cTemp1);
				sprintf(menuitem[i++]," %s",cTemp1);
				sprintf(menuitem[i++],"�������޵���/��:");
				memset(cTemp1,0,sizeof(cTemp1));
				Fmt03ToStr(&bAlrBuf[5],cTemp1);
				sprintf(menuitem[i++]," %s",cTemp1);
				sprintf(menuitem[i++],"��բ���޵���/��:");
				memset(cTemp1,0,sizeof(cTemp1));
				Fmt03ToStr(&bAlrBuf[9],cTemp1);
				sprintf(menuitem[i++]," %s",cTemp1);
			}
		}
		break;
	case CTL_YkCtrl_CLOSE:
	case CTL_ENG_MONTH_CLOSE:
	case CTL_ENG_BUY_CLOSE:
	case CTL_PWR_TMP_CLOSE:
	case CTL_PWR_SHUTOUT_CLOSE:
	case CTL_PWR_REST_CLOSE:
	case CTL_PWR_PERIOD_CLOSE:
		if (bTmpBuf[0] == CTL_YkCtrl_CLOSE)
		{
			sprintf(title,"ң�غ�բ�澯");

		}
		else if (bTmpBuf[0] == CTL_ENG_MONTH_CLOSE)
		{
			sprintf(title,"�µ�غ�բ�澯");
		}
		else if (bTmpBuf[0] == CTL_ENG_BUY_CLOSE)
		{
			sprintf(title,"����غ�բ�澯");
		}
		else if (bTmpBuf[0] == CTL_PWR_TMP_CLOSE)
		{
			sprintf(title,"��ʱ�¸��غ�բ");
		}
		else if (bTmpBuf[0] == CTL_PWR_SHUTOUT_CLOSE)
		{
			sprintf(title,"Ӫҵ��ͣ�غ�բ");
		}
		else if (bTmpBuf[0] == CTL_PWR_REST_CLOSE)
		{
			sprintf(title,"���ݿغ�բ�澯");
		}
		else if (bTmpBuf[0] == CTL_PWR_PERIOD_CLOSE)
		{
			sprintf(title,"ʱ�οغ�բ�澯");
		}

		sprintf(menuitem[i++],"��բ�ִ�:");
		if (bTmpBuf[0] == CTL_YkCtrl_CLOSE)
		{
			memset(cTemp1,  0, sizeof(cTemp1));
			sprintf(cTemp1, "%d", bTmpBuf[1]);
		}
		else
		{
			BYTE j;
			memset(cTemp1,  0, sizeof(cTemp1));
			memset(cTemp2,  0, sizeof(cTemp2));
			for (j=0; j<8; j++)
			{
				if ( (bTmpBuf[1]>>j) & 0x01 )
				{
					sprintf(cTemp2, "%d ", (j+1));
					strcat(cTemp1, cTemp2);
				}
			}
		}
		sprintf(menuitem[i++],"%s",cTemp1);
		break;
	}
	
	bCount++;
	if(bCount >= 8)
	{
		bCount = 0;
		g_bCurAlrPt = (g_bCurAlrPt+1) % g_bAlrCnt;
		
	}
//	DTRACE(DB_FAPROTO, ("g_bAlrCnt is %d --------############-----------> !\n",g_bAlrCnt));
//	DTRACE(DB_FAPROTO, ("bCount is %d --------############-----------> !\n",bCount));
//	DTRACE(DB_FAPROTO, ("g_bCurAlrPt is %d --------############-----------> !\n",g_bCurAlrPt));

	tmp[i].text = NULL;
	CListBoxEx listbox;
	while(1)
	{
		listbox.Show(0, title, tmp, KEY_ESC | KEY_OK << 8, 1000,false);

		if(listbox.key == KEY_OK || listbox.key == KEY_ESC || listbox.key == KEY_NULL)
			break;
	}
}

int QueryCtrlPara(void *arg)
{
	struct ListBoxExItem items[] = { 
		{ (char*) "ʱ�οز���", MENU_ONELEVEL_HAVE_NO, PeriodCtrlPara, (void *) 1 },
		{ (char*) "���ݿز���", MENU_ONELEVEL_HAVE_NO, RecessCtrlPara, (void *) 1 },
		{ (char*) "��ͣ�ز���", MENU_ONELEVEL_HAVE_NO, StopCtrlPara, (void *) 1 },
		{ (char*) "�¸��ز���", MENU_ONELEVEL_HAVE_NO, PowerDecreCtrlPara, (void *) 1 },
		{ (char*) "�µ�ز���", MENU_ONELEVEL_HAVE_NO, MonEngCtrlPara, NULL }, 
		{ NULL, MENU_ONELEVEL_HAVE_NO, NULL, NULL }, 
	};

	CListBoxEx listbox;
	while(1)
	{
		listbox.Show(0, (char*) "���Ʋ�����ѯ", items, KEY_ESC, 60000);
		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

int QueryTermData(void *arg)
{
	CListBoxEx listbox;

	struct ListBoxExItem items[] = {
		{ (char *) "ң��״̬", MENU_ONELEVEL_HAVE_NO, RemoteSignal, (void *)1},
#ifdef FK_TERM
		{ (char *) "����״̬", MENU_ONELEVEL_HAVE_NO, CtrlState, (void *)2},
#endif
		{ (char *) "�¼���Ϣ", MENU_ONELEVEL_HAVE_NO, DisplayMssage, (void *)3},
		{ NULL, 0xFF, NULL, NULL },
	};

	while(1)
	{
		listbox.Show(0 , "������������ʾ", items, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

int DispPnData(void *arg)
{
	CListBoxEx listbox;

	struct ListBoxExItem items[] = {
		{ (char *) "ʵʱ����", MENU_ONELEVEL_HAVE_NO, QueryRealUIP, (void *)1},
		{ (char *) "������", MENU_ONELEVEL_HAVE_NO, QueryDataOfDay, (void *)2},
		{ (char *) "������", MENU_ONELEVEL_HAVE_NO, QueryDataOfMonth, (void *)3},
		{ NULL, 0xFF, NULL, NULL },
	};

	while(1)
	{
		listbox.Show(0 , "������������ʾ", items, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

int SetAndQueryPara(void *arg)
{
	CListBoxEx listbox;

	struct ListBoxExItem items[] = {
		{ (char *) "ͨѶ��������", MENU_ONELEVEL_HAVE_NO, ChoseCommunicationMode, (void *)1 },
		{ (char *) "����������", MENU_ONELEVEL_HAVE_NO, SetMPBasicPara, (void *)2 },
		{ (char *) "�����������", MENU_ONELEVEL_HAVE_NO, SetPulsePara, (void *)3 },
		//{ (char *) "���Ʋ�����ѯ", MENU_ONELEVEL_HAVE_NO, QueryCtrlPara, (void *)4 },   //�Ƚϸ��ӣ�Һ��������ͨ����̬���úͲ鿴
		{ (char *) "�������õ�Ԫ��ѯ", MENU_ONELEVEL_HAVE_NO, QueryTaskCfgUnit, (void *)4 },
		{ (char *) "��ͨ�ɼ�������ѯ", MENU_ONELEVEL_HAVE_NO, QueryCommonSchCfg, (void *)5 },
		{ (char *) "�¼��ɼ�������ѯ", MENU_ONELEVEL_HAVE_NO, QueryEvtSchCfg, (void *)6 },
		{ (char *) "״̬����������", MENU_ONELEVEL_HAVE_NO, SetStatusPara, (void *)7 }, 
		{ (char *) "�ն�ʱ������", MENU_ONELEVEL_HAVE_NO, SetMySysTime, (void *)8 },
		{ (char *) "�ն˵�ַ����", MENU_ONELEVEL_HAVE_NO, SetTermAddr, (void *)9 },
		{ NULL, 0xFF, NULL, NULL },
	};

	while(1)
	{
		listbox.Show(0 , "�����������ѯ", items, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

int ManageAndMaintainTerm(void *arg)
{
	CListBoxEx listbox;

	struct ListBoxExItem items[] = {
		{ (char *) "�ն�����", MENU_ONELEVEL_HAVE_NO, QueryTermData, (void *)1 },
		{ (char *) "�ն���Ϣ", MENU_ONELEVEL_HAVE_NO, TermiManageHelp, (void *)2 },
		{ (char *) "�ն˹���", MENU_ONELEVEL_HAVE_NO, ManageTerm, (void *)3 },
		{ NULL, 0xFF, NULL, NULL },
	};

	while(1)
	{
		listbox.Show(0 , "�ն˹�����ά��", items, KEY_ESC << 8, 60000);

		if ((KEY_NULL == listbox.key)||(KEY_ESC == listbox.key))
			break;
	}

	return -1;
}

/*
struct ListBoxExItem tmMainMenu[] = { { (char *) "ʵʱ����", MENU_ONELEVEL_HAVE_NO, QueryRealUIP, NULL },//
{ (char *) "������ֵ", MENU_ONELEVEL_HAVE_NO, ParamReadWrite, NULL },//
//{ (char *) "����״̬", MENU_ONELEVEL_HAVE_NO, CtrlState, NULL },//
//{ (char *) "���ܱ�ʾ��", MENU_ONELEVEL_HAVE_NO, EnergyStat, NULL },//
//{ (char *) "������Ϣ", MENU_ONELEVEL_HAVE_NO, DisplayMssage, NULL },//
//{ (char *) "������Ϣ", MENU_ONELEVEL_HAVE_NO, BuyEngInfo, NULL },//
{ (char *) "�ն���Ϣ", MENU_ONELEVEL_HAVE_NO, TermiManageHelp, (void*)2 },//
{ (char *) "��������", MENU_ONELEVEL_HAVE_NO, SetPara, NULL },//
{ (char *) "�ն˹���", MENU_ONELEVEL_HAVE_NO, ManageTerm, NULL },//
{ (char *) "������Ϣ", MENU_ONELEVEL_HAVE_NO, DebugInfo, NULL },//
{ NULL, 0xFF, NULL, NULL }, //
};*/

struct ListBoxExItem tmNewMainMenu[] = {
	{ (char *) "������������ʾ", MENU_ONELEVEL_HAVE_NO, DispPnData, NULL },
	{ (char *) "����������鿴", MENU_ONELEVEL_HAVE_NO, SetAndQueryPara, NULL },
	{ (char *) "�ն˹�����ά��", MENU_ONELEVEL_HAVE_NO, ManageAndMaintainTerm, NULL },
	{ NULL, 0xFF, NULL, NULL }, 
};

int LoopDisplay(void *arg);
TThreadRet DisplayThread(void* pvArg)
{
	static BYTE fLedOff = 0;
	TTime tmNow;
	int iYpos;
	char szMsg[32];
	struct KeyState keyState;
	BYTE bBuf[2] = {0};
	//BYTE szName[128];

	DTRACE(DB_CRITICAL, ("display: Version V2.00!\n"));

	g_SemGUI = NewSemaphore(1);
	g_MsgQueue.Init(10);
	
#ifndef SYS_WIN
	g_Key.Init();
#endif
	GUI_Init();

	BlightOn(true);	//�ϵ翪����
	GetCurTime(&tmNow);
	ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());

	iYpos = GetMaxLinePerScreen() * GetFontHeight() / 4;
	//DrawStringHCenterAtLock("ר��III���ն�", LCD_GET_XSIZE() / 2, iYpos, GUI_WHITE, GUI_BLACK);
	DrawStringHCenterAtLock("��ѹ����������", LCD_GET_XSIZE() / 2, iYpos, GUI_WHITE, GUI_BLACK);
	//DrawStringHCenterAtLock("��������ն�", LCD_GET_XSIZE() / 2, iYpos+30, GUI_WHITE, GUI_BLACK);
//	GetSysInfo(bBuf);
//	DrawStringHCenterAtLock(Svrinfo[bBuf[1]], LCD_GET_XSIZE() / 2, iYpos+30, GUI_WHITE, GUI_BLACK);
	
	sprintf(szMsg, "%04d-%02d-%02d", tmNow.nYear,tmNow.nMonth,tmNow.nDay);
//	DrawStringHCenterAtLock(szMsg, LCD_GET_XSIZE() / 2, iYpos + 60, GUI_WHITE, GUI_BLACK);
	DrawStringHCenterAtLock(szMsg, LCD_GET_XSIZE() / 2, iYpos + 30, GUI_WHITE, GUI_BLACK);

	sprintf(szMsg, "%02d:%02d:%02d",tmNow.nHour, tmNow.nMinute, tmNow.nSecond);
//	DrawStringHCenterAtLock(szMsg, LCD_GET_XSIZE() / 2, iYpos + 80, GUI_WHITE, GUI_BLACK);
	DrawStringHCenterAtLock(szMsg, LCD_GET_XSIZE() / 2, iYpos + 50, GUI_WHITE, GUI_BLACK);

	DrawStateTask();

	LcdRefresh();
#ifdef SYS_WIN
	while(GetClick() < 45)	//45s֮�ڱ���Һ����ʾ����
	{
		//g_Key.MonitorKey();
		keyState = GetKey();
		if(keyState.key != 0)
			break;
		
		ClearWDG();
		Sleep(1000);
	}
#endif

//	g_iTermiType = GetTermType();
	int iMonitorID = ReqThreadMonitorID("Display-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ60��
	SetDispMonitorID(iMonitorID);
	//struct ListBoxExItem *pTmp =  tmMainMenu;
	struct ListBoxExItem *pTmp =  tmNewMainMenu;	
	GetCurTime(&tmNow);
	g_fLcdRefresh = true;
	CListBoxEx listBox;
	while (1)
	{
		fLedOff = 1;
		keyState = GetKey();
		
		while (1)
		{
			GetCurTime(&tmNow);

			if (GetDisplayMsgNum() > 0)
			{		
				ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());		
				char *p = (char *)GetDisplayMsg();
				if (p != NULL)
				{
					BlightOn(true);
					MessageBox((char *) p, KEY_ESC, 2*1000);
					ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
				}				
			}
			else
			{
				if (keyState.idle < 60 || fNeedDisplay == false)
				{
					ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
					//char szMsg[32];

					iYpos = GetMaxLinePerScreen() * GetFontHeight() / 4;
					DrawStringHCenterAtLock("��ѹ����������", LCD_GET_XSIZE() / 2, iYpos, GUI_WHITE, GUI_BLACK);
					//DrawStringHCenterAtLock("��������ն�", LCD_GET_XSIZE() / 2, iYpos+30, GUI_WHITE, GUI_BLACK);
					memset(bBuf, 0, sizeof(bBuf));
//					GetSysInfo(bBuf);
//					DrawStringHCenterAtLock(Svrinfo[bBuf[1]], LCD_GET_XSIZE() / 2, iYpos+30, GUI_WHITE, GUI_BLACK);

					//GUI_DispStringHCenterAtLock((char *)szName, LCD_GET_XSIZE() / 2, iYpos+40, GUI_WHITE, GUI_BLACK);

					sprintf(szMsg, "%04d-%02d-%02d", tmNow.nYear,tmNow.nMonth,tmNow.nDay);
//					DrawStringHCenterAtLock(szMsg, LCD_GET_XSIZE() / 2, iYpos + 60, GUI_WHITE, GUI_BLACK);
					DrawStringHCenterAtLock(szMsg, LCD_GET_XSIZE() / 2, iYpos + 30, GUI_WHITE, GUI_BLACK);

					sprintf(szMsg, "%02d:%02d:%02d",tmNow.nHour, tmNow.nMinute, tmNow.nSecond);
//					DrawStringHCenterAtLock(szMsg, LCD_GET_XSIZE() / 2, iYpos + 80, GUI_WHITE, GUI_BLACK);
					DrawStringHCenterAtLock(szMsg, LCD_GET_XSIZE() / 2, iYpos + 50, GUI_WHITE, GUI_BLACK);

					if (GetClick() - g_PastClick >= 8)
					{
						g_PastClick = GetClick();
						fNeedDisplay = true;
					}
					g_fDispAlrInf = false; 
				}
				else
				{
					/*if (!IsAlertChange())
					{

						IsAlertExist();
						
					}
					if (g_fDispAlrInf)
					{
						//RemoveDisplayMsg();
						DTRACE(DB_FAPROTO, ("MainDisplay I am here -------------------> !\n"));
						CtrlAlertDisp();
					}
					else	*/		
					{
						//����
						BlightOn(false);
						LoopDisplay(NULL);
					 } 
				}
			}
			
			DWORD dwTick = GetTick();
			if (g_dwStateTaskClick==0 || dwTick>(g_dwStateTaskClick+500))
			{
				DrawStateTask();
				g_fLcdRefresh = true;
				g_dwStateTaskClick = dwTick;
			}
			
			if (g_fLcdRefresh)
			{
				g_fLcdRefresh = false;
				LcdRefresh();
			}
			
			Sleep(50);
			keyState = GetKey();
			if (keyState.key!=KEY_NULL && keyState.key!=KEY_ESC)
			{
				RemoveDisplayMsg();
				BlightOn(true);
				break;
			}
			JudgeLcdBlight();
			UpdThreadRunClick(iMonitorID);
		}
		ClearScreenRec(0, GetFontHeight(), LCD_SIM_WIDTH, LCD_SIM_HEIGHT - GetFontHeight());
		while (1)
		{
			//listBox.Show(35, "���˵�", pTmp, KEY_ESC, 60000);
			listBox.Show(25, "���˵�", pTmp, KEY_ESC, 60000);
			
			if (listBox.key==KEY_NULL || listBox.key==KEY_ESC)
				break;
			UpdThreadRunClick(iMonitorID);
		}
		UpdThreadRunClick(iMonitorID);
	}
	ReleaseThreadMonitorID(iMonitorID);
}


//---------------------------------------����-------------------------------------------------------
int LoopDisplay(void *arg)
{
	static DWORD dwLoopClick = 0;
	static char j =  0;
	BYTE bLoopPageNum = 11;
	DWORD dwStartSecond = GetClick();

	if (dwStartSecond<dwLoopClick+8 && dwLoopClick!=0)
		return 0;
	
	BYTE buff[256];
	BYTE bTmpBuf[10] = {0};

	BYTE bAddrLen;
	BYTE bBuf[20] = {0};
	char szServAddr[32] = {0};
	BYTE bAddrBuf[35] = {0};
	int i;

	//BYTE bNum = 0;
	//char dir = 1;
	WORD  wAcPn = GetAcPn();
	char title[20] = {0};
	char menuitem[8][24];
	dwLoopClick = dwStartSecond;
	dwStartSecond = GetCurTime()/(24*60*60)*24*60*60;
	struct ListBoxExItem tmReadPowerMeter[] = { { menuitem[0], 0xFE, Dummy, (void *) 1 },//
	{ menuitem[1], 0xFE, Dummy, (void *) 2 },//
	{ menuitem[2], 0xFE, Dummy, (void *) 3 },//			
	{ menuitem[3], 0xFE, Dummy, (void *) 4 },//		
	{ menuitem[4], 0xFE, Dummy, (void *) 5 },//			
	{ menuitem[5], 0xFE, Dummy, (void *) 6 },//			
	{ menuitem[6], 0xFE, Dummy, (void *) 7 },//
	{ menuitem[7], 0xFE, Dummy, (void *) 8 },//
	{ NULL, 0xFF, NULL, NULL }, //
	};
	memset(menuitem, 0, sizeof(menuitem));

	//DWORD dwTime = 0;	
	TBankItem BI = {BN0,GetAcPn(),0x901f};
	int rxlen;
	char str[128];

	if(IsMountUsb())//
	{
		return 1;
	}

	ReadItemEx(BN1, PN0, 0x2049, bTmpBuf);

	while(j<bLoopPageNum)
	{
		if (j>=0 && j<9) //0~8�����Խ����ǹ��������������ܵ����ģ���δ��,�Ȳ���ʾ
		{
			j++;
		}
		else if (( bTmpBuf[j/8] & (0x01<<(j%8)) ) == 0x00)
		{
			break;
		}
		else
		{
			j++;
		}

		/*if(( bTmpBuf[j/8] & (0x01<<(j%8)) ) == 0x00)
			break;
		else
		{
			//bNum++;
			j++;
		}*/

		//if(j>=10)
		//{
		//	j = 0;
		//	if(bNum >= 10)
		//	{
		//		fNeedDisplay = false;
		//		g_PastClick = GetClick();
		//		return -1;
		//	}
		//	bNum = 0;
		//}
	}
	
	
	switch (j)
	{
	case 0:
		{
			BI.wID = 0x901f;
		 	rxlen = ReadItemEx(&BI, 1, buff,dwStartSecond);

			if (rxlen > 0)
			{
				BYTE *cp = buff;
				memset(title,0,sizeof(title));
				sprintf(title,"�����й�����ʾֵ");
				sprintf(menuitem[0], "������: %d",wAcPn);
				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[1], "��: %skWh", str);

				cp += 5;	

				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[2], "��: %skWh", str);

				cp += 5;

				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[3], "��: %skWh", str);

				cp += 5;

				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[4], "ƽ��%skWh", str);

				cp += 5;

				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[5], "�ȣ�%skWh", str);
			}
		}
		break;
	case 1:
		//�����й�����ʾֵ
		{
			BI.wID = 0x902f;
			rxlen = ReadItemEx(&BI, 1, buff,dwStartSecond);

			if (rxlen > 0)
			{

				BYTE *cp = buff;		
				memset(title,0,sizeof(title));
				sprintf(title,"�����й�����ʾֵ");
				sprintf(menuitem[0], "������: %d",wAcPn);
				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[1], "��: %skWh", str);

				cp += 5;	

				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[2], "��: %skWh", str);

				cp += 5;

				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[3], "��: %skWh", str);

				cp += 5;

				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[4], "ƽ��%skWh", str);

				cp += 5;

				memset(str, 0, sizeof(str));
				Fmt14ToStr(cp, str);//
				sprintf(menuitem[5], "�ȣ�%skWh", str);
			}
		}
		break;
	case 2:
		//�����޹�����ʾֵ
		{
			TBankItem NewItemBank[] = {
				{BN0,wAcPn,0x9110},
				{BN0,wAcPn,0x9120},
			};
			rxlen = ReadItemEx(NewItemBank, 2, buff,dwStartSecond);

			if (rxlen > 0)
			{
				BYTE *cp = buff;
				memset(title,0,sizeof(title));
				sprintf(title,"�޹�����ʾֵ");
				sprintf(menuitem[0], "������: %d",wAcPn);
				memset(str, 0, sizeof(str));
				Fmt11ToStr(cp, str);//
				sprintf(menuitem[1], "�����޹���:%skvarh", str);

				cp += 4;	
				Fmt11ToStr(cp, str);
				sprintf(menuitem[2], "�����޹���:%skvarh", str);
			}
		}
		break;
	case 3:
		//��ѹ
		{
			BI.wID = 0xB61F;
			rxlen = ReadItemEx(&BI, 1, buff,dwStartSecond);

			if (rxlen > 0)
			{
				BYTE *cp = buff;

				memset(title,0,sizeof(title));
				sprintf(title,"��ǰ��ѹ");
				sprintf(menuitem[0], "������: %d",wAcPn);
				memset(str, 0, sizeof(str));
				Fmt7ToStr(cp, str);
				sprintf(menuitem[1], "A���ѹ %sV", str);
				sprintf(menuitem[2], "%s", "");
				memset(str, 0, sizeof(str));
				Fmt7ToStr(cp+2, str);
				sprintf(menuitem[3], "B���ѹ %sV", str);
				sprintf(menuitem[4], "%s", "");
				memset(str, 0, sizeof(str));
				Fmt7ToStr(cp+4, str);
				sprintf(menuitem[5], "C���ѹ %sV", str);
			}
		}
		break;
	case 4:
		//����
		{
			BI.wID = 0xB62F;
			rxlen = ReadItemEx(&BI, 1, buff,dwStartSecond);

			if (rxlen > 0)
			{
				BYTE *cp = buff;
				memset(title,0,sizeof(title));
				sprintf(title,"��ǰ����");
				sprintf(menuitem[0], "������: %d",wAcPn);
				memset(str, 0, sizeof(str));
				Fmt25ToStr(cp, str);
				sprintf(menuitem[1], "A����� %sA", str);
				sprintf(menuitem[2], "%s", "");
				memset(str, 0, sizeof(str));
				Fmt25ToStr(cp+3, str);
				sprintf(menuitem[3], "B����� %sA", str);
				sprintf(menuitem[4], "%s", "");
				memset(str, 0, sizeof(str));
				Fmt25ToStr(cp+6, str);
				sprintf(menuitem[5], "C����� %sA", str);
			}
		}
		break;
	case 5:
		//�й�����
		{
			BI.wID = 0xB63F;
			rxlen = ReadItemEx(&BI, 1, buff,dwStartSecond);

			if (rxlen > 0)
			{
				BYTE *cp = buff;
				memset(title,0,sizeof(title));
				sprintf(title,"��ǰ�й�����");
				sprintf(menuitem[0], "������: %d",wAcPn);
				//	sprintf(menuitem[1], "�й�����:");
				memset(str, 0, sizeof(str));
				Fmt9ToStr(cp, str);//
				sprintf(menuitem[1], " ���й����� %skW", str);

				cp += 3;	

				memset(str, 0, sizeof(str));
				Fmt9ToStr(cp, str);//
				sprintf(menuitem[2], "A���й����� %skW", str);

				cp += 3;

				memset(str, 0, sizeof(str));
				Fmt9ToStr(cp, str);//
				sprintf(menuitem[3], "B���й����� %skW", str);

				cp += 3;

				memset(str, 0, sizeof(str));
				Fmt9ToStr(cp, str);//
				sprintf(menuitem[4], "C���й����� %skW", str);	

				sprintf(menuitem[5], "%s", "");
			}
		}
		break;
	case 6:
		//�޹�����
		{
			BI.wID = 0xB64F;
			rxlen = ReadItemEx(&BI, 1, buff,dwStartSecond);

			if (rxlen > 0)
			{
				BYTE *cp = buff;
				memset(title,0,sizeof(title));
				sprintf(title,"��ǰ�޹�����");
				sprintf(menuitem[0], "������: %d",wAcPn);
				//	sprintf(menuitem[1], "�޹�����:");
				memset(str, 0, sizeof(str));
				Fmt9ToStr(cp, str);//
				sprintf(menuitem[1], " ���޹����� %skvar", str);

				cp += 3;	

				memset(str, 0, sizeof(str));
				Fmt9ToStr(cp, str);//
				sprintf(menuitem[2], "A���޹����� %skvar", str);

				cp += 3;

				memset(str, 0, sizeof(str));
				Fmt9ToStr(cp, str);//
				sprintf(menuitem[3], "B���޹����� %skvar", str);

				cp += 3;

				memset(str, 0, sizeof(str));
				Fmt9ToStr(cp, str);//
				sprintf(menuitem[4], "C���޹����� %skvar", str);	

				sprintf(menuitem[5], "%s", "");
			}
		}
		break;
	case 7:
		//��������
		{
			BI.wID = 0xB65F;
			rxlen = ReadItemEx(&BI, 1, buff,dwStartSecond);

			if (rxlen > 0)
			{
				BYTE *cp = buff;
				memset(title,0,sizeof(title));
				sprintf(title,"��ǰ��������");
				sprintf(menuitem[0], "������: %d",wAcPn);
				//	sprintf(menuitem[1], "��������:");
				memset(str, 0, sizeof(str));
				Fmt5ToStr(cp, str);
				sprintf(menuitem[1], " �ܹ������� %s%%", str);
				memset(str, 0, sizeof(str));
				Fmt5ToStr(cp+2, str);
				sprintf(menuitem[2], "A�๦������ %s%%", str);
				memset(str, 0, sizeof(str));
				Fmt5ToStr(cp+4, str);
				sprintf(menuitem[3], "B�๦������ %s%%", str);
				memset(str, 0, sizeof(str));
				Fmt5ToStr(cp+6, str);
				sprintf(menuitem[4], "C�๦������ %s%%", str);

				sprintf(menuitem[5], "%s", "");
			}
		}
		break;
	case 8://�ܼ���ʣ�����
		{
			int iVal = 0;
			memset(title,0,sizeof(title));
			sprintf(title,"�ܼ���ʣ�����");
			for (BYTE bGrp=1; bGrp<=8; bGrp++)
			{
				if (IsGrpValid(bGrp))
				{
					if (ReadItemEx(BN0, bGrp, 0x110f, buff) > 0) //C1F23 �ܼ��� �ն˵�ǰʣ��������ѣ�
					{
						iVal += Fmt3ToVal(buff, 4);
					}
				}
			}

			ValToFmt3(iVal, buff, 4);
			Fmt03ToStr(buff, str);
			sprintf(menuitem[0], "");
			sprintf(menuitem[1], "��ʣ�����: %s", str); //��ɽ���������ļ����Աȷ�ϣ���������Ч�ܼ����ʣ�����֮��
		}
		break;
	case 9://�ն��߼���ַ
		{
			i = 0;
			BYTE *pbFmt = NULL;
			WORD wFmtLen = 0;
			BYTE *p = NULL;
			if (0 > OoReadAttr(0x4001, 0x02, bBuf, &pbFmt, &wFmtLen))
				return -1;

			bAddrLen = bBuf[1];

			/*while(i < bAddrLen)
			{
				szServAddr[i] = (bBuf[2+i]&0xf0)>>4;
				szServAddr[i+1] = bBuf[2+i]&0x0f;	
				i++;
			}
			ByteToASCII();*/
			for (i=0; i<bAddrLen; i++)
			{
				p = (BYTE *)(szServAddr+i);
				ByteToASCII(bBuf[2+i], &p);
			}
			//rxlen = ReadItemEx(BN10, PN0, 0xa04f,buff);

			//if (rxlen > 0)
			//{
			i = 0;
			memset(title,0,sizeof(title));
			sprintf(title,"�ն���Ϣ");

			sprintf(menuitem[i++],"���̴���: %C%C%C%C",g_bTermSoftVer[4],g_bTermSoftVer[5],g_bTermSoftVer[6],g_bTermSoftVer[7]);
			//sprintf(menuitem[i++],"�豸���: %s",&TmpBuff[8]);
			sprintf(menuitem[i++],"����汾: %c%c%c%c",g_bTermSoftVer[10],g_bTermSoftVer[11],g_bTermSoftVer[12],g_bTermSoftVer[13]);
			sprintf(menuitem[i++],"�������: %c%c-%c%c-%c%c",g_bTermSoftVer[16],g_bTermSoftVer[17],g_bTermSoftVer[18],g_bTermSoftVer[19],g_bTermSoftVer[20],g_bTermSoftVer[21]);
			sprintf(menuitem[i++],"Ӳ���汾: %c%c%c%c",g_bTermSoftVer[24],g_bTermSoftVer[25],g_bTermSoftVer[26],g_bTermSoftVer[27]);
			sprintf(menuitem[i++],"Ӳ������: %c%c-%c%c-%c%c",g_bTermSoftVer[30],g_bTermSoftVer[31],g_bTermSoftVer[32],g_bTermSoftVer[33],g_bTermSoftVer[34],g_bTermSoftVer[35]);
			sprintf(menuitem[i++], "��������ַ��");  //��������ַ�����Ǳ䳤����Ҫ�����д���
			memset(bAddrBuf, 0, sizeof(bAddrBuf));
			memcpy(bAddrBuf, &szServAddr[0], 16);
			sprintf(menuitem[i++], "%s", (char *)bAddrBuf);
			memcpy(bAddrBuf, &szServAddr[16], 16);
			if (IsAllAByte(bAddrBuf, 0, sizeof(bAddrBuf)))
				sprintf(menuitem[i], " "); //����û���
			else
				sprintf(menuitem[i], "%s", (char *)bAddrBuf);
			//}
		}
		break;
	case 10://��ǰ���ڼ�ʱ��
		{
			memset(title,0,sizeof(title));
			sprintf(title,"��ǰ���ڼ�ʱ��");
			TTime tmTimeNow;
			GetCurTime(&tmTimeNow);
			sprintf(menuitem[0], " ��ǰ����: %04d-%02d-%02d", tmTimeNow.nYear,tmTimeNow.nMonth,tmTimeNow.nDay);
			sprintf(menuitem[1], " ��ǰʱ��: %02d:%02d",tmTimeNow.nHour, tmTimeNow.nMinute); //����ʾ�룬�������²���
		}
		break;
	}

	
	fNeedDisplay = true;

	//j++;
	if (j == bLoopPageNum)
	{
		j = 0;
		fNeedDisplay = false;
		g_PastClick = GetClick();
	}
	else if (j > bLoopPageNum) // j = bLoopPageNum������������ʾ
	{
		j = 0;
	}
	else
		j++;


	if (fNeedDisplay)
	{
		static CListBoxEx listbox(LISTBOXVALUE);
		listbox.Show(0, title, tmReadPowerMeter, KEY_RIGHT | KEY_LEFT << 8 | KEY_OK << 16, 1000,false);
	}

	return -1;
}

