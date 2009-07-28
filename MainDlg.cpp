// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"
#include "AboutDlg.h"

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON),
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON),
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

    // System Menu
    CMenu sysmenu(GetSystemMenu(FALSE));
    sysmenu.AppendMenu(MF_SEPARATOR);
    sysmenu.AppendMenu(MF_ENABLED, IDM_ABOUT, _T("About"));

    // Tree View
    m_tree = WTL::CTreeViewCtrl(GetDlgItem(IDC_TREE));
    WTL::CImageList imageList;
    imageList.Create(16, 16, ILC_COLOR32, 0, 0);
    imageList.AddIcon(LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_SERVER)));
    imageList.AddIcon(LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_PRINTER)));
    m_tree.SetImageList(imageList, TVSIL_NORMAL);

    m_status = WTL::CStatusBarCtrl(GetDlgItem(IDC_STATUSBAR));
    m_status.SetText(0, _T("Searching for printers"));

    DWORD tid;
    m_thEnum = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CMainDlg::PopulateTreeView, (LPVOID) this, 0, &tid);
	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
	return 0;
}

LRESULT CMainDlg::OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    switch (wParam)
    {
        case SC_CLOSE:
            SendMessage(WM_COMMAND, IDC_BUTTON_CANCEL);
            break;
        case IDM_ABOUT:
            {
            	CAboutDlg dlg;
	            dlg.DoModal();
                break;
            }
        default:
            bHandled = FALSE;
    }
    return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
#define MAX_ITEM_VALUE 100

    wchar_t value[MAX_ITEM_VALUE];
    HTREEITEM selected = m_tree.GetSelectedItem();
    m_tree.GetItemText(selected, value, MAX_ITEM_VALUE - 1);
    if (m_tree.GetParentItem(selected))
    {
        MessageBox(value, _T("Selected"), MB_OK);
        return 0;
    }
	return 1;
}

LRESULT CMainDlg::OnTest(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (::WaitForSingleObject(m_thEnum, 0) != WAIT_OBJECT_0)
        ::TerminateThread(m_thEnum, -1);
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}
