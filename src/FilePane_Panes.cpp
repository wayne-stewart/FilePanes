
#include "FilePane_Common.h"

inline void FilePane_SetSplitHandleCursor(Pane *pane)
{
    if (pane->content.container.split_direction == SplitDirection::Horizontal) {
        SetCursor(g_idc_sizens);
    }
    else if (pane->content.container.split_direction == SplitDirection::Vertical) {
        SetCursor(g_idc_sizewe);
    }
}

Pane* FilePane_GetFolderBrowserPane() {
    for(int i = 0; i < MAX_PANES; i++) {
        if (g_panes[i].content_type == PaneType::FolderBrowser) {
            return &g_panes[i];
        }
    }
    return NULL;
}

Pane* FilePane_GetPaneById(int id)
{
    Pane *pane = &g_panes[id];
    return pane;
}

Pane* FilePane_GetRootPane()
{
    Pane *pane = FilePane_GetPaneById(1);
    return pane;
}

Pane* FilePane_AllocatePane()
{
    for(int i = 1; i < MAX_PANES; i++) {
        Pane *pane = &g_panes[i];
        if (pane->content_type == PaneType::NotSet) {
            memset(pane, 0, sizeof(Pane));
            pane->id = i;
            g_panes_count++;
            return pane;
        }
    }
    return NULL;
}

void FilePane_DeallocatePane(Pane *pane)
{
    if (pane == NULL) return;
    if (pane->content_type == PaneType::ExplorerBrowser) {
        pane->content.explorer.browser->Destroy();
        DestroyWindow(pane->content.explorer.txt_path);
        DestroyWindow(pane->content.explorer.btn_split_h);
        DestroyWindow(pane->content.explorer.tt_split_h);
        DestroyWindow(pane->content.explorer.btn_split_v);
        DestroyWindow(pane->content.explorer.tt_split_v);
        DestroyWindow(pane->content.explorer.btn_back);
        DestroyWindow(pane->content.explorer.tt_back);
        DestroyWindow(pane->content.explorer.btn_up);
        DestroyWindow(pane->content.explorer.tt_up);
        DestroyWindow(pane->content.explorer.btn_refresh);
        DestroyWindow(pane->content.explorer.tt_refresh);
        DestroyWindow(pane->content.explorer.btn_remove);
        DestroyWindow(pane->content.explorer.tt_remove);
    }
    else if(pane->content_type == PaneType::FolderBrowser) {
        CloseHandle(pane->content.folder.tree->hwnd);
        DeleteObject(pane->content.folder.tree->font);
        ImageList_Destroy(pane->content.folder.tree->image_list);
    }
    else if (pane->content_type == PaneType::Container) {
        FilePane_DeallocatePane(FilePane_GetPaneById(pane->content.container.lpane_id));
        FilePane_DeallocatePane(FilePane_GetPaneById(pane->content.container.rpane_id));
    }
    g_panes_count--;
    memset(pane, 0, sizeof(Pane));
}

Pane* FilePane_GetExplorerPaneById(int id)
{
    Pane *pane = FilePane_GetPaneById(id);
    if (pane == NULL) return NULL;
    if (pane->content_type == PaneType::ExplorerBrowser) return pane;
    return NULL;
}

Pane* FilePane_GetActiveExplorerPane()
{
    // first try to get the currently focused pane
    BEGIN_ENUM_EXPLORERS
    if (pane->content.explorer.focused) return pane;
    END_ENUM_EXPLORERS

    // if there was none, get the first explorer pane we find
    BEGIN_ENUM_EXPLORERS
    return pane;
    END_ENUM_EXPLORERS

    return NULL;
}

void FilePane_SetFocus(int id)
{
    BEGIN_ENUM_EXPLORERS
    if (pane->id == id) {
        pane->content.explorer.focused = true;
    }
    else {
        pane->content.explorer.focused = false;
    }
    END_ENUM_EXPLORERS
}

Pane* FilePane_GetExplorerPaneByPt(POINT pt)
{
    BEGIN_ENUM_EXPLORERS
    if (PtInRect(&pane->rc, pt)) return pane;
    END_ENUM_EXPLORERS
    return NULL;
}

Pane* InitContainerPane(HWND hwnd)
{
    Pane *pane = FilePane_AllocatePane();
    pane->parent_id = 0;
    pane->content_type = PaneType::Container;
    GetClientRect(hwnd, &pane->rc);
    pane->content.container.split_direction = SplitDirection::Vertical;
    pane->content.container.split_type = SplitType::Fixed;
    pane->content.container.split = 200;
    return pane;
}

