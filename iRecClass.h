#pragma once

//#include "Caio.h"
#define MapPosNum 9
#define MapNum (MapPosNum*5)

struct CameraView_Parameter
{
	bool IsPupilOutline;
	bool IsPupilCenter;
	bool IsUnderThreshold;
	bool IsPupilEdge;
	bool IsRoi;
	bool IsInformation;
	bool IsMirrorImage;
	bool IsMinorAxis;
	bool IsReflection;
	CameraView_Parameter()
	{
		IsPupilOutline = true;
		IsPupilCenter = true;
		IsUnderThreshold = true;
		IsPupilEdge = true;
		IsRoi = true;
		IsInformation = true;
		IsMirrorImage = false;
		IsMinorAxis = false;
		IsReflection = true;
	}

private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(IsPupilOutline);
		ar& BOOST_SERIALIZATION_NVP(IsPupilCenter);
		ar& BOOST_SERIALIZATION_NVP(IsUnderThreshold);
		ar& BOOST_SERIALIZATION_NVP(IsPupilEdge);
		ar& BOOST_SERIALIZATION_NVP(IsRoi);
		ar& BOOST_SERIALIZATION_NVP(IsInformation);
		ar& BOOST_SERIALIZATION_NVP(IsMirrorImage);
		ar& BOOST_SERIALIZATION_NVP(IsMinorAxis);
		ar& BOOST_SERIALIZATION_NVP(IsReflection);
	}
};

struct FixationSection
{
	INT64 S_co, E_co;
	int S_pos, E_pos;
	int position;
	FixationSection()
	{
		S_co = E_co = 0;
		S_pos = E_pos = 0;
		position = 0;
	}
};

struct TimeLine_Parameter
{
	int TimeRange;

	TimeLine_Parameter()
	{
		TimeRange = 10;

	}
};


class STAT {
private:
public:
	double mean;
	double sd;
	double num;
	double sum;
	double sum2;
	double max;
	double min;
	std::deque<double> v;
	STAT();
	~STAT();

	int Store(double x);
	int Calc(void);
	void Clear();
};

class MaxMin {
private:
	double num;
public:
	double max, min;
	MaxMin() {
		num = 0;
		max = min = 0;
	}
	void store(double& data) {
		if (num == 0) {
			max = min = data;
		}
		else {
			if (max < data) max = data;
			if (min > data)min = data;
		}
		num++;
	}
	void reset(void) {
		num = max = min = 0;
	}
};

struct GazePoint {

	STAT pcx, pcy, rpx, rpy, pal, pas, psl, pss, psl_ang, pss_ang, pasl, pasl_ang, gx, gy, width, height;

