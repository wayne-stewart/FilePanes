
#include "FilePane_Common.h"

void ExplorerBrowserEvents::SetPaneId(int id)
{
    _pane_id = id;
}

IFACEMETHODIMP ExplorerBrowserEvents::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(ExplorerBrowserEvents, IServiceProvider),
        //QITABENT(ExplorerBrowserEvents, ICommDlgBrowser),
        //QITABENT(ExplorerBrowserEvents, ICommDlgBrowser2),
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

// LRESULT 
// Explorer_SubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
// {
//     switch(msg)
//     {
//         case WM_NOTIFY: {
//             //Alert(MB_OK, L"message", L"notify message");
//             LPNMHDR hdr = (LPNMHDR)lParam;
//             switch(hdr->code) {

//             }
//         } break;
//         case WM_LBUTTONDOWN: {
//             Alert(MB_OK, L"message", L"lbuttondown message");
//         } break;
//         case WM_LBUTTONUP: {
//             //Alert(MB_OK, L"message", L"lbuttonup message");
//         } break;
//         case WM_MOUSEMOVE: {
//             //Alert(MB_OK, L"message", L"mouse move");
//         } break;
//     }
//     return DefSubclassProc(hwnd, msg, wParam, lParam);
// }

// IExplorerBrowserEvents
IFACEMETHODIMP ExplorerBrowserEvents::OnViewCreated(IShellView *psv)
{
    // HWND hwnd;
    // psv->GetWindow(&hwnd);
    // ASSERT(SetWindowSubclass(hwnd, Explorer_SubClassProc, 2, NULL), L"did not set subclass");
    // return S_OK;
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationPending(PCIDLIST_ABSOLUTE pidlFolder)
{
            //WCHAR buffer[1024] = {};
            //wsprintfW(buffer, L"navigation pending %s", (LPCWSTR)pidlFolder);
            //SendMessageW(GetParent(NULL), WM_SETTEXT, 0, LPARAM(buffer));
            //Alert(buffer);

    // must return S_OK for navigation to proceed. otherwise navigation is halted.
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