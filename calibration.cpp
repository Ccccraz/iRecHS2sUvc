#include "framework.h"
#include "iRecHS2s.h"

#define SearchNum 5000
//#define CONWIN 1

STAT::STAT()
{
	Clear();
}

STAT::~STAT()
{
	Clear();
}

void STAT::Clear(void)
{
	mean = 0;
	sd = 0;
	num = 0;
	sum = 0;
	sum2 = 0;
	max = min = 0;
	v.clear();
}

int STAT::Store(double x)
{
	if (num == 0) {
		max = min = x;
	}
	else {
		if (max < x) max = x;
		if (min > x) min = x;
	}
	num++;
	sum += x;
	mean = sum / num;
	v.push_back(x);
	return (int)num;

}

int STAT::Calc(void)
{
	int i;
	if (v.size() <= 1) return (int)1;
	for (i = 0, sum2 = 0; i < v.size(); i++) {
		sum2 += (v[i] - mean) * (v[i] - mean);
	}
	sd = sqrt(sum2 / (num - 1));

	return (int)num;
}



iRec::Calibration::Calibration(HWND hWnd, iRec& ir)
{
	iRecPtr = &ir;
	hWndParent = hWnd;
	bmp.reset();
	info.reset();
	current_gp_num = 0;
}

iRec::Calibration::~Calibration()
{
	bmp.reset();
	info.reset();
	current_eye_deque.clear();
}


INT64 iRec::Calibration::IsGazeMoveEnough(INT64 iCount, int num, std::deque<PointData>& v)
{
	INT64 ic;
	int hit, checked;
	double ang_sd;
	STAT s_ang, ls_ang, w_ref, h_ref;;
	EyeData* eye = &iRecPtr->Eye;

	hit = checked = 0;
	ic = iCount;
	PointData cur;

	while ((hit < num) && (checked < num * 10) && (ic > 0)) {
		if ((eye->v[ic].pupil.size.w != 0) && (eye->v[ic].pupil.densityx > 60) && (eye->v[ic].ref.size.w != 0) && (eye->v[ic].pupil.axis.l != 0)) {
			s_ang.Store(atan2(fabs(eye->v[ic].pupil.slope.s), 1) * 180.0 / M_PI);
			ls_ang.Store(acos(eye->v[ic].pupil.axis.s / eye->v[ic].pupil.axis.l) * 180.0 / M_PI);
			w_ref.Store(eye->v[ic].ref.size.w);
			h_ref.Store(eye->v[ic].ref.size.h);
			hit++;
		}
		ic--;
		checked++;
	}
	if ((hit * 10 < num) || (iCount - ic < num)) return -1;


	s_ang.Calc();
	ls_ang.Calc();
	w_ref.Calc();
	h_ref.Calc();
	// ls_ang‚Ì‚Ý 2sd‚Ì’l‚Æ‚·‚é
	ang_sd = ls_ang.sd * 2;
	if (s_ang.sd < ang_sd) ang_sd = s_ang.sd;

	if (ang_sd < iRecPtr->CalibrationParam.PassiveCalibSd) return -1;


	hit = checked = 0;
	ic = iCount;

	s_ang.Clear();
	ls_ang.Clear();
	while ((hit < num) && (checked < num * 10) && (ic > 0)) {
		if ((eye->v[ic].pupil.size.w != 0)
			&& (eye->v[ic].pupil.densityx > 60)
			&& (eye->v[ic].ref.size.w != 0)
			&& (eye->v[ic].pupil.axis.l != 0)
			&& (eye->v[ic].ref.size.w < (w_ref.mean + w_ref.sd))
			&& ((w_ref.mean - w_ref.sd) < eye->v[ic].ref.size.w)
			&& (eye->v[ic].ref.size.h < (h_ref.mean + h_ref.sd))
			&& ((h_ref.mean - h_ref.sd) < eye->v[ic].ref.size.h)) {
			s_ang.Store(atan2(fabs(eye->v[ic].pupil.slope.s), 1) * 180.0 / M_PI);
			ls_ang.Store(acos(eye->v[ic].pupil.axis.s / eye->v[ic].pupil.axis.l) * 180.0 / M_PI);

			cur.pcx = eye->v[ic].pupil.center.x;
			cur.pcy = eye->v[ic].pupil.center.y;
			cur.rpx = eye->v[ic].ref.pos.x;
			cur.rpy = eye->v[ic].ref.pos.y;
			cur.psl = eye->v[ic].pupil.slope.l;
			cur.pss = eye->v[ic].pupil.slope.s;
			cur.pal = eye->v[ic].pupil.axis.l;
			cur.pas = eye->v[ic].pupil.axis.s;
			cur.num = 1;

			v.push_back(cur);
			hit++;
		}
		ic--;
		checked++;
	}
	if ((hit * 10 < num) || (iCount - ic < num)) return -1;


	s_ang.Calc();
	ls_ang.Calc();
	// ls_ang‚Ì‚Ý 2sd‚Ì’l‚Æ‚·‚é
	ang_sd = ls_ang.sd * 2;
	if (s_ang.sd < ang_sd) ang_sd = s_ang.sd;
	if (ang_sd < iRecPtr->CalibrationParam.PassiveCalibSd) return -1;


#ifdef CONWIN
	{
		wstringstream wss;
		wss << endl;
		wss << L"S_max    : " << s_ang.max << endl;
		wss << L"S_min    : " << s_ang.min << endl;
		wss << L"s_sd     : " << s_ang.sd << endl;
		wss << L"ls_sd    : " << ls_ang.sd << endl;

		wss << L"s_range     : " << s_ang.max - s_ang.min << endl;
		wss << L"ls_range    : " << ls_ang.max - ls_ang.min << endl;

		wss << L"ref_w    :" << w_ref.mean << endl;
		wss << L"ref_w_sd :" << w_ref.sd << endl;
		wss << L"ref_h    :" << h_ref.mean << endl;
		wss << L"ref_h_sd :" << h_ref.sd << endl;
		conwin.write(wss.str());
	}
#endif
	return ic;
}

