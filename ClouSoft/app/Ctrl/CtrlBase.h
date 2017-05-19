/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CtrlBase.h
 * ժ    Ҫ�����ļ���Ҫʵ��CCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 ----------------------------------------------------------------------------------------------------------
--2008-04-09 17:45
᯹��޸����.
�����Ͷ��ʱ
1 �統ǰ��ң����բͶ������,ɾ��������,��������ң��բ��λ.
2 �統ǰ�е��Ͷ��,�����еĿ���״̬��λ,���������Ͷ��״̬.
3	���ǹ����,����������ʣ�����.
4 �統ǰ�й���Ͷ��,�����еĿ���״̬��λ,���������Ͷ��״̬.

���ʣ�
1 ��ʱ�¸��صĵ�ǰ���ʶ�ֵ�����Ƿ��ڱ����������¼��㣿
2 ��� Do()
{
	if (!IsValid())
	{
		ResetCtrl();
		return true;
	}

	return DoCtrl();
}

���������ֽṹ����,�ͱȽ��Ѵ��� ����� �ڱ���󻹼�������ʣ����������

�������� F65 F66 ��Ҫ��������ͳ��
�ں�᯹��������ۺ�,ȷ��,���۹��ػ���,ֻҪ����Ч������ֵ����Ҫ���г���ͳ��.

IsCtrlValid()
IsCmdValid()

--2008-04-10 08:48
᯹��޸����.
1. ��ʱ�¸��ص��ڱ�������Ҳ���.
2. ״̬���ж�,ͳ��,�ŵ�DoCtrl()�������.
3. ���µ�����ֵ(�µ�����ֵ*(1+�µ�������ϵ��))

--2008-04-17 1601
��ѯ�˹���,��2������F50,F52,F65,F66��ͳ��Ӧ����Ӧ�Ŀ���Ͷ��û�й�ϵ,��������ʱ��ı궨��λ���и���,������
����,�ڻ��պ�Ӧ��������,����������,�ڻ��º�Ӧ��������.
*********************************************************************************************************/

#ifndef CTRL_H
#define CTRL_H

//�����ִ�,�� TURN_NUM == 4, CTL_TURN_MASK == 00001111B
#define CTL_TURN_MASK				((BYTE)((1<<TURN_NUM)-1))

//�����ִ�,�� GRP_NUM == 8, CTL_GRP_MASK == 11111111B
#define CTL_GRP_MASK				((BYTE)((1<<GRP_NUM)-1))

#define CTL_POWERON_LOCKTIME		(10*60)		//բ����ϵ�����ʱ��(Ĭ��10����)<��λ:��>.
//#define CTL_POWERON_LOCKTIME			30		//��ֵ���ڵ���ʱʹ��.

#define CTL_POWER_ALR_MIN_TIME		(1*60)		//���ر�������ʱ��(����ʱ��)<��λ:��>

#define CTL_TMPCTRL_SLIP_INTERVAL	(1*60)		//��ʱ�¸���ʱ,���㻬��ƽ��������Ϊ���ض�ֵʱ��ʵʱ���ʲ������.

#define MAX_CTLSTAT_ARRAY_NUM		10*GBC4_MAXSUMGROUP			//������״̬array����
//Control Type
#define CTL_GUARANTEE			0
#define CTL_YkCtrl				1
#define CTL_ENG_MONTH			2
#define CTL_ENG_BUY				3
#define CTL_PWR_TMP				4
#define CTL_PWR_SHUTOUT			5
#define CTL_PWR_REST			6
#define CTL_PWR_PERIOD			7

#define CTL_YkCtrl_CLOSE			8
#define CTL_ENG_MONTH_CLOSE			9
#define CTL_ENG_BUY_CLOSE			10
#define CTL_PWR_TMP_CLOSE			11
#define CTL_PWR_SHUTOUT_CLOSE		12
#define CTL_PWR_REST_CLOSE			13
#define CTL_PWR_PERIOD_CLOSE		14

#define CTL_PWR_TMP_ALLCLOSE			15
#define CTL_PWR_SHUTOUT_ALLCLOSE		16
#define CTL_PWR_REST_ALLCLOSE			17
#define CTL_PWR_PERIOD_ALLCLOSE		18

