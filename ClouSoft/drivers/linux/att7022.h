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

//寄存器地址
//计量寄存器
#define r_DeviceID	0x00 //Device ID
//功率寄存器
#define r_Pa  0x01	//A相有功功率
#define r_Pb  0x02	//B相有功功率
#define r_Pc  0x03	//C相有功功率
#define r_Pt  0x04	//合相有功功率
#define r_Qa  0x05	//A相无功功率
#define r_Qb  0x06	//B相无功功率
#define r_Qc  0x07	//C相无功功率
#define r_Qt  0x08	//合相无功功率
#define r_Sa  0x09	//A相视在功率
#define r_Sb  0x0A	//B相视在功率
#define r_Sc  0x0B	//C相视在功率
#define r_St  0x0C	//合相视在功率

//有效值寄存器
#define r_UaRms  0x0D		//A相电压有效值
#define r_UbRms  0x0E		//B相电压有效值
#define r_UcRms  0x0F		//C相电压有效值
#define r_IaRms  0x10		//A相电流有效值
#define r_IbRms  0x11		//B相电流有效值
#define r_IcRms  0x12		//C相电流有效值
#define r_ItRms	 0X13		//合相电流矢量有效值和

#define r_Pfa  0x14			//A相功率因素
#define r_Pfb  0x15			//B相功率因素
#define r_Pfc  0x16			//C相功率因素
#define r_Pft  0x17			//合相功率因素
#define r_Pga  0x18			//A相电流与电压相角
#define r_Pgb  0x19			//B相电流与电压相角
#define r_Pgc  0x1A			//C相电流与电压相角
#define r_INTFlag  0x1B		//中断标志，读后清零
#define r_Freq 0x1C			//线频率

#define r_EFlag	0x1D		//电能寄存器的工作状态，读后清零

#define r_Epa  0x1E			//A相有功电能
#define r_Epb  0x1F			//B相有功电能
#define r_Epc  0x20			//C相有功电能
#define r_Ept  0x21			//合相有功电能
#define r_Eqa  0x22			//A相无功电能
#define r_Eqb  0x23			//B相无功电能
#define r_Eqc  0x24			//C相无功电能
#define r_Eqt  0x25			//合相无功电能

#define r_YUaUb 0x26		//Ua与Ub的电压夹角
#define r_YUaUc 0x27		//Ua与Uc的电压夹角
#define r_YUbUc 0x28		//Ub与Uc的电压夹角

#define r_IORms 0x29		//零序电流IO通道有效值
#define r_TPSD  0x2A		//温度传感器的输出

#define r_UtRms 0x2B		//三相电压矢量和的有效值

#define r_Sflag 0x2C 		//存放断相、相序、SIG等标志状态


#define r_BckReg 		0x2D	//通讯数据备份寄存器
#define r_ComChksum 	0x2E	//通信校验和寄存器
#define r_Sample_IA		0x2F	//A相电流通道ADC采样数据
#define r_Sample_IB		0x30	//B相电流通道ADC采样数据
#define r_Sample_IC		0x31	//C相电流通道ADC采样数据
#define r_Sample_UA		0x32	//A相电压通道ADC采样数据
#define r_Sample_UB		0x33	//B相电压通道ADC采样数据
#define r_Sample_UC		0x34	//C相电压通道ADC采样数据