bool iRec::Calibration::CalcPR_ObjParam(std::deque<PointData>& v)
{
	if (!pr.CalcObjParam(v, iRecPtr->OutputParam.Flip)) return false;
	if (iRecPtr->OutputParam.IsAutoMode) {
		if (pr.mode_enabled >= 2) {
			pr.mode = 2;
		}
	}

	return true;
}

bool iRec::Calibration::CalcPC_ObjParam(std::deque<PointData>& v)
{
	if (!pc.CalcObjParam(v, iRecPtr->OutputParam.Flip)) return false;

	if (iRecPtr->OutputParam.IsAutoMode) {
		if (pc.mode_enabled >= 2) {
			pc.mode = 2;
		}
	}

	return  true;
}

bool iRec::Calibration::CalcRC_ObjParam(std::deque<PointData>& v)
{
	if (!rc.CalcObjParam(v, iRecPtr->OutputParam.Flip)) return false;

	if (iRecPtr->OutputParam.IsAutoMode) {
		if (rc.mode_enabled >= 2) {
			rc.mode = 2;
		}
	}

	return  true;
}

bool iRec::Calibration::Calc_ObjParam(void)
{
	std::deque<PointData> dpd;
	PointData pd;
	for (int i = 0; i < MapPosNum; i++) {
		if (gp[i].pcx.num == 0) continue;
		if (iRecPtr->CalibrationParam.map[i * 5 + 2] == 0) continue;
		pd.num = static_cast<int>(gp[i].pcx.num);
		pd.objx = iRecPtr->CalibrationParam.map[i * 5 + 0];
		pd.objy = iRecPtr->CalibrationParam.map[i * 5 + 1];
		pd.objz = iRecPtr->CalibrationParam.map[i * 5 + 2];
		pd.objax = iRecPtr->CalibrationParam.map[i * 5 + 3];
		pd.objay = iRecPtr->CalibrationParam.map[i * 5 + 4];
		pd.pcx = gp[i].pcx.mean;
		pd.pcy = gp[i].pcy.mean;
		pd.rpx = gp[i].rpx.mean;
		pd.rpy = gp[i].rpy.mean;
		pd.pas = gp[i].pas.mean;
		pd.pal = gp[i].pal.mean;
		pd.pss = gp[i].pss.mean;
		pd.psl = gp[i].psl.mean;
		iRecPtr->CalibrationParam.pd[i] = pd;

		dpd.push_back(pd);
	}
	if (dpd.size() == 0) return false;
	pc = iRecPtr->ImgProcParam.pc;
	pr = iRecPtr->ImgProcParam.pr;
	rc = iRecPtr->ImgProcParam.rc;

	if (dpd.size() > 2) {
		if (!CalcPC_param(dpd)) {
			pc = iRecPtr->ImgProcParam.pc;
		}
		if (!CalcPR_param(dpd)) {
			pr = iRecPtr->ImgProcParam.pr;
		}
		if (!CalcRC_param(dpd)) {
			rc = iRecPtr->ImgProcParam.rc;
		}
	}

	if (CalcPC_ObjParam(dpd)) {
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.pc = pc;
		}
	}
	if (CalcRC_ObjParam(dpd)) {
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.rc = rc;
		}
	}

	if (CalcPR_ObjParam(dpd)) {
		{
			std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
			iRecPtr->ImgProcParam.pr = pr;

			//wstringstream  wss;
			//wss << endl;
			//wss << (boost::wformat(L"pr.mode           :%d ") % pr.mode) << endl;
			//wss << (boost::wformat(L"pr.mode_enabled   :%d ") % pr.mode_enabled) << endl;
			//conwin.write(wss.str());
		}
	}
	return true;
}



