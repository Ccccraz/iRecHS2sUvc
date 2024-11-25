#include "UvcCamera.h"
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

#pragma region util

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

#pragma endregion util

#pragma region CameraImage

CameraImage::CameraImage()
{
	CPU_FreqCount.QuadPart = 0;
}

CameraImage::~CameraImage()
{
	delete[] img;
}

#pragma endregion CameraImage

#pragma region ConsoleWindow

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

void ConsoleWindow::write(const std::wstring& ws) const
{
	DWORD dwWriteByte;
	WriteConsole(hStdOutput, ws.c_str(), static_cast<DWORD>(ws.size()), &dwWriteByte, nullptr);
}

#pragma endregion ConsoleWindow

#pragma region UvcCamera

UvcCamera::UvcCamera()
{
	uvc_init(&ctx, nullptr);

	const auto res = uvc_get_device_list(ctx, &uvc_cam_list);
	if (res != UVC_SUCCESS) ErrorProc(res, L"UvcCamera");

	for (auto i = 0; uvc_cam_list[i] != nullptr; i++)
	{
		uvc_cam_list_size++;
	}

	uvc_cam = nullptr;
}

UvcCamera::~UvcCamera()
{
	if (uvc_cam_handle != nullptr)
	{
		/* Release our handle on the device */
		uvc_close(uvc_cam_handle);
		uvc_unref_device(uvc_cam);

		uvc_exit(ctx);
	}
}

// TODO: working need review
/// <summary>
/// Open camera
/// </summary>
/// <param name="no"></param>
/// <returns></returns>
auto UvcCamera::SetCamera(const unsigned int no) -> bool
{
	if (no >= uvc_cam_list_size) return false;

	if (IsReady())
	{
		if (uvc_cam_handle != nullptr) uvc_close(uvc_cam_handle);
		//uvc_unref_device(uvc_cam);
		uvc_cam = nullptr;
		uvc_cam_handle = nullptr;
	}

	uvc_cam = uvc_cam_list[no];

	auto res = uvc_open(uvc_cam, &uvc_cam_handle, should_detach_kernel_driver);
	if (res != UVC_SUCCESS) ErrorProc(res, L"SetCamera open camera");

	res = uvc_get_device_descriptor(uvc_cam, &uvc_cam_desc);
	if (res != UVC_SUCCESS) ErrorProc(res, L"SetCamera get device info");


	GetCameraParameter(CameraParam);
	uvc_cam_serial = std::string(uvc_cam_desc->serialNumber);
	CameraParam.w = MaxWidth();
	CameraParam.h = MaxHeight();

	// TODO: Implement
	CameraParam.ExposureMode = std::string("Timed");
	CameraParam.ExposureAuto = string("On");
	CameraParam.ExposureTime = 1000;

	CameraParam.BinningSelector = std::string("All");
	CameraParam.BinningH = 1;
	CameraParam.BinningV = 1;
	CameraParam.OffsetX = 0;
	CameraParam.OffsetY = 0;

	CameraParam.GainAuto = string("Off");;
	CameraParam.Gain = 0;
	CameraParam.BlackLevelSelector = string("All");
	CameraParam.BlackLevel = 0;

	CameraParam.GammaEnable = true;
	CameraParam.Gamma = 1;

	// uvc camera doesn't have these properties
	// so hardcode some fake values
	CameraParam.StreamBufferHandlingMode = std::string("NewestOnly");
	CameraParam.StreamBufferCountMode = std::string("Manual");
	CameraParam.StreamBufferCount = 10;
	CameraParam.AdcBitDepth = std::string("Bit8");
	CameraParam.PixelFormat = std::string("Mono8");
	CameraParam.AcquisitionMode = std::string("Continuous");
	CameraParam.AcquisitionFrameRateEnable = true;
	CameraParam.AcquisitionFrameRateAuto = std::string("Continuous");
	CameraParam.AcquisitionFrameRate = 30.0;
	CameraParam.pgrExposureCompensationAuto = std::string("Continuous");
	CameraParam.pgrExposureCompensation = 0;

	SetCameraParameter(CameraParam);

	Start();

	return true;
}

// TODO: Implement
auto UvcCamera::SetCamera(std::string serial) -> bool
{
	return true;
}

// TODO: wait for implementation
auto UvcCamera::SetFullScreen() -> bool
{
	if (!IsReady()) return false;
	UVC_CAMERA_PARAM p;

	// ------Properties that uvc camera doesn't have------//
	// -------------------Keep default--------------------//
	p.StreamBufferCount = 3;

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

	p.AdcBitDepth = string("Bit8");

	p.AcquisitionMode = std::string("Continuous");
	p.PixelFormat = string("Mono8");
	// -------------------Keep default--------------------//
	// ------Properties that uvc camera doesn't have------//

	p.BinningSelector = string("All");
	p.BinningH = 1;
	p.BinningV = 1;
	p.OffsetX = 0;
	p.OffsetY = 0;
	p.w = this->MaxWidth();
	p.h = this->MaxHeight();


	return SetCameraParameter(p);
}

