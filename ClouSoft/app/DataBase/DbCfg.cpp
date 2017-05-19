/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbCfg.cpp
 * ժ    Ҫ��ϵͳ���ݿ�������ļ�,��Ҫ��������ϵͳ�����������ƽṹ
 * ��ǰ�汾��1.0
 * ��    �ߣ��׳ɲ�
 * ������ڣ�2016��8��
 *********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "FaAPI.h"
#include "sysfs.h"
#include "DbAPI.h"
#include "DbConst.h"
#include "FrzTask.h"

//����汾
BYTE g_bTermSoftVer[OOB_SOFT_VER_LEN] = {
						DT_STRUCT, 0x06, 
						DT_VIS_STR, 0x04, 'C', 'L', 'O', 'U',			//���̴��� 4
//						DT_VIS_STR, 0x04, 'Z', 'J', 'S', 'J',			//����汾 4 ,��̨��Զ��������
						DT_VIS_STR, 0x04, '0', '7', '9', 'g',			//����汾 4 ,��̨��Զ��������
//						DT_VIS_STR, 0x06, '1','7', '0','2','2','3',		//����汾����
						DT_VIS_STR, 0x06, '1','7', '0','5','0','8',		//����汾����
						DT_VIS_STR, 0x04, 'V', 'C', '8', '2',			//Ӳ���汾 4 
						DT_VIS_STR, 0x06, '1','6', '1','0','1','3',		//Ӳ���汾����
						DT_VIS_STR, 0x08, '0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
};	//�����豸�����汾��Ϣ

//�ڲ�����汾
BYTE g_bInnerSoftVer[INN_SOFT_VER_LEN] = 
{
	'6', '9', '8', '.', '4', '5', '-', 'Z', 'h', 'e', 'J', 'i', 'a', 'n',  'g', 0,  //���� 16���ֽڣ�ͨ��Ϊʡ��ȫƴ����׼����ΪStandard
	'0', '0', '0', '1',     //�汾4���ֽ� x.xx A ���汾.���汾 ���԰汾����ʽ�鵵�İ汾���԰汾��Ϊ0
	 0x08, 0x05, 0x17,        		 //����3���ֽ� BCD�룬�ն�����������ڡ�
};

