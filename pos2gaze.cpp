#include "framework.h"
#include "iRecHS2s.h"

bool trans_param::det()
{
	double detA;
	double im0, im1, im2;
	im0 = tr[4] * tr[8] - tr[5] * tr[7];
	im1 = tr[2] * tr[7] - tr[1] * tr[8];
	im2 = tr[1] * tr[5] - tr[2] * tr[4];

	detA = tr[0] * im0 + tr[3] * im1 + tr[6] * im2;
	if (fabs(detA) <= 0.0001) return false;
	return true;
}
bool trans_param::InverseMatrix(double* m, double* im)
{
	double detA;
	int i;
	im[0] = m[4] * m[8] - m[5] * m[7];
	im[3] = m[5] * m[6] - m[3] * m[8];
	im[6] = m[3] * m[7] - m[4] * m[6];
	im[1] = m[2] * m[7] - m[1] * m[8];
	im[4] = m[0] * m[8] - m[2] * m[6];
	im[7] = m[1] * m[6] - m[0] * m[7];
	im[2] = m[1] * m[5] - m[2] * m[4];
	im[5] = m[2] * m[3] - m[0] * m[5];
	im[8] = m[0] * m[4] - m[1] * m[3];
	detA = m[0] * im[0] + m[3] * im[1] + m[6] * im[2];
	if (fabs(detA) <= 0.0001) {
		for (i = 0; i < 9; i++) im[i] = 0;
		return false;
	}
	for (i = 0; i < 9; i++) {
		im[i] = im[i] / detA;
	}
	return true;
}
bool trans_param::obj_vec2ang(GazeVec& gv, GazeAngle& ga) {
	if (mode_enabled < 2) return false;
	ga.x = atan2(gv.x, gv.z) * 180 / M_PI;
	ga.y = atan2(gv.y, gv.z) * 180 / M_PI;
	return true;
}

bool trans_param::cam_vec2ang(GazeVec& gv, XY_double& flip, GazeAngle& ga)
{
	ga.x = (atan2(gv.x, gv.z) * 180 / M_PI)*flip.x;
	ga.y = (atan2(gv.y, gv.z) * 180 / M_PI)*flip.y;
	return true;
}
bool trans_param::objVec(PointData& p, GazeVec& gv)
{
	double len;
	len = p.objx * p.objx + p.objy * p.objy + p.objz * p.objz;
	if (len == 0) return false;
	len = sqrt(len);
	gv.x = p.objx / len;
	gv.y = p.objy / len;
	gv.z = p.objz / len;
	return true;
}
bool trans_param::objAng1(GazeAngle& cga, GazeAngle& oga)
{
	if (mode_enabled < 2) return false;
	if (mode_enabled <4) {
		double cax, cay;
		cax = (cga.x - offset_ax) * exprate;
		cay = (cga.y - offset_ay) * exprate;
		oga.x = rotation_cos * cax - rotation_sin * cay + offset_ox;
		oga.y = rotation_sin * cax + rotation_cos * cay + offset_oy;
	}
	return true;
}

bool trans_param::camVec2objAng(GazeVec& cv, GazeAngle& oa)
{
	GazeVec ov;
	ov.x = tr[0] * cv.x + tr[1] * cv.y + tr[2] * cv.z;
	ov.y = tr[3] * cv.x + tr[4] * cv.y + tr[5] * cv.z;
	ov.z = tr[6] * cv.x + tr[7] * cv.y + tr[8] * cv.z;
	if (ov.z == 0) return false;
	oa.x = atan2(ov.x, ov.z) * 180 / M_PI;
	oa.y = atan2(ov.y, ov.z) * 180 / M_PI;
	return true;
}



bool trans_param::CalcMatrix(std::deque<PointData>& v)
{
	GazeVec cv,ov;
	GazeAngle ca, oa;

	double im[9], m[9];
	double x2, y2, z2, xy, yz, zx, xxi, yxi, zxi, xyi, yyi, zyi, xzi, yzi, zzi, num;
	x2 = y2 = z2 = xy = yz = zx = xxi = yxi = zxi = xyi = yyi = zyi = xzi = yzi = zzi = num = 0;
	for (int i = 0; i < 9; i++) {
		tr[i] = itr[i] = 0;
	}
	if (v.size() < 3) return false;

	for (auto& pd: v) {
		if (!camVec(pd, cv)) break;
		if (!objVec(pd, ov)) break;

		x2 += cv.x * cv.x;
		y2 += cv.y * cv.y;
		z2 += cv.z * cv.z;
		xy += cv.x * cv.y;
		yz += cv.y * cv.z;
		zx += cv.z * cv.x;
		xxi += cv.x * ov.x;
		yxi += cv.y * ov.x;
		zxi += cv.z * ov.x;
		xyi += cv.x * ov.y;
		yyi += cv.y * ov.y;
		zyi += cv.z * ov.y;
		xzi += cv.x * ov.z;
		yzi += cv.y * ov.z;
		zzi += cv.z * ov.z;
		num++;
	}
	if (num < 3) return false;

	m[3 * 0 + 0] = x2; m[3 * 0 + 1] = xy; m[3 * 0 + 2] = zx;
	m[3 * 1 + 0] = xy; m[3 * 1 + 1] = y2; m[3 * 1 + 2] = yz;
	m[3 * 2 + 0] = zx; m[3 * 2 + 1] = yz; m[3 * 2 + 2] = z2;
	if (!InverseMatrix(m, im)) return false;

	tr[0] = im[0] * xxi + im[1] * yxi + im[2] * zxi;
	tr[3] = im[0] * xyi + im[1] * yyi + im[2] * zyi;
	tr[6] = im[0] * xzi + im[1] * yzi + im[2] * zzi;
	tr[1] = im[3] * xxi + im[4] * yxi + im[5] * zxi;
	tr[4] = im[3] * xyi + im[4] * yyi + im[5] * zyi;
	tr[7] = im[3] * xzi + im[4] * yzi + im[5] * zzi;
	tr[2] = im[6] * xxi + im[7] * yxi + im[8] * zxi;
	tr[5] = im[6] * xyi + im[7] * yyi + im[8] * zyi;
	tr[8] = im[6] * xzi + im[7] * yzi + im[8] * zzi;

	if (!InverseMatrix(tr, itr)) {
		for (int i = 0; i < 9; i++) tr[i] = 0;
		return false;
	}
	return true;
}



