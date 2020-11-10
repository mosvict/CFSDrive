#include "pch.h"
#include "rest.h"
#include "MainDlg.h"
#include <time.h>
#include <iostream>

//CFSDriveAPI _CFSDriveAPI;

CFSDriveAPI::CFSDriveAPI() : m_pGuestAccount(NULL)
{
}

CFSDriveAPI::~CFSDriveAPI()
{
	if (m_pGuestAccount != NULL)
		delete m_pGuestAccount;
}

HRESULT CFSDriveAPI::Init()
{
	// Poor-mans semaphore
	static volatile bool s_bInited = false;
	if (s_bInited)
		return S_OK;
	s_bInited = true;

	m_lock.Init();               // Create thread lock

	HR(_InitAPI());            // Clear stuff

	HR(_InitCrypto());         // Prepare Windows Crypto accessnd
	
	_ReadAccountList();

	memset(&m_ApiToken, 0, sizeof(m_ApiToken));

	return S_OK;
}

HRESULT CFSDriveAPI::InitHttpService()
{
	if (m_hInternet != NULL)
		return S_OK;
	CComCritSecLock<CComCriticalSection> lock(m_lock);
	if (m_hInternet != NULL)
		return S_OK;
	ATLTRACE(L"CFSDrive::InitHttpService\n");
	HRESULT Hr = S_OK;
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG Proxy = { 0 };
	BOOL bRes = ::WinHttpGetIEProxyConfigForCurrentUser(&Proxy);
	if (!bRes || Proxy.fAutoDetect || Proxy.lpszAutoConfigUrl != NULL) {
		HINTERNET hProxy = ::WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (hProxy != NULL) {
			WINHTTP_AUTOPROXY_OPTIONS AutoProxy = { 0 };
			if (Proxy.fAutoDetect) {
				AutoProxy.dwFlags |= WINHTTP_AUTOPROXY_AUTO_DETECT;
				AutoProxy.dwAutoDetectFlags |= WINHTTP_AUTO_DETECT_TYPE_DHCP;
			}
			if (Proxy.lpszAutoConfigUrl != NULL) {
				AutoProxy.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
				AutoProxy.lpszAutoConfigUrl = Proxy.lpszAutoConfigUrl;
			}
			if (!bRes) {
				AutoProxy.dwFlags |= WINHTTP_AUTOPROXY_AUTO_DETECT;
				AutoProxy.dwAutoDetectFlags |= WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
			}
			AutoProxy.fAutoLogonIfChallenged = TRUE;
			WINHTTP_PROXY_INFO ProxyInfo = { 0 };
			if (::WinHttpGetProxyForUrl(hProxy, L"http://demo.cirrusfileserver.com", &AutoProxy, &ProxyInfo)) {
				if (ProxyInfo.lpszProxy != NULL) Proxy.lpszProxy = ProxyInfo.lpszProxy;
				if (ProxyInfo.lpszProxyBypass != NULL) ::GlobalFree(ProxyInfo.lpszProxyBypass);
			}
			::WinHttpCloseHandle(hProxy);
		}
	}
	m_hInternet.m_h = ::WinHttpOpen(BROWSER_USERAGENT, Proxy.lpszProxy == NULL ? WINHTTP_ACCESS_TYPE_DEFAULT_PROXY : WINHTTP_ACCESS_TYPE_NAMED_PROXY, Proxy.lpszProxy, Proxy.lpszProxyBypass, 0);
	if (m_hInternet == NULL)
		Hr = AtlHresultFromLastError();
	if (Proxy.lpszProxy != NULL)
		::GlobalFree(Proxy.lpszProxy);
	if (Proxy.lpszProxyBypass != NULL)
		::GlobalFree(Proxy.lpszProxyBypass);
	if (Proxy.lpszAutoConfigUrl != NULL)
		::GlobalFree(Proxy.lpszAutoConfigUrl);
	return Hr;
}


HRESULT CFSDriveAPI::_CreateAccountFolder(HKEY hKeyCollection, LPCTSTR pszFolderRegName, UINT uDisplayName)
{
	if (hKeyCollection == NULL)
		return E_FAIL;
	CRegKey regFolder;
	if (regFolder.Create(hKeyCollection, pszFolderRegName) != ERROR_SUCCESS)
		return E_ACCESSDENIED;
	CComBSTR bstrName; bstrName.LoadString(uDisplayName);
	regFolder.SetStringValue(_T("Name"), bstrName);
	//regFolder.SetDWORDValue(_T("Type"), ImagesetType);
	regFolder.Close();
	return S_OK;
}

