
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
        QITABENT(ExplorerBrowserEvents, ICommDlgBrowser2),
        QITABENT(ExplorerBrowserEvents, IExplorerPaneVisibility),
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
    else if (guidService == SID_ExplorerPaneVisibility)
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
    DEBUGWRITE(L"NAVIGATION FAILED");

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

// ICommDlgBrowser2
HRESULT ExplorerBrowserEvents::Notify(IShellView *shell_view, DWORD notify_type)
{
    UNREFERENCED_PARAMETER(shell_view);
    UNREFERENCED_PARAMETER(notify_type);
    return E_NOTIMPL;
}

HRESULT ExplorerBrowserEvents::GetDefaultMenuText(IShellView *shell_view, WCHAR *buffer, int buffer_size)
{
    UNREFERENCED_PARAMETER(shell_view);
    UNREFERENCED_PARAMETER(buffer);
    UNREFERENCED_PARAMETER(buffer_size);
    return E_NOTIMPL;
}

HRESULT ExplorerBrowserEvents::GetViewFlags(DWORD *flags)
{
    *flags = CDB2GVF_SHOWALLFILES;
    return S_OK;
}

// IExplorerPaneVisibility
IFACEMETHODIMP ExplorerBrowserEvents::GetPaneState(REFEXPLORERPANE ep, EXPLORERPANESTATE *peps)
{
    if (ep == EP_NavPane || ep == EP_Commands || ep == EP_Commands_Organize || ep == EP_Commands_View)
    {
        *peps = EPS_FORCE | EPS_DEFAULT_OFF;
    }
    else
    {
        *peps = EPS_DONTCARE;
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

bool ExplorerBrowser_GetPath(Pane *pane, WCHAR *path_buffer, int buffer_size)
{
    if (pane == NULL || pane->content_type != PaneType::ExplorerBrowser) return false;
    Edit_GetText(pane->content.explorer.txt_path, path_buffer, buffer_size);
    return true;
}

void ExplorerBrowser_HandleDeleteKeyPress()
{
    Pane *pane = FilePane_GetActiveExplorerPane();
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

void ExplorerBrowser_HandleControlAKeyPress()
{
    Pane *pane = FilePane_GetActiveExplorerPane();
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

void ExplorerBrowser_HandleControlCXKeyPress(DWORD drop_effect)
{
    Pane *pane = FilePane_GetActiveExplorerPane();
    CHECK_EXPLORER_BROWSER_HAS_FOCUS(pane);
    AutoRelease<IFolderView> folder_view;
    AutoRelease<IShellFolder> shell_folder;
    AutoRelease<IShellItem> folder_shell_item;
    AutoRelease<IShellItemArray> selected_items;
    AutoRelease<IFileOperation> file_operation;
    AutoCoMemFree<ITEMIDLIST> folder_pidl;
    DWORD cidl = 0;
    AutoRelease<IDataObject> data_object;
    CHECK_S_OK(pane->content.explorer.browser->GetCurrentView(IID_IFolderView, folder_view));
    CHECK_S_OK(folder_view->Items(SVGIO_SELECTION, IID_IShellItemArray, selected_items));
    CHECK_S_OK(folder_view->GetFolder(IID_IShellItem, folder_shell_item));
    CHECK_S_OK(SHGetIDListFromObject(folder_shell_item, folder_pidl));
    CHECK_S_OK(selected_items->GetCount(&cidl));
    if (cidl == 0) return;

    AutoLocalFree<HLOCAL> item_mem(LocalAlloc(GPTR, sizeof(LPITEMIDLIST) * cidl));
    LPITEMIDLIST *items = (LPITEMIDLIST*)item_mem.instance;
    for(DWORD i = 0; i < cidl; i++) {
        IShellItem *shell_item;
        if (S_OK == selected_items->GetItemAt(i, &shell_item)) {
            SHGetIDListFromObject(shell_item, &items[i]);
            shell_item->Release();
        }
    }
    LPCITEMIDLIST *apidl = (LPCITEMIDLIST*)(void*)items;

    AutoRelease<IShellFolder> shell_desktop;
    CHECK_S_OK(SHGetDesktopFolder(&shell_desktop));
    CHECK_S_OK(shell_desktop->GetUIObjectOf(NULL, cidl, apidl, IID_IDataObject, NULL, data_object));

    FORMATETC format = {};
    format.cfFormat = CF_PREFFEREDDROPEFFECT;
    format.tymed = TYMED_HGLOBAL;
    format.dwAspect = DVASPECT_CONTENT;
    format.lindex = -1;

    STGMEDIUM medium = {};
    DWORD *pde = (DWORD*)GlobalAlloc(GPTR, sizeof(DWORD));
    *pde = drop_effect;
    medium.hGlobal = pde;
    medium.tymed = TYMED_HGLOBAL;
    medium.pUnkForRelease = NULL;

    if (FAILED(data_object->SetData(&format, &medium, TRUE))) {
         GlobalFree(pde);
         DEBUGWRITE(L"SetData failed when writing drop effect");
         return;
    }

    CHECK_S_OK(OleSetClipboard(data_object));

    DEBUGWRITE(L"ExplorerBrowser_HandleControlCXKeyPress success");
}

void ExplorerBrowser_HandleControlVKeyPress()
{
    Pane *pane = FilePane_GetActiveExplorerPane();
    CHECK_EXPLORER_BROWSER(pane);

    AutoRelease<IDataObject> clipboard_object;
    AutoRelease<IFolderView> folder_view;
    AutoRelease<IFileOperation> file_operation;
    AutoRelease<IShellItem> destination_shell_item;

    CHECK_S_OK(OleGetClipboard(clipboard_object));

    FORMATETC format = {};
    STGMEDIUM stgmedium = {};
    LPVOID global_value = 0;
    DWORD drop_effect = 0;

    format.cfFormat = CF_PREFFEREDDROPEFFECT;
    format.tymed = TYMED_HGLOBAL;
    format.dwAspect = DVASPECT_CONTENT;

    CHECK_S_OK(clipboard_object->GetData(&format, &stgmedium));
    global_value = GlobalLock(stgmedium.hGlobal);
    drop_effect = *(DWORD*)global_value;
    GlobalUnlock(stgmedium.hGlobal);
    ReleaseStgMedium(&stgmedium);

    CHECK_S_OK(pane->content.explorer.browser->GetCurrentView(IID_IFolderView, folder_view));
    CHECK_S_OK(CreateAndInitializeFileOperation(IID_IFileOperation, file_operation));
    CHECK_S_OK(folder_view->GetFolder(IID_IShellItem, destination_shell_item));
    if ((drop_effect & DROPEFFECT_COPY) == DROPEFFECT_COPY) {
        CHECK_S_OK(file_operation->CopyItems(clipboard_object, destination_shell_item));
    }
    else if ((drop_effect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE) {
        CHECK_S_OK(file_operation->MoveItems(clipboard_object, destination_shell_item));
    }
    CHECK_S_OK(file_operation->PerformOperations());

    DEBUGWRITE(L"ExplorerBrowser_HandleControlVKeyPress success");
}

