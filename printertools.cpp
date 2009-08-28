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
#include "rptray.h"

#define CALL(func, msg) do \
{ \
    if (!func) \
    { \
        err = GetLastError(); \
        ::MessageBox(NULL, AtlGetErrorDescription(::GetLastError()), _T("RawPrinter::")_T(#msg), MB_OK | MB_ICONERROR); \
        goto end; \
    } \
} while (0)

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
                    WTL::CString message(_(IDS_SCANNING));
                    switch (lpnrLocal[i].dwDisplayType)
                    {
                        case RESOURCEDISPLAYTYPE_DOMAIN:
                            message += _T(" Domain / Group ");
                            break;
                        case RESOURCEDISPLAYTYPE_SERVER:
                            message += _T(" Server ");
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
    HTREEITEM local = pThis->m_tree.InsertItem(_(IDS_LOCAL), 0, 0, NULL, NULL);
    pThis->EnumerateLocalPrinters(local);

    HTREEITEM network = pThis->m_tree.InsertItem(_(IDS_NETWORK), 0, 0, NULL, NULL);
    pThis->EnumerateNetworkPrinters(NULL, network);

    pThis->m_status.SetText(0, WTL::CString(_(IDS_SELECT_PRINTER)));
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

    CPInfoDlg pDlg(pInfo);
    pDlg.DoModal();
    delete pInfo;
    return TRUE;
}

BOOL CMainDlg::RawPrint(LPTSTR fileName)
{
    HANDLE p, f;
    p = f = INVALID_HANDLE_VALUE;
    LPBYTE buffer = NULL;
    DWORD err = ERROR_SUCCESS, w, size, r;
    WTL::CString ballon, message;
    CRPTray rwtray;

    WTL::CString printer = CMainDlg::GetRawPrinter();
    if (!printer.GetLength())
    {
        ::MessageBox(NULL, _(IDS_NOPRINTER), _T("RawPrinter"), MB_OK | MB_ICONERROR);
        return FALSE;
    }

    PRINTER_DEFAULTS defaults = { _T("RAW"), 0, PRINTER_ACCESS_USE };
    DOC_INFO_1 doc = { fileName, NULL, _T("RAW") };

    if ((f = ::CreateFile(fileName,
        GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        message.Format(_(IDS_ERROR_CREATEFILE), fileName, AtlGetErrorDescription(::GetLastError()));
        ::MessageBox(NULL, message, _T("RawPrinter"), MB_OK | MB_ICONERROR);
            goto end;
    }

    size = ::GetFileSize(f, NULL);
    buffer = (LPBYTE) new BYTE[size];
    ::ReadFile(f, buffer, size, &r, NULL);
    ::CloseHandle(f);

    LPTSTR fn = _tcsrchr(fileName, _T('\\'));
    ballon.Format(_(IDS_PRINTING), fn ? fn + 1: fileName, printer);
    rwtray.Ballon(ballon, 3000);

    CALL(::OpenPrinter(printer.GetBuffer(0), &p, &defaults), OpenPrinter);
    CALL(::StartDocPrinter(p, 1, (LPBYTE) &doc), StartDocPrinter);
    CALL(::StartPagePrinter(p), StartPagePrinter);
    CALL(::WritePrinter(p, buffer, size, &w), WritePrinter);
    CALL(::EndPagePrinter(p), EndPagePrinter);
    CALL(::EndDocPrinter(p), EndDocPrinter);

end:
    if (buffer) delete buffer;
    ClosePrinter(p);
    return TRUE;
}

WTL::CString CMainDlg::GetIniPath(void)
{
    TCHAR *ls, exepath[MAX_PATH];
    if (!::GetModuleFileName(NULL, exepath, MAX_PATH - 1))
        return _T("");

    if (!(ls = _tcsrchr(exepath, _T('.'))))
        return _T("");

    ls++; *ls = 0;

    WTL::CString path(exepath);
    path += _T("ini");

    return path;
}

WTL::CString CMainDlg::GetRawPrinter(void)
{
    TCHAR printer[MAX_PATH] = _T("");
    ::GetPrivateProfileString(_T("RawPrinter"), _T("printer"), _T(""), printer, MAX_PATH - 1, GetIniPath().GetBuffer(0));
    return WTL::CString(printer);
}

BOOL CMainDlg::SetRawPrinter(LPTSTR printer)
{
    if (!::WritePrivateProfileString(_T("RawPrinter"), _T("printer"), printer, GetIniPath().GetBuffer(0)))
        return FALSE;
    return TRUE;
}
