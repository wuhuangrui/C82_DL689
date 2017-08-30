/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TermEvtTask.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��DL/T 698.45 ���¼�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��10��
 *********************************************************************************************************/
#include "stdafx.h"
#include "syscfg.h"
#include "FaCfg.h"
#include "Mem.h"
#include <fcntl.h>
#include "FrzTask.h"
#include "FaAPI.h"
#include "DbOIAPI.h"
#include "OIObjInfo.h"
#include "CctTaskMangerOob.h"
#include "TermEvtTask.h"
#include "DL69845.h"
#include "MeterAPI.h"
#include "MtrHook.h"
#include "MtrCtrl.h"
#include "StdReader.h"
#include "CctSchMtr.h"


//�¼�����Ĭ��
//�ն�����---��ͬ���͵��ն���Ҫ֧�ֵ��¼���ͬ
#if FA_TYPE == FA_TYPE_C82
const WORD g_wValidCfg[] = {
	MTR_VLOSS,					//ʧѹ					1
	MTR_VLESS,					//Ƿѹ					2
	MTR_VOVER,					//��ѹ					3
	MTR_VBREAK,					//����					4
	MTR_ILOSS,					//ʧ��					5
	MTR_IOVER,					//����					6
	MTR_IBREAK,					//����					7
	MTR_PREVERSE,				//��������				8
	MTR_POVER,					//����					9
	MTR_PDMDOVER,				//�����й���������		10
	MTR_RPDMDOVER,				//�����й���������		11
	MTR_QDMDOVER,				//�޹���������			12
	MTR_PFUNDER,				//��������������		13
	MTR_ALLVLOSS,				//ȫʧѹ				14
	MTR_VDISORDER,				//��ѹ������			15
	MTR_IDISORDER,				//����������			16
	MTR_MTRCLEAR,				//�������				17
	MTR_DMDCLEAR,				//��������				18
	MTR_EVTCLEAR,				//�¼�����				19
	MTR_VUNBALANCE,				//��ѹ��ƽ��			20
	MTR_IUNBALANCE,				//������ƽ��			21
	MTR_ISUNBALANCE,			//�������ز�ƽ��		22
	MTR_CLKERR,					//ʱ�ӹ���				23
	MTR_MTRCHIPERR,				//����оƬ����			24

	TERM_INIT,					//�ն˳�ʼ���¼�		1
	TERM_VERCHG,				//�ն˰汾����¼�		2
	TERM_YXCHG,					//�ն�״̬����λ�¼�	3
	TERM_POWOFF,				//�ն�ͣ/�ϵ��¼�		4
	TERM_MSGAUTH,				//�ն���Ϣ��֤�����¼�	5
	TERM_DEVICEERR,				//�豸���ϼ�¼			6			enum
	TERM_FLUXOVER,				//��ͨ�����������¼�	7	
	TERM_UNKNOWNMTR,			//����δ֪���ܱ��¼�	8
	TERM_STEPAREA,				//��̨�����ܱ��¼�	9
	TERM_CLOCKPRG,				//�ն˶�ʱ�¼�			10
	TERM_TERMPRG,				//�ն˱�̼�¼			12
	TERM_MTRCLKPRG,				//�ն˶Ե��Уʱ��¼	14			TSA
};
#elif FA_TYPE == FA_TYPE_K32
const WORD g_wValidCfg[] = {
	MTR_MTRCLEAR,				//�������				17
	MTR_EVTCLEAR,				//�¼�����				19
	MTR_CLKERR,					//ʱ�ӹ���				23
	MTR_MTRCHIPERR,				//����оƬ����			24

	TERM_INIT,					//�ն˳�ʼ���¼�		1
	TERM_VERCHG,				//�ն˰汾����¼�		2
	TERM_YXCHG,					//�ն�״̬����λ�¼�	3
	TERM_POWOFF,				//�ն�ͣ/�ϵ��¼�		4
	TERM_MSGAUTH,				//�ն���Ϣ��֤�����¼�	5
	TERM_DEVICEERR,				//�豸���ϼ�¼			6			enum
	TERM_FLUXOVER,				//��ͨ�����������¼�	7	
	TERM_CLOCKPRG,				//�ն˶�ʱ�¼�			8
	TERM_TERMPRG,				//�ն˱�̼�¼			10
	TERM_MTRCLKPRG,				//�ն˶Ե��Уʱ��¼	12			TSA
};
#elif FA_TYPE == FA_TYPE_D82
const WORD g_wValidCfg[] = {
	MTR_VLOSS,					//ʧѹ					1
	MTR_VLESS,					//Ƿѹ					2
	MTR_VOVER,					//��ѹ					3
	MTR_VBREAK,					//����					4
	MTR_ILOSS,					//ʧ��					5
	MTR_IOVER,					//����					6
	MTR_IBREAK,					//����					7
	MTR_PREVERSE,				//��������				8
	MTR_POVER,					//����					9
	MTR_PDMDOVER,				//�����й���������		10
	MTR_RPDMDOVER,				//�����й���������		11
	MTR_QDMDOVER,				//�޹���������			12
	MTR_PFUNDER,				//��������������		13
	MTR_ALLVLOSS,				//ȫʧѹ				14
	MTR_VDISORDER,				//��ѹ������			15
	MTR_IDISORDER,				//����������			16
	MTR_MTRCLEAR,				//�������				17
	MTR_DMDCLEAR,				//��������				18
	MTR_EVTCLEAR,				//�¼�����				19
	MTR_VUNBALANCE,				//��ѹ��ƽ��			20
	MTR_IUNBALANCE,				//������ƽ��			21
	MTR_ISUNBALANCE,			//�������ز�ƽ��		22
	MTR_CLKERR,					//ʱ�ӹ���				23
	MTR_MTRCHIPERR,				//����оƬ����			24

	TERM_INIT,					//�ն˳�ʼ���¼�		1
	TERM_VERCHG,				//�ն˰汾����¼�		2
	TERM_YXCHG,					//�ն�״̬����λ�¼�	3
	TERM_POWOFF,				//�ն�ͣ/�ϵ��¼�		4
	TERM_MSGAUTH,				//�ն���Ϣ��֤�����¼�	5
	TERM_DEVICEERR,				//�豸���ϼ�¼			6			enum
	TERM_FLUXOVER,				//��ͨ�����������¼�	7	
	TERM_UNKNOWNMTR,			//����δ֪���ܱ��¼�	8
	TERM_STEPAREA,				//��̨�����ܱ��¼�	9	
	TERM_CLOCKPRG,				//�ն˶�ʱ�¼�			10
	TERM_YKCTRLBREAK,			//ң����բ��¼			11			OAD
	TERM_EPOVER,				//�й��ܵ������Խ���¼���¼		12
	TERM_TERMPRG,				//�ն˱�̼�¼			13
	TERM_CURCIRC,				//�ն˵�����·�쳣�¼�	11			enum
	TERM_MTRCLKPRG,				//�ն˶Ե��Уʱ��¼	12			TSA
	TERM_POWCTRLBREAK,			//������բ��¼			13			OI
	TERM_ELECTRLBREAK,			//�����բ��¼			14			OI
	TERM_PURCHPARACHG,			//����������ü�¼		15			OI 				
	TERM_ELECTRLALARM,			//��ظ澯�¼���¼		16			OI
};
#endif

//�������Ա����Ĭ��
//0x3000 ���ܱ�ʧѹ�¼�
const BYTE g_bVLoCfg[] = {
	0x01,0x26,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,0x51,0x20,0x29,0x62,0x00,	
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3001 ���ܱ�Ƿѹ�¼�
const BYTE g_bVLeCfg[] = {
	0x01,0x26,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,0x51,0x20,0x29,0x62,0x00,	
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3002 ���ܱ��ѹ�¼�
const BYTE g_bVOCfg[] = {
	0x01,0x26,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,0x51,0x20,0x29,0x62,0x00,	
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3003 ���ܱ�����¼�
const BYTE g_bVBCfg[] = {
	0x01,0x26,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,0x51,0x20,0x29,0x62,0x00,	
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3004 ���ܱ�ʧ���¼�
const BYTE g_bILoCfg[] = {
	0x01,0x25,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3005 ���ܱ�����¼�
const BYTE g_bIOCfg[] = {
	0x01,0x25,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3006 ���ܱ�����¼�
const BYTE g_bIBCfg[] = {
	0x01,0x25,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3007 ���ܱ�����¼�
const BYTE g_bPRCfg[] = {
	0x01,0x20,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3008 ���ܱ�����¼�
const BYTE g_bPOCfg[] = {
	0x01,0x20,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3009 ���ܱ������й����������¼�
const BYTE g_bPDCfg[] = {
	0x01,0x00,
};
//0x300A ���ܱ����й����������¼�
const BYTE g_bRPDCfg[] = {
	0x01,0x00,
};
//0x300B ���ܱ��޹����������¼�
const BYTE g_bQDCfg[] = {
	0x01,0x00,
};
//0x300C ���ܱ��������������¼�
const BYTE g_bPfOCfg[] = {
	0x01,0x08,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
};
//0x300D ���ܱ�ȫʧѹ�¼�
const BYTE g_bAVLCfg[] = {
	0x01,0x01,
	0x51,0x20,0x01,0x22,0x00,
};
//0x300F ���ܱ��ѹ�������¼�
const BYTE g_bVDCfg[] = {
	0x01,0x20,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3010 ���ܱ�����������¼�
const BYTE g_bIDCfg[] = {
	0x01,0x20,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3013 ��������¼�
const BYTE g_bMCCfg[] = {
	0x01,0x18,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x50,0x22,0x01,0x51,0x00,0x60,0x22,0x01,0x51,0x00,0x70,0x22,0x01,0x51,0x00,0x80,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x51,0x22,0x01,0x51,0x00,0x61,0x22,0x01,0x51,0x00,0x71,0x22,0x01,0x51,0x00,0x81,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x52,0x22,0x01,0x51,0x00,0x62,0x22,0x01,0x51,0x00,0x72,0x22,0x01,0x51,0x00,0x82,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x53,0x22,0x01,0x51,0x00,0x63,0x22,0x01,0x51,0x00,0x73,0x22,0x01,0x51,0x00,0x83,0x22,0x01,
};

//0x3014 ���������¼�
const BYTE g_bDCCfg[] = {
	0x01,0x18,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x50,0x22,0x01,0x51,0x00,0x60,0x22,0x01,0x51,0x00,0x70,0x22,0x01,0x51,0x00,0x80,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x51,0x22,0x01,0x51,0x00,0x61,0x22,0x01,0x51,0x00,0x71,0x22,0x01,0x51,0x00,0x81,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x52,0x22,0x01,0x51,0x00,0x62,0x22,0x01,0x51,0x00,0x72,0x22,0x01,0x51,0x00,0x82,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x53,0x22,0x01,0x51,0x00,0x63,0x22,0x01,0x51,0x00,0x73,0x22,0x01,0x51,0x00,0x83,0x22,0x01,
};
//0x3015 ���ܱ��¼������¼�
const BYTE g_bECCfg[] = {
	0x01,0x00,
};
//0x301D ���ܱ��ѹ��ƽ���¼�
const BYTE g_bVNCfg[] = {
	0x01,0x21,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x26,0x62,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x301E ���ܱ������ƽ���¼�
const BYTE g_bINCfg[] = {
	0x01,0x21,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x27,0x62,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x302D ���ܱ�������ز�ƽ���¼�
const BYTE g_bISNCfg[] = {
	0x01,0x21,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x27,0x62,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x302E ʱ�ӹ���
const BYTE g_bCECfg[] = {
	0x01,0x00,
};
//0x302F ����оƬ����
const BYTE g_bMCEDCfg[] = {
	0x01,0x00,
};
//0x3100 �ն˳�ʼ���¼�
const BYTE g_bITCfg[] = {
	0x01,0x00,
};
//0x3101 �ն˰汾����¼�
const BYTE g_bVChDCfg[] = {
	0x01,0x02,
	0x51,0x43,0x00,0x23,0x02,
	0x51,0x43,0x00,0x43,0x02, 
};
//0x3104 �ն�״̬����λ�¼�
const BYTE g_bYXCfg[] = {
	0x01,0x05,
	0x51,0x20,0x1E,0x42,0x00,
	0x51,0xF2,0x03,0x42,0x01, 
	0x51,0xF2,0x03,0x42,0x02, 
	0x51,0xF2,0x03,0x42,0x03, 
	0x51,0xF2,0x03,0x42,0x04, 
};
//0X3106�ն�ͣ/�ϵ��¼� �ն�ͣ/�ϵ��¼�
const BYTE g_bPowCfg[] = {
	0x01,0x00,
};
//0x3109 �ն���Ϣ��֤�����¼�
const BYTE g_bMsgCfg[] = {
	//0x01,0x01,
	//0x51,0x44,0x01,0x22,0x00,	//����Ŀǰ��֧�ִ�ID����ʱ���մ���
	0x01,0x00,
};
//0x310A �豸���ϼ�¼
const BYTE g_bDevCfg[] = {
	0x01,0x00,
};
//0x3110 ��ͨ�����������¼�
const BYTE g_bFluxCfg[] = {
	0x01,0x02,
	0x51,0x22,0x00,0x42,0x02,
	0x51,0x31,0x10,0x06,0x01, 
};
//0x3111 ����δ֪���ܱ��¼�
const BYTE g_bUKnMtrCfg[] = {
	0x01,0x00,
};
//0x3112 ��̨�����ܱ��¼�
const BYTE g_bStepACfg[] = {
	0x01,0x00,
};
//0x3114 �ն˶�ʱ�¼�
const BYTE g_bClkCfg[] = {
	0x01,0x02,
	0x51,0x40,0x00,0x22,0x00,
	0x51,0x40,0x00,0x82,0x00,
};
//0x3115 ң����բ��¼
const BYTE g_bYKCfg[] = {
	0x01,0x00,
};
//0x3116 �й��ܵ������Խ���¼���¼
const BYTE g_bEpOCfg[] = {
	0x01,0x00,
};
//0x3118 �ն˱�̼�¼
const BYTE g_bPrgCfg[] = {
	0x01,0x00,
};
//0x3119 �ն˵�����·�쳣�¼�
const BYTE g_bCurCCfg[] = {
	0x01,0x02,
	0x51,0x20,0x03,0x42,0x00,
	0x51,0x00,0x10,0x42,0x00,	
};
//0x311B �ն˶Ե��Уʱ��¼
const BYTE g_bMtrClkPrgCfg[] = {
	0x01,0x00,
};
//0x3200 ������բ��¼
const BYTE g_bPCtCfg[] = {
	0x01,0x01,
	0x51,0x23,0x01,0x23,0x00,
};
//0x3201 �����բ��¼
const BYTE g_bECtBCfg[] = {
	0x01,0x01,
	0x51,0x23,0x01,0x49,0x00,
};
//0x3202 ����������ü�¼
const BYTE g_bPChCfg[] = {
	0x01,0x00,
	//0x51,0x81,0x0C,0x22,0x01,
};
//0x3203 ��ظ澯�¼���¼
const BYTE g_bECtCfg[] = {
	0x01,0x00,
};


//�ն������¼�������֧�ֵĺͲ�֧�ֵ�
//����EVT_CLR_ID(�¼�����)����EVT_TRIG_IDID(����һ���¼�)�ķ��䡣
const WORD g_wTermEvt[EVT_TOTAL_NUM] = {
	MTR_VLOSS,MTR_VLESS,MTR_VOVER,MTR_VBREAK,MTR_ILOSS,MTR_IOVER,MTR_IBREAK,MTR_PREVERSE,MTR_POVER,MTR_PDMDOVER,
	MTR_RPDMDOVER,MTR_QDMDOVER,MTR_PFUNDER,MTR_ALLVLOSS,MTR_SUPLYPOWDOWN,MTR_VDISORDER,MTR_IDISORDER,MTR_POWERDOWN,MTR_PROGRAM,MTR_MTRCLEAR,
	MTR_DMDCLEAR,MTR_EVTCLEAR,MTR_SETCLOCK,MTR_DAYSTAGE,MTR_TIMEZONE,MTR_WEEKREST,MTR_ACOUNTDAY,MTR_OPENCOVER,MTR_OPENTAILOVER,MTR_VUNBALANCE,
	MTR_IUNBALANCE,MTR_RELAYLAZHA,MTR_RELAYHEZHA,MTR_HOLIDAY,MTR_MIXDPEXP,MTR_MIXDQEXP,MTR_TARIFFPRICE,MTR_STAIRPRICE,MTR_UPDATEKEY,MTR_CARDABNAORMAL,
	MTR_PURCHASE,MTR_DECREASEPURSE,MTR_MAGNTEITCINT,MTR_SWITCHABNORMAL,MTR_POWERABNORMAL,MTR_ISUNBALANCE,MTR_CLKERR,MTR_MTRCHIPERR,MTR_MODULECHANGE,TERM_INIT,
	TERM_VERCHG,TERM_YXCHG,TERM_POWOFF,TERM_DIGITOVER,TERM_DIGITUNDER,TERM_MSGAUTH,TERM_DEVICEERR,TERM_FLUXOVER,TERM_UNKNOWNMTR,TERM_STEPAREA,
	TERM_CLOCKPRG,TERM_YKCTRLBREAK,TERM_EPOVER,TERM_OUTPUTSTACHG,TERM_TERMPRG,TERM_CURCIRC,TERM_ONLINESTACHG,TERM_MTRCLKPRG,TERM_POWCTRLBREAK,TERM_ELECTRLBREAK,
	TERM_PURCHPARACHG,TERM_ELECTRLALARM,
};
//��������ȡ�¼�Sn(���)�������¼�����ʹ���һ���¼���wPn�Ļ�ȡ
//������@wOI �����ʶ
//���أ���ȷ�򷵻�wOI��Ӧ����ţ����򷵻�-1
int GetEvtSn(WORD wOI)
{
	for(BYTE i=0; i<EVT_TOTAL_NUM; i++)
	{
		if (g_wTermEvt[i] == wOI) 
			return i;
	}
	return -1;	
}


//�¼���������ʵ����Class7
const TEvtAttr g_tIC7EvtAttr = {
	IC7_ATTRTAB,
	IC7_CURRECNUM,
	IC7_MAXNUM,
	IC7_PARA,
	IC7_RECORDTAB,
	IC7_CURRECLIST,
	IC7_REPROTFLAG,
	IC7_VALIDFLAG,
};
//�¼���������ʵ����Class24����ABC���޹�1234
const TEvtAttr g_tIC24EvtAttr4Item = {
	IC24_ATTRTAB,
	IC24_CURRECNUM,
	IC24_MAXNUM,
	IC24_PARA,
	IC24_RECORDTAB1,
	IC24_CURRECLIST,
	IC24_REPROTFLAG,
	IC24_VALIDFLAG,
};
//�¼���������ʵ����Class24��ֻ��ABC
const TEvtAttr g_tIC24EvtAttr3Item = {
	IC24_ATTRTAB,
	IC24_CURRECNUM,
	IC24_MAXNUM,
	IC24_PARA,
	IC24_RECORDTAB2,
	IC24_CURRECLIST,
	IC24_REPROTFLAG,
	IC24_VALIDFLAG,
};

//�¼��̶��ֶΣ�������������Ϊ0ֵ
//IC24 �����¼���¼��Ԫ
BYTE g_bIC24EvtFixList[] = {
	DT_ARRAY,
	0x04,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
};
// 3301 8 ��׼�¼���¼��Ԫ
BYTE g_bStdEvtFixList[] = {
	DT_ARRAY,
	0x05,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	//�¼���¼���  double-long-unsigned	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	//�¼�����ʱ��  date_time_s		
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	//�¼�����ʱ��  date_time_s		
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	//�¼�����Դ    instance-specific		
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	//�¼��ϱ�״̬  array ͨ���ϱ�״̬		
};
// 3303 8 ����δ֪���ܱ��¼���Ԫ
BYTE g_bUnKnMtrFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x03, 0x02, 0x06,	//�ѱ���      array һ���ѱ���
};

// 3304 8 ��̨�����ܱ��¼���Ԫ
BYTE g_bStepAreaFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x04, 0x02, 0x06,	//��̨���ѱ���  array  һ����̨�����	
};

// 3302 8 ��̼�¼�¼���Ԫ
BYTE g_bTermPrgEvtFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x02, 0x02, 0x06,	//��̶����б�  array OAD	
};



// 3305 8 ������բ��¼��Ԫ
BYTE g_bPowCtrlEvtFixList[] = {
	DT_ARRAY,
	0x0A,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x05, 0x02, 0x06,	//�¼�������2���ӹ���  long64(��λ��W������-1)
	DT_OAD, 0x33, 0x05, 0x02, 0x07,	//���ƶ���  OI
	DT_OAD, 0x33, 0x05, 0x02, 0x08,	//��բ�ִ�  bit-string(SIZE(8))
	DT_OAD, 0x33, 0x05, 0x02, 0x09,	//���ض�ֵ  long64����λ��kW������-4��
	DT_OAD, 0x33, 0x05, 0x02, 0x0A,	//��բ����ǰ�ܼ��й�����    long64����λ��kW������-4��
};
// 3306 8 �����բ��¼��Ԫ
BYTE g_bEleCtrlEvtFixList[] = {
	DT_ARRAY,
	0x09,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x06, 0x02, 0x06,	//���ƶ���  OI
	DT_OAD, 0x33, 0x06, 0x02, 0x07,	//��բ�ִ�  bit-string(SIZE(8))
	DT_OAD, 0x33, 0x06, 0x02, 0x08,	//���ض�ֵ  long64����λ��kW������-4��
	DT_OAD, 0x33, 0x06, 0x02, 0x09,	//��բ����ǰ�ܼ��й����� long64����λ��kW������-4��
};
// 3307 8 ��ظ澯�¼���Ԫ
BYTE g_bEleAlarmEvtFixList[] = {
	DT_ARRAY,
	0x07,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x07, 0x02, 0x06,	//���ƶ���  OI	
	DT_OAD, 0x33, 0x07, 0x02, 0x07,	//��ض�ֵ  long64����λ��kWh������-4��	
};
// 3308 8 ���ܱ����������¼���Ԫ
BYTE g_bDmdEvtFixList[] = {
	DT_ARRAY,
	0x07,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x08, 0x02, 0x06,	//�����ڼ��������ֵ  double-long-unsigned
	DT_OAD, 0x33, 0x08, 0x02, 0x07,	//�����ڼ��������ֵ����ʱ��  date_time_s
};
// 3309 8 ͣ/�ϵ��¼���¼��Ԫ
BYTE g_bPowOffEvtFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x09, 0x02, 0x06,	//���Ա�־	bit-string��SIZE(8)��
};
// 330A 8 ң���¼���¼��Ԫ
BYTE g_bYKCtrlEvtFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x0A, 0x02, 0x06,	//�غ�2�����ܼ��鹦�� array long64
};
// 330B 8 �й��ܵ������Խ���¼���¼��Ԫ
BYTE g_bEpOverlEvtFixList[] = {
	DT_ARRAY,
	0x09,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x0B, 0x02, 0x06,	//Խ��ʱ�Ա��ܼ����й��ܵ����� long64����λ��kWh�����㣺-4����
	DT_OAD, 0x33, 0x0B, 0x02, 0x07,	//Խ��ʱ�����ܼ����й��ܵ����� long64����λ��kWh�����㣺-4����
	DT_OAD, 0x33, 0x0B, 0x02, 0x08,	//Խ��ʱ�Խ�����ƫ��ֵ integer����λ��%�����㣺0����
	DT_OAD, 0x33, 0x0B, 0x02, 0x09,	//Խ��ʱ�Խ�޾���ƫ��ֵ long64����λ��kWh�����㣺-4��
};

// 330C 8 �¼������¼���¼��Ԫ
BYTE g_bEvtClrEvtFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x0C, 0x02, 0x06,	//�¼������б�  array OMD
};

// 330D 8 �ն˶Ե��Уʱ��¼��Ԫ
BYTE g_bMtrClkPrgFixList[] = {
	DT_ARRAY,
	0x07,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x0D, 0x02, 0x06,	//Уʱǰʱ��    date_time_s
	DT_OAD, 0x33, 0x0D, 0x02, 0x07,	//ʱ�����      integer����λ���룬�޻��㣩
};
#define FIXLIST_ELEMENT_MAXNUM	0x0a//0x3301~0x3311��¼��Ԫ�����й̶��ֶ�Ԫ�ص�������


//IC7 ��ǰ��¼�����ݽṹ
const BYTE g_bIC7CurNum[3] = {
	DT_LONG_U,0x00,0x00,	
};
//IC24 ��ǰ��¼�����ݽṹ
const BYTE g_bIC24CurNum[14] = {
	DT_STRUCT,	
	0x04,
	DT_LONG_U,0x00,0x00,
	DT_LONG_U,0x00,0x00,
	DT_LONG_U,0x00,0x00,
	DT_LONG_U,0x00,0x00,
};

//�¼�����Դ
BYTE g_bEvtSrcNullFmt[] = {DT_NULL};	
BYTE g_bEvtSrcOADFmt[] = {DT_OAD};	
BYTE g_bEvtSrcEnumFmt[] = {DT_ENUM};	
BYTE g_bEvtSrcOIFmt[] = {DT_OI};	
BYTE g_bEvtSrcTSAFmt[] = {DT_TSA};	

//IC7�ڱ��¼���ǰֵ��¼�����ݽṹ
//��g_bEvtSrcNullFmtƥ��
const BYTE g_bSrcNullCurRecList[17] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	DT_NULL,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
};
//��g_bEvtSrcOADFmtƥ��
const BYTE g_bSrcOADCurRecList[21] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	DT_OAD,	0x00,0x00,0x00,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
};
//��g_bEvtSrcEnumFmtƥ��
const BYTE g_bSrcEnumCurRecList[18] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	DT_ENUM, 0x00,
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
};
//��g_bEvtSrcOIFmtƥ��
const BYTE g_bSrcOICurRecList[19] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	DT_OI,0x00,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
};
//��g_bEvtSrcTSAFmtƥ��
const BYTE g_bSrcTSACurRecList[35] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	0x00,//DT_TSA,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//�¼�����Դ�����ֽ�
};

//IC24��ǰֵ��¼�����ݽṹ
const BYTE g_bIC24CurRecList[50] = {
	DT_STRUCT,	
	0x04,
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
};
//ʧѹͳ�����ݽṹ
const BYTE g_bAllVLossSta[28] = {
	DT_STRUCT,	
	0x04,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	DT_DATE_TIME_S,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	DT_DATE_TIME_S,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
};
//ͨ���ϱ�״̬�ṹ
const BYTE g_bCnRptState[CN_RPT_TOTAL_LEN] = {
	DT_STRUCT,	
	0x02,
	DT_OAD, 0x00, 0x00, 0x00, 0x00,	
	DT_UNSIGN, 0x00,		
};


#define TERM_EVT_NUM	64			//��Ҫ��g_EvtCtrl[]��Ķ���ƥ��
TEvtBase g_EvtBase[TERM_EVT_NUM];	//�¼��������ݽṹ
TTermMem g_TermMem;					//�¼���ʱ�ռ�
TVLoss g_VLoss;
TDmd g_PDmd;
TDmd g_RPDmd;
TDmd g_QDmd;
TUnKnMtr g_UnKnMtr;
TStepArea g_StepArea;
TEvtClr g_EvtClr;
TYXChgCtrl g_YXChgCtrl;
TPowOff g_PowOff;
TDeviceErr g_DeviceErr;
TYKCtrl g_YKCtrl;
TEpOver g_EpOver;
TTermPrg g_TermPrg;
TCurCirc g_CurCirc;
TMtrClkPrg g_MtrClkPrg;
TPowCtrl g_PowCtrl;
TEleCtrl g_EleCtrl;
TPurchParaChg g_PurchParaChg;
TEleAlram g_EleAlram;
TAdjTermTime g_AdjTermTime = { 0 };

TTermEvtCtrl g_EvtCtrl[] = 	//�¼����ƽṹ
{
	//wOI-bClass-������-��ʱʱ��- �̶��ֶ�------- �̶��ֶγ���--------- ����Դ��ʽ--------  ����Դ��ʽ����---------------�������Ա�Ĭ�����ü��䳤��--------����- ----- ˽������-----------------��ʼ��------------�ж�-------------ִ��--
	{0x3000, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL, 				0,							g_bVLoCfg,	sizeof(g_bVLoCfg),	&g_EvtBase[0], 	&g_VLoss, 				InitVLoss, 		VLossJudge, 		DoVLoss},	//ʧѹ
	{0x3001, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL, 				0,							g_bVLeCfg,	sizeof(g_bVLeCfg),	&g_EvtBase[3], 	NULL, 					InitEvt, 		VLessJudge, 		DoEvt},		//Ƿѹ
	{0x3002, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bVOCfg,	sizeof(g_bVOCfg),	&g_EvtBase[6], 	NULL, 					InitEvt, 		VOverJudge, 		DoEvt},		//��ѹ
	{0x3003, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bVBCfg, 	sizeof(g_bVBCfg),	&g_EvtBase[9], 	NULL, 					InitEvt, 		VBreakJudge, 		DoEvt},		//����
	{0x3004, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bILoCfg, 	sizeof(g_bILoCfg),	&g_EvtBase[12],	NULL, 					InitEvt, 		ILossJudge, 		DoEvt},		//ʧ��
	{0x3005, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bIOCfg, 	sizeof(g_bIOCfg),	&g_EvtBase[15],	NULL, 					InitEvt, 		IOverJudge, 		DoEvt},		//����
	{0x3006, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bIBCfg, 	sizeof(g_bIBCfg),	&g_EvtBase[18],	NULL, 					InitEvt, 		IBreakJudge, 		DoEvt},		//����
	{0x3007, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bPRCfg, 	sizeof(g_bPRCfg),	&g_EvtBase[21],	NULL, 					InitEvt, 		PReverseJudge, 		DoEvt},		//��������
	{0x3008, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bPOCfg, 	sizeof(g_bPOCfg),	&g_EvtBase[24],	NULL, 					InitEvt, 		POverJudge, 		DoEvt},		//����
	{0x3009, 7, 1, 0, g_bDmdEvtFixList, 	sizeof(g_bDmdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bPDCfg, 	sizeof(g_bPDCfg),	&g_EvtBase[27],	&g_PDmd, 				InitDmd, 		PDmdOverJudge, 		DoDmd},		//�����й���������
	{0x300A, 7, 1, 0, g_bDmdEvtFixList, 	sizeof(g_bDmdEvtFixList),	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bRPDCfg, 	sizeof(g_bRPDCfg),	&g_EvtBase[28],	&g_RPDmd,			 	InitDmd, 		RPDmdOverJudge, 	DoDmd},		//�����й���������
	{0x300B, 24, 4, 0, g_bDmdEvtFixList, 	sizeof(g_bDmdEvtFixList), 	NULL, 				0,							g_bQDCfg, 	sizeof(g_bQDCfg),	&g_EvtBase[29],	&g_QDmd, 				InitDmd, 		QDmdOverJudge, 		DoDmd},		//�޹���������
	{0x300C, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bPfOCfg, 	sizeof(g_bPfOCfg),	&g_EvtBase[33], NULL, 					InitEvt, 		PfUnderJudge, 		DoEvt},		//��������������
	{0x300D, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bAVLCfg, 	sizeof(g_bAVLCfg),	&g_EvtBase[34],	&g_PowerOffTmp.tAllVLoss, InitAVLoss, 	AVLossJudge, 		DoAVLoss},	//ȫʧѹ
	{0x300F, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bVDCfg, 	sizeof(g_bVDCfg),	&g_EvtBase[35],	NULL, 					InitEvt, 		DisOrderJudge, 		DoEvt},		//��ѹ������
	{0x3010, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bIDCfg, 	sizeof(g_bIDCfg),	&g_EvtBase[36],	NULL, 					InitEvt, 		DisOrderJudge, 		DoEvt},		//����������
	{0x3013, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bMCCfg, 	sizeof(g_bMCCfg),	&g_EvtBase[37],	NULL, 					InitEvt, 		MtrClrJudge, 		DoNullEvt},	//�������
	{0x3014, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bDCCfg, 	sizeof(g_bDCCfg),	&g_EvtBase[38],	NULL, 					InitEvt, 		DmdClrJudge, 		DoNullEvt},	//��������
	{0x3015, 7, 1, 0, g_bEvtClrEvtFixList,	sizeof(g_bEvtClrEvtFixList), g_bEvtSrcNullFmt, sizeof(g_bEvtSrcNullFmt),	g_bECCfg, 	sizeof(g_bECCfg),	&g_EvtBase[39],	&g_EvtClr, 				InitEvtClr, 	EvtClrJudge, 		DoEvt},		//�¼�����
	{0x301D, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bVNCfg, 	sizeof(g_bVNCfg),	&g_EvtBase[40],	NULL, 					InitEvt, 		VUnBalanceJudge, 	DoEvt},		//��ѹ��ƽ��
	{0x301E, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bINCfg, 	sizeof(g_bINCfg),	&g_EvtBase[41],	NULL, 					InitEvt, 		IUnBalanceJudge, 	DoEvt},		//������ƽ��
	{0x302D, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bISNCfg, 	sizeof(g_bISNCfg),	&g_EvtBase[42],	NULL, 					InitEvt, 		IUnBalanceJudge, 	DoEvt},		//�������ز�ƽ��
	{0x302E, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bCECfg, 	sizeof(g_bCECfg),	&g_EvtBase[43],	NULL, 					InitEvt, 		TermErrJudge, 		DoEvt},		//ʱ�ӹ���
	{0x302F, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bMCEDCfg, sizeof(g_bMCEDCfg),	&g_EvtBase[44],	NULL, 					InitEvt, 		TermErrJudge, 		DoEvt},		//����оƬ����
	{0x3100, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt,	sizeof(g_bEvtSrcNullFmt),	g_bITCfg, 	sizeof(g_bITCfg),	&g_EvtBase[45],	NULL, 					InitEvt, 		TermInitJudge, 		DoNullEvt},	//�ն˳�ʼ���¼�
	{0x3101, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bVChDCfg, sizeof(g_bVChDCfg),	&g_EvtBase[46],	NULL, 					InitEvt, 		TermVerChgJudge, 	DoEvt},		//�ն˰汾����¼�
	{0x3104, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList),	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bYXCfg, 	sizeof(g_bYXCfg),	&g_EvtBase[47],	&g_YXChgCtrl,			InitYXEvtCtrl,  DoYXChgJudge, 		DoEvt},		//�ն�״̬����λ�¼�
	{0x3106, 7, 1, 0, g_bPowOffEvtFixList,	sizeof(g_bPowOffEvtFixList),g_bEvtSrcEnumFmt, 	sizeof(g_bEvtSrcEnumFmt),	g_bPowCfg, 	sizeof(g_bPowCfg),	&g_EvtBase[48],	&g_PowOff, 				InitPowOff, 	PowOffJudge, 		DoEvt},		//�ն�ͣ/�ϵ��¼�
	{0x3109, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bMsgCfg, 	sizeof(g_bMsgCfg),	&g_EvtBase[49], NULL, 					InitEvt, 		GsgQAuthJudge, 		DoEvt},		//�ն���Ϣ��֤�����¼�
	{0x310A, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcEnumFmt, 	sizeof(g_bEvtSrcEnumFmt),	g_bDevCfg, 	sizeof(g_bDevCfg),	&g_EvtBase[50],	&g_DeviceErr,			InitDeviceErr, 	DeviceErrJudge, 	DoEvt},		//�豸���ϼ�¼
	{0x3110, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bFluxCfg, sizeof(g_bFluxCfg),	&g_EvtBase[51],	NULL, 					InitEvt, 		FluxOverJudge, 		DoEvt},		//��ͨ�����������¼�
	{0x3111, 7, 1, 0, g_bUnKnMtrFixList, 	sizeof(g_bUnKnMtrFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bUKnMtrCfg, sizeof(g_bUKnMtrCfg),	&g_EvtBase[52],	&g_UnKnMtr, 		InitUnKnMtr, 	UnKnMtrJudge, 		DoEvt},		//��ͨ�����������¼�
	{0x3112, 7, 1, 0, g_bStepAreaFixList, sizeof(g_bStepAreaFixList),	g_bEvtSrcNullFmt,	sizeof(g_bEvtSrcNullFmt),	g_bStepACfg, sizeof(g_bStepACfg), &g_EvtBase[53], &g_StepArea,			InitStepArea,	StepAreaJudge,		DoEvt}, 	//��ͨ�����������¼�
	{0x3114, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bClkCfg, 	sizeof(g_bClkCfg),	&g_EvtBase[54],	&g_AdjTermTime, 		InitEvt, 		TermClockPrgJudge, 	DoNullEvt},		//�ն˶�ʱ�¼�
	{0x3115, 7, 1, 0, g_bYKCtrlEvtFixList,	sizeof(g_bYKCtrlEvtFixList),g_bEvtSrcOADFmt, 	sizeof(g_bEvtSrcOADFmt),	g_bYKCfg, 	sizeof(g_bYKCfg),	&g_EvtBase[55],	&g_YKCtrl, 				InitYKCtrl, 	YKCtrlBreakJudge,	DoEvt},		//ң����բ��¼
	{0x3116, 7, 1, 0, g_bEpOverlEvtFixList, 	sizeof(g_bEpOverlEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bEpOCfg, 	sizeof(g_bEpOCfg),	&g_EvtBase[56],	&g_EpOver, 		InitEpOver, 		EpOverJudge, 	DoEvt},		//�й��ܵ������Խ���¼���¼
	{0x3118, 7, 1, 0, g_bTermPrgEvtFixList,	sizeof(g_bTermPrgEvtFixList),g_bEvtSrcNullFmt,	sizeof(g_bEvtSrcNullFmt),	g_bPrgCfg, 	sizeof(g_bPrgCfg),	&g_EvtBase[57],	&g_TermPrg, 			InitTermPrg, 	TermPrgJudge, 		DoNullEvt},		//�ն˱�̼�¼	
	{0x3119, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcEnumFmt, 	sizeof(g_bEvtSrcEnumFmt),	g_bCurCCfg, sizeof(g_bCurCCfg),	&g_EvtBase[58],	&g_CurCirc, 			InitCurCirc, 	CurCircJudge, 		DoEvt},		//�ն˵�����·�쳣�¼�
	{0x311B, 7, 1, 0, g_bMtrClkPrgFixList, 	sizeof(g_bMtrClkPrgFixList), g_bEvtSrcTSAFmt, 	sizeof(g_bEvtSrcTSAFmt),	g_bMtrClkPrgCfg, sizeof(g_bMtrClkPrgCfg),&g_EvtBase[59],&g_MtrClkPrg, 	InitMtrClkPrg, 	MtrClkPrgJudge, 	DoEvt},		//�ն˶Ե��Уʱ��¼
	{0x3200, 7, 1, 0, g_bPowCtrlEvtFixList,	sizeof(g_bPowCtrlEvtFixList),g_bEvtSrcOIFmt,	sizeof(g_bEvtSrcOIFmt),		g_bPCtCfg, 	sizeof(g_bPCtCfg),	&g_EvtBase[60],	&g_PowCtrl, 			InitPowCtrl, 	PowCtrlBreakJudge, 	DoEvt},		//������բ��¼
	{0x3201, 7, 1, 0, g_bEleCtrlEvtFixList,	sizeof(g_bEleCtrlEvtFixList),g_bEvtSrcOIFmt,	sizeof(g_bEvtSrcOIFmt),		g_bECtBCfg, sizeof(g_bECtBCfg),	&g_EvtBase[61],	&g_EleCtrl, 			InitEleCtrl, 	EleCtrlBreakJudge, 	DoEvt},		//�����բ��¼
	{0x3202, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcOIFmt,	 	sizeof(g_bEvtSrcOIFmt),		g_bPChCfg, 	sizeof(g_bPChCfg),	&g_EvtBase[62],	&g_PurchParaChg, 		InitPurchParaChg,PurChParaChgJudge, DoEvt},		//����������ü�¼
	{0x3203, 7, 1, 0, g_bEleAlarmEvtFixList,sizeof(g_bEleAlarmEvtFixList),g_bEvtSrcOIFmt,	sizeof(g_bEvtSrcOIFmt),		g_bECtCfg, 	sizeof(g_bECtCfg),	&g_EvtBase[63],	&g_EleAlram, 			InitEleAlram, 	EleCtrlAlarmJudge, 	DoEvt},		//��ظ澯�¼���¼
};
#define EVT_NUM (sizeof(g_EvtCtrl)/sizeof(TTermEvtCtrl))


//����������Ĭ�ϲ����������������Ա�����¼������Ч��ʶ
//������@ pEvtCtrl�¼�����
//���أ���
void SetTermEvtOadDefCfg(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bBuf[EVT_ATTRTAB_LEN];
	BYTE bValidBuf[2] = {DT_BOOL, 0x01};
	BYTE bINValidBuf[2] = {DT_BOOL, 0x00};
	BYTE bINValidMaxNumBuf[3] = {DT_LONG_U, 0x00, 0x00};
	BYTE bRptBuf[2] = {DT_ENUM, 0x02};
	BYTE bPwrOffRptBuf[2] = {DT_ENUM, 0x03};
	WORD wOI;
	DWORD dwOAD;
	int iLen;
	const TEvtAttr* pEvtAttr; 
	const ToaMap* pOI;
	BYTE i;
	bool fIsValidEvt = false;

	if (pEvtCtrl == NULL)	//��β��Ϸ�
		return;
	
	wOI = pEvtCtrl->wOI;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;
	dwOAD = GetOAD(wOI, pEvtAttr->bRela, 0);
	pOI = GetOIMap(dwOAD);
	if (pOI == NULL)
		return;

	memset(bBuf, 0, sizeof(bBuf));	
	iLen = ReadItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
	if (iLen>0 && IsAllAByte(bBuf, 0, sizeof(bBuf)))
	{
		memcpy(bBuf, pEvtCtrl->pbDefCfg, pEvtCtrl->wDefCfgLen);
		WriteItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
		for (i = 0; i < sizeof(g_wValidCfg)/sizeof(WORD); i++)
		{
			if (wOI == g_wValidCfg[i])
			{
				fIsValidEvt = true;
				break;
			}
		}

		if (fIsValidEvt)
		{	
			OoWriteAttr(wOI, pEvtAttr->bValidFlg, bValidBuf);
		}
		else
		{	
			OoWriteAttr(wOI, pEvtAttr->bValidFlg, bINValidBuf);
			OoWriteAttr(wOI, pEvtAttr->bMaxRecNum, bINValidMaxNumBuf);
		}

#if FA_TYPE == FA_TYPE_D82
		if (wOI==TERM_POWOFF || wOI==TERM_DEVICEERR || wOI==TERM_CLOCKPRG || wOI==TERM_CURCIRC || wOI==TERM_MTRCLKPRG)
		{
			if (wOI == TERM_POWOFF)
				OoWriteAttr(wOI, pEvtAttr->bRepFlg, bPwrOffRptBuf);
			else
				OoWriteAttr(wOI, pEvtAttr->bRepFlg, bRptBuf);
		}
#endif	
		TrigerSaveBank(BN0, SECT3, -1);
	}
	
	return;
}

//��������ȡ�¼����ƽṹ
//������@wOI �����ʶ
//���أ���ȷ�򷵻�wOI��Ӧ���¼����ƽṹ�����򷵻�NULL
TTermEvtCtrl* GetTermEvtCtrl(WORD wOI)
{
	for(BYTE i=0; i<EVT_NUM; i++)
	{
		if (g_EvtCtrl[i].wOI == wOI) 
			return &g_EvtCtrl[i];
	}
	return NULL;	
}
//�����������¼����࣬ȡ���¼����ԵĶ���
//������@ pEvtCtrl�¼�����
//����:�����ȷ�򷵻��¼����ԵĶ��壬���򷵻�NULL
const TEvtAttr* GetEvtAttr(TEvtCtrl* pEvtCtrl)
{
	if (pEvtCtrl == NULL)	//��β��Ϸ�
		return NULL;
	
	if (pEvtCtrl->bClass == IC7)
		return &g_tIC7EvtAttr;
	else if ((pEvtCtrl->bClass==IC24) && (pEvtCtrl->bItemNum==4))
		return &g_tIC24EvtAttr4Item;
	else if ((pEvtCtrl->bClass==IC24) && (pEvtCtrl->bItemNum==3))
		return &g_tIC24EvtAttr3Item;	
	else
		return NULL;
}

//��������ȡ�õ��¼�����Դʵ������
//������@ pEvtCtrl �¼�����
//		@pbSrcBuf �����¼�����Դʵ������
//		@bType = 0 ֻ���� ���ȣ�bType = 1 ֱ�ӿ�����bType=2��ǰֵ��¼��������Ҫ��λ��ȡֵ�������TSA�����
//���أ���ȷ�����¼�����Դ���ȣ����򷵻�-1
int GetEvtSrcData(TEvtCtrl* pEvtCtrl, BYTE* pbSrcBuf, BYTE bType)
{
	int iRet = -1;
	int iTsaLen;
	BYTE bBuf[12] = {0};

	if ((pEvtCtrl==NULL) || (pbSrcBuf==NULL) || (bType > 3))		//��β��Ϸ�
		return -1;
	if (pEvtCtrl->pbSrcFmt == NULL)	//��Դ���Ҳ���Ҫ��¼�¼�����Դ
		return 0;

	if (bType)	//��Ҫ�������Ȼ�ȡ����
		*pbSrcBuf++ = pEvtCtrl->pbSrcFmt[0];
	else
		pbSrcBuf++;
	
	if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcNullFmt[0])
	{
		return 1;	//ԴΪ�գ��̶�һ���ֽ�00	
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOADFmt[0])
	{	
		iRet = 5;	
		if (bType)	//��Ҫ����
		{
			if (pEvtCtrl->wOI == TERM_YKCTRLBREAK)
			{	
				TYKCtrl* pEvtPriv = (TYKCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOAD[0] != DT_OAD)	//˽�����ݸ�ֵ�Ƿ�Ϸ�
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOAD, iRet);
			}
			else 
				return -1;
		}	
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcEnumFmt[0])
	{	
		iRet = 2;	
		if (bType)	//��Ҫ����
		{
			if (pEvtCtrl->wOI == TERM_DEVICEERR)
			{	
				TDeviceErr* pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				*pbSrcBuf =  pEvtPriv->bEvtSrcEnum;
			}
			else if (pEvtCtrl->wOI == TERM_CURCIRC)
			{
				TCurCirc* pEvtPriv = (TCurCirc* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				*pbSrcBuf =  pEvtPriv->bEvtSrcEnum;
			}
			else if (pEvtCtrl->wOI == TERM_POWOFF)
			{
				TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;

				DTRACE(DB_FAPROTO, ("GetEvtFieldParser: wOI=%u PowOnoff EvtSrcEnum = %d.\r\n", pEvtCtrl->wOI, pEvtPriv->bEvtSrcEnum));
				*pbSrcBuf =  pEvtPriv->bEvtSrcEnum;
			}
			else 
				return -1;
		}	
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOIFmt[0])
	{		
		iRet = 3;	
		if (bType)	//��Ҫ����
		{		
			if (pEvtCtrl->wOI == TERM_POWCTRLBREAK)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOI[0] != DT_OI)	//˽�����ݸ�ֵ�Ƿ�Ϸ�
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOI, iRet);
			}
			else if (pEvtCtrl->wOI == TERM_ELECTRLBREAK)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOI[0] != DT_OI)	//˽�����ݸ�ֵ�Ƿ�Ϸ�
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOI, iRet);
			}
			else if (pEvtCtrl->wOI == TERM_PURCHPARACHG)
			{	
				TPurchParaChg* pEvtPriv = (TPurchParaChg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOI[0] != DT_OI)	//˽�����ݸ�ֵ�Ƿ�Ϸ�
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOI, iRet);
			}
			else if (pEvtCtrl->wOI == TERM_ELECTRLALARM)
			{	
				TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOI[0] != DT_OI)	//˽�����ݸ�ֵ�Ƿ�Ϸ�
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOI, iRet);
			}
			else 
				return -1;
			
		}
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcTSAFmt[0])
	{	
		iTsaLen = *pbSrcBuf;
		
		if (bType == 0)
			iRet = iTsaLen+2;
		else if (bType == 1)
		{
			if (pEvtCtrl->wOI == TERM_MTRCLKPRG)
			{	
				TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				iRet =  pEvtPriv->bEvtSrcTSA[1] + 2;	//�������ͼ�����
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcTSA, iRet);	//ֻ������Ч���ݲ���
			}
			else 
				return -1;
		}
		else  if (bType == 2)
		{
			if (pEvtCtrl->wOI == TERM_MTRCLKPRG)
			{	
				TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				
				memcpy(bBuf, pbSrcBuf+1+iTsaLen, sizeof(bBuf));//���ݺ�������
				iRet =  pEvtPriv->bEvtSrcTSA[1] + 2;	//�������ͼ�����
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcTSA, iRet);	//ֻ������Ч���ݲ���
				memcpy(pbSrcBuf-1+iRet, bBuf,sizeof(bBuf));	//������������				
			}
			else
				return -1;
		}
		else
			return -1;
	}	
	else
		return -1;
	
	return iRet;	
}
	
