
#include "pch.h"

#include "FileSystem.h"


///////////////////////////////////////////////////////////////////////////////
// CFSDriveModule

BOOL CFSDriveModule::GetConfigBool(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_INSTALL_SENDTO:
   case VFS_INSTALL_PREVIEW:
   case VFS_INSTALL_PROPSHEET:
   case VFS_INSTALL_DROPTARGET:
   case VFS_INSTALL_CONTEXTMENU:
   case VFS_INSTALL_CUSTOMSCRIPT:
   case VFS_INSTALL_STARTMENU_LINK:
      return TRUE;

   case VFS_CAN_SLOW_COPY:
   case VFS_CAN_SLOW_ENUM:
   case VFS_CAN_PROGRESSUI:
   case VFS_CAN_ATTACHMENTSERVICES:
      return TRUE;

   case VFS_HAVE_UNIQUE_NAMES:
   case VFS_HAVE_VIRTUAL_FILES:
      return TRUE;

   }
   return FALSE;
}

LONG CFSDriveModule::GetConfigInt(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_INT_LOCATION:
      return VFS_LOCATION_MYCOMPUTER;

   case VFS_INT_MAX_FILENAME_LENGTH:
      return MAX_PATH;

   case VFS_INT_MAX_PATHNAME_LENGTH:
      return MAX_PATH;

   case VFS_INT_SHELLROOT_SFGAO:
       return SFGAO_CANCOPY 
              | SFGAO_CANMOVE 
              | SFGAO_CANRENAME 
              | SFGAO_DROPTARGET
              | SFGAO_STREAM
              | SFGAO_FOLDER 
              | SFGAO_BROWSABLE 
              | SFGAO_HASSUBFOLDER 
              | SFGAO_HASPROPSHEET 
              | SFGAO_FILESYSANCESTOR
              | SFGAO_STORAGEANCESTOR;

   }
   return 0;
}

LPCWSTR CFSDriveModule::GetConfigStr(VFS_CONFIG Item)
{
   switch( Item ) {

   case VFS_STR_FILENAME_CHARS_NOTALLOWED:
      return L":<>\\/|\"'*?[]";

   }
   return NULL;
}

HRESULT CFSDriveModule::DllInstall()
{
   return S_OK;
}

HRESULT CFSDriveModule::DllUninstall()
{
   return S_OK;
}

HRESULT CFSDriveModule::ShellAction(LPCWSTR pstrType, LPCWSTR pstrCmdLine)
{
   return S_OK;
}

BOOL CFSDriveModule::DllMain(DWORD dwReason, LPVOID lpReserved)
{
   return TRUE;
}

HRESULT CFSDriveModule::CreateFileSystem(PCIDLIST_ABSOLUTE pidlRoot, CNseFileSystem** ppFS)
{
   CFSDriveFileSystem* pFS = new CFSDriveFileSystem();
   HRESULT Hr = pFS->Init();
   if( SUCCEEDED(Hr) ) *ppFS = pFS;
   else delete pFS; 
   return Hr;
}


///////////////////////////////////////////////////////////////////////////////
// CFSDriveFileSystem

CFSDriveFileSystem::CFSDriveFileSystem() : m_cRef(1)
{
}

CFSDriveFileSystem::~CFSDriveFileSystem()
{
}

HRESULT CFSDriveFileSystem::Init()
{
   HR( _CFSDriveAPI.Init() );
   return S_OK;
}

VOID CFSDriveFileSystem::AddRef()
{
   ::InterlockedIncrement(&m_cRef);
}

VOID CFSDriveFileSystem::Release()
{
   if( ::InterlockedDecrement(&m_cRef) == 0 ) 
      delete this;
}

CNseItem* CFSDriveFileSystem::GenerateRoot(CShellFolder* pFolder)
{
   return new CFSDriveItemRoot(pFolder);
}

