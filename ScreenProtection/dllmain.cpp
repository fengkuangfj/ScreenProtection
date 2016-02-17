// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	CSimpleDump			SimpleDump;
	CSimpleLog			SimpleLog;
	CHook				Hook;
	CPrintfEx			PrintfEx;

	CRUSH_HANDLER_INFO	CrushHandlerInfo;
	TCHAR				tchPath[MAX_PATH] = { 0 };
	LPTSTR				lpPosition = NULL;


	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			PrintfEx.Init();

			if (!GetModuleFileName(hModule, tchPath, _countof(tchPath)))
				break;

			lpPosition = _tcsrchr(tchPath, _T('.'));
			if (!lpPosition)
				break;

			*lpPosition = _T('\0');

			StringCchPrintf(tchPath, _countof(tchPath), _T("%lS_%05d.log"), tchPath, GetCurrentProcessId());

			SimpleLog.Init(tchPath);

			ZeroMemory(&CrushHandlerInfo, sizeof(CrushHandlerInfo));
			CrushHandlerInfo.EhType = EH_TYPE_S;
			CrushHandlerInfo.bFirstHandler = TRUE;
			CrushHandlerInfo.MiniDumpType = MiniDumpWithFullMemory;

			SimpleDump.RegisterCrushHandler(&CrushHandlerInfo);

			Hook.Init(hModule);

			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
		{
			Hook.Unload();

			SimpleLog.Unload();

			break;
		}
	}
	return TRUE;
}
