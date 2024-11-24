#include "framework.h"
#include "iRecHS2s.h"

#define CONWIN 1

#define PALETTECOLORS 256
#define ID_W_CAMERAVIEW_TIMER 10012
#define CAMERAVIEW_INTERAVAL 33
#define FONT_SIZE 14
#define FONT_CHARSET ANSI_CHARSET
#define FONT_TYPEFACE TEXT("Courier New")
#define PALETTECOLORS 256

iRec::W_CameraView::W_CameraView(HWND parent, iRec& ir, int x, int y, int w, int h)
{
	WNDCLASSEX wcex;
	RECT rect;
	int width, height;

	iRecPtr = &ir;
	hWndParent = parent;
	hBmpMem = hOldBmpMem = NULL;
	hBrush = hOldBrush = NULL;
	hCursorCross = hCursorOld = NULL;
	hFont = hOldFont = NULL;
	hPen = hOldPen = NULL;
	hdc_mem = NULL;

	LButtonDown = false;
	start = { 0,0 };
	end = { 0,0 };

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)CallbackProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("W_CameraView");
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);

	rect.top = rect.left = 0;
	winsize.w = rect.right = w;
	winsize.h = rect.bottom = h;

	AdjustWindowRect(&rect, WS_CHILD | WS_BORDER | WS_VISIBLE, FALSE);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
	hWnd = CreateWindow(TEXT("W_CameraView"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		x, y, width, height, hWndParent, NULL, hInst, this);
	PosSize.left = x;
	PosSize.right = x + width;
	PosSize.top = y;
	PosSize.bottom = y + height;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}
