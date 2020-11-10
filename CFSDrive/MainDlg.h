#pragma once

#include "resource.h"
#include "rest.h"
#include <atlhost.h>

using namespace ATL;

// CMainDlg

class CMainDlg :
	public CAxDialogImpl<CMainDlg>
{
public:
	CMainDlg()
	{
	}

	~CMainDlg()
	{
	}

	enum { IDD = IDD_MAINDLG };

BEGIN_MSG_MAP(CMainDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	CHAIN_MSG_MAP(CAxDialogImpl<CMainDlg>)
	COMMAND_HANDLER(IDC_BUTTON1_SIGNIN, BN_CLICKED, OnBnClickedButton1Signin)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

LRESULT OnBnClickedButton1Signin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

LRESULT init_login_screen(BOOL bMode);
LRESULT init_loading_screen(BOOL bMode);

LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


UINT DoWebAuthentication(HWND hWnd, LPCTSTR pstrURL);
UINT DoWebAuthenticationCompleted(HWND hWnd, LPCTSTR pstrURL);

