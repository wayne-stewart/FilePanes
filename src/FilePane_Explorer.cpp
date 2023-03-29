
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

void ExplorerBrowser_DeleteSelected(Pane *pane)
{
    if (pane == NULL || pane->content_type != PaneType::ExplorerBrowser) return;

    IFolderView *folder_view;
    if (S_OK == pane->content.explorer.browser->GetCurrentView(IID_IFolderView, (LPVOID*)&folder_view))
    {
        IShellItemArray *shell_items;
        if (S_OK == folder_view->Items(SVGIO_SELECTION, IID_IShellItemArray, (LPVOID*)&shell_items))
        {
            IFileOperation *file_operation;
            if (SUCCEEDED(CreateAndInitializeFileOperation(IID_PPV_ARGS(&file_operation))))
            {
                // shell_item does not need to be freed,
                // it will be freed when shell_items is freed
                // IShellItem *shell_item;
                // if (SUCCEEDED(shell_items->GetItemAt(0, &shell_item)))
                // {
                //     LPWSTR item_name;
                //     if (SUCCEEDED(shell_item->GetDisplayName(SIGDN_NORMALDISPLAY, &item_name)))
                //     {
                //         Alert(NULL, L"Shell Item", item_name);
                //         CoTaskMemFree(item_name);
                //     }
                // }
                if (SUCCEEDED(file_operation->DeleteItems(shell_items)))
                {
                    file_operation->PerformOperations();
                }
                file_operation->Release();
            }
            shell_items->Release();
        }
        folder_view->Release();
    }
}
