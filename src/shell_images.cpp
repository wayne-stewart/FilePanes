// this program should read all the images from SHGetImageList and
// display them in a simple win32 gui app

#define UNICODE 1

#pragma warning(push)
#pragma warning(disable:4668 4458)

#include <windows.h>
#include <commctrl.h>
#include <commoncontrols.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_core.h>
#include <shellapi.h>
#include <stdio.h>

#pragma warning(pop)

// 4820 is an informative warning about how much padding is added to a struct
// for memory alignment. this is fine.
#pragma warning(disable:4820)

// 4711 is an informative warning about functions selected for automatic
// inline expansion. this is fine.
#pragma warning(disable:4711)

// 5045 is an informative warning that the compiler will insert Spectre mitigations
// when /Qspecter switch is applied to the cl command. this is fine.
#pragma warning(disable:5045)

#pragma comment(lib, "user32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "propsys")
#pragma comment(lib, "shell32")
#pragma comment(lib, "Gdi32")
#pragma comment(lib, "Gdiplus")

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HWND g_main_window_hwnd;
HINSTANCE g_hinstance;
IImageList *g_image_list;

LRESULT CALLBACK 
WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                // display images in a grid
                int image_count = 0;
                g_image_list->GetImageCount(&image_count);
                //int rows = 10;
                int cols = 20;
                int image_width = 0;
                int image_height = 0;
                g_image_list->GetIconSize(&image_width, &image_height);
                int margin = 10;
                int x = margin;
                int y = margin;
                for (int i = 0; i < image_count; i++) {
                    HICON icon;
                    g_image_list->GetIcon(i, ILD_TRANSPARENT, &icon);
                    DrawIconEx(hdc, x, y, icon, image_width, image_height, 0, NULL, DI_NORMAL);
                    x += image_width + margin;
                    if (x >= cols * (image_width + margin)) {
                        x = margin;
                        y += image_height + margin;
                    }
                }

                y += image_height + margin;
                x = margin;

                for (int i = 0; i < SIID_MAX_ICONS; i++) {
                    SHSTOCKICONINFO icon;
                    icon.cbSize = sizeof(SHSTOCKICONINFO);
                    if (SUCCEEDED(SHGetStockIconInfo((SHSTOCKICONID)i, SHGSI_ICON | SHGFI_SMALLICON, &icon)))
                    {
                        DrawIconEx(hdc, x, y, icon.hIcon, image_width, image_height, 0, NULL, DI_NORMAL);
                        x += image_width + margin;
                        if (x >= cols * (image_width + margin)) {
                            x = margin;
                            y += image_height + margin;
                        }
                        wchar_t debug_buffer[1024] = {0};
                        DestroyIcon(icon.hIcon);
                        wsprintf(debug_buffer, L"printing icon %d - (%d, %d) (%d, %d)\r\n", i, x, y, image_width, image_height);
                        OutputDebugStringW(debug_buffer);
                    }
                    else
                    {
                        DWORD dw = GetLastError();
                        wchar_t debug_buffer[4096] = {0};
                        LPWSTR msg_buf;
                        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, dw, 
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msg_buf, 0, NULL);
                        wsprintf(debug_buffer, L"failed getting icon %d - error %d - %s\r\n", i, dw, msg_buf);
                        OutputDebugStringW(debug_buffer);
                        LocalFree(msg_buf);
                    }
                }

                EndPaint(hwnd, &ps);
            }
            break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

HWND CreateMainWindow(HINSTANCE hInstance)
{
    WNDCLASSW wc;
    wc.style        = CS_HREDRAW | CS_VREDRAW;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = 0;
    wc.lpszClassName    = L"Shell Images";
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

int WINAPI 
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    HWND hwnd;
    MSG msg;

    InitCommonControls();

    hwnd = CreateMainWindow(hInstance);

    SHGetImageList(SHIL_SMALL, IID_PPV_ARGS(&g_image_list));

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // main message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}