#define r_Esa			0x35	//A相视在电能（可配置为读后清零） 
#define r_Esb			0x36	//B相视在电能（可配置为读后清零） 
#define r_Esc			0x37	//C相视在电能（可配置为读后清零） 
#define r_Est			0x38	//合相视在电能（可配置为读后清零） 
#define r_FstCntA		0x39	//A相快速脉冲计数 
#define r_FstCntB		0x3A	//B相快速脉冲计数 
#define r_FstCntC		0x3B	//C相快速脉冲计数 
#define r_FstCntT		0x3C	//合相快速脉冲计数 
#define r_PFlag			0x3D	//有功/无功功率方向，正向为0，负向为1 
#define r_ChkSum		0x3E	//校表数据校验和
#define r_Sample_I0		0x3F	//零线电流I0通道采样数据输出 
#define r_LinePa		0x40	//A相基波有功功率 
#define r_LinePb		0x41	//B相基波有功功率  
#define r_LinePc		0x42	//C相基波有功功率 
#define r_LinePt		0x43	//合相基波有功功率 
#define r_LineEpa		0x44	//A相基波有功电能（可配置为读后清零） 
#define r_LineEpb		0x45	//B相基波有功电能（可配置为读后清零） 
#define r_LineEpc		0x46	//C相基波有功电能（可配置为读后清零） 
#define r_LineEpt		0x47	//合相基波有功电能（可配置为读后清零） 
#define r_LineUaRrms	0x48	//基波A相电压有效值 
#define r_LineUbRrms	0x49	//基波B相电压有效值 
#define r_LineUcRrms	0x4A	//基波C相电压有效值 
#define r_LineIaRrms	0x4B	//基波A相电流有效值 
#define r_LineIbRrms	0x4C	//基波B相电流有效值 
#define r_LineIcRrms	0x4D	//基波C相电流有效值 
#define r_LEFlag			0x4E	//基波电能寄存器的工作状态，读后清零 
#define r_ChipID        0x5D  //芯片版本指示寄存器
#define r_ChkSumNew		0x5E	//新增校表寄存器校验和(0x60~0x70)
#define r_PtrWavebuff	0x7E	//缓冲数据指针，指示内部缓冲buffer已有数据长度 
#define r_WaveBuff		0x7F	//缓冲数据寄存器，内部自增益，重复读取直至读完缓冲数据长度 



