#pragma once

#ifndef MOD_SCREEN_PROTECTION
#define MOD_SCREEN_PROTECTION L"½ØÆÁ±£»¤"
#endif

#ifndef DllExport
#define DllExport __declspec(dllexport)
#endif

extern "C"
DllExport
BOOL
ScreenProtectionStart();

extern "C"
DllExport
BOOL
ScreenProtectionStop();
