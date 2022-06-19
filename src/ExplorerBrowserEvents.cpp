
#include "FilePane_Common.h"

IFACEMETHODIMP ExplorerBrowserEvents::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(ExplorerBrowserEvents, IServiceProvider),
        QITABENT(ExplorerBrowserEvents, ICommDlgBrowser),
        QITABENT(ExplorerBrowserEvents, ICommDlgBrowser2),
        QITABENT(ExplorerBrowserEvents, IExplorerBrowserEvents),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) ExplorerBrowserEvents::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) ExplorerBrowserEvents::Release()
{
    long cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
    {
        delete this;
    }
    return cRef;
}

// IServiceProvider
IFACEMETHODIMP ExplorerBrowserEvents::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_NOINTERFACE;
    if (guidService == SID_SExplorerBrowserFrame)
    {
        hr = QueryInterface(riid, ppv);
    }
    return hr;
}

// ICommDlgBrowser
IFACEMETHODIMP ExplorerBrowserEvents::OnDefaultCommand(IShellView * /* psv */)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnStateChange(IShellView * /* psv */, ULONG uChange)
{
    // if (uChange == CDBOSC_SELCHANGE)
    // {
    //     _OnSelChange();
    // }
    return S_OK;
}

IFACEMETHODIMP ExplorerBrowserEvents::IncludeObject(IShellView * /* psv */, PCUITEMID_CHILD /* pidl */)
{
    return E_NOTIMPL;
}

// ICommDlgBrowser2
IFACEMETHODIMP ExplorerBrowserEvents::Notify(IShellView * /* ppshv */ , DWORD /* dwNotifyType */)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::GetDefaultMenuText(IShellView * /* ppshv */, PWSTR /* pszText */, int /* cchMax */)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::GetViewFlags(DWORD *pdwFlags)
{
    // setting this flag is needed to avoid the poor perf of having
    // ICommDlgBrowser::IncludeObject() for every item when the result
    // set is large.
    *pdwFlags = CDB2GVF_NOINCLUDEITEM;
    return E_NOTIMPL;
}

// IExplorerBrowserEvents
IFACEMETHODIMP ExplorerBrowserEvents::OnViewCreated(IShellView * /* psv */)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationPending(PCIDLIST_ABSOLUTE pidlFolder)
{
            //WCHAR buffer[1024] = {};
            //wsprintfW(buffer, L"navigation pending %s", (LPCWSTR)pidlFolder);
            //SendMessageW(GetParent(NULL), WM_SETTEXT, 0, LPARAM(buffer));
            //Alert(buffer);
    return S_OK;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationComplete(PCIDLIST_ABSOLUTE pidlFolder)
{
    // if (_fPerformRenavigate)
    // {
    //     KillTimer(_hdlg, IDT_SEARCHSTART);
    //     _OnSearch();
    //     _fPerformRenavigate = FALSE;
    // }
            //WCHAR buffer[1024] = {};
            //wsprintfW(buffer, L"navigation complete %s", (LPCWSTR)pidlFolder);
            //SendMessageW(GetParent(NULL), WM_SETTEXT, 0, LPARAM(buffer));
            //Alert(buffer);
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationFailed(PCIDLIST_ABSOLUTE /* pidlFolder */)
{
    return E_NOTIMPL;
}

// void ExplorerBrowserCOM::_OnSelChange()
// {
//     IShellItem *psi;
//     HRESULT hr = _GetSelectedItem(IID_PPV_ARGS(&psi));
//     if (SUCCEEDED(hr))
//     {
//         PWSTR pszName;
//         hr = psi->GetDisplayName(SIGDN_NORMALDISPLAY, &pszName);
//         if (SUCCEEDED(hr))
//         {
//             SetDlgItemText(_hdlg, IDC_NAME, pszName);
//             CoTaskMemFree(pszName);
//         }
//         psi->Release();
//     }
// }