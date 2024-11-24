#include "framework.h"
#include "iRecHS2s.h"



#define ID_XYGRAPH_TIMER 1200
#define XYGRAPH_INTERVAL 33
#define FrameLeft 50
#define FrameRight 25
#define FrameTop 25
#define FrameBottom 50
#define MaxXYGraph_W 1024
#define MaxXYGraph_H 1024


void iRec::W_XY_Graph::Resize(void)
{
	int x, y, w, h;
	RECT rect, rectParent, rectCamView;
	GetClientRect(hWndParent, &rectParent);
	GetWindowRect(iRecPtr->wCamView->hWnd, &rectCamView);

	w = (rectCamView.right - rectCamView.left);
	h = (rectParent.bottom - rectParent.top) - (rectCamView.bottom - rectCamView.top) - 10 - Status_Offset;
	if (h < 480) {
		ShowWindow(hWnd, SW_HIDE);
		return;
	}
	else {
		if (!IsWindowVisible(hWnd))
			ShowWindow(hWnd, SW_SHOW);
	}

	if (w < 480) w = 480;
	if (w > h) w = h;
	else h = w;

	x = 0;
	y = (rectParent.bottom - rectParent.top-Status_Offset) - h;
	
	

	MoveWindow(hWnd, x, y, w, h, true);
	GetClientRect(hWnd, &rect);
	winsize.w = rect.right - rect.left;
	winsize.h = rect.bottom - rect.top;
	if (winsize.w > MaxXYGraph_W) {
		winsize.w = MaxXYGraph_W;
	}
	if (winsize.h > MaxXYGraph_H) {
		winsize.h = MaxXYGraph_H;
	}

	GraphArea.left = FrameLeft;
	GraphArea.right = winsize.w - FrameRight;
	GraphArea.top = FrameTop;
	GraphArea.bottom = winsize.h - FrameBottom;
}





iRec::W_XY_Graph::W_XY_Graph(HWND parent, iRec& ir, int x, int y, int w, int h)
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
	GraphMode = 1;

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
	wcex.lpszClassName = TEXT("W_XY_Graph");
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

	hWnd = CreateWindow(TEXT("W_XY_Graph"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		x, y, rect.right - rect.left, rect.bottom - rect.top, hWndParent, NULL, hInst, this);

	PosSize.left = x;
	PosSize.right = x + rect.right - rect.left;
	PosSize.top = y;
	PosSize.bottom = y + rect.bottom - rect.top;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
    Resize();
}


