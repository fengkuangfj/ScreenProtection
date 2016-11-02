#include "stdafx.h"

CHook * CHook::ms_pInstance = NULL;

CHook::CHook(
	__in HMODULE hModule
	)
{
	m_hModule = NULL;
	m_hGetMsgHook = NULL;
	m_hKeyboardHook = NULL;
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
	m_hGetMsgHook = NULL;
	m_hKeyboardHook = NULL;
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
	BOOL		bRet = FALSE;

	HWND		hWndSrc = NULL;
	HWND		hWndDesktop = NULL;
	HWND		hWnd = NULL;
	WINDOWINFO	WindowInfoSrc = { 0 };
	WINDOWINFO	WindowInfo = { 0 };
	DWORD		dwPid = 0;
	BOOL		bFind = FALSE;
	BOOL		bUsedBitBlt = FALSE;
	DWORD		dwPidCurrent = 0;
	TCHAR		tchProcName[MAX_PATH] = { 0 };


	__try
	{
		hWndSrc = WindowFromDC(hdcSrc);
		if (!hWndSrc)
			__leave;

		GetWindowThreadProcessId(hWndSrc, &dwPid);

		dwPidCurrent = GetCurrentProcessId();
		if (dwPid == dwPidCurrent)
			__leave;

		if (CHook::GetInstance()->m_IsNeedProtect(dwPid)
			||
			(CProcessControl::GetInstance()->GetName(
			FALSE,
			dwPid,
			tchProcName,
			_countof(tchProcName)
			) &&
			0 != _tcsnicmp(_T("csrss.exe"), tchProcName, _tcslen(tchProcName))))
		{
			bRet = TrueBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, BLACKNESS);
			if (!bRet)
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "TrueBitBlt failed. (%d)", GetLastError());
				__leave;
			}

			bUsedBitBlt = TRUE;

			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "BitBlt BLACKNESS direct. (%d) X (%d) - (%d) Y (%d) - (%d)",
				dwPid,
				nXDest,
				nXDest + nWidth,
				nYDest,
				nYDest + nHeight
				);

			__leave;
		}

		WindowInfoSrc.cbSize = sizeof(WindowInfoSrc);
		if (!GetWindowInfo(hWndSrc, &WindowInfoSrc))
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindowInfo failed. (%d)", GetLastError());
			__leave;
		}

		hWndDesktop = GetDesktopWindow();
		if (!hWndDesktop)
			__leave;

		hWnd = GetWindow(hWndDesktop, GW_CHILD);
		if (!hWnd)
		{
			if (0 != GetLastError())
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindow failed. (%d)", GetLastError());

			__leave;
		}

		hWnd = GetWindow(hWnd, GW_HWNDLAST);
		if (!hWnd)
		{
			if (0 != GetLastError())
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindow failed. (%d)", GetLastError());

			__leave;
		}

		CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "BitBlt begin");

		do 
		{
			ZeroMemory(&WindowInfo, sizeof(WindowInfo));
			dwPid = 0;

			hWnd = GetWindow(hWnd, GW_HWNDPREV);
			if (!hWnd)
			{
				if (0 == GetLastError() || 2 == GetLastError() || 6 == GetLastError())
					break;

				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindow failed. (%d)", GetLastError());
				__leave;
			}

			WindowInfo.cbSize = sizeof(WindowInfo);
			if (!GetWindowInfo(hWnd, &WindowInfo))
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "GetWindowInfo failed. (%d)", GetLastError());
				__leave;
			}

			if (WS_VISIBLE != (WindowInfo.dwStyle & WS_VISIBLE) ||
				WindowInfo.rcWindow.left == WindowInfo.rcWindow.right ||
				WindowInfo.rcWindow.top == WindowInfo.rcWindow.bottom)
				continue;

			if (!bFind)
			{
				if (!(((WindowInfo.rcWindow.left <= WindowInfoSrc.rcWindow.left && WindowInfoSrc.rcWindow.left <= WindowInfo.rcWindow.right) ||
					(WindowInfoSrc.rcWindow.left <= WindowInfo.rcWindow.left && WindowInfo.rcWindow.right <= WindowInfoSrc.rcWindow.right) ||
					(WindowInfo.rcWindow.left <= WindowInfoSrc.rcWindow.right && WindowInfoSrc.rcWindow.right <= WindowInfo.rcWindow.right))
					&&
					((WindowInfo.rcWindow.top <= WindowInfoSrc.rcWindow.top && WindowInfoSrc.rcWindow.top <= WindowInfo.rcWindow.bottom) ||
					(WindowInfoSrc.rcWindow.top <= WindowInfo.rcWindow.top && WindowInfo.rcWindow.bottom <= WindowInfoSrc.rcWindow.bottom) ||
					(WindowInfo.rcWindow.top <= WindowInfoSrc.rcWindow.bottom && WindowInfoSrc.rcWindow.bottom <= WindowInfo.rcWindow.bottom))))
					continue;

				GetWindowThreadProcessId(hWnd, &dwPid);

				if (dwPid == dwPidCurrent ||
					!CHook::GetInstance()->m_IsNeedProtect(dwPid))
					continue;

				bFind = TRUE;

				bRet = TrueBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
				if (!bRet)
				{
					CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "TrueBitBlt failed. (%d)", GetLastError());
					__leave;
				}

				bUsedBitBlt = TRUE;

				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "BitBlt all. (%d) X (%d) - (%d) Y (%d) - (%d)",
					dwPid,
					nXDest,
					nXDest + nWidth,
					nYDest,
					nYDest + nHeight
					);

				bRet = TrueBitBlt(
					hdcDest,
					WindowInfo.rcWindow.left,
					WindowInfo.rcWindow.top,
					WindowInfo.rcWindow.right - WindowInfo.rcWindow.left,
					WindowInfo.rcWindow.bottom - WindowInfo.rcWindow.top,
					hdcSrc,
					WindowInfo.rcWindow.left,
					WindowInfo.rcWindow.top,
					BLACKNESS
					);
				if (!bRet)
				{
					CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "TrueBitBlt failed. (%d)", GetLastError());
					__leave;
				}

				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "BitBlt BLACKNESS first. (%d) X (%d) - (%d) Y (%d) - (%d)",
					dwPid,
					WindowInfo.rcWindow.left,
					WindowInfo.rcWindow.right,
					WindowInfo.rcWindow.top,
					WindowInfo.rcWindow.bottom
					);
			}
			else
			{
				if (!(((WindowInfo.rcWindow.left <= WindowInfoSrc.rcWindow.left && WindowInfoSrc.rcWindow.left <= WindowInfo.rcWindow.right) ||
					(WindowInfoSrc.rcWindow.left <= WindowInfo.rcWindow.left && WindowInfo.rcWindow.right <= WindowInfoSrc.rcWindow.right) ||
					(WindowInfo.rcWindow.left <= WindowInfoSrc.rcWindow.right && WindowInfoSrc.rcWindow.right <= WindowInfo.rcWindow.right))
					&&
					((WindowInfo.rcWindow.top <= WindowInfoSrc.rcWindow.top && WindowInfoSrc.rcWindow.top <= WindowInfo.rcWindow.bottom) ||
					(WindowInfoSrc.rcWindow.top <= WindowInfo.rcWindow.top && WindowInfo.rcWindow.bottom <= WindowInfoSrc.rcWindow.bottom) ||
					(WindowInfo.rcWindow.top <= WindowInfoSrc.rcWindow.bottom && WindowInfoSrc.rcWindow.bottom <= WindowInfo.rcWindow.bottom))))
				{
					bRet = TrueBitBlt(
						hdcDest,
						WindowInfo.rcWindow.left,
						WindowInfo.rcWindow.top,
						WindowInfo.rcWindow.right - WindowInfo.rcWindow.left,
						WindowInfo.rcWindow.bottom - WindowInfo.rcWindow.top,
						hdcSrc,
						WindowInfo.rcWindow.left,
						WindowInfo.rcWindow.top,
						dwRop
						);
					if (!bRet)
					{
						CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "TrueBitBlt failed. (%d)", GetLastError());
						__leave;
					}

					bUsedBitBlt = TRUE;

					CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "BitBlt dwRop no overlap. (%d) X (%d) - (%d) Y (%d) - (%d)",
						dwPid,
						WindowInfo.rcWindow.left,
						WindowInfo.rcWindow.right,
						WindowInfo.rcWindow.top,
						WindowInfo.rcWindow.bottom
						);

					continue;
				}

				GetWindowThreadProcessId(hWnd, &dwPid);

				if (dwPid == dwPidCurrent ||
					!CHook::GetInstance()->m_IsNeedProtect(dwPid))
				{
					if (WS_EX_LAYERED == (WindowInfo.dwExStyle & WS_EX_LAYERED))
						continue;

					bRet = TrueBitBlt(
						hdcDest,
						WindowInfo.rcWindow.left,
						WindowInfo.rcWindow.top,
						WindowInfo.rcWindow.right - WindowInfo.rcWindow.left,
						WindowInfo.rcWindow.bottom - WindowInfo.rcWindow.top,
						hdcSrc,
						WindowInfo.rcWindow.left,
						WindowInfo.rcWindow.top,
						dwRop
						);
					if (!bRet)
					{
						CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "TrueBitBlt failed. (%d)", GetLastError());
						__leave;
					}

					bUsedBitBlt = TRUE;

					CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "BitBlt dwRop overlap, but need not protect. (%d) X (%d) - (%d) Y (%d) - (%d)",
						dwPid,
						WindowInfo.rcWindow.left,
						WindowInfo.rcWindow.right,
						WindowInfo.rcWindow.top,
						WindowInfo.rcWindow.bottom
						);

					continue;
				}

				bRet = TrueBitBlt(
					hdcDest,
					WindowInfo.rcWindow.left,
					WindowInfo.rcWindow.top,
					WindowInfo.rcWindow.right - WindowInfo.rcWindow.left,
					WindowInfo.rcWindow.bottom - WindowInfo.rcWindow.top,
					hdcSrc,
					WindowInfo.rcWindow.left,
					WindowInfo.rcWindow.top,
					BLACKNESS
					);
				if (!bRet)
				{
					CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "TrueBitBlt failed. (%d)", GetLastError());
					__leave;
				}

				bUsedBitBlt = TRUE;

				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "BitBlt BLACKNESS not first. (%d) X (%d) - (%d) Y (%d) - (%d)",
					dwPid,
					WindowInfo.rcWindow.left,
					WindowInfo.rcWindow.right,
					WindowInfo.rcWindow.top,
					WindowInfo.rcWindow.bottom
					);
			}
		} while (TRUE);

		CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "BitBlt end");
	}
	__finally
	{
		if (!bUsedBitBlt)
			bRet = TrueBitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, dwRop);
	}

	return bRet;
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

