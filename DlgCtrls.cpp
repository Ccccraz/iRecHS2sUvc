#include "framework.h"
#include "DlgCtrls.h"

using namespace boost;
using namespace std;

//errno_t SelectCamera::a2t(char* src, TCHAR* des, rsize_t num)
//{
//	return MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, src, (int)num, des, (int)num);
//}
//
//errno_t SelectCamera::t2a(TCHAR* src, char* des, size_t num)
//{
//	return WideCharToMultiByte(CP_THREAD_ACP, 0, src, (int)num, des, (int)num, NULL, NULL);
//}

DlgCtrl::DlgCtrl(HWND hDlg, int DlgItem)
{
	RECT rc;
	POINT pt;
	hWndParent = hDlg;
	id = DlgItem;
	hWnd = GetDlgItem(hDlg, id);
	GetWindowRect(hWnd, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
	ScreenToClient(hWndParent, &pt);
	xpos = pt.x;
	ypos = pt.y;
}
DlgCtrl::~DlgCtrl()
{

}
void DlgCtrl::move(int x, int y)
{
	xpos = x;
	ypos = y;
	MoveWindow(hWnd, x, y, width, height, true);
}
void DlgCtrl::movex(int x)
{
	xpos = x;
	MoveWindow(hWnd, xpos, ypos, width, height, true);
}

void DlgCtrl::movey(int y)
{
	ypos = y;
	MoveWindow(hWnd, xpos, ypos, width, height, true);
}

void DlgCtrl::size(int w, int h)
{
	width = w;
	height = h;
	MoveWindow(hWnd, xpos, ypos, w, h, true);
}
void DlgCtrl::enable(void)
{
	EnableWindow(hWnd, true);
}
void DlgCtrl::disable(void)
{
	EnableWindow(hWnd, false);
}
BOOL DlgCtrl::IsEnabled(void)
{
	return IsWindowEnabled(hWnd);
}

DlgListBox::DlgListBox(HWND hDlg, int DlgItem):DlgCtrl(hDlg,DlgItem)
{
	ListBox_ResetContent(hWnd);
}
DlgListBox::~DlgListBox()
{
	ListBox_ResetContent(hWnd);
}
bool DlgListBox::add(std::wstring ws)
{
	return ListBox_AddString(hWnd, ws.c_str());
}
bool DlgListBox::Set(int num)
{
	return ListBox_SetCurSel(hWnd, num);
}
int DlgListBox::Get(void)
{
	return ListBox_GetCurSel(hWnd);
}

DlgButton::DlgButton(HWND hDlg, int DlgItem) :DlgCtrl(hDlg,DlgItem)
{

}

DlgBitmap::DlgBitmap(HWND hDlg, int DlgItem, int w, int h):DlgCtrl(hDlg,DlgItem)
{
	HDC hdc;
	int wh;
	bmwidth = w;
	bmheight = h;

	hdc = GetDC(hDlg);
	SetStretchBltMode(hdc, COLORONCOLOR);
	hBmp = CreateCompatibleBitmap(hdc, w, h);
	hdc_temp = (HDC)CreateCompatibleDC(hdc);
	ReleaseDC(hDlg, hdc);

	hBmpOrig = (HBITMAP)SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
	MoveWindow(hWnd, 0, 0, w, h, true);
	
	xpos = 0;
	ypos = 0;
	width = w;
	height = h;
	wh = w * h;
	if (img != NULL) {
		delete[]img;
	}
	img = new unsigned char[wh]();
}
DlgBitmap::~DlgBitmap()
{
	SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpOrig);
	DeleteObject(hBmp);
	DeleteDC(hdc_temp);
	if (img != NULL) {
		delete[]img;
	}
	img = NULL;
}
void DlgBitmap::fill(DWORD rop)
{
	HBITMAP hBmp_temp;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);
	PatBlt(hdc_temp, 0, 0, width, height, rop);
	SelectObject(hdc_temp, hBmp_temp);
}
void DlgBitmap::line(int sx, int sy, int ex, int ey, COLORREF c,int width=1)
{
	HBITMAP hBmp_temp;
	HPEN hPen_temp, hPen;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);

	hPen = CreatePen(PS_SOLID, width, c);
	hPen_temp = (HPEN)SelectObject(hdc_temp, hPen);

	MoveToEx(hdc_temp, sx, sy, NULL);
	LineTo(hdc_temp, ex, ey);

	SelectObject(hdc_temp, hPen_temp);
	SelectObject(hdc_temp, hBmp_temp);

	DeleteObject(hPen);
}