//��������ȡ�õ��¼�����Դ��Ӧ�ĵ�ǰֵ��¼��ṹ
//������@ pEvtCtrl �¼�����
//���أ���ȷ�����¼���ǰֵ��¼��ṹ�ĳ��ȣ����򷵻�-1
int  EvtSrctoCurRecList(TEvtCtrl* pEvtCtrl, BYTE* bCurRecList)
{
	if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcNullFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcNullCurRecList, sizeof(g_bSrcNullCurRecList));
		return sizeof(g_bSrcNullCurRecList);
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOADFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcOADCurRecList, sizeof(g_bSrcOADCurRecList));
		return sizeof(g_bSrcOADCurRecList);
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcEnumFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcEnumCurRecList, sizeof(g_bSrcEnumCurRecList));
		return sizeof(g_bSrcEnumCurRecList);
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOIFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcOICurRecList, sizeof(g_bSrcOICurRecList));
		return sizeof(g_bSrcOICurRecList);
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcTSAFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcTSACurRecList, sizeof(g_bSrcTSACurRecList));
		return sizeof(g_bSrcTSACurRecList);
	}
	else
	{	
		bCurRecList = NULL;
		return -1;
	}
}

//����������OAD��ȡ����/����/Ԫ��
//������@dwOAD ���ݱ�ʶ
//		@pwOI ���صĶ����ʶ
//		@pbAttr ���ص�����(��ȥ������������)
//		@pbIndex ���ص�������Ԫ������
//���أ��޷���
void GetOIAttrIndex(DWORD dwOAD, WORD* pwOI, BYTE* pbAttr, BYTE* pbIndex)
{
	if (pwOI != NULL) *pwOI = (WORD)(dwOAD>>16);
	if (pbAttr != NULL) *pbAttr = (BYTE)((dwOAD&0x00001f00)>>8);
	if (pbIndex != NULL) *pbIndex = (BYTE)(dwOAD&0x000000ff);	
}

//��������ȡ�¼��̶��ֶ�/�����ֶ�
//������@pEvtCtrl �¼�����
//		@pFixFields ���صĹ̶��ֶ�
//		@pDataFields ���ص������ֶ�
//		@pbAtrrTabBuf �������Ա�����
//		@wBufSize pbDataCfg�������Ĵ�С
//���أ���ȷ��ȡ���̶��ֶ�/�����ֶη���true�����򷵻�false
bool GetEvtFieldParser(struct TEvtCtrl* pEvtCtrl, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize)
{
	BYTE bFixListFmt[] = {DT_ARRAY, FIXLIST_ELEMENT_MAXNUM, DT_OAD};
	BYTE* pbFmt;
	WORD wOI, wFmtLen = 0;
	int iLen;
	const TEvtAttr* pEvtAttr; 
	
	wOI = pEvtCtrl->wOI;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;
	
	// �̶��ֶ�
	if (pFixFields != NULL)
	{
		pFixFields->pbCfg = pEvtCtrl->pbFixField;
		pFixFields->wCfgLen = pEvtCtrl->wFixFieldLen;
		if (OoParseField(pFixFields, bFixListFmt, sizeof(bFixListFmt), true) == false)	//���й̶��ֶ�
		{	
			//DTRACE(DB_INMTR, ("GetEvtFieldParser: wOI=%u OoParseField() pFixFields fail.\r\n", wOI));
			return false;
		}	
	}

	//�����ֶΣ��������Ա����ΪNULL
	if (pDataFields != NULL)
	{
		if (wBufSize < EVT_ATTRTAB_LEN)
			return false;
		memset(pbAtrrTabBuf, 0x00, wBufSize);
		pDataFields->pbCfg = pbAtrrTabBuf;
		iLen= OoReadAttr(wOI, pEvtAttr->bRela, pDataFields->pbCfg, &pbFmt, &wFmtLen);		
		if (iLen > 0)
		{
			pDataFields->wCfgLen = iLen;
			if (OoParseField(pDataFields, pbFmt, wFmtLen, true) == false)
			{	
				//DTRACE(DB_INMTR, ("GetEvtFieldParser: wOI=%u OoParseField() pDataFields fail.\r\n", wOI));
				return false;
			}
		}
		else
		{	
			memset(pbAtrrTabBuf, 0x00, wBufSize);
			pDataFields->pbCfg = 0;
			pDataFields->wNum= 0;
		}
	}
	
	return true;
}

struct TEvtCtrl* GetEvtCtrl(DWORD dwOAD)
{
	WORD i, wOI, wNum;
	wOI = dwOAD>>16;
	wNum = sizeof(g_EvtCtrl)/sizeof(TEvtCtrl);
	struct TEvtCtrl* pEvtCtrl = NULL;

	for (i=0; i<wNum; i++)
	{
		if (wOI == g_EvtCtrl[i].wOI)
		{
			pEvtCtrl = &g_EvtCtrl[i];
			break;
		}
	}

	if (i==wNum && wOI>0x3008 && wOI<=0x3030)
	{
		pEvtCtrl = &g_EvtCtrl[12];
	}

	return pEvtCtrl;
}

//��������ȡ�¼�ROAD����OAD�����ó���
//������@dwOAD �¼�OAD
//���أ����ڷ���ʵ�ʳ��ȣ����򷵻�0
DWORD GetEvtMainOadDataLen(DWORD dwOAD)
{
	BYTE bBuf[EVT_ATTRTAB_LEN];
	WORD wOI;
	BYTE bAttr, bIndex;
	DWORD dwDataLen = 0;
	const TEvtAttr* pEvtAttr; 
	TFieldParser tFixFields;
	TFieldParser tDataFields;

	struct TEvtCtrl* pEvtCtrl = GetEvtCtrl(dwOAD);
	if (pEvtCtrl == NULL)
		return 0;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));

	//��ȡ�¼�����
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return 0;

	GetOIAttrIndex(dwOAD, &wOI, &bAttr, &bIndex);
	if ((pEvtCtrl->bClass==7 && bAttr!=ATTR2) || (pEvtCtrl->bClass==24 && bAttr!=ATTR6 && bAttr!=ATTR7 && bAttr!=ATTR8 && bAttr!=ATTR9))
		return 0;

	//��ȡ�̶��ֶκ������ֶ�
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		return 0;
	}

	//if (pEvtCtrl->pbSrcFmt != NULL)
	//{
	//	if (tFixFields.wTotalLen >= EVT_SRC_LEN)
	//	{
	//		dwDataLen = tFixFields.wTotalLen - EVT_SRC_LEN;
	//		switch (pEvtCtrl->pbSrcFmt[0])
	//		{
	//		case DT_NULL:
	//			dwDataLen += 3;
	//			break;
	//		case DT_ENUM:
	//			dwDataLen += 4;
	//			break;
	//		case DT_OI:
	//			dwDataLen += 5;
	//			break;
	//		case DT_OAD:
	//			dwDataLen += 7;
	//			break;
	//		case DT_TSA:
	//			dwDataLen += (EVT_SRC_LEN+2);
	//			break;
	//		}
	//	}
	//	else
	//		dwDataLen = tFixFields.wTotalLen;
	//}

	dwDataLen = 2+5; //�¼�ROAD�Ĵ������� +2�ֽڼ�¼����ʵ�ʳ���
	return dwDataLen;
}

//�������Ƿ�Ϊ�¼�����ǰ/�¼�����ǰ��OAD
//������@dwOAD���ݱ�ʶ
//���أ�true/false
//ע������OAD��Ҫ����ȫ�ֿռ�
bool IsEvtBeforeOAD(DWORD dwOAD)
{
	BYTE bFeat;
	bFeat = (BYTE)((dwOAD&~OAD_FEAT_MASK) >> OAD_FEAT_BIT_OFFSET);

	if ((bFeat!=EVT_S_BF_HP) && (bFeat!=EVT_S_AFT_HP) && (bFeat!=EVT_S_BF_END)  &&  (bFeat!=EVT_S_AFT_END))
		bFeat = EVT_S_AFT_HP;

	if ((bFeat==EVT_S_BF_HP) ||(bFeat==EVT_S_BF_END))
		return true;
	else
		return false;
}

//��������OAD��ǰ�Ƿ���Ҫ�ɼ�����
//������@dwOAD���ݱ�ʶ
//		@bState��ǰ�¼�״̬
//���أ�true/false
bool IsOADNeedAcqData(DWORD dwOAD, BYTE bState)
{
	BYTE bFeat;
	bFeat = (BYTE)((dwOAD&~OAD_FEAT_MASK) >> OAD_FEAT_BIT_OFFSET);

	if ((bFeat!=EVT_S_BF_HP) && (bFeat!=EVT_S_AFT_HP) && (bFeat!=EVT_S_BF_END)  &&  (bFeat!=EVT_S_AFT_END))
		bFeat = EVT_S_AFT_HP;

	if ((bState==bFeat) ||((bState!=EVT_S_BF_HP)&&(bFeat==EVT_S_BF_END)))	//״̬һ�£������¼������������ǰEVT_S_BF_END		
		return true;
	else
		return false;	
}

//��������OAD��ǰ�Ƿ���Ҫ���浽�������¼���¼��
//������@dwOAD���ݱ�ʶ
//		@bState��ǰ�¼�״̬
//���أ�true/false
bool IsOADNeedSaveData(DWORD dwOAD, BYTE bState)
{
	BYTE bFeat;
	bFeat = (BYTE)((dwOAD&~OAD_FEAT_MASK) >> OAD_FEAT_BIT_OFFSET);

	if ((bFeat!=EVT_S_BF_HP) && (bFeat!=EVT_S_AFT_HP) && (bFeat!=EVT_S_BF_END)  &&  (bFeat!=EVT_S_AFT_END))
		bFeat = EVT_S_AFT_HP;

	//�¼������󣬴洢����ǰ�ͷ����������
	//�¼������󣬴洢����ǰ�ͽ����������
	if (((bState==EVT_S_AFT_HP) && ((bFeat==EVT_S_BF_HP)||(bFeat==EVT_S_AFT_HP)))
		|| ((bState==EVT_S_AFT_END) && ((bFeat==EVT_S_BF_END)||(bFeat==EVT_S_AFT_END))))		
		return true;
	else
		return false;	
}

//��������ʼ����ʱ�ռ䡣�������Ա����¼�����ǰ/�¼�����ǰ��OAD��Ҫ������ʱȫ�ֱ����ռ�
//������@pEvtCtrl �¼�����
//		@pDataFields �����ֶ�
//���أ�������ȷ����true�����򷵻�false
bool InitTmpMem(struct TEvtCtrl* pEvtCtrl, TFieldParser* pDataFields)
{
	BYTE bAttrTab, bOadNum, bIndex, bType, bItem, bOadBuf[10], bBuf[SYSDB_ITEM_MAXSIZE];
	WORD wItemOffset, wItemLen, wDataLen, wTotalLen;	
	DWORD dwOAD, dwROAD;
	int iLen;
	const TEvtAttr* pEvtAttr; 

	//�������ֶΣ�����Ҫ������ʱ�ռ�
	if ((pDataFields==NULL) || (pDataFields->wCfgLen==0) || (pDataFields->wNum==0))
		return true;

	//��ȡ�¼�������
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;
	bAttrTab = pEvtAttr->bRecTabStart;

	//������ʱ�ռ�
	bOadNum = 0;	//��ʱ�ռ�OAD����
	wDataLen = 0;	//��ʱ�ռ����ݳ���
	wTotalLen = 0;	//��ʱ�ռ������ܳ���
	memset(bBuf, 0, sizeof(bBuf));
	for (bIndex=0; bIndex<pDataFields->wNum; bIndex++)
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(pDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			//DTRACE(DB_INMTR, ("InitTmpMem: ReadParserField() fail.\r\n"));
			return false;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		if (IsEvtBeforeOAD(dwOAD))
		{
			OoDWordToOad(dwOAD, &bBuf[1+bOadNum*5]);
			bBuf[5+bOadNum*5] = (BYTE)wItemLen;	//Ŀǰ��������󳤶�С��255
			wDataLen += wItemLen;
			bOadNum++;	//�����Լ�
			bBuf[0] = bOadNum;
		}
	}
	if (wDataLen)
		wTotalLen = wDataLen+1+bOadNum*5;//����+����*��OAD+LEN��+����
	else
		return true;	//����Ҫ������ʱ�ռ�

	//������ʱ�ռ�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		dwROAD = GetOAD(pEvtCtrl->wOI, bAttrTab, 0);
		
		//��ʼ��ʱ��Ҫ��֤��ֵ��ȷ
		if (pEvtCtrl->pEvtBase[bItem].bMemType != MEM_TYPE_NONE)
			return false;

		if (wTotalLen > 64)
		{
			pEvtCtrl->pEvtBase[bItem].bMemType = MEM_TYPE_TERM_EVTREC;
			if (EvtAllocRecMem(dwROAD, &g_TermMem, pDataFields->wTotalLen) == false)
				return false;
		}
		else if (wTotalLen != 0)
		{
			pEvtCtrl->pEvtBase[bItem].bMemType = MEM_TYPE_TERM_EVTITEM;
			if (EvtAllocItemMem(dwROAD, &g_TermMem, wTotalLen))
			{
				if (EvtWriteItemMem(dwROAD, &g_TermMem, bBuf) <= 0)
					return false;
			}
			else
				return false;
		}
		else
			return false;

		//��һ����
		bAttrTab++;
	}
	return true;
}

//������ͨ���¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bAttrTab, bItem, bBuf[EVT_ATTRTAB_LEN];
	WORD wOI, wMaxNum; 
	DWORD  dwROAD;
	int iLen;
	const TEvtAttr* pEvtAttr; 
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	const ToaMap* pOaMap = NULL;
	
	//DTRACE(DB_INMTR, ("InitEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));

	//ȫ�ֱ�����ʼ��
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)	
		memset((BYTE*)&pEvtCtrl->pEvtBase[bItem], 0x00, sizeof(TEvtBase));
	pEvtCtrl->dwLastClick = 0;
	pEvtCtrl->dwNewClick = 0;

	//��ȡ�¼�����
	wOI = pEvtCtrl->wOI;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;
	bAttrTab = pEvtAttr->bRecTabStart;

	SetTermEvtOadDefCfg(pEvtCtrl);

	//��ȡ����¼��
	iLen = OoReadAttr(wOI, pEvtAttr->bMaxRecNum, bBuf, NULL, NULL);
	if (iLen <= 0)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u Init fail because Read wMaxNum fail.\r\n", wOI));
		return false;
	}
	wMaxNum = OoLongUnsignedToWord(bBuf+1);	
	DTRACE(DB_INMTR, ("InitEvt: wOI=%u & wMaxNum=%u.\r\n", wOI, wMaxNum));
	if (wMaxNum == 0)	//����¼��Ϊ0����ʼ��ʧ��
		return false;

	//��ȡ�̶��ֶκ������ֶ�
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return false;
	}

	//�������ֶ���������/��ʱ�ռ�
	if (InitTmpMem(pEvtCtrl, &tDataFields) == false)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u InitTmpMem() fail.\r\n", wOI));
		return false;
	}

	//����
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		dwROAD = GetOAD(wOI, bAttrTab, 0);
		pOaMap = GetOIMap(dwROAD);
		iLen = CreateTable(pOaMap->pszTableName, &tFixFields, &tDataFields, (DWORD)wMaxNum);
		if (iLen <= 0)	
		{	
			DTRACE(DB_INMTR, ("InitEvt: wOI=%u bItem=%d CreateTable fail.\r\n", wOI, bItem));
			return false;
		}
		//�ó�ʼ����ʶ
		pEvtCtrl->pEvtBase[bItem].fInitOk = true;
		pEvtCtrl->pEvtBase[bItem].bState= EVT_S_BF_HP;
		//��һ����
		bAttrTab++;
	}

	//��ʼ����ǿ��ˢ������
	UpdateRecMem(pEvtCtrl, 1);
	UpdateItemMem(pEvtCtrl, 1);
	
	DTRACE(DB_INMTR, ("InitEvt: wOI=%u Init sucess.\r\n", wOI));
	return true;
}

//������ʧѹ�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitVLoss(struct TEvtCtrl* pEvtCtrl)
{
	TVLoss* pEvtPriv = (TVLoss* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0x00, sizeof(TVLoss));
	return InitEvt(pEvtCtrl);
}

//�����������¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitDmd(struct TEvtCtrl* pEvtCtrl)
{
	TDmd* pEvtPriv = (TDmd* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0x00, sizeof(TDmd));	
	return InitEvt(pEvtCtrl);
}

//������ȫʧѹ�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitAVLoss(struct TEvtCtrl* pEvtCtrl)
{
	TAllVLoss* pEvtPriv = (TAllVLoss* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	if (InitEvt(pEvtCtrl) == false)
		return false;
	if (pEvtPriv->tEvtBase.fInitOk)
		memcpy((BYTE*)&pEvtCtrl->pEvtBase[0]+1, (BYTE*)pEvtPriv+1, sizeof(TAllVLoss)-1);	//Ϊ��������������ĳ�ʼ����ʶ
	return true;
}

//�������¼������ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitEvtClr(struct TEvtCtrl* pEvtCtrl)
{
	TEvtClr* pEvtPriv = (TEvtClr* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0x00, sizeof(TEvtClr));
	return InitEvt(pEvtCtrl);
}

//�������¼��ӿں�����ʼ���������̵߳���
//��������
//���أ���
void InitTermEvt()
{
	for(BYTE i=0; i<EVT_NUM; i++)
		g_EvtCtrl[i].pfnInitEvt(&g_EvtCtrl[i]);
}

//������ʧѹ�¼��жϺ���
//�����๩��ϵͳ�У�ĳ����������趨��ʧѹ�¼������������ޣ�
//ͬʱ�����ѹ�����趨��ʧѹ�¼���ѹ�������ޣ�
//�ҳ���ʱ������趨��ʧѹ�¼��ж���ʱʱ�䣬���й�����Ϊ����ʧѹ��
//ע1: ������������£����ж�B��ʧѹ��
//ע2: ȫʧѹ����ʱ������ʧѹ�¼���¼������
//ע3: ��"ʧѹ�¼���������"�趨Ϊ"0"ʱ����ʾ�����á�
//@����5�����ò�������=structure
//{
//	��ѹ��������  long-unsigned����λ��V�����㣺-1����
//	��ѹ�ָ�����  long-unsigned����λ��V�����㣺-1����
//	������������  double-long����λ��A�����㣺-4����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int VLossJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVUp;			
		WORD wVDown;			
		int iIDown;			
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3];	//��ǰ��ѹ		
	int iIval[3];		//��ǰ����			
	TTermEvtCtrl* pAllVLossEvtCtrl;
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_VLOSS)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VLossJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��ȡȫʧѹ�¼����ƽṹ
	pAllVLossEvtCtrl = GetTermEvtCtrl(MTR_ALLVLOSS);
	if (pAllVLossEvtCtrl == NULL)
		return 0;	
	if  (pAllVLossEvtCtrl->pEvtBase[0].fExcValid)
	{
		DTRACE(DB_INMTR, ("VLossJudge: AllVLoss is valid.\r\n"));
		return 0;
	}

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VLossJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VLossJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVUp = OoLongUnsignedToWord(bBuf+3);
		tPara.wVDown = OoLongUnsignedToWord(bBuf+6);
		tPara.iIDown = OoDoubleLongToInt(bBuf+9);
		tPara.bDelaySec = *(bBuf+14);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.wVUp == 0)
	{	
		DTRACE(DB_INMTR, ("VLossJudge: para wVUp=%u.\r\n", tPara.wVUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 �����������ͣ�double-long����λ��A���㣺-3��A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//С��λ�������¼����ò���һ�£���4λС�� XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
#if 0	//����ʹ��								
		if ((g_TestFlag == 1)&&(bItem == 0))	
		{	if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
			else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		}	
		else if ((g_TestFlag == 2)&&(bItem == 1))	
		{	if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
			else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		}	
		else if ((g_TestFlag == 3)&&(bItem == 2))	
		{	if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
			else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		}
		else
		{	if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}
#else
		if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
#endif	
		//DTRACE(DB_INMTR, ("************************VLossJudge: wVol=%u, tPara.wVUp=%u, tPara.wVDown=%u,iIval=%u, tPara.iIDown=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVUp, tPara.wVDown, iIval[bItem], tPara.iIDown, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	

	}	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//������Ƿѹ�¼��жϺ���
//������(����)����ϵͳ�У�ĳ���ѹС���趨��Ƿѹ�¼���ѹ�������ޣ�
//�ҳ���ʱ������趨��Ƿѹ�¼��ж���ʱʱ����ֹ�����ΪǷѹ��
//ע:��"Ƿѹ�¼���ѹ��������"�趨Ϊ"0"ʱ����ʾ"Ƿѹ�¼�"�����á�
//ע:������������£����ж�B�ࡣ
//����5�����ò�������=structure
//{
//	��ѹ��������  long-unsigned����λ��V�����㣺-1����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int VLessJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVUp;			
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3];	//��ǰ��ѹ		
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_VLESS)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VLessJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VLessJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VLessJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVUp = OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.wVUp == 0)
	{	
		DTRACE(DB_INMTR, ("VLessJudge: para wVUp=%u.\r\n", tPara.wVUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
#if 0	//����ʹ��								
		if ((g_TestFlag == 1)&&(bItem == 0))	
		{	
			if (wVol[bItem]<tPara.wVUp)		
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}	
		else if ((g_TestFlag == 2)&&(bItem == 1))	
		{	
			if (wVol[bItem]<tPara.wVUp)		
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}	
		else if ((g_TestFlag == 3)&&(bItem == 2))	
		{	
			if (wVol[bItem]<tPara.wVUp)		
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}
		else
		{
			if (wVol[bItem]<tPara.wVUp)		
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		}
#else
		if (wVol[bItem]<tPara.wVUp)		
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
#endif	
		
		//DTRACE(DB_INMTR, ("************************VLessJudge: wVol=%u, tPara.wVUp=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVUp, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}


//��������ѹ�¼��жϺ���
//������(����)����ϵͳ�У�ĳ���ѹ�����趨�Ĺ�ѹ�¼���ѹ�������ޣ�
//�ҳ���ʱ������趨�Ĺ�ѹ�¼��ж���ʱʱ����ֹ�����Ϊ��ѹ��
//ע:��"��ѹ�¼���ѹ��������"�趨Ϊ"0"ʱ����ʾ"��ѹ�¼�"�����á�
//ע:������������£����ж�B�ࡣ
//����5�����ò�������=structure
//{
//	��ѹ��������  long-unsigned����λ��V�����㣺-1����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int VOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVDown;			
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3];	//��ǰ��ѹ		
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_VOVER)	
		return -1;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVDown = OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.wVDown == 0)
	{	
		DTRACE(DB_INMTR, ("VOverJudge: para wVDown=%u.\r\n", tPara.wVDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;		
		if (wVol[bItem]>tPara.wVDown)		
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		//DTRACE(DB_INMTR, ("************************VOverJudge: wVol=%u, tPara.wVDown=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVDown, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//�����������¼��жϺ���
//�����๩��ϵͳ�У���ĳ���ѹ�����趨�Ķ����¼���ѹ�������ޣ�
//ͬʱ�������С���趨�Ķ����¼������������ޣ�
//�ҳ���ʱ������趨�Ķ����¼��ж���ʱʱ�䣬���� ������Ϊ���ࡣ
//ע1:������������£�������B�ࡣ
//ע2:��"�����¼���ѹ��������"�趨Ϊ"0"ʱ����ʾ�����á�
//@����5�����ò�������=structure
//{
//	��ѹ��������  long-unsigned����λ��V�����㣺-1����
//	������������  double-long����λ��A�����㣺-4����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int VBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVUp;		
		int iIUp;			
		BYTE bDelaySec;	
	}tPara;
	WORD wVol[3];	//��ǰ��ѹ		
	int iIval[3];		//��ǰ����			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_VBREAK)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VBreakJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VBreakJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VBreakJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVUp = OoLongUnsignedToWord(bBuf+3);
		tPara.iIUp= OoDoubleLongToInt(bBuf+6);
		tPara.bDelaySec = *(bBuf+11);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.wVUp == 0)
	{	
		DTRACE(DB_INMTR, ("VBreakJudge: para wVUp=%u.\r\n", tPara.wVUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 �����������ͣ�double-long����λ��A���㣺-3��A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//С��λ�������¼����ò���һ�£���4λС�� XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;		
		if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])<labs(tPara.iIUp)))			
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		//DTRACE(DB_INMTR, ("************************VBreakJudge: wVol=%u, tPara.wVUp=%u, iIval=%u, tPara.iIUp=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVUp, iIval[bItem], tPara.iIUp, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}


