// FaDoc.h : CFaDoc ��Ľӿ�
//


#pragma once


class CFaDoc : public CDocument
{
protected: // �������л�����
	CFaDoc();
	DECLARE_DYNCREATE(CFaDoc)

// ����
public:

// ����
public:

// ��д
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// ʵ��
public:
	virtual ~CFaDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
};