void InitFolderBrowserPane(Pane *parent)
{
    Pane *pane = FilePane_AllocatePane();
    pane->parent_id = parent->id;
    pane->content_type = PaneType::FolderBrowser;
    parent->content.container.lpane_id = pane->id;
    InitFolderBrowserPaneUI(pane);
}

void InitFolderBrowserPaneUI(Pane *pane)
{
    if (pane->id == 0) return;
    FolderBrowserTree *tree = (FolderBrowserTree*)calloc(1, sizeof(FolderBrowserTree));
    pane->content.folder.tree = tree;
    FolderBrowser_Create(tree, g_main_window_hwnd, g_hinstance, &pane->rc);
    FolderBrowser_FillRoot(tree);
}

Pane* InitExplorerBrowserPane(Pane *parent)
{
    ASSERT(parent!=NULL&&parent->content_type==PaneType::Container,L"Explorer Parent must be a container!");

    // create the Pane
    Pane *pane = FilePane_AllocatePane();
    pane->parent_id = parent->id;
    pane->content_type = PaneType::ExplorerBrowser;
    parent->content.container.rpane_id = pane->id;

    InitExplorerBrowserPaneUI(pane, NULL);

    return pane;
}

void InitExplorerBrowserPaneUI(Pane *pane, LPCWSTR path)
{
    HRESULT hr;

    // create the com instance for IExplorerBrowser
    IExplorerBrowser *browser;
    hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&browser));
    if FAILED(hr) return;

    // initialize the ExplorerBrowser
    FOLDERSETTINGS fs = {};
    fs.ViewMode = FVM_DETAILS;// FVM_LIST; // FVM_CONTENT; //FVM_ICON; // FVM_DETAILS;
    fs.fFlags = FWF_NOWEBVIEW; // | FWF_NOCOLUMNHEADER;
    browser->SetOptions(EBO_NOBORDER | EBO_NOWRAPPERWINDOW);
    browser->Initialize(g_main_window_hwnd, &pane->rc, &fs);

    // subscribe to events from the explorer window
    ExplorerBrowserEvents *browser_events = new ExplorerBrowserEvents();
    browser_events->SetPaneId(pane->id);

    DWORD cookie;
    hr = browser->Advise(browser_events, &cookie);
    ASSERT(hr==S_OK, L"could not subscribe to IExplorerBrowser events!");

    pane->content.explorer.browser = browser;
    pane->content.explorer.events = browser_events;
    pane->content.explorer.event_cookie = cookie;

    // create pane controls
    pane->content.explorer.txt_path = CreateTextBox(g_main_window_hwnd, g_hinstance, pane->id);
    pane->content.explorer.btn_split_h = CreateButton(g_main_window_hwnd, g_hinstance, L"Split Horizontal", pane->id, ButtonFunction::SplitHorizontal);
    pane->content.explorer.tt_split_h = CreateToolTip(g_main_window_hwnd, pane->content.explorer.btn_split_h, L"Split Horizontal");
    pane->content.explorer.btn_split_v = CreateButton(g_main_window_hwnd, g_hinstance, L"Split Vertical", pane->id, ButtonFunction::SplitVertical);
    pane->content.explorer.tt_split_v = CreateToolTip(g_main_window_hwnd, pane->content.explorer.btn_split_v, L"Split Vertical");
    pane->content.explorer.btn_back = CreateButton(g_main_window_hwnd, g_hinstance, L"Go Back", pane->id, ButtonFunction::Back);
    pane->content.explorer.tt_back = CreateToolTip(g_main_window_hwnd, pane->content.explorer.btn_back, L"Go Back");
    pane->content.explorer.btn_up = CreateButton(g_main_window_hwnd, g_hinstance, L"Go Up a Directory", pane->id, ButtonFunction::Up);
    pane->content.explorer.tt_up = CreateToolTip(g_main_window_hwnd, pane->content.explorer.btn_up, L"Go Up a Directory");
    pane->content.explorer.btn_refresh = CreateButton(g_main_window_hwnd, g_hinstance, L"Refresh", pane->id, ButtonFunction::Refresh);
    pane->content.explorer.tt_refresh = CreateToolTip(g_main_window_hwnd, pane->content.explorer.btn_refresh, L"Refresh");
    pane->content.explorer.btn_remove = CreateButton(g_main_window_hwnd, g_hinstance, L"Remove Pane", pane->id, ButtonFunction::Remove);
    pane->content.explorer.tt_remove = CreateToolTip(g_main_window_hwnd, pane->content.explorer.btn_remove, L"Remove Pane");

    // browse to folder location
    if (path == NULL) {
        IShellFolder *pshf;
        SHGetDesktopFolder(&pshf);
        browser->BrowseToObject(pshf, NULL);
        pshf->Release();
    }
    else {
        ExplorerBrowser_SetPath(path, pane);
    }
}