#define CTL_TURNCLOSE_TICK           30     //��բ״̬��ϵͳ����ͣ����������

//����״̬����
#define QUIT_GUARANTEE			0
#define INPUT_GUARANTEE			1
#define AUTO_GUARANTEE			2

//���Ƶ��������ඨ��
#define PWR_CTL		0	//����
#define ENG_CTL		1	//���

extern bool GetBitStatus(BYTE* pbBuf, WORD wLen, WORD wIdx);	//��ȡĳ��������ָ��λ��״̬.
extern bool SetBitStatus(BYTE* pbBuf, WORD wLen, WORD wIdx, bool fStatus);	//����ĳ��������ָ��λ��״̬.
extern DWORD TranDataFmt19(BYTE* pb);	//ת�����ݸ�ʽ19.
extern DWORD TranDataFmt20(BYTE* pb);	//ת�����ݸ�ʽ20.
extern int GetIdxOfMostRight1(BYTE bFlgs);	//��ȡ�ֽ��������1��λ��(0 ~ 7).
extern int GetIdxOfMostLeft1(BYTE bFlgs);
extern int GetIdxOfAll1InPst(BYTE bFlgs, int iIdx);	//��ȡָ��λ���������ұߵ�1�е�λ�ñ��(0 ~ 7).
extern int GetSumOf1(BYTE bFlgs);		//��ȡ����1�ĸ���.
extern int64 GetValidCurPwr(int iGrp);	//��ȡָ���ܼ�����Ч��ǰ�й�����.
extern int64 GetCurPwr(int iGrp);		//��ȡָ���ܼ���ĵ�ǰ�й�����.
extern int64 GetSelEng(int iGrp, int iSel);	//��ȡָ���ܼ���ָ�����������й��ܵ���.
extern bool IsGuarantee(void);						//�ж��Ƿ��ڱ���״̬.

//����: ��ȡָ���ܼ��鵱ǰ���������й��ܵ���.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//����: ��ǰ���������й��ܵ���.
inline int64 GetCurEng(int iGrp)
{
	return GetSelEng(iGrp, 0);
}

typedef struct
{
	bool	fIfOverLimit;		//�Ƿ���.
	DWORD	dwClick;				//������ʼʱ��.
	int		iGrp;				//���޵��ܼ���.
	int64	iEng;				//����ʱ�ĵ�ǰ����.
} TPwrOverLimitStat;

typedef struct
{
	bool	fIfOverLimit;		//�Ƿ���.
	DWORD	dwClick;				//������ʼʱ��.
	int		iGrp;				//���޵��ܼ���.
	int64	iEng;				//����ʱ�ĵ�ǰ����.
} TEngOverLimitStat;

typedef struct
{
	BYTE bCtrlType;           //�澯���ࣻ
	BYTE bCtrlTurn;			  //�澯�ִΣ�
} TCtrl;

//������������ʱʱ������ת�����롢�֣�BCD�룩
//������@dwCountDown  ����ʱʱ��������@pbBuf-�洢����ʱʱ�䣨�룬�֣��Ļ�������
//���أ�����ʱ������Ӧ��*��*��
inline void CntDownToFmt(DWORD dwCntDown, BYTE* pbBuf)
{
	DWORD dwMin = dwCntDown / 60;
	pbBuf[1] = ByteToBcd((BYTE )dwMin);   //�֣�
	pbBuf[0] = ByteToBcd((BYTE )(dwCntDown - dwMin*60)); //�룻
}

//========================================== CCtrlBase =============================================
class CCtrlBase
{
public:
	CCtrlBase(void){}
	virtual ~CCtrlBase(){}

	virtual bool Init(void) = 0;				//��ʼ��.
	virtual bool DoCtrl(void) = 0;				//���п���.

protected:
	//������,ʹ����������̬ʱ���Ա����,֮���Զ���ɾ�̬��Ա����,����Ϊ��ʵ��ʹ��ʱ,�ж��
	//�����඼��������ʱ����в���,Ϊ����ʱ����ʵ�־�ȷ��ͬ��,����ڻ����ж�������������,
	//��������������ʾ��ͬ��ʱ��,��֮���Զ�������,����Ϊ�ڴ�����Ƶ�������ʹ�������ָ�ʽ��
	//ʱ��ֵ,�������ָ�ʽת�������ַǳ��ز����Һ�ʱ,��������LoadCtrl.DoCtrl()��ת��һ��,
	//�����ϵͳЧ��.
	static TTime	m_tmNow;					//��ǰʱ��.
	static DWORD	m_dwNow;					//��ǰʱ��.

