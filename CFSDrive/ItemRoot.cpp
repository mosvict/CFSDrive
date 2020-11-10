
#include "pch.h"

#include "FileSystem.h"
#include "ShellFolder.h"
#include "PropSheetExt.h"


///////////////////////////////////////////////////////////////////////////////
// CFSDriveItemRoot

CFSDriveItemRoot::CFSDriveItemRoot(CShellFolder* pFolder) :
   CNseBaseItem(pFolder, NULL, NULL, FALSE)
{
}

BYTE CFSDriveItemRoot::GetType()
{
   return CFSDRIVE_MAGIC_ID_ROOT;
}

SFGAOF CFSDriveItemRoot::GetSFGAOF(SFGAOF dwMask)
{
   return SFGAO_FOLDER
          | SFGAO_CANCOPY
          | SFGAO_BROWSABLE
          | SFGAO_HASSUBFOLDER
          | SFGAO_HASPROPSHEET
          | SFGAO_DROPTARGET
          | SFGAO_STORAGE
          | SFGAO_STORAGEANCESTOR
          | SFGAO_FILESYSANCESTOR;
}

HRESULT CFSDriveItemRoot::GetColumnInfo(UINT iColumn, VFS_COLUMNINFO& Info)
{
   static VFS_COLUMNINFO aColumns[] = {
      { PKEY_ItemNameDisplay,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_ItemPathDisplay,            SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },
      { PKEY_Contact_FullName,           SHCOLSTATE_TYPE_STR  | SHCOLSTATE_ONBYDEFAULT,                    0 },
      { PKEY_Contact_PrimaryAddressCity, SHCOLSTATE_TYPE_STR  | SHCOLSTATE_SECONDARYUI,                    0 },      
      { PKEY_FileName,                   SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_FileAttributes,             SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ParsingPath,                SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_ItemPathDisplayNarrow,      SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         0 },
      { PKEY_SFGAOFlags,                 SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_Volume_IsRoot,              SHCOLSTATE_TYPE_INT  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_InfoTip,           SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_TileInfo,          SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_FullDetails,       SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_PreviewTitle,      SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_PreviewDetails,    SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
      { PKEY_PropList_ExtendedTileInfo,  SHCOLSTATE_TYPE_STR  | SHCOLSTATE_HIDDEN,                         VFS_COLF_NOTCOLUMN },
   };
   if( iColumn >= lengthof(aColumns) ) 
      return E_FAIL;
   Info = aColumns[iColumn];
   return S_OK;
}

HRESULT CFSDriveItemRoot::GetPropertyPage(/*CNsePropertyPage** ppPage*/)
{
   //*ppPage = new CNseFileRootPropertyPage();
   //return *ppPage != NULL ? S_OK : E_OUTOFMEMORY;
	return S_OK;
}

HRESULT CFSDriveItemRoot::GetExtractIcon(REFIID riid, LPVOID* ppRetVal)
{
   return ::SHCreateFileExtractIcon(_T("Folder"), FILE_ATTRIBUTE_DIRECTORY, riid, ppRetVal);
}

HRESULT CFSDriveItemRoot::IsDropDataAvailable(IDataObject* pDataObj)
{
   // We support file drops for the Shell SendTo feature
   return DataObj_HasFileClipFormat(pDataObj) ? S_OK : S_FALSE;
}

VFS_FIND_DATA CFSDriveItemRoot::GetFindData()
{
   VFS_FIND_DATA wfd = { 0 };
   wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
   return wfd;
}

VFS_FOLDERSETTINGS CFSDriveItemRoot::GetFolderSettings()
{
   VFS_FOLDERSETTINGS Settings = { 0 };
   Settings.ViewMode = FLVM_ICONS;
   Settings.cxyIcon = 64;
   return Settings;
}

HRESULT CFSDriveItemRoot::GetProperty(REFPROPERTYKEY pkey, CComPropVariant& v)
{
   if( pkey == PKEY_ItemNameDisplay ) {
	   TCFSDriveAccount* pAccount = NULL;
      //HR( _CFSDriveAPI.GetPrimaryAccount(&pAccount) );
      return ::InitPropVariantFromString(pAccount->sUserID, &v);
   }
   if( pkey == PKEY_ItemTypeText ) {
      return ::InitPropVariantFromString(L"CFSDrive Drive", &v);
   }
   if( pkey == PKEY_Volume_IsRoot ) {
      return ::InitPropVariantFromBoolean(TRUE, &v);
   }
   // In the Property Page, the File System name is displayed
   if( pkey == PKEY_Volume_FileSystem ) {
      return ::InitPropVariantFromString(L"CFSDriveFS", &v);
   }
   // Our default root Property Page will show a pie chart with capacity/free space
   //if( pkey == PKEY_Capacity ) {
   //   HR( _CFSDriveAPI.ReadUploadStatus() );
   //   return ::InitPropVariantFromUInt64(_CFSDriveAPI.m_VolumeInfo.ullUploadMaxBytes, &v);
   //}
   //if( pkey == PKEY_FileAllocationSize ) {
   //   HR( _CFSDriveAPI.ReadUploadStatus() );
   //   return ::InitPropVariantFromUInt64(_CFSDriveAPI.m_VolumeInfo.ullUploadUsedBytes, &v);
   //}
   //if( pkey == PKEY_FreeSpace ) {
   //   HR( _CFSDriveAPI.ReadUploadStatus() );
   //   return ::InitPropVariantFromUInt64(_CFSDriveAPI.m_VolumeInfo.ullUploadRemainingBytes, &v);
   //}
   //if( pkey == PKEY_PercentFull ) {
   //   HR( _CFSDriveAPI.ReadUploadStatus() );
   //   if( _CFSDriveAPI.m_VolumeInfo.ullUploadMaxBytes == 0 ) 
   //      return ::InitPropVariantFromUInt32(0, &v);
   //   return ::InitPropVariantFromUInt32((ULONG)((DOUBLE)_CFSDriveAPI.m_VolumeInfo.ullUploadRemainingBytes / (DOUBLE)_CFSDriveAPI.m_VolumeInfo.ullUploadMaxBytes * 100.0), &v);
   //}
   // Define what information to display in tool tip and tile information
   if( pkey == PKEY_PropList_TileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.Volume.FileSystem;", &v);
   }
   if( pkey == PKEY_PropList_ExtendedTileInfo ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;", &v);
   }
   if( pkey == PKEY_PropList_PreviewTitle ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.ItemTypeText;", &v);
   }
   if( pkey == PKEY_PropList_PreviewDetails ) {
      return ::InitPropVariantFromString(L"prop:System.Volume.FileSystem;", &v);
   }
   if( pkey == PKEY_PropList_InfoTip ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;Volume.FileSystem;", &v);
   }
   if( pkey == PKEY_PropList_FullDetails ) {
      return ::InitPropVariantFromString(L"prop:System.ItemNameDisplay;System.PercentFull;System.Volume.FileSystem;", &v);
   }
   return CNseBaseItem::GetProperty(pkey, v);
}

CNseItem* CFSDriveItemRoot::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, PCITEMID_CHILD pidlItem, BOOL bReleaseItem)
{
	if (!CPidlMemPtr<CFSDRIVEFS_PIDL_ACCOUNT>(pidlItem).IsType(CFSDRIVE_MAGIC_ID_ACCOUNT))
		return NULL;
	return new CFSDriveItemAccount(pFolder, pidlFolder, pidlItem, bReleaseItem);
}