// TODO: Implement
auto UvcCamera::SetCameraParameter(UVC_CAMERA_PARAM& p) -> bool
{
	return true;
}

// TODO: Implement
auto UvcCamera::GetCameraParameter(UVC_CAMERA_PARAM& p) -> bool
{
	return true;
}

// FINISHED
/// <summary>
/// Check if the camera is existing
/// </summary>
/// <returns></returns>
auto UvcCamera::IsReady() const -> bool
{
	return uvc_cam != nullptr;
}

// TODO: Optimize startup flow
// TODO: Decide whether to retain IsOpen as a separate judgment condition or merge with IsReady
auto UvcCamera::IsOpen() const -> bool
{
	return uvc_cam_handle != nullptr;
}


// TODO: working need review
// TODO: Convert mjpeg to gray8
// TODO: Confirm timestamp format
// TODO: Confirm pixel format
// TODO: Convert mjpeg to gray8
/// <summary>
/// Capture an image from the camera
/// </summary>
/// <param name="ci"></param>
/// <param name="wstr"></param>
/// <returns></returns>
auto UvcCamera::CaptureImage(CameraImage& ci, const std::wstring& wstr) const -> bool
{
	using nanoseconds = std::chrono::nanoseconds;
	using steady_clock = std::chrono::steady_clock;

	uvc_frame_t* frame_raw{ nullptr };

	// try to get a frame from the stream and store it in frame_raw. never timeout.
	const auto res = uvc_stream_get_frame(stream_handle, &frame_raw, 0);

	// if the frame is not valid, pop up an error message and return false
	if (res != UVC_SUCCESS)
	{
		std::wstringstream wss;
		wss << "CaptureImage from: " << wstr << '\n';
		ErrorProc(res, wss.str());
		return false;
	}

	// Get high-precision system timestamp
	QueryPerformanceCounter(&ci.CPU_FreqCount);

	// TODO: confirm the timestamp unit is nanoseconds currently use std::chrono library to get the current timestamp as a substitute
	// TODO: try to get timestamp from frame_raw
	ci.TimeStamp = std::chrono::duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
	ci.FrameID = frame_raw->sequence;

	std::unique_ptr<unsigned char[]> frame_gray{};
	int width{};
	int height{};

	// convert mjpeg to gray8
	const auto res_gray = Mjpeg2Gray8(frame_raw, frame_gray, width, height);
	if (!res_gray)
	{
		ErrorProc(UVC_ERROR_OTHER, L"Convert mjpeg to gray8 failed");
	}

	const auto wh = width * height;

	/// TODO: confirm pixel format
	constexpr auto pbits = 8;

	if ((ci.w != width) || (ci.h != height) || (ci.pixelbits != pbits))
	{
		ci.w = width;
		ci.h = height;
		ci.pixelbits = pbits;

		delete[] ci.img;

		ci.img = new unsigned char[static_cast<size_t>(wh * pbits / 8)];
	}

	memcpy_s(ci.img, wh, frame_gray.get(), wh);

	return true;
}

// TODO: working need review
// TODO: clear IsReady state and IsOpen state
/// <summary>
/// Check if the camera is streaming
/// </summary>
/// <returns></returns>
auto UvcCamera::IsStreaming() const -> bool
{
	return IsReady() && stream_handle != nullptr;
}

// TODO: working need review
/// <summary>
/// create a stream handle and start streaming
/// </summary>
/// <returns></returns>
auto UvcCamera::Start() -> bool
{
	if (!IsReady()) return false;
	if (!IsOpen()) return false;
	if (IsStreaming()) return true;

	// Get the stream control
	const auto frame_desc = uvc_get_format_descs(uvc_cam_handle)->frame_descs;

	constexpr auto frame_format{ UVC_COLOR_FORMAT_MJPEG };
	const uint16_t width{ static_cast<uint16_t>(MaxWidth()) };
	const uint16_t height{ static_cast<uint16_t>(MaxHeight()) };
	constexpr uint16_t fps{ 120 };

	auto res = uvc_get_stream_ctrl_format_size(uvc_cam_handle, &ctrl, frame_format, width, height, fps,
		should_detach_kernel_driver);
	if (res != UVC_SUCCESS) return false;

	// Create a stream handle
	res = uvc_stream_open_ctrl(uvc_cam_handle, &stream_handle, &ctrl, should_detach_kernel_driver);
	if (res != UVC_SUCCESS) return false;

	// Start the stream, without callback
	res = uvc_stream_start(stream_handle, nullptr, nullptr, 2.0, 0);
	if (res != UVC_SUCCESS) return false;

	return true;
}

// FINISHED
/// <summary>
/// Stop streaming but keep the camera open
/// </summary>
/// <returns></returns>
auto UvcCamera::Stop() const -> bool
{
	if (!IsReady()) return false;

	if (!IsStreaming()) return true;

	uvc_stream_stop(stream_handle);
	uvc_stop_streaming(uvc_cam_handle);

	return true;
}

