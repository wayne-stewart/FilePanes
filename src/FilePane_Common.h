
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
    bool focused;
};

struct NavigationItemData {
    WCHAR path[MAX_PATH];
    int level;
    bool items_checked;
    bool has_items;
    bool expanded;
};

class ExplorerBrowserEvents : public IServiceProvider, public IExplorerBrowserEvents //, public ICommDlgBrowser, public ICommDlgBrowser2
{
public:
    void SetPaneId(int id);

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IServiceProvider
    IFACEMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

    // IExplorerBrowserEvents
    IFACEMETHODIMP OnViewCreated(IShellView * /* psv */);
    IFACEMETHODIMP OnNavigationPending(PCIDLIST_ABSOLUTE pidlFolder);
    IFACEMETHODIMP OnNavigationComplete(PCIDLIST_ABSOLUTE pidlFolder);
    IFACEMETHODIMP OnNavigationFailed(PCIDLIST_ABSOLUTE /* pidlFolder */);
private:
    long _cRef;
    int _pane_id;
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
    NotSet = 0,
    Container = 1,
    FolderBrowser = 2,
    ExplorerBrowser = 3
};

struct ContainerPane {
    SplitDirection split_direction;
    SplitType split_type;
    float split;
    int rpane_id;
    int lpane_id;
};

struct ExplorerBrowserPane {
    IExplorerBrowser *browser;
    ExplorerBrowserEvents *events;
    DWORD event_cookie;
    bool focused;
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
    RECT rc;
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

void Alert(DWORD mb_type, LPCWSTR caption, LPCWSTR format, ...)
{
    va_list args;
    va_start(args, format);
    WCHAR buffer[1024] = {};
    vswprintf(buffer, format, args);
    va_end(args);
    MessageBoxW(NULL, buffer, caption, MB_OK | mb_type);
}

#define ASSERT(test, msg_format, ...) if(!(test)) { Alert(MB_ICONERROR, L"Assert Error", msg_format, __VA_ARGS__); }

#define BEGIN_ENUM_EXPLORERS for(int i = 0; i < MAX_PANES; i++) { \
    Pane *pane = &g_panes[i]; \
    if (pane->content_type == PaneType::ExplorerBrowser) {
#define END_ENUM_EXPLORERS }}

FolderBrowserPane* FilePane_GetFolderBrowserPane() {
    for(int i = 0; i < g_panes_count; i++) {
        if (g_panes[i].content_type == PaneType::FolderBrowser) {
            return &g_panes[i].content.folder;
        }
    }
    return NULL;
}

Pane* FilePane_GetPaneById(int id)
{
    for (int i = 0; i < MAX_PANES; i++) {
        Pane *pane = &g_panes[i];
        if (pane->content_type != PaneType::NotSet && pane->id == id) {
            return pane;
        }
    }
    return NULL;
}

Pane* FilePane_AllocatePane()
{
    for(int i = 0; i < MAX_PANES; i++) {
        Pane *pane = &g_panes[i];
        if (pane->content_type == PaneType::NotSet) {
            memset(pane, 0, sizeof(Pane));
            pane->id = NextId();
            g_panes_count++;
            return pane;
        }
    }
    return NULL;
}

void FilePane_DeallocatePane(Pane *pane)
{
    if (pane == NULL) return;
    if (pane->content_type == PaneType::ExplorerBrowser) {
        pane->content.explorer.browser->Release();
    }
    else if(pane->content_type == PaneType::FolderBrowser) {
        CloseHandle(pane->content.folder.tree->hwnd);
        DeleteObject(pane->content.folder.tree->font);
        ImageList_Destroy(pane->content.folder.tree->image_list);
    }
    else if (pane->content_type == PaneType::Container) {
        FilePane_DeallocatePane(FilePane_GetPaneById(pane->content.container.lpane_id));
        FilePane_DeallocatePane(FilePane_GetPaneById(pane->content.container.rpane_id));
    }
    g_panes_count--;
    memset(pane, 0, sizeof(Pane));
}

ExplorerBrowserPane* FilePane_GetExplorerPaneById(int id)
{
    Pane *pane = FilePane_GetPaneById(id);
    if (pane == NULL) return NULL;
    if (pane->content_type == PaneType::ExplorerBrowser) return &pane->content.explorer;
    return NULL;
}

Pane* FilePane_GetActiveExplorerPane()
{
    // first try to get the currently focused pane
    BEGIN_ENUM_EXPLORERS
    if (pane->content.explorer.focused) return pane;
    END_ENUM_EXPLORERS

    // if there was none, get the first explorer pane we find
    BEGIN_ENUM_EXPLORERS
    return pane;
    END_ENUM_EXPLORERS

    return NULL;
}

void FilePane_SetFocus(int id)
{
    BEGIN_ENUM_EXPLORERS
    if (pane->id == id) {
        pane->content.explorer.focused = true;
    }
    else {
        pane->content.explorer.focused = false;
    }
    END_ENUM_EXPLORERS
}

Pane* FilePane_GetExplorerPaneByPt(POINT pt)
{
    BEGIN_ENUM_EXPLORERS
    if (PtInRect(&pane->rc, pt)) return pane;
    END_ENUM_EXPLORERS
    return NULL;
}

#endif
