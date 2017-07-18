#ifndef CCTSCHMTR_H
#define CCTSCHMTR_H

#include "ComStruct.h"
#include "TaskManager.h"

typedef struct {
	//0x6002 ����08
	bool fAutoSchMtr;	//�Ƿ�����ÿ�������ѱ�
	bool fAutoUpdMtr;	//�Ƿ��Զ����²ɼ�����
	bool fIsGenEvt;		//�Ƿ��������¼�
	BYTE bClrMtrChoice;	//����ѱ���ѡ��
}TSchMtrParam;	//�ѱ���� 

typedef struct {
	TTime tStartTime;	//��ʼʱ�� 
	WORD wKeptTime;	//�ѱ�ʱ����min��
}TSchMtrTimeParam;	//��ʱ�ѱ����

typedef struct {
	DWORD dwOAD;	//������������ֵ
	BYTE bData[16];	//����ֵ
}TSchAddInfo;	//�ѵ��ĸ�����Ϣ

typedef struct {
	DWORD dwPort;
	BYTE bMtrLen;	//�ѱ��ַ����
	BYTE bMtr[TSA_LEN];	//�ѱ��ַ
	BYTE bAcqLen;	//�����ɼ���ַ����
	BYTE bAcqAddr[TSA_LEN];	//�����ɼ���ַ
	BYTE bMtrPro;	//��Լ����	0-δ֪, 1-DL/T645-1997, 2-DL/T645-2007, 3-DL/T698.45, 4-CJ/T 188-2004
	BYTE bPhase;	//��λ	0-δ֪, 1-A, 2-B, 3-C
	BYTE bSQ;		//�ź�Ʒ��
	TTime tSchMtrSucTime;	//�ѱ�ɹ�ʱ��
	BYTE bCS;	//ǰ�沿�����ݵ�У���
	//BYTE bSchAddInfoCnt;	//�ѵ��ĸ�����Ϣ����
	//TSchAddInfo tTSchAddInfo[8];	
}
/*#ifdef SYS_WIN*/
TSchMtrRlt;	//�ѱ���
// #else
// __attribute__((packed))TSchMtrRlt;	//�ѱ���
// #endif


typedef struct {
	BYTE bMtrAddr[TSA_LEN+1];	//ͨ�ŵ�ַ, bMtrAddr[0]�ǵ�ַ����
	BYTE bMainAddr[TSA_LEN+1];	//���ڵ��ַ, bMtrAddr[0]�ǵ�ַ����
	TTime tUpdTime;	//���ʱ��
}
/*#ifdef SYS_WIN*/
TCrossSchMtrRlt;	//��̨���ѱ���
// #else
// __attribute__((packed))TCrossSchMtrRlt;	//��̨���ѱ���
// #endif


#define MK_SCH_MTR_FILE(pszTabName)				sprintf(pszTabName,"%sSearchMtrFile.dat", USER_DATA_PATH)
#define PER_MTR_OFFSET_LEN(X)					(sizeof(TSchMtrRlt)*(X-1))	//X>0
#define PER_RLT_LEN								sizeof(TSchMtrRlt)	//һ���ѱ����ĳ���

#define MK_CROSS_SCH_MTR_FILE(pszTabName)		sprintf(pszTabName, "%sSearchCrossMtrFile.dat", USER_DATA_PATH)
#define PER_CROSS_MTR_OFFSET_LEN(X)				(sizeof(TCrossSchMtrRlt)*(X-1))	//X>0
#define PER_CROSS_RLT_LEN						sizeof(TCrossSchMtrRlt)	


typedef enum {
	SCH_MTR_EMPTY = 0,		//�ѱ�״̬δ����
	START_BOARD_SCH_MTR,	//�����㲥�ѱ�
	START_NODE_ACTIVE,		//�����ڵ�ע��
	WAIT_MTR_REPORT,		//�ȴ�����ϱ�
	FINISH_SCH_MTR,			//����ѱ�	
}TSEARCH_STATE;

class CCctSchMeter{
public:
	CCctSchMeter(void);

	virtual ~CCctSchMeter(void){};

	TSchMtrParam m_TSchMtrParm;

	bool m_fStartBoardCast;	//�����㲥��ʶ

	DWORD m_dwStartBoardCastClk;	//�����㲥�ѱ��ʱ��

	bool m_fRightNowSchMtr;	//���������ѱ�

	bool m_fPeriodSchMtr;	//ʱ���ѱ�

	bool m_fRptSchMtrEnd;	//�ѱ��������

	bool m_fClrFile;	//�Ƿ�����ļ�

	BYTE m_bActCnt;	//����ע�����

	DWORD m_dwLastRptMtrClk;	//��һ���ѱ���·��ģ�齻����ʱ�ӵδ�

	TSem m_tSchMtrSem;

	TSem m_tAlarmSem;	//�¼������ź���

	bool DoSchMtrAddr();

private:
	BYTE m_bSchMtrState;

	bool m_fUdpMtrToDB;	//�Ƿ���Ҫ���µ�����ϵͳ��

public:
	//��������ȡ�ѱ����
	bool GetSchMtrParam(TSchMtrParam *pSchMtrParam);

