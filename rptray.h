
// TODO:
// - correct icon for tray
// - make the ballon not to use internet connection icon (uh?)
// - use IDS_PRINT_DONE or remove it from resources

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