CNseItem* CFSDriveItemRoot::GenerateChild(CShellFolder* pFolder, PCIDLIST_RELATIVE pidlFolder, const WIN32_FIND_DATA wfd)
{
   return new CFSDriveItemAccount(pFolder, pidlFolder, CFSDriveItemAccount::GenerateITEMID(wfd), TRUE);
}

HRESULT CFSDriveItemRoot::GetChild(LPCWSTR pwstrName, SHGNO ParseType, CNseItem** ppItem)
{
   CFSDriveAccountList aList;
   //HR( _CFSDriveAPI.GetAccountList(aList) );
   //for( int i = 0; i < aList.GetSize(); i++ ) {
   //   const TCFSDriveAccount* pAccount = aList[i];
   //   if( pAccount->sNSID == pwstrName || pAccount->sUserName == pwstrName ) {
   //      *ppItem = new CFSDriveItemAccount(m_pFolder, m_pidlFolder, CFSDriveItemAccount::GenerateITEMID(pAccount), TRUE);
   //      return *ppItem != NULL ? S_OK : E_OUTOFMEMORY;
   //   }
   //}
   //return E_FAIL;
   return S_OK;
}

HRESULT CFSDriveItemRoot::EnumChildren(HWND hwndOwner, SHCONTF grfFlags, CSimpleValArray<CNseItem*>& aItems)
{
   if( !IsBitSet(grfFlags, SHCONTF_FOLDERS|SHCONTF_STORAGE) ) 
      return S_OK;

	HR(_CFSDriveAPI._ReadAccountList());
	
	if (!_CFSDriveAPI._IsAuthAccount()) {		
		HR(_CFSDriveAPI.WelcomeAccount(hwndOwner));
	}
	else {
		OutputDebugString(L">>>>> Account Exist");
		//HRESULT Hr = _CFSDriveAPI.CheckAccount2();
		//if (FAILED(Hr)) {
		//	HR(_CFSDriveAPI.WelcomeAccount(hwndOwner));
		//}
	}
	return S_OK;
}

