#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "FaAPI.h"
#include "sysfs.h"
#include "DbAPI.h"
#include "DL645Fmt.h"
//#include "V07Task.h"
#include "AcConst.h"

#ifdef EN_INMTR

DWORD g_dwAmpMS[3]={0, 0, 0};


T07To97IDCfg g_07to97IdMap[]= 
{
	//����
	{	BN_645_DATA,		0x00000000,		0xe000,		NULL},

	{	BN_645_DATA,		0x00010000,		0x9010,		NULL},
	{	BN_645_DATA,		0x00020000,		0x9020,		NULL},
	{	BN_645_DATA,		0x00030000,		0x9110,		NULL},
	{	BN_645_DATA,		0x00040000,		0x9120,		NULL},

	{	BN_645_DATA,		0x00050000,		0x9130,		NULL},
	{	BN_645_DATA,		0x00060000,		0x9150,		NULL},
	{	BN_645_DATA,		0x00070000,		0x9160,		NULL},
	{	BN_645_DATA,		0x00080000,		0x9140,		NULL},

	//��X�����յ���
	{	BN_645_DATA,		0x00000001,		0x6080,		MONTH_ENERGY_FILE},

	{	BN_645_DATA,		0x00010001,		0x6000,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00020001,		0x6020,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00030001,		0x6010,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00040001,		0x6030,		MONTH_ENERGY_FILE},

	{	BN_645_DATA,		0x00050001,		0x6040,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00060001,		0x6060,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00070001,		0x6070,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00080001,		0x6050,		MONTH_ENERGY_FILE},
	
	//A�����
	{	BN_645_DATA,		0x00150000,		0x9070,		NULL},
	{	BN_645_DATA,		0x00160000,		0x9080,		NULL},
	{	BN_645_DATA,		0x00170000,		0xe112,		NULL},
	{	BN_645_DATA,		0x00180000,		0xe113,		NULL},
	{	BN_645_DATA,		0x00190000,		0xe114,		NULL},
	{	BN_645_DATA,		0x001a0000,		0xe115,		NULL},
	{	BN_645_DATA,		0x001b0000,		0xe116,		NULL},
	{	BN_645_DATA,		0x001c0000,		0xe117,		NULL},
	
	//��X������A�����
	{	BN_645_DATA,		0x00150001,		0x4000,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00160001,		0x4020,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00170001,		0x4010,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00180001,		0x4030,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00190001,		0x4040,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x001a0001,		0x4060,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x001b0001,		0x4070,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x001c0001,		0x4050,		PHASE_MONTH_ENERGY_FILE},

	//B�����
	{	BN_645_DATA,		0x00290000,		0x9071,		NULL},
	{	BN_645_DATA,		0x002a0000,		0x9081,		NULL},
	{	BN_645_DATA,		0x002b0000,		0xe122,		NULL},
	{	BN_645_DATA,		0x002c0000,		0xe123,		NULL},
	{	BN_645_DATA,		0x002d0000,		0xe124,		NULL},
	{	BN_645_DATA,		0x002e0000,		0xe125,		NULL},
	{	BN_645_DATA,		0x002f0000,		0xe126,		NULL},
	{	BN_645_DATA,		0x00300000,		0xe127,		NULL},

	//��X������B�����
	{	BN_645_DATA,		0x00290001,		0x4001,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002a0001,		0x4021,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002b0001,		0x4011,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002c0001,		0x4031,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002d0001,		0x4041,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002e0001,		0x4061,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002f0001,		0x4071,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00300001,		0x4051,		PHASE_MONTH_ENERGY_FILE},

	//C�����
	{	BN_645_DATA,		0x003d0000,		0x9072,		NULL},
	{	BN_645_DATA,		0x003e0000,		0x9082,		NULL},
	{	BN_645_DATA,		0x003f0000,		0xe132,		NULL},
	{	BN_645_DATA,		0x00400000,		0xe133,		NULL},
	{	BN_645_DATA,		0x00410000,		0xe134,		NULL},
	{	BN_645_DATA,		0x00420000,		0xe135,		NULL},
	{	BN_645_DATA,		0x00430000,		0xe136,		NULL},
	{	BN_645_DATA,		0x00440000,		0xe137,		NULL},

	//��X������C�����
	{	BN_645_DATA,		0x003d0001,		0x4002,		PHASE_MONTH_ENERGY_FILE},	//C�������й�����  0x4002
	{	BN_645_DATA,		0x003e0001,		0x4022,		PHASE_MONTH_ENERGY_FILE},	//C�෴���й�����  0x4022
	{	BN_645_DATA,		0x003f0001,		0x4012,		PHASE_MONTH_ENERGY_FILE},	//C������޹�1���� 0x4012
	{	BN_645_DATA,		0x00400001,		0x4032,		PHASE_MONTH_ENERGY_FILE},	//C������޹�2���� 0x4032
	{	BN_645_DATA,		0x00410001,		0x4042,		PHASE_MONTH_ENERGY_FILE},	//C���һ�����޹����� 0x4042
	{	BN_645_DATA,		0x00420001,		0x4062,		PHASE_MONTH_ENERGY_FILE},	//C��ڶ������޹����� 0x4062
	{	BN_645_DATA,		0x00430001,		0x4072,		PHASE_MONTH_ENERGY_FILE},	//C����������޹����� 0x4072
	{	BN_645_DATA,		0x00440001,		0x4052,		PHASE_MONTH_ENERGY_FILE},	//C����������޹����� 0x4052
	
	//����������ʱ��
	{	BN_645_DATA,		0x01010000,		0xa010,		NULL},
	{	BN_645_DATA,		0x01020000,		0xa020,		NULL},
	{	BN_645_DATA,		0x01030000,		0xa110,		NULL},
	{	BN_645_DATA,		0x01040000,		0xa120,		NULL},

	{	BN_645_DATA,		0x01050000,		0xa130,		NULL},
	{	BN_645_DATA,		0x01060000,		0xa150,		NULL},
	{	BN_645_DATA,		0x01070000,		0xa160,		NULL},
	{	BN_645_DATA,		0x01080000,		0xa140,		NULL},

	//��X����������������ʱ��
	{	BN_645_DATA,		0x01010001,		0x7000,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01020001,		0x7020,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01030001,		0x7010,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01040001,		0x7030,		MONTH_DEMAND_FILE},

	{	BN_645_DATA,		0x01050001,		0x7040,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01060001,		0x7060,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01070001,		0x7070,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01080001,		0x7050,		MONTH_DEMAND_FILE},

	//A������������ʱ��
	{	BN_645_DATA,		0x01150000,		0xe810,		NULL},
	{	BN_645_DATA,		0x01160000,		0xe811,		NULL},
	{	BN_645_DATA,		0x01170000,		0xe812,		NULL},
	{	BN_645_DATA,		0x01180000,		0xe813,		NULL},

	{	BN_645_DATA,		0x01190000,		0xe814,		NULL},
	{	BN_645_DATA,		0x011a0000,		0xe815,		NULL},
	{	BN_645_DATA,		0x011b0000,		0xe816,		NULL},
	{	BN_645_DATA,		0x011c0000,		0xe817,		NULL},

	//B������������ʱ��
	{	BN_645_DATA,		0x01290000,		0xe820,		NULL},
	{	BN_645_DATA,		0x012a0000,		0xe821,		NULL},
	{	BN_645_DATA,		0x012b0000,		0xe822,		NULL},
	{	BN_645_DATA,		0x012c0000,		0xe823,		NULL},

	{	BN_645_DATA,		0x012d0000,		0xe824,		NULL},
	{	BN_645_DATA,		0x012e0000,		0xe825,		NULL},
	{	BN_645_DATA,		0x012f0000,		0xe826,		NULL},
	{	BN_645_DATA,		0x01300000,		0xe827,		NULL},

	//C������������ʱ��
	{	BN_645_DATA,		0x013d0000,		0xe830,		NULL},
	{	BN_645_DATA,		0x013e0000,		0xe831,		NULL},
	{	BN_645_DATA,		0x013f0000,		0xe832,		NULL},
	{	BN_645_DATA,		0x01400000,		0xe833,		NULL},

	{	BN_645_DATA,		0x01410000,		0xe834,		NULL},
	{	BN_645_DATA,		0x01420000,		0xe835,		NULL},
	{	BN_645_DATA,		0x01430000,		0xe836,		NULL},
	{	BN_645_DATA,		0x01440000,		0xe837,		NULL},


	//����A������������ʱ��
	{	BN_645_DATA,		0x01150001,		0x2000,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01160001,		0x2020,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01170001,		0x2010,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01180001,		0x2030,		PHASE_MONTH_DEMAND_FILE},

	{	BN_645_DATA,		0x01190001,		0x2040,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x011a0001,		0x2060,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x011b0001,		0x2070,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x011c0001,		0x2050,		PHASE_MONTH_DEMAND_FILE},

	//����B������������ʱ��
	{	BN_645_DATA,		0x01290001,		0x2001,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012a0001,		0x2021,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012b0001,		0x2011,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012c0001,		0x2031,		PHASE_MONTH_DEMAND_FILE},

	{	BN_645_DATA,		0x012d0001,		0x2041,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012e0001,		0x2061,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012f0001,		0x2071,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01300001,		0x2051,		PHASE_MONTH_DEMAND_FILE},

	//����C������������ʱ��
	{	BN_645_DATA,		0x013d0001,		0x2002,		NULL},
	{	BN_645_DATA,		0x013e0001,		0x2022,		NULL},
	{	BN_645_DATA,		0x013f0001,		0x2012,		NULL},
	{	BN_645_DATA,		0x01400001,		0x2032,		NULL},

	{	BN_645_DATA,		0x01410001,		0x2042,		NULL},
	{	BN_645_DATA,		0x01420001,		0x2062,		NULL},
	{	BN_645_DATA,		0x01430001,		0x2072,		NULL},
	{	BN_645_DATA,		0x01440001,		0x2052,		NULL},

	//��ѹ
	{	BN_645_DATA,		0x02010100,		0xb611,		NULL},
	{	BN_645_DATA,		0x02010200,		0xb612,		NULL},
	{	BN_645_DATA,		0x02010300,		0xb613,		NULL},
	{	BN_645_DATA,		0x0201FF00,		0xb61f,		NULL},

	//����
	{	BN_645_DATA,		0x02020100,		0xb621,		NULL},
	{	BN_645_DATA,		0x02020200,		0xb622,		NULL},
	{	BN_645_DATA,		0x02020300,		0xb623,		NULL},
	{	BN_645_DATA,		0x0202FF00,		0xb62f,		NULL},

	//�й�����
	{	BN_645_DATA,		0x02030000,		0xb630,		NULL},
	{	BN_645_DATA,		0x02030100,		0xb631,		NULL},
	{	BN_645_DATA,		0x02030200,		0xb632,		NULL},
	{	BN_645_DATA,		0x02030300,		0xb633,		NULL},
	{	BN_645_DATA,		0x0203FF00,		0xb63f,		NULL},

	//�޹�����
	{	BN_645_DATA,		0x02040000,		0xb640,		NULL},
	{	BN_645_DATA,		0x02040100,		0xb641,		NULL},
	{	BN_645_DATA,		0x02040200,		0xb642,		NULL},
	{	BN_645_DATA,		0x02040300,		0xb643,		NULL},
	{	BN_645_DATA,		0x0204FF00,		0xb64f,		NULL},

	//���ڹ���
	{	BN_645_DATA,		0x02050000,		0xb670,		NULL},
	{	BN_645_DATA,		0x02050100,		0xb671,		NULL},
	{	BN_645_DATA,		0x02050200,		0xb672,		NULL},
	{	BN_645_DATA,		0x02050300,		0xb673,		NULL},
	{	BN_645_DATA,		0x0205FF00,		0xb67f,		NULL},

	//��������
	{	BN_645_DATA,		0x02060000,		0xb650,		NULL},
	{	BN_645_DATA,		0x02060100,		0xb651,		NULL},
	{	BN_645_DATA,		0x02060200,		0xb652,		NULL},
	{	BN_645_DATA,		0x02060300,		0xb653,		NULL},
	{	BN_645_DATA,		0x0206FF00,		0xb65f,		NULL},

	{	BN2,				0x02800002,		0x1054,		NULL},	//����Ƶ��

	/////////////////////////////////////////////////////////////
	//�¼�ͳ��
	{	BN_645_DATA, 		0x10000001,		0xea10,		NULL}, //ʧѹ�ܴ���
	{	BN_645_DATA, 		0x10000002,		0xea11,		NULL}, //ʧѹ���ۼ�ʱ��
	{	BN_645_DATA, 		0x10000101,		0xea12,		NULL}, //���1��ʧѹ����ʱ��
	{	BN_645_DATA, 		0x10000201,		0xea13,		NULL}, //���1��ʧѹ����ʱ��

	{	BN_645_DATA, 		0x10010001,		0xea14,		NULL}, //A��ʧѹ�ܴ���
	{	BN_645_DATA, 		0x10010002,		0xea15,		NULL}, //A��ʧѹ���ۼ�ʱ��
	{	BN_645_DATA, 		0x10020001,		0xea16,		NULL}, //B��ʧѹ�ܴ���
	{	BN_645_DATA, 		0x10020002,		0xea17,		NULL}, //B��ʧѹ���ۼ�ʱ��
	{	BN_645_DATA, 		0x10030001,		0xea18,		NULL}, //C��ʧѹ�ܴ���
	{	BN_645_DATA, 		0x10030002,		0xea19,		NULL}, //C��ʧѹ���ۼ�ʱ��

	{	BN_645_DATA, 		0x03050000,		0xea20,		NULL}, //ȫʧѹ�ܴ��������ۼ�ʱ�� XXXXXX��XXXXXX
	{	BN_645_DATA, 		0x14000001,		0xea21,		NULL}, //��ѹ�������ܴ���
	{	BN_645_DATA, 		0x14000002,		0xea22,		NULL}, //��ѹ�������ܴ���															   
	{	BN_645_DATA, 		0x15000001,		0xea23,		NULL}, //�����������ܴ���
	{	BN_645_DATA, 		0x15000002,		0xea24,		NULL}, //�������������ۼ�ʱ��
	{	BN_645_DATA, 		0x03300200,		0xea25,		NULL}, //���������ܴ���
	{	BN_645_DATA, 		0x03300100,		0xea26,		NULL}, //��������ܴ���
	{	BN_645_DATA, 		0x03300000,		0xea27,		NULL}, //����ܴ���
	{	BN_645_DATA, 		0x03300400,		0xea28,		NULL}, //Уʱ�ܴ���

	{	BN_645_DATA, 		0x13010001,		0xea30,		NULL}, //A������ܴ���
	{	BN_645_DATA, 		0x13010002,		0xea31,		NULL}, //A��������ۼ�ʱ��
	{	BN_645_DATA, 		0x13020001,		0xea32,		NULL}, //B������ܴ���
	{	BN_645_DATA, 		0x13020002,		0xea33,		NULL}, //B��������ۼ�ʱ��
	{	BN_645_DATA, 		0x13030001,		0xea34,		NULL}, //C������ܴ���
	{	BN_645_DATA, 		0x13030002,		0xea35,		NULL}, //C��������ۼ�ʱ��

	{	BN_645_DATA, 		0x18010001,		0xea40,		NULL}, //A��ʧ���ܴ���
	{	BN_645_DATA, 		0x18010002,		0xea41,		NULL}, //A��ʧ�����ۼ�ʱ��
	{	BN_645_DATA, 		0x18020001,		0xea42,		NULL}, //B��ʧ���ܴ���
	{	BN_645_DATA, 		0x18020002,		0xea43,		NULL}, //B��ʧ�����ۼ�ʱ��
	{	BN_645_DATA, 		0x18030001,		0xea44,		NULL}, //C��ʧ���ܴ���
	{	BN_645_DATA, 		0x18030002,		0xea45,		NULL}, //C��ʧ�����ۼ�ʱ��

	{	BN_645_DATA, 		0x1c010001,		0xea50,		NULL}, //A������ܴ���
	{	BN_645_DATA, 		0x1c010002,		0xea51,		NULL}, //A��������ۼ�ʱ��
	{	BN_645_DATA, 		0x1c020001,		0xea52,		NULL}, //B������ܴ���
	{	BN_645_DATA, 		0x1c020002,		0xea53,		NULL}, //B��������ۼ�ʱ��
	{	BN_645_DATA, 		0x1c030001,		0xea54,		NULL}, //C������ܴ���
	{	BN_645_DATA, 		0x1c030002,		0xea55,		NULL}, //C��������ۼ�ʱ��

	///////////////////////////////////////////////////////////////////////////
	//�α���
	{	BN_645_DATA,		0x04000101,		0xc010,		NULL},//����
	{	BN_645_DATA,		0x04000102,		0xc011,		NULL},//ʱ��
	{	BN_645_PARA,		0x04000103,		0xc111,		NULL},//�����������
	{	BN_645_PARA,		0x04000104,		0xc112,		NULL},//����ʱ��
	{	BN_645_PARA,		0x04000105,		0x0640,		NULL},//У��������
	{	BN_645_PARA,		0x04000106,		0x0641,		NULL},//����ʱ�����л�ʱ��
	{	BN_645_PARA,		0x04000107,		0x0642,		NULL}, //������ʱ�α��л�ʱ��

	{	BN_645_PARA,		0x04000201,		0xc310,		NULL},//��ʱ����
	{	BN_645_PARA,		0x04000202,		0xc311,		NULL},//��ʱ�α���
	{	BN_645_PARA,		0x04000203,		0xc312,		NULL},//��ʱ��
	{	BN_645_PARA,		0x04000204,		0xc313,		NULL},//������
	{	BN_645_PARA,		0x04000205,		0xc855,		NULL},//����������
	
	{	BN_645_PARA,		0x04000302,		0xc113,		NULL},//ѭ��ÿ����ʾʱ��
	{	BN_645_PARA,		0x04000303,		0xc115,		NULL},//��ʾ����С��λ	
	{	BN_645_PARA,		0x04000304,		0xc116,		NULL},//��ʾ����(�������)С��λ�� 

	{	BN_645_PARA,		0x04000401,		0x0610,		NULL},//ͨ�ŵ�ַ	//0xc033
	{	BN_645_PARA,		0x04000402,		0xc032,		NULL},//���
	{	BN_645_PARA,		0x04000403,		0x0611,		NULL},//�ʲ��������(ASCII��)
	{	BN_645_PARA,		0x04000404,		0x0612,		NULL},//���ѹ(ASCII��)
	{	BN_645_PARA,		0x04000405,		0x0613,		NULL},//�����/��������(ASCII��)
	{	BN_645_PARA,		0x04000406,		0x0614,		NULL},//������(ASCII��)
	{	BN_645_PARA,		0x04000407,		0x0615,		NULL},//�й�׼ȷ�ȵȼ�(ASCII��)
	{	BN_645_PARA,		0x04000408,		0x0616,		NULL},//�޹�׼ȷ�ȵȼ�(ASCII��)
	{	BN_645_PARA,		0x04000409,		0xc030,		NULL},//����й�����
	{	BN_645_PARA,		0x0400040a,		0xc031,		NULL},//����޹�����
	{	BN_645_PARA,		0x0400040b,		0x0617,		NULL},//����ͺ�(ASCII��)
	{	BN_645_PARA,		0x0400040c,		0x0618,		NULL},//��������(ASCII��)
	{	BN_645_PARA,		0x0400040d,		0x0619,		NULL},//Э��汾��(ASCII��)

	{	BN_645_DATA,		0x04000501,		0xc860,		NULL},//���״̬��1
	{	BN_645_DATA,		0x04000502,		0xc861,		NULL},//���״̬��2
	{	BN_645_DATA,		0x04000503,		0xc862,		NULL},//���״̬��3
	{	BN_645_DATA,		0x04000504,		0xc863,		NULL},//���״̬��4
	{	BN_645_DATA,		0x04000505,		0xc864,		NULL},//���״̬��5
	{	BN_645_DATA,		0x04000506,		0xc865,		NULL},//���״̬��6
	{	BN_645_DATA,		0x04000507,		0xc866,		NULL},//���״̬��7

	{	BN_645_PARA,		0x04000601,		0x0620,		NULL},//�й���Ϸ�ʽ
	{	BN_645_PARA,		0x04000602,		0x0621,		NULL},//�޹���Ϸ�ʽ1
	{	BN_645_PARA,		0x04000603,		0x0622,		NULL},//�޹���Ϸ�ʽ2

	{	BN_645_DATA,		0x04000701,		0x0660,		NULL},//�����ͺ�����ͨ������������
	{	BN_645_DATA,		0x04000702,		0x0661,		NULL},//�Ӵ�ʽ������ͨ������������
	{	BN_645_DATA,		0x04000703,		0x0662,		NULL},//ͨ�ſ�1ͨ������������
	{	BN_645_DATA,		0x04000704,		0x0663,		NULL},//ͨ�ſ�2ͨ������������
	{	BN_645_DATA,		0x04000705,		0x0664,		NULL},//ͨ�ſ�3ͨ������������

	{	BN_645_PARA,		0x04000801,		0xc022,		NULL},//������״̬��
	{	BN_645_PARA,		0x04000802,		0xc41e,		NULL},//�����ղ��õ���ʱ�α��

	/*{	BN_645_PARA,		0x04000901,		0x0623,		NULL},//���ɼ�¼ģʽ��
	{	BN_645_PARA,		0x04000902,		0x0624,		NULL},//��������ģʽ��

	{	BN_645_PARA,		0x04000a01,		0x0630,		NULL},//���ɼ�¼��ʼʱ��
	{	BN_645_PARA,		0x04000a02,		0x0631,		NULL},//��1�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a03,		0x0632,		NULL},//��2�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a04,		0x0633,		NULL},//��3�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a05,		0x0634,		NULL},//��4�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a06,		0x0635,		NULL},//��5�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a07,		0x0636,		NULL},//��6�ฺ�ɼ�¼���ʱ��
*/
	{	BN_645_PARA,		0x04000b01,		0x0650,		NULL},//ÿ�µ�1������  //0xc117
	{	BN_645_PARA,		0x04000b02,		0x0651,		NULL},//ÿ�µ�2������
	{	BN_645_PARA,		0x04000b03,		0x0652,		NULL},//ÿ�µ�3������	

	{	BN_645_PARA,		0x04010000,		0xc32f,		NULL},//��һ�׵�ʱ������ʼ���ڼ�ʱ�α��
	{	BN_645_PARA,		0x04010001,		0xc33f,		NULL},//��һ�׵�һ��ʱ�α��1ʱ����ʼʱ�估���ʺ�	
	{	BN_645_PARA,		0x04010002,		0xc34f,		NULL},//��һ�׵ڶ���ʱ�α��1ʱ����ʼʱ�估���ʺ�	
	{	BN_645_PARA,		0x04010003,		0xc35f,		NULL},//��һ�׵�����ʱ�α��1ʱ����ʼʱ�估���ʺ�	
	{	BN_645_PARA,		0x04010004,		0xc36f,		NULL},//��һ�׵�����ʱ�α��1ʱ����ʼʱ�估���ʺ�	
	{	BN_645_PARA,		0x04010005,		0xc37f,		NULL},//��һ�׵�����ʱ�α��1ʱ����ʼʱ�估���ʺ�	
	{	BN_645_PARA,		0x04010006,		0xc38f,		NULL},//��һ�׵�����ʱ�α��1ʱ����ʼʱ�估���ʺ�	
	{	BN_645_PARA,		0x04010007,		0xc39f,		NULL},//��һ�׵�����ʱ�α��1ʱ����ʼʱ�估���ʺ�	
	{	BN_645_PARA,		0x04010008,		0xc3af,		NULL},//��һ�׵ڰ���ʱ�α��1ʱ����ʼʱ�估���ʺ�

	{	BN_645_PARA,		0x04030001,		0xc861,		NULL},//��1�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030002,		0xc862,		NULL},//��2�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030003,		0xc863,		NULL},//��3�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030004,		0xc864,		NULL},//��4�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030005,		0xc865,		NULL},//��5�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030006,		0xc866,		NULL},//��6�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030007,		0xc867,		NULL},//��7�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030008,		0xc868,		NULL},//��8�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030009,		0xc869,		NULL},//��9�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403000a,		0xc86a,		NULL},//��10�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403000b,		0xc86b,		NULL},//��11�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403000c,		0xc86c,		NULL},//��12�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403000d,		0xc86d,		NULL},//��13�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403000e,		0xc86e,		NULL},//��14�����������ڼ���ʱ�α��

	{	BN_645_PARA,		0x0403000f,		0xc871,		NULL},//��15�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030010,		0xc872,		NULL},//��16�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030011,		0xc873,		NULL},//��17�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030012,		0xc874,		NULL},//��18�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030013,		0xc875,		NULL},//��19�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030014,		0xc876,		NULL},//��20�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030015,		0xc877,		NULL},//��21�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030016,		0xc878,		NULL},//��22�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030017,		0xc879,		NULL},//��23�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030018,		0xc87a,		NULL},//��24�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030019,		0xc87b,		NULL},//��25�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403001a,		0xc87c,		NULL},//��26�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403001b,		0xc87d,		NULL},//��27�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403001c,		0xc87e,		NULL},//��28�����������ڼ���ʱ�α��

	{	BN_645_PARA,		0x0403001d,		0xc881,		NULL},//��29�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403001e,		0xc882,		NULL},//��30�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403001f,		0xc883,		NULL},//��31�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030020,		0xc884,		NULL},//��32�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030021,		0xc885,		NULL},//��33�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030022,		0xc886,		NULL},//��34�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030023,		0xc887,		NULL},//��35�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030024,		0xc888,		NULL},//��36�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030025,		0xc889,		NULL},//��37�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030026,		0xc88a,		NULL},//��38�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030027,		0xc88b,		NULL},//��39�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030028,		0xc88c,		NULL},//��40�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x04030029,		0xc88d,		NULL},//��41�����������ڼ���ʱ�α��
	{	BN_645_PARA,		0x0403002a,		0xc88e,		NULL},//��42�����������ڼ���ʱ�α��

	{	BN_645_PARA,		0x04800001,		0x0643,		NULL},//��������汾��(ASCII��)
	{	BN_645_PARA,		0x04800002,		0x0644,		NULL},//����Ӳ���汾��(ASCII��)
	{	BN_645_PARA,		0x04800003,		0x0645,		NULL},//���ұ��(ASCII��)

	{	BN_645_PARA,		0x04090101,		0xc810,		NULL},//ʧѹ�¼���ѹ��������
	{	BN_645_PARA,		0x04090102,		0xc811,		NULL},//ʧѹ�¼���ѹ�ָ�����
	{	BN_645_PARA,		0x04090103,		0xc812,		NULL},//ʧѹ�¼�������������
	{	BN_645_PARA,		0x04090104,		0xc813,		NULL},//ʧѹ�¼��ж���ʱʱ��

	{	BN_645_PARA,		0x04090201,		0xc814,		NULL},//Ƿѹ�¼���ѹ��������
	{	BN_645_PARA,		0x04090202,		0xc815,		NULL},//Ƿѹ�¼��ж���ʱʱ��

	{	BN_645_PARA,		0x04090301,		0xc816,		NULL},//��ѹ�¼���ѹ��������
	{	BN_645_PARA,		0x04090302,		0xc817,		NULL},//��ѹ�¼��ж���ʱʱ��

	{	BN_645_PARA,		0x04090401,		0xc818,		NULL},//�����¼���ѹ��������          
	{	BN_645_PARA,		0x04090402,		0xc819,		NULL},//�����¼�������������          
	{	BN_645_PARA,		0x04090403,		0xc81a,		NULL},//�����¼��ж���ʱʱ��          
                                                                                  
	{	BN_645_PARA,		0x04090501,		0xc81b,		NULL},//��ѹ��ƽ������ֵ              
	{	BN_645_PARA,		0x04090502,		0xc81c,		NULL},//��ѹ��ƽ�����ж���ʱʱ��      
                                                                                  
	{	BN_645_PARA,		0x04090601,		0xc81d,		NULL},//������ƽ������ֵ              
	{	BN_645_PARA,		0x04090602,		0xc81e,		NULL},//������ƽ�����ж���ʱʱ��      
                                                                                  
	{	BN_645_PARA,		0x04090701,		0xc820,		NULL},//ʧ���¼���ѹ��������          
	{	BN_645_PARA,		0x04090702,		0xc821,		NULL},//ʧ���¼�������������          
	{	BN_645_PARA,		0x04090703,		0xc822,		NULL},//ʧ���¼�������������          
	{	BN_645_PARA,		0x04090704,		0xc823,		NULL},//ʧ���¼��ж���ʱʱ��          
                                                                                  
	{	BN_645_PARA,		0x04090801,		0xc824,		NULL},//�����¼�������������          
	{	BN_645_PARA,		0x04090802,		0xc825,		NULL},//�����¼��ж���ʱʱ��          
                                                                                  
	{	BN_645_PARA,		0x04090901,		0xc826,		NULL},//�����¼���ѹ��������          
	{	BN_645_PARA,		0x04090902,		0xc827,		NULL},//�����¼�������������          
	{	BN_645_PARA,		0x04090903,		0xc828,		NULL},//�����¼��ж���ʱʱ��          
                                                                                  
	{	BN_645_PARA,		0x04090a01,		0xc829,		NULL},//���������¼��й����ʴ�������  
	{	BN_645_PARA,		0x04090a02,		0xc82a,		NULL},//���������¼��ж���ʱʱ��      
                                                                                  
	{	BN_645_PARA,		0x04090b01,		0xc82b,		NULL},//�����¼��й����ʴ�������      
	{	BN_645_PARA,		0x04090b02,		0xc82c,		NULL},//�����¼��ж���ʱʱ��          
                                                                                  
	{	BN_645_PARA,		0x04090c01,		0xc82d,		NULL},//��ѹ��������                  
	{	BN_645_PARA,		0x04090c02,		0xc82e,		NULL},//��ѹ��������                  
                                                                                  
	{	BN_645_PARA,		0x04090d01,		0xc830,		NULL},//�й����������¼�������������  
	{	BN_645_PARA,		0x04090d02,		0xc831,		NULL},//�޹����������¼�������������  
	{	BN_645_PARA,		0x04090d03,		0xc832,		NULL},//���������¼��ж���ʱʱ��      
                                                                                  
	{	BN_645_PARA,		0x04090e01,		0xc833,		NULL},//�ܹ������������޷�ֵ          
	{	BN_645_PARA,		0x04090e02,		0xc834,		NULL},//�ܹ��������������ж���ʱʱ��  
                                                                                  
	{	BN_645_PARA,		0x04090f01,		0xc835,		NULL},//�������ز�ƽ����ֵ            
	{	BN_645_PARA,		0x04090f02,		0xc836,		NULL},//�������ز�ƽ�ⴥ����ʱʱ��    

	{	BN_645_PARA,		0x04000901,		0xc840,		NULL},//���ɼ�¼ģʽ��
	//{	BN_645_PARA,		0x04000902,		0xc841,		NULL},//��������ģʽ��
	{	BN_645_PARA,		0x04000a01,		0xc842,		NULL},//���ɼ�¼��ʼʱ��
	{	BN_645_PARA,		0x04000a02,		0xc843,		NULL},//��1�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a03,		0xc844,		NULL},//��2�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a04,		0xc845,		NULL},//��3�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a05,		0xc846,		NULL},//��4�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a06,		0xc847,		NULL},//��5�ฺ�ɼ�¼���ʱ��
	{	BN_645_PARA,		0x04000a07,		0xc848,		NULL},//��6�ฺ�ɼ�¼���ʱ��

	{	BN_645_PARA,		0x04000902,		0xc849,		NULL},//��ʱ��������ģʽ��
	{	BN_645_PARA,		0x04000903,		0xc84a,		NULL},//˲ʱ��������ģʽ��
	{	BN_645_PARA,		0x04000904,		0xc84b,		NULL},//Լ����������ģʽ��
	{	BN_645_PARA,		0x04000905,		0xc84c,		NULL},//���㶳������ģʽ��
	{	BN_645_PARA,		0x04000906,		0xc84d,		NULL},//�ն�������ģʽ��

	{	BN_645_PARA,		0x04001201,		0xc850,		NULL},//���㶳����ʼʱ��
	{	BN_645_PARA,		0x04001202,		0xc851,		NULL},//���㶳��ʱ����
	{	BN_645_PARA,		0x04001203,		0xc852,		NULL},//�ն���ʱ��

	{	BN_645_PARA,		0x0280000b,		0xea60,		NULL},//��ǰ���ݵ��
	{	BN_645_PARA,		0x02800020,		0xea61,		NULL},//��ǰ���
	{	BN_645_PARA,		0x02800021,		0xea62,		NULL},//��ǰ���ʵ��
};



