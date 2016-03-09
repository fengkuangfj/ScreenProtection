#include "stdafx.h"

CHook * CHook::ms_pInstance = NULL;

CHook::CHook(
	__in HMODULE hModule
	)
{
	m_hModule = NULL;
	m_hHook = NULL;
	m_ulCount = 0;
	m_ulPid = 0;
	m_IsNeedProtect = NULL;
	m_hModule3rd = NULL;

	ZeroMemory(&m_CsHook, sizeof(m_CsHook));
	ZeroMemory(m_tchProcPath, sizeof(m_tchProcPath));
	ZeroMemory(m_tchWindowsDir, sizeof(m_tchWindowsDir));

	if (!Init(hModule))
		CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Init failed");
}

CHook::~CHook()
{
	if (!Unload())
		CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Unload failed");

	m_hModule = NULL;
	m_hHook = NULL;
	m_ulCount = 0;
	m_ulPid = 0;
	m_IsNeedProtect = NULL;
	m_hModule3rd = NULL;

	ZeroMemory(&m_CsHook, sizeof(m_CsHook));
	ZeroMemory(m_tchProcPath, sizeof(m_tchProcPath));
	ZeroMemory(m_tchWindowsDir, sizeof(m_tchWindowsDir));
}

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
		if (!CHook::GetInstance()->m_IsNeedProtect(dwProcessId))
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

		m_hModule = hModule;

		InitializeCriticalSection(&m_CsHook);

		m_ulPid = GetCurrentProcessId();

		if (!CProcessControl::GetInstance()->Get(TRUE, 0, m_tchProcPath, _countof(m_tchProcPath)))
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Get failed");
			__leave;
		}

		if (!GetWindowsDirectory(m_tchWindowsDir, _countof(m_tchWindowsDir)))
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindowsDirectory failed. (%d)", GetLastError());
			__leave;
		}

		m_hModule3rd = LoadLibrary(_T("G:\\GitHub\\ScreenProtection\\Debug\\3rd.dll"));
		if (!m_hModule3rd)
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "LoadLibrary failed. (%d)", GetLastError());
			__leave;
		}

		m_IsNeedProtect = (ISNEEDPROTECT)GetProcAddress(m_hModule3rd, "IsNeedProtect");
		if (!m_IsNeedProtect)
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
		if (m_hModule3rd)
		{
			FreeLibrary(m_hModule3rd);
			m_hModule3rd = NULL;
		}

		CProcessControl::ReleaseInstance();

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
	do 
	{
		EnterCriticalSection(&CHook::GetInstance()->m_CsHook);

		if (!CHook::GetInstance()->m_ulCount)
		{
			CHook::GetInstance()->m_ulCount++;

			if (CHook::GetInstance()->IsNeedNotAttach())
				break;

			if (!CHook::GetInstance()->Attach())
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Attach failed");
				break;
			}
		}
	} while (FALSE);

	LeaveCriticalSection(&CHook::GetInstance()->m_CsHook);

	return CallNextHookEx(NULL, code, wParam, lParam);
}

BOOL
CHook::Hook()
{
	BOOL bRet = FALSE;


	__try
	{
		if (NULL != m_hHook)
			__leave;

		m_hHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, m_hModule, 0);
		if (NULL == m_hHook)
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
		if (NULL != m_hHook)
		{
			if (!UnhookWindowsHookEx(m_hHook))
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "UnhookWindowsHookEx failed. (%d)", GetLastError());
				__leave;
			}

			m_hHook = NULL;
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
		if (_tcslen(m_tchProcPath) >= _tcslen(_T("DbgView.exe")) &&
			(0 == _tcsnicmp(m_tchProcPath + (_tcslen(m_tchProcPath) - _tcslen(_T("DbgView.exe"))), _T("DbgView.exe"), _tcslen(_T("DbgView.exe")))))
		{
			bRet = TRUE;
			__leave;
		}

		if (_tcslen(m_tchProcPath) >= _tcslen(_T("rdfsnap.exe")) &&
			(0 == _tcsnicmp(m_tchProcPath + (_tcslen(m_tchProcPath) - _tcslen(_T("rdfsnap.exe"))), _T("rdfsnap.exe"), _tcslen(_T("rdfsnap.exe")))))
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

CHook *
	CHook::GetInstance(
	__in HMODULE hModule
	)
{
	if (!ms_pInstance)
	{
		do 
		{
			ms_pInstance = new CHook(hModule);
			if (!ms_pInstance)
				Sleep(1000);
			else
				break;
		} while (TRUE);
	}

	return ms_pInstance;
}

VOID
	CHook::ReleaseInstance()
{
	if (ms_pInstance)
	{
		delete ms_pInstance;
		ms_pInstance = NULL;
	}
}
