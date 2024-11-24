#include "framework.h"
#include "iRecHS2s.h"


NRS::NRS()
{
	clear();
}
NRS::~NRS()
{
	clear();
}

void NRS::clear()
{
	int i;
	head = num = 0;
	for (i = 0; i < NRS_MAX; i++) {
		v[i] = m[i] = 0;
	}
}

void NRS::store(double data)
{
	v[head] = data;
	head++;
	if (head >= NRS_MAX) head = 0;
	if (num < NRS_MAX) 	num++;

}

double NRS::average(int points)
{
	int i, position;
	double sum;
	if (num == 0) return 0;
	if (points <= 0) return 0;
	if (num < points) return v[head - 1];

	if (points > NRS_MAX) points = NRS_MAX;

	for (i = 0, position = head - 1, sum = 0; i < points; i++, position--) {
		if (position < 0) position += NRS_MAX;
		sum += v[position];
	}
	return sum / points;
}


static int compare_double(const void* a, const void* b)
{
	double aa, bb;
	aa = *(double*)a;
	bb = *(double*)b;
	if (aa - bb > 0) return 1;
	if (bb - aa > 0) return -1;
	return 0;
}

double NRS::median(int points)
{
	int i, position;


	if (num == 0) return 0;
	if (points <= 0) return 0;

	if (points > NRS_MAX) points = NRS_MAX;
	if (num < points) return v[head - 1];


	for (i = 0, position = head - 1; i < points; i++, position--) {
		if (position < 0) position += NRS_MAX;
		m[i] = v[position];
	}
	qsort(m, points, sizeof(double), compare_double);


	return m[points / 2];


}




void EyeNumData::CalcPC()
{
	if (!pc.camVec(pupil,ref, GazePC.camVec)) return;
	if (!pc.cam_vec2ang(GazePC.camVec, OutParam.Flip,GazePC.camAng)) return;
	if (pc.mode_enabled < 4) {
		if (!pc.objAng1(GazePC.camAng, GazePC.objAng)) return;
	}
	if (pc.mode_enabled == 4) {
		if (!pc.camVec2objAng(GazePC.camVec, GazePC.objAng)) return;
	}
}


void EyeNumData::CalcPR()
{
	if (!pr.camVec(pupil,ref, GazePR.camVec)) return;
	if (!pr.cam_vec2ang(GazePR.camVec, OutParam.Flip, GazePR.camAng)) return;
	if (pr.mode_enabled < 4) {
		if (!pr.objAng1(GazePR.camAng, GazePR.objAng)) return;
	}
	if (pr.mode_enabled == 4) {
		if (!pr.camVec2objAng(GazePR.camVec, GazePR.objAng)) return;
	}
}


void EyeNumData::CalcRC()
{
	if (!rc.camVec(pupil, ref, GazeRC.camVec)) return;
	if (!rc.cam_vec2ang(GazeRC.camVec, OutParam.Flip, GazeRC.camAng)) return;
	if (rc.mode_enabled < 4) {
		if (!rc.objAng1(GazeRC.camAng, GazeRC.objAng)) return;
	}
	if (rc.mode_enabled == 4) {
		if (!rc.camVec2objAng(GazeRC.camVec, GazeRC.objAng)) return;
	}
}


EyeData::EyeData()
{

}

EyeData::~EyeData()
{
	v.clear();
}