	static TTime	m_tmOldTime;				//�ϴ�ִ�е�ʱ��.
	static DWORD	m_dwOldTime;				//�ϴ�ִ�е�ʱ��.
};

//============================================ CCtrl ===============================================
class CCtrl : public CCtrlBase
{
public:
	CCtrl(void) : m_fCtrlValid(false){}
	virtual ~CCtrl(){}

	virtual bool Init(void);					//��ʼ��.
	bool IsValid(void)	{ return m_fCtrlValid; }; //��ǰ�����Ƿ�Ͷ��.
	bool IsInCtrl(void)	{ return (m_fCtrlValid && m_fInCtrl); }; 	  //�ÿ����Ƿ��ڿ���״̬
	virtual void DoCmdScan(void) = 0;			//ɨ��ϵͳ���е�����.

protected:
	virtual void RstCtrl(void) = 0;				//��λ�ڴ��б������״̬��.
	virtual void ClrCmd(void) = 0;				//����ڴ��б�����ƵĿ�������.

	//---------------------------------------------------------------
	bool SetValidStatus(bool fStatus)			//����Ͷ��״̬(<ture>:Ͷ��; <false>:δͶ��).
	{
		return (m_fCtrlValid = fStatus);
	}
protected:
	bool m_fCtrlValid;	//����Ͷ��״̬.
	bool m_fInCtrl;		//�ÿ����Ƿ��ڿ���״̬
	bool m_fGuarantee;	//�ÿ����Ƿ��Ѿ����ڱ���״̬,��Ҫ�����жϱ�����л�
};

//========================================= CGrpCtrl ===============================================
class CGrpCtrl : public CCtrl
{
public:                             			
	CGrpCtrl();
	virtual ~CGrpCtrl(){}
	void DoCmdScan(void);							//ɨ��ϵͳ����ܼ���ı����������.

	int GetGrp(void)								//��ȡ��ǰ�ܼ���.
	{
		return m_iGrp;
	}
	BYTE GetTurnsStatus(void)						//����ִ�״̬.
	{
		return m_bTurnsStatus;
	}
	
	bool IfOverLimit(void)							//�Ƿ���.
	{
		return m_fIfOverLimit;
	}
	WORD GetOpenTimes(void)							//��ȡ��բ����,Ȼ��������բ����.
	{
		WORD w = m_wOpenTimes;

		m_wOpenTimes = 0;

		return w;
	}

	void MakeDisp(BYTE bTurnsStatus);              //��ʾ���ɺ�����
protected:
	virtual char* CtrlType(char* psz) = 0;			//��ÿ��Ƶ�����(���ؿ������͵��ַ�������).
	virtual int CtrlType(void) = 0;					//��ÿ��Ƶ�����(������������).

	virtual int NewCmdAct(void) = 0;				//��ȡ������Ķ�����.
	virtual DWORD NewCmdTime(void) = 0;				//��ȡ������Ľ���ʱ��.
	virtual int CurCmdAct(void) = 0;				//��ȡ��ǰ����Ķ�����.
	virtual DWORD CurCmdTime(void) = 0;				//��ȡ��ǰ����Ľ���ʱ��.

	virtual bool GetSysCmd(int iGrp) = 0;			//��ȡϵͳ��ָ���ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
	virtual void SaveNewCmd(void) = 0;				//��������(m_NewCmd)���浽m_CtrlCmd��.(ע��: �Բ�ͬ����,m_CtrlCmd�Ľṹ�ǲ�ͬ��)
	virtual bool ClrSysCmd(int iGrp) = 0;			//���ϵͳ��ָ���ܼ��鱾���������.