bool Map07To97ID(DWORD dw07Id, T07To97IDCfg* pIdCfg)
{
	for (WORD i=0; i<sizeof(g_07to97IdMap)/sizeof(T07To97IDCfg); i++)
	{
		if (dw07Id == g_07to97IdMap[i].dw07Id)
		{
			*pIdCfg = g_07to97IdMap[i];
			return true;
		}
	}

	return false;
}

WORD Map97ToBank(DWORD dw97Id)
{
	for (WORD i=0; i<sizeof(g_07to97IdMap)/sizeof(T07To97IDCfg); i++)
	{
		if (dw97Id == g_07to97IdMap[i].w97Id)
			return g_07to97IdMap[i].bBank;	
	}

	return 0;
}

//����ֵ��>0�򷵻�645��ʽ���ȣ�<=0ʧ��
WORD FmtGBTo645(WORD wBn, WORD wID, BYTE* pbIn, BYTE* pbOut)
{
	WORD wRead=0;
	WORD i, wGBLen, w645Len, wIdNum=1;
	BYTE bTmpBuf[80];
	DWORD dwVal=0;
	bool fBlockID = ((wID&0x000f) == 0x000f);
	if ((wID & 0xf000) == 0x9000 || (wID & 0xf000) == 0xe000)
	{		
		if ((wID >= 0x9000 && wID < 0x9110) || (wID > 0x9400 && wID < 0x9510) || (wID > 0x9800 && wID < 0x9910) || 
			(wID >= 0xe000 && wID < 0xe110) || (wID >= 0xe200 && wID < 0xe210))	//�й�
		{
			wGBLen = 5;
			w645Len = 4;
			if (fBlockID == true)
				wIdNum = TOTAL_RATE_NUM;			

			for (i=0; i<wIdNum; i++)		//RATE_NUM+1
			{		    	    
				memcpy(bTmpBuf, pbIn+i*wGBLen+1, w645Len);
				memcpy(pbOut+i*w645Len, bTmpBuf, w645Len);			    	
			}
			wRead = w645Len*wIdNum;		//RATE_NUM+1	    	
		}
		else if ((wID >= 0x9110 && wID <= 0x916f) || (wID >= 0x9510 && wID <= 0x956f) || (wID >= 0x9910 && wID <= 0x996f)
				|| (wID&0xff00) == 0xe100 || (wID&0xff00) == 0xe300)	//�޹�
		{
			w645Len = 4;
			if (fBlockID)
				wIdNum = TOTAL_RATE_NUM;	//RATE_NUM+1

			memcpy(pbOut, pbIn, w645Len*wIdNum);
			wRead = w645Len*wIdNum;
		}		
	}	
	else if ((wID & 0xff00) == 0xb600)
	{
		if (wID < 0xb620)	//U
		{
			if (fBlockID)
				wGBLen = 2*3;
			else
				wGBLen = 2;

			memcpy(pbOut, pbIn, wGBLen);
			wRead = wGBLen;
		}
		else if (wID < 0xb650)	//I, P
		{		
			wGBLen = 3;
			if (fBlockID == true)
			{
				if (wID < 0xb630)	//I
					wIdNum = 3;
				else
					wIdNum = 4;
			}

			memcpy(pbOut, pbIn, wGBLen*wIdNum);
			for (i=0; i<wIdNum; i++)
				pbOut[wGBLen*i+2] &= 0x7f;

			wRead = wGBLen*wIdNum;
		}
		else if(wID < 0xb660)	//COS
		{
			wGBLen = 2;
			w645Len = 2;
			if (fBlockID)
				wIdNum = 4;

			memcpy(pbOut, pbIn, wGBLen*wIdNum);
			for (i=0; i<wIdNum; i++)
				pbOut[wGBLen*i+1] &= 0x7f;

			wRead = w645Len*wIdNum;
		}
		else if (wID>=0xb670 && wID < 0xb680)	//S
		{
			wGBLen = 3;
			if (fBlockID == true)
				wIdNum = 4;			

			memcpy(pbOut, pbIn, wGBLen*wIdNum);
			for (i=0; i<wIdNum; i++)
				pbOut[wGBLen*i+2] &= 0x7f;

			wRead = wGBLen*wIdNum;
		}
	}

	if (wRead  <= 0)	//��ʽ����Ҫת����ֱ�ӿ���
	{
		int iLen = GetItemLen(wBn, wID);
		if (iLen > 0)
		{
			memcpy(pbOut, pbIn, iLen);
			wRead = iLen;
		}
	}

	return wRead;
}



