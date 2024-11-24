#pragma once

//errno_t a2t(char* src, TCHAR* des, rsize_t num);
//errno_t t2a(TCHAR* src, char* des, size_t num);

class DlgCtrl {
private:
public:
	HWND hWnd, hWndParent;
	int xpos, ypos, width, height;
	int id;

	DlgCtrl(HWND hDlg, int DlgItem);
	~DlgCtrl();
	void enable(void);
	void disable(void);
	BOOL IsEnabled(void);
	void move(int x, int y);
	void size(int w, int h);
	void movex(int x);
	void movey(int y);
};

class DlgListBox: public DlgCtrl {
private:
public:
	DlgListBox(HWND hDlg, int DlgItem);
	~DlgListBox();
	bool add(std::wstring ws);
	bool Set(int num);
	int  Get(void);
};

class DlgButton: public DlgCtrl {
private:
public:
	DlgButton(HWND hDlg, int DlgItem);
};

class DlgBitmap:public DlgCtrl{
private:
	
public:
	unsigned char* img = NULL;
  int bmwidth, bmheight;
	HBITMAP hBmp, hBmpOrig;
	HDC hdc_temp;

	DlgBitmap(HWND hWnd, int DlgItem, int x, int y);
	~DlgBitmap();
	void fill(DWORD rop);
	void line(int sx, int sy, int ex, int ey, COLORREF c,int width);
	void dotline(int sx, int sy, int ex, int ey, COLORREF c);
	void grey_plot(int sx, int sy, int ex, int ey);
	void grey_plot(double x, double y, double a);
	void grey_plot_img(double x, double y, double a);
	void grey_plot_img(int sx, int sy, int ex, int ey);
	void calc_path(double x, double y, double a, int& sx, int& sy, int& ex, int& ey);
	void circle(int x, int y, int r, COLORREF c);
	void rect(int x, int y, int w, int h, COLORREF c);
	void pset(int x, int y, COLORREF c);

	void refresh(void);
	void copy(unsigned char* ptr, int width, int height, int depth);
	void copyimg(void);
	void mirror_copy(unsigned char* ptr, int width, int height, int depth);
	void histogram(int x, int y,int hist[], COLORREF c,DWORD mode);
};

class DlgSlider: public DlgCtrl {
private:
public:
	DlgSlider(HWND hDlg, int DlgItem, int min, int max, int value,int tic,int page);
	~DlgSlider();
	void Set(int value);
	void SetMax(int value);
	void SetMin(int value);
	void SetLinesize(int value);
	int Get(void);
};

class DlgCheckbox:public DlgCtrl {
private:
public:
	DlgCheckbox(HWND hDlg, int DlgItem);
	~DlgCheckbox();
	void Set(bool);
	bool Get(void);
	bool Flip(void);
};

class DlgLabel:public DlgCtrl {
private:
public:
	DlgLabel(HWND hDlg, int DlgItem);
	~DlgLabel();
	bool SetText(std::wstring ws);
};

class DlgRadioButton:public DlgCtrl {
private:
public:
	DlgRadioButton(HWND hDlg, int DlgItem);
	~DlgRadioButton();
	void Set(bool);
	bool Get(void);
	bool SetText(std::wstring ws);
};


class DlgComboList :public DlgCtrl {
private:
public:
	DlgComboList(HWND hDlg, int DlgItem);
	~DlgComboList();
	bool add(std::wstring ws);
	bool Set(int num);
	bool Set(std::wstring &ws);
	int Get(void);
	std::wstring GetString(void);
};

class DlgEditBox :public DlgCtrl {
private:
public:
	DlgEditBox(HWND hDlg, int DlgItem);
	~DlgEditBox();
	bool SetNum(int num);
	bool SetDouble(const std::wstring& fmt, double f);
	int GetNum(void);
	double GetDouble(void);
	bool Set(std::wstring& ws);
	std::wstring Get(void);
};

class DlgSpin :public DlgCtrl {
private:
public:
	DlgSpin(HWND hDlg, int DlgItem);
	~DlgSpin();
	void SetMinMax(int min, int max);
	int Set(int num);
	void SetBuddy(HWND hWndEdit);
	void SetAccel(int step);
	int Get(void);
};