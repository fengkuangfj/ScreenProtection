// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>



// TODO:  在此处引用程序需要的其他头文件
#include "ScreenProtection.h"
#include "Hook.h"
#include "hookhelper.h"
#include "..\\..\\..\\GitHub\\Public\\ModulePath\\ModulePath.h"
#include "..\\..\\..\\GitHub\\Public\\ProcessPath\\ProcessPath.h"
#include "..\\..\\..\\GitHub\\Public\\ProcessType\\ProcessType.h"
#include "..\\..\\..\\GitHub\\Public\\PrintfEx\\PrintfEx.h"
#include "..\\..\\..\\GitHub\\Public\\OperationSystemVersion\\OperationSystemVersion.h"
#include "..\\..\\..\\GitHub\\Public\\SimpleDump\\SimpleDump.h"
#include "..\\..\\..\\GitHub\\Public\\SimpleLog\\SimpleLog.h"
#include "..\\..\\..\\GitHub\\Public\\StackBacktrace\\StackBacktrace.h"

#ifdef _AMD64_
#pragma comment(lib, "hookhelperx64.lib")
#else
#pragma comment(lib, "hookhelperx86.lib")
#endif