/*˵��: ���һ���¶����¼
 *@szTbName:    ��򿪵��ļ���;
 *@bPtr: Ҫ����ϴ��ĸ��µĶ�������,��1��ʼ;
 *@pbBuf:       �涳�����ݻ�����;
 *@iLen:        ���ݻ�������С;
 ����ֵ: <=0����; >0������ݵĴ�С
*/
int ReadLastNRec(char* szTbName, BYTE bPtr, BYTE* pbBuf, int iLen)
{
	if (iLen <= 0)
		return -1;
		
	int fd = TdbOpenTable(szTbName, O_RDONLY);
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("ReadFrzRec: fail to open table:%s.\n", szTbName));
   		//return -2;
		memset(pbBuf, 0, iLen);	//INVALID_645_DATA
		return iLen;
	}
	
    int iRet, iIdx;
	iIdx = GetRecIdx(fd, bPtr);
	if (iIdx < 0)
		memset(pbBuf, 0, iLen);	//INVALID_645_DATA
	else
	{
		iRet = TdbReadRec(fd, iIdx, pbBuf, iLen);
		if (iRet <= 0)
		{
			DTRACE(DB_TASK, ("ReadFrzRec: TdbReadRec fail! bPtr:%d, iIdx:%d, iRet:%d.\r\n", bPtr, iIdx, iRet));
			memset(pbBuf, INVALID_645_DATA, iLen);
		}
	}	
	TdbCloseTable(fd);
	
	return iLen;
}