//������ʧ���¼��жϺ���
//�����๩��ϵͳ�У�������������һ�ฺ�ɵ�������ʧ���¼������������ޣ�
//ĳ���ѹ�����趨��ʧ���¼���ѹ�������ޣ�ͬʱ�������С���趨��ʧ���¼�������������ʱ��
//�ҳ���ʱ������趨��ʧ���¼��ж���ʱʱ�䣬���ֹ�����Ϊ����ʧ����
//ע:��"ʧ���¼�������������"�趨Ϊ"0"ʱ���������á�
//ע:������������£����ж�B�ࡣ
//@����5�����ò�������=structure
//{
//	��ѹ��������  long-unsigned����λ��V�����㣺-1����
//	������������  double-long����λ��A�����㣺-4����
//	�����ָ�����  double-long����λ��A�����㣺-4����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int ILossJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVDowm;		
		int iIUp;				
		int iIDown;		
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3];	//��ǰ��ѹ		
	int iIval[3];		//��ǰ����			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_ILOSS)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("ILossJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("ILossJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("ILossJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVDowm = OoLongUnsignedToWord(bBuf+3);
		tPara.iIUp = OoDoubleLongToInt(bBuf+6);
		tPara.iIDown = OoDoubleLongToInt(bBuf+11);
		tPara.bDelaySec = *(bBuf+15);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iIUp == 0)
	{	
		DTRACE(DB_INMTR, ("ILossJudge: para iIUp=%u.\r\n", tPara.iIUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 �����������ͣ�double-long����λ��A���㣺-3��A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//С��λ�������¼����ò���һ�£���4λС�� XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if (bPhase==2)
		{
			if (((labs(iIval[0])>labs(tPara.iIDown))||(labs(iIval[2])>labs(tPara.iIDown))) 
				&& (wVol[bItem]>tPara.wVDowm) && (labs(iIval[bItem])<labs(tPara.iIUp)))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}
		else if (bPhase==3)
		{
			if (((labs(iIval[0])>labs(tPara.iIDown))||(labs(iIval[1])>labs(tPara.iIDown))||(labs(iIval[2])>labs(tPara.iIDown))) 
				&& (wVol[bItem]>tPara.wVDowm) && (labs(iIval[bItem])<labs(tPara.iIUp)))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//�����������¼��жϺ���
//������(����)����ϵͳ�У�ĳ�ฺ�ɵ��������趨�Ĺ����¼������������ޣ�
//�ҳ���ʱ������趨�Ĺ����¼��ж���ʱʱ�䣬���ֹ�����Ϊ������
//ע: ��"����������������"�趨Ϊ"0"ʱ���������á�
//ע:������������£����ж�B�ࡣ
//����5�����ò�������=structure
//{
//	������������  double-long����λ��A�����㣺-4����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int IOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int iDown;		
		BYTE bDelaySec;		
	}tPara;
	int iIval[3];		//��ǰ����			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_IOVER)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("IOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("IOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("IOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iDown= OoDoubleLongToInt(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iDown == 0)
	{	
		DTRACE(DB_INMTR, ("IOverJudge: para iDown=%u.\r\n", tPara.iDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//2001 3 �����������ͣ�double-long����λ��A���㣺-3��A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//С��λ�������¼����ò���һ�£���4λС�� XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if (labs(iIval[bItem])>labs(tPara.iDown))	
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		//DTRACE(DB_INMTR, ("************************IOverJudge: iIval=%u,  iDown=%u, bItem=%d, bJudgeState=%d.\r\n",iIval[bItem],  tPara.iDown, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//�����������¼��жϺ���
//������(����)����ϵͳ�У�ĳ���ѹ���ڶ����¼��������ޣ�
//ͬʱ�����С���趨�Ķ����¼������������ޣ�
//�ҳ���ʱ������趨�Ķ����¼��ж���ʱʱ�䣬���ֹ�����Ϊ������
//ע:��"�����¼�������������"�趨Ϊ"0"ʱ����ʾ�����á�
//ע:������������£����ж�B�ࡣ
//@����5�����ò�������=structure
//{
//	��ѹ��������  long-unsigned����λ��V�����㣺-1����
//	������������  double-long����λ��A�����㣺-4����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int IBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVDowm;		
		int iIUp;				
		BYTE bDelaySec;	
	}tPara;
	WORD wVol[3];	//��ǰ��ѹ		
	int iIval[3];		//��ǰ����			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_IBREAK)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("IBreakJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("IBreakJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("IBreakJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVDowm = OoLongUnsignedToWord(bBuf+3);
		tPara.iIUp = OoDoubleLongToInt(bBuf+6);
		tPara.bDelaySec = *(bBuf+11);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iIUp == 0)
	{	
		DTRACE(DB_INMTR, ("IBreakJudge: para iIUp=%u.\r\n", tPara.iIUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 �����������ͣ�double-long����λ��A���㣺-3��A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//С��λ�������¼����ò���һ�£���4λС�� XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if ((wVol[bItem]>tPara.wVDowm) && (labs(iIval[bItem])<labs(tPara.iIUp)))		
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		//DTRACE(DB_INMTR, ("************************IBreakJudge: wVol=%u, tPara.wVDowm=%u, iIval=%u, tPara.iIUp=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVDowm, iIval[bItem], tPara.iIUp, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//���������������¼��жϺ���
//�����๩��ϵͳ�У������й����ʷ���ı䷽��ʱ��ͬʱ�й����ʴ���
//�趨�ĳ��������¼��й����ʴ������ޣ��ҳ���ʱ������趨�ĳ���������ʱʱ�䣬
//����ʽ����Ϊ��������
//ע:��'���������¼��й����ʴ�������'����Ϊ0ʱ����ʾ�����á�
//ע:������������£����ж�B�ࡣ
//@����5�����ò�������=structure
//{
//	�й����ʴ�������  double-long����λ��W�����㣺-1����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int PReverseJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int iPowDown;		
		BYTE bDelaySec;		
	}tPara;
	int iPow[4];	//��ǰ����			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_PREVERSE)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("PReverseJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("PReverseJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("PReverseJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iPowDown= OoDoubleLongToInt(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iPowDown == 0)
	{	
		DTRACE(DB_INMTR, ("PReverseJudge: para iPowDown=%u.\r\n", tPara.iPowDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//2004 4 �й����� �������ͣ�double-long����λ��W�����㣺-1���ܡ�A�ࡢB�ࡢC��
	if (OoReadAttr(0x2004, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<4; i++)
			iPow[i] = OoDoubleLongToInt(bBuf+3+i*5);
	}	
	else
		memset((BYTE*)&iPow, 0x00, sizeof(iPow));

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if ((iPow[bItem+1]<0) && (labs(iPow[bItem+1])>labs(tPara.iPowDown)))	
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//�����������¼��жϺ���
//������(����)����ϵͳ�У�ĳ�๦�ʴ����趨�Ĺ����¼��й����ʴ������ޣ�
//�ҳ���ʱ������趨�Ĺ����¼��ж���ʱʱ�䣬���ֹ�����Ϊ���ء�
//ע:��'�����¼��й����ʴ�������'����Ϊ0ʱ����ʾ�����á�
//ע:������������£����ж�B�ࡣ
//@����5�����ò�������=structure
//{
//	�й����ʴ�������  double-long����λ��W�����㣺-1����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int POverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int iPowDown;		
		BYTE bDelaySec;		
	}tPara;
	int iPow[4];	//��ǰ����			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_POVER)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("POverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("POverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("POverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iPowDown= OoDoubleLongToInt(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iPowDown == 0)
	{	
		DTRACE(DB_INMTR, ("POverJudge: para iPowDown=%u.\r\n", tPara.iPowDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//2004 4 �й����� �������ͣ�double-long����λ��W�����㣺-1���ܡ�A�ࡢB�ࡢC��
	if (OoReadAttr(0x2004, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<4; i++)
			iPow[i] = OoDoubleLongToInt(bBuf+3+i*5);
	}	
	else
		memset((BYTE*)&iPow, 0x00, sizeof(iPow));

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if (labs(iPow[bItem+1]) > labs(tPara.iPowDown))	
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}


//���������������жϺ���
//������(����)����ϵͳ�У������й����������趨���й����������¼��������ޣ�
//�Ҽ���ʱ������趨�����������¼��ж���ʱʱ�䣬���ֹ�����Ϊ�й�����Խ�ޡ�
//����������ֵΪ0ʱ��ʾ�����á�
//@����6�����ò�������=structure
//{
//	��������  double-long-unsigned����λ��kW�����㣺-4����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int PDmdOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[60];	
	struct
	{	
		DWORD dwDown;		
		BYTE bDelaySec;			
	}tPara;
	DWORD dwDmd;	//��ǰ�����й�������	
	
	if (pEvtCtrl->wOI != MTR_PDMDOVER)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("PDmdOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("PDmdOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("PDmdOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.dwDown= OoDoubleLongUnsignedToDWord(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.dwDown == 0)
	{	
		DTRACE(DB_INMTR, ("PDmdOverJudge: para dwDown=%u.\r\n", tPara.dwDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//��ǰ�����й�����Ҫ
	if (ReadItemEx(BN2, PN0, 0x3010, bBuf) > 0)
		dwDmd = OoDoubleLongUnsignedToDWord(bBuf+1);
	else
		dwDmd = 0;

	//״̬�ж�
	if (dwDmd > tPara.dwDown)
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	else
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;

	DTRACE(DB_INMTR, ("************************PDmdOverJudge: dwDmd=%x,  dwDown=%x, bJudgeState=%d.\r\n",dwDmd, tPara.dwDown, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//���������������жϺ���
//������(����)����ϵͳ�У������й����������趨���й����������¼��������ޣ�
//�Ҽ���ʱ������趨�����������¼��ж���ʱʱ�䣬���ֹ�����Ϊ�й�����Խ�ޡ�
//����������ֵΪ0ʱ��ʾ�����á�
//@����6�����ò�������=structure
//{
//	��������  double-long-unsigned����λ��kW�����㣺-4����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int RPDmdOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[60];	
	struct
	{	
		DWORD dwDown;		
		BYTE bDelaySec;			
	}tPara;
	DWORD dwDmd;	//��ǰ�����й�������	
	
	if (pEvtCtrl->wOI != MTR_RPDMDOVER)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("RPDmdOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("RPDmdOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("RPDmdOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.dwDown= OoDoubleLongUnsignedToDWord(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.dwDown == 0)
	{	
		DTRACE(DB_INMTR, ("RPDmdOverJudge: para dwDown=%u.\r\n", tPara.dwDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//��ǰ�����й�����Ҫ
	if (ReadItemEx(BN2, PN0, 0x3020, bBuf) > 0)
		dwDmd = OoDoubleLongUnsignedToDWord(bBuf+1);
	else
		dwDmd = 0;

	//״̬�ж�
	if (dwDmd > tPara.dwDown)
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	else
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;

	DTRACE(DB_INMTR, ("************************RPDmdOverJudge: dwDmd=%x,  dwDown=%x, bJudgeState=%d.\r\n",dwDmd, tPara.dwDown, pEvtCtrl->pEvtBase[0].bJudgeState));	

	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//���������������жϺ���
//������(����)����ϵͳ�У������޹����������趨���޹����������¼��������ޣ�
//�ҳ���ʱ������趨�����������¼��ж���ʱʱ�䣬���ֹ�����Ϊ�й�����Խ�ޡ�
//����������ֵΪ0ʱ��ʾ������?
//@����6�����ò�������=structure
//{
//	��������  double-long-unsigned����λ��kW�����㣺-4����
//	�ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int QDmdOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[180];	
	struct
	{	
		DWORD dwDown;		
		BYTE bDelaySec;			
	}tPara;
	DWORD dwDmd[4];	//��ǰ����		
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_QDMDOVER)	
		return -1;
		
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("QDmdOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("QDmdOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("QDmdOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.dwDown= OoDoubleLongUnsignedToDWord(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.dwDown == 0)
	{	
		DTRACE(DB_INMTR, ("QDmdOverJudge: para dwDown=%u.\r\n", tPara.dwDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//��ǰ�޹�����
	for(i=0; i<4; i++)
	{
		if (ReadItemEx(BN2, PN0, (0x3030+i), bBuf) > 0)
			dwDmd[i] = OoDoubleLongUnsignedToDWord(bBuf+1);
		else
			dwDmd[i] = 0;
	}

	//״̬�ж�
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)	
	{	
		if (dwDmd[bItem] > tPara.dwDown)
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;

		DTRACE(DB_INMTR, ("************************QDmdOverJudge: dwDmd[%d]=%x,  dwDown=%x, bJudgeState=%d.\r\n",bItem, dwDmd[bItem], tPara.dwDown, pEvtCtrl->pEvtBase[0].bJudgeState));
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//�������������� �������жϺ���
//�����๩��ϵͳ�У����ܹ�������С���趨�Ĺ������������޷�ֵ��
//ͬʱ����һ���������5%����������
//�ҳ���ʱ������趨�Ĺ��������������ж�ʱ�䣬���ֹ�����Ϊ�ܹ������������ޡ�
//ע:��"�������������޷�ֵ"�趨Ϊ"0"ʱ����ʾ"�ܹ��������������¼�������"
//�����Ҫ���ܹ������������ޡ�
//@����6�����ò�������=structure
//{
//  ���޷�ֵ  long����λ��%�����㣺-1����
//  �ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int PfUnderJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int16 iDown;			//XXX.X (��30%��Ӧ300)
		BYTE bDelaySec;			
	}tPara;
	int16 iPf;
	int iIval[3];	
	DWORD dwIb;	//��������
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_PFUNDER)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("PfUnderJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("PfUnderJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("PfUnderJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iDown= OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iDown == 0)
	{	
		DTRACE(DB_INMTR, ("PfUnderJudge: para iDown=%u.\r\n", tPara.iDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//2001 3 ���� �������ͣ�double-long����λ��A���㣺-3��A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5);	
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	
	
	//��������
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	dwIb= BcdToDWORD(bBuf, 3)*0.05;	//NNN.NNNA, 5%Ib
	
	// 200A 4 �������� �������ͣ�long����λ���ޣ����㣺-3�� �ܡ�A�ࡢB�ࡢC�ֻ࣬��ȡ��
	if (OoReadAttr(0x200A, 0x02, bBuf, NULL, NULL) > 0)
		iPf = OoLongToInt16(bBuf+3);
	else
		iPf = -1000;		

	//״̬�ж�
	if (bPhase == 2)
	{
		if ((iPf<tPara.iDown) && ((labs(iIval[0])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	else if(bPhase == 3)
	{
		if ((iPf<tPara.iDown) && ((labs(iIval[0])>dwIb)||(labs(iIval[1])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	//DTRACE(DB_INMTR, ("************************PfUnderJudge: iPf=%04x, tPara.iDown=%04x, iIval=%u-%u-%u,  dwIb=%u, bJudgeState=%d.\r\n",iPf, tPara.iDown, iIval[0], iIval[1], iIval[2], dwIb, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//������ȫʧѹ�¼��жϺ���
//�����๩��ϵͳ�У��������ѹ�����ڵ��ܱ���ٽ��ѹ��
//��������һ������ฺ�ɵ�������5%����������
//�ҳ���ʱ�����60s�����ֹ�������Ϊȫʧѹ��
//ȫʧѹʱ����ѹ����ֱ�����ܱ��ܹ���ʱ������¼ȫʧѹ������ֱ����ѹ�ָ���
//���ܱ���������ʱ���ٽ���ȫʧѹ�¼����жϡ�
//���ֹͣ��������ֹͣ����60sʱ����ҽ�������һ�Σ�����ȫʧѹ�¼���¼���жϣ��˺��ټ�������
//����6�����ò�������=structure
//{
//}
int AVLossJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	WORD wVol[3], wUn;		//��ѹ XXX.X v
	int iIval[3];				//ABCN ���� XXXXX.XXX A
	DWORD dwIb;
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_ALLVLOSS)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("AVLossJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("AVLossJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("AVLossJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	pEvtCtrl->bDelaySec = 60;	//ȫʧѹ�̶��ж���ʱΪ60��

	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 ���� �������ͣ�double-long����λ��A���㣺-3��A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5);	
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	// ������ѹ
	ReadItemEx(BN25, PN0, 0x5001, bBuf);
	wUn = (WORD)(BcdToDWORD(bBuf, 3)/10); //NNNN.NNתΪXXX.XV
	wUn *= 0.6;	//�ٽ��ѹ
	
	//��������
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	dwIb= BcdToDWORD(bBuf, 3)*0.05;	//NNN.NNNA, 5%Ib

	//״̬�ж�
	if (bPhase == 2)
	{
		if ((wVol[0]<wUn) && (wVol[2]<wUn) && ((labs(iIval[0])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	else if(bPhase == 3)
	{
		if ((wVol[0]<wUn) && (wVol[1]<wUn) && (wVol[2]<wUn) && ((labs(iIval[0])>dwIb)||(labs(iIval[1])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	//DTRACE(DB_INMTR, ("************************AVLossJudge: wVol=%u-%u-%u, wUn=%u,  iIval=%u-%u-%u, dwIb=%u, bJudgeState=%d.\r\n",wVol[0], wVol[1], wVol[2], wUn,  iIval[0], iIval[1], iIval[2], dwIb, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//�������������жϺ�����������ѹ������͵���������
//�����๩��ϵͳ�У������ѹ�����ڵ��ܱ���ٽ��ѹ��
//�����ѹ�������ҳ���ʱ�����60�룬��¼Ϊ��ѹ�������¼���
//�����๩��ϵͳ�У������ѹ�����ڵ��ܱ���ٽ��ѹ��
//�������������5%���������������������ҳ���ʱ�����60�룬��¼Ϊ�����������¼�
//@����6�����ò�������=structure
//{
//  �ж���ʱ  unsigned����λ��s�����㣺0��
//}
int DisOrderJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3], wUn;		//��ѹ XXX.X v
	int iIval[3];				//ABCN ���� XXXXX.XXX A
	DWORD dwIb;
	BYTE bPolar, i;
	
	if ((pEvtCtrl->wOI!=MTR_VDISORDER) && (pEvtCtrl->wOI!=MTR_IDISORDER))
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("DisOrderJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("DisOrderJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("DisOrderJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò���
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
		tPara.bDelaySec = *(bBuf+3);
	else
		tPara.bDelaySec = 0;
	if (tPara.bDelaySec == 0)
		tPara.bDelaySec = DEFAULT_DELAY_SEC;//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;
	
	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 �����������ͣ�double-long����λ��A���㣺-3��A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5);	//С��λ�������¼����ò���һ�£���4λС�� XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	// ������ѹ
	ReadItemEx(BN25, PN0, 0x5001, bBuf);
	wUn = (WORD)(BcdToDWORD(bBuf, 3)/10); //NNNN.NNתΪXXX.XV
	wUn *= 0.6;	//�ٽ��ѹ
	
	//��������
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	dwIb= BcdToDWORD(bBuf, 3)*0.05;	//NNN.NNNA, 5%Ib

	// ��������չID
	ReadItemEx(BN2, PN0, 0x1120, &bPolar);

	//״̬�ж�
	if (pEvtCtrl->wOI == MTR_VDISORDER)
	{
		if (bPhase == 2)
		{
			if ((bPolar==1) && (wVol[0]>wUn) && (wVol[2]>wUn))
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		}
		else if(bPhase == 3)
		{
			if ((bPolar==1) && (wVol[0]>wUn) && (wVol[1]>wUn) && (wVol[2]>wUn))
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		}
		//DTRACE(DB_INMTR, ("************************UDisOrderJudge: bPolar=%d,  wVol=%u-%u-%u, wUn=%u, bJudgeState=%d.\r\n",bPolar,  wVol[0], wVol[1],wVol[2],wUn, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	}
	else if (pEvtCtrl->wOI == MTR_IDISORDER)
	{
		if (bPhase == 2)
		{
			if ((bPolar==2) && (wVol[0]>wUn) && (wVol[2]>wUn) && (labs(iIval[0])>dwIb) && (labs(iIval[2])>dwIb))
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		}
		else if(bPhase == 3)
		{
			if ((bPolar==2) && (wVol[0]>wUn) && (wVol[1]>wUn) && (wVol[2]>wUn) && (labs(iIval[0])>dwIb) && (labs(iIval[1])>dwIb) && (labs(iIval[2])>dwIb))
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		}
		//DTRACE(DB_INMTR, ("************************iDisOrderJudge: bPolar=%d,  wVol=%u-%u-%u, iIval=%u, wVol=%u-%u-%u, dwIb=%u, bJudgeState=%d.\r\n",bPolar,  wVol[0], wVol[1],wVol[2],wUn, iIval[0], iIval[1],iIval[2],dwIb, pEvtCtrl->pEvtBase[0].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}


//����: ��������жϺ���
int MtrClrJudge(struct TEvtCtrl* pEvtCtrl)
{	
	return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_EVT_MTRCLR);
}

//����: ���������жϺ���
int DmdClrJudge(struct TEvtCtrl* pEvtCtrl)
{
	return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_EVT_DMDCLR);	
}

//����: �¼������жϺ���
int EvtClrJudge(struct TEvtCtrl* pEvtCtrl)
{	
	return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_EVT_EVTCLR);
}

//��������ѹ��ƽ���жϺ���
//�������ѹ�е���һ����ڵ��ܱ���ٽ��ѹ��
//��ѹ��ƽ���ʴ����趨�ĵ�ѹ��ƽ������ֵ��
//�ҳ���ʱ������趨�ĵ�ѹ��ƽ�����ж���ʱʱ�䣬���ֹ���Ϊ��ѹ��ƽ�⡣
//ע:��"���粻ƽ������ֵ"�趨Ϊ"0"ʱ����ʾ"��ѹ��ƽ���¼�"������
//@����6�����ò�������=structure
//{
//  ��ֵ  long����λ��%�����㣺-2����
//  �ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int VUnBalanceJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int16 iRate;			//XX.XX (��30%��Ӧ3000)
		BYTE bDelaySec;		//XX s	
	}tPara;
	WORD wVol[3], wUn;		//��ѹ XXX.X v
	WORD wRate = 0;	
	BYTE i;
	
	if (pEvtCtrl->wOI!=MTR_VUNBALANCE)
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VUnBalanceJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VUnBalanceJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VUnBalanceJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iRate = OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iRate == 0)
	{	
		DTRACE(DB_INMTR, ("VUnBalanceJudge: para iRate=%x.\r\n", tPara.iRate));
		return false;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;
		
	// 2000 3 ��ѹ �������ͣ�long-unsigned����λ��V�����㣺-1��A�ࡢB�ࡢC��
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	// ������ѹ
	ReadItemEx(BN25, PN0, 0x5001, bBuf);
	wUn = (WORD)(BcdToDWORD(bBuf, 3)/10); //NNNN.NNתΪXXX.XV
	wUn *= 0.6;	//�ٽ��ѹ
	
	//2026 6 ��ѹ��ƽ���� �������ͣ�long-unsigned����λ��%�����㣺-2
	if (OoReadAttr(0x2026, 0x02, bBuf, NULL, NULL) > 0)
		wRate = OoLongUnsignedToWord(bBuf+1);
	else
		wRate = 0;

	//״̬�ж�
	if (bPhase == 2)
	{
		if ((wRate>labs(tPara.iRate)) && ((wVol[0]>wUn)||(wVol[2]>wUn)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	else if (bPhase == 3)
	{
		if ((wRate>labs(tPara.iRate)) && ((wVol[0]>wUn)||(wVol[1]>wUn)||(wVol[2]>wUn)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}

	DTRACE(DB_INMTR, ("************************VUnBalanceJudge: wRate=%u,  tPara.iRate=%u, wVol=%u-%u-%u, wUn=%u, bJudgeState=%d.\r\n",wRate, tPara.iRate, wVol[0], wVol[1], wVol[2], wUn, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//������������ƽ���¼��жϺ���������������ƽ��͵������ز�ƽ��
//����������е���һ���������5%����������
//������ƽ���ʴ����趨�ĵ���(����)��ƽ������ֵ��
//�ҳ���ʱ������趨�ĵ���(����)��ƽ���ж���ʱʱ�䣬���ֹ���Ϊ����(����)��ƽ�⡣
//��"����(����)��ƽ������ֵ"�趨Ϊ"0"ʱ����ʾ����(����)��ƽ���¼�������
//@����6�����ò�������=structure
//{
//  ��ֵ  long����λ��%�����㣺-2��
//  �ж���ʱʱ��  unsigned����λ��s�����㣺0��
//}
int IUnBalanceJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int16 iRate;			//XX.XX (��30%��Ӧ3000)
		BYTE bDelaySec;		//XX s	
	}tPara;
	int iIval[3];				//ABCN ���� XXXXX.XXX A
	DWORD dwIb;
	WORD wRate = 0;	
	BYTE i;
	
	if ((pEvtCtrl->wOI!=MTR_IUNBALANCE) && (pEvtCtrl->wOI!=MTR_ISUNBALANCE))
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("IUnBalanceJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("IUnBalanceJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("IUnBalanceJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò�����ֵΪ0��ʾ������
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iRate = OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iRate == 0)
	{	
		DTRACE(DB_INMTR, ("IUnBalanceJudge: para iRate=%x.\r\n", tPara.iRate));
		return false;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;
		
	//2001 3 ���� �������ͣ�double-long����λ��A���㣺-3
	//A�ࡢB�ࡢC�ࡢN��
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5);	
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//��������
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	dwIb= BcdToDWORD(bBuf, 3)*0.05;	//NNN.NNNA, 5%Ib
 
	//2027 6 ������ƽ�����������ͣ�long-unsigned����λ��%�����㣺-2
	if (OoReadAttr(0x2027, 0x02, bBuf, NULL, NULL) > 0)
		wRate = OoLongUnsignedToWord(bBuf+1);
	else
		wRate = 0;

	//״̬�ж�
	if (bPhase == 2)
	{
		if ((wRate>labs(tPara.iRate)) &&((labs(iIval[0])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	else if(bPhase == 3)
	{
		if ((wRate>labs(tPara.iRate)) && ((labs(iIval[0])>dwIb)||(labs(iIval[1])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}

	//DTRACE(DB_INMTR, ("************************IUnBalanceJudge:  wOI=%04x,  wRate=%u,  tPara.iRate=%u, iIval=%u-%u-%u, dwIb=%u, bJudgeState=%d.\r\n", pEvtCtrl->wOI, wRate, tPara.iRate, iIval[0], iIval[1], iIval[2], dwIb, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//�������ն˹����жϺ���������ʱ�ӹ��Ϻͼ���оƬ����
//@����6�����ò�������=structure
//{
//  �ж���ʱ  unsigned����λ��s�����㣺0��
//}
int TermErrJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		BYTE bDelaySec;		
	}tPara;
	BYTE bErr = 0;
	BYTE i;
	
	if ((pEvtCtrl->wOI!=MTR_CLKERR) && (pEvtCtrl->wOI!=MTR_MTRCHIPERR))
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("TermErrJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("TermErrJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 ����Ԫ����, �����Ϊ1���������߱�Ϊ2���������߱�Ϊ3��
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("TermErrJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//���ò���
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
		tPara.bDelaySec = *(bBuf+3);
	else
		tPara.bDelaySec = 0;
	if (tPara.bDelaySec == 0)
		tPara.bDelaySec = DEFAULT_DELAY_SEC;//��ʱʱ��Ϊ0Ĭ��Ϊ60��
	pEvtCtrl->bDelaySec = tPara.bDelaySec;
	
	if (pEvtCtrl->wOI == MTR_CLKERR)
		ReadItemEx(BN2, PN0, 0x1122, &bErr);
	else if (pEvtCtrl->wOI == MTR_MTRCHIPERR)
		ReadItemEx(BN2, PN0, 0x1123, &bErr);

	//״̬�ж�
	if(bErr)	
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	else
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;		

	//DTRACE(DB_INMTR, ("************************TermErrJudge:  wOI=%04x,  bErr=%d, bJudgeState=%d.\r\n", pEvtCtrl->wOI, bErr, pEvtCtrl->pEvtBase[0].bJudgeState));	

	return pEvtCtrl->pEvtBase[0].bJudgeState;
}


//��������ϵͳ�����OAD����������
//������@dwOAD ���ݱ�ʶ
//		@pbBuf ���ݻ�����
//		@wDataLen OAD�����ݳ���
//		@wBufSize OAD�����ݻ�������С
//���أ�true/false
bool OoReadOad(DWORD dwOAD, BYTE* pbBuf, WORD wDataLen, WORD wBufSize)
{
	int iStart = -1;
	BYTE bAttr, bIndex, bType;
	BYTE bBuf[4096];	//tll ͳ���Ǳ����û�����ݽ����ط������󳤶� SYSDB_ITEM_MAXSIZE
	BYTE bBufIdx[50];	//����Ԫ�����ݳ��Ȳ������50
	BYTE* pbFmt;
	WORD wOI, wFmtLen, wID;
	int iLen;
	TTime now;
	BYTE bOadBuf[4];

	if (wDataLen > wBufSize)
		return false;
	
	GetOIAttrIndex(dwOAD, &wOI, &bAttr, &bIndex);

	// ʱ����Ҫ���⴦��
	if (wOI==0x4000 || wOI==0x201e || wOI==0x2020)		//ʱ����¼�����ʱ����¼�����ʱ��
	{	
		if (8 != wDataLen)
			return false;

		GetCurTime(&now);
		*pbBuf++ = DT_DATE_TIME_S;
		*pbBuf++ = now.nYear/256;
		*pbBuf++ = now.nYear%256;
		*pbBuf++ = now.nMonth;
		*pbBuf++ = now.nDay;
		*pbBuf++ = now.nHour;
		*pbBuf++ = now.nMinute;
		*pbBuf++ = now.nSecond;
		return true;
	}

	const ToaMap* pOI = GetOIMap(dwOAD&0xffff1f00);
	if (pOI == NULL)
		return false;

	if (IsNeedRdSpec(pOI) && bIndex==0)		//����OI�Ķ�ȡ��������������ʱ���
	{
		iStart = -1;
		iLen = OIRead_Spec((ToaMap *)pOI, bBuf, sizeof(bBuf), &iStart);
		pbFmt = pOI->pFmt;
		wFmtLen = pOI->wFmtLen;
	}
	else
	{
		if (IsNeedRdSpec(pOI))
		{
			if(pOI->wMode == MAP_SYSDB)		//
				iLen = ReadItemEx(BN0, bIndex-1, pOI->wID, bBuf);		//����OAD�ĵ�bIndex�����Զ�Ӧ��bIndex-1��������
			else
				iLen = OoReadAttr(wOI, bAttr, bBuf, &pbFmt, &wFmtLen);

			bIndex = 0;	//����OI������Ϊ��ֳ���OAD������ֱ�Ӷ���OAD�����ݣ���ͬ�ڶ�����OAD�ĵڼ�������
		}
		else
		{
			memset(bOadBuf, 0, sizeof(bOadBuf));
			OoDWordToOad(dwOAD, bOadBuf);
			if (sizeof(bBuf) < OoGetDataLen(DT_OAD, bOadBuf))
			{
				DTRACE(DB_INMTR, ("OoReadOad: dwOAD=%08x Buf Not Enough, return!\r\n", dwOAD));
				return false;
			}

			iLen = OoReadAttr(wOI, bAttr, bBuf, &pbFmt, &wFmtLen);
			if (IsSpecFrzOAD(dwOAD) && IsIntervMatch(dwOAD))	//ͳ��OAD,��ȡ��һ����ͳ�ƽ��
			{
				wID = GetLastCycleFrzMapID(dwOAD);
				if (wID > 0)
					iLen = ReadItemEx(BN11, PN0, wID, bBuf);	//��ȡ��һ����ͳ�ƽ��ID
			}
		}
	}

	if (iLen > 0)
	{	
		if (bIndex > 0)	//��ȡ����������
		{	
			iLen = OoReadField(bBuf, pbFmt, wFmtLen, (WORD)bIndex-1, bBufIdx, &bType);
			if (iLen > 0)
				memcpy(bBuf, bBufIdx, iLen);
		}
	}

	if (iLen>0 && iLen==wDataLen)
	{	
		memcpy(pbBuf, bBuf, wDataLen);
		return true;
	}
	
	return false;
}


//�����������ֶ�OAD����¼��������ֶ�
//������@ dwROAD �¼���¼��OAD
//		@ dwFieldOAD �ֶ�OAD
//		@ pbField �ֶλ������������ֵ��������������ֵ
//		@ wFieldLen �ֶλ������ĳ���
//		@ wFieldSize �ֶλ������Ĵ�С
//���أ�����ж������ֶν�������ȷ�����򷵻�true�����򷵻�false
bool MakeEvtSpecField(DWORD dwROAD, DWORD dwFieldOAD, BYTE* pbField, WORD wFieldLen, WORD wFieldSize)
{
	BYTE bByte = 0, bBit = 0, bSaveMtrNum = 0;
	BYTE bAttr, bItem, bState, bCnNum;
	BYTE bBuf[50];
	BYTE* pbData = pbField;
	BYTE* pbData0 = pbData; 
	WORD wOI, wOldVal, wNewVal, wIndex = 0;
	DWORD dwRecSN, dwOldVal,dwNewVal;
	int iLen, iRet = 0;
	TTime tmCurRec;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 
	BYTE i;
	TDmd* pDmd = NULL;
	if (wFieldLen > wFieldSize)
		return false;

	GetOIAttrIndex(dwROAD, &wOI, &bAttr, NULL);
	//��ȡ�¼����ƽṹ
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return -1;	
	//��ȡ�¼�����
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return -1;
	//��ȡ�¼������
	if (bAttr >= pEvtAttr->bRecTabStart)
		bItem = bAttr-pEvtAttr->bRecTabStart;
	else 
		return -1;

 	bState = pEvtCtrl->pEvtBase[bItem].bState;	//��ǰ״̬
 	
	switch(dwFieldOAD)
	{
		case 0x20220200:		//�¼���¼���
			if (wFieldLen != 5)
				return false;
			if (bState == EVT_S_AFT_HP)
			{
				dwRecSN = OoDoubleLongUnsignedToDWord(pbField+1);
				dwRecSN++;
				*pbData++ = DT_DB_LONG_U;
				OoDWordToDoubleLongUnsigned(dwRecSN, pbData);
			}
			return true;
		case 0x201E0200:		//�¼�����ʱ��
			if (wFieldLen != 8)
				return false;
			if (wFieldSize < 16)
				return false;

			if (bState == EVT_S_AFT_HP)
			{
				if (wOI == TERM_CLOCKPRG)	//ȡУʱǰ�ն�ʱ��
				{
					TAdjTermTime* pEvtPriv = (TAdjTermTime* )pEvtCtrl->pEvtPriv;
					if (pEvtPriv == NULL)
						return false;

					memcpy(pbData, pEvtPriv->bClock, 8);
					return true;
				}

				if (wOI==TERM_POWOFF && !IsInvalidTime(g_PowerOffTmp.tPoweroff))		//�ն�ͣ�ϵ��¼�
					tmCurRec = g_PowerOffTmp.tPoweroff;		//�¼�����ʱ��ȡ���һ��ͣ��ʱ��
				else
					GetCurTime(&tmCurRec);

				*pbData++ = DT_DATE_TIME_S;
				OoTimeToDateTimeS(&tmCurRec, pbData);pbData+=sizeof(TDTime);
				if (wOI == TERM_TERMPRG)	 //��̨��ֱ�Ӹ�ֵ
					memcpy(pbData, pbData-sizeof(TDTime)-1, sizeof(TDTime)+1);
				else
					memset(pbData, 0x00, sizeof(TDTime)+1);	//������������
			}	
			return true;
		case 0x20200200:		//�¼�����ʱ��
			if (wFieldLen != 8)
				return false;
			if (bState == EVT_S_AFT_END)
			{
				if (wOI == TERM_TERMPRG)	//�Ѿ���ȡ
					return true;
				else if (wOI==TERM_POWOFF )
				{
					//��ȡ˽�б���
					TPowOff* pCtrl = (TPowOff* )pEvtCtrl->pEvtPriv;
					if (pCtrl == NULL)
						return false;
					if ((pCtrl->bRptFlag==2) && (!IsInvalidTime(g_tPowerOn)))		//�ն��ϵ��¼�
						tmCurRec = g_tPowerOn;		//�¼�����ʱ��ȡ���һ���ϵ�ʱ��
					else
						return true;
				}			
				else
					GetCurTime(&tmCurRec);

				*pbData++ = DT_DATE_TIME_S;
				OoTimeToDateTimeS(&tmCurRec, pbData);
			}
			return true;
		case 0x20240200:		//�¼�����Դ
			//if (wOI == TERM_POWOFF)
			//{
			//	if (bState==EVT_S_AFT_HP || bState==EVT_S_AFT_END)
			//	{
			//		 if (GetEvtSrcData(pEvtCtrl, pbData, 1) < 0)	//ֱ�ӿ���
			//	 		return false;
			//	}
			//}
			//else
			//{
				if (bState == EVT_S_AFT_HP)
				{
					 if (GetEvtSrcData(pEvtCtrl, pbData, 1) < 0)	//ֱ�ӿ���
				 		return false;			
				}
			//}
			return true;
		case 0x33080206:		//�����ڼ������й��������ֵ������ʱ��Ҳ�ڴ˴���
			pDmd = (TDmd* )pEvtCtrl->pEvtPriv;
			if (pDmd == NULL)
				return false;
			if (wFieldLen != 5)
				return false;
			if (wFieldSize < 13)
				return false;
			if (bState >= EVT_S_AFT_HP)	
			{
				*pbData++ = DT_DB_LONG_U;
				OoDWordToDoubleLongUnsigned(pDmd->tDmd[bItem].dwDmdVal, pbData);pbData+=sizeof(DWORD);
				*pbData++ = DT_DATE_TIME_S;
				memcpy(pbData, (BYTE*)&pDmd->tDmd[bItem].tTime, sizeof(TDTime));
			}
			return true;
		case 0x33080207:		//�����ڼ��������ֵ����ʱ��
			if (wFieldLen != 8)
				return false;
			return true;
		case 0x330C0206:		//�¼������б�
			if (bState == EVT_S_AFT_HP)
			{	
				TEvtClr* pEvtClr = (TEvtClr* )pEvtCtrl->pEvtPriv;
				if (pEvtClr == NULL)
					return false;
				memcpy(pbData, pEvtClr,sizeof(TEvtClr));
				memset((BYTE*)pEvtClr, 0x00, sizeof(TEvtClr));	//ʹ�ú�����
			}		
			return true;
		case 0x33000200:		//�¼��ϱ�״̬
			if (wFieldLen != CN_RPT_TOTAL_LEN)
				return false;
			if (bState == EVT_S_AFT_HP)
			{	
				memset(pbData, 0x00, CN_RPT_TOTAL_LEN);	//Ĭ��Ϊ��
				if (OoReadAttr(0x4300, 10, bBuf, NULL, NULL) > 0)	//�ϱ�ͨ��
				{
					if (bBuf[1])
					{
						bCnNum = bBuf[1];
						if (bCnNum > CN_RPT_NUM)
							bCnNum = CN_RPT_NUM;
						*pbData = DT_ARRAY;
						*(pbData+1) = bCnNum;
						for (i=0; i<bCnNum; i++)	
						{
							memcpy(pbData+2+i*CN_RPT_STATE_LEN, g_bCnRptState,sizeof(g_bCnRptState));
							memcpy(pbData+5+i*CN_RPT_STATE_LEN, bBuf+3+5*i,sizeof(DWORD));
						}
					}
				}
			}	
			return true;
		case 0x20296200:		//��ʱֵ
			if (bState == EVT_S_BF_HP)
				return false;
			if (OoReadOad(dwFieldOAD, bBuf, wFieldLen, wFieldSize) == false)
				return false;
			if (bState == EVT_S_AFT_HP)
				memcpy(pbData, bBuf, wFieldLen);	
			else if (bState == EVT_S_AFT_END)
			{
				dwOldVal = OoDoubleLongUnsignedToDWord(pbData+1);
				dwNewVal = OoDoubleLongUnsignedToDWord(bBuf+1);
				if (dwOldVal < dwNewVal)
					dwNewVal -= dwOldVal;
				OoDWordToDoubleLongUnsigned(dwNewVal, pbData+1);	
			}
			return true;			
		case 0x20266200:		//��ƽ����
		case 0x20276200:		//��ƽ����	
			if (bState == EVT_S_BF_HP)
				return false;
			if (OoReadOad(dwFieldOAD, bBuf, wFieldLen, wFieldSize) == false)
				return false;
			if (bState == EVT_S_AFT_HP)
				memcpy(pbData, bBuf, wFieldLen);	
			else if ((bState==EVT_S_BF_END)||(bState==EVT_S_AFT_END))	//��ƽ����ȡ��ֵ
			{
				wOldVal= OoLongUnsignedToWord(pbData+1);
				wNewVal= OoLongUnsignedToWord(bBuf+1);
				if (wOldVal < wNewVal)
					memcpy(pbData, bBuf, wFieldLen);	
			}
			return true;	
		case 0x33020206:		//��̶����б�  array OAD	
			if (bState == EVT_S_AFT_HP)
			{	
				TTermPrg* pEvtPriv = (TTermPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				memcpy(pbData, pEvtPriv->bOAD, TERM_PRG_LIST_LEN);
				*pbData = DT_ARRAY;	//ȷ��ΪARRAY��������
				memset((BYTE*)pEvtPriv, 0x00, TERM_PRG_LIST_LEN);	//ʹ�ú�����
			}		
			return true;	

		case 0x33030206:		//�ѱ���      array һ���ѱ���	
			if (bState == EVT_S_AFT_HP)
			{	
				TUnKnMtr* pEvtPriv = (TUnKnMtr* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				//��ǿ����pEvtPriv->bSaveFlag�����λ��ȡ�������ѱ�����
				//��֡��array һ���ѱ�����pbData
				//*pbData++ = DT_ARRAY;��������������������
				//ʹ���������

				bSaveMtrNum = 0;
				*pbData++ = DT_ARRAY;

				pbData0 = pbData;	//��¼����Ԫ�ظ���ָ��λ��
				pbData++;

				for (bByte=0; bByte<SCH_MTR_SAVE_LEN; bByte++)
				{
					if (pEvtPriv->bSaveFlag[bByte] == 0x00)	//���ֽڲ���Ҫ��¼
						continue;

					for (bBit=0; bBit<8; bBit++)
					{
						if ((pEvtPriv->bSaveFlag[bByte] & (1<<bBit)) != 0) //������ѵ����ˣ���Ҫ��¼
						{
							pEvtPriv->bSaveFlag[bByte] &= ~(0x01<<bBit);

							wIndex = bByte*8 + bBit;
							iRet = GetSchMtrEvtData(wIndex, pbData);	//
							if (iRet > 0)
							{
								pbData += iRet;
								bSaveMtrNum++;
							}							
						}

						if (bSaveMtrNum == SCH_MTR_SAVE_REC_NUM) //��������
						{
							*pbData0 = bSaveMtrNum;		//��������Ԫ�ظ���
							memset((BYTE*)pEvtPriv->bSaveFlag, 0x00, SCH_MTR_SAVE_LEN);
							return true;	
						}
					}	
				}

				*pbData0 = bSaveMtrNum;		//��������Ԫ�ظ���
				memset((BYTE*)pEvtPriv->bSaveFlag, 0x00, SCH_MTR_SAVE_LEN);
			}		
			return true;	
		case 0x33040206:		// ��̨���ѱ���  array  һ����̨�����	
			if (bState == EVT_S_AFT_HP)
			{	
				TUnKnMtr* pEvtPriv = (TUnKnMtr* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				//��ǿ����pEvtPriv->bSaveFlag�����λ��ȡ�������ѱ�����
				//��֡��array  һ����̨�������pbData
				//*pbData++ = DT_ARRAY;��������������������
				//ʹ���������
				memset((BYTE*)pEvtPriv->bSaveFlag, 0x00, SCH_MTR_SAVE_LEN);
			}		
			return true;	
		case 0x33050206:		//�¼�������2���ӹ���  long64(��λ��W������-1)
			if (bState == EVT_S_AFT_HP)
			{
				memset(pbData, 0x00, 9);
				*pbData++  = DT_LONG64;
			}
			else if (bState == EVT_S_BF_END)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bHpAfPow[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bHpAfPow, 9);
				memset(pEvtPriv->bHpAfPow, 0x00, 9);//ʹ�ú�����
			}		
			return true;	
		case 0x33050207:		//���ƶ���  OI
			if (bState == EVT_S_AFT_HP)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bCtrlOI[0] != DT_OI)
					return false;
				memcpy(pbData, pEvtPriv->bCtrlOI, 3);
				memset(pEvtPriv->bCtrlOI, 0x00, 3);//ʹ�ú�����
			}		
			return true;	
		case 0x33050208:		//��բ�ִ�  bit-string(SIZE(8))
			if (bState == EVT_S_AFT_HP)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bBreakCnt[0] != DT_BIT_STR)
					return false;
				memcpy(pbData, pEvtPriv->bBreakCnt, 3);
				memset(pEvtPriv->bBreakCnt, 0x00, 3);//ʹ�ú�����
			}		
			return true;	
		case 0x33050209:		//���ض�ֵ  long64����λ��kW������-4��
			if (bState == EVT_S_AFT_HP)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bPowCtrlVal[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bPowCtrlVal, 9);
				memset(pEvtPriv->bPowCtrlVal, 0x00, 9);//ʹ�ú�����
			}		
			return true;	
		case 0x3305020A:		//��բ����ǰ�ܼ��й�����    long64����λ��kW������-4��
			if (bState == EVT_S_AFT_HP)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bHpBfPow[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bHpBfPow, 9);
				memset(pEvtPriv->bHpBfPow, 0x00, 9);//ʹ�ú�����
			}		
			return true;	
		case 0x33060206:		//���ƶ���  OI
			if (bState == EVT_S_AFT_HP)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bCtrlOI[0] != DT_OI)
					return false;
				memcpy(pbData, pEvtPriv->bCtrlOI, 3);
				memset(pEvtPriv->bCtrlOI, 0x00, 3);//ʹ�ú�����
			}		
			return true;	
		case 0x33060207:		//��բ�ִ�  bit-string(SIZE(8))
			if (bState == EVT_S_AFT_HP)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bBreakCnt[0] != DT_BIT_STR)
					return false;
				memcpy(pbData, pEvtPriv->bBreakCnt, 3);
				memset(pEvtPriv->bBreakCnt, 0x00, 3);//ʹ�ú�����
			}		
			return true;	
		case 0x33060208:		//���ض�ֵ  long64����λ��kW������-4��
			if (bState == EVT_S_AFT_HP)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bEleCtrlVal[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bEleCtrlVal, 9);
				memset(pEvtPriv->bEleCtrlVal, 0x00, 9);//ʹ�ú�����
			}		
			return true;	
		case 0x33060209:		//��բ����ǰ�ܼ��й����� long64����λ��kW������-4��
			if (bState == EVT_S_AFT_HP)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bHpEng[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bHpEng, 9);
				memset(pEvtPriv->bHpEng, 0x00, 9);//ʹ�ú�����
			}		
			return true;	
		case 0x33070206:		//���ƶ���  OI
			if (bState == EVT_S_AFT_HP)
			{	
				TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bCtrlOI[0] != DT_OI)
					return false;
				memcpy(pbData, pEvtPriv->bCtrlOI, 3);
				memset(pEvtPriv->bCtrlOI, 0x00, 3);//ʹ�ú�����
			}		
			return true;	
		case 0x33070207:		//��ض�ֵ  long64����λ��kWh������-4��
			if (bState == EVT_S_AFT_HP)
			{	
				TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bEleAlrCtrlVal[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bEleAlrCtrlVal, 9);
				memset(pEvtPriv->bEleAlrCtrlVal, 0x00, 9);//ʹ�ú�����
			}		
			return true;	
		case 0x33090206:		//���Ա�־	bit-string��SIZE(8)��
			if ((bState == EVT_S_AFT_HP) || (bState == EVT_S_AFT_END))
			{	
				TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				*pbData++ = DT_BIT_STR;
				*pbData++ = 0x08;
				*pbData = pEvtPriv->bAttr;
				//pEvtPriv->bAttr = 0;	//ʹ�ú�����
			}		
			return true;	
		case 0x330A0206:		//�غ�2�����ܼ��鹦�� array long64
			if (bState == EVT_S_AFT_HP)
			{
				memset(pbData, 0x00, 74);
				*pbData++ = DT_ARRAY;
			}
			else if (bState == EVT_S_AFT_END)
			{	
				TYKCtrl* pEvtPriv = (TYKCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bArrayPow[0] != DT_ARRAY)
					return false;
				memcpy(pbData, pEvtPriv->bArrayPow, 74);
				memset(pEvtPriv->bArrayPow, 0x00, 74);//ʹ�ú�����
			}		
			return true;	
		case 0x330B0206:		//�й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Ա��ܼ����й��ܵ����� long64����λ��kWh�����㣺-4����
			if (bState == EVT_S_AFT_HP)
			{	
				TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bCompEng[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bCompEng, 9);
				memset(pEvtPriv->bCompEng, 0x00, 9);//ʹ�ú�����
			}		
			return true;	
		case 0x330B0207:		//�й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�����ܼ����й��ܵ����� long64����λ��kWh�����㣺-4����
			if (bState == EVT_S_AFT_HP)
			{	
				TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bReferEng[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bReferEng, 9);
				memset(pEvtPriv->bReferEng, 0x00, 9);//ʹ�ú�����
			}		
			return true;	
		case 0x330B0208:		//�й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Խ�����ƫ��ֵ integer����λ��%�����㣺0��	
			if (bState == EVT_S_AFT_HP) 
			{	
				TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				memcpy(pbData, pEvtPriv->bRelaErr, 2);
				memset(pEvtPriv->bRelaErr, 0x00, 2);//ʹ�ú�����
			}		
			return true;			
		case 0x330B0209:		//�й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Խ�޾���ƫ��ֵ long64����λ��kWh�����㣺-4��	
			if (bState == EVT_S_AFT_HP)
			{	
				TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bAbsoErr[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bAbsoErr, 9);
				memset(pEvtPriv->bAbsoErr, 0x00, 9);//ʹ�ú�����
			}		
			return true;				
		case 0x330D0206:		//Уʱǰʱ��    date_time_s
			if (bState == EVT_S_AFT_HP) 
			{	
				TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				memcpy(pbData, pEvtPriv->bClock, 8);
				memset(pEvtPriv->bClock, 0x00, 8);//ʹ�ú�����
			}		
			return true;	
		case 0x330D0207:		//ʱ�����      integer����λ���룬�޻��㣩
			if (bState == EVT_S_AFT_HP) 
			{	
				TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				memcpy(pbData, pEvtPriv->bClkErr, 2);
				memset(pEvtPriv->bClkErr, 0x00, 2);//ʹ�ú�����
			}		
			return true;
		/*case 0x40002200:	//Уʱǰ�ն�ʱ��
			if (bState == EVT_S_AFT_HP)
			{
				TAdjTermTime* pEvtPriv = (TAdjTermTime* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;

				memcpy(pbData, pEvtPriv->bClock, 8);
				memset(pEvtPriv->bClock, 0, 8);//ʹ�ú�����
			}
			return true;*/	//Уʱǰ�������м����ݸ���
		default:
			return false;
	}

	return false;
}



//���������һ����¼�������ֶ��������ڴ洢
//������@ dwROAD �¼���¼��OAD
//		@ pRecBuf ������
//		@ wBufSize �������Ĵ�С
//���أ���ȷ��ȡ�����ݷ������ݳ��ȣ����򷵻ظ���
int EvtGetRecData(DWORD dwROAD, BYTE* pRecBuf, WORD wBufSize)
{
	BYTE bAttr, bItem, bType;
	BYTE bOadBuf[10];
	BYTE bAttrBuf[EVT_ATTRTAB_LEN];		//�������Ա�OAD
	BYTE bMemBuf[EVTREC_MAXSIZE];
	BYTE* pbMem = bMemBuf;
 	WORD wOI, wItemOffset, wItemLen;
	DWORD dwOAD;
	TFieldParser tDataFields;
	TTermEvtCtrl* pEvtCtrl;
	BYTE bIndex;
	const TEvtAttr* pEvtAttr; 
	
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	GetOIAttrIndex(dwROAD, &wOI, &bAttr, NULL);
	
	//��ȡ�¼����ƽṹ
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return -1;	

	//��ȡ�¼�����
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return -1;
	
	//��ȡ�¼������
	if (bAttr >= pEvtAttr->bRecTabStart)
		bItem = bAttr-pEvtAttr->bRecTabStart;
	else 
		return -1;

	//��ȡ�����ֶ�
	if (GetEvtFieldParser(pEvtCtrl, NULL, &tDataFields, bAttrBuf, sizeof(bAttrBuf)) == false)
		return -1;
	if (tDataFields.wNum == 0)	//�������Ա����Ϊ�գ�����0ֵ��ʾ�������ֶ�����
		return 0;
	if(tDataFields.wTotalLen==0)	//������
		return -1;	
	if (wBufSize < tDataFields.wTotalLen)	//����Χ��
		return -1;
	
	//���������ʱ�ռ�ȫ�ֱ���������bMemBuf
	memset((BYTE*)&bMemBuf, 0x00, sizeof(bMemBuf));
	 if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)	//������������ʱ��¼�����
	{	
		if (EvtReadRecMem(dwROAD, &g_TermMem, bMemBuf) <= 0)
			return -1;
	}
	else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTITEM)	//������������ʱ��¼�����
	{	
		if (EvtReadItemMem(dwROAD, &g_TermMem, bMemBuf) <= 0)
			return -1;
	}

	//��ȡ�����ֶε�����
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
			return -1;
		if (bType != DT_OAD) 
			return -1;
		if (wItemLen == 0) 
			return -1;
		dwOAD = OoOadToDWord(bOadBuf+1);	
		if (IsOADNeedSaveData(dwOAD, pEvtCtrl->pEvtBase[bItem].bState))
		{
			if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_NONE)	//��ϵͳ��ֱ�ӻ�ȡֵ
			{
				if (OoReadOad(dwOAD, bMemBuf, wItemLen, sizeof(bMemBuf)) == false)
				{
					pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//��Ҫ���³�ʼ��
					continue;
				}
				memcpy(pRecBuf, bMemBuf, wItemLen);
			}
			else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)
			{
				memcpy(pRecBuf, pbMem, wItemLen);
			}
			else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTITEM)	//����ʱ�ռ��ȡֵ
			{
				if (EvtReadOneItem (dwOAD, bMemBuf, bMemBuf+100) <= 0)	//�ȶ���ʱ����
				{
					if (OoReadOad(dwOAD, bMemBuf+100, wItemLen, sizeof(bMemBuf)) == false)	//������ֱ�Ӵ�ϵͳ���ȡ
					{
						pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//��Ҫ���³�ʼ��
						continue;
					}
				}
				memcpy(pRecBuf, bMemBuf+100, wItemLen);				
			}
			else
				return -1;		
		}
		if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)
			pbMem += wItemLen;
		pRecBuf += wItemLen;
	}
	
	return tDataFields.wTotalLen;
}


//�������˲���ʱ������״̬����ά��pEvtCtrl->pEvtBase[bItem].bState
//������@pEvtCtrl �¼�����
//���أ���
void UpdateState(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bItem;
	WORD wDelaySec = 0;
	TEvtBase* pEvtBase;
	bool fUpDataFlag = false;
	TTime time;

	wDelaySec = pEvtCtrl->bDelaySec;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//δ��ʼ��
			continue;

		if (pEvtCtrl->pEvtBase[bItem].wTrigEvtDelaySec != 0)
			wDelaySec = pEvtCtrl->pEvtBase[bItem].wTrigEvtDelaySec;		//�����д������¼���������ʱʱ��

		//״̬���ı仯���̱��������£�����ǰ--->������--->����ǰ--->������--->����ǰ
		if (pEvtBase->bState == EVT_S_BF_HP)
		{	
			if (pEvtBase->bJudgeState == EVT_JS_HP)	//����ǰ��������Ҫ�˲���ʱ
			{
				pEvtBase->dwRecvClick = 0;
				if (wDelaySec == 0)
				{
					pEvtBase->dwEstClick = GetClick();
					if (!pEvtBase->fExcValid)   //�²���һ���¼�
					{
						pEvtBase->bState = EVT_S_AFT_HP;	
						pEvtBase->fExcValid = true;
						fUpDataFlag = true;
					}  
				}
				else
				{
					if(pEvtBase->dwEstClick == 0)
					{
						GetCurTime(&time);
						DTRACE(DB_METER_EXC, ("UpdateState-EVT_S_AFT_HP-s time= %02d %02d:%02d:%02d, pEvtCtrl->wOI=0x%02x, bItem=%d, pEvtBase->dwEstClick=%ld, GetClick()=%ld.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond, pEvtCtrl->wOI, bItem, pEvtBase->dwEstClick, GetClick()));	
						pEvtBase->dwEstClick = GetClick();
					}
					else
					{
						if (GetClick()-pEvtBase->dwEstClick < wDelaySec) 
							continue;
						if (!pEvtBase->fExcValid)   //�²���һ���¼�
						{
							GetCurTime(&time);
							DTRACE(DB_METER_EXC, ("UpdateState-EVT_S_AFT_HP-s time= %02d %02d:%02d:%02d, pEvtCtrl->wOI=0x%02x, bItem=%d, pEvtBase->dwEstClick=%ld, GetClick()=%ld.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond, pEvtCtrl->wOI, bItem, pEvtBase->dwEstClick, GetClick()));	
							pEvtBase->bState = EVT_S_AFT_HP;	
							pEvtBase->fExcValid = true;
							fUpDataFlag = true;
						}  
					}
				}
			}			
		}
		else if (pEvtBase->bState == EVT_S_AFT_HP)
		{	
			pEvtBase->bState = EVT_S_BF_END;
			fUpDataFlag = true;
			if (pEvtBase->bJudgeState == EVT_JS_FORCE_END)	//��������������ǿ�ƽ���
			{
				if (pEvtBase->fExcValid)
				{	
					pEvtBase->bState = EVT_S_AFT_END;	
					pEvtBase->fExcValid = false;
				}
			}
		}
		else if (pEvtBase->bState == EVT_S_BF_END)
		{
			if (pEvtBase->bJudgeState == EVT_JS_FORCE_END)	//��������������ǿ�ƽ���
			{
				if (pEvtBase->fExcValid)
				{	
					pEvtBase->bState = EVT_S_AFT_END;	
					pEvtBase->fExcValid = false;
					fUpDataFlag = true;
				}
			}
			else if (pEvtBase->bJudgeState == EVT_JS_END)	//���������н���ǰ��������Ҫ�˲���ʱ
			{
				pEvtBase->dwEstClick = 0;
				if (wDelaySec == 0)
				{
					pEvtBase->dwRecvClick = GetClick();
					if (pEvtBase->fExcValid)  //һ���¼��ָ���
					{
						pEvtBase->bState = EVT_S_AFT_END;	
						pEvtBase->fExcValid = false;
						fUpDataFlag = true;
					}
				}
				else
				{
					if(pEvtBase->dwRecvClick == 0)
					{
						pEvtBase->dwRecvClick = GetClick();
						GetCurTime(&time);
						DTRACE(DB_METER_EXC, ("UpdateState-EVT_S_AFT_END-s time= %02d %02d:%02d:%02d, pEvtCtrl->wOI=0x%02x, bItem=%d, pEvtBase->dwEstClick=%ld, GetClick()=%ld.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond, pEvtCtrl->wOI, bItem, pEvtBase->dwEstClick, GetClick()));	
					}
					else
					{
						if (GetClick()-pEvtBase->dwRecvClick < wDelaySec) 
							continue;

						if (pEvtBase->fExcValid)  //һ���¼��ָ���
						{
							GetCurTime(&time);
							DTRACE(DB_METER_EXC, ("UpdateState-EVT_S_AFT_END-s time= %02d %02d:%02d:%02d, pEvtCtrl->wOI=0x%02x, bItem=%d, pEvtBase->dwEstClick=%ld, GetClick()=%ld.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond, pEvtCtrl->wOI, bItem, pEvtBase->dwEstClick, GetClick()));	
							pEvtBase->bState = EVT_S_AFT_END;	
							pEvtBase->fExcValid = false;
							fUpDataFlag = true;
						}
					}
				}
			}
		}
		else if (pEvtBase->bState == EVT_S_AFT_END)
		{	
			pEvtBase->bState = EVT_S_BF_HP;
			fUpDataFlag = true;
		}
		else
		{
			pEvtBase->fInitOk = false;	//���ݲ���ȷ���³�ʼ����
		}
	}

	 if (fUpDataFlag)
	{	
		UpdateRecMem(pEvtCtrl, 1);	//״̬�ո��л�ʱǿ��ˢ�����ݣ�����������ˢ��
		UpdateItemMem(pEvtCtrl, 1);
	}
	
	return;
}

//��������������ʼ�¼�ռ�Ĵ����з���/����ʱ������������ǰÿ2�����һ��
//������@pEvtCtrl �¼�����
//	@bSaveType ǿ��ˢ������
//���أ���
void UpdateRecMem(struct TEvtCtrl* pEvtCtrl, BYTE bSaveType)
{
	BYTE bAttrTab, bType, bItem, bIndex;
	BYTE bBuf[EVT_ATTRTAB_LEN];						//�������Ա�OAD
	BYTE bOadBuf[10] = {0};				//�������Ա��ȡ����OAD����BUF
	BYTE bRecMemBuf[EVTREC_MAXSIZE];	//�ö�/д��ʱ�ռ�Ļ�����
	BYTE* pbRec = bRecMemBuf;
	WORD wItemOffset, wItemLen;
	DWORD dwROAD, dwOAD;
	int iLen;
	TFieldParser tDataFields;
	const TEvtAttr* pEvtAttr; 
	TEvtBase* pEvtBase;

	//��ȡ�����ֶε�
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;
	bAttrTab = pEvtAttr->bRecTabStart;
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	if (GetEvtFieldParser(pEvtCtrl, NULL, &tDataFields, bBuf, sizeof(bBuf)) == false)
		return;
	if ((tDataFields.wCfgLen==0) ||(tDataFields.wTotalLen==0) ||(tDataFields.wNum==0)) 
		return;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pbRec = bRecMemBuf;
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//δ��ʼ��
			{bAttrTab++;continue;}
		if (pEvtBase->bMemType != MEM_TYPE_TERM_EVTREC)	//δ������ʱ�ռ�ֱ���˳�
			{bAttrTab++;continue;}

		if ((bSaveType == 1) || (pEvtCtrl->dwNewClick -pEvtCtrl->dwLastClick > EVT_UPDATE_CYC))
		{
			//��ȡ��¼��OAD
			dwROAD = GetOAD(pEvtCtrl->wOI, bAttrTab, 0);	

			//Step :	�ȶ�����ֵ
			if (EvtReadRecMem(dwROAD, &g_TermMem, bRecMemBuf) <= 0)
			{
				pEvtBase->fInitOk = false;	
				{bAttrTab++;continue;}
			}
			
			//Step :	bRecMemBuf����������ȡֵ
			for(bIndex=0; bIndex<tDataFields.wNum; bIndex++)
			{
				memset(bOadBuf, 0, sizeof(bOadBuf));
				if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
				{	
					pEvtBase->fInitOk = false;	
					return;	//ֱ���˳�
				}	
				if ((bType!=DT_OAD) || (wItemLen==0))
				{	
					pEvtBase->fInitOk = false;
					return;
				}	
				dwOAD = OoOadToDWord(bOadBuf+1);
				if (IsOADNeedAcqData(dwOAD, pEvtBase->bState))
				{
					if (MakeEvtSpecField(dwROAD, dwOAD, pbRec, wItemLen, sizeof(bRecMemBuf)) == false)
					{	
						if (OoReadOad(dwOAD, pbRec, wItemLen, sizeof(bRecMemBuf)) == false)
						{	
							pEvtBase->fInitOk = false;	
							return;
						}
					}
				}
				pbRec += wItemLen;
			}

			//Step :	��ȡ����������֮��ˢ����ֵ
			iLen = EvtWriteRecMem(dwROAD, &g_TermMem, bRecMemBuf);
			if ((iLen<=0) || (iLen!= tDataFields.wTotalLen))
			{
				pEvtBase->fInitOk = false;	//��Ҫ���³�ʼ��
				{bAttrTab++;continue;}
			}	
		}

		//��һ����¼��
		bAttrTab++;
	}
	
	return;
}


//�������������ʱ�ռ�Ĵ���ÿ2�����һ��
//	@bSaveType ǿ��ˢ������
void UpdateItemMem(struct TEvtCtrl* pEvtCtrl, BYTE bSaveType)
{
	BYTE bAttrTab, bItemLen,bItem, bIndex;
	BYTE bItemMemBuf[MEMORY_BLOCK_SIZE] ;	//�ö�/д��ʱ�ռ�Ļ�����
	BYTE bBuf[SYSDB_ITEM_MAXSIZE];			//ϵͳ�����������
	DWORD dwROAD, dwOAD;
	int iLen;
	const TEvtAttr* pEvtAttr; 
	TEvtBase* pEvtBase;

	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;
	bAttrTab = pEvtAttr->bRecTabStart;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//δ��ʼ��
			{bAttrTab++;continue;}
		if (pEvtBase->bMemType != MEM_TYPE_TERM_EVTITEM)	//δ������ʱ�ռ�ֱ���˳�
			{bAttrTab++;continue;}

		if ((bSaveType == 1) || (pEvtCtrl->dwNewClick -pEvtCtrl->dwLastClick > EVT_UPDATE_CYC))
		{
			//��ȡ��¼��OAD
			dwROAD = GetOAD(pEvtCtrl->wOI, bAttrTab, 0);	

			//Step :	�ȶ�����ֵ
			if (EvtReadItemMem(dwROAD, &g_TermMem, bItemMemBuf) <= 0)
			{
				pEvtBase->fInitOk = false;	//��Ҫ���³�ʼ��
				{bAttrTab++;continue;}
			}
			if (bItemMemBuf[0] == 0)	//OAD����Ϊ0
			{
				pEvtBase->fInitOk = false;	
				{bAttrTab++;continue;}
			}

			//Step : ����������ȡֵ����ȡ����ֵ��Ҫֱ��ˢ��
			for(bIndex=0; bIndex<bItemMemBuf[0]; bIndex++)
			{
				dwOAD = OoOadToDWord(bItemMemBuf+1+bIndex*5);
				bItemLen = *(bItemMemBuf+5+bIndex*5);
				if (IsEvtBeforeOAD(dwOAD) == false)
				{
					pEvtBase->fInitOk = false;	
					return;
				}
				if (IsOADNeedAcqData(dwOAD, pEvtBase->bState))
				{
					if (EvtReadOneItem(dwOAD, bItemMemBuf, bBuf) != bItemLen)	//������ֵ
					{
						pEvtBase->fInitOk = false;	
						return;
					}
					if (MakeEvtSpecField(dwROAD, dwOAD, bBuf, (WORD)bItemLen, sizeof(bBuf)) == false)
					{	
						if (OoReadOad(dwOAD, bBuf, (WORD)bItemLen, sizeof(bBuf)) == false)	
						{
							pEvtBase->fInitOk = false;	
							return;
						}
					}
					EvtWriteOneItem(dwOAD, bItemMemBuf, bBuf);
				}
			}
			if (EvtWriteItemMem(dwROAD, &g_TermMem, bItemMemBuf) <= 0)
				return;
		}

		//��һ����¼��
		bAttrTab++;
	}

	return;
}


//����:	�����¼�ϵͳ����DYN�������ݣ�������¼�����ԡ���ǰֵ��¼������
bool UpdateEvtStaData(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bClass, bItem, bItemNo, bState, bOffset;
	BYTE bBuf[EVT_ATRR_MAXLEN];
	WORD wOI, wMaxNum, wCurRecNum;
	DWORD dwTimes, dwClick, dwSec;
	int iLen;
	TTime tmCurRec;
	TEvtBase* pEvtBase;
	const TEvtAttr* pEvtAttr; 
	BYTE bCurRecList[30] = {0};

	wOI = pEvtCtrl->wOI;
	bClass = pEvtCtrl->bClass;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//δ��ʼ��
			continue;
		bState = pEvtBase->bState;
		bItemNo = bItem;
		if (pEvtCtrl->bItemNum == 3)
			bItemNo++;	//��ABC���Թ���

		//�¼�����ʱ�ۼƴ���
		if (bState == EVT_S_AFT_HP)
		{
			//����¼��
			if (OoReadAttr(wOI, pEvtAttr->bMaxRecNum, bBuf, NULL, NULL) > 0)
				wMaxNum = OoLongUnsignedToWord(bBuf+1);	
			else
				wMaxNum = 0;
			if (wMaxNum == 0)
			{
				pEvtBase->fInitOk = false;
				return false;
			}

			//��ǰ��¼���ۼ�
			iLen = OoReadAttr(wOI, pEvtAttr->bCurRecNum, bBuf, NULL, NULL);
			if (pEvtCtrl->bClass == IC7)
			{
				if (iLen <= 0)	
					memcpy(bBuf, g_bIC7CurNum, sizeof(g_bIC7CurNum));
				bOffset = 1;
			}
			else if (bClass == IC24)
			{
				if (iLen <= 0)	
					memcpy(bBuf, g_bIC24CurNum, sizeof(g_bIC24CurNum));
				bOffset = 3+bItemNo*3;
			}
			else
				return false;
			wCurRecNum = OoLongUnsignedToWord(bBuf+bOffset);
			if (wCurRecNum < wMaxNum)	//��ǰ��¼����1��ֱ����¼����������¼��
			{
				wCurRecNum++;
				OoWordToLongUnsigned(wCurRecNum, bBuf+bOffset);
				OoWriteAttr(wOI, pEvtAttr->bCurRecNum, bBuf);
			}
	
			//��ǰֵ��¼���ۼƴ����ۼӣ�IC7��ȡ�¼�����Դ
			iLen =OoReadAttr(wOI, pEvtAttr->bCurVal, bBuf, NULL, NULL);
			if (pEvtCtrl->bClass == IC7)
			{
				//��ǰֵ��¼���ȡ�д�ʱ����
				if (iLen <= 0)
				{	
					iLen = EvtSrctoCurRecList(pEvtCtrl, bCurRecList);
					if (iLen > 0)
						memcpy(bBuf, bCurRecList, iLen);
					else
						return false;
				}
				//��ȡ�¼�����Դ
				iLen =  GetEvtSrcData(pEvtCtrl, bBuf+4, 2);	//��Ҫ������
				if (iLen <= 0)
					return false;
				//�ۼƴ���ƫ��
				bOffset = 7+iLen;	
			}
			else if (bClass == IC24)
			{
				if (iLen <= 0)	
					memcpy(bBuf, g_bIC24CurRecList, sizeof(g_bIC24CurRecList));
				bOffset = 5+bItemNo*12;
			}
			else
				return false;	
			dwTimes = OoDoubleLongUnsignedToDWord(bBuf+bOffset);
			dwTimes++;
			OoDWordToDoubleLongUnsigned(dwTimes, bBuf+bOffset);
			OoWriteAttr(wOI, pEvtAttr->bCurVal, bBuf);
			pEvtBase->dwStaClick = GetClick();
			if (pEvtCtrl->wOI == MTR_VLOSS)	//������ʹͳ�����ݸ�׼ȷ
			{
				TVLoss* pVLoss = (TVLoss* )pEvtCtrl->pEvtPriv;
				if (pVLoss->dwVLossStaClick == 0)	//��δ��ʼͳ��ʱ
					pVLoss->dwVLossStaClick = pEvtBase->dwStaClick;
			}
		}		
	}	

	//�ۼ�ʱ��ÿ2�����һ��
	if (pEvtCtrl->dwNewClick -pEvtCtrl->dwLastClick <= EVT_UPDATE_CYC)
		return true;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//δ��ʼ��
			continue;
		bState = pEvtBase->bState;
		bItemNo = bItem;
		if (pEvtCtrl->bItemNum == 3)
			bItemNo++;	//��ABC���Թ���

		//�¼������ڼ��ۼ�ʱ��
		if ((bState==EVT_S_BF_END) || (bState==EVT_S_AFT_END))
		{
			iLen =OoReadAttr(wOI, pEvtAttr->bCurVal, bBuf, NULL, NULL);
			if (iLen <= 0)	//ͳ��֮ǰ����������
			{
				pEvtBase->fInitOk = false;
				return false;
			}
			if (pEvtCtrl->bClass == IC7)
			{
				//��ȡ�¼�����Դ����
				iLen =  GetEvtSrcData(pEvtCtrl, bBuf+4, 0);	//ֻ��ȡ����
				if (iLen <= 0)
					return false;
				//�ۼƴ���ƫ��
				bOffset = 12+iLen;	
			}
			else if (bClass == IC24)
				bOffset = 10+bItemNo*12;
			else
				return false;	
			dwClick =pEvtCtrl->dwNewClick;
			dwSec = OoDoubleLongUnsignedToDWord(bBuf+bOffset);
			if ((pEvtBase->dwStaClick) && ((dwClick-pEvtBase->dwStaClick)>0))	
			{	
				if (dwClick > pEvtBase->dwStaClick)
					dwSec += dwClick-pEvtBase->dwStaClick;
				OoDWordToDoubleLongUnsigned(dwSec, bBuf+bOffset);
				OoWriteAttr(wOI, pEvtAttr->bCurVal, bBuf);		
				pEvtBase->dwStaClick = dwClick;
			}
			if (bState==EVT_S_AFT_END)
				pEvtBase->dwStaClick = 0;
		}	
	}	
	
	return true; 
}

//������ʧѹ�¼�˽�к���
void UpdateVLossPriv(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bItem, bIsVLoss=0, bBuf[EVT_ATRR_MAXLEN];
	DWORD dwCnt, dwSec, dwNewClick =pEvtCtrl->dwNewClick;
	TTime tmCurRec;
	TVLoss* pVLoss = (TVLoss* )pEvtCtrl->pEvtPriv;
	if (pVLoss == NULL)
		return;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)	//δ��ʼ��
			continue;

		//˽�б���ʵʱά��
		if (pEvtCtrl->pEvtBase[bItem].bState == EVT_S_AFT_HP) 	//�����¼�����
		{	
			//if (pVLoss->dwVLossStaClick == 0)	//��δ��ʼͳ��ʱ
			//	pVLoss->dwVLossStaClick = GetClick();
			
			memset(bBuf, 0x00, sizeof(bBuf));
			if (OoReadAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf, NULL, NULL) <= 0)
				memcpy(bBuf, g_bAllVLossSta, sizeof(g_bAllVLossSta));
			dwCnt = OoDoubleLongUnsignedToDWord(bBuf+3);
			dwCnt++;
			OoDWordToDoubleLongUnsigned(dwCnt, bBuf+3);
			GetCurTime(&tmCurRec);
			OoTimeToDateTimeS(&tmCurRec, bBuf+13);
			OoWriteAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf);
			bIsVLoss = 1;			
		}		
		else if (pEvtCtrl->pEvtBase[bItem].bState == EVT_S_BF_END)
			bIsVLoss = 1;
	}	

	if (pVLoss->dwVLossStaClick == 0)
		return;

	//�����������Ҫͳ�ƣ��ۼ�ʱ��ÿ2��ͳ��һ��
	if ((dwNewClick -pEvtCtrl->dwLastClick >EVT_UPDATE_CYC) && (dwNewClick-pVLoss->dwVLossStaClick>0))
	{		
		memset(bBuf, 0x00, sizeof(bBuf));
		if (OoReadAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf, NULL, NULL) <= 0)
			memcpy(bBuf, g_bAllVLossSta, sizeof(g_bAllVLossSta));
		dwSec = OoDoubleLongUnsignedToDWord(bBuf+8);
		if (dwNewClick > pVLoss->dwVLossStaClick)
			dwSec += dwNewClick-pVLoss->dwVLossStaClick;
		OoDWordToDoubleLongUnsigned(dwSec, bBuf+8);
		pVLoss->dwVLossStaClick = dwNewClick;
		OoWriteAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf);			
	}
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (pEvtCtrl->pEvtBase[bItem].bState == EVT_S_AFT_END)	//�¼�����
		{
			memset(bBuf, 0x00, sizeof(bBuf));
			if (OoReadAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf, NULL, NULL) <= 0)
				memcpy(bBuf, g_bAllVLossSta, sizeof(g_bAllVLossSta));
			GetCurTime(&tmCurRec);
			OoTimeToDateTimeS(&tmCurRec, bBuf+21);		
			OoWriteAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf);	
			if (bIsVLoss == 0)	pVLoss->dwVLossStaClick = 0;
		}
	}
}

//�����������¼�˽�к���
//�����ڼ������й��������ֵ  double-long-unsigned��
//�����ڼ��������ֵ����ʱ��  date_time_s��
void UpdateDmdPriv(struct TEvtCtrl* pEvtCtrl)
{
	TDmd* pDmd = (TDmd* )pEvtCtrl->pEvtPriv;
	int iLen;
	BYTE bItem, bBuf[200] = {0};
	WORD wDmdID0, wDmdID;
	DWORD dwDmd[4] = {0};
	TTime tmCurRec;

	if (pDmd == NULL)
		return;
	
	if (pEvtCtrl->wOI == MTR_PDMDOVER)
		wDmdID0 = 0x3010;
	else if (pEvtCtrl->wOI == MTR_RPDMDOVER)
		wDmdID0 = 0x3020;
	else if (pEvtCtrl->wOI == MTR_QDMDOVER)
		wDmdID0 = 0x3030;
	else
		return;

	wDmdID = wDmdID0;
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk) //δ��ʼ��
			{wDmdID++;continue;}
		if (pEvtCtrl->pEvtBase[bItem].bState == EVT_S_BF_HP)	//δ�����¼�����Ҫˢ��
		{	
			if (pDmd->tDmd[bItem].dwDmdVal != 0)
				pDmd->tDmd[bItem].dwDmdVal = 0;
			{wDmdID++;continue;}
		}
		else if (pEvtCtrl->pEvtBase[bItem].bState==EVT_S_AFT_HP) //ˢ����ֵ
		{
			if (ReadItemEx(BN2, PN0, wDmdID, bBuf) > 0)
				dwDmd[bItem] = OoDoubleLongUnsignedToDWord(bBuf+1);
			else
				dwDmd[bItem] = 0;
			
			pDmd->tDmd[bItem].dwDmdVal = dwDmd[bItem];
			//��ȡ��ǰʱ��
			GetCurTime(&tmCurRec);
			OoTimeToDateTimeS(&tmCurRec, (BYTE*)&pDmd->tDmd[bItem].tTime);
		}
		wDmdID++;
	}

	//ÿ1�����һ��
	if (pEvtCtrl->dwNewClick -pEvtCtrl->dwLastClick <= EVT_UPDATE_CYC)
		return;

	wDmdID = wDmdID0;
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk) //δ��ʼ��
			{wDmdID++;continue;}
		if (pEvtCtrl->pEvtBase[bItem].bState <= EVT_S_AFT_HP)	//δ�����¼�����Ҫˢ��
			{wDmdID++;continue;}

		if (ReadItemEx(BN2, PN0, wDmdID, bBuf) > 0)
			dwDmd[bItem] = OoDoubleLongUnsignedToDWord(bBuf+1);
		else
			dwDmd[bItem] = 0;

		//ˢ����ֵ
		if (pDmd->tDmd[bItem].dwDmdVal<dwDmd[bItem]) 
		{
			pDmd->tDmd[bItem].dwDmdVal = dwDmd[bItem];
			//��ȡ��ǰʱ��
			GetCurTime(&tmCurRec);
			OoTimeToDateTimeS(&tmCurRec, (BYTE*)&pDmd->tDmd[bItem].tTime);
		}
		wDmdID++;
	}
}

//������ȫʧѹ�¼�˽�к���
//���������ʱˢ��
void UpdateAVLossPriv(struct TEvtCtrl* pEvtCtrl)
{	
	TAllVLoss* pAVLoss = (TAllVLoss* )pEvtCtrl->pEvtPriv;
	if (pAVLoss == NULL)
		return;
	memcpy((BYTE*)pAVLoss, pEvtCtrl->pEvtBase, sizeof(TAllVLoss));
}


//�����������¼���״̬�������¼����ʱ��¼���������
//������@pEvtCtrl�¼�����
//���أ������ȷ�򷵻�true�����򷵻�false
bool SaveTermEvtRec(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bAttrTab, bRptFlag, bItem, bState, bIndex, bType, bOffset, bSendRptFlag = EVT_STAGE_UNCARE;
	BYTE bOadBuf[10];
	BYTE bBuf[EVT_ATTRTAB_LEN];	//�������Ա��Ż�����
	BYTE bRecBuf[EVTREC_MAXSIZE];	//һ����¼������
	BYTE pvDataBuf[20];				//���ӻ�����
	BYTE* pbRec = bRecBuf;
	WORD wOI, wItemOffset, wItemLen, wRecIdx;
	int iLen;
	const ToaMap* pOaMap = NULL;
	TTime tmCurRec;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	const TEvtAttr* pEvtAttr; 
	BYTE i, bCnNum;
	DWORD dwROAD, dwOAD, dwCnOAD[CN_RPT_NUM] = {0};
	
	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));

	//��ȡ�¼�������
	wOI = pEvtCtrl->wOI;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;
	bAttrTab = pEvtAttr->bRecTabStart;

	//�ϱ���ʶ�����ϱ���0�����¼������ϱ���1�����¼��ָ��ϱ���2�����¼������ָ����ϱ���3��
	if (OoReadAttr(pEvtCtrl->wOI,  pEvtAttr->bRepFlg, bBuf, NULL, NULL) > 0)
		bRptFlag = bBuf[1];
	else
		bRptFlag = 0;

	//��ȡ�̶��ֶκ������ֶ�
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		DTRACE(DB_INMTR, ("SaveTermEvtRec: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return false;
	}
		
	//��¼
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		bSendRptFlag = EVT_STAGE_UNCARE;
		pbRec = bRecBuf;	
		bState = pEvtCtrl->pEvtBase[bItem].bState;
		if ((bState!=EVT_S_AFT_HP) && (bState!=EVT_S_AFT_END))	//ֻ�з����ͽ���ʱ����Ҫ��¼
			{bAttrTab++;continue;}
		
		dwROAD = GetOAD(wOI, bAttrTab, 0);
		pOaMap = GetOIMap(dwROAD);
		if (pOaMap==NULL || pOaMap->pszTableName==NULL)
		{
			DTRACE(DB_TASK, ("SaveTermEvtRec: Read dwOA :%x failed Or Tdb Table is null!!\r\n", dwROAD));
			{bAttrTab++;continue;}
		}
		//�ȶ�����1�ʼ�¼
		memset(bRecBuf, 0, sizeof(bRecBuf));
		iLen = ReadLastNRec(pOaMap->pszTableName, LAST_REC, bRecBuf, sizeof(bRecBuf));
		if (iLen <= 0)
		{	
			if (bState == EVT_S_AFT_END)	//����ǰ״̬Ϊ�ָ�����������1�ʼ�¼ֱ�ӷ���false
				return false;
			memset(bRecBuf, 0x00, sizeof(bRecBuf));
		}
		else
		{
			if (bState == EVT_S_AFT_HP)
				memset(bRecBuf+5, 0x00, sizeof(bRecBuf)-5);	//�²���һ����¼��Ҫ������˼�¼����������
		}

		//����̶��ֶ�
		for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
		{
			memset(bOadBuf, 0, sizeof(bOadBuf));
			if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
			{	
				pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//��Ҫ���³�ʼ��
				return false;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
			}	
			if ((bType!=DT_OAD) || (wItemLen==0))
			{	
				pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//��Ҫ���³�ʼ��
				return false;
			}		
			dwOAD = OoOadToDWord(bOadBuf+1);
			if (MakeEvtSpecField(dwROAD, dwOAD, pbRec, wItemLen, sizeof(bRecBuf)) == false)
			{	
				pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//��Ҫ���³�ʼ��
				return false;
			}	
			if (dwOAD == 0x33000200)	//�����ϱ���Ϣ
			{
				bCnNum = *(pbRec+1);
				if (bCnNum > CN_RPT_NUM)
					bCnNum = CN_RPT_NUM;
				for(i=0; i<bCnNum; i++)
					dwCnOAD[i] = OoOadToDWord(pbRec+5+CN_RPT_STATE_LEN*i);
			}
			pbRec += wItemLen;	
		}
	
		//���������ֶΣ��������Ա����Ϊ��
		if (tDataFields.wNum != 0)
		{
			iLen = sizeof(bRecBuf)-(pbRec - bRecBuf);
			if (iLen <= 0)
				return false;
			iLen = EvtGetRecData(dwROAD, pbRec, iLen);
			if (iLen <= 0)
				return false;
		}

		//�����¼�������
		if (bState == EVT_S_AFT_HP)
		{	
			//DTRACE(DB_FAPROTO, ("SaveTermEvtRec: wOI = %04x evt happen!\r\n", wOI));
			SaveRecord(pOaMap->pszTableName, bRecBuf);	
			//TrigerSaveBank(BN0, SECT3, -1);
			//TrigerSaveBank(BN0, SECT16, -1);
		}
		else if (bState == EVT_S_AFT_END)
		{	
			DTRACE(DB_FAPROTO, ("SaveTermEvtRec: wOI = %04x evt reover!\r\n", wOI));
			SaveHisRecord(pOaMap->pszTableName, 1, bRecBuf);
			memset(bRecBuf, 0, sizeof(bRecBuf));
			if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)
				EvtWriteRecMem(dwROAD, &g_TermMem, bRecBuf);
			else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTITEM)
			{
				EvtReadItemMem(dwROAD, &g_TermMem, bRecBuf);
				bOffset = 1+5*bRecBuf[0];
				memset(bRecBuf+bOffset, 0, sizeof(bRecBuf)-bOffset);
				EvtWriteItemMem(dwROAD, &g_TermMem, bRecBuf);
			}
			//TrigerSaveBank(BN0, SECT3, -1);
			//TrigerSaveBank(BN0, SECT16, -1);
		}
		else 
			return false;
		
		//�����ϱ���Ϣ		
		if ((bState==EVT_S_AFT_HP) && ((bRptFlag&0x01)==0x01))
			bSendRptFlag = EVT_STAGE_HP;
		else if ((bState==EVT_S_AFT_END) && ((bRptFlag&0x02)==0x02))
			bSendRptFlag = EVT_STAGE_END;

		if  (wOI == TERM_POWOFF)	//ͣ�ϵ��¼�������¼��ϱ���־���ж��Ƿ��ϱ�
		{
			TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
			if (pEvtPriv != NULL)
			{
				if (pEvtPriv->bRptFlag==1 && bSendRptFlag==EVT_STAGE_HP)		//�ϱ�����
					bSendRptFlag = EVT_STAGE_HP;
				else if (pEvtPriv->bRptFlag==2 && bSendRptFlag==EVT_STAGE_END)	//�ϱ��ָ�
					bSendRptFlag = EVT_STAGE_END;
				else
					bSendRptFlag = EVT_STAGE_UNCARE;

				DTRACE(DB_FAPROTO, ("SaveTermEvtRec: wOI = %04x,  bSendRptFlag=%d, pEvtPriv->bRptFlag=%d.\r\n", wOI, bSendRptFlag, pEvtPriv->bRptFlag));
			}
		}

		if ((bState==EVT_S_AFT_HP) || (bState==EVT_S_AFT_END))
			AddEvtOad(dwROAD, bSendRptFlag);

		DTRACE(DB_FAPROTO, ("SaveTermEvtRec: wOI = %04x,  bSendRptFlag=%d.\r\n", wOI, bSendRptFlag));
		if (bSendRptFlag != EVT_STAGE_UNCARE)	//��Ҫ�ϱ�
		{
			wRecIdx = GetRecPhyIdx(pOaMap->pszTableName, 1);
			if (wRecIdx >= 0)
			{	
				for(i=0; i<bCnNum; i++)
					SendEvtMsg(dwCnOAD[i], dwROAD, wRecIdx, bSendRptFlag);
			}	
		}
		bAttrTab++;
	}

	return true;
}
/*bool DoTrigeEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bItem, bItemNo, bOffse = 0;
	BYTE bBuf[EVT_TRIG_PARA_LEN] = {0};
	int iLen;
	bool fParaChgFlag = false;
	WORD wPn, wHpDelaySec, wEndDelaySec;
	TEvtBase* pEvtBase;
	TTime tmCurRec;
	
	if (GetEvtSn(pEvtCtrl->wOI) >= 0)
		wPn = GetEvtSn(pEvtCtrl->wOI);
	else
		return false;
	
	if (ReadItemEx(BN11, wPn, EVT_TRIG_ID, bBuf) <= 0)
		return false;
	if (IsAllAByte(bBuf, 0x00, sizeof(bBuf)))
		return false;	

	if (pEvtCtrl->bClass == IC7)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[0];
		//����һ�μ�¼���¼�����Դ��������ʱʱ�䣬�ָ���ʱʱ�䣩
		//�¼�����Դ��=instance-specific
		//������ʱʱ���=long-unsigned 
		//�ָ���ʱʱ���=long-unsigned
		iLen =  GetEvtSrcData(pEvtCtrl, bBuf, 0);	//ֻ��ȡ���� 
		if (iLen <= 0)
		{	
			memset(bBuf, 0x00, sizeof(bBuf));
			fParaChgFlag = true;
			goto End;
		}
		//�ж������Ƿ���Ч
		if ( (bBuf[0]!=pEvtCtrl->pbSrcFmt[0]) 
			|| ((bBuf[iLen]!=DT_LONG_U) && (bBuf[iLen]!=0xfe) && (bBuf[iLen]!=0xff))
			|| (bBuf[iLen+3]!=DT_LONG_U))	
		{	
			memset(bBuf, 0x00, sizeof(bBuf));
			fParaChgFlag = true;
			goto End;
		}
		//������ʱʱ��
		if (bBuf[iLen] == DT_LONG_U)
		{	
			pEvtBase->bJudgeState = 0;	//��ǿ�ƽ����¼�
			bBuf[iLen] = 0xfe;						//״̬��Ϊ���Կ�ʼ�����¼���
			fParaChgFlag = true;
		}
		else if (bBuf[iLen] == 0xfe)
		{
			pEvtBase->bJudgeState = 1;	//�¼�����
			pEvtBase->wTrigEvtDelaySec = OoLongUnsignedToWord(&bBuf[iLen+1]);	//��ȡ������ʱʱ��
			if (pEvtBase->wTrigEvtDelaySec == 0)
				pEvtBase->wTrigEvtDelaySec  = 0x02;
			if (iLen != 1)	//IC7�¼�����Դ�Ĵ���
			{
				if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOADFmt[0])
				{	
					if (pEvtCtrl->wOI == TERM_YKCTRLBREAK)
					{	
						TYKCtrl* pEvtPriv = (TYKCtrl* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TYKCtrl));
						memcpy(pEvtPriv->bEvtSrcOAD, bBuf, 5);
						pEvtPriv->bArrayPow[0] = DT_ARRAY;						
					}
				}
				else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcEnumFmt[0])
				{	
					if (pEvtCtrl->wOI == TERM_DEVICEERR)
					{	
						TDeviceErr* pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						 pEvtPriv->bEvtSrcEnum = bBuf[1];
					}
					else if (pEvtCtrl->wOI == TERM_CURCIRC)
					{
						TCurCirc* pEvtPriv = (TCurCirc* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						 pEvtPriv->bEvtSrcEnum = bBuf[1];
					}
					else if (pEvtCtrl->wOI == TERM_POWOFF)
					{
						TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						pEvtPriv->bEvtSrcEnum = bBuf[1];
					}
				}
				else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOIFmt[0])
				{
					if (pEvtCtrl->wOI == TERM_POWCTRLBREAK)
					{	
						TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TPowCtrl));
						memcpy(pEvtPriv->bEvtSrcOI, bBuf, 3);
						pEvtPriv->bHpAfPow[0] = DT_LONG64;
						pEvtPriv->bCtrlOI[0] = DT_OI;
						pEvtPriv->bBreakCnt[0] = DT_BIT_STR;
						pEvtPriv->bPowCtrlVal[0] = DT_LONG64;
						pEvtPriv->bHpAfPow[0] = DT_LONG64;						
					}
					else if (pEvtCtrl->wOI == TERM_ELECTRLBREAK)
					{
						TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TEleCtrl));
						memcpy(pEvtPriv->bEvtSrcOI, bBuf, 3);
						pEvtPriv->bCtrlOI[0] = DT_OI;
						pEvtPriv->bBreakCnt[0] = DT_BIT_STR;
						pEvtPriv->bEleCtrlVal[0] = DT_LONG64;
						pEvtPriv->bHpEng[0] = DT_LONG64;	
						
					}
					else if (pEvtCtrl->wOI == TERM_PURCHPARACHG)
					{
						TPurchParaChg* pEvtPriv = (TPurchParaChg* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TPurchParaChg));
						memcpy(pEvtPriv->bEvtSrcOI, bBuf, 3);
					}
					else if (pEvtCtrl->wOI == TERM_ELECTRLALARM)
					{
						TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TEleAlram));
						memcpy(pEvtPriv->bEvtSrcOI, bBuf, 3);
						pEvtPriv->bCtrlOI[0] = DT_OI;
						pEvtPriv->bEleAlrCtrlVal[0] = DT_LONG64;
					}
				}
				else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcTSAFmt[0])
				{		
					if (pEvtCtrl->wOI == TERM_MTRCLKPRG)
					{	
						TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memcpy(pEvtPriv->bEvtSrcTSA, bBuf, bBuf[1]+2);
						GetCurTime(&tmCurRec);
						pEvtPriv->bClock[0] = DT_DATE_TIME_S;
						OoTimeToDateTimeS(&tmCurRec, &pEvtPriv->bClock[1]);
						pEvtPriv->bClkErr[0] = DT_INT;
						pEvtPriv->bClkErr[1] = 0;
					}
				}
			}
			if (pEvtBase->bState == EVT_S_AFT_HP) 
			{
				bBuf[iLen] = 0xff;						//״̬תΪ�����¼��洢�����׼���ָ��¼�
				fParaChgFlag = true;
			}
		}

		if (bBuf[iLen] == 0xff)
		{
			if ((bBuf[iLen+3]==DT_LONG_U) && ((pEvtBase->bState == EVT_S_AFT_HP) || (pEvtBase->bState == EVT_S_BF_END)))
			{
				pEvtBase->bJudgeState = 2;	//�¼��ָ�
				pEvtBase->wTrigEvtDelaySec = OoLongUnsignedToWord(&bBuf[iLen+4]);	//��ȡ������ʱʱ��
				if (pEvtBase->wTrigEvtDelaySec == 0)
					pEvtBase->wTrigEvtDelaySec  = 0x02;
			}
			else if ((bBuf[iLen+3]==DT_LONG_U) && (pEvtBase->bState == EVT_S_AFT_END))
			{
				memset(bBuf, 0x00, sizeof(bBuf));
				pEvtBase->wTrigEvtDelaySec = 0;
				fParaChgFlag = true;
			}			
		}		
	}
	else if (pEvtCtrl->bClass == IC24)
	{
		//����һ�μ�¼���¼���𣬴�����ʱʱ�䣬�ָ���ʱʱ�䣩
		//�¼�����=enum
		//{
		//	�¼���¼1��0����
		//	�¼���¼2��1����
		//	�¼���¼3��2����
		//	�¼���¼4��3��
		//}
		//������ʱʱ���=long-unsigned 
		//�ָ���ʱʱ���=long-unsigned
		for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)	
		{	
			pEvtBase = &pEvtCtrl->pEvtBase[bItem];
			bItemNo = bItem;
			if (pEvtCtrl->bItemNum == 3)
				bItemNo++;
			bOffse = bItemNo*8;

			//�ж������Ƿ���Ч
			if ((bBuf[bOffse]!=DT_ENUM) || (bBuf[bOffse+1]!=bItemNo)
				|| ((bBuf[bOffse+2]!=DT_LONG_U) && (bBuf[bOffse+2]!=0xfe) && (bBuf[bOffse+2]!=0xff))
				|| (bBuf[bOffse+5]!=DT_LONG_U))		
			{	
				memset(bBuf+bOffse, 0x00, 8);
				fParaChgFlag = true;
				continue;
			}
			//������ʱʱ��
			if (bBuf[bOffse+2] == DT_LONG_U)
			{	
				pEvtBase->bJudgeState = 0;	//��ǿ�ƽ����¼�
				bBuf[bOffse+2] = 0xfe;						//״̬��Ϊ���Կ�ʼ�����¼���
				fParaChgFlag = true;
			}
			else if (bBuf[bOffse+2] == 0xfe)
			{
				pEvtBase->bJudgeState = 1;	//�¼�����
				pEvtBase->wTrigEvtDelaySec = OoLongUnsignedToWord(&bBuf[bOffse+3]);	//��ȡ������ʱʱ��
				if (pEvtBase->wTrigEvtDelaySec == 0)
					pEvtBase->wTrigEvtDelaySec  = 0x02;
				if (pEvtBase->bState == EVT_S_AFT_HP) 
				{
					bBuf[bOffse+2] = 0xff;						//״̬תΪ�����¼��洢�����׼���ָ��¼�
					fParaChgFlag = true;
				}
			}
			
			if (bBuf[bOffse+2] == 0xff)
			{
				if ((bBuf[bOffse+5]==DT_LONG_U) && ((pEvtBase->bState == EVT_S_AFT_HP) || (pEvtBase->bState == EVT_S_BF_END)))
				{
					pEvtBase->bJudgeState = 2;	//�¼��ָ�
					pEvtBase->wTrigEvtDelaySec = OoLongUnsignedToWord(&bBuf[bOffse+6]);	//��ȡ������ʱʱ��
					if (pEvtBase->wTrigEvtDelaySec == 0)
						pEvtBase->wTrigEvtDelaySec  = 0x02;
				}
				else if ((bBuf[bOffse+5]==DT_LONG_U) && (pEvtBase->bState == EVT_S_AFT_END))
				{
					memset(bBuf+bOffse, 0x00, 8);
					pEvtBase->wTrigEvtDelaySec = 0;
					fParaChgFlag = true;
					continue;
				}
			}		
		}
	}

End:
	if (fParaChgFlag == true)
	{
		WriteItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);
		TrigerSaveBank(BN11, 0, -1);
	}
	return true;
}*/
bool DoNullEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bItem;
	BYTE bFlag = false;
	
	//�س�ʼ��
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoNullEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//�س�ʼ��������ʧ���ֱ�ӷ���
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}

	//�¼���ǰʱ��
	pEvtCtrl->dwNewClick = GetClick();

	if (pEvtCtrl->pEvtBase[0].bState == EVT_S_BF_HP)
		bFlag = true;
	//if (DoTrigeEvt(pEvtCtrl) == true)
	//	bFlag = true;

	if (bFlag)
	{
		//�˲���ʱ,����״̬��
		UpdateState(pEvtCtrl);

		//�������ʱ�ռ�
		UpdateRecMem(pEvtCtrl, 0);
		UpdateItemMem(pEvtCtrl, 0);

		//��¼������ǰֵ��¼����
		UpdateEvtStaData(pEvtCtrl);

		//����״̬�������¼��
		SaveTermEvtRec(pEvtCtrl);
	}
	//�¼����㴦��
	ClearTermEvt(pEvtCtrl);

	//ˢ���¼�ʱ��
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;
	
	return true;
}

//������ͨ�õ��¼�ִ�к���
bool DoEvt(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE bItem;
	
	//�س�ʼ��
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//�س�ʼ��������ʧ���ֱ�ӷ���
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}

	//�¼���ǰʱ��
	pEvtCtrl->dwNewClick = GetClick();

	//�ж�״̬
	pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

	//�����¼�
	//DoTrigeEvt(pEvtCtrl);

	//�˲���ʱ,����״̬��
	UpdateState(pEvtCtrl);

	//�������ʱ�ռ�
	UpdateRecMem(pEvtCtrl, 0);
	UpdateItemMem(pEvtCtrl, 0);

	//��¼������ǰֵ��¼����
	UpdateEvtStaData(pEvtCtrl);

	//����״̬�������¼��
	SaveTermEvtRec(pEvtCtrl);

	//�¼����㴦��
	ClearTermEvt(pEvtCtrl);

	//ˢ���¼�ʱ��
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;

	return true;
}

//������ʧѹ�¼�ִ�к���
bool DoVLoss(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE bItem;
	
	//�س�ʼ��
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoVLoss: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//�س�ʼ��������ʧ���ֱ�ӷ���
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}
	
	//�¼���ǰʱ��
	pEvtCtrl->dwNewClick = GetClick();

	//�ж�״̬
	pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

	//�����¼�
	//DoTrigeEvt(pEvtCtrl);

	//�˲���ʱ,����״̬��
	UpdateState(pEvtCtrl);

	//�������ʱ�ռ�
	UpdateRecMem(pEvtCtrl, 0);
	UpdateItemMem(pEvtCtrl, 0);

	//��¼������ǰֵ��¼����
	UpdateEvtStaData(pEvtCtrl);

	//ˢ��˽�б�����ˢ��ʧѹͳ������
	UpdateVLossPriv(pEvtCtrl);

	//����״̬�������¼��
	SaveTermEvtRec(pEvtCtrl);

	//�¼����㴦��
	ClearTermEvt(pEvtCtrl);

	//ˢ���¼�ʱ��
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;

	return true;
}


//���������������¼�ִ�к���
bool DoDmd(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE bItem;
	
	//�س�ʼ��
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoDmd: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//�س�ʼ��������ʧ���ֱ�ӷ���
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}

	//�¼���ǰʱ��
	pEvtCtrl->dwNewClick = GetClick();

	//�ж�״̬
	pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

	//�����¼�
	//DoTrigeEvt(pEvtCtrl);

	//�˲���ʱ,����״̬��
	UpdateState(pEvtCtrl);

	//ˢ��˽�б������¼������ڼ�ȡ�������ֵ
	UpdateDmdPriv(pEvtCtrl);

	//�������ʱ�ռ�
	UpdateRecMem(pEvtCtrl, 0);
	UpdateItemMem(pEvtCtrl, 0);

	//��¼������ǰֵ��¼����
	UpdateEvtStaData(pEvtCtrl);

	//����״̬�������¼��
	SaveTermEvtRec(pEvtCtrl);

	//�¼����㴦��
	ClearTermEvt(pEvtCtrl);

	//ˢ���¼�ʱ��
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;

	return true;
}


//������ȫʧѹ�¼�ִ�к���
bool DoAVLoss(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE bItem;
	
	//�س�ʼ��
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoAVLoss: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//�س�ʼ��������ʧ���ֱ�ӷ���
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}

	//�¼���ǰʱ��
	pEvtCtrl->dwNewClick = GetClick();

	//�ж�״̬
	pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

	//�����¼�
	//DoTrigeEvt(pEvtCtrl);

	//�˲���ʱ,����״̬��
	UpdateState(pEvtCtrl);

	//ˢ��˽�б��������µ������
	UpdateVLossPriv(pEvtCtrl);

	//�������ʱ�ռ�
	UpdateRecMem(pEvtCtrl, 0);
	UpdateItemMem(pEvtCtrl, 0);

	//��¼������ǰֵ��¼����
	UpdateEvtStaData(pEvtCtrl);

	//����״̬�������¼��
	SaveTermEvtRec(pEvtCtrl);

	//�¼����㴦��
	ClearTermEvt(pEvtCtrl);

	//ˢ���¼�ʱ��
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;

	return true;
}


//�������¼��ӿں���ִ��
/*void DoTermEvt()
{
	static DWORD dwLastClick = GetClick();

	if (GetClick() - dwLastClick > 10)
	{
		dwLastClick = GetClick();

		WaitSemaphore(g_semTermEvt);
		for(BYTE i=0; i<EVT_NUM; i++)
			g_EvtCtrl[i].pfnDoEvt(&g_EvtCtrl[i]);
		SignalSemaphore(g_semTermEvt);
	}	
}*/

TThreadRet  DoTermEvt(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("DoTermEvt-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ60��
	DTRACE(DB_CRITICAL, ("DoTermEvt : started!\n"));

	while(1)
	{
		WaitSemaphore(g_semTermEvt);
		
	 	for(BYTE i=0; i<EVT_NUM; i++)
	 		g_EvtCtrl[i].pfnDoEvt(&g_EvtCtrl[i]);
		
		SignalSemaphore(g_semTermEvt);

		Sleep(100);
		UpdThreadRunClick(iMonitorID);
	}

	ReleaseThreadMonitorID(iMonitorID);
}


//**************************�¼����㹦��*********************************************
//��������ͳ�����ݣ�������ǰ��¼���͵�ǰֵ��¼��
//������@pEvtCtrl�¼�����
//���أ���
void ClearEvtStaData(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bCurRecList[50] = {0};
	const TEvtAttr* pEvtAttr; 

	//��ȡ�¼�������
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;

	if (pEvtCtrl->bClass == IC24)
	{
		OoWriteAttr(pEvtCtrl->wOI, pEvtAttr->bCurRecNum, (BYTE*)g_bIC24CurNum);	
		OoWriteAttr(pEvtCtrl->wOI, pEvtAttr->bCurVal, (BYTE*)g_bIC24CurRecList);	
	}
	else if (pEvtCtrl->bClass == IC7)
	{
		OoWriteAttr(pEvtCtrl->wOI, pEvtAttr->bCurRecNum, (BYTE*)g_bIC7CurNum);
		if (EvtSrctoCurRecList(pEvtCtrl, bCurRecList))
			OoWriteAttr(pEvtCtrl->wOI, pEvtAttr->bCurVal, bCurRecList);
	}

	return;
}

//ʧѹ�¼�˽����������
void ClearVLossPriv(struct TEvtCtrl* pEvtCtrl)
{
	if (pEvtCtrl->wOI == MTR_VLOSS)
	{	
		TVLoss* pVLoss = (TVLoss* )pEvtCtrl->pEvtPriv;
		if (pVLoss == NULL)
			return;
		memset((BYTE*)pVLoss, 0x00, sizeof(TVLoss));
		OoWriteAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, (BYTE*)g_bAllVLossSta);	//��ʧѹͳ��
	}	
}

//ȫʧѹ�¼�˽����������
void ClearVAllLossPriv(struct TEvtCtrl* pEvtCtrl)
{
	if (pEvtCtrl->wOI == MTR_ALLVLOSS)
	{	
		TAllVLoss* pAVLoss = (TAllVLoss* )pEvtCtrl->pEvtPriv;
		if (pAVLoss == NULL)
			return;
		memset((BYTE*)pAVLoss, 0x00, sizeof(pAVLoss));
	}	
}


//������һ���¼���¼����
//������@pEvtCtrl�¼�����
//���أ���
void ClearOneEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bAttrTab, bItem;
	DWORD dwROAD;	
	const ToaMap* pOaMap = NULL;
	const TEvtAttr* pEvtAttr; 

	//��ȡ�¼�������
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;
	bAttrTab = pEvtAttr->bRecTabStart;

	//�嵱ǰֵ
	ClearEvtStaData(pEvtCtrl);

	//�����¼���˽���������㡣
	ClearVLossPriv(pEvtCtrl);
	ClearVAllLossPriv(pEvtCtrl);
	if (pEvtCtrl->wOI == TERM_POWOFF)
		UpdateTermPowerOffTime();
	
	//����ʱ�ռ估���¼��
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		dwROAD = GetOAD(pEvtCtrl->wOI, bAttrTab, 0);

		if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTITEM)
			EvtFreeItemMem(dwROAD, &g_TermMem);
		else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)
			EvtFreeRecMem(dwROAD, &g_TermMem);

		//ɾ����¼��
		pOaMap = GetOIMap(dwROAD);
		TdbDeleteTable(pOaMap->pszTableName);

		//�ó�ʼ����ʶ��
		memset((BYTE*)&pEvtCtrl->pEvtBase[bItem], 0x00, sizeof(TEvtBase));

		//��һ����
		bAttrTab++;
	}

	DTRACE(DB_INMTR, ("ClearOneEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
	pEvtCtrl->pfnInitEvt(pEvtCtrl);

	if (GetInfo(INFO_EVT_EVTRESET))	//�����㣬�����¼������¼���¼
		SetInfo(INFO_EVT_EVTCLR);
	if (GetInfo(INFO_EVT_CLREVTRESET))	//�����㣬�����¼������¼���¼
		GetEvtClearOMD(MTR_EVTCLEAR, 0x01, 0x00);	
}

//��������������ʱ���������㴦��
//������@pEvtCtrl�¼�����
//���أ���
//ע���������������һ���¼���¼������
// 1. �������Ա���---���ò���/���ɾ��OAD����
// 2. ����¼���ݱ��
// 3. �������---��λ����/�������/�¼�������/��������
void ClearTermEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bAttrTab, bItem, bBuf[EVT_TRIG_PARA_LEN];
	WORD wPn;
	BYTE bClrFlag;
	DWORD dwROAD;	
	const ToaMap* pOaMap = NULL;
	const TEvtAttr* pEvtAttr; 
	
	if (GetEvtSn(pEvtCtrl->wOI) >= 0)
		wPn = GetEvtSn(pEvtCtrl->wOI);
	else
		return;
	
	if (ReadItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag) <= 0)
		return;
	if (bClrFlag != EVT_CLR_VALID) return;	//����Ҫ����

	//���¼�����
	ClearOneEvt(pEvtCtrl);

	//�������ʶ���
	bClrFlag = 0;
	WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);	//��������е������ٽ���ʶ����

	//���������ʶ
	memset(bBuf, 0x00, sizeof(bBuf));
	WriteItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);	//��������е������ٽ���ʶ����
	TrigerSaveBank(BN11, 0, -1);	
}

//��������¼���ⴥ�����¼�
bool RecordSpecTrigerEvt(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE i;
	BYTE bItem;

	 for(i=0; i<6; i++)
 	{
		//�س�ʼ��
		for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		{
			if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			{	
				DTRACE(DB_INMTR, ("RecordSpecTrigerEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
				pEvtCtrl->pfnInitEvt(pEvtCtrl);
			}
		}
		//�س�ʼ��������ʧ���ֱ�ӷ���
		for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		{
			if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
				return false;
		}
			
		//�¼���ǰʱ��
		pEvtCtrl->dwNewClick = GetClick();
		pEvtCtrl->dwLastClick = 0;

		//�ж�״̬
		pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

		//�˲���ʱ,����״̬��
		UpdateState(pEvtCtrl);

		//�������ʱ�ռ�
		UpdateRecMem(pEvtCtrl, 0);
		UpdateItemMem(pEvtCtrl, 0);

		//��¼������ǰֵ��¼����
		UpdateEvtStaData(pEvtCtrl);

		//����״̬�������¼��
		SaveTermEvtRec(pEvtCtrl);
	 }

	//ˢ���¼�ʱ��
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;
}

//���������������¼���ֱ������¼�ṩ�����нӿ�
//������@wOI �����ʶ
//���أ���
void DealSpecTrigerEvt(WORD wOI)
{
	BYTE i;
	TTermEvtCtrl* pEvtCtrl;

	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return;
	if (pEvtCtrl->bClass != IC7)	//��IC7�д�����	
		return;

	//WaitSemaphore(g_semTermEvt);
	//�������/�¼�����
	if ((wOI==MTR_MTRCLEAR) ||(wOI==MTR_EVTCLEAR))
	{
		for(i=0; i<EVT_NUM; i++)
		{	
			if (g_EvtCtrl[i].wOI == wOI)	//���������¼���¼
				continue;
			//if (g_EvtCtrl[i].wOI==TERM_INIT)	//�����ն˳�ʼ���¼�����Ҫ���
			//	continue;
			else if ((wOI==MTR_EVTCLEAR) && (g_EvtCtrl[i].wOI==MTR_MTRCLEAR))//0x43000500�¼���ʼ�������¼�������ʱ�����������¼���¼���¼������¼���¼
				continue;
			ClearOneEvt(&g_EvtCtrl[i]);	
		}
	}

	switch(wOI)
	{
		case MTR_MTRCLEAR:
			SetInfo(INFO_EVT_MTRCLR);//�������
			break;
		case MTR_DMDCLEAR:		//��������
			SetInfo(INFO_EVT_DMDCLR);			
			break;	
		case MTR_EVTCLEAR:		//�¼�����
			SetInfo(INFO_EVT_EVTCLR);
			GetEvtClearOMD(0x4300, 0x05, 0x00);
			//SignalSemaphore(g_semTermEvt);
			return;
		case TERM_INIT:		//�ն˳�ʼ��
			SetInfo(INFO_TERM_INIT);
			break;
		case TERM_CLOCKPRG:		//�ն˶�ʱ�¼�
			SetInfo(INFO_ADJ_TERM_TIME);
			break;	
		case TERM_TERMPRG:		//�ն˱�̼�¼	
			SetInfo(INFO_TERM_PROG);
			break;	
		default:
			//SignalSemaphore(g_semTermEvt);
			return;
	}

	RecordSpecTrigerEvt(pEvtCtrl);
	//SignalSemaphore(g_semTermEvt);
}


//��������ȡ�¼�����OMD������ͨѶ����
//������@wOI �����ʶ
//	   @ bMethod ���󷽷����
//	   @ bOpMode ����ģʽ
//���أ���
void GetEvtClearOMD(WORD wOI, BYTE bMethod, BYTE bOpMode)
{	
	BYTE bClrNum;	//�������
	DWORD dwOMD;
	TTermEvtCtrl* pEvtCtrl;
	TTermEvtCtrl* pEvtClrEvtCtrl;
	TEvtClr* pEvtClr;

	//ȷ���Ƿ�Ϊ�¼�����OAD�������¼���ʼ��0x43000500�͸��¼��ĸ�λ����
	dwOMD =  GetOAD(wOI, bMethod, bOpMode);	
	if (dwOMD != 0x43000500)	
	{
		if (bMethod != EVT_RESET)	//�¼���������
		{	
			if (wOI!=MTR_EVTCLEAR)
				return;
		}
		//��ȡ�¼����ƽṹ
		pEvtCtrl = GetTermEvtCtrl(wOI);
		if (pEvtCtrl == NULL)
			return;	
	}

	//��ȡ�¼������¼����ƽṹ
	pEvtClrEvtCtrl = GetTermEvtCtrl(MTR_EVTCLEAR);
	if (pEvtClrEvtCtrl == NULL)
		return;

	pEvtClr = (TEvtClr* )pEvtClrEvtCtrl->pEvtPriv;
	if (pEvtClr == NULL)
		return;
	pEvtClr->bOMD[0] = DT_ARRAY;
	if (pEvtClr->bOMD[1] < EVT_CLR_OMD_NUM)	//���10��
		pEvtClr->bOMD[1]++;
	else
		return;
	bClrNum = pEvtClr->bOMD[1];
	
	pEvtClr->bOMD[2+5*(bClrNum-1)] = DT_OMD;
	OoDWordToOad(dwOMD, &pEvtClr->bOMD[3+5*(bClrNum-1)]);
}


//��������ȡ��̼�¼�¼���̶����б�OAD������ͨѶ����
//������@dwOAD �����ʶ
//	   @ bAttr ��������
//	   @ bIndex 
//���أ���
void GetTermPrgOAD(WORD wOI, BYTE bAttr, BYTE Index)
{	
	BYTE bPrgNum;	//��̸���
	DWORD dwOAD;
	TTermEvtCtrl* pTermPrgCtrl;
	TTermPrg* pEvtPriv;

	dwOAD =  GetOAD(wOI, bAttr, Index);	
	//��ȡ�ն˱�̼�¼�¼����ƽṹ
	pTermPrgCtrl = GetTermEvtCtrl(TERM_TERMPRG);
	if (pTermPrgCtrl == NULL)
		return;

	pEvtPriv = (TTermPrg* )pTermPrgCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return;

	pEvtPriv->bOAD[0] = DT_ARRAY;
	if (pEvtPriv->bOAD[1] < TERM_PRG_OAD_NUM)	//���10��
		pEvtPriv->bOAD[1]++;
	else
		return;
	
	bPrgNum = pEvtPriv->bOAD[1];
	pEvtPriv->bOAD[2+5*(bPrgNum-1)] = DT_OAD;
	OoDWordToOad(dwOAD, &pEvtPriv->bOAD[3+5*(bPrgNum-1)]);

	DTRACE(DB_FAPROTO, ("GetTermPrgOAD: ******************************************************************bPrgNum=%d, dwOAD = %x.\r\n", bPrgNum, dwOAD));
}


//**************************�¼���¼��ȡ�ӿ�*********************************************
int GetEvtSpecField(struct TEvtCtrl* pEvtCtrl, DWORD dwFieldOAD, BYTE* pbField, WORD wFieldLen)
{
	WORD i = 0;
	BYTE bArrayNum, bTsaAddrLen = 0;
	int iRet = -1, iSchMtrLen = 0;
	 	
	switch(dwFieldOAD)
	{
		case 0x20200200:		//�¼�����ʱ��
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_DATE_TIME_S)
				iRet = 8;		//ʱ�䳤�ȣ��Ѽ�����
			break;	
		case 0x20240200:		//�¼�����Դ
			iRet = GetEvtSrcData(pEvtCtrl, pbField, 0);		//ֻ��ȡ���� 	
			break;
		case 0x330C0206:		//�¼������б�
		case 0x33020206:		//��̶����б�  array OAD	
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				iRet = 2+5*bArrayNum;
			}			
			break;	
		case 0x33030206:		//�ѱ���    array һ���ѱ�����ȡ	
			if (pbField[0] == DT_NULL)
			{
				iRet = 1;
			}
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];

				pbField += 2;	//ָ���ַ ָ����һ���ѱ���02 07 ...��02λ��

				iRet = 0;
				iSchMtrLen = 0;
				bTsaAddrLen = 0;
				for (i=0; i<bArrayNum; i++)
				{	
					iSchMtrLen = 0;

					iSchMtrLen += 2;		//02 07 

					iSchMtrLen += 2;		//55 pbField[3] 
					bTsaAddrLen = pbField[3];	//ͨ�ŵ�ַʵ�ʳ���
					iSchMtrLen += bTsaAddrLen;	//ͨ�ŵ�ַʵ�ʳ���					

					iSchMtrLen += 2;		//55 pbField[3+bTsaAddrLen+2]
					iSchMtrLen += pbField[3+bTsaAddrLen+2];	//ͨ�ŵ�ַʵ�ʳ���

					iSchMtrLen += 16;		//ʣ�ಿ�ֳ���֮��Ϊ16

					pbField += iSchMtrLen;	//ָ����N���ѱ���02 07 ...��02λ��

					iRet += iSchMtrLen;
				}

				iRet += 2;	//DT_ARRAY bArrayNum

				DTRACE(DB_FAPROTO, ("GetEvtSpecField: bArrayNum=%d, iRet = %ld.\r\n", bArrayNum, iRet));

				//iRet = 2+ONE_SCH_MTR_RLT_LEN*bArrayNum;	//һ���ѱ�������Ϊ56,���ܰ���󳤶ȼ��㣬Ӧ������ַʵ�ʳ��ȼ��㣡
			}			
			break;	
		case 0x33040206:		//��̨���ѱ���  array  һ����̨�����	
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				//iRet = 2+Len*bArrayNum;	//��ǿ��Len�޸�Ϊ�����ֵ
			}			
			break;		
		case 0x33000200:		//�¼��ϱ�״̬
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				iRet = 2+CN_RPT_STATE_LEN*bArrayNum;
			}
			break;	
		default:
			if (pbField[0] == DT_NULL)	//��Ч���ݷ���NULL
				iRet = 1;
			else
				return wFieldLen;
	}

	if ((iRet>0) && (iRet<=wFieldLen))
		return iRet;
	else
		return 1;
}

int OoProReadEvtRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize)
{
	BYTE bType, bOadBuf[10], bBuf[EVT_ATTRTAB_LEN];
	BYTE bTmpRecBuf[EVTREC_MAXSIZE];	//һ����¼������
	BYTE* pbTmpRec = bTmpRecBuf;
	BYTE* pbRec = pbRecBuf;
	WORD wItemOffset, wItemLen, wTotalLen;
	DWORD dwOAD;
	int iLen;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	TTermEvtCtrl* pEvtCtrl;
	const ToaMap* pOaMap = NULL;
	BYTE bIndex;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return -1;

	//��ȡ�̶��ֶκ������ֶ�
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return -1;
	}

	//�����������ͺ�Ԫ�ظ���
	wTotalLen = 0;
	*pbTmpRec++ = DT_STRUCT;
	wTotalLen++;
	*pbTmpRec++ = tFixFields.wNum+tDataFields.wNum;
	wTotalLen++;

	//����̶��ֶε��¼�����Դ���ϱ���Ϣ���¼������б����������
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//��������
		iLen = GetEvtSpecField(pEvtCtrl, dwOAD, pbRec, wItemLen);
		if ((iLen>0) && (iLen<=wItemLen))
			memcpy(pbTmpRec, pbRec, iLen);
		else 
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}
	//���������ֶ�
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//��������
		iLen = GetEvtSpecField(pEvtCtrl, dwOAD, pbRec, wItemLen);
		if ((iLen>0) && (iLen<=wItemLen))
			memcpy(pbTmpRec, pbRec, iLen);
		else 
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}
	//memcpy(pbTmpRec, pbRec, tDataFields.wTotalLen);
	//wTotalLen +=  tDataFields.wTotalLen;
	if (wTotalLen <= wBufSize)
	{	
		memcpy(pbRecBuf, bTmpRecBuf, wTotalLen);
		//TrigerSaveBank(BN0, SECT3, -1);
		return wTotalLen;
	}			

	return -1;
}


int OoProRptEvtRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize)
{
	BYTE bType, bOadBuf[10], bBuf[EVT_ATTRTAB_LEN];
	BYTE bTmpRecBuf[EVTREC_MAXSIZE];	//һ����¼������
	BYTE* pbTmpRec = bTmpRecBuf;
	BYTE* pbRec = pbRecBuf;
	WORD wItemOffset, wItemLen, wTotalLen;
	DWORD dwOAD;
	int iLen;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	TTermEvtCtrl* pEvtCtrl;
	const ToaMap* pOaMap = NULL;
	BYTE bIndex;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return -1;

	//��ȡ�̶��ֶκ������ֶ�
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return false;
	}

	wTotalLen = 0;
	
	// 4���ֽڵ��¼���¼OAD
	OoDWordToOad(GetOAD(wOI, bAttr, 0), pbTmpRec);
	pbTmpRec += 4;wTotalLen+= 4;
	
	// 1�ֽ�Ԫ�ظ���
	*pbTmpRec++ = tFixFields.wNum+tDataFields.wNum;wTotalLen++;
	
	// 5�ֽ�ÿ��Ԫ������OAD*Ԫ�ظ���
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		//ѡ��OAD����
		*pbTmpRec++ = 0;wTotalLen++;
		//OAD
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}		
		memcpy(pbTmpRec, bOadBuf+1, 4);
		pbTmpRec += 4;
		wTotalLen += 4;
	}	
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		//ѡ��OAD����
		*pbTmpRec++ = 0;wTotalLen++;
		//OAD
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}		
		memcpy(pbTmpRec, bOadBuf+1, 4);
		pbTmpRec += 4;
		wTotalLen += 4;	
	}

	// 1�ֽڽ��
	*pbTmpRec++ = 1;wTotalLen++;
	// 1�ֽڽ��������1��
	*pbTmpRec++ = 1;wTotalLen++;

	//��������	
	//����̶��ֶε��¼�����Դ���ϱ���Ϣ���¼������б����������
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//��������
		iLen = GetEvtSpecField(pEvtCtrl, dwOAD, pbRec, wItemLen);
		if ((iLen>0) && (iLen<=wItemLen))
			memcpy(pbTmpRec, pbRec, iLen);
		else 
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}
	//���������ֶ�
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//��������
		iLen = GetEvtSpecField(pEvtCtrl, dwOAD, pbRec, wItemLen);
		if ((iLen>0) && (iLen<=wItemLen))
			memcpy(pbTmpRec, pbRec, iLen);
		else 
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}
	//memcpy(pbTmpRec, pbRec, tDataFields.wTotalLen);
	//wTotalLen +=  tDataFields.wTotalLen;
	if (wTotalLen <= wBufSize)
	{	
		memcpy(pbRecBuf, bTmpRecBuf, wTotalLen);
		//TrigerSaveBank(BN0, SECT3, -1);
		return wTotalLen;
	}			

	return -1;
}



