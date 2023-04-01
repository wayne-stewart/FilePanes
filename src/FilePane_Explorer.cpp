
#include "FilePane_Common.h"

void ExplorerBrowserEvents::SetPaneId(int id)
{
    _pane_id = id;
}

bool ExplorerBrowserEvents::HasFocus()
{
    return _hasfocus;
}

IFACEMETHODIMP ExplorerBrowserEvents::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(ExplorerBrowserEvents, IServiceProvider),
        QITABENT(ExplorerBrowserEvents, IExplorerBrowserEvents),
        QITABENT(ExplorerBrowserEvents, ICommDlgBrowser),
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

// ICommDlgBrowser
HRESULT ExplorerBrowserEvents::IncludeObject(IShellView *shell_view, PCUITEMID_CHILD pidl)
{
    UNREFERENCED_PARAMETER(shell_view);
    UNREFERENCED_PARAMETER(pidl);
    return E_NOTIMPL;
}
HRESULT ExplorerBrowserEvents::OnDefaultCommand(IShellView *shell_view)
{
    UNREFERENCED_PARAMETER(shell_view);
    //DEBUGALERT(L"DEFAULT COMMAND HAPPENED");
    return E_NOTIMPL;
}
HRESULT ExplorerBrowserEvents::OnStateChange(IShellView *shell_view, ULONG uChange)
{
    UNREFERENCED_PARAMETER(shell_view);
    UNREFERENCED_PARAMETER(uChange);
    switch(uChange)
    {
        // view got focus
        case CDBOSC_SETFOCUS:
        _hasfocus = true;
        break;

        // view lost focus
        case CDBOSC_KILLFOCUS:
        _hasfocus = false;
        break;

        // selection has changed
        case CDBOSC_SELCHANGE:
        break;

        // an item was renamed
        case CDBOSC_RENAME:
        break;

        // an item has been checked or unchecked
        case CDBOSC_STATECHANGE:
        break;
    }
    return S_OK;
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

void ExplorerBrowser_HandleDeleteKeyPress(Pane *pane)
{
    CHECK_EXPLORER_BROWSER_HAS_FOCUS(pane);

    AutoRelease<IFolderView> folder_view;
    AutoRelease<IShellItemArray> selected_items;
    AutoRelease<IFileOperation> file_operation;

    CHECK_S_OK(pane->content.explorer.browser->GetCurrentView(IID_IFolderView, folder_view));
    CHECK_S_OK(folder_view->Items(SVGIO_SELECTION, IID_IShellItemArray, selected_items));
    CHECK_S_OK(CreateAndInitializeFileOperation(IID_IFileOperation, file_operation));
    CHECK_S_OK(file_operation->DeleteItems(selected_items));
    CHECK_S_OK(file_operation->PerformOperations());
}

void ExplorerBrowser_HandleControlAKeyPress(Pane *pane)
{
    CHECK_EXPLORER_BROWSER_HAS_FOCUS(pane);

    AutoRelease<IFolderView> folder_view;
    int folder_item_count = 0;

    CHECK_S_OK(pane->content.explorer.browser->GetCurrentView(IID_IFolderView, folder_view));
    CHECK_S_OK(folder_view->ItemCount(SVGIO_ALLVIEW, &folder_item_count));

    for(int i = 0; i < folder_item_count; i++)
    {
        folder_view->SelectItem(i, SVSI_SELECT);
    }
}

void ExplorerBrowser_HandleControlCKeyPress(Pane *pane)
{
    CHECK_EXPLORER_BROWSER_HAS_FOCUS(pane);
    
}