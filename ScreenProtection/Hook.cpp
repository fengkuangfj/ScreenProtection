#include "stdafx.h"

HMODULE				CHook::ms_hModule = NULL;
CRITICAL_SECTION	CHook::ms_CsHook = { 0 };
HHOOK				CHook::ms_hHook = NULL;
ULONG				CHook::ms_ulCount = 0;
TCHAR				CHook::ms_tchProcPath[MAX_PATH] = { 0 };
TCHAR				CHook::ms_tchWindowsDir[MAX_PATH] = { 0 };
ULONG				CHook::ms_ulPid = 0;
ISNEEDPROTECT		CHook::ms_IsNeedProtect = NULL;
HMODULE				CHook::ms_hModule3rd = NULL;

BOOL
CHook::Attach()
{
	BOOL bRet = FALSE;


	__try
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueBitBlt, NewBitBlt);
		DetourTransactionCommit();

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOL
CHook::Detach()
{
	BOOL bRet = FALSE;


	__try
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)TrueBitBlt, NewBitBlt);
		DetourTransactionCommit();

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOL
CALLBACK
CHook::EnumWindowsProc(
_In_ HWND   hwnd,
_In_ LPARAM lParam
)
{
	WINDOWINFO					WindowInfo = { 0 };
	DWORD						dwProcessId = 0;
	LPENUM_WINDOWS_PROC_PARAM	lpEnumWindowsProcParam = NULL;


	__try
	{
		if (!hwnd || !lParam)
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "input arguments error");
			__leave;
		}

		lpEnumWindowsProcParam = (LPENUM_WINDOWS_PROC_PARAM)lParam;

		WindowInfo.cbSize = sizeof(WindowInfo);

		if (!GetWindowInfo(hwnd, &WindowInfo))
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindowInfo failed. (%d)", GetLastError());
			__leave;
		}

		if ((0 == WindowInfo.rcWindow.left && WindowInfo.rcWindow.left == WindowInfo.rcWindow.right) || 
			(0 == WindowInfo.rcWindow.top && WindowInfo.rcWindow.top == WindowInfo.rcWindow.bottom))
			__leave;

		if (!(((WindowInfo.rcWindow.left < lpEnumWindowsProcParam->Rect.left &&  lpEnumWindowsProcParam->Rect.left < WindowInfo.rcWindow.right) ||
			(lpEnumWindowsProcParam->Rect.left <= WindowInfo.rcWindow.left && WindowInfo.rcWindow.right <= lpEnumWindowsProcParam->Rect.right) ||
			(WindowInfo.rcWindow.left < lpEnumWindowsProcParam->Rect.right &&  lpEnumWindowsProcParam->Rect.right < WindowInfo.rcWindow.right))
			&&
			((WindowInfo.rcWindow.top < lpEnumWindowsProcParam->Rect.top &&  lpEnumWindowsProcParam->Rect.top < WindowInfo.rcWindow.bottom) ||
			(lpEnumWindowsProcParam->Rect.top <= WindowInfo.rcWindow.top && WindowInfo.rcWindow.bottom <= lpEnumWindowsProcParam->Rect.bottom) ||
			(WindowInfo.rcWindow.top < lpEnumWindowsProcParam->Rect.bottom &&  lpEnumWindowsProcParam->Rect.bottom < WindowInfo.rcWindow.bottom))))
			__leave;

		GetWindowThreadProcessId(hwnd, &dwProcessId);
		if (!ms_IsNeedProtect(dwProcessId))
			__leave;

		lpEnumWindowsProcParam->ProtectWindowsInfo[lpEnumWindowsProcParam->ulCount].Rect.left = WindowInfo.rcWindow.left;
		lpEnumWindowsProcParam->ProtectWindowsInfo[lpEnumWindowsProcParam->ulCount].Rect.top = WindowInfo.rcWindow.top;
		lpEnumWindowsProcParam->ProtectWindowsInfo[lpEnumWindowsProcParam->ulCount].Rect.right = WindowInfo.rcWindow.right;
		lpEnumWindowsProcParam->ProtectWindowsInfo[lpEnumWindowsProcParam->ulCount].Rect.bottom = WindowInfo.rcWindow.bottom;
		lpEnumWindowsProcParam->ulCount++;
	}
	__finally
	{
		;
	}

	return TRUE;
}

