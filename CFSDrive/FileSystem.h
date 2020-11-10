/////////////////////////////////////////////////////////////////////////////
// CFSDrive Shell Extension
//

#pragma once

#include "NseFileSystem.h"
#include "rest.h"


///////////////////////////////////////////////////////////////////////////////
// Definitions

#define CFSDRIVE_MAGIC_ID_ROOT      0xC0
#define CFSDRIVE_MAGIC_ID_ACCOUNT   0xC1
#define CFSDRIVE_MAGIC_ID_IMAGESET  0xC2
#define CFSDRIVE_MAGIC_ID_PHOTO     0xC8


///////////////////////////////////////////////////////////////////////////////
// CFSDriveModule

class CFSDriveModule : public CNseModule
{
public:
   // CShellModule

   LONG GetConfigInt(VFS_CONFIG Item);
   BOOL GetConfigBool(VFS_CONFIG Item);
   LPCWSTR GetConfigStr(VFS_CONFIG Item);

   HRESULT DllInstall();
   HRESULT DllUninstall();
   HRESULT ShellAction(LPCWSTR pstrType, LPCWSTR pstrCmdLine);

   BOOL DllMain(DWORD dwReason, LPVOID lpReserved);

   HRESULT CreateFileSystem(PCIDLIST_ABSOLUTE pidlRoot, CNseFileSystem** ppFS);
};


///////////////////////////////////////////////////////////////////////////////
// CFSDriveFileSystem

class CFSDriveFileSystem : public CNseFileSystem
{
public:
   volatile LONG m_cRef;

   // Constructor

   CFSDriveFileSystem();
   virtual ~CFSDriveFileSystem();

   // Operations

   HRESULT Init();

   // CShellFileSystem

   VOID AddRef();
   VOID Release();

   CNseItem* GenerateRoot(CShellFolder* pFolder);
};


///////////////////////////////////////////////////////////////////////////////
// CFSDriveItemRoot

class CFSDriveItemRoot : public CNseBaseItem
{
public:
	CFSDriveItemRoot(CShellFolder* pFolder);

   // CNseBaseItem

   BYTE GetType();
   VFS_FIND_DATA GetFindData();
   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_FOLDERSETTINGS GetFolderSettings();

   HRESULT IsDropDataAvailable(IDataObject* pDataObj);
   HRESULT GetPropertyPage(/*CNsePropertyPage** ppPage*/);
   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);

   HMENU GetMenu();
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

  
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);

   // Implementation

   //HRESULT _DoPasteSendTo(VFS_MENUCOMMAND& Cmd);
};


///////////////////////////////////////////////////////////////////////////////
// CFSDriveItemAccount

#if !defined(_M_X64) && !defined(_M_IA64)
#include <pshpack1.h>
#endif
typedef struct tagCFSDRIVEFS_PIDL_ACCOUNT
{
   // SHITEMID 
   USHORT cb;
   // Type identifiers
   BYTE magic;
   BYTE reserved;
   // Account ID
   WCHAR cNSID[62];
} CFSDRIVEFS_PIDL_ACCOUNT;
#if !defined(_M_X64) && !defined(_M_IA64)
#include <poppack.h>
#endif


class CFSDriveItemAccount : public CNseBaseItem
{
public:
   TCFSDriveAccount* m_pAccount;

   // Constructor 

   CFSDriveItemAccount(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseBaseItem

   BYTE GetType();
   VFS_FIND_DATA GetFindData();
   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_FOLDERSETTINGS GetFolderSettings();

   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);

   HMENU GetMenu();
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   //CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   //CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   //HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   //HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);

   HRESULT Delete();
   HRESULT Refresh(VFS_REFRESH Reason);

   // Static members

   static PCITEMID_CHILD GenerateITEMID(const CFSDRIVEFS_PIDL_ACCOUNT& src);
   static PCITEMID_CHILD GenerateITEMID(const TCFSDriveAccount* pAccount);
   static PCITEMID_CHILD GenerateITEMID(const WIN32_FIND_DATA& wfd);

   // Implementation

   HRESULT _EnsureCFSDriveRef();
   HRESULT _DoCreatePhotoSet(VFS_MENUCOMMAND& Cmd);
   HRESULT _DoCreateTagFolder(VFS_MENUCOMMAND& Cmd);
};


