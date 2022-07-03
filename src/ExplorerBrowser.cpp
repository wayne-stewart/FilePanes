
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
    return (ULONG)InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) ExplorerBrowserEvents::Release()
{
    ULONG cRef = (ULONG)InterlockedDecrement(&_cRef);
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
IFACEMETHODIMP ExplorerBrowserEvents::OnViewCreated(IShellView  * /* psv */)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationPending(PCIDLIST_ABSOLUTE /* pidlFolder */)
{
    // must return S_OK for navigation to proceed. otherwise navigation is halted.
    return S_OK;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationComplete(PCIDLIST_ABSOLUTE pidlFolder)
{
    WCHAR buffer[1024] = {};
    SHGetPathFromIDListW(pidlFolder, buffer);
    ExplorerBrowserPane *pane = &FilePane_GetExplorerPaneById(_pane_id)->content.explorer;
    SetWindowTextW(pane->txt_path, buffer);
    SendMessageW(pane->txt_path, EM_SETCARETINDEX, (WPARAM)ARRAYSIZE(buffer), NULL);
    return E_NOTIMPL;
}

IFACEMETHODIMP ExplorerBrowserEvents::OnNavigationFailed(PCIDLIST_ABSOLUTE /* pidlFolder */)
{
    return E_NOTIMPL;
}

void ExplorerBrowser_SetPath(LPCWSTR path, Pane *pane)
{
    if (pane == NULL || pane->content_type != PaneType::ExplorerBrowser) return;

    LPITEMIDLIST pidl;
    HRESULT hr = SHParseDisplayName(path, NULL, &pidl, NULL, NULL);
    if (SUCCEEDED(hr))
    {
        pane->content.explorer.browser->BrowseToIDList(pidl, SBSP_ABSOLUTE);
        CoTaskMemFree(pidl);
    }
}