iRec::W_CameraView::~W_CameraView()
{

}
LRESULT CALLBACK iRec::W_CameraView::CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	W_CameraView* thisPtr = NULL;

	switch (message) {
	case WM_CREATE:
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
		thisPtr = (W_CameraView*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		break;
	case WM_DESTROY:
		thisPtr = (W_CameraView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		break;
	default:
		thisPtr = (W_CameraView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		break;
	}
	if (thisPtr) {
		return thisPtr->WndProc(hWnd, message, wParam, lParam);
	}
	return (DefWindowProc(hWnd, message, wParam, lParam));

}
void  iRec::W_CameraView::CopyImage(int co)
{
	PBITMAPINFO pbmi;
	int i, rows, cols, pixelbits;
	cols = static_cast<int>(iRecPtr->pEyeImage[co].ci.w);
	rows = static_cast<int>(iRecPtr->pEyeImage[co].ci.h);
	pixelbits = static_cast<int>(iRecPtr->pEyeImage[co].ci.pixelbits);


	if (pixelbits <= 8) {
		pbmi = (PBITMAPINFO) new unsigned char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PALETTECOLORS];
		pbmi->bmiHeader.biClrUsed = (1 << pixelbits);
		for (i = 0; i < PALETTECOLORS; i++) {
			if ((iRecPtr->CameraViewParam.IsUnderThreshold) && (i < iRecPtr->pEyeImage[co].ImgParam.PupilThreshold)) {
				pbmi->bmiColors[i].rgbRed = 0;
				pbmi->bmiColors[i].rgbGreen = 255;
				pbmi->bmiColors[i].rgbBlue = 0;
				pbmi->bmiColors[i].rgbReserved = 0;
			}
			else {
				if ((iRecPtr->CameraViewParam.IsReflection) && (i >= iRecPtr->pEyeImage[co].ImgParam.RefThreshold)) {
					pbmi->bmiColors[i].rgbRed = 255;
					pbmi->bmiColors[i].rgbGreen = 0;
					pbmi->bmiColors[i].rgbBlue = 0;
					pbmi->bmiColors[i].rgbReserved = 0;
				}
				else {
					pbmi->bmiColors[i].rgbRed = i;
					pbmi->bmiColors[i].rgbGreen = i;
					pbmi->bmiColors[i].rgbBlue = i;
					pbmi->bmiColors[i].rgbReserved = 0;
				}
			}
		}
	}
	else {
		pbmi = (PBITMAPINFO) new unsigned char[sizeof(BITMAPINFOHEADER)];
		pbmi->bmiHeader.biClrUsed = 0;
	}
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = cols;
	pbmi->bmiHeader.biHeight = -rows;
	pbmi->bmiHeader.biXPelsPerMeter = 2835;
	pbmi->bmiHeader.biYPelsPerMeter = 2835;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = pixelbits;
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biSizeImage = ((cols * pixelbits + 31) & ~31) / 8 * rows;
	pbmi->bmiHeader.biClrImportant = 0;
	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		StretchDIBits(hdc_mem, cols, 0, -cols, rows, 0, 0, cols, rows, iRecPtr->pEyeImage[co].ci.img, pbmi, DIB_RGB_COLORS, SRCCOPY);
	}
	else {
		StretchDIBits(hdc_mem, 0, 0, cols, rows, 0, 0, cols, rows, iRecPtr->pEyeImage[co].ci.img, pbmi, DIB_RGB_COLORS, SRCCOPY);
	}
	delete[] pbmi;
}
void iRec::W_CameraView::DrawPupilEdge(int co)
{
	int  xi, yi;
	if (iRecPtr->pEyeImage[co].pupil.size.w == 0) return;

	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		for (int i = 0; i < iRecPtr->pEyeImage[co].pe.x_num; i++) {
			if (iRecPtr->pEyeImage[co].pe.Xe[i].errflag.le == false) {
				xi = iRecPtr->pEyeImage[co].ci.w - iRecPtr->pEyeImage[co].pe.Xe[i].le.x;
				yi = iRecPtr->pEyeImage[co].pe.Xe[i].le.y;
				SetPixelV(hdc_mem, xi, yi, RGB(255, 255, 0));
			}
			if (iRecPtr->pEyeImage[co].pe.Xe[i].errflag.re == false) {
				xi = iRecPtr->pEyeImage[co].ci.w - iRecPtr->pEyeImage[co].pe.Xe[i].re.x;
				yi = iRecPtr->pEyeImage[co].pe.Xe[i].re.y;
				SetPixelV(hdc_mem, xi, yi, RGB(255, 255, 0));
			}
		}
		for (int i = 0; i < iRecPtr->pEyeImage[co].pe.y_num; i++) {
			if (iRecPtr->pEyeImage[co].pe.Ye[i].flag == true) {
				xi = iRecPtr->pEyeImage[co].ci.w - iRecPtr->pEyeImage[co].pe.Ye[i].x;
				yi = iRecPtr->pEyeImage[co].pe.Ye[i].y;
				SetPixelV(hdc_mem, xi, yi, RGB(255, 255, 0));
			}
		}
	}
	else {
		for (int i = 0; i < iRecPtr->pEyeImage[co].pe.x_num; i++) {
			if (iRecPtr->pEyeImage[co].pe.Xe[i].errflag.le == false) {
				xi = iRecPtr->pEyeImage[co].pe.Xe[i].le.x;
				yi = iRecPtr->pEyeImage[co].pe.Xe[i].le.y;
				SetPixelV(hdc_mem, xi, yi, RGB(255, 255, 0));
			}
			if (iRecPtr->pEyeImage[co].pe.Xe[i].errflag.re == false) {
				xi = iRecPtr->pEyeImage[co].pe.Xe[i].re.x;
				yi = iRecPtr->pEyeImage[co].pe.Xe[i].re.y;
				SetPixelV(hdc_mem, xi, yi, RGB(255, 255, 0));
			}
		}
		for (int i = 0; i < iRecPtr->pEyeImage[co].pe.y_num; i++) {
			if (iRecPtr->pEyeImage[co].pe.Ye[i].flag == true) {
				xi = iRecPtr->pEyeImage[co].pe.Ye[i].x;
				yi = iRecPtr->pEyeImage[co].pe.Ye[i].y;
				SetPixelV(hdc_mem, xi, yi, RGB(255, 255, 0));
			}
		}
	}

}
void iRec::W_CameraView::DrawROI(int co)
{
	HPEN hPen, hOldPen;
	int ltx, rbx;

	hPen = (HPEN)GetStockObject(WHITE_PEN);
	hOldPen = (HPEN)SelectObject(hdc_mem, hPen);
	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		ltx = iRecPtr->pEyeImage[co].ci.w - iRecPtr->pEyeImage[co].ImgParam.roi.left;
		rbx = iRecPtr->pEyeImage[co].ci.w - iRecPtr->pEyeImage[co].ImgParam.roi.right;
		Rectangle(hdc_mem, ltx, iRecPtr->pEyeImage[co].ImgParam.roi.top, rbx, iRecPtr->pEyeImage[co].ImgParam.roi.bottom);
	}
	else {
		Rectangle(hdc_mem, iRecPtr->pEyeImage[co].ImgParam.roi.left,
			iRecPtr->pEyeImage[co].ImgParam.roi.top,
			iRecPtr->pEyeImage[co].ImgParam.roi.right,
			iRecPtr->pEyeImage[co].ImgParam.roi.bottom);

		//Rectangle(hdc_mem, iRecPtr->pEyeImage[co].pupil.roi.left,
		//	iRecPtr->pEyeImage[co].pupil.roi.top,
		//	iRecPtr->pEyeImage[co].pupil.roi.right,
		//	iRecPtr->pEyeImage[co].pupil.roi.bottom);
	}
	SelectObject(hdc_mem, hOldPen);
	//	DeleteObject(hPen);
}
void iRec::W_CameraView::DrawPupilOutline(int co)
{
	int i, j;
	POINT pt[361];
	double a, b, c, d, e, cx, cy, c1, ang, ansx, ansy;
	HPEN hPen, hOldPen;

	if (iRecPtr->pEyeImage[co].pupil.size.w == 0) return;

	hPen = (HPEN)GetStockObject(WHITE_PEN);
	hOldPen = (HPEN)SelectObject(hdc_mem, hPen);
	cy = iRecPtr->pEyeImage[co].pupil.center.y;
	cx = iRecPtr->pEyeImage[co].pupil.center.x;
	a = iRecPtr->pEyeImage[co].pupil.eq.a;
	b = iRecPtr->pEyeImage[co].pupil.eq.b;
	c = iRecPtr->pEyeImage[co].pupil.eq.c;
	d = iRecPtr->pEyeImage[co].pupil.eq.d;
	e = iRecPtr->pEyeImage[co].pupil.eq.e;

	c1 = cx * cx + a * cx * cy + b * cy * cy + c * cx + d * cy + e;

	pt[0].x = (int)(cx + 0.5);
	pt[0].y = (int)((cy - sqrt(-1 * c1 / b)) + 0.5);
	pt[180].x = pt[0].x;
	pt[180].y = (int)((cy + sqrt(-1 * c1 / b)) + 0.5);

	for (i = -89, j = 1; i < 90; i++, j++) {
		ang = tan((double)i * M_PI / 180.0);
		ansx = sqrt(-1 * c1 / (b * ang * ang + a * ang + 1));
		ansy = ang * ansx;
		pt[j].x = (int)(cx + ansx + 0.5);
		pt[j + 180].x = (int)(cx - ansx + 0.5);
		pt[j].y = (int)(cy + ansy + 0.5);
		pt[j + 180].y = (int)(cy - ansy + 0.5);
	}

	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		for (i = 0; i < 361; i++) {
			pt[i].x = winsize.w - pt[i].x;
		}
	}
	Polygon(hdc_mem, pt, 360);

	//Rectangle(hdc_mem, iRecPtr->pEyeImage[co].pupil.roi.left,
	//	iRecPtr->pEyeImage[co].pupil.roi.top,
	//	iRecPtr->pEyeImage[co].pupil.roi.right,
	//	iRecPtr->pEyeImage[co].pupil.roi.bottom);


	SelectObject(hdc_mem, hOldPen);
}
void iRec::W_CameraView::DrawRadiusLine(int co)
{
	double a, x, y, b, sx, sy, ex, ey, tx, ty, w, h;
	int isx, isy, iex, iey;

	HPEN hPen, hOldPen;

	if (iRecPtr->pEyeImage[co].pupil.size.w == 0) return;

	hPen = (HPEN)GetStockObject(WHITE_PEN);
	hOldPen = (HPEN)SelectObject(hdc_mem, hPen);

	a = iRecPtr->pEyeImage[co].pupil.slope.s;
	x = iRecPtr->pEyeImage[co].pupil.center.x;
	y = iRecPtr->pEyeImage[co].pupil.center.y;
	w = static_cast<double>(iRecPtr->pEyeImage[co].ci.w) - 2;
	h = static_cast<double>(iRecPtr->pEyeImage[co].ci.h) - 2;


	b = y - x * a;
	if (a < 0) {
		if ((b >= 0) && (b < h)) {
			sx = 0; sy = b;
		}
		else {
			sy = h;	sx = (sy - b) / a;
		}
		ey = a * w + b;
		if (ey < 0) {
			ey = 0; ex = (-b) / a;
		}
		else {
			ex = w;
		}
	}
	if (a > 0) {
		if ((b >= 0) && (b < h)) {
			sx = 0; sy = b;
		}
		else {
			sy = 0; sx = (-b) / a;
		}
		ey = a * w + b;
		if (ey > h) {
			ey = h; ex = (ey - b) / a;
		}
		else {
			ex = w;
		}
	}
	if (a == 0) {
		sx = 0;
		ex = w;
		sy = ey = b;
	}
	if (sy > ey) {
		tx = ex; ex = sx; sx = tx;
		ty = ey; ey = sy; sy = ty;
	}

	isx = (int)(sx + 0.5);
	iex = (int)(ex + 0.5);
	isy = (int)(sy + 0.5);
	iey = (int)(ey + 0.5);

	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		isx = (int)w - isx;
		iex = (int)w - iex;
	}

	MoveToEx(hdc_mem, isx, isy, NULL);
	LineTo(hdc_mem, iex, iey);

	SelectObject(hdc_mem, hOldPen);
}
void iRec::W_CameraView::DrawRotationCenter(int co)
{
	int isx, isy, iex, iey,x,y;
	HPEN hPen, hOldPen;

	if (iRecPtr->pEyeImage[co].ImgParam.pc.radius == 0) return;

	hPen = (HPEN)GetStockObject(WHITE_PEN);
	hOldPen = (HPEN)SelectObject(hdc_mem, hPen);

	isx = 0;
	iex = iRecPtr->pEyeImage[co].ci.w;
	isy = 0;
	iey = iRecPtr->pEyeImage[co].ci.h;
	x = static_cast<int>(iRecPtr->pEyeImage[co].ImgParam.pc.ox + 0.5);
	y = static_cast<int>(iRecPtr->pEyeImage[co].ImgParam.pc.oy + 0.5);
	


	MoveToEx(hdc_mem, isx, y, NULL);
	LineTo(hdc_mem, iex, y);

	MoveToEx(hdc_mem, x, isy, NULL);
	LineTo(hdc_mem, x, iey);

	SelectObject(hdc_mem, hOldPen);


}
void iRec::W_CameraView::DrawPCLine(int co)
{
	double a, x, y, b, sx, sy, ex, ey, tx, ty, w, h;
	double dx, dy,cx,cy;
	int isx, isy, iex, iey;

	HPEN hPen, hOldPen;

	if (iRecPtr->pEyeImage[co].pupil.size.w == 0) return;
	if (iRecPtr->pEyeImage[co].ImgParam.pr.pcl == 0) return;


	cx = iRecPtr->pEyeImage[co].ref.pos.x - iRecPtr->pEyeImage[co].ImgParam.pr.offx;
	cy = iRecPtr->pEyeImage[co].ref.pos.y - iRecPtr->pEyeImage[co].ImgParam.pr.offy;


	x = iRecPtr->pEyeImage[co].pupil.center.x;
	y = iRecPtr->pEyeImage[co].pupil.center.y;
	dx = x - cx;
	dy = y - cy;
	if (dx == 0) return;
	a = dy / dx;

	w = static_cast<double>(iRecPtr->pEyeImage[co].ci.w) - 2;
	h = static_cast<double>(iRecPtr->pEyeImage[co].ci.h) - 2;


	b = y - x * a;
	if (a < 0) {
		if ((b >= 0) && (b < h)) {
			sx = 0; sy = b;
		}
		else {
			sy = h;	sx = (sy - b) / a;
		}
		ey = a * w + b;
		if (ey < 0) {
			ey = 0; ex = (-b) / a;
		}
		else {
			ex = w;
		}
	}
	if (a > 0) {
		if ((b >= 0) && (b < h)) {
			sx = 0; sy = b;
		}
		else {
			sy = 0; sx = (-b) / a;
		}
		ey = a * w + b;
		if (ey > h) {
			ey = h; ex = (ey - b) / a;
		}
		else {
			ex = w;
		}
	}
	if (a == 0) {
		sx = 0;
		ex = w;
		sy = ey = b;
	}
	if (sy > ey) {
		tx = ex; ex = sx; sx = tx;
		ty = ey; ey = sy; sy = ty;
	}

	isx = (int)(sx + 0.5);
	iex = (int)(ex + 0.5);
	isy = (int)(sy + 0.5);
	iey = (int)(ey + 0.5);

	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		isx = (int)w - isx;
		iex = (int)w - iex;
	}

	hPen = (HPEN)GetStockObject(WHITE_PEN);
	hOldPen = (HPEN)SelectObject(hdc_mem, hPen);
	MoveToEx(hdc_mem, isx, isy, NULL);
	LineTo(hdc_mem, iex, iey);
	SelectObject(hdc_mem, hOldPen);
}
#define CROSSSIZE 3
void iRec::W_CameraView::DrawPupilCenter(int co)
{
	int icx, icy, sx, ex, sy, ey;
	HPEN hPen, hOldPen;

	if (iRecPtr->pEyeImage[co].pupil.size.w == 0) return;

	hPen = (HPEN)GetStockObject(WHITE_PEN);
	hOldPen = (HPEN)SelectObject(hdc_mem, hPen);

	icx = (int)(iRecPtr->pEyeImage[co].pupil.center.x + 0.5);
	icy = (int)(iRecPtr->pEyeImage[co].pupil.center.y + 0.5);


	sx = (icx - CROSSSIZE < 1) ? 1 : icx - CROSSSIZE;
	sy = (icy - CROSSSIZE < 1) ? 1 : icy - CROSSSIZE;
	ex = (icx + CROSSSIZE + 1 > iRecPtr->pEyeImage[co].ci.w - 2) ? iRecPtr->pEyeImage[co].ci.w - 2 : icx + CROSSSIZE + 1;
	ey = (icy + CROSSSIZE + 1 > iRecPtr->pEyeImage[co].ci.h - 2) ? iRecPtr->pEyeImage[co].ci.h - 2 : icy + CROSSSIZE + 1;

	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		sx = iRecPtr->pEyeImage[co].ci.w - sx;
		ex = iRecPtr->pEyeImage[co].ci.w - ex;
		icx = iRecPtr->pEyeImage[co].ci.w - icx;
	}
	MoveToEx(hdc_mem, icx, sy, NULL);
	LineTo(hdc_mem, icx, ey);
	MoveToEx(hdc_mem, sx, icy, NULL);
	LineTo(hdc_mem, ex, icy);
	SelectObject(hdc_mem, hOldPen);

}