HRESULT CFSDriveAPI::_DoDefaultAccountSetup(LPCTSTR strKey, LPCTSTR strValue)
{
	// Create account in Registry database...
	CRegKey regAccount;
	//if (regAccount.Create(HKEY_CURRENT_USER, REGPATH_CFSDRIVEFS REGPATH_CFSDRIVEFS_ACCOUNTS) != ERROR_SUCCESS)
	//	return E_ACCESSDENIED;
	//if (regAccount.Open(HKEY_CURRENT_USER, REGPATH_CFSDRIVEFS REGPATH_CFSDRIVEFS_ACCOUNTS, KEY_WRITE) != ERROR_SUCCESS)
	//	return E_ACCESSDENIED;
	regAccount.Create(HKEY_CURRENT_USER, REGPATH_CFSDRIVEFS);
	regAccount.SetStringValue(strKey, strValue);
	regAccount.Close();
	
	return S_OK;
}


HRESULT CFSDriveAPI::_RefreshAccountCache()
{
	CComCritSecLock<CComCriticalSection> lock(m_lock);
	m_aAccounts.RemoveAll();

	// Re-read the account list and match up photos with new account entries
	return S_OK;
}

HRESULT CFSDriveAPI::AddAccount(LPCTSTR pstrTunnantUrl, LPCTSTR pstrUserID, LPCTSTR pstrPassword, LPCTSTR pstrClientID, LPCTSTR pstrClientSecret, LPCTSTR pstrGrantType, BOOL bCheckFirst)
{
	HR(_DoDefaultAccountSetup(_T("UserID"), pstrUserID));
	HR(_DoDefaultAccountSetup(_T("Password"), pstrPassword));
	HR(_DoDefaultAccountSetup(_T("ClientID"), pstrClientID));
	HR(_DoDefaultAccountSetup(_T("ClientSecret"), pstrClientSecret));
	HR(_DoDefaultAccountSetup(_T("GrantType"), pstrGrantType));
	return S_OK;
}

HRESULT CFSDriveAPI::WelcomeAccount(HWND hWnd)
{
	ATLASSERT(::IsWindow(hWnd));
	// Call this method only when we haven't set up an account yet
	//ATLASSERT(m_aAccounts.GetSize() == 0);
	// Let user create the first account...
	CMainDlg dlg;
	if (dlg.DoModal(hWnd) != IDOK)
		return E_ABORT;
	return S_OK;
}


BOOL CFSDriveAPI::_IsAuthAccount() const
{
	if (m_pGuestAccount->sAccessToken.IsEmpty())
		return FALSE;
	else
		return TRUE;
}

HRESULT CFSDriveAPI::_ReadAccountList()
{
	OutputDebugString(L">>>>ReadAccountList - Start");
	m_pGuestAccount = new TCFSDriveAccount();
	::SetErrorInfo(0, NULL);
	CComCritSecLock<CComCriticalSection> lock(m_lock);
	CRegKey regAccount;
	if (regAccount.Open(HKEY_CURRENT_USER, REGPATH_CFSDRIVEFS, KEY_READ) != ERROR_SUCCESS)
	{
		OutputDebugString(L"ReadAccount Failed.");
		return S_FALSE;
	}

	TCHAR sUserID[64] = { 0 };
	TCHAR sUserPassword[64] = { 0 };
	TCHAR sClientID[128] = { 0 };
	TCHAR sClientSecret[128] = { 0 };
	TCHAR sGrantType[80] = { 0 };
	TCHAR sAccessToken[1024] = { 0 };
	TCHAR sTokenType[128] = { 0 };
	TCHAR sRefreshToken[128] = { 0 };

	ULONG ccsUserID = lengthof(sUserID);
	ULONG ccsUserPassword = lengthof(sUserPassword);
	ULONG ccsClientID = lengthof(sClientID);
	ULONG ccsClientSecret = lengthof(sClientSecret);
	ULONG ccsGrantType = lengthof(sGrantType);
	ULONG ccsAccessToken = lengthof(sAccessToken);
	ULONG ccsTokenType = lengthof(sTokenType);
	ULONG ccsRefreshToken = lengthof(sRefreshToken);
	DWORD dwIsPrimary = 0, dwAccess = 0;

	regAccount.QueryStringValue(_T("UserID"), sUserID, &ccsUserID);
	regAccount.QueryStringValue(_T("Password"), sUserPassword, &ccsUserPassword);
	regAccount.QueryStringValue(_T("ClientID"), sClientID, &ccsClientID);
	regAccount.QueryStringValue(_T("ClientSecret"), sClientSecret, &ccsClientSecret);
	regAccount.QueryStringValue(_T("GrantType"), sGrantType, &ccsGrantType);
	regAccount.QueryStringValue(_T("AccessToken"), sAccessToken, &ccsAccessToken);
	regAccount.QueryStringValue(_T("TokenType"), sTokenType, &ccsTokenType);
	regAccount.QueryStringValue(_T("RefreshToken"), sRefreshToken, &ccsRefreshToken);
	regAccount.QueryDWORDValue(_T("Access"), dwAccess);

	m_pGuestAccount->sUserID = sUserID;
	m_pGuestAccount->sUserPassword = sUserPassword;
	m_pGuestAccount->sClientID = sClientID;
	m_pGuestAccount->sClientSecret = sClientSecret;
	m_pGuestAccount->sGrantType = sGrantType;
	m_pGuestAccount->sAccessToken = sAccessToken;
	m_pGuestAccount->sTokenType = sTokenType;
	m_pGuestAccount->sRefreshToken = sRefreshToken;
	m_pGuestAccount->Access = (TCFSDriveAccountType)dwAccess;

	OutputDebugString(_T(">>>>ReadAccountList"));
	OutputDebugString(sUserID);
	OutputDebugString(sUserPassword);
	OutputDebugString(sClientID);
	OutputDebugString(sClientSecret);
	OutputDebugString(sGrantType);
	OutputDebugString(sAccessToken);
	OutputDebugString(sTokenType);
	OutputDebugString(sRefreshToken);

	regAccount.Close();
	return S_OK;
}

