#include "framework.h"
#include "iRecHS2s.h"



#define ID_TIMELINE_TIMER 1100
#define TIMELINE_INTERVAL 33
#define FrameLeft 50
#define FrameRight 25
#define FrameTop 25
#define FrameBottom 50
#define MaxTimeLine_Width 3840
#define MaxTimeLine_Height  2160


#define CONWIN 1
void iRec::W_TimeLine::Resize()
{
	int x, y, w, h;
	RECT rect, rectParent, rectCamView;
	GetClientRect(hWndParent, &rectParent);
	GetWindowRect(iRecPtr->wCamView->hWnd, &rectCamView);
	x = (rectCamView.right - rectCamView.left) + 10;
	if (x < 480) x = 490;
	y = 0;
	w = (rectParent.right - rectParent.left) - x ;
	h = (rectParent.bottom - rectParent.top-Status_Offset);
	if (w < 256) {
		ShowWindow(hWnd, SW_HIDE);
		return;
	}
	else {
		if (!IsWindowVisible(hWnd))
			ShowWindow(hWnd, SW_SHOW);
	}
	MoveWindow(hWnd, x, y, w, h, true);
	GetClientRect(hWnd, &rect);
	winsize.w = rect.right - rect.left;
	winsize.h = rect.bottom - rect.top;
	if (winsize.w > MaxTimeLine_Width) {
		winsize.w = MaxTimeLine_Width;
	}
	if (winsize.h > MaxTimeLine_Height) {
		winsize.h = MaxTimeLine_Height;
	}

	GraphArea.left = FrameLeft;
	GraphArea.right = winsize.w - FrameRight;
	GraphArea.top = FrameTop;
	GraphArea.bottom = winsize.h - FrameBottom;

}

void iRec::W_TimeLine::ClickGraph(double ti)
{
	GraphBaseInfo gbix;
	
	SetGraphBaseInfoX(gbix, GraphArea.right - GraphArea.left);
	int x = GraphArea.right - static_cast<int>(ti * gbix.UnitPixel);
	SendMessage(hWnd, WM_LBUTTONDOWN, 0, MAKELPARAM(x, GraphArea.top));
	SendMessage(hWnd, WM_LBUTTONUP, 0, MAKELPARAM(x, GraphArea.top));
}


