// iRecHS2s.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "iRecHS2s.h"

#define MY_STATUS 10000

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
HWND hMainWindow;
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名
iRec iR;

ConsoleWindow conwin;

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


HWND MyCreateStatusbar(HWND hWnd)
{
	HWND hStatus;
	hStatus = CreateWindowEx(
		0,
		STATUSCLASSNAME,
		NULL,
		WS_CHILD | SBARS_SIZEGRIP | CCS_BOTTOM | WS_VISIBLE,
		0, 0, 0, 0,
		hWnd,
		(HMENU)MY_STATUS,
		hInst,
		NULL);
	SendMessage(hStatus, SB_SIMPLE, 0, 0);
	
	return hStatus;
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ここにコードを挿入してください。

	// グローバル文字列を初期化する
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_IRECHS2S, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!iR.cam.DialogSelect()) {
		std::wstringstream wss;
		wss << "No Camera seleced!" << endl;
		MessageBox(NULL, (wss.str()).c_str(), TEXT("Spinnaker Error Message"), MB_OK);
		return false;
	}

	if (!iR.LoadParameter()) {
		iR.cam.DialogRegion();
		iR.cam.DialogParameters();
	}

	iR.SetDaconverter(NULL);
		//{
		//	wstringstream wss;
		//	wss << "Hello Console:" << endl;
		//	conwin.write(wss.str());
		//}

	iR.StartCaptureThread();

	iR.SetNetIO();

	// アプリケーション初期化の実行:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IRECHS2S));

	MSG msg;

	iR.SetCameraView(hMainWindow, 0, 0);




	// メイン メッセージ ループ:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IRECHS2S));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_IRECHS2S);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

	hMainWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, MinimumWindowWidth, MinimumWindowHeight, nullptr, nullptr, hInstance, nullptr);

	if (!hMainWindow)
	{
		return FALSE;
	}

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//

void Flip(bool& target)
{
	if (target) target = false;
	else target = true;
}

void SetMenuCheck(HMENU& hm, const UINT& item, bool ischeck)
{
	MENUITEMINFO mi;
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_STATE;
	if (ischeck) {
		mi.fState = MFS_CHECKED;
	}
	else {
		mi.fState = MFS_UNCHECKED;
	}
	SetMenuItemInfo(hm, item, FALSE, &mi);
}

