#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"
#include <lm.h>

#define CALL(func) do { if (!func) { err = GetLastError(); goto end; } } while (0)

BOOL CMainDlg::EnumeratePrinters(LPNETRESOURCE lpnr, std::vector<std::wstring> &printers)
{
    DWORD dwResult, dwResultEnum;
    HANDLE hEnum;
    DWORD cbBuffer = 16384;
    DWORD cEntries = -1;        // enumerate all possible entries
    LPNETRESOURCE lpnrLocal;
    DWORD i;

    if (m_eAbort)
        return FALSE;

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
                    if (!EnumeratePrinters(&lpnrLocal[i], printers) && m_eAbort)
                        return FALSE;
                }
                else if (lpnrLocal[i].dwType == RESOURCETYPE_PRINT)
                    printers.push_back(std::wstring(lpnrLocal[i].lpRemoteName));
            }
        }
        else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
            break;
    }
    while (dwResultEnum != ERROR_NO_MORE_ITEMS);

    delete lpnrLocal;

    dwResult = ::WNetCloseEnum(hEnum);

    if (dwResult != NO_ERROR)
        return FALSE;

    return TRUE;
}

DWORD WINAPI CMainDlg::PopulateTreeView(LPVOID lpParameter)
{
    CMainDlg *pThis = static_cast<CMainDlg *> (lpParameter);
    std::vector<std::wstring> printers;
    pThis->EnumeratePrinters(NULL, printers);

    if (pThis->m_eAbort)
        return -1;

    for (unsigned int i = 0; i < printers.size(); i++)
        pThis->m_tree.InsertItem(printers[i].c_str(), 1, 1, NULL, NULL);
    pThis->m_status.SetText(0, _T("Please select the printer"));

    return 0;
}

static BOOL RawPrint(LPWSTR fileName, LPWSTR printer)
{
    HANDLE p = INVALID_HANDLE_VALUE;
    HANDLE f = INVALID_HANDLE_VALUE;
    char *buffer = NULL;
    DWORD err = ERROR_SUCCESS, w, size, r;

    PRINTER_DEFAULTS defaults = { L"RAW", 0, PRINTER_ACCESS_USE };
    DOC_INFO_1 doc = { fileName, NULL, L"RAW" };

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