void iRec::W_CameraView::DrawReflection(int co)
{
	int icx, icy, sx, ex, sy, ey;

	HPEN hPen, hOldPen;

	if (iRecPtr->pEyeImage[co].pupil.size.w == 0) return;


	hPen = (HPEN)GetStockObject(BLACK_PEN);
	hOldPen = (HPEN)SelectObject(hdc_mem, hPen);

	icx = (int)(iRecPtr->pEyeImage[co].ref.pos.x + 0.5);
	icy = (int)(iRecPtr->pEyeImage[co].ref.pos.y + 0.5);

	sx = (icx - CROSSSIZE < 1) ? 1 : icx - CROSSSIZE;
	sy = (icy - CROSSSIZE < 1) ? 1 : icy - CROSSSIZE;
	ex = (icx + CROSSSIZE + 1 > iRecPtr->pEyeImage[co].ci.w - 2) ? iRecPtr->pEyeImage[co].ci.w - 2 : icx + CROSSSIZE + 1;
	ey = (icy + CROSSSIZE + 1 > iRecPtr->pEyeImage[co].ci.h - 2) ? iRecPtr->pEyeImage[co].ci.h - 2 : icy + CROSSSIZE + 1;

	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		sx = iRecPtr->pEyeImage[co].ci.w - sx;
		ex = iRecPtr->pEyeImage[co].ci.w - ex;
		icx = iRecPtr->pEyeImage[co].ci.w - icx;
	}
	MoveToEx(hdc_mem, icx, sy, NULL);
	LineTo(hdc_mem, icx, ey);
	MoveToEx(hdc_mem, sx, icy, NULL);
	LineTo(hdc_mem, ex, icy);


	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		ex = iRecPtr->pEyeImage[co].ci.w - static_cast<int>(iRecPtr->pEyeImage[co].ref.range.left) + 1;
		sx = iRecPtr->pEyeImage[co].ci.w - static_cast<int>(iRecPtr->pEyeImage[co].ref.range.right);
		sy = static_cast<int>(iRecPtr->pEyeImage[co].ref.range.top);
		ey = static_cast<int>(iRecPtr->pEyeImage[co].ref.range.bottom) + 1;
		if (sx < 1) sx = 1;
		if (ex > iRecPtr->pEyeImage[co].ci.w - 2) ex = iRecPtr->pEyeImage[co].ci.w - 2;
		if (sy < 1) sy = 1;
		if (ey > iRecPtr->pEyeImage[co].ci.h - 2) ex = iRecPtr->pEyeImage[co].ci.h - 2;


		Rectangle(hdc_mem, sx, sy, ex, ey);
	}
	else {
		sx = static_cast<int>(iRecPtr->pEyeImage[co].ref.range.left);
		ex = static_cast<int>(iRecPtr->pEyeImage[co].ref.range.right) + 1;
		sy = static_cast<int>(iRecPtr->pEyeImage[co].ref.range.top);
		ey = static_cast<int>(iRecPtr->pEyeImage[co].ref.range.bottom) + 1;
		if (sx < 1) sx = 1;
		if (ex > iRecPtr->pEyeImage[co].ci.w - 2) ex = iRecPtr->pEyeImage[co].ci.w - 2;
		if (sy < 1) sy = 1;
		if (ey > iRecPtr->pEyeImage[co].ci.h - 2) ex = iRecPtr->pEyeImage[co].ci.h - 2;
		Rectangle(hdc_mem, sx, sy, ex, ey);
	}

	SelectObject(hdc_mem, hOldPen);
}

