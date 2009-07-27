#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"
#include <lm.h>

#define CALL(func) do { if (!func) { err = GetLastError(); goto end; } } while (0)

BOOL CMainDlg::EnumeratePrinters(LPNETRESOURCE lpnr, HTREEITEM parent)
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

    //if (!parent)
    //    parent = m_tree->InsertItem(L"ZZZ", NULL, NULL);

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
                //if (RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER))
                if (lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER)
                    EnumeratePrinters(&lpnrLocal[i], parent);
                else if (lpnrLocal[i].dwType == RESOURCETYPE_PRINT)
                    m_tree->InsertItem(lpnrLocal[i].lpRemoteName, 1, 1, parent, NULL);
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

BOOL CMainDlg::PopulateTreeView(void)
{
    LPNETRESOURCE lpnr = NULL;
    EnumeratePrinters(NULL, NULL);
    return TRUE;
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
