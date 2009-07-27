#include "stdafx.h"
#include "resource.h"
#include "MainDlg.h"
#include <lm.h>

#define CALL(func) do { if (!func) { err = GetLastError(); goto end; } } while (0)

BOOL CMainDlg::PopulateTreeView(void)
{
    HTREEITEM root;
    LPSERVER_INFO_101 sInfo = NULL;

    DWORD i, r, total, resume = 0;
    if (::NetServerEnum(NULL,
        101,
        (LPBYTE *) &sInfo,
        MAX_PREFERRED_LENGTH,
        &r,
        &total,
        SV_TYPE_PRINTQ_SERVER,
        NULL,
        &resume) != NERR_Success)
        return FALSE;

    for (i = 0; i < total; i++)
    {
        PSHARE_INFO_502 BufPtr, p;
        NET_API_STATUS res;
        DWORD er = 0, tr = 0, j;
        root = NULL;
        resume = 0;
        do
        {
            res = ::NetShareEnum(sInfo[i].sv101_name, 502, (LPBYTE *) &BufPtr, -1, &er, &tr, &resume);
            if ((res != ERROR_SUCCESS) && (res != ERROR_MORE_DATA)) break;

            p = BufPtr;
            for (j = 1; j <= er; j++, p++)
            {
                if (!::IsValidSecurityDescriptor(p->shi502_security_descriptor))
                    continue;

                if (p->shi502_type == STYPE_PRINTQ)
                {
                    if (!root)
                        root = m_tree->InsertItem(sInfo[i].sv101_name, 0, 0, NULL, NULL);
                    m_tree->InsertItem(p->shi502_netname, 1, 1, root, NULL);
                }
            }
            ::NetApiBufferFree(BufPtr);
        } while (res == ERROR_MORE_DATA);

        if (root) m_tree->Expand(root);
    }
    if (sInfo) ::NetApiBufferFree(sInfo);
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