void iRec::W_CameraView::DrawInformation(int co)
{
	static int coprev = -1;

	int hh, mm;
	double ss;
	double ti;

	TEXTMETRIC tm;
	RECT rc;
	if (coprev < 0) {
		coprev = co;
		return;
	}

	if (coprev == co) return;

	GetTextMetrics(hdc_mem, &tm);
	rc.left = 0;
	rc.right = iRecPtr->pEyeImage[co].ci.w - 5;
	rc.bottom = 10 + tm.tmHeight / 2;
	rc.top = rc.bottom - tm.tmHeight;

	ti = static_cast<double>(iRecPtr->pEyeImage[co].ci.TimeStamp - iRecPtr->CAM_StartTimeStamp) / 1.0e7;
	if (ti > 0) {
		hh = static_cast<int>(ti) / 360000;
		mm = (static_cast<int>(ti) - hh * 360000) / 6000;
		ss = static_cast<double>(ti - static_cast<double>(hh) * 360000 - static_cast<double>(mm) * 6000) / 100.0;
		std::wstring ws = (boost::wformat(L"%02d:%02d:%05.2f") % hh % mm % ss).str();
		DrawText(hdc_mem, ws.c_str(), -1, &rc, DT_RIGHT | DT_SINGLELINE);
	}

	rc.bottom = rc.bottom + tm.tmHeight;
	rc.top = rc.bottom - tm.tmHeight;
	double diff_time = static_cast<double>(iRecPtr->pEyeImage[co].ci.TimeStamp - iRecPtr->pEyeImage[coprev].ci.TimeStamp);
	double diff_num = static_cast<double>(iRecPtr->pEyeImage[co].ci.CaptureCounter - iRecPtr->pEyeImage[coprev].ci.CaptureCounter);
	if ((diff_time > 0) && (diff_num > 0)) {
		double freq = diff_num * 1.0e9 / diff_time;
		std::wstring ws = (boost::wformat(L"%6.2f[Hz]") % freq).str();
		DrawText(hdc_mem, ws.c_str(), -1, &rc, DT_RIGHT | DT_SINGLELINE);
	}

	coprev = co;
}

