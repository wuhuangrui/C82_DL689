/********************************************************************
* Description: att7022.h
*
*
* Author:	Francis.Yang
* License: GPL Version 2
* System: Linux2.6.21
*
* Copyright (c) 2009 All rights reserved.
*********************************************************************
*/
#ifndef ATT7022_H
#define ATT7022_H

//�Ĵ�����ַ
//�����Ĵ���
#define r_DeviceID	0x00 //Device ID
//���ʼĴ���
#define r_Pa  0x01	//A���й�����
#define r_Pb  0x02	//B���й�����
#define r_Pc  0x03	//C���й�����
#define r_Pt  0x04	//�����й�����
#define r_Qa  0x05	//A���޹�����
#define r_Qb  0x06	//B���޹�����
#define r_Qc  0x07	//C���޹�����
#define r_Qt  0x08	//�����޹�����
#define r_Sa  0x09	//A�����ڹ���
#define r_Sb  0x0A	//B�����ڹ���
#define r_Sc  0x0B	//C�����ڹ���
#define r_St  0x0C	//�������ڹ���

//��Чֵ�Ĵ���
#define r_UaRms  0x0D		//A���ѹ��Чֵ
#define r_UbRms  0x0E		//B���ѹ��Чֵ
#define r_UcRms  0x0F		//C���ѹ��Чֵ
#define r_IaRms  0x10		//A�������Чֵ
#define r_IbRms  0x11		//B�������Чֵ
#define r_IcRms  0x12		//C�������Чֵ
#define r_ItRms	 0X13		//�������ʸ����Чֵ��

#define r_Pfa  0x14			//A�๦������
#define r_Pfb  0x15			//B�๦������
#define r_Pfc  0x16			//C�๦������
#define r_Pft  0x17			//���๦������
#define r_Pga  0x18			//A��������ѹ���
#define r_Pgb  0x19			//B��������ѹ���
#define r_Pgc  0x1A			//C��������ѹ���
#define r_INTFlag  0x1B		//�жϱ�־����������
#define r_Freq 0x1C			//��Ƶ��

#define r_EFlag	0x1D		//���ܼĴ����Ĺ���״̬����������

#define r_Epa  0x1E			//A���й�����
#define r_Epb  0x1F			//B���й�����
#define r_Epc  0x20			//C���й�����
#define r_Ept  0x21			//�����й�����
#define r_Eqa  0x22			//A���޹�����
#define r_Eqb  0x23			//B���޹�����
#define r_Eqc  0x24			//C���޹�����
#define r_Eqt  0x25			//�����޹�����

#define r_YUaUb 0x26		//Ua��Ub�ĵ�ѹ�н�
#define r_YUaUc 0x27		//Ua��Uc�ĵ�ѹ�н�
#define r_YUbUc 0x28		//Ub��Uc�ĵ�ѹ�н�

#define r_IORms 0x29		//�������IOͨ����Чֵ
#define r_TPSD  0x2A		//�¶ȴ����������

#define r_UtRms 0x2B		//�����ѹʸ���͵���Чֵ

#define r_Sflag 0x2C 		//��Ŷ��ࡢ����SIG�ȱ�־״̬


#define r_BckReg 		0x2D	//ͨѶ���ݱ��ݼĴ���
#define r_ComChksum 	0x2E	//ͨ��У��ͼĴ���
#define r_Sample_IA		0x2F	//A�����ͨ��ADC��������
#define r_Sample_IB		0x30	//B�����ͨ��ADC��������
#define r_Sample_IC		0x31	//C�����ͨ��ADC��������
#define r_Sample_UA		0x32	//A���ѹͨ��ADC��������
#define r_Sample_UB		0x33	//B���ѹͨ��ADC��������
#define r_Sample_UC		0x34	//C���ѹͨ��ADC��������


