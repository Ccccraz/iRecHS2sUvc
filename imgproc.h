#pragma once

//#define CONWIN 1

namespace boost {
	namespace serialization {
		template<class Archive>  void serialize(Archive& ar, RECT& r, unsigned int ver)
		{
			ar& make_nvp("left", r.left);
			ar& make_nvp("top", r.top);
			ar& make_nvp("right", r.right);
			ar& make_nvp("bottom", r.bottom);
		}
	}
}

struct GazeVec {
	double x, y, z;
	GazeVec() {
		x = y = z = 0;
	}
};

struct GazeAngle {
	double x, y;
	GazeAngle() {
		x = y = 0;
	}
};

//struct pr_param {
//	double offx, offy, pcl, pcl_sd, tr[9];
//	double exprate, offset_ax, offset_ay;
//	int mode, mode_enabled;
//	void reset() {
//		mode = mode_enabled = 0;
//		offx = offy = pcl = pcl_sd = 0;
//		exprate = 0;
//		offset_ax = offset_ay = 0;
//		for (int i = 0; i < 9; i++) {
//			tr[i] = 0;
//		}
//	}
//	pr_param() {
//		reset();
//	}
//
//private:
//	friend class boost::serialization::access;
//	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
//	{
//		ar& BOOST_SERIALIZATION_NVP(offx);
//		ar& BOOST_SERIALIZATION_NVP(offy);
//		ar& BOOST_SERIALIZATION_NVP(pcl);
//		ar& BOOST_SERIALIZATION_NVP(tr);
//		ar& BOOST_SERIALIZATION_NVP(mode);
//		ar& BOOST_SERIALIZATION_NVP(mode_enabled);
//		ar& BOOST_SERIALIZATION_NVP(exprate);
//		ar& BOOST_SERIALIZATION_NVP(offset_ax);
//		ar& BOOST_SERIALIZATION_NVP(offset_ay);
//	}
//};
//
//struct rc_param {
//	double ox, oy, radius, radius_sd, tr[9], offx, offy;
//	double exprate, offset_ax, offset_ay;
//	int mode, mode_enabled;
//	void reset() {
//		mode = mode_enabled = 0;
//		ox = oy = radius = offx = offy = radius_sd = 0;
//		exprate = 0;
//		offset_ax = offset_ay = 0;
//		for (int i = 0; i < 9; i++) {
//			tr[i] = 0;
//		}
//	}
//	rc_param() {
//		reset();
//	}
//private:
//	friend class boost::serialization::access;
//	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
//	{
//		ar& BOOST_SERIALIZATION_NVP(ox);
//		ar& BOOST_SERIALIZATION_NVP(oy);
//		ar& BOOST_SERIALIZATION_NVP(radius);
//		ar& BOOST_SERIALIZATION_NVP(offx);
//		ar& BOOST_SERIALIZATION_NVP(offy);
//		ar& BOOST_SERIALIZATION_NVP(tr);
//		ar& BOOST_SERIALIZATION_NVP(mode);
//		ar& BOOST_SERIALIZATION_NVP(mode_enabled);
//		ar& BOOST_SERIALIZATION_NVP(exprate);
//		ar& BOOST_SERIALIZATION_NVP(offset_ax);
//		ar& BOOST_SERIALIZATION_NVP(offset_ay);
//	}
//};

struct AngleFreq {
	double angle;
	int freq;
	AngleFreq() {
		angle = 0;
		freq = 0;
	}
};

struct RefPoint {
	int amount;
	int length;
	int index;
};

struct LABEL {
	int l, r, t, b;
	int co;
	LABEL() {
		l = r = t = b = 0;
		co = 0;
	}
};

struct XY_int {
	int x;
	int y;
};

struct XY_double {
	double x;
	double y;
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(x);
		ar& BOOST_SERIALIZATION_NVP(y);
	}
};

struct lere_bool {
	bool le;
	bool re;

};

struct WH_int
{
	int w, h;
};

struct WHWH_int {
	int w, h, wh;
};

struct WH_double {
	double w, h;
};

struct ellipse_eq { double a, b, c, d, e; };

struct LS_double {
	double l, s;
};

