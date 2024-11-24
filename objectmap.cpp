#include "framework.h"
#include "iRecHS2s.h"

//#define CONWIN 1

iRec::DlgObjectmap::DlgObjectmap(HWND hWnd, iRec& ir)
{
	hWndParent = hWnd;
	iRecPtr = &ir;
}

iRec::DlgObjectmap::~DlgObjectmap()
{

}


INT_PTR iRec::DlgObjectmap::Open()
{
	INT_PTR iptr;

	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_OBJECTMAP), hWndParent, CallbackProc, (LPARAM)this);
	return iptr;
}


bool iRec::DlgObjectmap::InitCtrl(HWND hDlg)
{
	for (int i = 0; i < MapNum; i++) {
		edbox[i].reset(new DlgEditBox(hDlg,IDC_EDIT1+i));
		edbox[i]->SetDouble(L"%5.2f", iRecPtr->CalibrationParam.map[i]);
	}
	cb_position.reset(new DlgCheckbox(hDlg, IDC_RADIO_POSITION));
	cb_angle.reset(new DlgCheckbox(hDlg, IDC_RADIO_ANGLE));
	if (iRecPtr->CalibrationParam.mapPosition) {
		cb_position->Set(true);
		for (INT64 i = 0; i < MapNum / 5; i++) {
			edbox[i * 5 + 0]->enable();
			edbox[i * 5 + 1]->enable();
			edbox[i * 5 + 2]->enable();
			edbox[i * 5 + 3]->disable();
			edbox[i * 5 + 4]->disable();
		}
	}
	else {
		cb_angle->Set(true);
		for (INT64  i = 0; i < MapNum / 5; i++) {
			edbox[i * 5 + 0]->disable();
			edbox[i * 5 + 1]->disable();			
			edbox[i * 5 + 2]->disable();
			edbox[i * 5 + 3]->enable();
			edbox[i * 5 + 4]->enable();
		}
	}

	return true;
}


void iRec::DlgObjectmap::EndCtrl(HWND hDlg)
{

}
bool iRec::DlgObjectmap::UpdateEditBox(int id, int event)
{
	double tmp, x, y, z, h, v;
	INT64 xpos, ypos, zpos, hpos, vpos;
	bool flag = false;
	INT64 i,row, col;
	std::wstring ws;
	if (event != EN_KILLFOCUS) return false;

	for (i = 0; i < MapNum; i++) {
		if (edbox[i]->id == id) {
			ws = edbox[i]->Get();
			try {
				tmp = std::stof(ws);
			}
			catch (std::exception const& /*ex*/) {
				edbox[i]->SetDouble(L"%5.2f", 0);
				SetFocus(edbox[i]->hWnd);
				Edit_SetSel(edbox[i]->hWnd, 0, Edit_GetTextLength(edbox[i]->hWnd));
				MessageBeep((UINT)-1);
				return true;
			}
			edbox[i]->SetDouble(L"%5.2f", tmp);
			flag = true;
			break;
		}
	}
	if (!flag) return false;
	row = i / 5;
	col = i % 5;
	xpos = row * 5;
	ypos = row * 5 + 1;
	zpos = row * 5 + 2;
	hpos = row * 5 + 3;
	vpos = row * 5 + 4;
	switch (col) {
	case 0:
		z = edbox[zpos]->GetDouble();
		x= edbox[xpos]->GetDouble();
		if (z != 0) {
			h = atan2(x, z) * 180/M_PI;
			edbox[hpos]->SetDouble(L"%5.2f", h);
		}
		break;
	case 1:
		z = edbox[zpos]->GetDouble();
		y = edbox[ypos]->GetDouble();
		if (z != 0) {
			v = atan2(y, z) * 180 / M_PI;
			edbox[vpos]->SetDouble(L"%5.2f", v);
		}
		break;
	case 2:
		x = edbox[xpos]->GetDouble();
		y = edbox[ypos]->GetDouble();
		z = edbox[zpos]->GetDouble();
		if (z != 0) {
			h = atan2(x, z) * 180 / M_PI;
			edbox[hpos]->SetDouble(L"%5.2f", h);
			v = atan2(y, z) * 180 / M_PI;
			edbox[vpos]->SetDouble(L"%5.2f", v);
		}
		break;
	case 3:
		z = edbox[zpos]->GetDouble();
		h = edbox[hpos]->GetDouble();
		if (z != 0) {
			x = tan(h * M_PI / 180.0) * z;
			edbox[xpos]->SetDouble(L"%5.2f", x);
		}
		break;
	case 4:
		z = edbox[zpos]->GetDouble();
		v = edbox[vpos]->GetDouble();
		if (z != 0) {
			y = tan(v * M_PI / 180.0) * z;
			edbox[ypos]->SetDouble(L"%5.2f", y);
		}
		break;
	}

	return true;
}
bool iRec::DlgObjectmap::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_INITDIALOG:
		if (InitCtrl(hDlg)) return (INT_PTR)true;
		EndDialog(hDlg, false);
		return (INT_PTR)true;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		if (UpdateEditBox(wmId, wmEvent)) {
			return (INT_PTR)true;
		}
		switch (wmId) 	
		{
		case IDC_RADIO_POSITION:
			if (wmEvent == BN_CLICKED) {
				iRecPtr->CalibrationParam.mapPosition = true;
				for (INT64 i = 0; i < MapNum / 5; i++) {
					edbox[i * 5 + 0]->enable();
					edbox[i * 5 + 1]->enable();
					edbox[i * 5 + 2]->enable();
					edbox[i * 5 + 3]->disable();
					edbox[i * 5 + 4]->disable();
				}
			}
			return (INT_PTR)true;
		case IDC_RADIO_ANGLE:
			if (wmEvent == BN_CLICKED) {
				iRecPtr->CalibrationParam.mapPosition = false;
				for (INT64  i = 0; i < MapNum / 5; i++) {
					edbox[i * 5 + 0]->disable();
					edbox[i * 5 + 1]->disable();
					edbox[i * 5 + 2]->disable();
					edbox[i * 5 + 3]->enable();
					edbox[i * 5 + 4]->enable();
				}
			}
			return (INT_PTR)true;
		case IDOK:
			if (iRecPtr->CalibrationParam.mapPosition) {
				if (edbox[5 * 8 + 2]->hWnd == GetFocus()) {
					SetFocus(edbox[0]->hWnd);
					Edit_SetSel(edbox[0]->hWnd,0,Edit_GetTextLength(edbox[0]->hWnd));
					return true;
				}
			}
			else {
				if (edbox[5 * 8 + 4]->hWnd == GetFocus()) {
					SetFocus(edbox[3]->hWnd);
					Edit_SetSel(edbox[3]->hWnd, 0, Edit_GetTextLength(edbox[0]->hWnd));
					return true;
				}
			}
			for (int i = 0; i < MapNum; i++) {
				if (edbox[i]->hWnd == GetFocus()) {
					PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
					return true;
				}
			}
			for (int i = 0; i < MapNum; i++) {
				iRecPtr->CalibrationParam.map[i] = edbox[i]->GetDouble();
			}
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

INT_PTR CALLBACK iRec::DlgObjectmap::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DlgObjectmap* thisPtr = NULL;

	switch (message) {
	case WM_INITDIALOG:
		thisPtr = (DlgObjectmap*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
		break;
	case WM_DESTROY:
		thisPtr = (DlgObjectmap*)GetWindowLongPtr(hDlg, DWLP_USER);
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)0);
		break;
	default:
		thisPtr = (DlgObjectmap*)GetWindowLongPtr(hDlg, DWLP_USER);
		break;
	}
	if (thisPtr) {
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}