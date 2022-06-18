
#include "FilePane_Common.h"

#include "MainWindow.cpp"
#include "NavigationTree.cpp"
#include "ExplorerBrowserCOM.cpp"

void ComputeLayout(HWND hwnd, RECT *rc0, RECT *rc1, RECT *rc2)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    UINT s = 5;
    UINT tvw = 250;
    UINT l = tvw + s * 2;
    UINT w = (rc.right - (tvw + 4 * s)) / 2;
    UINT h = (rc.bottom - 10);

    rc0->top = s;
    rc0->bottom = s + h;
    rc0->left = s;
    rc0->right = rc0->left + tvw;

    rc1->top = s;
    rc1->bottom = s + h;
    rc1->left = rc0->right + s;
    rc1->right = rc1->left + w;

    rc2->top = s;
    rc2->bottom = s + h;
    rc2->left = rc1->right + s;
    rc2->right = rc2->left + w;
}

void ComputeLayout(RECT *rc, Pane *pane)
{
    //Alert(L"compute layout rc: %d %d %d %d %d %d", rc->left, rc->top, rc->right, rc->bottom, pane->id, pane->content_type);
    if (pane->content_type == PaneType::Container) {
        //Alert(L"%d %d %d %f", pane->id, rc->left, rc->right, pane->content.container.split);
        RECT rcl, rcr;
        pane->content.container.rc = *rc;
        rcl.left = rc->left;
        rcl.right = rc->right;
        rcl.top = rc->top;
        rcl.bottom = rc->bottom;
        rcr.left = rc->left;
        rcr.right = rc->right;
        rcr.top = rc->top;
        rcr.bottom = rc->bottom;
        if (pane->content.container.split_direction == SplitDirection::Horizontal) {
            if (pane->content.container.split_type == SplitType::Fixed) {
                rcl.bottom = rcl.top + (int)pane->content.container.split;
            }
            else {
                rcl.bottom = rcl.top + (int)(pane->content.container.split * (rc->bottom - rc->top));
            }
            rcr.top = rcl.bottom;
        }
        else if (pane->content.container.split_direction == SplitDirection::Vertical) {
            if (pane->content.container.split_type == SplitType::Fixed) {
                rcl.right = rcl.left + (int)pane->content.container.split;
            }
            else {
                rcl.right = rcl.left + (int)(pane->content.container.split * (rc->right - rc->left));
            }
            rcr.left = rcl.right;
        }
        //Alert(L"%d %d %d %d", rcl.left, rcl.right, rcr.left, rcr.right);
        Pane *lpane = FilePane_GetPaneById(pane->content.container.lpane_id);
        Pane *rpane = FilePane_GetPaneById(pane->content.container.rpane_id);
        ComputeLayout(&rcl, lpane);
        ComputeLayout(&rcr, rpane);
    } else if (pane->content_type == PaneType::ExplorerBrowser) {
        //Alert(L"browser rc: %d %d %d %d", rc->left, rc->top, rc->right, rc->bottom);
        RECT rc_;
        rc_.left = rc->left + 5;
        rc_.right = rc->right - 5;
        rc_.top = rc->top + 5;
        rc_.bottom = rc->bottom -5;
        pane->content.explorer.browser->SetRect(NULL, rc_);
    } else if (pane->content_type == PaneType::FolderBrowser) {
        //Alert(L"folder rc: %d %d %d %d", rc->left, rc->top, rc->right, rc->bottom);
        RECT rc_;
        rc_.left = rc->left + 5;
        rc_.right = rc->right - 5;
        rc_.top = rc->top + 5;
        rc_.bottom = rc->bottom -5;
        SetWindowPos(pane->content.folder.tree->hwnd, NULL, rc_.left, rc_.top, rc_.right - rc_.left, rc_.bottom - rc_.top, NULL);
    }
}

void ComputeLayout(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    ComputeLayout(&rc, g_panes);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            for (int i = 0; i < g_panes_count; i++)
            {
                //RECT rc,rc0,rc1,rc2;
                //ComputeLayout(hwnd, &rc0, &rc1, &rc2);
                // SetWindowPos(g_nav_tree.hwnd, NULL, rc0.left, rc0.top, rc0.right - rc0.left, rc0.bottom - rc0.top, NULL);
                // _peb1->SetRect(NULL, rc1);
                // _peb2->SetRect(NULL, rc2);

                ComputeLayout(hwnd);
            }
            break;
        case WM_NOTIFY: {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            FolderBrowserPane *folder_pane = FilePane_GetFolderBrowserPane();
            if (folder_pane == NULL) goto DEFWNDPROC;
            if (nmhdr->hwndFrom == folder_pane->tree->hwnd)
            {
                switch (nmhdr->code)
                {
                    case NM_CUSTOMDRAW: {
                        return NavigationTree_OnCustomDraw(folder_pane->tree, (LPNMTVCUSTOMDRAW)nmhdr);
                    } break;
                    case NM_KILLFOCUS: {
                        
                    } break;
                    case NM_SETFOCUS: {

                    } break;
                }
            }}
            break;
    }

    DEFWNDPROC:
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