bool iRec::W_CameraView::TimerWatch(int co)
{
	static int coprev = -1;
	static LARGE_INTEGER PrevCPU_FreqCount;
	LARGE_INTEGER CurrentCPU_FreqCount;
	bool ret = false;
	if (coprev < 0) {
		coprev = co;
		QueryPerformanceCounter(&PrevCPU_FreqCount);
		return ret;
	}
	QueryPerformanceCounter(&CurrentCPU_FreqCount);
	if (co == coprev) {
		double diff_num = static_cast<double>(iRecPtr->pEyeImage[co].ci.FrameID - iRecPtr->pEyeImage[coprev].ci.FrameID);
#ifdef CONWIN
		std::wstringstream  wss;
		wss << std::endl;
		wss << (boost::wformat(L"co    :%d ") % co) << endl;
		wss << (boost::wformat(L"cop   :%d ") % coprev) << endl;
		wss << (boost::wformat(L"co  id:%d ") % iRecPtr->pEyeImage[co].ci.FrameID) << endl;
		wss << (boost::wformat(L"cop id:%d ") % iRecPtr->pEyeImage[coprev].ci.FrameID) << endl;
		wss << (boost::wformat(L"%d diff_num") % diff_num) << endl;
		double aa = static_cast<double>(CurrentCPU_FreqCount.QuadPart - PrevCPU_FreqCount.QuadPart);
		double bb = static_cast<double>(iRecPtr->CPU_Freq.QuadPart);
		double cc = aa / bb;
		wss << (boost::wformat(L"%6.3f Sec") % cc) << endl;
		conwin.write(wss.str());
#endif
		ret = true;
	}

	coprev = co;
	PrevCPU_FreqCount = CurrentCPU_FreqCount;
	return ret;
}