bool iRec::Calibration::CalcPR_param(std::deque<PointData>& v)
{
	double A, B, C, D, E;
	double ai, bi, ci, bb, bs, sn, sl;
	double xk, yk, lu, ld;
	double offx, offy, pcl;

	A = B = C = D = E = 0;
	offx = offy = pcl = 0;

	for (const auto& eye : v) {
		if (eye.pss != 0) {
			ai = eye.pss;
			bi = -1;
			ci = eye.pcy - eye.pss * eye.pcx;
		}
		else {
			ai = 1;
			bi = 0;
			ci = -eye.pcx;
		}
		A += (ai * ai) / (ai * ai + bi * bi);
		B += (ai * bi) / (ai * ai + bi * bi);
		C += (ai * ai * eye.rpx + ai * bi * eye.rpy + ai * ci) / (ai * ai + bi * bi);
		D = D + (bi * bi) / (ai * ai + bi * bi);
		E = E + (ai * bi * eye.rpx + bi * bi * eye.rpy + bi * ci) / (ai * ai + bi * bi);

	}
	bb = B * B - A * D;
	if ((bb == 0) || (A == 0)) {
		return false;
	}

	bs = B * C - A * E;
	offy = bs / bb;
	offx = (C - B * (offy)) / A;

	sn = sl = 0;

	for (const auto& eye : v) {
		xk = eye.rpx - (offx);
		yk = eye.rpy - (offy);

		lu = sqrt((eye.pcx - xk) * (eye.pcx - xk) + (eye.pcy - yk) * (eye.pcy - yk));
		ld = sqrt(1 - eye.pas * eye.pas / (eye.pal * eye.pal));
		if (ld != 0) {
			sl += lu / ld;
			sn++;
		}

	}
	if (sn == 0) {

		return false;
	}
	pcl = sl / sn;

	pr.offx = rc.offx = offx;
	pr.offy = rc.offy = offy;
	pr.pcl = pcl;
	pr.mode_enabled = 1;
	if (iRecPtr->OutputParam.IsAutoMode) {
		pr.mode = pr.mode_enabled;
	}

	return true;
}

