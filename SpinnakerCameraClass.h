#pragma once
#include <Spinnaker.h>
#include <SpinGenApi/SpinnakerGenApi.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <vector>
#include <codecvt>
#include <locale>
#include <mutex>
#include <thread>

#include <boost/format.hpp>
#include <boost/foreach.hpp>


//#define CONWIN 1

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;


#define EyeImageArray 1000
//
// Convert a wide Unicode string to an UTF8 string -> utf8_encode
// Convert an UTF8 string to a wide Unicode String -> utf8_decode
// https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte/3999597#3999597
// https://torini.hateblo.jp/entry/2018/10/04/C%2B%2B17%E3%81%8B%E3%82%89%E3%81%AFwstring_convert%E3%81%8C%E9%9D%9E%E6%8E%A8%E5%A5%A8%E3%81%A8%E3%81%AA%E3%82%8B%E3%81%AE%E3%81%A7%E7%92%B0%E5%A2%83%E6%AF%8E%E3%81%AE%E3%82%B3%E3%83%BC%E3%83%89%E3%81%8C
//
std::string utf8_encode(const std::wstring& wstr);
std::wstring utf8_decode(const std::string& str);

class CameraImage {
private:
public:
	unsigned char* img;
	int w;
	int h;
	int pixelbits;
	INT64 TimeStamp;
	INT64 FrameID;
	INT64 CaptureCounter;
	LARGE_INTEGER CPU_FreqCount;
	CameraImage();
	~CameraImage();
};

class ConsoleWindow
{
public:
	HANDLE hStdOutput;
	ConsoleWindow();
	~ConsoleWindow();

	void write(const std::wstring& ws);
};

extern ConsoleWindow conwin;


struct SPINNAKER_CAMERA_PARAM
{
	std::string SerialNumber;
	std::string DeviceModelName;
	std::string StreamBufferHandlingMode;
	std::string StreamBufferCountMode;
	INT64 StreamBufferCount;
	std::string AdcBitDepth;

	std::string PixelFormat;
	std::string BinningSelector;
	INT64 BinningH;
	INT64 BinningV;

	int w;
	int h;
	int OffsetX;
	int OffsetY;

	std::string AcquisitionMode;
	bool AcquisitionFrameRateEnable;

	std::string AcquisitionFrameRateAuto;
	double AcquisitionFrameRate;

	std::string pgrExposureCompensationAuto;
	double pgrExposureCompensation;

	std::string ExposureMode;
	std::string ExposureAuto;
	double ExposureTime;

	std::string GainAuto;
	double Gain;

	std::string BlackLevelSelector;
	double BlackLevel;

	bool GammaEnable;
	double Gamma;

	SPINNAKER_CAMERA_PARAM() {

		StreamBufferHandlingMode = string("NewestOnly");

		StreamBufferCountMode = string("Manual");

		StreamBufferCount = 10;

		AdcBitDepth = string("Bit8");

		PixelFormat = string("Mono8");

		BinningSelector = string("All");
		BinningH = 1;
		BinningV = 1;
		OffsetX = 0;
		OffsetY = 0;
		w = 0;
		h = 0;

		AcquisitionMode = string("Continuous");

		AcquisitionFrameRateEnable = true;
		AcquisitionFrameRateAuto = string("Continuous");
		AcquisitionFrameRate = 0;

		pgrExposureCompensationAuto = string("Continuous");
		pgrExposureCompensation = 0;

		ExposureMode = string("Timed");
		ExposureAuto = string("Off");
		ExposureTime = 1000;

		GainAuto = string("Off");;
		Gain = 0;
		BlackLevelSelector = string("All");
		BlackLevel = 0;

		GammaEnable = true;
		Gamma = 1;
	}