	virtual BYTE GetSysCtrlTurnsCfg(int iGrp) = 0;	//��ȡϵͳ��ָ���ܼ��鱾����ִ�����״��.
	virtual bool GetSysCtrlFlg(int iGrp) = 0;					//��ȡϵͳ��ָ���ܼ��鱾����Ƶ�״̬��־.
	virtual bool SetSysCtrlFlg(int iGrp, bool fStatus) = 0;		//����ϵͳ��ָ���ܼ��鱾����Ƶ�״̬��־.
	virtual bool RstSysCtrlStatus(int iGrp) = 0;	//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ��״̬��־,����״̬��־�ȵ�).

	//��ʾ������
	virtual bool IsAlarmStatus() = 0;
	virtual BYTE GetCtrlType() = 0;
	virtual BYTE GetInvCtrlType() = 0;

	//---------------------------------------------------------------
	BYTE GetSysCtrlTurnsCfg(int iGrp, int iSel);						//��ȡϵͳ��ָ���ܼ���ָ����������ִ�����״��.
	BYTE GetSysCtrlFlgs(int iGrp, int iSel);							//��ȡϵͳ��ָ���ܼ���ָ������������б�־λ.
	bool ChgSysCtrlFlgs(int iGrp, BYTE bFlgs, bool fStatus, int iCtrlType);	//�ı�ϵͳ��ָ���ܼ���ָ���������ָ����־λ״̬.

	//��ʾ������
	void RemoveDispItem(TCtrl tInvCtrl);
	void AddDispItem(TCtrl tTopCtrl);

	DWORD GetInitClick()
	{
		return m_dwInitClick;
	}

	bool IsOpenStatus()
	{//�Ƿ�����բ״̬��
		return (m_bTurnsStatus>0);
	}

protected:
	int				m_iGrp;							//��ǰ�ܼ���(������ʾû���κ��ܼ���Ͷ��).
	int             m_iCtrlGrp;                     //��ǰ���ڿ���״̬���ܼ���
	BYTE			m_bTurnsStatus;					//��ǰ���ִ�״̬.
	BYTE			m_bWarnStatus;					//��ǰ�ĸ澯�ִ�.	
	bool			m_fIfOverLimit;					//�Ƿ񳬹��޶�ֵ.
	WORD			m_wOpenTimes;					//��բ����.

	DWORD m_dwInitClick;				//��ʾ��բ��ʼʱ�̣�
	bool  m_fAlarmStatus;				//�������Ƿ��ڸ澯״̬��
	bool  m_fOpenStatus;				//�������Ƿ�����բ״̬
	BYTE  m_bCloseTurn;					//��բ���ִΣ�
};

//========================================= CEngCtrl ===============================================
class CEngCtrl : public CGrpCtrl
{
public:                             			
	CEngCtrl(void){ m_fAlrStauts = 0; m_dwOpenTurnTime =0; };
	virtual ~CEngCtrl(){};
	virtual bool IsBeepAlr(void) = 0;			//�Ƿ���������.

protected:
	virtual bool GetSysStatus(void) = 0;			//���ڳ�ʼ��ʱ,��ϵͳ���б�����Ƶ��ִ�״̬,����״̬��ͬ�����ڴ��ж�Ӧ�ı���.

	virtual bool GetSysCtrlAlr(int iGrp) = 0;		//��ȡϵͳ��ָ���ܼ��鱾����Ƶı���״̬��־.
	virtual bool SetSysCtrlAlr(int iGrp, bool fStatus) = 0;		//����ϵͳ��ָ���ܼ��鱾����Ƶı���״̬��־.

	virtual bool SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus) = 0;			//�趨ϵͳ��ָ���ܼ��鱾��������ִ�״̬.
	virtual bool ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus) = 0;	//�ı�ϵͳ��ָ���ܼ��鱾���������Ӧ�ִ�״̬.

