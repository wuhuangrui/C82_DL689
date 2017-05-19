#ifndef ATT_H
#define ATT_H

#include "apptypedef.h"
#include "att7022.h"

#define  TEP_AUTO   1   //自动温度补偿
#define 		PI			3.1415927

#ifndef CONNECT_3P4W
#define			CONNECT_3P4W		4			//三相四线
#endif

#ifndef CONNECT_1P
#define			CONNECT_1P			1			//单相
#endif

#ifndef CONNECT_3P3W
#define			CONNECT_3P3W		3			//三相三线
#endif

#ifndef SCN_NUM
#define	SCN_NUM						6			//通道数
#endif

#ifndef QUAD_NEG_P
#define QUAD_NEG_P        0x01
#define QUAD_NEG_Q        0x02
#endif

#define DATA_2EXP15			32768.0		//7022E
#define DATA_2EXP16			65536.0		//7022E
#define DATA_2EXP20			1048576.0
#define DATA_2EXP21			2097152.0
#define DATA_2EXP22			4194304.0
#define DATA_2EXP23			8388608.0
#define DATA_2EXP24			16777216.0
#define DATA_2EXP13			8192.0

#define VALID_FLAG				0x5A	// 有效标志
#define INVALID_FLAG			0xA5	// 无效标志

// ------------------------------------------------------
// ATT电压、电流通道增益,由校表时指定.
// 7022E配置原则: 输入Un、Ib时，电压通道采样值*增益倍数=0.22V，电流采样值*增益倍数=0.05V
// 应用笔记推荐:电压取样电阻两端电压0.1V,增益倍数配置为2.
//目前电压增益固定2倍，以下倍数均针对电流而言	lx20121230
#define ATT_VC_GAIN1			0x100					// 1倍增益.即不放大
#define ATT_VC_GAIN2			0x154					// 2倍增益
#define ATT_VC_GAIN8			0x1A8					// 8倍增益
#define ATT_VC_GAIN16			0x1FC					// 16倍增益				
// ------------------------------------------------------
// ATT7022E芯片相关的固定值
#define	ATT_PHReg_Unit					(long)(1.8919*256)				// 0.01误差对应的ATT7022E的相位校准寄存器的值(扩大256倍便于+-运算).
#define	ATT_IStart_RegValue_001Ib		0x189				// 启动电流为0.001Ib时需写入芯片启动电流寄存器的值
//#define	IrmsN							60					//电流有效值比例系数N,与Vi有关lx

//efine ATT_VERSION 0X02
#define BIT0                (0x00000001)
#define BIT1                (0x00000002)
#define BIT2                (0x00000004)
#define BIT3                (0x00000008)
#define BIT4                (0x00000010)
#define BIT5                (0x00000020)
#define BIT6                (0x00000040)
#define BIT7                (0x00000080)
#define BIT8                (0x00000100)
#define BIT9                (0x00000200)

#define BITA                (0x00000400)
#define BITB                (0x00000800)
#define BITC                (0x00001000)
#define BITD                (0x00002000)
#define BITE                (0x00004000)
#define BITF                (0x00008000)

#define BIT10                (0x00010000)
#define BIT11                (0x00020000)
#define BIT12                (0x00040000)
#define BIT13                (0x00080000)
#define BIT14                (0x00100000)
#define BIT15                (0x00200000)
#define BIT16                (0x00400000)
#define BIT17                (0x00800000)
#define BIT18                (0x01000000)
#define BIT19                (0x02000000)
#define BIT1A                (0x04000000)
#define BIT1B                (0x08000000)
#define BIT1C                (0x10000000)
#define BIT1D                (0x20000000)
#define BIT1E                (0x40000000)
#define BIT1F                (0x80000000)



#define MAX_FRACTIONY  4
#define MAX_FRACTIONF  3


#define MAX_EPIECE	 32		//7022电能寄存器：ABC合相正向、反向无功；基波有功


#define AR_I 0
#define AR_U 1
#define AR_E 2
#define AR_P 3
#define AR_Y 4
#define AR_F 5


typedef struct {
    unsigned int UI[6+1];	//Ua Ub Uc Ia Ib Ic Inal
    unsigned int Phase[9];	//PhUb,PhUc,PhIa,PhIb,PhIc,Angle A,Angle B,Angle C, PhIc-PhIa 
    long Power[4][3];		//{Pa Pb Pc} {Qa Qb Qc} {Sa Sb Sc} {P Q S}
	unsigned int Frequency;/*电网频率，单位：0.001Hz*/
    unsigned int UI_Line[6];	//基波Ua Ub Uc Ia Ib Ic 
}TFrontData;	/*电表采集前端类型*/

