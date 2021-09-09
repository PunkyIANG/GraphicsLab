
// GraphicsLabView.cpp : implementation of the CGraphicsLabView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "GraphicsLab.h"
#endif

#include "GraphicsLabDoc.h"
#include "GraphicsLabView.h"

#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGraphicsLabView

IMPLEMENT_DYNCREATE(CGraphicsLabView, CView)

BEGIN_MESSAGE_MAP(CGraphicsLabView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CGraphicsLabView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CGraphicsLabView construction/destruction

int factorialLength;
uint64_t* factorialCache;

double CustomSinX(double x, double eps);
double OtherCustomSinX(double x, double eps);
uint64_t CachedFactorial(int k);

CGraphicsLabView::CGraphicsLabView() noexcept
{
	// TODO: add construction code here
	factorialCache = new uint64_t[4]{
		1, 6, 120, 5040
	};

	factorialLength = 4;
}

CGraphicsLabView::~CGraphicsLabView()
{
	delete[] factorialCache;
}

BOOL CGraphicsLabView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CGraphicsLabView drawing

void CGraphicsLabView::OnDraw(CDC* pDC)
{
	CGraphicsLabDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect rcClient;
	GetClientRect(rcClient);
	auto center = rcClient.CenterPoint();

	pDC->MoveTo(0, center.y);
	pDC->LineTo(rcClient.Width(), center.y);

	pDC->MoveTo(center.x, 0);
	pDC->LineTo(center.x, rcClient.Height());

	int gridInterval = 25;
	int gridLength = 2;

	int xGridLines = center.x / 25;

	for (int i = 1; i <= xGridLines; i++) {
		pDC->MoveTo(center.x + gridInterval * i, center.y - gridLength);
		pDC->LineTo(center.x + gridInterval * i, center.y + gridLength + 1);

		pDC->MoveTo(center.x - gridInterval * i, center.y - gridLength);
		pDC->LineTo(center.x - gridInterval * i, center.y + gridLength + 1);
	}

	int yGridLines = center.y / 25;

	for (int i = 1; i <= yGridLines; i++) {
		pDC->MoveTo(center.x - gridLength, center.y + gridInterval * i);
		pDC->LineTo(center.x + gridLength + 1, center.y + gridInterval * i);

		pDC->MoveTo(center.x - gridLength, center.y - gridInterval * i);
		pDC->LineTo(center.x + gridLength + 1, center.y - gridInterval * i);
	}

	for (int i = 0; i < rcClient.Width(); i++) {
		double x = (double)(i - center.x) / gridInterval;

		if (x != 0) {
			double y = OtherCustomSinX(x, 0.01);
			pDC->SetPixel(i, -(y * gridInterval) + center.y, RGB(255, 0, 0));

			// TODO: change rendering method to use LineTo
			y = sin(x) / x;
			pDC->SetPixel(i, -(y * gridInterval) + center.y, RGB(0, 255, 0));
		}
	}
}


double CustomSinX(double x, double eps) {
	if (x == 0)
		return 0;

	double cycleRes, sumRes = 0;
	int k = 1;
	uint64_t factorial = 1;

	do {
		cycleRes = pow(x, 2 * k - 2);

		if (k != 1) {
			factorial *= (2 * k - 2) * (2 * k - 1);
			cycleRes /= factorial;
		}

		// cycleRes /= CachedFactorial(k);

		// (-1)^(k-1)
		if ((k & 1) == 0)
			cycleRes *= -1;

		sumRes += cycleRes;
		k++;
	} while (abs(cycleRes) > eps);

	return sumRes;
}

double OtherCustomSinX(double x, double eps)
{
	if (x == 0)
		return 0;

	double cycleRes, oldCycleRes, sumRes = 0;
	int k = 1;

	do
	{
		// inits the successive calculations process
		if (k == 1)
			cycleRes = pow(x, 2 * k - 2);
		else
			cycleRes = -oldCycleRes * (x * x) / ((2 * k - 2) * (2 * k - 1));

		oldCycleRes = cycleRes;
		sumRes += cycleRes;
		k++;
	} while (abs(cycleRes) > eps);

	return sumRes;
}

/// <summary>
/// Cached factorial compacted for the series of sin(x)/x formula.
/// Be aware as it overflows at k > 10
/// </summary>
/// <param name="k">The k of (2k - 1) from the formula, k > 0</param>
uint64_t CachedFactorial(int k) {
	int index = k - 1;


	if (index >= factorialLength) {
		// extend factorialCache array
		// TODO: don't just double the array, extend it until size > k
		int newSize = factorialLength;

		// bit shift until size is bigger, thus we always get a 2^n size
		while (newSize <= index)
			newSize <<= 1;

		uint64_t* temp = new uint64_t[newSize];
		memcpy(temp, factorialCache, factorialLength * sizeof(uint64_t));
		delete[] factorialCache;
		factorialCache = temp;

		// calculate factorials
		for (int i = factorialLength; i < newSize; i++) {
			// factorialCache[i] = factorialCache[i - 1] * (2 * i - 2) * (2 * i - 1);
			// move it one unit left

			factorialCache[i] = factorialCache[i - 1] * (4 * i * i + 2 * i);
		}

		factorialLength = newSize;
	}

	return factorialCache[index];
}


// CGraphicsLabView printing


void CGraphicsLabView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CGraphicsLabView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CGraphicsLabView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CGraphicsLabView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CGraphicsLabView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CGraphicsLabView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CGraphicsLabView diagnostics

#ifdef _DEBUG
void CGraphicsLabView::AssertValid() const
{
	CView::AssertValid();
}

void CGraphicsLabView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGraphicsLabDoc* CGraphicsLabView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGraphicsLabDoc)));
	return (CGraphicsLabDoc*)m_pDocument;
}
#endif //_DEBUG


// CGraphicsLabView message handlers