TItemDesc g_EngDataDesc[] =   //�����������
{
    {0x0000, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�汾����
	//����й����ܣ�����ʽ��02 05 06 ********
    {0x0001, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����й����ܣ�����ʽ��02 05 06 ********
	//�����й����ܣ��ܣ�A/B/C
    {0x0010, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����й����ܣ�����ʽ��
    {0x0011, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A�������й����ܣ�����ʽ��
    {0x0012, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B�������й����ܣ�����ʽ��
    {0x0013, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C�������й����ܣ�����ʽ��
	//�����й����ܣ��ܣ�A/B/C
    {0x0020, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����й����ܣ�����ʽ��
    {0x0021, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A�෴���й����ܣ�����ʽ��
    {0x0022, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B�෴���й����ܣ�����ʽ��
    {0x0023, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C�෴���й����ܣ�����ʽ��
	//����޹�1���ܣ��ܣ�A/B/C
    {0x0030, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����޹�1���ܣ�����ʽ��
    {0x0031, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A������޹�1���ܣ�����ʽ��
    {0x0032, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B������޹�1���ܣ�����ʽ��
    {0x0033, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C������޹�1���ܣ�����ʽ��
	//����޹�2���ܣ��ܣ�A/B/C
    {0x0040, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����޹�2���ܣ�����ʽ��
    {0x0041, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A������޹�2���ܣ�����ʽ��
    {0x0042, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B������޹�2���ܣ�����ʽ��
    {0x0043, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C������޹�2���ܣ�����ʽ��
	//��1�����޹����ܣ��ܣ�A/B/C
    {0x0050, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0051, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0052, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0053, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��2�����޹����ܣ��ܣ�A/B/C
    {0x0060, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0061, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0062, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0063, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��3�����޹����ܣ��ܣ�A/B/C
    {0x0070, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0071, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0072, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0073, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��4�����޹����ܣ��ܣ�A/B/C
    {0x0080, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0081, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0082, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0083, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�������ڵ��ܣ��ܣ�A/B/C
    {0x0090, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0091, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0092, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0093, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�������ڵ��ܣ��ܣ�A/B/C
    {0x00A0, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x00A1, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x00A2, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x00A3, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й��������ܣ��ܣ�A/B/C
    {0x0110, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0111, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0112, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0113, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й��������ܣ��ܣ�A/B/C
    {0x0120, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0121, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0122, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0123, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й�г�����ܣ��ܣ�A/B/C
    {0x0210, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0211, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0212, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0213, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й�г�����ܣ��ܣ�A/B/C
    {0x0220, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0221, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0222, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0223, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//ͭ���й��ܵ��ܲ��������ܣ�A/B/C
    {0x0300, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0301, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0302, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0303, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й��ܵ��ܲ��������ܣ�A/B/C
    {0x0400, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0401, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0402, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0403, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����ܵ��ܣ��ܣ�A/B/C
    {0x0500, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0501, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0502, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0503, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},

	//�߾�������й����ܣ�����ʽ��02 05 21 ********
    {0x0601, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����й����ܣ�����ʽ��02 05 06 ********
	//�����й����ܣ��ܣ�A/B/C
    {0x0610, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����й����ܣ�����ʽ��
    {0x0611, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A�������й����ܣ�����ʽ��
    {0x0612, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B�������й����ܣ�����ʽ��
    {0x0613, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C�������й����ܣ�����ʽ��
	//�����й����ܣ��ܣ�A/B/C
    {0x0620, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����й����ܣ�����ʽ��
    {0x0621, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A�෴���й����ܣ�����ʽ��
    {0x0622, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B�෴���й����ܣ�����ʽ��
    {0x0623, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C�෴���й����ܣ�����ʽ��
	//����޹�1���ܣ��ܣ�A/B/C
    {0x0630, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����޹�1���ܣ�����ʽ��
    {0x0631, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A������޹�1���ܣ�����ʽ��
    {0x0632, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B������޹�1���ܣ�����ʽ��
    {0x0633, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C������޹�1���ܣ�����ʽ��
	//����޹�2���ܣ��ܣ�A/B/C
    {0x0640, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����޹�2���ܣ�����ʽ��
    {0x0641, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A������޹�2���ܣ�����ʽ��
    {0x0642, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B������޹�2���ܣ�����ʽ��
    {0x0643, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C������޹�2���ܣ�����ʽ��
	//��1�����޹����ܣ��ܣ�A/B/C
    {0x0650, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0651, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0652, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0653, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��2�����޹����ܣ��ܣ�A/B/C
    {0x0660, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0661, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0662, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0663, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��3�����޹����ܣ��ܣ�A/B/C
    {0x0670, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0671, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0672, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0673, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��4�����޹����ܣ��ܣ�A/B/C
    {0x0680, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0681, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0682, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0683, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�������ڵ��ܣ��ܣ�A/B/C
    {0x0690, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0691, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0692, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0693, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�������ڵ��ܣ��ܣ�A/B/C
    {0x06A0, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x06A1, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x06A2, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x06A3, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й��������ܣ��ܣ�A/B/C
    {0x0710, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0711, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0712, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0713, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й��������ܣ��ܣ�A/B/C
    {0x0720, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0721, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0722, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0723, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й�г�����ܣ��ܣ�A/B/C
    {0x0810, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0811, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0812, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0813, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й�г�����ܣ��ܣ�A/B/C
    {0x0820, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0821, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0822, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0823, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//ͭ���й��ܵ��ܲ��������ܣ�A/B/C
    {0x0900, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0901, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0902, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0903, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����й��ܵ��ܲ��������ܣ�A/B/C
    {0x0A00, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0A01, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0A02, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0A03, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//�����ܵ��ܣ��ܣ�A/B/C
    {0x0B00, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0B01, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0B02, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0B03, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
};

TItemDesc g_DemDataDesc[] =   //������������
{
    {0x1000, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�汾����
	//�����й���������ܣ�A/B/C
    {0x1010, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����й��������������ʽ��02 05 06 ********
    {0x1011, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A�������й��������������ʽ��
    {0x1012, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B�������й��������������ʽ��
    {0x1013, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C�������й��������������ʽ��
	//�����й���������ܣ�A/B/C
    {0x1020, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1021, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1022, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1023, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���1�޹���������ܣ�A/B/C
    {0x1030, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1031, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1032, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1033, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���2�޹���������ܣ�A/B/C
    {0x1040, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1041, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1042, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1043, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��1������������ܣ�A/B/C
    {0x1050, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1051, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1052, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1053, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��2������������ܣ�A/B/C
    {0x1060, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1061, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1062, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1063, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��3������������ܣ�A/B/C
    {0x1070, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1071, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1072, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1073, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��4������������ܣ�A/B/C
    {0x1080, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1081, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1082, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1083, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//����������������ܣ�A/B/C
    {0x1090, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1091, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1092, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1093, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//����������������ܣ�A/B/C
    {0x10A0, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x10A1, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x10A2, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x10A3, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���������������й���������ܣ�A/B/C
    {0x1110, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1111, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1112, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1113, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���������ڷ����й���������ܣ�A/B/C
    {0x1120, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1121, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1122, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1123, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��������������޹�1��������ܣ�A/B/C
    {0x1130, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1131, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1132, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1133, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��������������޹�2��������ܣ�A/B/C
    {0x1140, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1141, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1142, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1143, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���������ڵ�1������������ܣ�A/B/C
    {0x1150, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1151, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1152, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1153, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���������ڵ�2������������ܣ�A/B/C
    {0x1160, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1161, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1162, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1163, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���������ڵ�3������������ܣ�A/B/C
    {0x1170, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1171, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1172, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1173, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���������ڵ�4������������ܣ�A/B/C
    {0x1180, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1181, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1182, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1183, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//��������������������������ܣ�A/B/C
    {0x1190, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1191, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1192, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1193, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//���������ڷ���������������ܣ�A/B/C
    {0x11A0, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x11A1, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x11A2, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x11A3, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
};

TItemDesc g_VariableDesc[] =   //���������
{
    {0x2000, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ѹA/B/C������ʽ��02 03 12 ********
    {0x2001, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����A/B/C/���򣬴���ʽ��
    {0x2002, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ѹ���A/B/C������ʽ��
    {0x2003, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ѹ�����н�A/B/C������ʽ��
    {0x2004, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�й������ܣ�A/B/C������ʽ��
    {0x2005, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�޹������ܣ�A/B/C������ʽ��
    {0x2006, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ڹ������ܣ�A/B/C������ʽ��
    {0x2007, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//һ����ƽ���й������ܣ�A/B/C������ʽ��
    {0x2008, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//һ����ƽ���޹������ܣ�A/B/C������ʽ��
    {0x2009, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//һ����ƽ�����ڹ����ܣ�A/B/C������ʽ��
    {0x200A, 	14, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���������ܣ�A/B/C������ʽ��
    {0x200B, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ѹ����ʧ����ܣ�A/B/C������ʽ��
    {0x200C, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��������ʧ����ܣ�A/B/C������ʽ��
    {0x200D, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ѹг��������������ʽ���ܼ�2~n��
    {0x200E, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����г��������������ʽ���ܼ�2~n�Σ�
    {0x200F, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����Ƶ�ʣ�����ʽ��
    {0x2010, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����¶�
    {0x2011, 	3, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ʱ�ӵ�ص�ѹ
    {0x2012, 	3, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ͣ�糭���ص�ѹ
    {0x2013, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ʱ�ӵ�ع���ʱ��

    {0x2014, 	30, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����״̬��
    {0x2015, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�Գ��ܳ�״̬��
    {0x2016, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//֤��״̬��
    {0x2017, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�й�����
    {0x2018, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�޹�����
    {0x2019, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ��������
    {0x201A, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ���
    {0x201B, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ���ʵ��
    {0x201C, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ���ݵ��
    {0x201D, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ȫ��֤ʣ��ʱ��
    {0x201E, 	8, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼�����ʱ��
    {0x2020, 	8, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼�����ʱ��
	{0x2021, 	8, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ݶ���ʱ��
    {0x2022, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼���¼���
    {0x2023, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����¼���
    {0x2024, 	EVT_SRC_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼�����Դ,��ʽ����
    {0x2025, 	12, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼���ǰֵ
    {0x2026, 	3, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ѹ��ƽ��
    {0x2027, 	3, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������ƽ��
    {0x2028, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������
    {0x2029, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ʱֵ
	
	{0x202A, 	17, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//Ŀ�ķ�������ַ

	{0x202c, 	12, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����ǰ��Ǯ���ļ�
	{0x202d, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����ǰ��Ǯ���ļ�
	{0x202e, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ۼƹ�����
//ͳ����ID
//��������ID, 2+5+2+11*(2+5+5)=141,Ŀǰ��11������ֵ������
	{0x2108, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��������ͳ��
	{0x2109, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//Сʱ����ͳ��
	{0x210a, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������ͳ��
	{0x210b, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������ͳ��
	{0x210c, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������ͳ��

//�ۼ�ƽ��ͳ������ID, 2+5+5+5=17
	{0x2118, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����ƽ��
	{0x2119, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//Сʱƽ��
	{0x211a, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ƽ��
	{0x211b, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ƽ��
	{0x211c, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ƽ��
//��ֵͳ������ID,2+5+2+4=33
	{0x2128, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���Ӽ�ֵ
	{0x2129, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//Сʱ��ֵ
	{0x212a, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ռ�ֵ
	{0x212b, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼�ֵ
	{0x212c, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�꼫ֵ
//��ѹ�ϸ�������ID,2+2*(2+5+3+3+5+5)=48
	{0x2130, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ܵ�ѹ�ϸ���
	{0x2131, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����A���ѹ�ϸ���
	{0x2132, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����B���ѹ�ϸ���
	{0x2133, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����C���ѹ�ϸ���
//������й����ʼ�����ʱ��
	{0x2140, 	15, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������й����ʼ�����ʱ��
	{0x2141, 	15, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������й����ʼ�����ʱ��

	{0x2200, 	12, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ͨ������
	{0x2203, 	12, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����ʱ��
	{0x2204, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��λ����


//�ܼ���ID
	{0x2301, 	102, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ����ñ� ֧��4������������
	{0x2302, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��й�����
	{0x2303, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��޹�����
	{0x2304, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼӻ���ʱ����ƽ���й�����
	{0x2305, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼӻ���ʱ����ƽ���޹�����
	{0x2306, 	47, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��������й�����
	{0x2307, 	47, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��������޹�����
	{0x2308, 	47, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��������й�����
	{0x2309, 	47, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��������޹�����
	{0x230a, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ�ʣ��������ѣ�
	{0x230b, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//��ǰ�����¸��ؿغ��ܼ��й����ʶ���ֵ
	{0x230c, 	2, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��黬��ʱ������
	{0x230d, 	3,	 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��鹦���ִ�����
	{0x230e, 	3,	 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ������ִ�����
	{0x230f, 	19, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ����������״̬
	{0x2310, 	28, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//�ܼ��鵱ǰ����״̬
	{0x2311, 	40, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//���㼰��λ	
	
	{0x2404, 	5, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�й�����  --- ��������
	{0x2405, 	5, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�޹�����
	{0x2406, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���������й�����
	{0x2407, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���������й�����
	{0x2408, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���շ����й�����
	{0x2409, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���·����й�����
	{0x2410, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���������޹�����
	{0x2411, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���������޹�����
	{0x2412, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���շ����޹�����
	{0x2413, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���·����޹�����
	{0x2414, 	47, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�����й�����ʾֵ
	{0x2415, 	47, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�����޹�����ʾֵ
	{0x2416, 	47, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�����й�����ʾֵ
	{0x2417, 	47, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�����޹�����ʾֵ
	{0x2418, 	44, 				DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//���㼰��λ(14*3+2)	--- ��������

	{0x2419, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�����й�����ʾֵ-�;���
	{0x241a, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�����޹�����ʾֵ-�;���
	{0x241b, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�����й�����ʾֵ-�;���
	{0x241c, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//�����޹�����ʾֵ-�;���

	{0x2500, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�ۼ�ˮ���ȣ�����
	{0x2501, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�ۼ�������
	{0x2502, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�ۼ�����
	{0x2503, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�ȹ���
	{0x2504, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�ۼƹ���ʱ��
	{0x2505, 	12, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//ˮ��
	{0x2506, 	6, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//���Ǳ�״̬ST

    {0x2600, 	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A���ѹг��������������ʽ���ܼ�2~n��
	{0x2601,	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//B���ѹг��������������ʽ���ܼ�2~n��
	{0x2602,	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//C���ѹг��������������ʽ���ܼ�2~n��
    {0x2603, 	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A�����г��������������ʽ���ܼ�2~n�Σ�
	{0x2604,	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//B�����г��������������ʽ���ܼ�2~n�Σ�
	{0x2605,	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//C�����г��������������ʽ���ܼ�2~n�Σ�
	{0x2606,	(2+2), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//г��������



};

TItemDesc g_EventParaDesc[] =   //�¼���������
{
    {0x2FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�汾����
    
     //�¼����ò���****************************************   
    {0x3000, 	15, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ʧѹ�¼�������5�����ò���
    {0x3001, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�Ƿѹ�¼�������5�����ò���
    {0x3002, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��ѹ�¼�������5�����ò���
    {0x3003, 	12, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����¼�������5�����ò���
    {0x3004, 	17, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ʧ���¼�������5�����ò���
    {0x3005, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����¼�������5�����ò���
    {0x3006, 	12, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����¼�������5�����ò���
    {0x3007, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��������¼�������5�����ò���
    {0x3008, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����¼�������5�����ò���
    {0x3009, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ������й����������¼�������6�����ò���
    {0x300A, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����й����������¼�������6�����ò���
    {0x300B, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��޹����������¼�������5�����ò���
    {0x300C, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��������������¼�������6�����ò���
    //{0x300D, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ȫʧѹ�¼�������6�����ò����������ò���
    /* ��֧��
    {0x300E, 	4, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����Դ�����¼�������6�����ò�������֧��
    */ 
    {0x300F, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��ѹ�������¼�������6�����ò���
    {0x3010, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����������¼�������2������6
    /* ��֧��
    {0x3011, 	4, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����¼�������6�����ò����������ò���
    {0x3012, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����¼�������6�����ò����������ò���
    */
   //{0x3013, 	0, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ������¼�������6�����ò����������ò���
   //{0x3014, 	0, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����������¼�������6�����ò����������ò���
   //{0x3015, 	0, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��¼������¼�������6�����ò����������ò���
    /* ��֧��
    {0x3016, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�Уʱ�¼�������6�����ò����������ò���
    {0x3017, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ʱ�α����¼�������6�����ò����������ò���
    {0x3018, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ʱ�������¼�������6�����ò����������ò���
    {0x3019, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ������ձ���¼�������6�����ò����������ò���
    {0x301A, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����ձ���¼�������6�����ò����������ò���
    {0x301B, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����¼�������6�����ò����������ò���
    {0x301C, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ���ť���¼�������6�����ò����������ò���
    */
    {0x301D, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��ѹ��ƽ���¼�������2������6
    {0x301E, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ������ƽ���¼�������2������6
    /* ��֧��
    {0x301F, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ���բ�¼�������6�����ò����������ò���
    {0x3020, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��բ�¼�������6�����ò����������ò���
    {0x3021, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ڼ��ձ���¼�������6�����ò����������ò���
    {0x3022, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��й���Ϸ�ʽ����¼�������6�����ò����������ò���
    {0x3023, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��޹���Ϸ�ʽ����¼�������6�����ò����������ò���
    {0x3024, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ���ʲ��������¼�������6�����ò����������ò���
    {0x3025, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ���ݱ����¼�������6�����ò����������ò���
    {0x3026, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ���Կ�����¼�������6�����ò����������ò���
    {0x3027, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��쳣�忨�¼�������6�����ò����������ò���
    {0x3028, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����¼������6�����ò����������ò���
    {0x3029, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��˷Ѽ�¼������6�����ò����������ò���
    {0x302A, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�㶨�ų������¼�������2������6
    {0x302B, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��ɿ��������¼�������2������6
    {0x302C, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��Դ�쳣�¼�������2������6
    */
    {0x302D, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�������ز�ƽ���¼�������2������6
    {0x302E, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ʱ�ӹ����¼�������2������6
    {0x302F, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����оƬ�����¼�������2������6
	/* ��֧��
	{0x3030, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ͨ��ģ�����¼�������6
	*/

	//{0x3100, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˳�ʼ���¼�������6�������ò���
	//{0x3101, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˰汾����¼�������6�������ò���
	//{0x3104, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն�״̬����λ�¼�������6�������ò���
#ifdef GW_OOB_DEBUG_0x31050600	
	{0x3105, 	5, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ʱ�ӳ����¼�������6
#else
	{0x3105, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ʱ�ӳ����¼�������6
#endif
	{0x3106, 	SAMPLE_CFG_ID_LEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն�ͣ/�ϵ��¼�������6
	/* ��֧��
	{0x3107, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն�ֱ��ģ����Խ�����¼�������6
	{0x3108, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն�ֱ��ģ����Խ�����¼�������6
	*/
	//{0x3109, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն���Ϣ��֤�����¼�������6�������ò���
	//{0x310A, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˹��ϼ�¼������6�������ò���
	{0x310B, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ʾ���½��¼�������6
#ifdef GW_OOB_DEBUG_0x310C0600
	{0x310C, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����������¼�������6
#else
	{0x310C, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����������¼�������6
#endif
#ifdef GW_OOB_DEBUG_0x310D0600
	{0x310D, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����¼�������6
#else
	{0x310D, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����¼�������6
#endif
#ifdef GW_OOB_DEBUG_0x310E0600
	{0x310E, 	6, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ͣ���¼�������6
#else
	{0x310E, 	8, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�ͣ���¼�������6
#endif
	{0x310F, 	6, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˳���ʧ���¼�������6
	{0x3110, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ͨ�����������¼�������6
	/* ��֧��		
	//{0x3111, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����δ֪���ܱ��¼�������6�������ò���
	//{0x3112, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��̨�����ܱ��¼�������6�������ò���
	*/
	//{0x3114, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˶�ʱ�¼�������6�������ò���
	//{0x3115, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ң����բ��¼������6�������ò���
	{0x3116, 	DIFF_COMP_CFG_ID_LEN, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	1,		},//�й��ܵ������Խ���¼���¼������2������6	28*n+2 = 282, n = 10
	/* ��֧��
	//{0x3117, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����·����״̬��λ�¼���¼������6�������ò���
	*/
	//{0x3118, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˱�̼�¼������6�������ò���
	//{0x3119, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˵�����·�쳣�¼�������6�������ò���
	/* ��֧��
	{0x311A, 	5, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����״̬�л��¼�������6
	//{0x311B, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˶Ե��Уʱ��¼������6�������ò���
	*/
	{0x311C, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����ݱ����ؼ�¼������6

	//{0x3200, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������բ��¼������6�������ò���
	//{0x3201, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����բ��¼������6�������ò���
	//{0x3202, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����������ü�¼������6�������ò���
	//{0x3203, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ظ澯�¼���¼������6�������ò���

	{0x3300, 	CN_RPT_TOTAL_LEN, 		DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼��ϱ�״̬
	{0x3301, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//��׼�¼���¼��Ԫ����ʽ����
	{0x3302, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//��̼�¼�¼���Ԫ
	{0x3303, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//����δ֪���ܱ��¼���Ԫ
	{0x3304, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//��̨�����ܱ��¼���Ԫ
	{0x3305, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//������բ��¼��Ԫ
	{0x3306, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�����բ��¼��Ԫ
	{0x3307, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//��ظ澯�¼���Ԫ
	{0x3308, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����������¼���Ԫ����ʽ����
	{0x3309, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//ͣ�ϵ��¼���¼��Ԫ
	{0x330A, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//ң���¼���¼��Ԫ
	{0x330B, 	1, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�й��ܵ������Խ���¼���¼��Ԫ
	{0x330C, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼������¼���¼��Ԫ����ʽ����
	{0x330D, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˶Ե��Уʱ��¼��Ԫ
	{0x3320, 	EVT_ADDOAD_MAXLEN, 			DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//��������3320

	//�¼���������****************************************
	{0x3600, 	EVT_ATTRTAB_LEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		INMTR_EVT_NUM,		},//IC24�ڱ��¼�������2�������������Ա�  IC7�ڱ��¼�������3�������������Ա�
	{0x3601, 	3, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		INMTR_EVT_NUM,		},//IC24�ڱ��¼�������4������¼��   IC7�ڱ��¼�������5������¼��
	{0x3602, 	2, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		INMTR_EVT_NUM,		},//IC24�ڱ��¼�������11���ϱ���ʶ  IC7�ڱ��¼�������8���ϱ���ʶ
	{0x3603, 	2, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		INMTR_EVT_NUM,		},//IC24�ڱ��¼�������12����Ч��ʶ  IC7�ڱ��¼�������9����Ч��ʶ

	{0x3700, 	EVT_ATTRTAB_LEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		TERM_EXC_NUM,		},//�ն��¼�������3�������������Ա�
	{0x3701, 	3, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		TERM_EXC_NUM,		},//�ն��¼�������5������¼�� 
	{0x3702, 	2, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		TERM_EXC_NUM,		},//�ն��¼�������8���ϱ���ʶ
	{0x3703, 	2, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		TERM_EXC_NUM,		},//�ն��¼�������9����Ч��ʶ

	{0x3704, 	EVT_ADDOI_MAXLEN, 		DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//��������3320 ����3�����ϱ��¼������б�ֻ������= array OI
	//ע:0x3AXX��ЩID�����ڼ��㳤��,���������ݵĴ洢.
	{0x3A00, 	TERM_PRG_LIST_LEN, 		DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//��̼�¼�¼���Ԫ�˱�̶����б�  array OAD	
	{0x3A01, 	600, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//����δ֪���ܱ��¼����ѱ���      array һ���ѱ���
	//0x3A02��2+46*STEP_AREA_SAVE_REC_NUM����,��Ҫ���ڻ���ڴ˼����ֵ
	{0x3A02, 	500, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//��̨�����ܱ��¼���Ԫ�˿�̨���ѱ���  array  һ����̨�����
	{0x3A03, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//������բ��¼��Ԫ���¼�������2���ӹ���  long64(��λ��W������-1)��
	{0x3A04, 	3, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//������բ��¼��Ԫ�˿��ƶ���      OI��
	{0x3A05, 	2, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//������բ��¼��Ԫ����բ�ִ�      bit-string(SIZE(8))��
	{0x3A06, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//������բ��¼��Ԫ�˹��ض�ֵ      long64����λ��kWh������-4����
	{0x3A07, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//������բ��¼��Ԫ����բ����ǰ�ܼ��й�����    long64����λ��kW������-4����
	{0x3A08, 	3, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�����բ��¼��Ԫ�˿��ƶ���      OI��
	{0x3A09, 	2, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�����բ��¼��Ԫ����բ�ִ�      bit-string(SIZE(8))��
	{0x3A0A, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�����բ��¼��Ԫ�˵�ض�ֵ      long64����λ��kWh������-4����
	{0x3A0B, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�����բ��¼��Ԫ����բ����ʱ�ܼӵ�����  long64����λ��kwh/Ԫ������-4��
	{0x3A0C, 	3, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//��ظ澯�¼���Ԫ�˿��ƶ���      OI��
 	{0x3A0D, 	9, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//��ظ澯�¼���Ԫ�˵�ض�ֵ      long64����λ��kWh������-4����	
	{0x3A0E, 	5, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����������¼���Ԫ�˳����ڼ��������ֵ  double-long-unsigned
	{0x3A0F, 	8, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����������¼���Ԫ�˳����ڼ��������ֵ����ʱ��  date_time_s
	{0x3A10, 	3, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//ͣ/�ϵ��¼���¼��Ԫ�����Ա�־     bit-string��SIZE(8)��
	{0x3A11, 	74, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//ң���¼���¼��Ԫ�˿غ�2�����ܼ��鹦�� array long64
 	{0x3A12, 	9, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Ա��ܼ����й��ܵ����� long64����λ��kWh�����㣺-4����
 	{0x3A13, 	9, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�����ܼ����й��ܵ����� long64����λ��kWh�����㣺-4����
 	{0x3A14, 	2, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Խ�����ƫ��ֵ integer����λ��%�����㣺0��	
 	{0x3A15, 	9, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Խ�޾���ƫ��ֵ long64����λ��kWh�����㣺-4��	
	{0x3A16, 	EVT_CLR_LIST_LEN, 		DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼������¼���¼��Ԫ���¼������¼���¼��Ԫ��array OMD
	{0x3A17, 	8, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˶Ե��Уʱ��¼��Ԫ��Уʱǰʱ��    date_time_s��
	{0x3A18, 	2, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˶Ե��Уʱ��¼��Ԫ��ʱ�����      integer����λ���룬�޻��㣩
	//{0x3A19, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����״̬�л��¼���Ԫ��״̬��Ǩ�¼�  array structure 
	{0x3A1A, 	MTEDATACHG_CSD_LEN, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����ݱ����ؼ�¼��Ԫ�˼�����ݶ���  CSD��
	{0x3A1B, 	MTEDATACHG_DATA_LEN, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����ݱ����ؼ�¼��Ԫ�˱仯ǰ����    Data��
	{0x3A1C, 	MTEDATACHG_DATA_LEN, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ����ݱ����ؼ�¼��Ԫ�˱仯������    Data
	//{0x3A1D, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�쳣�忨�¼���¼��Ԫ�˿����к�	  octet-string��
	//{0x3A1E, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�쳣�忨�¼���¼��Ԫ�˲忨������Ϣ��	 unsigned��
	//{0x3A1F, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�쳣�忨�¼���¼��Ԫ�˲忨��������ͷ 	 octet-string��
	//{0x3A20, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�쳣�忨�¼���¼��Ԫ�˲忨������Ӧ״̬  long-unsigned��
	//{0x3A21, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//�˷��¼���¼��Ԫ���˷ѽ��      double-long-unsigned����λ��Ԫ�����㣺-2����
};

BYTE g_EventParaDefault[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x2FF0,Ver(20)
	0x02,0x04,0x12,0x06,0xb4,0x12,0x07,0x4e,0x05,0x00,0x00,0x00,0xfa,0x11,0x3c,	// 0x3000,���ò���,ʧѹ
	0x02,0x02,0x12,0x06,0xb4,0x11,0x3c,			//0x3001,���ò���,Ƿѹ
	0x02,0x02,0x12,0x0a,0x50,0x11,0x3c,			//0x3002,���ò���,��ѹ
	0x02,0x03,0x12,0x05,0x28,0x05,0x00,0x00,0x00,0xfa,0x11,0x3c,	//0x3003,���ò���,����
	0x02,0x04,0x12,0x06,0x04,0x05,0x00,0x00,0x00,0xfa,0x05,0x00,0x00,0x09,0xc4,0x11,0x3c,	// 0x3004,���ò���,ʧ��
	0x02,0x02,0x05,0x00,0x0a,0xfc,0x80,0x11,0x3c,	//0x3005,���ò���,����
	0x02,0x03,0x12,0x05,0x28,0x05,0x00,0x00,0x00,0xfa,0x11,0x3c,	//0x3006,���ò���,����
	0x02,0x02,0x05,0x00,0x00,0x00,0x37,0x11,0x3c,	//0x3007,���ò���,�������� ��λ��W�����㣺-1
	0x02,0x02,0x05,0x00,0x02,0x6a,0xc0,0x11,0x3c,	//0x3008,���ò���,���� ��λ��W�����㣺-1
	0x02,0x02,0x06,0x00,0x02,0x17,0xd6,0x11,0x3c,	//0x3009,���ò���,�����й��������� 1.2*Imax*Un*1.732/2/1000 ����������Ĭ�� ��λ��kW�����㣺-4
	0x02,0x02,0x06,0x00,0x02,0x17,0xd6,0x11,0x3c,	//0x300A,���ò���,�����й��������� 1.2*Imax*Un*1.732/2/1000 ����������Ĭ�� ��λ��kW�����㣺-4
	0x02,0x02,0x06,0x00,0x02,0x17,0xd6,0x11,0x3c,	//0x300B,���ò���,�޹��������� 1.2*Imax*Un*1.732/2/1000 ����������Ĭ�� ��λ��kW�����㣺-4
	0x02,0x02,0x10,0x01,0x2c,0x11,0x3c,			//0x300C,���ò���,��������������
	0x02,0x01,0x11,0x3c,							//0x300F,���ò���,��ѹ������
	0x02,0x01,0x11,0x3c,							//0x3010,���ò���,����������
	0x02,0x02,0x10,0x0b,0xb8,0x11,0x3c,			//0x301D,���ò���,��ѹ��ƽ��
	0x02,0x02,0x10,0x0b,0xb8,0x11,0x3c,			//0x301E,���ò���,������ƽ��
	0x02,0x02,0x10,0x23,0x28,0x11,0x3c,			//0x302D,���ò���,������ƽ��
	0x02,0x01,0x11,0x3c,							//0x302E,���ò���,ʱ�ӹ���
	0x02,0x01,0x11,0x3c,							//0x302F,���ò���,����оƬ����

#ifdef GW_OOB_DEBUG_0x31050600	
	DT_STRUCT,0x01,DT_LONG_U,0x00,0x00,			//0x3105 ���ܱ�ʱ�ӳ����¼�������6
#else
	DT_STRUCT,0x02,DT_LONG_U,0x00,0x00,DT_UNSIGN,0x00,		//0x3105 ���ܱ�ʱ�ӳ����¼�������6
#endif

	DT_STRUCT,0x02,
		DT_STRUCT,0x04,
			DT_BIT_STR,0x08,0x00,
			DT_UNSIGN,0x00,
			DT_UNSIGN,0x05,
			DT_ARRAY,0,	//12,��<��Ӧ��������󻥲������ݽ���Э��ĵ��ܱ��ɼ��ն���ؼ���Ҫ��2016-09-09>Ĭ�ϲ���
/*#ifdef MTREXC_ADDR_TPYE_TSA
				DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//30
				DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//48
				DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//66
				DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//84
#else
				DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//30
				DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//48
				DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//66
				DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//84
#endif*/
		DT_STRUCT,0x06,
			DT_LONG_U,0x00,0x01,
			DT_LONG_U,0x10,0xE0,
			DT_LONG_U,0x00,0x05,
			DT_LONG_U,0x00,0x01,
			DT_LONG_U,0x05,0x28,
			DT_LONG_U,0x06,0xE0,

		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3106 104 �ն�ͣ/�ϵ��¼�������6

	DT_STRUCT,0x01,DT_UNSIGN,0x00,				//0x310B ���ܱ�ʾ���½��¼�������6

#ifdef GW_OOB_DEBUG_0x310C0600
	DT_STRUCT,0x01,DT_DB_LONG_U,0x00,0x00,0x00,0x00, //0x310C 9 �����������¼�������6
#else
	DT_STRUCT,0x02,DT_DB_LONG_U,0x00,0x00,0x00,0x00,DT_UNSIGN,0x00, //0x310C 9 �����������¼�������6
#endif
#ifdef GW_OOB_DEBUG_0x310D0600
	DT_STRUCT,0x01,DT_DB_LONG_U,0x00,0x00,0x00,0x00, //0x310D 9 ���ܱ�����¼�������6
#else
	DT_STRUCT,0x02,DT_DB_LONG_U,0x00,0x00,0x00,0x00,DT_UNSIGN,0x00, //0x310D 9 ���ܱ�����¼�������6
#endif
#ifdef GW_OOB_DEBUG_0x310E0600
	DT_STRUCT,0x01,DT_TI,0x00,0x00,0x00, //0x310E ���ܱ�ͣ���¼�������6
#else
	DT_STRUCT,0x02,DT_TI,0x00,0x00,0x00,DT_UNSIGN,0x00, //0x310E ���ܱ�ͣ���¼�������6
#endif
	DT_STRUCT,0x02,DT_UNSIGN,0x00,DT_UNSIGN,0x00,		//0x310F �ն˳���ʧ���¼�������6
	DT_STRUCT,0x01,DT_DB_LONG_U,0x00,0x00,0x00,0x00,	//0x3110 ��ͨ�����������¼�������6

	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3116�й��ܵ������Խ���¼���¼������2������6	28*n+2 = 282, n = 10
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3//300

	DT_STRUCT,0x01,DT_UNSIGN,0x00,				//0x311C ���ܱ����ݱ����ؼ�¼������6

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3300,�¼��ϱ�״̬
	0x00,										//0x3301
	0x00,										//0x3302
	0x00,										//0x3303
	0x00,										//0x3304	
	0x00,										//0x3305
	0x00,										//0x3306
	0x00,										//0x3307
	0x00,										//0x3308
	0x00,										//0x3309
	0x00,										//0x330A
	0x00,										//0x330B
	0x00,										//0x330C
	0x00,										//0x330D
	
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3320 �������� 632�ֽ�
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3//300
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3320 �������� 352�ֽ�
	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,								//0x3600, IC24�ڱ��¼�������2�������������Ա�
	0x12,0x00,0x0a,							//0x3601, IC24�ڱ��¼�������4������¼��	
	0x16,0x00,								//0x3602, IC24�ڱ��¼�������11���ϱ���ʶ
	0x03,0x01,								//0x3603, IC24�ڱ��¼�������12����Ч��ʶ

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,	//0x3700, 322 IC7����2�������������Ա�

	0x12,0x00,0x0f,	//0x3701 IC7����5 ����¼��
	0x16,0x00, //0x3702 IC7����8 �ϱ���ʶ
	0x03,0x01, //0x3703 IC7����9 ��Ч��ʶ	//FOR TEST Ĭ����Ч������

	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3704 �������� 212�ֽ�
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//0x3704 �������� 212�ֽ�

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x3A00 ��̶����б�  array OAD	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A01����δ֪���ܱ��¼����ѱ���  500�ֽ�

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A02��̨�����ܱ��¼���Ԫ�˿�̨���ѱ���   500�ֽ�

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A03 �¼�������2���ӹ���  long64
	0x00,0x00,0x00,	//0x3A04 ���ƶ���      OI
	0x00,0x00,	//0x3A05 ��բ�ִ�      bit-string
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A06 ���ض�ֵ      long64
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A07 ��բ����ǰ�ܼ��й�����    long64

	0x00,0x00,0x00,	//0x3A08 ���ƶ���      OI
	0x00,0x00,	//0x3A09 ��բ�ִ�      bit-string
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A0A ��ض�ֵ      long64
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A0B ��բ����ʱ�ܼӵ�����    long64

	
	0x00,0x00,0x00,	//0x3A0C ���ƶ���      OI
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A0D ��ض�ֵ      long64
	0x00,0x00,0x00,0x00,0x00,						//0x3A0E,�����ڼ��������ֵ  double-long-unsigned
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 		//0x3A0F,�����ڼ��������ֵ����ʱ��  date_time_s
	0x00,0x00,0x00,	//0x3A10 ���Ա�־     bit-string��SIZE(8)��
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A11 �غ�2�����ܼ��鹦�� array long64

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A12 �й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Ա��ܼ����й��ܵ����� long64����λ��kWh�����㣺-4����
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A13 �й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�����ܼ����й��ܵ����� long64����λ��kWh�����㣺-4����
	0x00,0x00,	//0x3A14 �й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Խ�����ƫ��ֵ integer����λ��%�����㣺0��
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A15 �й��ܵ������Խ���¼���¼��Ԫ��Խ��ʱ�Խ�޾���ƫ��ֵ long64����λ��kWh�����㣺-4��	

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x3A16 �¼������¼���¼��Ԫ��array OMD
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x3A17 �ն˶Ե��Уʱ��¼��Ԫ��Уʱǰʱ��    date_time_s
	0x00,0x00, //0x3A18 �ն˶Ե��Уʱ��¼��Ԫ��ʱ�����      integer����λ���룬�޻��㣩


	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//���ܱ����ݱ����ؼ�¼��Ԫ�˼�����ݶ���  CSD��60�ֽ�
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//���ܱ����ݱ����ؼ�¼��Ԫ�˱仯ǰ����    Data��200�ֽ�
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//���ܱ����ݱ����ؼ�¼��Ԫ�˱仯������    Data��200�ֽ�
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
};

TItemDesc g_EventDataDesc[] =   //�¼����������
{
	{0x3B00, 	14, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		INMTR_IC24EVT_NUM,	},//IC24�ڱ��¼�������3����ǰ��¼��
	{0x3B01, 	50, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		INMTR_IC24EVT_NUM,	},//IC24�ڱ��¼�������10����ǰֵ��¼��
	{0x3B02, 	28, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,					},//���ܱ�ʧѹ�¼�������13��ʧѹͳ��

	{0x3B03, 	3, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		INMTR_EVT_NUM,		},//IC7�ڱ�ԴNULL�¼�������4����ǰ��¼��
	{0x3B04, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		INMTR_EVT_NUM,		},//IC7�ڱ�ԴNULL�¼�������7����ǰֵ��¼��

	{0x3B10, 	3, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		TERM_EXC_NUM,		},//IC7�ն˳����¼�������4����ǰ��¼��
	{0x3B11, 	34,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		MTR_EXC_NUM,				},//IC7�ն˳����¼�������7����ǰֵ��¼��(���¼�����Դ)
	{0x3B12, 	17,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		TERM_EXC_NUM,		},//IC7�ն������¼�������7����ǰֵ��¼��
	{0x3B13, 	18,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x310A �豸���ϼ�¼	enum����ǰֵ��¼��
	{0x3B14, 	21,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x3115 ң����բ��¼	OAD����ǰֵ��¼��
	{0x3B15, 	18,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x3119 �ն˵�����·�쳣�¼�	enum����ǰֵ��¼��
	{0x3B16, 	19,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x3202 ����������ü�¼	OI����ǰֵ��¼��	
	{0x3B17, 	18,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x3106 ͣ�ϵ��¼	enum����ǰֵ��¼��
	{0x3B18, 	35,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x311b �ն˶Ե��Уʱ��¼	tsa����ǰֵ��¼��
};

BYTE g_EventDataDefault[] = 
{
	0x02,0x04,0x12,0x00,0x00,0x12,0x00,0x00,0x12,0x00,0x00,0x12,0x00,0x00,				//0x3B00, IC24�ڱ��¼�������3����ǰ��¼��
	0x02,0x04,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
   			0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
   			0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
   			0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,					//0x3B01, IC24�ڱ��¼�������10����ǰֵ��¼��
	0x02,0x04,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
			0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,									//0x3B02, ���ܱ�ʧѹ�¼�������13��ʧѹͳ��
	0x12,0x00,0x00,																	//0x3B03, IC7�ڱ��¼�������4����ǰ��¼��
	0x01,0x01,0x02,0x02,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,		//0x3B04, IC7�ڱ��¼�������7����ǰֵ��¼��

	0x12,0x00,0x00, //0x3B10, IC7�ն˳����¼�������4����ǰ��¼��

	0x01,0x01,0x02,0x02,
#ifdef MTREXC_ADDR_TPYE_TSA		//GW_OOB_DEBUG_0x31050700
	DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
#else
	DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
#endif
	0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B11, IC7�ն˳����¼�������7����ǰֵ��¼�����¼�����Դ��	

	0x01,0x01,0x02,0x02,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B12, IC7�ն������¼�������7����ǰֵ��¼��	
	0x01,0x01,0x02,0x02,0x22,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B13, IC7�ն������¼�������7����ǰֵ��¼��	
	0x01,0x01,0x02,0x02,0x81,0x00,0x00,0x00,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B14, IC7�ն������¼�������7����ǰֵ��¼��	
	0x01,0x01,0x02,0x02,0x22,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B15, IC7�ն������¼�������7����ǰֵ��¼��	
	0x01,0x01,0x02,0x02,0x80,0x00,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,  //0x3B16, IC7�ն������¼�������7����ǰֵ��¼��	
	0x01,0x01,0x02,0x02,0x16,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B17, IC7�ն������¼�������7����ǰֵ��¼��	

	DT_ARRAY,0x01,
	DT_STRUCT,0x02,
	DT_TSA,0x00,	
	DT_STRUCT,0x02,DT_DB_LONG_U,0x00,0x00,0x00,0x00,DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3B18, IC7�ն������¼�������7����ǰֵ��¼��		
};


TItemDesc g_ParaDesc[] =   //�α��������
{
	{0x3FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�汾����
	{0x4000, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����ʱ�䣬����2,����3,����4,����127
	{0x4001, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ͨ�ŵ�ַ������2, ����ʽ TSA ********
	{0x4002, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ţ�����2
	{0x4003, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ͻ���ţ�����2
	{0x4004, 	27, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�豸����λ�ã�����2,
	{0x4005, 	82, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ַ������2,
	{0x4006, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ʱ��Դ������2������127���ã�����128���ã���
	{0x4007, 	20, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//LCD����������2
	{0x4008, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//����ʱ�����л�ʱ�䣬����2
	{0x4009, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//������ʱ���л�ʱ�䣬����2
	{0x400A, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//���׷�ʱ�����л�ʱ�䣬����2
	{0x400B, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//���׽��ݵ���л�ʱ�䣬����2
	{0x400C, 	12, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//ʱ��ʱ����������2
	{0x400D, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//������������2
	{0x400E, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//г����������������2
	{0x400F, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��Կ������������2

	{0x4010, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����Ԫ����������2
	{0x4011, 	202, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//�������ձ�����2
	{0x4012, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//�����������֣�����2
	{0x4013, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//��������õ���ʱ�α�ţ�����2
	{0x4014, 	114,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//��ǰ��ʱ��������2
	{0x4015, 	114,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//������ʱ��������2
	{0x4016, 	530,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//��ǰ����ʱ�α�����2
	{0x4017, 	530,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//��������ʱ�α�����2
	{0x4018, 	162,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�׷��ʵ�ۣ�����2
	{0x4019, 	162, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����׷��ʵ�ۣ�����2

	{0x401A, 	128, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�׽��ݵ�ۣ�����2
	{0x401B, 	128, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����׽��ݵ�ۣ�����2

	{0x401C, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������������ȣ�����2
	{0x401D, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ѹ��������ȣ�����2
	{0x401E, 	12, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���������ֵ������2
	{0x401F, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���������ֵ������2

	{0x4020, 	12, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����������ֵ������2
	{0x4021, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����������ֵ������2
	{0x4022, 	4, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�忨״̬�֣�����2
	{0x4023, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��֤��Чʱ��������2
	{0x4024, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�޳�������2

	{0x4030, 	14, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_STAT_PARA,	FMT_UNK,		1,		},//��ѹ�ϸ��ʲ���������2

	{0x4100, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//����������ڣ�����2
	{0x4101, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//����ʱ�䣬����2
	{0x4102, 	2,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//У�������ȣ�����2
	{0x4103, 	34, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ʲ�������룬����2
	{0x4104, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ѹ������2
	{0x4105, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����/��������������2
	{0x4106, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������������2
	{0x4107, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�й�׼ȷ�ȵȼ�������2
	{0x4108, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�޹�׼ȷ�ȵȼ�������2
	{0x4109, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��й�����������2
	{0x410A, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��޹�����������2
	{0x410B, 	34, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ��ͺţ�����2
	{0x410C, 	11, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ABC����絼ϵ��������2
	{0x410D, 	11, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ABC����翹ϵ��������2
	{0x410E, 	11, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ABC�������ϵ��������2
	{0x410F, 	11, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ABC�������ϵ��������2

	{0x4110, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ܱ�����������1������2
	{0x4111, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��������ţ�����2
	{0x4112, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//�й���Ϸ�ʽ�����֣�����2
	{0x4113, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//�޹���Ϸ�ʽ1�����֣�����2
	{0x4114, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//�޹���Ϸ�ʽ2�����֣�����2
	{0x4115, 	4, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//IC��
	{0x4116,	(BALANCE_DAY_NUM*6+2), 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,	},//�����գ�����2 
	{0x4117, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ڼ������������ڣ�����2

	{0x4200, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//·�ɱ�����2������127��128��129��134
	{0x4201, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//·����Ϣ��Ԫ������2
	{0x4202, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����ͨ�Ų���������2
	{0x4204, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˹㲥Уʱ������2
	{0x4205,   10, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˵���ַ�㲥Уʱ����������3
	

	{0x4302, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸�����豸������
	{0x4303, 	46, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸�����汾��Ϣ
	{0x4304, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸������������
	{0x4305, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸�������豸�б�
	{0x4306,	13, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸����֧�ֹ�Լ�б�
	{0x4307, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸������������ϱ�
	{0x4308, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸�������������ϱ�
	{0x4309, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸��������������ͨ��
	{0x430a, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����豸�����ϱ�ͨ��


	{0x4400, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//Ӧ������
	{0x4401, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//Ӧ��������֤���룬����2

	//0x4500,0x4501����������ʽһ����������ͬ������ӳ�䵽��ͬ������
	{0x4500, 	153, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---ͨѶ����
	{0x4501, 	24, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---��վͨ�Ų�����
	{0x4502, 	204, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---����ͨ�Ų���
	{0x4503, 	46, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---�汾��Ϣ
	{0x4504, 	1,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---֧�ֹ�Լ�б�
	{0x4505, 	22, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---SIM��ICCID
	{0x4506, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---IMSI
	{0x4507, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---�ź�ǿ��
	{0x4508, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---SIM������
	{0x4509, 	6,	 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//����ͨ��ģ��---����IP

	//0x4510~0x4517 ��̫��ͨ��ģ��,8��ģ�飬ӳ�䵽8����ͬ�Ĳ�����
	{0x4510, 	49, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		8,		},//��̫��ͨ��ģ��---����2��ͨѶ����
	{0x4511, 	24, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		8,		},//��̫��ͨ��ģ��---����3����վͨ�Ų�����
	{0x4512, 	102, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		8,		},//��̫��ͨ��ģ��---����4����������
	{0x4513, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		8,		},//��̫��ͨ��ģ��---����5��MAC��ַ

	{0x4520, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,			FMT_UNK,		1,		},//����ʱ�䣬����3��Уʱģʽ
	{0x4521, 	12, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,			FMT_UNK,		1,		},//����ʱ�䣬����4����׼Уʱ����
};

BYTE g_bParaDefault[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	//0x4000 DataTimeBCD--7byte+��ʽ--8byte
	0x1c,0x14,0x10,0x01,0x01,0x00,0x00,0x00,//ʱ�����ڣ�ʵ�ʳ���ʱ��Ҫȡ��ǰʱ��
	//0x4001�� ͨѶ��ַ��Ĭ��01--8byte
	DT_OCT_STR,0x06,
		0x11,0x22,0x33,0x44,0x55,0x66,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,
	//0x4002�� ��ţ�Ĭ��01---8byte
	DT_OCT_STR,0x06,
		0x00,0x00,0x00,0x00,0x00,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,
	//0x4003, �ͻ�����---8byte
	DT_OCT_STR,0x06,	//��Ч����6
		0x00,0x00,0x00,0x00,0x00,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,

	//0x4004, �豸����λ��---27byte
	DT_STRUCT, 0x03,
		DT_STRUCT, 0x04,	//����
			DT_ENUM,0x00,	//��λ
			DT_UNSIGN,0x00,	//��
			DT_UNSIGN,0x00, //��
			DT_UNSIGN,0x00,	//��
		DT_STRUCT, 0x04,	//γ��
			DT_ENUM,0x00,	//��λ
			DT_UNSIGN,0x00,	//��
			DT_UNSIGN,0x00,	//��
			DT_UNSIGN,0x00,	//��
		DT_DB_LONG_U,0x00,0x00,0x00,0x00,	//�߶ȣ�cm��	

	//0x4005, ���ַ��������趨10�����ַ---82byte
	0x01,0x01,
		DT_OCT_STR, 0x06,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//0x4006,ʱ��Դ---6byte
	DT_STRUCT, 0x02,
		DT_ENUM, 0x01,
		DT_ENUM, 0x01,
	//0x4007,LCD����--20byte
	DT_STRUCT, 0x07,
		DT_UNSIGN, 0x03,
		DT_LONG_U, 0x3c, 0x00,
		DT_LONG_U, 0x1e, 0x00,
		DT_LONG_U, 0x0a, 0x00,
		DT_UNSIGN, 0x05,
		DT_UNSIGN, 0x02,
		DT_UNSIGN, 0x04,
		0x00,
	//0x4008,����ʱ�����л�ʱ�� DT_DATE_TIME_S,---8byte
	DT_DATE_TIME_S,0x07,0xe0,0x01,0x01,0x00,0x00,0x00,
	//0x4009,������ʱ���л�ʱ�� DT_DATE_TIME_S,---8byte
	DT_DATE_TIME_S,0x07,0xe0,0x01,0x01,0x00,0x00,0x00,
	//0x400A,���׷�ʱ�����л�ʱ�� DT_DATE_TIME_S,---8byte
	DT_DATE_TIME_S,0x07,0xe0,0x01,0x01,0x00,0x00,0x00,
	//0x400B,���׽��ݵ���л�ʱ�� DT_DATE_TIME_S,---8byte
	DT_DATE_TIME_S,0x07,0xe0,0x01,0x01,0x00,0x00,0x00,
	//400C,ʱ��ʱ����---12 byte
	0x02, 0x05,
		0x11,MAX_ZONE_NUM,
		0x11,MAX_DAY_CHART_NUM,
		0x11,RATE_PERIOD_NUM,
		0x11,RATE_NUM,
		0x11,MAX_HOLIDAY_NUM,
	//400D,������---2byte
	0x11,0x04,
	//400E,г����������---2byte
	0x11,0x15,
	//400F,�ܳ�������---2byte
	0x11,0x04,
	//4010,����Ԫ����---2byte
	0x11,0x03,
	//4011,�������ձ����20�����հ�---182byte
	0x01,0x14,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
	//4012,������������---3byte
	DT_BIT_STR,0x08,0x4E,
	//4013,�����ղ��õ���ʱ�α��---2byte
	DT_UNSIGN,0x01,
	//4014,��ǰʱ����---114byte
	0x01,MAX_ZONE_NUM,//���14��ʱ��
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	//4015,������ʱ����----114byte
	0x01,MAX_ZONE_NUM,//���14��ʱ��
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	//4016����ǰ����ʱ�α�----530byte
	0x01,MAX_DAY_CHART_NUM,//�����ʱ�����ݶ�Ϊ8
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	//4017,��������ʱ�α�---530byte
	0x01,MAX_DAY_CHART_NUM,//�����ʱ�����ݶ�Ϊ8
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//��ʱ�α����8��
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	//4018,��ǰ�׷��ʵ��---162byte
	0x01,0x20,//���֧��32�����ʵ��--ֻ��
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
	//4019,�����׷��ʵ��---162byte
	0x01,0x20,//���֧��32�����ʵ��--ֻ��
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
	//0x401A,��ǰ�׽��ݵ��----128---ֻ��

	0x02,0x03,
		0x01,0x08,
			0x06,0xc8,0x00,0x00,0x00,0x06,0x90,0x01,0x00,0x00,0x06,0x58,0x02,0x00,0x00,0x06,0x20,0x03,0x00,0x00,0x06,0xe8,0x03,0x00,0x00,0x06,0xdc,0x05,0x00,0x00,0x06,0xd0,0x07,0x00,0x00,0x06,0xb8,0x0b,0x00,0x00,//40
		0x01,0x08,
			0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,//40
		0x01,0x08,//������
			0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,
	//0x401B,�����׽��ݵ��----128---ֻ��
	0x02,0x03,
		0x01,0x08,
			0x06,0xc8,0x00,0x00,0x00,0x06,0x90,0x01,0x00,0x00,0x06,0x58,0x02,0x00,0x00,0x06,0x20,0x03,0x00,0x00,0x06,0xe8,0x03,0x00,0x00,0x06,0xdc,0x05,0x00,0x00,0x06,0xd0,0x07,0x00,0x00,0x06,0xb8,0x0b,0x00,0x00,//40
		0x01,0x08,
			0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,//40
		0x01,0x08,//������
			0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,
	//0x401C,�������������---5byte
	0x06,0x64,0x00,0x00,0x00,
	//0x401D,��ѹ���������---5byte
	0x06,0x64,0x00,0x00,0x00,
	//0x401E,���������ֵ---12byte
	0x02,0x02,0x06,0xe8,0x03,0x00,0x00,0x06,0x64,0x00,0x00,0x00,
	//0x401f,���������ֵ---17byte
	0x02,0x02,0x06,0xe8,0x03,0x00,0x00,0x06,0x64,0x72,0x77,0x16,0x06,0x64,0x00,0x00,0x00,

	//0x4020,����������ֵ--12byte
	0x02,0x02,0x06,0xe8,0x03,0x00,
	0x00,0x06,0x64,0x00,0x00,0x00,
	//0x4021,����������ֵ---17byte
	0x02,0x02,0x06,0xe8,0x03,0x00,
	0x00,0x06,0x64,0x72,0x77,0x16,
	0x06,0x64,0x00,0x00,0x00,
	//4022���忨״̬��----4byte
	0x04,0x10,0x00,0x00,
	//4023,��֤��Чʱ��---3byte
	0x12,0x05,0x00,
	//4024 �޳�
	DT_ENUM, 0x00,
	//4030,��ѹ�ϸ��ʲ���---18byte
	0x02,0x04,0x12,0x00,0x00,0x12,0x00,0x00,0x12,0x00,0x00,0x12,0x00,0x00,
///////////////////begin////////////////////

	//4100,�����������---2byte
	0x11,0x05,
	//4101,����ʱ��---2byte
	0x11,0x05,
	//4102,У��������---2byte
	0x11,0x05,
	//4103,�ʲ��������---34byte
	DT_VIS_STR,0x20,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//4104,���ѹ--8byte
	DT_VIS_STR, 0x06, 
	'2', '2', '0', 'V',	0x00,0x00,
	//4105,�����--8byte
	DT_VIS_STR, 0x06, 
	'1', '.', '5', 'A',	0x00,0x00,
	//4106,������--8byte
	DT_VIS_STR, 0x06, 
	'6', '.', '0', 'A',	0x00,0x00,
	//4107,�й�׼ȷ�ȵȼ�--6byte
	DT_VIS_STR, 0x04, 
	'1', '.', '0', 0x00,
	//4108,�޹�׼ȷ�ȵȼ�--6byte
	DT_VIS_STR, 0x04, 
	'2', '.', '0', 0x00,
	//4109,���ܱ��й�����---5byte
	DT_DB_LONG_U,0x00,0x19,0x00,0x00,
	//410A,���ܱ��޹�����---5byte
	DT_DB_LONG_U,0x00,0x19,0x00,0x00,
	//410B,���ܱ��ͺ�---34byte
	DT_VIS_STR,32,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//410C,ABC����絼ϵ��---11byte
	0x02,0x03,0x10,0x00,0x00,16,0x00,0x00,16,0x00,0x00,
	//410D,ABC����翹ϵ��---11byte
	0x02,0x03,0x10,0x00,0x00,16,0x00,0x00,16,0x00,0x00,
	//410E,ABC�������ϵ��---11byte
	0x02,0x03,0x10,0x00,0x00,16,0x00,0x00,16,0x00,0x00,
	//410F,ABC�������ϵ��---11byte
	0x02,0x03,0x10,0x00,0x00,16,0x00,0x00,16,0x00,0x00,

	//4110,���ܱ�����������1---3byte
	0x04,0x08,0x00,
	//4111 ���������---18byte
	0x0a,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//4112 �й���Ϸ�ʽ������--3byte
	0x04,0x08,0x05,
	//4113 �޹���Ϸ�ʽ1������--3byte
	0x04,0x08,0x41,
	//4114 �޹���Ϸ�ʽ2������--3byte
	0x04,0x08,0x14,
	//4115 IC��---4byte
	0x02,0x01,0x16,0x00,
	//4116 ������--���3�������գ�20byte
	0x01,BALANCE_DAY_NUM,
		0x02,0x02,0x11,0x01,0x11,0x00,
		0x02,0x02,0x11,0x0f,0x11,0x00,
		0x02,0x02,0x11,0x19,0x11,0x00,
	//4117 �ڼ�������������---5byte
	84,0x16,0x01,0x12,0x0f,
	//4200,·�ɱ�
	0x00,
	//4201,·����Ϣ��Ԫ
	0x00,
	//4202,����ͨѶ��Ԫ
	0x00,
	//4204,�㲥Уʱʱ��
	DT_STRUCT, 2,
		DT_TIME, 0x00, 0x00, 0x00,	//�ն˹㲥Уʱ����ʱ��
		DT_BOOL, 0x00,	//�Ƿ�����
	//4205,�ն˵���ַ�㲥Уʱ����
	DT_STRUCT, 3,
		DT_UNSIGN, 0x01,	// ʱ�������ֵ  
		DT_TIME, 0x00, 0x00, 0x00,	//�ն˹㲥Уʱ����ʱ��
		DT_BOOL, 0x00,	//�Ƿ�����

	//4302 �豸������
	DT_VIS_STR, 0x10,
		'0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
		'0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
	//4303 �����豸
	DT_STRUCT, 0x06, 
		DT_VIS_STR, 0x04, 'C', 'L', 'O', 'U',	//���̴��� 4
		DT_VIS_STR, 0x04, '0', '0', '0', '2',	//����汾 4 
		DT_VIS_STR, 0x06, '1','6', '1','0','1','3',	//����汾����
		DT_VIS_STR, 0x04, 'V', '0', '1', '0',	//Ӳ���汾 4 
		DT_VIS_STR, 0x06, '1','6', '1','0','1','3',	//Ӳ���汾����
		DT_VIS_STR, 0x08, '0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
	//4304 �����豸������������
	DT_DATE_TIME_S,
		0x07, 0xE0, 0x0A, 0x1B, 0x11, 0x2A, 0x1E,	//2016-10-27 17:42:30
	//4305 �����豸�������豸�б�
	0x00,
	//4306 �����豸����֧�ֹ�Լ�б�
	DT_ARRAY, 0x01,
	DT_VIS_STR, 9, 'D','L','/','T','6','9','8','4','5',
	//4307 �����豸������������ϱ�
	DT_BOOL, 0x00,
	//4308 �����豸�������������ϱ�
	DT_BOOL, 0x01,
	//4309 �����豸��������������ͨ��
	DT_BOOL, 0x01,
	//430a �����豸�����ϱ�ͨ��
	0x01,0x02,
	0x51,0x45,0x00,0x00,0x00,
	0x51,0x45,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
	//4400��Ӧ������
	0x00,
	//4401��Ӧ��������֤����
	0x00,

	//4500 ����ͨ��ģ��1����ͨѶ����
	DT_STRUCT, 0x0c,
		DT_ENUM, 0x01,	//����ģʽ ���ģʽ��0�����ͻ���ģʽ��1����������ģʽ��2��
		DT_ENUM, 0x00,	//�������ߣ�0�����������1��
		DT_ENUM, 0x00,	//���ӷ�ʽ TCP��0����UDP��1��
		DT_ENUM, 0x00,	//����Ӧ�÷�ʽ ����ģʽ��0����������ģʽ��1��
		DT_ARRAY, 0x01,	//֡���˿��б�
			DT_LONG_U, 0x11, 0x22, 
		DT_VIS_STR, 0x05,		//APN
			'c', 'm', 'n',  'e', 't',
		DT_VIS_STR, 0x04,		///�û���
			'c', 'a', 'r', 'd',	
		DT_VIS_STR, 0x04,		///����
			'c', 'a', 'r', 'd',	
		DT_OCT_STR, 4,	//�����������ַ
			 0xC0, 0x00, 0x00, 0x01,
		DT_LONG_U,	//����˿�
			0x88, 0x88,
		DT_UNSIGN,	//��ʱʱ�估�ط�����
			0x7B,	//��ʱʱ��30S�����Դ���3�� 	
		DT_LONG_U,	//��������
			0x01, 0x2C,	//300s
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00,
		

	//4501 ����ͨ��ģ��1������վͨ�Ų�����
	DT_ARRAY, 2,
		DT_STRUCT, 0x02,
			DT_OCT_STR, 4, 
				0x3A, 0xFB, 0x4A, 0x65,	//58.251.74.101
			DT_LONG_U, 
				0x19, 0x22,	//6434
		DT_STRUCT, 0x02,
			DT_OCT_STR, 4, 
				0x00, 0x00, 0x00, 0x00,
			DT_LONG_U, 
				0x00, 0x00,


	//4502 ����ͨ��ģ��1��������ͨ�Ų���
		DT_STRUCT, 0x03,
			DT_VIS_STR, 8, 	//�������ĺ���
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			DT_ARRAY, 0x01,
				DT_VIS_STR, 8,	//��վ����
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			DT_ARRAY, 0x01, 
				DT_VIS_STR, 8,	//����֪ͨĿ�ĺ���
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		

	//4503 ����ͨ��ģ��1�����汾��Ϣ   ��ע��������ҪGPRSģ�����Ϣ����ʱ���ն˰汾��Ϣ������
	DT_STRUCT, 0x06, 
		DT_VIS_STR, 0x04, 'C', 'L', 'O', 'U',	//���̴��� 4
		DT_VIS_STR, 0x04, 'V', '1', '.', '1',	//����汾 4 
		DT_VIS_STR, 0x06, '1','6', '1','0','1','3',	//����汾����
		DT_VIS_STR, 0x04, 'V', '0', '1', '0',	//Ӳ���汾 4 
		DT_VIS_STR, 0x06, '1','6', '1','0','1','3',	//Ӳ���汾����
		DT_VIS_STR, 0x08, '0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
	//4504 ����ͨ��ģ��1����֧�ֹ�Լ�б�
	0x00,
	//4505 ����ͨ��ģ��1����SIM��ICCID
	DT_VIS_STR,0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//4506 ����ͨ��ģ��1����IMSI
	DT_VIS_STR,0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 
	//4507 ����ͨ��ģ��1�����ź�ǿ��
	DT_LONG, 0x00, 0x00,
	//4508 ����ͨ��ģ��1����SIM������
	DT_OCT_STR, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//4509 ����ͨ��ģ��1��������IP
	DT_OCT_STR, 0x04, 
	0x00, 0x00, 0x00, 0x00,
	//0x4510~0x4517 ��̫��ģ������2����ͨ������
	DT_STRUCT, 0x08,
		DT_ENUM, 0x01,	//����ģʽ ���ģʽ��0�����ͻ���ģʽ��1����������ģʽ��2��
		DT_ENUM, 0x00,	//���ӷ�ʽ TCP��0����UDP��1��
		DT_ENUM, 0x00,	//����Ӧ�÷�ʽ ����ģʽ��0����������ģʽ��1��
		DT_ARRAY, 0x01,	//֡���˿��б�
			DT_LONG_U, 0x24, 0x54,	//9300
		DT_OCT_STR, 4,	//�����������ַ
			0xC0, 0x00, 0x00, 0x01,
		DT_LONG_U,	//����˿�
			0x88, 0x88,
		DT_UNSIGN,	//��ʱʱ�估�ط�����
			0x7B,	//��ʱʱ��30S�����Դ���3�� 	
		DT_LONG_U,	//��������
			0x01, 0x2c,	//300s
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,

	//0x4510~0x4517 ��̫��ģ������3������վͨ�Ų�����
	DT_ARRAY, 2,
		DT_STRUCT, 0x02,
			DT_OCT_STR, 0x04, 
				0xC0, 0xA8, 0x01, 0x64,	//192.168.1.100
			DT_LONG_U, 
				0x23, 0xF0,	//9200
		DT_STRUCT, 0x02,
			DT_OCT_STR, 0x04, 
				0x00, 0x00, 0x00, 0x00,
			DT_LONG_U, 
				0x00, 0x00,

	//0x4510~0x4517 ��̫��ģ������4������������
	DT_STRUCT, 0x06,
		DT_ENUM, 0x01,	//IP���÷�ʽ DHCP��0��,��̬��1����PPPoE��2��
		DT_OCT_STR, 0x04,	//IP��ַ	192.168.1.200
			0xC0, 0xA8, 0x01, 0xC8, 
		DT_OCT_STR, 0x04,	//��������	255.255.255.0
			0xFF, 0xFF, 0xFF, 0x00,
		DT_OCT_STR, 0x04,	//���ص�ַ	192.168.1.1
			0xC0, 0xA8, 0x01, 0x01, 
		DT_VIS_STR, 0x04,	//PPPoE�û���
			'T',	'E',   'S',	'T',   
		DT_VIS_STR, 0x04,	//PPPoE����
			'T',	'E',   'S',	'T',   
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 

	//0x4510~0x4517 ��̫��ģ������5����MAC��ַ
	DT_MAC, 0x06,
		0x11, 0x22, 0x33, 0x44, 0x55, 0x66,

	//0x4520 ����ʱ�䣬����3��Уʱģʽ
	DT_ENUM, 0x00,	//��վ��ʱ��0�����ն˾�ȷУʱ��1��������/GPS��2����������255��
	//0x4521 ����ʱ�䣬����4����׼Уʱ����
	DT_STRUCT, 0x05,
		DT_UNSIGN, 0x00,	//�������ʱ���ܸ���
		DT_UNSIGN, 0x00,	//���ֵ�޳�����
		DT_UNSIGN, 0x00,	//��Сֵ�޳�����
		DT_UNSIGN, 0x00,	//ͨѶ��ʱ��ֵ
		DT_UNSIGN, 0x00,	//������Ч����
};

TItemDesc g_FrzDesc[] =   //����������ʶ����
{
	{0x4FF0, 	BN_VER_LEN,				DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,			FMT_UNK,		1,				},//�汾����
	{0x5000, 	FRZRELA_ID_LEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_FRZPARA_CHG,	FMT_UNK,		FRZ_TYPE_NUM,	},//����������Ա�
};

BYTE g_FrzDescDefault[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //300
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //600
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x5000 834Bytes
};



TItemDesc g_CollecMonitorDesc[] =   //�ɼ����������ʶ����
{
    {0x5FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�汾����
	{0x6000, 	PNPARA_LEN, DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		POINT_NUM,		},//�ɼ��������ñ�����2������127~134
	{0x6001, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ɼ��������õ�Ԫ������2��

	{0x6002, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����ѱ���
	{0x6003, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��̨���ѱ���
	{0x6004, 	3, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����ѱ�����¼��
	{0x6005, 	3, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��̨���ѱ�����¼��
	{0x6006, 	10, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ѱ����
	{0x6007, 	38, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		MAX_TIME_SCH_MTR_NUM,},//ÿ�������ѱ�������ã���ʱ�ѱ������ʱ�ѱ������
	{0x6008, 	2, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ѱ�״̬
	{0x6009, 	3, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_SCH_MTR,FMT_UNK,		1,		},//�ѱ�ʱ��
	{0x600A, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����ѱ���
	{0x600B, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��տ�̨���ѱ���

	{0x6012, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�������ñ�����2~4������127~129
	{0x6013, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�������õ�Ԫ������2
	{0x6014, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ͨ�ɼ�������������2������127~130
	{0x6015, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ͨ�ɼ�����������2
	{0x6016, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼��ɼ�������������2������127~130
	{0x6017, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�¼��ɼ�����������2��
	{0x6018, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//͸��������������2������127~131
	{0x6019, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//͸������������2��
	{0x601A, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//͸�����������������2��
	{0x601B, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//һ��͸���������
	{0x601C, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ϱ�������������2������127~129
	{0x601D, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ϱ�����������2
	
	{0x601E, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ɼ�����⣬����2������127~129

	{0x6032, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ɼ�״̬��������2
	{0x6033, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//һ���ɼ�״̬
	{0x6034, 	34, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	TASK_NUM,	},//�ɼ������ؼ�������2
	{0x6035, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ɼ������ص�Ԫ

	{0x6040, 	7, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ɼ�����ʱ�꣬����2
	{0x6041, 	7, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ɼ��ɹ�ʱ�꣬����2
	{0x6042, 	7, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ɼ��洢ʱ�꣬����2

	{0x6051, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ʵʱ��زɼ�������������2������127~130
	{0x6052, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ʵʱ��زɼ�����������2

	{0x6700, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���һ���ɼ��������õ�Ԫ�����õ�Ԫ����Ϣ�л�����Ϣ������������
	{0x6701, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������Ӳɼ��������õ�Ԫ
	{0x6702, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�������õ�Ԫ�Ļ�����Ϣ����
	{0x6703, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�������õ�Ԫ����չ��Ϣ�Լ�������Ϣ������ΪNULL��ʾ������
	{0x6704, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ɾ�����õ�Ԫ��ͨ���������ɾ��
	{0x6705, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ɾ�����õ�Ԫ��ͨ��������Ϣ����ɾ��
	{0x6706, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ɾ�����õ�Ԫ��ͨ��ͨ�ŵ�ַ���˿�ɾ��
	{0x6707, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ղɼ��������ñ�

};

TItemDesc g_SetDesc[] = //���������
{
    {0x6FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�汾����
	{0x7000, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ļ����ϣ�����127~129
	{0x7001, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ļ�������2,

	{0x7010, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ű����ϣ�����127~130
	{0x7011, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ű�������2
	{0x7012, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ű�ִ�н����������2
	{0x7013, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//һ���ű�ִ�н��
	{0x7100, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��չ�������󼯺ϣ�����2
	{0x7101, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��չ�α������󼯺ϣ�����2
};

TItemDesc g_CtrlDesc[] = //���������
{
    {0x7FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�汾����
	{0x8000, 	10, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ң�أ�����2
	{0x8001, 	2, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//���磬����2
	{0x8002, 	2, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//�߷Ѹ澯������2
	{0x8003, 	216, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXCOMCHNNOTE,			},//һ��������Ϣ������2
	{0x8004, 	216, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXIMPCHNNOTE,			},//��Ҫ������Ϣ������2

	{0x8100, 	9, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˱�����ֵ
	{0x8101, 	26,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ն˹���ʱ�Σ�����2
	{0x8102, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ظ澯ʱ�䣬����2 
	{0x8103, 	242,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//ʱ�ι��أ�����2
	{0x8104, 	28,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//���ݿأ�����2
	{0x8105, 	30,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//Ӫҵ��ͣ�أ�����2
	//{0x8106, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//��ǰ�����¸��أ�����2������127��
	{0x8107, 	43,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//����أ�����2
	{0x8108, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//�µ�أ�����2
	{0x8109, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ʱ�ι������õ�Ԫ������2
	{0x810A, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ݿ����õ�Ԫ������2
	{0x810B, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//Ӫҵ��ͣ�����õ�Ԫ������2
	{0x810C, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��������õ�Ԫ������2
	{0x810D, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�µ�����õ�Ԫ������2
	{0x810E, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���ƶ���
	{0x810F, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��բ�ִ�
	{0x8110, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ض�ֵ

	{0x8200, 	2, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//ң�أ�����3���̵������״̬��ֻ��)
	{0x8201, 	2, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//ң�أ�����4(�澯״̬��ֻ��)
	{0x8202, 	2,	 	DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//ң�أ�����5������״̬��ֻ��)
	{0x8203, 	8,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXCONTROLTURN,			},//ң������

	{0x8210, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���磬����3,���������ͨѶʱ��
	{0x8211, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���磬����4,�ϵ��Զ�����ʱ��
	{0x8212, 	146, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���磬����5,�Զ�����ʱ��(���24��ʱ��)
	{0x8213, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����Ͷ������

	{0x8220, 	207, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�߷Ѹ澯Ͷ�루������

	{0x8230, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//ʱ�ι��أ�����3,Ͷ��״̬
	{0x8231, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//ʱ�ι��أ�����4,���״̬
	{0x8232, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//ʱ�ι��أ�����5,Խ�޸澯״̬
	{0x8233, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//ʱ�ι���Ͷ������

	{0x8240, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//���ݿأ�����3,Ͷ��״̬
	{0x8241, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//���ݿأ�����4,���״̬
	{0x8242, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//���ݿأ�����5,Խ�޸澯״̬
	{0x8243, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//���ݿ�Ͷ������

	{0x8250, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//Ӫҵ��ͣ�أ�����3,Ͷ��״̬
	{0x8251, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//Ӫҵ��ͣ�أ�����4,���״̬
	{0x8252, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//Ӫҵ��ͣ�أ�����5,Խ�޸澯״̬
	{0x8253, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//Ӫҵ��ͣ��Ͷ������

	{0x8260, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//��ǰ�����¸��أ�����3,Ͷ��״̬
	{0x8261, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//��ǰ�����¸��أ�����4,���״̬
	{0x8262, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//��ǰ�����¸��أ�����5,Խ�޸澯״̬
	{0x8263, 	19,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//��ǰ�����¸���Ͷ������

	{0x8270, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//����أ�����3,Ͷ��״̬
	{0x8271, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//����أ�����4,���״̬
	{0x8272, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//����أ�����5,Խ�޸澯״̬
	{0x8273, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//�����Ͷ������

	{0x8280, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//�µ�أ�����3,Ͷ��״̬
	{0x8281, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//�µ�أ�����4,���״̬
	{0x8282, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//�µ�أ�����5,Խ�޸澯״̬
	{0x8283, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//�µ��Ͷ������
};

TItemDesc g_FileTransDesc[] = //�ļ�����������ʶ����
{
	{0xF000, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ļ���֡�����������4������5
	{0xF001, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ļ��ֿ鴫���������4������7~10
	{0xF002, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ļ���չ�����������4������7������8

};

TItemDesc g_ESAMIfDesc[] = //EASM�ӿ������
{
	{0xF102, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	{0xF103, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	{0xF104,	34,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF105,	38,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF106,	5,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF107,	5,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF108,	17,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF109,	34,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF10A,	2052,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF10B,	34,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF10C,	2052,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},

	{0xF112,	2,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF113,	130,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF114, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PN_NUM,	}, //���ESAM���к�
	{0xF115, 	16,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PN_NUM,	}, //��������
};

TItemDesc g_InOutDevDesc[] = //��������豸�����
{
    {0xF1F0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�汾����
	{0xF200, 	28, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_RS232_PARACHG,	FMT_UNK,	MAX_232_PORT_NUM,		},//RS232������2������127
	{0xF201, 	28, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_RS485_PARACHG,	FMT_UNK,	MAX_485_PORT_NUM,		},//RS485-1, RS485-2, RS485-3, ����2������127
	{0xF202, 	26, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_INFRA_PARACHG,	FMT_UNK,	MAX_HW_PORT_NUM,		},//���⣬����2������127
	{0xF203, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,			MAX_SW_PORT_NUM,		},//���������룬����2
//		{0xF204, 	22, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,					MAX_DC_PORT_NUM,		},//ֱ��ģ����������2������4
	{0xF205, 	26, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_RELAY_PARACHG,	FMT_UNK,			MAX_RLY_PORT_NUM,		},//�̵������������2������127
	{0xF206, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,			MAX_ALRM_PORT_NUM,		},//�澯���������2������4
	{0xF207, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,			MAX_MUL_PORT_NUM,		},//�๦�ܶ��ӣ�����2������127
	{0xF208,	20,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_MULPORT_PARACHG,	FMT_UNK,			1,						},//���ɽӿڣ�����2������127
	{0xF209, 	45, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_PLC_PARACHG,	FMT_UNK,			MAX_PLC_PORT_NUM,		},//�ز�/΢�������߽ӿڣ�����2������127
	{0xF20A, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,			MAX_PLUS_PORT_NUM,		},//���������豸������2
//		{0xF20B, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,					1,		},//����������2
	{0xF800, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_YX_PARA,	FMT_UNK,		MAX_SW_PORT_NUM,		},//���������룬����4
	{0xF801,	10, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,			MAX_SW_PORT_NUM,		},//�澯���������4

};

BYTE g_InOutDevDefault[] = {	 //��������豸�����
	//F1F0
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)
	
	//F200����2--RS232�˿�
	DT_STRUCT, 0x03,
		DT_VIS_STR, 0x10,	//�˿�������
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_COMDCB,	//�˿ڲ���
			0x06,	//9600bps
			0x00,	//��У��
			0x08,	//8λ����λ
			0x01,	//ֹͣλ
			0x00,	//����
		DT_ENUM,	//�˿ڹ���
			0x00,	//����ͨ�ţ�0��������1����������2����ͣ�ã�3��
	//F201����2--RS485�˿�1\2\3
	DT_STRUCT, 0x03,
		DT_VIS_STR, 0x10,	//�˿�������
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_COMDCB,	//�˿ڲ���
			0x03,	//2400bps
			0x02,	//��У��
			0x08,	//8λ����λ
			0x01,	//ֹͣλ
			0x00,	//����
		DT_ENUM,	//�˿ڹ���
			0x01,	//����ͨ�ţ�0��������1����������2����ͣ�ã�3��
	//F202����2--����
	DT_STRUCT, 0x02,
		DT_VIS_STR, 0x10,	//�˿�������
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_COMDCB,	//�˿ڲ���
			0x03,	//2400bps
			0x02,	//��У��
			0x08,	//8λ����λ
			0x01,	//ֹͣλ
			0x00,	//����
	//F203����2--������
	DT_STRUCT, 0x02,
		DT_UNSIGN, 0x00,	//״̬ST
		DT_UNSIGN, 0x00,	//��λCD 

	//F205����2--�̵���
	DT_STRUCT, 0x04,
		DT_VIS_STR, 0x10,	//�˿�������
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_ENUM,	//��ǰ״̬ 
			0x01,	//δ�����0���������1��
		DT_ENUM,	//�������� 
			0x01,	//����ʽ��0��������ʽ��1��
		DT_ENUM,	//����״̬
			0x01,	//���루0����δ���루1)
	//F206����2--�澯���
	DT_ENUM,	//�澯���
		0x00,	//δ�����0���������1��
	//F207����2--�๦�ܶ���
	DT_ENUM,	//����
		0x00,	//�����������0������������  ��1����ʱ��Ͷ��  ��2��
	//F208����2--���ɽӿ�
	DT_STRUCT, 0x01,
		DT_VIS_STR, 0x10,	//����������
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
	//F209����2--�ز�/΢�������߽ӿ�
	DT_STRUCT, 0x03,
		DT_VIS_STR, 0x10,	//�˿�������
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_COMDCB,	//ͨ�Ų���
			0x06,	//9600bps
			0x02,	//��У��
			0x08,	//8λ����λ
			0x01,	//ֹͣλ
			0x00,	//����
		DT_STRUCT, 0x04,//�汾��Ϣ
			DT_VIS_STR, 0x02,	//���̴���
				'0', '0',
			DT_VIS_STR, 0x02,	//оƬ���� 
				'0', '0',
			DT_DATE, 	//�汾����
				0x00,0x00,0x00,0x00,0x00,
			DT_LONG_U,	//����汾
				0x00,0x00,
	//F20a����2--���������豸
	DT_VIS_STR, 0x10,	//��������˿�������
		'0', '0', '0', '0', '0', '0', '0', '0',
		'0', '0', '0', '0', '0', '0', '0', '0',
	
	//F800����4--������
	DT_STRUCT, 0x02,
		DT_BIT_STR, 0x08,	//�����������־
			0x00,
		DT_BIT_STR, 0x08,	//���������Ա�־ 
			0x00,
	
	//F801����4--�澯���
	DT_STRUCT, 0x02,
		DT_TIME, 	//��ʼʱ��
			0x00,0x00,0x00,
		DT_TIME,	//����ʱ��
			0x00,0x00,0x00,
};



TItemDesc g_DisPlayDesc[] = //��������豸�����
{
	{0xF300, 	15, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�Զ����ԣ�����2����ʾ�����б�
	{0xF301, 	15, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�������ԣ�����2����ʾ�����б�
	{0xF900, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�Զ����ԣ�����3����ʾʱ��
	{0xF901, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�������ԣ�����3����ʾʱ��
	{0xF902, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�Զ����ԣ�����4����ʾ����
	{0xF903, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�������ԣ�����4����ʾ����
};


//����ԭ�е�һЩ��������������
TItemDesc g_AcDataDesc[] = 
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����-------��ʽ----------Pn����------
	//����й�����ʾֵ
	{0x9000,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x9001,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x9002,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x9003,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x9004,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x900f,    0,		    DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},

	//�����й�����
    {0x9010,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9011,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9012,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9013,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9014,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x901f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    //�����й�����
    {0x9020,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9021,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9022,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9023,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9024,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x902f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},

	//���������й�����
    {0x9070,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9071,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9072,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x907f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    
	//���෴���й�����
    {0x9080,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9081,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9082,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x908f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},

	//�����޹�����
    {0x9110,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9111,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9112,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9113,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9114,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x911f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//�����޹�����
    {0x9120,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9121,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9122,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9123,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9124,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x912f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
	//һ�����޹�����
    {0x9130,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9131,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9132,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9133,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9134,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x913f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//�������޹�����
    {0x9140,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9141,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9142,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9143,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9144,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x914f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//�������޹�����
    {0x9150,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9151,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9152,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9153,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9154,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x915f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//�������޹�����
    {0x9160,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9161,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9162,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9163,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9164,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x916f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//��������޹�����
    {0x9170,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9171,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9172,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x917f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    
	//���������޹�����
    {0x9180,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9181,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9182,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x918f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},	
};

//------------------------------------------------------------------------------------------------------
//������������������ʶ--����--Ȩ��--��д--ƫ��
TItemDesc  g_PointDataDesc[] = 
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����-------��ʽ----------Pn����------
	//����й����ܣ�����ʽ��02 05 06 ********
	{0xa000, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����й����ܣ�����ʽ��02 05 06 ********
	//�����й����ܣ��ܣ�1~4����
	{0xa010, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����й����ܣ�����ʽ��
	//�����й����ܣ��ܣ�1~4����
	{0xa020, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�����й����ܣ�����ʽ��
	//����޹�1���ܣ��ܣ�1~4����
	{0xa030, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����޹�1���ܣ�����ʽ��
	//����޹�2���ܣ��ܣ�1~4����
	{0xa040, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����޹�2���ܣ�����ʽ��

	{0xa050, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ѹA/B/C������ʽ��02 03 12 ********
	{0xa051, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����A/B/C/���򣬴���ʽ��
	{0xa052, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�й������ܣ�A/B/C������ʽ��
	{0xa053, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�޹������ܣ�A/B/C������ʽ��
	{0xa054, 	14, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//���������ܣ�A/B/C������ʽ��
};



//sect15 Ext-Variable-Para
TItemDesc g_ExtVarParaDesc[] = 
{
// ����ͳ�Ʋ���ID,2+5+(2+5*10)+2+4=65,�����10�����޲���������
	{0x2100, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//��������ͳ��
	{0x2101, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//Сʱ����ͳ��
	{0x2102, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//������ͳ��
	{0x2103, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//������ͳ��
	{0x2104, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//������ͳ��
//�ۼ�ƽ��ͳ�Ʋ���ID, 2+5+2+4=13
	{0x2110, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//����ƽ��
	{0x2111, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//Сʱƽ��
	{0x2112, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//��ƽ��
	{0x2113, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//��ƽ��
	{0x2114, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//��ƽ��
//��ֵͳ�Ʋ���ID,2+5+2+4=13
	{0x2120, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//���Ӽ�ֵ
	{0x2121, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//Сʱ��ֵ
	{0x2122, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//�ռ�ֵ
	{0x2123, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//�¼�ֵ
	{0x2124, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//�꼫ֵ
	//����ӿ������
	{0x2401, 	(TSA_LEN+1), 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//ͨ�ŵ�ַ
	{0x2402, 	8,	 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//����������
	{0x2403, 	PULSE_CFG_ID_LEN, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_PULSE,	FMT_UNK,		PULSE_PN_NUM,		},//��������	

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Bank1 ���ݿⶨ��

//�ն˲�����������ʶ--����--Ȩ��--��д--ƫ��
TItemDesc  g_Bank1Desc[] =   //��׼��
{ 
	{0x1001, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //��������Զ��ض�ʱ��,��λ����,0���Զ��ض�
	{0x1002, 16,DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�����������
	{0x100f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	//0
	{0x2000, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},  //�ط��汾,01�㶫��,02����,03�е�
	{0x200f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	//1
    {0x2010, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //��չЭ�����õ��ı���˾�ĳ��̱��
    {0x2011, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_COMM_RLD},    //�������,��λ����HEX
    {0x2012, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},  //GPRSģ���ͺ�,0��ʾGR47 1.3��,1��ʾSIM,2��ʾWAVECOM,3��ʾ��Ϊ,4��ʾGR47 1.5��
	{0x2013, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_WK_PARA},    //GPRS���ӵȴ�ʱ�� HEX ��
	{0x201f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    

	//2
    {0x2020, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //������Դ
	{0x2021, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�����Դ
	{0x2022, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},  //�̵��������ʽ,0��ƽ,1����
	{0x2023, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�Աȶ� 
	{0x2024, 32, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //����ʹ������
	{0x202f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    

	{0x2030, 16,DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	  //У׼����
	{0x2031, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	  //�������ߵĵ�ѹ����������,0��2:2��, 3:3��
	{0x2032, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},  //TCP/IPЭ��⣺0ģ���Դ�,1�ն�
	{0x2033, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CTһ�β��·��ֵ��HEX
	{0x2034, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT���β��·��ֵ��HEX
	{0x2035, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT���β࿪·��ֵ��HEX
	{0x2036, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CTһ�β��·��ƽ��ֵ��HEX
	{0x2037, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT���β��·��ֵ��HEX
	{0x2038, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT���Ͷ��,0�˳�,1Ͷ��
	{0x2039, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //��ѹ�ٷֱ�
	{0x203a, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�����ٷֱ�
	{0x203b, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //���ɹ�����Ա�ѹ�������ı���
	{0x203c, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //���ɹ��ػָ���Ա�ѹ�������ı���
	{0x203d, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //����쳣���жϼ��,BCD,��λ����
	{0x203e, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_COMM_RLD},    //���������ȼ���8a60,RRTT BCD RR��������,ȱʡ3��,TT�����ȴ����,ȱʡ30����	
	{0x203f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    
	
	{0x2040, 16, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�澯��������,������8033����,0�޶�ÿ����һ��,1���޶�
	{0x2041, 48, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //PPP�û���
	{0x2042, 48, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //PPP����
	{0x2043, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},     //ͨ�Ŵ���λ�ն�ʱ��,BCD,HHMM,0��ʾ����λ
	{0x204f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    

	{0x2050, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�0��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	{0x2051, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�1��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	{0x2052, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�2��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	{0x2053, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�3��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	{0x2054, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�4��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	{0x2055, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�5��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	{0x2056, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�6��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	{0x2057, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�7��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	{0x2058, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //���->645���ʶ��ձ�8��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
    {0x205f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //
	
	{0x2060, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	    //����ʱ����,BCD,1-15,
	{0x2061, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������0�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x2062, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������1�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x2063, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������2�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x2064, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������3�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x2065, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������4�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x2066, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������5�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x2067, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������6�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x2068, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������7�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x2069, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //������8�ȼ���892a�жϸ��ɹ�����Զֵ�ı�����ȱʡΪ1�� 
	{0x206f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //
	
	{0x2070, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //�ȼ���8038������ѹ(NN)ȱʡΪ���70%���ָ���ѹ(MM)ȱʡΪ���85%����������(LL)ȱʡΪ���10% 
	{0x2071, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8065 �ж϶��ࡢȱ��ʱ�䣬ȱʡ5����
	{0x2072, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8066�ж϶��ࡢȱ��ָ�ʱ�䣬ȱʡ5����
	{0x2073, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8067�жϵ�ѹ������ʱ�䣬ȱʡ1����
	{0x2074, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8068�жϵ�ѹ������ָ�ʱ�䣬ȱʡ1����
	{0x2075, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8069�жϵ���������ʱ�䣬ȱʡֵ1����
	{0x2076, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //806a�жϵ��������Իָ�ʱ�䣬ȱʡֵ1����
	{0x2077, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //806bʱ�������ֵ��ȱʡ10����
	{0x2078, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //806c���ܱ������ֵ��ȱʡΪ10��
	{0x2079, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //806d���ܱ�ͣ����ֵ������ʾ��ֹͣ����ʱ�����й����ʼ���Ӧ�ߵ���ֵ��Խ��ֵ����ͣ�ߣ�ȱʡֵΪ0.1kWh
	{0x207f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //	
	
	{0x2080, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT����ʱ����ֵ
	{0x2081, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CTһ�β��·������ֵ
	{0x2082, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CTһ�β��·������ֵ
	{0x2083, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT���β��·������ֵ
	{0x2084, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT���β��·������ֵ
	{0x2085, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT���β࿪·������ֵ
	{0x2086, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},		//��Ӧλ��1�������1��˲ʱ����ֵȡ�Բ�����1��15���ӵ�ƽ������ֵ
	{0x208f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //	
	
	{0x2090, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},		//�ն˵����������ͺ�,0--5A,1--10A
	{0x2091, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_WK_PARA},   //���ߵ�ģ��������,0��ʾ�������ģ�����,HEX
	{0x209f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //
	
	{0x2100, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},  //863�ն˹��ܣ�1:��������2:ר���ն�
	{0x210f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	
	{0x2110, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},  //����ģ���ƽ���Ʒ�ʽ
	{0x211f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},		
	//���ÿռ�
	{0x2f01, 15, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //���ÿռ�
	{0x2f0f,  0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	{0x3000,  2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//�������--������ģʽ�µ��������ڴ���
	
};

BYTE g_bBank1Default[] =   
{
	0x1e, 0x00,     //0x1001 2 ��������Զ��ض�ʱ��,��λ����,0���Զ��ض�
	0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                //0x1002 16 ����ĵ����������
	0x01,           //0x2000 1 �ط��汾,01�㶫��,02����,03�е�

	//1
	0x00,   //0x2010 1 ���̱��
	0x0f,   //0x2011 1 �������,��λ����HEX
	0x16,	//0x2012 1 GPRSģ���ͺ�,ff��ʾ�Զ�ʶ��,0��ʾGR47 1.3��,1��ʾSIM,2��ʾWAVECOM,3��ʾ��Ϊ,4��ʾGR47 1.5��
	0x1e,   //0x2013 1 GPRS���ӵȴ�ʱ�� HEX ��
	        
	//2
	0x00,   //0x2020 1 ������Դ
	0x00,   //0x2021 1 �����Դ
	0x01,	//0x2022 1 �̵��������ʽ,0��ƽ,1����
	0x1a,   //0x2023 1 �Աȶ� 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x00~0x3f  //0x2024 32 ����ʹ������
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,  //0x40~0x7f	
  	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //0x80~0xbf
  	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //0xc0~0xff
  	
  	0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10,  //0x2030  У׼������Ĭ��4096
	0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10,  //0x2030  У׼����
	
	//��Ӳ���
	0x02, //0x2031 1 �������ߵĵ�ѹ����������,0��2:2��, 3:3��
	0x01, //0x2032 1 TCP/IPЭ��⣺0ģ���Դ�,1�ն�
	0x28, 0x00,	  //0x2033 2 CTһ�β��·��ֵ��HEX
	0x80, 0x00,   //0x2034 2 CT���β��·��ֵ��HEX
	0x08, 0x00,	  //0x2035 2 CT���β࿪·��ֵ��HEX
	0x07, 0x00,	  //0x2036 2 CTһ�β��·��ƽ��ֵ��HEX
	0x0d, 0x00,   //0x2037 2 CT���β��·��ֵ��HEX
	0x00, //0x2038 1 CT���Ͷ��,0�˳�,1Ͷ��

	0x10,   //0x2039 1 ��ѹ�ٷֱ�
	0x10,   //0x203a 1 �����ٷֱ�
	0x10,	//0x203b 1 ���ɹ�����Ա�ѹ�������ı���
	0x10,	//0x203c 1 ���ɹ��ػָ���Ա�ѹ�������ı���
	0x01,	//0x203d 1 ����쳣���жϼ��,BCD,��λ����
	0x00, 0x05,  //0x203e 2 ��������,RRTT BCD RR��������,ȱʡ3��,TT�����ȴ����,ȱʡ30����	
	
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, //0x2040 16 �澯��������,������8033����,0�޶�ÿ����һ��,1���޶�
	'C',  'A',  'R',  'D',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x2041 48 PPP�û���	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	'C',  'A',  'R',  'D',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x2042 48 PPP����
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00,	 //0x2043 2 ͨ�Ŵ���λ�ն�ʱ��,BCD,HHMM
	
	
	0x00, 0x00, 0x00, 0x00, //0x2050 4 ���->645���ʶ��ձ�0��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	0x00, 0x00, 0x00, 0x00, //0x2051 4 ���->645���ʶ��ձ�1��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	0x00, 0x00, 0x00, 0x00, //0x2052 4 ���->645���ʶ��ձ�2��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	0x00, 0x00, 0x00, 0x00, //0x2053 4 ���->645���ʶ��ձ�3��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	0x00, 0x00, 0x00, 0x00, //0x2054 4 ���->645���ʶ��ձ�4��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	0x00, 0x00, 0x00, 0x00, //0x2055 4 ���->645���ʶ��ձ�5��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	0x00, 0x00, 0x00, 0x00, //0x2056 4 ���->645���ʶ��ձ�6��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	0x00, 0x00, 0x00, 0x00, //0x2057 4 ���->645���ʶ��ձ�7��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	0x00, 0x00, 0x00, 0x00, //0x2058 4 ���->645���ʶ��ձ�8��N4N3N2N1,N1��ʾ���ĵ�һ�����ʶ�Ӧ645�ķ���
	
	0x01,//0x2060 ����ʱ����
	
	0x20, 0x01, //0x2061 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	0x20, 0x01, //0x2062 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	0x20, 0x01, //0x2063 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	0x20, 0x01, //0x2064 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	0x20, 0x01, //0x2065 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	0x20, 0x01, //0x2066 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	0x20, 0x01, //0x2067 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	0x20, 0x01, //0x2068 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	0x20, 0x01, //0x2069 �жϸ��ɹ�����Զֵ�ı��� (ȱʡΪ1)
	
	0x85, 0x10, 0x70, //0x2070 3 ������ѹ �ָ���ѹ �������� �ȼ���8038
	
	0x05,//2071
	0x05,//2072
	
	0x01,//2073
	0x01,//2074
	
	0x01,//2075
	0x01,//2076
	
	0x10,//2077
	
	0x10,//2078
	
	0x10, 0x00,//2079
	
	//��8
	0x00, 0x01,//2080
	0x05, 0x00,//2081
	0x20, 0x00,//2082
	0x05, 0x00,//2083
	0x20, 0x00,//2084
	0x40, 0x00,//2085
	
	0x00,//2086
	0x01, //0x2090 1 �ն˵����������ͺ�,0--5A,1--10A
	0x02, //0x2091 1 ���ߵ�ģ��������,HEX
	0x01, //0x2100�� 863�ն˹��� 1���������� 2��ר���նˣ�������������
	0x00, // 0x2110����ģ���ƽ���Ʒ�ʽ1-���巽ʽ,0-��ƽ��ʽ
	0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00,  //0x2fX1 16 ����

	//0x3000
	0x00, 0x10,	//1000��
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Bank2 ���ݿⶨ��

//�ն˲�����������ʶ--����--Ȩ��--��д--ƫ��
TItemDesc  g_Bank2Desc[] =   //��׼��
{ 
	//----------���汾���õķǱ������ݷŵ�0x2000ǰ(����,���ɵ�)-----------
	{0x1001, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A��CTֵ,HEX
	{0x1002, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A��CTֵ,HEX
	{0x1003, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A��CTֵ,HEX
    {0x1004, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	 //A��CT��� 0:NORMAL 1:SHORT1 2 :SHORT2 3:OPEN2
    {0x1005, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	 //B��CT���
    {0x1006, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	 //C��CT���
	{0x100f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	
	{0x1011, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A���ѹ���,BCD
	{0x1012, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //B���ѹ���,BCD
	{0x1013, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //C���ѹ���,BCD
	{0x1014, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A��������,BCD
	{0x1015, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //B��������,BCD
	{0x1016, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //C��������,BCD
	{0x101f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	
	{0x1021, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�ն˸�λ����,HEX
	{0x1022, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //���AD���,HEX
	{0x1023, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //��СAD���,HEX
	{0x1024, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //��ǰAD���,HEX
	{0x1025, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //AD��λ����,HEX
	{0x1026, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ƶ�ʸ��ٸ�λ����,HEX
	{0x1027, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ƶ��,HEX,ʵ��Ƶ��*256
	{0x1028, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //��ص�ѹ NN.NN V
	{0x1029, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�̼߳�ظ�λ����,HEX
	{0x102a, 32,DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�̼߳�ظ�λ���һ�θ�λ���߳�����
	{0x102f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	
	{0x1031, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ua NNNNN.N
	{0x1032, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ub
	{0x1033, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Uc
	{0x1034, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ia NNN.NNN
	{0x1035, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ib
	{0x1036, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ic
	{0x103f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ic

	{0x1041, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ʵ����״̬,1����ʵ����״̬,0������
	{0x1042, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�ն˵�ǰ���ʺ�
	{0x1043, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�ն˵�ǰʱ�α��
	{0x1044, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�ն�ʱ�ӵ�ص�ѹ NN.NN V
	{0x1045, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�ն��ڲ��¶�
	{0x1046, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //��һ·ģ����
	{0x1047, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�ڶ�·ģ����
	
	{0x1051, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A��CTֵ,HEX
	{0x1052, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A��CTֵ,HEX
	{0x1053, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A��CTֵ,HEX
	{0x1054, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //����Ƶ��,HEX
	{0x1055, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //���ڹ���,NNNN.NNNN
	{0x1056, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //����IP
	{0x1057, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ӳ���汾
	{0x1058, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //GPRS�ź�ǿ��

	{0x1060, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    //�����й�����
	{0x1061, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // A���й�����
	{0x1062, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // B���й�����
	{0x1063, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // C���й�����
	{0x1064, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    //�����޹�����
	{0x1065, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // A���޹�����
	{0x1066, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // B���޹�����
	{0x1067, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // C���޹�����
	{0x1068, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    //���๦������
	{0x1069, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // A�๦������
	{0x106a, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // B�๦������
	{0x106b, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // C�๦������
	{0x106f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    

	{0x1070, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//��¼��ǰ�Ƿ�����ͨ�Ź���
    
    {0x10d0, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 	//��ǰ״̬
	{0x10d1, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 	//��һ�δ�������
	{0x10d2, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 	//����ʣ��ʱ��
	{0x10d3, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 	//ģ������

	{0x1100, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //8·ң�ţ�ÿλ��Ӧһ·ң��(Hex)
	{0x1101, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��1(Hex)
	{0x1102, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��2(Hex)
	{0x1103, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��3(Hex)
	{0x1104, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��4(Hex)
	{0x1105, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��5(Hex)
	{0x1106, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��6(Hex)
	{0x1107, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��7(Hex)
	{0x1108, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��8(Hex)
	{0x110f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�����������	

	{0x1110, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //8·ң�ţ�ÿλ��Ӧһ·ң��(Hex)
	{0x1111, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��9(Hex)
	{0x1112, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��10(Hex)
	{0x1113, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��11(Hex)
	{0x1114, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��12(Hex)
	{0x1115, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��13(Hex)
	{0x1116, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��14(Hex)
	{0x1117, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��15(Hex)
	{0x1118, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ң��16(Hex)
	{0x111f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�����������	
	
	{0x1120, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //����״̬
	{0x1121, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�������״̬��
	{0x1122, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //ʱ�ӹ���
	{0x1123, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //����оƬ����
	
	//---------Ӧ����صķǱ������ݷŵ�0x2000��----------------------
	{0x2001, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ua��г��������,���ݸ�ʽ5
	{0x2002, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ub��г��������,���ݸ�ʽ5
	{0x2003, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Uc��г��������,���ݸ�ʽ5
	{0x2004, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ia��г��������,���ݸ�ʽ5
	{0x2005, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ib��г��������,���ݸ�ʽ5
	{0x2006, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ic��г��������,���ݸ�ʽ5
	{0x200f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�����������

	{0x2011, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ua��г����Чֵ,���ݸ�ʽ7
	{0x2012, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ub��г����Чֵ,���ݸ�ʽ7
	{0x2013, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Uc��г����Чֵ,���ݸ�ʽ7
	{0x2014, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ia��г����Чֵ,���ݸ�ʽ6
	{0x2015, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ib��г����Чֵ,���ݸ�ʽ6
	{0x2016, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ic��г����Чֵ,���ݸ�ʽ6
	{0x201f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //�����������
	
	{0x2020, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //״̬����λ��־������ʾ�ã�
	{0x2021, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //������ͨѶ��ʱ����뱣��״̬

	{0x2030, 21, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //����������״̬(��ʾ��:��������/�Լ����)
	{0x2031, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //��ǰ��ʾ�¼����(��ʾ��)
	
	{0x2032, 21, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //����������״̬(��ʾ�ã����ڳ�������������š�����ִ�����)
	{0x2033, 21, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //����������״̬(��ʾ�ã����С�ͨѶ�С�����)
	{0x2034, 21, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //����������״̬(��ʾ��)
	
	{0x2040, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //CT���
	{0x2050, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //ģ���������ͣ�������ʾר�ã�
	{0x2051, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //pIf->Connect()״̬��1��ʾconnectok������ʾר�ã�
	{0x2052, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //ping���

	{0x2101, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //A���ѹг�����������ݿ�
	{0x2102, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //B���ѹг�����������ݿ�
	{0x2103, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //C���ѹг�����������ݿ�
	{0x2104, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //A�����г�����������ݿ�
	{0x2105, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //B�����г�����������ݿ�
	{0x2106, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //C�����г�����������ݿ�
	{0x2107, 23,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //�ڲ�����汾��
	{0x2108,  2,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE}, 	 //������ʽ2: 2G 3: 3G  4: 4G����Ӫ��:1--�ƶ���2--��ͨ��3--����
	{0x210e, 1, DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //�ն�ͣ�ϵ�״̬
	{0x2201, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //A���ѹг����Чֵ���ݿ�, 2~19��г����Чֵ
	{0x2202, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //B���ѹг����Чֵ���ݿ飬2~19��г����Чֵ
	{0x2203, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //C���ѹг����Чֵ���ݿ飬2~19��г����Чֵ
	{0x2204, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //A�����г����Чֵ���ݿ飬2~19��г����Чֵ
	{0x2205, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //B�����г����Чֵ���ݿ飬2~19��г����Чֵ
	{0x2206, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //C�����г����Чֵ���ݿ飬2~19��г����Чֵ

	{0x2300, 1,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE, FMT_BIN,  	 MTR_EXC_NUM},	 //�����¼��Ƿ��ʼ��

	//����Ϊ��ǰʵʱ��������������ID�������������
    {0x3010, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�����й�����
	{0x3011,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰA�������й�����
	{0x3012,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰB�������й�����
	{0x3013,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰC�������й�����
	{0x3020,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�����й�����
	{0x3021,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰA�෴���й�����
	{0x3022,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰB�෴���й�����
	{0x3023,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰC�෴���й�����
	{0x3030,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ����޹�1����
	{0x3031,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰA������޹�1����
	{0x3032,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰB������޹�1����
	{0x3033,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰC������޹�1����
	{0x3040,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ����޹�2����
	{0x3041,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰA������޹�2����
	{0x3042,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰB������޹�2����
	{0x3043,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰC������޹�2����
	{0x3050,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰһ�����޹�����
	{0x3051,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰA��һ�����޹�����
	{0x3052,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰB��һ�����޹�����
	{0x3053,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰC��һ�����޹�����
	{0x3060,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�������޹�����
	{0x3061,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰA��������޹�����
	{0x3062,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰB��������޹�����
	{0x3063,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰC��������޹�����
	{0x3070,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�������޹�����
	{0x3071,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰA���������޹�����
	{0x3072,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰB���������޹�����
	{0x3073,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰC���������޹�����
	{0x3080,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�������޹�����
	{0x3081,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰA���������޹�����
	{0x3082,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰB���������޹�����
	{0x3083,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰC���������޹�����

	{0x3117,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�й�����
	{0x3118,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ�޹�����
	{0x3119,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//��ǰ��������

	//{0x5001, 1, DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //Э���л�
	{0x5039, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //BYTE	m_PulseRatio;	// ����Ŵ��� liuzhixing 20170225

};

//�ն˲�����������ʶ--����--Ȩ��--��д--ƫ��
TItemDesc  g_Bank3Desc[] =   //��׼��
{ 
	{0x3000, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //���ѿ��� HEX 0 ��Ҫ���� ����0��ʾ�����ַ���
	{0x3001, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3002, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3003, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3004, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3005, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3006, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3007, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3008, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x300f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	
	{0x3010, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //����С��λ�� Hex Ĭ�� 2
	{0x3011, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3012, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3013, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3014, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3015, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3016, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3017, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3018, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x301f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	
	{0x3020, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //����С��λ�� Hex Ĭ�� 4
	{0x3021, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3022, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3023, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3024, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3025, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3026, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3027, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3028, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x302f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3030, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //��ѹС��λ�� Hex Ĭ�� 0
	{0x3031, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3032, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3033, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3034, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3035, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3036, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3037, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3038, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x303f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3040, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //����С��λ�� Hex Ĭ�� 2
	{0x3041, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3042, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3043, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3044, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3045, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3046, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3047, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3048, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x304f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3050, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //�й�����С��λ�� Ĭ��4
	{0x3051, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3052, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3053, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3054, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3055, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3056, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3057, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3058, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x305f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	
	{0x3060, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //�޹�����С��λ�� Ĭ��2
	{0x3061, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3062, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3063, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3064, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3065, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3066, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3067, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3068, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x306f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3070, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //��������С��λ�� Ĭ�� 3
	{0x3071, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3072, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3073, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3074, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3075, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3076, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3077, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3078, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x307f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3080, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //���ݿ�˲ʱ�����¼��
	{0x3081, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //���ݿ��˲ʱ�����¼��
	{0x3082, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //�������������Ƿ���Ҫ��Ӧ�� 0 ����Ҫ��1��Ҫ,2 ������Ҫ
	{0x3083, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//�����Э������(Ŀǰֻ��Ժ����) 1����Mk3,��1����Mk6
	{0x3084, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//���Ϲ��ػָ�����
	{0x308f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	
	{0x3090,24, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //�µ�У������
	{0x309f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   
	
	{0x30a0, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //CTһ�ζ�·��ֵ
	{0x30a1, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //CT���ζ�·��ֵ
	{0x30a2, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //CT���ο�·��ֵ
	{0x30af, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   

	{0x30b0, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //�Ӷ��½��ն��ߡ�
	{0x30b1, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //ʾ���½�����ߡ�
	{0x30b2, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //ѹ��Ͷ��,0�˳�,1Ͷ��
	{0x30b3, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //Э������,0 TCP,1 UDP
	{0x30b4, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //�޹�����Ƿ��Ͷ��, 0 ��������������ʱ�޹�����Ƿ�����ж�,1 �ж�
	{0x30b5, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //��عضϵ�ѹֵ
	{0x30bf, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	{0x30c0, 72, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//7022У׼����	
	
	{0x30d0, 1,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//ң�������Ƿ��� 0�������� 1������
	
	{0x30d1, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//��1·�����������
	{0x30d2, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//��2·�����������
	{0x30d3, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//��3·�����������
	{0x30d4, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//��4·�����������
	{0x30d5, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//��5·�����������
	{0x30d6, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//��6·�����������
	{0x30d7, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//��7·�����������
	{0x30d8, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//��8·�����������
	{0x30d9, 10,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//���Ե�ʹ�ܣ�ÿbit��Ӧһ����ʾ
	{0x30e0, 4,   DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//��1·ֱ��ģ����У׼����
	{0x30e1, 4,   DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//��1·ֱ��ģ����У׼����
	{0x30e2, 4,   DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//��2·ֱ��ģ����У׼����
	{0x30e3, 4,   DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//��2·ֱ��ģ����У׼����

	{0x3100, 4,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//���ѹ������������߷�ʽ
	{0x3101, 7,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//���ѹNNNNN.N�������NNN.NNN�����߷�ʽNN	

	
	//ʣ��ռ�
	{0x3f01, 165, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //���ÿռ�
	{0x3f0f,   0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	
	{0x5031, 8, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Iregion[2];	// ��λУ׼�����㣬7022Eֻ��2����λУ׼������
	{0x5032, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Pgain[3];			// ����У׼ֵA,B,C
	{0x5033, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Phsreg[3];			// ����У׼�����λ����
	{0x5034, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Poffset[3]; 		// �����й�����OffsetУ��ֵ
	{0x5035, 6, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //WORD Ioffset[3];		// ������ЧֵOffsetУ��ֵ
	{0x5036, 18, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},//signed short	AngleOffset[3][3];//3������ABC������λ����ֵ����λ��0.01��������
	{0x5037, 6, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //signed short EngErrAdj[3];			// ��Ϊ������ֵ,��λ0.0001%.
	{0x5038, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //BYTE	AdjustFlag; 		// У׼��־��У�����λ
	{0x503f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
  
};

BYTE g_bBank3Default[] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //�����ַ���
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, //����С��λ
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, //����С��λ
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //��ѹС��λ
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, //����С��λ
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, //�й�����С��λ
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, //�޹�����С��λ
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, //��������С��λ
	0x09, 0x00, //3080
	0x3c, 0x00, //3081
	0x01, //3082 
	0x00, //3083
	0x95, 0x00,	//3084
	
	//0x3090 24 �µ�У������
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 
	//0x00, 0x00, 0x00, 0x00, 
	0x8E, 0x32, 0x32, 0x00, 0x7D, 0x32, 0x40, 0x00, 0x7D, 0x32, 
	0x41, 0x00, 0x6B, 0x30, 0x00, 0x00, 0x62, 0x30, 0x00, 0x00,
	0x58, 0x30, 0x00, 0x00,	
	
	0xA0, 0x86, 0x01, 0x00, //30a0 CTһ�ζ�·��ֵ10w
	0xc0, 0x27, 0x09, 0x00,	//30a1 CT���ζ�·��ֵ60w
	0x58, 0x02, 0x00, 0x00, //30a2 CT���ο�·��ֵ600
	
	0x05, 0x00, //30b0
	0x02, 0x00, //30b1
	
	0x00, //0x30b2 1 ѹ��Ͷ��,0�˳�,1Ͷ��
	0x01, //0x30b3 1 Э������,0 TCP,1 UDP
	0x00, //0x30b4 1 �޹�����Ƿ��Ͷ��, 0 ��������������ʱ�޹�����Ƿ�����ж�,1 �ж�
	
	0x40, 0x03, //0x30b5 2 ��عضϵ�ѹֵ

	0xc4, 0x21, 0xac, 0x00, 0xfc, 0xe5, 0xab, 0x00, 0xc8, 0x83, 0xab, 0x00, 0xe3, 0x7f, 0x00, 0x00, 
	0xf4, 0x87, 0x00, 0x00, 0x96, 0xb0, 0x00, 0x00, 0xe1, 0x2d, 0x00, 0x00, 0x6e, 0x42, 0x00, 0x00, 
	0x0b, 0x01, 0x00, 0x00, 0xd0, 0xa2, 0x00, 0x00, 0xa4, 0x34, 0x00, 0x00, 0x7d, 0x73, 0x00, 0x00, 
	0x32, 0x50, 0xea, 0x00, 0xb0, 0xe0, 0xe9, 0x00, 0x52, 0x90, 0xe8, 0x00, 0x86, 0xcd, 0x9c, 0x00, 
	0x64, 0xc4, 0x9c, 0x00, 0x8e, 0xe0, 0x9c, 0x00, //0x30c0 ATT7022��У������
	
	0x00,	//0x30D0
	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 	//��1·�����������0x30d1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, //��2·�����������0x30d2
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//��3·�����������0x30d3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //��4·�����������0x30d4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, //��5·�����������0x30d5
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //��6·�����������0x30d6
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //��7·�����������0x30d7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //��8·�����������0x30d8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//���Ե�ʹ�ܣ�ÿbit��Ӧһ����ʾ0x30d9
	0x00, 0x00, 0x05, 0x00, //��1·ֱ��ģ����У׼����
	0x00, 0x00, 0x15, 0x00, //��1·ֱ��ģ����У׼����
	0x00, 0x00, 0x05, 0x00, //��2·ֱ��ģ����У׼����
	0x00, 0x00, 0x15, 0x00, //��2·ֱ��ģ����У׼����
	0x00,0x00,0x00,0x00, //���ѹ������������߷�ʽ0x3100
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //���ѹ������������߷�ʽ0x3101
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	
		//new 75
	//DWORD	Iregion[2];			// ��λУ׼�����㣬7022Eֻ��2����λУ׼������
	0xC8, 0x00, 0x00, 0x00,
	0x0A, 0x00, 0x00, 0x00,
	//DWORD Pgain[3];			// ����У׼ֵA,B,C
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//DWORD Phsreg[3];			// ����У׼�����λ����
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//DWORD Poffset[3]; 		// �����й�����OffsetУ��ֵ
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//WORD	Ioffset[3];			// ������ЧֵOffsetУ��ֵ
	0x08, 0x00,
	0x08, 0x00,
	0x08, 0x00,
	//signed short	AngleOffset[3][3];	// 3������ABC������λ����ֵ����λ��0.01��������
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//signed short EngErrAdj[3];			// ��Ϊ������ֵ,��λ0.0001%.
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00,
	//BYTE	AdjustFlag; 		// У׼��־��У�����λ
	0x00, 
	
};


//645����
TItemDesc g_Bank4Desc[] =   //��׼��
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����-----------��ʽ----------Pn����------    
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,				},//Ver
	{0x0002, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //645Э���ѡ��
        
    {0x0600, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //485�˿ڵĲ�����//ȱʡֵ4��1200bps,�˿�ͨ�Ų�����/300
	{0x0601, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BCD, 		1}, //485�˿ڵ�У��λ//ȱʡ��У��0, 0-��,1-ż,2-��
	{0x0602, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BCD, 		1}, //485�˿ڵ�����λ//ȱʡ����λ8
	{0x0603, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BCD, 		1}, //485�˿ڵ�ֹͣλ//ȱʡֵ0,0-1λֹͣλ,1-1.5ֹͣλ,2-2λֹͣλ
	{0x060f, 	0,			DI_HIGH_PERM, 	DI_WRITE, 		  0,	INFO_NONE, 		FMT_UNK, 		GBC4_MTRPORTNUM},	
   	
	{0x0610, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BCD, 		1}, //ͨ�ŵ�ַ
	{0x0611, 	32, 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�ʲ��������
	{0x0612, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //���ѹ(ASCII��)
	{0x0613, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�����/��������(ASCII��)
	{0x0614, 	6,	 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //������(ASCII��)
	{0x0615, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�й�׼ȷ�ȵȼ�(ASCII��)
	{0x0616, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�޹�׼ȷ�ȵȼ�(ASCII��)
	{0x0617, 	10,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //����ͺ�(ASCII��)
	{0x0618, 	10,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��������(ASCII��)
	{0x0619, 	16,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Э��汾��(ASCII��)	
	{0x061a, 	30,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //645_97 10������	
	
	{0x0620, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //�й���Ϸ�ʽ������
	{0x0621, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //�޹���Ϸ�ʽ1������
	{0x0622, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //�޹���Ϸ�ʽ2������	

	{0x0623, 	1,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //���ɼ�¼ģʽ��
	{0x0624, 	1,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��������ģʽ��

	{0x0630, 	4,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1},	//���ɼ�¼��ʼʱ��
	{0x0631, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��1�ฺ�ɼ�¼���ʱ��
	{0x0632, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��2�ฺ�ɼ�¼���ʱ��
	{0x0633, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��3�ฺ�ɼ�¼���ʱ��
	{0x0634, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��4�ฺ�ɼ�¼���ʱ��
	{0x0635, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��5�ฺ�ɼ�¼���ʱ��
	{0x0636, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��6�ฺ�ɼ�¼���ʱ��
	{0x063f, 	0,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, 

	{0x0640, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //У��������
	{0x0641, 	5,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //����ʱ�����л�ʱ��
	{0x0642, 	5,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //������ʱ�α��л�ʱ��
	{0x0643, 	32,			DI_LOW_PERM, 	DI_READ,		  0,	INFO_NONE, 		FMT_UNK, 		1}, //��������汾��(ASCII��)
	{0x0644, 	32,			DI_LOW_PERM, 	DI_READ,		  0,	INFO_NONE, 		FMT_UNK, 		1}, //����Ӳ���汾��(ASCII��)
	{0x0645, 	32,			DI_LOW_PERM, 	DI_READ,		  0,	INFO_NONE, 		FMT_UNK, 		1}, //���ұ��(ASCII��)

	{0x0650, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //ÿ�µ�һ������
	{0x0651, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //ÿ�µڶ�������
	{0x0652, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //ÿ�µ���������
	{0xc022,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,	1},	//������״̬��
	
    //������峣��
    {0xc030,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //������峣��(�й�) 
    {0xc031,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //������峣��(�޹�)
    
    //����ַ
	{0xc032,	6,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //���(	645ͨѶ��ַ )
    {0xc033,	6,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //�û���
	{0xc034,	6,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //�豸��
	{0xc03f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, 

    {0xc111,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,	FMT_BIN,	1}, //�����������
    {0xc112,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,	FMT_BIN,	1}, //����ʱ��
    {0xc113, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //ѭ��ʱ��
	{0xc114, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //ͣ��ʱ��
	{0xc115,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //��ʾ����С��λ
    {0xc116,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //��ʾ����(�������)С��λ�� 
    {0xc117,	2,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,	FMT_BCD,	1}, //�Զ�������
    {0xc118, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //���ɴ�����
	{0xc119, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //�й����ܱ���ʼ����
	{0xc11a, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //�޹����ܱ���ʼ����
	{0xc11f, 	0, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1},

#ifdef VER_LIAONING
	//����������չ
    {0xc148,	1, 			DI_LOW_PERM,DI_READ|DI_WRITE,	 0,	INFO_AC_PARA, 		FMT_BCD,	1}, //�й����ܼ�����ʽѡ��
    {0xc149,	1, 			DI_LOW_PERM,DI_READ|DI_WRITE,	 0,	INFO_AC_PARA, 		FMT_BCD,	1}, //�޹����ܼ�����ʽѡ��
#endif    

    {0xc211,	2, 			DI_LOW_PERM,DI_READ|DI_WRITE,	 0,	INFO_NONE, 		FMT_BCD, 	1}, //���������
    {0xc212,	4, 			DI_LOW_PERM,DI_WRITE,	 		 0,	INFO_NONE, 		FMT_BIN, 	1}, //����Ȩ�޼�����
    
    {0xc310,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //��ʱ����P
    {0xc311,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //��ʱ�α���q
    {0xc312,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //��ʱ��(ÿ���л���) mС�ڵ���10
	{0xc313,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //������ kС�ڵ���14
	{0xc314,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //����������n
	{0xc31f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},

	//ȫ�����֧��14��ʱ����
    {0xc321,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//1ʱ����ʼ���ڼ���ʱ�α��
    {0xc322,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//2ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc323,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//3ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc324,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//4ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc325,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//5ʱ����ʼ���ڼ���ʱ�α�� 
    {0xc326,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//6ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc327,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//7ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc328,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//8ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc329,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//9ʱ����ʼ���ڼ���ʱ�α�� 
    {0xc32a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//10ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc32b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//11ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc32c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//12ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc32d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//13ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc32e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//14ʱ����ʼ���ڼ���ʱ�α�� 
	{0xc32f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	
	//��ʱ�α�1	(���8����ʱ�α�)	//������������ͬ������0
	{0xc331,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc332,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc333,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc334,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc335,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc336,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc337,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc338,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc339,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 

	//��ʱ�α�2
	{0xc341,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc342,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc343,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc344,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc345,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc346,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc347,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc348,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc349,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
    
	//��ʱ�α�3
	{0xc351,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc352,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc353,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc354,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc355,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc356,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc357,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc358,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc359,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	
	//��ʱ�α�4
	{0xc361,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc362,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc363,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc364,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc365,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc366,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc367,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc368,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc369,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	
	//��ʱ�α�5
	{0xc371,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc372,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc373,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc374,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc375,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc376,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc377,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc378,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc379,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 

	//��ʱ�α�6
	{0xc381,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc382,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc383,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc384,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc385,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc386,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc387,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc388,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc389,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
    
	//��ʱ�α�7
	{0xc391,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc392,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc393,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc394,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc395,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc396,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc397,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc398,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc399,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	
	//��ʱ�α�8
	{0xc3a1,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a2,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a3,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a4,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a5,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a6,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a7,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a8,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a9,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3aa,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3ab,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3ac,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3ad,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3ae,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3af,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 

	//��������
    {0xc411,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��һ�����������ڼ���ʱ�α��
    {0xc412,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ������������ڼ���ʱ�α��
    {0xc413,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���������������ڼ���ʱ�α��
    {0xc414,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���Ĺ����������ڼ���ʱ�α��
    {0xc415,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���幫���������ڼ���ʱ�α��
    {0xc416,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���������������ڼ���ʱ�α��
    {0xc417,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���߹����������ڼ���ʱ�α��
    {0xc418,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڰ˹����������ڼ���ʱ�α��
	{0xc419,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ھŹ����������ڼ���ʱ�α��
    {0xc41a,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�����������ڼ���ʱ�α��
    {0xc41b,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮһ�����������ڼ���ʱ�α��
    {0xc41c,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�������������ڼ���ʱ�α��
    {0xc41d,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�������������ڼ���ʱ�α��
    {0xc41e,	1, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�����ղ��õ���ʱ�α��
    {0xc41f,	0, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1},

	{0xc510,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE, 		FMT_BCD, 		1}, //���ɼ�¼��ʼʱ��,����ʱ��
    {0xc511,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //��������1�����ݼ�¼���,��
       
    {0xc512,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //��������2�����ݼ�¼���,��
    {0xc513,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //��������3�����ݼ�¼���,��
    {0xc514,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //��������4�����ݼ�¼���,��
    {0xc515,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //��������5�����ݼ�¼���,��
    {0xc516,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //��������6�����ݼ�¼���,��
    {0xc517,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //��������7�����ݼ�¼���,��
    {0xc51f,	0, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //��������7�����ݼ�¼���,��
#ifdef VER_LIAONING     
    {0xc610, 	6,			DI_HIGH_PERM, 	DI_WRITE, 		  0,	INFO_NONE, 		FMT_UNK, 		1}, //�ֶ���λ����
    //{0xc611, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BCD,		1},	//��������ʧ���ж�ֵ
    //{0xc612, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BCD,		1},	//��������ʧ���ж�ֵ

    {0xc720, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧѹ������ѹ��ֵ
    {0xc721, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧѹ�ָ���ѹ��ֵ
    {0xc722, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧѹ����������ֵ
    {0xc723, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧ������ƽ��ֵ��ֵ
    {0xc724, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧ��������ƽ���ʷ�ֵ
    //{0xc725, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//
#endif

	{0xc810, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧѹ�¼���ѹ��������
	{0xc811, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧѹ�¼���ѹ�ָ�����
	{0xc812, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧѹ�¼�������������
	{0xc813, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧѹ�¼��ж���ʱʱ��

	{0xc814, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//Ƿѹ�¼���ѹ��������
	{0xc815, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//Ƿѹ�¼��ж���ʱʱ��

	{0xc816, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��ѹ�¼���ѹ��������
	{0xc817, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��ѹ�¼��ж���ʱʱ��

	{0xc818, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼���ѹ��������
	{0xc819, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼�������������
	{0xc81a, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼��ж���ʱʱ��

	{0xc81b, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��ѹ��ƽ������ֵ
	{0xc81c, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��ѹ��ƽ�����ж���ʱʱ��

	{0xc81d, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//������ƽ������ֵ
	{0xc81e, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//������ƽ�����ж���ʱʱ��

	{0xc820, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧ���¼���ѹ��������
	{0xc821, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧ���¼�������������
	{0xc822, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧ���¼�������������
	{0xc823, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//ʧ���¼��ж���ʱʱ��

	{0xc824, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼�������������
	{0xc825, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼��ж���ʱʱ��

	{0xc826, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼���ѹ��������
	{0xc827, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼�������������
	{0xc828, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼��ж���ʱʱ��

	{0xc829, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//���������¼��й����ʴ�������
	{0xc82a, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//���������¼��ж���ʱʱ��

	{0xc82b, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼��й����ʴ�������
	{0xc82c, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�����¼��ж���ʱʱ��

	{0xc82d, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��ѹ��������
	{0xc82e, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��ѹ��������

	{0xc830, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�й����������¼�������������
	{0xc831, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�޹����������¼�������������
	{0xc832, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//���������¼��ж���ʱʱ��

	{0xc833, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�ܹ������������޷�ֵ
	{0xc834, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�ܹ��������������ж���ʱʱ��

	{0xc835, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�������ز�ƽ����ֵ
	{0xc836, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�������ز�ƽ�ⴥ����ʱʱ��

	{0xc840, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//���ɼ�¼ģʽ��
	{0xc841, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��������ģʽ��
	{0xc842, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//���ɼ�¼��ʼʱ��
	{0xc843, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��1�ฺ�ɼ�¼���ʱ��
	{0xc844, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��2�ฺ�ɼ�¼���ʱ��
	{0xc845, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��3�ฺ�ɼ�¼���ʱ��
	{0xc846, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��4�ฺ�ɼ�¼���ʱ��
	{0xc847, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��5�ฺ�ɼ�¼���ʱ��
	{0xc848, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��6�ฺ�ɼ�¼���ʱ��

	{0xc849, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��ʱ��������ģʽ��
	{0xc84a, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//˲ʱ��������ģʽ��
	{0xc84b, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//Լ����������ģʽ��
	{0xc84c, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//���㶳������ģʽ��
	{0xc84d, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�ն�������ģʽ��

	{0xc850, 	5, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//���㶳����ʼʱ��
	{0xc851, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//���㶳��ʱ����
	{0xc852, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//�ն���ʱ��

	{0xc853, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//��������
	{0xc854, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//˲ʱ��������

	{0xc855,	2,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //����������n	2	//��0xc314ӳ����

	{0xc856,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //��̿�����Чʱ��,��λ����	BCD

	//��������
	{0xc861,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��һ�����������ڼ���ʱ�α��
	{0xc862,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ������������ڼ���ʱ�α��
	{0xc863,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���������������ڼ���ʱ�α��
	{0xc864,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���Ĺ����������ڼ���ʱ�α��
	{0xc865,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���幫���������ڼ���ʱ�α��
	{0xc866,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���������������ڼ���ʱ�α��
	{0xc867,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���߹����������ڼ���ʱ�α��
	{0xc868,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڰ˹����������ڼ���ʱ�α��
	{0xc869,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ھŹ����������ڼ���ʱ�α��
	{0xc86a,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�����������ڼ���ʱ�α��
	{0xc86b,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮһ�����������ڼ���ʱ�α��
	{0xc86c,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�������������ڼ���ʱ�α��
	{0xc86d,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�������������ڼ���ʱ�α��
	{0xc86e,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�Ĺ����������ڼ���ʱ�α��

	{0xc871,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�幫���������ڼ���ʱ�α��
	{0xc872,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�������������ڼ���ʱ�α��
	{0xc873,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�߹����������ڼ���ʱ�α��
	{0xc874,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�˹����������ڼ���ʱ�α��
	{0xc875,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //��ʮ�Ź����������ڼ���ʱ�α��
	{0xc876,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ�ʮ�����������ڼ���ʱ�α��
	{0xc877,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ�һ�����������ڼ���ʱ�α��
	{0xc878,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ��������������ڼ���ʱ�α��
	{0xc879,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ��������������ڼ���ʱ�α��
	{0xc87a,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ��Ĺ����������ڼ���ʱ�α��
	{0xc87b,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ��幫���������ڼ���ʱ�α��
	{0xc87c,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ��������������ڼ���ʱ�α��
	{0xc87d,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ��߹����������ڼ���ʱ�α��
	{0xc87e,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ��˹����������ڼ���ʱ�α��

	{0xc881,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�ڶ��Ź����������ڼ���ʱ�α��
	{0xc882,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //����ʮ�����������ڼ���ʱ�α��
	{0xc883,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //����һ�����������ڼ���ʱ�α��
	{0xc884,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�����������������ڼ���ʱ�α��
	{0xc885,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�����������������ڼ���ʱ�α��
	{0xc886,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�����Ĺ��������ڼ���ʱ�α��
	{0xc887,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�����干�������ڼ���ʱ�α��
	{0xc888,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���������������ڼ���ʱ�α��
	{0xc889,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�����߹��������ڼ���ʱ�α��
	{0xc88a,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�����˹��������ڼ���ʱ�α��
	{0xc88b,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //�����Ź����������ڼ���ʱ�α��
	{0xc88c,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //����ʮ�����������ڼ���ʱ�α��
	{0xc88d,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //����һ�����������ڼ���ʱ�α��
	{0xc88e,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //���Ķ������������ڼ���ʱ�α��

	{0xc890,	1, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE, 			FMT_BCD, 		1}, //�Ƿ񷵻�ʵ�ʸ������ʲ���
	{0xc900,	1, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE, 			FMT_BCD, 		1}, //ʱ��ʱ���л���־
};	//    

BYTE g_bBank4Default[] = {
    //0x05ee	 20
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	0x00,	//0x0002 1 00:645/2007Э�� 01:645/1997Э��

   	0x08,	//0x0600 1
	0x01,	//0x0601 1
	0x08,	//0x0602 1
	0x00,	//0x0603 1
	
	0x01,0x00,0x00,0x00,0x00,0x00,	//0x0610 6

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0611 32

	0x00,0x00,0x00,0x00,0x00,0x00,	//0x0612 6
	0x00,0x00,0x00,0x00,0x00,0x00,	//0x0613 6
	0x00,0x00,0x00,0x00,0x00,0x00,	//0x0614 6
	0x00,0x00,0x00,0x00, //0x0615 4
	0x00,0x00,0x00,0x00, //0x0616 4
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0617 10
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0618 10
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0619 16
	
	0x00,0x00,0x00,	//�߼�����
	0x00,0x00,0x00,	//�������
	0x00,0x00,0x00,	//����Ա����
	0x00,0x00,0x00,
	0x11,0x11,0x11,	//����Ա����
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,	//0x061a 30	
	
	0x05, //0x0620
	0x41, //0x0621	//�����޹� Ĭ��I+IV
	0x14, //0x0622	//�����޹� Ĭ��II+III

	0x00, //0x0623
	0x00, //0x0624

	0x00,0x00,0x00,0x00, //0x0630 4
	0x00,0x00, //0x0631
	0x00,0x00, //0x0632
	0x00,0x00, //0x0633
	0x00,0x00, //0x0634
	0x00,0x00, //0x0635
	0x00,0x00, //0x0636
	
	0x00,0x00, //0x0640
	0x00,0x00,0x00,0x00,0x00, //0x0641 5
	0x00,0x00,0x00,0x00,0x00, //0x0642 5

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0643 32
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0644 32
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0645 32
	
	0x00,0x01,//0x0650
	0xff,0xff,//0x0651
	0xff,0xff,//0x0652

	/*0x00,0x00, //0x0650
	0x00,0x00, //0x0651
	0x00,0x00, //0x0652
	0x00,0x00, //0x0653
	0x00,0x00, //0x0654
	0x00,0x00, //0x0655
	0x00,0x00, //0x0656*/	

	//0x00, //0x0660
	//0x00, //0x0661
	//0x00, //0x0662
	//0x00, //0x0663
	//0x00, //0x0664

	0xff, //0xc022 1
	
	0x00,0x64,0x00,	//0xc030 3
	0x00,0x64,0x00,	//0xc031 3
	
	0x01,0x00,0x00,0x00,0x00,0x00,	//0xc032 6
	0x01,0x00,0x00,0x00,0x00,0x00,	//0xc033 6
	0x01,0x00,0x00,0x00,0x00,0x00,	//0xc034 6
	
	0x15, //0xc111 ��������
	0x01, //0xc112 ����ʱ�� 
	0x08, //0xc113 ѭ��ʱ�� 
	0x60, //0xc114 ͣ��ʱ�� 
	0x02, //0xc115 ��ʾ����С��λ
	0x04, //0xc116
	0x00,0x01, //0xc117 �Զ�������
	0x01, //0xc118 ���ɴ�����
	0x00,0x00,0x00,0x00, //0xc119 �й����ܱ���ʼ����
	0x00,0x00,0x00,0x00, //0xc11a �޹����ܱ���ʼ����

#ifdef VER_LIAONING
	0x00, //0xc148 �й����ܼ�����ʽѡ��
	0xc3, //0xc149 �޹����ܼ�����ʽѡ��
#endif

	0x00,0x00, //0xc211
	0x00,0x00,0x00,0x00, //0xc212
	
	0x01, //0xc310
	0x01, //0xc311
	0x07, //0xc312
	0x04, //0xc313
	0x00, //0xc314

	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc32f
	
	0x04,0x00,0x00,	//00:00~05:00 4 ��
	0x03,0x00,0x05,	//05:00~07:30 3 ƽ
	0x02,0x30,0x07,	//07:30~11:30 2 ��
	0x03,0x30,0x11,	//11:30~17:00 3 ƽ
	0x02,0x00,0x17, //17:00~21:00 2 ��
	0x03,0x00,0x21,	//21:00~22:00 3 ƽ
	0x04,0x00,0x22, //22:00~24:00 4 ��
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc33f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc34f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc35f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc36f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc37f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc38f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc39f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc3af

	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00, //0xc41f
	
	0x00,0x00,0x01,0x01, //0xc510, 4, ���ɼ�¼��ʼʱ��,��ʱ����
    0x15,0x00,		//0xc511, 2, ��������1�����ݼ�¼���   
    0x0,0x00,		//0xc512, 2, ��������2�����ݼ�¼���
    0x0,0x00,		//0xc513, 2, ��������3�����ݼ�¼���
    0x0,0x00,		//0xc514, 2, ��������4�����ݼ�¼���
    0x0,0x00,		//0xc515, 2, ��������5�����ݼ�¼���
    0x0,0x00,		//0xc516, 2, ��������6�����ݼ�¼���
    0x0,0x00,		//0xc517, 2, ��������7�����ݼ�¼���
#ifdef VER_LIAONING	
	0x31,0x31,0x31,0x31,0x00,0x00,	//0xc610, 6�ֽ�ASCII���ַ�, �ֶ���λ����
	//0x30,			//0xc611, 1, ��������ʧ���ж�ֵ
	//0x50,			//0xc612, 1, ��������ʧ���ж�ֵ

    0x40,0x15, 	//0xc720, 2, ʧѹ������ѹ��ֵ %70Un 	154.0
    0x70,0x18,	//0xc721, 2, ʧѹ�ָ���ѹ��ֵ %85Un 	187.0
    0x00,0x01,	//0xc722, 2, ʧѹ����������ֵ %2Ib  	0.1
    0x05,0x00,	//0xc723, 2, ʧ������ƽ��ֵ��ֵ(%Ib)	5
    0x50,0x00,	//0xc724, 2, ʧ��������ƽ���ʷ�ֵ(%)	50
    //{0xc725, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//

#endif	
	0x16, 0x17, //0xc810 2 ʧѹ�¼���ѹ�������� NNN.N
	0x60, 0x17, //0xc811 2 ʧѹ�¼���ѹ�ָ����� NNN.N
	0x00, 0x05, 0x00, //0xc812 3 ʧѹ�¼������������� NN.NNNN
	0x60, //0xc813 1 ʧѹ�¼��ж���ʱʱ�� ��

	0x16, 0x17, //0xc814 2 Ƿѹ�¼���ѹ�������� NNN.N
	0x60, //0xc815 1 Ƿѹ�¼��ж���ʱʱ�� ��

	0x00, 0x00, //0xc816 2 ��ѹ�¼���ѹ��������
	0x00, //0xc817 1 ��ѹ�¼��ж���ʱʱ��

	0x16, 0x17, //0xc818 2 �����¼���ѹ��������
	0x00, 0x05, 0x00, //0xc819 3 �����¼�������������
	0x60, //0xc81a 1 �����¼��ж���ʱʱ��

	0x00, 0x00, //0xc81b 2 ��ѹ��ƽ������ֵ
	0x60, //0xc81c 1 ��ѹ��ƽ�����ж���ʱʱ��

	0x00, 0x00, //0xc81d 2 ������ƽ������ֵ
	0x60, //0xc81e 1 ������ƽ�����ж���ʱʱ��

	0x16, 0x17, //0xc820 2 ʧ���¼���ѹ��������
	0x00, 0x00, 0x00, //0xc821 3 ʧ���¼�������������
	0x00, 0x00, 0x00, //0xc822 3 ʧ���¼�������������
	0x60, //0xc823 1 ʧ���¼��ж���ʱʱ��

	0x00, 0x00, //0xc824 2 �����¼�������������
	0x60, //0xc825 1 �����¼��ж���ʱʱ��

	0x00, 0x00, //0xc826 2 �����¼���ѹ��������
	0x00, 0x00, 0x00, //0xc827 3 �����¼�������������
	0x60, //0xc828 1 �����¼��ж���ʱʱ��

	0x00, 0x00, 0x00, //0xc829 3 ���������¼��й����ʴ�������
	0x60, //0xc82a 1 ���������¼��ж���ʱʱ��

	0x00, 0x32, 0x01, //0xc82b 3 �����¼��й����ʴ������� NN.NNNN
	0x60, //0xc82c 1 �����¼��ж���ʱʱ��

	0x00, 0x00, //0xc82d 2 ��ѹ��������
	0x00, 0x00, //0xc82e 2 ��ѹ��������

	0x00, 0x00, 0x00, //0xc830 3 �й����������¼�������������
	0x00, 0x00, 0x00, //0xc831 3 �޹����������¼�������������
	0x60, //0xc832 1 ���������¼��ж���ʱʱ��

	0x00, 0x00, //0xc833 2 �ܹ������������޷�ֵ
	0x60, //0xc834 1 �ܹ��������������ж���ʱʱ��

	0x00, 0x00, //0xc835 2 �������ز�ƽ����ֵ
	0x60, //0xc836 1 �������ز�ƽ�ⴥ����ʱʱ��

	0xff, //0xc840 1 ���ɼ�¼ģʽ��
	0xff, //0xc841 1 ��������ģʽ��
	0x00, 0x00, 0x01, 0x01, //0xc842 4 ���ɼ�¼��ʼʱ��
	0x15, 0x00, //0xc843 2 ��1�ฺ�ɼ�¼���ʱ��
	0x15, 0x00, //0xc844 2 ��2�ฺ�ɼ�¼���ʱ��
	0x15, 0x00, //0xc845 2 ��3�ฺ�ɼ�¼���ʱ��
	0x15, 0x00, //0xc846 2 ��4�ฺ�ɼ�¼���ʱ��
	0x15, 0x00, //0xc847 2 ��5�ฺ�ɼ�¼���ʱ��
	0x15, 0x00, //0xc848 2 ��6�ฺ�ɼ�¼���ʱ��

	0xff, //0xc849 1 ��ʱ��������ģʽ��
	0xff, //0xc84a 1 ˲ʱ��������ģʽ��
	0xff, //0xc84b 1 Լ����������ģʽ��
	0x03, //0xc84c 1 ���㶳������ģʽ��
	0xff, //0xc84d 1 �ն�������ģʽ��

	0x00, 0x00, 0x01, 0x01, 0x00,//0xc850 5 ���㶳����ʼʱ��
	0x60, //0xc851 1 ���㶳��ʱ����
	0x00, 0x00, //0xc852 2 �ն���ʱ��

	0x00, 0x00, 0x00, 0x00, //0xc853 4 ��������
	0x00, 0x00, 0x00, 0x00, //0xc854 4 ˲ʱ��������

	0x00, 0x00,	//0xc855 2 ����������n

	0x60, //0xc856 1 ��̿�����Чʱ��

	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00, //0xc861-0xc86e  ��1��ʮ�Ĺ����������ڼ���ʱ�α��

	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00, //0xc871-0xc87e  ��15��28�����������ڼ���ʱ�α��

	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00, //0xc881-0xc88e  ��29��42�����������ڼ���ʱ�α��

	0x01,	//0xc890 �Ƿ񷵻�ʵ�ʸ������ʲ���
	0x00,	//0xc900 ʱ��ʱ���л���־
};

//07�ڱ���չ����
TItemDesc g_Bank5Desc[] =   //��׼��
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����-----------��ʽ----------Pn����------    
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1},//Ver
        
    {0x5001, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //A��ʧѹ��ʱ��¼
    {0x5002, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //B��ʧѹ��ʱ��¼
    {0x5003, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //C��ʧѹ��ʱ��¼

	{0x5004, 	15,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //ȫʧѹ��ʱ��¼

	{0x5005, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //A�������ʱ��¼
	{0x5006, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //B�������ʱ��¼
	{0x5007, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //C�������ʱ��¼

	{0x5008, 	179,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //A��ʧ����ʱ��¼
	{0x5009, 	179,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //B��ʧ����ʱ��¼
	{0x500a, 	179,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //C��ʧ����ʱ��¼

	{0x500b, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��ѹ��������ʱ��¼
	{0x500c, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //������������ʱ��¼

	{0x500d, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //A�������ʱ��¼
	{0x500e, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //B�������ʱ��¼
	{0x500f, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //C�������ʱ��¼

	{0x5010, 	8,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //���һ�α��ʱ�� TTime��ʽ	
};

TItemDesc g_Bank10Desc[] =
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����--------------��ʽ----------Pn����------��ʽ������------������ӳ���----
	{0xa000, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver
	
	//���ܼ���
	{0xa010, 	3,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT10,			1,	},//���峣��,BCD��
	{0xa011, 	1,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	},//�й���˫�����ģʽ:0,˫�� 1����
	{0xa012, 	1,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	},//�ն��޹������ۼӱ�־,
																										  //��4λ���������޹�:D0,D1,D2,D3�ֱ��ʾһ,��,��,�������޹�����,Ϊ1:���,Ϊ0:����;
																										  //��4λ���ڷ����޹�:D4,D5,D6,D7�ֱ��ʾһ,��,��,�������޹�����,Ϊ1:���,Ϊ0:����;
	{0xa013, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	}, //��̿�����Ч����ʱ�� NN(1~99��)
	{0xa014, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	}, //��Ǳ�����������ʱ�� NNNN(0~9999��)
	{0xa015, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	}, //�Ƕȷ���,0��ʾ�ǶȰ�����ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,240,120
																										   //	1��ʾ�ǶȰ���˳ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,120,240	
	//����
	{0xa020, 	1,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	},//���¶���ģʽ BIN 0��ʾ0�㳭���ٶ���,1��ʾ��ǰ����0�㶳��
	
	//�澯
	{0xa030, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,  	INFO_NONE, 	    FMT_BIN, 	GB_MAXMETER,	NULL,		MTRPNMAP}, //�����������
	{0xa031, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //����/ѹ��ƽ�ⷢ��ʱ����ֵ 0xa031 HEX
	{0xa032, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //����/ѹ��ƽ��ָ�ʱ����ֵ 0xa032 HEX г��Խ�ޣ���������ѹ������Խ��ͬ����/ѹ��ƽ�ⲻƽ��
	{0xa033, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //��ѹ�쳣����ʱ����ֵ 0xa033 HEX
	{0xa034, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //��ѹ�쳣�ָ�ʱ����ֵ 0xa034 HEX
	{0xa035, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //�����쳣�ӳ٣�����/�ָ���ʱ����ֵ 0xa035 HEX
	{0xa036, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},	  //�жϵ�ѹ��·�쳣ʱ��ѹ����(ȱ�ࣩ���޵�ѹ�ٷֱ� HEX
	{0xa037,    1,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},	  //�ж϶���ȱ��ʱ�Ķ����ѹ����(V),HEX,-----��������ȥ������
	{0xa038,    1,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},	  //��������Ϊ��������ǧ�ֱȣ�
	
	//�ն���չ����
	{0xa040, 	2, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_COMM_RLD, 		FMT_UNK, 		1}, //����������
	{0xa041, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_COMM_RLD, 		FMT_UNK, 		1}, //�ն˵�ַ
	{0xa042, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //���̴���
	{0xa043, 	8, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�豸���
	{0xa044, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //����Ĭ�ϲ������,0��ʾ��Ĭ��
	{0xa045, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //ϵͳ��������
	{0xa046, 	11, 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //SIM������
	{0xa04f, 	0, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�ն���չ����������
	
	//���������ò���
	{0xa05f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //��һ����������ò���
	{0xa06f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //�ڶ�����������ò���
	{0xa07f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //��������������ò���
	{0xa08f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //��������������ò���
	{0xa09f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //��������������ò���
	{0xa0af, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //��������������ò���
	{0xa0bf, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //��������������ò���
	{0xa0cf, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //�ڰ�����������ò���
	{0xa0df, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //�ھ�����������ò���
	
	//����ά���˿����ò���
	{0xa100, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //������
	{0xa101, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //У��λ
	{0xa102, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //����λ
	{0xa103, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //ֹͣλ
	{0xa104, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�˿ڹ���
	{0xa10f, 	0, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, 
	
	//�����˿����ò���
	{0xa110, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //������
	{0xa111, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //У��λ
	{0xa112, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //����λ
	{0xa113, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //ֹͣλ
	{0xa114, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�˿ڹ���
	{0xa11f, 	0, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, 
	
	//���Ʋ���
	{0xa120, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�ϵ籣��ʱ��,HEX,��λ����
	{0xa121, 	3, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�µ�ظ澯ʱ��,D0~D23�ֱ��Ӧ0��~23��

	{0xa122, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_UNK, 		1}, //�й����ܼ�����ʽѡ��
	{0xa123, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_UNK, 		1}, //�����޹������ۼӱ�־
	{0xa124, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_UNK, 		1}, //�����޹������ۼӱ�־
	
	{0xa130, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //ֱ��ģ�����������,HEX, 01��PT100���裬 ����ֵ��ֱ����ѹ��
	
	{0xa131, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 	FMT_UNK, 		1}, //���������1��485�ڹ��ܶ���
	{0xa132, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 	FMT_UNK, 		1}, //���������2��485�ڹ��ܶ���

	{0xa133, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //������У�鷽ʽ 0:��У�飬1����У�飬2��żУ��
	//����汾
	{0xa140, 	1,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	},//����汾ѡ�� BIN 0��ʾ����2005��,1��ʾ����2004��
	//�����ϱ�
	{0xa141, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	GB_MAXCOMMTHREAD,	},//���ඨʱ�ϱ�ģʽ��BIN 1��ʾ����ģʽ��0��ʾ��ͨģʽ    
	//GPRSͨ��
	{0xa142, 	2, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_UNK,		1,	},//GPRS��ͨѶ��λ�ն�ʱ��,��λ����,HEX
	//�ܼ�����ز������
	{0xa143, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//�ܼ�����ز���������Ƿ���ԭ�������ۼƵ����Ļ������ۼƣ�BIN 1��ʾ���ۼƣ�0��ʾ�ۼ�    
	//�ն˵�ַ��ʾ
	{0xa144, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//�ն˵�ַ��ʾ��ʽ	BIN	1	ȱʡֵ0,0��ʾʮ������ʾ��1��ʾʮ��������ʾ
	//�ն�ͨ�ŷ�ʽ
	{0xa145, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_UNK,		1,	},//�ն�����1ͨ�ŷ�ʽBIN	1	ȱʡֵ0,0��ʾTCP��ʽ��1��ʾUDP��ʽ

	{0xa150, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},   //��������1�����ַ
	{0xa151, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},   //��������1IP
	{0xa152, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},   //��������
	{0xa153, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},   //����
	{0xa15f, 	0, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},

	{0xa160, 	4, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //����ڲ�����
	{0xa161, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //�����У��λ	
	{0xa162, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //232�ڲ�����
	{0xa164, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //232��У��λ

	{0xa165, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 			FMT_UNK, 		1}, //��վͨ������У��ĳ���ѡ��0-2�ֽڣ�1-16�ֽ�
	{0xa166, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //485���������˳��,0:��->��ֱ��ǿ�1,��2...; 1:��->�ҷֱ��ǿ�1,��2...
  
  //��ʾר��
	{0xa170, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����1��
	{0xa171, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����2��
	{0xa172, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����3��
	{0xa173, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����4��
	{0xa174, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����5��
	{0xa175, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����6��
	{0xa176, 	21, 		DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����7��
	{0xa177, 	21, 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����8��
	{0xa178, 	21, 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Һ������ӭ�����9��
	{0xa179, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��¼���ԵĲ������

	{0xa180, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 	FMT_UNK, 		1}, //���������3��485�ڹ��ܶ���

	{0xa190, 	1 , 		DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMTASK_PARA,		FMT_BIN,		GB_MAXMETER,},	//������߶���ģʽ��,��97�治������  0:����ȡ�����,�ն����ж��ᡢ1:��ȡ���������
	{0xa191, 	1 , 		DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMTASK_PARA,		FMT_BIN,		GB_MAXMETER,},	//����ն���ģʽ��,�����ն��Ṧ�ܵı������� 0:����ȡ�����,�ն����ж��ᡢ1:��ȡ��������ն���

	{0xa1a0, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //���������ϱ�Ϊ��֡ʱ֡���ÿ֡������֡���䣬���ǰ�����֡������ 0:ÿ֡���ǵ�֡ 1:������֡����
	{0xa1a1, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�����Ƿ���ݵ���ص������ݵ�ʵ�ʳ����޸Ĳ�����ķ�������0:���޸ķ���������F10������ 1:���յ��ʵ�ʷ��ص��ܿ鳤���޸�

	{0xa1a5, 	2, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //��������ģ���Ƿ���Ҫ����Dormant������Ϊ0��ʱ����ʹ��Dormant����0��ʱ��������ͨ�ź�wDormantIntervʱ���ڽ���Dormant��	
	{0xa1a6, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //ң������ȡ����־λ 0-05 1-698
	{0xa1a7, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //Խ�޷�ֵ��������λ
	{0xa1a8, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�ն˹���ģʽ

	{0xa1a9, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //������ʽѡ��0x00��ʾ���ܹ��ʼ���������0x01��ʾ�Է��๦�ʼ�������	

	{0xa1b0,	1,			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK,		GB_MAXMETER,}, //�޹�����ģ��  �޹�����������ģʽ��1Ϊһ�βࡢ2Ϊ���βࡢ����ֵ��Ч,Ĭ��Ϊ���β�

	{0xa1b1, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_COMTASK_PARA, 		FMT_UNK, 		1}, //���¶����ͺ�0�㿪ʼִ�еķ�������ȱʡΪ0��ʾ�����ͺ�ִ��

	{0xa1b2, 	1 , 		DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMTASK_PARA,	FMT_BIN,	GB_MAXMETER,},	//������ն���ģʽ��, 0:��ʵʱ���ݡ�1:��ȡ��������

    {0xa1b3, 	12, 		DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_BIN,		1},	//�豸�� ascii�ַ�  12 BYTE
	{0xa1b4, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1},	//�س�������0~7,0��ʾ�������������е��س�,7��ʾһֱ�����������е��س�,������ʾ����
	{0xa1b5, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1},	//��4��485�ڹ��ܶ��� 0��ά����, 0xFFΪ�������
	{0xa1b6, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 	FMT_UNK, 		1}, //��̫���ڽ��뷽ʽѡ��0���Զ���ȡIP��1��pppoe����
	{0xa1b7, 	2, 			DI_HIGH_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE, 		FMT_BIN, 		1},	//Һ���������ʱ��(��λ����)
	{0xa1b8, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1},	//Ĭ��GPRSģ������
	
    {0xa1b9, 	21, 		DI_HIGH_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //�ϴζ�ȡ�������ļ���
	
	{0xa1ba, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMTASK_PARA,		FMT_BIN,		GB_MAXMETER,},	//����¶���ģʽ��, 0:����ȡ�����,�ն����ж��ᡢ1:��ȡ��������ն���; 2.��ȡ��������
	{0xa1bb, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,},//���ö����༰�����ϱ����������У����û�ж������ʷ���ݣ��Ƿ�����Ч����: 0:������Ч�������ͣ�; 1:����Ч

	{0xa1bc, 	4, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,},//����������ĳ�����ʱms
	{0xa1bd, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,},//��1������˿�����Ӧ�Ŀ�����
	{0xa1be, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,},//����LCD�Աȶ�

	{0xa1c0, 	2, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,}, //TCP̽������BIN,��λ��
	{0xa1c1, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_BIN,		1},   //��������2�����ַ

	//�ն�ͨ�ŷ�ʽ
	{0xa1c2, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_UNK,		1,	},//�ն�����2ͨ�ŷ�ʽBIN	1	ȱʡֵ0,0��ʾTCP��ʽ��1��ʾUDP��ʽ
	{0xa323,	1,	  		DI_LOW_PERM,	DI_READ|DI_WRITE,0, 	INFO_NONE,		FMT_UNK,		1,	}, //�Ƿ񱣴������Ϣ���ļ�

};

BYTE g_bBank10Default[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)	
	
	//���ܼ���
	0x00, 0x64, 0x00, //0xa010 3 ���峣��,BCD��
	0x00, //0xa011 1 �й���˫�����ģʽ:0,˫�� 1����
	0xc3, //0xa012 �ն��޹������ۼӱ�־,
	
	0x30,	//0xa013 1 ��̿�����Ч����ʱ�� NN(1~99��)
	0x60, 0x00,	//0xa014 2 ��Ǳ�����������ʱ�� NNNN(0~9999��)
	0x01,	//0xa015 1	�Ƕȷ���,0��ʾ�ǶȰ�����ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,240,120
			//					 1��ʾ�ǶȰ���˳ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,120,240

	//����
	0x00, //0xa020 1 ���¶���ģʽ BIN 0��ʾ0�㳭���ٶ���,1��ʾ��ǰ����0�㶳��
	
	//�澯
	0x03, //0xa030 1 �����������
	0x01, //0xa031 1 ����/ѹ��ƽ�ⷢ��ʱ����ֵ
	0x01, //0xa032 1 ����/ѹ��ƽ��ָ�ʱ����ֵ
	0x01, //0xa033 1 ��ѹ�쳣����ʱ����ֵ 0xa033��
	0x01, //0xa034 1 ��ѹ�쳣�ָ�ʱ����ֵ 0xa034��
	0x01, //0xa035 1 �����쳣�ӳ٣�����/�ָ���ʱ����ֵ 0xa035;
	0x4e,	//0xa036 1 �жϵ�ѹ��·�쳣ʱ��ѹ����(ȱ�ࣩ���޵�ѹ�ٷֱ� HEX
	0x00,   //0xa037  1 �ж϶���ȱ��ʱ�Ķ����ѹ����(V)�� Ĭ��Ϊ��Чֵ��
	0x50,   //0xa038  1 ��������Ϊ��������ǧ�ֱȣ�Ĭ��Ϊ��Чֵ��

	0x11, 0x22, //����������
	0x33, 0x44, //�ն˵�ַ
	'C', 'L', 'O', 'U',//���̴���	
    #ifdef EN_AC
    'C', 'L', '8', '1', '8', 'C', 'G', 'B', //�豸���	
    0x01, 0x00, //0xa044 2 ����Ĭ�ϲ������,0��ʾ��Ĭ��
    #else
    'C', 'L', '8', '1', '8', 'C', 'J', 'C', //�豸���	
	0x00, 0x00, //0xa044 2 ����Ĭ�ϲ������,0��ʾ��Ĭ��
    #endif
	'0', '0', '0', '0', '0', '0', //ϵͳ��������
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //�ն���չ����������

	//���������ò���
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa05f 7 ��һ��
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa06f 7 �ڶ���
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa07f 7 ������
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa08f 7 ������
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa09f 7 ������
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa0af 7 ������
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa0bf 7 ������
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa0cf 7 �ڰ���
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa0df 7 �ھ���
	                         
	//����ά���˿����ò���
	0x08,//������  ȱʡֵ8��2400bps,�˿�ͨ�Ų�����/300
	0x00,//У��λ  ȱʡ��У��0, 0-��,1-ż,2-�� 
	0x08,//����λ  ȱʡ����λ8
	0x00,//ֹͣλ  ȱʡֵ0,0-1λֹͣλ,1-1.5ֹͣλ,2-2λֹͣλ
	0x00,//�˿ڹ��� ȱʡֵ0
	 
	//����ά���˿����ò���
	0x06,//������  ȱʡֵ6��9600bps,�˿�ͨ�Ų�����/300
	0x00,//У��λ  ȱʡ��У��0, 0-��,1-ż,2-�� 
	0x08,//����λ  ȱʡ����λ8
	0x00,//ֹͣλ  ȱʡֵ0,0-1λֹͣλ,1-1.5ֹͣλ,2-2λֹͣλ
	0x01,//�˿ڹ��� ȱʡֵ0,0-�๦�ܵ��ܱ���,1-485���߼���
	                              

	//���Ʋ���
	0x00, //0xa120 1 �ϵ籣��ʱ��,HEX,��λ����
	0xff, 0xff, 0xff, //0xa121 3 �µ�ظ澯ʱ��,D0~D23�ֱ��Ӧ0��~23��
	
	0x00, //0xa122 1 �й����ܼ�����ʽѡ��
	0x0f, //0xa123 1 �����޹������ۼӱ�־
	0xf0, //0xa124 1 �����޹������ۼӱ�־

	0x00, //0xa130 1 ֱ��ģ��������,HEX, 01��PT100���裬 ����ֵ��ֱ����ѹģ����
#ifdef SYS_WIN
    0x00,//0x04, //0xa131 1 ���������1��485�ڹ��ܶ��� 0������ڣ� 1�������ڣ�2�������ڣ�3�����޹�����װ��
#else
	0x00, //0xa131 1 ���������1��485�ڹ��ܶ��� 0������ڣ� 1�������ڣ�2�������ڣ�3�����޹�����װ��
#endif
	0x00, //0xa132 1 ���������2��485�ڹ��ܶ��� 0������ڣ� 1�������ڣ�2�������ڣ�3�����޹�����װ��
	0xc3, //0xa133 ������ͨѶ����,Ĭ��9600,8λ����λ,��У��
	
	//����汾
	0x00, //����汾ѡ�� BIN 0��ʾ����2005��,1��ʾ����2004��
	//�����ϱ�
	0x01, //���ඨʱ�ϱ�ģʽ��BIN 1��ʾ����ģʽ��0��ʾ��ͨģʽ
	//GPRSͨ��
	0x80, 0x16,	  //0xa142 2 GPRS��ͨѶ��λ�ն�ʱ��,��λ����,HEX,Ĭ��4��
	//�ܼ�����ز������
	0x00, //�ܼ�����ز���������Ƿ���ԭ�������ۼƵ����Ļ������ۼƣ�BIN 1��ʾ���ۼƣ�0��ʾ�ۼ�    
	//�ܼ�����ز������
	0x00, //�ն˵�ַ��ʾ��ʽ ȱʡֵ0, 0��ʾʮ������ʾ��1��ʾʮ��������ʾ   	
#ifdef SYS_WIN
	0x00,//�ն�����1ͨ�ŷ�ʽBIN	1	ȱʡֵ0,0��ʾTCP��ʽ��1��ʾUDP��ʽ
#else
	0x01,//�ն�����1ͨ�ŷ�ʽBIN	1	ȱʡֵ0,0��ʾTCP��ʽ��1��ʾUDP��ʽ
#endif

	//0xa150
	0x00, 0x87, 0xFC, 0x74, 0xA7, 0x00, //���������ַ
	0xC8, 0x01, 0xA8, 0xC0,  //����IP	test:192.168.1.200
	0x00, 0xFF, 0xFF, 0xFF, //��������
	0x00, 0x00, 0x00, 0x00, //����

	0x03, 0x00, 0x00, 0x00, //����ڲ�����
	0x00,//�����У��λ	
	0x03, 0x00, 0x00, 0x00, //232�ڲ�����
	0x00,//232��У��λ

	16,//��վͨ������У��ĳ���ѡ��0-Ĭ�ϣ�2�ֽڣ�������Ϊ������ֽ���
	0x00,//0xa166 1 485���������˳��,0:��->��ֱ��ǿ�1,��2...; 1:��->�ҷֱ��ǿ�1,��2...
	
	//��ʾר�� 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
//	0xc9,0xee,0xdb,0xda,0xbf,0xc6,0xc2,0xbd,0xb5,0xe7,0xd7,0xd3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
#ifdef EN_CONCENTRATOR
	#ifdef ENGLISH_DISP
		'L', 'o' ,'w' ,'-' ,'V' ,'o' ,'t' ,'-','C' ,'o' ,'n' ,'c' ,'e' ,'n' ,'t' ,'o' ,'r' ,0x00,0x00,0x00,0x00,//(21) ��ѹ����������---Ӣ����ʾ
	#else
		0xB5,0xCD,0xD1,0xB9,0xBC,0xAF,0xB3,0xAD,0xBC,0xAF,0xD6,0xD0,0xC6,0xF7,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21) ��ѹ����������
	#endif
#else
	#ifdef ENGLISH_DISP	//EN_SPECIAL_TRANSFORMER
		'S' ,'P' ,'E' ,'C' ,'I','A' ,'L' ,'-' ,'T' ,'R' ,'A' ,'N' ,'S' ,'F' ,'O','R','M','E','R',0x00,0x00,//(21) ��ѹ����������---Ӣ����ʾ
	#else
		0xD6,0xC7,0xC4,0xDC,0xD7,0xA8,0xB1,0xE4,0xD6,0xD5,0xB6,0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21) ����ר���ն�
	#endif
#endif
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,
	//9*21

	0xFF, //0xa180 1 ���������3��485�ڹ��ܶ��� 0������ڣ� 1�������ڣ�2�������ڣ�3�����޹�����װ��, 0xFFΪ�������
	0x00, //0xa190
	0x00, //0xa191
	0x00, //0xa1a0
	0x00, //0xa1a1
	0x10, 0x00,//0xa1a5	��������ģ���Ƿ���Ҫ����Dormant������Ϊ0��ʱ����ʹ��Dormant����0��ʱ��������ͨ�ź�wDormantIntervʱ���ڽ���Dormant��	
	0x00, //0xa1a6 ң������ȡ����־λ0-05,1-697
	0x00,// 0xa1a7Խ�޷�ֵ��������λ0��־1+ϵ����1ֱ��ʹ��ϵ��
	0x00,// 0xa1a8 ����ģʽ���أ�0���ǲ���ģʽ��1.����ģʽ
	0x00, //0xa1a9 0��ʾ���ܹ��ʼ���������1��ʾ�Է��๦�ʼ�������
	0x02, //0xa1b0 1Ϊһ�βࡢ2Ϊ���βࡢ����ֵ��Ч Ĭ��Ϊ���β�
	0x00, //0xa1b1	
    0x00, //0xa1b2

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa1b3 �豸��
	0x02, //0xa1b4 1 �س�������0~7,0��ʾ�������������е��س�,7��ʾһֱ�����������е��س�,������ʾ����
	0x00, //0xa1b5
	0x00, //0xa1b6 ��̫���ڽ��뷽ʽѡ��0���Զ���ȡIP��1��pppoe����
	60, 0, //LCD����ʱ��
	11,  //0xa1b8 Ĭ��GPRSģ������ GC864==11
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,////0xa1b9(21)
	0x00, //0xa1ba	
	0x01, //0xa1bb	

	0xF4, 0x01, 0x00, 0x00, //0xa1bc
	0x03,//0xa1bd
	0xa0,//0xa1be LCDĬ�϶Աȶ�

	0x3c, 0x00, //0xa1c0 2 TCP̽������BIN,��λ��
	0x00, 0x87, 0xFD, 0x75, 0xA6, 0x00, //��������2�����ַ���������������ڱ�׼������
	0x01,//�ն�����2ͨ�ŷ�ʽBIN	1	ȱʡֵ0,0��ʾTCP��ʽ��1��ʾUDP��ʽ
	0x00,  //0xa323    //�Ƿ񱣴������Ϣ���ļ�
};

//------------------------------------------------------------------------------------------------------
//�м�����������
TItemDesc g_Bank11Desc[] =
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����-------��ʽ----------Pn����------------��ʽ��������--------------------------------------
	//���Ӱ汾���ƣ��Ѿ���ȥ���ն˰汾ע�ⲻҪ���׺ϳ�����������������������ݶ�ʧ
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//Ver  	
//ͳ�ƺϸ��ʵĺϸ�ʱ����Խ��ʱ��,���ֽڴ�ǰ,�ϸ���+�ϸ���+Խ����+Խ����,������ʽ�ֽڷ���
	{0x0010, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//�ܵ�ѹ�ϸ�ʱ�䣬
	{0x0011, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//A��ѹ�ϸ�ʱ�䣬���ֽڴ�ǰ
	{0x0012, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//B��ѹ�ϸ�ʱ�䣬���ֽڴ�ǰ
	{0x0013, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//C��ѹ�ϸ�ʱ�䣬���ֽڴ�ǰ
//����ͳ�Ƶ�,�����ڼ��ʱ��ֵ��Ƶ��ִ��ʱ��
	{0x0014, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//��������ͳ�����ڼ��ʱ��㱣��,2��TTime��ʽ
	{0x0015, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//Сʱ����ͳ�����ڼ��ʱ��㱣��
	{0x0016, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//������ͳ�����ڼ��ʱ��㱣��
	{0x0017, 16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//������ͳ�����ڼ��ʱ��㱣��
	{0x0018, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//������ͳ�����ڼ��ʱ��㱣��
//�ۼ�ƽ��ͳ�Ƶ�,�����ڼ��ʱ��ֵ��Ƶ��ִ��ʱ��
	{0x0019, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//�����ۼ�ƽ��ͳ�����ڼ��ʱ��㱣��,2��TTime��ʽ
	{0x001a, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//Сʱ�ۼ�ƽ��ͳ�����ڼ��ʱ��㱣��
	{0x001b, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//���ۼ�ƽ��ͳ�����ڼ��ʱ��㱣��
	{0x001c, 16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//���ۼ�ƽ��ͳ�����ڼ��ʱ��㱣��
	{0x001d, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//���ۼ�ƽ��ͳ�����ڼ��ʱ��㱣��
//��ֵͳ�Ƶ�,�����ڼ��ʱ��ֵ��Ƶ��ִ��ʱ��
	{0x001e, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//���Ӽ�ֵͳ�����ڼ��ʱ��㱣��,2��TTime��ʽ
	{0x001f, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//Сʱ��ֵͳ�����ڼ��ʱ��㱣��
	{0x0020, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//�ռ�ֵͳ�����ڼ��ʱ��㱣��
	{0x0021, 16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//�¼�ֵͳ�����ڼ��ʱ��㱣��
	{0x0022, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//�꼫ֵͳ�����ڼ��ʱ��㱣��
//ͳ�ƵĶ���ID
//������ն���
	{0x0023, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x0024, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x0025, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x0026, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x0027, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
//�ۼ�ƽ�����ն���
	{0x0028, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x0029, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x002a, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x002b, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x002c, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
//��ֵ���ն���
	{0x002d, 	33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x002e, 33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x002f, 	33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x0030, 	33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
	{0x0031, 	33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//��ʱ������
//��ѹ�ϸ�������ID,2+2*(2+5+3+3+5+5)=48
	{0x0032, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//�ܵ�ѹ�ϸ���
	{0x0033, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����A���ѹ�ϸ���
	{0x0034, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����B���ѹ�ϸ���
	{0x0035, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����C���ѹ�ϸ���

//������й����ʼ�����ʱ����ն���
	{0x0036, 	15, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������й����ʼ�����ʱ��
	{0x0037, 	15, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//������й����ʼ�����ʱ��

	{0x0038, 	12, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ͨ������
	{0x0039, 	12, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//����ʱ��
	{0x003a, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//��λ����
//�ۼ�ƽ�����ۼƴ���
	{0x003b, 	2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//��
	{0x003c, 	2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//ʱ
	{0x003d, 2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//��
	{0x003e, 2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//��
	{0x003f, 2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//��






#if 0	//tll �±ߵ���ЩID���ȷ������Ҫ���˿��Կ���ȥ��
    {0x00df, 	36, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F27+
    {0x00ef, 	36, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F35+ 
    {0x010f, 	32, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F25 
    {0x011f, 	24, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F26 
    {0x012f, 	66, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F27 

    {0x013f, 	14 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F28 
    {0x014f, 	38, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F29
	{0x015f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F30 
    {0x017f, 	32, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F33
    {0x018f, 	24, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F34
    {0x019f, 	66, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F35
    {0x01af, 	16 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F36
    {0x01bf, 	38, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F37
    {0x01cf, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F38
    {0x01df, 	72, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F41 
    {0x01ef, 	8 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F42 

    {0x020f, 	6 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F43 
    {0x021f, 	6 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F44 
    {0x022f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C2F49
    {0x023f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C2F50
    {0x024f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C2F51
    {0x025f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C2F52

    {0x035f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F113 
    {0x036f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F114                                                                                                                     
    {0x037f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F115 
    {0x038f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F116 
    {0x039f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F117 
    {0x03af, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F118 
    {0x03bf, 	77, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F121 
    {0x03cf, 	77, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F122 
    {0x03df, 	77, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F123 
    {0x03ef, 	14, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMEASURE,},//C2F129 
    {0x040f, 	14, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMEASURE,},//C2F130  
	
    {0x052f, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCOMMTHREAD,	},//�¼���ָ��  
    {0x055f, 	7, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//ң����բ�м�����
    {0x056f, 	9, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//ʱ�ο���բ�м�����
    {0x057f, 	9, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//���ݿ���բ�м�����
    {0x058f, 	9, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//Ӫҵ��ͣ���м�����
    {0x059f, 	9, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//��ʱ�¸�����բ�м�����
    
    {0x0600, PN_MASK_SIZE,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	}, //����������λ
	{0x0601, PN_MASK_SIZE,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	}, //��������������־λ
	{0x0602, 1,				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //����������Ӧ�Ĳ��������־(bit0~3��Ӧ���С����ޡ����С�����)

    {0x0610, 	4,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //���һ���ն���ʱ��
	{0x0611, 	4,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //���һ���¶���ʱ��

	{0x0b01, 	5,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //�¼�ERC31����ʱ��
	{0x0b02, 	4,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //�¼�ERC31�����й�����ʾֵ
	{0x0b03, 	4,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //�¼�ERC31�����޹�ʾֵ
	{0x0b0f, 	0,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, 

#endif

	{0x0b10, 	6,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		FRZ_TASK_NUM,	}, //�������񴥷���������(�ӳ�ʱ��2 + ��������ʱ��4)
	{0x0b11, 	2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		FRZ_TASK_NUM,	}, //��λ������������(��������1 + ��Ч��־1)
	{0x0b12, 	1,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		EVT_TOTAL_NUM,	}, //�¼����¼��ʶ
	{0x0b13, 	EVT_TRIG_PARA_LEN,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		EVT_TOTAL_NUM,	}, //�¼�������������
	{0x0b14, 	2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //��λ������������(��������1 + ��Ч��־1)	
	{0x0b15, 	1,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		MTR_EXC_NUM,	}, //�����¼����¼��ʶ

	{0x0b16, 	8,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		FRZ_TASK_NUM,	}, //�������񴥷���������(��ʼʱ��4 + ����ʱ��4)

	{0x0b20, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //3105������������(�ӳٷ���ʱ��2 + �ӳٷ���ʱ��2 + ��������ʱ��4 + ״̬��1)
	{0x0b21, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310B������������(�ӳٷ���ʱ��2 + �ӳٷ���ʱ��2 + ��������ʱ��4 + ״̬��1)
	{0x0b22, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310C������������(�ӳٷ���ʱ��2 + �ӳٷ���ʱ��2 + ��������ʱ��4 + ״̬��1)
	{0x0b23, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310D������������(�ӳٷ���ʱ��2 + �ӳٷ���ʱ��2 + ��������ʱ��4 + ״̬��1)
	{0x0b24, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310E������������(�ӳٷ���ʱ��2 + �ӳٷ���ʱ��2 + ��������ʱ��4 + ״̬��1)
	{0x0b25, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310F������������(�ӳٷ���ʱ��2 + �ӳٷ���ʱ��2 + ��������ʱ��4 + ״̬��1)

#ifdef EN_SBJC_V2
	{0x0c00,	1,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	}, //ˮ���ȱ�ʧ�ܼ������ܱ�־
	{0x0c01,	1,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		POINT_NUM,	}, //ˮ���ȱ�ʧ�ܼ�������ʧ�ܴ�������3�κ󣬲���ȥ����
#endif

	{0x0d00,	5,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1������ͳ��ʱ��
	{0x0d01,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1������������ͳ�����ֵ
	{0x0d02,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1������������ͳ�����ֵ
	{0x0d03,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1�������շ���ͳ�����ֵ
	{0x0d04,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1�������·���ͳ�����ֵ
	{0x0d05,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1������������ͳ�����ֵ
	{0x0d06,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1������������ͳ�����ֵ
	{0x0d07,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1�������շ���ͳ�����ֵ
	{0x0d08,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //���1�������·���ͳ�����ֵ

};

//------------------------------------------------------------------------------------------------------
TItemDesc  g_Bank16Desc[] =
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����-------��ʽ----------Pn����------
	//Oob
	{0x6001,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //
	{0x6002,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //��¼�����ϱ���ִ��ʱ��
	{0x6003,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //��¼�����ϱ���ִ��ʱ��IF_GPRSͨ��
	{0x6004,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //��¼�����ϱ���ִ��ʱ��
	{0x6005,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //��¼�����ϱ���ִ��ʱ��IF_SOCKET
	{0x6006,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //��¼�����ϱ���ִ��ʱ��
	{0x6007,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //��¼�����ϱ���ִ��ʱ��
	{0x6008,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //��¼�����ϱ���ִ��ʱ��(����ģʽ)
	{0x6010,	PN_MASK_SIZE,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //�ز�����δ֪���澯�¼�������
};

//------------------------------------------------------------------------------------------------------
TItemDesc  g_Bank17Desc[] =
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����-------��ʽ----------Pn����------
	//Oob
	{0x6001,   PN_MASK_SIZE,DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //����������λ,�����ز���485
	{0x6002,   PN_MASK_SIZE,DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //485����������λ
	{0x6003,   PN_MASK_SIZE,DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //�ز�����������λ
	{0x6004,   PN_MASK_SIZE,DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //�ز�������ɼ�������λ
	
	{0x6010,	1,			DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //����ͨ��ģ�飺0--����ͨ��ģ��1��1--����ͨ��ģ��2
	{0x6011,	1,			DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //ÿ������ͨ��ģ����ڶ����վIP��ַ���������������ֲ�ͬIP��ַ
	{0x6012,	1,			DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //��̫��ͨ��ģ�飺0--��̫��ͨ��ģ��1��1--��̫��ͨ��ģ��2...
	{0x6013,	1,			DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //ÿ����̫����ģ����ڶ����վIP��ַ���������������ֲ�ͬIP��ַ
};

//------------------------------------------------------------------------------------------------------
//�м�����������
TItemDesc g_Bank18Desc[] =
{//----��ʶ-----����------------Ȩ��-----------��д--------ƫ��----д����-------��ʽ----------Pn����------------��ʽ��������--------------------------------------
    {0x0001, 	BN_VER_LEN,			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver
    {0x003f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F41+
    {0x004f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F42+                                                                                                                    
    {0x005f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F43+
    {0x006f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F44+
    //{0x007f, 	41, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		g_bC1F21Fmt},//C1F21+
    //{0x008f, 	41, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		g_bC1F22Fmt},//C1F22+
    {0x009f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F45+
    {0x00af, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F46+
    {0x00bf, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F47+
    {0x00cf, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F48+
	{0x026f, 	12, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,	},//C2F57
	{0x029f, 	12, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,	},//C2F60
	{0x02cf, 	6 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,	},//C2F65 
    {0x02df, 	6 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,	},//C2F66 
	
	{0x031f, 	8 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F97
    {0x032f, 	8 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F98
    {0x033f, 	8 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F99
    {0x034f, 	8 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F100

	//�ܼ���������㼰���ʱ���ۼ�ֵ�����ڴ���������
	{0x035f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F19
    {0x036f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F20
	{0x037f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F21
    {0x038f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F22
	{0x039f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F19+
    {0x03af, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F20+
	{0x03bf, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F21+
    {0x03cf, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F22+

	//�������������ʱ���ۼ�ֵ,���ڴ�����ʾ���½�������,Ŀǰ���� 
	{0x040f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F41
    {0x041f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F42
	{0x042f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F43
    {0x043f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F44
	{0x044f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F45
	{0x045f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F46
    {0x046f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F47
	{0x047f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F48
	
	{0x0610, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //���һ���ն���ʱ��
	{0x0611, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //���һ���¶���ʱ��
	{0x0612, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //���һ���¶���ʱ��
	{0x0613, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //���һ���¶���ʱ��
	{0x0614, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	8,			}, //�������һ���¶���ʱ��,ÿһ·�����Ӧһ��������
	{0x0615, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	8,			}, //�������һ���¶���ʱ��,ÿһ·�����Ӧһ��������
	{0x0616, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	8,			}, //�������һ���¶���ʱ��,ÿһ·�����Ӧһ��������

};

//�ն�������Լ��������ʶ--����--Ȩ��--��д--ƫ��
TItemDesc  g_Bank24Desc[] =   //
{
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver

	{0x001f,	5,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, GB_MAXCONTROLTURN}, //C4F91
	{0x002f,	12,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, GB_MAXMETER}, //C492
	{0x003f,	36,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //C493
	{0x0040,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //��̨�����Ƿ���Ҫǰ���ֽڿ��Ʋ���
	{0x0041,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //��̨�����Ƿ���ҪRS������Ʋ���
	{0x0042,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //��̨�����Ƿ���Ҫ���Ƶ�̨����
	{0x0043,	4,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //GPRS���̨�ӿڲ�����
	{0x0044,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //GPRS���̨�ӿ�У��λ
	{0x0045,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_230M_PARA, FMT_UNK, 1}, //GPRS���̨�ӿ�У��λ
	{0x0046,	4,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //��̨�ӿڶ�������ʱms
	{0x0047,	4,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //��̨���ͺ��л���ʱms
	{0x0048,	4,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_230M_PARA, FMT_UNK, 1}, //��̨������ʱms
	{0x0049,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //����ʱ��������(δ��)

	{0x004a,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //0x004a����ģ������0-ISD4002��1-tts6188��
	{0x004b,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, // 0x004b tts6188����������С

	{0x0060,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //����230�ն˵����汾ѡ�� 0:���� 1:���� 2:�̽� 3:��Ϫ 4:����
	{0x0061,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //�Ƿ�����ý��ɹ��� 0:���� 1:��
	{0x0062,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //���ý���װ����ʷ�������ɷ�ʽ 0:������װ�� 1:�ն˱�����
	{0x4001,	140,	DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1},//FTP���������ļ������У�
	{0x4002,	25,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1},//FTP���������ļ���������У�
	{0x4003,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1},//FTP������־��1:��������0:δ���� Ĭ��0
	{0x4102,	6,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN, GB_MAXMETER,},//��¼���һ���յ���վ�ظ��Ķ������������ϱ�ȷ��֡��ʱ��
	
	{0x5001,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//�Ƿ���а汾�ж�,0�����а汾�жϣ���0�����汾�ж�
	{0x5002,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//�Ƿ����ü���ģ��,1���ü���ģ�飬����ֵ������
	{0x5003,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_PORTSCH_PARA,	FMT_BIN,1},//�Ƿ����������Զ��ѱ�˿ں�,1���ã�����ֵ������
	{0x5004,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//�Ƿ񿪷�ERC70�¼�,1���ţ�����ֵ�ر�
	{0x5005,	1, 		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK,	1},	//��բ�汾1.Ϊ����97Э��,2Ϊ������ģʽ
	{0x5006,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//��Ч�ִθ���
	{0x5007,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//Ĭ�������������Э���	
	{0x5008,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},// �Ե��ĳ������������Ӧ���жԵ��������Ƿ���Ҫ��ȥ0x33
	{0x5009,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},// ��3P3W���߷�ʽ�ĵ���ѹ��ƽ��ȼ��㷽ʽ 0:��������� 1:���������
	{0x5010, 	64, 	DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_UNK,	1,},//(����05��չ)�ն�IP��ַ�Ͷ˿�
	{0x5011, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//F85(���) �ն˵�ַ����
	{0x5012, 	8, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//F243(���)�ն�sim������
	{0x5015,    520,    DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//(�¹���1������)�ļ���װδ�յ����ݶ� 
	{0x5016, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//������ʽ��0:������ʱ�� 1:�����������
	{0x5017, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//�����������ݿ�ʼ��ʱ�䣨Сʱ��
	{0x5018,    10,     DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//��������F91
	{0x5020, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//������ʽ��0:������ʱ�� 1:�����������
	{0x5021, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//��������:0-ȫ���ر� 1-ֻ�������� 2-ֻ�������¶��� 3-������
	{0x5023, 	2, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BCD,	1,},//������ ��λ����
	{0x5024, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,	1,},//�漴��ʱ��½��D0:����ʱ��½ D1:ʧ��ʱ��½��
	{0x5025, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,	1,},//�Ƿ񱣴������Ϣ��0���� 1���ǣ�
};

BYTE g_bBank24Default[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	0x00, 0x00, 0x00, 0x00, 0x00, //C491(5) �¸��صĸ澯ʱ��Ϳ���ʱ��
	'1', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //C492(12) ���ܱ�ֱ��
	0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 
	0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 
	0x00, 0x05, 0x00, 0x05, //C493(36) ����װ�õĸ���г����ѹ���������޲���
	0x01, //0x0040 1 ��̨�����Ƿ���Ҫǰ���ֽڿ��Ʋ���
	0x00, //0x0041 1 ��̨�����Ƿ���ҪRS������Ʋ���
	0x01, //0x0042 1 ��̨�����Ƿ���Ҫ���Ƶ�̨����
	0x02, 0x00, 0x00, 0x00, //0x0043 4 BCD GPRS���̨�ӿڲ�����
	0x02, //0x0044 1 GPRS���̨�ӿ�У��λ
	0x02, //0x0045 1 ��̨�ӿڷ�������ʱ��У��λ
	0x00, 0x03, 0x00, 0x00, //0x0046 4 BCD ��̨�ӿڶ�������ʱms
	0x00, 0x00, 0x00, 0x00, //0x0047 4 BCD ��̨���ͺ��л���ʱms
	0x50, 0x02, 0x00, 0x00, //0x0048 4 BCD ��̨������ʱms
	0x10, //����ʱ��������(δ��)

	0x01, //0x004a����ģ������0-ISD4002��1-tts6188��
	0x05, // 0x004b tts6188����������С

	0x00, //0x0060 ����230�ն˵����汾ѡ�� 0:���� 1:���� 2:�̽� 3:��Ϫ 4:����
	0x00, //0x0061 �Ƿ�����ý��ɹ��� 0:���� 1:��
	0x01, //0x0062 ���ý���װ����ʷ�������ɷ�ʽ 0:������װ�� 1:�ն˱�����
	//0x4001 FTP���������ļ������У�
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	//0x4002 FTP���������ļ���������У�
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,

	0x00,	//0x4003 FTP������־��1:��������0:δ���� Ĭ��0

	0x00,0x00,0x00,0x00,0x00,0x00,//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,//�Ƿ���а汾�ж� ,0�����а汾�ж�
	0x01,//�Ƿ����ü���ģ��,0�����в�ʹ�ü���ģ��
	0x01, //0x5003 �Ƿ����������Զ��ѱ�˿ں�,1���ã�����ֵ������
	0x01, //0x5004 �Ƿ񿪷�ERC70�¼�,1���ţ�����ֵ�ر�
	0x01, //0x5005 ��բ�汾1.Ϊ����97Э��,2Ϊ������ģʽ	
	0x04,//Ĭ�Ͽ���ģ����Ч�ִθ���
	0x00,//Ĭ�������������Э���	
	0x00, //0x5008  �Ե��ĳ������������Ӧ���жԵ��������Ƿ���Ҫ��ȥ0x33 1--����Ҫ������ֵ����Ҫ
	0x00, //0x5009 ��3P3W���߷�ʽ�ĵ���ѹ��ƽ��ȼ��㷽ʽ 0:���������(̨����Է�ʽ) 1:���������
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,//FN98 (64) �ն�IP��ַ�Ͷ˿�
	0x00, //0x5011
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//0x5012

	//0x5015 ��װ�ļ���־λ����1��δ�ɹ�����0���ɹ�
	0x00,0x00,//��1
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,//��2
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,//��3
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,//��4
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x01,//0x5016
	0x01,//0x5017
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,//0x5020
	0x00,//0x5021
	0x00,0x03,//0x5023
	0x00,//0x5024
	0x01,//0x5025
};

//�ն�У׼������������ʶ--����--Ȩ��--��д--ƫ��
TItemDesc  g_Bank25Desc[] =   //��׼��
{
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver

	{0x5001, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //���ѹ BCD NNNN.NN
	{0x5002, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //����� BCD NNN.NNN
	{0x5003, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //���߷�ʽ BCD NN
	{0x5004, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //���峣�� BCD NNNNNN
	{0x5005, 72, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//7022У׼����	
	{0x500f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

  {0x5011, 96, DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE,FMT_BIN,1},//ֱ��У׼ϵ��
  {0x501f, 0,  DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE},	 

//  {0x5020, 1,  DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE},	//У׼ʱ���¶� 
	{0x5021, 12, DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE},//�������
	{0x5022, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //����У׼ʱ���¶�
	{0x5023, 4, DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE}, // kֵӦ���ڶ��ѹ�����£��źŶ�Ӧ50mv������7022����Ŵ��ĵ�Vu����Vi�ĳ˻�����k=(Vu*����)*��Vi*���棩
};

BYTE g_bBank25Default[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	0x00, 0x00, 0x00, //0x5001 ���ѹ BCD NNNN.NN
	0x00, 0x00, 0x00, //0x5002 ����� BCD NNN.NNN
	0x00, //0x5003 ���߷�ʽ BCD NN
	0x00, 0x00, 0x00, //0x5004 ���峣�� BCD NNNNNN
	0xDA, 0x0B, 0x00, 0x00, 0x45, 0x0C, 0x00, 0x00, 0xB9, 0x0E, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 
	0xAA, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x86, 0x00, 0x00, 0x00, 0x9B, 0x00, 0x00, 0x00, 
	0x81, 0x00, 0x00, 0x00, 0xEF, 0xFF, 0x00, 0x00, 0xED, 0xFF, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 
	0xF6, 0x11, 0x00, 0x00, 0x30, 0x12, 0x00, 0x00, 0xD2, 0x14, 0x00, 0x00, 0x4D, 0xA8, 0x00, 0x00, 
	0x61, 0xA8, 0x00, 0x00, 0x58, 0xA8, 0x00, 0x00, //0x5005 ATT7022��У������

    0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 
	0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 
    0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 
	0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 	
    0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 
	0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, //5011 ֱ��У׼ϵ��

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //�������
	0x00,//����У׼ʱ���¶�
	0x52, 0x03, 0x00, 0x00, //5023 kֵ  0.05*0.17*100000
};

TItemDesc  g_Bank28Desc[] =   //��׼��
{
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver

	{0x0002, 32, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	{0x0003, 8, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	{0x0011, 8, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Iregion[2]; 		// ��λУ׼�����㣬7022Eֻ��2����λУ׼������
	{0x0012, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Pgain[3];			// ����У׼ֵA,B,C
	{0x0013, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Phsreg[3];			// ����У׼�����λ����
	{0x0014, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Poffset[3]; 		// �����й�����OffsetУ��ֵ
	{0x0015, 6, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //WORD Ioffset[3];		// ������ЧֵOffsetУ��ֵ
	{0x0016, 18, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //signed short	AngleOffset[3][3];	// 3������ABC������λ����ֵ����λ��0.01��������
	{0x0017, 6, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //signed short EngErrAdj[3];			// ��Ϊ������ֵ,��λ0.0001%.
	{0x0018, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //WORD	KDivFactor; 		// Kֵ�Ŵ��� *100
	{0x0019, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //BYTE	AdjustFlag; 		// У׼��־��У�����λ
	{0x001f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
};

BYTE g_bBank28Default[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, //0002 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, //0002 

	//new 77
	//DWORD Iregion[2]; 		// ��λУ׼�����㣬7022Eֻ��2����λУ׼������
	0xC8, 0x00, 0x00, 0x00,
	0x0A, 0x00, 0x00, 0x00,
	//DWORD Pgain[3];			// ����У׼ֵA,B,C
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//DWORD Phsreg[3];			// ����У׼�����λ����
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//DWORD Poffset[3]; 		// �����й�����OffsetУ��ֵ
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//WORD	Ioffset[3]; 		// ������ЧֵOffsetУ��ֵ
	0x08, 0x00,
	0x08, 0x00,
	0x08, 0x00,
	//signed short	AngleOffset[3][3];	// 3������ABC������λ����ֵ����λ��0.01��������
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//signed short EngErrAdj[3];			// ��Ϊ������ֵ,��λ0.0001%.
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00,
	//WORD	KDivFactor; 		// Kֵ�Ŵ��� *100
	0x64, 0x00,
	//BYTE	AdjustFlag; 		// У׼��־��У�����λ
	0x00,
	
};

//���ݿ������ƿ�
TBankCtrl g_Bank0Ctrl[SECT_NUM] = {
	//SECTION0
	{"sect0 Eng Data",								//��SECTION������
	 USER_DATA_PATH"EngData.dat",					//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_EngDataDesc,									//��SECTION������������
	 sizeof(g_EngDataDesc)/sizeof(TItemDesc),		//��SECTION����������������������
	 NULL,											//��SECTION���ݿ��Ĭ��ֵ	
	 0x00,											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,											//��SECTION���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,												//��SECTION���ݵĲ��������
	 1,												//��SECTION���ݵľ������
	 false,											//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION1
	{"sect1 Dem-Data",								//��SECTION������
	 USER_DATA_PATH"DemData.dat",					//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_DemDataDesc,									//��SECTION������������
	 sizeof(g_DemDataDesc)/sizeof(TItemDesc),		//��SECTION����������������������
	 NULL,											//��SECTION���ݿ��Ĭ��ֵ	
	 0x00,											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,											//��SECTION���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,												//��SECTION���ݵĲ��������
	 1,												//��SECTION���ݵľ������
	 false,											//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION2
	{"sect2 Variable-Data",							//��SECTION������
	 USER_DATA_PATH"VarData.dat",					//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_VariableDesc,								//��SECTION������������
	 sizeof(g_VariableDesc)/sizeof(TItemDesc),		//��SECTION����������������������
	 NULL,											//��SECTION���ݿ��Ĭ��ֵ	
	 0x00,											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,											//��SECTION���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,												//��SECTION���ݵĲ��������
	 1,												//��SECTION���ݵľ������
	 true,											//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION3
	{"sect3 Event-Data",							//��SECTION������
	 USER_PARA_PATH"EventData.cfg",					//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_EventParaDesc,									//��SECTION������������
	 sizeof(g_EventParaDesc)/sizeof(TItemDesc),			//��SECTION����������������������
	 g_EventParaDefault,											//��SECTION���ݿ��Ĭ��ֵ	
	 sizeof(g_EventParaDefault),											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,											//��SECTION���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,												//��SECTION���ݵĲ��������
	 1,												//��SECTION���ݵľ������
	 false,											//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION4
	{"sect4 para-varible",							//��SECTION������
	 USER_PARA_PATH"ParaVar.dat",					//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_ParaDesc,									//��SECTION������������
	 sizeof(g_ParaDesc)/sizeof(TItemDesc),			//��SECTION����������������������
	 g_bParaDefault,								//��SECTION���ݿ��Ĭ��ֵ	
	 sizeof(g_bParaDefault),						//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,											//��SECTION���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,												//��SECTION���ݵĲ��������
	 1,												//��SECTION���ݵľ������
	 false,											//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION5
	{"sect5 Frz-data",							//��SECTION������
	 USER_PARA_PATH"Frz-Data.cfg",					//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_FrzDesc,									//��SECTION������������
	 sizeof(g_FrzDesc)/sizeof(TItemDesc),		//��SECTION����������������������
	 g_FrzDescDefault,							//��SECTION���ݿ��Ĭ��ֵ	
	 sizeof(g_FrzDescDefault),					//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x02,										//��SECTION���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 false,										//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION6
	{"sect6 Acq-Moni-para",				//��SECTION������
	 USER_PARA_PATH"acq-moni.cfg",			//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_CollecMonitorDesc,				        //��SECTION������������
	 sizeof(g_CollecMonitorDesc)/sizeof(TItemDesc), //��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ	
	 0,											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,										//��SECTION���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 false,										//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION7
	{"sect7 Class-set object",					//��SECTION������
	 USER_DATA_PATH"Class-Set.dat",				//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_SetDesc,									//��SECTION������������
	 sizeof(g_SetDesc)/sizeof(TItemDesc),		//��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ	
	 0,											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,										//��SECTION���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 true,										//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION8
	{"sect8 Class-Ctrl object",					//��SECTION������
	 USER_PARA_PATH"Ctrl.cfg",				//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_CtrlDesc,								//��SECTION������������
	 sizeof(g_CtrlDesc)/sizeof(TItemDesc),		//��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ
	 0,											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 true,										//��BANK�����Ƿ���Ҫ����ʱ��
	 MTRPNMAP,									//��SECTION����ѡ�õĶ�̬�����㷽����,0x00��ʾ��֧������BANK��֧�ֶ�̬������
	},


	//SECTION9
	{"sect9 File-Tran object",					//��SECTION������
	 USER_DATA_PATH"FileTran.dat",					//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_FileTransDesc,							//��SECTION������������
	 sizeof(g_FileTransDesc)/sizeof(TItemDesc),	//��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ
	 0,											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 true,										//��BANK�����Ƿ���Ҫ����ʱ��
	 MTRPNMAP,									//��SECTION����ѡ�õĶ�̬�����㷽����,0x00��ʾ��֧������BANK��֧�ֶ�̬������
	},

	//SECTION10
	{"sect10 Esam-If object",					//��SECTION������
	 USER_PARA_PATH"Esam.cfg",					//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_ESAMIfDesc,								//��SECTION������������
	 sizeof(g_ESAMIfDesc)/sizeof(TItemDesc),	//��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ
	 0x00,										//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 false,										//��BANK�����Ƿ���Ҫ����ʱ��
	},	

	//SECTION11
	{"sect11 In-Out-Dev object",				//��SECTION������
	 USER_PARA_PATH"In_Out_Dev.cfg",			//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_InOutDevDesc,					        //��SECTION������������
	 sizeof(g_InOutDevDesc)/sizeof(TItemDesc),	//��SECTION����������������������
	 g_InOutDevDefault,							//��SECTION���ݿ��Ĭ��ֵ
	 sizeof(g_InOutDevDefault),					//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 false,										//��BANK�����Ƿ���Ҫ����ʱ��
	},	
	
	//SECTION12
	{"sect12 Display object",					//��SECTION������
	 USER_PARA_PATH"Display.cfg",				//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_DisPlayDesc,								//��SECTION������������
	 sizeof(g_DisPlayDesc)/sizeof(TItemDesc),	//��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ
	 0x00,										//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 false,										//��BANK�����Ƿ���Ҫ����ʱ��
	},
	
	//SECTION13
	{"sect13 Ac-Eng-data",						//��SECTION������
	 USER_DATA_PATH"AcEngData.dat",				//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_AcDataDesc,								//��SECTION������������
	 sizeof(g_AcDataDesc)/sizeof(TItemDesc),	//��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ
	 0x00,										//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 false,										//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION14
	{"sect14 point data",						//��SECTION������
	 USER_DATA_PATH"point%d.dat",				//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_PointDataDesc,					        //��SECTION������������
	 sizeof(g_PointDataDesc)/sizeof(TItemDesc), //��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ
	 0,											//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 PN_NUM,									//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 true,										//��BANK�����Ƿ���Ҫ����ʱ��
	 MTRPNMAP,									//��SECTION����ѡ�õĶ�̬�����㷽����,0x00��ʾ��֧������BANK��֧�ֶ�̬������
	},

	//SECTION15
	{"sect15 Ext-Variable-Para",				//��SECTION������
	USER_PARA_PATH"ExtVarPara.cfg",				//��SECTION���ݱ����·���ļ���
	NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	g_ExtVarParaDesc,							//��SECTION������������
	sizeof(g_ExtVarParaDesc)/sizeof(TItemDesc),	//��SECTION����������������������
	NULL,										//��SECTION���ݿ��Ĭ��ֵ	g_ExtVarParaDefault
	0x00,										//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С	sizeof(g_ExtVarParaDefault)
	0x00,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	1,											//��SECTION���ݵĲ��������
	1,											//��SECTION���ݵľ������
	false,										//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION16
	{"sect16 Ext-Event-Data",					//��SECTION������
	USER_DATA_PATH"ExtEventData.dat",			//��SECTION���ݱ����·���ļ���
	NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	g_EventDataDesc,							//��SECTION������������
	sizeof(g_EventDataDesc)/sizeof(TItemDesc),	//��SECTION����������������������
	g_EventDataDefault,							//��SECTION���ݿ��Ĭ��ֵ
	sizeof(g_EventDataDefault),					//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	0x01,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	1,											//��SECTION���ݵĲ��������
	1,											//��SECTION���ݵľ������
	false,										//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//SECTION17
	{"sect17 Ext-Frz-data",						//��SECTION������
	 USER_DATA_PATH"Ext-Frz-Data.dat",			//��SECTION���ݱ����·���ļ���
	 NULL,										//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,										//��SECTION������������
	 0x00,										//��SECTION����������������������
	 NULL,										//��SECTION���ݿ��Ĭ��ֵ
	 0x00,										//��SECTION���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,										//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,											//��SECTION���ݵĲ��������
	 1,											//��SECTION���ݵľ������
	 false,										//��BANK�����Ƿ���Ҫ����ʱ��
	 },
};


//���ݿ������ƿ�
TBankCtrl g_BankCtrl[BANK_NUM] = {

	//BANK0		�㽭Э��涨�Ļ������ݱ�ʶ���ñ����ƿ�������
	{"bank0 sys data",						//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK1
	{"bank1 expara",						//��BANK������
	 USER_PARA_PATH"bank1.cfg",				//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank1Desc,					        //��BANK������������
	 sizeof(g_Bank1Desc)/sizeof(TItemDesc), //��BANK����������������������
	 g_bBank1Default,						//��BANK���ݿ��Ĭ��ֵ	
	 sizeof(g_bBank1Default),				//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},
	 
	//BANK2
	{"bank2 runtime data",					//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���,����Ҫ����
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank2Desc,					        //��BANK������������
	 sizeof(g_Bank2Desc)/sizeof(TItemDesc), //��BANK����������������������
	 NULL,						            //��BANK���ݿ��Ĭ��ֵ,��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK3
	{"bank3 expara",						//��BANK������
	 USER_PARA_PATH"bank3.cfg",             //��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank3Desc,					        //��BANK������������
	 sizeof(g_Bank3Desc)/sizeof(TItemDesc), //��BANK����������������������
	 g_bBank3Default,						//��BANK���ݿ��Ĭ��ֵ	
	 sizeof(g_bBank3Default),				//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},
	 
	//BANK4 --------------�����չ����-----------------
	{"bank4 net meter expara",				//��BANK������
	 USER_PARA_PATH"bank4.cfg",        		//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank4Desc,					        //��BANK������������
	 sizeof(g_Bank4Desc)/sizeof(TItemDesc), //��BANK����������������������
	 g_bBank4Default,						//��BANK���ݿ��Ĭ��ֵ	
	 sizeof(g_bBank4Default),				//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x04,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},
	 
	//BANK5 -------------- �����չ���� ---------------
	{"bank5 07 645 exdata",					//��BANK������
	 USER_PARA_PATH"bank5.dat",        		//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank5Desc,					        //��BANK������������
	 sizeof(g_Bank5Desc)/sizeof(TItemDesc), //��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x02,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK6 -------------- ��������� ---------------
	{"bank6 net meter frozen data",			//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK7 -------------- �㶫�����չ���� ---------------
	{"bank7 gdpb expara",					//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK8 -------------- ��չ ---------------
	 {"bank8",								//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	 },

	//BANK9 -------------- ��չ ---------------
	{"bank9 sd expara",						//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK10 -------------- ��չ ---------------
	{"bank10 GB expara",					//��BANK������
	 USER_PARA_PATH"bank10.cfg",           	//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank10Desc,					        //��BANK������������
	 sizeof(g_Bank10Desc)/sizeof(TItemDesc),//��BANK����������������������
	 g_bBank10Default,						//��BANK���ݿ��Ĭ��ֵ	
	 sizeof(g_bBank10Default),				//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x11,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK11-------------- ������м����� ---------------
	{"bank11 GB tmp data",					//��BANK������
	 USER_DATA_PATH"bank11.dat",			//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank11Desc,					        //��BANK������������
	 sizeof(g_Bank11Desc)/sizeof(TItemDesc),//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x02,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 true,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK12 -------------- ��չ ---------------
	{"bank12 sd expara",						//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK13 -------------- ��չ ---------------
	{"bank13 sd expara",						//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK14 -------------- ��չ ---------------
	{"bank14 sd expara",						//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK15 --------------  ---------------
	 {"bank15 ",								//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	 },

	 //BANK16 --------------  ---------------
	{"bank16 temp data",					//��BANK������
	USER_DATA_PATH"bank16.dat",            //��BANK���ݱ����·���ļ���
	NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	g_Bank16Desc,					        //��BANK������������
	sizeof(g_Bank16Desc)/sizeof(TItemDesc),//��BANK����������������������
	NULL,									//��BANK���ݿ��Ĭ��ֵ
	0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	0x01,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	1,										//��BANK���ݵĲ��������
	1,										//��BANK���ݵľ������
	false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	 //BANK17 ----------- �����Ǳ������� ------------------------
	 {"bank17 cct unsave data",				//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank17Desc,					        //��BANK������������
	 sizeof(g_Bank17Desc)/sizeof(TItemDesc),//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	 },
	
	//BANK18-------------- ����������������� �����ֳ�����ΪҪƵ������ ---------------
	{"bank18 GB tmp data",					//��BANK������
	 USER_DATA_PATH"bank18.dat",			//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank18Desc,					        //��BANK������������
	 sizeof(g_Bank18Desc)/sizeof(TItemDesc),//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 true,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK19 --------------  ---------------
	{"bank19 ",								//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK20 --------------  ---------------
	 {"bank20 ",								//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	 },

	 //BANK21 --------------  ---------------
	 {"bank21 ",								//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	 },

	 //BANK22 --------------  ---------------
	 {"bank22 ",								//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	 },

	 //BANK23 --------------  ---------------
	 {"bank23 ",								//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	 },

		//BANK24 -------------- ��չ ---------------
	{"bank24 GB Temp expara",						//��BANK������
	 USER_PARA_PATH"bank24.cfg",        //��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank24Desc,					        		//��BANK������������
	 sizeof(g_Bank24Desc)/sizeof(TItemDesc), 		//��BANK����������������������
	 g_bBank24Default,							//��BANK���ݿ��Ĭ��ֵ	
	 sizeof(g_bBank24Default),					//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x03,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},

	//BANK25 У׼������չ
	{"bank25 att adj expara",						//��BANK������
	 USER_PARA_PATH"bank25.cfg",             //��BANK���ݱ����·���ļ���
	 USER_BAK_PATH"bank25.cfg",				//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank25Desc,					        //��BANK������������
	 sizeof(g_Bank25Desc)/sizeof(TItemDesc), //��BANK����������������������
	 g_bBank25Default,						//��BANK���ݿ��Ĭ��ֵ	
	 sizeof(g_bBank25Default),				//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x04,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},
	
		//BANK26 ------------- δ�ò��� ---------------
	{"bank26 no use para",				//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},
	
		//BANK27 ------------- δ�ò��� ---------------
	{"bank27 no use para",				//��BANK������
	 NULL,           						//��BANK���ݱ����·���ļ���
	 NULL,									//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 NULL,					        		//��BANK������������
	 0, 									//��BANK����������������������
	 NULL,									//��BANK���ݿ��Ĭ��ֵ	
	 0,										//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x00,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},
	
	//BANK28 У׼������չ
	{"bank28 exd expara",						//��BANK������
	 USER_PARA_PATH"bank28.cfg",             //��BANK���ݱ����·���ļ���
	 USER_BAK_PATH"bank28.cfg",				//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ������ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�(����ֻ֧�ֽ��ɲ�����0������֧��ʱ��ı���,
	 g_Bank28Desc,					        //��BANK������������
	 sizeof(g_Bank28Desc)/sizeof(TItemDesc), //��BANK����������������������
	 g_bBank28Default,						//��BANK���ݿ��Ĭ��ֵ	
	 sizeof(g_bBank28Default),				//��BANK���ݿ��Ĭ��ֵ�Ĵ�С
	 0x01,									//��BANK���ݿ�ĵ�ǰ�汾,0��ʾû�а汾����
	 1,										//��BANK���ݵĲ��������
	 1,										//��BANK���ݵľ������
	 false,									//��BANK�����Ƿ���Ҫ����ʱ��
	},
};


TPnMapCtrl g_PnMapCtrl[PNMAP_NUM] = {	//�����㶯̬ӳ����ƽṹ
//������֧�ֲ������----ʵ��֧�ֲ�������
	//{POINT_NUM,			MFM_NUM},		//�����๦�ܱ�ӳ�䷽��
	{POINT_NUM,			20},		//�����ص㻧ӳ�䷽��,
};