bool trans_param::CalcObjParam(std::deque<PointData>& v,XY_double& flip)
{
	GazeVec gv0, gv1;
	GazeAngle ga0, ga1;
	double x1, x2, y1, y2, lenC, lenO;
	if (v.size() < 1) return false;
	if (mode_enabled < 1) return false;

	camVec(v[0], gv0);
	cam_vec2ang(gv0, flip, ga0);
	exprate = 1;
	offset_ax = ga0.x - v[0].objax;
	offset_ay = ga0.y - v[0].objay;
	offset_ox = v[0].objax;
	offset_oy = v[0].objay;
	rotation_sin = 0;
	rotation_cos = 1;

	if (v.size() == 1) {
		mode_enabled = 2;
		return true;
	}

  if (v.size() ==2){
		camVec(v[1], gv1);
		cam_vec2ang(gv1, flip, ga1);

		x1 = ga1.x - ga0.x;
		y1 = ga1.y - ga0.y;
		x2 = v[1].objax - v[0].objax;
		y2 = v[1].objay - v[0].objay;

		lenC = sqrt(x1 * x1 + y1 * y1);
		lenO = sqrt(x2 * x2 + y2 * y2);

		if (lenC == 0) return false;

		exprate = lenO / lenC;
		if (exprate > 10) {
			exprate = 1;
			return false;
		}
		x1 = x1 * exprate;
		y1 = y1 * exprate;
		rotation_sin = (x1 * y2 - x2 * y1) / (x1 * x1 + y1 * y1);
		rotation_cos = (x1 * x2 + y1 * y2) / (x1 * x1 + y1 * y1);
		mode_enabled = 3;
		return true;
	}

	if (v.size() > 2) {
		if (!CalcMatrix(v)) return false;
		mode_enabled = 4;
	}
	return true;
}




void trans_param::reset(void)
{
	mode = mode_enabled = 0;
	exprate = 0;
	offset_ax = offset_ay = 0;
	offset_ox = offset_oy = 0;
	rotation_cos = 1;
	rotation_sin = 0;
	for (int i = 0; i < 9; i++) {
		tr[i] = 0;
	}
}
trans_param::trans_param()
{
	reset();
}

void pc_param::reset()
{
	trans_param::reset();
	ox = oy = radius = radius_sd = 0;
}

bool pc_param::camVec(ellipse& p, reflect& r, GazeVec& gv)
{
	if (radius == 0) return false;
	if (p.size.w == 0) return false;
	gv.x = (p.center.x - ox) / radius;
	gv.y = (p.center.y - oy) / radius;
	gv.z = sqrt(fabs(1 - gv.x * gv.x - gv.y * gv.y));
	return true;
}
bool pc_param::camVec(PointData& p, GazeVec& gv)
{
	if (radius == 0) return false;
	gv.x = (p.pcx - ox) / radius;
	gv.y = (p.pcy - oy) / radius;
	gv.z = sqrt(fabs(1 - gv.x * gv.x - gv.y * gv.y));
	return true;
}

pc_param::pc_param()
{
	reset();
}


void pr_param::reset()
{
	trans_param::reset();
	offx = offy = pcl = pcl_sd = 0;
}

bool pr_param::camVec(ellipse& p, reflect& r, GazeVec& gv)
{
	if (pcl == 0) return false;
	if (p.size.w == 0) return false;
	if (r.size.w == 0) return false;
	gv.x = (p.center.x - r.pos.x + offx) / pcl;
	gv.y = (p.center.y - r.pos.y + offy) / pcl;
	gv.z = sqrt(fabs(1 - gv.x * gv.x - gv.y * gv.y));
	return true;
}
bool pr_param::camVec(PointData& p, GazeVec& gv)
{
	if (pcl == 0) return false;
	gv.x = (p.pcx - p.rpx + offx) / pcl;
	gv.y = (p.pcy - p.rpy + offy) / pcl;
	gv.z = sqrt(fabs(1 - gv.x * gv.x - gv.y * gv.y));
	return true;
}

pr_param::pr_param()
{
	reset();
}

void rc_param::reset()
{
	trans_param::reset();
	ox = oy = radius = offx = offy = radius_sd = 0;
}

bool rc_param::camVec(ellipse& p, reflect& r, GazeVec& gv)
{
	if (radius == 0) return false;
	if (p.size.w == 0) return false;
	if (r.size.w == 0) return false;
	gv.x = (r.pos.x - offx - ox) / radius;
	gv.y = (r.pos.y - offy - oy) / radius;
	gv.z = sqrt(fabs(1 - gv.x * gv.x - gv.y * gv.y));
	return true;
}
bool rc_param::camVec(PointData& p, GazeVec& gv)
{
	if (radius == 0) return false;
	gv.x = (p.rpx - offx - ox) / radius;
	gv.y = (p.rpy - offy - oy) / radius;
	gv.z = sqrt(fabs(1 - gv.x * gv.x - gv.y * gv.y));
	return true;
}

rc_param::rc_param()
{
	reset();
}