#ifdef _DEBUG
#ifdef _X86_
		m_hModule3rd = LoadLibrary(_T("G:\\GitHub\\Application\\ScreenProtection\\Debug\\3rdx86.dll"));
#else
		m_hModule3rd = LoadLibrary(_T("G:\\GitHub\\Application\\ScreenProtection\\x64\\Debug\\3rdx64.dll"));
#endif	
#else
#ifdef _X86_
		m_hModule3rd = LoadLibrary(_T("G:\\GitHub\\Application\\ScreenProtection\\Release\\3rdx86.dll"));
#else
		m_hModule3rd = LoadLibrary(_T("G:\\GitHub\\Application\\ScreenProtection\\x64\\Release\\3rdx64.dll"));
#endif	
#endif
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
	__try
	{
		EnterCriticalSection(&CHook::GetInstance()->m_CsHook);

		if (!CHook::GetInstance()->m_ulCount)
		{
			CHook::GetInstance()->m_ulCount++;

			if (CHook::GetInstance()->IsNeedNotAttach())
				__leave;

			if (!CHook::GetInstance()->Attach())
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "Attach failed");
				__leave;
			}
		}
	}
	__finally
	{
		LeaveCriticalSection(&CHook::GetInstance()->m_CsHook);
	}

	return CallNextHookEx(CHook::GetInstance()->m_hGetMsgHook, code, wParam, lParam);
}