	void reset() {
		pcx.Clear();
		pcy.Clear();
		rpx.Clear();
		rpy.Clear();
		pal.Clear();
		pas.Clear();
		psl.Clear();
		pss.Clear();
		psl_ang.Clear();
		pss_ang.Clear();
		pasl.Clear();
		pasl_ang.Clear();
		gx.Clear();
		gy.Clear();
		width.Clear();
		height.Clear();
	}
	GazePoint()
	{
		reset();
	}
	void Store(EyeNumData& v)
	{
		pcx.Store(v.pupil.center.x);
		pcy.Store(v.pupil.center.y);
		rpx.Store(v.ref.pos.x);
		rpy.Store(v.ref.pos.y);
		pal.Store(v.pupil.axis.l);
		pas.Store(v.pupil.axis.s);
		psl.Store(v.pupil.slope.l);
		pss.Store(v.pupil.slope.s);
		psl_ang.Store(atan2(v.pupil.slope.l, 1) * 180.0 / M_PI);
		pss_ang.Store(atan2(v.pupil.slope.s, 1) * 180.0 / M_PI);
		pasl.Store(v.pupil.axis.s / v.pupil.axis.l);
		pasl_ang.Store(acos(v.pupil.axis.s / v.pupil.axis.l) * 180.0 / M_PI);
		gx.Store(v.Gaze.x);
		gy.Store(v.Gaze.y);
		width.Store(v.img.w);
		height.Store(v.img.h);
	}
	void Calc()
	{
		pcx.Calc();
		pcy.Calc();
		rpx.Calc();
		rpy.Calc();
		pal.Calc();
		pas.Calc();
		psl.Calc();
		pss.Calc();
		psl_ang.Calc();
		pss_ang.Calc();
		pasl.Calc();
		pasl_ang.Calc();
		gx.Calc();
		gy.Calc();
		width.Calc();
		height.Calc();
	}
#ifdef CONWIN
	void print()
	{
		wstringstream wss;

		wss << L"pcx :" << pcx.mean << endl;
		wss << L"pcy :" << pcy.mean << endl;
		wss << L"rpx :" << rpx.mean << endl;
		wss << L"rpy :" << rpy.mean << endl;

		wss << L"pal :" << pal.mean << endl;
		wss << L"pas :" << pas.mean << endl;
		wss << L"psl :" << psl.mean << endl;
		wss << L"psl_sd :" << psl.sd << endl;

		wss << L"psl to ang :" << (atan2(psl.mean, 1) * 180 / M_PI) << endl;
		wss << L"psl_ang    :" << psl_ang.mean << endl;
		wss << L"psl_ang_sd :" << psl_ang.sd << endl;

		wss << L"pss :" << pss.mean << endl;
		wss << L"pss_sd :" << pss.sd << endl;
		wss << L"pss to ang :" << (atan2(pss.mean, 1) * 180 / M_PI) << endl;
		wss << L"pss_ang    :" << pss_ang.mean << endl;
		wss << L"pss_ang_sd :" << pss_ang.sd << endl;

		wss << L"pasl     :" << pasl.mean << endl;
		wss << L"paslto ang   :" << acos(pasl.mean) * 180 / M_PI << endl;
		wss << L"pasl_ang     :" << pasl_ang.mean << endl;
		wss << L"pasl_ang_sd  :" << pasl_ang.sd << endl;
		wss << L"width max    :" << width.max << endl;
		wss << L"height max    :" << height.max << endl;
		conwin.write(wss.str());
	}
#endif
};

enum class NetProc {
	tcpip, udpint, udpstr,
};

struct Calibration_Parameter
{
	double PassiveCalibSd;
	double FixationRangePixel;
	double FixationRangeAngle;
	double FixationDuration;
	double map[MapNum];
	PointData pd[MapPosNum];
	bool mapPosition;