HRESULT CFSDriveAPI::GetPrimaryAccount()
{
	m_pGuestAccount = new TCFSDriveAccount();
	CComCritSecLock<CComCriticalSection> lock(m_lock);
	for (int i = 0; i < m_aAccounts.GetSize(); i++) {
		if (m_aAccounts[i]->Access) {
			m_pGuestAccount = m_aAccounts[i];
			return S_OK;
		}
	}
	return AtlHresultFromWin32(ERROR_NOT_AUTHENTICATED);
}
HRESULT CFSDriveAPI::CheckAccount2()
{
	HR(CheckAccount(m_pGuestAccount->sTunantUrl,
		m_pGuestAccount->sUserID,
		m_pGuestAccount->sUserPassword,
		m_pGuestAccount->sClientID,
		m_pGuestAccount->sClientSecret,
		m_pGuestAccount->sGrantType));
	return S_OK;
}

HRESULT CFSDriveAPI::CheckAccount(LPCTSTR pstrTunantUrl, LPCTSTR pstrUserID, LPCTSTR pstrUserPassword, LPCTSTR pstrClientID, LPCTSTR pstrClientSecret, LPCTSTR pstrGrantType)
{
	// Create account...
	TCFSDriveAccount Account;
	Account.sTunantUrl = pstrTunantUrl;
	Account.sUserID = pstrUserID;
	Account.sUserPassword = pstrUserPassword;
	Account.sClientID = pstrClientID;
	Account.sClientSecret = pstrClientSecret;
	Account.sGrantType = pstrGrantType;
	Account.Access = CFSDRIVEFS_ACCOUNT_OWNED;

	CFSDRIVE_APIARG ApiPeopleFindByUsername[] = {
		{ "username",		pstrUserID },
		{ "password",		pstrUserPassword},
		{ "client_id",		pstrClientID},
		{ "client_secret",	pstrClientSecret}, 
		{ "grant_type",		pstrGrantType}
	};
	
	if (FAILED(_SubmitCFSDriveRestAction(	CFSDRIVE_APITYPE_AUTH, 
											pstrTunantUrl, 
											_T("/auth/requestToken"), 
											CFSDRIVE_VERB_POST, 
											&Account, 
											ApiPeopleFindByUsername, 
											lengthof(ApiPeopleFindByUsername))))
		return AtlHresultFromWin32(ERROR_BAD_USER_PROFILE);

	return S_OK;
}

HRESULT CFSDriveAPI::_InitAPI()
{
	::GetLocalTime(&m_stThisDay);
	m_dwThisCount = 0;
	return S_OK;
}

HRESULT CFSDriveAPI::_InitCrypto()
{
	if (!::CryptAcquireContext(&m_hCrypt.m_h, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET))
		return AtlHresultFromLastError();
	return S_OK;
}


// CFSDrive REST API