bool iRec::Calibration::CalcPC_param(std::deque<PointData>& v)
{
	bool ret = false;

	double sa2, sa, sac, sc, num;
	double c, a2;
	double sl, sr, sn, tr, sr2;
	double pcx, pcy, rcx, rcy, radius, radius_sd;



	sa2 = sa = sac = sc = sn = num = 0;
	rcx = rcy = 0;

	for (const auto& eye : v) {
		num++;
		c = eye.pcy - eye.pss * eye.pcx;
		a2 = eye.pss * eye.pss;
		sa2 += (a2 / (a2 + 1));
		sa += (-eye.pss / (a2 + 1));
		sac += (eye.pss * c / (a2 + 1));
		sn += (1 / (a2 + 1));
		sc += (-c / (a2 + 1));
	}

	if ((num < 2) || (sa * sa - sa2 * sn == 0) || (sa2 == 0)) {
		return false;
	}

	rcy = (sa2 * sc - sa * sac) / (sa * sa - sa2 * sn);
	rcx = (-sac - sa * (rcy)) / sa2;

	sl = sr = sn = sr2 = 0;
	radius = radius_sd = 0;

	for (const auto& eye : v) {
		sl = eye.pas / eye.pal;
		pcx = eye.pcx;
		pcy = eye.pcy;
		tr = sqrt((pcx - rcx) * (pcx - rcx) + (pcy - rcy) * (pcy - rcy)) / sqrt(1 - sl * sl);
		sr += tr;
		sr2 += tr * tr;
		sn++;
	}

	if (sn < 2) {
		return false;
	}
	radius_sd = sqrt((sr2 - sn * (sr / sn) * (sr / sn)) / sn);
	//	if (radius_sd> 30) return false;

	radius = sr / sn;

	pc.ox = rc.ox = rcx;
	pc.oy = rc.oy = rcy;
	pc.radius = radius;
	pc.radius_sd = radius_sd;
	pc.mode_enabled = 1;
	if (iRecPtr->OutputParam.IsAutoMode) {
		pc.mode = pc.mode_enabled;
	}
	return true;
}

bool iRec::Calibration::CalcRC_param(std::deque<PointData>& v)
{
	bool ret = false;

	double sl, sr, sn, tr, sr2;
	double ccx, ccy, radius, radius_sd;

	sl = sr = sn = sr2 = 0;
	radius = 0;

	for (const auto& eye : v) {
		sl = eye.pas / eye.pal;
		ccx = eye.rpx - rc.offx;
		ccy = eye.rpy - rc.offy;
		tr = sqrt((ccx - rc.ox) * (ccx - rc.ox) + (ccy - rc.oy) * (ccy - rc.oy)) / sqrt(1 - sl * sl);
		sr += tr;
		sr2 += tr * tr;
		sn++;
	}

	if (sn < 2) 	return false;

	radius_sd = sqrt((sr2 - sn * (sr / sn) * (sr / sn)) / sn);
	//	if (radius_sd) return false;

	radius = sr / sn;
	ret = true;
	rc.radius = radius;
	rc.radius_sd = radius_sd;
	rc.mode_enabled = 1;
	if (iRecPtr->OutputParam.IsAutoMode) {
		rc.mode = rc.mode_enabled;
	}
	return ret;
}

