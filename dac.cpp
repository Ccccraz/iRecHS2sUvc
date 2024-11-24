#include "framework.h"
#include "iRecHS2s.h"


#define USE_DACONVERTER

#ifdef USE_DACONVERTER
#pragma comment(lib, "caio.lib")
#include <Caio.h>
#define DACMAX (10)
#define DACMIN (-10)
#define BLINK_MAX 200


std::wstring iRec::DacInfo::NameModel2listname(std::wstring& name, std::wstring& model)
{
	std::wstring nm = name + std::wstring(L" ") + model;
	return nm;
}

std::wstring iRec::DacInfo::Num2listname(int n)
{
	if (n >= num) return std::wstring(L"");
	return NameModel2listname(cards[n].name, cards[n].model);
}

int  iRec::DacInfo::Listname2num(std::wstring name_model)
{
	int n;
	for (n = 0; n < num; n++) {
		if (Num2listname(n) == name_model) {
			return n;
		}
	}
	return -1;
}

iRec::DacInfo::DacInfo()
{
	char name[DAC_MAX_LOADSTRING];
	char model[DAC_MAX_LOADSTRING];
	short ID;
	short AoMaxChannels;
	DacCard card;

	num = 0;
	for (int i = 0; i < DAC_MAX_CARD; i++) {
		if (AioQueryDeviceName(i, name, model) != 0) break;

		card.model = utf8_decode(std::string(model));
		card.name = utf8_decode(std::string(name));

		if (AioInit(name, &ID) != 0) {
			continue;
		}

		if ((AioGetAoMaxChannels(ID, &AoMaxChannels) != 0) || (AoMaxChannels < 4)) {
			AioExit(ID);
			continue;
		}
		num++;
		AioExit(ID);
		card.model = utf8_decode(std::string(model));
		card.name = utf8_decode(std::string(name));

		card.channels = static_cast<int>(AoMaxChannels);
		cards.push_back(card);
	}

}

iRec::DacInfo::~DacInfo()
{

}


iRec::DlgDac::DlgDac(HWND hWnd, iRec& ir)
{
	hWndParent = hWnd;
	iRecPtr = &ir;
	dp = &(iRecPtr->DacParam);

	di.reset(new DacInfo());
	IsDacOpen = false;
	AioID = 0;
	DACBITS = 0;
	MaxDigit = 0;
	for (int i=0; i < DAC_MAX_CHANNEL; i++) {
		ch[i] = 0;
	}
}



iRec::DlgDac::~DlgDac()
{
	if (IsDacOpen) {
		OutZero();
		IsDacOpen = false;
		AioExit(AioID);
	}
}


INT_PTR iRec::DlgDac::Open()
{
	INT_PTR iptr;
	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DAC), hWndParent, CallbackProc, (LPARAM)this);
	return iptr;
}

