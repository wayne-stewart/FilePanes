
#include <windows.h>
#include <windowsx.h>           // for WM_COMMAND handling macros
#include <shlobj.h>             // shell stuff
#include <shlwapi.h>            // QISearch, easy way to implement QI
#include <propkey.h>
#include <propvarutil.h>
#include <strsafe.h>
#include <objbase.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "propsys")
#pragma comment(lib, "shell32")
#pragma comment(lib, "Gdi32")

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

IExplorerBrowser *_peb1;
IExplorerBrowser *_peb2;
HWND hwnd_tree_view;


#include "MainWindow.cpp"
#include "NavigationTree.cpp"
#include "ExplorerBrowserCOM.cpp"
#include "ShellUtil.cpp"

void ComputeLayout(HWND hwnd, RECT *rc0, RECT *rc1, RECT *rc2)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    UINT s = 5;
    UINT tvw = 200;
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            if (_peb1)
            {
                RECT rc,rc0,rc1,rc2;
                ComputeLayout(hwnd, &rc0, &rc1, &rc2);
                SetWindowPos(hwnd_tree_view, NULL, rc0.left, rc0.top, rc0.right - rc0.left, rc0.bottom - rc0.top, NULL);
                _peb1->SetRect(NULL, rc1);
                _peb2->SetRect(NULL, rc2);
            }

            break;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    HRESULT hr;
    HWND hwnd;
    ExplorerBrowserCOM service;

    hwnd = CreateMainWindow(hInstance);

    // ensure the dll for common controls is loaded
    InitCommonControls();

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if FAILED(hr) return 1;

    hr = OleInitialize(NULL);
    if FAILED(hr) return 2;

    hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_peb1));
    if FAILED(hr) return 3;

    hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_peb2));
    if FAILED(hr) return 3;

    RECT rc,rc0,rc1,rc2;
    ComputeLayout(hwnd, &rc0, &rc1, &rc2);

    FOLDERSETTINGS fs = {};
    fs.ViewMode = FVM_DETAILS;
    fs.fFlags = FWF_NOWEBVIEW;
    //fs.fFlags = FWF_NOWEBVIEW | FWF_NOCOLUMNHEADER;
    //IUnknown_SetSite(_peb, static_cast<IServiceProvider *>(&service));

    IShellFolder *pshf;
    LPITEMIDLIST lpiidl;
    HTREEITEM tree_item;

    hwnd_tree_view = CreateNavigationTree(hwnd, hInstance, &rc0);
    FillNavigationRootItems(hwnd_tree_view);
    // tree_item = InsertNavigationItem(hwndNavigationTree, L"TESTA", TVI_ROOT, 1);
    // InsertNavigationItem(hwndNavigationTree, L"TESTB", tree_item, 2);
    // InsertNavigationItem(hwndNavigationTree, L"TESTC", tree_item, 2);

    _peb1->Initialize(hwnd, &rc1, &fs);
    SHGetDesktopFolder(&pshf);
    _peb1->BrowseToObject(pshf, NULL);
    pshf->Release();
    
    _peb2->Initialize(hwnd, &rc2, &fs);
    //SHGetDesktopFolder(&pshf);
    //SHGetSpecialFolderLocation(hwnd, CSIDL_MYDOCUMENTS, &lpiidl);
    SHGetSpecialFolderLocation(hwnd, CSIDL_DRIVES, &lpiidl);
    // int sz = sizeof(lpiidl)/sizeof(lpiidl[0]);
    // WCHAR buffer[1024];
    // swprintf(buffer, L"item count: %d", sz);
    // MessageBoxW(hwnd, buffer, L"Alert", MB_OK);
    
    _peb2->BrowseToIDList(lpiidl, NULL);
    CoTaskMemFree(lpiidl);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    RunMainWindowLoop();

    _peb1->Release();
    _peb2->Release();

    OleUninitialize();
    CoUninitialize();

    return 0;
}



