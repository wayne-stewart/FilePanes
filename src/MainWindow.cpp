
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
            SolidBrush gbrush(Color(200,200,200));
            PointF local_points[8+4+8];
            float w,h,fx,fy,fw,fh;
            fx = (float)pnmcd->rc.left;
            fy = (float)pnmcd->rc.top;
            fw = (float)(pnmcd->rc.right - pnmcd->rc.left);
            fh = (float)(pnmcd->rc.bottom - pnmcd->rc.top);

            DWORD_PTR ref_data;
            GetWindowSubclass(pnmcd->hdr.hwndFrom, Button_SubclassProc, IDC_BUTTON, &ref_data);
            ButtonFunction function = (ButtonFunction)HIBYTE(LOWORD(ref_data));

            if (function == ButtonFunction::SplitHorizontal) {
                memcpy(local_points, g_horizontal_split_points, sizeof(g_horizontal_split_points));
                w = GetWidth(local_points, ARRAYSIZE(local_points));
                h = GetHeight(local_points, ARRAYSIZE(local_points));
                Translate(local_points, ARRAYSIZE(local_points), fx + (fw - w)/2.0f, fy + (fh - h)/2.0f);
                g.FillPolygon(&gbrush, local_points, 8);
                g.FillPolygon(&gbrush, local_points+8, 4);
                g.FillPolygon(&gbrush, local_points+8+4, 8);
            }
            else if (function == ButtonFunction::SplitVertical) {
                memcpy(local_points, g_vertical_split_points, sizeof(g_vertical_split_points));
                w = GetWidth(local_points, ARRAYSIZE(local_points));
                h = GetHeight(local_points, ARRAYSIZE(local_points));
                Translate(local_points, ARRAYSIZE(local_points), fx + (fw - w)/2.0f, fy + (fh - h)/2.0f);
                g.FillPolygon(&gbrush, local_points, 8);
                g.FillPolygon(&gbrush, local_points+8, 4);
                g.FillPolygon(&gbrush, local_points+8+4, 8);
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
            }
            else if (function == ButtonFunction::SplitVertical) {
                SplitPane(pane, SplitType::Float, SplitDirection::Vertical, 0.5);
            }
            ComputeLayout(g_main_window_hwnd);
            UpdateWindow(g_main_window_hwnd);
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

 HWND CreateTextBox(HWND parent, HINSTANCE hInstance)
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
    SetWindowSubclass(hwnd, TextBox_SubClassProc, IDC_URI, NULL);
    return hwnd;
 }