#define r_Esa			0x35	//A�����ڵ��ܣ�������Ϊ�������㣩 
#define r_Esb			0x36	//B�����ڵ��ܣ�������Ϊ�������㣩 
#define r_Esc			0x37	//C�����ڵ��ܣ�������Ϊ�������㣩 
#define r_Est			0x38	//�������ڵ��ܣ�������Ϊ�������㣩 
#define r_FstCntA		0x39	//A������������ 
#define r_FstCntB		0x3A	//B������������ 
#define r_FstCntC		0x3B	//C������������ 
#define r_FstCntT		0x3C	//�������������� 
#define r_PFlag			0x3D	//�й�/�޹����ʷ�������Ϊ0������Ϊ1 
#define r_ChkSum		0x3E	//У������У���
#define r_Sample_I0		0x3F	//���ߵ���I0ͨ������������� 
#define r_LinePa		0x40	//A������й����� 
#define r_LinePb		0x41	//B������й�����  
#define r_LinePc		0x42	//C������й����� 
#define r_LinePt		0x43	//��������й����� 
#define r_LineEpa		0x44	//A������й����ܣ�������Ϊ�������㣩 
#define r_LineEpb		0x45	//B������й����ܣ�������Ϊ�������㣩 
#define r_LineEpc		0x46	//C������й����ܣ�������Ϊ�������㣩 
#define r_LineEpt		0x47	//��������й����ܣ�������Ϊ�������㣩 
#define r_LineUaRrms	0x48	//����A���ѹ��Чֵ 
#define r_LineUbRrms	0x49	//����B���ѹ��Чֵ 
#define r_LineUcRrms	0x4A	//����C���ѹ��Чֵ 
#define r_LineIaRrms	0x4B	//����A�������Чֵ 
#define r_LineIbRrms	0x4C	//����B�������Чֵ 
#define r_LineIcRrms	0x4D	//����C�������Чֵ 
#define r_LEFlag			0x4E	//�������ܼĴ����Ĺ���״̬���������� 
#define r_ChipID        0x5D  //оƬ�汾ָʾ�Ĵ���
#define r_ChkSumNew		0x5E	//����У��Ĵ���У���(0x60~0x70)
#define r_PtrWavebuff	0x7E	//��������ָ�룬ָʾ�ڲ�����buffer�������ݳ��� 
#define r_WaveBuff		0x7F	//�������ݼĴ������ڲ������棬�ظ���ȡֱ�����껺�����ݳ��� 



