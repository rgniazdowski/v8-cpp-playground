#pragma once
#ifndef FG_INC_WINDOWS_STD
#define FG_INC_WINDOWS_STD

#if (defined __WINDOWS__ || defined __WIN32__ || defined _WIN32 || defined _WIN64 || defined __TOS_WIN__ || defined WIN32 || defined win32)
#define WIN32_LEAN_AND_MEAN
#define STRICT
#ifndef UNICODE
#define UNICODE 1
#endif
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501 /* Need 0x410 for AlphaBlend() and 0x500 for EnumDisplayDevices(), 0x501 for raw input */

#include <windows.h>

#endif
#endif //> FG_INC_WINDOWS_STD