struct ellipse {
	ellipse_eq eq;
	XY_double center;
	LS_double axis;
	LS_double slope;
	WH_double size;
	RECT roi;
	double percentx;
	double densityx;
	double UnderThresholdArea;
	double OpenRatio;
	ellipse() {
		eq.a = eq.b = eq.c = eq.d = eq.e = 0;
		center.x = center.y = 0;
		axis.l = axis.s = 0;
		slope.l = slope.s = 0;
		size.w = size.h = 0;
		percentx = 0;
		densityx = 0;
		UnderThresholdArea = 0;
		OpenRatio = 0;
		roi = { 0,0,0,0 };
	}
};

struct reflect {
	XY_double pos;
	WH_double size;
	RECT range;
	reflect() {
		pos = { 0,0 };
		size = { 0,0 };
		range = { 0,0,0,0 };
	}
};

struct ALine { // Angle Line
	XY_int le;   //left edge
	XY_int re;     //right edge
	XY_double ce; //center of edge
	int AngMax;
	int ang[181] = {};
	lere_bool errflag;


	ALine() {
		le = { 0,0 };
		re = { 0,0 };
		ce = { 0,0 };
		AngMax = 0;
		errflag = { false,false };
	}
};

struct YEdge {
	int x, y;
	bool flag;
	YEdge() {
		x = y = 0;
		flag = false;
	}
};

struct PointData {
	double pcx, pcy, rpx, rpy, pas, pal, pss, psl;
	double objx, objy, objz, objax, objay;
	int num;

	PointData() {
		pcx = pcy = rpx = rpy = pas = pal = pss = psl = 0;
		objx = objy = objz = objax = objay = 0;
		num = 0;
	}
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(pcx);
		ar& BOOST_SERIALIZATION_NVP(pcy);
		ar& BOOST_SERIALIZATION_NVP(rpx);
		ar& BOOST_SERIALIZATION_NVP(rpy);
		ar& BOOST_SERIALIZATION_NVP(pas);
		ar& BOOST_SERIALIZATION_NVP(pal);
		ar& BOOST_SERIALIZATION_NVP(pss);
		ar& BOOST_SERIALIZATION_NVP(psl);
		ar& BOOST_SERIALIZATION_NVP(objx);
		ar& BOOST_SERIALIZATION_NVP(objy);
		ar& BOOST_SERIALIZATION_NVP(objz);
		ar& BOOST_SERIALIZATION_NVP(objax);
		ar& BOOST_SERIALIZATION_NVP(objay);
		ar& BOOST_SERIALIZATION_NVP(num);
	}
};

class trans_param {
public:
	double tr[9], itr[9];
	double exprate, offset_ax, offset_ay;
	double rotation_cos, rotation_sin,offset_ox,offset_oy;
	int mode, mode_enabled;
	virtual void reset();
	virtual bool camVec(PointData& p, GazeVec& gv) = 0;
	virtual bool camVec(ellipse& p, reflect& r,GazeVec& gv) = 0;

	bool objVec(PointData& p, GazeVec& gv);
	bool det();
	bool InverseMatrix(double* m, double* im);
	bool obj_vec2ang(GazeVec& gv, GazeAngle& ga);
	bool cam_vec2ang(GazeVec& gv, XY_double&flip,GazeAngle& ga);
	bool objAng1(GazeAngle& cga, GazeAngle& oga);
	bool camVec2objAng(GazeVec& gv, GazeAngle& ga);
	bool CalcObjParam(std::deque<PointData>& v, XY_double& flip);
	bool CalcMatrix(std::deque<PointData>& v);
	trans_param();

};



class pc_param : public trans_param
{
public:
	double ox, oy, radius, radius_sd;
	void reset();
	bool camVec(ellipse& p, reflect& r, GazeVec& gv);
	bool camVec(PointData& p, GazeVec& gv);
	pc_param();
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(ox);
		ar& BOOST_SERIALIZATION_NVP(oy);
		ar& BOOST_SERIALIZATION_NVP(radius);
		ar& BOOST_SERIALIZATION_NVP(tr);
		ar& BOOST_SERIALIZATION_NVP(mode);
		ar& BOOST_SERIALIZATION_NVP(mode_enabled);
		ar& BOOST_SERIALIZATION_NVP(exprate);
		ar& BOOST_SERIALIZATION_NVP(offset_ax);
		ar& BOOST_SERIALIZATION_NVP(offset_ay);
		ar& BOOST_SERIALIZATION_NVP(rotation_cos);
		ar& BOOST_SERIALIZATION_NVP(rotation_sin);
	}
};