	Calibration_Parameter() {
		mapPosition = true;
		PassiveCalibSd = 5;
		FixationRangePixel = 3;
		FixationRangeAngle = 1;
		FixationDuration = 0.5;
		for (int i = 0; i < MapNum / 5; i++) {
			map[i * 5 + 0] = 0;
			map[i * 5 + 1] = 0;
			map[i * 5 + 2] = 0;
			map[i * 5 + 3] = 0;
			map[i * 5 + 4] = 0;
		}
	}
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(PassiveCalibSd);
		ar& BOOST_SERIALIZATION_NVP(map);
		ar& BOOST_SERIALIZATION_NVP(pd);
		ar& BOOST_SERIALIZATION_NVP(mapPosition);
		ar& BOOST_SERIALIZATION_NVP(FixationRangePixel);
		ar& BOOST_SERIALIZATION_NVP(FixationRangeAngle);
		ar& BOOST_SERIALIZATION_NVP(FixationDuration);

	}
};
struct Dac_Parameter
{
	std::wstring name, model;
	std::wstring failuremode;
	double pixel_h, pixel_v, pixel_amp;
	double cam_h, cam_v, cam_amp;
	double obj_h, obj_v, obj_amp;
	double pupil, pupil_amp;
	bool IsHalf;
	Dac_Parameter()
	{
		name = std::wstring(L"DAC_BOARD_001");
		model = std::wstring(L"1608");
		failuremode = std::wstring(L"HOLD");
		pixel_h = 50.0;
		pixel_v = 50.0;
		pixel_amp = 10;

		cam_h = cam_v = 0;
		cam_amp = 6.0;

		obj_h = obj_v = 0;
		obj_amp = 6.0;

		pupil = 0.0;
		pupil_amp = 32;

		IsHalf = false;
	}
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(model);
		ar& BOOST_SERIALIZATION_NVP(name);
		ar& BOOST_SERIALIZATION_NVP(failuremode);
		ar& BOOST_SERIALIZATION_NVP(pixel_h);
		ar& BOOST_SERIALIZATION_NVP(pixel_v);
		ar& BOOST_SERIALIZATION_NVP(pixel_amp);
		ar& BOOST_SERIALIZATION_NVP(cam_h);
		ar& BOOST_SERIALIZATION_NVP(cam_v);
		ar& BOOST_SERIALIZATION_NVP(cam_amp);
		ar& BOOST_SERIALIZATION_NVP(obj_h);
		ar& BOOST_SERIALIZATION_NVP(obj_v);
		ar& BOOST_SERIALIZATION_NVP(obj_amp);
		ar& BOOST_SERIALIZATION_NVP(pupil);
		ar& BOOST_SERIALIZATION_NVP(pupil_amp);
		ar& BOOST_SERIALIZATION_NVP(IsHalf);
	}
};

struct Nio_Parameter
{
	int tcp_port;
	int udp_int_port;
	int udp_str_port;
	Nio_Parameter()
	{
		tcp_port = 50001;
		udp_int_port = 50002;
		udp_str_port = 50003;
	}
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(tcp_port);
		ar& BOOST_SERIALIZATION_NVP(udp_int_port);
		ar& BOOST_SERIALIZATION_NVP(udp_str_port);
	}
};





struct GraphBaseInfo {
	double Range;
	double MajorStep;
	double MinorStep;
	double UnitPixel;
	double Offset;
	int RangePixel;
	int MajorPixelStep;
	int MinorPixelStep;
	GraphBaseInfo() {
		Range = 0;
		MajorStep = 0;
		MinorStep = 0;
		UnitPixel = 0;
		Offset = 0;
		RangePixel = 0;
		MajorPixelStep = 0;
		MinorPixelStep = 0;
	}
};

struct GraphPlotX {
	INT64 counter;
	double time;
	double pupil_w;
	double ref_w;
	GraphPlotX() {
		time = 0;
		pupil_w = 0;
		ref_w = 0;
		counter = 0;
	}
};

struct FixationPoints {
	int S_pos;
	int E_pos;

	INT64 S_co;
	INT64 E_co;

	double duration;

	double S_time;
	double E_time;
	double base_x;
	double base_y;

	void reset() {
		S_pos = E_pos = 0;
		duration = 0;
		S_time = E_time = 0;
		base_x = base_y = 0;
		S_co = E_co = 0;

	}
	FixationPoints() {
		reset();
	}
};


struct UdpIntData {
	LARGE_INTEGER FreqCount;
	double ti;
	int data;
	std::string port;
	std::string fromAddress;
	UdpIntData(){
		ti = 0;
		FreqCount.QuadPart = 0;
		data = 0;
	}
};

struct UdpStrData {
	LARGE_INTEGER FreqCount;
	double ti;
	std::string msg;
	std::string port;
	std::string fromAddress;
	UdpStrData() {
		ti = 0;
		FreqCount.QuadPart = 0;
		
	}
};

struct TcpCalData {
	LARGE_INTEGER FreqCount;
	double ti;
	double sti, h, v, s;
	std::string port;
	std::string fromAddress;
	TcpCalData() {
		ti = 0;
		sti = h = v = 0;
		FreqCount.QuadPart = 0;

	}
};



