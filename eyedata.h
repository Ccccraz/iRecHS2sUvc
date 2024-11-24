#pragma once



#define NRS_MAX 256

class NRS {
private:
public:
	double v[NRS_MAX + 1];
	double m[NRS_MAX + 1];
	int head;
	int num;

	void store(double);
	double median(int num);
	double average(int num);
	void clear(void);

	NRS();
	~NRS();
};



class EyeNumData {
private:
public:
	ellipse pupil;
	reflect ref;
	INT64 TimeStamp;
	INT64 FrameID;
	INT64 Counter;
	LARGE_INTEGER CPU_FreqCount;
	Output_Parameter OutParam;
	pc_param pc;
	pr_param pr;
	rc_param rc;
	WH_double img;

	struct gaze {
		GazeVec camVec;
		GazeVec objVec;
		GazeAngle camAng;
		GazeAngle objAng;
	};
	struct Output_Data {
		double x;
		double y;
		double l;
		double openratio;
		int mode;
		int method;

		Output_Data() {
			x = y = l = openratio=0;
			mode = method = 0;
		}
	};
	gaze GazePC, GazePR, GazeRC;
	Output_Data Gaze;
	void CalcPC();
	void CalcPR();
	void CalcRC();
	EyeNumData() {
		TimeStamp = 0;
		FrameID = 0;
		Counter = 0;
		CPU_FreqCount.QuadPart = 0;
		img.h = 0;
		img.w = 0;
	}
};

class EyeData {
public:
	std::deque<EyeNumData> v;
	NRS nr_x, nr_y,nr_l,nr_openratio;
	EyeNumData current;
	EyeData();
	~EyeData();
	void Store(EyeImage & ei,INT64 co);
};