// TODO: working need review
/// <summary>
/// Get maximum width of the frame
/// </summary>
/// <returns></returns>
auto UvcCamera::MaxWidth() const -> int
{
	if (IsOpen()) return 320;
	const auto format_desc = uvc_get_format_descs(uvc_cam_handle);
	return format_desc->frame_descs->wWidth;
}

// TODO: working need review
/// <summary>
/// Get maximum height of the frame
/// </summary>
/// <returns></returns>
auto UvcCamera::MaxHeight() const -> int
{
	if (IsOpen()) return 240;
	const auto format_desc = uvc_get_format_descs(uvc_cam_handle);
	return format_desc->frame_descs->wHeight;
}

/// <summary>
/// Convert MJPEG to grayscale
/// </summary>
/// <param name="frame"> uvc_frame_t* raw mjpeg frame </param>
/// <param name="frame_gray"> buffer to store grayscale frame </param>
/// <param name="width"> width of the frame </param>
/// <param name="height"> height of the frame</param>
/// <returns> true if successful </returns>
auto UvcCamera::Mjpeg2Gray8(const uvc_frame_t* frame, std::unique_ptr<unsigned char[]>& frame_gray, int& width,
	int& height) -> bool
{
	const tjhandle handle = tjInitDecompress();
	if (!handle)
	{
		std::cerr << "Error initializing TurboJPEG decompressor: " << tjGetErrorStr() << '\n';
		return false;
	}

	int jpeg_subsamp{};
	int jpeg_colorspace{};

	// Get the dimensions of the image.
	if (tjDecompressHeader3(handle, static_cast<const unsigned char*>(frame->data), frame->data_bytes, &width, &height,
		&jpeg_subsamp, &jpeg_colorspace) != 0)
	{
		std::cerr << "Error reading JPEG header: " << tjGetErrorStr2(handle) << '\n';
		tjDestroy(handle);
		return false;
	}

	// Allocate memory for the output buffer.
	const int pixel_count = width * height;
	frame_gray = std::make_unique<unsigned char[]>(pixel_count);

	// Decompress the JPEG image to grayscale.
	if (tjDecompress2(handle, static_cast<const unsigned char*>(frame->data), frame->data_bytes, frame_gray.get(),
		width, 0, height, TJPF_GRAY, 0) != 0)
	{
		std::cerr << "Decode failed: " << tjGetErrorStr2(handle) << '\n';
		tjDestroy(handle);
		return false;
	}

	tjDestroy(handle);
	return true;
}

// TODO: compatible with more error types
/// <summary>
/// Create a message box with the error message
/// </summary>
/// <param name="err"> uvc error code </param>
/// <param name="msg"> additional message </param>
void UvcCamera::ErrorProc(const uvc_error_t err, const std::wstring& msg)
{
	std::wstringstream wss;
	const auto err_msg = uvc_strerror(err);

	wss << "Error: " << err_msg << '\n';

	if (!msg.empty())
	{
		wss << "Addition:  " << msg << '\n';
	}

	MessageBox(nullptr, (wss.str()).c_str(), TEXT("UVC Camera Error Message"), MB_OK);
}

#pragma endregion UvcCamera

#pragma region CameraSelectDialog
// Camera Select Dialog
UvcCamera::DlgSelect::DlgSelect()
{
	lb.reset();
	CameraListVector.clear();
}

UvcCamera::DlgSelect::~DlgSelect()
{
	CameraListVector.clear();
}

// TODO: working need review
/// <summary>
///  Enumerate all UVC cameras and add to the list
/// </summary>
/// <returns></returns>
auto UvcCamera::DlgSelect::SetCameraList() -> bool
{
	UVC_CAMERA_DATA camera_data{}; // camera data

	// Initialize libuvc
	uvc_context_t* uvc_context{ nullptr };
	uvc_device_t** devs{ nullptr };

	// clear camera list
	CameraListVector.clear();

	auto result = false;

	// Initialize libuvc and get all devices
	auto res = uvc_init(&uvc_context, nullptr);

	if (res != UVC_SUCCESS) return false;

	res = uvc_get_device_list(uvc_context, &devs);

	if (res != UVC_SUCCESS) return false;

	for (auto i = 0; devs[i] != nullptr; i++)
	{
		camera_data.index = i;
		uvc_device_descriptor_t* desc;
		uvc_get_device_descriptor(devs[i], &desc);
		std::string serial_number = desc->serialNumber;
		camera_data.SerialNumber = utf8_decode(serial_number);

		// Uvc camera doesn't exist this property
		std::string device_mode = "Null";
		camera_data.DeviceModelName = utf8_decode(device_mode);

		// Use product name as link speed
		std::string product = desc->product;
		camera_data.DeviceCurrentSpeed = utf8_decode(product);
		CameraListVector.push_back(camera_data);

		// release the descriptor
		uvc_free_device_descriptor(desc);
		result = true;
	}

	// release the device list and context
	uvc_free_device_list(devs, 1);
	uvc_exit(uvc_context);
	return result;
}