class pr_param : public trans_param
{
public:
	double offx, offy, pcl, pcl_sd;
	void reset();
	bool camVec(ellipse& p, reflect& r, GazeVec& gv);
	bool camVec(PointData& p, GazeVec& gv);
	pr_param();
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(offx);
		ar& BOOST_SERIALIZATION_NVP(offy);
		ar& BOOST_SERIALIZATION_NVP(pcl);
		ar& BOOST_SERIALIZATION_NVP(tr);
		ar& BOOST_SERIALIZATION_NVP(mode);
		ar& BOOST_SERIALIZATION_NVP(mode_enabled);
		ar& BOOST_SERIALIZATION_NVP(exprate);
		ar& BOOST_SERIALIZATION_NVP(offset_ax);
		ar& BOOST_SERIALIZATION_NVP(offset_ay);
		ar& BOOST_SERIALIZATION_NVP(rotation_cos);
		ar& BOOST_SERIALIZATION_NVP(rotation_sin);
	}
};

class rc_param : public trans_param
{
public:
	double ox, oy, radius, radius_sd, offx, offy, pcl, pcl_sd;
	void reset();
	bool camVec(ellipse& p, reflect& r, GazeVec& gv);
	bool camVec(PointData& p, GazeVec& gv);

	rc_param();
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(ox);
		ar& BOOST_SERIALIZATION_NVP(oy);
		ar& BOOST_SERIALIZATION_NVP(radius);
		ar& BOOST_SERIALIZATION_NVP(offx);
		ar& BOOST_SERIALIZATION_NVP(offy);
		ar& BOOST_SERIALIZATION_NVP(tr);
		ar& BOOST_SERIALIZATION_NVP(mode);
		ar& BOOST_SERIALIZATION_NVP(mode_enabled);
		ar& BOOST_SERIALIZATION_NVP(exprate);
		ar& BOOST_SERIALIZATION_NVP(offset_ax);
		ar& BOOST_SERIALIZATION_NVP(offset_ay);
		ar& BOOST_SERIALIZATION_NVP(rotation_cos);
		ar& BOOST_SERIALIZATION_NVP(rotation_sin);
	}
};



class Cluster {
public:
	int nlabel;
	WHWH_int rsize;
	WH_int imgsize;
	LABEL* labelPtr;
	unsigned int* labelimgPtr;
	Cluster();
	~Cluster();
	void SetArray(int w, int h);
	void CheckPoint(int x, int y);
	int JoinGroup(int id1, int id2);
};

class PupilEdge {
public:
	WH_int cur, mem; //current & memory
	ALine* Xe;
	YEdge* Ye;
	int x_num, y_num;
	PupilEdge();
	~PupilEdge();
	void Set(int w, int h);
};

class EP {
private:
	double x, y, xy;
	double x2, y2, x2y, xy2, x2y2;
	double x3, y3, x3y, xy3;
	double y4, n;

public:
	double minx, maxx, miny, maxy;



	EP();
	void Init(ellipse& pupil);
	int Store(int x, int y);

	int Calc(ellipse& pupil, int w, int h, int l = 0);
};

struct ImgProc_Parameter
{
public:
	int PupilThreshold;
	int RefThreshold;
	int PermissiveAngle;
	int PermissiveLength;
	bool IsTopAcquisition;
	bool IsBottomAcquisition;
	bool IsGleaningAcquisition;
	bool IsMovingROI;
	bool IsUsePreviousImage;
	int SearchEdgeLength;
	int BlinkThreshold;
	RECT roi;
	pc_param pc;
	pr_param pr;
	rc_param rc;

