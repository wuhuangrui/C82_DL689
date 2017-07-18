// FaView.cpp : CFaView 类的实现
//

#include "stdafx.h"
#include "Fa.h"

#include "FaDoc.h"
#include "FaView.h"
#include "sysdebug.h"
#include "sysarch.h"
#include "Key.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


int g_bkey = 0;
unsigned char m_ucGDRam[160*160/8];
TSem g_SemVmLcd;
int GetKey()
{
	int bKey = g_bkey;
	g_bkey = 0;
	return bKey;
}
int sys_getchar(void)
{
	return GetKey();
}



// CFaView

IMPLEMENT_DYNCREATE(CFaView, CView)

BEGIN_MESSAGE_MAP(CFaView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_MESSAGE(WM_SYS_NOTIFY, OnSysNotify)

	ON_WM_TIMER()

END_MESSAGE_MAP()

// CFaView 构造/析构

CFaView::CFaView()
{
	// TODO: 在此处添加构造代码

}

CFaView::~CFaView()
{
}
void CFaView::OnTimer(UINT nIDEvent)
{
	DisplayImage();
	CView::OnTimer(nIDEvent);
}

static const unsigned char SysBitsPattern[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

void CFaView::DisplayImage()
{
	CClientDC dc(this);
	BYTE bLcdBuf[160*160/8];

	WaitSemaphore(g_SemVmLcd);
	memcpy(bLcdBuf, m_ucGDRam, 160*160/8);
	SignalSemaphore(g_SemVmLcd);


	//if(ImageDisplay != NULL)
	{		
		CPen pen(PS_SOLID,1,RGB(255,255,255));
		dc.SelectObject(pen);

		dc.SetBkColor(RGB(255,255,255));

		for (int i=0; i<160; i++)
		{
			for (BYTE j=0; j<160; j++)
			{
				BYTE pos = j>>3;
				BYTE mask = SysBitsPattern[j&0x07];

				if ((bLcdBuf[i*20+pos]&mask) != 0)
					dc.SetPixel(j, i, RGB(0,0,0));//黑色
				else
					dc.SetPixel(j, i, RGB(255,255,255));//白色
			}
		}
	}
	//dc.MoveTo(i*2+35,Imagex_y(Image_val,DisplayBuffer[i]));
	//dc.LineTo((i+1)*2+35,Imagex_y(Image_val,DisplayBuffer[i+1]));


}

LRESULT CFaView::OnSysNotify(WPARAM wParam, LPARAM lParam)
{
    
	BYTE* p = (BYTE* )wParam;
	DWORD dwLen = (DWORD) lParam;
	if (dwLen + m_dwSysBufPtr >= SYS_BUF_SIZE - 1)
    {
		m_dwSysBufPtr = 0; 
    }
#if 0
	memcpy(&m_szSysBuf[m_dwSysBufPtr], p, dwLen);
    m_dwSysBufPtr += dwLen;
    m_szSysBuf[m_dwSysBufPtr] = 0;

	SetWindowText((const char *)&m_szSysBuf);  
	GetScrollRange(SB_VERT,&nMin,&nMax);   
	SetScrollPos(SB_VERT,nMax);   

	//m_editSys.SetWindowText((const char *)&m_szSysBuf);  
	//m_editSys.LineScroll(9999);  
#endif
    delete [] p; 


	return 1;
}

BOOL CFaView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CCL760A11View 绘制

bool first = true;
void CFaView::OnDraw(CDC* /*pDC*/)
{
	CFaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (first)
	{		
		SetTimer(1, 100, NULL);		
		first = false;
	}
	DisplayImage();
}


// CCL760A11View 打印

BOOL CFaView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CFaView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CFaView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清除过程
}


// CFaView 诊断

#ifdef _DEBUG
void CFaView::AssertValid() const
{
	CView::AssertValid();
}

void CFaView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFaDoc* CFaView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaDoc)));
	return (CFaDoc*)m_pDocument;
}
#endif //_DEBUG


// CFaView 消息处理程序
BOOL CFaView::PreTranslateMessage(MSG*   pMsg)     
{
	if (  pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)   
		{
		case   38:   
			g_bkey = KEYUP;
			break;
		case   40:   
			g_bkey = KEYDOWN;
			break;
		case   37:   
			g_bkey = KEYLEFT;
			break;
		case   39:   
			g_bkey = KEYRIGHT;
			break;
		case   46:   
			g_bkey = KEYEXIT;
			break;
		case   34:   
			g_bkey = KEYOK;
			break;
		default:
			g_bkey = 0;
			break;
		}   
	}
	return   CView::PreTranslateMessage(pMsg);   
}