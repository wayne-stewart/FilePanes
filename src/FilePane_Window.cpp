
#include "FilePane_Common.h"

LRESULT CALLBACK
Button_SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

HWND CreateMainWindow(HINSTANCE hInstance)
{
    WNDCLASSW wc;
    wc.style        = CS_HREDRAW | CS_VREDRAW;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = 0;
    wc.lpszClassName    = L"File Panes";
    wc.hInstance        = hInstance;
    wc.hbrBackground    = GetSysColorBrush(COLOR_3DFACE);
    wc.lpszMenuName     = NULL;
    wc.lpfnWndProc      = WndProc;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    //wc.hIcon            = (HICON)LoadImage(hInstance, L"../assets/appicon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    wc.hIcon            = (HICON)LoadImage(hInstance, L"AppIcon", IMAGE_ICON, 0, 0, LR_COPYFROMRESOURCE | LR_DEFAULTSIZE);

    RegisterClassW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, wc.lpszClassName,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, 1024, 768, NULL, NULL, hInstance, NULL);

    g_main_window_hwnd = hwnd;
    g_hinstance = hInstance;
    return hwnd;
}

 void RunMainWindowLoopWhileMessagesExist()
 {
    MSG msg;
    while(PeekMessageW(&msg, NULL, NULL, NULL, TRUE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
 }

LRESULT CALLBACK
Button_OnCustomDraw(LPNMCUSTOMDRAW pnmcd)
{
    HBRUSH brush;
    switch (pnmcd->dwDrawStage)
    {
        case CDDS_PREERASE:
            if ((pnmcd->uItemState & CDIS_FOCUS) > 0) {
                brush = CreateSolidBrush(RGB(205, 232, 255));
            }
            else if ((pnmcd->uItemState & CDIS_HOT) > 0) {
                brush = CreateSolidBrush(RGB(229, 243, 255));
            }
            else {
                brush = CreateSolidBrush(RGB(255,255, 255));
            }
            FillRect(pnmcd->hdc, &pnmcd->rc, brush);
            DeleteObject(brush);

            Graphics g(pnmcd->hdc);
            SolidBrush gbrush(Color(150,200,255));
            Pen pen(&gbrush, 3.0f);
            float fx,fy,fw,fh;
            fx = (float)pnmcd->rc.left;
            fy = (float)pnmcd->rc.top;
            fw = (float)(pnmcd->rc.right - pnmcd->rc.left);
            fh = (float)(pnmcd->rc.bottom - pnmcd->rc.top);

            DWORD_PTR ref_data;
            GetWindowSubclass(pnmcd->hdr.hwndFrom, Button_SubclassProc, IDC_BUTTON, &ref_data);
            ButtonFunction function = (ButtonFunction)HIBYTE(LOWORD(ref_data));

            if (function == ButtonFunction::SplitHorizontal) {
                PointF local_points[ARRAYSIZE(g_horizontal_split_points)];
                memcpy(local_points, g_horizontal_split_points, sizeof(g_horizontal_split_points));
                Center(local_points, ARRAYSIZE(local_points), fx, fy, fw, fh);
                g.FillPolygon(&gbrush, local_points, 8);
                g.FillPolygon(&gbrush, local_points+8, 4);
                g.FillPolygon(&gbrush, local_points+8+4, 8);
            }
            else if (function == ButtonFunction::SplitVertical) {
                PointF local_points[ARRAYSIZE(g_vertical_split_points)];
                memcpy(local_points, g_vertical_split_points, sizeof(g_vertical_split_points));
                Center(local_points, ARRAYSIZE(local_points), fx, fy, fw, fh);
                g.FillPolygon(&gbrush, local_points, 8);
                g.FillPolygon(&gbrush, local_points+8, 4);
                g.FillPolygon(&gbrush, local_points+8+4, 8);
            }
            else if (function == ButtonFunction::Up) {
                PointF local_points[ARRAYSIZE(g_up_points)];
                memcpy(local_points, g_up_points, sizeof(g_up_points));
                DrawPointsAsLinePairsCenteredInBox(&g, &pen, local_points, ARRAYSIZE(local_points), fx, fy, fw, fh);
            }
            else if (function == ButtonFunction::Back) {
                PointF local_points[ARRAYSIZE(g_up_points)];
                memcpy(local_points, g_back_points, sizeof(g_back_points));
                DrawPointsAsLinePairsCenteredInBox(&g, &pen, local_points, ARRAYSIZE(local_points), fx, fy, fw, fh);
            }
            else if (function == ButtonFunction::Refresh) {
                //RECT
                g.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
                g.DrawArc(&pen, 6.0f, 6.0f, 16.0f, 16.0f, -60.0f, 290.0f);
                PointF points[] = { {5.0f, 5.0f}, {13.0f, 13.0f}, {13.0f, 5.0f} };
                g.FillPolygon(&gbrush, points, ARRAYSIZE(points));
            }
            else if (function == ButtonFunction::Remove) {
                PointF local_points[ARRAYSIZE(g_remove_points)];
                memcpy(local_points, g_remove_points, sizeof(g_remove_points));
                DrawPointsAsLinePairsCenteredInBox(&g, &pen, local_points, ARRAYSIZE(local_points), fx, fy, fw, fh);
            }

            return CDRF_SKIPDEFAULT;
    }
    return 0;
}

LRESULT CALLBACK
Button_OnNotify(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR nmhdr = (LPNMHDR)lParam;
    LPNMCUSTOMDRAW pnmcd = (LPNMCUSTOMDRAW)lParam;
    switch (nmhdr->code)
    {
        case NM_CUSTOMDRAW: {
            return Button_OnCustomDraw(pnmcd);
        } break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

 LRESULT CALLBACK
 Button_SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
 {
    UNREFERENCED_PARAMETER(dwRefData);
    UNREFERENCED_PARAMETER(uIdSubclass);

    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch (msg)
    {
        case WM_DESTROY:
            RemoveWindowSubclass(hwnd, Button_SubclassProc, uIdSubclass);
            break;
        case WM_LBUTTONUP:
            UINT8 pane_id = LOBYTE(LOWORD(dwRefData));
            ButtonFunction function = (ButtonFunction)HIBYTE(LOWORD(dwRefData));

            // unfocus the button
            SetFocus(g_main_window_hwnd);

            Pane *pane = FilePane_GetPaneById(pane_id);

            if (function == ButtonFunction::SplitHorizontal) {
                SplitPane(pane, SplitType::Float, SplitDirection::Horizontal, 0.5);
                ComputeLayout(g_main_window_hwnd);
                InvalidateRect(g_main_window_hwnd, NULL, FALSE);
                UpdateWindow(g_main_window_hwnd);
            }
            else if (function == ButtonFunction::SplitVertical) {
                SplitPane(pane, SplitType::Float, SplitDirection::Vertical, 0.5);
                ComputeLayout(g_main_window_hwnd);
                InvalidateRect(g_main_window_hwnd, NULL, FALSE);
                UpdateWindow(g_main_window_hwnd);
            }
            else if (function == ButtonFunction::Back) {
                pane->content.explorer.browser->BrowseToIDList(NULL, SBSP_NAVIGATEBACK);
            }
            else if (function == ButtonFunction::Up) {
                pane->content.explorer.browser->BrowseToIDList(NULL, SBSP_PARENT);
            }
            else if (function == ButtonFunction::Refresh) {
                IShellView *ppv;
                pane->content.explorer.browser->GetCurrentView(IID_IShellView, (void **)&ppv);
                ppv->Refresh();
            }
            else if (function == ButtonFunction::Remove) {
                RemovePane(pane);
                ComputeLayout(g_main_window_hwnd);
                InvalidateRect(g_main_window_hwnd, NULL, FALSE);
                UpdateWindow(g_main_window_hwnd);
            }

            break;
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
 }

 HWND CreateButton(HWND parent, HINSTANCE hInstance, LPCWSTR text, int pane_id, ButtonFunction function)
 {
    WORD low_data = MAKEWORD(UINT8(pane_id), function);
    DWORD_PTR ref_data = (DWORD_PTR)MAKELONG(low_data, 0);

    // create he address text box
    HWND hwnd = CreateWindowExW(
          0  // dwExStyle
        , WC_BUTTON // class name
        , text // window name
        , WS_VISIBLE | WS_CHILD // dwStyle
        , 0, 0, 0, 0 // x y w h
        , parent // parent
        , (HMENU)IDC_BUTTON // (HMENU)id // hmenu
        , hInstance // GetWindowLongPtr(hwnd, GWLP_HINSTANCE)
        , NULL // lpParam
    );
    SetWindowFont(hwnd, GetWindowFont(FilePane_GetFolderBrowserPane()->content.folder.tree->hwnd), NULL);
    SetWindowSubclass(hwnd, Button_SubclassProc, IDC_BUTTON, ref_data);
    return hwnd;
 }

LRESULT CALLBACK
TextBox_SubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    UNREFERENCED_PARAMETER(dwRefData);
    UNREFERENCED_PARAMETER(uIdSubclass);

    switch(msg)
    {
        case WM_CHAR: {
            if (wParam == VK_RETURN) {
                Pane *pane = FilePane_GetPaneById((int)dwRefData);
                WCHAR buffer[1024] = {};
                Edit_GetText(hwnd, buffer, ARRAYSIZE(buffer));
                ExplorerBrowser_SetPath(buffer, pane);
                return 0;
            }
        } break;
        case WM_PASTE: {
            DefSubclassProc(hwnd, msg, wParam, lParam);
            WCHAR buffer[1024] = {};
            Edit_GetLine(hwnd, 0, buffer, ARRAYSIZE(buffer));
            Edit_SetText(hwnd, buffer);
            return 0;
        } break;
        case WM_DESTROY: {
            RemoveWindowSubclass(hwnd, TextBox_SubClassProc, IDC_URI);
        } break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

HWND CreateTextBox(HWND parent, HINSTANCE hInstance, int pane_id)
{
    HWND hwnd = CreateWindowExW(
          0  // dwExStyle
        , WC_EDITW // class name
        , NULL // window name
        , WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL | ES_MULTILINE // dwStyle
        , 0, 0, 0, 0 // x y w h
        , parent // parent
        , NULL // hmenu
        , hInstance // GetWindowLongPtr(hwnd, GWLP_HINSTANCE)
        , NULL // lpParam
    );
    SetWindowFont(hwnd, GetWindowFont(FilePane_GetFolderBrowserPane()->content.folder.tree->hwnd), NULL);
    SetWindowSubclass(hwnd, TextBox_SubClassProc, IDC_URI, (DWORD_PTR)pane_id);
    return hwnd;
}

HWND CreateToolTip(HWND window, HWND control, LPWSTR text)
{
    HWND tip = CreateWindowExW(
        NULL // dwExStyle
        , TOOLTIPS_CLASS // lpClassName
        , NULL // lpWindowName
        , WS_POPUP | TTS_ALWAYSTIP // dwStyle
        , CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT // x, y, w, h
        , window // hwndParent
        , NULL // hMenu
        , g_hinstance
        , NULL // lpParam
    );

    if (tip == NULL) return NULL;

    TOOLINFO info = {};
    info.cbSize = sizeof(info);
    info.hwnd = window;
    info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    info.uId = (UINT_PTR)control;
    info.lpszText = text;
    SendMessage(tip, TTM_ADDTOOL, 0, (LPARAM)&info);

    return tip;
}