/*˵��:��ȡһ����¼�������ַ
 *@szTbName:    ��򿪵��ļ���;
 *@bPtr: Ҫ����ϴ��ĸ��µĶ�������,��1��ʼ;
 ����ֵ: <=0����; >0��õ������ַ
*/
int GetRecPhyIdx(char* szTbName, BYTE bPtr)
{	
	int iIdx;
	int fd = TdbOpenTable(szTbName, O_RDONLY);
	
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("ReadFrzRec: fail to open table:%s.\n", szTbName));
		TdbCloseTable(fd);
		return -1;
	}
	
	iIdx = GetRecIdx(fd, bPtr);
	TdbCloseTable(fd);
	return iIdx;
}
int Read97Id(WORD wBn, WORD wID, BYTE* pbTx)
{
	int iLen = -1;
	BYTE bVer=0;
	BYTE bTmpBuf[30];
	BYTE bBuf[200];

	/*if (IsMonthFrzID(wID, bTmpBuf, 4))	//�ϵڼ�������
	{
		iLen = GetMonthFrzData(bTmpBuf, 4, pbTx);
	}
	else*/
	{
		memset(bBuf, 0, sizeof(bBuf));
		iLen = ReadItemEx(wBn, PN0, wID, bBuf);
		if (iLen > 0)
		{
			iLen = FmtGBTo645(wBn, wID, bBuf, pbTx);	//תΪ07-645��ʽ
			
			int iRet = ReadItemEx(BN_645_PARA, MTRPN, 0xc890, (BYTE* )&bVer);	//ʵ��ʱ����
			if (iRet > 0 && bVer == 1)	//���Ҫ�󷵻�ʵ�ʸ���ʱ�η��ʲ���
			{
				if (wBn == BN_645_PARA)
				{
					if ((wID&0xff0f) == 0xc30f && wID > 0xc330)
					{
						BYTE bRateNum=0;
						ReadItemEx(BN_645_PARA, MTRPN, 0xc312, (BYTE* )&bRateNum);	//ʵ��ʱ����
						bRateNum = BcdToByte(bRateNum);
						if (bRateNum>RATE_PERIOD_NUM || bRateNum==0)
							bRateNum = RATE_PERIOD_NUM;

						iLen = 3*bRateNum;		//���Ҫ�󷵻�ʵ�ʵ�ʱ���� 3Ϊ1��ʱ�εĳ���
					}
					else if (wID == 0xc32f)
					{
						BYTE bZoneNum=0;
						ReadItemEx(BN_645_PARA, MTRPN, 0xc310, (BYTE* )&bZoneNum);	//ʵ����ʱ����
						//��ʱ����
						bZoneNum = BcdToByte(bZoneNum);      
						if (bZoneNum > MAX_ZONE_NUM)
							bZoneNum = MAX_ZONE_NUM;

						iLen = 3*bZoneNum;		//���Ҫ�󷵻�ʵ����ʱ����
					}
				}	
			}			
		}
		else
		{
			int iItemLen = GetItemLen(wBn, wID);
			if (iItemLen > 0)
			{
				iLen = iItemLen;
				memset(pbTx, INVALID_645_DATA, iItemLen);
			}
		}
	}

	return iLen;
}

