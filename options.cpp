#include "framework.h"
#include "iRecHS2s.h"

//#define CONWIN 1

iRec::DlgOptions::DlgOptions(HWND hWnd, iRec& ir)
{
	hWndParent = hWnd;
	iRecPtr = &ir;
}

iRec::DlgOptions::~DlgOptions()
{

}


INT_PTR iRec::DlgOptions::Open()
{
	INT_PTR iptr;

	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_OPTIONS), hWndParent, CallbackProc, (LPARAM)this);
	return iptr;
}


bool iRec::DlgOptions::InitCtrl(HWND hDlg)
{
	//	btn_OK.reset(new DlgButton(hDlg, IDOK));
	//	btn_Cancel.reset(new DlgButton(hDlg, IDCANCEL));
	int tmpInt;
	double tmpDouble;
	lb_Threshold.reset(new DlgLabel(hDlg, IDC_STATIC_THRESHOLD));
	sl_Threshold.reset(new DlgSlider(hDlg, IDC_SLIDER_THRESHOLD, 0, 255, iRecPtr->ImgProcParam.PupilThreshold, 50, 10));
	wstring ws = (boost::wformat(L"Threshold [%1%]") % sl_Threshold->Get()).str();
	lb_Threshold->SetText(ws);

	lb_RefThreshold.reset(new DlgLabel(hDlg, IDC_STATIC_REFTHRESHOLD));
	sl_RefThreshold.reset(new DlgSlider(hDlg, IDC_SLIDER_REFTHRESHOLD, 0, 255, iRecPtr->ImgProcParam.RefThreshold, 50, 10));
	ws = (boost::wformat(L"Reflection Threshold [%1%]") % sl_RefThreshold->Get()).str();
	lb_RefThreshold->SetText(ws);


	lb_P_Angle.reset(new DlgLabel(hDlg, IDC_STATIC_P_ANGLE));
	sl_P_Angle.reset(new DlgSlider(hDlg, IDC_SLIDER_P_ANGLE, 1, 10, iRecPtr->ImgProcParam.PermissiveAngle, 1, 5));
	ws = (boost::wformat(L"Permissive angle [%1%]deg") % sl_P_Angle->Get()).str();
	lb_P_Angle->SetText(ws);

	lb_P_Length.reset(new DlgLabel(hDlg, IDC_STATIC_P_LENGTH));
	sl_P_Length.reset(new DlgSlider(hDlg, IDC_SLIDER_P_LENGTH, 1, 10, iRecPtr->ImgProcParam.PermissiveLength, 1, 5));
	ws = (boost::wformat(L"Permissive length [%1%]pixel") % sl_P_Length->Get()).str();
	lb_P_Length->SetText(ws);

	lb_SE_Length.reset(new DlgLabel(hDlg, IDC_STATIC_SE_LENGTH));
	sl_SE_Length.reset(new DlgSlider(hDlg, IDC_SLIDER_SE_LENGTH, 5, 32, iRecPtr->ImgProcParam.SearchEdgeLength, 5, 5));
	ws = (boost::wformat(L"Search edge range [%1%]pixel") % sl_SE_Length->Get()).str();
	lb_SE_Length->SetText(ws);

	lb_BlinkThreshold.reset(new DlgLabel(hDlg, IDC_STATIC_BLINK));
	sl_BlinkThreshold.reset(new DlgSlider(hDlg, IDC_SLIDER_BLINK, 0, 100, iRecPtr->ImgProcParam.BlinkThreshold, 10, 10));
	ws = (boost::wformat(L"Blink Threshold [%1%]%%") % sl_BlinkThreshold->Get()).str();
	lb_BlinkThreshold->SetText(ws);

	tmpInt = static_cast<int>(iRecPtr->CalibrationParam.FixationRangePixel);
	sl_FixationRange_Pixel.reset(new DlgSlider(hDlg, IDC_SLIDER_FIXATION_PIXEL, 1, 10, tmpInt, 1, 1));

	tmpInt = static_cast<int>(iRecPtr->CalibrationParam.FixationRangeAngle);
	sl_FixationRange_Angle.reset(new DlgSlider(hDlg, IDC_SLIDER_FIXATION_ANGLE, 1, 10, tmpInt, 1, 1));

	tmpInt = static_cast<int>(iRecPtr->CalibrationParam.FixationDuration*10);
	sl_Fixation_Duration.reset(new DlgSlider(hDlg, IDC_SLIDER_FIXATION_DURATION, 1, 10, tmpInt, 1, 1));

	tmpInt = static_cast<int>(iRecPtr->CalibrationParam.PassiveCalibSd);
	sl_Passive_sd.reset(new DlgSlider(hDlg, IDC_SLIDER_PASSIVE_SD, 1, 10, tmpInt, 1, 1));

	lb_FixationRange_Pixel.reset(new DlgLabel(hDlg, IDC_STATIC_FIXATION_PIXEL));
	ws = (boost::wformat(L"Fixation range [%1%]pixel") % sl_FixationRange_Pixel->Get()).str();
	lb_FixationRange_Pixel->SetText(ws);

	lb_FixationRange_Angle.reset(new DlgLabel(hDlg, IDC_STATIC_FIXATION_ANGLE));
	ws = (boost::wformat(L"Fixation range [%1%]deg") % sl_FixationRange_Angle->Get()).str();
	lb_FixationRange_Angle->SetText(ws);


	lb_Fixation_Duration.reset(new DlgLabel(hDlg, IDC_STATIC_FIXATION_DURATION));
	tmpDouble = static_cast<double>(sl_Fixation_Duration->Get()) / 10;
	ws = (boost::wformat(L"Fixation duration [%3.1f]sec") % tmpDouble).str();
	lb_Fixation_Duration->SetText(ws);

	lb_Passive_sd.reset(new DlgLabel(hDlg, IDC_STATIC_PASSIVE_SD));
	ws = (boost::wformat(L"Passive calibration sd [%1%]deg") % sl_Passive_sd->Get()).str();
	lb_Passive_sd->SetText(ws);

	rb_none.reset(new DlgRadioButton(hDlg, IDC_RADIO_NONE));

	sl_Median_num.reset(new DlgSlider(hDlg, IDC_SLIDER_Median_NUM, 0, 7, iRecPtr->OutputParam.me_num/2, 1, 1));
	rb_Median.reset(new DlgRadioButton(hDlg, IDC_RADIO_Median));
	ws = (boost::wformat(L"Median [%1%]points") % (sl_Median_num->Get() * 2 + 1)).str();
	rb_Median->SetText(ws);

	sl_MA_num.reset(new DlgSlider(hDlg, IDC_SLIDER_MA_NUM, 1, 15, iRecPtr->OutputParam.mv_num, 1, 1));
	rb_MA.reset(new DlgRadioButton(hDlg, IDC_RADIO_MVA));
	ws = (boost::wformat(L"Moving average [%1%]points") % sl_MA_num->Get()).str();
	rb_MA->SetText(ws);

	switch (iRecPtr->OutputParam.nr_mode) {
	case Output_Parameter::NONE:
		rb_none->Set(true);
		break;
	case Output_Parameter::MEDIAN:
		rb_Median->Set(true);
		break;
	case Output_Parameter::MOVING_AVERAGE:
		rb_MA->Set(true);
		break;
	}

	cl_method.reset(new DlgComboList(hDlg, IDC_COMBO_METHOD));
	ws = wstring(L"Head free");
	cl_method->add(ws);
	ws = wstring(L"Head fixed(Pupil Center)");
	cl_method->add(ws);
	ws = wstring(L"Head fixed(Corneal eflection)");
	cl_method->add(ws);

	switch (iRecPtr->OutputParam.method) {
		case Output_Parameter::PR_METHOD:
			cl_method->Set(0);
			break;
		case Output_Parameter::PC_METHOD:
			cl_method->Set(1);
			break;
		case Output_Parameter::RC_METHOD:
			cl_method->Set(2);
			break;
	}
	cb_PupilTop.reset(new DlgCheckbox(hDlg, IDC_UseTopFitting));
	cb_PupilTop->Set(iRecPtr->ImgProcParam.IsTopAcquisition);
	cb_PupilBottom.reset(new DlgCheckbox(hDlg, IDC_UseBottomFitting));
	cb_PupilBottom->Set(iRecPtr->ImgProcParam.IsBottomAcquisition);
	cb_PupilElse.reset(new DlgCheckbox(hDlg, IDC_UseGleaning));
	cb_PupilElse->Set(iRecPtr->ImgProcParam.IsGleaningAcquisition);
	cb_IsUseMovingROI.reset(new DlgCheckbox(hDlg, IDC_UseMovingROI));
	cb_IsUseMovingROI->Set(iRecPtr->ImgProcParam.IsMovingROI);
	cb_IsUsePreviousImage.reset(new DlgCheckbox(hDlg, IDC_UsePreviousImage));
	cb_IsUsePreviousImage->Set(iRecPtr->ImgProcParam.IsUsePreviousImage);
	
	cb_IsSave.reset(new DlgCheckbox(hDlg, IDC_CHECK_SAVE));
	cb_IsSave->Set(iRecPtr->OutputParam.IsSave);
	cb_IsSaveDev.reset(new DlgCheckbox(hDlg, IDC_CHECK_SAVEDEV));
	cb_IsSaveDev->Set(iRecPtr->OutputParam.IsSaveDev);


	cb_FlipH.reset(new DlgCheckbox(hDlg, IDC_CHECK_FLIPH));
	if (iRecPtr->OutputParam.Flip.x == 1) {
		cb_FlipH->Set(false);
	}
	else {
		cb_FlipH->Set(true);
	}

	cb_FlipV.reset(new DlgCheckbox(hDlg, IDC_CHECK_FLIPV));
	if (iRecPtr->OutputParam.Flip.y == 1) {
		cb_FlipV->Set(false);
	}
	else {
		cb_FlipV->Set(true);
	}

	return true;
}