class iRec
{
private:
	std::mutex Mutex;
	std::condition_variable ConditionVariable;
	bool ConditionWake = false;

	bool CaptureLoopFlag;
	std::unique_ptr <std::thread> CaptureThreadPtr;


	bool pause_ready;
public:
	bool IsPause;
	iRec();
	~iRec();

	SpinnakerCamera cam;

	LARGE_INTEGER CPU_StartFreqCount;
	LARGE_INTEGER CPU_Freq;
	INT64 CAM_StartTimeStamp;
	INT64 FrameCounter;
	INT64 EyeImageNum;
	EyeImage* pEyeImage;
	EyeData  Eye;

	ImgProc_Parameter ImgProcParam;
	CameraView_Parameter CameraViewParam;
	TimeLine_Parameter TimeLineParam;
	Calibration_Parameter CalibrationParam;
	Output_Parameter OutputParam;
	Dac_Parameter  DacParam;
	Nio_Parameter NioParam;

	//	bool IsMirrorImage;

	void CaptureThread(void);
	bool StartCaptureThread(void);
	void EndCaptureThread(void);
	void PauseCaptureThread(void);
	void ResumeCaptureThread(void);

	class Calibration {
	private:
		iRec* iRecPtr;
		HWND hWndParent;
		std::deque<EyeNumData> current_eye_deque;
		GazePoint current_gp;
		int current_gp_num;

		std::unique_ptr<DlgEditBox> info;
		std::unique_ptr<DlgEditBox> stored_info;
		std::unique_ptr<DlgBitmap> bmp;
		std::unique_ptr<DlgComboList> gaze_position;
		std::unique_ptr<DlgButton> btn_ok;
		std::unique_ptr<DlgButton> btn_cancel;
		void SetBitmapCtrl(HWND hWnd, int x, int y);
		void SetInfo(HWND hWnd, int x, int width);
		void SetGazePoint(HWND hWnd);
		void SetButton(HWND hWnd);
		void SetStoredInfo(HWND hWnd, int x, int width);
		std::wstring SetStrings(GazePoint& g);
		int SetNextGP();
		bool Calc_ObjParam();
	public:
		pc_param pc;
		rc_param rc;
		pr_param pr;
		GazePoint gp[MapPosNum];
		Calibration(HWND hWnd, iRec&);
		~Calibration();

		INT64 IsGazeMoveEnough(INT64 co, int num, std::deque<PointData>& v);
		bool CalcPR_param(std::deque<PointData>& v);
		bool CalcPC_param(std::deque<PointData>& v);
		bool CalcRC_param(std::deque<PointData>& v);

		bool CalcPR_ObjParam(std::deque<PointData>& v);
		bool CalcPC_ObjParam(std::deque<PointData>& v);
		bool CalcRC_ObjParam(std::deque<PointData>& v);

		bool PassiveCalibration(INT64 co, int num);
		void ResetCalibration(void);

		INT_PTR Open(FixationSection& fs);
		bool InitCtrl(HWND hDlg);
		void EndCtrl(HWND hDlg);
		bool DlgProc(HWND, UINT message, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};

	class W_CameraView {
	private:
		HDC hdc_mem;
		HBITMAP hBmpMem, hOldBmpMem;
		HPEN hPen, hOldPen;
		HBRUSH hBrush, hOldBrush;
		HCURSOR hCursorOld, hCursorCross;
		HFONT hFont, hOldFont;
		BOOL LButtonDown;
		POINTS start, end;
	public:
		HWND hWnd;
		HWND hWndParent;
		RECT PosSize;
		iRec* iRecPtr;
		struct { LONG w, h; } winsize;

