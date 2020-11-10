#include "pch.h"
#include "MainDlg.h"

#include <commoncontrols.h>

// CMainDlg

LRESULT CMainDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CMainDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CMainDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnBnClickedButton1Signin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString strTunnantUrl;
	CString strUserID;
	CString strPassword;
	CString strClientID = L"CFSMobileApp";
	CString strClientSecret = L"nI7tHrQ1fP88BpkNd1Y0glLJnZ!as4HL&Om@3tE%Emxz!";
	CString strGrantType = L"password";	

	GetDlgItemText(IDC_EDIT_TUNANTURL, strTunnantUrl);
	GetDlgItemText(IDC_EDIT_USERID, strUserID);
	GetDlgItemText(IDC_EDIT_PASSWORD, strPassword);

	if (strTunnantUrl.IsEmpty())
	{
		SetDlgItemText(IDC_STATIC_ALERTMSG, _T("Incorrect Tenant URL"));
		return 0;
	}

	if (strUserID.IsEmpty() || strPassword.IsEmpty())
	{
		SetDlgItemText(IDC_STATIC_ALERTMSG, _T("Incorrect login or password"));
		return 0;
	}

	init_loading_screen(TRUE);
	init_login_screen(FALSE);

	OutputDebugString(strTunnantUrl);
	OutputDebugString(strUserID);
	OutputDebugString(strPassword);
	OutputDebugString(strClientID);
	OutputDebugString(strClientSecret);
	OutputDebugString(strGrantType);

	if (strTunnantUrl.Find(_T("https://")) < 0)
		strTunnantUrl = _T("https://") + strTunnantUrl;

	HRESULT Hr = _CFSDriveAPI.CheckAccount(strTunnantUrl, strUserID, strPassword, strClientID, strClientSecret, strGrantType);
	if (FAILED(Hr)) {
		OutputDebugString(_T(">>SignIn Failed - CheckAccount"));
	}
	else {
		OutputDebugString(_T(">>SignIn Success - CheckAccount"));
	}
	EndDialog(IDOK);
	return 0;
}


LRESULT CMainDlg::init_login_screen(BOOL bMode)
{
	if (bMode)
	{
		GetDlgItem(IDC_EDIT_TUNANTURL).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_USERID).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_PASSWORD).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_ALERTMSG).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BUTTON1_SIGNIN).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BUTTON_FORGOTPASSWORD).ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_EDIT_TUNANTURL).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_USERID).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_PASSWORD).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_ALERTMSG).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON1_SIGNIN).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BUTTON_FORGOTPASSWORD).ShowWindow(SW_HIDE);
	}

	return 0;
}

LRESULT CMainDlg::init_loading_screen(BOOL bMode)
{
	if (bMode)
	{
		GetDlgItem(IDC_STATIC_PROGRESS).ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_SIGNIN).ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_STATIC_PROGRESS).ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_SIGNIN).ShowWindow(SW_HIDE);
	}

	return 0;
}


UINT DoWebAuthentication(HWND hWnd, LPCTSTR pstrURL)
{
	TASKDIALOGCONFIG tdc = { 0 };
	tdc.cbSize = sizeof(TASKDIALOGCONFIG);
	tdc.hInstance = _pModule->GetResourceInstance();
	tdc.hwndParent = hWnd;
	tdc.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
	tdc.pszWindowTitle = _T("CFSDrive");
	tdc.pszMainIcon = MAKEINTRESOURCE(IDI_ICON_LOGO);
	tdc.pszMainInstruction = _T("CFSDrive");
	tdc.pszContent = _T("CFSDrive");
	TASKDIALOG_BUTTON aButtons[] = {
	  { 200, L"OK" },
	  { 201, L"Cancel" },
	};
	tdc.cButtons = lengthof(aButtons);
	tdc.pButtons = aButtons;
	int iButton = 0;
	::TaskDialogIndirect(&tdc, &iButton, NULL, NULL);
	if (iButton == 200) {
		::ShellExecute(hWnd, _T("open"), pstrURL, NULL, NULL, SW_SHOW);
		::Sleep(3000);
	}
	return iButton == 200 ? IDOK : IDCANCEL;
}

static HRESULT CALLBACK TaskDialogCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
	switch (msg) {
	case TDN_HYPERLINK_CLICKED:
		::ShellExecute(hWnd, _T("open"), reinterpret_cast<LPCTSTR>(lpRefData), NULL, NULL, SW_SHOW);
		break;
	}
	return 0;
}