bool iRec::Calibration::PassiveCalibration(INT64 iCount, int num)
{
	bool ret = false;

	std::deque<PointData> v;

	INT64 ed;

	if ((iRecPtr->ImgProcParam.pc.radius != 0)
		&& (iRecPtr->ImgProcParam.rc.radius != 0)
		&& (iRecPtr->ImgProcParam.pr.pcl != 0)) {
		return false;
	}

	ed = IsGazeMoveEnough(iCount, num, v);

	if (ed < 0) return ret;

	if (iRecPtr->ImgProcParam.pr.pcl == 0) {
		if (CalcPR_param(v)) {
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.pr = pr;
			}
#ifdef CONWIN	
			{
				wstringstream  wss;
				wss << endl;
				wss << (boost::wformat(L"pr.offx    :%f ") % pr.offx) << endl;
				wss << (boost::wformat(L"pr.offy    :%f ") % pr.offy) << endl;
				wss << (boost::wformat(L"pr.pcl     :%f ") % pr.pcl) << endl;
				conwin.write(wss.str());
			}
#endif
		}
	}

	if (iRecPtr->ImgProcParam.pc.radius == 0) {
		if (CalcPC_param(v)) {
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.pc = pc;
			}
#ifdef CONWIN
			{
				wstringstream  wss;
				wss << endl;
				wss << (boost::wformat(L"center.x    :%f ") % pc.ox) << endl;
				wss << (boost::wformat(L"center.y    :%f ") % pc.oy) << endl;
				wss << (boost::wformat(L"p_radius    :%f ") % pc.radius) << endl;
				wss << (boost::wformat(L"p_radius_sd :%f ") % pc.radius_sd) << endl;
				conwin.write(wss.str());
			}
#endif
		}
	}

	if ((iRecPtr->ImgProcParam.rc.radius == 0) && (iRecPtr->ImgProcParam.pc.radius != 0) && (iRecPtr->ImgProcParam.pr.pcl != 0)) {
		if (CalcRC_param(v)) {
			{
				std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
				iRecPtr->ImgProcParam.rc = rc;
			}
#ifdef CONWIN
			{
				wstringstream  wss;
				wss << endl;
				wss << (boost::wformat(L"rc.offx      :%f ") % rc.offx) << endl;
				wss << (boost::wformat(L"rc.offy      :%f ") % rc.offy) << endl;
				wss << (boost::wformat(L"center.x     :%f ") % rc.ox) << endl;
				wss << (boost::wformat(L"center.y     :%f ") % rc.oy) << endl;
				wss << (boost::wformat(L"rc.radius    :%f ") % rc.radius) << endl;
				wss << (boost::wformat(L"rc.radius_sd :%f ") % rc.radius_sd) << endl;
				conwin.write(wss.str());
			}
#endif
		}
	}

	return true;
}
void iRec::Calibration::ResetCalibration(void)
{
	pr.reset();
	pc.reset();
	rc.reset();
	current_gp.reset();
	current_gp_num = 0;
	
	{
		std::lock_guard<std::mutex> lock(iRecPtr->Mutex);
		iRecPtr->ImgProcParam.pr.reset();
		iRecPtr->ImgProcParam.pc.reset();
		iRecPtr->ImgProcParam.rc.reset();
	}
	for (int i = 0; i < MapPosNum; i++) {
		gp[i].reset();
		iRecPtr->CalibrationParam.pd[i].num = 0;
	}

}



INT_PTR iRec::Calibration::Open(FixationSection& fs)
{
	INT_PTR iptr;

	current_eye_deque.clear();
	current_gp.reset();
	for (INT64 i = fs.S_co; i <= fs.E_co; i++) {
		current_eye_deque.push_back(iRecPtr->Eye.v[i]);
		current_gp.Store(iRecPtr->Eye.v[i]);
	}
	current_gp.Calc();

	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_CALIBRESULT), hWndParent, CallbackProc, (LPARAM)this);


	return iptr;
}