///////////////////////////////////////////////////////////////////////////////
// CFSDriveItemImageset
/*
#if !defined(_M_X64) && !defined(_M_IA64)
#include <pshpack1.h>
#endif
typedef struct tagCFSDRIVEFS_PIDL_IMAGESET
{
   // SHITEMID 
   USHORT cb;
   // Type identifiers
   BYTE magic;
   BYTE reserved;
   // Imageset ID
   WCHAR cImagesetID[62];
} CFSDRIVEFS_PIDL_IMAGESET;
#if !defined(_M_X64) && !defined(_M_IA64)
#include <poppack.h>
#endif


class CFSDriveItemAccount : public CNseBaseItem
{
public:
   //TCFSDriveImageset* m_pImageset;

   // Constructor

   CFSDriveItemAccount(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseBaseItem

   BYTE GetType();
   VFS_FIND_DATA GetFindData();
   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_FOLDERSETTINGS GetFolderSettings();
   VFS_PROPSTATE GetPropertyState(REFPROPERTYKEY pk);

   HRESULT IsDropDataAvailable(IDataObject* pDataObj);
   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Column);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v);
   HRESULT GetPaneState(REFEXPLORERPANE ep, EXPLORERPANESTATE* peps);

   HMENU GetMenu();
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);
   CNseItem* GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd);

   HRESULT GetChild(LPCWSTR pstrName, SHGNO ParseType, CNseItem** pItem);
   HRESULT EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aList);

   HRESULT Rename(LPCWSTR pstrNewName, LPWSTR pstrOutputName);
   HRESULT Delete();
   HRESULT Refresh(VFS_REFRESH Reason);

   // Static members

   static PCITEMID_CHILD GenerateITEMID(const CFSDRIVEFS_PIDL_IMAGESET& src);
   //static PCITEMID_CHILD GenerateITEMID(const TCFSDriveImageset* pImageset);
   static PCITEMID_CHILD GenerateITEMID(const WIN32_FIND_DATA& wfd);

   // Implementation

   HRESULT _EnsureCFSDriveRef();
   HRESULT _DoPastePhotos(VFS_MENUCOMMAND& Cmd);
};
*/

///////////////////////////////////////////////////////////////////////////////
// CFSDriveItemAccount
/*
#if !defined(_M_X64) && !defined(_M_IA64)
#include <pshpack1.h>
#endif
typedef struct tagCFSDRIVEFS_PIDL_PHOTO
{
   // SHITEMID 
   USHORT cb;
   // Type identifiers
   BYTE magic;
   BYTE reserved;
   // Photo ID
   WCHAR cPhotoID[50];
} CFSDRIVEFS_PIDL_PHOTO;
#if !defined(_M_X64) && !defined(_M_IA64)
#include <poppack.h>
#endif


class CFSDriveItemPhoto : public CNseBaseItem
{
public:
   TCFSDrivePhoto* m_pPhoto;

   // Constructor 

   CFSDriveItemPhoto(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem);

   // CNseBaseItem

   BYTE GetType();
   VFS_FIND_DATA GetFindData();
   SFGAOF GetSFGAOF(SFGAOF dwMask);
   VFS_PROPSTATE GetPropertyState(REFPROPERTYKEY pkey);

   HRESULT GetExtractIcon(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetThumbnail(REFIID riid, LPVOID* ppRetVal);
   HRESULT GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v);
   HRESULT SetProperty(REFPROPERTYKEY pkey, const CComPropVariant& v);

   HMENU GetMenu();
   HRESULT SetMenuState(const VFS_MENUSTATE& State);
   HRESULT ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd);

   HRESULT Delete();
   HRESULT MarkForDelete();

   HRESULT GetStream(const VFS_STREAM_REASON& Reason, CNseFileStream** ppFile);

   // Static members

   static PCITEMID_CHILD GenerateITEMID(const CFSDRIVEFS_PIDL_PHOTO& src);
   static PCITEMID_CHILD GenerateITEMID(const TCFSDrivePhoto* pPhoto);
   static PCITEMID_CHILD GenerateITEMID(const WIN32_FIND_DATA& wfd);

   // Implementation

   HRESULT _EnsureCFSDriveRef();
   TCFSDriveImageset* _FindParentImageset() const;
};

*/
///////////////////////////////////////////////////////////////////////////////
// CFSDriveItemFileStream
/*
class CFSDriveItemFileStream : public CNseFileStream
{
public:
   VFS_STREAM_REASON m_Reason;
   CRefPtr<CShellFolder> m_spFolder;
   TCFSDriveImageset* m_pImageset;
   CString m_sPhotoID;
   HINTERNET m_hConnect;
   HINTERNET m_hRequest;
   DWORD m_dwCurPos;
   DWORD m_dwFileSize;
   DWORD m_cbPostFooter;
   CString m_sPostHeader;
   CString m_sPostFooter;
   CAutoVectorPtr<CHAR> m_aPostHeader;
   CAutoVectorPtr<CHAR> m_aPostFooter;

   // Constructor

   CFSDriveItemFileStream(const VFS_STREAM_REASON& Reason, CShellFolder* pFolder, TCFSDrivePhoto* pPhoto, TCFSDriveImageset* pImageset);
   virtual ~CFSDriveItemFileStream();

   // CNseFileStream

   HRESULT Init();
   HRESULT Read(LPVOID pData, ULONG dwSize, ULONG& dwRead);
   HRESULT Write(LPCVOID pData, ULONG dwSize, ULONG& dwWritten);
   HRESULT Seek(DWORD dwPos);
   HRESULT GetCurPos(DWORD* pdwPos);
   HRESULT GetFileSize(DWORD* pdwFileSize);
   HRESULT SetFileSize(DWORD dwSize);
   HRESULT Commit();
   HRESULT Close();

   // Implementation

   HRESULT _StartPhotoGetRequest();
   HRESULT _StartPhotoPostRequest();
   HRESULT _WriteFileHeader();
   CString _DetectContentType(const CString& sID) const;
};
*/