	void print() {
		cout << "hello" << endl;
		std::stringstream ss;
		ss << "SerialNumber                : " << SerialNumber << endl;
		ss << "DeviceModelName             : " << DeviceModelName << endl;

		ss << "StreamBufferHandlingMode    : " << StreamBufferHandlingMode << endl;
		ss << "StreamBufferCountMode       : " << StreamBufferCountMode << endl;
		ss << "StreamBufferCount           : " << StreamBufferCount << endl;
		ss << "AdcBitDepth                 : " << AdcBitDepth << endl;

		ss << "PixelFormat                 : " << PixelFormat << endl;
		ss << "BinningSelector             : " << BinningSelector << endl;
		ss << "BiningH                     : " << BinningH << endl;
		ss << "BiningV                     : " << BinningV << endl;
		ss << "OffsetX                     : " << OffsetX << endl;
		ss << "OffsetY                     : " << OffsetY << endl;
		ss << "Width                       : " << w << endl;
		ss << "Height                      : " << h << endl;
		ss << "AcquisitionMode             : " << AcquisitionMode << endl;
		ss << "AcquisitionFrameRateEnable  : " << AcquisitionFrameRateEnable << endl;
		ss << "AcquisitionFrameRateAuto    : " << AcquisitionFrameRateAuto << endl;
		ss << "AcquisitionFrameRate        : " << AcquisitionFrameRate << endl;
		ss << "ExposureCompensationAuto    : " << pgrExposureCompensationAuto << endl;
		ss << "ExposureCompensation        : " << pgrExposureCompensation << endl;
		ss << "ExposureMode                : " << ExposureMode << endl;
		ss << "ExposureAuto                : " << ExposureAuto << endl;
		ss << "ExposureTime                : " << ExposureTime << endl;

		ss << "GainAuto                    : " << GainAuto << endl;
		ss << "Gain                        : " << Gain << endl;
		ss << "BlackLevelSelector          : " << BlackLevelSelector << endl;
		ss << "BlackLevel                  : " << BlackLevel << endl;
		ss << "GammaEnable                 : " << GammaEnable << endl;
		ss << "Gamma                       : " << Gamma << endl;
		std::cout << ss.str();
	}
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& BOOST_SERIALIZATION_NVP(SerialNumber);
		ar& BOOST_SERIALIZATION_NVP(DeviceModelName);
		ar& BOOST_SERIALIZATION_NVP(StreamBufferHandlingMode);
		ar& BOOST_SERIALIZATION_NVP(StreamBufferCountMode);
		ar& BOOST_SERIALIZATION_NVP(StreamBufferCount);
		ar& BOOST_SERIALIZATION_NVP(AdcBitDepth);
		ar& BOOST_SERIALIZATION_NVP(PixelFormat);
		ar& BOOST_SERIALIZATION_NVP(BinningSelector);
		ar& BOOST_SERIALIZATION_NVP(BinningH);
		ar& BOOST_SERIALIZATION_NVP(BinningV);
		ar& BOOST_SERIALIZATION_NVP(OffsetX);
		ar& BOOST_SERIALIZATION_NVP(OffsetY);
		ar& BOOST_SERIALIZATION_NVP(w);
		ar& BOOST_SERIALIZATION_NVP(h);
		ar& BOOST_SERIALIZATION_NVP(AcquisitionMode);
		ar& BOOST_SERIALIZATION_NVP(AcquisitionFrameRateEnable);
		ar& BOOST_SERIALIZATION_NVP(AcquisitionFrameRateAuto);
		ar& BOOST_SERIALIZATION_NVP(AcquisitionFrameRate);
		ar& BOOST_SERIALIZATION_NVP(pgrExposureCompensationAuto);
		ar& BOOST_SERIALIZATION_NVP(pgrExposureCompensation);
		ar& BOOST_SERIALIZATION_NVP(ExposureMode);
		ar& BOOST_SERIALIZATION_NVP(ExposureAuto);
		ar& BOOST_SERIALIZATION_NVP(ExposureTime);
		ar& BOOST_SERIALIZATION_NVP(GainAuto);
		ar& BOOST_SERIALIZATION_NVP(Gain);
		ar& BOOST_SERIALIZATION_NVP(BlackLevel);
		ar& BOOST_SERIALIZATION_NVP(GammaEnable);
		ar& BOOST_SERIALIZATION_NVP(Gamma);
	}
};