BOOL
WINAPI
CHook::NewBitBlt(
_In_ HDC   hdcDest,
_In_ int   nXDest,
_In_ int   nYDest,
_In_ int   nWidth,
_In_ int   nHeight,
_In_ HDC   hdcSrc,
_In_ int   nXSrc,
_In_ int   nYSrc,
_In_ DWORD dwRop
)
{
	BOOL					bNeedTureBitBlt = TRUE;
	HWND					hWnd = NULL;
	WINDOWINFO				WindowInfo = { 0 };
	ENUM_WINDOWS_PROC_PARAM EnuWindowsProcParam = { 0 };


	__try
	{
		hWnd = WindowFromDC(hdcSrc);
		if (!hWnd)
		{
			// CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "WindowFromDC failed");
			__leave;
		}

		WindowInfo.cbSize = sizeof(WindowInfo);

		if (!GetWindowInfo(hWnd, &WindowInfo))
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindowInfo failed. (%d)", GetLastError());
			__leave;
		}

		EnuWindowsProcParam.Rect.left = WindowInfo.rcWindow.left;
		EnuWindowsProcParam.Rect.top = WindowInfo.rcWindow.top;
		EnuWindowsProcParam.Rect.right = WindowInfo.rcWindow.right;
		EnuWindowsProcParam.Rect.bottom = WindowInfo.rcWindow.bottom;

		if (!EnumWindows(EnumWindowsProc, (LPARAM)&EnuWindowsProcParam))
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "EnumWindows failed. (%d)", GetLastError());
			__leave;
		}

		if (!EnuWindowsProcParam.ulCount)
			__leave;

		bNeedTureBitBlt = FALSE;
	}
	__finally
	{
		;
	}

	if (bNeedTureBitBlt)
		return TrueBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	else
		return TRUE;
}

BOOL
CHook::Init(
__in HMODULE hModule
)
{
	BOOL bRet = FALSE;


	__try
	{
		if (!hModule)
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "argument error");
			__leave;
		}

		ms_hModule = hModule;

		InitializeCriticalSection(&ms_CsHook);

		ms_ulPid = GetCurrentProcessId();

		if (!CProcessPath::Get(TRUE, 0, ms_tchProcPath, _countof(ms_tchProcPath)))
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Get failed");
			__leave;
		}

		if (!GetWindowsDirectory(ms_tchWindowsDir, _countof(ms_tchWindowsDir)))
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindowsDirectory failed. (%d)", GetLastError());
			__leave;
		}

		ms_hModule3rd = LoadLibrary(_T("D:\\1\\3rd.dll"));
		if (!ms_hModule3rd)
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "LoadLibrary failed. (%d)", GetLastError());
			__leave;
		}

		ms_IsNeedProtect = (ISNEEDPROTECT)GetProcAddress(ms_hModule3rd, "IsNeedProtect");
		if (!ms_IsNeedProtect)
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetProcAddress failed. (%d)", GetLastError());
			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		if (!bRet)
		{
			if (!Unload())
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Unload failed");
		}
	}

	return bRet;
}

BOOL
CHook::Unload()
{
	BOOL bRet = FALSE;


	__try
	{
		if (ms_hModule3rd)
		{
			FreeLibrary(ms_hModule3rd);
			ms_hModule3rd = NULL;
		}

		if (!Detach())
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Detach failed");
			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

LRESULT
CALLBACK
CHook::GetMsgProc(
_In_ int    code,
_In_ WPARAM wParam,
_In_ LPARAM lParam
)
{
	CHook Hook;


	__try
	{
		EnterCriticalSection(&ms_CsHook);

		if (!ms_ulCount)
		{
			ms_ulCount++;

			if (Hook.IsNeedNotAttach())
				__leave;

			if (!Hook.Attach())
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Attach failed");
				__leave;
			}
		}
	}
	__finally
	{
		LeaveCriticalSection(&ms_CsHook);
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}

BOOL
CHook::Hook()
{
	BOOL bRet = FALSE;


	__try
	{
		if (NULL != ms_hHook)
			__leave;

		ms_hHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, ms_hModule, 0);
		if (NULL == ms_hHook)
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "SetWindowsHookEx failed. (%d)", GetLastError());
			__leave;
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOL
CHook::UnHook()
{
	BOOL bRet = FALSE;


	__try
	{
		if (NULL != ms_hHook)
		{
			if (!UnhookWindowsHookEx(ms_hHook))
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "UnhookWindowsHookEx failed. (%d)", GetLastError());
				__leave;
			}

			ms_hHook = NULL;
		}

		bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}

BOOL
CHook::IsNeedNotAttach()
{
	BOOL bRet = FALSE;


	__try
	{
		if (_tcslen(ms_tchProcPath) >= _tcslen(_T("DbgView.exe")) &&
			(0 == _tcsnicmp(ms_tchProcPath + (_tcslen(ms_tchProcPath) - _tcslen(_T("DbgView.exe"))), _T("DbgView.exe"), _tcslen(_T("DbgView.exe")))))
		{
			bRet = TRUE;
			__leave;
		}

		if (_tcslen(ms_tchProcPath) >= _tcslen(_T("FSCapture.exe")) &&
			(0 == _tcsnicmp(ms_tchProcPath + (_tcslen(ms_tchProcPath) - _tcslen(_T("FSCapture.exe"))), _T("FSCapture.exe"), _tcslen(_T("FSCapture.exe")))))
			bRet = FALSE;
		else
			bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}