//��������ȡ�¼���¼��ı������ṩ�����нӿ�
char* GetEvtRecFileName(DWORD dwROAD)
{
	const ToaMap* pOaMap = NULL;
	pOaMap = GetOIMap(dwROAD);

	if (pOaMap == NULL)
		return NULL;
	
	return pOaMap->pszTableName;
}

//��������ȡ�¼���¼��Ĺ̶��ֶκ������ֶΣ��ṩ�����нӿ�
bool GetEvtRecFieldParser(DWORD dwROAD, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize)
{
	WORD wOI;
	TTermEvtCtrl* pEvtCtrl;
	
	GetOIAttrIndex(dwROAD, &wOI, NULL, NULL);
	
	//��ȡ�¼����ƽṹ
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return false;	
	
	//��ȡ�̶��ֶκ������ֶ�
	if (GetEvtFieldParser(pEvtCtrl, pFixFields, pDataFields, pbAtrrTabBuf, wBufSize) == false)
	{	
		DTRACE(DB_INMTR, ("GetEvtFieldParser: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return false;
	}

	DelEvtOad(dwROAD, 0);
	TrigerSaveBank(BN0, SECT3, -1);
	return true;
}

//����������һ���¼���¼���ṩ�����нӿ�
//������@wOI 	�����ʶ
//		@bAtrr	���Ա�ʶ�������� bit-string��SIZE��8����
//		@bIndex������Ԫ������
//		@pbRecBuf��¼���ջ�����
//		@wBufSize��¼���ջ������Ĵ�С
//���أ���ȷ���ؼ�¼�ĳ��ȣ����򷵻ظ���
int GetEvtRecord(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbRecBuf, WORD wBufSize)
{
	DWORD dwROAD;
	int iLen;
	char* pszFileName;
	BYTE bBuf[5];
	WORD wMaxNum, wTotalLen = 0;
	const TEvtAttr* pEvtAttr; 
	TTermEvtCtrl* pEvtCtrl;
	
	dwROAD= GetOAD(wOI, bAttr, bIndex);
	
	pszFileName = GetEvtRecFileName(dwROAD&0xffff1f00);
	if (pszFileName == NULL)
		return -1;

	DelEvtOad(dwROAD, 0);

	if (bIndex == 0)	//����ȫ�����м�¼
	{
		pEvtCtrl = GetTermEvtCtrl(wOI);
		if (pEvtCtrl == NULL)
			return -1;

		//��ȡ�¼�����
		pEvtAttr = GetEvtAttr(pEvtCtrl);
		if (pEvtAttr == NULL)
			return -1;

		//��ȡ����¼��
		if (OoReadAttr(wOI, pEvtAttr->bMaxRecNum, bBuf, NULL, NULL) <= 0)
			return -1;
		wMaxNum = OoLongUnsignedToWord(bBuf+1);	
		if (wMaxNum == 0)	//����¼��Ϊ0,�����ݿɶ�ȡ
			return -1;

		//�����������ͺ�Ԫ�ظ���
		pbRecBuf[0] = DT_ARRAY;
		wTotalLen = 2;

 		for(bIndex=1; bIndex<=wMaxNum; bIndex++)
 		{
			// ��ȡ��¼
			iLen = ReadLastNRec(pszFileName, bIndex, pbRecBuf+wTotalLen, wBufSize);
			if (iLen<=0)	
			{	
				if (bIndex==1) //�޼�¼
					return iLen;
				else
					break;	//�Ѷ����������˳�
			}
			if ((wTotalLen+iLen) > wBufSize)	//�����ѳ�
				return -1;
			iLen = OoProReadEvtRecord(wOI, bAttr, pbRecBuf+wTotalLen, iLen, wBufSize);
			if (iLen<=0)
				return iLen;
			else
				wTotalLen += iLen;
		}
		pbRecBuf[1] = bIndex-1;	//��������
		return wTotalLen;
	}
	else
	{
		// ��ȡ��¼
		iLen = ReadLastNRec(pszFileName, bIndex, pbRecBuf, wBufSize);
		if (iLen <= 0)
			return iLen;
		return OoProReadEvtRecord(wOI, bAttr, pbRecBuf, iLen, wBufSize);
	}
}

//�������¼����������Ҫ���³�ʼ���¼����ṩ�����нӿ�
//������@dwOAD���ݱ�ʶ
//���أ���
void ReInitMrtEvtPara(DWORD dwOAD)
{
	BYTE bAttr, bClrFlag;
	WORD wOI, wPn;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 

	GetOIAttrIndex(dwOAD, &wOI, &bAttr, NULL);
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return;		
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;		

	//����¼�����������Ա��б��ʱ��Ҫ���¼������³�ʼ��
	if ((bAttr == pEvtAttr->bMaxRecNum) || (bAttr == pEvtAttr->bRela))
	{
		if (GetEvtSn(wOI) >= 0)
		{	
			wPn = GetEvtSn(wOI);
			bClrFlag = EVT_CLR_VALID;
			WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);		//д�����ʶ
			TrigerSaveBank(BN11, 0, -1);
		}
	}
}

//**************************�¼���������*********************************************
//����1����λ
int DoTermEvtMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bClrFlag;
	WORD wPn;
	TTermEvtCtrl* pEvtCtrl;

	if ((bMethod!=EVT_RESET) || (iParaLen != 0x02))	
		goto END_ERR;
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;
	
	if ((pbPara[0] == DT_INT) &&(pbPara[1] == 0x00))
	{
		if (GetEvtSn(wOI) >= 0)
		{	
			wPn = GetEvtSn(wOI);
			SetInfo(INFO_EVT_EVTRESET);
			if (wOI == MTR_EVTCLEAR)
				SetInfo(INFO_EVT_CLREVTRESET);	
			bClrFlag = EVT_CLR_VALID;
			WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);	
			TrigerSaveBank(BN11, 0, -1);
			GetEvtClearOMD(wOI, bMethod, bOpMode);
			*pbRes = 0;	//�ɹ�  ��0�� 
			return 0;
		}
	}
	