void SplitPane(Pane *explorer_pane, SplitType split_type, SplitDirection split_direction, float split_value)
{
    ASSERT(explorer_pane!=NULL&&explorer_pane->content_type==PaneType::ExplorerBrowser, L"Only split explorer panes!");

    Pane *parent_container_pane = FilePane_GetPaneById(explorer_pane->parent_id);

    ASSERT(parent_container_pane->content_type==PaneType::Container, L"Parent of explorer was not a container!");

    Pane *container_pane = FilePane_AllocatePane();
    if (container_pane == NULL) return;

    container_pane->content_type = PaneType::Container;
    container_pane->content.container.split_type = split_type;
    container_pane->content.container.split_direction = split_direction;
    container_pane->content.container.split = split_value;
    container_pane->content.container.lpane_id = explorer_pane->id;
    container_pane->parent_id = parent_container_pane->id;
    explorer_pane->parent_id = container_pane->id;

    if (parent_container_pane->content.container.lpane_id == explorer_pane->id) {
        parent_container_pane->content.container.lpane_id = container_pane->id;
    }
    else if (parent_container_pane->content.container.rpane_id == explorer_pane->id) {
        parent_container_pane->content.container.rpane_id = container_pane->id;
    }

    Pane *pane = InitExplorerBrowserPane(container_pane);

    FilePane_SetFocus(pane->id);
}

void ReplaceParentWithChildOnGrandParent(Pane *grand_parent, Pane *parent, Pane *child)
{
    if (grand_parent->content.container.lpane_id == parent->id) {
        grand_parent->content.container.lpane_id = child->id;
    }
    else if (grand_parent->content.container.rpane_id == parent->id) {
        grand_parent->content.container.rpane_id = child->id;
    }
    else {
        return;
    }

    child->parent_id = grand_parent->id;
    if (parent->content.container.lpane_id == child->id) {
        parent->content.container.lpane_id = NULL;
    }
    else if (parent->content.container.rpane_id == child->id) {
        parent->content.container.rpane_id = NULL;
    }
}

void RemovePane(Pane *pane)
{
    // bad input to function, just return without doing anything
    if (pane == NULL) return;

    // only explorer panes can be removed.
    if (pane->content_type != PaneType::ExplorerBrowser) return;

    // the root pane is always at the 1st index
    // if the parent is the root, then this is the last explorer pane
    // never want to remove the last explorer pane
    if (pane->parent_id == 1) return;

    Pane *parent = NULL, *remaining_pane = NULL, *grand_parent = NULL;
    parent = FilePane_GetPaneById(pane->parent_id);

    // if the parent was not found or the parent is not a container
    // just return without doing anything
    if(parent == NULL || parent->content_type  != PaneType::Container) return;

    grand_parent = FilePane_GetPaneById(parent->parent_id);

    // since we don't support removing explorer directly off the root,
    // there should always be a grand parent.
    if (grand_parent == NULL || parent->content_type != PaneType::Container) return;

    if (parent->content.container.lpane_id == pane->id) {
        remaining_pane = FilePane_GetPaneById(parent->content.container.rpane_id);
    }
    else if (parent->content.container.rpane_id == pane->id) {
        remaining_pane = FilePane_GetPaneById(parent->content.container.lpane_id);
    }
    else {
        // unable to determine which position the pane to be deleted is in...
        // just return without doing anything
        return;
    }

    // could not find the remaining pane, just return without doing anything
    if (remaining_pane == NULL) return;

    // Panes are organized into a binary tree (not balanced) where containers always have two
    // children and are considered branch nodes. Leaf nodes are always the folder browser or
    // explorer panes. Both leaf nodes of a container are always filled. When removing a Leaf
    // node (explorer pane), also remove the container and set the remaining explorer pane
    // as child of the container's parent container.

    ReplaceParentWithChildOnGrandParent(grand_parent, parent, remaining_pane);

    // this should deallocate the parent and the removed pane
    FilePane_DeallocatePane(parent);
    ASSERT(pane->content_type == PaneType::NotSet, L"removed pane was not deallocated!");
    ASSERT(parent->content_type == PaneType::NotSet, L"container pane was not deallocated");
    ASSERT(remaining_pane->parent_id == grand_parent->id, L"remaining pane did not get added to grandparent");
}

