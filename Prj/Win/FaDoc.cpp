// FaDoc.cpp : CFaDoc ���ʵ��
//

#include "stdafx.h"
#include "Fa.h"

#include "FaDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFaDoc

IMPLEMENT_DYNCREATE(CFaDoc, CDocument)

BEGIN_MESSAGE_MAP(CFaDoc, CDocument)
END_MESSAGE_MAP()


// CFaDoc ����/����

CFaDoc::CFaDoc()
{
	// TODO: �ڴ����һ���Թ������
   
}

CFaDoc::~CFaDoc()
{
}

BOOL CFaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	reinterpret_cast<CEditView*>(m_viewList.GetHead())->SetWindowText(NULL);

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

	return TRUE;
}




// CFaDoc ���л�

void CFaDoc::Serialize(CArchive& ar)
{
	// CEditView ����һ�������������л��ı༭�ؼ�
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->SerializeRaw(ar);
}


// CFaDoc ���

#ifdef _DEBUG
void CFaDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFaDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CFaDoc ����
