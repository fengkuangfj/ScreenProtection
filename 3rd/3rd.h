#pragma once

#ifndef MOD_3RD
#define MOD_3RD L"������"
#endif

#ifndef DllExport
#define DllExport __declspec(dllexport)
#endif

extern "C"
	DllExport
	BOOL
	IsNeedProtect(
	__in ULONG ulPid
	);