iRec::W_TimeLine::W_TimeLine(HWND parent, iRec& ir, int x, int y, int w, int h)
{
	WNDCLASSEX wcex;
	RECT rect;


	iRecPtr = &ir;
	hWndParent = parent;
	hBmpMem = hOrigBmpMem = NULL;

	hdc_mem = NULL;
	hPenBlack = NULL;
	hPenRed1 = hPenRed2 = hPenRed3 = NULL;
	hPenBlue1 = hPenBlue2 = hPenBlue3 = NULL;
	hPenGreen1 = hPenGreen2 = hPenGrey = hPenYellow = NULL;
	hOrigPen = NULL;

	hBrush = hBrushNull = hOrigBrush = NULL;

	hFont = hOrigFont = NULL;

	LButtonDown = false;


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
	wcex.lpszClassName = TEXT("W_TimeLine");
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);

	rect.top = rect.left = 0;
	winsize.w = rect.right = w;
	winsize.h = rect.bottom = h;


	GraphArea.left = FrameLeft;
	GraphArea.right = winsize.w - FrameRight;
	GraphArea.top = FrameTop;
	GraphArea.bottom = winsize.h - FrameBottom;


	AdjustWindowRect(&rect, WS_CHILD | WS_BORDER | WS_VISIBLE, FALSE);

	hWnd = CreateWindow(TEXT("W_TimeLine"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		x, y, rect.right - rect.left, rect.bottom - rect.top, hWndParent, NULL, hInst, this);

	PosSize.left = x;
	PosSize.right = x + rect.right - rect.left;
	PosSize.top = y;
	PosSize.bottom = y + rect.bottom - rect.top;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	Resize();
}
iRec::W_TimeLine::~W_TimeLine()
{

}
LRESULT CALLBACK iRec::W_TimeLine::CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	W_TimeLine* thisPtr = NULL;

	switch (message) {
	case WM_CREATE:
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
		thisPtr = (W_TimeLine*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		break;
	case WM_DESTROY:
		thisPtr = (W_TimeLine*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		break;
	default:
		thisPtr = (W_TimeLine*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		break;
	}
	if (thisPtr) {
		return thisPtr->WndProc(hWnd, message, wParam, lParam);
	}
	return (DefWindowProc(hWnd, message, wParam, lParam));
}


void iRec::W_TimeLine::GraphClear(void)
{
	HBRUSH hBrushGrey, hBrushOrig;
	hBrushGrey = CreateSolidBrush(RGB(0xf0, 0xf0, 0xf0));
	hBrushOrig = (HBRUSH)SelectObject(hdc_mem, hBrushGrey);
	PatBlt(hdc_mem, 0, 0, MaxTimeLine_Width, MaxTimeLine_Height, PATCOPY);
	DeleteObject(SelectObject(hdc_mem, hBrushOrig));
}

bool iRec::W_TimeLine::IsMajorStep(double pos, double step)
{
	double a, b, c;
	if (step == 0) return false;
	c = fabs(pos) / step;
	a = floor(c + 0.0001);
	b = fabs(fabs(a * step) - fabs(pos));

	//{
	//	static int count = 0;
	//	if (count < 100) {
	//		wstringstream wss;
	//		wss << L"pos=" << pos << endl;
	//		wss << L"fabs(pos)/step=" << c << endl;
	//		wss << L"(int)(fabs(pos)/step)=" << d << endl;
	//		wss << L"floor(fabs(pos)/step)=" << a << endl;
	//		wss << L"b=" << b << endl;
	//		conwin.write(wss.str());
	//	}
	//	count++;
	//}

	if (b < 0.0001) return true;

	return false;
}



void iRec::W_TimeLine::SetGraphBaseInfoX(GraphBaseInfo& gbi, int range)
{
	double MiStepW[] = { 0.1,0.2,0.5,1.0 };
	gbi.RangePixel = range;
	gbi.Range = static_cast<double>(iRecPtr->TimeLineParam.TimeRange);
	gbi.MajorStep = 1;
	 
	for (double mw : MiStepW) {
		gbi.MinorStep = mw;
		gbi.MinorPixelStep = static_cast<int>((gbi.MinorStep * gbi.RangePixel / gbi.Range) + 0.0001);
		if (gbi.MinorPixelStep >= 5) break;
	}

	gbi.MajorPixelStep = gbi.MinorPixelStep * static_cast<int>(gbi.MajorStep / gbi.MinorStep + 0.0001);
	gbi.UnitPixel = static_cast<double>(gbi.MinorPixelStep) / gbi.MinorStep;
	gbi.Range = static_cast<double>(gbi.RangePixel) / gbi.UnitPixel;
}





void iRec::W_TimeLine::SyncYstep(GraphBaseInfo& src, GraphBaseInfo& des)
{
	double tmin;
	if (src.Range > des.Range) {
		tmin = ceil(des.Offset / src.MajorStep) * src.MajorStep;
		while (tmin > des.Offset) {
			tmin -= src.MajorStep;
		}
		des = src;
		des.Offset = tmin;
	}
	else {
		tmin = ceil(src.Offset / des.MajorStep) * des.MajorStep;
		while (tmin > src.Offset) {
			tmin -= des.MajorStep;
		}
		src = des;
		src.Offset = tmin;
	}
}


void iRec::W_TimeLine::SetGraphBaseInfoY(GraphBaseInfo& gbi, int range, double max, double min)
{
	double MiStepH[] = { 0.1,0.2,0.5, 1 ,2, 5,10,20,50,100,200,500,1000,2000,5000 };
	double ytmax, ytmin;
	gbi.RangePixel = range;

	for (int i = 0; i < sizeof(MiStepH) / sizeof(*MiStepH); i++) {
		gbi.MinorStep = MiStepH[i];
		gbi.MajorStep = MiStepH[i] * 5;
		ytmax = ceil(max / gbi.MajorStep) * gbi.MajorStep;
		ytmin = ceil(min / gbi.MajorStep) * gbi.MajorStep;

		while (ytmax < max) {
			ytmax += gbi.MajorStep;
		}
		while (ytmin > min) {
			ytmin -= gbi.MajorStep;
		}
		if ((ytmax - ytmin) < gbi.MajorStep) {
			ytmax += gbi.MajorStep;
			ytmin -= gbi.MajorStep;
		}

		if ((ytmax - ytmin) < gbi.MajorStep * 2) {
			if ((fabs(ytmax - max) < fabs(ytmin - min))) {
				ytmax += gbi.MajorStep;
			}
			else {
				ytmin -= gbi.MajorStep;
			}
		}
		gbi.Range = ytmax - ytmin;
		gbi.Offset = ytmin;

		gbi.MinorPixelStep = static_cast<int>(gbi.MinorStep * static_cast<double>(gbi.RangePixel) / gbi.Range + 0.0001);
		gbi.MajorPixelStep = gbi.MinorPixelStep * 5;
		gbi.UnitPixel = static_cast<double>(gbi.MinorPixelStep) / gbi.MinorStep;
		if (gbi.MinorPixelStep > 3) break;
	}

}


void iRec::W_TimeLine::SetFixation(GraphParam& gp, INT64 co)
{

	FixationPoints current;
	bool retflag = false;
	if (currentFXP.duration > iRecPtr->CalibrationParam.FixationDuration) {
		current = currentFXP;
		current.S_time = (iRecPtr->Eye.v[current.S_co].TimeStamp - iRecPtr->Eye.v[co].TimeStamp) / 1e9;
		current.S_pos = gp.Area.right + static_cast<int>(current.S_time * gp.x.UnitPixel);
		current.E_time = (iRecPtr->Eye.v[current.E_co].TimeStamp - iRecPtr->Eye.v[co].TimeStamp) / 1e9;
		current.E_pos = gp.Area.right + static_cast<int>(current.E_time * gp.x.UnitPixel);
		if (current.E_pos < gp.Area.left) {
			return;
		}
		if (current.S_pos < gp.Area.left) {
			current.S_pos = gp.Area.left;
		}
		gp.fpv.push_back(current);
	}
	for (auto& fxp : FxPoints) {
		current = fxp;
		current.S_time = (iRecPtr->Eye.v[current.S_co].TimeStamp - iRecPtr->Eye.v[co].TimeStamp) / 1e9;
		current.S_pos = gp.Area.right + static_cast<int>(current.S_time * gp.x.UnitPixel);
		current.E_time = (iRecPtr->Eye.v[current.E_co].TimeStamp - iRecPtr->Eye.v[co].TimeStamp) / 1e9;
		current.E_pos = gp.Area.right + static_cast<int>(current.E_time * gp.x.UnitPixel);
		if (current.E_pos < gp.Area.left) {
			return;
		}
		if (current.S_pos < gp.Area.left) {
			current.S_pos = gp.Area.left;
		}
		gp.fpv.push_back(current);
	}
}


void iRec::W_TimeLine::DetectFixation(INT64 co)
{
	static INT64 prevCounter = 0;
	INT64 prevTS;
	int prevMethod, prevMode;
	double interval, diff_x, diff_y, length, range;
	bool fixation_reset = false, IsChange = false;
	INT64 ic;

	if (prevCounter == co) return;

	if (prevCounter == 0) {
		prevTS = 0;
	}
	else {
		prevTS = iRecPtr->Eye.v[prevCounter].TimeStamp;
	}
	prevMethod = iRecPtr->Eye.v[prevCounter].Gaze.method;
	prevMode = iRecPtr->Eye.v[prevCounter].Gaze.mode;

	for (ic = prevCounter + 1; ic <= co; ic++) {

		interval = fabs(iRecPtr->Eye.v[ic].TimeStamp - prevTS) / 1e9;
		prevTS = iRecPtr->Eye.v[ic].TimeStamp;

		IsChange = false;
		if (prevMode != iRecPtr->Eye.v[ic].Gaze.mode) IsChange = true;
		prevMode = iRecPtr->Eye.v[ic].Gaze.mode;
		if (prevMethod != iRecPtr->Eye.v[ic].Gaze.method) IsChange = true;
		prevMethod = iRecPtr->Eye.v[ic].Gaze.method;

		if (iRecPtr->Eye.v[ic].pupil.size.w == 0) {
			if (currentFXP.duration > iRecPtr->CalibrationParam.FixationDuration) {
				FxPoints.push_front(currentFXP);
			}
			fixation_reset = true;
			currentFXP.duration = 0;
			continue;
		}

		diff_x = iRecPtr->Eye.v[ic].Gaze.x - currentFXP.base_x;
		diff_y = iRecPtr->Eye.v[ic].Gaze.y - currentFXP.base_y;
		length = sqrt(diff_x * diff_x + diff_y * diff_y);

		if (iRecPtr->Eye.v[ic].Gaze.mode == 0) {
			range = iRecPtr->CalibrationParam.FixationRangePixel;
		}
		else {
			range = iRecPtr->CalibrationParam.FixationRangeAngle;
		}
		if ((interval > 0.1 || length > range || IsChange == true) && (fixation_reset != true)) {
			if (currentFXP.duration > iRecPtr->CalibrationParam.FixationDuration) {
				FxPoints.push_front(currentFXP);
			}
			fixation_reset = true;
		}

		if (fixation_reset == true) {
			currentFXP.S_co = ic;
			currentFXP.E_co = ic;
			currentFXP.S_time = (iRecPtr->Eye.v[ic].TimeStamp) / 1e9;
			currentFXP.E_time = (iRecPtr->Eye.v[ic].TimeStamp) / 1e9;
			currentFXP.duration = 0;
			currentFXP.base_x = iRecPtr->Eye.v[ic].Gaze.x;
			currentFXP.base_y = iRecPtr->Eye.v[ic].Gaze.y;
			fixation_reset = false;
			continue;
		}
		currentFXP.E_co = ic;
		currentFXP.E_time = (iRecPtr->Eye.v[ic].TimeStamp) / 1e9;
		currentFXP.duration = fabs(currentFXP.E_time - currentFXP.S_time);
	}
	prevCounter = co;
}


void iRec::W_TimeLine::GraphMark(GraphParam& gp, FixationSection& fs)
{
	if (fs.S_co < fs.E_co) {
		HBRUSH hBrushOrig = (HBRUSH)SelectObject(hdc_mem, hBrushNull);
		HPEN hPenOrig = (HPEN)SelectObject(hdc_mem, hPenRed2);
		Rectangle(hdc_mem, fs.S_pos, gp.Area.top, fs.E_pos + 1, gp.Area.bottom + 1);
		SelectObject(hdc_mem, hPenOrig);
		SelectObject(hdc_mem, hBrushOrig);
	}
}

bool iRec::W_TimeLine::SetCalibSection(GraphParam& gp, FixationSection& fs)
{
	fs.S_co = fs.E_co = 0;
	fs.S_pos = fs.E_pos = 0;
	for (auto& fp : gp.fpv) {
		if ((fp.S_pos <= fs.position) && (fp.E_pos >= fs.position)) {
			fs.S_pos = fp.S_pos;
			fs.E_pos = fp.E_pos;
			fs.S_co = fp.S_co;
			fs.E_co = fp.E_co;
			break;
		}
	}
	if (fs.S_co == fs.E_co) return false;
	return true;
}


void iRec::W_TimeLine::GraphPaintFixation(GraphParam& gp,int pos,INT64 co)
{
	HPEN hPenOrig;
	HBRUSH hBrushGreen;
	HPEN hPenGreen;
	HBRUSH hBrushOrig;
	double range;
	int yt,yb;
	double diff,y;
	hPenGreen = (HPEN)CreatePen(PS_SOLID, 1, RGB(0x55, 0x77, 0x55));
	hPenOrig=(HPEN)SelectObject(hdc_mem, hPenGreen);
	hBrushGreen = (HBRUSH)CreateSolidBrush(RGB(0xcc, 0xff, 0xcc));
	hBrushOrig = (HBRUSH)SelectObject(hdc_mem, hBrushGreen);


	for (auto& fp : gp.fpv) {
		if (pos == 0) {
			y = fp.base_x;
		}
		else {
			y = fp.base_y;
		}

		y=gp.Area.bottom - static_cast<int>((y - gp.y.Offset) * gp.y.UnitPixel);
		if (iRecPtr->Eye.v[co].Gaze.mode == 0) {
			range = iRecPtr->CalibrationParam.FixationRangePixel;
		}
		else {
			range = iRecPtr->CalibrationParam.FixationRangeAngle;
		}

		diff = (range * gp.y.UnitPixel);
		yt = static_cast<int>(y - diff);
		yb = static_cast<int>(y + diff);
		if (yt < gp.Area.top) yt = gp.Area.top+1;
		if (yb >= gp.Area.bottom) yb = gp.Area.bottom;
		Rectangle(hdc_mem, fp.S_pos,yt, fp.E_pos + 1, yb);
		
		//Rectangle(hdc_mem, fp.S_pos, gp.Area.top + 1, fp.E_pos + 1, gp.Area.bottom);
	}

	DeleteObject(SelectObject(hdc_mem, hPenOrig));
	DeleteObject(SelectObject(hdc_mem, hBrushOrig));

}


void iRec::W_TimeLine::GraphBase(GraphParam& gp)
{
	wstring ws;
	HPEN hPenOrig;
	HBRUSH hBrushOrig;
	//HBRUSH hBrushGreen;
	//HPEN hPenGreen;

	int ih;
	double dh;

	hPenOrig = (HPEN)SelectObject(hdc_mem, hPenBlack);
	Rectangle(hdc_mem, gp.Area.left, gp.Area.top, gp.Area.right + 1, gp.Area.bottom + 1);

	(HPEN)SelectObject(hdc_mem, hPenBlue3);
	for (int h = gp.Area.right; h > gp.Area.left; h -= gp.x.MinorPixelStep) {
		MoveToEx(hdc_mem, h, gp.Area.top + 1, NULL);
		LineTo(hdc_mem, h, gp.Area.bottom);
	}

	for (dh = gp.y.Offset + gp.y.MinorStep, ih = gp.Area.bottom; ih > gp.Area.top; dh += gp.y.MinorStep) {
		MoveToEx(hdc_mem, gp.Area.left + 1, ih, NULL);
		LineTo(hdc_mem, gp.Area.right, ih);
		ih = gp.Area.bottom - static_cast<int>((dh - gp.y.Offset) * gp.y.UnitPixel);
	}

	(HPEN)SelectObject(hdc_mem, hPenBlue2);
	for (int h = gp.Area.right; h > gp.Area.left; h -= gp.x.MajorPixelStep) {
		MoveToEx(hdc_mem, h, gp.Area.top + 1, NULL);
		LineTo(hdc_mem, h, gp.Area.bottom);
	}

	for (dh = gp.y.Offset, ih = gp.Area.bottom; ih >= gp.Area.top; ) {
		if (IsMajorStep(dh, gp.y.MajorStep)) {
			MoveToEx(hdc_mem, gp.Area.left + 1, ih, NULL);
			LineTo(hdc_mem, gp.Area.right, ih);
		}
		dh += gp.y.MinorStep;
		ih = gp.Area.bottom - static_cast<int>((dh - gp.y.Offset) * gp.y.UnitPixel);
	}

	SelectObject(hdc_mem, hPenBlack);
	hBrushOrig = (HBRUSH)SelectObject(hdc_mem, hBrushNull);
	Rectangle(hdc_mem, gp.Area.left, gp.Area.top, gp.Area.right + 1, gp.Area.bottom + 1);

	SetBkMode(hdc_mem, TRANSPARENT);
	SelectObject(hdc_mem, hPenOrig);
	SelectObject(hdc_mem, hBrushOrig);

	for (dh = gp.y.Offset, ih = gp.Area.bottom; ih > gp.Area.top; ) {
		if (ih - 5 < gp.Area.top) break;
		if (IsMajorStep(dh, gp.y.MajorStep)) {
			ws = (boost::wformat(L"%5.1f") % dh).str();
			TextOutW(hdc_mem, static_cast<int>(gp.Area.left) - 40, ih - 10, ws.c_str(), static_cast<int>(ws.size()));
		}
		dh += gp.y.MinorStep;
		ih = gp.Area.bottom - static_cast<int>((dh - gp.y.Offset) * gp.y.UnitPixel);
	}
}

void iRec::W_TimeLine::GraphScaleX(GraphParam& gp)
{
	int i, pos;
	wstring ws;

	for (i = 0; i <= gp.x.Range; i++) {
		pos = (gp.Area.right - gp.x.MajorPixelStep * i);
		if (i > 0) {
			ws = (boost::wformat(L"-%d") % i).str();
		}
		else {
			ws = (boost::wformat(L"%d") % i).str();
		}
		TextOutW(hdc_mem, static_cast<int>(pos) - 5, gp.Area.bottom, ws.c_str(), static_cast<int>(ws.size()));
	}
}

void iRec::W_TimeLine::GraphAnnotationHorizontalLine(GraphParam& gp, double dy)
{
	int  sx, ex,y;
	HPEN hPenOrig;

	hPenOrig = (HPEN)SelectObject(hdc_mem, hPenRed1);

	sx = gp.Area.left;
	ex = gp.Area.right;
	y = gp.Area.bottom - static_cast<int>((dy - gp.y.Offset) * gp.y.UnitPixel);

	MoveToEx(hdc_mem, sx, y, NULL);
	LineTo(hdc_mem, ex, y);
	
	SelectObject(hdc_mem, hPenOrig);
}

void iRec::W_TimeLine::GraphPlot(GraphParam& gp, vector<double>& data)
{
	int  x, y, prevx, prevy;
	double interval;
	double prevw, w;
	double prevTS;
	HPEN hPenOrig;

	hPenOrig = (HPEN)SelectObject(hdc_mem, hPenBlack);

	prevx = x = gp.Area.right;
	prevy = y = gp.Area.bottom - static_cast<int>((data[0] - gp.y.Offset) * gp.y.UnitPixel);
	prevw = w = gp.ti[0].pupil_w;
	prevTS = gp.ti[0].time;
	MoveToEx(hdc_mem, x, y, NULL);
	std::vector<GraphPlotX>::iterator vt;
	std::vector<double>::iterator dat;

	for (vt = gp.ti.begin(), dat = data.begin(); vt != gp.ti.end() && dat != data.end(); vt++, dat++) {

		interval = (vt->time - prevTS);
		prevTS = vt->time;

		w = vt->pupil_w;
		if (w == 0) {
			prevw = 0;
			continue;
		}
		x = gp.Area.right + static_cast<int>(vt->time * gp.x.UnitPixel);
		y = gp.Area.bottom - static_cast<int>((*dat - gp.y.Offset) * gp.y.UnitPixel);

		if (y >= gp.Area.bottom) y = gp.Area.bottom - 1;
		if (y <= gp.Area.top) y = gp.Area.top + 1;

		if ((fabs(interval) > 0.5) || (prevw == 0)) {
			MoveToEx(hdc_mem, x, y, NULL);
		}
		else {
			if (x != prevx || y != prevy) {
				LineTo(hdc_mem, x, y);
			}
		}
		prevx = x;
		prevy = y;
		prevw = w;
	}
	SelectObject(hdc_mem, hPenOrig);
	TextOutW(hdc_mem, static_cast<int>(gp.Area.left) + 2, static_cast<int>(gp.Area.top), gp.name.c_str(), static_cast<int>(gp.name.size()));
}





void iRec::W_TimeLine::SetGraphText(wstring& method, wstring& unit)
{
	switch (iRecPtr->OutputParam.method)
	{
	case Output_Parameter::PC_METHOD:
		method = L" PC method ";
		switch (iRecPtr->ImgProcParam.pc.mode) {
		case 0:
			method += L"Pupil Center ";
			unit = L"[pixel]";
			break;
		case 1:
			method += L"Camera coordinates ";
			unit = L"[deg]";
			break;
		case 2:
			method += L"Object coordinates ";
			unit = L"[deg]";
			break;
		}
		break;
	case Output_Parameter::PR_METHOD:
		method = L" PR method ";
		switch (iRecPtr->ImgProcParam.pr.mode) {
		case 0:
			method += L"Pupil Center";
			unit = L"[pixel]";
			break;
		case 1:
			method += L"Camera coordinates ";
			unit = L"[deg]";
			break;
		case 2:
			method += L"Object coordinates ";
			unit = L"[deg]";
			break;
		}
		break;
	case Output_Parameter::RC_METHOD:
		method = L" RC method ";
		switch (iRecPtr->ImgProcParam.rc.mode) {
		case 0:
			method += L"Reflection Center ";
			unit = L"[pixel]";
			break;
		case 1:
			method += L"Camera coordinates ";
			unit = L"[deg]";
			break;
		case 2:
			method += L"Object coordinates ";
			unit = L"[deg]";
			break;
		}
		break;
	}
}


void iRec::W_TimeLine::OutputGraph(INT64 co, bool IsRedRect, FixationSection& fs)
{
	if (co < 5) return;

	GraphPlotX gpp;
	GraphBaseInfo gbix, gbiy;
	GraphParam gp;

	wstring wstr_deg_x, wstr_deg_y, wstr_p_l,wstr_openratio, wstr_method, wstr_unit;
	vector<double> out_deg_x, out_deg_y, pupil_l,openratio;

	int  x;
	INT64 ic;
	MaxMin mmx, mmy, mml,mmo;

	gp.Area = GraphArea;
	int gap = 14;
	int graphnum = 4;
	int graphheight = GraphArea.bottom - GraphArea.top;
	int each_graph_height = (graphheight - gap * (graphnum - 1)) / graphnum;

	gp.Area.bottom = gp.Area.top + each_graph_height;
	SetGraphBaseInfoX(gp.x, gp.Area.right - gp.Area.left);


	for (ic = co, x = gp.Area.right; ic >= 0 && (x > gp.Area.left); ic--) {
		gpp.counter = ic;
		gpp.time = (iRecPtr->Eye.v[ic].TimeStamp - iRecPtr->Eye.v[co].TimeStamp) / 1e9;
		x = gp.Area.right + static_cast<int>(gpp.time * gp.x.UnitPixel);
		gpp.pupil_w = iRecPtr->Eye.v[ic].pupil.size.w;
		gpp.ref_w = iRecPtr->Eye.v[ic].ref.size.w;
		gp.ti.push_back(gpp);

		out_deg_x.push_back(iRecPtr->Eye.v[ic].Gaze.x);
		out_deg_y.push_back(iRecPtr->Eye.v[ic].Gaze.y);
		pupil_l.push_back(iRecPtr->Eye.v[ic].Gaze.l);
		openratio.push_back(iRecPtr->Eye.v[ic].Gaze.openratio);

		if (gpp.pupil_w == 0) continue;
		mmx.store(iRecPtr->Eye.v[ic].Gaze.x);
		mmy.store(iRecPtr->Eye.v[ic].Gaze.y);
		mml.store(iRecPtr->Eye.v[ic].Gaze.l);
		mmo.store(iRecPtr->Eye.v[ic].Gaze.openratio);
	}
	DetectFixation(co);
	SetFixation(gp, co);
	fs.S_co = fs.E_co;
	if (IsRedRect) {
		SetCalibSection(gp, fs);
	}

	SetGraphText(wstr_method, wstr_unit);
	if (iRecPtr->Eye.v[co].pupil.size.w != 0) {
		wstr_deg_x = (boost::wformat(L"% 5.1f") % iRecPtr->Eye.v[co].Gaze.x).str();
		wstr_deg_y = (boost::wformat(L"% 5.1f") % iRecPtr->Eye.v[co].Gaze.y).str();
		wstr_p_l = (boost::wformat(L"% 5.1f") % iRecPtr->Eye.v[co].Gaze.l).str();
		wstr_openratio = (boost::wformat(L"% 5.1f") % iRecPtr->Eye.v[co].Gaze.openratio).str();
	}
	else {
		wstr_deg_x = L"---";
		wstr_deg_y = L"---";
		wstr_p_l = L"---";
		wstr_openratio = L"---";
	}


	SetGraphBaseInfoY(gbix, each_graph_height, mmx.max, mmx.min);
	SetGraphBaseInfoY(gbiy, each_graph_height, mmy.max, mmy.min);
	SyncYstep(gbix, gbiy);

	gp.y = gbix;
	gp.name = wstring(L"Gaze_H") + wstr_deg_x + wstr_unit + wstr_method ;
	GraphBase(gp);
	GraphPaintFixation(gp,0,co);
	GraphMark(gp, fs);
	GraphPlot(gp, out_deg_x);

	gp.Area.top = gp.Area.bottom + gap;
	gp.Area.bottom = gp.Area.top + each_graph_height;
	gp.y = gbiy;
	gp.name = wstring(L"Gaze_V") + wstr_deg_y + wstr_unit + wstr_method ;
	GraphBase(gp);
	GraphPaintFixation(gp,1,co);
	GraphMark(gp, fs);
	GraphPlot(gp, out_deg_y);

	gp.Area.top = gp.Area.bottom + gap;
	gp.Area.bottom = gp.Area.top + each_graph_height;
	SetGraphBaseInfoY(gp.y, each_graph_height, mml.max, mml.min);
	gp.name = wstring(L"Pupil radius ") + wstr_p_l + wstring(L"[pixel]");
	GraphBase(gp);
	GraphMark(gp, fs);
	GraphPlot(gp, pupil_l);

	gp.Area.top = gp.Area.bottom + gap;
	gp.Area.bottom = gp.Area.top + each_graph_height;
	SetGraphBaseInfoY(gp.y, each_graph_height, 100, 0);
	gp.name = wstring(L"EyeOpenRatio ") + wstr_openratio + wstring(L"[%%]");
	GraphBase(gp);
	GraphMark(gp, fs);
	GraphAnnotationHorizontalLine(gp, iRecPtr->ImgProcParam.BlinkThreshold);
	GraphPlot(gp, openratio);




	GraphScaleX(gp);
}

LRESULT   iRec::W_TimeLine::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	INT64 iCounter;
	static bool timerflag = true;
	static FixationSection fs;
	switch (message) {

	case WM_CREATE:
		hdc = GetDC(hWnd);
		hdc_mem = CreateCompatibleDC(hdc);

		hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, TEXT("Consolas"));
		hOrigFont = (HFONT)SelectObject(hdc_mem, hFont);

		SetStretchBltMode(hdc_mem, COLORONCOLOR);
		hBmpMem = CreateCompatibleBitmap(hdc,MaxTimeLine_Width, MaxTimeLine_Height);
		hOrigBmpMem = (HBITMAP)SelectObject(hdc_mem, hBmpMem);

		hPenBlack = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		hPenBlue1 = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
		hPenBlue2 = CreatePen(PS_SOLID, 1, RGB(200, 200, 230));
		hPenBlue3 = CreatePen(PS_SOLID, 1, RGB(230, 230, 230));

		hPenGrey = CreatePen(PS_SOLID, 1, RGB(192, 192, 192));

		hPenRed1 = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		hPenRed2 = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		hPenRed3 = CreatePen(PS_SOLID, 1, RGB(128, 0, 0));

		hPenGreen1 = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
		hPenGreen2 = CreatePen(PS_SOLID, 1, RGB(0, 128, 0));

		hPenYellow = CreatePen(PS_SOLID, 1, RGB(255, 183, 76));

		hBrushNull = (HBRUSH)GetStockObject(NULL_BRUSH);

		hOrigPen = (HPEN)SelectObject(hdc, hPenRed1);

		ReleaseDC(hWnd, hdc);

		SetTimer(hWnd, ID_TIMELINE_TIMER, TIMELINE_INTERVAL, NULL);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_TIMER:
		if (wParam != ID_TIMELINE_TIMER) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		if (!timerflag) {
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iCounter = iRecPtr->FrameCounter;
		}

		GraphClear();
		OutputGraph(iCounter, LButtonDown, fs);

		if (LButtonDown) {
			if (fs.S_co < fs.E_co) {
				timerflag = false;
				iRecPtr->PauseCaptureThread();
				iRecPtr->nCalib->Open(fs);
				iRecPtr->ResumeCaptureThread();
				timerflag = true;
			}
			LButtonDown = false;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		iRecPtr->nCalib->PassiveCalibration(iCounter, 5000);

		break;
		//case WM_COMMAND:
		//{
		//	int wmId = LOWORD(wParam);
		//	int wmEvent = HIWORD(wParam);
		//	switch (wmId)
		//	{

		//	default:
		//		return DefWindowProc(hWnd, message, wParam, lParam);
		//	}
		//}
		//break;
	case WM_LBUTTONDOWN:
		fs.position = LOWORD(lParam);
		{
			int isMap = 0;
			for (int i = 0; i < MapPosNum; i++) {
				if (iRecPtr->CalibrationParam.map[i * 5 + 2] != 0) isMap++;
			}
			if (isMap == 0) break;
		}
		LButtonDown = true;
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		break;
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
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
		KillTimer(hWnd, ID_TIMELINE_TIMER);

		SelectObject(hdc_mem, hOrigBmpMem);
		SelectObject(hdc_mem, hOrigBrush);
		SelectObject(hdc_mem, hOrigPen);
		SelectObject(hdc_mem, hOrigFont);

		DeleteObject(hPenBlack);
		DeleteObject(hBmpMem);
		DeleteObject(hFont);

		DeleteObject(hPenRed1);
		DeleteObject(hPenRed2);
		DeleteObject(hPenRed3);

		DeleteObject(hPenBlue1);
		DeleteObject(hPenBlue2);
		DeleteObject(hPenBlue3);

		DeleteObject(hPenGreen1);
		DeleteObject(hPenGreen2);

		DeleteObject(hPenYellow);
		DeleteObject(hPenGrey);

		DeleteDC(hdc_mem);

		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}