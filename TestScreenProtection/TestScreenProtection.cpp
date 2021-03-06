// TestScreenProtection.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE					hModule = NULL;
	SCREENPROTECTIONSTART	ScreenProtectionStart = NULL;
	SCREENPROTECTIONSTOP	ScreenProtectionStop = NULL;


	__try
	{
#ifdef _X86_
		hModule = LoadLibrary(L"ScreenProtectionx86.dll");
#else
		hModule = LoadLibrary(L"ScreenProtectionx64.dll");
#endif
		if (!hModule)
		{
			printfEx(MOD_TEST_SCREEN_PROTECTION, PRINTF_LEVEL_ERROR, "LoadLibrary failed. (%d)", GetLastError());
			__leave;
		}

		ScreenProtectionStart = (SCREENPROTECTIONSTART)GetProcAddress(hModule, "ScreenProtectionStart");
		if (!ScreenProtectionStart)
		{
			printfEx(MOD_TEST_SCREEN_PROTECTION, PRINTF_LEVEL_ERROR, "GetProcAddress failed. (%d)", GetLastError());
			__leave;
		}

		ScreenProtectionStop = (SCREENPROTECTIONSTOP)GetProcAddress(hModule, "ScreenProtectionStop");
		if (!ScreenProtectionStop)
		{
			printfEx(MOD_TEST_SCREEN_PROTECTION, PRINTF_LEVEL_ERROR, "GetProcAddress failed. (%d)", GetLastError());
			__leave;
		}

		if (!ScreenProtectionStart())
			__leave;
		else
			printfEx(MOD_TEST_SCREEN_PROTECTION, PRINTF_LEVEL_INFORMATION, "ScreenProtectionStart");

		_getch();

		if (!ScreenProtectionStop())
			__leave;
		else
			printfEx(MOD_TEST_SCREEN_PROTECTION, PRINTF_LEVEL_INFORMATION, "ScreenProtectionStop");
	}
	__finally
	{
		if (hModule)
		{
			FreeLibrary(hModule);
			hModule = NULL;
		}

		CPrintfEx::ReleaseInstance();
	}

	return 0;
}