		//	int img_w, img_h;
		W_CameraView(HWND hWndParent, iRec& ir, int x, int y, int w, int h);
		~W_CameraView();
		void CopyImage(int);
		void DrawROI(int);
		void DrawPupilOutline(int co);
		void DrawPupilEdge(int co);
		void DrawPupilCenter(int co);
		void DrawRadiusLine(int co);
		void DrawReflection(int co);
		void DrawInformation(int co);
		void DrawRotationCenter(int co);
		void DrawPCLine(int co);
		bool TimerWatch(int co);
		bool SetROI(void);
		void ResizeWindow(int co);

		LRESULT  WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};

	class DlgOptions {
	private:
		HWND hWndParent;
		iRec* iRecPtr;
		std::unique_ptr<DlgButton> btn_OK;
		std::unique_ptr<DlgButton> btn_Cancel;

		std::unique_ptr<DlgLabel> lb_Threshold;
		std::unique_ptr<DlgSlider> sl_Threshold;

		std::unique_ptr<DlgLabel> lb_RefThreshold;
		std::unique_ptr<DlgSlider> sl_RefThreshold;

		std::unique_ptr<DlgLabel> lb_P_Angle;
		std::unique_ptr<DlgSlider> sl_P_Angle;

		std::unique_ptr<DlgLabel> lb_P_Length;
		std::unique_ptr<DlgSlider> sl_P_Length;

		std::unique_ptr<DlgLabel> lb_SE_Length;
		std::unique_ptr<DlgSlider> sl_SE_Length;

		std::unique_ptr<DlgLabel>  lb_BlinkThreshold;
		std::unique_ptr<DlgSlider> sl_BlinkThreshold;

		std::unique_ptr<DlgCheckbox> cb_PupilTop;
		std::unique_ptr<DlgCheckbox> cb_PupilBottom;
		std::unique_ptr<DlgCheckbox> cb_PupilElse;
		std::unique_ptr<DlgCheckbox> cb_IsUseMovingROI;
		std::unique_ptr<DlgCheckbox> cb_IsUsePreviousImage;
		std::unique_ptr<DlgCheckbox> cb_IsSave;
		std::unique_ptr<DlgCheckbox> cb_IsSaveDev;
		std::unique_ptr<DlgSlider> sl_Median_num;
		std::unique_ptr<DlgSlider> sl_MA_num;

		std::unique_ptr<DlgSlider> sl_FixationRange_Pixel;
		std::unique_ptr<DlgSlider> sl_FixationRange_Angle;
		std::unique_ptr<DlgSlider> sl_Fixation_Duration;
		std::unique_ptr<DlgSlider> sl_Passive_sd;

		std::unique_ptr<DlgRadioButton> rb_none;
		std::unique_ptr<DlgRadioButton> rb_Median;
		std::unique_ptr<DlgRadioButton> rb_MA;


		std::unique_ptr<DlgLabel> lb_FixationRange_Pixel;
		std::unique_ptr<DlgLabel> lb_FixationRange_Angle;
		std::unique_ptr<DlgLabel> lb_Fixation_Duration;
		std::unique_ptr<DlgLabel> lb_Passive_sd;

		std::unique_ptr<DlgCheckbox> cb_FlipH;
		std::unique_ptr<DlgCheckbox> cb_FlipV;

		std::unique_ptr<DlgComboList> cl_method;
	public:
		DlgOptions(HWND hWnd, iRec& ir);
		~DlgOptions();
		INT_PTR Open();

		bool InitCtrl(HWND hDlg);
		void EndCtrl(HWND hDlg);
		bool DlgProc(HWND, UINT message, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};


	class DlgObjectmap {
	private:
		HWND hWndParent;
		iRec* iRecPtr;
		std::array<std::unique_ptr<DlgEditBox>, 45> edbox;
		std::unique_ptr<DlgCheckbox> cb_position, cb_angle;

	public:
		bool UpdateEditBox(int id, int event);
		DlgObjectmap(HWND hWnd, iRec& ir);
		~DlgObjectmap();
		INT_PTR Open();
		bool InitCtrl(HWND hDlg);
		void EndCtrl(HWND hDlg);
		bool DlgProc(HWND, UINT message, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};


#define DAC_MAX_CARD 4
#define DAC_MAX_LOADSTRING 256
#define DAC_MAX_CHANNEL 4
	struct DacCard
	{
		std::wstring name;
		std::wstring model;

