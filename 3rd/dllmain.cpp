// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	TCHAR tchPath[MAX_PATH] = { 0 };


	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		if (!GetModuleFileName(hModule, tchPath, _countof(tchPath)))
			break;

		PathRemoveExtension(tchPath);

		_tcscat_s(tchPath, _countof(tchPath), _T(".log"));

		CSimpleLog::GetInstance(tchPath);
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			CProcessControl::ReleaseInstance();
			CSimpleLog::ReleaseInstance();
			CPrintfEx::ReleaseInstance();
			break;
		}
	}
	return TRUE;
}

