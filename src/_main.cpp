
#include "FilePane_Common.h"

#include "MainWindow.cpp"
#include "FolderBrowser.cpp"
#include "ExplorerBrowser.cpp"

void ComputeLayout(RECT *rc, Pane *pane)
{
    if (pane->content_type == PaneType::Container) {
        RECT rcl, rcr, *split_handle;
        pane->rc = *rc;
        rcl = *rc;
        rcr = *rc;
        split_handle = &pane->content.container.split_handle;
        if (pane->content.container.split_direction == SplitDirection::Horizontal) {
            if (pane->content.container.split_type == SplitType::Fixed) {
                rcl.bottom = rcl.top + (int)pane->content.container.split;
            }
            else {
                rcl.bottom = rcl.top + (int)(pane->content.container.split * float(rc->bottom - rc->top));
            }
            rcr.top = rcl.bottom;
            split_handle->top = rcl.bottom - HALF_FRAME_WIDTH;
            split_handle->bottom = split_handle->top + FRAME_WIDTH;
            split_handle->left = rcl.left;
            split_handle->right = rcl.right;
        }
        else if (pane->content.container.split_direction == SplitDirection::Vertical) {
            if (pane->content.container.split_type == SplitType::Fixed) {
                rcl.right = rcl.left + (int)pane->content.container.split;
            }
            else {
                rcl.right = rcl.left + (int)(pane->content.container.split * float(rc->right - rc->left));
            }
            rcr.left = rcl.right;
            split_handle->left = rcl.right - HALF_FRAME_WIDTH;
            split_handle->right = split_handle->left + FRAME_WIDTH;
            split_handle->top = rcl.top;
            split_handle->bottom = rcl.bottom;
        }
        Pane *lpane = FilePane_GetPaneById(pane->content.container.lpane_id);
        Pane *rpane = FilePane_GetPaneById(pane->content.container.rpane_id);
        ComputeLayout(&rcl, lpane);
        ComputeLayout(&rcr, rpane);
    } else if (pane->content_type == PaneType::ExplorerBrowser) {

        pane->rc = *rc;
        SHRINK_RECT(pane->rc, HALF_FRAME_WIDTH);
        
        RECT pos;
        pos.left = pane->rc.left;
        pos.top = pane->rc.top;
        pos.right = pane->rc.right - 60;
        pos.bottom = pane->rc.top + 30;
        SetWindowPos(pane->content.explorer.txt_path, NULL,
            pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, NULL);
        RECT offset = pos;
        OffsetRect(&offset, -pos.left + 5, -pos.top + 5);
        Edit_SetRect(pane->content.explorer.txt_path, &offset);

        pos.left = pos.right;
        pos.right += 30;
        SetWindowPos(pane->content.explorer.btn_split_h, NULL,
            pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, NULL);

        pos.left = pos.right;
        pos.right += 30;
        SetWindowPos(pane->content.explorer.btn_split_v, NULL,
            pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top, NULL);

        pos.left = pane->rc.left;
        pos.right = pane->rc.right;
        pos.top = pos.bottom;
        pos.bottom = pane->rc.bottom;
        pane->content.explorer.browser->SetRect(NULL, pos);
    } else if (pane->content_type == PaneType::FolderBrowser) {
        pane->rc = *rc;
        SHRINK_RECT(pane->rc, HALF_FRAME_WIDTH);
        SetWindowPos(pane->content.folder.tree->hwnd, NULL, 
            pane->rc.left, pane->rc.top, pane->rc.right - pane->rc.left, pane->rc.bottom - pane->rc.top, NULL);
    }
}

void ComputeLayout(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    SHRINK_RECT(rc, HALF_FRAME_WIDTH);
    ComputeLayout(&rc, FilePane_GetRootPane());
}