//�����������д����ID
bool IsDemandId(WORD wDesID)
{
	WORD wDmdID[] = { 0xa010, 0xa020, 0xa110, 0xa120, 0xa130, 0xa140, 0xa150, 0xa160,	//��ǰ������
					  0x7000, 0x7010, 0x7020, 0x7030, 0x7040, 0x7050, 0x7060, 0x7070,	//����������
					  0xe810, 0xe812, 0xe811, 0xe813, 0xe814, 0xe817, 0xe815, 0xe816, 
					  0xe820, 0xe822, 0xe821, 0xe823, 0xe824, 0xe827, 0xe825, 0xe826, 
					  0xe830, 0xe832, 0xe831, 0xe833, 0xe834, 0xe837, 0xe835, 0xe836,	//��ǰA/B/C��������
					  0x2000, 0x2010, 0x2020, 0x2030, 0x2040, 0x2050, 0x2060, 0x2070,
					  0x2001, 0x2011, 0x2021, 0x2031, 0x2041, 0x2051, 0x2061, 0x2071,
					  0x2002, 0x2012, 0x2022, 0x2032, 0x2042, 0x2052, 0x2062, 0x2072,	//����A/B/C��������
	};

	WORD i;
	for (i=0; i<sizeof(wDmdID)/sizeof(WORD); i++)
	{
		if (wDmdID[i] == wDesID)
			return true;
	}

	return false;
}


