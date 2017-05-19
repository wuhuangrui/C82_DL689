#ifndef DEFAD173360AR
#define DEFAD173360AR
// 15   14      13 12 11        10 9 8      7 6 5 4 3 2 1 0
// C/D  R/W  DEVICE ADDRESSS   REGISTER    <ADDRESS REGISTER DATA>
#define  CRA 0
#define  CRB 1
#define  CRC 2
#define  CRD 3
#define  CRE 4
#define  CRF 5
#define  CRG 6
#define  CRH 7

enum CRegAdr
{
	kCRA,
	kCRB,
	kCRC,
	kCRD,	
	kCRE,	
	kCRF,
	kCRG,	
	kCRH,
};

#define  CD           (1<<15)
#define  RW           (1<<14)
#define  DEVICEADR(x) ((x)<<11)
#define  REGISTERADR(x) ((x)<<8)


//CRA

#define DATAPGM (1<<0)//0 DATA/PGM Operating Mode (0 = Program; 1 = Data Mode)
#define MM      (1<<1)//1 MM Mixed Mode (0 = OFF; 1 = Enabled)
//#define             //2 Reserved Must Be Programmed to Zero (0)
#define SLB     (1<<3)//3 SLB SPORT Loop-Back Mode (0 = OFF; 1 = Enabled)
#define DC0     (1<<4)//4 DC0 Device Count (Bit 0)
#define DC1     (1<<5)//5 DC1 Device Count (Bit 1)
#define DC2     (1<<6)//6 DC2 Device Count (Bit 2)
#define RESET   (1<<7)//7 RESET Software Reset (0 = OFF; 1 = Initiates Reset)

#define DeviceCount(x)  ((x)<<4)//x>1
//CRB
#define DR0  (1<<0)//0 DR0 Decimation Rate (Bit 0)
#define DR1  (1<<1)//1 DR1 Decimation Rate (Bit 1)
#define SCD0 (1<<2)//2 SCD0 Serial Clock Divider (Bit 0)
#define SCD1 (1<<3)//3 SCD1 Serial Clock Divider (Bit 1)
#define MCD0 (1<<4)//4 MCD0 Master Clock Divider (Bit 0)
#define MCD1 (1<<5)//5 MCD1 Master Clock Divider (Bit 1)
#define MCD2 (1<<6)//6 MCD2 Master Clock Divider (Bit 2)
#define CEE  (1<<7)//7 CEE Control Echo Enable (0 = OFF; 1 = Enabled)

//假设 DMCLK = 16.384K
#define  SAMPLE8K      (0 * DR1 + 0 * DR0)
#define  SAMPLE16K     (0 * DR1 + 1 * DR0)
#define  SAMPLE32K     (1 * DR1 + 0 * DR0)
#define  SAMPLE64K     (1 * DR1 + 1 * DR0)
#define  DMCLKDIV(x)   ((x-1) << 4)//x为分频数>1
#define  SCLKDIV8      (0 * SCD1 + 0 * SCD0)
#define  SCLKDIV4      (0 * SCD1 + 1 * SCD0)
#define  SCLKDIV2      (1 * SCD1 + 0 * SCD0)
#define  SCLKDIV1      (1 * SCD1 + 1 * SCD0)


//CRC
#define GPU (1<<0)//0 GPU Global Power-Up Device (0 = Power Down; 1 = Power Up)
//1 Reserved Must Be Programmed to Zero (0)
//2 Reserved Must Be Programmed to Zero (0)
//3 Reserved Must Be Programmed to Zero (0)
//4 Reserved Must Be Programmed to Zero (0)
#define PUREF  (1<<5)//5 PUREF REF Power (0 = Power Down; 1 = Power Up)
#define REFOUT (1<<6) //6 RU REFOUT Use (0 = Disable REFOUT; 1 = Enable REFOUT)
#define P5VEN  (1<<7)//7 5VEN Enable 5 V Operating Mode (0 = Disable 5 V Mode;1 = Enable 5 V Mode)


//CRD