typedef struct{
	DWORD dwConst; 	//脉冲常数
	BYTE  bConnectType; //终端接线方式 1	1:单相;3:三项三线;4:三相四线
	DWORD dwUn;		   //额定电压,格式NNNNN.N
	DWORD dwIn;		   //额定电流,格式NNN.NNN
	BYTE bEpMode;	   //有功电能累加模式
					   //		D2			|		D1			|			D0			|
					   // 反向计入正向 		| 	反向不计量		| 		各相累加模式	|
					   // 0:不计入;1计入	| 0:计量;1不计量	| 0:代数和;1绝对值和	|
					   // D2D1=00:正->正,反->反;正反向分别计量
					   // D2D1=01:正->正;		只计正向,反向不计
					   // D2D1=10:正反->正,反->反;
					   // D2D1=11:正反->正		反向不计
}TAttPara;

typedef struct{
//unsigned long Ioffset[3];			//电流有效值Offset校正值
//unsigned long Poffset[3];//三相有功功率Offset校正值
  unsigned long Pgain[3];//A,B,C  --30
  unsigned long Phsreg[3][3];//-----//2Vi,100%Vi,10%Vi,A,B,C  --42
  unsigned long UI[6];//Ua,Ub,Uc,Ia,Ib,Ic --78    
}TAdjPara;	//校准参数72bytes; 


typedef struct
{
	DWORD	Iregion[2];			// 相位校准分区点，7022E只有2个相位校准分区点 0.01Ib
	DWORD	Pgain[3];			// 功率校准值A,B,C
	DWORD	Phsreg[3];			// 三相校准点的相位补偿
	DWORD	Poffset[3];			// 三相有功功率Offset校正值
	WORD	Ioffset[3];			// 电流有效值Offset校正值
	signed short AngleOffset[3][3];	// 3个分区ABC三相相位误差补偿值，单位：0.01，带符号
	signed short EngErrAdj[3];			// 人为误差调整值,单位0.0001%.
	WORD	KDivFactor;			// K值放大倍数 *100
	BYTE	AdjustFlag;			// 校准标志，校表后置位
}TNewAdjPara;//128bytes; 


class CAtt{
public:
	CAtt();
	virtual ~CAtt();
    
	TFrontData m_Front;

	bool Init();
	void Reset();
	bool Run();	
	bool CalibrateAtt();
			
	bool IfNeedInit();
	void EnableInit(bool fNeeInit);
	bool CheckSum();
	
	void SetAdjStep(WORD wAdjStep);
	void SetPulseRatio(BYTE bPulseRatio);// lzx 20170225
	BYTE GetPulseRatio(void);			
	void GetStdMtrVal(BYTE* pbBuf, WORD wLen);		//取标准表功率数据
	
	WORD GetQuad(WORD i);
	void CalcAdjPara(BYTE *pbBuf);
	void ReadHarmonic(WORD wChannel);
	void ReadAdcBuf();
	bool ReadAdcBuf_new();
	bool GetAttChipStatus();
	
	DWORD Read(unsigned char RegAddr);
	void Write(unsigned char RegAddr,unsigned long RegData);
	DWORD ReadCheck(unsigned char RegAddr);
	bool WriteCheck(unsigned char RegAddr,unsigned long RegData);
	DWORD FreqDetection(void);
	WORD GetDeltaEnery(WORD wIndex);
	DWORD ReadDataCheck(BYTE bRegAddr);  //liyan

private:
	WORD m_wDeltaE[MAX_EPIECE];
	bool m_fNeedInitAtt;
	WORD m_wAdjStep;
	BYTE m_bConnectType;	
	//WORD m_wQuad;
	DWORD m_dwConst;
	DWORD m_dwCalCheckSum;
	DWORD m_dwCalCheckSumNew;  //新增校表寄存器校验和(0x60~0x70)
	DWORD m_dwRatio[6];
	DWORD m_dwCheckClick;
	DWORD m_dwHFConst;
	
	DWORD m_dwStdMtrData[6];	//标准表功率数据
	
	int   m_iFd;
	TAttPara m_AcPara;
	TAdjPara m_AdjPara;
	
	TNewAdjPara m_NewAdjPara;// 新校准方法参数liuzhixing 20170225
	BYTE	m_PulseRatio;	// 脉冲放大倍数 liuzhixing 20170225			//
	
	int   m_iAdcBufReadWaitTimer;//adc 采样次数
	bool  m_fAdcBufOk;//adc buf ok 
	int   m_iAdcBufReadCnt;//adc 采样值读取次数，分次读取
	
#ifdef TEP_AUTO	
	BYTE  m_bTemp;  //校准时的温度
	DWORD m_Ver;
	BYTE ReadTemp();
#endif
	bool m_fAttFault;
	
	void CfgReg();
	void InitRatio();
	int	 RunErrorCheck(void);
	void ReadVoltageAndCurrent(void);
	void ReadLineVoltageAndCurrent(void);
	void ReadPower(void);
	void ReadPowerFactor(void);
	void ReadFreq(void);
	void ReadPhaseAngle(void);
	void ReadEnergy(void);
	void PrintChksum(int step);
};


#endif
