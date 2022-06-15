
#ifndef __GLOBALSTATE__H__
#define __GLOBALSTATE__H__

#include <windows.h>
#include <windowsx.h>           // for WM_COMMAND handling macros
#include <shlobj.h>             // shell stuff
#include <shlwapi.h>            // QISearch, easy way to implement QI
#include <propkey.h>
#include <propvarutil.h>
#include <strsafe.h>
#include <objbase.h>
#include <ShlObj_core.h>
#include <KnownFolders.h>
#include <commoncontrols.h>
#include <Gdiplus.h>
#include <commctrl.h>
#include <wingdi.h>

#pragma comment(lib, "user32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "comctl32")
#pragma comment(lib, "propsys")
#pragma comment(lib, "shell32")
#pragma comment(lib, "Gdi32")
#pragma comment(lib, "Gdiplus")

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using Graphics = Gdiplus::Graphics;
using SolidBrush = Gdiplus::SolidBrush;
using Color = Gdiplus::Color;
using PointF = Gdiplus::PointF;
using Font = Gdiplus::Font;
using RectF = Gdiplus::RectF;

struct NavigationTree
{
    HWND hwnd;
    HIMAGELIST image_list;
    HFONT font;
};

struct NavigationItemData {
    WCHAR path[MAX_PATH];
    int level;
    bool items_checked;
    bool has_items;
    bool expanded;
};

enum SplitDirection {
    Vertical = 0,
    Horizontal = 1
};

// fixed:   the split value in Pane represents the number of pixels the first
//          item in the pane is set to. the second item in the pane fills the
//          rest of the space.
// float:   the split value in Pane represents the percentage from 0 to 1 that
//          divides the first and second panes in the container
//
//          these values are only used when the content_type = Container
enum SplitType {
    Fixed = 0,
    Float = 1
};

enum PaneType {
    Container = 0,
    FolderBrowser = 1,
    ExplorerBrowser = 2
};

struct ContainerPane {
    SplitDirection split_direction;
    SplitType split_type;
    float split;
    RECT rc;
    int rpane_id;
    int lpane_id;
};

struct ExplorerBrowserPane {
    IExplorerBrowser *browser;
};

struct FolderBrowserPane {
    NavigationTree *tree;
};

union _content {
    ContainerPane container;
    ExplorerBrowserPane explorer;
    FolderBrowserPane folder;
};

struct Pane {
    int id;
    int parent_id;
    PaneType content_type;
    _content content;
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND CreateMainWindow(HINSTANCE hInstance);
void RunMainWindowLoop();
void RunMainWindowLoopWhileMessagesExist();
void NavigationTree_OnItemPaint(NavigationTree *tree, LPNMTVCUSTOMDRAW nmtvcd);
void NavigationTree_FillItem(NavigationTree *tree, TVITEMW *parent);

/// GLOBAL STATE

int g_next_id = 0;
int NextId() { return ++g_next_id; }

#define MAX_PANES 50
int g_panes_count = 0;
Pane g_panes[MAX_PANES] = {};

PointF g_right_arrow_points[6] = {
     {1,0}
    ,{4,3}
    ,{1,6}
    ,{0,5}
    ,{2,3}
    ,{0,1}
};

// UTILITY FUNCTIONS

void Alert(LPCWSTR format, ...)
{
    va_list args;
    va_start(args, format);
    WCHAR buffer[1024] = {};
    vswprintf(buffer, format, args);
    va_end(args);
    MessageBoxW(NULL, buffer, L"Alert", MB_OK | MB_ICONEXCLAMATION);
}

FolderBrowserPane* FilePane_GetFolderBrowserPane() {
    for(int i = 0; i < g_panes_count; i++) {
        if (g_panes[i].content_type == PaneType::FolderBrowser) {
            return &g_panes[i].content.folder;
        }
    }
    return NULL;
}

ExplorerBrowserPane* FilePane_GetExplorerPaneById(int id)
{
    for (int i = 0; i < g_panes_count; i++) {
        if (g_panes[i].content_type == PaneType::ExplorerBrowser) {
            return &g_panes[i].content.explorer;
        }
    }
    return NULL;
}

ExplorerBrowserPane* FilePane_GetActiveExplorerPane()
{
    return FilePane_GetExplorerPaneById(3);
}

#endif
