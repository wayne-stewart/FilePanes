
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

// IExplorerBrowserEvents
IFACEMETHODIMP ExplorerBrowserEvents::OnViewCreated(IShellView *psv)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationPending(PCIDLIST_ABSOLUTE pidlFolder)
{
    // must return S_OK for navigation to proceed. otherwise navigation is halted.
    return S_OK;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationComplete(PCIDLIST_ABSOLUTE pidlFolder)
{
    WCHAR buffer[1024] = {};
    SHGetPathFromIDListW(pidlFolder, buffer);
    ExplorerBrowserPane *pane = &FilePane_GetExplorerPaneById(_pane_id)->content.explorer;
    SetWindowTextW(pane->txt_uri, buffer);
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationFailed(PCIDLIST_ABSOLUTE /* pidlFolder */)
{
    return E_NOTIMPL;
}
