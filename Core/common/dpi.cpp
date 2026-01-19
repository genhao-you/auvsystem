#include "stdafx.h"
#include "dpi.h"
#include <QScreen>
#include <QGuiApplication>

#if defined(_WIN32_WINNT_WIN7)
#include <windows.h>
#include <atlstr.h>
#include <SetupApi.h>
#include <cfgmgr32.h>   // for MAX_DEVICE_ID_LEN
#pragma comment(lib, "setupapi.lib")
#define DEFALTDPI 96.
HMONITOR  g_hMonitor;
const GUID GUID_CLASS_MONITOR =
{ 0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 };
#endif

qreal Dpi::curDpi = 0;
Dpi::Dpi()
{}

Dpi::~Dpi()
{}

//************************************
// Method:    setCurrentDpi
// Brief:	  设置当前DPI
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: qreal dpi
//************************************
void Dpi::setCurrentDpi(qreal dpi)
{
	curDpi = dpi;
}

//************************************
// Method:    getCurrentDpi
// Brief:	  获取当前DPI
// Returns:   QT_NAMESPACE::qreal
// Author:    cl
// DateTime:  2022/07/21
//************************************
qreal Dpi::getCurrentDpi()
{
	return curDpi;
}

//************************************
// Method:    screenDpi
// Brief:	  屏幕DPI
// Returns:   QT_NAMESPACE::qreal
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::DpiMode mode
//************************************
qreal Dpi::screenDpi(XT::DpiMode mode /*= DPI*/)
{
	QScreen *screen = QGuiApplication::screens()[0];
	switch (mode)
	{
	case XT::DPI:
		return screen->physicalDotsPerInch();
	case XT::DPI_X:
		return screen->physicalDotsPerInchX();
	case XT::DPI_Y:
		return screen->physicalDotsPerInchY();
		break;
	default:
		return screen->physicalDotsPerInch();
	}
}
#if defined(_WIN32_WINNT_WIN7)
float Dpi::getDPI(HDC pdc)
{
	if (pdc == NULL)
		return DEFALTDPI;

	float widthP = GetDeviceCaps(pdc, HORZRES);			/*屏幕宽度，像素*/
	float heightP = GetDeviceCaps(pdc, VERTRES);		/*屏幕高度，像素*/
	float widthMM = GetDeviceCaps(pdc, HORZSIZE);		/*屏幕宽度，毫米*/
	float heightMM = GetDeviceCaps(pdc, VERTSIZE);		/*屏幕高度，毫米*/
	int DevTech = GetDeviceCaps(pdc, TECHNOLOGY);		/*设备技术。它可以是以下值之一*/
														//	DT_PLOTTER	矢量绘图仪
														//	DT_RASDISPLAY	栅格显示
														//	DT_RASPRINTER	光栅打印机
														//	DT_RASCAMERA	光栅相机
														//	DT_CHARSTREAM	字符流
														//	DT_METAFILE	图元文件
														//	DT_DISPFILE	显示文件

	if (DevTech != DT_RASDISPLAY || !IsWinVerEqualTo(6, 1))
	{
		return sqrt(widthP*widthP + heightP * heightP) / sqrt(widthMM*widthMM + heightMM * heightMM)*25.4;
	}
	else
	{
		EnumDisplayMonitors(NULL, NULL, MyMonitorEnumProc, NULL);
		DISPLAY_DEVICE ddMon;
		if (FALSE == DisplayDeviceFromHMonitor(g_hMonitor, ddMon))
			return DEFALTDPI;

		CString DeviceID;
		DeviceID.Format(_T("%s"), ddMon.DeviceID);
		DeviceID = Get2ndSlashBlock(DeviceID);

		short WidthMm, HeightMm;
		bool bFoundDevice = GetSizeForDevID(DeviceID, WidthMm, HeightMm);
		if (!bFoundDevice)
			return DEFALTDPI;
		return sqrt(widthP*widthP + heightP * heightP) / sqrt(WidthMm*WidthMm + HeightMm * HeightMm)*25.4;
	}
}