		int channels;
		DacCard()
		{
			channels = 0;
		}
	};

	class DacInfo {
	private:
	public:
		std::vector<DacCard> cards;
		int num;

		DacInfo();
		~DacInfo();
		std::wstring Num2listname(int n);
		std::wstring NameModel2listname(std::wstring &n,std::wstring &m);

		int  Listname2num(std::wstring name_model);
	};


	class DlgDac {
	private:
		HWND hWndParent;
		iRec* iRecPtr;
		std::unique_ptr<DlgComboList> cl_DacCard;
		std::unique_ptr<DlgComboList> cl_DacFailure;

		std::unique_ptr<DlgLabel> lb_Pixel;
		std::unique_ptr<DlgSlider> sl_Pixel_h;
		std::unique_ptr<DlgSlider> sl_Pixel_v;
		std::unique_ptr<DlgLabel> lb_Pixel_amp;
		std::unique_ptr<DlgSlider> sl_Pixel_amp;

		std::unique_ptr<DlgLabel> lb_Cam;
		std::unique_ptr<DlgSlider> sl_Cam_h;
		std::unique_ptr<DlgSlider> sl_Cam_v;
		std::unique_ptr<DlgLabel> lb_Cam_amp;
		std::unique_ptr<DlgSlider> sl_Cam_amp;

		std::unique_ptr<DlgLabel> lb_Obj;
		std::unique_ptr<DlgSlider> sl_Obj_h;
		std::unique_ptr<DlgSlider> sl_Obj_v;
		std::unique_ptr<DlgLabel> lb_Obj_amp;
		std::unique_ptr<DlgSlider> sl_Obj_amp;

		std::unique_ptr<DlgLabel> lb_Pupil;
		std::unique_ptr<DlgSlider> sl_Pupil;
		std::unique_ptr<DlgLabel> lb_Pupil_amp;
		std::unique_ptr<DlgSlider> sl_Pupil_amp;

		std::unique_ptr<DlgCheckbox> cb_DacLimitVoltage;

		std::unique_ptr<DlgLabel> lb_DacInfo;

	public:
		Dac_Parameter* dp;
		std::unique_ptr<DacInfo> di;
		short AioID;
		int DACBITS;
		int MaxDigit;
		long ch[DAC_MAX_CHANNEL];
		DlgDac(HWND hWnd, iRec& ir);
		~DlgDac();
		INT_PTR Open();
		bool InitCtrl(HWND hDlg);
		void EndCtrl(HWND hDlg);
		bool DlgProc(HWND, UINT message, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
		bool IsDacOpen;
		bool DacInit();
		void HalfOut(void);
		void OutMax(void);
		void OutMin(void);
		void OutZero(void);
		void OutFailure(void);
		void OutPixel(double x, double y, double r, double t, double ix, double iy);
		void OutCamAng(double x, double y, double r, double t);
		void OutObjAng(double x, double y, double r, double t);
		void Out(EyeNumData& ed);
	};

	class W_TimeLine {
	private:
		HDC hdc_mem;
		HBITMAP hBmpMem, hOrigBmpMem;
		HPEN hPenBlue1, hPenBlue2, hPenBlue3;
		HPEN hPenRed1, hPenRed2, hPenRed3;
		HPEN hPenGreen1, hPenGreen2, hPenYellow, hOrigPen;
		HPEN hPenGrey;
		HPEN hPenBlack;

		HBRUSH hBrush, hBrushNull, hOrigBrush;
		HFONT hFont, hOrigFont;

		BOOL LButtonDown;

		RECT GraphArea;


