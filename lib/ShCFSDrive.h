
#pragma once

#ifndef STRICT
#define STRICT
#endif // STRICT

// Targeting Windows Vista and above
#define WINVER          _WIN32_WINNT_LONGHORN
#define _WIN32_WINNT    _WIN32_WINNT_LONGHORN
#define _WIN32_WINDOWS  _WIN32_WINNT_LONGHORN
#define _WIN32_IE       _WIN32_IE_IE70

#define ISOLATION_AWARE_ENABLED 1
#define STRICT_TYPED_ITEMIDS

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#define _ATL_APARTMENT_THREADED
#define _ATL_STATIC_REGISTRY

#define _WTL_USE_VSSYM32

#ifdef _DEBUG
#define ATL_TRACE_CATEGORY 0xFFFFFFFF
#define ATL_TRACE_LEVEL 4
#define _ATL_DEBUG_QI
#endif // _DEBUG

#ifndef _UNICODE
#error This framework only compiles with the UNICODE version of Windows
#endif //_UNICODE


// Compiling the code requires a recent version of the Microsoft ATL library.
// A version is often available for download in the Windows Driver Kit (WDK).
#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlwin.h>

// Compiling the code requires a recent version of the Microsoft SDK.
// A version is available for download on the Microsoft MSDN site.
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlguid.h>
#include <ntquery.h>
#include <thumbcache.h>

#pragma warning(disable : 4100)

#define INITGUID

using namespace ATL;

extern const CLSID CLSID_ShellFolder;
extern const CLSID CLSID_SendTo;
extern const CLSID CLSID_Preview;
extern const CLSID CLSID_DropTarget;
extern const CLSID CLSID_ContextMenu;
extern const CLSID CLSID_PropertySheet;

#include "ShUtils.h"
#include "ShellExt.h"
#include "ShellExt2.h"
#include "NseFileSystem.h"