void DlgBitmap::dotline(int sx, int sy, int ex, int ey, COLORREF c)
{
	HBITMAP hBmp_temp;
	HPEN hPen_temp, hPen;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);

	hPen = CreatePen(PS_DOT, 1, c);
	hPen_temp = (HPEN)SelectObject(hdc_temp, hPen);

	MoveToEx(hdc_temp, sx, sy, NULL);
	LineTo(hdc_temp, ex, ey);

	SelectObject(hdc_temp, hPen_temp);
	SelectObject(hdc_temp, hBmp_temp);

	DeleteObject(hPen);
}
#define GRAYMIN 32
void DlgBitmap::grey_plot(int x1, int y1, int x2, int y2)
{

	HBITMAP hBmp_temp;
	int dx, dy, x, y, e, gray;
	COLORREF col;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);

	if (abs(x1 - x2) >= abs(y1 - y2)) {
		if (x1 > x2) {
			x = x1; x1 = x2; x2 = x;
			y = y1; y1 = y2; y2 = y;
		}
		dx = x2 - x1;
		dy = abs(y2 - y1);
		e = dy;
		col = GetPixel(hdc_temp, x1, y1);
		gray = GetRValue(col);

		if (gray < GRAYMIN) gray = GRAYMIN;
		else gray++;
		if (gray > 255) gray = 255;
		SetPixel(hdc_temp, x1, y1, RGB(gray, gray, gray));
		for (x = x1 + 1, y = y1; x < x2; x++, e += 2 * dy) {
			if (e >= 2 * dx) {
				y += (y2 - y1) / dy;
				e -= 2 * dx;
			}
			col = GetPixel(hdc_temp, x, y);
			gray = GetRValue(col);
			if (gray < GRAYMIN) gray = GRAYMIN;
			else gray++;
			if (gray > 255) gray = 255;

			SetPixel(hdc_temp, x, y, RGB(gray, gray, gray));
		}
	}
	else {
		if (y1 > y2) {
			x = x1; x1 = x2; x2 = x;
			y = y1; y1 = y2; y2 = y;
		}
		dx = abs(x2 - x1);
		dy = y2 - y1;
		e = dx;
		col = GetPixel(hdc_temp, x1, y1);
		gray = GetRValue(col);
		if (gray < GRAYMIN) gray = GRAYMIN;
		else gray++;
		if (gray > 255) gray = 255;
		SetPixel(hdc_temp, x1, y1, RGB(gray, gray, gray));
		for (y = y1 + 1, x = x1; y < y2; y++, e += 2 * dx) {
			if (e >= 2 * dy) {
				x += (x2 - x1) / dx;
				e -= 2 * dy;
			}
			col = GetPixel(hdc_temp, x, y);
			gray = GetRValue(col);
			if (gray < GRAYMIN) gray = GRAYMIN;
			else gray++;

			if (gray > 255) gray = 255;
			SetPixel(hdc_temp, x, y, RGB(gray, gray, gray));
		}
	}
	SelectObject(hdc_temp, hBmp_temp);
}

