//
// Dot Matrix Raw Printing Tool
//
// Copyright (c) 2009 Gianluigi Tiesi <sherpya@netfarm.it>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this software; if not, write to the
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include "stdafx.h"
#include "resource.h"
#include "Dialogs.h"

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
    // Cancel button localization
    ::SetWindowText(GetDlgItem(IDC_BUTTON_EXIT), _(IDS_CANCEL));
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
    WTL::CImageList imageList = m_tree.GetImageList();
    imageList.Create(16, 16, ILC_COLOR32, 0, 0);
    imageList.AddIcon(LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_SERVER)));
    imageList.AddIcon(LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_ICON_PRINTER)));
    m_tree.SetImageList(imageList, TVSIL_NORMAL);

    m_status = WTL::CStatusBarCtrl(GetDlgItem(IDC_STATUSBAR));

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
            SendMessage(WM_COMMAND, IDC_BUTTON_EXIT);
            break;
        case IDM_ABOUT:
            {
                CSimpleDialog<IDD_ABOUT, TRUE> about;
                about.DoModal();
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

    TCHAR value[MAX_ITEM_VALUE];
    HTREEITEM selected = m_tree.GetSelectedItem();
    if (m_tree.GetParentItem(selected))
    {
        m_tree.GetItemText(selected, value, MAX_ITEM_VALUE - 1);
        if (SetRawPrinter(value))
            SendMessage(WM_COMMAND, IDC_BUTTON_EXIT);
        else
            MessageBox(_(IDS_ERROR_SETVALUE), _(IDS_ERROR), MB_OK | MB_ICONERROR);
    }
	return 0;
}

LRESULT CMainDlg::OnInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    TCHAR value[MAX_PATH];
    HTREEITEM selected = m_tree.GetSelectedItem();

    if (m_tree.GetParentItem(selected))
    {
        m_tree.GetItemText(selected, value, MAX_PATH - 1);
        TestPrinter(value);
        return 0;
    }
	return 0;
}

LRESULT CMainDlg::OnExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT CMainDlg::OnDoubleClickTree(int idCtrl, LPNMHDR /*pnmh*/, BOOL& bHandled)
{
    ATLASSERT(idCtrl == IDC_TREE);
    SendMessage(WM_COMMAND, IDC_BUTTON_INFO);
    return TRUE;
}