HRESULT CFSDriveAPI::_SubmitCFSDriveRestAction(	CFSDRIVE_APITYPE apiType, 
												LPCTSTR pstrTunantUrl, 
												LPCTSTR pstrFunction, 
												LPCTSTR pstrVerb, 
												TCFSDriveAccount* pAccount, 
												CFSDRIVE_APIARG* pArgs, 
												int nArgs, 
												TCFSDriveAccount** ppAccount /*= NULL*/)
{
	ATLTRACE(L"CFSDriveAPI::SubmitCFSDriveRestAction  method='%s'\n", pstrFunction);
	// Reset COM error
	::SetErrorInfo(0, NULL);

	// Build CFSDrive url and submit HTTP request
	CString sURL = _RestBuildActionUrl(apiType, pstrFunction, pstrVerb, pAccount, pArgs, nArgs);
	OutputDebugString(sURL);
	CAutoHttpData data;	
	if (apiType == CFSDRIVE_APITYPE_AUTH) {
		OutputDebugString(_T(">>CFSDRIVE_APITYPE_AUTH"));
		char sParams[1024];
		ZeroMemory(sParams, sizeof(sParams));
		::WideCharToMultiByte(CP_UTF8, NULL, (const wchar_t*)sURL.GetBuffer(), -1, sParams, sURL.GetLength(), NULL, NULL);
		sParams[_tcslen(sURL)] = '\0';
		OutputDebugStringA(sParams);
		HR(_RestSendRequest(CFSDRIVE_URL_AUTH, pstrVerb, 0, sParams, strlen(sParams), data));
	}
	else {
		HR(_RestSendRequest(sURL, pstrVerb, 0, NULL, 0, data));
	}


	HR(AddAccount(pAccount->sTunantUrl, pAccount->sUserID, pAccount->sUserPassword, pAccount->sClientID, pAccount->sClientSecret, pAccount->sGrantType, FALSE));
	//LPCSTR wStr = (char*)data.m_p;
	//OutputDebugStringA(wStr);
	HR(JsonExtractResult(reinterpret_cast<LPSTR>(data.m_p)));
	return S_OK;
}

HRESULT CFSDriveAPI::JsonExtractResult(LPCSTR pstrJson)
{
	TCHAR szBuf[1024] = {'\0'};
	using json = nlohmann::json;
	auto strJson = (char*)pstrJson;
	auto jsonData = json::parse(strJson);

	if (!jsonData.is_null()) {
		strcpy(m_ApiToken.access_token, jsonData["access_token"].get<std::string>().c_str());
		strcpy(m_ApiToken.token_type, jsonData["token_type"].get<std::string>().c_str());
		strcpy(m_ApiToken.refresh_token, jsonData["refresh_token"].get<std::string>().c_str());
		strcpy(m_ApiToken.userName, jsonData["userName"].get<std::string>().c_str());

		::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, m_ApiToken.access_token, -1, szBuf, strlen(m_ApiToken.access_token));
		HR(_DoDefaultAccountSetup(_T("AccessToken"), szBuf));
		OutputDebugString(szBuf);
		memset(szBuf, 0, 1024);
		::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, m_ApiToken.token_type, -1, szBuf, strlen(m_ApiToken.token_type));
		HR(_DoDefaultAccountSetup(_T("TokenType"), szBuf));
		OutputDebugString(szBuf);
		memset(szBuf, 0, 1024);
		::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, m_ApiToken.refresh_token, -1, szBuf, strlen(m_ApiToken.refresh_token));
		HR(_DoDefaultAccountSetup(_T("RefreshToken"), szBuf));
		OutputDebugString(szBuf);
	}

	OutputDebugStringA(">>>>>>>>>>JsonExtractResult");
	OutputDebugStringA(m_ApiToken.access_token);
	OutputDebugStringA(m_ApiToken.token_type);
	OutputDebugStringA(m_ApiToken.refresh_token);
	OutputDebugStringA(m_ApiToken.userName);
	return S_OK;
}

static CString UrlEncode(CString s)
{
	CW2AEX<> str(s, CP_UTF8);
	size_t cchText = strlen(str);
	CString sResult, sTemp;
	for (size_t i = 0; i < cchText; i++) {
		TCHAR ch = str[i];
		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || _tcschr(_T("0123456789-_.~"), ch) != NULL)  // RFC-3986
			sResult += ch;
		else {
			sTemp.Format(_T("%%%02X"), (unsigned char)ch);
			sResult += sTemp;
		}
	}
	return sResult;
}

