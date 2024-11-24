#include "framework.h"
#include "iRecHS2s.h"

#define BUTTON_W 250
#define BUTTON_H 30
#define STATIC_W 150
#define STATIC_H 15

#define FONT_SIZE 14
#define FONT_CHARSET ANSI_CHARSET
#define FONT_TYPEFACE TEXT("Courier New")


WinCtrls::WinCtrls()
{
	Wpos = { 0,0 };
	Wsize = { 0,0 };
	hWnd = NULL;
	id = (HMENU)0;
	hFont = NULL;
}
WinCtrls::~WinCtrls()
{
	if (hFont != NULL)
		DeleteObject(hFont);
}

void WinCtrls::move(int x, int y, int w, int h)
{
	Wpos.x = x;
	Wpos.y = y;
	Wsize.x = w;
	Wsize.y = h;
	MoveWindow(hWnd, Wpos.x, Wpos.y, Wsize.x, Wsize.y, true);

}
void WinCtrls::move(int x, int y)
{
	Wpos.x = x;
	Wpos.y = y;
	MoveWindow(hWnd, Wpos.x, Wpos.y, Wsize.x, Wsize.y, true);
}
void WinCtrls::movex(int x)
{
	Wpos.x = x;
	MoveWindow(hWnd, Wpos.x, Wpos.y, Wsize.x, Wsize.y, true);
}
void WinCtrls::movey(int y)
{
	Wpos.y = y;
	MoveWindow(hWnd, Wpos.x, Wpos.y, Wsize.x, Wsize.y, true);
}
void WinCtrls::resize(int w, int h)
{
	Wsize.x = w;
	Wsize.y = h;
	MoveWindow(hWnd, Wpos.x, Wpos.y, Wsize.x, Wsize.y, true);
}
void WinCtrls::enable(void)
{
	EnableWindow(hWnd, true);
}
void WinCtrls::disable(void)
{
	EnableWindow(hWnd, false);
}
BOOL WinCtrls::IsEnabled(void)
{
	return IsWindowEnabled(hWnd);
}
void WinCtrls::SetText(std::wstring& txt)
{
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)txt.c_str());
}

WinCheckbox::WinCheckbox(HWND hWndParent, HMENU idid, std::wstring& name, BOOL state, int x, int y)
{
	Wpos.x = x;
	Wpos.y = y;
	Wsize.x = BUTTON_W;
	Wsize.y = BUTTON_H;

	id = idid;
	hFont = CreateFont(FONT_SIZE, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, FONT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FONT_TYPEFACE);
	hWnd = CreateWindowEx(0, TEXT("BUTTON"), TEXT(""), WS_CHILD | BS_AUTOCHECKBOX, Wpos.x, Wpos.y, Wsize.x, Wsize.y, hWndParent, (HMENU)id, hInst, NULL);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)name.c_str());
	Set(state);
	ShowWindow(hWnd, SW_SHOW);
}
void WinCheckbox::Set(BOOL m)
{
	if (m) {
		SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
		mode = true;
	}
	else {
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
		mode = false;
	}
}
BOOL WinCheckbox::Get(void)
{
	return mode;
}
BOOL WinCheckbox::Flip(void)
{
	if (mode) {
		mode = false;
		Set(mode);
	}
	else {
		mode = true;
		Set(mode);
	}
	return mode;
}


void WinSlider::Set(int pos)
{
	if (pos > max)pos = max;
	if (pos < min)pos = min;
	SendMessage(hWnd, TBM_SETPOS, true, pos);

}

void WinSlider::SetMin(int mi)
{
	min = mi;
	SendMessage(hWnd, TBM_SETRANGEMIN, true, min);
}

void WinSlider::SetMax(int mx)
{
	max = mx;
	SendMessage(hWnd, TBM_SETRANGEMAX, true, max);
}

int WinSlider::Get(void)
{
	return (int)SendMessage(hWnd, TBM_GETPOS, 0, 0);
}

LRESULT CALLBACK WinSlider::CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//if (message == WM_COMMAND && LOWORD(wParam) == ID_ACCELERATOR_ESC) {
	//	SendMessage(GetAncestor(hWnd, GA_ROOT), WM_COMMAND, ID_FILE_PAUSE, 0);
	//	return 0;
	//}
	//if (message == WM_COMMAND && LOWORD(wParam) == ID_ACCELERATOR_SPACE) {
	//	SendMessage(GetAncestor(hWnd, GA_ROOT), WM_COMMAND, ID_ACCELERATOR_SPACE, 0);
	//	return 0;
	//}

	WNDPROC thisPtr = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	return CallWindowProc(thisPtr, hWnd, message, wParam, lParam);
}

WinSlider::WinSlider(HWND hWndParent, HMENU idid, int x, int y, int mi, int ma)
{
	Wpos.x = x;
	Wpos.y = y;
	Wsize.x = BUTTON_W;
	Wsize.y = BUTTON_H;
	min = mi;
	max = ma;
	id = idid;

	hWnd = CreateWindowEx(0, TEXT("msctls_trackbar32"), TEXT(""), WS_CHILD | TBS_HORZ | TBS_BOTH | TBS_NOTICKS, Wpos.x, Wpos.y, Wsize.x, Wsize.y, hWndParent, (HMENU)id, hInst, NULL);
	SendMessage(hWnd, TBM_SETRANGEMAX, false, max);
	SendMessage(hWnd, TBM_SETRANGEMIN, false, min);
	Set(max);
	defWinSliderWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)CallbackProc);
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)defWinSliderWndProc);
	ShowWindow(hWnd, SW_SHOW);
}


WinStatic::WinStatic(HWND hWndParent, HMENU idid, LPCTSTR name, int x, int y)
{
	Wpos.x = x;
	Wpos.y = y;
	Wsize.x = STATIC_W;
	Wsize.y = STATIC_H;
	id = idid;
	hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, FONT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FONT_TYPEFACE);
	hWnd = CreateWindowEx(0, TEXT("STATIC"), TEXT(""), WS_CHILD | WS_VISIBLE | SS_CENTER, Wpos.x, Wpos.y, Wsize.x, Wsize.y, hWndParent, (HMENU)id, hInst, NULL);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)name);
	ShowWindow(hWnd, SW_SHOW);
}


