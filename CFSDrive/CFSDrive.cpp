// CFSDrive.cpp : Implementation of DLL Exports.

//
// Note: COM+ 1.0 Information:
//      Please remember to run Microsoft Transaction Explorer to install the component(s).
//      Registration is not done by default.

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "CFSDrive_i.h"
#include "dllmain.h"
#include "compreg.h"


using namespace ATL;

// Used to determine whether the DLL can be unloaded by OLE.
_Use_decl_annotations_
STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
_Use_decl_annotations_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
_Use_decl_annotations_
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	//HRESULT hr = _AtlModule.DllRegisterServer();
	//return hr;
	ATLTRACE(L"DllRegisterServer\n");
	HR(_AtlModule.DllRegisterServer(FALSE));
	HR(_ShellModule.DllInstall());
	return S_OK;
}

// DllUnregisterServer - Removes entries from the system registry.
_Use_decl_annotations_
STDAPI DllUnregisterServer(void)
{
	//HRESULT hr = _AtlModule.DllUnregisterServer();
	//return hr;
	ATLTRACE(L"DllUnregisterServer\n");
	HR(_ShellModule.DllUninstall());
	HR(_AtlModule.DllUnregisterServer(FALSE));
	return S_OK;
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != nullptr)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if (bInstall)
	{
		hr = DllRegisterServer();
		if (FAILED(hr))
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	return hr;
}

void CALLBACK ShellNew(HWND hwndStub, HINSTANCE hInstance, LPSTR pszCmdLine, int nCmdShow)
{
	ATLTRACE(L"ShellNew\n");
	// Called with VFS_INSTALL_SHELLNEW integration in place
	CComBSTR bstrCmdline = pszCmdLine;
	_ShellModule.ShellAction(L"ShellNew", bstrCmdline);
}

CFSDriveModule _ShellModule;
CFSDriveAPI _CFSDriveAPI;

const CLSID CLSID_ShellFolder = { 0xab54b533, 0xd6e5, 0x449b, { 0x8b, 0x76, 0x37, 0x8e, 0x49, 0xef, 0x10, 0xc1 } };
const CLSID CLSID_SendTo = { 0xab54b533, 0xd6e5, 0x449b, { 0x8b, 0x76, 0x37, 0x8e, 0x49, 0xef, 0x10, 0xc2 } };
const CLSID CLSID_Preview = { 0xab54b533, 0xd6e5, 0x449b, { 0x8b, 0x76, 0x37, 0x8e, 0x49, 0xef, 0x10, 0xc3 } };
const CLSID CLSID_DropTarget = { 0xab54b533, 0xd6e5, 0x449b, { 0x8b, 0x76, 0x37, 0x8e, 0x49, 0xef, 0x10, 0xc4 } };
const CLSID CLSID_ContextMenu = { 0xab54b533, 0xd6e5, 0x449b, { 0x8b, 0x76, 0x37, 0x8e, 0x49, 0xef, 0x10, 0xc5 } };
const CLSID CLSID_PropertySheet = { 0xab54b533, 0xd6e5, 0x449b, { 0x8b, 0x76, 0x37, 0x8e, 0x49, 0xef, 0x10, 0xc6 } };