void DlgBitmap::grey_plot_img(int x1, int y1, int x2, int y2)
{
	int dx, dy, x, y, e;
	unsigned char gray;

	if (abs(x1 - x2) >= abs(y1 - y2)) {
		if (x1 > x2) {
			x = x1; x1 = x2; x2 = x;
			y = y1; y1 = y2; y2 = y;
		}
		dx = x2 - x1;
		dy = abs(y2 - y1);
		e = dy;
		if ((x1 >= 0) && (x1 < bmwidth) && (y1 >= 0) && (y1 < bmheight)) {
			gray = img[x1 + y1 * bmwidth];
			if (gray < GRAYMIN) gray = GRAYMIN;
			else {
				if (gray < 255) gray++;
			}
			img[x1 + y1 * bmwidth] = gray;
		}

		for (x = x1 + 1, y = y1; x < x2; x++, e += 2 * dy) {
			if (e >= 2 * dx) {
				y += (y2 - y1) / dy;
				e -= 2 * dx;
			}
			if ((x >= 0) && (x < bmwidth) && (y >= 0) && (y < bmheight)) {
				gray = img[x + y * bmwidth];
				if (gray < GRAYMIN) gray = GRAYMIN;
				else {
					if (gray < 255) gray++;
				}
				img[x + y * bmwidth] = gray;
			}
		}
	}
	else {
		if (y1 > y2) {
			x = x1; x1 = x2; x2 = x;
			y = y1; y1 = y2; y2 = y;
		}
		dx = abs(x2 - x1);
		dy = y2 - y1;
		e = dx;
		if ((x1 >= 0) && (x1 < bmwidth) && (y1 >= 0) && (y1 < bmheight)) {
			gray = img[x1 + y1 * bmwidth];
			if (gray < GRAYMIN) gray = GRAYMIN;
			else {
				if (gray < 255) gray++;
			}
			img[x1 + y1 * bmwidth] = gray;
		}
		for (y = y1 + 1, x = x1; y < y2; y++, e += 2 * dx) {
			if (e >= 2 * dy) {
				x += (x2 - x1) / dx;
				e -= 2 * dy;
			}
			if ((x >= 0) && (x < bmwidth) && (y >= 0) && (y < bmheight)) {
				gray = img[x + y * bmwidth];
				if (gray < GRAYMIN) gray = GRAYMIN;
				else {
					if (gray < 255) gray++;
				}
				img[x + y * bmwidth] = gray;
			}
		}
	}
}
void DlgBitmap::calc_path(double x, double  y, double a, int& isx, int& isy, int& iex, int& iey)
{
	double b, tx, ty;
	double sx, ex, sy, ey;

	b = y - x * a;
	if (a < 0) {
		if ((b >= 0) && (b < bmheight)) {
			sx = 0; sy = b;
		}
		else {
			sy = bmheight;	sx = (sy - b) / a;
		}
		ey = a * bmwidth + b;
		if (ey < 0) {
			ey = 0; ex = (-b) / a;
		}
		else {
			ex = bmwidth;
		}
	}
	if (a > 0) {
		if ((b >= 0) && (b < bmheight)) {
			sx = 0; sy = b;
		}
		else {
			sy = 0; sx = (-b) / a;
		}
		ey = a * bmwidth + b;
		if (ey > bmheight) {
			ey = bmheight; ex = (ey - b) / a;
		}
		else {
			ex = bmwidth;
		}
	}
	if (a == 0) {
		sx = 0;
		ex = bmwidth;
		sy = ey = b;
	}
	if (sy > ey) {
		tx = ex; ex = sx; sx = tx;
		ty = ey; ey = sy; sy = ty;
	}

	isx = (int)sx;
	iex = (int)ex;
	isy = (int)sy;
	iey = (int)ey;
}

void DlgBitmap::grey_plot(double x, double  y, double a)
{

	int isx, iex, isy, iey;
	calc_path(x, y, a, isx, isy, iex, iey);
	grey_plot(isx, isy, iex, iey);

}

void DlgBitmap::grey_plot_img(double x, double  y, double a)
{
	int isx, iex, isy, iey;

	calc_path(x, y, a, isx, isy, iex, iey);
	grey_plot_img(isx, isy, iex, iey);

}


