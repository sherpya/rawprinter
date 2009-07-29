#pragma once

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_DIALOG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
        COMMAND_ID_HANDLER(IDC_BUTTON_OK, OnOK)
		COMMAND_ID_HANDLER(IDC_BUTTON_TEST, OnTest)
		COMMAND_ID_HANDLER(IDC_BUTTON_CANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTest(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    static BOOL RawPrint(LPTSTR fileName, LPTSTR printer);
    static DWORD WINAPI PopulateTreeView(LPVOID lpParameter);

	void CloseDialog(int nVal);
    BOOL EnumerateLocalPrinters(HTREEITEM parent);
    BOOL EnumerateNetworkPrinters(LPNETRESOURCE lpnr, HTREEITEM parent);
    BOOL TestPrinter(LPTSTR printer);
    HANDLE m_thEnum;

private:
    WTL::CTreeViewCtrl m_tree;
    WTL::CStatusBarCtrl m_status;
};


class CPInfoDlg : public CDialogImpl<CPInfoDlg>
{
public:
    CPInfoDlg::CPInfoDlg(CString info) : m_info(info) {};
	enum { IDD = IDD_PRINTER };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
	END_MSG_MAP()

    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        if (wParam == SC_CLOSE)
            SendMessage(WM_COMMAND, IDOK);
        else
            bHandled = FALSE;
        return TRUE;
    }
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());
        CEdit m_edit(GetDlgItem(IDC_PRINTER_INFO));
        m_edit.Clear();
        m_edit.AppendText(m_info);
        return TRUE;
    }
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(wID);
        return TRUE;
    }

private:
    CString m_info;
};