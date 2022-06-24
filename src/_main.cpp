
#include "FilePane_Common.h"

#include "MainWindow.cpp"
#include "NavigationTree.cpp"
#include "ExplorerBrowserEvents.cpp"

void ComputeLayout(RECT *rc, Pane *pane)
{
    int half_margin = 2;
    //Alert(L"compute layout rc: %d %d %d %d %d %d", rc->left, rc->top, rc->right, rc->bottom, pane->id, pane->content_type);
    if (pane->content_type == PaneType::Container) {
        //Alert(L"%d %d %d %f", pane->id, rc->left, rc->right, pane->content.container.split);
        RECT rcl, rcr;
        pane->rc = *rc;
        rcl = *rc;
        rcr = *rc;
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
        pane->rc.left = rc->left + half_margin;
        pane->rc.right = rc->right - half_margin;
        pane->rc.top = rc->top + half_margin;
        pane->rc.bottom = rc->bottom - half_margin;
        
        RECT pos;
        pos.left = pane->rc.left;
        pos.top = pane->rc.top;
        pos.right = pane->rc.right;
        pos.bottom = pane->rc.top + 30;
        SetWindowPos(pane->content.explorer.txt_uri, NULL,
            pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, NULL);
        //Edit_SetRect(pane->content.explorer.txt_uri, &pos);
        
        pos.top = pos.bottom;
        pos.bottom = pane->rc.bottom;
        pane->content.explorer.browser->SetRect(NULL, pos);
    } else if (pane->content_type == PaneType::FolderBrowser) {
        //Alert(L"folder rc: %d %d %d %d", rc->left, rc->top, rc->right, rc->bottom);
        pane->rc.left = rc->left + half_margin;
        pane->rc.right = rc->right - half_margin;
        pane->rc.top = rc->top + half_margin;
        pane->rc.bottom = rc->bottom - half_margin;
        SetWindowPos(pane->content.folder.tree->hwnd, NULL, 
            pane->rc.left, pane->rc.top, pane->rc.right - pane->rc.left, pane->rc.bottom - pane->rc.top, NULL);
    }
}

void ComputeLayout(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.bottom -= 2;
    rc.left += 2;
    rc.right -= 2;
    rc.top += 2;
    ComputeLayout(&rc, g_panes);
}

void DrawExplorerFrame(Pane *pane, HDC hdc, HBRUSH brush)
{
    RECT left, top, right, bottom;
    int margin = 4;
    
    left.left = pane->rc.left - margin;
    left.top = pane->rc.top - margin;
    left.right = pane->rc.left;
    left.bottom = pane->rc.bottom + margin;

    top.left = pane->rc.left - margin;
    top.top = pane->rc.top - margin;
    top.right = pane->rc.right + margin;
    top.bottom = pane->rc.top;

    right.left = pane->rc.right;
    right.top = pane->rc.top - margin;
    right.right = pane->rc.right + margin;
    right.bottom = pane->rc.bottom + margin;

    bottom.left = pane->rc.left - margin;
    bottom.top = pane->rc.bottom;
    bottom.right = pane->rc.right + margin;
    bottom.bottom = pane->rc.bottom + margin;

    FillRect(hdc, &left, brush);
    FillRect(hdc, &top, brush);
    FillRect(hdc, &right, brush);
    FillRect(hdc, &bottom, brush);
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

Pane* InitExplorerBrowserPane(HWND hwnd, HINSTANCE hInstance, Pane *parent)
{
    ASSERT(parent!=NULL&&parent->content_type==PaneType::Container,L"Explorer Parent must be a container!");
    HRESULT hr;

    // create the com instance for IExplorerBrowser
    IExplorerBrowser *browser;
    hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&browser));
    if FAILED(hr) return NULL;

    // initialize the ExplorerBrowser
    FOLDERSETTINGS fs = {};
    RECT rc = {};
    fs.ViewMode = FVM_DETAILS;// FVM_LIST; // FVM_CONTENT; //FVM_ICON; // FVM_DETAILS;
    fs.fFlags = FWF_NOWEBVIEW; // | FWF_NOCOLUMNHEADER;
    browser->SetOptions(EBO_NOBORDER | EBO_NOWRAPPERWINDOW);
    browser->Initialize(hwnd, &rc, &fs);

    // subscribe to events from the explorer window
    ExplorerBrowserEvents *browser_events = new ExplorerBrowserEvents();
    DWORD cookie;
    hr = browser->Advise(browser_events, &cookie);
    ASSERT(hr==S_OK, L"could not subscribe to IExplorerBrowser events!");

    // create the Pane
    Pane *pane = FilePane_AllocatePane();
    pane->parent_id = parent->id;
    pane->content_type = PaneType::ExplorerBrowser;
    pane->content.explorer.browser = browser;
    pane->content.explorer.events = browser_events;
    pane->content.explorer.event_cookie = cookie;
    parent->content.container.rpane_id = pane->id;
    browser_events->SetPaneId(pane->id);

    // create he address text box
    pane->content.explorer.txt_uri = CreateWindowExW(
          0  // dwExStyle
        , L"EDIT" // class name
        , NULL // window name
        , WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL // dwStyle
        , 0, 0, 0, 0 // x y w h
        , hwnd // parent
        , NULL // hmenu
        , hInstance // GetWindowLongPtr(hwnd, GWLP_HINSTANCE)
        , NULL // lpParam
    );
    SetWindowFont(pane->content.explorer.txt_uri, GetWindowFont(hwnd), NULL);

    // browse to folder location
    IShellFolder *pshf;
    SHGetDesktopFolder(&pshf);
    browser->BrowseToObject(pshf, NULL);
    pshf->Release();

    return pane;
}