void DlgBitmap::circle(int x, int y, int r, COLORREF c)
{

	HBITMAP hBmp_temp;
	HPEN hPen_temp, hPen;
	HBRUSH hBrush_temp, hBrush;
	int sx, sy, ex, ey;


	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);

	hPen = CreatePen(PS_SOLID, 1, c);
	hPen_temp = (HPEN)SelectObject(hdc_temp, hPen);

	hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	hBrush_temp= (HBRUSH)SelectObject(hdc_temp, hBrush);

	sx = x - r;
	sy = y - r;
	ex = x + r;
	ey = y + r;

	Ellipse(hdc_temp, sx, sy, ex, ey);

	SelectObject(hdc_temp, hPen_temp);
	SelectObject(hdc_temp, hBrush_temp);
	SelectObject(hdc_temp, hBmp_temp);
	DeleteObject(hPen);

}
void DlgBitmap::rect(int x, int y, int w, int h, COLORREF c)
{
	
	HBITMAP hBmp_temp;
	HPEN hPen_temp, hPen;
	HBRUSH hBrush_temp, hBrush;
	int sx, sy, ex, ey;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);

	hPen = CreatePen(PS_SOLID, 1, c);
	hPen_temp = (HPEN)SelectObject(hdc_temp, hPen);

	hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	hBrush_temp = (HBRUSH)SelectObject(hdc_temp, hBrush);

	sx = x;
	sy = y;
	ex = x + w;
	ey = y + h;

	Rectangle(hdc_temp, sx, sy, ex, ey);

	SelectObject(hdc_temp, hPen_temp);
	SelectObject(hdc_temp, hBrush_temp);
	SelectObject(hdc_temp, hBmp_temp);

	DeleteObject(hPen);

}
void DlgBitmap::pset(int x, int y, COLORREF c)
{	
	HBITMAP hBmp_temp;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);
	SetPixel(hdc_temp, x, y, c);
	SelectObject(hdc_temp, hBmp_temp);
}
#define PALETTECOLORS 256
void DlgBitmap::copy(unsigned char* ptr, int cols, int rows, int pixelbits)
{
	PBITMAPINFO pbmi;
	HBITMAP hBmp_temp;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);

	if (pixelbits <= 8) {
		pbmi = (PBITMAPINFO) new unsigned char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PALETTECOLORS];
		pbmi->bmiHeader.biClrUsed = (1 << pixelbits);
		for (int i = 0; i < PALETTECOLORS; i++) {
			pbmi->bmiColors[i].rgbRed = i;
			pbmi->bmiColors[i].rgbGreen = i;
			pbmi->bmiColors[i].rgbBlue = i;
			pbmi->bmiColors[i].rgbReserved = 0;
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

	SetStretchBltMode(hdc_temp, COLORONCOLOR);
	StretchDIBits(hdc_temp, 0, 0, bmwidth, bmheight, 0, 0, cols, rows, ptr, pbmi, DIB_RGB_COLORS, SRCCOPY);
	SelectObject(hdc_temp, hBmp_temp);

	delete[] pbmi;
}

void DlgBitmap::copyimg(void)
{
	PBITMAPINFO pbmi;
	HBITMAP hBmp_temp;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);


		pbmi = (PBITMAPINFO) new unsigned char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PALETTECOLORS];
		pbmi->bmiHeader.biClrUsed = (1 << 8);
		for (int i = 0; i < PALETTECOLORS; i++) {
			pbmi->bmiColors[i].rgbRed = i;
			pbmi->bmiColors[i].rgbGreen = i;
			pbmi->bmiColors[i].rgbBlue = i;
			pbmi->bmiColors[i].rgbReserved = 0;
		}

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmwidth;
	pbmi->bmiHeader.biHeight = -bmheight;
	pbmi->bmiHeader.biXPelsPerMeter = 2835;
	pbmi->bmiHeader.biYPelsPerMeter = 2835;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 8;
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biSizeImage = ((bmwidth * 8 + 31) & ~31) / 8 * bmheight;
	pbmi->bmiHeader.biClrImportant = 0;

	SetStretchBltMode(hdc_temp, COLORONCOLOR);
	StretchDIBits(hdc_temp, 0, 0, bmwidth, bmheight, 0, 0, bmwidth, bmheight, img, pbmi, DIB_RGB_COLORS, SRCCOPY);
	SelectObject(hdc_temp, hBmp_temp);

	delete[] pbmi;
}