END_ERR:
	*pbRes = 3;	
	return -1;
}

//����2��ִ��
//�պ���
int DoTermEvtMethod2(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTermEvtCtrl* pEvtCtrl;

	if (bMethod != EVT_RUN)	
		goto END_ERR;
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;

	//nothing to do

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;

END_ERR:
	*pbRes = 3;	
	return -1;	
}



//����3������һ�μ�¼
//����һ�μ�¼���¼�����Դ��������ʱʱ�䣬�ָ���ʱʱ�䣩
//�¼�����Դ��=instance-specific
//������ʱʱ���=long-unsigned 
//�ָ���ʱʱ���=long-unsigned
int DoTermEvtIC7Method3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
/*	BYTE bBuf[EVT_TRIG_PARA_LEN];
	int iLen;
	WORD wPn;
	TTermEvtCtrl* pEvtCtrl;

	if (bMethod!=EVT_TRIG) 	
		goto END_ERR;
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;
	if (pEvtCtrl->bClass != IC7	)
		goto END_ERR;
	iLen =  GetEvtSrcData(pEvtCtrl, pbPara, 0);	//ֻ��ȡ����
	if (iLen <= 0)
		goto END_ERR;
	//if ( (iParaLen!=(6+iLen)) || (pbPara[0]!=pEvtCtrl->pbSrcFmt[0]) || (pbPara[iLen]!=DT_LONG_U) || (pbPara[iLen+3]!=DT_LONG_U))	
	//	goto END_ERR;

	if ((pbPara[0]!=pEvtCtrl->pbSrcFmt[0]) || (pbPara[iLen]!=DT_LONG_U) || (pbPara[iLen+3]!=DT_LONG_U))	
		goto END_ERR;
	
	memset(bBuf, 0x00, sizeof(bBuf));
	memcpy(bBuf, pbPara, (6+iLen));

	if (GetEvtSn(wOI) >= 0)
	{	
		wPn = GetEvtSn(wOI);	
		WriteItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);		//д�����ʶ
		TrigerSaveBank(BN11, 0, -1);
		if (wOI == MTR_EVTCLEAR)	
			GetEvtClearOMD(MTR_EVTCLEAR, 0x03, 0x00);	
	}

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
	
END_ERR:*/
	*pbRes = 3;	
	return -1;	
}