iRec::W_XY_Graph::~W_XY_Graph()
{

}
LRESULT CALLBACK iRec::W_XY_Graph::CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	W_XY_Graph* thisPtr = NULL;

	switch (message) {
	case WM_CREATE:
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
		thisPtr = (W_XY_Graph*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		break;
	case WM_DESTROY:
		thisPtr = (W_XY_Graph*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		break;
	default:
		thisPtr = (W_XY_Graph*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		break;
	}
	if (thisPtr) {
		return thisPtr->WndProc(hWnd, message, wParam, lParam);
	}
	return (DefWindowProc(hWnd, message, wParam, lParam));
}

void iRec::W_XY_Graph::GraphClear(void)
{
	HBRUSH hBrushGrey, hBrushOrig;
	hBrushGrey = CreateSolidBrush(RGB(0xf0, 0xf0, 0xf0));
	hBrushOrig = (HBRUSH)SelectObject(hdc_mem, hBrushGrey);
	PatBlt(hdc_mem, 0, 0, MaxXYGraph_W, MaxXYGraph_H, PATCOPY);
	DeleteObject(SelectObject(hdc_mem, hBrushOrig));
}

bool iRec::W_XY_Graph::IsMajorStep(double pos, double step)
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

void iRec::W_XY_Graph::SyncStep(GraphBaseInfo& src, GraphBaseInfo& des)
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

void iRec::W_XY_Graph::SetGraphBaseInfo(GraphBaseInfo& gbi, int range, double max, double min)
{
	double MiStepH[] = {0.1,0.2,0.5, 1 ,2, 5,10,20,50,100,200,500,1000,2000,5000 };
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
		if (gbi.MinorPixelStep > 10) break;
	}
}

void iRec::W_XY_Graph::GraphBase(GraphParam& gp)
{
	wstring ws;

	HBRUSH hBrushOrig;

	HPEN hPenOrig;
	int ih,iw;
	double dh,dw;

	hPenOrig = (HPEN)SelectObject(hdc_mem, hPenBlack);
	Rectangle(hdc_mem, gp.Area.left, gp.Area.top, gp.Area.right + 1, gp.Area.bottom + 1);

	(HPEN)SelectObject(hdc_mem, hPenBlue3);

	for (dw = gp.x.Offset + gp.x.MinorStep, iw = gp.Area.left; iw < gp.Area.right; dw += gp.x.MinorStep) {
		MoveToEx(hdc_mem, iw,gp.Area.top+ 1, NULL);
		LineTo(hdc_mem, iw,gp.Area.bottom);
		iw = gp.Area.left + static_cast<int>((dw - gp.x.Offset) * gp.x.UnitPixel);
	}


	for (dh = gp.y.Offset + gp.y.MinorStep, ih = gp.Area.bottom; ih > gp.Area.top; dh += gp.y.MinorStep) {
		MoveToEx(hdc_mem, gp.Area.left + 1, ih, NULL);
		LineTo(hdc_mem, gp.Area.right, ih);
		ih = gp.Area.bottom - static_cast<int>((dh - gp.y.Offset) * gp.y.UnitPixel);
	}

	(HPEN)SelectObject(hdc_mem, hPenBlue2);

	for (dw = gp.x.Offset + gp.x.MinorStep, iw = gp.Area.left; iw < gp.Area.right; ) {
		if (IsMajorStep(dw, gp.x.MajorStep)) {
			MoveToEx(hdc_mem, iw, gp.Area.top + 1, NULL);
			LineTo(hdc_mem, iw, gp.Area.bottom);
		}
		dw += gp.x.MinorStep;
		iw = gp.Area.left + static_cast<int>((dw - gp.x.Offset) * gp.y.UnitPixel);
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

	for (dw = gp.x.Offset, iw = gp.Area.left; iw < gp.Area.right; ) {
		if (iw - 20 > gp.Area.right) break;
		if (IsMajorStep(dw, gp.x.MajorStep)) {
			ws = (boost::wformat(L"%5.1f") % dw).str();
			TextOutW(hdc_mem, iw-18,static_cast<int>(gp.Area.bottom) +10, ws.c_str(), static_cast<int>(ws.size()));
		}
		dw += gp.x.MinorStep;
		iw = gp.Area.left + static_cast<int>((dw - gp.x.Offset) * gp.x.UnitPixel);
	}

	for (dh = gp.y.Offset, ih = gp.Area.bottom; ih > gp.Area.top; ) {
		if (ih +5 < gp.Area.top) break;
		if (IsMajorStep(dh, gp.y.MajorStep)) {
			ws = (boost::wformat(L"%5.1f") % dh).str();
			TextOutW(hdc_mem, static_cast<int>(gp.Area.left) - 40, ih - 10, ws.c_str(), static_cast<int>(ws.size()));
		}
		dh += gp.y.MinorStep;
		ih = gp.Area.bottom - static_cast<int>((dh - gp.y.Offset) * gp.y.UnitPixel);
	}
}
#define TraceMaxIntensity 255
#define TraceMinIntensity 0
#define TRACE_POINTS 5000

bool iRec::W_XY_Graph::CalcCalibratedPoint(int i,int method, double& x, double& y)
{
	if (i < 0) return false;
	if (i >= MapPosNum) return false;
	auto po = iRecPtr->CalibrationParam.pd[i];
	GazeVec gv;
	GazeAngle gca,goa;
	switch (method) {
	case Output_Parameter::PR_METHOD:
		if (!iRecPtr->ImgProcParam.pr.camVec(po, gv)) return false;
		if (!iRecPtr->ImgProcParam.pr.cam_vec2ang(gv, iRecPtr->OutputParam.Flip, gca)) return false;
		if (iRecPtr->ImgProcParam.pr.mode_enabled < 4) {
			iRecPtr->ImgProcParam.pr.objAng1(gca, goa); break;
		}
		if (iRecPtr->ImgProcParam.pr.mode_enabled == 4) {
			if (!iRecPtr->ImgProcParam.pr.camVec2objAng(gv, goa)) break;
		}
		break;
	case Output_Parameter::PC_METHOD:
		if (!iRecPtr->ImgProcParam.pc.camVec(po, gv)) return false;
		if (!iRecPtr->ImgProcParam.pc.cam_vec2ang(gv, iRecPtr->OutputParam.Flip, gca)) return false;;
		if (iRecPtr->ImgProcParam.pc.mode_enabled < 4) {
			iRecPtr->ImgProcParam.pc.objAng1(gca, goa); break;
		}
		if (iRecPtr->ImgProcParam.pc.mode_enabled == 4) {
			if (!iRecPtr->ImgProcParam.pc.camVec2objAng(gv, goa)) break;
		}
		break;
	case Output_Parameter::RC_METHOD:
		if (!iRecPtr->ImgProcParam.rc.camVec(po, gv)) return false;
		if (!iRecPtr->ImgProcParam.rc.cam_vec2ang(gv, iRecPtr->OutputParam.Flip, gca)) return false;
		if (iRecPtr->ImgProcParam.rc.mode_enabled < 4) {
			iRecPtr->ImgProcParam.rc.objAng1(gca, goa); break;
		}
		if (iRecPtr->ImgProcParam.rc.mode_enabled == 4) {
			if (!iRecPtr->ImgProcParam.rc.camVec2objAng(gv, goa)) break;
		}
		break;
	}
	x = goa.x;
	y = goa.y;
	return true;
}

void iRec::W_XY_Graph::GraphCalibratedPoints(GraphParam& gp, int method)
{
	HPEN hPenOrig;
	HBRUSH hBrushOrig;
	hPenOrig = (HPEN)SelectObject(hdc_mem, hPenBlack);
	hBrushOrig = (HBRUSH)SelectObject(hdc_mem, hBrushNull);
	double x, y;
	for (int i = 0; i < MapPosNum; i++) {
		if (iRecPtr->CalibrationParam.pd[i].num == 0) continue;
		if (!CalcCalibratedPoint(i, method, x, y)) continue;
		int cur_x = (int)(gp.Area.left + (x - gp.x.Offset) * gp.x.UnitPixel + 0.5);
		int cur_y = (int)(gp.Area.bottom - (y- gp.y.Offset) * gp.y.UnitPixel + 0.5);
		Ellipse(hdc_mem, cur_x - 5, cur_y - 5, cur_x + 6, cur_y + 6);

	}
	SelectObject(hdc_mem, hBrushOrig);
	SelectObject(hdc_mem, hPenOrig);
}
void iRec::W_XY_Graph::GraphPlot(GraphParam& gp, vector<double>& data_x,vector<double>&data_y)
{
	int  x, y,col;
	double col_steps, d_col;
	HPEN hPenOrig;
	HBRUSH hBrushOrig;
	wstring ws;
	if ((data_x.size() == 0) || (data_y.size() == 0)) return;

	hPenOrig = (HPEN)SelectObject(hdc_mem, hPenRed1);
	hBrushOrig = (HBRUSH)SelectObject(hdc_mem, hBrushNull);

	//BITMAP bmBitmap;
	std::vector<double>::reverse_iterator dx;
	std::vector<double>::reverse_iterator dy;

	//std::vector<double>::iterator dx;
	//std::vector<double>::iterator dy;

	//GetObject(hdc_mem, sizeof(BITMAP), &bmBitmap);
	
	col_steps=(double)(double)(TraceMaxIntensity - TraceMinIntensity) / (double)TRACE_POINTS;

	
	int cur_x = (int)(gp.Area.left + (data_x[0] - gp.x.Offset) * gp.x.UnitPixel + 0.5);
	int cur_y = (int)(gp.Area.bottom - (data_y[0] - gp.y.Offset) * gp.y.UnitPixel + 0.5);
	Ellipse(hdc_mem, cur_x - 5, cur_y - 5, cur_x + 6, cur_y + 6);

	for (dx = data_x.rbegin(), dy = data_y.rbegin(),d_col= (double)TraceMaxIntensity-col_steps*(TRACE_POINTS-data_x.size()); dx != data_x.rend() && dy != data_y.rend(); dx++, dy++,d_col-=col_steps) {

		x = gp.Area.left   + static_cast<int>((*dx - gp.x.Offset) * gp.x.UnitPixel);
		y = gp.Area.bottom - static_cast<int>((*dy - gp.y.Offset) * gp.y.UnitPixel);
		col = static_cast<int>(d_col);
		if (x >= gp.Area.right) continue;
		if (x <= gp.Area.left) continue;
		if (y >= gp.Area.bottom) continue;
		if (y <= gp.Area.top) continue;
		
		SetPixelV(hdc_mem, x, y, RGB(col, col, col));

	}
	
	for (int i = 0; i < MapPosNum; i++) {
		if (iRecPtr->CalibrationParam.map[i * 5 + 2] == 0) continue;
		double hh1 = iRecPtr->CalibrationParam.map[i * 5 + 3] - 1;
		double hh2 = iRecPtr->CalibrationParam.map[i * 5 + 3] + 1;
		double vv1 = iRecPtr->CalibrationParam.map[i * 5 + 4] - 1;
		double vv2 = iRecPtr->CalibrationParam.map[i * 5 + 4] + 1;
		int h1 = gp.Area.left + static_cast<int>((hh1 - gp.x.Offset) * gp.x.UnitPixel);
		int h2 = gp.Area.left + static_cast<int>((hh2 - gp.x.Offset) * gp.x.UnitPixel);
		int v1 = gp.Area.bottom - static_cast<int>((vv1 - gp.y.Offset) * gp.y.UnitPixel);
		int v2 = gp.Area.bottom - static_cast<int>((vv2 - gp.y.Offset) * gp.y.UnitPixel);
		if ((h1 >= gp.Area.right) || (h2 >= gp.Area.right))continue;
		if ((h1 <= gp.Area.left) || (h2 <= gp.Area.left)) continue;
		if ((v1 >= gp.Area.bottom)|| (v2 >= gp.Area.bottom)) continue;
		if ((v1 <= gp.Area.top) || (v2 <= gp.Area.top)) continue;
		ws = (boost::wformat(L"%d") % (i+1)).str();
		Rectangle(hdc_mem, h1, v1, h2, v2);
		TextOutW(hdc_mem, h2, v2, ws.c_str(), static_cast<int>(ws.size()));
	}



	SelectObject(hdc_mem, hBrushOrig);
	SelectObject(hdc_mem, hPenOrig);
//	TextOutW(hdc_mem, static_cast<int>(gp.Area.left) + 2, static_cast<int>(gp.Area.top), gp.name.c_str(), static_cast<int>(gp.name.size()));
}


void iRec::W_XY_Graph::OutputGraph(INT64 co, bool IsRedRect)
{
	if (co < 5) return;

	GraphBaseInfo gbix, gbiy;
	GraphParam gp;

//	wstring wstr_deg_x, wstr_deg_y, wstr_p_l, wstr_method, wstr_unit;
	vector<double> out_deg_x, out_deg_y;

	int  x;
	INT64 ic;
	MaxMin mmx, mmy;

	gp.Area = GraphArea;


	auto current_mode = iRecPtr->Eye.v[co].Gaze.mode;
	auto current_method = iRecPtr->Eye.v[co].Gaze.method;

	for (ic = co, x = 0; ic >= 0 && x< TRACE_POINTS; ic--,x++) {
		if (iRecPtr->Eye.v[ic].pupil.size.w == 0) continue;
		if (iRecPtr->Eye.v[ic].Gaze.mode != current_mode) continue;
		out_deg_x.push_back(iRecPtr->Eye.v[ic].Gaze.x);
		out_deg_y.push_back(iRecPtr->Eye.v[ic].Gaze.y);
		if (current_mode==0 || current_mode==1 || GraphMode == 1 || GraphMode == 2) {
			mmx.store(iRecPtr->Eye.v[ic].Gaze.x);
			mmy.store(iRecPtr->Eye.v[ic].Gaze.y);
		}
	}
	if (current_mode==2  && (GraphMode == 0 || GraphMode == 1)) {
		for (int i = 0; i < MapPosNum; i++) {
			if (iRecPtr->CalibrationParam.map[i * 5 + 2] == 0) continue;
			double h1 = iRecPtr->CalibrationParam.map[i * 5 + 3] + 1;
			double h2 = iRecPtr->CalibrationParam.map[i * 5 + 3] - 1;
			mmx.store(h1);
			mmx.store(h2);
			double v1 = iRecPtr->CalibrationParam.map[i * 5 + 4] + 1;
			double v2 = iRecPtr->CalibrationParam.map[i * 5 + 4] - 1;
			mmy.store(v1);
			mmy.store(v2);
		}
	}

	SetGraphBaseInfo(gbix, gp.Area.right - gp.Area.left, mmx.max, mmx.min);
	SetGraphBaseInfo(gbiy, gp.Area.bottom - gp.Area.top, mmy.max, mmy.min);
	SyncStep(gbix, gbiy);
	gp.x = gbix;
	gp.y = gbiy;
	//gp.name = wstring(L"Gaze_H") + wstr_deg_x + wstr_unit + wstr_method;
	GraphBase(gp);
	//GraphMark(gp, fs);
	GraphPlot(gp, out_deg_x,out_deg_y);
	if (current_mode == 2 && (GraphMode == 0 || GraphMode == 1)) {
		GraphCalibratedPoints(gp, current_method);
	}
	//GraphScaleX(gp);
}

LRESULT   iRec::W_XY_Graph::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	INT64 iCounter;

//	static FixationSection fs;
	switch (message) {

	case WM_CREATE:
		hdc = GetDC(hWnd);
		hdc_mem = CreateCompatibleDC(hdc);

		hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, TEXT("Consolas"));
		hOrigFont = (HFONT)SelectObject(hdc_mem, hFont);

		SetStretchBltMode(hdc_mem, COLORONCOLOR);
		hBmpMem = CreateCompatibleBitmap(hdc, MaxXYGraph_W, MaxXYGraph_H);
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

		SetTimer(hWnd, ID_XYGRAPH_TIMER, XYGRAPH_INTERVAL, NULL);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_TIMER:
		if (wParam != ID_XYGRAPH_TIMER) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iCounter = iRecPtr->FrameCounter;
		}

		GraphClear();
		OutputGraph(iCounter, LButtonDown);



		InvalidateRect(hWnd, NULL, FALSE);
	

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
//		fs.position = LOWORD(lParam);
//		LButtonDown = true;
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
		KillTimer(hWnd, ID_XYGRAPH_TIMER);

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