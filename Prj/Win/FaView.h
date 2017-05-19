// FaView.h : CFaView 类的接口
//


#pragma once

#define SYS_BUF_SIZE       0x10000
class CFaView : public CView
{
protected: // 仅从序列化创建
	CFaView();
	DECLARE_DYNCREATE(CFaView)

// 属性
public:
	CFaDoc* GetDocument() const;

// 操作
public:
	void DisplayImage();
	afx_msg void OnTimer(UINT nIDEvent);

	afx_msg BOOL PreTranslateMessage(MSG*   pMsg) ;

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
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

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // FaView.cpp 中的调试版本
inline CFaDoc* CFaView::GetDocument() const
   { return reinterpret_cast<CFaDoc*>(m_pDocument); }
#endif

