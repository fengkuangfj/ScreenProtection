// 3rd.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

BOOL
	IsNeedProtect(
	__in ULONG ulPid
	)
{
	BOOL	bRet = FALSE;

	TCHAR	tchProcPath[MAX_PATH] = { 0 };


	__try
	{
		if (!CProcessPath::GetInstance()->Get(FALSE, ulPid, tchProcPath, _countof(tchProcPath)))
		{
			printfEx(MOD_3RD, PRINTF_LEVEL_ERROR, "Get failed");
			__leave;
		}

		if (_tcslen(tchProcPath) >= _tcslen(_T("notepad.exe")) &&
			(0 == _tcsnicmp(tchProcPath + (_tcslen(tchProcPath) - _tcslen(_T("notepad.exe"))), _T("notepad.exe"), _tcslen(_T("notepad.exe")))))
			bRet = TRUE;
	}
	__finally
	{
		;
	}

	return bRet;
}
