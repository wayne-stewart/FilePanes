
#include "FilePane_Common.h"

#include "FilePane_Window.cpp"
#include "FilePane_FolderTree.cpp"
#include "FilePane_Explorer.cpp"
#include "FilePane_Drawing.cpp"
#include "FilePane_Panes.cpp"

inline void Position(HWND hwnd, RECT *rc)
{
    SetWindowPos(hwnd, NULL, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, NULL);
}

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
        
        int button_dim = 30;

        RECT pos, offset;
        pos.left = pane->rc.left;
        pos.top = pane->rc.top;

        pos.right = pos.left + button_dim;
        pos.bottom = pane->rc.top + button_dim;
        Position(pane->content.explorer.btn_back, &pos);

        pos.left = pos.right;
        pos.right += button_dim;
        Position(pane->content.explorer.btn_up, &pos);

        pos.left = pos.right;
        pos.right = pane->rc.right - (4*button_dim);
        Position(pane->content.explorer.txt_path, &pos);
        offset = pos;
        OffsetRect(&offset, -pos.left + 5, -pos.top + 5);
        Edit_SetRect(pane->content.explorer.txt_path, &offset);

        pos.left = pos.right;
        pos.right += button_dim;
        Position(pane->content.explorer.btn_refresh, &pos);

        pos.left = pos.right;
        pos.right += button_dim;
        Position(pane->content.explorer.btn_split_h, &pos);

        pos.left = pos.right;
        pos.right += button_dim;
        Position(pane->content.explorer.btn_split_v, &pos);

        pos.left = pos.right;
        pos.right += button_dim;
        Position(pane->content.explorer.btn_remove, &pos);

        pos.left = pane->rc.left;
        pos.right = pane->rc.right;
        pos.top = pos.bottom;
        pos.bottom = pane->rc.bottom;
        pane->content.explorer.browser->SetRect(NULL, pos);
    } else if (pane->content_type == PaneType::FolderBrowser) {
        pane->rc = *rc;
        SHRINK_RECT(pane->rc, HALF_FRAME_WIDTH);
        Position(pane->content.folder.tree->hwnd, &pane->rc);
    }
}

void ComputeLayout(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    SHRINK_RECT(rc, HALF_FRAME_WIDTH);
    ComputeLayout(&rc, FilePane_GetRootPane());
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
    }
}

int PreDispatch_OnKeyPress(HWND hwnd, MSG *msg, bool down)
{
    UNREFERENCED_PARAMETER(hwnd);
    static bool control_down = false;

    switch(msg->wParam)
    {
    case VK_DELETE: 
        if (down) ExplorerBrowser_HandleDeleteKeyPress(); break;
    case VK_CONTROL: 
        control_down = down; break;
    case 'A': 
        if (control_down && down) ExplorerBrowser_HandleControlAKeyPress(); break;
    case 'C': 
        if (control_down && down) ExplorerBrowser_HandleControlCXKeyPress(DROPEFFECT_COPY); break;
    case 'X':
        if (control_down && down) ExplorerBrowser_HandleControlCXKeyPress(DROPEFFECT_MOVE); break;
    case 'V': 
        if (control_down && down) ExplorerBrowser_HandleControlVKeyPress(); break;
    }
    return 1;
}



int PreDispatchMessage(HWND hwnd, MSG *msg)
{
    /*
        PreDispatchMessage allows this program to respond to
        messages that are destined for hosted windows
    */
   int result = 1;
    switch(msg->message)
    {
        case WM_ERASEBKGND:
            if (b_block_wm_paint) result = 0;
            break;
        case WM_PAINT:
            if (b_block_wm_paint) result = 0;
            break;
        case WM_LBUTTONDOWN:
            result = PreDispatch_OnLButtonDown(hwnd, msg);
            break;
        case WM_LBUTTONUP:
            PreDispatch_OnLButtonUp(hwnd, msg);
            break;
        case WM_MOUSEMOVE:
            result = PreDispatch_OnMouseMove(hwnd, msg);
            break;
        case WM_XBUTTONDOWN:
            PreDispatch_OnXButtonDown(hwnd, msg);
            break;
        case WM_KEYDOWN:
            result = PreDispatch_OnKeyPress(hwnd, msg, true);
            break;
        case WM_KEYUP:
            result = PreDispatch_OnKeyPress(hwnd, msg, false);
            break;
    }
    return result;
}

LRESULT CALLBACK 
WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY:
            FilePane_SaveState();
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

    g_quick_access_ico = (HICON)LoadImage(hInstance, L"QuickAccessIcon", IMAGE_ICON, 0, 0, NULL);
    g_back_ico = (HICON)LoadImage(hInstance, L"BackIcon", IMAGE_ICON, 0, 0, NULL);
    g_up_ico = (HICON)LoadImage(hInstance, L"UpIcon", IMAGE_ICON, 0, 0, NULL);
    g_split_horizontal_ico = (HICON)LoadImage(hInstance, L"SplitHorizontalIcon", IMAGE_ICON, 0, 0, NULL);
    g_split_vertical_ico = (HICON)LoadImage(hInstance, L"SplitVerticalIcon", IMAGE_ICON, 0, 0, NULL);
    g_close_ico = (HICON)LoadImage(hInstance, L"CloseIcon", IMAGE_ICON, 0, 0, NULL);
    g_refresh_ico = (HICON)LoadImage(hInstance, L"RefreshIcon", IMAGE_ICON, 0, 0, NULL);
    g_chevron_right_ico = (HICON)LoadImage(hInstance, L"ChevronRightIcon", IMAGE_ICON, 0, 0, NULL);
    g_chevron_down_ico = (HICON)LoadImage(hInstance, L"ChevronDownIcon", IMAGE_ICON, 0, 0, NULL);

    // required for all COM calls later
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if FAILED(hr) return 1;

    // enabled drag and drop between explorer windows
    hr = OleInitialize(NULL);
    if FAILED(hr) return 2;

    g_idc_sizens = LoadCursorW(0, IDC_SIZENS);
    g_idc_sizewe = LoadCursorW(0, IDC_SIZEWE);
    g_idc_arrow = LoadCursorW(0, IDC_ARROW);

    CF_PREFFEREDDROPEFFECT = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);

    hwnd = CreateMainWindow(hInstance);
    FilePane_LoadState();
    //FilePane_LoadDefaultState();

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

    DestroyIcon(g_quick_access_ico);
    DestroyIcon(g_back_ico);
    DestroyIcon(g_up_ico);
    DestroyIcon(g_split_horizontal_ico);
    DestroyIcon(g_split_vertical_ico);
    DestroyIcon(g_close_ico);
    DestroyIcon(g_refresh_ico);

    return 0;
}

