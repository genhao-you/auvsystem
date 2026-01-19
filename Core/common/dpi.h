#pragma once
#include "xt_global.h"
#include "core_global.h"
#if defined(_WIN32_WINNT_WIN7)
#include <atlstr.h>
#endif

class Dpi
{
public:
	Dpi();
	~Dpi();

	static void	 setCurrentDpi(qreal dpi);
	static qreal getCurrentDpi();
	static qreal screenDpi(XT::DpiMode mode /*= DPI*/);

#if defined(_WIN32_WINNT_WIN7)
	static float getDPI(HDC pdc);
	static float getXDPI(HDC pdc);
	static float getYDPI(HDC pdc);
private:
	static BOOL MyMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
	static BOOL DisplayDeviceFromHMonitor(HMONITOR hMonitor, DISPLAY_DEVICE & ddMonOut);
	static CString Get2ndSlashBlock(const CString & sIn);
	static bool GetSizeForDevID(const CString & TargetDevID, short & WidthMm, short & HeightMm);
	static bool GetMonitorSizeFromEDID(const HKEY hEDIDRegKey, short & WidthMm, short & HeightMm);
	static BOOL IsWinVerEqualTo(DWORD dwMajorVersion, DWORD dwMinorVersion);
#endif

	static qreal curDpi;
};
