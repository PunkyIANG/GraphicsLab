
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


int factorialLength;
uint64_t* factorialCache;

const double epsilon = 0.01;
const int gridLineLength = 2;
const bool useFixedLimits = true;

const int gridInterval = 25;

const double a = -3;
const double b = 5;
const double step = 0.1;


double CustomSinX(double x, double eps);
double OtherCustomSinX(double x, double eps);
uint64_t CachedFactorial(int k);

// CGraphicsLabView construction/destruction

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

int NumToCoordX(double x, double a, double b, int width) {
	const int padding = 5;
	return padding + (x - a) / (b - a) * (width - 2 * padding);
}

int NumToCoordY(double y, double min, double max, int height) {
	const int padding = 5;
	return height - (padding + (y - min) / (max - min) * (height - 2 * padding));
}


void CGraphicsLabView::OnDraw(CDC* pDC)
{
	CGraphicsLabDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (useFixedLimits) {
		// get all values
		int valueCount = (int)((b - a) / step) + 1;
		double *customResults = new double[valueCount];
		double *baseResults = new double[valueCount];

		double maxY;
		double minY;


		for (int i = 0; i < valueCount; i++) {
			double x = a + i * step;
			customResults[i] = OtherCustomSinX(x, epsilon);
			baseResults[i] = sin(x) / x;

			double tempMax = max(customResults[i], baseResults[i]);
			double tempMin = min(customResults[i], baseResults[i]);

			if (i == 0) {
				maxY = tempMax;
				minY = tempMin;
			}
			else 
			{
				maxY = max(tempMax, maxY);
				minY = min(tempMin, minY);
			}
		}

		CRect rcClient;
		GetClientRect(rcClient);
		auto center = rcClient.CenterPoint();
		auto width = rcClient.Width();
		auto height = rcClient.Height();

		if (maxY * minY < 0) {
			pDC->MoveTo(0,                NumToCoordY(0, minY, maxY, height));
			pDC->LineTo(rcClient.Width(), NumToCoordY(0, minY, maxY, height));
		}

		if (a * b < 0) {
			pDC->MoveTo(NumToCoordX(0, a, b, width), 0);
			pDC->LineTo(NumToCoordX(0, a, b, width), rcClient.Height());
		}

		// getting floor then ceil to render one more line 
		// most likely out of sight but does well for sin(x)/x
		int firstGridLine = floor(a);
		int lastGridLine = ceil(b);

		for (int i = firstGridLine; i <= lastGridLine; i++) {
			int xPos = NumToCoordX(i, a, b, width);
			if (minY > 0) {
				// draw grid at lower window limit
				pDC->MoveTo(xPos, height - gridLineLength);
				pDC->LineTo(xPos, height + 1);
			}
			else if (maxY < 0) {
				// draw grid at upper window limit
				pDC->MoveTo(xPos, 0);
				pDC->LineTo(xPos, gridLineLength + 1);
			}
			else {
				// draw as usual
				pDC->MoveTo(xPos, NumToCoordY(0, minY, maxY, height) - gridLineLength);
				pDC->LineTo(xPos, NumToCoordY(0, minY, maxY, height) + gridLineLength + 1);
			}
		}

		firstGridLine = floor(minY);
		lastGridLine = ceil(maxY);

		for (int i = firstGridLine; i <= lastGridLine; i++) {
			int yPos = NumToCoordY(i, minY, maxY, height);
			if (a > 0) {
				// draw grid at left window limit
				pDC->MoveTo(0,					yPos);
				pDC->LineTo(gridLineLength + 1, yPos);
			}
			else if (b < 0) {
				// draw grid at right window limit
				pDC->MoveTo(width - gridLineLength, yPos);
				pDC->LineTo(width + 1,				yPos);
			}
			else {
				// draw as usual
				pDC->MoveTo(NumToCoordX(0, a, b, width) - gridLineLength,	  yPos);
				pDC->LineTo(NumToCoordX(0, a, b, width) + gridLineLength + 1, yPos);
			}
		}

		CPen redPen(PS_SOLID, 1, RGB(0, 255, 0));
		pDC->SelectObject(&redPen);

		double prevValue = customResults[0];
		pDC->MoveTo(NumToCoordX(a, a, b, width), NumToCoordY(prevValue, minY, maxY, height));

		for (int i = 0; i < valueCount; i++) {
			pDC->LineTo(NumToCoordX(a + i * step, a, b, width), NumToCoordY(customResults[i], minY, maxY, height));
			pDC->SetPixel(NumToCoordX(a + i * step, a, b, width), NumToCoordY(baseResults[i], minY, maxY, height), RGB(255, 0, 0));

		}

		/*
		for (int i = 0; i < rcClient.Width(); i++) {
			double x = (double)(i - center.x) / gridInterval;

			if (x != 0) {
				double y = OtherCustomSinX(x, epsilon);
				pDC->LineTo(i, -(y * gridInterval) + center.y);

				y = sin(x) / x;
				pDC->SetPixel(i, -(y * gridInterval) + center.y, RGB(255, 0, 0));
			}
		}

		*/
	}
	else {
		CRect rcClient;
		GetClientRect(rcClient);
		auto center = rcClient.CenterPoint();

		pDC->MoveTo(0, center.y);
		pDC->LineTo(rcClient.Width(), center.y);

		pDC->MoveTo(center.x, 0);
		pDC->LineTo(center.x, rcClient.Height());

		int xGridLines = center.x / gridInterval;

		for (int i = 1; i <= xGridLines; i++) {
			pDC->MoveTo(center.x + gridInterval * i, center.y - gridLineLength);
			pDC->LineTo(center.x + gridInterval * i, center.y + gridLineLength + 1);

			pDC->MoveTo(center.x - gridInterval * i, center.y - gridLineLength);
			pDC->LineTo(center.x - gridInterval * i, center.y + gridLineLength + 1);
		}

		int yGridLines = center.y / gridInterval;

		for (int i = 1; i <= yGridLines; i++) {
			pDC->MoveTo(center.x - gridLineLength, center.y + gridInterval * i);
			pDC->LineTo(center.x + gridLineLength + 1, center.y + gridInterval * i);

			pDC->MoveTo(center.x - gridLineLength, center.y - gridInterval * i);
			pDC->LineTo(center.x + gridLineLength + 1, center.y - gridInterval * i);
		}

		CPen redPen(PS_SOLID, 1, RGB(0, 255, 0));
		pDC->SelectObject(&redPen);

		double prevValue = OtherCustomSinX((double)center.x / gridInterval, 0.01);
		pDC->MoveTo(0, -(prevValue * gridInterval) + center.y);

		for (int i = 0; i < rcClient.Width(); i++) {
			double x = (double)(i - center.x) / gridInterval;

			if (x != 0) {
				double y = OtherCustomSinX(x, epsilon);
				pDC->LineTo(i, -(y * gridInterval) + center.y);

				y = sin(x) / x;
				pDC->SetPixel(i, -(y * gridInterval) + center.y, RGB(255, 0, 0));
			}
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