bool  SetMenuString(HMENU& hm, const UINT& item, UINT uID, std::wstring& ws)
{
	return ModifyMenu(hm, item, MFT_STRING, uID, ws.c_str());
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HMENU hMenu;
	static HWND hStatus;
	INITCOMMONCONTROLSEX ic;
	MINMAXINFO* lpmin;

	int sb_size[] = { 300, 600, -1 };
	switch (message)
	{
	case WM_CREATE:
		hMenu = GetMenu(hWnd);
		ic.dwICC = ICC_BAR_CLASSES;
		ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCommonControlsEx(&ic);
		hStatus = MyCreateStatusbar(hWnd);
		SendMessage(hStatus, SB_SETPARTS, (WPARAM)3, (LPARAM)(LPINT)sb_size);
		SendMessage(hStatus, SB_SETTEXT, 1 | 0, (LPARAM)TEXT("UDP_INT"));
		SendMessage(hStatus, SB_SETTEXT, 2 | 0, (LPARAM)TEXT("UDP_STR"));
		iR.nio->SetStatusWindow(hStatus);

		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch (wmId)
		{
		case ID_DIALOG_INPUTREGION:
			iR.PauseCaptureThread();
			iR.cam.DialogRegion(hWnd);
			iR.ResumeCaptureThread();
			break;
		case ID_DIALOG_CAMERACONTROL:
			iR.PauseCaptureThread();
			iR.cam.DialogParameters(hWnd);
			iR.ResumeCaptureThread();
			break;
		case ID_DIALOG_OPTIONS:
			iR.DialogOptions(hWnd);
			break;
		case ID_DIALOG_OBJECTMAP:
			iR.DialogObjectmap(hWnd);
			break;

		case ID_PUPIL_OUTLINE:
			Flip(iR.CameraViewParam.IsPupilOutline);
			break;
		case ID_PUPIL_CENTER:
			Flip(iR.CameraViewParam.IsPupilCenter);
			break;
		case ID_CAMERAVIEW_UNDERTHRESHOLD:
			Flip(iR.CameraViewParam.IsUnderThreshold);
			break;
		case ID_PUPIL_EDGE:
			Flip(iR.CameraViewParam.IsPupilEdge);
			break;
		case ID_PUPIL_MINORAXIS:
			Flip(iR.CameraViewParam.IsMinorAxis);
			break;
		case ID_CORNEALREFLECTION:
			Flip(iR.CameraViewParam.IsReflection);
			break;
		case ID_REGIONOFINTEREST:
			Flip(iR.CameraViewParam.IsRoi);
			break;
		case ID_CAMERAVIEW_INFORMATION:
			Flip(iR.CameraViewParam.IsInformation);
			break;
		case ID_VIEW_MIRRORIMAGE:
			Flip(iR.CameraViewParam.IsMirrorImage);
			break;
		case ID_COMMAND_RESETCALIBRATION:
			iR.nCalib->ResetCalibration();
			break;
			//case ID_METHOD_HEADFREE:
			//	iR.OutputParam.method = Output_Parameter::PR_METHOD;
			//	break;
			//case ID_HEADFIXED_PUPILCENTER:
			//	iR.OutputParam.method = Output_Parameter::PC_METHOD;
			//	break;
			//case ID_HEADFIXED_CORNEALREFLECTION:
			//	iR.OutputParam.method = Output_Parameter::RC_METHOD;
			//	break;

		case ID_OUTPUT_AUTO:
			Flip(iR.OutputParam.IsAutoMode);
			if (iR.OutputParam.IsAutoMode) {
				iR.ImgProcParam.pr.mode = iR.ImgProcParam.pr.mode_enabled;
				iR.ImgProcParam.pc.mode = iR.ImgProcParam.pc.mode_enabled;
				iR.ImgProcParam.rc.mode = iR.ImgProcParam.rc.mode_enabled;
			}
			break;
		case ID_OUTPUT_PUPILCENTER:
			iR.ImgProcParam.pr.mode = 0;
			iR.ImgProcParam.pc.mode = 0;
			iR.ImgProcParam.rc.mode = 0;
			break;
		case ID_OUTPUT_CAMERACOORDINATES:
			if (iR.ImgProcParam.pr.mode_enabled >= 1) {
				iR.ImgProcParam.pr.mode = 1;
			}
			if (iR.ImgProcParam.pc.mode_enabled >= 1) {
				iR.ImgProcParam.pc.mode = 1;
			}
			if (iR.ImgProcParam.rc.mode_enabled >= 1) {
				iR.ImgProcParam.rc.mode = 1;
			}
			break;
		case ID_OUTPUT_OBJECTCOORDINATES:
			if (iR.ImgProcParam.pr.mode_enabled >= 2) {
				iR.ImgProcParam.pr.mode = 2;
			}
			if (iR.ImgProcParam.pc.mode_enabled >= 2) {
				iR.ImgProcParam.pc.mode = 2;
			}
			if (iR.ImgProcParam.rc.mode_enabled >= 2) {
				iR.ImgProcParam.rc.mode = 2;
			}
			break;

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case ID_FILE_PAUSE:
		case ID_ACCELERATOR_ESC:
		
			if (iR.IsPause) {
				iR.ResumeCaptureThread();
			}
			else {
				iR.PauseCaptureThread();
			}
			break;
		case ID_ACCELERATOR_SPACE:
			iR.wTimeLine->ClickGraph(0.3);
			break;
		case IDM_EXIT:

			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: HDC を使用する描画コードをここに追加してください...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_GETMINMAXINFO:
		lpmin = (LPMINMAXINFO)lParam;
		lpmin->ptMinTrackSize.x = MinimumWindowWidth;
		lpmin->ptMinTrackSize.y = MinimumWindowHeight;

		lpmin->ptMaxTrackSize.x = 3840;
		lpmin->ptMaxTrackSize.y = 2160;

		break;
	case WM_SIZE:
		SendMessage(hStatus, message, wParam, lParam);

		if (iR.wTimeLine != NULL) {
			iR.wTimeLine->Resize();
		}
		if (iR.wXYGraph != NULL) {
			iR.wXYGraph->Resize();
		}

		break;
	case WM_INITMENU:
		SetMenuCheck(hMenu, ID_PUPIL_OUTLINE, iR.CameraViewParam.IsPupilOutline);
		SetMenuCheck(hMenu, ID_PUPIL_CENTER, iR.CameraViewParam.IsPupilCenter);
		SetMenuCheck(hMenu, ID_CAMERAVIEW_UNDERTHRESHOLD, iR.CameraViewParam.IsUnderThreshold);
		SetMenuCheck(hMenu, ID_PUPIL_EDGE, iR.CameraViewParam.IsPupilEdge);
		SetMenuCheck(hMenu, ID_REGIONOFINTEREST, iR.CameraViewParam.IsRoi);
		SetMenuCheck(hMenu, ID_PUPIL_MINORAXIS, iR.CameraViewParam.IsMinorAxis);
		SetMenuCheck(hMenu, ID_VIEW_MIRRORIMAGE, iR.CameraViewParam.IsMirrorImage);
		SetMenuCheck(hMenu, ID_CORNEALREFLECTION, iR.CameraViewParam.IsReflection);
		SetMenuCheck(hMenu, ID_CAMERAVIEW_INFORMATION, iR.CameraViewParam.IsInformation);
		SetMenuCheck(hMenu, ID_FILE_PAUSE, iR.IsPause);

		switch (iR.OutputParam.method) {
		case Output_Parameter::PR_METHOD:
			//SetMenuCheck(hMenu, ID_METHOD_HEADFREE, true);
			//SetMenuCheck(hMenu, ID_HEADFIXED_PUPILCENTER,false);
			//SetMenuCheck(hMenu, ID_HEADFIXED_CORNEALREFLECTION, false);
			ModifyMenu(hMenu, ID_OUTPUT_PUPILCENTER, MFT_STRING, ID_OUTPUT_PUPILCENTER, L"Pupil Center");
			switch (iR.ImgProcParam.pr.mode) {
			case 0:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, true);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, false);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, false);
				break;
			case 1:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, false);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, true);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, false);
				break;
			case 2:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, false);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, false);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, true);
				break;
			}
			break;
		case Output_Parameter::PC_METHOD:
			ModifyMenu(hMenu, ID_OUTPUT_PUPILCENTER, MFT_STRING, ID_OUTPUT_PUPILCENTER, L"Pupil Center");
			//ws = std::wstring(L"Pupil center");
			//SetMenuString(hMenu, ID_OUTPUT_PUPILCENTER, ID_OUTPUT_PUPILCENTER, ws);
			//SetMenuCheck(hMenu, ID_METHOD_HEADFREE, false);
			//SetMenuCheck(hMenu, ID_HEADFIXED_PUPILCENTER, true);
			//SetMenuCheck(hMenu, ID_HEADFIXED_CORNEALREFLECTION, false);
			switch (iR.ImgProcParam.pc.mode) {
			case 0:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, true);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, false);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, false);
				break;
			case 1:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, false);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, true);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, false);
				break;
			case 2:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, false);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, false);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, true);
				break;
			}
			break;
		case Output_Parameter::RC_METHOD:
			ModifyMenu(hMenu, ID_OUTPUT_PUPILCENTER, MFT_STRING, ID_OUTPUT_PUPILCENTER, L"Corneal reflection center");
			//ws = std::wstring(L"Corneal reflection center");
			//SetMenuString(hMenu, ID_OUTPUT_PUPILCENTER, ID_OUTPUT_PUPILCENTER, ws);
			//SetMenuCheck(hMenu, ID_METHOD_HEADFREE,false);
			//SetMenuCheck(hMenu, ID_HEADFIXED_PUPILCENTER, false);
			//SetMenuCheck(hMenu, ID_HEADFIXED_CORNEALREFLECTION, true);
			switch (iR.ImgProcParam.rc.mode) {
			case 0:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, true);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, false);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, false);
				break;
			case 1:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, false);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, true);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, false);
				break;
			case 2:
				SetMenuCheck(hMenu, ID_OUTPUT_PUPILCENTER, false);
				SetMenuCheck(hMenu, ID_OUTPUT_CAMERACOORDINATES, false);
				SetMenuCheck(hMenu, ID_OUTPUT_OBJECTCOORDINATES, true);
				break;
			}
			break;
		}
		SetMenuCheck(hMenu, ID_OUTPUT_AUTO, iR.OutputParam.IsAutoMode);
		if (iR.OutputParam.IsAutoMode) {
			EnableMenuItem(hMenu, ID_OUTPUT_PUPILCENTER, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, ID_OUTPUT_CAMERACOORDINATES, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, ID_OUTPUT_OBJECTCOORDINATES, MF_BYCOMMAND | MF_GRAYED);
		}
		else {
			EnableMenuItem(hMenu, ID_OUTPUT_PUPILCENTER, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, ID_OUTPUT_CAMERACOORDINATES, MF_BYCOMMAND | MF_GRAYED);
			if (iR.OutputParam.method == Output_Parameter::PC_METHOD) {
				if (iR.ImgProcParam.pc.mode_enabled >= 1) {
					EnableMenuItem(hMenu, ID_OUTPUT_CAMERACOORDINATES, MF_BYCOMMAND | MF_ENABLED);
				}
			}
			if (iR.OutputParam.method == Output_Parameter::PR_METHOD) {
				if (iR.ImgProcParam.pr.mode_enabled >= 1) {
					EnableMenuItem(hMenu, ID_OUTPUT_CAMERACOORDINATES, MF_BYCOMMAND | MF_ENABLED);
				}
			}
			if (iR.OutputParam.method == Output_Parameter::RC_METHOD) {
				if (iR.ImgProcParam.rc.mode_enabled >= 1) {
					EnableMenuItem(hMenu, ID_OUTPUT_CAMERACOORDINATES, MF_BYCOMMAND | MF_ENABLED);
				}
			}
			EnableMenuItem(hMenu, ID_OUTPUT_OBJECTCOORDINATES, MF_BYCOMMAND | MF_GRAYED);
			if (iR.OutputParam.method == Output_Parameter::PC_METHOD) {
				if (iR.ImgProcParam.pc.mode_enabled >= 2) {
					EnableMenuItem(hMenu, ID_OUTPUT_OBJECTCOORDINATES, MF_BYCOMMAND | MF_ENABLED);
				}
			}
			if (iR.OutputParam.method == Output_Parameter::PR_METHOD) {
				if (iR.ImgProcParam.pr.mode_enabled >= 2) {
					EnableMenuItem(hMenu, ID_OUTPUT_OBJECTCOORDINATES, MF_BYCOMMAND | MF_ENABLED);
				}
			}
			if (iR.OutputParam.method == Output_Parameter::RC_METHOD) {
				if (iR.ImgProcParam.rc.mode_enabled >= 2) {
					EnableMenuItem(hMenu, ID_OUTPUT_OBJECTCOORDINATES, MF_BYCOMMAND | MF_ENABLED);
				}
			}
		}
		break;
	case WM_DESTROY:
		iR.EndCaptureThread();
		iR.SaveParameter();
		iR.fIO->savedata();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
