#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"

#define CALL(func) do { if (!func) { err = GetLastError(); goto end; } } while (0)

BOOL CMainDlg::EnumerateLocalPrinters(HTREEITEM parent)
{
    PRINTER_INFO_2 *pInfo;
    DWORD need, ret, i;
    EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, NULL, 0, &need, &ret);
    pInfo = (PRINTER_INFO_2 *) new BYTE[need];
    EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, (LPBYTE) pInfo, need, &need, &ret);
    for (i = 0; i < ret; i++)
        m_tree.InsertItem(pInfo[i].pPrinterName, 1, 1, parent, NULL);
    delete pInfo;
    m_tree.Expand(parent);
    return TRUE;
}

BOOL CMainDlg::EnumerateNetworkPrinters(LPNETRESOURCE lpnr, HTREEITEM parent)
{
    DWORD dwResult, dwResultEnum;
    HANDLE hEnum;
    DWORD cbBuffer = 16384;
    DWORD cEntries = -1;        // enumerate all possible entries
    LPNETRESOURCE lpnrLocal;
    DWORD i;

    dwResult = ::WNetOpenEnum(RESOURCE_GLOBALNET, // all network resources
        RESOURCETYPE_ANY,
        0,  // enumerate all resources
        lpnr,
        &hEnum);

    if (dwResult != NO_ERROR)
        return FALSE;

    lpnrLocal = (LPNETRESOURCE) new byte[cbBuffer];
    if (lpnrLocal == NULL)
        return FALSE;

    do
    {
        memset(lpnrLocal, 0, cbBuffer);
        dwResultEnum = WNetEnumResource(hEnum,  // resource handle
            &cEntries,      // defined locally as -1
            lpnrLocal,      // LPNETRESOURCE
            &cbBuffer);     // buffer size

        if (dwResultEnum == NO_ERROR)
        {
            for (i = 0; i < cEntries; i++)
            {
                if (lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER)
                {
                    WTL::CString message(_T("Scanning "));
                    switch (lpnrLocal[i].dwDisplayType)
                    {
                        case RESOURCEDISPLAYTYPE_DOMAIN:
                            message += _T("Domain / Group ");
                            break;
                        case RESOURCEDISPLAYTYPE_SERVER:
                            message += _T("Server ");
                            break;
                    }
                    message += lpnrLocal[i].lpRemoteName;
                    m_status.SetText(0, message);
                    EnumerateNetworkPrinters(&lpnrLocal[i], parent);
                }
                else if (lpnrLocal[i].dwType == RESOURCETYPE_PRINT)
                {
                    m_tree.InsertItem(lpnrLocal[i].lpRemoteName, 1, 1, parent, NULL);
                    m_tree.Expand(parent);
                }
            }
        }
        else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
            break;
    }
    while (dwResultEnum != ERROR_NO_MORE_ITEMS);

    delete lpnrLocal;

    dwResult = ::WNetCloseEnum(hEnum);

    return FALSE;
}

DWORD WINAPI CMainDlg::PopulateTreeView(LPVOID lpParameter)
{
    CMainDlg *pThis = static_cast<CMainDlg *> (lpParameter);
    HTREEITEM local = pThis->m_tree.InsertItem(_T("Local"), 0, 0, NULL, NULL);
    pThis->EnumerateLocalPrinters(local);

    HTREEITEM network = pThis->m_tree.InsertItem(_T("Network"), 0, 0, NULL, NULL);
    pThis->EnumerateNetworkPrinters(NULL, network);

    pThis->m_status.SetText(0, _T("Please select the printer"));
    return 0;
}

BOOL CMainDlg::TestPrinter(LPTSTR printer)
{
    HANDLE p;
    PRINTER_INFO_2 *pInfo;
    DWORD need = 0;

    if (!::OpenPrinter(printer, &p, NULL))
        return FALSE;

    ::GetPrinter(p, 2, NULL, 0, &need);

    if (!need)
        return FALSE;

    pInfo = (PRINTER_INFO_2 *) new BYTE[need];

    ::GetPrinter(p, 2, (LPBYTE) pInfo, need, &need);

    //CDialogImpl<CSimpleDialog> pdlg;
    //CSimpleDialog<IDD_PRINTER, TRUE> pdlg;
    //pdlg.Create(NULL);
    //HWND z = pdlg.GetDlgItem(IDC_PRINTER_INFO);
    //CEdit edit(pdlg.GetDlgItem(IDC_PRINTER_INFO));
    //CString pinfotxt(_T(""));
    //pinfotxt += _T("Prova");
    //edit.Clear();
    //edit.AppendText(pinfotxt);
    //pdlg.DoModal();

    delete pInfo;
    return TRUE;
}

static BOOL RawPrint(LPTSTR fileName, LPTSTR printer)
{
    HANDLE p = INVALID_HANDLE_VALUE;
    HANDLE f = INVALID_HANDLE_VALUE;
    char *buffer = NULL;
    DWORD err = ERROR_SUCCESS, w, size, r;

    PRINTER_DEFAULTS defaults = { _T("RAW"), 0, PRINTER_ACCESS_USE };
    DOC_INFO_1 doc = { fileName, NULL, _T("RAW") };

    if ((f = ::CreateFile(fileName,
        GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
            goto end;

    size = ::GetFileSize(f, NULL);
    buffer = (char *) new char[size];
    ::ReadFile(f, buffer, size, &r, NULL);
    ::CloseHandle(f);

    CALL(::OpenPrinter(printer, &p, &defaults));
    CALL(::StartDocPrinter(p, 1, (LPBYTE) &doc));
    CALL(::StartPagePrinter(p));
    CALL(::WritePrinter(p, buffer, size, &w));
    CALL(::EndPagePrinter(p));
    CALL(::EndDocPrinter(p));

end:
    if (buffer) delete buffer;
    ClosePrinter(p);
    return TRUE;
}
