// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change this value to use different versions
#define WINVER 0x0420
#include <atlbase.h>
#include <atlstr.h>

#define _WTL_NO_CSTRING
#include <atlapp.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <stdlib.h>
#include <stdio.h>

extern CAppModule _Module;
extern CAtlString selfPath;

#include <atlwin.h>

#include <tpcshell.h>
#include <aygshell.h>
#include <mmsystem.h>
#pragma comment(lib, "aygshell.lib")