// TODO: working need review
// TODO: use static_cast instead of c_style cast
/// <summary>
/// Open a dialog to select a camera
/// </summary>
/// <param name="hWnd"></param>
/// <param name="pcam"></param>
/// <returns></returns>
INT_PTR UvcCamera::DlgSelect::Open(HWND hWnd, UvcCamera& pcam)
{
	hWndParent = hWnd;
	CamPtr = &pcam;

	// Check if the camera list is empty
	if (!SetCameraList())
	{
		return (INT_PTR)false;
	}

	// show the dialog
	const auto iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CAMERA_LIST), hWndParent, CallbackProc, (LPARAM)this);
	return iptr;
}

// TODO: working need review
// TODO: create a new dialog structure for uvc camera
/// <summary>
/// Init the dialog controls
/// </summary>
/// <param name="hDlg"></param>
/// <returns></returns>
auto UvcCamera::DlgSelect::InitCtrl(HWND hDlg) -> bool
{
	std::wstringstream wss;
	lb.reset(new DlgListBox(hDlg, IDC_CAMERA_LIST));
	for (auto vcl : CameraListVector)
	{
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

// FINISHED
/// <summary>
/// Clean up the dialog controls
/// </summary>
/// <param name="hDlg"></param>
/// <returns></returns>
auto UvcCamera::DlgSelect::EndCtrl(HWND hDlg) -> void
{
	lb.reset();
}

// TODO: working need review
// TODO: use static_cast instead of c_style cast
/// <summary>
/// Process data from the dialog
/// </summary>
/// <param name="hDlg"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
auto UvcCamera::DlgSelect::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> bool
{
	int wmId;
	int wmEvent;

	switch (message)
	{
	case WM_INITDIALOG:
		// Initialize dialog controls
		if (InitCtrl(hDlg)) return (INT_PTR)true;

		// Close dialog if initialization failed
		EndDialog(hDlg, false);
		return (INT_PTR)true;

	case WM_COMMAND:
		// Get control id and event
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case IDC_CAMERA_LIST:
			// Get selected camera index
			camIndex = lb->Get();
			return (INT_PTR)true;
		case IDOK:
			// Open the selected camera
			CamPtr->SetCamera(camIndex);

			// Close the dialog
			EndCtrl(hDlg);
			EndDialog(hDlg, true);
			return (INT_PTR)true;
		case IDCANCEL:
			EndCtrl(hDlg);
			EndDialog(hDlg, false);
			return (INT_PTR)true;
		default:;
		}
		return (INT_PTR)false;
	default:;
	}
	return false;
}

// TODO: working need review
// TODO: use static_cast instead of c_style cast
/// <summary>
/// Init a camera selection dialog
/// </summary>
/// <param name="hDlg"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
auto UvcCamera::DlgSelect::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR
{
	DlgSelect* thisPtr{ nullptr };

	switch (message)
	{
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
	if (thisPtr)
	{
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}

// FINISHED
/// <summary>
/// Open a camera selection dialog
/// </summary>
/// <param name="hWnd"></param>
/// <returns></returns>
auto UvcCamera::DialogSelect(HWND hWnd) -> bool
{
	DlgSelect ds;
	return ds.Open(hWnd, *this);
}

#pragma endregion CameraSelectDialog

#pragma region InputRegionDialog

/// <summary>
/// Calculate the input region offset
/// </summary>
void UvcCamera::InputRegion::SetInputOffset()
{
	int lx, ly, rx, ry;

	// offset in left and top
	lx = (start.x - BMP_OFFSETX) * ReductionRatio - w.min / 2;
	ly = (start.y - BMP_OFFSETY) * ReductionRatio - h.min / 2;

	lx = lx - lx % offx.inc;
	if (lx < offx.min) lx = offx.min;
	if (lx > offx.max) lx = offx.max;

	ly = ly - ly % offy.inc;
	if (ly < offy.min) ly = offy.min;
	if (ly > offy.max) ly = offy.max;


	// offset in right and bottom
	rx = lx + w.min;
	if (rx >= w.max) rx = w.max;

	ry = ly + h.min;
	if (ry >= h.max) ry = h.max;


	// set the offset
	offx.val = lx;
	offy.val = ly;
	w.val = rx - lx;
	h.val = ry - ly;

	// update the dialog
	sp_offsetx->Set(offx.val);
	sp_offsety->Set(offy.val);
	sp_width->Set(w.val);
	sp_height->Set(w.val);
}

/// <summary>
/// Set size of the input region
/// </summary>
void UvcCamera::InputRegion::SetInputSize()
{
	int lx, ly, rx, ry;
	int flag = 1;

	// Determine if the starting x coordinate is less than the ending x coordinate
	if (start.x < end.x)
	{
		lx = (start.x - BMP_OFFSETX) * ReductionRatio - w.val / 2;
		rx = (end.x - BMP_OFFSETX) * ReductionRatio;
	}
	else
	{
		flag = flag * -1;
		rx = (start.x - BMP_OFFSETX) * ReductionRatio + w.val / 2;
		lx = (end.x - BMP_OFFSETX) * ReductionRatio;
	}
	if (start.y < end.y)
	{
		ly = (start.y - BMP_OFFSETY) * ReductionRatio - h.val / 2;
		ry = (end.y - BMP_OFFSETY) * ReductionRatio;
	}
	else
	{
		flag = flag * -1;
		ry = (start.y - BMP_OFFSETY) * ReductionRatio + h.val / 2;
		ly = (end.y - BMP_OFFSETY) * ReductionRatio;
	}
	if (flag == 1)
	{
		SetCursor(hCursorSIZENWSE);
	}
	else
	{
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

	if ((lx + w.val) > w.max)
	{
		w.val = w.max - lx;
	}
	if ((ly + h.val) > h.max)
	{
		h.val = h.max - ly;
	}
	offx.val = lx;
	offy.val = ly;
	sp_offsetx->Set(offx.val);
	sp_offsety->Set(offy.val);
	sp_width->Set(w.val);
	sp_height->Set(h.val);
}

/// <summary>
/// Set the center of the input region
/// </summary>
void UvcCamera::InputRegion::SetInputCenter()
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
	if (rx >= w.max)
	{
		rx = w.max;
		lx = rx - w.val;
	}


	ry = ly + h.val;
	if (ry >= h.max)
	{
		ry = h.max;
		ly = ry - h.val;
	}


	SetCursor(hCursorSIZEALL);
	offx.val = lx;
	offy.val = ly;
	sp_offsetx->Set(offx.val);
	sp_offsety->Set(offy.val);
}

/// <summary>
/// Set the input region offset in horizontal direction
/// </summary>
void UvcCamera::InputRegion::SetOffsetX()
{
	offx.val = sp_offsetx->Get();
	w.val = sp_width->Get();
	if ((offx.val + w.val) > w.max)
	{
		w.val = w.max - offx.val;
		sp_width->Set(w.val);
	}
}

/// <summary>
/// Set the input region offset in vertical direction
/// </summary>
void UvcCamera::InputRegion::SetOffsetY()
{
	offy.val = sp_offsety->Get();
	h.val = sp_height->Get();
	if ((offy.val + h.val) > h.max)
	{
		h.val = h.max - offy.val;
		sp_height->Set(h.val);
	}
}

/// <summary>
/// Set width of the input region
/// </summary>
void UvcCamera::InputRegion::SetWidth()
{
	offx.val = sp_offsetx->Get();
	w.val = sp_width->Get();

	if ((offx.val + w.val) > w.max)
	{
		offx.val = w.max - w.val;
		if (offx.val > offx.min)
		{
			sp_offsetx->Set(offx.val);
		}
		else
		{
			offx.val = offx.min;
			w.val = w.max - offx.min;
			sp_offsetx->Set(offx.val);
			sp_width->Set(w.val);
		}
	}
}

/// <summary>
/// Set height of the input region
/// </summary>
void UvcCamera::InputRegion::SetHeight()
{
	offy.val = sp_offsety->Get();
	h.val = sp_height->Get();

	if ((offy.val + h.val) > h.max)
	{
		offy.val = h.max - h.val;
		if (offy.val > offy.min)
		{
			sp_offsety->Set(offy.val);
		}
		else
		{
			offy.val = offy.min;
			h.val = h.max - offy.min;
			sp_offsety->Set(offy.val);
			sp_height->Set(h.val);
		}
	}
}

UvcCamera::InputRegion::InputRegion()
{
	CamPtr = nullptr;
	hWndParent = nullptr;
	hCursorSIZEALL = LoadCursor(nullptr, IDC_SIZEALL);
	hCursorSIZENESW = LoadCursor(nullptr, IDC_SIZENESW);
	hCursorSIZENWSE = LoadCursor(nullptr, IDC_SIZENWSE);
	hCursorArrow = LoadCursor(nullptr, IDC_ARROW);
	start = { 0, 0 };
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
	CameraImage ci{};
}

UvcCamera::InputRegion::~InputRegion()
= default;

/// <summary>
/// Initialize the input region controls
/// </summary>
/// <param name="hDlg"></param>
void UvcCamera::InputRegion::InitCtrl(HWND hDlg)
{
	RECT rc;
	std::wstring ws;

	GetWindowRect(hDlg, &rc);
	rc.right = rc.left + (LONG)w.max / ReductionRatio;
	rc.bottom = rc.top + (LONG)h.max / ReductionRatio + BMP_OFFSETY;
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	if (rc.top < 0)
	{
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

/// <summary>
/// Clears all the controls
/// </summary>
/// <param name="hDlg">¶Ô»°¿ò¾ä±ú</param>
void UvcCamera::InputRegion::EndCtrl(HWND hDlg)
{
	KillTimer(hDlg, ID_CAMREGION_TIMER);
}

// TODO:
INT_PTR UvcCamera::InputRegion::Open(HWND hWnd, UvcCamera& pcam)
{
	INT_PTR iptr;
	hWndParent = hWnd;
	CamPtr = &pcam;
	if (!CamPtr->IsReady()) return false;
	if (!CamPtr->IsStreaming()) CamPtr->Start();

	w.max = static_cast<int>(320);
	w.min = static_cast<int>(320);
	w.inc = static_cast<int>(1);
	w.val = CamPtr->CameraParam.w;


	h.max = static_cast<int>(240);
	h.min = static_cast<int>(240);
	h.inc = static_cast<int>(1);
	h.val = CamPtr->CameraParam.h;


	offx.inc = static_cast<int>(1);
	offx.min = static_cast<int>(0);
	offx.max = w.max - w.min;
	offx.val = CamPtr->CameraParam.OffsetX;

	offy.inc = static_cast<int>(1);
	offy.min = static_cast<int>(0);
	offy.max = h.max - h.min;
	offy.val = CamPtr->CameraParam.OffsetY;

	ReductionRatio = static_cast<int>(ceil(static_cast<double>(w.max) / 1000.0));

	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CAM_REGION), hWndParent, CallbackProc, (LPARAM)this);
	return iptr;
}

BOOL UvcCamera::InputRegion::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
		if (LButtonDown)
		{
			return (INT_PTR)TRUE;
		}
		start = end = MAKEPOINTS(lParam);
		if ((start.x > BMP_OFFSETX) && (start.y > BMP_OFFSETY))
		{
			RButtonDown = true;
			SetCursor(hCursorSIZENWSE);
			SetInputOffset();

			SetCapture(hDlg);
		}
		return (INT_PTR)TRUE;
	case WM_MOUSEMOVE:
		if (RButtonDown)
		{
			end = MAKEPOINTS(lParam);
			SetInputSize();
		}
		if (LButtonDown)
		{
			start = MAKEPOINTS(lParam);
			SetInputCenter();
		}
		return (INT_PTR)TRUE;
	case WM_RBUTTONUP:
		if (RButtonDown)
		{
			RButtonDown = false;
			SetCursor(hCursorArrow);
			ReleaseCapture();
		}
		return (INT_PTR)TRUE;
	case WM_LBUTTONDOWN:
		if (RButtonDown)
		{
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
		if ((HWND)lParam == sp_offsetx->hWnd && LOWORD(wParam) == SB_THUMBPOSITION)
		{
			SetOffsetX();
		}

		if ((HWND)lParam == sp_offsety->hWnd && LOWORD(wParam) == SB_THUMBPOSITION)
		{
			SetOffsetY();
		}

		if ((HWND)lParam == sp_width->hWnd && LOWORD(wParam) == SB_THUMBPOSITION)
		{
			SetWidth();
		}

		if ((HWND)lParam == sp_height->hWnd && LOWORD(wParam) == SB_THUMBPOSITION)
		{
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
		switch (wmId)
		{
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

INT_PTR UvcCamera::InputRegion::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	InputRegion* thisPtr = NULL;

	switch (message)
	{
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
	if (thisPtr)
	{
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}

auto UvcCamera::DialogRegion(HWND hWnd) -> bool
{
	bool result = false;
	InputRegion ir;
	if (!IsReady()) return result;
	GetCameraParameter(CameraParam);
	if (hWnd == NULL)
	{
		if (CameraParam.w > INIT_INPUT_W)
		{
			CameraParam.w = INIT_INPUT_W;
		}
		if (CameraParam.h > INIT_INPUT_H)
		{
			CameraParam.h = INIT_INPUT_H;
		}
	}
	// TODO:
	SetFullScreen();
	result = ir.Open(hWnd, *this);
	SetCameraParameter(CameraParam);
	return result;
}


#pragma endregion InputRegionDialog

#pragma region ParametersDialog

#define CAMPARAM_BMP_OFFSETX 450
#define CAMPARAM_BMP_OFFSETY 0
#define CAMPARAM_BMP_W 512
#define CAMPARAM_BMP_H 480

UvcCamera::Parameters::Parameters()
{
	CamPtr = nullptr;
	hWndParent = nullptr;

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

UvcCamera::Parameters::~Parameters()
= default;

/// TODO:
void UvcCamera::Parameters::ShowParameters()
{
	NodeValues fr;
	wstring fmt = L"%6.2f";

	cb_FrameRate->Set(true);
	sl_FrameRate->disable();
	eb_FrameRate->disable();
	sl_FrameRate->Set(static_cast<int>(120));
	sl_FrameRate->SetMax(120);
	sl_FrameRate->SetMin(30);

	if (eb_FrameRate->hWnd != GetFocus())
	{
		eb_FrameRate->SetDouble(fmt, FrameRate.val);
	}


	cb_ExposureCompensation->Set(true);
	sl_ExposureCompensation->disable();
	eb_ExposureCompensation->disable();

	sl_ExposureCompensation->Set(100);


	sl_ExposureCompensation->SetMax(100);
	sl_ExposureCompensation->SetMin(0);

	cb_ExposureCompensation->disable();
	sl_ExposureCompensation->disable();
	eb_ExposureCompensation->disable();

	double FrameRatePriorityExposureTimeMax = 1 / FrameRate.val * 1000 * 1000;

	ExposureTime.val = 100;
	ExposureTime.max = 100;
	cb_ExposureTime->Set(true);
	sl_ExposureTime->disable();
	eb_ExposureTime->disable();

	sl_ExposureTime->Set(20);
	sl_ExposureTime->SetMax(100);
	sl_ExposureTime->SetMin(0);

	if (eb_ExposureTime->hWnd != GetFocus())
	{
		eb_ExposureTime->SetDouble(fmt, ExposureTime.val);
	}

	cb_Gain->Set(true);
	sl_Gain->disable();
	eb_Gain->disable();

	sl_Gain->Set(20);
	sl_Gain->SetMax(100);
	sl_Gain->SetMin(0);
	if (eb_Gain->hWnd != GetFocus())
	{
		eb_Gain->SetDouble(fmt, Gain.val);
	}

	sl_BlackLevel->Set(20);
	sl_BlackLevel->SetMax(100);
	sl_BlackLevel->SetMin(0);

	if (eb_BlackLevel->hWnd != GetFocus())
	{
		eb_BlackLevel->SetDouble(fmt, BlackLevel.val);
	}
}

/// <summary>
/// Initialize the camera options control dialog
/// </summary>
/// <param name="hDlg"></param>
void UvcCamera::Parameters::InitCtrl(HWND hDlg)
{
	NodeValues pv;
	RECT rc;
	LONG cam_w, cam_h, cam_x, cam_y;

	// Frame component
	cb_FrameRate.reset(new DlgCheckbox(hDlg, IDC_CHECK_FrameRateAuto));
	sl_FrameRate.reset(new DlgSlider(hDlg, IDC_SLIDER_FrameRate, 0, 100, 0, 100, 1));
	eb_FrameRate.reset(new DlgEditBox(hDlg, IDC_EDIT_FrameRate));

	// Exposure Compensation component
	cb_ExposureCompensation.reset(new DlgCheckbox(hDlg, IDC_CHECK_ExposureCompensationAuto));
	sl_ExposureCompensation.reset(new DlgSlider(hDlg, IDC_SLIDER_ExposureCompensation, 0, 10, 0, 1, 1));
	eb_ExposureCompensation.reset(new DlgEditBox(hDlg, IDC_EDIT_ExposureCompensation));

	// Exposure Time component
	cb_ExposureTime.reset(new DlgCheckbox(hDlg, IDC_CHECK_ExposureTimeAuto));
	sl_ExposureTime.reset(new DlgSlider(hDlg, IDC_SLIDER_ExposureTime, 0, 100, 0, 100, 1));
	eb_ExposureTime.reset(new DlgEditBox(hDlg, IDC_EDIT_ExposureTime));

	// Gain component
	cb_Gain.reset(new DlgCheckbox(hDlg, IDC_CHECK_GainAuto));
	sl_Gain.reset(new DlgSlider(hDlg, IDC_SLIDER_Gain, 0, 100, 0, 10, 1));
	eb_Gain.reset(new DlgEditBox(hDlg, IDC_EDIT_Gain));

	// BlackLevel component
	sl_BlackLevel.reset(new DlgSlider(hDlg, IDC_SLIDER_BlackLevel, 0, 10, 0, 1, 2));
	eb_BlackLevel.reset(new DlgEditBox(hDlg, IDC_EDIT_BlackLevel));

	// Ok button
	btn_OK.reset(new DlgButton(hDlg, IDOK));

	// Get camera parameters
	/// TODO: Get camera parameters from camera
	cam_w = CamPtr->CameraParam.w;
	cam_h = CamPtr->CameraParam.h;
	cam_x = CAMPARAM_BMP_OFFSETX;
	cam_y = CAMPARAM_BMP_OFFSETY;

	// adjust width according to camera height
	cam_w = (LONG)((double)cam_w * (double)CAMPARAM_BMP_H / (double)cam_h);
	cam_h = CAMPARAM_BMP_H;

	GetWindowRect(hDlg, &rc);
	rc.right = rc.left + CAMPARAM_BMP_OFFSETX + cam_w;
	rc.bottom = rc.top + CAMPARAM_BMP_OFFSETY + cam_h;
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	if (rc.top < 0)
	{
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


/// <summary>
/// close the camera options control dialog
/// </summary>
/// <param name="hDlg"></param>
void UvcCamera::Parameters::EndCtrl(HWND hDlg)
{
	KillTimer(hDlg, ID_CAMPARAM_TIMER);
}


/// <summary>
/// Open the camera options control dialog
/// </summary>
/// <param name="hWnd"></param>
/// <param name="pcam"></param>
/// <returns></returns>
INT_PTR UvcCamera::Parameters::Open(HWND hWnd, UvcCamera& pcam)
{
	INT_PTR iptr;
	CamPtr = &pcam;

	hWndParent = hWnd;
	iptr = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_CameraParameter), hWndParent, CallbackProc, (LPARAM)this);

	return iptr;
}

/// <summary>
/// TODO: Wait for implementation
/// </summary>
/// <param name="hDlg"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
BOOL UvcCamera::Parameters::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
		if ((HWND)lParam == sl_FrameRate->hWnd)
		{
			FrameRate.val = static_cast<double>(sl_FrameRate->Get());
		}
		if ((HWND)lParam == sl_ExposureCompensation->hWnd)
		{
			ExposureCompensation.val = static_cast<double>(sl_ExposureCompensation->Get());
		}
		if ((HWND)lParam == sl_ExposureTime->hWnd)
		{
			ExposureTime.val = static_cast<double>(sl_ExposureTime->Get());
		}
		if ((HWND)lParam == sl_Gain->hWnd)
		{
			Gain.val = static_cast<double>(sl_Gain->Get());
		}
		if ((HWND)lParam == sl_BlackLevel->hWnd)
		{
			BlackLevel.val = static_cast<double>(sl_BlackLevel->Get());
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
		for (i = max = 0; i < 256; i++)
		{
			hist[i] = 0;
		}

		for (int i = 0; i < w * h; i++)
		{
			++hist[*(ci.img + i)];
		}

		for (int i = max = 0; i < 256; i++)
		{
			if (max < hist[i]) max = hist[i];
		}
		for (int i = 0; i < 256; i++)
		{
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
		switch (wmId)
		{
		case IDC_CHECK_FrameRateAuto:
			FrameRate.IsAuto = cb_FrameRate->Get();
			ShowParameters();
			return (INT_PTR)true;
		case IDC_CHECK_ExposureCompensationAuto:
			ExposureCompensation.IsAuto = cb_ExposureCompensation->Get();
			ShowParameters();
			return (INT_PTR)true;

		case IDC_CHECK_ExposureTimeAuto:
			ExposureTime.IsAuto = cb_ExposureTime->Get();
			ShowParameters();
			return (INT_PTR)true;

		case IDC_CHECK_GainAuto:
			Gain.IsAuto = cb_Gain->Get();
			ShowParameters();
			return (INT_PTR)true;
		case IDC_EDIT_FrameRate:
			if (wmEvent == EN_KILLFOCUS)
			{
				FrameRate.val = eb_FrameRate->GetDouble();
				ShowParameters();
			}
			return true;
		case IDC_EDIT_ExposureCompensation:
			if (wmEvent == EN_KILLFOCUS)
			{
				ExposureCompensation.val = eb_ExposureCompensation->GetDouble();
				ShowParameters();
			}
			return true;
		case IDC_EDIT_ExposureTime:
			if (wmEvent == EN_KILLFOCUS)
			{
				ExposureTime.val = eb_ExposureTime->GetDouble();
				ShowParameters();
			}
			return true;
		case IDC_EDIT_Gain:
			if (wmEvent == EN_KILLFOCUS)
			{
				Gain.val = eb_Gain->GetDouble();
				ShowParameters();
			}
			return true;
		case IDC_EDIT_BlackLevel:
			if (wmEvent == EN_KILLFOCUS)
			{
				BlackLevel.val = eb_BlackLevel->GetDouble();
				ShowParameters();
			}
			return true;
		case IDOK:
			if (eb_FrameRate->hWnd == GetFocus() ||
				eb_ExposureCompensation->hWnd == GetFocus() ||
				eb_ExposureTime->hWnd == GetFocus() ||
				eb_Gain->hWnd == GetFocus() ||
				eb_BlackLevel->hWnd == GetFocus()
				)
			{
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			EndCtrl(hDlg);
			EndDialog(hDlg, wmId);
			return (INT_PTR)true;
		case IDCANCEL:
			if (eb_FrameRate->hWnd == GetFocus())
			{
				eb_FrameRate->SetDouble(fmt, FrameRate.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			if (eb_ExposureCompensation->hWnd == GetFocus())
			{
				eb_ExposureCompensation->SetDouble(fmt, ExposureCompensation.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			if (eb_ExposureTime->hWnd == GetFocus())
			{
				eb_ExposureTime->SetDouble(fmt, ExposureTime.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			if (eb_Gain->hWnd == GetFocus())
			{
				eb_Gain->SetDouble(fmt, Gain.val);
				PostMessage(hDlg, WM_NEXTDLGCTL, 0, 0L);
				return true;
			}
			if (eb_BlackLevel->hWnd == GetFocus())
			{
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


/// <summary>
/// Callback for the parameters dialog
/// </summary>
/// <param name="hDlg"></param>
/// <param name="message"></param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <returns></returns>
INT_PTR UvcCamera::Parameters::CallbackProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Parameters* thisPtr = nullptr;

	switch (message)
	{
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
	if (thisPtr)
	{
		return thisPtr->DlgProc(hDlg, message, wParam, lParam);
	}
	return false;
}

auto UvcCamera::DialogParameters(HWND hWnd) -> bool
{
	INT_PTR result = false;
	Parameters param;
	if (!IsReady()) return result;

	GetCameraParameter(CameraParam);
	result = param.Open(hWnd, *this);
	if (result == IDCANCEL)
	{
		SetCameraParameter(CameraParam);
	}
	GetCameraParameter(CameraParam);

	return result;
}

#pragma endregion ParametersDialog