	//�������Ƿ����ѱ�ʱ�β���(60020900)
	//���أ�>0��ʾ���ѱ�ʱ�䷶Χ��
	bool IsSchMtrPeriod();

	//�����������ѱ�״̬
	//������true-�ѱ��У�false-����
	int SetSchMtrState(bool fState);

	//������У����ַ
	void CheckMtrAddr(TSchMtrRlt *pSchMtrRlt);

	//������дһ���ѱ���
	//������	@pbBuf �ѱ���
	bool SaveOneSchMtrResult(TSchMtrRlt *pSchMtrRlt);

	//�������滻һ���ѱ���
	//������@wIdx ����
	//		@pSchMtrRlt Ҫ�滻�ĵ���
	bool ReplaceOneSchMtrResult(WORD wIdx, TSchMtrRlt *pSchMtrRlt);

	//������ͨ���ļ�������ɾ����Ӧ�ı���
	void DelSchMtrResult(WORD wIdx);

	//�����������ѱ����Ƿ�ȱ��
	//��ע�����滻ԭ�еĵ���֮�������������
	//	1.���滻���ַ�󣬲�ѯ��ԭ�ɼ����Ƿ��Ѿ������ڣ������ھ�Ҫ����һ��ACQ+NULL�Ĳɼ���
	//  2.��ѯ�Ƿ���ΪNULL�Ĳɼ�������
	void LoopSchMtrResult(TSchMtrRlt *pSchMtrRlt);

	void LoopMtrSysDb(TOobMtrInfo tMtrInfo);

	//�������洢�ѱ���
	//������	@pbBuf �ѱ���
	int SaveSchMtrResult(DWORD dwPortOad, BYTE *pbBuf, WORD wLen, BYTE bMtrAddrLen = 6);

	//��������ȡ�ѱ���
	//������@piStart �״δ���Ϊ-1���ѱ���δ��ȡ��ͷ�����Ӧ��ֵ����ȡ��������0xFFFE
	//		@fGetAll ����ǻ�ȡ�������ݣ���������*piStart=-1 �� fGetAll = true
	//���أ� -1������>0���ζ�ȡ�Ĵ���
	int GetSchMtrResult(int *piStart, TSchMtrRlt *pSchMtrRlt, bool fGetAll = true);

	//��������ȡ�ѱ���
	int GetSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen, DWORD dwStartSec = 0, DWORD dwEndSec = 0);

	//��������ȡ��̨���ѱ���
	int GetCrossSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen);

	//����������̨���ѱ�����¼��
	int CalSchMtrNum();

	//�������洢��̨���ѱ���
	//������	@pbBuf �ѱ���
	int SaveCrossSchMtrResult(BYTE *pbBuf, WORD wLen);

	int SaveOneCrossSchMtrResult(TCrossSchMtrRlt *pSchMtrRlt);

	//����������һ�������ݿ�ĵ������
	void UpdataSchMtrToSysDb(TSchMtrRlt *pSchMtrRlt);

	//������Ѱ��һ���յı���Ŵ洢�ѻصı���
	void SearchEmptySaveMeter(TSchMtrRlt *pSchMtrRlt, BYTE *pbMtrMask);

	//�����������ѱ��¼�
	void SaveAlarmEvent(TSchMtrRlt *pSchMtrRlt);

	void SchMtrRltConvertMtrInfo(TSchMtrRlt *pSchMtrRlt, TOobMtrInfo *pMtrInfo);

	void MtrInfoConvertSchMtrRlt(TOobMtrInfo *pMtrInfo, TSchMtrRlt *pSchMtrRlt);

	//������ɾ���ѱ���
	void DeleteSearchMtrFile();

	//������ɾ����̨�����
	void DeleteCrossSearchMtrFile();

	int GetRightNowSchMtrKeepTime();

	//����������ӽڵ�ȴ�ʱ��
	int GetNodeActWaitMin();

	//���������µ������ʶ
	void SetUdpMtrFlg(bool fState);

	//��������ȡ�������ʶ
	bool GetUdpMtrFlg();

	//���������¸澯�¼��ź�������
	//������@wIndex ����δ֪�������
	//		@fState false:�������wIndex��Ӧ�ı�ʶ����֮
	bool SetSchMtrEvtMask(WORD wIndex, bool fState);

	//���������¸澯�¼��ź�������
	bool UpdataSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen);
	//��������ȡ�澯�¼��ź�������
	bool GetSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen);

	//����������澯�¼�������
	void ClearSchMtrEvtMask();

	//������ͨ��������ȡ�澯����
	//������@iIndex �澯����
	//		@pbBuf ���صĸ澯��������
	//���أ�-1��ȡ����ʧ�ܣ�>0�澯���ݵĳ���
	int GetSchMtrEvtData(int iIndex, BYTE *pbBuf);

	//������У���ϴ��ѱ�ʱ���������Ƿ񳬹�30��
	void CheckMeterSearchTime();

protected:

	virtual bool StartBoardCast(int iMin) { iMin; return false;};

	virtual int StartSchMtr(){return -1;};

	virtual bool StartNodeActive(){return false;}

	virtual bool WaitMtrReport(){return false;};

	virtual int FinishSchMtr(){return -1;};

};

#endif