struct NodeValues {
	string nodeAuto;
	string auto_off;
	string auto_on;

	string nodeName;
	double min;
	double max;
	double val;
	bool IsAuto;
	NodeValues() {
		min = max = val = 0;
		IsAuto = false;
		auto_off = string("Off");
		auto_on = string("Continuous");
		string nodeAuto = "None";
		string nodeName = "None";
	}
	void print(void)
	{
		cout << "nodeAuto : " << nodeAuto << endl;
		cout << "nodeName : " << nodeName << endl;
		cout << "auto_off : " << auto_off << endl;
		cout << "auto_on  : " << auto_on << endl;
		cout << "value    : " << val << endl;
		cout << "max      : " << max << endl;
		cout << "min      : " << min << endl;
		cout << "IsAuto   : " << IsAuto << endl;
	}
};



class SpinnakerCamera {
private:
public:
	SystemPtr SpinnakerSystemPtr;
	CameraList SpinnakerCameraList;
	int SetAllChunkMode(bool flag);
	bool SetChunkMode(gcstring, bool);
	bool GetChunkMode(gcstring, bool&);
	bool SetTLSteamNodeEnumParameter(const char* nodename, const char* val);
	bool GetTLSteamNodeEnumParameter(const char* name, string& symbol);
	bool SetTLSteamNodeIntParameter(const char* nodename, INT64 val);
	bool GetTLSteamNodeIntParameter(const char* nodename, INT64& val);

	bool SetNodeFloatParameter(const char* nodename, double val);
	bool GetNodeFloatParameter(const char* nodename, double& val, double& max, double& min);
	bool SetNodeIntParameter(const char* name, INT64 val);
	bool GetNodeIntParameter(const char* name, INT64& val, INT64& max, INT64& min);

	bool SetNodeEnumParameter(const char* nodename, int val);
	bool SetNodeEnumParameter(const char* nodename, const char* val);
	bool GetNodeEnumParameter(const char* nodename, string& val);

	bool SetNodeBoolParameter(const char* nodename, bool val);
	bool GetNodeBoolParameter(const char* nodename, bool& val);

	bool SetNodeParameter(NodeValues& p);
	bool GetNodeParameter(NodeValues& p);

	bool SetWidthOffsetX(int w, int x);
	bool GetWidthOffsetX(int& w, int& x);
	bool SetHeightOffsetY(int w, int x);
	bool GetHeightOffsetY(int& w, int& x);

	bool SetCamera(string serial);
	bool SetCamera(unsigned int no);
	bool SetFullScreen(void);

	CameraPtr spCam;
	std::string spCamSerial;
public:
	enum AcquisitionFrameAutoEnums :int64_t { AcquisitionFrameAuto_Off = 0, AcquisitionFrameAuto_Once = 1, AcquisitionFrameAuto_Continuous = 2 };


	void ErrorProc(Spinnaker::Exception& e, wstring addtion = L"");

	class DlgSelect {
	private:
		HWND hWndParent;
		struct SPINNAKER_CAMERA_DATA {
			unsigned int index;
			std::wstring SerialNumber;
			std::wstring DeviceModelName;
			std::wstring DeviceCurrentSpeed;
			SPINNAKER_CAMERA_DATA() {
				index = 0;
			}
		};

	public:

		SpinnakerCamera* CamPtr;
		std::unique_ptr <DlgListBox> lb;
		unsigned int camIndex;

		vector<SPINNAKER_CAMERA_DATA> CameraListVector;
		bool SetCameraList(void);
		DlgSelect();
		~DlgSelect();

		INT_PTR Open(HWND, SpinnakerCamera& sc);
		bool InitCtrl(HWND hDlg);
		void EndCtrl(HWND hDlg);
		bool DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static INT_PTR CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
	class InputRegion
	{
	private:

