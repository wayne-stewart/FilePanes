
#include "FilePane_Common.h"

class ExplorerBrowserCOM : public IServiceProvider, public ICommDlgBrowser2, public IExplorerBrowserEvents
{
public:

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(ExplorerBrowserCOM, IServiceProvider),
            QITABENT(ExplorerBrowserCOM, ICommDlgBrowser),
            QITABENT(ExplorerBrowserCOM, ICommDlgBrowser2),
            QITABENT(ExplorerBrowserCOM, IExplorerBrowserEvents),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IServiceProvider
    IFACEMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv)
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
    IFACEMETHODIMP OnDefaultCommand(IShellView * /* psv */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnStateChange(IShellView * /* psv */, ULONG uChange)
    {
        // if (uChange == CDBOSC_SELCHANGE)
        // {
        //     _OnSelChange();
        // }
        return S_OK;
    }

    IFACEMETHODIMP IncludeObject(IShellView * /* psv */, PCUITEMID_CHILD /* pidl */)
    {
        return E_NOTIMPL;
    }

    // ICommDlgBrowser2
    IFACEMETHODIMP Notify(IShellView * /* ppshv */ , DWORD /* dwNotifyType */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP GetDefaultMenuText(IShellView * /* ppshv */, PWSTR /* pszText */, int /* cchMax */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP GetViewFlags(DWORD *pdwFlags)
    {
        // setting this flag is needed to avoid the poor perf of having
        // ICommDlgBrowser::IncludeObject() for every item when the result
        // set is large.
        *pdwFlags = CDB2GVF_NOINCLUDEITEM;
        return E_NOTIMPL;
    }

    // IExplorerBrowserEvents
    IFACEMETHODIMP OnViewCreated(IShellView * /* psv */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnNavigationPending(PCIDLIST_ABSOLUTE pidlFolder)
    {
                //WCHAR buffer[1024] = {};
                //wsprintfW(buffer, L"navigation pending %s", (LPCWSTR)pidlFolder);
                //SendMessageW(GetParent(NULL), WM_SETTEXT, 0, LPARAM(buffer));
                //Alert(buffer);
        return S_OK;
    }

    IFACEMETHODIMP OnNavigationComplete(PCIDLIST_ABSOLUTE pidlFolder)
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

    IFACEMETHODIMP OnNavigationFailed(PCIDLIST_ABSOLUTE /* pidlFolder */)
    {
        return E_NOTIMPL;
    }
private:
    long _cRef;
};

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