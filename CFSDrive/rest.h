#pragma once

#include <atlstr.h>
#include <winhttp.h>
#include <iostream>
#include "json.h"

#define CFSDRIVE_URL_AUTH                _T("https://demo.cirrusfileserver.com/auth/requestToken")
#define CFSDRIVE_URL_UPLOAD              _T("https://up.cfsdrive.com/services/upload/")
#define CFSDRIVE_URL_REST                _T("https://demo.cirrusfileserver.com")
#define CFSDRIVE_LOGIN_AUTH				 _T("/auth/requestToken")

#define CFSDRIVE_URL_VERIFY_CALLBACK     _T("http://demo.cirrusfileserver.com/cfsdrive-oauth/")
#define CFSDRIVE_URL_VERIFY_TOKEN        _T("http://demo.cirrusfileserver.com/cfsdrive-oauth/")

#define CFSDRIVE_URLPART_STATIC          _T("demo.cirrusfileserver.com")

#define BROWSER_USERAGENT				 _T("Mozilla/5.0 (compatible; MSIE 11.0; Windows NT 10.0; CFSDrive)")
#define CFSDRIVE_PHOTO_EXTRAS            _T("date_taken,last_update")

#define CFSDRIVE_VERB_GET                _T("GET")
#define CFSDRIVE_VERB_POST               _T("POST")

#define REGPATH_CFSDRIVEFS               _T("SOFTWARE\\cirrus\\CFSDrive")
#define REGPATH_CFSDRIVEFS_SETTINGS      _T("\\Setting")
#define REGPATH_CFSDRIVEFS_ACCOUNTS      _T("\\Accounts")
#define REGPATH_CFSDRIVEFS_COLLECTIONS   _T("Collections")

#define CFSDRIVEFS_ACCOUNTSTR_PUBLIC     _T("<public>")
#define CFSDRIVEFS_ACCOUNTSTR_GUEST      _T("<guest>")

#define CFSDRIVEFS_FOLDERSTR_INTERESTING _T("<interesting>")
#define CFSDRIVEFS_FOLDERSTR_FAVOURITES  _T("<favourites>")
#define CFSDRIVEFS_FOLDERSTR_NOTINSET    _T("<notinset>")
#define CFSDRIVEFS_FOLDERSTR_HOTTAGS     _T("<hottags>")
#define CFSDRIVEFS_FOLDERSTR_RECENT      _T("<recent>")

typedef struct tagCFSDRIVE_APIARG
{
	LPCSTR pstrName;
	LPCTSTR pstrValue;
} CFSDRIVE_APIARG;


struct CAutoInternetHandle
{
	HINTERNET m_h;
	CAutoInternetHandle(HINTERNET h = NULL) : m_h(h) { };
	~CAutoInternetHandle() { if (m_h != NULL) ::WinHttpCloseHandle(m_h); };
	operator HINTERNET() const { return m_h; };
};

struct CAutoCryptContextHandle
{
	HCRYPTPROV m_h;
	CAutoCryptContextHandle(HCRYPTPROV h = NULL) : m_h(h) { };
	~CAutoCryptContextHandle() { if (m_h != NULL) ::CryptReleaseContext(m_h, 0UL); };
	operator HCRYPTPROV() const { return m_h; };
};

struct CAutoCryptHashHandle
{
	HCRYPTHASH m_h;
	CAutoCryptHashHandle(HCRYPTHASH h = NULL) : m_h(h) { };
	~CAutoCryptHashHandle() { if (m_h != NULL) ::CryptDestroyHash(m_h); };
	operator HCRYPTHASH() const { return m_h; };
};

struct CAutoCryptKeyHandle
{
	HCRYPTKEY m_h;
	CAutoCryptKeyHandle(HCRYPTKEY h = NULL) : m_h(h) { };
	~CAutoCryptKeyHandle() { if (m_h != NULL) ::CryptDestroyKey(m_h); };
	operator HCRYPTKEY() const { return m_h; };
};

struct CAutoHttpData
{
	LPBYTE m_p;
	DWORD m_dwSize;
	CAutoHttpData(LPBYTE p = NULL) : m_p(p) { }
	~CAutoHttpData() { Free(); };
	void Init(LPBYTE p, DWORD dwSize) { Free(); m_p = p; m_dwSize = dwSize; };
	void Free() { free(m_p); m_p = NULL; };
	operator LPBYTE() const { return m_p; };
	DWORD GetSize() const { m_dwSize; };
};

struct TCFSDriveAccount;

extern HRESULT DoDisplayErrorMessage(HWND hWnd, UINT nTitle, HRESULT Hr);

typedef enum CFSDRIVE_APITYPE { CFSDRIVE_APITYPE_REST, CFSDRIVE_APITYPE_UPLOAD, CFSDRIVE_APITYPE_AUTH, CFSDRIVE_APITYPE_VERIFY } CFSDRIVE_APITYPE;

typedef enum TCFSDriveAccountType
{
	CFSDRIVEFS_ACCOUNT_GUEST = 0,
	CFSDRIVEFS_ACCOUNT_OWNED = 1,
	CFSDRIVEFS_ACCOUNT_PUBLIC = 2,
} TCFSDriveAccountType;