//����ֵ��>0�򷵻�645-1997��ʽ���� <=0:ʧ��
int Read07Id(DWORD dw07Id, BYTE* pbTx)
{
	int iLen, n645Len=-1;
	BYTE b07Id[4];	
	BYTE bBuf[200];	//
	BYTE bTmpBuf[60];

	if (dw07Id>=0x020a0101 && dw07Id<=0x020b03ff)
	{
		WORD wHarId = 0;
		BYTE n;
		BYTE bPhase = (dw07Id>>8) & 0xff;
		if (bPhase>=1 && bPhase<=3)
		{
			if ((dw07Id&0xffff0000) == 0x020a0000)
				wHarId = 0x2101 + bPhase - 1;
			else if ((dw07Id&0xffff0000) == 0x020b0000)
				wHarId = 0x2104 + bPhase - 1;
		}

		if (wHarId != 0)
		{
			n = (BYTE )(dw07Id&0xff);	//г������
			if (n == 0xff)	//��ID
			{
				return ReadItemEx(BN2, PN0, wHarId, pbTx);
			}
			else if (n>=1 && n<=21)	//��ID
			{
				ReadItemEx(BN2, PN0, wHarId, bTmpBuf);
				memcpy(pbTx, &bTmpBuf[(n-1)*2], 2); 
				return 2;
			}
		}

		//ID������������������ȥ
	}

	iLen = Read07TaskId(dw07Id, pbTx); //���û��ƥ���ID�򷵻�-4
	if (iLen != -4) //��ƥ��ID������û��������
		return iLen;

	for (WORD i=0; i<4; i++)
	{
		b07Id[i] = (BYTE ) (dw07Id >> (i<<3)) & 0xff;
	}

	if (b07Id[0] == 0xff || b07Id[2] == 0xff)	//DI[0]��DI[2]�������ݲ�֧��
		return -1;
	
	if (b07Id[3] < 2 )	//���ܺ�����ID  //����ַ��ʼ���ID
	{
		dw07Id &= 0xffff00ff;	//�ַ��ʼ���IDת�����ٲ�ӳ���
		if (b07Id[0] > 1 && b07Id[0] <= 0xc)	//��X������IDת����һ������ID��ӳ���
			dw07Id = (dw07Id & 0xffffff00) + 1;
	}

	T07To97IDCfg IdCfg;
	bool fValidId = Map07To97ID(dw07Id, &IdCfg);
	if (fValidId == false)
		return -1;	//ʧ�ܴ���

	TTime tmNow;
	WORD wBaseID, wIdNum;
	WORD wBn = IdCfg.bBank;
	WORD w97Id = IdCfg.w97Id;
	char* psTdbName = IdCfg.psTdbName;
	BYTE bRateNo = b07Id[1];	//���ʺ�
	BYTE bLastDayPtr = 0;		//�ڼ�������
	BYTE bVal = 0;
	WORD wDmdTimeId=0;

	if (b07Id[3] < 2 )	//���ܺ�����ID �ַ��ʼ���ID����
	{
		if (b07Id[2] <= 0x0a)
		{
			if (bRateNo == 0xff)
				w97Id |= 0xf;
			else
				w97Id = (w97Id&0xfff0) + bRateNo;
		}

		bLastDayPtr = b07Id[0];		//�ڼ�������
		w97Id |= ((WORD ) (bLastDayPtr&0xf) <<8 );
	}

	if (!IsDemandId(IdCfg.w97Id))	//������ID
	{
		n645Len = Read97Id(wBn, w97Id, pbTx);
	}
	else	// ����IDΪ������ֵ����������ʱ�䣩���
	{
		n645Len = 0;
		if (bRateNo == 0xff)
		{
			wIdNum = 5;
			wBaseID = (w97Id&0xfff0);
		}
		else
		{
			wIdNum = 1;
			wBaseID = w97Id;
		}

		for (WORD j=0; j<wIdNum; j++)
		{
			w97Id = wBaseID + j;
			Read97Id(wBn, w97Id, pbTx);
			pbTx += 3;
			n645Len += 3;

			if ((w97Id&0xf000) == 0xe000)
				wDmdTimeId = w97Id+0x100;
			else
				wDmdTimeId = w97Id+0x1000;
			Read97Id(wBn, wDmdTimeId, pbTx);	//��������ʱ��(������ʱ��) 5Bytes
			if (IsAllAByte(pbTx, 0, 4))	//ԭ����ȫ0���������0
			{
				pbTx[4] = 0;	//��
			}
			else
			{
				if (!bLastDayPtr)	//��ǰ����ʱ�����ݣ�����ǰ��  //�ϣؽ���������ʱ�䲻�ò��꣬�ڱ����¼��ʱ���Ѿ�������
				{
					GetCurTime(&tmNow);
					pbTx[4] = ByteToBcd(tmNow.nYear%100);	//��
				}
			}

			pbTx += 5;
			n645Len += 5;
		}
	}

	return n645Len;
}