		struct GraphParam {
			vector <GraphPlotX> ti;
			vector <FixationPoints> fpv;
			GraphBaseInfo x;
			GraphBaseInfo y;
			RECT Area;
			wstring name;
			GraphParam() {
				Area = { 0,0,0,0 };
			}
		};


		void SetGraphBaseInfoX(GraphBaseInfo& gp, int range);
		void SetGraphBaseInfoY(GraphBaseInfo& gp, int range, double max, double min);

		void DetectFixation(INT64 co);
		void SetFixation(GraphParam& gp, INT64 co);

		void SyncYstep(GraphBaseInfo& src, GraphBaseInfo& des);

	public:
		HWND hWnd;
		HWND hWndParent;
		iRec* iRecPtr;
		RECT PosSize;
		std::deque <FixationPoints> FxPoints;
		FixationPoints currentFXP;
		struct { LONG w, h; } winsize;

		void ClickGraph(double ti);
		void Resize(void);
		void GraphScaleX(GraphParam& gp);
		void GraphClear();
		void GraphBase(GraphParam& gp);
		void GraphPaintFixation(GraphParam& gp,int pos,INT64 co);
		void GraphMark(GraphParam& gp, FixationSection& fs);
		bool SetCalibSection(GraphParam& gp, FixationSection& fs);
		void GraphPlot(GraphParam& gp, vector<double>& data);
		void GraphAnnotationHorizontalLine(GraphParam& gp, double y);

		void SetGraphText(wstring& method, wstring& unit);
		void OutputGraph(INT64 co, bool IsRedRect, FixationSection& fs);

		bool IsMajorStep(double pos, double step);
		W_TimeLine(HWND hWndParent, iRec& ir, int x, int y, int w, int h);
		~W_TimeLine();
		LRESULT  WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};


	class W_XY_Graph {
	private:
		HDC hdc_mem;
		HBITMAP hBmpMem, hOrigBmpMem;
		HPEN hPenBlue1, hPenBlue2, hPenBlue3;
		HPEN hPenRed1, hPenRed2, hPenRed3;
		HPEN hPenGreen1, hPenGreen2, hPenYellow, hOrigPen;
		HPEN hPenGrey;
		HPEN hPenBlack;

		HBRUSH hBrush, hBrushNull, hOrigBrush;
		HFONT hFont, hOrigFont;
		BOOL LButtonDown;

		RECT GraphArea;
		void SetGraphBaseInfo(GraphBaseInfo& gp, int range, double max, double min);
//		void DetectFixation(INT64 co);
//		void SetFixation(GraphParam& gp, INT64 co);
		void SyncStep(GraphBaseInfo& src, GraphBaseInfo& des);


		struct GraphParam {
	//		vector <GraphPlotX> ti;
	//		vector <FixationPoints> fpv;
			GraphBaseInfo x;
			GraphBaseInfo y;
			RECT Area;
			wstring name;
			GraphParam() {
				Area = { 0,0,0,0 };
			}
		};
	public:
		HWND hWnd;
		HWND hWndParent;
		iRec* iRecPtr;
		RECT PosSize;
		std::deque <FixationPoints> FxPoints;
		FixationPoints currentFXP;
		struct { LONG w, h; } winsize;
		int GraphMode;
		void Resize();

		void GraphClear();
		void GraphBase(GraphParam& gp);
//		void GraphMark(GraphParam& gp, FixationSection& fs);

		void GraphPlot(GraphParam& gp, vector<double>& x,vector<double>& y);
		bool CalcCalibratedPoint(int i, int method,double& x, double& y);
		void GraphCalibratedPoints(GraphParam& gp, int method);
//		void SetGraphText(wstring& method, wstring& unit);
		void OutputGraph(INT64 co, bool IsRedRect);

		bool IsMajorStep(double pos, double step);
		W_XY_Graph(HWND hWndParent, iRec& ir, int x, int y, int w, int h);
		~W_XY_Graph();
		LRESULT  WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	};