float Dpi::getXDPI(HDC pdc)
{
	if (pdc == NULL)
		return DEFALTDPI;

	float widthP = GetDeviceCaps(pdc, HORZRES);			/*屏幕宽度，像素*/
	float widthMM = GetDeviceCaps(pdc, HORZSIZE);		/*屏幕宽度，毫米*/
	int DevTech = GetDeviceCaps(pdc, TECHNOLOGY);		/*设备技术。*/

	if (DevTech != DT_RASDISPLAY || !IsWinVerEqualTo(6, 1))
	{
		return widthP / widthMM * 25.4;
	}
	else
	{
		EnumDisplayMonitors(NULL, NULL, MyMonitorEnumProc, NULL);
		DISPLAY_DEVICE ddMon;
		if (FALSE == DisplayDeviceFromHMonitor(g_hMonitor, ddMon))
			return DEFALTDPI;

		CString DeviceID;
		DeviceID.Format(_T("%s"), ddMon.DeviceID);
		DeviceID = Get2ndSlashBlock(DeviceID);

		short WidthMm, HeightMm;
		bool bFoundDevice = GetSizeForDevID(DeviceID, WidthMm, HeightMm);
		if (!bFoundDevice)
			return DEFALTDPI;
		return widthP / WidthMm * 25.4;
	}
}

float Dpi::getYDPI(HDC pdc)
{
	if (pdc == NULL)
		return DEFALTDPI;

	float heightP = GetDeviceCaps(pdc, VERTRES);		/*屏幕高度，像素*/
	float heightMM = GetDeviceCaps(pdc, VERTSIZE);		/*屏幕高度，毫米*/
	int DevTech = GetDeviceCaps(pdc, TECHNOLOGY);		/*设备技术。它可以是以下值之一*/
	if (DevTech != DT_RASDISPLAY || !IsWinVerEqualTo(6, 1))
	{
		return heightP / heightMM*25.4;
	}
	else
	{
		EnumDisplayMonitors(NULL, NULL, MyMonitorEnumProc, NULL);
		DISPLAY_DEVICE ddMon;
		if (FALSE == DisplayDeviceFromHMonitor(g_hMonitor, ddMon))
			return DEFALTDPI;

		CString DeviceID;
		DeviceID.Format(_T("%s"), ddMon.DeviceID);
		DeviceID = Get2ndSlashBlock(DeviceID);

		short WidthMm, HeightMm;
		bool bFoundDevice = GetSizeForDevID(DeviceID, WidthMm, HeightMm);
		if (!bFoundDevice)
			return DEFALTDPI;
		return heightP / HeightMm * 25.4;
	}
}
//-------------------------------------------------------------------------
// 函数    : IsWinVerEqualTo
// 功能    : 判断是否=某个特定的系统版本
// 返回值  : BOOL
// 参数    : DWORD dwMajorVersion
// 参数    : DWORD dwMinorVersion
// 附注    :
//-------------------------------------------------------------------------

BOOL Dpi::IsWinVerEqualTo(DWORD dwMajorVersion, DWORD dwMinorVersion)
{
	OSVERSIONINFOEXW osvi = { 0 };
	DWORDLONG dwlConditionMask = 0;

	// 1、初始化系统版本信息数据结构
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = dwMajorVersion;
	osvi.dwMinorVersion = dwMinorVersion;

	// 2、初始化条件掩码
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_EQUAL);

	return ::VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
}

BOOL CALLBACK Dpi::MyMonitorEnumProc(
	_In_  HMONITOR hMonitor,
	_In_  HDC hdcMonitor,
	_In_  LPRECT lprcMonitor,
	_In_  LPARAM dwData
)

{
	// Use this function to identify the monitor of interest: MONITORINFO contains the Monitor RECT.
	MONITORINFOEX mi;
	mi.cbSize = sizeof(MONITORINFOEX);

	GetMonitorInfo(hMonitor, &mi);
	//OutputDebugString(mi.szDevice);

	// For simplicity, we set the last monitor to be the one of interest
	g_hMonitor = hMonitor;

	return TRUE;
}

