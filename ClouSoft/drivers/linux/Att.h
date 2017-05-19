#ifndef ATT_H
#define ATT_H

#include "apptypedef.h"
#include "att7022.h"

#define  TEP_AUTO   1   //�Զ��¶Ȳ���
#define 		PI			3.1415927

#ifndef CONNECT_3P4W
#define			CONNECT_3P4W		4			//��������
#endif

#ifndef CONNECT_1P
#define			CONNECT_1P			1			//����
#endif

#ifndef CONNECT_3P3W
#define			CONNECT_3P3W		3			//��������
#endif

#ifndef SCN_NUM
#define	SCN_NUM						6			//ͨ����
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

#define VALID_FLAG				0x5A	// ��Ч��־
#define INVALID_FLAG			0xA5	// ��Ч��־

// ------------------------------------------------------
// ATT��ѹ������ͨ������,��У��ʱָ��.
// 7022E����ԭ��: ����Un��Ibʱ����ѹͨ������ֵ*���汶��=0.22V����������ֵ*���汶��=0.05V
// Ӧ�ñʼ��Ƽ�:��ѹȡ���������˵�ѹ0.1V,���汶������Ϊ2.
//Ŀǰ��ѹ����̶�2�������±�������Ե�������	lx20121230
#define ATT_VC_GAIN1			0x100					// 1������.�����Ŵ�
#define ATT_VC_GAIN2			0x154					// 2������
#define ATT_VC_GAIN8			0x1A8					// 8������
#define ATT_VC_GAIN16			0x1FC					// 16������				
// ------------------------------------------------------
// ATT7022EоƬ��صĹ̶�ֵ
#define	ATT_PHReg_Unit					(long)(1.8919*256)				// 0.01����Ӧ��ATT7022E����λУ׼�Ĵ�����ֵ(����256������+-����).
#define	ATT_IStart_RegValue_001Ib		0x189				// ��������Ϊ0.001Ibʱ��д��оƬ���������Ĵ�����ֵ
//#define	IrmsN							60					//������Чֵ����ϵ��N,��Vi�й�lx

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


#define MAX_EPIECE	 32		//7022���ܼĴ�����ABC�������򡢷����޹��������й�


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
	unsigned int Frequency;/*����Ƶ�ʣ���λ��0.001Hz*/
    unsigned int UI_Line[6];	//����Ua Ub Uc Ia Ib Ic 
}TFrontData;	/*���ɼ�ǰ������*/

typedef struct{
	DWORD dwConst; 	//���峣��
	BYTE  bConnectType; //�ն˽��߷�ʽ 1	1:����;3:��������;4:��������
	DWORD dwUn;		   //���ѹ,��ʽNNNNN.N
	DWORD dwIn;		   //�����,��ʽNNN.NNN
	BYTE bEpMode;	   //�й������ۼ�ģʽ
					   //		D2			|		D1			|			D0			|
					   // ����������� 		| 	���򲻼���		| 		�����ۼ�ģʽ	|
					   // 0:������;1����	| 0:����;1������	| 0:������;1����ֵ��	|
					   // D2D1=00:��->��,��->��;������ֱ����
					   // D2D1=01:��->��;		ֻ������,���򲻼�
					   // D2D1=10:����->��,��->��;
					   // D2D1=11:����->��		���򲻼�
}TAttPara;

typedef struct{
//unsigned long Ioffset[3];			//������ЧֵOffsetУ��ֵ
//unsigned long Poffset[3];//�����й�����OffsetУ��ֵ
  unsigned long Pgain[3];//A,B,C  --30
  unsigned long Phsreg[3][3];//-----//2Vi,100%Vi,10%Vi,A,B,C  --42
  unsigned long UI[6];//Ua,Ub,Uc,Ia,Ib,Ic --78    
}TAdjPara;	//У׼����72bytes; 


typedef struct
{
	DWORD	Iregion[2];			// ��λУ׼�����㣬7022Eֻ��2����λУ׼������ 0.01Ib
	DWORD	Pgain[3];			// ����У׼ֵA,B,C
	DWORD	Phsreg[3];			// ����У׼�����λ����
	DWORD	Poffset[3];			// �����й�����OffsetУ��ֵ
	WORD	Ioffset[3];			// ������ЧֵOffsetУ��ֵ
	signed short AngleOffset[3][3];	// 3������ABC������λ����ֵ����λ��0.01��������
	signed short EngErrAdj[3];			// ��Ϊ������ֵ,��λ0.0001%.
	WORD	KDivFactor;			// Kֵ�Ŵ��� *100
	BYTE	AdjustFlag;			// У׼��־��У�����λ
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
	void GetStdMtrVal(BYTE* pbBuf, WORD wLen);		//ȡ��׼��������
	
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
	DWORD m_dwCalCheckSumNew;  //����У��Ĵ���У���(0x60~0x70)
	DWORD m_dwRatio[6];
	DWORD m_dwCheckClick;
	DWORD m_dwHFConst;
	
	DWORD m_dwStdMtrData[6];	//��׼��������
	
	int   m_iFd;
	TAttPara m_AcPara;
	TAdjPara m_AdjPara;
	
	TNewAdjPara m_NewAdjPara;// ��У׼��������liuzhixing 20170225
	BYTE	m_PulseRatio;	// ����Ŵ��� liuzhixing 20170225			//
	
	int   m_iAdcBufReadWaitTimer;//adc ��������
	bool  m_fAdcBufOk;//adc buf ok 
	int   m_iAdcBufReadCnt;//adc ����ֵ��ȡ�������ִζ�ȡ
	
#ifdef TEP_AUTO	
	BYTE  m_bTemp;  //У׼ʱ���¶�
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