CString CFSDriveAPI::_HMACSHA1(const CString& sText, const CString& sKey)
{
	// Key creation based on:
	// http://brookmiles.ca/2010/07/14/twitter-oauth-in-c-for-win32-part-2/
	CW2AEX<> strText(sText, CP_UTF8);
	CW2AEX<> strKey(sKey);
	HMAC_INFO HmacInfo = { 0 };
	HmacInfo.HashAlgid = CALG_SHA1;
	CAutoCryptHashHandle hHash;
	if (!::CryptCreateHash(m_hCrypt, CALG_SHA1, 0, 0, &hHash.m_h)) return _T("");
	if (!::CryptHashData(hHash, (LPBYTE) static_cast<LPCSTR>(strKey), (DWORD)strlen(strKey), 0)) return _T("");
	struct {
		BLOBHEADER hdr;
		DWORD len;
		BYTE key[1024];
	} key_blob;
	key_blob.hdr.bType = PLAINTEXTKEYBLOB;
	key_blob.hdr.bVersion = CUR_BLOB_VERSION;
	key_blob.hdr.reserved = 0;
	key_blob.hdr.aiKeyAlg = CALG_RC2;
	key_blob.len = (DWORD)strlen(strKey);
	::ZeroMemory(key_blob.key, sizeof(key_blob.key));
	_ASSERTE(strlen(strKey) <= sizeof(key_blob.key));
	::CopyMemory(key_blob.key, static_cast<LPCSTR>(strKey), min(strlen(strKey), sizeof(key_blob.key)));
	CAutoCryptKeyHandle hKey;
	if (!::CryptImportKey(m_hCrypt, (LPBYTE)&key_blob, sizeof(key_blob), 0, CRYPT_IPSEC_HMAC_KEY, &hKey.m_h)) return _T("");
	CAutoCryptHashHandle hHmacHash;
	if (!::CryptCreateHash(m_hCrypt, CALG_HMAC, hKey, 0, &hHmacHash.m_h)) return _T("");
	if (!::CryptSetHashParam(hHmacHash, HP_HMAC_INFO, (LPBYTE)&HmacInfo, 0)) return _T("");
	if (!::CryptHashData(hHmacHash, (LPBYTE) static_cast<LPCSTR>(strText), (DWORD)strlen(strText), 0)) return _T("");
	DWORD dwDataLen = 0;
	if (!::CryptGetHashParam(hHmacHash, HP_HASHVAL, NULL, &dwDataLen, 0)) return _T("");
	CAutoVectorPtr<BYTE> pbHash(new BYTE[dwDataLen]);
	if (NULL == pbHash) return _T("");
	if (!::CryptGetHashParam(hHmacHash, HP_HASHVAL, pbHash, &dwDataLen, 0)) return _T("");
	TCHAR szResult[200] = { 0 };
	DWORD cchResult = lengthof(szResult);
	::CryptBinaryToString(pbHash, dwDataLen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, szResult, &cchResult);
	return szResult;
}

CString CFSDriveAPI::_RestBuildActionUrl(CFSDRIVE_APITYPE ApiType, LPCTSTR pstrFunctionName, LPCTSTR pstrVerb, const TCFSDriveAccount* pAccount, const CFSDRIVE_APIARG* pArgs, int nArgs)
{
	CSimpleValArray<CFSDRIVE_APIARG> aArgs;

	for (int i = 0; i < nArgs; i++)
		aArgs.Add(pArgs[i]);

	CString sUrlSig;
	CString sSignatureResult;

	if (ApiType != CFSDRIVE_APITYPE_AUTH) {
		sUrlSig.Format(_T("%s%s"), CFSDRIVE_URL_REST, pstrFunctionName);
	}

	for (int i = 0; i < aArgs.GetSize(); i++) {
		//sUrlSig += (i > 0 ? _T("&") : _T("?"));
		sUrlSig += (i > 0 ? _T("&") : _T(""));
		sUrlSig += aArgs[i].pstrName;
		sUrlSig += _T("=");
		if (ApiType != CFSDRIVE_APITYPE_AUTH) {
			sUrlSig += UrlEncode(aArgs[i].pstrValue);
		}
		else {
			sUrlSig += aArgs[i].pstrValue;
		}
	}
	
	/*return (bUploadForm ? sSignatureResult : sUrlSig);*/
	return sUrlSig;
}