//����3������һ�μ�¼
//����һ�μ�¼���¼���𣬴�����ʱʱ�䣬�ָ���ʱʱ�䣩
//�¼�����=enum
//{
//	�¼���¼1��0����
//	�¼���¼2��1����
//	�¼���¼3��2����
//	�¼���¼4��3��
//}
//������ʱʱ���=long-unsigned 
//�ָ���ʱʱ���=long-unsigned
int DoTermEvtIC24Method3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
/*	BYTE bItem, bBuf[EVT_TRIG_PARA_LEN];
	WORD wPn;
	TTermEvtCtrl* pEvtCtrl;

	//if ((bMethod!=EVT_TRIG) || (iParaLen!=8) || (pbPara[0]!=DT_ENUM) || (pbPara[2]!=DT_LONG_U) || (pbPara[5]!=DT_LONG_U))		
	//	goto END_ERR;

	if ((bMethod!=EVT_TRIG) || (pbPara[0]!=DT_ENUM) || (pbPara[2]!=DT_LONG_U) || (pbPara[5]!=DT_LONG_U))		
		goto END_ERR;
	
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;
	if (pEvtCtrl->bClass != IC24)
		goto END_ERR;
	bItem = pbPara[1];
	if (bItem > 3)
		goto END_ERR;
	if (pEvtCtrl->bItemNum == 3)
	{
		if (bItem == 0)
			goto END_ERR;
	}
	
	if (GetEvtSn(wOI) >= 0)
	{	
		wPn = GetEvtSn(wOI);	
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);	
		memcpy(bBuf+bItem*8, pbPara, 8);
		WriteItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);		//д�����ʶ
		TrigerSaveBank(BN11, 0, -1);
	}

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
	
END_ERR:*/
	*pbRes = 3;	
	return -1;	
}


//����4�����һ����������������ԣ�������
//������=FRZRELA ��������������
int DoTermEvtMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bAttrBuf[EVT_ATTRTAB_LEN];	//�������Ա�����
	BYTE bAttrOADNum, bIndex;
	BYTE bClrFlag;
	WORD wPn;
	int iLen;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 

	if ((bMethod!=EVT_ADDATTR) || (iParaLen!=5) || (pbPara[0]!=DT_OAD))
		goto END_ERR;
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		goto END_ERR;	

	//�������OAD
	iLen = OoReadAttr(wOI, pEvtAttr->bRela, bAttrBuf, NULL, NULL);
	if ((iLen<=0) || (iLen > sizeof(bAttrBuf)) || (bAttrBuf[0]!=DT_ARRAY) || (bAttrBuf[1]==0x00))	//����������������Ϊ�գ���������OAD
	{
		memset(bAttrBuf, 0x00, sizeof(bAttrBuf));
		bAttrBuf[0] = DT_ARRAY;
		bAttrBuf[1] = 1;
		memcpy(bAttrBuf+2, pbPara, 5);
		goto END_OK;
	}
	else if (bAttrBuf[1] ==CAP_OAD_NUM)	//�Ѵ���
		goto END_ERR;	

	bAttrOADNum = bAttrBuf[1];
	for (bIndex=0; bIndex<bAttrOADNum; bIndex++)
	{
		if (memcmp(&bAttrBuf[2+bIndex*5], pbPara, 5) == 0)	//�Ѵ���OAD
			goto END_OK_OK;	
	}
	if (bIndex == bAttrOADNum)
	{
		if (iLen+5 > sizeof(bAttrBuf))
			goto END_ERR;	
		bAttrBuf[1]++;
		memcpy(&bAttrBuf[iLen], pbPara, 5);
		goto END_OK;
	}

END_ERR:
	*pbRes = 3;	
	return -1;	
	
END_OK:
	if (OoWriteAttr(wOI, pEvtAttr->bRela, bAttrBuf) < 0)
	{
		*pbRes = 3;	
		return -1;	
	}
	if (GetEvtSn(wOI) >= 0)
	{	
		wPn = GetEvtSn(wOI);	
		bClrFlag = EVT_CLR_VALID;
		WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);		//д�����ʶ
		TrigerSaveBank(BN11, 0, -1);
	}
END_OK_OK:
	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}

//����5��ɾ��һ������������ԣ�������
//������=OAD ��������������
int DoTermEvtMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bAttrBuf[EVT_ATTRTAB_LEN];	//�������Ա�����
	BYTE bAttrOADNum, bIndex;
	BYTE bClrFlag;
	WORD wPn;
	int iLen;
	bool fDelFlag = false;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 

	if ((bMethod!=EVT_DELATTR)	|| (iParaLen!=5) || (pbPara[0]!=DT_OAD))
		goto END_ERR;	
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;		
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		goto END_ERR;	

	//����ɾ��
	iLen = OoReadAttr(wOI, pEvtAttr->bRela, bAttrBuf, NULL, NULL);
	if ((iLen<=0) || (bAttrBuf[0]!=DT_ARRAY) || (bAttrBuf[1]==0x00))	//����������������Ϊ�գ��޷�ɾ��
		goto END_ERR;	

	bAttrOADNum = bAttrBuf[1];
	for (bIndex=0; bIndex<bAttrOADNum; bIndex++)
	{
		if (memcmp(&bAttrBuf[2+bIndex*5], pbPara, 5) == 0)	//�������У��Ѵ��ڵ�OAD��ɾ��
		{
			memcpy(&bAttrBuf[2+bIndex*5], &bAttrBuf[2+(bIndex+1)*5], 5*(bAttrOADNum-bIndex));
			memset(&bAttrBuf[2+bAttrOADNum*5], 0x00, 5);
			bAttrBuf[1]--;bAttrOADNum--;bIndex--;
			fDelFlag = true;
		}	
	}
	if (fDelFlag)
		goto END_OK;
	else
		goto END_OK_OK;	//δ�鵽Ҳ�ظ�ȷ��֡����ʾ��ȷ��ɴ���

END_ERR:
	*pbRes = 3;	
	return -1;	
	