void DrawExplorerFrame(Pane *pane, HDC hdc, HBRUSH brush)
{
    RECT left, top, right, bottom;
    
    left.left = pane->rc.left - FRAME_WIDTH;
    left.top = pane->rc.top - FRAME_WIDTH;
    left.right = pane->rc.left;
    left.bottom = pane->rc.bottom + FRAME_WIDTH;

    top.left = pane->rc.left - FRAME_WIDTH;
    top.top = pane->rc.top - FRAME_WIDTH;
    top.right = pane->rc.right + FRAME_WIDTH;
    top.bottom = pane->rc.top;

    right.left = pane->rc.right;
    right.top = pane->rc.top - FRAME_WIDTH;
    right.right = pane->rc.right + FRAME_WIDTH;
    right.bottom = pane->rc.bottom + FRAME_WIDTH;

    bottom.left = pane->rc.left - FRAME_WIDTH;
    bottom.top = pane->rc.bottom;
    bottom.right = pane->rc.right + FRAME_WIDTH;
    bottom.bottom = pane->rc.bottom + FRAME_WIDTH;

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
    FolderBrowserTree *tree = (FolderBrowserTree*)calloc(1, sizeof(FolderBrowserTree));
    Pane *pane = FilePane_AllocatePane();
    pane->parent_id = parent->id;
    pane->content_type = PaneType::FolderBrowser;
    pane->content.folder.tree = tree;
    parent->content.container.lpane_id = pane->id;
    RECT rc = {};
    FolderBrowser_Create(tree, hwnd, hInstance, &rc);
    FolderBrowser_FillRoot(tree);
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

    // create pane controls
    pane->content.explorer.txt_path = CreateTextBox(hwnd, hInstance);
    pane->content.explorer.btn_split_h = CreateButton(hwnd, hInstance, L"Split Horizontal", pane->id, ButtonFunction::SplitHorizontal);
    pane->content.explorer.tt_split_h = CreateToolTip(g_main_window_hwnd, pane->content.explorer.btn_split_h, L"Split Horizontal");
    pane->content.explorer.btn_split_v = CreateButton(hwnd, hInstance, L"Split Vertical", pane->id, ButtonFunction::SplitVertical);
    pane->content.explorer.tt_split_v = CreateToolTip(g_main_window_hwnd, pane->content.explorer.btn_split_v, L"Split Vertical");

    // browse to folder location
    IShellFolder *pshf;
    SHGetDesktopFolder(&pshf);
    browser->BrowseToObject(pshf, NULL);
    pshf->Release();

    return pane;
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

    InitExplorerBrowserPane(g_main_window_hwnd, g_hinstance, container_pane);
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

int PreDispatch_OnLButtonDown(HWND hwnd, MSG *msg)
{
    POINT pt = GetPoint(hwnd, msg);

    BEGIN_ENUM_CONTAINERS
        if (PtInRect(&pane->content.container.split_handle, pt)) {
            FilePane_SetSplitHandleCursor(pane);
            g_dragging_split_handle = true;
            g_dragged_split_handle_pane_id = pane->id;
            return 0;
            break;
        }
    END_ENUM_CONTAINERS

    if (g_dragging_split_handle == false) {
        Pane *pane = FilePane_GetExplorerPaneByPt(pt);
        if (pane != NULL && !pane->content.explorer.focused) {
            FilePane_SetFocus(pane->id);
            InvalidateRect(hwnd, NULL, FALSE);
        }
    }
    return 1;
}

void PreDispatch_OnLButtonUp(HWND hwnd, MSG *msg)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(msg);

    if (g_dragging_split_handle) g_dragging_split_handle = false;
}

int PreDispatch_OnMouseMove(HWND hwnd, MSG *msg)
{
    POINT pt = GetPoint(hwnd, msg);

    if (g_dragging_split_handle) {
        Pane *pane = FilePane_GetPaneById(g_dragged_split_handle_pane_id);
        RECT rc = pane->rc;
        pt.x = pt.x - rc.left;
        if (pt.x > rc.right) pt.x = rc.right;
        if (pt.x < 0) pt.x = 0;
        pt.y = pt.y - rc.top;
        if (pt.y > rc.bottom) pt.y = rc.bottom;
        if (pt.y < 0) pt.y = 0;

        if (pane->content.container.split_direction == SplitDirection::Horizontal) {
            if (pane->content.container.split_type == SplitType::Fixed) {
                pane->content.container.split = float(pt.y);
            }
            else {
                pane->content.container.split = CLAMP(float(pt.y) / float((rc.bottom - rc.top)), 0.0f, 1.0f);
            }
        }
        else if (pane->content.container.split_direction == SplitDirection::Vertical) {
            if (pane->content.container.split_type == SplitType::Fixed) {
                pane->content.container.split = float(pt.x);
            }
            else {
                pane->content.container.split = CLAMP(float(pt.x) / float((rc.right - rc.left)), 0.0f, 1.0f);
            }
        }
        
        SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0);
        b_block_wm_paint = true;
        ComputeLayout(&rc, pane);
        b_block_wm_paint = false;
        SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0);
        EXPAND_RECT(rc, HALF_FRAME_WIDTH);
        RedrawWindow(hwnd, &rc, NULL, RDW_FRAME | RDW_NOERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);
        UpdateWindow(hwnd);
        return 0;
    }
    else {
        BEGIN_ENUM_CONTAINERS
            if (PtInRect(&pane->content.container.split_handle, pt)) {
                FilePane_SetSplitHandleCursor(pane);
                break;
            }
        END_ENUM_CONTAINERS
    }
    return 1;
}