/**
 * Prompt to complete web authentication.
 * Since we cannot reliably wait for the Internet Browser and the user completing the
 * web authentication process, we'll show a prompt and allow the user to dismiss it
 * when he completes the authentication.
 */
UINT DoWebAuthenticationCompleted(HWND hWnd, LPCTSTR pstrURL)
{
	TASKDIALOGCONFIG tdc = { 0 };
	tdc.cbSize = sizeof(TASKDIALOGCONFIG);
	tdc.hInstance = _pModule->GetResourceInstance();
	tdc.hwndParent = hWnd;
	tdc.pfCallback = TaskDialogCallback;
	tdc.lpCallbackData = (LONG_PTR)pstrURL;
	tdc.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW | TDF_ENABLE_HYPERLINKS;
	tdc.pszWindowTitle = _T("CFSDrive");
	tdc.pszMainIcon = MAKEINTRESOURCE(IDI_ICON_LOGO);
	tdc.pszMainInstruction = _T("CFSDrive");
	tdc.pszContent = _T("CFSDrive");
	tdc.pszFooter = _T("CFSDrive");
	TASKDIALOG_BUTTON aButtons[] = {
	  { 200, L"OK" },
	  { 201, L"Cancel" },
	};
	tdc.cButtons = lengthof(aButtons);
	tdc.pButtons = aButtons;
	int iButton = 0;
	::TaskDialogIndirect(&tdc, &iButton, NULL, NULL);
	return iButton == 200 ? IDOK : IDCANCEL;
}


HRESULT DoDisplayErrorMessage(HWND hWnd, UINT nTitle, HRESULT Hr)
{
	HRESULT HrOrig = Hr;
	if (HRESULT_FACILITY(Hr) == FACILITY_WINDOWS)
		Hr = HRESULT_CODE(Hr);
	if (HRESULT_FACILITY(Hr) == FACILITY_WIN32)
		Hr = HRESULT_CODE(Hr);
	// Format error message; try various sources
	LPTSTR pstr = NULL;
	if (pstr == NULL)
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL, Hr, 0, (LPTSTR)&pstr, 0, NULL);
	static LPCTSTR s_ppstrModules[] = { _T("winhttp"), _T("mswsock"), _T("netmsg"), _T("crypt32") };
	for (int i = 0; pstr == NULL && i < lengthof(s_ppstrModules); i++) {
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE, ::GetModuleHandle(s_ppstrModules[i]), Hr, 0, (LPTSTR)&pstr, 0, NULL);
	}
	bool bReleaseStr = (pstr != NULL);
	CString sTemp(_T("ERROR"));
	CComBSTR bstrErr;
	CComPtr<IErrorInfo> spErrInfo;
	::GetErrorInfo(0, &spErrInfo);
	if (spErrInfo != NULL)
		spErrInfo->GetDescription(&bstrErr);
	if (pstr == NULL)
		pstr = bstrErr.m_str;
	if (pstr == NULL)
		pstr = const_cast<LPTSTR>(static_cast<LPCTSTR>(sTemp));
	while (pstr[0] != '\0' && _tcschr(_T("\r\n "), pstr[_tcslen(pstr) - 1]) != NULL)
		pstr[_tcslen(pstr) - 1] = '\0';
	// Display message as a TaskDialog prompt
	CString sMessage, sFooter, sExtra;
	if (Hr >= WINHTTP_ERROR_BASE && Hr <= WINHTTP_ERROR_LAST)
		sExtra = _T("HTTP ERROR");
	sMessage.Format(_T("Description Error"), pstr, sExtra);
	sFooter.Format(_T("HResult Error"), HrOrig, bstrErr);
	if (bReleaseStr)
		::LocalFree(pstr);
	TASKDIALOGCONFIG tdc = { 0 };
	tdc.cbSize = sizeof(TASKDIALOGCONFIG);
	tdc.hInstance = _pModule->GetResourceInstance();
	tdc.hwndParent = hWnd;
	tdc.pszMainIcon = TD_ERROR_ICON;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
	tdc.dwCommonButtons = TDCBF_OK_BUTTON;
	tdc.pszWindowTitle = _T("CFSDrive");
	tdc.pszMainInstruction = MAKEINTRESOURCE(nTitle);
	tdc.pszContent = sMessage;
	tdc.pszFooter = sFooter;
	::TaskDialogIndirect(&tdc, NULL, NULL, NULL);
	return HrOrig;
}



LRESULT CMainDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{	
	return 0;
}


LRESULT CMainDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// TODO: Add your message handler code here and/or call default

	bHandled = FALSE;
	return 0;
}
