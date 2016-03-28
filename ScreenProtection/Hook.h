#pragma once

#ifndef MOD_HOOK
#define MOD_HOOK L"HOOK"
#endif

typedef struct _PROTECT_WINDOWS_INFO
{
	RECT Rect;
} PROTECT_WINDOWS_INFO, *PPROTECT_WINDOWS_INFO, *LPPROTECT_WINDOWS_INFO;

typedef struct _ENUM_WINDOWS_PROC_PARAM
{
	RECT					Rect;
	ULONG					ulCount;
	PROTECT_WINDOWS_INFO	ProtectWindowsInfo[1000];
} ENUM_WINDOWS_PROC_PARAM, *PENUM_WINDOWS_PROC_PARAM, *LPENUM_WINDOWS_PROC_PARAM;

typedef
	BOOL
	(*ISNEEDPROTECT)(
	__in ULONG ulPid
	);

static
BOOL
(WINAPI*TrueBitBlt)(
_In_ HDC   hdcDest,
_In_ int   nXDest,
_In_ int   nYDest,
_In_ int   nWidth,
_In_ int   nHeight,
_In_ HDC   hdcSrc,
_In_ int   nXSrc,
_In_ int   nYSrc,
_In_ DWORD dwRop
) = BitBlt;

class CHook
{
public:
	static
		CHook *
		GetInstance(
		__in HMODULE hModule = NULL
		);

	static
		VOID
		ReleaseInstance();

	BOOL
		Hook();

	BOOL
		UnHook();

private:
	static CHook		*	ms_pInstance;

	HMODULE					m_hModule;
	CRITICAL_SECTION		m_CsHook;
	HHOOK					m_hHook;
	ULONG					m_ulCount;
	TCHAR					m_tchProcPath[MAX_PATH];
	TCHAR					m_tchWindowsDir[MAX_PATH];
	ULONG					m_ulPid;
	ISNEEDPROTECT			m_IsNeedProtect;
	HMODULE					m_hModule3rd;

	CHook(
		__in HMODULE hModule
		);

	~CHook();

	BOOL
		Init(
		__in HMODULE hModule
		);

	BOOL
		Unload();

	BOOL
		Attach();

	BOOL
		Detach();

	static
		LRESULT
		CALLBACK
		GetMsgProc(
		_In_ int    code,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam
		);

	static
		BOOL
		WINAPI
		NewBitBlt(
		_In_ HDC   hdcDest,
		_In_ int   nXDest,
		_In_ int   nYDest,
		_In_ int   nWidth,
		_In_ int   nHeight,
		_In_ HDC   hdcSrc,
		_In_ int   nXSrc,
		_In_ int   nYSrc,
		_In_ DWORD dwRop
		);

	BOOL
		IsNeedNotAttach();
};