BOOL Dpi::DisplayDeviceFromHMonitor(HMONITOR hMonitor, DISPLAY_DEVICE& ddMonOut)
{
	MONITORINFOEX mi;
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &mi);

	DISPLAY_DEVICE dd;
	dd.cb = sizeof(dd);
	DWORD devIdx = 0; // device index

	CString DeviceID;
	bool bFoundDevice = false;
	while (EnumDisplayDevices(0, devIdx, &dd, 0))
	{
		devIdx++;
		if (0 != _tcscmp(dd.DeviceName, mi.szDevice))
			continue;

		DISPLAY_DEVICE ddMon;
		ZeroMemory(&ddMon, sizeof(ddMon));
		ddMon.cb = sizeof(ddMon);
		DWORD MonIdx = 0;

		while (EnumDisplayDevices(dd.DeviceName, MonIdx, &ddMon, 0))
		{
			MonIdx++;

			ddMonOut = ddMon;
			return TRUE;

			ZeroMemory(&ddMon, sizeof(ddMon));
			ddMon.cb = sizeof(ddMon);
		}

		ZeroMemory(&dd, sizeof(dd));
		dd.cb = sizeof(dd);
	}

	return FALSE;
}

CString Dpi::Get2ndSlashBlock(const CString& sIn)
{
	int FirstSlash = sIn.Find(_T('\\'));
	CString sOut = sIn.Right(sIn.GetLength() - FirstSlash - 1);
	FirstSlash = sOut.Find(_T('\\'));
	sOut = sOut.Left(FirstSlash);
	return sOut;
}
bool Dpi::GetSizeForDevID(const CString& TargetDevID, short& WidthMm, short& HeightMm)
{
	HDEVINFO devInfo = SetupDiGetClassDevsEx(
		&GUID_CLASS_MONITOR, //class GUID
		NULL, //enumerator
		NULL, //HWND
		DIGCF_PRESENT | DIGCF_PROFILE, // Flags //DIGCF_ALLCLASSES|
		NULL, // device info, create a new one.
		NULL, // machine name, local machine
		NULL);// reserved

	if (NULL == devInfo)
		return false;

	bool bRes = false;

	for (ULONG i = 0; ERROR_NO_MORE_ITEMS != GetLastError(); ++i)
	{
		SP_DEVINFO_DATA devInfoData;
		memset(&devInfoData, 0, sizeof(devInfoData));
		devInfoData.cbSize = sizeof(devInfoData);

		if (SetupDiEnumDeviceInfo(devInfo, i, &devInfoData))
		{
			TCHAR Instance[MAX_DEVICE_ID_LEN];
			SetupDiGetDeviceInstanceId(devInfo, &devInfoData, Instance, MAX_PATH, NULL);

			CString sInstance(Instance);
			if (-1 == sInstance.Find(TargetDevID))
				continue;

			HKEY hEDIDRegKey = SetupDiOpenDevRegKey(devInfo, &devInfoData,
				DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

			if (!hEDIDRegKey || (hEDIDRegKey == INVALID_HANDLE_VALUE))
				continue;

			bRes = GetMonitorSizeFromEDID(hEDIDRegKey, WidthMm, HeightMm);

			RegCloseKey(hEDIDRegKey);
		}
	}
	SetupDiDestroyDeviceInfoList(devInfo);
	return bRes;
}
bool Dpi::GetMonitorSizeFromEDID(const HKEY hEDIDRegKey, short& WidthMm, short& HeightMm)
{
	BYTE EDIDdata[1024];
	DWORD edidsize = sizeof(EDIDdata);

	if (ERROR_SUCCESS != RegQueryValueEx(hEDIDRegKey, _T("EDID"), NULL, NULL, EDIDdata, &edidsize))
		return false;
	WidthMm = ((EDIDdata[68] & 0xF0) << 4) + EDIDdata[66];
	HeightMm = ((EDIDdata[68] & 0x0F) << 8) + EDIDdata[67];

	return true; // valid EDID found
}
#endif
