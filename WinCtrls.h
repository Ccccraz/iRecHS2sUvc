#pragma once

class WinCtrls {
private:

public:
	POINTS Wpos, Wsize;
	HMENU id;
	HWND hWnd;
	HFONT hFont;
	WinCtrls();
	~WinCtrls();
	void move(int x, int y);
	void movex(int x);
	void movey(int y);
	void move(int x, int y, int w, int h);
	void resize(int w, int h);
	void SetText(std::wstring& txt);
	void enable(void);
	void disable();
	BOOL IsEnabled(void);
};

class WinCheckbox : public WinCtrls
{
private:
	BOOL mode;

public:
	WinCheckbox(HWND hWndParent, HMENU id, std::wstring& ws, BOOL state, int x, int y);
	void Set(BOOL mode);
	BOOL Get(void);
	BOOL Flip(void);
};

class WinSlider : public WinCtrls
{
private:
	int max;
	int min;
public:
	WinSlider(HWND hWndParent, HMENU id, int xx, int yy, int min, int max);
	WNDPROC defWinSliderWndProc;
	void Set(int pos);
	void SetMin(int min);
	void SetMax(int max);
	int GetMax(void) { return max; };
	int GetMin(void) { return min; };
	int Get(void);
	static LRESULT CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

class WinStatic : public WinCtrls
{
private:
public:
	WinStatic(HWND hWndParent, HMENU id, LPCTSTR name, int x, int y);
};