bool iRec::W_CameraView::SetROI()
{
	RECT roi;
	LONG tmp;
	if (abs(start.x - end.x) < 32) return false;
	if (abs(start.y - end.y) < 32) return false;

	if (start.x < end.x) {
		roi.left = start.x;
		roi.right = end.x;
	}
	else {
		roi.left = end.x;
		roi.right = start.x;
	}

	if (start.y < end.y) {
		roi.top = start.y;
		roi.bottom = end.y;
	}
	else {
		roi.top = end.y;
		roi.bottom = start.y;
	}
	if (roi.left < 1) roi.left = 1;
	if (roi.right >= winsize.w - 1) roi.right = winsize.w - 2;
	if (roi.top < 1) roi.top = 1;
	if (roi.bottom >= winsize.h - 1) roi.bottom = winsize.h - 2;


	if (iRecPtr->CameraViewParam.IsMirrorImage) {
		tmp = roi.left;
		roi.left = winsize.w - roi.right;
		roi.right = winsize.w - tmp;
	}
	{
		std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
		iRecPtr->ImgProcParam.roi = roi;
	}
	return true;
}

void iRec::W_CameraView::ResizeWindow(int co)
{
	int w, h;
	RECT rect;
	w = iRecPtr->pEyeImage[co].ci.w;
	h = iRecPtr->pEyeImage[co].ci.h;
	if ((w != winsize.w) || (h != winsize.h)) {
		rect.top = rect.left = 0;
		winsize.w = rect.right = w;
		winsize.h = rect.bottom = h;
		AdjustWindowRect(&rect, WS_CHILD | WS_BORDER | WS_VISIBLE, FALSE);
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
		MoveWindow(hWnd, 0, 0, w, h, true);
		
		iRecPtr->wTimeLine->Resize();
		iRecPtr->wXYGraph->Resize();
	}
}

