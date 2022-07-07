
#ifndef __GLOBALSTATE__H__
#define __GLOBALSTATE__H__

#define UNICODE 1

// 4668 warns when replacing macro definitions with 0.
// 4458 warns when one definition hides another.
// I don't need to be warned about what is in Microsoft's header files.
#pragma warning(push)
#pragma warning(disable:4668 4458)

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

using Graphics = Gdiplus::Graphics;
using SolidBrush = Gdiplus::SolidBrush;
using Pen = Gdiplus::Pen;
using Color = Gdiplus::Color;
using PointF = Gdiplus::PointF;
using Font = Gdiplus::Font;
using RectF = Gdiplus::RectF;
using Gdiplus::GdiplusShutdown;
using Gdiplus::GdiplusStartup;

#define IDC_FOLDERBROWSER 1
#define IDC_URI 2
#define IDC_BUTTON 3

#define FRAME_WIDTH 6
#define HALF_FRAME_WIDTH 3

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

enum ControlType : UINT8 {
    Button = 1,
    Text = 2
};

enum ButtonFunction : UINT8 {
    SplitHorizontal = 1,
    SplitVertical = 2,
    Back = 3,
    Up = 4,
    Refresh = 5,
    Remove = 6
};

struct FolderBrowserTree
{
    HWND hwnd;
    HIMAGELIST image_list;
    HFONT font;
    bool focused;
};

struct FolderItemData {
    WCHAR path[MAX_PATH];
    int level;
    bool items_checked;
    bool has_items;
    bool expanded;
};

struct ButtonID {
    ControlType type;
    UINT8 pane_id;
    ButtonFunction function;
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

    // fix compiler warning 5204 about not having a virtual deconstructor
    // in case this class is subclassed.
    virtual ~ExplorerBrowserEvents() { }
private:
    long _cRef;
    int _pane_id;
};

struct ContainerPane {
    SplitDirection split_direction;
    SplitType split_type;
    float split;
    int rpane_id;
    int lpane_id;
    RECT split_handle;
};

struct ExplorerBrowserPane {
    IExplorerBrowser *browser;
    ExplorerBrowserEvents *events;
    DWORD event_cookie;
    bool focused;

    HWND txt_path;
    HWND btn_split_h;
    HWND tt_split_h;
    HWND btn_split_v;
    HWND tt_split_v;
    HWND btn_back;
    HWND tt_back;
    HWND btn_up;
    HWND tt_up;
    HWND btn_refresh;
    HWND tt_refresh;
    HWND btn_remove;
    HWND tt_remove;
};