END_OK:
	OoWriteAttr(wOI, pEvtAttr->bRela, bAttrBuf);
	if (GetEvtSn(wOI) >= 0)
	{	
		wPn = GetEvtSn(wOI);
		bClrFlag = EVT_CLR_VALID;
		WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);		//д�����ʶ
		TrigerSaveBank(BN11, 0, -1);
	}
END_OK_OK:
	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}


//**************************�����ϱ�����*********************************************
//��������ĳ��ͨ�������¼��ϱ���Ϣ
//������@dwCnOAD ͨ��OAD
//		@dwOAD�¼�OAD
//		@wRecIdx������еļ�¼����
//		@bStage�����׶�
//���أ���ȷ����true�����򷵻�false
bool SendEvtMsg(DWORD dwCnOAD, DWORD dwEvtOAD,WORD wRecIdx, BYTE bStage, BYTE bSchNo, WORD wIdex, BYTE* pbROAD, WORD wRoadLen)
{
	BYTE bBuf[2] = {0};
	TEvtMsg tEvtMsg;

	if (OoReadAttr(0x4300, 8, bBuf, NULL, NULL) > 0)	
	{
		if ((bBuf[0]==DT_BOOL) && (bBuf[1]==1))		//�ϱ������ŷ���Ϣ
		{
		}		
		else
		{
			return false;
		}
	}
	else
		return false;

	memset(&tEvtMsg, 0, sizeof(tEvtMsg));

	tEvtMsg.dwOAD = dwEvtOAD|1;	//����Ϊ��1��
	tEvtMsg.wRecIdx = wRecIdx;
	tEvtMsg.bStage = bStage;

	if (bStage==EVT_STAGE_TASK || (bStage==EVT_STAGE_ERCRPT && (dwEvtOAD&0xFF000000)==0x30000000))
	{
		tEvtMsg.bSchNo = bSchNo;
		tEvtMsg.wIdex = wIdex;
		DWORD dwOAD = 0x60120300;
		BYTE* pbPtr = tEvtMsg.bRcsd;
		pbPtr += OoDWordToOad(dwOAD, pbPtr);
		*pbPtr++ = 2; //RCSD����
		*pbPtr++ = 0; //OAD
		pbPtr += OoDWordToOad(0x202A0200, pbPtr);
		*pbPtr++ = 1; //ROAD
		if (sizeof(tEvtMsg.bRcsd) - (pbPtr - tEvtMsg.bRcsd) >= wRoadLen)
		{
			memcpy(pbPtr, pbROAD, wRoadLen);
			pbPtr += wRoadLen;
		}
		tEvtMsg.wRcsdLen = pbPtr - tEvtMsg.bRcsd;
	}
	else if (bStage == EVT_STAGE_ERCRPT)
	{
		tEvtMsg.bSchNo = bSchNo;
		tEvtMsg.wIdex = wIdex;
		BYTE* pbPtr = tEvtMsg.bRcsd;
		*pbPtr++ = DT_OAD;
		pbPtr += OoDWordToOad(dwEvtOAD, pbPtr);
		*pbPtr++ = DT_OCT_STR;
		pbPtr += EncodeLength(wRoadLen, pbPtr);
		if (sizeof(tEvtMsg.bRcsd) - (pbPtr - tEvtMsg.bRcsd) >= wRoadLen)
		{
			memcpy(pbPtr, pbROAD, wRoadLen);
			pbPtr += wRoadLen;
		}

		tEvtMsg.wRcsdLen = pbPtr - tEvtMsg.bRcsd;
	}

	if ((dwCnOAD&0xfff00000) == 0x45000000)	//GPRSͨ��
	{
#ifndef SYS_WIN		
		g_pGprsFaProto.AppendEvtMsg(&tEvtMsg);
#else
		g_pEthFaProto.AppendEvtMsg(&tEvtMsg);
#endif
	}
	else if ((dwCnOAD&0xfff00000) == 0x45100000)	//��̫��ͨ��
		g_pEthFaProto.AppendEvtMsg(&tEvtMsg);
	else if ((dwCnOAD&0xffff0000) == 0xF2010000)
		g_FapTest.AppendEvtMsg(&tEvtMsg);
	else if ((dwCnOAD&0xffff0000) == 0xF2020000)
		g_FapLocal.AppendEvtMsg(&tEvtMsg);
	else
		return false;

	DTRACE(DB_INMTR, ("***EVTRPT****EVTRPT****EVTRPT***SendEvtMsg: dwOAD=%x,  wRecIdx=%u, bStage=%d.\r\n", tEvtMsg.dwOAD,  tEvtMsg.wRecIdx, tEvtMsg.bStage));	
	
	return true;
}

//������ȡ�ϱ��¼��ļ�¼
//������@pEvtMsg�¼��ϱ���Ϣ
//		@pbRecBuf��¼���ջ�����
//		@wBufSize��¼���ջ������Ĵ�С
//		@bType bType = 0�Դ洢������������ʹ��������Э�飬bType != 0�������洢����
//���أ���ȷ���ؼ�¼�ĳ��ȣ����򷵻ظ���
int GetEvtRec(TEvtMsg* pEvtMsg, BYTE* pbRecBuf, WORD wBufSize, BYTE bType)
{
	BYTE bAttr;
	WORD wOI;
	int iLen;
	char* pszFileName;
	char szTableName[32];
	TTermEvtCtrl* pEvtCtrl;

	if (pEvtMsg->bStage == 0)
		return -1;

	if (pEvtMsg->bStage == EVT_STAGE_TASK)
	{
		memset(szTableName, 0, sizeof(szTableName));
		GetEvtTaskTableName(pEvtMsg->bSchNo, pEvtMsg->wIdex, szTableName);
		pszFileName = szTableName;
	}
	else if (pEvtMsg->bStage == EVT_STAGE_ERCRPT)
	{
		if (wBufSize < pEvtMsg->wRcsdLen)
		{
			DTRACE(DB_FAPROTO, ("GetEvtRec: no space.\r\n"));
			return -1;
		}
		memcpy(pbRecBuf, pEvtMsg->bRcsd, pEvtMsg->wRcsdLen);

		return pEvtMsg->wRcsdLen;
	}
	else
	{
		pszFileName = GetEvtRecFileName(pEvtMsg->dwOAD&0xffff1f00);
	}

	if (pszFileName == NULL)
		return -1;

	if (bType) //������ͨѶʹ�ã�����Ҫ����������
		return  ReadRecByPhyIdx(pszFileName, pEvtMsg->wRecIdx, pbRecBuf, wBufSize);
	
	iLen = ReadRecByPhyIdx(pszFileName, pEvtMsg->wRecIdx, pbRecBuf, wBufSize);
	if (iLen <= 0)
		return iLen;

	if (pEvtMsg->bStage == EVT_STAGE_TASK)
	{
		WORD wOff = GetEvtTaskTableFixFieldLen();
		if (iLen > (wOff+7))
		{
			iLen -= wOff;
			WORD wDataLen;
			memcpy(&wDataLen, &pbRecBuf[wOff], 2);
			wOff += 2;
			iLen -= 2;
			if (pbRecBuf[wOff] == DT_DB_LONG_U) //ȫ�¼�����
			{
				wOff += 5;
				iLen -= 5;
				wDataLen -= 5;
			}

			BYTE bAddr[17];
			BYTE bAddrLen = pbRecBuf[0];
			BYTE bAddrLenBak = bAddrLen;
			memcpy(bAddr, pbRecBuf+1, bAddrLen);
			pbRecBuf[0] = DT_TSA;
			pbRecBuf[1] = bAddrLen + 1;
			pbRecBuf[2] = bAddrLen - 1;
			memcpy(pbRecBuf+3, bAddr, bAddrLen);
			bAddrLen += 3;
			pbRecBuf[bAddrLen++] = DT_ARRAY;
			pbRecBuf[bAddrLen++] = pEvtMsg->bRcsd[15];
			if (wDataLen > iLen)
				wDataLen = 1;
#if 1
			TOobMtrInfo tMtrInfo;
			GetMeterInfo(bAddr, bAddrLenBak, &tMtrInfo);
			if (tMtrInfo.bProType != PRO_TYPE_69845)	//645�����������⴦��
			{
				BYTE *pbSrc = pbRecBuf+wOff;
				BYTE *pbDst = pbRecBuf+bAddrLen;
				BYTE bRcsd[256];
				BYTE *pRcsd = pEvtMsg->bRcsd+11;
				WORD wRcsdLen = pEvtMsg->wRcsdLen-11;
				int iRet;

				bRcsd[0] = DT_ROAD;
				memcpy(bRcsd+1, pRcsd, wRcsdLen);
				iRet = OoFormatSrcData(pbSrc, wDataLen, bRcsd, wRcsdLen+1, pbDst);
				if (iRet < 0)
				{
					*pbDst = 0x00;
					iRet = 1;
				}
				iLen = iRet + bAddrLen;
			}
			else
			{
				memmove(pbRecBuf+bAddrLen, pbRecBuf+wOff, wDataLen);
				iLen = wDataLen + bAddrLen;
			}
#else
			
			memmove(pbRecBuf+bAddrLen, pbRecBuf+wOff, wDataLen);
			iLen = wDataLen + bAddrLen;
#endif
		}
		return iLen;
	}

	GetOIAttrIndex(pEvtMsg->dwOAD, &wOI, &bAttr, NULL);

	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl != NULL)
		return OoProRptEvtRecord(wOI, bAttr, pbRecBuf, iLen , wBufSize);
	else
		return OoProRptMtrExcRecord(wOI, bAttr, pbRecBuf, iLen , wBufSize);		//�����¼���֡����
}

//�����������¼���¼���ϱ�״̬
//������@dwCnOAD ͨ��OAD
//		@pEvtMsg�¼��ϱ���Ϣ
//		@bRptState Ҫ�õı�־λ����¼�е�ͨ���ϱ�״̬��ԭ��ֵ��������ֵ
//���أ������ȷ�򷵻�true,���򷵻�false
bool UpdateEvtRptState(DWORD dwCnOAD, TEvtMsg* pEvtMsg, BYTE bRptState)
{
	BYTE bAttr, bItem, bIndex, bType, bCnNum;
	BYTE bOadBuf[10];
	BYTE pbRecBuf[EVTREC_MAXSIZE];
	BYTE* pbRec = pbRecBuf;
	WORD wOI, wItemOffset, wItemLen;
	DWORD dwOAD, dwRecCnOAD;
	int iLen, nIndex = 0;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 
	TFieldParser tFixFields;
	BYTE i;
	char* pszFileName;

	if (pEvtMsg->bStage == EVT_STAGE_TASK)
		return false;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	GetOIAttrIndex(pEvtMsg->dwOAD, &wOI, &bAttr, NULL);

	DelEvtOad(0, wOI);
	
	//��ȡ�¼����ƽṹ
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
	{	
		nIndex = GetMtrExcIndex(wOI);
		if (nIndex < 0)
			return false;	
		else
			return UpdateMtrExcRptState(dwCnOAD, pEvtMsg, bRptState);
	}
	
	//��ȡ�¼�����
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;

	//��ȡ�¼������
	if (bAttr >= pEvtAttr->bRecTabStart)
		bItem = bAttr-pEvtAttr->bRecTabStart;
	else 
		return false;
	
	//ȡ���ϱ��¼��ļ�¼
	memset(pbRecBuf, 0, sizeof(pbRecBuf));
	iLen = GetEvtRec(pEvtMsg, pbRecBuf, sizeof(pbRecBuf), 1);
	if (iLen <= 0)
		return false;
	
	//��ȡ�̶��ֶ�
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, NULL, NULL, 0) == false)
		return false;
	if (tFixFields.wNum == 0)
		return false;

	//����ˢ��ͨ���ϱ�״̬
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	//�̶��ֶθ���
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
			return false;
		if (bType != DT_OAD) 
			return false;
		if (wItemLen == 0) 
			return false;
		dwOAD = OoOadToDWord(bOadBuf+1);
		if (dwOAD == 0x33000200)	//ͨ���ϱ�״̬ˢ��
		{
			if (pbRec != DT_NULL)
			{
				bCnNum = *(pbRec+1);
				if (bCnNum >= CN_RPT_NUM)
					bCnNum = CN_RPT_NUM;
				for(i=0; i<bCnNum; i++)
				{	
					dwRecCnOAD = OoDoubleLongUnsignedToDWord(pbRec+5+i*9);
					if ((dwCnOAD&0xfff00000) == 0x45000000)	//Ҫ�뺯��SendEvtMsg()��ƥ��
					{
						dwRecCnOAD &=0xfff00000;
						dwCnOAD &=0xfff00000;
					}
					else if	((dwCnOAD&0xfff00000) == 0x45100000)
					{	
						dwRecCnOAD &=0xfff00000;
						dwCnOAD &=0xfff00000;
					}
					else
					{	
						dwRecCnOAD &=0xffff0000;
						dwCnOAD &=0xffff0000;
					}
					if (dwCnOAD == dwRecCnOAD)
						*(pbRec+10+i*9) |= bRptState;
				}
			}
		}
		pbRec += wItemLen;
	}
	pszFileName = GetEvtRecFileName(pEvtMsg->dwOAD&0xffff1f00);
	if (pszFileName == NULL)
		return false;

	//if (bRptState&0x0a)
	//	AddEvtOad(pEvtMsg->dwOAD, 1);	//���ϱ�

	//������¼
	SaveRecordByPhyIdx(pszFileName, pEvtMsg->wRecIdx, pbRecBuf);
	return true;
}



//������ң��״̬����λ�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitYXEvtCtrl(struct TEvtCtrl* pEvtCtrl)
{
	TYXChgCtrl* pCtrl = (TYXChgCtrl* )pEvtCtrl->pEvtPriv;
	if (pCtrl == NULL)
		return false;

	memset((BYTE*)pCtrl, 0, sizeof(TYXChgCtrl));
	return InitEvt(pEvtCtrl);
}



//������ң��״̬����λ�жϣ�
int DoYXChgJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bIsValid, bValidFlag;
	BYTE bStaByte = 0;//ң��״̬��־
	BYTE bPreByte = 0; //ң��״̬�ɱ�־
	BYTE bChgByte = 0;//״̬��λ��־��
	BYTE bBuf[20];
	BYTE bDoorStat = 0x10;//�Žڵ�״̬
	TTime time;
	TYXChgCtrl* pCtrl = (TYXChgCtrl* )pEvtCtrl->pEvtPriv;
	if (pCtrl == NULL)
		return 0;

    memset(&time, 0, sizeof(time));
	if (IsAcPowerOff(&bIsValid) || bIsValid == 1)
		return 0;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)	//��Ч��־
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;

	if (!bValidFlag)
	{
		DTRACE(DB_INMTR, ("MtrClrJudge: ValidFlag=0.\r\n"));
		return 0;
	}

#ifdef SYS_LINUX
	bDoorStat = GetDoorStatus(); //�Žڵ��״̬Ϊ�ߵ�ƽ
#endif

	int nRead = ReadItemEx(BN2, PN0, 0x1100, &bStaByte);
    if (nRead <= 0)
    	return -1;

	if(bDoorStat > 0)	//�����Žڵ�״̬����λ�¼�
		bDoorStat = 0x00;
	else
		bDoorStat = 0x10;

	bStaByte = (bStaByte&0xef) | bDoorStat;
    	
    //bStaByte >>= 4; //ȡң��״̬������λ����

	if (!pCtrl->fInit)	//��һ�ν�����ȡң��״̬λ�����жϣ�
    {
		DTRACE(DB_TASK, ("DoYXChgJudge: YX Power On Init, bStaByte=%x.\r\n", bStaByte));
        pCtrl->bStaByte = bStaByte;
        pCtrl->fInit = true;
        return 0;
    }
 
	bChgByte = bStaByte ^ pCtrl->bStaByte;
	pCtrl->bStaByte = bStaByte;

	if (bChgByte != 0)	//״̬�ı������λ�¼�
	{ 
		if (pEvtCtrl->pEvtBase[0].bJudgeState != EVT_JS_HP)		//�����¼�
		{
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			pEvtCtrl->bDelaySec = 2;
			DTRACE(DB_TASK, ("DoYXChgJudge: YX Power On Init, bJudgeState=%d, bState=%d, Click=%x.\r\n", pEvtCtrl->pEvtBase[0].bJudgeState, pEvtCtrl->pEvtBase[0].bState, GetClick()));	
		}
	}
	else
	{	
		if (pEvtCtrl->pEvtBase[0].bState == EVT_S_AFT_HP)	//�����¼���û�м�¼����ԭbJudgeState״̬ 
		{
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
			pEvtCtrl->bDelaySec = 0;	
			DTRACE(DB_TASK, ("DoYXChgJudge: YX Power On Init, bJudgeState=%d, bState=%d, Click=%x.\r\n", pEvtCtrl->pEvtBase[0].bJudgeState, pEvtCtrl->pEvtBase[0].bState, GetClick()));
		}
	}
	//DTRACE(DB_TASK, ("DoYXChgJudge: YX Power On Init, bJudgeState=%d, bState=%d, Click=%x.\r\n", pEvtCtrl->pEvtBase[0].bJudgeState, pEvtCtrl->pEvtBase[0].bState, GetClick()));
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}


//����ʼ��������������˽�б���ǿ���ر�������������������������������������������������������������������������������������������������������������
//�������豸���ϼ�¼�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitDeviceErr(struct TEvtCtrl* pEvtCtrl)
{
	TDeviceErr* pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TDeviceErr));
	return InitEvt(pEvtCtrl);
}

//����������δ֪���ܱ��¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitUnKnMtr(struct TEvtCtrl* pEvtCtrl)
{
	TUnKnMtr* pEvtPriv = (TUnKnMtr* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TUnKnMtr));
	return InitEvt(pEvtCtrl);
}

//��������̨�����ܱ��¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitStepArea(struct TEvtCtrl* pEvtCtrl)
{
	TStepArea* pEvtPriv = (TStepArea* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TStepArea));
	return InitEvt(pEvtCtrl);
}

//������ң����բ��¼�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitYKCtrl(struct TEvtCtrl* pEvtCtrl)
{
	TYKCtrl* pEvtPriv = (TYKCtrl* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TYKCtrl));
	return InitEvt(pEvtCtrl);
}

//�������й��ܵ������Խ���¼���¼
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitEpOver(struct TEvtCtrl* pEvtCtrl)
{
	TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TEpOver));
	return InitEvt(pEvtCtrl);
}

//�������ն˱�̼�¼�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitTermPrg(struct TEvtCtrl* pEvtCtrl)
{
	TTermPrg* pEvtPriv = (TTermPrg* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TTermPrg));
	return InitEvt(pEvtCtrl);
}
//�������ն˵�����·�쳣�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitCurCirc(struct TEvtCtrl* pEvtCtrl)
{
	TCurCirc* pEvtPriv = (TCurCirc* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TCurCirc));
	return InitEvt(pEvtCtrl);
}

//�������ն˶Ե��Уʱ��¼��ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitMtrClkPrg(struct TEvtCtrl* pEvtCtrl)
{
	TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TMtrClkPrg));
	return InitEvt(pEvtCtrl);
}

//������������բ��¼����ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitPowCtrl(struct TEvtCtrl* pEvtCtrl)
{
	TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TPowCtrl));
	return InitEvt(pEvtCtrl);
}
//�����������բ��¼�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitEleCtrl(struct TEvtCtrl* pEvtCtrl)
{
	TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TEleCtrl));
	return InitEvt(pEvtCtrl);
}
//����������������ü�¼�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitPurchParaChg(struct TEvtCtrl* pEvtCtrl)
{
	TPurchParaChg* pEvtPriv = (TPurchParaChg* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TPurchParaChg));
	return InitEvt(pEvtCtrl);
}
//��������ظ澯�¼���¼�¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitEleAlram(struct TEvtCtrl* pEvtCtrl)
{
	TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0, sizeof(TEleAlram));
	return InitEvt(pEvtCtrl);
}

//���жϺ���ʵ��ǿ����ӡ�����������������������������������������������������������������������������������������������������������
//�������ն˳�ʼ���¼��жϣ�
int TermInitJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_TERM_INIT);
}
//�������ն˰汾����¼��жϣ�
int TermVerChgJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_TERM_VER_CHG);
}

//�������ն���Ϣ��֤�����¼��жϣ�
int GsgQAuthJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ESAM_AUTH_FAIL);
}
//�������豸���ϼ�¼�жϣ�
int DeviceErrJudge(struct TEvtCtrl* pEvtCtrl)
{
	int nRet = 0;

	nRet = OnInfoTrigerEvtJudge(pEvtCtrl, INFO_DEVICE_485_ERR);
	if (nRet != 1)	//��1���¼��������Ȳ��ж������¼�
		nRet = OnInfoTrigerEvtJudge(pEvtCtrl, INFO_DEVICE_CCT_ERR);

    return nRet;
}
//��������ͨ�����������¼��жϣ�
int FluxOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag;
	BYTE bBuf[60];
	DWORD dwMonthFluxLimit = 0;
	DWORD dwCurMonthFlux = 0;

	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)		//��Ч��־
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;

	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("TermClockPrgJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)		//���ò���
	{
		if (bBuf[0]==DT_STRUCT && bBuf[1]==1 && bBuf[2]==DT_DB_LONG_U)
			dwMonthFluxLimit = OoDoubleLongUnsignedToDWord(bBuf+3);
	}

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(OI_FLUX, ATTR2, bBuf, NULL, NULL) > 0)		//���յ�������ֵ ����2
	{
		if (bBuf[0]==DT_STRUCT && bBuf[1]==2)
			dwCurMonthFlux = OoDoubleLongUnsignedToDWord(bBuf+8);
	}

	if (dwMonthFluxLimit != 0)	//��Ϊ0��������Ч
	{
		if (dwCurMonthFlux > dwMonthFluxLimit)
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}

    return pEvtCtrl->pEvtBase[0].bJudgeState;
}

#if 0	//����ʹ��
	BYTE bMtrShTest = 0;
#endif
bool IsNeedSaveShMtrEvt(BYTE *pShMtrFlag, BYTE *pSaveFlag)
{
	BYTE bByte, bBit, bSaveMtrNum = 0;

#if 0	//����ʹ��
	BYTE bBuf[PN_MASK_SIZE] = {0};
	bBuf[0] = 0xff;
	bBuf[1] = 0xff;
	bBuf[2] = 0x0f;
	if (bMtrShTest == 1)
	{
		WriteItemEx(BANK16, PN0, 0x6010, bBuf);
		bMtrShTest = 0;
	}
#endif

	//��ȡ�ѱ��־
	memset(pShMtrFlag, 0x00, SCH_MTR_SAVE_LEN);
	GetSchMtrEvtMask(pShMtrFlag, SCH_MTR_SAVE_LEN);
	
	if (IsAllAByte(pShMtrFlag, 0x00, SCH_MTR_SAVE_LEN))
		return false;
	memset(pSaveFlag, 0x00, SCH_MTR_SAVE_LEN);

	for (bByte=0; bByte<SCH_MTR_SAVE_LEN; bByte++)
	{
		if (pShMtrFlag[bByte] == 0x00)	//���ֽڲ���Ҫ��¼
			continue;

		for (bBit=0; bBit<8; bBit++)
		{
			if ((pShMtrFlag[bByte] & (1<<bBit)) != 0) //������ѵ����ˣ���Ҫ��¼
			{
				pShMtrFlag[bByte] &= ~(0x01<<bBit);
				pSaveFlag[bByte] |= (0x01<<bBit);
				bSaveMtrNum++;
				
			}
	
			if (bSaveMtrNum == SCH_MTR_SAVE_REC_NUM) //��������
			{	
				UpdataSchMtrEvtMask(pShMtrFlag, SCH_MTR_SAVE_LEN);
				return true;	
			}
		}	
	}
	UpdataSchMtrEvtMask(pShMtrFlag, SCH_MTR_SAVE_LEN);
	return true;
}

//����������δ֪���ܱ��¼��жϣ�
int UnKnMtrJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[10] = {0};	
	TUnKnMtr* pEvtPriv = (TUnKnMtr* )pEvtCtrl->pEvtPriv;
	
	if (pEvtPriv == NULL)
		return false;	
	
	if (pEvtCtrl->wOI != TERM_UNKNOWNMTR)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("UnKnMtrJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("UnKnMtrJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	if (pEvtPriv->bRunStep == 0)
	{	
		if (IsNeedSaveShMtrEvt(pEvtPriv->bShMtrFlag, pEvtPriv->bSaveFlag))	//ά��bShMtrFlag��bSaveFlag
		{	
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			pEvtPriv->bRunStep = 1;
		}
	}
	else
	{
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		pEvtPriv->bRunStep++;
		if (pEvtPriv->bRunStep > 6)	//�༸��û��ϵ
			pEvtPriv->bRunStep = 0;

	}

	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//��������̨�����ܱ��¼���
int StepAreaJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[10] = {0};	
	TStepArea* pEvtPriv = (TStepArea* )pEvtCtrl->pEvtPriv;
	
	if (pEvtPriv == NULL)
		return false;	
	
	if (pEvtCtrl->wOI != TERM_STEPAREA)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("StepAreaJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("StepAreaJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	if (pEvtPriv->bRunStep == 0)
	{
		//̨���ѱ���֪�������ӿ�,�����ٵ�����
		//if (IsNeedSaveShMtrEvt(pEvtPriv->bShMtrFlag, pEvtPriv->bSaveFlag, SCH_MTR_SAVE_LEN, STEP_AREA_SAVE_REC_NUM))	//ά��bShMtrFlag��bSaveFlag
		//{	
		//	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		//	pEvtPriv->bRunStep = 1;
		//}
	}
	else
	{
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		pEvtPriv->bRunStep++;
		if (pEvtPriv->bRunStep > 6)	//�༸��û��ϵ
			pEvtPriv->bRunStep = 0;

	}

	return pEvtCtrl->pEvtBase[0].bJudgeState;

}


//�������ն˶�ʱ�¼��жϣ�
int TermClockPrgJudge(struct TEvtCtrl* pEvtCtrl)
{
	return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ADJ_TERM_TIME);
}

//������ң����բ��¼�жϣ�
int YKCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_YK_REC);
}

//#if 1 //��¼�����ʹ��
#if 0
BYTE bEpOverTestFlag = 0;
#endif

//�������й��ܵ������Խ���¼���¼�жϣ�
int EpOverJudge(struct TEvtCtrl* pEvtCtrl)
{

	BYTE bValidFlag, bBuf[DIFF_COMP_CFG_ID_LEN];	
	
	if (pEvtCtrl->wOI != TERM_EPOVER)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//����
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("EpOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//��Ч��־
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("EpOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//���ò���
	//����6�����ò�������=array �й��ܵ������������
	//�й��ܵ�����������á�=structure
	//{
	//  �й��ܵ����������� unsigned��
	//  �Աȵ��ܼ���           OI��
	//  ���յ��ܼ���           OI��
	//  �����ĵ�������ʱ�����估�Աȷ�����־ unsigned��
	//  �Խ�����ƫ��ֵ integer����λ��%�����㣺0����
	//  �Խ�޾���ƫ��ֵ long64����λ��kWh�����㣺-4��
	//}
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		//���ò�����Ԥ����������ӡ�������������������������

	}
	else
	{
		//���ò�����Ԥ����������ӡ�����������������������������
	}

	//״̬�ж�
	//�������ò������ն˵�ǰ����ֵ�ж�״̬��������ӡ�����������������������������
	//if (������¼�)
	//{
	//	��g_EpOver.bCompEng��ֵ
	//	��g_EpOver.bReferEng��ֵ
	//	��g_EpOver.bRelaErr��ֵ
	//	��g_EpOver.bAbsoErr��ֵ
	//	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	//}
	//else if(δ������¼�)
	//{
	//	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	//}

//#if 1	//��¼�����ʹ��
#if 0
	BYTE bBuf1[] = {DT_LONG64, 1,2,3,4,5,6,7,8};
	BYTE bBuf2[] = {DT_LONG64, 2,2,3,4,5,6,7,8};
	BYTE bBuf3[] = {DT_INT, 0x03,};
	BYTE bBuf4[] = {DT_LONG64, 4,2,3,4,5,6,7,8};

	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	if (bEpOverTestFlag)
	{
		//bEpOverTestFlag = 0;
		memcpy(&g_EpOver.bCompEng[0], bBuf1, 9);
		memcpy(&g_EpOver.bReferEng[0], bBuf2, 9);
		memcpy(&g_EpOver.bRelaErr[0], bBuf3, 2);
		memcpy(&g_EpOver.bAbsoErr[0], bBuf4, 9);
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	}
#endif

	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//�������ն˱�̼�¼�жϣ�
int TermPrgJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_TERM_PROG);
}
//�������ն˵�����·�쳣�¼��жϣ�
int CurCircJudge(struct TEvtCtrl* pEvtCtrl)
{
    return pEvtCtrl->pEvtBase[0].bJudgeState;
}

#if 0
BYTE bClkPrgTestFlag = 0;
#endif
//�������ն˶Ե��Уʱ��¼�жϣ�
int MtrClkPrgJudge(struct TEvtCtrl* pEvtCtrl)
{
#if 0
	BYTE bBuf1[] = {DT_TSA, 0x07, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0,0,0,0,0,0,0,0,};
	BYTE bBuf2[] = {DT_DATE_TIME_S, 0x07, 0xE0, 0x01, 0x02, 0x03, 0x04, 0x05,};
	BYTE bBuf3[] = {DT_INT, 0x03,};

	if (bClkPrgTestFlag)
	{
		bClkPrgTestFlag = 0;
		memcpy(&g_MtrClkPrg.bEvtSrcTSA[0], bBuf1, 18);
		memcpy(&g_MtrClkPrg.bClock[0], bBuf2, 8);
		memcpy(&g_MtrClkPrg.bClkErr[0], bBuf3, 2);
		SetInfo(INFO_TERM_MTRCLKPRG);
	}
#endif


    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_TERM_MTRCLKPRG);
}

//������������բ��¼�жϣ�
int PowCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_POWERCTRL_REC);
}
//�����������բ��¼�жϣ�
int EleCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ENERGYCTRL_REC);
}
//����������������ü�¼�жϣ�
int PurChParaChgJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ENERGYBUY_PARACHG);
}
//��������ظ澯�¼���¼�жϣ�
int EleCtrlAlarmJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ENERGYCTRL_ALARM);
}