void iRec::DlgOptions::EndCtrl(HWND hDlg)
{

}

bool iRec::DlgOptions::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_INITDIALOG:
		if (InitCtrl(hDlg)) return (INT_PTR)true;
		EndDialog(hDlg, false);
		return (INT_PTR)true;

	case WM_HSCROLL:
		if ((HWND)lParam == sl_Threshold->hWnd) {
			wstring ws = (boost::wformat(L"Threshold [%1%]") % sl_Threshold->Get()).str();
			lb_Threshold->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.PupilThreshold = sl_Threshold->Get();
			}
		}

		if ((HWND)lParam == sl_RefThreshold->hWnd) {
			wstring ws = (boost::wformat(L"Reflection Threshold [%1%]") % sl_RefThreshold->Get()).str();
			lb_RefThreshold->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.RefThreshold = sl_RefThreshold->Get();
			}
		}

		if ((HWND)lParam == sl_P_Angle->hWnd) {
			wstring ws = (boost::wformat(L"Permissive angle [%1%]deg") % sl_P_Angle->Get()).str();
			lb_P_Angle->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.PermissiveAngle = sl_P_Angle->Get();
			}
		}

		if ((HWND)lParam == sl_P_Length->hWnd) {
			wstring ws = (boost::wformat(L"Permissive length [%1%]pixel") % sl_P_Length->Get()).str();
			lb_P_Length->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.PermissiveLength = sl_P_Length->Get();
			}
		}

		if ((HWND)lParam == sl_SE_Length->hWnd) {
			wstring ws = (boost::wformat(L"Search edge range [%1%]pixel") % sl_SE_Length->Get()).str();
			lb_SE_Length->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.SearchEdgeLength = sl_SE_Length->Get();
			}
		}

		if ((HWND)lParam == sl_BlinkThreshold->hWnd) {
			wstring ws = (boost::wformat(L"Blink Threshold [%1%]%%") % sl_BlinkThreshold->Get()).str();
			lb_BlinkThreshold->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.BlinkThreshold = sl_BlinkThreshold ->Get();
			}
		}

		if ((HWND)lParam == sl_Median_num->hWnd) {
			wstring ws = (boost::wformat(L"Median [%1%]points") % (sl_Median_num->Get()*2+1)).str();

			rb_Median->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->OutputParam.me_num = sl_Median_num->Get()*2+1;
			}
		}

		if ((HWND)lParam == sl_MA_num->hWnd) {
			wstring ws = (boost::wformat(L"Moving average [%1%]points") % sl_MA_num->Get()).str();

			rb_MA->SetText(ws);
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->OutputParam.mv_num = sl_MA_num->Get();
			}
		}

		if ((HWND)lParam == sl_FixationRange_Pixel->hWnd) {
			wstring ws = (boost::wformat(L"Fixation range [%1%]pixel") % sl_FixationRange_Pixel->Get()).str();
			lb_FixationRange_Pixel->SetText(ws);
			iRecPtr->CalibrationParam.FixationRangePixel = static_cast<double>(sl_FixationRange_Pixel->Get());
		}
		if ((HWND)lParam == sl_FixationRange_Angle->hWnd) {
			wstring ws = (boost::wformat(L"Fixation range [%1%]deg") % sl_FixationRange_Angle->Get()).str();
			lb_FixationRange_Angle->SetText(ws);
			iRecPtr->CalibrationParam.FixationRangeAngle = static_cast<double>(sl_FixationRange_Angle->Get());
		}

		if ((HWND)lParam == sl_Fixation_Duration->hWnd) {
			double tmpDouble = static_cast<double>(sl_Fixation_Duration->Get()) / 10;
			wstring ws = (boost::wformat(L"Fixation duration [%3.1f]sec") % tmpDouble).str();
			lb_Fixation_Duration->SetText(ws);
			iRecPtr->CalibrationParam.FixationDuration = static_cast<double>(sl_Fixation_Duration->Get())/10;
		}

		if ((HWND)lParam == sl_Passive_sd->hWnd) {
			wstring ws = (boost::wformat(L"Passive calibration sd [%1%]deg") % sl_Passive_sd->Get()).str();
			lb_Passive_sd->SetText(ws);
			iRecPtr->CalibrationParam.PassiveCalibSd = static_cast<double>(sl_Passive_sd->Get());
		}

		return (INT_PTR)true;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId) {
		case IDC_UseTopFitting:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.IsTopAcquisition = iRecPtr->ImgProcParam.IsTopAcquisition ^ 1;
		}
		return (INT_PTR)true;
		case IDC_UseBottomFitting:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.IsBottomAcquisition = iRecPtr->ImgProcParam.IsBottomAcquisition ^ 1;
		}
		return (INT_PTR)true;
		case IDC_UseGleaning:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.IsGleaningAcquisition = iRecPtr->ImgProcParam.IsGleaningAcquisition ^ 1;
		}
		return (INT_PTR)true;
		case IDC_UseMovingROI:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.IsMovingROI = iRecPtr->ImgProcParam.IsMovingROI ^ 1;
		}
		return (INT_PTR)true;

		case IDC_UsePreviousImage:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.IsUsePreviousImage = iRecPtr->ImgProcParam.IsUsePreviousImage ^ 1;
		}
		return (INT_PTR)true;

		case IDC_CHECK_SAVE:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->OutputParam.IsSave = iRecPtr->OutputParam.IsSave ^ 1;
		}
		return (INT_PTR)true;

		case IDC_CHECK_SAVEDEV:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->OutputParam.IsSaveDev = iRecPtr->OutputParam.IsSaveDev ^ 1;
		}
		return (INT_PTR)true;

		case IDC_CHECK_FLIPH:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->OutputParam.Flip.x = iRecPtr->OutputParam.Flip.x * -1;
		}
		return (INT_PTR)true;

		case IDC_CHECK_FLIPV:
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->OutputParam.Flip.y = iRecPtr->OutputParam.Flip.y * -1;
		}

		return (INT_PTR)true;
		case IDC_RADIO_NONE:
			if (wmEvent == BN_CLICKED) {
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					iRecPtr->OutputParam.nr_mode = Output_Parameter::NONE;
				}
			}
			return (INT_PTR)true;
		case IDC_RADIO_Median:
			if (wmEvent == BN_CLICKED) {
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					iRecPtr->OutputParam.nr_mode = Output_Parameter::MEDIAN;
				}
			}
			return (INT_PTR)true;
		case IDC_RADIO_MVA:
			if (wmEvent == BN_CLICKED) {
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					iRecPtr->OutputParam.nr_mode = Output_Parameter::MOVING_AVERAGE;
				}
			}
			return (INT_PTR)true;

		case IDC_COMBO_METHOD:
			if (wmEvent == CBN_SELCHANGE) {
				switch (cl_method->Get()) {
				case 0:
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					iRecPtr->OutputParam.method = Output_Parameter::PR_METHOD;
				}
				return (INT_PTR)true;
				case 1:
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					iRecPtr->OutputParam.method = Output_Parameter::PC_METHOD;
				}
				return (INT_PTR)true;
				case 2:
				{
					std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
					iRecPtr->OutputParam.method = Output_Parameter::RC_METHOD;
				}
				return (INT_PTR)true;
				}
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

INT_PTR CALLBACK iRec::DlgOptions::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DlgOptions* thisPtr = NULL;

	switch (message) {
	case WM_INITDIALOG:
		thisPtr = (DlgOptions*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
		break;
	case WM_DESTROY:
		thisPtr = (DlgOptions*)GetWindowLongPtr(hDlg, DWLP_USER);
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)0);
		break;
	default:
		thisPtr = (DlgOptions*)GetWindowLongPtr(hDlg, DWLP_USER);
		break;
	}
	if (thisPtr) {
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}