void PreDispatch_OnXButtonDown(HWND hwnd, MSG *msg)
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

int PreDispatchMessage(HWND hwnd, MSG *msg)
{
    /*
        PreDispatchMessage allows this program to respond to
        messages that are destined for hosted windows
    */
    switch(msg->message)
    {
        case WM_ERASEBKGND:
            if (b_block_wm_paint) return 0;
            break;
        case WM_PAINT:
            if (b_block_wm_paint) return 0;
            break;
        case WM_LBUTTONDOWN:
            return PreDispatch_OnLButtonDown(hwnd, msg);
            break;
        case WM_LBUTTONUP:
            PreDispatch_OnLButtonUp(hwnd, msg);
            break;
        case WM_MOUSEMOVE:
            return PreDispatch_OnMouseMove(hwnd, msg);
            break;
        case WM_XBUTTONDOWN:
            PreDispatch_OnXButtonDown(hwnd, msg);
            break;
    }
    return 1;
}

LRESULT CALLBACK 
WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            ComputeLayout(hwnd);
            break;
        case WM_SETCURSOR:
            if (g_dragging_split_handle) return 1;
            break;
        case WM_NOTIFY: {
            LPNMHDR nmhdr = (LPNMHDR)lParam;

            if (nmhdr->idFrom == IDC_FOLDERBROWSER)
                return FolderBrowser_OnNotify(hwnd, msg, wParam, lParam);
            
            if (nmhdr->idFrom == IDC_BUTTON)
                return Button_OnNotify(hwnd, msg, wParam, lParam);

        } break;
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
            DrawExplorerFrame(FilePane_GetFolderBrowserPane(), hdc, inactive_brush);
            if (active_pane != NULL) {
                DrawExplorerFrame(active_pane, hdc, active_brush);
            }
            DeleteObject(active_brush);
            DeleteObject(inactive_brush);
            EndPaint(hwnd, &ps);
        } break;
        // case WM_COMMAND: {
        //     USHORT ctl_id = LOWORD(wParam);
        //     USHORT code = HIWORD(wParam);
        //     //HWND ctl_handle = (HWND)lParam;
        //     switch(code)
        //     {
        //         case BN_CLICKED: {
        //             DEBUGALERT(L"clicked");
        //             // int pane_id = (int)LOBYTE(ctl_id);
        //             // UNREFERENCED_PARAMETER(pane_id);
        //             // ButtonFunction function = (ButtonFunction)HIBYTE(ctl_id);
        //             // if (function == ButtonFunction::SplitHorizontal) {
        //             //     DEBUGALERT(L"split horizontal");
        //             // }
        //             // else if (function == ButtonFunction::SplitVertical) {
        //             //     DEBUGALERT(L"split vertical");
        //             // }
        //         } break;
        //     }
        // } break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI 
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    HRESULT hr;
    HWND hwnd;
    MSG msg;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    // ensure the dll for common controls is loaded
    InitCommonControls();
   
    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Scale(g_right_arrow_points, ARRAYSIZE(g_right_arrow_points), 10.0f/6.0f);
    Scale(g_vertical_split_points, ARRAYSIZE(g_vertical_split_points), 18.0f/6.0f);
    Scale(g_horizontal_split_points, ARRAYSIZE(g_horizontal_split_points), 18.0f/6.0f);

    hwnd = CreateMainWindow(hInstance);

    // required for all COM calls later
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if FAILED(hr) return 1;

    // enabled drag and drop between explorer windows
    hr = OleInitialize(NULL);
    if FAILED(hr) return 2;

    g_idc_sizens = LoadCursorW(0, IDC_SIZENS);
    g_idc_sizewe = LoadCursorW(0, IDC_SIZEWE);
    g_idc_arrow = LoadCursorW(0, IDC_ARROW);

    Pane *primary_pane = InitContainerPane(hwnd);
    InitFolderBrowserPane(hwnd, hInstance, primary_pane);
    InitExplorerBrowserPane(hwnd, hInstance, primary_pane);

    ComputeLayout(hwnd);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // main message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        if (PreDispatchMessage(hwnd, &msg)) {
            DispatchMessage(&msg);
        }
    }

    // cleanup
    OleUninitialize();
    CoUninitialize();
    GdiplusShutdown(gdiplusToken);


    return 0;
}