protected:
	bool GetSysEngStatus(int iSel);					//���ڳ�ʼ��ʱ,��ϵͳ����ָ������Ƶ��ִ�״̬,����״̬��ͬ�����ڴ��ж�Ӧ�ı���.

	BYTE GetSysEngAlrFlgs(int iGrp);											//��ȡϵͳ��ָ���ܼ������������������б���״̬��־λ.
	bool ChgSysEngAlrFlgs(int iGrp, BYTE bFlgs, bool fStatus);					//�ı�ϵͳ��ָ���ܼ���ָ���������ָ������״̬��־.

	BYTE GetSysCtrlTurnsCfg(int iGrp)											//��ȡָ���ܼ��鱾����ִ�����״��.
	{
		return CGrpCtrl::GetSysCtrlTurnsCfg(iGrp, 1);
	}
	bool SetSysEngTurnsStatus(int iGrp, BYTE bTurnsStatus, int iSel);			//�趨ϵͳ��ָ���ܼ���ָ����������ִ�״̬.
	bool ChgSysEngTurnsStatus(int iGrp, BYTE bTurns, bool fStatus, int iSel);	//�ı�ϵͳ��ָ���ܼ���ָ�����������Ӧ�ִ�״̬.
	bool RstSysCtrlStatus(int iGrp)												//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
	{
		return SetSysCtrlFlg(iGrp, false);
	}

	bool IsAlarmStatus()
	{
		return m_fAlrStauts;
	}
	DWORD GetEngTurnInv(int iTurn);
protected:
	bool					m_fAlrStauts;										//����״̬.
	DWORD					m_dwOpenTurnTime;									//�ϴ���բʱ��.
};

//========================================= CPwrCtrl ===============================================
class CPwrCtrl : public CGrpCtrl
{
public:
	CPwrCtrl(void);
	virtual ~CPwrCtrl(){}
	bool IsAlr(void)
	{
		return ((m_dwAlrTime != 0) && GetCtrlType()<CTL_YkCtrl_CLOSE);
	}
	bool IsBeepAlr(void)								//�Ƿ���������.
	{
		return (m_dwAlrTime != 0);
	}
	
	void SumOverLimitPara(int& riGrp, DWORD& rdwTime, int64& riEng);//�ۼӹ��ʶ�ֵ���޲���(��Ӧ�ܼ������������ʱ�估���޵���).

	virtual	void DoSaveOpenRec(void) = 0;							//������բ��¼.
	

protected:
	void DoSavePwrCtrlOpenRec(int iSel);							//���湦����բ��¼.
	BYTE GetSysCtrlTurnsCfg(int iGrp)								//��ȡָ���ܼ��鱾����ִ�����״��.
	{
		return CGrpCtrl::GetSysCtrlTurnsCfg(iGrp, 0);
	}
	bool RstSysCtrlStatus(int iGrp)									//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
	{
		return SetSysCtrlFlg(iGrp, false);
	}

	int64 GetPwrSafeLimit(void);									//��ȡ���ر�����ֵ.
	DWORD GetPwrAlrPersistTime(int iTurn);							//��ȡָ���ִεĹ��ر�������ʱ��.
	int64 GetPwrLimit(void)											//��ȡ��ǰ�Ĺ��ʶ�ֵ,������.
	{
		return m_iPwrLimit;
	}

	DWORD GetPwrSlideInterv(int iGrp);

	bool SetSysCurPwrLimit (int iGrp, int64 iPwrLimit);				//�趨ָ���ܼ��鵱ǰ���ض�ֵ.

	bool IsAlarmStatus()
	{
		return (m_dwAlrTime > 0);	
	}
	
	void SaveDisp(WORD wDelayTime, DWORD dwStime, int64 iCurPwr);
	bool RestoreTurnStatus();
	
protected:
	int64					m_iPwrLimit;							//��ǰ�Ĺ��ʶ�ֵ,������.
	DWORD					m_dwFrzDly;								//������բ���ʶ�����ʱ.
	DWORD					m_dwAlrTime;							//����������ʱ��.
	DWORD					m_dwRstTime;							//��բ�ָ�������ʱ��.	
	DWORD					m_dwGuaranteeAlrTime;							//����״̬�±���������ʱ��.

	int64					m_iCurPwrLimit;							//��ǰ���ʶ�ֵ.

	TPwrOverLimitStat		m_OLStat;								//����ͳ��.

	DWORD				m_dwPwrStartClick;	//���¹��ʿ�ʼ�����ʱ��,0��ʾ֮ǰûͶ��,��ûͶ��תΪͶ��,Ҫ�ȴ����ػ���ʱ�����ȡ����
};

class CAllPwrCtrl;	//�����ṩ��'��ʱ�¸���','Ӫҵ��ͣ��','���ݿ�','ʱ�ο�'������Ԫ.

#endif  //CTRL_H