		struct  MaxMinInc {
			int max;
			int min;
			int inc;
			int val;
			MaxMinInc() {
				max = min = inc = val = 0;
			}
		};

		MaxMinInc w, h, offx, offy;

		int ReductionRatio = 2;
		CameraImage ci;
	public:
		HWND hWndParent;
		SpinnakerCamera* CamPtr;
		HCURSOR hCursorArrow, hCursorSIZENWSE, hCursorSIZENESW, hCursorSIZEALL;
		POINTS start, end;
		std::unique_ptr<DlgBitmap> bmp;
		std::unique_ptr<DlgButton> btn_OK;
		std::unique_ptr<DlgEditBox> eb_width;
		std::unique_ptr<DlgEditBox> eb_height;
		std::unique_ptr<DlgEditBox> eb_offsetx;
		std::unique_ptr<DlgEditBox> eb_offsety;

		std::unique_ptr<DlgSpin> sp_width;
		std::unique_ptr<DlgSpin> sp_height;
		std::unique_ptr<DlgSpin> sp_offsetx;
		std::unique_ptr<DlgSpin> sp_offsety;

		void SetInputOffset();
		void SetInputSize();
		void SetInputCenter();

		void SetOffsetX(void);
		void SetOffsetY(void);
		void SetWidth(void);
		void SetHeight(void);

		InputRegion();
		~InputRegion();
		void InitCtrl(HWND hDlg);
		void EndCtrl(HWND hDlg);
		INT_PTR Open(HWND, SpinnakerCamera& sc);
		BOOL DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lPalram);
		static 	INT_PTR CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
	class Parameters
	{
	private:
		CameraImage ci;
	public:
		HWND hWndParent;
		SpinnakerCamera* CamPtr;

		std::unique_ptr<DlgButton> btn_OK;
		std::unique_ptr<DlgBitmap> bmp;
		NodeValues FrameRate;
		std::unique_ptr<DlgCheckbox> cb_FrameRate;
		std::unique_ptr<DlgSlider> sl_FrameRate;
		std::unique_ptr<DlgEditBox> eb_FrameRate;

		NodeValues ExposureCompensation;
		std::unique_ptr<DlgCheckbox> cb_ExposureCompensation;
		std::unique_ptr<DlgSlider> sl_ExposureCompensation;
		std::unique_ptr<DlgEditBox> eb_ExposureCompensation;

		NodeValues ExposureTime;
		std::unique_ptr<DlgCheckbox> cb_ExposureTime;
		std::unique_ptr<DlgSlider> sl_ExposureTime;
		std::unique_ptr<DlgEditBox> eb_ExposureTime;

		NodeValues Gain;
		std::unique_ptr<DlgCheckbox> cb_Gain;
		std::unique_ptr<DlgSlider> sl_Gain;
		std::unique_ptr<DlgEditBox> eb_Gain;

		NodeValues BlackLevel;
		std::unique_ptr<DlgSlider> sl_BlackLevel;
		std::unique_ptr<DlgEditBox> eb_BlackLevel;

		Parameters();
		~Parameters();
		void ShowParameters(void);
		void InitCtrl(HWND hDlg);
		void EndCtrl(HWND hDlg);
		INT_PTR Open(HWND, SpinnakerCamera& pcam);
		BOOL DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lPalram);
		static 	INT_PTR CALLBACK CallbackProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};

	SpinnakerCamera();
	~SpinnakerCamera();

	bool SetCameraParameter(SPINNAKER_CAMERA_PARAM& p);
	bool GetCameraParameter(SPINNAKER_CAMERA_PARAM& p);

	bool IsReady(void);

	bool DialogSelect(HWND hWnd = NULL);
	bool DialogRegion(HWND hWnd = NULL);
	bool DialogParameters(HWND hWnd = NULL);
	bool CaptureImage(CameraImage&, wstring = L"");
	bool IsStreaming(void);
	bool Start();
	bool Stop();
	int MaxWidth();
	int MaxHeight();

	SPINNAKER_CAMERA_PARAM CameraParam;



};