LRESULT
CALLBACK
CHook::KeyboardProc(
	_In_ int    code,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	LPKBDLLHOOKSTRUCT	lpKbDllHookStruct = NULL;
	BOOL				bReject = FALSE;


	__try
	{
		if (HC_ACTION != code)
			__leave;

		if (!lParam)
			__leave;

		lpKbDllHookStruct = (LPKBDLLHOOKSTRUCT)lParam;

		if (VK_SNAPSHOT != lpKbDllHookStruct->vkCode)
			__leave;

		if (WM_KEYDOWN == wParam || WM_KEYUP == wParam || WM_SYSKEYDOWN == wParam || WM_SYSKEYUP == wParam)
		{
			// CSimpleLogSR(MOD_HOOK, LOG_LEVEL_INFORMATION, "%s", CHook::GetInstance()->m_tchProcPath);
			bReject = TRUE;
		}
	}
	__finally
	{
		;
	}

	if (bReject)
		return 1;
	else
		return CallNextHookEx(CHook::GetInstance()->m_hKeyboardHook, code, wParam, lParam);
}

BOOL
CHook::Hook()
{
	BOOL bRet = FALSE;


	__try
	{
		if (NULL != m_hGetMsgHook)
			__leave;

		m_hGetMsgHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgProc, m_hModule, 0);
		if (NULL == m_hGetMsgHook)
		{
			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "SetWindowsHookEx WH_GETMESSAGE failed. (%d)", GetLastError());
			__leave;
		}

// 		if (NULL != m_hKeyboardHook)
// 			__leave;
// 
// 		m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardProc, m_hModule, 0);
// 		if (NULL == m_hKeyboardHook)
// 		{
// 			CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "SetWindowsHookEx WH_KEYBOARD_LL failed. (%d)", GetLastError());
// 			__leave;
// 		}

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
		if (NULL != m_hGetMsgHook)
		{
			if (!UnhookWindowsHookEx(m_hGetMsgHook))
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "UnhookWindowsHookEx WH_GETMESSAGE failed. (%d)", GetLastError());
				__leave;
			}

			m_hGetMsgHook = NULL;
		}

		if (NULL != m_hKeyboardHook)
		{
			if (!UnhookWindowsHookEx(m_hKeyboardHook))
			{
				CSimpleLogSR(MOD_HOOK, LOG_LEVEL_ERROR, "UnhookWindowsHookEx WH_KEYBOARD_LL failed. (%d)", GetLastError());
				__leave;
			}

			m_hKeyboardHook = NULL;
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
