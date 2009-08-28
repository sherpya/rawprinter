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


class CRPTray : public CWindowImpl<CRPTray>
{
public:
    DECLARE_EMPTY_MSG_MAP()

    CRPTray::CRPTray(void)
    {
        Create(GetDesktopWindow(), 0, 0, WS_EX_TOOLWINDOW);
        memset(&m_nid, 0, sizeof(m_nid));
        m_nid.cbSize        = sizeof(NOTIFYICONDATA);
        m_nid.uVersion      = NOTIFYICON_VERSION;
	    m_nid.hIcon         = (HICON) ::LoadImage(_Module.GetResourceInstance(),
            MAKEINTRESOURCE(IDI_ICON_PRINTER), IMAGE_ICON,
            ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
        m_nid.hWnd          = m_hWnd;
        m_nid.uID           = 0;

        _tcscpy(m_nid.szInfoTitle, _T("RawPrinter"));
        AddIcon();

        OSVERSIONINFO osv;
        memset(&osv, 0, sizeof(osv));
        osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        m_nid.dwInfoFlags = NIIF_NONE;
        if (!::GetVersionEx(&osv))
            return;

        if (osv.dwMajorVersion >= 5)
            m_nid.dwInfoFlags = NIIF_USER;
        
    }
    void AddIcon(void)
    {
        m_nid.uFlags = NIF_ICON;
        ::Shell_NotifyIcon(NIM_ADD, &m_nid);
    }
    void Ballon(WTL::CString &message, UINT timeout)
    {
        if (m_nid.dwInfoFlags == NIIF_NONE)
            return;

        m_nid.uFlags        = NIF_INFO;
        m_nid.uTimeout      = timeout;
        _tcsncpy(m_nid.szInfo, message.GetBuffer(0), (sizeof(m_nid.szInfo) / sizeof(TCHAR)) - 1);
        ::Shell_NotifyIcon(NIM_MODIFY, &m_nid);
        Sleep(timeout);
    }

    CRPTray::~CRPTray(void)
    {
        m_nid.uFlags = NIF_ICON;
        ::Shell_NotifyIcon(NIM_DELETE, &m_nid);
        SendMessage(WM_CLOSE);
    }

private:
    NOTIFYICONDATA  m_nid;
};
