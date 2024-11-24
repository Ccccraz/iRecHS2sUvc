
#include "framework.h"
#include "iRecHS2s.h"
#define MAX_LOADSTRING 256

#define DEFAULT_SETTING_FILE TEXT("setting.xml")

using namespace boost;
namespace fs = boost::filesystem;
using namespace boost::archive;


struct iRecParam {
	ImgProc_Parameter ipp;
	SPINNAKER_CAMERA_PARAM scscp;
	CameraView_Parameter cvp;
	Calibration_Parameter cp;
	Output_Parameter op;
	Dac_Parameter dac;
	Nio_Parameter niop;
private:
	friend class boost::serialization::access;
	template<class Archive>  void serialize(Archive& ar, unsigned int ver)
	{
		ar& make_nvp("ImgProc_Parameter", ipp);
		ar& make_nvp("SPINNAKER_CAMERA_PARAM", scscp);
		ar& make_nvp("CameraView_Parameter", cvp);
		ar& make_nvp("Calibration_Parameter", cp);
		ar& make_nvp("Output_Parameter", op);
		ar& make_nvp("Dac_Parameter", dac);
		ar& make_nvp("Nio_Parameter", niop);
	}
};


iRec::FileIO::FileIO(iRec& ir)
{
	iRecPtr = &ir;
	TCHAR buff[MAX_PATH];
	GetModuleFileName(hInst, buff, MAX_LOADSTRING);
	PathRemoveFileSpec(buff);
	Settings = fs::path(buff) / fs::path(DEFAULT_SETTING_FILE);
}

iRec::FileIO::~FileIO()
{

}

void iRec::FileIO::save()
{
	iRecParam sd;
	
	sd.ipp = iRecPtr->ImgProcParam;
	sd.scscp = iRecPtr->cam.CameraParam;
	sd.cvp = iRecPtr->CameraViewParam;
	sd.cp = iRecPtr->CalibrationParam;
	sd.op = iRecPtr->OutputParam;
	sd.dac = iRecPtr->DacParam;
	sd.niop = iRecPtr->NioParam;
	fs::wofstream file(Settings);
	xml_woarchive oa(file);
	oa << make_nvp("iRecHS2s_Parameter", (const iRecParam&)sd);

}

bool iRec::FileIO::load()
{
	bool ret = true;
	if (!exists(Settings)) {
		save();
		ret = false;
	}
	fs::wifstream file(Settings);

	iRecParam sd, sd1;
	try {
		xml_wiarchive ia(file);
		ia >> make_nvp("iRecHS2s_Parameter", sd);
	}
	catch (std::exception const& ex)
	{
		std::wstringstream wss;
		wss << L"FileIO load error" << endl;
		wss << ex.what() << endl;
		wss << L"Use default settings." << endl;
		MessageBox(NULL, (wss.str()).c_str(), TEXT("iRecHS2s Error Message"), MB_OK);
		sd = sd1;
		ret = false;
	}

	iRecPtr->ImgProcParam=sd.ipp;
	if (iRecPtr->cam.CameraParam.DeviceModelName != sd.scscp.DeviceModelName) {
		sd.scscp.DeviceModelName = iRecPtr->cam.CameraParam.DeviceModelName;
		ret = false;
	}
	if (iRecPtr->cam.CameraParam.SerialNumber != sd.scscp.SerialNumber) {
		sd.scscp.SerialNumber = iRecPtr->cam.CameraParam.SerialNumber;
		ret = false;
	}
	iRecPtr->cam.CameraParam=sd.scscp;

	iRecPtr->CameraViewParam = sd.cvp;
	iRecPtr->CalibrationParam = sd.cp;
	iRecPtr->OutputParam = sd.op;
	iRecPtr->DacParam = sd.dac;
	iRecPtr->NioParam = sd.niop;
	iRecPtr->cam.SetCameraParameter(sd.scscp);
	iRecPtr->cam.GetCameraParameter(iRecPtr->cam.CameraParam);

	sd.ipp.print();
	sd.scscp.print();
	return ret;
}

bool iRec::FileIO::get_EnvStr(const char* env, std::string& str)
{
	char* buf = 0;
	size_t sz = 0;
	if (_dupenv_s(&buf, &sz, env) == 0) {
		if (buf == 0) {
			return false;
		}
		else {
			str = buf;
			free(buf);
			return true;
		}
	}
	return false;
}

