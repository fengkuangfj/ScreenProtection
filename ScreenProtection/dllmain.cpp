// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	CRUSH_HANDLER_INFO	CrushHandlerInfo;
	TCHAR				tchPath[MAX_PATH] = { 0 };
	LPTSTR				lpPosition = NULL;


	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			if (!GetModuleFileName(hModule, tchPath, _countof(tchPath)))
				break;

			lpPosition = _tcsrchr(tchPath, _T('.'));
			if (!lpPosition)
				break;

			*lpPosition = _T('\0');

			StringCchPrintf(tchPath, _countof(tchPath), _T("%lS_%05d.log"), tchPath, GetCurrentProcessId());

			CSimpleLog::GetInstance(tchPath);

			ZeroMemory(&CrushHandlerInfo, sizeof(CrushHandlerInfo));
			CrushHandlerInfo.EhType = EH_TYPE_S;
			CrushHandlerInfo.bFirstHandler = TRUE;
			CrushHandlerInfo.MiniDumpType = MiniDumpWithFullMemory;

			CSimpleDump::GetInstance()->RegisterCrushHandler(&CrushHandlerInfo);

			CHook::GetInstance(hModule);

			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
		{
			CHook::ReleaseInstance();
			CSimpleDump::ReleaseInstance();
			CSimpleLog::ReleaseInstance();
			CPrintfEx::ReleaseInstance();

			break;
		}
	}
	return TRUE;
}
