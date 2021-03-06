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
        COMMAND_ID_HANDLER(IDC_BUTTON_INFO, OnInfo)
        COMMAND_ID_HANDLER(IDC_BUTTON_EXIT, OnExit)
        NOTIFY_HANDLER(IDC_TREE, NM_DBLCLK, OnDoubleClickTree)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnInfo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnExit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnDoubleClickTree(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

    static BOOL RawPrint(LPTSTR fileName);
    static WTL::CString GetRawPrinter(void);
    static WTL::CString CMainDlg::GetIniPath(void);
    static BOOL SetRawPrinter(LPTSTR printer);
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
    CPInfoDlg::CPInfoDlg(PRINTER_INFO_2 *pinfo) : m_pinfo(pinfo) {};
    enum { IDD = IDD_PRINTER };

    BEGIN_MSG_MAP(CPInfoDlg)
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
        SetWindowText(_(IDS_PRINTER_DETAIL));
        CenterWindow(GetParent());
        WTL::CRichEditCtrl m_edit(GetDlgItem(IDC_PRINTER_INFO));
        m_edit.Clear();
        m_edit.SetUndoLimit(0);
        CHARFORMAT fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.cbSize = sizeof(fmt);
        fmt.dwMask = CFM_BOLD;

        fmt.dwEffects = CFE_BOLD;
        m_edit.SetCharFormat(fmt, SCF_SELECTION);
        m_edit.AppendText(m_pinfo->pDriverName);
        m_edit.AppendText(_T("\r\n"));

        fmt.dwEffects = 0;
        m_edit.SetCharFormat(fmt, SCF_SELECTION);
        m_edit.AppendText(m_pinfo->pPortName);

        fmt.dwMask = CFM_BOLD | CFM_COLOR;
        fmt.dwEffects = CFE_BOLD;

        m_edit.AppendText(_T("\r\n\r\n"));
        if (m_pinfo->Status)
        {
            fmt.crTextColor = RGB(240, 0, 0);
            m_edit.SetCharFormat(fmt, SCF_SELECTION);
            m_edit.AppendText(_(IDS_UNKNOWN));
        }
        else
        {
            fmt.crTextColor = RGB(0, 210, 0);
            m_edit.SetCharFormat(fmt, SCF_SELECTION);
            m_edit.AppendText(_(IDS_READY));
        }

        return TRUE;
    }
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(wID);
        return TRUE;
    }

private:
    PRINTER_INFO_2 *m_pinfo;
};
