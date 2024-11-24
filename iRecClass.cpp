
#include "framework.h"
#include "iRecHS2s.h"


#define EyeImageArray 1000
#define MAX_LOADSTRING 256

iRec::iRec()
{
	FrameCounter = 0;
	IsPause = false;
	CaptureLoopFlag = true;
	ConditionWake = false;
	EyeImageNum = EyeImageArray;

	pause_ready = false;
	CAM_StartTimeStamp = 0;
	CPU_Freq.QuadPart = 0;
	CPU_StartFreqCount.QuadPart = 0;
	pEyeImage = NULL;
}

iRec::~iRec()
{

}

bool iRec::StartCaptureThread(void)
{
	if (!cam.IsReady()) return false;
	pEyeImage = new EyeImage[EyeImageNum];
	QueryPerformanceFrequency(&CPU_Freq);

	if ((ImgProcParam.roi.left==0)&& (ImgProcParam.roi.right == 0)){
		ImgProcParam.roi = { 1,1,cam.CameraParam.w - 1,cam.CameraParam.h - 1 };
	}


	CaptureThreadPtr.reset(new thread([this]() {this->CaptureThread(); }));
	SetThreadPriority(CaptureThreadPtr->native_handle(), THREAD_BASE_PRIORITY_MAX);

	return true;
}
void iRec::EndCaptureThread(void)
{
	CaptureLoopFlag = false;
	CaptureThreadPtr->join();
	if (!cam.IsStreaming()) {
		cam.Stop();
	}
	delete[] pEyeImage;
}
void iRec::PauseCaptureThread(void)
{

	ConditionWake = false;
	IsPause = true;
	pause_ready = false;

	while (!pause_ready) {
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
}
void iRec::ResumeCaptureThread(void)
{
	std::lock_guard<std::mutex> lock(Mutex);
	IsPause = false;
	ConditionWake = true;
	ConditionVariable.notify_all();
}

void iRec::CaptureThread(void)
{
	INT64 NextCounter = 0;
	INT64 co = 0;
	FrameCounter = 0;
	INT64 prevCo = 0;
	RECT MovingROI;
	int roi_w, roi_h;
	cam.Start();
	cam.CaptureImage(pEyeImage[0].ci, L"CaptureThread");
	CPU_StartFreqCount = pEyeImage[0].ci.CPU_FreqCount;
	CAM_StartTimeStamp = pEyeImage[0].ci.TimeStamp;
	pEyeImage[0].ci.CaptureCounter = FrameCounter;
	{
		std::lock_guard<std::mutex> lock(Mutex);
		pEyeImage[0].ImgParam = ImgProcParam;
		MovingROI = ImgProcParam.roi;
		pEyeImage[0].OutParam = OutputParam;
	}
	pEyeImage[0].AnalyzeImage();
	Eye.Store(pEyeImage[0], FrameCounter);
	{
		std::lock_guard<std::mutex> lock(Mutex);
		FrameCounter = NextCounter++;
	}
	prevCo = co;
	co = NextCounter % EyeImageNum;


	while (CaptureLoopFlag) {
		
		cam.CaptureImage(pEyeImage[co].ci, L"CaptureThread");
		pEyeImage[co].ci.CaptureCounter = FrameCounter+1;
		{
			std::lock_guard<std::mutex> lock(Mutex);
			pEyeImage[co].ImgParam = ImgProcParam;
			if (ImgProcParam.IsMovingROI) {
				pEyeImage[co].ImgParam.roi = MovingROI;
			}
			roi_h = abs(ImgProcParam.roi.bottom - ImgProcParam.roi.top);
			roi_w = abs(ImgProcParam.roi.right - ImgProcParam.roi.left);
			pEyeImage[co].OutParam = OutputParam;
		}

		
		if (pEyeImage[co].ImgParam.IsUsePreviousImage) {
			pEyeImage[co].AnalyzeImage(Eye.current.FrameID, Eye.current.pupil,Eye.current.ref);
		}
		else {
			pEyeImage[co].AnalyzeImage();
		}
		Eye.Store(pEyeImage[co], FrameCounter+1);
		{
			std::lock_guard<std::mutex> lock(Mutex);
			FrameCounter = NextCounter++;
		}

		prevCo = co;
		co = NextCounter % EyeImageNum;
		
		if (Eye.current.pupil.percentx > 60) {
			MovingROI.left = static_cast<int>(Eye.current.pupil.center.x) - roi_w / 2;
			if (MovingROI.left < 0) MovingROI.left = 0;
			MovingROI.right = static_cast<int>(Eye.current.pupil.center.x) + roi_w / 2;
			if (MovingROI.right >= Eye.current.img.w) MovingROI.right = static_cast<int>(Eye.current.img.w) - 1;

			MovingROI.top = static_cast<int>(Eye.current.pupil.center.y) - roi_h / 2;
			if (MovingROI.top < 0) MovingROI.top = 0;
			MovingROI.bottom = static_cast<int>(Eye.current.pupil.center.y) + roi_h / 2;
			if (MovingROI.bottom >= Eye.current.img.h) MovingROI.bottom = static_cast<int>(Eye.current.img.h) - 1;
		}

		dac->Out(Eye.current);
		nio->SendData(Eye.current);

		if (IsPause) {
			pause_ready = true;
			std::unique_lock<std::mutex> lock(Mutex);
			ConditionVariable.wait(lock, [&] {return ConditionWake; });
		}


	}
}

void iRec::SetDaconverter(HWND parent)
{
	INT_PTR ret;
	dac.reset(new DlgDac(parent, *this));
	ret = dac->Open();
	//return ret;

}
void iRec::SetNetIO(void)
{
	nio.reset(new net_io(*this, NioParam.tcp_port, NioParam.udp_int_port, NioParam.udp_str_port));
}
void iRec::SetCameraView(HWND parent, int x, int y)
{
	int w, h,px,py;
	w = static_cast<int>(cam.CameraParam.w);
	h = static_cast<int>(cam.CameraParam.h);

	nCalib.reset(new Calibration(parent ,*this));

	wCamView.reset(new W_CameraView(parent, *this, x, y, w, h));
	px = x + w + 10;
	py = y;

	wTimeLine.reset(new W_TimeLine(parent, *this, px, py, 80,60));
	wXYGraph.reset(new W_XY_Graph(parent, *this, 10, h+10, 320, 320));
}

bool iRec::LoadParameter(void)
{
	fIO.reset(new FileIO(*this));
	return fIO->load();
	
}

void iRec::SaveParameter(void)
{
	fIO->save();
}

bool iRec::DialogOptions(HWND hWnd)
{
	INT_PTR ret;
	DlgOptions dlgOp(hWnd, *this);

	ret = dlgOp.Open();

	return ret;
}

bool iRec::DialogObjectmap(HWND hWnd)
{
	INT_PTR ret;
	DlgObjectmap dlgObjmap(hWnd, *this);

	ret = dlgObjmap.Open();

	return ret;
}

void iRec::ResizeWindows(void)
{

}


