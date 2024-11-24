
#include "framework.h"
#include "iRecHS2s.h"

#define ID_CAMREGION_TIMER 10001
#define ID_CAMPARAM_TIMER 10011

#define CAMREGION_INTERVAL 33
#define CAMPARAM_INTERVAL 33


#define BMP_OFFSETX 0
#define BMP_OFFSETY 80
#define PALETTECOLORS 256

#define INIT_INPUT_W  480
#define INIT_INPUT_H 320

//#define FONT_SIZE 14
//#define FONT_CHARSET ANSI_CHARSET
//#define FONT_TYPEFACE TEXT("Courier New")

std::string utf8_encode(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
std::wstring utf8_decode(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

CameraImage::CameraImage()
{
	w = h = 0;
	pixelbits = 0;
	TimeStamp = 0;
	FrameID = 0;
	CPU_FreqCount.QuadPart = 0;
	img = NULL;
}
CameraImage::~CameraImage()
{
	if (img != NULL) delete[] img;
}

ConsoleWindow::ConsoleWindow()
{
	CONSOLE_SCREEN_BUFFER_INFO screenBuffer;
	AllocConsole();
	hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hStdOutput, &screenBuffer);
}
ConsoleWindow::~ConsoleWindow()
{
	FreeConsole();
}
void ConsoleWindow::write(const std::wstring& ws)
{
	DWORD  dwWriteByte;
	WriteConsole(hStdOutput, ws.c_str(), (DWORD)ws.size(), &dwWriteByte, NULL);
}


void SpinnakerCamera::ErrorProc(Spinnaker::Exception& e, wstring addition)
{
	std::wstringstream wss;
	wss << "Error: " << e.what() << endl;
	if (addition != L"") {
		wss << "Addtion:  " << addition << endl;
	}
	MessageBox(NULL, (wss.str()).c_str(), TEXT("Spinnaker Error Message"), MB_OK);
}

// Camera Private funtions
bool SpinnakerCamera::SetChunkMode(gcstring chunkname, bool flag)
{
	bool result;
	result = false;
	if (!IsReady()) return false;

	try {
		INodeMap& nodeMap = spCam->GetNodeMap();

		CBooleanPtr ptrChunkModeActive = nodeMap.GetNode("ChunkModeActive");
		if (IsAvailable(ptrChunkModeActive) && IsWritable(ptrChunkModeActive)) {
			ptrChunkModeActive->SetValue(true);

			CEnumerationPtr ptrChunkSelector = nodeMap.GetNode("ChunkSelector");
			if (IsAvailable(ptrChunkSelector) && IsReadable(ptrChunkSelector)) {
				CEnumEntryPtr ptrChunkSelectorEntry = ptrChunkSelector->GetEntryByName(chunkname);
				if (IsAvailable(ptrChunkSelectorEntry) && IsReadable(ptrChunkSelectorEntry) && IsWritable(ptrChunkSelector)) {
					ptrChunkSelector->SetIntValue(ptrChunkSelectorEntry->GetValue());
					CBooleanPtr ptrChunkEnable = nodeMap.GetNode("ChunkEnable");
					if (IsAvailable(ptrChunkEnable) && IsWritable(ptrChunkEnable)) {
						ptrChunkEnable->SetValue(flag);
						result = true;
					}
				}
			}
		}
	}
	catch (Spinnaker::Exception& e)
	{
		ErrorProc(e, L"SetChunkMode");
		result = false;
	}
	return result;
}
bool SpinnakerCamera::GetChunkMode(gcstring chunkname, bool& flag)
{
	bool result;
	result = false;
	if (!IsReady()) return false;

	try {
		INodeMap& nodeMap = spCam->GetNodeMap();

		CBooleanPtr ptrChunkModeActive = nodeMap.GetNode("ChunkModeActive");
		if (IsAvailable(ptrChunkModeActive) && IsWritable(ptrChunkModeActive)) {
			ptrChunkModeActive->SetValue(true);

			CEnumerationPtr ptrChunkSelector = nodeMap.GetNode("ChunkSelector");
			if (IsAvailable(ptrChunkSelector) && IsReadable(ptrChunkSelector)) {
				CEnumEntryPtr ptrChunkSelectorEntry = ptrChunkSelector->GetEntryByName(chunkname);
				if (IsAvailable(ptrChunkSelectorEntry) && IsReadable(ptrChunkSelectorEntry) && IsWritable(ptrChunkSelector)) {
					ptrChunkSelector->SetIntValue(ptrChunkSelectorEntry->GetValue());
					CBooleanPtr ptrChunkEnable = nodeMap.GetNode("ChunkEnable");
					if (IsAvailable(ptrChunkEnable) && IsReadable(ptrChunkEnable)) {
						flag = ptrChunkEnable->GetValue();
						result = true;
					}
				}
			}
		}

	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"GetChunkMode");
		result = false;
	}

	return result;
}
int SpinnakerCamera::SetAllChunkMode(bool flag)
{
	if (!IsReady()) return false;
	int result = 0;

	try {
		INodeMap& nodeMap = spCam->GetNodeMap();
		NodeList_t entries;

		CBooleanPtr ptrChunkModeActive = nodeMap.GetNode("ChunkModeActive");

		if (IsAvailable(ptrChunkModeActive) && IsWritable(ptrChunkModeActive)) {
			ptrChunkModeActive->SetValue(true);
			CEnumerationPtr ptrChunkSelector = nodeMap.GetNode("ChunkSelector");
			if (IsAvailable(ptrChunkSelector) && IsReadable(ptrChunkSelector) && IsWritable(ptrChunkSelector)) {
				ptrChunkSelector->GetEntries(entries);
				for (size_t i = 0; i < entries.size(); i++) {
					CEnumEntryPtr ptrChunkSelectorEntry = entries.at(i);
					if (!IsAvailable(ptrChunkSelectorEntry) || !IsReadable(ptrChunkSelectorEntry)) continue;
					ptrChunkSelector->SetIntValue(ptrChunkSelectorEntry->GetValue());
					CBooleanPtr ptrChunkEnable = nodeMap.GetNode("ChunkEnable");
					if (IsAvailable(ptrChunkEnable) && IsWritable(ptrChunkEnable)) {
						ptrChunkEnable->SetValue(flag);
						result++;
					}
				}
			}
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetAllChunkMode");
		result = -1;
	}
	return result;
}

bool SpinnakerCamera::SetTLSteamNodeEnumParameter(const char* name, const char* val)
{
	std::string sval = string(val);
	std::transform(sval.begin(), sval.end(), sval.begin(), ::tolower);
	if (!IsReady()) return false;

	try {
		INodeMap& sNodeMap = spCam->GetTLStreamNodeMap();
		CEnumerationPtr EnumPtr = sNodeMap.GetNode(name);
		if (IsAvailable(EnumPtr) && IsWritable(EnumPtr)) {
			NodeList_t entries;
			EnumPtr->GetEntries(entries);
			for (int i = 0; i < entries.size(); i++) {
				CEnumEntryPtr eptr = entries.at(i);
				string str = string(eptr->GetSymbolic());
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				if (str == sval) {
					EnumPtr->SetIntValue(eptr->GetValue());
					return true;
				}
			}
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetTLSteamNodeEnumParameter char*");
	}

	return false;
}


bool SpinnakerCamera::GetTLSteamNodeEnumParameter(const char* name, std::string& symbol)
{
	if (!IsReady()) return false;
	symbol = string("None");

	try {
		INodeMap& sNodeMap = spCam->GetTLStreamNodeMap();
		CEnumerationPtr EnumPtr = sNodeMap.GetNode(name);
		if (IsAvailable(EnumPtr) && IsWritable(EnumPtr)) {
			symbol = string(EnumPtr->GetCurrentEntry()->GetSymbolic());
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"GetTLSteamNodeEnumParameter char*");
	}
	return false;
}

bool SpinnakerCamera::SetTLSteamNodeIntParameter(const char* name, INT64 val)
{
	bool result = false;
	INT64 max, min;
	if (!IsReady()) return result;

	try {
		INodeMap& sNodeMap = spCam->GetTLStreamNodeMap();

		CIntegerPtr IntPtr = sNodeMap.GetNode(name);
		if (IsAvailable(IntPtr) && IsReadable(IntPtr)) {
			max = IntPtr->GetMax();
			min = IntPtr->GetMin();
			if (val < min) val = min;
			if (val > max) val = max;
			if (IsWritable(IntPtr)) {
				IntPtr->SetValue(val);
				result = true;
			}
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetTLSteamNodeIntParameter");
		result = false;
	}
	return result;
}

bool SpinnakerCamera::GetTLSteamNodeIntParameter(const char* name, INT64& val)
{
	if (!IsReady()) return false;

	try {
		INodeMap& sNodeMap = spCam->GetTLStreamNodeMap();
		CIntegerPtr IntPtr = sNodeMap.GetNode(name);
		if (IsAvailable(IntPtr) && IsReadable(IntPtr)) {
			val = IntPtr->GetValue();
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetTLSteamNodeIntParameter");
	}
	return false;
}

bool SpinnakerCamera::SetNodeFloatParameter(const char* name, double val)
{
	double max, min;

	if (!IsReady()) return false;

	try {
		INodeMap& nodeMap = spCam->GetNodeMap();
		CFloatPtr ptrValue = nodeMap.GetNode(name);
		if (IsAvailable(ptrValue) && IsReadable(ptrValue)) {
			max = ptrValue->GetMax();
			min = ptrValue->GetMin();
			if (val < min) val = min;
			if (val > max) val = max;
			if (IsWritable(ptrValue)) {
				ptrValue->SetValue(val);
				return true;
			}
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetNodeFloatParameter");
	}
	return false;
}


bool SpinnakerCamera::GetNodeFloatParameter(const char* name, double& val, double& max, double& min)
{
	if (!IsReady()) return false;

	try {
		INodeMap& nodeMap = spCam->GetNodeMap();
		CFloatPtr ptrValue = nodeMap.GetNode(name);
		if (IsAvailable(ptrValue) && IsReadable(ptrValue)) {
			val = ptrValue->GetValue();
			max = ptrValue->GetMax();
			min = ptrValue->GetMin();
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"GetNodeFloatParameter");
	}
	return false;
}


bool SpinnakerCamera::SetNodeIntParameter(const char* name, INT64 val)
{
	INT64 max, min;
	if (!IsReady()) return false;

	try {
		INodeMap& nodeMap = spCam->GetNodeMap();
		CIntegerPtr ptrInt = nodeMap.GetNode(name);
		if (IsReadable(ptrInt) && IsWritable(ptrInt)) {
			max = ptrInt->GetMax();
			min = ptrInt->GetMin();
			if (val < min) val = min;
			if (val > max) val = max;
			ptrInt->SetValue(val);
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetNodeIntParameter");
	}
	return false;
}


bool SpinnakerCamera::GetNodeIntParameter(const char* name, INT64& val, INT64& max, INT64& min)
{
	try {
		INodeMap& nodeMap = spCam->GetNodeMap();
		CIntegerPtr ptrInt = nodeMap.GetNode(name);
		if (IsReadable(ptrInt)) {
			val = ptrInt->GetValue();
			max = ptrInt->GetMax();
			min = ptrInt->GetMin();
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"GetNodeIntParameter");
	}
	return false;
}








bool SpinnakerCamera::SetNodeEnumParameter(const char* name, int val)
{
	bool result = false;

	if (!IsReady()) return result;
	try {
		INodeMap& nodeMap = spCam->GetNodeMap();

		CEnumerationPtr ptrAuto = nodeMap.GetNode(name);
		if (IsAvailable(ptrAuto) && IsWritable(ptrAuto)) {
			ptrAuto->SetIntValue(val);
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetNodeEnumParameter");
		result = false;
	}
	return result;
}

bool SpinnakerCamera::SetNodeEnumParameter(const char* name, const char* val)
{
	if (!IsReady()) return false;

	std::string sval = string(val);
	std::transform(sval.begin(), sval.end(), sval.begin(), ::tolower);

	try {
		INodeMap& nodeMap = spCam->GetNodeMap();
		CEnumerationPtr ptrEnumNode = nodeMap.GetNode(name);
		if (IsAvailable(ptrEnumNode) && IsWritable(ptrEnumNode)) {
			NodeList_t entries;
			ptrEnumNode->GetEntries(entries);
			for (int i = 0; i < entries.size(); i++) {
				CEnumEntryPtr eptr = entries.at(i);
				string str = string(eptr->GetSymbolic());
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				if (str == sval) {
					ptrEnumNode->SetIntValue(eptr->GetValue());
					return true;
				}
			}
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetNodeEnumParameter");
	}
	return false;
}

bool SpinnakerCamera::GetNodeEnumParameter(const char* name, string& val)
{
	if (!IsReady()) return false;
	val = string("None");

	try {
		INodeMap& nodeMap = spCam->GetNodeMap();
		CEnumerationPtr ptrEnumNode = nodeMap.GetNode(name);
		if (IsAvailable(ptrEnumNode) && IsWritable(ptrEnumNode)) {
			val = string(ptrEnumNode->GetCurrentEntry()->GetSymbolic());
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"GetNodeEnumParameter");
	}
	return false;
}


bool SpinnakerCamera::SetNodeBoolParameter(const char* name, bool val)
{
	bool result = false;

	if (!IsReady()) return result;
	try {
		INodeMap& nodeMap = spCam->GetNodeMap();

		CBooleanPtr Boolptr = nodeMap.GetNode(name);
		if (IsAvailable(Boolptr) && IsWritable(Boolptr)) {
			Boolptr->SetValue(val);
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetNodeBoolParameter");
		result = false;
	}
	return result;
}



bool SpinnakerCamera::GetNodeParameter(NodeValues& p)
{
	bool result = false;

	if (!IsReady()) return result;
	try {
		INodeMap& nodeMap = spCam->GetNodeMap();

		p.IsAuto = false;
		if (p.nodeAuto != string("None")) {
			CEnumerationPtr ptrAuto = nodeMap.GetNode(p.nodeAuto.c_str());
			if (IsAvailable(ptrAuto) && IsReadable(ptrAuto)) {
				string symbol = string(ptrAuto->GetCurrentEntry()->GetSymbolic());
				std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
				string off = p.auto_off;
				std::transform(off.begin(), off.end(), off.begin(), ::tolower);
				if (symbol == off) p.IsAuto = false;
				else p.IsAuto = true;
			}
			else {
				p.nodeAuto = string("None");
			}
		}

		CFloatPtr ptrValue = nodeMap.GetNode(p.nodeName.c_str());
		if (IsAvailable(ptrValue) && IsReadable(ptrValue)) {
			p.max = ptrValue->GetMax();
			p.min = ptrValue->GetMin();
			p.val = ptrValue->GetValue();
			return true;
		}
		else {
			p.nodeName = string("None");
		}

	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"GetNodeParameter");
	}
	return false;
}

bool SpinnakerCamera::GetNodeBoolParameter(const char* name, bool& val)
{
	if (!IsReady()) return false;
	try {
		INodeMap& nodeMap = spCam->GetNodeMap();

		CBooleanPtr Boolptr = nodeMap.GetNode(name);
		if (IsAvailable(Boolptr) && IsWritable(Boolptr)) {
			val = Boolptr->GetValue();
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetNodeBoolParameter");
	}
	return false;
}
bool SpinnakerCamera::SetNodeParameter(NodeValues& p)
{
	bool result = false;
	string sval;
	if (!IsReady()) return result;

	try {
		if (!GetNodeEnumParameter(p.nodeAuto.c_str(), sval)) {
			p.nodeAuto = string("None");
			p.IsAuto = false;
		}
		else {
			if (p.IsAuto) {
				SetNodeEnumParameter(p.nodeAuto.c_str(), p.auto_on.c_str());
			}
			else {
				SetNodeEnumParameter(p.nodeAuto.c_str(), p.auto_off.c_str());
			}
		}
		if (!SetNodeFloatParameter(p.nodeName.c_str(), p.val)) {
			p.nodeName = string("None");
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetNodeParameter");
		result = false;
	}
	return result;
}


bool SpinnakerCamera::SetWidthOffsetX(int w, int x)
{
	try {
		if (IsReadable(spCam->OffsetX) && IsWritable(spCam->OffsetX)) {
			spCam->OffsetX.SetValue(spCam->OffsetX.GetMin());
			if (IsReadable(spCam->Width) && IsWritable(spCam->Width)) {
				int inc = static_cast<int>(spCam->Width.GetInc());
				w = w / inc * inc;
				if (w > spCam->Width.GetMax() || w < spCam->Width.GetMin()) return false;
				spCam->Width.SetValue(w);

				inc = static_cast<int>(spCam->OffsetX.GetInc());
				x = x / inc * inc;
				if (x > spCam->OffsetX.GetMax() || x < spCam->OffsetX.GetMin()) return false;
				spCam->OffsetX.SetValue(x);
				return true;
			}
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetWidthOffsetX");
	}
	return false;
}

bool SpinnakerCamera::GetWidthOffsetX(int& w, int& x)
{
	try {
		if (IsReadable(spCam->OffsetX) && IsReadable(spCam->Width)) {
			w = static_cast<int>(spCam->Width.GetValue());
			x = static_cast<int>(spCam->OffsetX.GetValue());
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"GetWidthOffsetX");
	}

	return false;
}

bool SpinnakerCamera::SetHeightOffsetY(int h, int y)
{
	try {
		if (IsReadable(spCam->OffsetY) && IsWritable(spCam->OffsetY)) {
			spCam->OffsetY.SetValue(spCam->OffsetY.GetMin());
			if (IsReadable(spCam->Height) && IsWritable(spCam->Height)) {
				int inc = static_cast<int>(spCam->Height.GetInc());
				h = h / inc * inc;
				if (h > spCam->Height.GetMax() || h < spCam->Height.GetMin()) return false;
				spCam->Height.SetValue(h);
				inc = static_cast<int>(spCam->OffsetY.GetInc());
				if (y > spCam->OffsetY.GetMax() || y < spCam->OffsetY.GetMin()) return false;
				spCam->OffsetY.SetValue(y);
				return true;
			}
		}
		cout << "spcam->OffsetY" << "isn't readable or writable" << endl;
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetHeightOffsetY");
	}
	return false;
}

bool SpinnakerCamera::GetHeightOffsetY(int& h, int& y)
{
	try {
		if (IsReadable(spCam->OffsetY) && IsReadable(spCam->Height)) {
			h = static_cast<int>(spCam->Height.GetValue());
			y = static_cast<int>(spCam->OffsetY.GetValue());
			return true;
		}
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"GetHeightOffsetY");
	}

	return false;
}
bool SpinnakerCamera::SetFullScreen(void)
{
	if (!IsReady()) return false;
	SPINNAKER_CAMERA_PARAM p;

	try {
		p.StreamBufferHandlingMode = string("NewestOnly");
		p.StreamBufferCountMode = string("Manual");
		p.StreamBufferCount = 3;

		p.AdcBitDepth = string("Bit8");

		p.AcquisitionMode = AcquisitionMode_Continuous;
		p.PixelFormat = string("Mono8");

		p.BinningSelector = string("All");

		p.BinningH = 1;
		p.BinningV = 1;
		p.OffsetX = 0;
		p.OffsetY = 0;
		p.w = static_cast<int>(spCam->SensorWidth());
		p.h = static_cast<int>(spCam->SensorHeight());

		p.AcquisitionMode = string("Continuous");

		p.AcquisitionFrameRateEnable = true;
		p.AcquisitionFrameRateAuto = string("Continuous");
		p.AcquisitionFrameRate = 60;

		p.pgrExposureCompensationAuto = string("Continuous");
		p.pgrExposureCompensation = 0;

		p.ExposureMode = string("Timed");
		p.ExposureAuto = string("Continuous");
		p.ExposureTime = 1000;

		p.GainAuto = string("Continuous");;
		p.Gain = 0;
		p.BlackLevelSelector = string("All");
		p.BlackLevel = 0;

		p.GammaEnable = false;
		p.Gamma = 1;
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetFullScreen");
		return false;
	}

	return SetCameraParameter(p);
}
bool SpinnakerCamera::SetCamera(std::string ser)
{
	if (IsReady()) {
		if (spCam->IsInitialized()) {
			spCam->DeInit();
		}
		spCam = nullptr;
	}
	spCam = SpinnakerCameraList.GetBySerial(ser);
	if (spCam == NULL) {
		return false;
	}
	return true;
}

bool SpinnakerCamera::SetCamera(unsigned int no)
{
	try {
		string str;
		int n = 0;

		if (no >= SpinnakerCameraList.GetSize()) {
			return false;
		}

		if (IsReady()) {
			if (spCam->IsInitialized()) {
				spCam->DeInit();
			}
			spCam = nullptr;
		}
		spCam = SpinnakerCameraList.GetByIndex(no);
		spCam->Init();

		SetAllChunkMode(true);
		GetCameraParameter(CameraParam);

		spCamSerial = CameraParam.SerialNumber;
		CameraParam.StreamBufferHandlingMode = string("NewestOnly");
		CameraParam.StreamBufferCountMode = string("Manual");
		CameraParam.StreamBufferCount = 10;

		CameraParam.AdcBitDepth = string("Bit8");
		CameraParam.PixelFormat = string("Mono8");

		CameraParam.BinningSelector = string("All");
		CameraParam.BinningH = 1;
		CameraParam.BinningV = 1;
		CameraParam.OffsetX = 0;
		CameraParam.OffsetY = 0;
		CameraParam.w = MaxWidth();
		CameraParam.h = MaxHeight();

		CameraParam.AcquisitionMode = string("Continuous");

		CameraParam.AcquisitionFrameRateEnable = true;
		CameraParam.AcquisitionFrameRateAuto = string("Continuous");
		CameraParam.AcquisitionFrameRate = 60;

		CameraParam.pgrExposureCompensationAuto = string("Continuous");
		CameraParam.pgrExposureCompensation = 0;

		CameraParam.ExposureMode = string("Timed");
		CameraParam.ExposureAuto = string("On");
		CameraParam.ExposureTime = 1000;

		CameraParam.GainAuto = string("Off");;
		CameraParam.Gain = 0;
		CameraParam.BlackLevelSelector = string("All");
		CameraParam.BlackLevel = 0;

		CameraParam.GammaEnable = true;
		CameraParam.Gamma = 1;

		SetCameraParameter(CameraParam);

		Start();
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetCamera");
		return false;
	}
	return true;
}






//Camera Select Dialog
SpinnakerCamera::DlgSelect::DlgSelect()
{
	camIndex = 0;
	CamPtr = NULL;
	hWndParent = NULL;
	lb.reset();
	CameraListVector.clear();
}
SpinnakerCamera::DlgSelect::~DlgSelect()
{
	CameraListVector.clear();
}
bool SpinnakerCamera::DlgSelect::SetCameraList(void)
{
	SPINNAKER_CAMERA_DATA scd;
	string str;
	CameraPtr cp;
	CameraListVector.clear();

	bool result = false;

	for (unsigned int i = 0; i < CamPtr->SpinnakerCameraList.GetSize(); i++) {

		cp = CamPtr->SpinnakerCameraList.GetByIndex(i);

		scd.index = i;
		str = cp->TLDevice.DeviceSerialNumber.ToString();
		scd.SerialNumber = utf8_decode(str);

		str = cp->TLDevice.DeviceModelName.ToString();
		scd.DeviceModelName = utf8_decode(str);

		str = cp->TLDevice.DeviceCurrentSpeed.ToString();
		scd.DeviceCurrentSpeed = utf8_decode(str);

		CameraListVector.push_back(scd);
		result = true;
	}
	cp = nullptr;

	return result;
}
INT_PTR SpinnakerCamera::DlgSelect::Open(HWND hWnd, SpinnakerCamera& pcam)
{
	INT_PTR iptr;
	hWndParent = hWnd;
	CamPtr = &pcam;
	if (!SetCameraList()) {
		return (INT_PTR)false;
	}
	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CAMERA_LIST), hWndParent, CallbackProc, (LPARAM)this);
	return iptr;
}


bool SpinnakerCamera::DlgSelect::InitCtrl(HWND hDlg)
{
	std::wstringstream wss;
	lb.reset(new DlgListBox(hDlg, IDC_CAMERA_LIST));
	for (auto vcl : CameraListVector) {
		wss.str(TEXT(""));
		wss.clear(std::stringstream::goodbit);
		wss << boost::wformat(TEXT("%-8d")) % vcl.index;
		wss << boost::wformat(TEXT("%-12s")) % vcl.SerialNumber;
		wss << boost::wformat(TEXT("%-40s")) % vcl.DeviceModelName;
		wss << boost::wformat(TEXT("%-20s")) % vcl.DeviceCurrentSpeed;
		lb->add(wss.str());
	}
	lb->Set(0);

	return true;
}
void SpinnakerCamera::DlgSelect::EndCtrl(HWND hDlg)
{
	lb.reset();
}
bool SpinnakerCamera::DlgSelect::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM)
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

		switch (wmId) {
		case IDC_CAMERA_LIST:
			camIndex = lb->Get();
			return (INT_PTR)true;
		case IDOK:
			CamPtr->SetCamera(camIndex);
			
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
INT_PTR CALLBACK SpinnakerCamera::DlgSelect::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DlgSelect* thisPtr = NULL;

	switch (message) {
	case WM_INITDIALOG:
		thisPtr = (DlgSelect*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
		break;
	case WM_DESTROY:
		thisPtr = (DlgSelect*)GetWindowLongPtr(hDlg, DWLP_USER);
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)0);
		break;
	default:
		thisPtr = (DlgSelect*)GetWindowLongPtr(hDlg, DWLP_USER);
		break;
	}
	if (thisPtr) {
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}

bool SpinnakerCamera::DialogSelect(HWND hWnd)
{
	DlgSelect ds;
	return ds.Open(hWnd, *this);
}



//Input Region Dialog
void SpinnakerCamera::InputRegion::SetInputOffset()
{
	int lx, ly, rx, ry;

	lx = (start.x - BMP_OFFSETX) * ReductionRatio - w.min / 2;
	ly = (start.y - BMP_OFFSETY) * ReductionRatio - h.min / 2;

	lx = lx - lx % offx.inc;
	if (lx < offx.min) lx = offx.min;
	if (lx > offx.max) lx = offx.max;

	ly = ly - ly % offy.inc;
	if (ly < offy.min) ly = offy.min;
	if (ly > offy.max) ly = offy.max;


	rx = lx + w.min;
	if (rx >= w.max) rx = w.max;

	ry = ly + h.min;
	if (ry >= h.max) ry = h.max;


	offx.val = lx;
	offy.val = ly;
	w.val = rx - lx;
	h.val = ry - ly;

	sp_offsetx->Set(offx.val);
	sp_offsety->Set(offy.val);
	sp_width->Set(w.val);
	sp_height->Set(w.val);
}
void SpinnakerCamera::InputRegion::SetInputSize()
{
	int lx, ly, rx, ry;
	int flag = 1;

	if (start.x < end.x) {
		lx = (start.x - BMP_OFFSETX) * ReductionRatio - w.val / 2;
		rx = (end.x - BMP_OFFSETX) * ReductionRatio;
	}
	else {
		flag = flag * -1;
		rx = (start.x - BMP_OFFSETX) * ReductionRatio + w.val / 2;
		lx = (end.x - BMP_OFFSETX) * ReductionRatio;
	}
	if (start.y < end.y) {
		ly = (start.y - BMP_OFFSETY) * ReductionRatio - h.val / 2;
		ry = (end.y - BMP_OFFSETY) * ReductionRatio;
	}
	else {
		ry = (start.y - BMP_OFFSETY) * ReductionRatio + h.val / 2;
		ly = (end.y - BMP_OFFSETY) * ReductionRatio;
		flag = flag * -1;
	}
	if (flag == 1) {
		SetCursor(hCursorSIZENWSE);
	}
	else {
		SetCursor(hCursorSIZENESW);
	}

	lx = lx - lx % offx.inc;
	if (lx < offx.min) lx = offx.min;
	if (lx > offx.max) lx = offx.max;

	ly = ly - ly % offy.inc;
	if (ly < offy.min) ly = offy.min;
	if (ly > offy.max) ly = offy.max;

	w.val = (rx - lx);
	w.val = w.val - w.val % w.inc;
	h.val = (ry - ly);
	h.val = h.val - h.val % h.inc;

	if ((lx + w.val) > w.max) {
		w.val = w.max - lx;
	}
	if ((ly + h.val) > h.max) {
		h.val = h.max - ly;
	}
	offx.val = lx;
	offy.val = ly;
	sp_offsetx->Set(offx.val);
	sp_offsety->Set(offy.val);
	sp_width->Set(w.val);
	sp_height->Set(h.val);
}
void SpinnakerCamera::InputRegion::SetInputCenter()
{
	int lx, ly, rx, ry;

	lx = (start.x - BMP_OFFSETX) * ReductionRatio - w.val / 2;
	ly = (start.y - BMP_OFFSETY) * ReductionRatio - h.val / 2;

	lx = lx - lx % offx.inc;
	if (lx < offx.min) lx = offx.min;
	if (lx > offx.max) lx = offx.max;

	ly = ly - ly % offy.inc;
	if (ly < offy.min) ly = offy.min;
	if (ly > offy.max) ly = offy.max;

	rx = lx + w.val;
	if (rx >= w.max) {
		rx = w.max;
		lx = rx - w.val;
	}


	ry = ly + h.val;
	if (ry >= h.max) {
		ry = h.max;
		ly = ry - h.val;
	}


	SetCursor(hCursorSIZEALL);
	offx.val = lx;
	offy.val = ly;
	sp_offsetx->Set(offx.val);
	sp_offsety->Set(offy.val);
}
void SpinnakerCamera::InputRegion::SetOffsetX()
{
	offx.val = sp_offsetx->Get();
	w.val = sp_width->Get();
	if ((offx.val + w.val) > w.max) {
		w.val = w.max - offx.val;
		sp_width->Set(w.val);

	}
}
void SpinnakerCamera::InputRegion::SetOffsetY()
{
	offy.val = sp_offsety->Get();
	h.val = sp_height->Get();
	if ((offy.val + h.val) > h.max) {
		h.val = h.max - offy.val;
		sp_height->Set(h.val);
	}
}
void SpinnakerCamera::InputRegion::SetWidth()
{

	offx.val = sp_offsetx->Get();
	w.val = sp_width->Get();

	if ((offx.val + w.val) > w.max) {
		offx.val = w.max - w.val;
		if (offx.val > offx.min) {
			sp_offsetx->Set(offx.val);
		}
		else {
			offx.val = offx.min;
			w.val = w.max - offx.min;
			sp_offsetx->Set(offx.val);
			sp_width->Set(w.val);
		}
	}

}
void SpinnakerCamera::InputRegion::SetHeight()
{
	offy.val = sp_offsety->Get();
	h.val = sp_height->Get();

	if ((offy.val + h.val) > h.max) {
		offy.val = h.max - h.val;
		if (offy.val > offy.min) {
			sp_offsety->Set(offy.val);
		}
		else {
			offy.val = offy.min;
			h.val = h.max - offy.min;
			sp_offsety->Set(offy.val);
			sp_height->Set(h.val);
		}
	}
}
SpinnakerCamera::InputRegion::InputRegion()
{
	CamPtr = NULL;
	hWndParent = NULL;
	hCursorSIZEALL = LoadCursor(NULL, IDC_SIZEALL);
	hCursorSIZENESW = LoadCursor(NULL, IDC_SIZENESW);
	hCursorSIZENWSE = LoadCursor(NULL, IDC_SIZENWSE);
	hCursorArrow = LoadCursor(NULL, IDC_ARROW);
	start = { 0,0 };
	end = start;
	bmp.reset();
	btn_OK.reset();
	eb_width.reset();
	eb_height.reset();
	eb_offsetx.reset();
	eb_offsety.reset();

	sp_width.reset();
	sp_height.reset();
	sp_offsetx.reset();
	sp_offsety.reset();
	CameraImage ci;
}
SpinnakerCamera::InputRegion::~InputRegion()
{

}
void SpinnakerCamera::InputRegion::InitCtrl(HWND hDlg)
{
	RECT rc;
	std::wstring ws;

	GetWindowRect(hDlg, &rc);
	rc.right = rc.left + (LONG)w.max / ReductionRatio;
	rc.bottom = rc.top + (LONG)h.max / ReductionRatio + BMP_OFFSETY;
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	if (rc.top < 0) {
		rc.bottom = rc.bottom - rc.top;
		rc.top = 0;
	}
	MoveWindow(hDlg, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
	GetClientRect(hDlg, &rc);
	btn_OK.reset(new DlgButton(hDlg, IDOK));

	bmp.reset(new DlgBitmap(hDlg, IDC_CAM_BITMAP, (int)(w.max / ReductionRatio), (int)(h.max / ReductionRatio)));
	bmp->fill(WHITENESS);
	bmp->move(BMP_OFFSETX, BMP_OFFSETY);
	bmp->refresh();

	eb_width.reset(new DlgEditBox(hDlg, IDC_EDIT_WIDTH));
	eb_height.reset(new DlgEditBox(hDlg, IDC_EDIT_HEIGHT));
	eb_offsetx.reset(new DlgEditBox(hDlg, IDC_EDIT_OFFSETX));
	eb_offsety.reset(new DlgEditBox(hDlg, IDC_EDIT_OFFSETY));

	sp_width.reset(new DlgSpin(hDlg, IDC_SPIN_WIDTH));
	sp_height.reset(new DlgSpin(hDlg, IDC_SPIN_HEIGHT));
	sp_offsetx.reset(new DlgSpin(hDlg, IDC_SPIN_OFFSETX));
	sp_offsety.reset(new DlgSpin(hDlg, IDC_SPIN_OFFSETY));

	sp_width->SetBuddy(eb_width->hWnd);
	sp_height->SetBuddy(eb_height->hWnd);
	sp_offsetx->SetBuddy(eb_offsetx->hWnd);
	sp_offsety->SetBuddy(eb_offsety->hWnd);

	sp_width->SetMinMax(w.min, w.max);
	sp_height->SetMinMax(h.min, h.max);
	sp_offsetx->SetMinMax(offx.min, offx.max);
	sp_offsety->SetMinMax(offy.min, offy.max);

	sp_width->Set(w.val);
	sp_height->Set(h.val);
	sp_offsetx->Set(offx.val);
	sp_offsety->Set(offy.val);

	sp_width->SetAccel(w.inc);
	sp_height->SetAccel(h.inc);
	sp_offsetx->SetAccel(offx.inc);
	sp_offsety->SetAccel(offy.inc);

	SetTimer(hDlg, ID_CAMREGION_TIMER, CAMREGION_INTERVAL, NULL);
}
void SpinnakerCamera::InputRegion::EndCtrl(HWND hDlg)
{
	KillTimer(hDlg, ID_CAMREGION_TIMER);
}
INT_PTR  SpinnakerCamera::InputRegion::Open(HWND hWnd, SpinnakerCamera& pcam)
{
	INT_PTR iptr;
	hWndParent = hWnd;
	CamPtr = &pcam;
	if (!CamPtr->IsReady()) return false;
	if (!CamPtr->IsStreaming()) CamPtr->Start();
	try {
		w.max = static_cast<int>(CamPtr->spCam->Width.GetMax());
		w.min = static_cast<int>(CamPtr->spCam->Width.GetMin());
		w.inc = static_cast<int>(CamPtr->spCam->Width.GetInc());
		w.val = CamPtr->CameraParam.w;


		h.max = static_cast<int>(CamPtr->spCam->Height.GetMax());
		h.min = static_cast<int>(CamPtr->spCam->Height.GetMin());
		h.inc = static_cast<int>(CamPtr->spCam->Height.GetInc());
		h.val = CamPtr->CameraParam.h;


		offx.inc = static_cast<int>(CamPtr->spCam->OffsetX.GetInc());
		offx.min = static_cast<int>(CamPtr->spCam->OffsetX.GetMin());
		offx.max = w.max - w.min;
		offx.val = CamPtr->CameraParam.OffsetX;

		offy.inc = static_cast<int>(CamPtr->spCam->OffsetY.GetInc());
		offy.min = static_cast<int>(CamPtr->spCam->OffsetY.GetMin());
		offy.max = h.max - h.min;
		offy.val = CamPtr->CameraParam.OffsetY;
	}
	catch (Spinnaker::Exception& e) {
		CamPtr->ErrorProc(e, L"InputRegion::Open");
	}

	ReductionRatio = static_cast<int>(ceil(static_cast<double>(w.max) / 1000.0));

	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CAM_REGION), hWndParent, CallbackProc, (LPARAM)this);
	return iptr;

}
BOOL  SpinnakerCamera::InputRegion::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	static bool LButtonDown = false;
	static bool RButtonDown = false;
	static bool flag = 0;
	//	IMAGE_HEADER img_header;
	std::wstringstream wss;
	static bool busy = false;
	int sx, sy, width, height;
	switch (message)
	{
	case WM_INITDIALOG:
		InitCtrl(hDlg);
		return (INT_PTR)TRUE;
	case WM_RBUTTONDOWN:
		if (LButtonDown) {
			return (INT_PTR)TRUE;
		}
		start = end = MAKEPOINTS(lParam);
		if ((start.x > BMP_OFFSETX) && (start.y > BMP_OFFSETY)) {
			RButtonDown = true;
			SetCursor(hCursorSIZENWSE);
			SetInputOffset();

			SetCapture(hDlg);
		}
		return (INT_PTR)TRUE;
	case WM_MOUSEMOVE:
		if (RButtonDown) {
			end = MAKEPOINTS(lParam);
			SetInputSize();
		}
		if (LButtonDown) {
			start = MAKEPOINTS(lParam);
			SetInputCenter();
		}
		return (INT_PTR)TRUE;
	case WM_RBUTTONUP:
		if (RButtonDown) {
			RButtonDown = false;
			SetCursor(hCursorArrow);
			ReleaseCapture();
		}
		return (INT_PTR)TRUE;
	case WM_LBUTTONDOWN:
		if (RButtonDown) {
			return (INT_PTR)TRUE;
		}
		LButtonDown = true;
		start = MAKEPOINTS(lParam);
		SetCursor(hCursorSIZEALL);
		SetInputCenter();
		return (INT_PTR)TRUE;
	case WM_LBUTTONUP:
		LButtonDown = false;
		SetCursor(hCursorArrow);
		return (INT_PTR)TRUE;
	case WM_VSCROLL:
		if ((HWND)lParam == sp_offsetx->hWnd && LOWORD(wParam) == SB_THUMBPOSITION) {
			SetOffsetX();
		}

		if ((HWND)lParam == sp_offsety->hWnd && LOWORD(wParam) == SB_THUMBPOSITION) {
			SetOffsetY();
		}

		if ((HWND)lParam == sp_width->hWnd && LOWORD(wParam) == SB_THUMBPOSITION) {
			SetWidth();
		}

		if ((HWND)lParam == sp_height->hWnd && LOWORD(wParam) == SB_THUMBPOSITION) {
			SetHeight();
		}
		return (INT_PTR)TRUE;
	case WM_TIMER:
		if (busy) return (INT_PTR)TRUE;
		busy = true;
		CamPtr->CaptureImage(ci);
		bmp->copy(ci.img, static_cast<int>(ci.w), static_cast<int>(ci.h), static_cast<int>(ci.pixelbits));
		sx = sp_offsetx->Get() / ReductionRatio;
		sy = sp_offsety->Get() / ReductionRatio;
		width = sp_width->Get() / ReductionRatio;
		height = sp_height->Get() / ReductionRatio;
		bmp->rect(sx, sy, width, height, RGB(255, 0, 0));
		bmp->dotline(sx + width / 2, sy, sx + width / 2, sy + height, RGB(255, 0, 0));
		bmp->dotline(sx, sy + height / 2, sx + width, sy + height / 2, RGB(255, 0, 0));
		bmp->refresh();
		busy = false;
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId) {

		case IDOK:
			CamPtr->CameraParam.w = sp_width->Get();
			CamPtr->CameraParam.h = sp_height->Get();
			CamPtr->CameraParam.OffsetX = sp_offsetx->Get();
			CamPtr->CameraParam.OffsetY = sp_offsety->Get();

			EndCtrl(hDlg);
			return EndDialog(hDlg, wmId);
		case IDCANCEL:

			EndCtrl(hDlg);
			return EndDialog(hDlg, wmId);
		}
		return (INT_PTR)false;
	}
	return false;
}
INT_PTR CALLBACK  SpinnakerCamera::InputRegion::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	InputRegion* thisPtr = NULL;

	switch (message) {
	case WM_INITDIALOG:
		thisPtr = (InputRegion*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
		break;
	case WM_DESTROY:
		thisPtr = (InputRegion*)GetWindowLongPtr(hDlg, DWLP_USER);
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)0);
		break;
	default:
		thisPtr = (InputRegion*)GetWindowLongPtr(hDlg, DWLP_USER);
		break;
	}
	if (thisPtr) {
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}

bool SpinnakerCamera::DialogRegion(HWND hWnd)
{
	bool result = false;
	InputRegion ir;
	if (!IsReady()) return result;
	GetCameraParameter(CameraParam);
	if (hWnd == NULL) {
		if (CameraParam.w > INIT_INPUT_W) {
			CameraParam.w = INIT_INPUT_W;
		}
		if (CameraParam.h > INIT_INPUT_H) {
			CameraParam.h = INIT_INPUT_H;
		}
	}
	SetFullScreen();
	result = ir.Open(hWnd, *this);
	SetCameraParameter(CameraParam);
	return result;
}

//Camera Parameter
#define CAMPARAM_BMP_OFFSETX 450
#define CAMPARAM_BMP_OFFSETY 0
#define CAMPARAM_BMP_W 512
#define CAMPARAM_BMP_H 480

SpinnakerCamera::Parameters::Parameters()
{
	CamPtr = NULL;
	hWndParent = NULL;

	FrameRate.nodeAuto = string("AcquisitionFrameRateAuto");
	FrameRate.nodeName = string("AcquisitionFrameRate");

	ExposureCompensation.nodeAuto = string("pgrExposureCompensationAuto");
	ExposureCompensation.nodeName = string("pgrExposureCompensation");

	ExposureTime.nodeAuto = string("ExposureAuto");
	ExposureTime.nodeName = string("ExposureTime");


	Gain.nodeAuto = string("GainAuto");
	Gain.nodeName = string("Gain");

	BlackLevel.nodeAuto = string("None");
	BlackLevel.nodeName = string("BlackLevel");

	btn_OK.reset();
	eb_FrameRate.reset();
	eb_ExposureCompensation.reset();
	eb_ExposureTime.reset();
	eb_Gain.reset();
	eb_BlackLevel.reset();

	sl_FrameRate.reset();
	sl_ExposureCompensation.reset();
	sl_ExposureTime.reset();
	sl_Gain.reset();
	sl_BlackLevel.reset();
	cb_FrameRate.reset();
	cb_ExposureCompensation.reset();
	cb_ExposureTime.reset();
	cb_Gain.reset();
}
SpinnakerCamera::Parameters::~Parameters()
{
}
void SpinnakerCamera::Parameters::InitCtrl(HWND hDlg)
{
	NodeValues pv;
	RECT rc;
	LONG cam_w, cam_h, cam_x, cam_y;
	cb_FrameRate.reset(new DlgCheckbox(hDlg, IDC_CHECK_FrameRateAuto));
	sl_FrameRate.reset(new DlgSlider(hDlg, IDC_SLIDER_FrameRate, 0, 100, 0, 100, 1));
	eb_FrameRate.reset(new DlgEditBox(hDlg, IDC_EDIT_FrameRate));

	cb_ExposureCompensation.reset(new DlgCheckbox(hDlg, IDC_CHECK_ExposureCompensationAuto));
	sl_ExposureCompensation.reset(new DlgSlider(hDlg, IDC_SLIDER_ExposureCompensation, 0, 10, 0, 1, 1));
	eb_ExposureCompensation.reset(new DlgEditBox(hDlg, IDC_EDIT_ExposureCompensation));

	cb_ExposureTime.reset(new DlgCheckbox(hDlg, IDC_CHECK_ExposureTimeAuto));
	sl_ExposureTime.reset(new DlgSlider(hDlg, IDC_SLIDER_ExposureTime, 0, 100, 0, 100, 1));
	eb_ExposureTime.reset(new DlgEditBox(hDlg, IDC_EDIT_ExposureTime));

	cb_Gain.reset(new DlgCheckbox(hDlg, IDC_CHECK_GainAuto));
	sl_Gain.reset(new DlgSlider(hDlg, IDC_SLIDER_Gain, 0, 100, 0, 10, 1));
	eb_Gain.reset(new DlgEditBox(hDlg, IDC_EDIT_Gain));

	sl_BlackLevel.reset(new DlgSlider(hDlg, IDC_SLIDER_BlackLevel, 0, 10, 0, 1, 2));
	eb_BlackLevel.reset(new DlgEditBox(hDlg, IDC_EDIT_BlackLevel));

	btn_OK.reset(new DlgButton(hDlg, IDOK));

	cam_w = CamPtr->CameraParam.w;
	cam_h = CamPtr->CameraParam.h;
	cam_x = CAMPARAM_BMP_OFFSETX;
	cam_y = CAMPARAM_BMP_OFFSETY;

	cam_w = (LONG)((double)cam_w * (double)CAMPARAM_BMP_H / (double)cam_h);
	cam_h = CAMPARAM_BMP_H;

	GetWindowRect(hDlg, &rc);
	rc.right = rc.left + CAMPARAM_BMP_OFFSETX + cam_w;
	rc.bottom = rc.top + CAMPARAM_BMP_OFFSETY + cam_h;
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	if (rc.top < 0) {
		rc.bottom = rc.bottom - rc.top;
		rc.top = 0;
	}
	MoveWindow(hDlg, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true);
	GetClientRect(hDlg, &rc);
	btn_OK.reset(new DlgButton(hDlg, IDOK));

	bmp.reset(new DlgBitmap(hDlg, IDC_CAMPARAM_BITMAP, (int)(cam_w), (int)(cam_h)));
	bmp->fill(BLACKNESS);
	bmp->move(CAMPARAM_BMP_OFFSETX, CAMPARAM_BMP_OFFSETY);
	bmp->refresh();
	CamPtr->Start();
	SetTimer(hDlg, ID_CAMPARAM_TIMER, CAMPARAM_INTERVAL, NULL);
}
void SpinnakerCamera::Parameters::EndCtrl(HWND hDlg)
{
	KillTimer(hDlg, ID_CAMPARAM_TIMER);

}
INT_PTR SpinnakerCamera::Parameters::Open(HWND hWnd, SpinnakerCamera& pcam)
{
	INT_PTR iptr;
	CamPtr = &pcam;

	hWndParent = hWnd;
	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_CameraParameter), hWndParent, CallbackProc, (LPARAM)this);

	return iptr;
}

void SpinnakerCamera::Parameters::ShowParameters(void)
{
	NodeValues fr;
	wstring fmt = L"%6.2f";

	CamPtr->GetNodeParameter(FrameRate);
	if (FrameRate.IsAuto) {
		cb_FrameRate->Set(true);
		sl_FrameRate->disable();
		eb_FrameRate->disable();
	}
	else {
		cb_FrameRate->Set(false);
		sl_FrameRate->enable();
		eb_FrameRate->enable();
	}
	sl_FrameRate->Set(static_cast<int>(FrameRate.val));
	sl_FrameRate->SetMax(static_cast<int>(FrameRate.max));
	sl_FrameRate->SetMin(static_cast<int>(FrameRate.min));
	if (eb_FrameRate->hWnd != GetFocus()) {
		eb_FrameRate->SetDouble(fmt, FrameRate.val);
	}


	if (CamPtr->GetNodeParameter(ExposureCompensation)) {
		cb_ExposureCompensation->enable();
		if (ExposureCompensation.IsAuto) {
			cb_ExposureCompensation->Set(true);
			sl_ExposureCompensation->disable();
			eb_ExposureCompensation->disable();
		}
		else {
			cb_ExposureCompensation->Set(false);
			sl_ExposureCompensation->enable();
			eb_ExposureCompensation->enable();
		}
		sl_ExposureCompensation->Set(static_cast<int>(ExposureCompensation.val));


		sl_ExposureCompensation->SetMax(static_cast<int>(ExposureCompensation.max));
		sl_ExposureCompensation->SetMin(static_cast<int>(ExposureCompensation.min));
		if (eb_ExposureCompensation->hWnd != GetFocus()) {
			eb_ExposureCompensation->SetDouble(fmt, ExposureCompensation.val);
		}
	}
	else {
		cb_ExposureCompensation->disable();
		sl_ExposureCompensation->disable();
		eb_ExposureCompensation->disable();
	}



	double FrameRatePriorityExposureTimeMax = 1 / FrameRate.val * 1000 * 1000;


	CamPtr->GetNodeParameter(ExposureTime);
	if (ExposureTime.val > FrameRatePriorityExposureTimeMax) {
		ExposureTime.val = FrameRatePriorityExposureTimeMax;
	}
	if (ExposureTime.max > FrameRatePriorityExposureTimeMax) {
		ExposureTime.max = FrameRatePriorityExposureTimeMax;
	}
	if (ExposureTime.IsAuto) {
		cb_ExposureTime->Set(true);
		sl_ExposureTime->disable();
		eb_ExposureTime->disable();
	}
	else {
		cb_ExposureTime->Set(false);
		sl_ExposureTime->enable();
		eb_ExposureTime->enable();
	}
	sl_ExposureTime->Set(static_cast<int>(ExposureTime.val));
	sl_ExposureTime->SetMax(static_cast<int>(ExposureTime.max));
	sl_ExposureTime->SetMin(static_cast<int>(ExposureTime.min));

	if (eb_ExposureTime->hWnd != GetFocus()) {
		eb_ExposureTime->SetDouble(fmt, ExposureTime.val);
	}

	CamPtr->GetNodeParameter(Gain);
	if (Gain.IsAuto) {
		cb_Gain->Set(true);
		sl_Gain->disable();
		eb_Gain->disable();
	}
	else {
		cb_Gain->Set(false);
		sl_Gain->enable();
		eb_Gain->enable();
	}
	sl_Gain->Set(static_cast<int>(Gain.val));
	sl_Gain->SetMax(static_cast<int>(Gain.max));
	sl_Gain->SetMin(static_cast<int>(Gain.min));
	if (eb_Gain->hWnd != GetFocus()) {
		eb_Gain->SetDouble(fmt, Gain.val);
	}

	CamPtr->GetNodeParameter(BlackLevel);
	sl_BlackLevel->Set(static_cast<int>(BlackLevel.val));
	sl_BlackLevel->SetMax(static_cast<int>(BlackLevel.max));
	sl_BlackLevel->SetMin(static_cast<int>(BlackLevel.min));

	if (eb_BlackLevel->hWnd != GetFocus()) {
		eb_BlackLevel->SetDouble(fmt, BlackLevel.val);
	}
}
BOOL SpinnakerCamera::Parameters::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	NodeValues pv;
	wstring fmt = L"%6.2f";
	static bool busy = false;
	switch (message)
	{
	case WM_INITDIALOG:
		InitCtrl(hDlg);
		return (INT_PTR)TRUE;

	case WM_HSCROLL:
		if ((HWND)lParam == sl_FrameRate->hWnd) {
			FrameRate.val = static_cast<double>(sl_FrameRate->Get());
			CamPtr->SetNodeParameter(FrameRate);
		}
		if ((HWND)lParam == sl_ExposureCompensation->hWnd) {
			ExposureCompensation.val = static_cast<double>(sl_ExposureCompensation->Get());
			CamPtr->SetNodeParameter(ExposureCompensation);
		}
		if ((HWND)lParam == sl_ExposureTime->hWnd) {
			ExposureTime.val = static_cast<double>(sl_ExposureTime->Get());
			CamPtr->SetNodeParameter(ExposureTime);
		}
		if ((HWND)lParam == sl_Gain->hWnd) {
			Gain.val = static_cast<double>(sl_Gain->Get());
			CamPtr->SetNodeParameter(Gain);
		}
		if ((HWND)lParam == sl_BlackLevel->hWnd) {
			BlackLevel.val = static_cast<double>(sl_BlackLevel->Get());
			CamPtr->SetNodeParameter(BlackLevel);
		}
		return (INT_PTR)TRUE;
	case WM_TIMER:
		if (busy) return (INT_PTR)TRUE;
		busy = true;
		ShowParameters();


		int hist[256], max, i;
		CamPtr->CaptureImage(ci);

		int w, h;

		w = static_cast<int>(ci.w);
		h = static_cast<int>(ci.h);
		for (i = max = 0; i < 256; i++) {
			hist[i] = 0;
		}

		for (int i = 0; i < w * h; i++) {
			++hist[*(ci.img + i)];
		}

		for (int i = max = 0; i < 256; i++) {
			if (max < hist[i]) max = hist[i];
		}
		for (int i = 0; i < 256; i++) {
			hist[i] = (int)((double)hist[i] * 100.0 / (double)max);
		}
		bmp->copy(ci.img, static_cast<int>(ci.w), static_cast<int>(ci.h), static_cast<int>(ci.pixelbits));
		bmp->histogram(0, CAMPARAM_BMP_H, hist, RGB(128, 255, 128), SRCPAINT);
		bmp->refresh();

		busy = false;
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId) {
		case IDC_CHECK_FrameRateAuto:
			FrameRate.IsAuto = cb_FrameRate->Get();
			CamPtr->SetNodeParameter(FrameRate);
			ShowParameters();
			return (INT_PTR)true;
		case IDC_CHECK_ExposureCompensationAuto:
			ExposureCompensation.IsAuto = cb_ExposureCompensation->Get();
			CamPtr->SetNodeParameter(ExposureCompensation);
			ShowParameters();
			return (INT_PTR)true;

		case IDC_CHECK_ExposureTimeAuto:
			ExposureTime.IsAuto = cb_ExposureTime->Get();
			CamPtr->SetNodeParameter(ExposureTime);
			ShowParameters();
			return (INT_PTR)true;

		case IDC_CHECK_GainAuto:
			Gain.IsAuto = cb_Gain->Get();
			CamPtr->SetNodeParameter(Gain);
			ShowParameters();
			return (INT_PTR)true;
		case IDC_EDIT_FrameRate:
			if (wmEvent == EN_KILLFOCUS) {
				FrameRate.val = eb_FrameRate->GetDouble();
				CamPtr->SetNodeParameter(FrameRate);
				ShowParameters();
			}
			return true;
		case IDC_EDIT_ExposureCompensation:
			if (wmEvent == EN_KILLFOCUS) {
				ExposureCompensation.val = eb_ExposureCompensation->GetDouble();
				CamPtr->SetNodeParameter(ExposureCompensation);
				ShowParameters();
			}
			return true;
		case IDC_EDIT_ExposureTime:
			if (wmEvent == EN_KILLFOCUS) {
				ExposureTime.val = eb_ExposureTime->GetDouble();
				CamPtr->SetNodeParameter(ExposureTime);
				ShowParameters();
			}
			return true;
		case IDC_EDIT_Gain:
			if (wmEvent == EN_KILLFOCUS) {
				Gain.val = eb_Gain->GetDouble();
				CamPtr->SetNodeParameter(Gain);
				ShowParameters();
			}
			return true;
		case IDC_EDIT_BlackLevel:
			if (wmEvent == EN_KILLFOCUS) {
				BlackLevel.val = eb_BlackLevel->GetDouble();
				CamPtr->SetNodeParameter(BlackLevel);
				ShowParameters();
			}
			return true;
		case IDOK:
			if (eb_FrameRate->hWnd == GetFocus() ||
				eb_ExposureCompensation->hWnd == GetFocus() ||
				eb_ExposureTime->hWnd == GetFocus() ||
				eb_Gain->hWnd == GetFocus() ||
				eb_BlackLevel->hWnd == GetFocus()
				) {
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			EndCtrl(hDlg);
			EndDialog(hDlg, wmId);
			return (INT_PTR)true;
		case IDCANCEL:
			if (eb_FrameRate->hWnd == GetFocus()) {
				eb_FrameRate->SetDouble(fmt, FrameRate.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			if (eb_ExposureCompensation->hWnd == GetFocus()) {
				eb_ExposureCompensation->SetDouble(fmt, ExposureCompensation.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			if (eb_ExposureTime->hWnd == GetFocus()) {
				eb_ExposureTime->SetDouble(fmt, ExposureTime.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			if (eb_Gain->hWnd == GetFocus()) {
				eb_Gain->SetDouble(fmt, Gain.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			if (eb_BlackLevel->hWnd == GetFocus()) {
				eb_BlackLevel->SetDouble(fmt, BlackLevel.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			EndCtrl(hDlg);
			EndDialog(hDlg, wmId);

			return (INT_PTR)true;
		}
		return (INT_PTR)false;
	}
	return false;
}
INT_PTR CALLBACK  SpinnakerCamera::Parameters::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Parameters* thisPtr = NULL;

	switch (message) {
	case WM_INITDIALOG:
		thisPtr = (Parameters*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
		break;
	case WM_DESTROY:
		thisPtr = (Parameters*)GetWindowLongPtr(hDlg, DWLP_USER);
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)0);
		break;
	default:
		thisPtr = (Parameters*)GetWindowLongPtr(hDlg, DWLP_USER);
		break;
	}
	if (thisPtr) {
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}

bool SpinnakerCamera::DialogParameters(HWND hWnd)
{
	INT_PTR result = false;
	Parameters param;
	if (!IsReady()) return result;

	GetCameraParameter(CameraParam);
	result = param.Open(hWnd, *this);
	if (result == IDCANCEL) {
		SetCameraParameter(CameraParam);
	}
	GetCameraParameter(CameraParam);

	return result;
}


//Camera Class functions
bool SpinnakerCamera::SetCameraParameter(SPINNAKER_CAMERA_PARAM& p)
{
	if (!IsReady()) return false;

	bool result = false;
	bool IsAcquisition = false;

	try {
		if (IsStreaming()) {
			IsAcquisition = true;
			Stop();
		}
		SetTLSteamNodeEnumParameter("StreamBufferHandlingMode", p.StreamBufferHandlingMode.c_str());
		SetTLSteamNodeEnumParameter("StreamBufferCountMode", p.StreamBufferCountMode.c_str());
		SetTLSteamNodeIntParameter("StreamBufferCountManual", p.StreamBufferCount);

		SetNodeEnumParameter("AdcBitDepth", p.AdcBitDepth.c_str());

		SetNodeEnumParameter("PixelFormat", p.PixelFormat.c_str());

		SetNodeEnumParameter("BinningSelector", p.BinningSelector.c_str());

		SetNodeIntParameter("BinningHorizontal", p.BinningH);
		SetNodeIntParameter("BinningVertical", p.BinningV);
		SetWidthOffsetX(p.w, p.OffsetX);
		SetHeightOffsetY(p.h, p.OffsetY);

		SetNodeEnumParameter("AcquisitionMode", p.AcquisitionMode.c_str());
		SetNodeBoolParameter("AcquisitionFrameRateEnable", p.AcquisitionFrameRateEnable);



		SetNodeEnumParameter("pgrExposureCompensationAuto", p.pgrExposureCompensationAuto.c_str());
		SetNodeFloatParameter("pgrExposureCompensation", p.pgrExposureCompensation);

		SetNodeEnumParameter("ExposureMode", p.ExposureMode.c_str());
		SetNodeEnumParameter("ExposureAuto", p.ExposureAuto.c_str());
		SetNodeFloatParameter("ExposureTime", p.ExposureTime);

		SetNodeEnumParameter("GainAuto", p.GainAuto.c_str());
		SetNodeFloatParameter("Gain", p.Gain);


		SetNodeEnumParameter("BlackLevelSelector", p.BlackLevelSelector.c_str());
		SetNodeFloatParameter("BlackLevel", p.BlackLevel);

		SetNodeBoolParameter("GammaEnable", p.GammaEnable);
		SetNodeFloatParameter("Gamma", p.Gamma);

		SetNodeEnumParameter("AcquisitionFrameRateAuto", p.AcquisitionFrameRateAuto.c_str());
		SetNodeFloatParameter("AcquisitionFrameRate", p.AcquisitionFrameRate);

		if (IsAcquisition) {
			Start();
		}
	}

	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetCameraParameter");
		result = false;
	}
	return result;
}


bool SpinnakerCamera::GetCameraParameter(SPINNAKER_CAMERA_PARAM& p)
{
	if (!IsReady()) return false;

	bool result = false;
	bool IsAcquisition = false;
	INT64 imax, imin;
	double max, min;
	try {
		if (IsStreaming()) {
			IsAcquisition = true;
			Stop();
		}
		p.DeviceModelName = spCam->TLDevice.DeviceModelName.ToString();
		p.SerialNumber = spCam->TLDevice.DeviceSerialNumber.ToString();
		GetTLSteamNodeEnumParameter("StreamBufferHandlingMode", p.StreamBufferHandlingMode);
		GetTLSteamNodeEnumParameter("StreamBufferCountMode", p.StreamBufferCountMode);
		GetTLSteamNodeIntParameter("StreamBufferCountManual", p.StreamBufferCount);

		GetNodeEnumParameter("AdcBitDepth", p.AdcBitDepth);

		GetNodeEnumParameter("PixelFormat", p.PixelFormat);

		GetNodeEnumParameter("BinningSelector", p.BinningSelector);

		GetNodeIntParameter("BinningHorizontal", p.BinningH, imax, imin);
		GetNodeIntParameter("BinningVertical", p.BinningV, imax, imin);

		GetWidthOffsetX(p.w, p.OffsetX);
		GetHeightOffsetY(p.h, p.OffsetY);

		GetNodeEnumParameter("AcquisitionMode", p.AcquisitionMode);
		GetNodeBoolParameter("AcquisitionFrameRateEnable", p.AcquisitionFrameRateEnable);

		GetNodeEnumParameter("AcquisitionFrameRateAuto", p.AcquisitionFrameRateAuto);
		GetNodeFloatParameter("AcquisitionFrameRate", p.AcquisitionFrameRate, max, min);

		GetNodeEnumParameter("pgrExposureCompensationAuto", p.pgrExposureCompensationAuto);
		GetNodeFloatParameter("pgrExposureCompensation", p.pgrExposureCompensation, max, min);

		GetNodeEnumParameter("ExposureMode", p.ExposureMode);
		GetNodeEnumParameter("ExposureAuto", p.ExposureAuto);
		GetNodeFloatParameter("ExposureTime", p.ExposureTime, max, min);

		GetNodeEnumParameter("GainAuto", p.GainAuto);
		GetNodeFloatParameter("Gain", p.Gain, max, min);


		GetNodeEnumParameter("BlackLevelSelector", p.BlackLevelSelector);
		GetNodeFloatParameter("BlackLevel", p.BlackLevel, max, min);

		GetNodeBoolParameter("GammaEnable", p.GammaEnable);
		GetNodeFloatParameter("Gamma", p.Gamma, max, min);

		if (IsAcquisition) {
			Start();
		}
	}

	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SetCameraParameter");
		result = false;
	}
	return result;
}




bool SpinnakerCamera::Start()
{
	bool result = false;
	if (!IsReady()) return false;
	if (IsStreaming()) return true;

	try {
		spCam->BeginAcquisition();
		result = true;
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"Start");
		return false;
	}
	return result;
}
bool SpinnakerCamera::Stop()
{
	bool result = false;
	if (!IsReady()) return false;
	if (!IsStreaming()) return true;

	try {
		spCam->EndAcquisition();
		result = true;
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"Stop()");
		return false;
	}
	return result;
}
bool SpinnakerCamera::CaptureImage(CameraImage& ci, wstring wstr)
{
	try {
		
		int width, height, pbits, wh;

		ImagePtr  pResultImage = spCam->GetNextImage();

		QueryPerformanceCounter(&ci.CPU_FreqCount);
		ChunkData chunkData = pResultImage->GetChunkData();
		ci.FrameID = chunkData.GetFrameID();
		ci.TimeStamp = chunkData.GetTimestamp();
		width = static_cast<int>(pResultImage->GetWidth());
		height = static_cast<int>(pResultImage->GetHeight());
		pbits = static_cast<int>(pResultImage->GetBitsPerPixel());
		wh = width * height;
		if ((ci.w != width) || (ci.h != height) || (ci.pixelbits != pbits)) {
			ci.w = width;
			ci.h = height;
			ci.pixelbits = pbits;
			if (ci.img != NULL) {
				delete[] ci.img;
			}
			ci.img = new unsigned char[static_cast<size_t>(wh * pbits / 8)];
		}
		memcpy_s(ci.img, wh, static_cast<unsigned char*>(pResultImage->GetData()), wh);
		pResultImage->Release();
	}
	catch (Spinnaker::Exception& e) {
		wstringstream wss;
		wss << "CaptureImage from: " << wstr << endl;
		ErrorProc(e, wss.str());
		return false;
	}
	return true;
}
bool SpinnakerCamera::IsReady(void)
{
	if (spCam == nullptr) return false;
	return true;
}
bool SpinnakerCamera::IsStreaming(void)
{
	bool result = false;
	if (!IsReady()) return result;
	try {
		result = spCam->IsStreaming();
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"IsStreaming");
#ifdef CONWIN
		conwin.write(L"Error on IsStreaming()");
#endif
		result = false;
	}
	return result;
}
int SpinnakerCamera::MaxWidth()
{
	int result = 0;
	try {
		result = static_cast<int>(spCam->SensorWidth());
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"MaxWidth");
		return result;
	}
	return result;
}
int SpinnakerCamera::MaxHeight()
{
	int result = 0;
	try {
		result = static_cast<int>(spCam->SensorHeight());
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"MaxHeight");
		return result;
	}
	return result;
}
SpinnakerCamera::SpinnakerCamera()
{
	try {
		SpinnakerSystemPtr = System::GetInstance();
		SpinnakerCameraList = SpinnakerSystemPtr->GetCameras();
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"SpinnakerCamera");
	}
	spCam = nullptr;

}
SpinnakerCamera::~SpinnakerCamera()
{
	try {
		if (IsReady()) {
			if (IsStreaming()) {
				spCam->EndAcquisition();
			}
			if (spCam->IsInitialized()) {
				spCam->DeInit();
			}
			spCam = nullptr;
		}

		SpinnakerCameraList.Clear();
		SpinnakerSystemPtr->ReleaseInstance();
	}
	catch (Spinnaker::Exception& e) {
		ErrorProc(e, L"~SpinnakerCamera");
	}
}



