// TestScreenProtection.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE					hModule = NULL;
	SCREENPROTECTIONSTART	ScreenProtectionStart = NULL;
	SCREENPROTECTIONSTOP	ScreenProtectionStop = NULL;

	CPrintfEx				PrintfEx;


	__try
	{
		PrintfEx.Init();

		hModule = LoadLibrary(L"ScreenProtection.dll");
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
	}

	return 0;
}