	ImgProc_Parameter()
	{
		PupilThreshold = 36;
		RefThreshold = 240;
		PermissiveAngle = 3;
		PermissiveLength = 3;
		IsTopAcquisition = true;
		IsBottomAcquisition = true;
		IsGleaningAcquisition = true;
		IsMovingROI = false;
		IsUsePreviousImage = true;
		SearchEdgeLength = 16;
		BlinkThreshold = 50;
		roi = { 0,0,0,0 };
	}
#ifdef CONWIN
	void print() {
		std::wstringstream wss;
		wss << L"PupilThreshold        : " << PupilThreshold << endl;
		wss << L"RefThreshold          : " << RefThreshold << endl;
		wss << L"PermissiveAngle       : " << PermissiveAngle << endl;
		wss << L"PermissiveLength      : " << PermissiveLength << endl;
		wss << L"IsTopAcquisition      : " << IsTopAcquisition << endl;
		wss << L"IsBottomAcquisition   : " << IsBottomAcquisition << endl;
		wss << L"IsGleaningAcquisition : " << IsGleaningAcquisition << endl;
		wss << L"IsMovighROI           : " << IsMovingROI << endl;
		wss << L"SearchEdgeLength      : " << SearchEdgeLength << endl;
		wss << L"roi.left              : " << roi.left << endl;
		wss << L"roi.top               : " << roi.top << endl;
		wss << L"roi.right             : " << roi.right << endl;
		wss << L"roi.bottom            : " << roi.bottom << endl;
		conwin.write(wss.str());
	}
#else 
	void print() {}
#endif
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(PupilThreshold);
		ar& BOOST_SERIALIZATION_NVP(RefThreshold);
		ar& BOOST_SERIALIZATION_NVP(PermissiveAngle);
		ar& BOOST_SERIALIZATION_NVP(PermissiveLength);
		ar& BOOST_SERIALIZATION_NVP(IsTopAcquisition);
		ar& BOOST_SERIALIZATION_NVP(IsBottomAcquisition);
		ar& BOOST_SERIALIZATION_NVP(IsGleaningAcquisition);
		ar& BOOST_SERIALIZATION_NVP(IsMovingROI);
		ar& BOOST_SERIALIZATION_NVP(IsUsePreviousImage);
		ar& BOOST_SERIALIZATION_NVP(SearchEdgeLength);
		ar& BOOST_SERIALIZATION_NVP(BlinkThreshold);
		ar& BOOST_SERIALIZATION_NVP(roi);
		ar& BOOST_SERIALIZATION_NVP(pc);
		ar& BOOST_SERIALIZATION_NVP(pr);
		ar& BOOST_SERIALIZATION_NVP(rc);
	}
};

class Output_Parameter
{
public:
	enum OUTPUT_METHOD :int { PC_METHOD = 0, PR_METHOD = 1, RC_METHOD = 2 };
	enum OUTPUT_NOISEREDUCTION : int { NONE = 0, MOVING_AVERAGE = 1, MEDIAN = 2 };
	OUTPUT_METHOD method;
	OUTPUT_NOISEREDUCTION nr_mode;
	int mv_num;
	int me_num;
	bool IsAutoMode;
	bool IsSave;
	bool IsSaveDev;
	//	double FlipH;
	//	double FlipV;
	XY_double Flip;
	Output_Parameter() {
		method = Output_Parameter::PR_METHOD;
		IsAutoMode = true;
		nr_mode = Output_Parameter::NONE;
		mv_num = 10;
		me_num = 11;
		Flip.x = -1;
		Flip.y = -1;
		IsSave = true;
		IsSaveDev = false;
		//		FlipH = -1;
		//		FlipV = -1;
	}
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(method);
		ar& BOOST_SERIALIZATION_NVP(IsAutoMode);
		ar& BOOST_SERIALIZATION_NVP(IsSave);
		ar& BOOST_SERIALIZATION_NVP(IsSaveDev);
		ar& BOOST_SERIALIZATION_NVP(nr_mode);
		ar& BOOST_SERIALIZATION_NVP(mv_num);
		ar& BOOST_SERIALIZATION_NVP(me_num);
		ar& BOOST_SERIALIZATION_NVP(Flip);
	}
};

class EyeImage {
private:
	Cluster pcl, rcl;
public:
	reflect ref;
	ellipse pupil;

	ImgProc_Parameter ImgParam;
	Output_Parameter OutParam;
	CameraImage ci;
	PupilEdge pe;
	EP ep;
	EyeImage();
	~EyeImage();
	int GetIntensityRef(size_t x, size_t y);
	int Median3x3(size_t  x, size_t y);
	int Average3x3(size_t  x, size_t y);
	int SearchResion(void);

	int GetXEdgeR(int& x, int y);
	int GetXEdgeL(int& x, int y);
	int GetYEdgeT(int x, int& y);
	int GetYEdgeB(int x, int& y);

	bool PredictTopEdge(double x, int& y);
	bool PredictBottomEdge(double x, int& y);
	bool FindYEdgeT(int x, int& yt);
	bool FindYEdgeB(int x, int& yt);

	int FindXEdge(int y, int& xl, int& yl);
	bool PredictXEdge(double y, int& xl, int& xr);
	int CalcEllipseShapeX(void);


	int GetReflection(int x, int y, RECT&, int step = 2);

	int GetReflection(void);


	int AnalyzeImage(void);

	int AnalyzeImage(INT64 FrameID, ellipse& pupil,reflect& ref);
};
