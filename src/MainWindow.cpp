
#include "FilePane_Common.h"

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

 HWND CreateButton(HWND parent, HINSTANCE hInstance, LPCWSTR text, int pane_id, ButtonFunction function)
 {
    WORD id = MAKEWORD(UINT8(pane_id), function);

    // create he address text box
    HWND hwnd = CreateWindowExW(
          0  // dwExStyle
        , WC_BUTTON // class name
        , text // window name
        , WS_VISIBLE | WS_CHILD // dwStyle
        , 0, 0, 0, 0 // x y w h
        , parent // parent
        , (HMENU)id // hmenu
        , hInstance // GetWindowLongPtr(hwnd, GWLP_HINSTANCE)
        , NULL // lpParam
    );
    SetWindowFont(hwnd, GetWindowFont(FilePane_GetFolderBrowserPane()->content.folder.tree->hwnd), NULL);
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