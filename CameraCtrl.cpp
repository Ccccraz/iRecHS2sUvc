#include "framework.h"
#include "iRecHS2s.h"

//FixThreshold
#define FIXTH_ID (HMENU)8
//Threshold slider
#define TS_ID (HMENU)9

#define ID_PermissiveAngle (HMENU)10
#define ID_PermissiveLength (HMENU)11
#define ID_IsTopAcquisition (HMENU)12
#define ID_IsBottomAcquisition (HMENU)13
#define ID_IsGleaningAcquisition (HMENU)14
#define ID_RefThreshold (HMENU)15
#define ID_SearchEdgeLength (HMENU)16
#define ID_PermissiveAngleCheckbox (HMENU)17
#define ID_PermissiveLengthCheckbox  (HMENU)18
#define ID_SearchEdgeLengthCheckbox  (HMENU)19
#define ID_RefThresholdCheckbox (HMENU)20

#define Ctrl_HEIGHT 30
#define Ctrl_WIDTH 250
#define Ctrl_OFFSETY 10
#define Ctrl_OFFSETX 10

ATOM iRec::W_iRecCtrl::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)CallbackProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("iRecCtrls");
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

iRec::W_iRecCtrl::W_iRecCtrl(HWND parent, iRec& ir, int x, int y, int w, int h)
{
	RECT rect;
	int width, height;

	iRecPtr = &ir;
	hWndParent = parent;


	rect.top = rect.left = 0;
	rect.right = w;
	rect.bottom = h;

	MyRegisterClass(hInst);

	AdjustWindowRect(&rect, WS_CHILD | WS_BORDER | WS_VISIBLE, FALSE);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	PosSize.left = x;
	PosSize.right = x + width;
	PosSize.top = y;
	PosSize.bottom = y + height;

	hWnd = CreateWindow(TEXT("iRecCtrls"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		x, y, width, height, parent, NULL, hInst, this);

	wstring ws;
	ws = (boost::wformat(L"Threshold [%1%]") % iRecPtr->ImgProcParam.PupilThreshold).str();
	PupilThresholdAuto = new WinCheckbox(hWnd, FIXTH_ID, ws, false, Ctrl_OFFSETX+Ctrl_WIDTH*0, Ctrl_OFFSETY + Ctrl_HEIGHT * 0);
	PupilThresholdSlider = new WinSlider(hWnd, TS_ID, Ctrl_OFFSETX + Ctrl_WIDTH * 1, Ctrl_OFFSETY + Ctrl_HEIGHT * 0, 1, 255);
	PupilThresholdSlider->Set(iRecPtr->ImgProcParam.PupilThreshold);

	ws = (boost::wformat(L"Reflection Threshold [%1%]") % iRecPtr->ImgProcParam.RefThreshold).str();
	RefThresholdCheckbox = new WinCheckbox(hWnd, ID_RefThresholdCheckbox, ws, false, Ctrl_OFFSETX + Ctrl_WIDTH * 0, Ctrl_OFFSETY + Ctrl_HEIGHT * 1);
	RefThreshold = new WinSlider(hWnd, ID_RefThreshold, Ctrl_OFFSETX + Ctrl_WIDTH * 1, Ctrl_OFFSETY + Ctrl_HEIGHT * 1, 1, 255);
	RefThreshold->Set(iRecPtr->ImgProcParam.RefThreshold);


	ws = (boost::wformat(L"Permissive angle [%1%]deg") % iRecPtr->ImgProcParam.PermissiveAngle).str();
	PupilPermissiveAngleCheckbox = new WinCheckbox(hWnd, ID_PermissiveAngleCheckbox, ws, false, Ctrl_OFFSETX + Ctrl_WIDTH * 0, Ctrl_OFFSETY + Ctrl_HEIGHT * 2);
	PupilPermissiveAngle = new WinSlider(hWnd, ID_PermissiveAngle, Ctrl_OFFSETX + Ctrl_WIDTH * 1, Ctrl_OFFSETY + Ctrl_HEIGHT * 2, 1, 10);
	PupilPermissiveAngle->Set(iRecPtr->ImgProcParam.PermissiveAngle);

	ws = (boost::wformat(L"Permissive length [%1%]") % iRecPtr->ImgProcParam.PermissiveLength).str();
	PupilPermissiveLengthCheckbox = new WinCheckbox(hWnd, ID_PermissiveLengthCheckbox, ws, false, Ctrl_OFFSETX + Ctrl_WIDTH * 0, Ctrl_OFFSETY + Ctrl_HEIGHT * 3);
	PupilPermissiveLength = new WinSlider(hWnd, ID_PermissiveLength, Ctrl_OFFSETX + Ctrl_WIDTH * 1, Ctrl_OFFSETY + Ctrl_HEIGHT * 3, 1, 10);
	PupilPermissiveLength->Set(iRecPtr->ImgProcParam.PermissiveLength);

	ws = (boost::wformat(L"Search edge length [%1%]") % iRecPtr->ImgProcParam.SearchEdgeLength).str();
	PupilSearchEdgeLengthCheckbox = new WinCheckbox(hWnd, ID_SearchEdgeLengthCheckbox, ws, false, Ctrl_OFFSETX + Ctrl_WIDTH * 0, Ctrl_OFFSETY + Ctrl_HEIGHT * 4);
	PupilSearchEdgeLength = new WinSlider(hWnd, ID_SearchEdgeLength, Ctrl_OFFSETX + Ctrl_WIDTH * 1, Ctrl_OFFSETY + Ctrl_HEIGHT * 4, 8, 32);
	PupilSearchEdgeLength->Set(iRecPtr->ImgProcParam.SearchEdgeLength);


	ws = wstring(L"Pupil top fitting");
	PupilIsTopAcquisition = new WinCheckbox(hWnd, ID_IsTopAcquisition, ws, iRecPtr->ImgProcParam.IsTopAcquisition, Ctrl_OFFSETX + Ctrl_WIDTH * 0, Ctrl_OFFSETY + Ctrl_HEIGHT * 5);
	ws = wstring(L"Pupil bottom fitting");
	PupilIsBottomAcquisition = new WinCheckbox(hWnd, ID_IsBottomAcquisition, ws, iRecPtr->ImgProcParam.IsBottomAcquisition, Ctrl_OFFSETX + Ctrl_WIDTH * 0, Ctrl_OFFSETY + Ctrl_HEIGHT * 6);
	ws = wstring(L"Pupil edge gleaning");
	PupilIsGleaningAcquisition = new WinCheckbox(hWnd, ID_IsGleaningAcquisition, ws, iRecPtr->ImgProcParam.IsGleaningAcquisition, Ctrl_OFFSETX + Ctrl_WIDTH * 0, Ctrl_OFFSETY + Ctrl_HEIGHT *7);



	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}
iRec::W_iRecCtrl::~W_iRecCtrl()
{
	delete PupilThresholdSlider;
	delete PupilThresholdAuto;
	delete PupilIsTopAcquisition;
	delete PupilIsBottomAcquisition;
	delete PupilIsGleaningAcquisition;
	delete PupilPermissiveAngleCheckbox;
	delete PupilPermissiveAngle;
	delete PupilPermissiveLengthCheckbox;
	delete PupilPermissiveLength;
	delete PupilSearchEdgeLengthCheckbox;
	delete PupilSearchEdgeLength;
	delete RefThreshold;
	delete RefThresholdCheckbox;
}
LRESULT iRec::W_iRecCtrl::CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	W_iRecCtrl* thisPtr = NULL;

	switch (message) {
	case WM_CREATE:
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
		thisPtr = (W_iRecCtrl*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		break;
	case WM_DESTROY:
		thisPtr = (W_iRecCtrl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		break;
	default:
		thisPtr = (W_iRecCtrl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		break;
	}
	if (thisPtr) {
		return thisPtr->WndProc(hWnd, message, wParam, lParam);
	}
	return (DefWindowProc(hWnd, message, wParam, lParam));
}

LRESULT   iRec::W_iRecCtrl::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case (UINT64)ID_IsTopAcquisition:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.IsTopAcquisition = iRecPtr->ImgProcParam.IsTopAcquisition ^ 1;
		}
			break;
		case (UINT64)ID_IsBottomAcquisition:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.IsBottomAcquisition = iRecPtr->ImgProcParam.IsBottomAcquisition ^ 1;
		}
			break;
		case (UINT64)ID_IsGleaningAcquisition:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.IsGleaningAcquisition = iRecPtr->ImgProcParam.IsGleaningAcquisition ^ 1;
		}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_LBUTTONDOWN:
		SetFocus(GetParent(hWnd));
		break;
	case WM_HSCROLL:
		if ((HWND)lParam == PupilThresholdSlider->hWnd) {

			wstring ws = (boost::wformat(L"Threshold [%1%]") % PupilThresholdSlider->Get()).str();
			PupilThresholdAuto->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.PupilThreshold = PupilThresholdSlider->Get();
			}
		}
		if ((HWND)lParam == PupilPermissiveAngle->hWnd) {
			wstring ws = (boost::wformat(L"Permissive angle [%1%]deg") % PupilPermissiveAngle->Get()).str();
			PupilPermissiveAngleCheckbox->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.PermissiveAngle = PupilPermissiveAngle->Get();
			}
		}
		if ((HWND)lParam == PupilPermissiveLength->hWnd) {
			wstring ws = (boost::wformat(L"Permissive length [%1%]") % PupilPermissiveLength->Get()).str();
			PupilPermissiveLengthCheckbox->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.PermissiveLength = PupilPermissiveLength->Get();
			}
		}
		if ((HWND)lParam == PupilSearchEdgeLength->hWnd) {
			wstring ws = (boost::wformat(L"Search edge length [%1%]") % PupilSearchEdgeLength->Get()).str();
			PupilSearchEdgeLengthCheckbox->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.SearchEdgeLength = PupilSearchEdgeLength->Get();
			}
		}
		if ((HWND)lParam == RefThreshold->hWnd) {
			wstring ws = (boost::wformat(L"Reflection Threshold [%1%]") % RefThreshold->Get()).str();
			RefThresholdCheckbox->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.RefThreshold = RefThreshold->Get();
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