//֪ͨ��Ϣ�������¼�
int OnInfoTrigerEvtJudge(struct TEvtCtrl* pEvtCtrl, BYTE bInfoType)
{
	BYTE bValidFlag;
	BYTE bBuf[10];
	TDeviceErr* pEvtPriv = NULL;

	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)		//��Ч��־
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;

	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("TermClockPrgJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	if (GetInfo(bInfoType))
	{
		if (bInfoType==INFO_DEVICE_485_ERR )
		{
			pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
			if (pEvtPriv == NULL)
				return 0;

			pEvtPriv->bEvtSrcEnum = 3;	//485�������
		}
		else if (bInfoType==INFO_DEVICE_CCT_ERR)
		{
			pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
			if (pEvtPriv == NULL)
				return 0;

			pEvtPriv->bEvtSrcEnum = 5;	//�ز�ͨ������
		}

		if ((pEvtCtrl->wOI==TERM_YKCTRLBREAK) || (pEvtCtrl->wOI==TERM_POWCTRLBREAK)) 
			pEvtCtrl->bDelaySec = 0;	//���⴦��ң�غ͹�����Ҫ���غ�2�������ݣ���ʱ130���ٽ����¼�
			
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	}
	else
	{
		if ((pEvtCtrl->wOI==TERM_YKCTRLBREAK) || (pEvtCtrl->wOI==TERM_POWCTRLBREAK)) 
			pEvtCtrl->bDelaySec = 130;	//���⴦��ң�غ͹�����Ҫ���غ�2�������ݣ���ʱ130���ٽ����¼�

		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}

    return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//����APDU����
//��698.45Э���ȡ���һ��ͣ�ϵ��¼���¼ʱ�걨��
int MakeEvtAPDUFrm(BYTE* pbBuf)
{
	BYTE* pbBuf0 = pbBuf;

	*pbBuf++ = 0x05;	//GET-Request
	*pbBuf++ = 0x03;	//GetRequestRecord
	*pbBuf++ = 0x03;	//PIID

	*pbBuf++ = 0x30;
	*pbBuf++ = 0x11;
	*pbBuf++ = 0x02;
	*pbBuf++ = 0x00;	//ͣ�ϵ��¼�0AD

	*pbBuf++ = 0x09;	//RSD
	*pbBuf++ = 0x01;	//��1��

	*pbBuf++ = 0x02;	//RCSD��SEQUENCE OF����=2

	*pbBuf++ = 0x00;	//OAD

	*pbBuf++ = 0x20;
	*pbBuf++ = 0x1E;
	*pbBuf++ = 0x02;
	*pbBuf++ = 0x00;	//ͣ�緢��ʱ��0AD

	*pbBuf++ = 0x00;	//OAD

	*pbBuf++ = 0x20;
	*pbBuf++ = 0x20;
	*pbBuf++ = 0x02;
	*pbBuf++ = 0x00;	//�ϵ緢����ͣ�������ʱ��0AD

	*pbBuf++ = 0x00;	//û��ʱ���ǩ

	return pbBuf - pbBuf0;
}

//ת������698.45���ͣ�ϵ��¼�������̫Ƶ����Ӱ�쳭��
int FwdRead69845PwrEvt(TMtrPara* pMtrPara, BYTE* pbBuf)
{
	int i = 0, iRet = 0;	
	WORD wTxLen, wAPDULen;
	BYTE bCs = 0, bProId = 0, bDarOffset = 0;
	TTime tmTime;
	BYTE bBuf[128];
	BYTE bCmdFrm[256];
	BYTE bFrmHead = pMtrPara->bAddr[0] + 8;

	if (pMtrPara->CommPara.dwBaudRate == 0)
		pMtrPara->CommPara.dwBaudRate = CBR_2400;

	wAPDULen = MakeEvtAPDUFrm(bCmdFrm+bFrmHead);	//�鳭��698.45���ͣ�ϵ��¼�֡
	wTxLen = DL69845MakeFrm(pMtrPara->wPn, pMtrPara->bAddr, bCmdFrm, bCmdFrm+bFrmHead, wAPDULen, false);

	for (i=0; i<3; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		iRet = MtrDoFwd(pMtrPara->CommPara, bCmdFrm, wTxLen, bBuf, sizeof(bBuf), 9000, 10);
		if (iRet > 0)
			break;
	}

	if (iRet<=0 || i>=3)
		return 0;

	for (i=0; i<iRet; i++)
	{
		if (bBuf[i] == 0x68)
			break;
	}

	if (iRet > i)
		memcpy(bBuf, &bBuf[i], iRet-i);

	//�����Ƿ�Ҫ������У���ж�

	bDarOffset = bFrmHead + 18;		//��λ����Ӧ����    CHOICE
	if (bBuf[bDarOffset]==1 && bBuf[bDarOffset+1]==1)	//��Ӧ�����ж�
	{
		if (bBuf[bDarOffset+2]==DT_DATE_TIME_S && bBuf[bDarOffset+10]==DT_DATE_TIME_S)
		{
			OoDateTimeSToTime(bBuf+bDarOffset+3, &tmTime);
			TimeToYMDHMS(tmTime, pbBuf);		//ת��07��ͣ�ϵ��¼�ʱ���ʽ 6�ֽ�BCD
			pbBuf += 6;

			OoDateTimeSToTime(bBuf+bDarOffset+11, &tmTime);
			TimeToYMDHMS(tmTime, pbBuf);
			pbBuf += 6;

			return 12;
		}
	}

	return 0;
}


int Make07PwrEvtFrm(BYTE* pbAddr, BYTE* pbBuf)
{
	WORD i;
	BYTE bCs = 0;
	BYTE* p = pbBuf; 

	*pbBuf++ = 0x68;

	//memcpy(pbBuf, pbAddr+1, 6);	//���ַ ����???
	memrcpy(pbBuf, pbAddr+1, 6);	//07���ַ�赹��!!!
	pbBuf += 6;

	*pbBuf++ = 0x68;

	*pbBuf++ = 0x11;	//������
	*pbBuf++ = 0x04;	//���ݳ���

	*pbBuf++ = (0x01+0x33);	//���ݱ�ʶ
	*pbBuf++ = (0x00+0x33);
	*pbBuf++ = (0x11+0x33);
	*pbBuf++ = (0x03+0x33);	//��һ��ͣ���¼ʱ��ID

	bCs = 0;
	for(i=0; i<pbBuf-p; i++)	//14
	{
		bCs += p[i];
	}

	*pbBuf++ = bCs;		//��һ��ͣ���¼ʱ��ID
	*pbBuf++ = 0x16;

	return pbBuf-p;		//16
}

//ת������07��ͣ�ϵ��¼�������̫Ƶ����Ӱ�쳭��
int FwdReadV07PwrEvt(TMtrPara* pMtrPara, BYTE* pbBuf)
{
	int i = 0, iRet = 0;
	WORD wTxLen;
	BYTE bCs = 0, bProId, bAddrLen;
	BYTE bBuf[128];
	BYTE bCmdFrm[256];

	if (pMtrPara->CommPara.dwBaudRate == 0)
		pMtrPara->CommPara.dwBaudRate = CBR_2400;

	wTxLen = Make07PwrEvtFrm(pMtrPara->bAddr, bCmdFrm);	//��07Э��ͣ�ϵ��¼�֡

	for (i=0; i<3; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		iRet = MtrDoFwd(pMtrPara->CommPara, bCmdFrm, wTxLen, bBuf, sizeof(bBuf), 9000, 10);
		if (iRet > 0)
			break;
	}

	if (iRet<=0 || i>=3)
		return 0;

	for (i=0; i<iRet; i++)
	{
		if (bBuf[i] == 0x68)
			break;
	}

	if (i+12 > iRet)
		return 0;

	if (iRet > i)
		memcpy(bBuf, &bBuf[i], iRet-i);

	//����֡��У����ж�
	bCs = 0x00;
	for(i=0; i<bBuf[9]+10; i++)
		bCs += bBuf[i];

	if(bCs != bBuf[i])
		return 0;

	for (i=0; i<bBuf[9]; i++)
	{
		bBuf[10+i] -= 0x33;
	}

	if(bBuf[8]==0x91 || bBuf[8]==0xB1)
	{
		memcpy(pbBuf, bBuf+14, 12);
		return 12;
	}

	return 0;
}

////////////////////////////////�ն�ͣ���¼�(0x3106)///////////////////////////
int GetPnPwrOff(WORD wPn, BYTE* pbBuf)
{
	int iRet = 0;
	TMtrPara tMtrPara;

	if (!GetMeterPara(wPn, &tMtrPara))
		return 0;
	
	tMtrPara.CommPara.wPort = GetPnPort(wPn);
	if (tMtrPara.bProId == PROTOCOLNO_DLT645_V07)	//07��
		iRet = FwdReadV07PwrEvt(&tMtrPara, pbBuf);
	else if (tMtrPara.bProId == PROTOCOLNO_DLT69845)	//698.45��
		iRet = FwdRead69845PwrEvt(&tMtrPara, pbBuf);

	return iRet;
}

DWORD GetDiffVal(DWORD dwVal1, DWORD dwVal2)
{
	DWORD dwVal = 0;
	if(dwVal1 >= dwVal2)
		dwVal = dwVal1 - dwVal2;
	else
		dwVal = dwVal2 - dwVal1;
	return dwVal;
}

bool IsMtrPwrOff(WORD wPn, DWORD dwPwrOffSec, BYTE* pbBuf, const TTime time, bool* fMtrOrTerm)
{
	int iLen;
	TTime tm;
	DWORD dwPwrOn=0, dwPwrOff=0, dwSec=0;
	WORD i, wLimt =0, wLimtSec=0;
	BYTE bBuf[SAMPLE_CFG_ID_LEN];
	bool fGetPnData = false;
	TMtrPara tMtrPara;
	WORD wBaseOffset = 0;
	BYTE* p;

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(TERM_POWOFF, ATTR6, bBuf, NULL, NULL); //��ȡ����6���ò���-ͣ�����ݲɼ����ò���
	if (iLen<=0 || bBuf[0]!=DT_STRUCT)
		return 0;

	p = &bBuf[12];	//ָ��TSA����
	for (i=0; i<bBuf[12]; i++)
	{
		wBaseOffset += 2;	//TSA��������1 + TSA����1
		wBaseOffset += p[wBaseOffset];
	}

	wBaseOffset += 10;	//ͣ���¼������ֵ-ͣ���¼���ֹʱ��ƫ����ֵ�����ӣ�����ƫ��
	wBaseOffset += 12;	//��bBuf��ʼ�������ǰ���12���ֽ�
	if (wBaseOffset >= sizeof(bBuf)-3)
		return 0;

	wLimt = OoLongUnsignedToWord(bBuf+wBaseOffset) * 60;		//�������
	wLimtSec = OoLongUnsignedToWord(bBuf+wBaseOffset+3) * 60;	//�������

	*fMtrOrTerm = false;

	if (!GetMeterPara(wPn, &tMtrPara))
		return false;

	if (tMtrPara.bProId!=PROTOCOLNO_DLT645_V07 && tMtrPara.bProId!=PROTOCOLNO_DLT69845)	//�ȷ�07��Ҳ��698.45��
		return false;

	DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step000 coming init false!\r\n"));
	for (int i=0; i<10; i++)
	{
		if(IsPnValid(wPn) && GetPnPwrOff(wPn, pbBuf)>0
			&& (!IsAllAByte(pbBuf, 0, 6)) && (!IsAllAByte(pbBuf, 0xff, 6)) //���緢��ʱ����Ч
			&& (!IsAllAByte(pbBuf+6, 0, 6)) && (!IsAllAByte(pbBuf+6, 0xff, 6))) //�������ʱ����Ч����ʵ���еĲ��������������Ȼ�ȫff�ˣ��೭2��
		{
			fGetPnData = true;
			*fMtrOrTerm = true; //����Ч
			DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step001 coming init true! wPn=%d.\r\n", wPn));
			if(IsBcdCode(pbBuf, sizeof(pbBuf)))
			{
				DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step222 coming init! wPn=%d.\r\n", wPn));
				memset(&tm, 0x00, sizeof(&tm));
				Fmt15ToTime(pbBuf+1, tm);
				tm.nSecond = BcdToByte(pbBuf[0]);
				dwPwrOff = TimeToSeconds(tm);
				memset(&tm, 0x00, sizeof(&tm));
				Fmt15ToTime(pbBuf+7, tm);
				tm.nSecond = BcdToByte(pbBuf[6]);
				dwPwrOn  = TimeToSeconds(tm);

				if(dwPwrOn > dwPwrOff)
				{
					dwSec = dwPwrOn - dwPwrOff;
					if(GetDiffVal(dwPwrOffSec, dwSec) > wLimtSec)//ͣ��ʱ��
					{
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step1 dwPwrOffSec=%ld, dwSec=%ld, delta=%ld > wLimtSec=%ld.\r\n", dwPwrOffSec, dwSec, GetDiffVal(dwPwrOffSec, dwSec), wLimtSec));
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step1 term PwrOn time= %02d %02d:%02d:%02d.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond));
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step1 term PwrOff time= %02d %02d:%02d:%02d.\r\n", g_PowerOffTmp.tPoweroff.nDay, g_PowerOffTmp.tPoweroff.nHour, g_PowerOffTmp.tPoweroff.nMinute, g_PowerOffTmp.tPoweroff.nSecond));
						*fMtrOrTerm = false;
					}

					if((GetDiffVal(TimeToSeconds(time), dwPwrOn)>wLimt)||//ͣ�ϵ�ʱ���
						(GetDiffVal(TimeToSeconds(g_PowerOffTmp.tPoweroff), dwPwrOff)>wLimt))
					{
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step2 Pwr On or Off exceed, Pwr On delta=%ld , wLimt=%ld.\r\n", GetDiffVal(TimeToSeconds(time), dwPwrOn), wLimt));			
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step2 Pwr On or Off exceed, Pwr Off delta=%ld , wLimt=%ld.\r\n", GetDiffVal(TimeToSeconds(g_PowerOffTmp.tPoweroff), dwPwrOff), wLimt));
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step2 term PwrOn time= %02d %02d:%02d:%02d.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond));			
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step2 term PwrOff time= %02d %02d:%02d:%02d.\r\n", g_PowerOffTmp.tPoweroff.nDay, g_PowerOffTmp.tPoweroff.nHour, g_PowerOffTmp.tPoweroff.nMinute, g_PowerOffTmp.tPoweroff.nSecond));			

						*fMtrOrTerm = false;
					}
				}
				else
				{
					DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step3 dwPwrOn=%ld, smaller than dwPwrOff=%ld.\r\n", dwPwrOn, dwPwrOff));			
					*fMtrOrTerm = false;
				}
			}
			else
			{
				DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step4 Para Invalid, wPn=%d!\r\n", wPn));
				*fMtrOrTerm = false;
			}

			break;
		}
		else
		{
			DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step009 no way to go! wPn=%d, i=%d.\r\n", wPn, i));
		}

		if (IsMtrErr(wPn))
			break;
		else
			Sleep(3000); //�������ԭ�����ƣ���ʱֻ������һֱȥ����һ��ʱ���ˣ���Ϊ��ʵ������ϵ��¼���Ҫ60�����ʱ�� �ն�����ʱ��20����+10*3000ms���һ���ӣ�����̫��ʱ����п���Ӱ�������澯�¼��ж�
	}
	return fGetPnData;
}


//�������ն�ͣ/�ϵ��¼���ʼ��
//������@pEvtCtrl �¼�����
//���أ�true/false
bool InitPowOff(struct TEvtCtrl* pEvtCtrl)
{
	TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0x00, sizeof(TPowOff));	
	return InitEvt(pEvtCtrl);
}

//�������ն�ͣ/�ϵ��¼���
//������ͣ���ϵ��¼��жϣ�
//#define POWOFF_DEBUG_TEST
#ifdef POWOFF_DEBUG_TEST	//����ʹ��
BYTE g_PowOffTestFlag = 0;	
#endif
int PowOffJudge(struct TEvtCtrl* pEvtCtrl)
{
	int iLen;
	TTime time;
	BYTE bIsValid = 0, bValidFlag = 0;
	DWORD dwSec = 0, dwPwrOffSec = 0;
	WORD i = 0, wPn = 0, wCnt = 0, wMaxSec = 0, wMinSec = 0;
	bool fPowerOff = false, fMtrOrTerm	= false, fPwrOffValid = true;
	char cTime[20];
	BYTE bF98Buf[13];
	BYTE bPnPwrOff[12];
	BYTE bBuf[SAMPLE_CFG_ID_LEN];
	BYTE bMtrAddr[MTR_ADDR_LEN];
	const WORD wOI = pEvtCtrl->wOI;
	WORD wBaseOffset = 0;
	BYTE* pbBuf = NULL;


	// 1 �㽭ͣ�ϵ�
	//*ZJ1*ͣ�缴��¼ͣ���¼���
	//*ZJ2*�ϵ�ʱ����ͣ��û������¼ͣ���¼�����ô�ϵ缴����ͣ���¼��
	//*ZJ3*�ϵ�������ϱ���1���ϵ硣�����õ���������������жϺ����������2���ϵ��¼��
	//*ZJ4*������¼�ϱ���������õ��ϱ������йء�
	
	// 2 ����ͣ�ϵ�
	//*GW1*ͣ��ʱ�������õ������Ҫ��ͣ���¼��
	//*GW2*ͣ��ʱ����δ���õ����Ҫ��ͣ���¼��
	//*GW3*�ϵ�ʱ����ͣ��ʱ��Ҫ��ͣ���¼��δ���ļ�������ô�ϵ缴����ͣ���¼��
	//*GW4*�ϵ�������õ�����ж������ͣ���¼�����������ϵ��¼��
	//*GW5*�ϵ����δ���õ�����ж�������ϵ��¼��
	//*GW6*������¼�ϱ���������õ��ϱ������йء�

	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;		//Ĭ����������

	//��ȡ˽�б���
	TPowOff* pCtrl = (TPowOff* )pEvtCtrl->pEvtPriv;
	if (pCtrl == NULL)
		return 0;
		
#ifdef POWOFF_DEBUG_TEST	//����ʹ��
	if (g_PowOffTestFlag == 0)	//�ϵ�
		fPowerOff = 0;
	else
		fPowerOff = 1;
#else
	fPowerOff = IsAcPowerOff(&bIsValid);
#endif

	if (bIsValid == 1)	//�ϵ�9����ٴ���ͣ�ϵ��¼�������ֱ���˳�
	{	
		return 0;
	}

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)	//��Ч��־
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL); 	//��ȡ����6���ò���-ͣ�����ݲɼ����ò���
	if (iLen<=0 || bBuf[0]!=DT_STRUCT)		//���ò������쳣ֱ���˳���ǿ�ƽ����¼�
	{	
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
		return 0;
	}

	//ͣ�ϵ��ʼ��������ͣ��δ����ͣ���¼���ϵ�ʱ����
	if (!pCtrl->fInit)	
	{
		if (fPowerOff)
			pCtrl->fPowerOff = true;
		else
			pCtrl->fPowerOff = false;

		pCtrl->bStep = PWR_OFF_RUN_CNT;	//�ϵ�ʱĬ�ϲ���Ҫ����

		DTRACE(DB_METER_EXC, ("PowOffJudge: Init g_tPowerOn=%s \r\n", TimeToStr(g_tPowerOn, cTime)));
		
#ifdef POWOFF_DEBUG_TEST	//����ʹ��
		g_PowerOffTmp.tPoweroff= g_tPowerOn;
#endif

		dwSec = SecondsPast(g_PowerOffTmp.tPoweroff, g_tPowerOn);
		DTRACE(DB_METER_EXC, ("PowOffJudge: Init dwSec = %d, fAlrPowerOff = %d\r\n", dwSec, g_PowerOffTmp.fAlrPowerOff));

		if (!g_PowerOffTmp.fAlrPowerOff && (SecondsPast(g_PowerOffTmp.tPoweroff, g_tPowerOn) > 60))	//ͣ��ʱû������ͣ���¼���ϵ�ʱ����
		{
#ifdef VER_ZJ
			if (bValidFlag)	//�㽭�����ܲɼ���־Ӱ�죬ͣ��̶��ϱ�.	//*ZJ2*�ϵ�ʱ����ͣ��û������¼ͣ���¼�����ô�ϵ缴����ͣ���¼��			
#else
			if (bValidFlag && (bBuf[6]&0x80)==0x00)		//������׼���ɼ���־��Чʱ��ͣ�粻�ϱ� 	//*GW3*�ϵ�ʱ����ͣ��ʱ��Ҫ��ͣ���¼��δ���ļ�������ô�ϵ缴����ͣ���¼��
#endif		
			{
				pCtrl->bAttr = 0x80;	//������Чͣ���¼�
				pCtrl->bEvtSrcEnum = 0;		//ͣ���¼�
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
				pCtrl->bStep = 1;		//��һ��ͣ���¼�����״̬
				pCtrl->bRptFlag = 1;	//��Ҫ�ϱ�����	
				DTRACE(DB_METER_EXC, ("PowOffJudge: Init save powoff event ########## Terminal __Bat Break__ is power off.\r\n"));	//������ص�ͣ���¼�
			}
			
			g_PowerOffTmp.fAlrPowerOff = true;	//����ǰ�ϱ���ͣ��澯
			SavePoweroffTmp();
		}
		
		pCtrl->fIsUp = false;	
		pCtrl->fIsUpRec = false;	
		pCtrl->fMtrOrTerm = false;	
		pCtrl->fOldPowerOff = g_PowerOffTmp.fAlrPowerOff;
		pCtrl->fInit = true;
		
		return pEvtCtrl->pEvtBase[0].bJudgeState;
	}

	//ͣ�ϵ�����������ȡ��ǰͣ�ϵ�״̬
	if (fPowerOff)
	{
		pCtrl->wLastPwrOnClick = 0;
		if(pCtrl->wLastPwrOffClick == 0)
		{
			pCtrl->wLastPwrOffClick = GetClick();
			return pEvtCtrl->pEvtBase[0].bJudgeState;
		}
		else if(GetClick() - pCtrl->wLastPwrOffClick >= 5)////����ͣ��5�����ϲű�
		{
			pCtrl->fPowerOff = true;
			WriteItemEx(BN2, PN0, 0x210e, (BYTE* )&fPowerOff);
		}
		else
		{
			return pEvtCtrl->pEvtBase[0].bJudgeState;
		}
	}
	else
	{
		pCtrl->wLastPwrOffClick = 0;
		if(pCtrl->wLastPwrOnClick == 0)
		{
			pCtrl->wLastPwrOnClick = GetClick();
			return pEvtCtrl->pEvtBase[0].bJudgeState;
		}
		else if(GetClick() - pCtrl->wLastPwrOnClick >= 5)////�����ϵ�5�����ϲű�
		{
			pCtrl->fPowerOff = false;
		}
		else
		{
			return pEvtCtrl->pEvtBase[0].bJudgeState;
		}		
	}

	if (pCtrl->bStep < PWR_OFF_RUN_CNT)	//����ʹ�������洢
	{
		pCtrl->bStep++;
		return pEvtCtrl->pEvtBase[0].bJudgeState;
	}

	//DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask pCtrl->fPowerOff=%d, pCtrl->fOldPowerOff=%d, fAlrPowerOff = %d.\r\n", pCtrl->fPowerOff, pCtrl->fOldPowerOff, g_PowerOffTmp.fAlrPowerOff));
	//ͣ�ϵ�״̬�����ı�ʱ����Ҫ����¼
	if (pCtrl->fPowerOff != pCtrl->fOldPowerOff)   //�����ı�
	{
		pCtrl->fOldPowerOff = pCtrl->fPowerOff;	
		GetCurTime(&time);

		if (pCtrl->fPowerOff)   //�ն�ͣ��
		{
#ifdef VER_ZJ
			if (bValidFlag)	//�㽭�����ܲɼ���־Ӱ�죬ͣ��̶��ϱ�.	//*ZJ1*ͣ�缴��¼ͣ���¼���
#else
			if (bValidFlag && (bBuf[6]&0x80)==0x00)		//������׼���ɼ���־��Чʱ��ͣ�粻�ϱ�	//*GW1*ͣ��ʱ�������õ������Ҫ��ͣ���¼��//*GW2*ͣ��ʱ����δ���õ����Ҫ��ͣ���¼��
#endif
			{
				pCtrl->bAttr = 0x80;	//������Чͣ���¼�
				pCtrl->bEvtSrcEnum = 0;		//ͣ���¼�
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
				pCtrl->bStep = 1;		//��һ��ͣ���¼�����״̬
				pCtrl->bRptFlag = 1;	//��Ҫ�ϱ�����	
				DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask save powoff event  ########## Terminal is power off. \r\n"));	
			}
			g_PowerOffTmp.fAlrPowerOff = true;	//����ǰ�ϱ���ͣ��澯
			memcpy(&g_PowerOffTmp.tPoweroff, &time,sizeof(time)); 
			SavePoweroffTmp();

			TrigerSaveBank(BN0, SECT3, -1);
			TrigerSaveBank(BN0, SECT16, -1);	//ͣ��ǰ����һ���¼��������

			pCtrl->fIsUp = false;	
			pCtrl->fIsUpRec = false;	
			pCtrl->fMtrOrTerm = false;
			SetInfo(INFO_PWROFF);
		}
		else if (!pCtrl->fPowerOff)		//�ն�����
		{
			GetCurTime(&g_tPowerOn);	//ˢ���ϵ�ʱ��
			if (!bValidFlag)	//�¼���Ч���ж���,ǿ�ƽ���
			{
				DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask ValidFlag=0.\r\n"));
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
				return pEvtCtrl->pEvtBase[0].bJudgeState;
			}
			
			pCtrl->fIsUp = true;	//�ϵ�
	
#ifdef VER_ZJ
			if (bValidFlag)		//*ZJ3*�ϵ�������ϱ���1���ϵ硣�����õ���������������жϺ����������2���ϵ��¼��
			{	
				pCtrl->bAttr = 0x80;	//�㽭Ҫ���ϵ�������ϱ�������Ч���ϵ��¼�
				pCtrl->bEvtSrcEnum = 0x01;		//�ϵ��¼�
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
				pCtrl->bStep = 1;
				pCtrl->bRptFlag = 2;	//��Ҫ�ϱ��ָ�
		
				DTRACE(DB_METER_EXC, ("PowOffJudge: Right now Save Power On Rec bJudgeState = %d Click=%ld .\r\n", pEvtCtrl->pEvtBase[0].bJudgeState, GetClick()));
				return pEvtCtrl->pEvtBase[0].bJudgeState;
			}
#endif
		}	
	}

	if (pCtrl->fIsUp && (pCtrl->bStep>=PWR_OFF_RUN_CNT))
	{
		pCtrl->fIsUp = false;
		
		//////////////////////////�¼���Ч�Ե��жϡ�////////////////////////////////////
		if (g_PowerOffTmp.fAlrPowerOff)
		{
			if (!IsInvalidTime(g_tPowerOn) && !IsInvalidTime(g_PowerOffTmp.tPoweroff))
			{
				if(TimeToSeconds(g_tPowerOn) > TimeToSeconds(g_PowerOffTmp.tPoweroff))
				{
					DTRACE(DB_METER_EXC, ("PowOffJudge1: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));
					wBaseOffset = 0;
					pbBuf = &bBuf[12];	//ָ��TSA����
					for (i=0; i<bBuf[12]; i++)
					{
						wBaseOffset += 2;	//TSA��������1 + TSA����1
						wBaseOffset += pbBuf[wBaseOffset];
					}

					wBaseOffset += 4;	//ͣ���¼������ֵ-ͣ��ʱ����С��Ч��������ӣ�����ƫ��
					wBaseOffset += 12;	//��bBuf��ʼ�������ǰ���12���ֽ�
					if (wBaseOffset >= sizeof(bBuf)-3)
						return pEvtCtrl->pEvtBase[0].bJudgeState;

					wMinSec = OoLongUnsignedToWord(bBuf+wBaseOffset) * 60;	//�������
					wMaxSec = OoLongUnsignedToWord(bBuf+wBaseOffset+3) * 60;	//�������
					dwPwrOffSec = TimeToSeconds(g_tPowerOn) - TimeToSeconds(g_PowerOffTmp.tPoweroff);		//����ʱ�� - ͣ��ʱ��

					if(dwPwrOffSec>wMaxSec || dwPwrOffSec<wMinSec)
						fPwrOffValid = false;	//���������ֵ
				}
				else
				{
					DTRACE(DB_METER_EXC, ("PowOffJudge2: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));
					fPwrOffValid = false;	//����ʱ������ͣ��ʱ��
				}
			}
			else
			{
				DTRACE(DB_METER_EXC, ("PowOffJudge3: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));
				fPwrOffValid = false;	//ʱ��Ϊ��
			}
		}
		else
		{
			DTRACE(DB_METER_EXC, ("PowOffJudge4: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));
			fPwrOffValid = false;
		}

		//DTRACE(DB_METER_EXC, ("PowOffJudge5: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));

		if(bBuf[6] & 0x80)	//�ɼ���־
		{
			DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead --step00000.\r\n"));
			//////////////////////////////̨������ն��¼��жϡ�////////////////////////////////

			wCnt = bBuf[12];
			if (wCnt > SAMPLE_MTR_NUM)
				wCnt = SAMPLE_MTR_NUM;

			if (wCnt>0 && !(bBuf[6]&0x40))	//�������õĲ������
			{
				DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead--sample given mtr addr.\r\n"));
				wBaseOffset = 0;
				pbBuf = &bBuf[12];	//ָ��TSA����

				for (i=0; i<wCnt; i++)
				{
					wBaseOffset += 2;	//TSA��������1 + TSA����1

					bMtrAddr[0] = pbBuf[wBaseOffset] - 1;	//��ַ����
					if (bMtrAddr[0] > 16)
						bMtrAddr[0] = 16;

					memset(&bMtrAddr[1], 0, 16);
					memcpy(&bMtrAddr[1], &pbBuf[wBaseOffset+2], bMtrAddr[0]);		//��ַ����
					wBaseOffset += pbBuf[wBaseOffset];

					//wPn = MtrAddrToPn(bMtrAddr, bMtrAddr[0]);
					wPn = GetMeterPn(bMtrAddr+1, bMtrAddr[0]);
					if (wPn == 0)
						continue;

					memset(bPnPwrOff, 0, sizeof(bPnPwrOff));
					if (IsMtrPwrOff(wPn, dwPwrOffSec, bPnPwrOff, g_tPowerOn, &fMtrOrTerm) && fMtrOrTerm)
						break;
				}
			}
			else if (bBuf[6] & 0x40)	//�����
			{
				DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead--random sample.\r\n"));
				for(i=0,wCnt=0; i<POINT_NUM; i++)
				{
					if (i == 0)
						DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead--step33333.\r\n"));

					if (!IsPnValid(i))
						continue;

					if(wCnt>=3)
						break;

					memset(bPnPwrOff, 0x00, sizeof(bPnPwrOff));	
					if(IsMtrPwrOff(i, dwPwrOffSec, bPnPwrOff, g_tPowerOn, &fMtrOrTerm))
						wCnt++;

					if (fMtrOrTerm)
						break;
				}
			}
		}
		else
		{
			DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead--step11111 fMtrOrTerm=false, sample flag is false.\r\n"));
			fMtrOrTerm = false;
		}

		if (fPwrOffValid)	//�¼��Ƿ�����
			pCtrl->bAttr = 0x80;
		else
			pCtrl->bAttr = 0x00;

#ifndef VER_ZJ
		//����ͣ���¼�
		if (g_PowerOffTmp.fAlrPowerOff && (bBuf[6]&0x80))		//�ɼ���־���ж�
		{
			DTRACE(DB_METER_EXC, ("PowOffJudge1: RunTask pCtrl->bAttr = %d.\r\n", pCtrl->bAttr));
			pCtrl->bEvtSrcEnum = 0;		//ͣ���¼�
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			pCtrl->bStep = 1;		//��һ��ͣ���¼�����״̬
			pCtrl->bRptFlag = 1;	//��Ҫ�ϱ�����
			DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask save powoff event  ########## Terminal __Bat Break__ is power off.\r\n"));
		}
#endif

		/////////////////////////�����ϵ��¼�-------	
		g_PowerOffTmp.fAlrPowerOff = false;
		pCtrl->fIsUpRec = true;	//��Ҫ���ϵ��¼
		pCtrl->fMtrOrTerm = fMtrOrTerm;
		DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask fMtrOrTerm=%d.\r\n", fMtrOrTerm));
	}
		
	//�����ϵ��¼�
	if (pCtrl->fIsUpRec && (pCtrl->bStep>=PWR_OFF_RUN_CNT))
	{
		pCtrl->fIsUpRec = false;

		DTRACE(DB_METER_EXC, ("PowOffJudge2: RunTask pCtrl->bAttr = %d.\r\n", pCtrl->bAttr));
		if (pCtrl->fMtrOrTerm)	 //�¼��Ƿ���Ч
			pCtrl->bAttr |= 0x40;
		else
			DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask--step5 fMtrOrTerm=false, invalid event, pCtrl->bAttr=%02x.\r\n", pCtrl->bAttr));

#ifdef VER_ZJ
		if ((pCtrl->bAttr&0xc0) == 0xc0)	//�㽭��������Ч�¼������ϱ�
#endif	
		{
			pCtrl->bEvtSrcEnum = 0x01;		//�ϵ��¼�
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			pCtrl->bStep = 1;		//��һ��ͣ���¼�����״̬
			pCtrl->bRptFlag = 2;	//��Ҫ�ϱ��ϵ�ָ��¼�
		}

		DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask save powon event  ########## Terminal is power on.\r\n"));				
	}

	//DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask pEvtCtrl->pEvtBase[0].bJudgeState=%d.\r\n", pEvtCtrl->pEvtBase[0].bJudgeState));
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//����һ���¼�����
//����2�������¼��б�ֻ������= array OAD
//����3�����ϱ��¼������б�ֻ������= array OI
//ע�������¼��б�������ÿͨ��ͨ����OAD���������б��е��¼���¼ͨ��"��ǰ"ͨ������ȡ���Զ��Ӹ��б���ɾ����OI����
void AddEvtOad(DWORD dwOAD, bool fRptFlag)
{
	BYTE bBuf[EVT_ADDOAD_MAXLEN], bOadBuf[4];	
	BYTE bArrayNum, bIndex;
	WORD wOI;
	int iLen;

	//�����¼��б�array OAD
	dwOAD &=0xffffff00;	//����ֽ�
	OoDWordToOad(dwOAD, bOadBuf);

	memset(bBuf, 0x00, sizeof(bBuf));
	iLen = OoReadAttr(0x3320, 0x02, bBuf, NULL, NULL);
	if ((iLen<=0) || (iLen > sizeof(bBuf)) || (bBuf[0]!=DT_ARRAY) || (bBuf[1]==0x00) || (bBuf[1]>EVT_ADDOAD_MAXNUM))	//����������������Ϊ�գ���������OI
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = DT_ARRAY;
		bBuf[1] = 1;
		bBuf[2] = DT_OAD;
		memcpy(bBuf+3, bOadBuf, 4);
		OoWriteAttr(0x3320, 0x02, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
		return;	
	}

	bArrayNum = bBuf[1];
	for (bIndex=0; bIndex<bArrayNum; bIndex++)
	{
		if (memcmp(&bBuf[3+bIndex*5], bOadBuf, 4) == 0)	//�Ѵ���OAD,����Ҫ���
			return;
	}

	if ((bIndex==bArrayNum) && (bArrayNum<EVT_ADDOAD_MAXNUM))
	{
		if (iLen+5 > sizeof(bBuf))
			return;	
		bBuf[1]++;
		bBuf[iLen] = DT_OAD;
		memcpy(&bBuf[iLen+1] , bOadBuf, 4);
		OoWriteAttr(0x3320, 0x02, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
		return;	
	}

	if (!fRptFlag)	
		return;

	//��Ҫ�ϱ�������ϱ��¼������б�array OI
	GetOIAttrIndex(dwOAD, &wOI, NULL, NULL);
	OoWordToOi(wOI, bOadBuf);

	memset(bBuf, 0x00, sizeof(bBuf));
	iLen = OoReadAttr(0x3320, 0x03, bBuf, NULL, NULL);

	if ((iLen<=0) || (iLen > sizeof(bBuf)) || (bBuf[0]!=DT_ARRAY) || (bBuf[1]==0x00) || (bBuf[1]>EVT_ADDOAD_MAXNUM))	//����������������Ϊ�գ���������OI
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = DT_ARRAY;
		bBuf[1] = 1;
		bBuf[2] = DT_OI;
		memcpy(bBuf+3, bOadBuf, 2);
		OoWriteAttr(0x3320, 0x03, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
		return;	
	}

	bArrayNum = bBuf[1];
	for (bIndex=0; bIndex<bArrayNum; bIndex++)
	{
		if (memcmp(&bBuf[3+bIndex*3], bOadBuf, 2) == 0)	//�Ѵ���OAD,����Ҫ���
			return;
	}

	if ((bIndex==bArrayNum) && (bArrayNum<EVT_ADDOAD_MAXNUM))
	{
		if (iLen+3 > sizeof(bBuf))
			return;	
		bBuf[1]++;
		bBuf[iLen] = DT_OI;
		memcpy(&bBuf[iLen+1] , bOadBuf, 2);
		OoWriteAttr(0x3320, 0x03, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
		return;	
	}	
		
	return;	
}



//ɾ��һ���¼�����
//dwOAD ��wOI ���Ϊ0ʱ��ɾ��
void DelEvtOad(DWORD dwOAD, WORD wOI)
{
	BYTE bBuf[EVT_ADDOAD_MAXLEN], bOadBuf[4];	
	BYTE bArrayNum, bIndex;
	int iLen;
	bool fDelFlag = false;

	if (dwOAD)
	{
		dwOAD &=0xffffff00;	//����ֽ�
		OoDWordToOad(dwOAD, bOadBuf);
		
		memset(bBuf, 0x00, sizeof(bBuf));	
		iLen = OoReadAttr(0x3320, 0x02, bBuf, NULL, NULL);
		if ((iLen<=0) || (bBuf[0]!=DT_ARRAY))	//������������,����
		{
			memset(bBuf, 0x00, sizeof(bBuf));
			bBuf[0] = DT_ARRAY;
			OoWriteAttr(0x3320, 0x02, bBuf);
			TrigerSaveBank(BN0, SECT3, -1);
			return;
		}	
		if (bBuf[1]==0x00)	//����Ϊ�գ��޷�ɾ��
			return;	
		
		bArrayNum = bBuf[1];
		for (bIndex=0; bIndex<bArrayNum; bIndex++)
		{
			if (memcmp(&bBuf[3+bIndex*5], bOadBuf, 4) == 0)	//�������У��Ѵ��ڵ�OAD��ɾ��
			{
				memcpy(&bBuf[2+bIndex*5], &bBuf[2+(bIndex+1)*5], 5*(bArrayNum-bIndex));
				memset(&bBuf[2+bArrayNum*5], 0x00, 5);
				bBuf[1]--;bArrayNum--;bIndex--;
				fDelFlag = true;
			}	
		}
		if (fDelFlag)
		{	
			OoWriteAttr(0x3320, 0x02, bBuf);
			TrigerSaveBank(BN0, SECT3, -1);
		}
	}
	
	if (wOI)
	{
		OoWordToOi(wOI, bOadBuf);
		
		memset(bBuf, 0x00, sizeof(bBuf));	
		iLen = OoReadAttr(0x3320, 0x03, bBuf, NULL, NULL);
		if ((iLen<=0) || (bBuf[0]!=DT_ARRAY))	//������������,����
		{
			memset(bBuf, 0x00, sizeof(bBuf));
			bBuf[0] = DT_ARRAY;
			OoWriteAttr(0x3320, 0x03, bBuf);
			TrigerSaveBank(BN0, SECT3, -1);
			return;
		}	
		if (bBuf[1]==0x00)	//����Ϊ�գ��޷�ɾ��
			return;	
		
		bArrayNum = bBuf[1];
		for (bIndex=0; bIndex<bArrayNum; bIndex++)
		{
			if (memcmp(&bBuf[3+bIndex*3], bOadBuf, 2) == 0)	//�������У��Ѵ��ڵ�OAD��ɾ��
			{
				memcpy(&bBuf[2+bIndex*3], &bBuf[2+(bIndex+1)*3], 3*(bArrayNum-bIndex));
				memset(&bBuf[2+bArrayNum*3], 0x00, 3);
				bBuf[1]--;bArrayNum--;bIndex--;
				fDelFlag = true;
			}	
		}
		if (fDelFlag)
		{	
			OoWriteAttr(0x3320, 0x03, bBuf);
			TrigerSaveBank(BN0, SECT3, -1);
		}
	}
		
	return;	
}


