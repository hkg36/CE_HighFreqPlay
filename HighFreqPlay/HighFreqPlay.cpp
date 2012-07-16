// HighFreqPlay.cpp : main source file for HighFreqPlay.exe
//

#include "stdafx.h"

#include <atlframe.h>
#include <atlctrls.h>
#define _WTL_CE_NO_ZOOMSCROLL
#define _WTL_CE_NO_FULLSCREEN
#include <atlwince.h>

#include "resourceppc.h"

#include "HighFreqPlayView.h"
#include "HighFreqPlayFrame.h"

CAppModule _Module;
CAtlString selfPath;
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	::GetModuleFileName(NULL,selfPath.GetBuffer(MAX_PATH),MAX_PATH);
	selfPath.ReleaseBuffer();
	selfPath.Delete(selfPath.ReverseFind('\\')+1,MAX_PATH);

	HRESULT hRes = CHighFreqPlayFrame::ActivatePreviousInstance(hInstance, lpstrCmdLine);

	if(FAILED(hRes) || S_FALSE == hRes)
	{
		return hRes;
	}

	hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	AtlInitCommonControls(ICC_DATE_CLASSES);
	SHInitExtraControls();

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = CHighFreqPlayFrame::AppRun(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