void EyeData::Store(EyeImage& ei, INT64 co)
{
	EyeNumData ed;
	static int prev_method = 0;
	static int prev_mode = 0;

	ed.Counter = co;
	ed.TimeStamp = ei.ci.TimeStamp;
	ed.FrameID = ei.ci.FrameID;
	ed.CPU_FreqCount = ei.ci.CPU_FreqCount;
	ed.OutParam = ei.OutParam;
	ed.pupil = ei.pupil;
	ed.ref = ei.ref;
	ed.pr = ei.ImgParam.pr;
	ed.pc = ei.ImgParam.pc;
	ed.rc = ei.ImgParam.rc;
	ed.CalcPC();
	ed.CalcRC();
	ed.CalcPR();
	ed.Gaze.method = ei.OutParam.method;
	ed.img.w = ei.ci.w;
	ed.img.h = ei.ci.h;

	switch (ed.Gaze.method) {
	case Output_Parameter::PR_METHOD:
		switch (ed.pr.mode) {
		case 0:
			if (ed.OutParam.Flip.x == 1) {
				ed.Gaze.x = ed.pupil.center.x;
			}
			else {
				ed.Gaze.x = ed.img.w - ed.pupil.center.x;
			}
			if (ed.OutParam.Flip.y == 1) {
				ed.Gaze.y = ed.pupil.center.y;
			}
			else {
				ed.Gaze.y = ed.img.h - ed.pupil.center.y;
			}
			ed.Gaze.mode = 0;
			break;
		case 1:
			ed.Gaze.x = ed.GazePR.camAng.x ;
			ed.Gaze.y = ed.GazePR.camAng.y;
			ed.Gaze.mode = 1;
			break;
		case 2:
			ed.Gaze.x = ed.GazePR.objAng.x;
			ed.Gaze.y = ed.GazePR.objAng.y;
			ed.Gaze.mode = 2;
			break;
		}
		break;
	case Output_Parameter::PC_METHOD:
		switch (ed.pc.mode) {
		case 0:
			if (ed.OutParam.Flip.x == 1) {
				ed.Gaze.x = ed.pupil.center.x;
			}
			else {
				ed.Gaze.x = ed.img.w - ed.pupil.center.x;
			}
			if (ed.OutParam.Flip.y== 1) {
				ed.Gaze.y = ed.pupil.center.y;
			}
			else {
				ed.Gaze.y = ed.img.h - ed.pupil.center.y;
			}
			ed.Gaze.mode = 0;
			break;
		case 1:
			ed.Gaze.x = ed.GazePC.camAng.x;
			ed.Gaze.y = ed.GazePC.camAng.y;
			ed.Gaze.mode = 1;
			break;
		case 2:
			ed.Gaze.x = ed.GazePC.objAng.x;
			ed.Gaze.y = ed.GazePC.objAng.y;
			ed.Gaze.mode = 2;
			break;
		}
		break;
	case Output_Parameter::RC_METHOD:
		switch (ed.rc.mode) {
		case 0:
			if (ed.OutParam.Flip.x == 1) {
				ed.Gaze.x = ed.ref.pos.x;
			}
			else {
				ed.Gaze.x = ed.img.w - ed.ref.pos.x;
			}
			if (ed.OutParam.Flip.y == 1) {
				ed.Gaze.y = ed.ref.pos.y;
			}
			else {
				ed.Gaze.y = ed.img.h - ed.ref.pos.y;
			}
			ed.Gaze.y = ed.ref.pos.y;
			ed.Gaze.mode = 0;
			break;
		case 1:
			ed.Gaze.x = ed.GazeRC.camAng.x;
			ed.Gaze.y = ed.GazeRC.camAng.y;
			ed.Gaze.mode = 1;
			break;
		case 2:
			ed.Gaze.x = ed.GazeRC.objAng.x;
			ed.Gaze.y = ed.GazeRC.objAng.y;
			ed.Gaze.mode = 2;
			break;
		}
		break;
	}

	if (prev_mode != ed.Gaze.mode || prev_method != ed.Gaze.method) {
		nr_x.clear();
		nr_y.clear();
		nr_l.clear();
		nr_openratio.clear();
	}
	prev_mode = ed.Gaze.mode;
	prev_method = ed.Gaze.method;
	ed.Gaze.l = ed.pupil.axis.l;
	ed.Gaze.openratio = ed.pupil.OpenRatio;
	nr_x.store(ed.Gaze.x);
	nr_y.store(ed.Gaze.y);
	nr_l.store(ed.Gaze.l);
	nr_openratio.store(ed.Gaze.openratio);

	switch (ed.OutParam.nr_mode) {
	case Output_Parameter::NONE:
		break;
	case Output_Parameter::MOVING_AVERAGE:
		ed.Gaze.x = nr_x.average(ed.OutParam.mv_num);
		ed.Gaze.y = nr_y.average(ed.OutParam.mv_num);
		ed.Gaze.l = nr_l.average(ed.OutParam.mv_num);
		ed.Gaze.openratio = nr_openratio.average(ed.OutParam.mv_num);
		break;
	case  Output_Parameter::MEDIAN:
		ed.Gaze.x = nr_x.median(ed.OutParam.me_num);
		ed.Gaze.y = nr_y.median(ed.OutParam.me_num);
		ed.Gaze.l = nr_l.median(ed.OutParam.me_num);
		ed.Gaze.openratio = nr_openratio.average(ed.OutParam.me_num);
		break;
	}
	current = ed;
	v.push_back(ed);
}