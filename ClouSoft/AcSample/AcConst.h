/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcConst.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԽ������������Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 * ��    ע: 
 *********************************************************************************************************/
#ifndef ACCONST_H
#define ACCONST_H
#include "LibAcConst.h"
#include "FaCfg.h"

////////////////////////////////////////////////////////////////////////////
//������������
//	#ifdef EN_INMTR
//	#define RATE_PERIOD_NUM    14
//	#else
#define RATE_PERIOD_NUM    8
//	#endif  //VER_METER

#define MAX_DAY_CHART_NUM  8
#define MAX_HOLIDAY_NUM    20
#define MAX_ZONE_NUM	   14	
//#define RATE_PERIOD_NUM    8

  						   



//�����ڲ����ݵ�����
#define PULSE_VAL_P		0
#define PULSE_VAL_Q		1
//#define PULSE_VAL_COS	2

#define PULSE_VAL_NUM	2

#define MAX_PULSE_TYPE  	4		//����������
//��������
#define EP_POS    0		//�����й�
#define EQ_POS    1		//�����޹�
#define EP_NEG    2		//�����й�
#define EQ_NEG    3		//�����޹�

#define MAX_YMNUM  8


/////////////////////////////////////////////////////////////////////////////
//����
#define AC_ENERGY_NUM   	39//8  //���ɵĵ�����������
#define AC_DEMAND_NUM   	34	//���ɵ�������������	

#define AC_ENG_LOG_NUM		AC_ENERGY_NUM	//���ɵ�����־����
#define AC_DMD_LOG_NUM		AC_DEMAND_NUM	//8				//����������־����
#define AC_ENERGY_BAR_NUM		42				//����δ��������������־����

#define PULSE_ENERGY_NUM   	1  //����ĵ�����������
#define PULSE_DEMAND_NUM   	0	//�����������������	


#define	EP_MAX 9999999999LL
#define	EQ_MAX 99999999LL

#endif //ACCONST_H