bool iRec::DlgDac::InitCtrl(HWND hDlg)
{
	wstring ws;
	int no;
	if (di->num == 0) return false;


	cl_DacCard.reset(new DlgComboList(hDlg, IDC_COMBO_DAC_CARD));

	for (int i = 0; i < di->num; i++) {
		cl_DacCard->add(di->Num2listname(i));
	}
	no = di->Listname2num(di->NameModel2listname(dp->name, dp->model));
	AioID = no;
	if (no == -1) {
		AioID = 0;
		cl_DacCard->Set(AioID);
		dp->name = di->cards[AioID].name;
		dp->model = di->cards[AioID].model;
	}
	else {
		ws = di->Num2listname(no);
		cl_DacCard->Set(ws);
	}

	DacInit();

	cl_DacFailure.reset(new DlgComboList(hDlg, IDC_COMBO_FAILURE));
	ws = wstring(L"MAX");
	cl_DacFailure->add(ws);
	ws = wstring(L"ZERO");
	cl_DacFailure->add(ws);
	ws = wstring(L"MIN");
	cl_DacFailure->add(ws);
	ws = wstring(L"HOLD");
	cl_DacFailure->add(ws);
	
	cl_DacFailure->Set(dp->failuremode);

	lb_Pixel.reset(new DlgLabel(hDlg, IDC_TEXT_PIXEL));
	sl_Pixel_h.reset(new DlgSlider(hDlg, IDC_SLIDER_PIXEL_H, 0, 1000, static_cast<int>(dp->pixel_h * 10), 100, 100));
	sl_Pixel_v.reset(new DlgSlider(hDlg, IDC_SLIDER_PIXEL_V, 0, 1000, static_cast<int>(dp->pixel_v * 10), 100, 100));
	ws = (boost::wformat(L"0 volt position (H,V)=(%3.1f, %3.1f) [%%]") % dp->pixel_h % dp->pixel_v).str();
	lb_Pixel->SetText(ws);

	lb_Pixel_amp.reset(new DlgLabel(hDlg, IDC_TEXT_PIXEL_AMP));
	sl_Pixel_amp.reset(new DlgSlider(hDlg, IDC_SLIDER_PIXEL_AMP, 1, 100, static_cast<int>(dp->pixel_amp), 10, 10));
	ws = (boost::wformat(L"Amplitude %d [pixel/volt]") % static_cast<int>(dp->pixel_amp)).str();
	lb_Pixel_amp->SetText(ws);

	lb_Cam.reset(new DlgLabel(hDlg, IDC_TEXT_CAMANG));
	sl_Cam_h.reset(new DlgSlider(hDlg, IDC_SLIDER_CAMANG_H, -900, 900, static_cast<int>(dp->cam_h*10), 100, 100));
	sl_Cam_v.reset(new DlgSlider(hDlg, IDC_SLIDER_CAMANG_V, -900, 900, static_cast<int>(dp->cam_v*10), 100, 100));
	ws = (boost::wformat(L"0 volt position (H,V)=(%3.1f, %3.1f) [deg]") % dp->cam_h % dp->cam_v).str();
	lb_Cam->SetText(ws);

	lb_Cam_amp.reset(new DlgLabel(hDlg, IDC_TEXT_CAMANG_AMP));
	sl_Cam_amp.reset(new DlgSlider(hDlg, IDC_SLIDER_CAMANG_AMP, 1, 100, static_cast<int>(dp->cam_amp*10), 10, 10));
	ws = (boost::wformat(L"Amplitude %3.1f [deg/volt]") % dp->cam_amp).str();
	lb_Cam_amp->SetText(ws);

	lb_Obj.reset(new DlgLabel(hDlg, IDC_TEXT_OBJANG));
	sl_Obj_h.reset(new DlgSlider(hDlg, IDC_SLIDER_OBJANG_H, -900, 900, static_cast<int>(dp->obj_h*10), 100, 100));
	sl_Obj_v.reset(new DlgSlider(hDlg, IDC_SLIDER_OBJANG_V, -900, 900, static_cast<int>(dp->obj_v*10), 100, 100));
	ws = (boost::wformat(L"0 volt position (H,V)=(%3.1f, %3.1f) [deg]") % dp->obj_h % dp->obj_v).str();
	lb_Obj->SetText(ws);

	lb_Obj_amp.reset(new DlgLabel(hDlg, IDC_TEXT_OBJANG_AMP));
	sl_Obj_amp.reset(new DlgSlider(hDlg, IDC_SLIDER_OBJANG_AMP, 1, 100, static_cast<int>(dp->obj_amp * 10), 10, 10));
	ws = (boost::wformat(L"Amplitude %3.1f [deg/volt]") % dp->obj_amp).str();
	lb_Obj_amp->SetText(ws);


	lb_Pupil.reset(new DlgLabel(hDlg, IDC_TEXT_PUPIL));
	sl_Pupil.reset(new DlgSlider(hDlg, IDC_SLIDER_PUPIL, -100, 100, static_cast<int>(dp->pupil*10), 10, 10));
	ws = (boost::wformat(L"0 pixel position %3.1f [volt]") % dp->pupil ).str();
	lb_Pupil->SetText(ws);

	lb_Pupil_amp.reset(new DlgLabel(hDlg, IDC_TEXT_PUPIL_AMP));
	sl_Pupil_amp.reset(new DlgSlider(hDlg, IDC_SLIDER_PUPIL_AMP, 1, 100, static_cast<int>(dp->pupil_amp), 10, 10));
	ws = (boost::wformat(L"Amplitude %d [pixel/volt]") % static_cast<int>(dp->pupil_amp)).str();
	lb_Pupil_amp->SetText(ws);

	cb_DacLimitVoltage.reset(new DlgCheckbox(hDlg, IDC_CHECK_VOLTAGE_LIMIT));
	cb_DacLimitVoltage->Set(dp->IsHalf);

	lb_DacInfo.reset(new DlgLabel(hDlg, IDC_TEXT_INFO));
	//ws = (boost::wformat(L"Hello")).str();
	//lb_DacInfo->SetText(ws);
	return true;
}


void iRec::DlgDac::EndCtrl(HWND hDlg)
{

}