void iRec::Calibration::SetBitmapCtrl(HWND hDlg, int ix, int iy)
{
	int isx, isy, iex, iey;
	double x, y, a, angle;

	bmp.reset(new DlgBitmap(hDlg, IDC_CALIB_BITMAP, static_cast<int>(current_gp.width.max), static_cast<int>(current_gp.height.max)));
	for (auto& cur : current_eye_deque) {
		bmp->grey_plot_img(cur.pupil.center.x, cur.pupil.center.y, cur.pupil.slope.s);
	}
	bmp->copyimg();

	if (iRecPtr->ImgProcParam.pc.radius != 0) {
		isx = 0;
		iex = static_cast<int>(current_gp.width.max);
		isy = static_cast<int>(iRecPtr->ImgProcParam.pc.oy + 0.5);
		iey = static_cast<int>(iRecPtr->ImgProcParam.pc.oy + 0.5);
		bmp->line(isx, isy, iex, iey, RGB(128, 128, 128), 1);
		isx = static_cast<int>(iRecPtr->ImgProcParam.pc.ox + 0.5);
		iex = static_cast<int>(iRecPtr->ImgProcParam.pc.ox + 0.5);
		isy = 0;
		iey = static_cast<int>(current_gp.height.max);
		bmp->line(isx, isy, iex, iey, RGB(128, 128, 128), 1);
	}
	x = current_gp.pcx.mean;
	y = current_gp.pcy.mean;

	angle = current_gp.pss_ang.mean;
	if (current_gp.psl_ang.sd < current_gp.pss_ang.sd) {
		angle = current_gp.psl_ang.mean + 90.0;
	}

	if (current_gp.psl.mean != 0) {
		a = tan(angle * M_PI / 180.0);
	}
	else {
		a = 100000;
	}
	bmp->calc_path(x, y, a, isx, isy, iex, iey);
	bmp->line(isx, isy, iex, iey, RGB(255, 0, 0), 2);

	bmp->refresh();
	bmp->move(ix, iy);
}

wstring iRec::Calibration::SetStrings(GazePoint& g)
{
	wstringstream wss;

	if (g.pcx.num != 0) {

		double angle, angle_sd;

		angle = g.pss_ang.mean;
		angle_sd = g.pss_ang.sd;

		if (g.psl_ang.sd < g.pss_ang.sd) {
			angle = g.psl_ang.mean + 90.0;
			angle_sd = g.psl_ang.sd;
		}
		if (angle > 90.0) angle -= 180.0;
		if (angle < -90.0) angle += 180.0;

		wss << (boost::wformat(L"pupil center mean \t\t: (%5.1f,%5.1f) pixel [sd %5.1f,%5.1f] \r") % g.pcx.mean % g.pcy.mean % g.pcx.sd % g.pcy.sd) << endl;
		wss << (boost::wformat(L"reflecton center mean \t: (%5.1f,%5.1f) pixel [sd %5.1f,%5.1f] \r") % g.rpx.mean % g.rpy.mean % g.rpx.sd % g.rpy.sd) << endl;
		wss << (boost::wformat(L"pupil minor axis slope\t:  %5.1f deg [sd %5.1f]\r") % angle % angle_sd) << endl;

		wss << (boost::wformat(L"minor axis(s)\t\t:  %5.1f pixel [sd %5.1f]\r") % g.pas.mean % g.pas.sd) << endl;
		wss << (boost::wformat(L"major axis(l)\t\t:  %5.1f pixel [sd %5.1f]\r") % g.pal.mean % g.pal.sd) << endl;
		wss << (boost::wformat(L"acos(s/l).mean\t\t:  %5.2f deg [sd %5.2f]\r") % g.pasl_ang.mean % g.pasl_ang.sd) << endl;
		wss << (boost::wformat(L"acos(s.mean/l.mean)\t:  %5.2f deg \r") % (acos(g.pas.mean / g.pal.mean) * 180.0 / M_PI)) << endl;
	}
	else {
		wss << L"Empty" << endl;
	}

	return wss.str();

}
void iRec::Calibration::SetInfo(HWND hDlg, int ix, int width)
{
	wstring ws;

	info.reset(new DlgEditBox(hDlg, IDC_EDIT_INFO));
	ws = SetStrings(current_gp);
	info->Set(ws);
	info->width = width;
	info->move(ix, bmp->ypos + bmp->height + 3);
}

void iRec::Calibration::SetStoredInfo(HWND hDlg, int ix, int width)
{
	wstring ws;

	stored_info.reset(new DlgEditBox(hDlg, IDC_EDIT_STORED));
	ws = SetStrings(gp[current_gp_num]);
	stored_info->Set(ws);
	stored_info->width = width;
	stored_info->move(ix, gaze_position->ypos + gaze_position->height + 3);
}