//У��Ĵ���
//#define Reserved		0x00	//У������Ĵ�����ʼ��־ 
#define w_ModeCfg		0x01	//ģʽ��ؿ��� 
#define w_PGACtrl		0x02	//ADC�������� 
#define w_EMUCfg		0x03	//EMU��Ԫ���� 
#define w_PgainA		0x04	//A���й��������� 
#define w_PgainB		0x05	//B���й��������� 
#define w_PgainC		0x06	//C���й��������� 
#define w_QgainA		0x07	//A���޹��������� 
#define w_QgainB		0x08	//B���޹��������� 
#define w_QgainC		0x09	//C���޹��������� 
#define w_SgainA		0x0A	//A�����ڹ������� 
#define w_SgainB		0x0B	//B�����ڹ������� 
#define w_SgainC		0x0C	//C�����ڹ������� 
#define w_PhSregApq0	0x0D	//A����λУ��0 
#define w_PhSregBpq0	0x0E	//B����λУ��0 
#define w_PhSregCpq0	0x0F	//C����λУ��0 
#define w_PhSregApq1	0x10	//A����λУ��1 
#define w_PhSregBpq1	0x11	//B����λУ��1 
#define w_PhSregCpq1	0x12	//C����λУ��1 
#define w_PoffsetA		0x13	//A���й�����offsetУ�� 
#define w_PoffsetB		0x14	//B���й�����offsetУ�� 
#define w_PoffsetC		0x15	//C���й�����offsetУ�� 
#define w_QPhscal		0x16	//�޹���λУ�� 
#define w_UgainA		0x17	//A���ѹ���� 
#define w_UgainB		0x18	//B���ѹ���� 
#define w_UgainC		0x19	//C���ѹ���� 
#define w_IgainA		0x1A	//A��������� 
#define w_IgainB		0x1B	//B��������� 
#define w_IgainC		0x1C	//��������� 
#define w_Istarup		0x1D	//�𶯵�����ֵ���� 
#define w_Hfconst		0x1E	//��Ƶ����������� 
#define w_FailVoltage	0x1F	//ʧѹ��ֵ����
#define w_GainADC7		0x20	//����·ADC�����ź����� 
#define w_QoffsetA		0x21	//A���޹�����offsetУ�� 
#define w_QoffsetB		0x22	//B���޹�����offsetУ�� 
#define w_QoffsetC		0x23	//C���޹�����offsetУ�� 
#define w_UaRmsoffse	0x24	//A���ѹ��ЧֵoffsetУ�� 
#define w_UbRmsoffse	0x25	//B���ѹ��ЧֵoffsetУ�� 
#define w_UcRmsoffse	0x26	//C���ѹ��ЧֵoffsetУ�� 
#define w_IaRmsoffse	0x27	//A�������ЧֵoffsetУ�� 
#define w_IbRmsoffse	0x28	//B�������ЧֵoffsetУ�� 
#define w_IcRmsoffse	0x29	//C�������ЧֵoffsetУ�� 
#define w_UoffsetA		0x2A	//A���ѹͨ��ADC offsetУ�� 
#define w_UoffsetB		0x2B	//B���ѹͨ��ADC offsetУ�� 
#define w_UoffsetC		0x2C	//C���ѹͨ��ADC offsetУ�� 
#define w_IoffsetA		0x2D	//A�����ͨ��ADC offsetУ�� 
#define w_IoffsetB		0x2E	//B�����ͨ��ADC offsetУ�� 
#define w_IoffsetC		0x2F	//C�����ͨ��ADC offsetУ�� 
#define w_EMUIE			0x30	//�ж�ʹ�� 
#define w_ModuleCFG		0x31	//��·ģ�����üĴ��� 
#define w_AllGain		0x32	//ȫͨ�����棬����Vref���¶�У�� 
#define w_HFDouble		0x33	//���峣���ӱ�ѡ�� 
#define w_LineGain		0x34	//��������У�� 
#define w_PinCtrl		0x35	//����pin����������ѡ����� 
#define w_Pstart		0x36	//�𶯹������üĴ��� 
#define w_Iregion		0x37	//��λ�����������üĴ��� 
#define w_Iregion1  0x60  //��λ�����������üĴ���1
#define w_PhSregApq2 0x61 //A����λУ��2
#define w_PhSregBpq2 0x62 //B����λУ��2
#define w_PhSregCpq2 0x63 //C����λУ��2
#define w_PoffsetAL		0x64	//A ���й�����offset У�����ֽ�
#define w_PoffsetBL		0x65	//B ���й�����offset У�����ֽ�
#define w_PoffsetCL		0x66	//C ���й�����offset У�����ֽ�
#define w_QoffsetAL		0x67	//A ���޹�����offset У�����ֽ�
#define w_QoffsetBL		0x68	//B ���޹�����offset У�����ֽ�
#define w_QoffsetCL		0x69	//C ���޹�����offset У�����ֽ�
#define w_ItRmsoffset	0x6A	//����ʸ����offsetУ��
#define w_TPSoffset 0x6B  //TPS��ֵУ���Ĵ���
#define w_TPSgain   0x6C  //TPSб��У���Ĵ��� 
#define TCcoffA     0x6D  //Vrefgain�Ķ���ϵ�� 
#define TCcoffB     0x6E  //Vrefgain��һ��ϵ�� 
#define TCcoffC     0x6F  //Vrefgain�ĳ�����
#define w_EMCfg     0x70  //���㷨���ƼĴ���



#define gWaveCommand   0x40
#define gWaveAddress   0x41
#define ptrWaveFormRd  0x7e
#define mWaveDatatmp   0x7f


#define ATT_CTRL_RESET		  _IOW('a', 1, char)   //��λ
#define ATT_CTRL_START_ADJ 	  _IOW('a', 2, char)   //����У��
#define ATT_CTRL_STOP_ADJ	  _IOW('a', 3, char) //ֹͣУ��,����У��Ĵ���������д
#define ATT_CTRL_GET_REGVAL   _IOW('a', 4, char) //��ȡָ���Ĵ���ֵ
#define ATT_CTRL_WR_REG		  _IOW('a', 5, char) //дָ���Ĵ��� 
#define ATT_CTRL_CHECKSUM 	  _IOW('a', 6, char)  //У��
#define ATT_CTRL_CLR_ADJREG	  _IOW('a', 7, char)  //У��Ĵ�����0
#define ATT_CTRL_TO_ADJREG    _IOW('a', 8, char)  //0x00~0x7f��У��Ĵ�����ַ
#define ATT_CTRL_TO_MREG	  _IOW('a', 9, char)  //0x00~0x7f�Ǽ����Ĵ�����ַ



typedef struct {
	unsigned char cRegAddr;
	unsigned int  iRegVal;
}TAttReg;

#define		ATT_DEV		"/dev/att7022"

#endif