struct FolderBrowserPane {
    FolderBrowserTree *tree;
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

struct mat2x2 {
    float col1[2];
    float col2[2];
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND CreateMainWindow(HINSTANCE hInstance);
HWND CreateToolTip(HWND window, HWND control, LPWSTR text);
void RunMainWindowLoop();
void RunMainWindowLoopWhileMessagesExist();
void FolderBrowser_OnItemPaint(FolderBrowserTree *tree, LPNMTVCUSTOMDRAW nmtvcd);
void FolderBrowser_FillItem(FolderBrowserTree *tree, TVITEMW *parent);
void ExplorerBrowser_SetPath(LPCWSTR path, Pane *pane);
void SplitPane(Pane *explorer_pane, SplitType split_type, SplitDirection split_direction, float split_value);
void RemovePane(Pane *pane);
void ComputeLayout(HWND hwnd);

/// GLOBAL STATE
HWND g_main_window_hwnd;
HINSTANCE g_hinstance;

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

PointF g_vertical_split_points[8+4+8] = {
    // left bracket
     {0,0}
    ,{2,0}
    ,{2,1}
    ,{1,1}
    ,{1,5}
    ,{2,5}
    ,{2,6}
    ,{0,6}

    // center line
    ,{2.5,0}
    ,{3.3f,0}
    ,{3.3f,6}
    ,{2.5,6}

    // right bracket
    ,{4,0}
    ,{6,0}
    ,{6,6}
    ,{4,6}
    ,{4,5}
    ,{5,5}
    ,{5,1}
    ,{4,1}
};

PointF g_horizontal_split_points[8+4+8] = {
    // top bracket
     {0,0}
    ,{6,0}
    ,{6,2}
    ,{5,2}
    ,{5,1}
    ,{1,1}
    ,{1,2}
    ,{0,2}

    // center line
    ,{0,2.5}
    ,{6,2.5}
    ,{6,3.3f}
    ,{0,3.3f}

    // bottom bracket
    ,{0,4}
    ,{1,4}
    ,{1,5}
    ,{5,5}
    ,{5,4}
    ,{6,4}
    ,{6,6}
    ,{0,6}
};

PointF g_back_points[] = {
     {0,2}
    ,{6,2}
    ,{0,2}
    ,{2.5,0}
    ,{0,2}
    ,{2.5,4}
};

PointF g_up_points[] = {
     {2,0}
    ,{2,6}
    ,{2,0}
    ,{0,2.5}
    ,{2,0}
    ,{4,2.5}
};

PointF g_remove_points[] = {
     {0,0}
    ,{5,5}
    ,{5,0}
    ,{0,5}
};

HCURSOR g_idc_sizewe;
HCURSOR g_idc_sizens;
HCURSOR g_idc_arrow;

bool g_dragging_split_handle = false;
int g_dragged_split_handle_pane_id = 0;
bool b_block_wm_paint = false;

// UTILITY FUNCTIONS

void Alert(UINT type, LPCWSTR caption, LPCWSTR format, ...)
{
    va_list args;
    va_start(args, format);
    WCHAR buffer[1024] = {};
    vswprintf(buffer, ARRAYSIZE(buffer), format, args);
    va_end(args);
    MessageBoxW(NULL, buffer, caption, MB_OK | type);
}

#define DEBUGALERT(msg_format, ...) Alert(MB_ICONINFORMATION, L"Debug Alert", msg_format, __VA_ARGS__)

#define ASSERT(test, msg_format, ...) if(!(test)) { Alert(MB_ICONERROR, L"Assert Error", msg_format, __VA_ARGS__); }
#define CLAMP(v, min, max) ((v) > (max) ? (max) : ((v) < (min) ? (min) : (v)))

#define BEGIN_ENUM_EXPLORERS for(int i = 0; i < MAX_PANES; i++) { \
    Pane *pane = &g_panes[i]; \
    if (pane->content_type == PaneType::ExplorerBrowser) {
#define END_ENUM_EXPLORERS }}

#define BEGIN_ENUM_CONTAINERS for(int i = 0; i < MAX_PANES; i++) { \
    Pane *pane = &g_panes[i]; \
    if (pane->content_type == PaneType::Container) {
#define END_ENUM_CONTAINERS }}

#define EXPAND_RECT(rc, v) { \
    (rc).left = (rc).left - (v); \
    (rc).right = (rc).right + (v); \
    (rc).top = (rc).top - (v); \
    (rc).bottom = (rc).bottom + (v); }

#define SHRINK_RECT(rc, v) { \
    (rc).left = (rc).left + (v); \
    (rc).right = (rc).right - (v); \
    (rc).top = (rc).top + (v); \
    (rc).bottom = (rc).bottom - (v); }

inline
void FilePane_SetSplitHandleCursor(Pane *pane)
{
    if (pane->content.container.split_direction == SplitDirection::Horizontal) {
        SetCursor(g_idc_sizens);
    }
    else if (pane->content.container.split_direction == SplitDirection::Vertical) {
        SetCursor(g_idc_sizewe);
    }
}

Pane* FilePane_GetFolderBrowserPane() {
    for(int i = 0; i < MAX_PANES; i++) {
        if (g_panes[i].content_type == PaneType::FolderBrowser) {
            return &g_panes[i];
        }
    }
    return NULL;
}

Pane* FilePane_GetPaneById(int id)
{
    Pane *pane = &g_panes[id];
    return pane;
}

Pane* FilePane_GetRootPane()
{
    Pane *pane = FilePane_GetPaneById(1);
    return pane;
}

Pane* FilePane_AllocatePane()
{
    for(int i = 1; i < MAX_PANES; i++) {
        Pane *pane = &g_panes[i];
        if (pane->content_type == PaneType::NotSet) {
            memset(pane, 0, sizeof(Pane));
            pane->id = i;
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
        pane->content.explorer.browser->Destroy();
        DestroyWindow(pane->content.explorer.txt_path);
        DestroyWindow(pane->content.explorer.btn_split_h);
        DestroyWindow(pane->content.explorer.tt_split_h);
        DestroyWindow(pane->content.explorer.btn_split_v);
        DestroyWindow(pane->content.explorer.tt_split_v);
        DestroyWindow(pane->content.explorer.btn_back);
        DestroyWindow(pane->content.explorer.tt_back);
        DestroyWindow(pane->content.explorer.btn_up);
        DestroyWindow(pane->content.explorer.tt_up);
        DestroyWindow(pane->content.explorer.btn_refresh);
        DestroyWindow(pane->content.explorer.tt_refresh);
        DestroyWindow(pane->content.explorer.btn_remove);
        DestroyWindow(pane->content.explorer.tt_remove);
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

Pane* FilePane_GetExplorerPaneById(int id)
{
    Pane *pane = FilePane_GetPaneById(id);
    if (pane == NULL) return NULL;
    if (pane->content_type == PaneType::ExplorerBrowser) return pane;
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

float GetHeight(PointF *points, int count)
{
    float min = 1e7f;
    float max = 0;
    for(int i = 0; i < count; i++) {
        if (points[i].Y > max) {
            max = points[i].Y;
        }
        if (points[i].Y < min) {
            min =  points[i].Y;
        }
    }
    return max - min;
}

float GetWidth(PointF *points, int count)
{
    float min = 1e7f;
    float max = 0;
    for(int i = 0; i < count; i++) {
        if (points[i].X > max) {
            max = points[i].X;
        }
        if (points[i].X < min) {
            min = points[i].X;
        }
    }
    return max - min;
}

void Scale(PointF *points, int count, float scale)
{
    for(int i = 0; i < count; i++)
    {
        points[i].X *= scale;
        points[i].Y *= scale;
    }
}

void Translate(PointF *points, int count, float x, float y)
{
    for(int i = 0; i < count; i++)
    {
        points[i].X += x;
        points[i].Y += y;
    }
}

void Center(PointF *points, int count, float rcx, float rcy, float rcw, float rch)
{
    float w = GetWidth(points, count);
    float h = GetHeight(points, count);
    Translate(points, count, rcx + (rcw - w)/2.0f, rcy + (rch - h)/2.0f);
}

void matmul(PointF *point, mat2x2 *mat)
{
    float x = point->X;
    float y = point->Y;
    point->X = x*mat->col1[0] + y*mat->col1[1];
    point->Y = x*mat->col2[0] + y*mat->col2[1];
}

void Rotate90(PointF *points, int count)
{
    mat2x2 mat = { 0.0f, 1.0f, -1.0f, 0.0f };
    for (int i = 0; i < count; i++) {
        matmul(&points[i], &mat);
    }
}

#endif
