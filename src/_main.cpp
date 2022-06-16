
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
    Alert(L"compute layout rc: %d %d %d %d %d %d", rc->left, rc->top, rc->right, rc->bottom, pane->id, pane->content_type);
    if (pane->content_type == PaneType::Container) {
        RECT rcl, rcr;
        pane->content.container.rc = *rc;
        if (pane->content.container.split_direction == SplitDirection::Horizontal) {
            if (pane->content.container.split_type == SplitType::Fixed) {
                //rcl.left = 
            }
            else {

            }
        }
        else if (pane->content.container.split_direction == SplitDirection::Vertical) {
            if (pane->content.container.split_type == SplitType::Fixed) {
                rcl.left = rc->left;
                rcl.right = (int)pane->content.container.split;
                rcl.top = rc->top;
                rcl.bottom = rc->bottom;
                rcr.left = rcl.right + 5;
                rcr.right = rc->right;
                rcr.top = rc->top;
                rcr.bottom = rc->bottom;
                Pane *lpane = FilePane_GetPaneById(pane->content.container.lpane_id);
                Pane *rpane = FilePane_GetPaneById(pane->content.container.rpane_id);
                Alert(L"lpane: %p, rpane: %p", lpane, rpane);
                Alert(L"IDs: %d, %d, %d", g_panes[0].id, g_panes[1].id, g_panes[2].id);
                ComputeLayout(&rcl, lpane);
                ComputeLayout(&rcr, rpane);
            }
            else {

            }
        }
    } else if (pane->content_type == PaneType::ExplorerBrowser) {
        Alert(L"browser rc: %d %d %d %d", rc->left, rc->top, rc->right, rc->bottom);
        pane->content.explorer.browser->SetRect(NULL, *rc);
    } else if (pane->content_type == PaneType::FolderBrowser) {
        Alert(L"folder rc: %d %d %d %d", rc->left, rc->top, rc->right, rc->bottom);
        SetWindowPos(pane->content.folder.tree->hwnd, NULL, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, NULL);
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

Pane* InitContainerPane(HWND hwnd, HINSTANCE hInstance)
{
    Pane *pane = &g_panes[0];
    pane->id = NextId();
    pane->parent_id = 0;
    pane->content_type = PaneType::Container;
    GetClientRect(hwnd, &pane->content.container.rc);
    pane->content.container.split_direction = SplitDirection::Vertical;
    pane->content.container.split_type = SplitType::Fixed;
    pane->content.container.split = 200;
    g_panes_count++;
    return pane;
}

void InitFolderBrowserPane(HWND hwnd, HINSTANCE hInstance, Pane *parent)
{
    NavigationTree *tree = (NavigationTree*)calloc(1, sizeof(NavigationTree));
    Pane *pane = &g_panes[1];
    pane->id = NextId();
    pane->parent_id = parent->id;
    pane->content_type = PaneType::FolderBrowser;
    pane->content.folder.tree = tree;
    parent->content.container.lpane_id = pane->id;
    RECT rc = {};
    NavigationTree_Create(tree, hwnd, hInstance, &rc);
    NavigationTree_FillRoot(tree);
    g_panes_count++;
    //Alert(L"InitFolderBrowserPane  done");
}

void InitExplorerBrowserPane(HWND hwnd, Pane *parent)
{
    HRESULT hr;
    IExplorerBrowser *browser;
    hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&browser));
    if FAILED(hr) return;

    FOLDERSETTINGS fs = {};
    fs.ViewMode = FVM_LIST;// FVM_LIST; // FVM_CONTENT; //FVM_ICON; // FVM_DETAILS;
    fs.fFlags = FWF_NOWEBVIEW | FWF_NOCOLUMNHEADER;

    IShellFolder *pshf;
    LPITEMIDLIST lpiidl;
    RECT rc = {};
    browser->Initialize(hwnd, &rc, &fs);
    SHGetDesktopFolder(&pshf);
    browser->BrowseToObject(pshf, NULL);
    pshf->Release();

    Pane *pane = &g_panes[2];
    pane->id = NextId();
    pane->parent_id = parent->id;
    pane->content_type = PaneType::ExplorerBrowser;
    pane->content.explorer.browser = browser;
    parent->content.container.rpane_id = pane->id;
    g_panes_count++;
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

    //Alert(L"TEST %d, %s", 1, L"two");

    // ensure the dll for common controls is loaded
    InitCommonControls();

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if FAILED(hr) return 1;

    hr = OleInitialize(NULL);
    if FAILED(hr) return 2;

    //hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_peb1));
    if FAILED(hr) return 3;

    //hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_peb2));
    if FAILED(hr) return 3;

    //RECT rc,rc0,rc1,rc2;
    //ComputeLayout(hwnd, &rc0, &rc1, &rc2);

    FOLDERSETTINGS fs = {};
    fs.ViewMode = FVM_LIST;// FVM_LIST; // FVM_CONTENT; //FVM_ICON; // FVM_DETAILS;
    fs.fFlags = FWF_NOWEBVIEW | FWF_NOCOLUMNHEADER;
    //fs.fFlags = FWF_NOWEBVIEW | FWF_NOCOLUMNHEADER;
    //IUnknown_SetSite(_peb1, static_cast<IServiceProvider *>(&service));
    DWORD cookie;
    //_peb1->Advise(&service, &cookie);

    // IShellFolder *pshf;
    // LPITEMIDLIST lpiidl;

    Pane *primary_pane = InitContainerPane(hwnd, hInstance);

    InitFolderBrowserPane(hwnd, hInstance, primary_pane);
    InitExplorerBrowserPane(hwnd, primary_pane);

    // _peb1->Initialize(hwnd, &rc1, &fs);
    // SHGetDesktopFolder(&pshf);
    // _peb1->BrowseToObject(pshf, NULL);
    // pshf->Release();
    
    // _peb2->Initialize(hwnd, &rc2, &fs);
    // SHGetSpecialFolderLocation(hwnd, CSIDL_DRIVES, &lpiidl);
    // _peb2->BrowseToIDList(lpiidl, NULL);
    // CoTaskMemFree(lpiidl);

    ComputeLayout(hwnd);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    RunMainWindowLoop();

    // _peb1->Release();
    // _peb2->Release();

    OleUninitialize();
    CoUninitialize();
    Gdiplus::GdiplusShutdown(gdiplusToken);


    return 0;
}



