// ScreenProtection.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

BOOL
ScreenProtectionStart()
{
	BOOL bRet = FALSE;


	__try
	{
		if (!CHook::GetInstance()->Hook())
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
	BOOL bRet = FALSE;


	__try
	{
		if (!CHook::GetInstance()->UnHook())
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