bool iRec::DlgDac::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	wstring ws;
	
	switch (message)
	{
	case WM_INITDIALOG:
		if (InitCtrl(hDlg)) return (INT_PTR)true;
		EndDialog(hDlg, false);
		return (INT_PTR)true;

	case WM_HSCROLL:
		if (((HWND)lParam == sl_Pixel_h->hWnd) || ((HWND)lParam == sl_Pixel_v->hWnd)) {
			dp->pixel_h = static_cast<double>(sl_Pixel_h->Get()) / 10.0;
			dp->pixel_v = static_cast<double>(sl_Pixel_v->Get()) / 10.0;
			ws = (boost::wformat(L"0 volt position (H, v)=(%3.1f, %3.1f) [%%]") % dp->pixel_h % dp->pixel_v).str();
			lb_Pixel->SetText(ws);
		}

		if ((HWND)lParam == sl_Pixel_amp->hWnd) {
			dp->pixel_amp = static_cast<double>(sl_Pixel_amp->Get());
			ws = (boost::wformat(L"Amplitude %3d [pixel/volt]") % static_cast<int>(dp->pixel_amp)).str();
			lb_Pixel_amp->SetText(ws);
		}

		if (((HWND)lParam == sl_Cam_h->hWnd) || ((HWND)lParam == sl_Cam_v->hWnd)) {
			dp->cam_h = static_cast<double>(sl_Cam_h->Get()) / 10.0;
			dp->cam_v = static_cast<double>(sl_Cam_v->Get()) / 10.0;
			ws = (boost::wformat(L"0 volt position (H, v)=(%3.1f, %3.1f) [deg]") % dp->cam_h % dp->cam_v).str();
			lb_Cam->SetText(ws);
		}

		if ((HWND)lParam == sl_Cam_amp->hWnd) {
			dp->cam_amp = static_cast<double>(sl_Cam_amp->Get()) / 10.0;
			ws = (boost::wformat(L"Amplitude %3.1f [deg/volt]") % dp->cam_amp).str();
			lb_Cam_amp->SetText(ws);
		}

		if (((HWND)lParam == sl_Obj_h->hWnd) || ((HWND)lParam == sl_Obj_v->hWnd)) {
			dp->obj_h = static_cast<double>(sl_Obj_h->Get()) / 10.0;
			dp->obj_v = static_cast<double>(sl_Obj_v->Get()) / 10.0;
			ws = (boost::wformat(L"0 volt position (H,V)=(%3.1f, %3.1f) [deg]") % dp->obj_h % dp->obj_v).str();
			lb_Obj->SetText(ws);
		}

		if ((HWND)lParam == sl_Obj_amp->hWnd) {
			dp->obj_amp = static_cast<double>(sl_Obj_amp->Get())/10.0;
			ws = (boost::wformat(L"Amplitude %3.1f [deg/volt]") % dp->obj_amp).str();
			lb_Obj_amp->SetText(ws);
		}

		if (((HWND)lParam == sl_Pupil->hWnd)) {
			dp->pupil = static_cast<double>(sl_Pupil->Get()) / 10.0;
			ws = (boost::wformat(L"0 pixel position %3.1f [volt]") % dp->pupil).str();
			lb_Pupil->SetText(ws);
		}

		if ((HWND)lParam == sl_Pupil_amp->hWnd) {
			dp->pupil_amp = static_cast<int>(sl_Pupil_amp->Get());
			ws = (boost::wformat(L"Amplitude %d [pixel/volt]") % static_cast<int>(dp->pupil_amp)).str();
			lb_Pupil_amp->SetText(ws);
		}

		return (INT_PTR)true;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId) {
		case IDC_CHECK_VOLTAGE_LIMIT:
			dp->IsHalf = cb_DacLimitVoltage->Get();
			return (INT_PTR)true;

		case IDC_COMBO_DAC_CARD:
			if (wmEvent == CBN_SELCHANGE) {
				ws = cl_DacCard->GetString();
				int no = di->Listname2num(ws);
				dp->model = di->cards[no].model;
				dp->name = di->cards[no].name;
				AioID = no;
				DacInit();
			}
			return (INT_PTR)true;
		case IDC_COMBO_FAILURE:
			if (wmEvent == CBN_SELCHANGE) {
				dp->failuremode = cl_DacFailure->GetString();
				OutFailure();
			}
			return (INT_PTR)true;
		case IDOK:
			EndCtrl(hDlg);
			EndDialog(hDlg, true);
			return (INT_PTR)true;
		case IDCANCEL:
			EndCtrl(hDlg);
			EndDialog(hDlg, false);
			return (INT_PTR)true;
		}
		return (INT_PTR)false;
	}
	return false;
}