LRESULT   iRec::W_CameraView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message) {

	case WM_CREATE:
		hdc = GetDC(hWnd);
		hdc_mem = CreateCompatibleDC(hdc);
		SetStretchBltMode(hdc_mem, COLORONCOLOR);
		hBmpMem = CreateCompatibleBitmap(hdc, iRecPtr->cam.MaxWidth(), iRecPtr->cam.MaxHeight());
		hOldBmpMem = (HBITMAP)SelectObject(hdc_mem, hBmpMem);
		hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		hOldBrush = (HBRUSH)SelectObject(hdc_mem, hBrush);
		SetDCPenColor(hdc_mem, RGB(255, 255, 255));
		hPen = (HPEN)GetStockObject(DC_PEN);
		hOldPen = (HPEN)SelectObject(hdc_mem, hPen);
		hFont = CreateFont(FONT_SIZE, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, FONT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FONT_TYPEFACE);
		hOldFont = (HFONT)SelectObject(hdc_mem, hFont);
		SetBkMode(hdc_mem, TRANSPARENT);
		SetTextColor(hdc_mem, RGB(255, 255, 255));
		hCursorCross = LoadCursor(NULL, IDC_CROSS);
		ReleaseDC(hWnd, hdc);
		SetTimer(hWnd, ID_W_CAMERAVIEW_TIMER, CAMERAVIEW_INTERAVAL, NULL);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_TIMER:
		if (wParam != ID_W_CAMERAVIEW_TIMER) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		{
			int iCounter;

			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iCounter = static_cast<int>(iRecPtr->FrameCounter % iRecPtr->EyeImageNum);
			}
//			TimerWatch(iCounter);

			CopyImage(iCounter);
			if (iRecPtr->CameraViewParam.IsPupilOutline) {
				DrawPupilOutline(iCounter);
			}
			if (iRecPtr->CameraViewParam.IsRoi) {
				DrawROI(iCounter);
			}
			if (iRecPtr->CameraViewParam.IsPupilEdge) {
				DrawPupilEdge(iCounter);
			}
			if (iRecPtr->CameraViewParam.IsPupilCenter) {
				DrawPupilCenter(iCounter);
			}
			if (iRecPtr->CameraViewParam.IsMinorAxis) {
				DrawRadiusLine(iCounter);
				DrawPCLine(iCounter);
				DrawRotationCenter(iCounter);
			}
			if (iRecPtr->CameraViewParam.IsReflection) {
				DrawReflection(iCounter);
			}
			if (iRecPtr->CameraViewParam.IsInformation) {
				DrawInformation(iCounter);
			}
			ResizeWindow(iCounter);
		}
		if (LButtonDown) Rectangle(hdc_mem, start.x, start.y, end.x, end.y);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONDOWN:
		LButtonDown = true;
		hCursorOld = SetCursor(hCursorCross);
		start = end = MAKEPOINTS(lParam);
		SetCapture(hWnd);
		break;
	case WM_RBUTTONDOWN:

		break;

	case WM_LBUTTONUP:
		if (LButtonDown) {
			LButtonDown = false;
			ReleaseCapture();
			SetCursor(hCursorOld);
			SetROI();
		}
		break;
	case WM_RBUTTONUP:

		break;
	case WM_MOUSEMOVE:
		if (LButtonDown) {
			end = MAKEPOINTS(lParam);
			break;
		}
		break;

	case WM_MOUSEWHEEL:
		if (LOWORD(wParam) == MK_CONTROL) {
			if (HIWORD(wParam) < 0x8000) {
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					if (iRecPtr->ImgProcParam.RefThreshold < 255) {
						iRecPtr->ImgProcParam.RefThreshold++;
					}

				}
			}
			else {
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					if (iRecPtr->ImgProcParam.RefThreshold > 0) {
						iRecPtr->ImgProcParam.RefThreshold--;
					}

				}
			}
		}
		if (LOWORD(wParam) == MK_SHIFT) {
			if (HIWORD(wParam) < 0x8000) {
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					if (iRecPtr->ImgProcParam.PupilThreshold < 255) {
						iRecPtr->ImgProcParam.PupilThreshold++;
					}

				}
			}
			else {
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					if (iRecPtr->ImgProcParam.PupilThreshold > 0) {
						iRecPtr->ImgProcParam.PupilThreshold--;
					}

				}
			}
		}





		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, winsize.w, winsize.h, hdc_mem, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);

		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, ID_W_CAMERAVIEW_TIMER);

		SelectObject(hdc_mem, hOldBmpMem);
		SelectObject(hdc_mem, hOldBrush);
		SelectObject(hdc_mem, hOldPen);
		SelectObject(hdc_mem, hOldFont);

		DeleteObject(hBmpMem);
		DeleteObject(hFont);
		DeleteObject(hCursorCross);

		DeleteDC(hdc_mem);

		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return false;
}
