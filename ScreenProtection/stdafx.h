// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>



// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include "ScreenProtection.h"
#include "Hook.h"
#include "hookhelper.h"
#include "..\\..\\Public\\Public.h"
#include "..\\..\\Public\\ModulePath\\ModulePath.h"
#include "..\\..\\Public\\ProcessControl\\ProcessControl.h"
#include "..\\..\\Public\\PrintfEx\\PrintfEx.h"
#include "..\\..\\Public\\OperationSystemVersion\\OperationSystemVersion.h"
#include "..\\..\\Public\\SimpleDump\\SimpleDump.h"
#include "..\\..\\Public\\SimpleLog\\SimpleLog.h"
#include "..\\..\\Public\\StackBacktrace\\StackBacktrace.h"

#ifdef _AMD64_
#pragma comment(lib, "hookhelperx64.lib")
#else
#pragma comment(lib, "hookhelperx86.lib")
#endif