HMENU CFSDriveItemRoot::GetMenu()
{
   //return ::LoadMenu(_pModule->GetResourceInstance(), MAKEINTRESOURCE(IDM_ROOT));
	return NULL;
}

HRESULT CFSDriveItemRoot::ExecuteMenuCommand(VFS_MENUCOMMAND& Cmd)
{
   HRESULT Hr = E_NOTIMPL;
   switch( Cmd.wMenuID ) {
   //case ID_FILE_PROPERTIES:   Hr = _DoShowProperties(Cmd); break;
   //case ID_FILE_NEW_USER:     Hr = _CFSDriveAPI.AddAccountWithSimplePrompt(Cmd.hWnd); break;
   //case ID_COMMAND_ADD_USER:  Hr = _CFSDriveAPI.AddAccountWithSimplePrompt(Cmd.hWnd); break;
   //case DFM_CMD_NEWFOLDER:    Hr = _CFSDriveAPI.AddAccountWithSimplePrompt(Cmd.hWnd); break;
   //case DFM_CMD_PASTE:        Hr = _DoPasteSendTo(Cmd); break;
   }
   if( SUCCEEDED(Hr) ) _RefreshFolderView();
   return Hr;
}

// Implementation

//HRESULT CFSDriveItemRoot::_DoPasteSendTo(VFS_MENUCOMMAND& Cmd)
//{
//   if( Cmd.pDataObject == NULL ) 
//      return E_FAIL;
//   // We can't paste photos directly to the root, but must find an appropriate imageset folder. We'll try
//   // the primary account's "NotInSet" folder.
//   TCFSDriveAccount* pAccount = NULL;
//   TCFSDriveImageset* pImageset = NULL;
//   HR( _CFSDriveAPI.GetPrimaryAccount(&pAccount) );
//   HR( _CFSDriveAPI.FindImageset(pAccount, CFSDRIVEFS_FOLDERSTR_NOTINSET, &pImageset) );
//   // Get a IShellItem for the target folder...
//   WCHAR wszTarget[100] = { 0 };
//   ::wnsprintf(wszTarget, lengthof(wszTarget) - 1, L"%s\\%s", pAccount->sNSID, pImageset->sImagesetID);
//   CComPtr<IShellItem> spTargetFolder;
//   HR( ::SHCreateItemFromRelativeName(m_pFolder, wszTarget, NULL, IID_PPV_ARGS(&spTargetFolder)) );
//   // Do paste operation...
//   if( Cmd.pFO == NULL ) {
//      HR( ::SHCreateFileOperation(Cmd.hWnd, FOF_NOCOPYSECURITYATTRIBS | FOF_NOCONFIRMATION | FOFX_NOSKIPJUNCTIONS, &Cmd.pFO) );
//   }
//   Cmd.pFO->CopyItems(Cmd.pDataObject, spTargetFolder);
//   if( Cmd.dwDropEffect == DROPEFFECT_MOVE ) 
//      Cmd.pFO->DeleteItems(Cmd.pDataObject);
//   // We handled this operation successfully for all items in selection
//   return NSE_S_ALL_DONE;
//}