void iRec::FileIO::get_DateTime(std::string& date, std::string& ti)
{
	struct tm newTime;
	__time64_t long_time;
	_time64(&long_time);
	_localtime64_s(&newTime, &long_time);

	std::string y = std::to_string(newTime.tm_year + 1900);
	std::string m = std::to_string(newTime.tm_mon + 1);
	if (m.length() == 1) {
		m = std::string("0") + m;
	}
	std::string d = std::to_string(newTime.tm_mday);
	if (d.length() == 1) {
		d = std::string("0") + d;
	}
	date = y + m + d;

	std::string hh = std::to_string(newTime.tm_hour);
	if (hh.length() == 1) {
		hh = std::string("0") + hh;
	}
	std::string mm = std::to_string(newTime.tm_min);
	if (mm.length() == 1) {
		mm = std::string("0") + mm;
	}

	std::string ss = std::to_string(newTime.tm_sec);
	if (ss.length() == 1) {
		ss = std::string("0") + ss;
	}
	ti = hh + mm + ss;
}


boost::filesystem::path iRec::FileIO::get_DATA_dir(std::string& date)
{
	std::string str;
	if (!get_EnvStr("IRECHS2S_STORE", str)) {
		get_EnvStr("USERPROFILE", str);
	}
	boost::filesystem::path const storedir(str);
	boost::filesystem::path const datadir = storedir / "iRecHS2" / "DATA" / date;

	if (!boost::filesystem::exists(datadir)) {
		boost::system::error_code error;
		bool const result = boost::filesystem::create_directories(datadir, error);
		if (!result || error) {
			//std::cout << "fail to create directories" << std::endl;
		}
	}

	return datadir;
}

bool iRec::FileIO::save_devdata(std::string date,std::string ti)
{
	boost::filesystem::path datadir = get_DATA_dir(date);
	std::string filename = ti + "dev.csv";
	boost::filesystem::path filepath = datadir / filename;
	boost::filesystem::ofstream f(filepath);

	INT64 offsetTi = iRecPtr->Eye.v[0].TimeStamp;
	INT64 offsetCPU_TI = iRecPtr->Eye.v[0].CPU_FreqCount.QuadPart;
	INT64 offsetFr = iRecPtr->Eye.v[0].FrameID;
	f << "time,"
		<< "cpu_time,"
		<< "PupilCenterX,"
		<< "PupilCenterY,"
		<< "PupilRadius,"
		<< "ReflectionCenterX," 
		<< "ReflectionCenterY," 
		<< "ReflectionWidth,"
		<< "ReflectionHeight,"
		<< "a,"
		<< "b,"
		<< "c,"
		<< "d,"
		<< "e,"
		<< "pc_cax,"
		<< "pc_cay,"
		<< "pc_oax,"
		<< "pc_oay,"
		<< "pr_cax,"
		<< "pr_cay,"
		<< "pr_oax,"
		<< "pr_oay,"
		<< "pc_cax,"
		<< "pc_cay,"
		<< "pc_oax,"
		<< "pc_oay,"
		<< "Frame,"
		<< "Counter"
		<< std::endl;

	INT64 fr = 0;
	double t = 0;
	double cpu_t = 0;
	for (auto& e : iRecPtr->Eye.v) {
		t = fabs(e.TimeStamp - offsetTi) / 1e9;
		cpu_t = fabs(e.CPU_FreqCount.QuadPart - offsetCPU_TI) / (double)(iRecPtr->CPU_Freq.QuadPart);
		fr = e.FrameID - offsetFr;
		f << std::fixed << t << ","
			<< cpu_t << ","
			<< e.pupil.center.x << ","
			<< e.pupil.center.y << ","
			<< e.pupil.axis.l << ","
			<< e.ref.pos.x << ","
			<< e.ref.pos.y << ","
			<< e.ref.size.w << ","
			<< e.ref.size.h << ","
			<< e.pupil.eq.a << ","
			<< e.pupil.eq.b << ","
			<< e.pupil.eq.c << ","
			<< e.pupil.eq.d << ","
			<< e.pupil.eq.e << ","
			<< e.GazePC.camAng.x << ","
			<< e.GazePC.camAng.y << ","
			<< e.GazePC.objAng.x << ","
			<< e.GazePC.objAng.y << ","
			<< e.GazePR.camAng.x << ","
			<< e.GazePR.camAng.y << ","
			<< e.GazePR.objAng.x << ","
			<< e.GazePR.objAng.y << ","
			<< e.GazePC.camAng.x << ","
			<< e.GazePC.camAng.y << ","
			<< e.GazePC.objAng.x << ","
			<< e.GazePC.objAng.y << ","
			<< fr << ","
			<< e.Counter 
			<< std::endl;
	}
	f.close();
	return true;
}
bool iRec::FileIO::save_gazedata(std::string date, std::string ti)
{
	boost::filesystem::path datadir = get_DATA_dir(date);
	std::string filename = ti + ".csv";
	boost::filesystem::path filepath = datadir / filename;
	boost::filesystem::ofstream f(filepath);

	INT64 offsetTi = iRecPtr->Eye.v[0].TimeStamp;
	INT64 offsetFr = iRecPtr->Eye.v[0].FrameID;
	f << "time,x,y,pupil,openratio,method,mode,Frame,Counter" << std::endl;

	INT64 fr = 0;
	double t = 0;
	for (auto& e : iRecPtr->Eye.v) {
		t = fabs(e.TimeStamp - offsetTi) / 1e9;
		fr = e.FrameID - offsetFr;
		f << std::fixed << t << ","
			<< e.Gaze.x << ","
			<< e.Gaze.y << ","
			<< e.Gaze.l << ","
			<< e.Gaze.openratio << ","
			<< e.Gaze.method << ","
			<< e.Gaze.mode << ","
			<< fr << ","
			<< e.Counter 
			<< std::endl;
	}
	f.close();
	return true;
}

