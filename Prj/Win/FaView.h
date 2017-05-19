// FaView.h : CFaView ��Ľӿ�
//


#pragma once

#define SYS_BUF_SIZE       0x10000
class CFaView : public CView
{
protected: // �������л�����
	CFaView();
	DECLARE_DYNCREATE(CFaView)

// ����
public:
	CFaDoc* GetDocument() const;

// ����
public:
	void DisplayImage();
	afx_msg void OnTimer(UINT nIDEvent);

	afx_msg BOOL PreTranslateMessage(MSG*   pMsg) ;

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CFaView();
	afx_msg LRESULT OnSysNotify(WPARAM wParam, LPARAM lParam);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CEdit       m_editSys; 
    BYTE        m_szSysBuf[SYS_BUF_SIZE];
	DWORD	    m_dwSysBufPtr;

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // FaView.cpp �еĵ��԰汾
inline CFaDoc* CFaView::GetDocument() const
   { return reinterpret_cast<CFaDoc*>(m_pDocument); }
#endif