INT_PTR CALLBACK iRec::DlgDac::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DlgDac* thisPtr = NULL;

	switch (message) {
	case WM_INITDIALOG:
		thisPtr = (DlgDac*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
		break;
	case WM_DESTROY:
		thisPtr = (DlgDac*)GetWindowLongPtr(hDlg, DWLP_USER);
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)0);
		break;
	default:
		thisPtr = (DlgDac*)GetWindowLongPtr(hDlg, DWLP_USER);
		break;
	}
	if (thisPtr) {
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}


bool iRec::DlgDac::DacInit()
{
	short aioresolution;
	char name[DAC_MAX_LOADSTRING];
	char model[DAC_MAX_LOADSTRING];

	if (IsDacOpen) AioExit(AioID);

	if ((AioQueryDeviceName(AioID, name, model) != 0) ||
		(AioInit(name, &AioID) != 0)) {
		MessageBox(NULL, TEXT("DAConvert Error"), TEXT("ERROR"), MB_OK);
		return false;
	}
	IsDacOpen = true;

	//bits detect
	AioGetAoResolution(AioID, &aioresolution);
	DACBITS = (int)aioresolution;
	MaxDigit = (1 << DACBITS) - 1;

	// +-10V
	AioSetAoRangeAll(AioID, PM10);

	//転送方式の設定 デバイスバッファーモード 0 のみ可
	AioSetAoTransferMode(AioID, 0);

	//デバイスバッファのメモリ形式 FIFO 0
	AioSetAoMemoryType(AioID, 0);

	//内部クロック 0を使用
	AioSetAoClockType(AioID, 0);

	// Sampling Clock 10microSecond
	AioSetAoSamplingClock(AioID, 10);

	//変換に使用するアナログ出力チャネル数 
	AioSetAoChannels(AioID, DAC_MAX_CHANNEL);

	//変換開始条件の設定 ソフトウェア 0で行う
	AioSetAoStartTrigger(AioID, 0);

	//変換停止条件の設定。設定回数変換終了 0
	AioSetAoStopTrigger(AioID, 0);

	OutZero();

	return true;
}

void iRec::DlgDac::HalfOut(void)
{
	int i;
	for (i = 0; i < DAC_MAX_CHANNEL; i++) {
		ch[i] = (ch[i] >> 1) + (1 << (DACBITS - 2)) - 1;
	}
}

void iRec::DlgDac::OutMax(void)
{
	int i;
	if (!IsDacOpen) return;
	for (i = 0; i < DAC_MAX_CHANNEL; i++) {
		ch[i] = (1 << DACBITS) - 1;
	}
	if (dp->IsHalf)	HalfOut();
	AioMultiAo(AioID, DAC_MAX_CHANNEL, ch);
}

void iRec::DlgDac::OutMin(void)
{
	int i;
	if (!IsDacOpen) return;
	for (i = 0; i < DAC_MAX_CHANNEL; i++) {
		ch[i] = 0;
	}
	if (dp->IsHalf)	HalfOut();
	AioMultiAo(AioID, DAC_MAX_CHANNEL, ch);
}

void iRec::DlgDac::OutZero(void)
{
	int i;
	if (!IsDacOpen) return;
	for (i = 0; i < DAC_MAX_CHANNEL; i++) {
		ch[i] = (1 << (DACBITS - 1)) - 1;
	}

	AioMultiAo(AioID, DAC_MAX_CHANNEL, ch);
}

void iRec::DlgDac::OutFailure(void)
{
	if (dp->failuremode == L"MAX") OutMax();
	if (dp->failuremode == L"ZERO") OutZero();
	if (dp->failuremode == L"MIN") OutMin();
}


void iRec::DlgDac::OutPixel(double x, double y, double r, double t, double ix, double iy)
{

	t = t * 100;

	double ox = ix * dp->pixel_h / 100.0;
	double oy = iy * dp->pixel_v / 100.0;

	double vx = (x - ox) / dp->pixel_amp;
	double vy = (y - oy) / dp->pixel_amp;

	if (vx <= DACMIN) vx = DACMIN;
	if (vx >= DACMAX) vx = DACMAX;
	if (vy <= DACMIN) vy = DACMIN;
	if (vy >= DACMAX) vy = DACMAX;

	double a = (double)MaxDigit / ((double)(DACMAX - DACMIN));
	double b = -a * DACMIN;
	ch[0] = (long)(a * vx + b);
	ch[1] = (long)(a * vy + b);

	double vp = r / dp->pupil_amp + dp->pupil;
	if (vp <= DACMIN) vp = DACMIN;
	if (vp >= DACMAX) vp = DACMAX;
	ch[2] = (long)(a * vp + b);


	if (t > BLINK_MAX) t = BLINK_MAX;
	ch[3] = (long)(t * (double)MaxDigit / (double)BLINK_MAX / 2) + (1 << (DACBITS - 1)) - 1;

	if (dp->IsHalf)	HalfOut();
	AioMultiAo(AioID, DAC_MAX_CHANNEL, ch);
}