HRESULT CFSDriveAPI::_RestSendRequest(LPCTSTR pstrUrl, LPCTSTR pstrMethod, DWORD dwFlags, LPSTR pstrPost, DWORD cbPost, CAutoHttpData& Result)
{
	ATLTRACE(_T("CFSDrive-URL: %s\n"), pstrUrl);

	HR(_CheckQueryQuota());

	HR(InitHttpService());

	URL_COMPONENTS url = { 0 };
	TCHAR szUrlHost[100] = { 0 };
	TCHAR szUrlPath[2000] = { 0 };
	url.dwStructSize = sizeof(URL_COMPONENTS);
	url.lpszHostName = szUrlHost;
	url.lpszUrlPath = szUrlPath;
	url.dwHostNameLength = lengthof(szUrlHost);
	url.dwUrlPathLength = lengthof(szUrlPath);
	::WinHttpCrackUrl(pstrUrl, 0, 0, &url);
	OutputDebugString(_T(">>WinHttpCrackUrl>>"));
	OutputDebugString(url.lpszHostName);
	OutputDebugString(url.lpszUrlPath);
	OutputDebugStringA(pstrPost);
	bool bSecure = (url.nScheme == INTERNET_SCHEME_HTTPS);

	CAutoInternetHandle hConnect(::WinHttpConnect(m_hInternet, url.lpszHostName, url.nPort, 0));
	if (hConnect == NULL) {
		OutputDebugString(L">>>WinHttpConnect");
		return AtlHresultFromLastError();
	}
	OutputDebugString(_T(">>>WinHttpConnect pass"));
	CAutoInternetHandle hRequest(::WinHttpOpenRequest(hConnect, pstrMethod, url.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, bSecure ? WINHTTP_FLAG_SECURE : 0));
	if (hRequest == NULL) {
		OutputDebugString(L">>>WinHttpOpenRequest");
		return AtlHresultFromLastError();
	}
	OutputDebugString(_T(">>>WinHttpOpenRequest pass"));
	BOOL bResult = FALSE;
	//BOOL bResult = ::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, pstrPost, cbPost, cbPost, 0);
	if (!_tcscmp(url.lpszUrlPath, CFSDRIVE_LOGIN_AUTH)) {
		OutputDebugString(_T(">>CFSDRIVE_LOGIN_AUTH-!"));
		LPCTSTR additionalHeaders = L"Content-Type: application/x-www-form-urlencoded\r\n";
		bResult = ::WinHttpSendRequest(hRequest, additionalHeaders, 0, pstrPost, cbPost, cbPost, 0);
	}
	else {
		OutputDebugString(_T(">>CFSDRIVE_LOGIN_AUTH-?"));
		bResult = ::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, pstrPost, cbPost, cbPost, 0);
	}

	if (!bResult) {
		OutputDebugString(_T(">>Login Failed - WinHttpSendRequest"));
		return AtlHresultFromLastError();
	}
	OutputDebugString(_T(">>Login Success - WinHttpSendRequest"));
	bResult = ::WinHttpReceiveResponse(hRequest, NULL);
	if (!bResult) {
		OutputDebugString(_T(">>Login Failed - WinHttpReceiveResponse"));
		return AtlHresultFromLastError();
	}
	DWORD dwPos = 0;
	DWORD cbAlloc = 20 * 1024;
	LPBYTE pstrData = (LPBYTE)malloc(cbAlloc);
	while (pstrData != NULL) {
		DWORD cbAvail = 0;
		bResult = ::WinHttpQueryDataAvailable(hRequest, &cbAvail);
		if (!bResult || cbAvail == 0)
			break;
		if (dwPos + cbAvail >= cbAlloc - 1) {
			while (dwPos + cbAvail >= cbAlloc - 1)
				cbAlloc *= 2;
			pstrData = (LPBYTE)realloc(pstrData, cbAlloc);
			if (pstrData == NULL)
				return E_OUTOFMEMORY;
		}
		DWORD dwBytesRead = 0;
		bResult = ::WinHttpReadData(hRequest, pstrData + dwPos, cbAvail, &dwBytesRead);
		if (!bResult) break;
		dwPos += dwBytesRead;
	}
		
	if (!bResult) {
		OutputDebugString(_T(">>>_RestSendRequest1"));
		HRESULT Hr = AtlHresultFromLastError();
		free(pstrData);
		return Hr;
	}
	pstrData[dwPos] = '\0';

	Result.Init(pstrData, dwPos);

	return S_OK;
}

HRESULT CFSDriveAPI::_CheckQueryQuota()
{
	// Must somehow prevent excessive REST lookups on the CFSDRIVE-site. If the PC has an index-service/search/anti-virus
	// or anything installed that causes too high load on CFSDRIVE, we'll simply disable the tool on the PC!!!
	const DWORD MAX_QUOTA = 3000;
	++m_dwThisCount;
	if (m_dwThisCount == MAX_QUOTA)
		::SHSetValue(HKEY_CURRENT_USER, REGPATH_CFSDRIVEFS, L"Disabled", REG_DWORD, &m_dwThisCount, sizeof(m_dwThisCount));
	return m_dwThisCount < MAX_QUOTA ? S_OK : AtlHresultFromWin32(ERROR_SERVICE_DISABLED);
}

HRESULT CFSDriveAPI::_RestExtractResult(LPCSTR pstrXML, IXMLDOMDocument** ppDoc)
{
	ATLASSERT(pstrXML);
	if (pstrXML == NULL)
		return E_FAIL;
	CComPtr<IXMLDOMDocument> spDoc;
	HRESULT Hr = E_UNEXPECTED;
	if (FAILED(Hr)) Hr = spDoc.CoCreateInstance(L"Msxml2.DOMDocument.6.0");
	if (FAILED(Hr)) Hr = spDoc.CoCreateInstance(L"Msxml2.DOMDocument.4.0");
	if (FAILED(Hr)) Hr = spDoc.CoCreateInstance(L"Msxml2.DOMDocument");
	if (FAILED(Hr)) return Hr;
	HR(spDoc->put_async(VARIANT_FALSE));
	CComVariant vXML;
	HR(::InitVariantFromBuffer(pstrXML, (UINT)strlen(pstrXML), &vXML));
	VARIANT_BOOL vbSuccess = VARIANT_FALSE;
	HR(spDoc->load(vXML, &vbSuccess));
	if (vbSuccess == VARIANT_FALSE)
		return E_FAIL;
	CString sStatus;
	HR(_XmlGetResultNode(spDoc, L"rsp/@stat", sStatus));
	if (sStatus == _T("ok"))
		return spDoc.CopyTo(ppDoc);
	return _XmlExtractError(spDoc);
}

