// ScreenProtection.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

BOOL
ScreenProtectionStart()
{
	BOOL	bRet = FALSE;

	CHook	Hook;


	__try
	{
		if (!Hook.Hook())
		{
			CSimpleLogSR(MOD_SCREEN_PROTECTION, LOG_LEVEL_ERROR, "Hook failed");
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
ScreenProtectionStop()
{
	BOOL	bRet = FALSE;

	CHook	Hook;


	__try
	{
		if (!Hook.UnHook())
		{
			CSimpleLogSR(MOD_SCREEN_PROTECTION, LOG_LEVEL_ERROR, "UnHook failed");
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