void iRec::DlgDac::OutCamAng(double x, double y, double r, double t)
{
	t = t * 100;

	double vx = (x - dp->cam_h) / dp->cam_amp;
	double vy = (y - dp->cam_v) / dp->cam_amp;

	if (vx <= DACMIN) vx = DACMIN;
	if (vx >= DACMAX) vx = DACMAX;
	if (vy <= DACMIN) vy = DACMIN;
	if (vy >= DACMAX) vy = DACMAX;

	double a = (double)MaxDigit / ((double)(DACMAX - DACMIN));
	double b = -a * DACMIN;
	ch[0] = (long)(a * vx + b);
	ch[1] = (long)(a * vy + b);

	double vp = r / dp->pupil_amp + dp->pupil;
	if (vp <= DACMIN) vp = DACMIN;
	if (vp >= DACMAX) vp = DACMAX;
	ch[2] = (long)(a * vp + b);


	if (t > BLINK_MAX) t = BLINK_MAX;
	ch[3] = (long)(t * (double)MaxDigit / (double)BLINK_MAX / 2) + (1 << (DACBITS - 1)) - 1;

	if (dp->IsHalf)	HalfOut();
	AioMultiAo(AioID, DAC_MAX_CHANNEL, ch);
}

void iRec::DlgDac::OutObjAng(double x, double y, double r, double t)
{
	t = t * 100;

	double vx = (x - dp->obj_h) / dp->obj_amp;
	double vy = (y - dp->obj_v) / dp->obj_amp;

	if (vx <= DACMIN) vx = DACMIN;
	if (vx >= DACMAX) vx = DACMAX;
	if (vy <= DACMIN) vy = DACMIN;
	if (vy >= DACMAX) vy = DACMAX;

	double a = (double)MaxDigit / ((double)(DACMAX - DACMIN));
	double b = -a * DACMIN;
	ch[0] = (long)(a * vx + b);
	ch[1] = (long)(a * vy + b);

	double vp = r / dp->pupil_amp + dp->pupil;
	if (vp <= DACMIN) vp = DACMIN;
	if (vp >= DACMAX) vp = DACMAX;
	ch[2] = (long)(a * vp + b);


	if (t > BLINK_MAX) t = BLINK_MAX;
	ch[3] = (long)(t * (double)MaxDigit / (double)BLINK_MAX / 2) + (1 << (DACBITS - 1)) - 1;

	if (dp->IsHalf)	HalfOut();
	AioMultiAo(AioID, DAC_MAX_CHANNEL, ch);
}

void iRec::DlgDac::Out(EyeNumData& ed)
{
	if (!IsDacOpen) return;
	if (ed.pupil.eq.b == 0) {
		OutFailure();
		return;
	}

	switch (ed.Gaze.mode) {
	case 0:
		OutPixel(ed.Gaze.x, ed.Gaze.y, ed.Gaze.l, ed.Gaze.openratio, ed.img.w, ed.img.h);
		break;
	case 1:
		OutCamAng(ed.Gaze.x, ed.Gaze.y, ed.Gaze.l, ed.Gaze.openratio);
		break;
	case 2:
		OutObjAng(ed.Gaze.x, ed.Gaze.y, ed.Gaze.l, ed.Gaze.openratio);
		break;
	}
}

#else

iRec::DlgDac::DlgDac(HWND hWnd, iRec& ir)
{
	hWndParent = hWnd;
	iRecPtr = &ir;
	dp = &(iRecPtr->DacParam);

	//di.reset(new DacInfo());
	IsDacOpen = false;
	AioID = 0;
	DACBITS = 0;
	MaxDigit = 0;
	for (int i = 0; i < DAC_MAX_CHANNEL; i++) {
		ch[i] = 0;
	}
}


INT_PTR iRec::DlgDac::Open(void)
{
	return false;
}

iRec::DlgDac::~DlgDac()
{
}

void iRec::DlgDac::Out(EyeNumData& ed)
{
	return;
}

iRec::DacInfo::DacInfo()
{


}

iRec::DacInfo::~DacInfo()
{

}


#endif