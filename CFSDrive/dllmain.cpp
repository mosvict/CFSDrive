// dllmain.cpp : Implementation of DllMain.

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "CFSDrive_i.h"
#include "dllmain.h"
#include "compreg.h"

CCFSDriveModule _AtlModule;

// DLL Entry Point
//extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	//hInstance;
	//return _AtlModule.DllMain(dwReason, lpReserved);
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		if (!_AtlModule.DllMain(dwReason, lpReserved)) return FALSE;
		if (!_ShellModule.DllMain(dwReason, lpReserved)) return FALSE;
		return TRUE;
	}
	return TRUE;
}