typedef struct TCFSDriveAccount
{
	TCFSDriveAccount(){ }
	CString sTunantUrl;
	CString sUserID;
	CString sUserPassword;
	CString sClientID;
	CString sClientSecret;
	CString sGrantType;
	CString sAccessToken;
	CString sTokenType;
	CString sRefreshToken;
	TCFSDriveAccountType Access;

} TCFSDriveAccount;

typedef struct  TCFSDriveAPIToken
{
	char access_token[1024];
	char token_type[64];
	char refresh_token[128];
	char userName[64];
} TCFSDriveAPIToken;

typedef CSimpleValArray<TCFSDriveAccount*> CFSDriveAccountList;


class CFSDriveAPI
{
public:
	CFSDriveAccountList m_aAccounts;        // Account cache
	CAutoInternetHandle m_hInternet;        // WinHTTP handle
	CAutoCryptContextHandle m_hCrypt;       // MS Crypto handle
	int m_aPhotoHashmap[501];               // Hashmap for photo cache
	TCFSDriveAccount* m_pGuestAccount;      // Dummy account for system folders
	SYSTEMTIME m_stThisDay;                 // Last known date
	DWORD m_dwThisCount;                    // Count of queries today
	CComCriticalSection m_lock;             // Semaphore
	TCFSDriveAPIToken m_ApiToken;

	// Constructor / destructor
	CFSDriveAPI();
	~CFSDriveAPI();

	// Operations

	HRESULT Init();
	HRESULT InitHttpService();
	HRESULT WelcomeAccount(HWND hWnd);
	HRESULT AddAccount(LPCTSTR pstrTunnantUrl, LPCTSTR pstrUserID, LPCTSTR pstrPassword, LPCTSTR pstrClientID, LPCTSTR pstrClientSecret, LPCTSTR pstrGrantType, BOOL bCheckFirst);
	HRESULT CheckAccount(LPCTSTR pstrTunantUrl, LPCTSTR pstrUserID, LPCTSTR pstrUserPassword, LPCTSTR pstrClientID, LPCTSTR pstrClientSecret, LPCTSTR pstrGrantType);
	HRESULT CheckAccount2();
	
	// Implementation
	BOOL _IsAuthAccount() const;
	HRESULT _ReadAccountList();
	HRESULT _RefreshAccountCache();
	HRESULT _CreateAccountFolder(HKEY hKeyCollection, LPCTSTR pszFolderRegName, UINT uDisplayName);
	HRESULT GetPrimaryAccount();

	HRESULT _InitAPI();
	HRESULT _InitCrypto();
	// Account setup helpers

	HRESULT _DoDefaultAccountSetup(LPCTSTR strKey, LPCTSTR strValue);
	HRESULT _DoAccountWebAuthentication(HWND hWnd, TCFSDriveAccount& Account);
	// CFSDrive REST API

	HRESULT _CheckQueryQuota();
	HRESULT _SubmitCFSDriveRestAction(CFSDRIVE_APITYPE apiType, LPCTSTR pstrTunantUrl, LPCTSTR pstrFunction, LPCTSTR pstrVerb, TCFSDriveAccount* pAccount, CFSDRIVE_APIARG* pArgs, int nArgs, TCFSDriveAccount** ppAccount = NULL);
	CString _RestBuildActionUrl(CFSDRIVE_APITYPE ApiType, LPCTSTR pstrFunctionName, LPCTSTR pstrVerb, const TCFSDriveAccount* pAccount, const CFSDRIVE_APIARG* pArgs, int nArgs);
	HRESULT _RestSendRequest(LPCTSTR pstrUrl, LPCTSTR pstrVerb, DWORD dwFlags, LPSTR pstrPost, DWORD cbPost, CAutoHttpData& Data);
	HRESULT _RestExtractResult(LPCSTR pstrXML, IXMLDOMDocument** ppDoc);
	HRESULT _XmlExtractError(CComPtr<IXMLDOMDocument>& spDoc);
	HRESULT _XmlGetResultNode(IUnknown* pUnk, LPCWSTR pstrPattern, CString& sValue);
	HRESULT _XmlGetResultList(IUnknown* pUnk, LPCWSTR pstrPattern, IXMLDOMNodeList** ppList, long* pCount);
	HRESULT _XmlGetResultList(IUnknown* pUnk, LPCWSTR pstrPattern, LPCWSTR pstrAttribute, CSimpleArray<CString>& aList);
	HRESULT _FormGetValue(LPCTSTR pstrVariable, const CAutoHttpData& data, CString& sResult) const;
	BOOL _MD5Hash(LPCSTR data, LPTSTR pstrResult);
	CString _HMACSHA1(const CString& sText, const CString& sKey);
	DWORD _UInt4Hash(LPCWSTR pstr) const;

	HRESULT JsonExtractResult(LPCSTR pstrJson);
};


extern CFSDriveAPI _CFSDriveAPI;