void SplitPane(HWND hwnd, HINSTANCE hInstance, Pane *explorer_pane, SplitType split_type, SplitDirection split_direction, float split_value)
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

    InitExplorerBrowserPane(hwnd, hInstance, container_pane);
}

POINT GetPoint(HWND hwnd, MSG *msg)
{
    POINTS pts = MAKEPOINTS(msg->lParam);
    POINT pt = {};
    POINTSTOPOINT(pt, pts);
    ClientToScreen(msg->hwnd, &pt);
    ScreenToClient(hwnd, &pt);
    return pt;
}

/*
    PreDispatchMessage allows this program to respond to
    messages that are destined for hosted windows
*/
void PreDispatchMessage(HWND hwnd, MSG *msg)
{
    if (msg->message == WM_LBUTTONDOWN)
    {
        POINT pt = GetPoint(hwnd, msg);
        Pane *pane = FilePane_GetExplorerPaneByPt(pt);
        if (pane != NULL && !pane->content.explorer.focused) {
            FilePane_SetFocus(pane->id);
            InvalidateRect(hwnd, NULL, FALSE);
        }
    }
    else if(msg->message == WM_XBUTTONDOWN)
    {
        POINT pt = GetPoint(hwnd, msg);
        UINT button = GET_XBUTTON_WPARAM(msg->wParam);
        Pane *pane = FilePane_GetExplorerPaneByPt(pt);
        if (pane != NULL) {
            if (button == XBUTTON1) { // back
                pane->content.explorer.browser->BrowseToIDList(NULL, SBSP_NAVIGATEBACK);
            }
            else if (button == XBUTTON2) { // forward
                pane->content.explorer.browser->BrowseToIDList(NULL, SBSP_NAVIGATEFORWARD);
            }
            //IFolderView *ppv;
            //pane->content.explorer.browser->BrowseToObject
            // if (SUCCEEDED(pane->content.explorer.browser->GetCurrentView(IID_IFolderView, (void**)&ppv))) {
                
            // }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            ComputeLayout(hwnd);
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
                        folder_pane->tree->focused = false;
                        InvalidateRect(folder_pane->tree->hwnd, NULL, FALSE);
                    } break;
                    case NM_SETFOCUS: {
                        folder_pane->tree->focused = true;
                        InvalidateRect(folder_pane->tree->hwnd, NULL, FALSE);
                    } break;
                }
            }} break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH active_brush = CreateSolidBrush(RGB(200,200,200));
            HBRUSH inactive_brush = CreateSolidBrush(RGB(240,240,240));
            Pane *active_pane = NULL;
            BEGIN_ENUM_EXPLORERS
                if (pane->content.explorer.focused) {
                    active_pane = pane;
                }
                else {
                    DrawExplorerFrame(pane, hdc, inactive_brush);
                }
            END_ENUM_EXPLORERS
            if (active_pane != NULL) {
                DrawExplorerFrame(active_pane, hdc, active_brush);
            }
            DeleteObject(active_brush);
            DeleteObject(inactive_brush);
            EndPaint(hwnd, &ps);
        } break;
    }

    DEFWNDPROC:
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    HRESULT hr;
    HWND hwnd;
    MSG msg;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    // ensure the dll for common controls is loaded
    InitCommonControls();
   
    // Initialize GDI+.
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Scale(g_right_arrow_points, ARRAYSIZE(g_right_arrow_points), 10.0/6.0);

    hwnd = CreateMainWindow(hInstance);

    // required for all COM calls later
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if FAILED(hr) return 1;

    // enabled drag and drop between explorer windows
    hr = OleInitialize(NULL);
    if FAILED(hr) return 2;

    Pane *primary_pane = InitContainerPane(hwnd);
    InitFolderBrowserPane(hwnd, hInstance, primary_pane);
    Pane *ex1 = InitExplorerBrowserPane(hwnd, hInstance, primary_pane);
    SplitPane(hwnd, hInstance, ex1, SplitType::Float, SplitDirection::Vertical, 0.5);
    Pane *ex2 = FilePane_GetPaneById(5);
    SplitPane(hwnd, hInstance, ex2, SplitType::Float, SplitDirection::Horizontal, 0.5);

    ComputeLayout(hwnd);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // main message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        PreDispatchMessage(hwnd, &msg);
        DispatchMessage(&msg);
    }

    // cleanup
    OleUninitialize();
    CoUninitialize();
    Gdiplus::GdiplusShutdown(gdiplusToken);


    return 0;
}