HRESULT CFSDriveAPI::_XmlExtractError(CComPtr<IXMLDOMDocument>& spDoc)
{
	ATLASSERT(spDoc);
	CString sErrCode, sErrorMsg;
	_XmlGetResultNode(spDoc, L"rsp/err/@code", sErrCode);
	_XmlGetResultNode(spDoc, L"rsp/err/@msg", sErrorMsg);
	// Translate error code if possible and create COM error report
	HRESULT Hr = E_FAIL;
	static DWORD s_aErrors[] = {
	   1,   ERROR_NOT_FOUND,
	   2,   ERROR_USER_PROFILE_LOAD,
	   3,   ERROR_BAD_ARGUMENTS,
	   4,   ERROR_ACCESS_DENIED,
	   5,   ERROR_FILE_CORRUPT,
	   8,   ERROR_FILE_TOO_LARGE,
	   9,   ERROR_DUPLICATE_TAG,
	   10,  ERROR_SERVICE_DISABLED,
	   14,  ERROR_DRIVE_LOCKED,
	   96,  ERROR_IPSEC_IKE_INVALID_SIGNATURE,
	   98,  ERROR_ACCESS_DENIED,
	   99,  ERROR_ACCESS_DENIED,
	   100, ERROR_SERVICE_DISABLED,
	   105, ERROR_SERVICE_DISABLED,
	   106, ERROR_WRITE_FAULT,
	};
	for (int i = 0; i < lengthof(s_aErrors); i += 2)
		if (_ttol(sErrCode) == (long)s_aErrors[i])
			Hr = AtlHresultFromWin32(s_aErrors[i + 1]);
	//return AtlReportError(CLSID_ShellFolder, sErrorMsg, GUID_NULL, Hr);
	return NULL;
}

HRESULT CFSDriveAPI::_XmlGetResultNode(IUnknown* pUnk, LPCWSTR pstrPattern, CString& sValue)
{
	ATLASSERT(pUnk);
	ATLASSERT(pstrPattern);
	if (pUnk == NULL) return E_FAIL;
	CComQIPtr<IXMLDOMNode> spNode = pUnk;
	if (spNode == NULL) return E_NOINTERFACE;
	CComPtr<IXMLDOMNode> spResult;
	HR(spNode->selectSingleNode(CComBSTR(pstrPattern), &spResult));
	if (spResult == NULL) return E_FAIL;
	CComBSTR bstr;
	HR(spResult->get_text(&bstr));
	sValue = bstr;
	return S_OK;
}

HRESULT CFSDriveAPI::_XmlGetResultList(IUnknown* pUnk, LPCWSTR pstrPattern, IXMLDOMNodeList** ppList, long* pCount)
{
	ATLASSERT(pUnk);
	ATLASSERT(pstrPattern);
	if (pUnk == NULL) return E_FAIL;
	CComQIPtr<IXMLDOMNode> spNode = pUnk;
	if (spNode == NULL) return E_NOINTERFACE;
	HR(spNode->selectNodes(CComBSTR(pstrPattern), ppList));
	if (*ppList == NULL) return E_FAIL;
	HR((*ppList)->get_length(pCount));
	return S_OK;
}

HRESULT CFSDriveAPI::_XmlGetResultList(IUnknown* pUnk, LPCWSTR pstrPattern, LPCWSTR pstrAttribute, CSimpleArray<CString>& aList)
{
	long lCount = 0;
	CComPtr<IXMLDOMNodeList> spNodeList;
	HR(_XmlGetResultList(pUnk, pstrPattern, &spNodeList, &lCount));
	for (long iIndex = 0; iIndex < lCount; iIndex++) {
		CComPtr<IXMLDOMNode> spNode;
		HR(spNodeList->get_item(iIndex, &spNode));
		CString sValue;
		HR(_XmlGetResultNode(spNode, pstrAttribute, sValue));
		if (!aList.Add(sValue)) return E_OUTOFMEMORY;
	}
	return S_OK;
}

HRESULT CFSDriveAPI::_FormGetValue(LPCTSTR pstrVariable, const CAutoHttpData& data, CString& sResult) const
{
	if (data.m_p == NULL)
		return E_UNEXPECTED;
	CStringA sKey; sKey.Format("%ls=", pstrVariable);
	LPCSTR p = strstr((LPCSTR)data.m_p, sKey);
	if (p == NULL)
		return E_UNEXPECTED;
	sResult = p + strlen(sKey);
	int iPos = sResult.Find(_T('&'));
	if (iPos >= 0)
		sResult = sResult.Left(iPos);
	return S_OK;
}