Pane* InitContainerPane(HWND hwnd)
{
    Pane *pane = FilePane_AllocatePane();
    pane->parent_id = 0;
    pane->content_type = PaneType::Container;
    GetClientRect(hwnd, &pane->content.container.rc);
    pane->content.container.split_direction = SplitDirection::Vertical;
    pane->content.container.split_type = SplitType::Fixed;
    pane->content.container.split = 200;
    return pane;
}

void InitFolderBrowserPane(HWND hwnd, HINSTANCE hInstance, Pane *parent)
{
    NavigationTree *tree = (NavigationTree*)calloc(1, sizeof(NavigationTree));
    Pane *pane = FilePane_AllocatePane();
    pane->parent_id = parent->id;
    pane->content_type = PaneType::FolderBrowser;
    pane->content.folder.tree = tree;
    parent->content.container.lpane_id = pane->id;
    RECT rc = {};
    NavigationTree_Create(tree, hwnd, hInstance, &rc);
    NavigationTree_FillRoot(tree);
}

Pane* InitExplorerBrowserPane(HWND hwnd, Pane *parent)
{
    ASSERT(parent!=NULL&&parent->content_type==PaneType::Container,L"Explorer Parent must be a container!");
    HRESULT hr;
    IExplorerBrowser *browser;
    hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&browser));
    if FAILED(hr) return NULL;

    FOLDERSETTINGS fs = {};
    fs.ViewMode = FVM_LIST;// FVM_LIST; // FVM_CONTENT; //FVM_ICON; // FVM_DETAILS;
    fs.fFlags = FWF_NOWEBVIEW | FWF_NOCOLUMNHEADER;

    IShellFolder *pshf;
    LPITEMIDLIST lpiidl;
    RECT rc = {};
    browser->Initialize(hwnd, &rc, &fs);
    IServiceProvider *service_provider;
    hr = browser->QueryInterface(IID_IServiceProvider, (void**)&service_provider);
    if SUCCEEDED(hr) {
        IOleWindow *window;
        hr = service_provider->QueryService(SID_SShellBrowser, IID_IOleWindow, (void**)&window);
        if SUCCEEDED(hr) {
            HWND hwnd_browser = NULL;
            if SUCCEEDED(window->GetWindow(&hwnd_browser)) {
                LONG style = GetWindowLongW(hwnd_browser, GWL_STYLE);
                style &= ~WS_BORDER;
                SetWindowLongW(hwnd_browser, GWL_STYLE, style);
            }
        }
    }
    SHGetDesktopFolder(&pshf);
    browser->BrowseToObject(pshf, NULL);
    pshf->Release();

    Pane *pane = FilePane_AllocatePane();
    pane->parent_id = parent->id;
    pane->content_type = PaneType::ExplorerBrowser;
    pane->content.explorer.browser = browser;
    parent->content.container.rpane_id = pane->id;
    return pane;
}

void SplitPane(HWND hwnd, Pane *explorer_pane, SplitType split_type, SplitDirection split_direction, float split_value)
{
    ASSERT(explorer_pane!=NULL&&explorer_pane->content_type==PaneType::ExplorerBrowser, L"Only split explorer panes!");

    Pane *parent_container_pane = FilePane_GetPaneById(explorer_pane->parent_id);

    ASSERT(parent_container_pane->content_type==PaneType::Container, L"Parent of explorer was not a container!");

    Pane *container_pane = FilePane_AllocatePane();
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

    InitExplorerBrowserPane(hwnd, container_pane);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    HRESULT hr;
    HWND hwnd;
    ExplorerBrowserCOM service;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
   
    // Initialize GDI+.
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Scale(g_right_arrow_points, ARRAYSIZE(g_right_arrow_points), 10.0/6.0);

    hwnd = CreateMainWindow(hInstance);

    // ensure the dll for common controls is loaded
    InitCommonControls();

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if FAILED(hr) return 1;

    hr = OleInitialize(NULL);
    if FAILED(hr) return 2;

    Pane *primary_pane = InitContainerPane(hwnd);
    InitFolderBrowserPane(hwnd, hInstance, primary_pane);
    Pane *ex1 = InitExplorerBrowserPane(hwnd, primary_pane);
    SplitPane(hwnd, ex1, SplitType::Float, SplitDirection::Vertical, 0.5);
    Pane *ex2 = FilePane_GetPaneById(5);
    SplitPane(hwnd, ex2, SplitType::Float, SplitDirection::Horizontal, 0.5);

    ComputeLayout(hwnd);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    RunMainWindowLoop();

    OleUninitialize();
    CoUninitialize();
    Gdiplus::GdiplusShutdown(gdiplusToken);


    return 0;
}



