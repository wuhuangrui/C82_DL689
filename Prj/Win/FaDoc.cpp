// FaDoc.cpp : CFaDoc 类的实现
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


// CFaDoc 构造/析构

CFaDoc::CFaDoc()
{
	// TODO: 在此添加一次性构造代码
   
}

CFaDoc::~CFaDoc()
{
}

BOOL CFaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	reinterpret_cast<CEditView*>(m_viewList.GetHead())->SetWindowText(NULL);

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CFaDoc 序列化

void CFaDoc::Serialize(CArchive& ar)
{
	// CEditView 包含一个处理所有序列化的编辑控件
	reinterpret_cast<CEditView*>(m_viewList.GetHead())->SerializeRaw(ar);
}


// CFaDoc 诊断

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


// CFaDoc 命令