	class FileIO {
	private:
		iRec* iRecPtr;
		boost::filesystem::path Settings;
	public:
		FileIO(iRec& ir);
		~FileIO();
		void save();
		bool load();
		bool savedata();
		bool save_gazedata(std::string date, std::string ti);
		bool save_devdata(std::string date, std::string ti);

		bool saveuidata(std::string date, std::string ti);
		bool saveusdata(std::string date, std::string ti);
		bool savecaldata(std::string date, std::string ti);
		bool get_EnvStr(const char* env, std::string& str);
		void get_DateTime(std::string& date, std::string& ti);
		boost::filesystem::path get_DATA_dir(std::string& date);
	};


	class net_io
	{
	public:
		class udp_int
		{
		public:
			net_io* nio;
			std::deque<UdpIntData> v;
			boost::asio::io_context& io;
			boost::asio::ip::udp::socket sok;
			boost::asio::ip::udp::endpoint remote_endpoint;
			boost::array<int, 1> recv_int;
			boost::asio::ip::port_type port_num;

			udp_int(net_io* nio, boost::asio::io_context& io, boost::asio::ip::port_type port_num);
			~udp_int();
			void wait_for_receive();
			void receive_int(const boost::system::error_code& error, std::size_t siz);
		};

		class udp_str
		{
		public:
			net_io* nio;
			std::deque<UdpStrData> v;
			boost::asio::io_context& io;
			boost::asio::ip::udp::socket sok;
			boost::asio::ip::udp::endpoint remote_endpoint;
			boost::array<char, 128> recv_str;
			boost::asio::ip::port_type port_num;

			udp_str(net_io* nio, boost::asio::io_context& io_context, boost::asio::ip::port_type port);
			~udp_str();
			void wait_for_receive();
			void receive_str(const boost::system::error_code& error, std::size_t siz);
		};

		class tcp_io
		{
		public:
			net_io* nio;
			std::deque<TcpCalData> v;
			boost::asio::io_context& io;
			boost::asio::ip::tcp::acceptor* acc = NULL;
			boost::asio::ip::tcp::socket* sok = NULL;
			boost::asio::streambuf buf;
			boost::asio::ip::port_type port;
			std::string address;

			bool IsSend=false;
			void listen();
			void interprete(const boost::system::error_code& e, size_t siz);
			void receive();
			void send(std::string& str);
			void async_send(std::string& str);
			void async_send_result(const boost::system::error_code& e, size_t siz);
			void accept(const boost::system::error_code& e);
			tcp_io(net_io* nio, boost::asio::io_context& io, boost::asio::ip::port_type port_num);
			~tcp_io();
		};


	private:
		iRec* iRecPtr;

	public:

		boost::asio::io_context io;
		udp_int ui_server;
		udp_str us_server;
		tcp_io t_server;
		boost::thread th;
		HWND hWnd = NULL;
		void SetStatusWindow(HWND wh);
		bool SendData(EyeNumData& ed);
		void printmsg(NetProc np,std::string str);
		net_io(iRec& ir, 
			boost::asio::ip::port_type tcp_port,
			boost::asio::ip::port_type udp_int_port,
			boost::asio::ip::port_type udp_str_port);
		~net_io();
	};
	

	std::unique_ptr<FileIO> fIO;
	std::unique_ptr<W_CameraView> wCamView;

	std::unique_ptr<W_TimeLine> wTimeLine;
	std::unique_ptr<W_XY_Graph> wXYGraph;
	std::unique_ptr<Calibration> nCalib;
	std::unique_ptr<DlgDac> dac;
	std::unique_ptr<net_io> nio;


	void SetCameraView(HWND, int, int);
	void SetDaconverter(HWND);
	void SetNetIO(void);
	bool LoadParameter(void);
	void SaveParameter(void);
	bool DialogOptions(HWND hWnd);
	bool DialogObjectmap(HWND hWnd);
	void ResizeWindows(void);
};