void iRec::Calibration::SetGazePoint(HWND hDlg)
{
	double x, y, z, h, v;
	gaze_position.reset(new DlgComboList(hDlg, IDC_COMBO_GAZE_POSITION));
	for (int i = 0; i < MapPosNum; i++) {
		wstringstream wss;
		x = iRecPtr->CalibrationParam.map[i * 5 + 0];
		y = iRecPtr->CalibrationParam.map[i * 5 + 1];
		z = iRecPtr->CalibrationParam.map[i * 5 + 2];
		h = iRecPtr->CalibrationParam.map[i * 5 + 3];
		v = iRecPtr->CalibrationParam.map[i * 5 + 4];

		wss << boost::wformat(TEXT("No.%d :")) % (i + 1);
		wss << boost::wformat(TEXT(" Gaze point viewing angle (h,v)=[%5.1f,%5.1f] deg")) % h % v;
		gaze_position->add(wss.str());
	}
	gaze_position->Set(current_gp_num);

	gaze_position->width = info->width;
	gaze_position->move(info->xpos, info->ypos + info->height + 10);
}
void iRec::Calibration::SetButton(HWND hDlg)
{
	int x;
	btn_ok.reset(new DlgButton(hDlg, IDOK));
	btn_cancel.reset(new DlgButton(hDlg, IDCANCEL));
	x = gaze_position->xpos + gaze_position->width - btn_ok->width;
	btn_ok->move(x, stored_info->ypos + stored_info->height + 10);
	btn_cancel->move(gaze_position->xpos, btn_ok->ypos);

}
bool iRec::Calibration::InitCtrl(HWND hDlg)
{
	int width, height, add_width, add_height, client_width, offset_x;

	RECT rw, rc;
	GetWindowRect(hDlg, &rw);
	GetClientRect(hDlg, &rc);
	add_width = (rw.right - rw.left) - (rc.right - rc.left);
	add_height = (rw.bottom - rw.top) - (rc.bottom - rc.top);

	client_width = static_cast<int>(current_gp.width.max);
	offset_x = 10;
	if (client_width < 340) {
		offset_x = (340 + 20 - client_width) / 2;
		client_width = 340;
	}
	width = client_width + add_width + 20;

	SetBitmapCtrl(hDlg, offset_x, 10);
	SetInfo(hDlg, 10, client_width);
	SetGazePoint(hDlg);
	SetStoredInfo(hDlg, 10, client_width);
	SetButton(hDlg);

	height = btn_ok->ypos + btn_ok->height + 10 + add_height;

	MoveWindow(hDlg, rw.left, rw.top, width, height, true);

	return true;
}


void iRec::Calibration::EndCtrl(HWND hDlg)
{

}

int iRec::Calibration::SetNextGP(void)
{
	int startpoint;
	startpoint = current_gp_num;
	do {
		if (++current_gp_num >= MapPosNum) current_gp_num = 0;
		if (gp[current_gp_num].pcx.num == 0) break;
	} while (current_gp_num != startpoint);

	if (current_gp_num == startpoint) current_gp_num = 0;

	return current_gp_num;
}
bool iRec::Calibration::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM)
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

		switch (wmId)
		{
		case IDC_COMBO_GAZE_POSITION:
			if (wmEvent == CBN_SELCHANGE) {
				current_gp_num = gaze_position->Get();
				wstring ws = SetStrings(gp[current_gp_num]);
				stored_info->Set(ws);
			}
			break;
		case IDOK:
			gp[current_gp_num] = current_gp;
			Calc_ObjParam();
			{
				int ngp = SetNextGP();
				wstringstream wss;
				wss << ngp << endl;
				conwin.write(wss.str());
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

INT_PTR CALLBACK iRec::Calibration::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Calibration* thisPtr = NULL;

	switch (message) {
	case WM_INITDIALOG:
		thisPtr = (Calibration*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
		break;
	case WM_DESTROY:
		thisPtr = (Calibration*)GetWindowLongPtr(hDlg, DWLP_USER);
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)0);
		break;
	default:
		thisPtr = (Calibration*)GetWindowLongPtr(hDlg, DWLP_USER);
		break;
	}
	if (thisPtr) {
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}