//����: ���ݳ���ID�����������
int GetItemLength( WORD wBank, const WORD wID)
{
	switch(wID)
	{
	case 0x901f:
		return GetItemLen(BN0, 0x941f);
	default :
		return GetItemLen(BN0, wID);
	}
}

//����ֵ��-1��ת��ʧ�ܣ�>0��ʾת��������ID�����ݳ���
int FmtGBTo645(WORD wBn, WORD* pwID, WORD wNum, BYTE* pbIn, BYTE* pbOut)
{
	int i, iGBLen, i645Len;
	BYTE* pbBase = pbOut;
	for (i=0; i<wNum; i++)
	{
		iGBLen = GetItemLength(wBn, pwID[i]);
		
		if (iGBLen <= 0)
			return -1;
			
		i645Len = (int)FmtGBTo645(wBn, pwID[i], pbIn, pbOut);

		pbOut += i645Len;		
		pbIn += iGBLen;
	}
	
	return (pbOut-pbBase);
}


//���������ݵ�ǰ��ļ�¼�ŵõ����ڱ��еļ�¼����
//������@fd ���ݿ��ľ��; @iRecNo ��ļ�¼��(��1��ʼ)
//���أ���ǰ���¼�ŵļ�¼����,С��0��ʾ����
int GetRecIdx(const int fd, int iRecNo)
{
	int iRecNum = TdbGetRecNum(fd);	//TDB_ERR_DBLOCKED:-11, TDB_ERR_TBNOEXIST:-3
	if (iRecNum < 0)
		return iRecNum;
		
	if (iRecNo > iRecNum)
		return ERR_OVER_RECNUM;		//ERR_OVER_RECNUM:-20
		
	int iRecPtr = TdbGetRecPtr(fd);	//TDB_ERR_DBLOCKED:-11, TDB_ERR_TBNOEXIST:-3
	if (iRecPtr < 0)
		return iRecPtr;
	
	int iRecIdx, iTmpIdx;
	iTmpIdx = iRecPtr - iRecNo;
	if ( iTmpIdx >= 0 )
		iRecIdx = iTmpIdx;
	else
		iRecIdx = iRecNum + iTmpIdx;
	
	return iRecIdx;
}