BOOL CFSDriveAPI::_MD5Hash(LPCSTR data, LPTSTR pstrResult)
{
	CAutoCryptHashHandle hHash;
	if (!::CryptCreateHash(m_hCrypt, CALG_MD5, 0, 0, &hHash.m_h))
		return FALSE;
	SIZE_T len = strlen(data);
	if (!::CryptHashData(hHash, (LPCBYTE)data, (DWORD)len, 0))
		return FALSE;
	BYTE bHash[16];
	DWORD dwHashLen = 16;
	if (!::CryptGetHashParam(hHash, HP_HASHVAL, bHash, &dwHashLen, 0))
		return FALSE;
	for (int i = 0; i < 16; i++)
		::wsprintf(pstrResult + (i * 2), _T("%02x"), bHash[i]);
	return TRUE;
}

DWORD CFSDriveAPI::_UInt4Hash(LPCWSTR pstr) const
{
	DWORD dwHash = 0x811C9DC5;
	for (; *pstr != '\0'; pstr++)
		dwHash += (dwHash * 8) + (*pstr);
	return dwHash;
}

// The "Add New Account" helpers

HRESULT CFSDriveAPI::_DoAccountWebAuthentication(HWND hWnd, TCFSDriveAccount& Account)
{
	/*
	// Only "managed accounts" require authentication; the other account types
	// have limited functionality.

	// https://www.cfsdrrive.com/services/api/auth.oauth.html

	CFSDRIVE_APIARG ApiAuthRequestToken[] = {
	   "oauth_callback", CFSDRIVE_URL_VERIFY_CALLBACK,
	};
	TCFSDriveAccount TempAccount;
	CString sURL = _RestBuildActionUrl(CFSDRIVE_APITYPE_AUTH, _T("request_token"), CFSDRIVE_VERB_GET, &TempAccount, ApiAuthRequestToken, lengthof(ApiAuthRequestToken));
	CAutoHttpData RequestTokenAnswer;
	HR(_RestSendRequest(sURL, CFSDRIVE_VERB_GET, 0, NULL, 0, RequestTokenAnswer));
	CString sCallbackConfirmed;
	_FormGetValue(_T("oauth_callback_confirmed"), RequestTokenAnswer, sCallbackConfirmed);
	_FormGetValue(_T("oauth_token"), RequestTokenAnswer, TempAccount.sAuthToken);
	_FormGetValue(_T("oauth_token_secret"), RequestTokenAnswer, TempAccount.sAuthSecret);

	if (sCallbackConfirmed != _T("true") || TempAccount.sAuthSecret.IsEmpty())
		return AtlHresultFromWin32(ERROR_USER_PROFILE_LOAD);

	// Prompt the user to manually accept authorization on CFSDrive web page
	sURL.Format(_T("%s/authorize?oauth_token=%s&perms=delete"), CFSDRIVE_URL_AUTH, TempAccount.sAuthToken);
	if (DoWebAuthentication(hWnd, sURL) != IDOK)
		return E_ABORT;

	// Make sure the user completes the authentication; ask for the access token
	// until we get it or the user cancels the operation.
	CString sVerifier;
	UINT nRes = IDCANCEL;
	do
	{
		if (sVerifier.IsEmpty()) {
			CFSDRIVE_APIARG ApiAuthVerifyToken[] = {
			   "oauth_token", TempAccount.sAuthToken,
			};
			sURL = _RestBuildActionUrl(CFSDRIVE_APITYPE_VERIFY, _T("get_verifier"), CFSDRIVE_VERB_GET, NULL, ApiAuthVerifyToken, lengthof(ApiAuthVerifyToken));
			CAutoHttpData VerifyTokenAnswer;
			_RestSendRequest(sURL, CFSDRIVE_VERB_GET, 0, NULL, 0, VerifyTokenAnswer);
			_FormGetValue(_T("oauth_verifier"), VerifyTokenAnswer, sVerifier);
		}
		if (!sVerifier.IsEmpty()) {
			CFSDRIVE_APIARG ApiAuthAccessToken[] = {
			   "oauth_verifier", sVerifier,
			};
			sURL = _RestBuildActionUrl(CFSDRIVE_APITYPE_AUTH, _T("access_token"), CFSDRIVE_VERB_GET, &TempAccount, ApiAuthAccessToken, lengthof(ApiAuthAccessToken));
			CAutoHttpData AccessTokenAnswer;
			HR(_RestSendRequest(sURL, CFSDRIVE_VERB_GET, 0, NULL, 0, AccessTokenAnswer));
			HR(_FormGetValue(_T("oauth_token"), AccessTokenAnswer, Account.sAuthToken));
			HR(_FormGetValue(_T("oauth_token_secret"), AccessTokenAnswer, Account.sAuthSecret));
			return S_OK;
		}

		nRes = DoWebAuthenticationCompleted(hWnd, sURL);
	} while (nRes == IDOK);

	return AtlHresultFromWin32(ERROR_NOT_AUTHENTICATED);
	*/
	return S_OK;
}