#define I1GS0 (1<<0)//0 I1GS0 ADC1:Input Gain Select (Bit 0)
#define I1GS1 (1<<1)//1 I1GS1 ADC1:Input Gain Select (Bit 1)
#define I1GS2 (1<<2)//2 I1GS2 ADC1:Input Gain Select (Bit 2)
#define PUI1  (1<<3)//3 PUI1 Power Control (ADC1); 1 = ON, 0 = OFF
#define I2GS0 (1<<4)//4 I2GS0 ADC2:Input Gain Select (Bit 0)
#define I2GS1 (1<<5)//5 I2GS1 ADC2:Input Gain Select (Bit 1)
#define I2GS2 (1<<6)//6 I2GS2 ADC2:Input Gain Select (Bit 2)
#define PUI2  (1<<7)//7 PUI2 Power Control (ADC2); 1 = ON, 0 = OFF
#define  GAINCH1(x) (x<<0)
#define  GAINCH2(x) (x<<4)
#define  GAINCH1ON  (1<<3)
#define  GAINCH2ON  (1<<7)

//CRE
#define I3GS0 (1<<0)//0 I3GS0 ADC3:Input Gain Select (Bit 0)
#define I3GS1 (1<<1)//1 I3GS1 ADC3:Input Gain Select (Bit 1)
#define I3GS2 (1<<2)//2 I3GS2 ADC3:Input Gain Select (Bit 2)
#define PUI3  (1<<3)//3 PUI3 Power Control (ADC3); 1 = ON, 0 = OFF
#define I4GS0 (1<<4)//4 I4GS0 ADC4:Input Gain Select (Bit 0)
#define I4GS1 (1<<5)//5 I4GS1 ADC4:Input Gain Select (Bit 1)
#define I4GS2 (1<<6)//6 I4GS2 ADC4:Input Gain Select (Bit 2)
#define PUI4  (1<<7)//7 PUI4 Power Control (ADC4); 1 = ON, 0 = OFF
#define  GAINCH3(x) (x<<0)
#define  GAINCH4(x) (x<<4)
#define  GAINCH3ON  (1<<3)
#define  GAINCH4ON  (1<<7)

//CRF
 #define I5GS0 (1<<0)//0 I5GS0 ADC5:Input Gain Select (Bit 0)
 #define I5GS1 (1<<1)//1 I5GS1 ADC5:Input Gain Select (Bit 1)
 #define I5GS2 (1<<2)//2 I5GS2 ADC5:Input Gain Select (Bit 2)
 #define PUI5  (1<<3)//3 PUI5 Power Control (ADC5); 1 = ON, 0 = OFF
 #define I6GS0 (1<<4)//4 I6GS0 ADC6:Input Gain Select (Bit 0)
 #define I6GS1 (1<<5)//5 I6GS1 ADC6:Input Gain Select (Bit 1)
 #define I6GS2 (1<<6)//6 I6GS2 ADC6:Input Gain Select (Bit 2)
 #define PUI6  (1<<7)//7 PUI6 Power Control (ADC6); 1 = ON, 0 = OFF
  
#define  GAINCH5(x) (x<<0)
#define  GAINCH6(x) (x<<4)
#define  GAINCH5ON  (1<<3)
#define  GAINCH6ON  (1<<7)

//CRG
#define CH1  (1<<0)//0 CH1 Channel 1 Select
#define CH2  (1<<1)//1 CH2 Channel 2 Select
#define CH3  (1<<2)//2 CH3 Channel 3 Select
#define CH4  (1<<3)//3 CH4 Channel 4 Select
#define CH5  (1<<4)//4 CH5 Channel 5 Select
#define CH6  (1<<5)//5 CH6 Channel 6 Select
#define RMOD (1<<6)//6 RMOD Reset Analog Modulator
#define SEEN (1<<7)//7 SEEN Enable Single-Ended Input Mode

//CRH
 
//0 CH1 Channel 1 Select
//1 CH2 Channel 2 Select
//2 CH3 Channel 3 Select
//3 CH4 Channel 4 Select
//4 CH5 Channel 5 Select
//5 CH6 Channel 6 Select

#define RMOD (1<<6)//6 TME Test Mode Enable
#define SEEN (1<<7)//7 INV Enable Invert Channel Mode

#endif