//#define AMP_MS_MAX 	(10*1000*3600)	//��λ:0.1����ʱ,��Fmt6ToVal�Ŵ���100������Ϊ10*
#define AMP_MS_MAX 	(10*1000*3600)	//��λ:1����ʱ,��Fmt6ToVal�Ŵ���100������Ϊ100*
									//1000*3600��ʱ�䶼ת���ɺ���
void DoAmpHour()
{
	static DWORD dwPreMS = 0;
	
	if (GetClick() < 10)
		return;

	if (dwPreMS == 0)
	{
		dwPreMS = GetTick();
		return;
	}
	
	BYTE bTmpBuf[6];
	BYTE buf[12];
	if (ReadItemEx(BN0, PN0, 0xb62f, buf)<= 0)//��������ݿ�
		return;
	
	bool fNewVal = false;
	BYTE bI;
	WORD wI[3];
	int  iIntervalMS, iAdd;
	DWORD dwAmpMS, dwCurMS;

	dwCurMS = GetTick();
	iIntervalMS = dwCurMS - dwPreMS;
	if (iIntervalMS <= 0)
		return;
		
	dwPreMS = dwCurMS;
	for (bI=0; bI<3; bI++)
	{
		wI[bI]= (WORD)ABS(Fmt25ToVal(buf+bI*3, 3));
		dwAmpMS = wI[bI] * iIntervalMS;
		g_dwAmpMS[bI] += dwAmpMS;
		
		if (g_dwAmpMS[bI] >= AMP_MS_MAX)
		{
			iAdd = g_dwAmpMS[bI] / AMP_MS_MAX;
			g_dwAmpMS[bI] = g_dwAmpMS[bI] - iAdd * AMP_MS_MAX;	
			
			//0xea01, 0xea02, 0xea03--A��B��C����ܰ����ʱ��, 4�ֽ�
			if (ReadItemEx(BN0, PN0, 0xea01+bI, bTmpBuf)<= 0)
				continue;
			
			DWORD dwTmpVal = BcdToDWORD(bTmpBuf, 4) + iAdd;
			DWORDToBCD(dwTmpVal, bTmpBuf, 4);
			WriteItemEx(BN0, PN0, 0xea01+bI, bTmpBuf);

			fNewVal = true;
		}
	}

	//�����ܰ�ʱ
	if (fNewVal)	
	{
		DWORD dwTotal = 0;
		for (bI=0; bI<3; bI++)
		{
			if (ReadItemEx(BN0, PN0, 0xea01+bI, bTmpBuf)<= 0)
				continue;

			dwTotal += BcdToDWORD(bTmpBuf, 4);
		}

		DWORDToBCD(dwTotal, bTmpBuf, 4);
		WriteItemEx(BN0, PN0, 0xea00, bTmpBuf);
	}
}


//�Ƿ���Ȩ��
bool HaveProgPermit(WORD wID, BYTE bPswPerm)
{
	if (IsInProgState() == false)
		return false;

	if (bPswPerm == 2)	//����ԱȨ��
	{
		return true;
	}
	else if (bPswPerm == 4)	//����ԱȨ��
	{
		if (wID == 0x19)	//���������������
			return true;
	}
	
	return false;
}
#endif