//校表寄存器
//#define Reserved		0x00	//校表参数寄存器起始标志 
#define w_ModeCfg		0x01	//模式相关控制 
#define w_PGACtrl		0x02	//ADC增益配置 
#define w_EMUCfg		0x03	//EMU单元配置 
#define w_PgainA		0x04	//A相有功功率增益 
#define w_PgainB		0x05	//B相有功功率增益 
#define w_PgainC		0x06	//C相有功功率增益 
#define w_QgainA		0x07	//A相无功功率增益 
#define w_QgainB		0x08	//B相无功功率增益 
#define w_QgainC		0x09	//C相无功功率增益 
#define w_SgainA		0x0A	//A相视在功率增益 
#define w_SgainB		0x0B	//B相视在功率增益 
#define w_SgainC		0x0C	//C相视在功率增益 
#define w_PhSregApq0	0x0D	//A相相位校正0 
#define w_PhSregBpq0	0x0E	//B相相位校正0 
#define w_PhSregCpq0	0x0F	//C相相位校正0 
#define w_PhSregApq1	0x10	//A相相位校正1 
#define w_PhSregBpq1	0x11	//B相相位校正1 
#define w_PhSregCpq1	0x12	//C相相位校正1 
#define w_PoffsetA		0x13	//A相有功功率offset校正 
#define w_PoffsetB		0x14	//B相有功功率offset校正 
#define w_PoffsetC		0x15	//C相有功功率offset校正 
#define w_QPhscal		0x16	//无功相位校正 
#define w_UgainA		0x17	//A相电压增益 
#define w_UgainB		0x18	//B相电压增益 
#define w_UgainC		0x19	//C相电压增益 
#define w_IgainA		0x1A	//A相电流增益 
#define w_IgainB		0x1B	//B相电流增益 
#define w_IgainC		0x1C	//相电流增益 
#define w_Istarup		0x1D	//起动电流阈值设置 
#define w_Hfconst		0x1E	//高频脉冲输出设置 
#define w_FailVoltage	0x1F	//失压阈值设置
#define w_GainADC7		0x20	//第七路ADC输入信号增益 
#define w_QoffsetA		0x21	//A相无功功率offset校正 
#define w_QoffsetB		0x22	//B相无功功率offset校正 
#define w_QoffsetC		0x23	//C相无功功率offset校正 
#define w_UaRmsoffse	0x24	//A相电压有效值offset校正 
#define w_UbRmsoffse	0x25	//B相电压有效值offset校正 
#define w_UcRmsoffse	0x26	//C相电压有效值offset校正 
#define w_IaRmsoffse	0x27	//A相电流有效值offset校正 
#define w_IbRmsoffse	0x28	//B相电流有效值offset校正 
#define w_IcRmsoffse	0x29	//C相电流有效值offset校正 
#define w_UoffsetA		0x2A	//A相电压通道ADC offset校正 
#define w_UoffsetB		0x2B	//B相电压通道ADC offset校正 
#define w_UoffsetC		0x2C	//C相电压通道ADC offset校正 
#define w_IoffsetA		0x2D	//A相电流通道ADC offset校正 
#define w_IoffsetB		0x2E	//B相电流通道ADC offset校正 
#define w_IoffsetC		0x2F	//C相电流通道ADC offset校正 
#define w_EMUIE			0x30	//中断使能 
#define w_ModuleCFG		0x31	//电路模块配置寄存器 
#define w_AllGain		0x32	//全通道增益，用于Vref的温度校正 
#define w_HFDouble		0x33	//脉冲常数加倍选择 
#define w_LineGain		0x34	//基波增益校正 
#define w_PinCtrl		0x35	//数字pin上下拉电阻选择控制 
#define w_Pstart		0x36	//起动功率设置寄存器 
#define w_Iregion		0x37	//相位补偿区域设置寄存器 
#define w_Iregion1  0x60  //相位补偿区域设置寄存器1
#define w_PhSregApq2 0x61 //A相相位校正2
#define w_PhSregBpq2 0x62 //B相相位校正2
#define w_PhSregCpq2 0x63 //C相相位校正2
#define w_PoffsetAL		0x64	//A 相有功功率offset 校正低字节
#define w_PoffsetBL		0x65	//B 相有功功率offset 校正低字节
#define w_PoffsetCL		0x66	//C 相有功功率offset 校正低字节
#define w_QoffsetAL		0x67	//A 相无功功率offset 校正低字节
#define w_QoffsetBL		0x68	//B 相无功功率offset 校正低字节
#define w_QoffsetCL		0x69	//C 相无功功率offset 校正低字节
#define w_ItRmsoffset	0x6A	//电流矢量和offset校正
#define w_TPSoffset 0x6B  //TPS初值校正寄存器
#define w_TPSgain   0x6C  //TPS斜率校正寄存器 
#define TCcoffA     0x6D  //Vrefgain的二次系数 
#define TCcoffB     0x6E  //Vrefgain的一次系数 
#define TCcoffC     0x6F  //Vrefgain的常数项
#define w_EMCfg     0x70  //新算法控制寄存器



#define gWaveCommand   0x40
#define gWaveAddress   0x41
#define ptrWaveFormRd  0x7e
#define mWaveDatatmp   0x7f


#define ATT_CTRL_RESET		  _IOW('a', 1, char)   //复位
#define ATT_CTRL_START_ADJ 	  _IOW('a', 2, char)   //启动校表
#define ATT_CTRL_STOP_ADJ	  _IOW('a', 3, char) //停止校表,保护校表寄存器不被误写
#define ATT_CTRL_GET_REGVAL   _IOW('a', 4, char) //读取指定寄存器值
#define ATT_CTRL_WR_REG		  _IOW('a', 5, char) //写指定寄存器 
#define ATT_CTRL_CHECKSUM 	  _IOW('a', 6, char)  //校验
#define ATT_CTRL_CLR_ADJREG	  _IOW('a', 7, char)  //校表寄存器清0
#define ATT_CTRL_TO_ADJREG    _IOW('a', 8, char)  //0x00~0x7f是校表寄存器编址
#define ATT_CTRL_TO_MREG	  _IOW('a', 9, char)  //0x00~0x7f是计量寄存器编址



typedef struct {
	unsigned char cRegAddr;
	unsigned int  iRegVal;
}TAttReg;

#define		ATT_DEV		"/dev/att7022"

#endif