bool iRec::FileIO::saveuidata(std::string date, std::string ti)
{

	if (iRecPtr->nio->ui_server.v.size() == 0) return false;

	boost::filesystem::path datadir = get_DATA_dir(date);
	std::string filename = ti + "ui.csv";
	boost::filesystem::path filepath = datadir / filename;
	boost::filesystem::ofstream f(filepath);

	f << "time,data,fromAddress,port" << std::endl;
	for (auto& e : iRecPtr->nio->ui_server.v) {
		f << std::fixed <<e.ti << ","
			<< e.data << ","
			<< e.fromAddress << ","
			<< e.port 
			<< std::endl;
	}
	f.close();
	return true;
}

bool iRec::FileIO::saveusdata(std::string date, std::string ti)
{

	if (iRecPtr->nio->us_server.v.size() == 0) return false;

	boost::filesystem::path datadir = get_DATA_dir(date);
	std::string filename = ti + "us.csv";
	boost::filesystem::path filepath = datadir / filename;
	boost::filesystem::ofstream f(filepath);

	f << "time,data,fromAddress,port" << std::endl;
	for (auto& e : iRecPtr->nio->us_server.v) {
		f << std::fixed <<e.ti << ","
			<< e.msg << ","
			<< e.fromAddress << ","
			<< e.port
			<< std::endl;
	}
	f.close();
	return true;
}

bool iRec::FileIO::savecaldata(std::string date, std::string ti)
{

	if (iRecPtr->nio->t_server.v.size() == 0) return false;

	boost::filesystem::path datadir = get_DATA_dir(date);
	std::string filename = ti + "cal.csv";
	boost::filesystem::path filepath = datadir / filename;
	boost::filesystem::ofstream f(filepath);

	f << "time,send_time,h,v,s,fromAddress,port" << std::endl;
	for (auto& e : iRecPtr->nio->t_server.v) {
		f << std::fixed << e.ti << ","
			<< e.sti<< ","
			<< e.h << ","
			<< e.v << ","
			<< e.s << ","
			<< e.fromAddress << ","
			<< e.port
			<< std::endl;
	}
	f.close();
	return true;
}


bool iRec::FileIO::savedata()
{
	std::string date, ti;
	get_DateTime(date, ti);
	
	if (iRecPtr->OutputParam.IsSave) {
		save_gazedata(date, ti);
	}
	if (iRecPtr->OutputParam.IsSaveDev) {
		save_devdata(date, ti);
	}

	saveuidata(date, ti);
	saveusdata(date, ti);
	savecaldata(date, ti);
	return true;
}