void DlgBitmap::mirror_copy(unsigned char* ptr, int cols, int rows, int pixelbits)
{
	PBITMAPINFO pbmi;
	HBITMAP hBmp_temp;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);

	if (pixelbits <= 8) {
		pbmi = (PBITMAPINFO) new unsigned char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PALETTECOLORS];
		pbmi->bmiHeader.biClrUsed = (1 << pixelbits);
		for (int i = 0; i < PALETTECOLORS; i++) {
			pbmi->bmiColors[i].rgbRed = i;
			pbmi->bmiColors[i].rgbGreen = i;
			pbmi->bmiColors[i].rgbBlue = i;
			pbmi->bmiColors[i].rgbReserved = 0;
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

	SetStretchBltMode(hdc_temp, COLORONCOLOR);
	StretchDIBits(hdc_temp, bmwidth, 0, -bmwidth, bmheight, 0, 0, cols, rows, ptr, pbmi, DIB_RGB_COLORS, SRCCOPY);
	SelectObject(hdc_temp, hBmp_temp);

	delete[] pbmi;

}
void DlgBitmap::refresh(void)
{
	SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
}
void DlgBitmap::histogram(int xpos, int ypos, int hist[], COLORREF c,DWORD mode)
{
	HDC hdc_hist;
	HBITMAP hBmpHist,hBmpHistOrig,hBmp_temp;
	HPEN hPen, hPenOrig;
	int x;

	hBmp_temp = (HBITMAP)SelectObject(hdc_temp, hBmp);
	hdc_hist = CreateCompatibleDC(hdc_temp);
	hBmpHist = CreateCompatibleBitmap(hdc_temp,256,100);
	hBmpHistOrig = (HBITMAP)SelectObject(hdc_hist, hBmpHist);
	SetDCPenColor(hdc_hist, c);
	hPen= (HPEN)GetStockObject(DC_PEN);
	hPenOrig = (HPEN)SelectObject(hdc_hist, hPen);

	for (x = 0; x < 256; x++) {
		MoveToEx(hdc_hist, x, 100, NULL);
		LineTo(hdc_hist, x, 100 - hist[x]);
	}

	BitBlt(hdc_temp, xpos, ypos - 100, 256, 100, hdc_hist, 0, 0, mode);

	SelectObject(hdc_hist, hBmpHistOrig);
	SelectObject(hdc_hist, hPenOrig);
	DeleteDC(hdc_hist);
	DeleteObject(hBmpHist);
	SelectObject(hdc_temp, hBmp_temp);
}

DlgSlider::DlgSlider(HWND hDlg, int DlgItem, int min, int max, int value,int tic,int page):DlgCtrl(hDlg,DlgItem)
{

	SendMessage(hWnd, TBM_SETRANGEMAX, true, max);
	SendMessage(hWnd, TBM_SETRANGEMIN, true, min);

	SendMessage(hWnd, TBM_SETPOS, true, value);
	SendMessage(hWnd, TBM_SETTICFREQ, tic, 0);
	SendMessage(hWnd, TBM_SETPAGESIZE, 0, page);
}
DlgSlider::~DlgSlider(void)
{

}
void DlgSlider::Set(int value)
{
	SendMessage(hWnd, TBM_SETPOS, true, value);
}
void DlgSlider::SetMax(int value)
{
	SendMessage(hWnd, TBM_SETRANGEMAX, true, value);
}
void DlgSlider::SetMin(int value)
{
	SendMessage(hWnd, TBM_SETRANGEMIN, true, value);
}
void DlgSlider::SetLinesize(int value)
{
	SendMessage(hWnd, TBM_SETLINESIZE, 0, value);
}
int  DlgSlider::Get(void)
{
	return (int)SendMessage(hWnd, TBM_GETPOS, 0, 0);
}

DlgCheckbox::DlgCheckbox(HWND hDlg, int DlgItem):DlgCtrl(hDlg,DlgItem)
{
}
DlgCheckbox::~DlgCheckbox()
{
}
void DlgCheckbox::Set(bool mode)
{
	if (mode) {
		SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else {
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}
bool DlgCheckbox::Get(void)
{
	return (BOOL)SendMessage(hWnd, BM_GETCHECK, 0, 0);
}
bool DlgCheckbox::Flip(void)
{
	int mode;
	mode = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
	if (mode) {
		Set(false);
		return false;
	}
	Set(true);
	return true;
}

DlgLabel::DlgLabel(HWND hDlg, int DlgItem):DlgCtrl(hDlg,DlgItem)
{
}
DlgLabel::~DlgLabel()
{
}
bool DlgLabel::SetText(std::wstring ws)
{
	bool ret;
	ret=SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)ws.c_str());
	return ret;
}

DlgRadioButton::DlgRadioButton(HWND hDlg, int DlgItem):DlgCtrl(hDlg,DlgItem)
{

}
DlgRadioButton::~DlgRadioButton()
{
}
void DlgRadioButton::Set(bool mode)
{
	if (mode) {
		SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
	}
	else {
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}
bool DlgRadioButton::Get(void)
{
	return (BOOL)SendMessage(hWnd, BM_GETCHECK, 0, 0);
}

bool DlgRadioButton::SetText(std::wstring ws)
{
	bool ret;
	ret = SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)ws.c_str());
	return ret;
}

DlgComboList::DlgComboList(HWND hDlg, int DlgItem):DlgCtrl(hDlg, DlgItem)
{

	ComboBox_ResetContent(hWnd);
}
DlgComboList::~DlgComboList()
{
	ComboBox_ResetContent(hWnd);
}
bool DlgComboList::add(std::wstring ws)
{
	return ComboBox_AddString(hWnd, ws.c_str());
}
bool DlgComboList::Set(int num)
{
	return ComboBox_SetCurSel(hWnd, num);
}
bool DlgComboList::Set(std::wstring &ws)
{
	int num;
	num = ComboBox_FindString(hWnd, 0, ws.c_str());
	return ComboBox_SetCurSel(hWnd, num);
}
int DlgComboList::Get(void)
{
	return ComboBox_GetCurSel(hWnd);
}
std::wstring DlgComboList::GetString(void)
{
	TCHAR* t;
	int index,length;
	index = Get();
	length = ComboBox_GetLBTextLen(hWnd, index) + 1;
	t = new TCHAR[length];
	ComboBox_GetLBText(hWnd, index, t);
	std::wstring ws(t);
	delete[] t;
	return ws;
}

DlgEditBox::DlgEditBox(HWND hDlg, int DlgItem) :DlgCtrl(hDlg, DlgItem)
{
}
DlgEditBox::~DlgEditBox()
{
}
bool DlgEditBox::SetNum(int num)
{
	std::wstring ws;
	ws = std::to_wstring(num);
	return Set(ws);
}
bool DlgEditBox::SetDouble(const std::wstring& fmt,double f)
{
	wstring ws;
	ws = (boost::wformat(fmt.c_str()) % f).str();
	return Set(ws);
}
bool DlgEditBox::Set(std::wstring& ws)
{
	return Edit_SetText(hWnd, ws.c_str());
}
int DlgEditBox::GetNum(void)
{
	std::wstring ws;
	ws = Get();
	return std::stoi(ws);
}
double DlgEditBox::GetDouble(void)
{
	std::wstring ws;
	ws = Get();

	double num;
	try {
		num = std::stof(ws);
	}
	catch (const std::invalid_argument& /*e*/) {
		num = 0;
  }
	return num;
}
std::wstring DlgEditBox::Get(void)
{
	TCHAR* t;
	int  length;

	length = Edit_GetTextLength(hWnd) + 1;
	t = new TCHAR[length];
	Edit_GetText(hWnd, t,length);
	std::wstring ws(t);
	delete[] t;
	return ws;
}

DlgSpin::DlgSpin(HWND hDlg, int DlgItem) :DlgCtrl(hDlg, DlgItem)
{
}
DlgSpin::~DlgSpin()
{
}
void DlgSpin::SetMinMax(int min, int max)
{
	SendMessage(hWnd, UDM_SETRANGE, 0, MAKELONG((short)max, (short)min));
}
int  DlgSpin::Set(int num)
{
	WPARAM wParam;
	wParam = SendMessage(hWnd, UDM_SETPOS, 0, MAKELONG(num, 0));
	return (int)LOWORD(wParam);
}
void  DlgSpin::SetBuddy(HWND hWndEdit)
{
	SendMessage(hWnd, UDM_SETBUDDY, 0, (WPARAM)hWndEdit);
}
int DlgSpin::Get(void)
{
	WPARAM wParam;
	wParam=SendMessage(hWnd, UDM_GETPOS, 0,0);
	return (int)LOWORD(wParam);
}
void DlgSpin::SetAccel(int step)
{
	UDACCEL ac;
	ac.nSec = 0;
	ac.nInc = step;
	SendMessage(hWnd, UDM_SETACCEL, 1, (LPARAM)&ac);
}