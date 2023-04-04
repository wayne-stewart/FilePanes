
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
#include <shellapi.h>           // shell constants
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

#define DEBUGALERT(...) {\
    wchar_t dbg_buf[512]; wsprintf(dbg_buf, __VA_ARGS__); \
    MessageBoxW(NULL, dbg_buf, L"Debug Alert", MB_OK|MB_ICONINFORMATION); }
#define DEBUGWRITE(msg_format, ...) {\
    wchar_t dbg_buf[512]; wsprintf(dbg_buf, msg_format L"\r\n", __VA_ARGS__); \
    OutputDebugStringW(dbg_buf); }

template <typename T>
struct AutoRelease
{
    T *instance;
    AutoRelease() { instance = nullptr; }
    AutoRelease(const AutoRelease&) = delete;
    ~AutoRelease() { if (instance) { instance->Release(); } }
    operator LPVOID*() const { return (LPVOID*)&instance; }
    operator LPDATAOBJECT*() const { return (LPDATAOBJECT*)&instance; }
    operator T*() const { return instance; }
    operator IUnknown*() const { return instance; }
    T *operator->() const { return instance; }
    T **operator&() { return &instance; }
};

template<typename T>
struct AutoCoMemFree
{
    T *instance;
    AutoCoMemFree() { instance = nullptr; }
    AutoCoMemFree(const AutoCoMemFree&) = delete;
    ~AutoCoMemFree() { if (instance) { CoTaskMemFree(instance); } }
    operator LPVOID*() const { return (LPVOID*)&instance; }
    operator LPITEMIDLIST*() const { return (LPITEMIDLIST*)&instance; }
    operator LPCITEMIDLIST*() const { return (LPCITEMIDLIST*)&instance; }
    operator T*() const { return instance; }
    T *operator->() const { return instance; }
};

template<typename T>
struct AutoLocalFree
{
    T *instance;
    AutoLocalFree() { instance = nullptr; }
    AutoLocalFree(HLOCAL instance) { this->instance = (T*)instance; }
    AutoLocalFree(const AutoLocalFree&) = delete;
    ~AutoLocalFree() { if (instance) { LocalFree(instance); } }
    operator T*() const { return instance; }
};

// struct defer_dummy {};
// template <class F> struct deferrer { F f; ~deferrer() { DEBUGWRITE(L"deferred function called"); f(); } };
// template <class F> deferrer<F> operator*(defer_dummy, F f) { return {f}; }
// #define DEFER_(LINE) zz_defer##LINE
// #define DEFER(LINE) DEFER_(LINE)
// #define defer auto DEFER(__LINE__) = defer_dummy{} *[&]()

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

class ExplorerBrowserEvents : public IServiceProvider, public IExplorerBrowserEvents, public ICommDlgBrowser2, public IExplorerPaneVisibility
{
public:
    virtual ~ExplorerBrowserEvents(){}

    void SetPaneId(int id);
    bool HasFocus();

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

    // ICommDlgBrowser
    HRESULT IncludeObject(IShellView *shell_view, PCUITEMID_CHILD pidl);
    HRESULT OnDefaultCommand(IShellView *shell_view);
    HRESULT OnStateChange(IShellView *shell_view, ULONG uChange);

    // ICommDlgBrowser2
    HRESULT Notify(IShellView *shell_view, DWORD dwNotifyType);
    HRESULT GetDefaultMenuText(IShellView *shell_view, LPWSTR pszText, int cchMax);
    HRESULT GetViewFlags(DWORD *pdwFlags);

    // IExplorerPaneVisibility
    IFACEMETHODIMP GetPaneState(REFEXPLORERPANE ep, EXPLORERPANESTATE *peps);

private:
    long _cRef;
    int _pane_id;
    bool _hasfocus;
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

// FilePane_Drawing
void DrawPointsAsLinePairsCenteredInBox(Graphics *g, Pen *pen, PointF *points, int count, float rcx, float rcy, float rcw, float rch);
float GetHeight(PointF *points, int count);
float GetWidth(PointF *points, int count);
void Scale(PointF *points, int count, float scale);
void Translate(PointF *points, int count, float x, float y);
void Center(PointF *points, int count, float rcx, float rcy, float rcw, float rch);
void matmul(PointF *point, mat2x2 *mat);
void Rotate90(PointF *points, int count);
void DrawExplorerFrame(Pane *pane, HDC hdc, HBRUSH brush);

// FilePane_Panes
inline void FilePane_SetSplitHandleCursor(Pane *pane);
Pane* FilePane_GetFolderBrowserPane();
Pane* FilePane_GetPaneById(int id);
Pane* FilePane_GetRootPane();
Pane* FilePane_AllocatePane();
void FilePane_DeallocatePane(Pane *pane);
Pane* FilePane_GetExplorerPaneById(int id);
Pane* FilePane_GetActiveExplorerPane();
void FilePane_SetFocus(int id);
Pane* FilePane_GetExplorerPaneByPt(POINT pt);
Pane* InitContainerPane(HWND hwnd);
void InitFolderBrowserPane(Pane *parent);
Pane* InitExplorerBrowserPane(Pane *parent);
void SplitPane(Pane *explorer_pane, SplitType split_type, SplitDirection split_direction, float split_value);
void ReplaceParentWithChildOnGrandParent(Pane *grand_parent, Pane *parent, Pane *child);
void RemovePane(Pane *pane);
void InitExplorerBrowserPaneUI(Pane *pane, LPCWSTR path);
void InitFolderBrowserPaneUI(Pane *pane);

/// GLOBAL STATE
HWND g_main_window_hwnd;
HINSTANCE g_hinstance;
CLIPFORMAT CF_PREFFEREDDROPEFFECT;

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

#define CHECK_EXPLORER_BROWSER(pane) \
    if (pane == NULL || pane->content_type != PaneType::ExplorerBrowser) return

#define CHECK_EXPLORER_BROWSER_HAS_FOCUS(pane) \
    CHECK_EXPLORER_BROWSER(pane); \
    if (!pane->content.explorer.events->HasFocus()) return

#define CHECK_S_OK(expr) if ((expr) != S_OK) return
#define CHECK_NOT_NULL(expr) if ((expr) != NULL) return

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

struct StringBuilder 
{
    WCHAR *buffer;
    int capacity;
    int length;
};

void StringBuilder_Init(StringBuilder *sb)
{
    sb->buffer = (WCHAR*)calloc(2048, 2);
    sb->capacity = 2048;
    sb->length = 0;
}

void StringBuilder_Append(StringBuilder *sb, LPCWSTR format, ...) 
{
    va_list args;
    va_start(args, format);
    WCHAR format_buffer[2048] = {};
    int formatted_length = vswprintf(format_buffer, ARRAYSIZE(format_buffer), format, args);
    va_end(args);

    // couldn't write the formatted string
    if (formatted_length < 0) return;

    // resize buffer if needed
    if ((sb->length + formatted_length + 1) > sb->capacity) {
        int a = sb->length + formatted_length;
        int b = a * 2; // increase buffer by length + what's needed * 2;
        int c = b % 2048;
        int d = b + c; // malloc 2k char at once, means 4k pages.
        WCHAR *nbuf = (WCHAR*)calloc((size_t)d, 2);
        memcpy((void*)nbuf, (void*)sb->buffer, (size_t)(sb->length * 2));
        free(sb->buffer);
        sb->buffer = nbuf;
    }

    memcpy((void*)(sb->buffer+sb->length), (void*)format_buffer, (size_t)(formatted_length * 2));

    sb->length += formatted_length;
    sb->buffer[sb->length] = '\0';
}

void StringBuilder_Destroy(StringBuilder *sb)
{
    free(sb->buffer);
}

void FilePane_SaveState()
{
    WCHAR path_buffer[MAX_PATH] = {};
    WINDOWINFO info = {};
    PWSTR app_data_path;
    HANDLE hfile;
    DWORD bytes_written = 0;
    int w, h;

    GetWindowInfo(g_main_window_hwnd, &info);
    w = info.rcWindow.right - info.rcWindow.left;
    h = info.rcWindow.bottom - info.rcWindow.top;

    StringBuilder sb;
    StringBuilder_Init(&sb);

    StringBuilder_Append(&sb, L"Window\n%d\n%d\n", w, h);

    for(int i = 0; i < MAX_PANES; i++)
    {
        Pane *pane = &g_panes[i];
        if (pane->content_type == PaneType::NotSet) continue;

        StringBuilder_Append(&sb, L"\nPane\n");
        StringBuilder_Append(&sb, L"%d\n", pane->id);
        StringBuilder_Append(&sb, L"%d\n", pane->parent_id);
        StringBuilder_Append(&sb, L"%d\n", pane->content_type);
        StringBuilder_Append(&sb, L"%d\n", pane->rc.left);
        StringBuilder_Append(&sb, L"%d\n", pane->rc.top);
        StringBuilder_Append(&sb, L"%d\n", pane->rc.right);
        StringBuilder_Append(&sb, L"%d\n", pane->rc.bottom);
        if (pane->content_type == PaneType::Container) {
            StringBuilder_Append(&sb, L"%d\n", pane->content.container.split_type);
            StringBuilder_Append(&sb, L"%d\n", pane->content.container.split_direction);
            StringBuilder_Append(&sb, L"%f\n", pane->content.container.split);
            StringBuilder_Append(&sb, L"%d\n", pane->content.container.lpane_id);
            StringBuilder_Append(&sb, L"%d\n", pane->content.container.rpane_id);
        }
        else if (pane->content_type == PaneType::ExplorerBrowser) {
            Edit_GetText(pane->content.explorer.txt_path, (LPWSTR)&path_buffer, ARRAYSIZE(path_buffer));
            StringBuilder_Append(&sb, L"%ls\n", path_buffer);
        }
    }

    if (S_OK != SHGetKnownFolderPath(FOLDERID_LocalAppData, NULL, NULL, &app_data_path)) goto CLEANUP;

    PathCombineW(path_buffer, app_data_path, L"FilePanes");
    CreateDirectoryW(path_buffer, NULL);

    PathCombineW(path_buffer, app_data_path, L"FilePanes\\config.txt");
    CoTaskMemFree(app_data_path);

    hfile = CreateFileW(path_buffer, GENERIC_READ|GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile != INVALID_HANDLE_VALUE)
    {
        WriteFile(hfile, sb.buffer, (DWORD)sb.length*2, &bytes_written, NULL);
        CloseHandle(hfile);
    }

    CLEANUP:
    StringBuilder_Destroy(&sb);
}

void FilePane_LoadDefaultState()
{
    Pane *primary_pane = InitContainerPane(g_main_window_hwnd);
    InitFolderBrowserPane(primary_pane);
    InitExplorerBrowserPane(primary_pane);
}

int ReadInt(WCHAR **context)
{
    WCHAR *token = wcstok_s(NULL, L"\n",  context);
    int x = wcstol(token, NULL, 10);
    return x;
}

float ReadFloat(WCHAR **context)
{
    WCHAR *token = wcstok_s(NULL, L"\n",  context);
    float x = wcstof(token, NULL);
    return x;
}

void FilePane_LoadState()
{
    WCHAR path_buffer[MAX_PATH] = {};
    PWSTR app_data_path;
    int load_default_state = 1;
    DWORD bytes_read;
    int w, h, id;
    WCHAR *text;

    if (S_OK == SHGetKnownFolderPath(FOLDERID_LocalAppData, NULL, NULL, &app_data_path)) 
    {
        PathCombineW(path_buffer, app_data_path, L"FilePanes\\config.txt");
        CoTaskMemFree(app_data_path);

        HANDLE hfile = CreateFileW(path_buffer, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile != INVALID_HANDLE_VALUE)
        {
            DWORD file_size = GetFileSize(hfile, NULL);
            if (file_size != INVALID_FILE_SIZE)
            {
                WCHAR *file_contents = (WCHAR*)malloc(file_size);
                if (file_contents)
                {
                    if(ReadFile(hfile, file_contents, file_size, &bytes_read, NULL))
                    {
                        WCHAR *context;
                        WCHAR *token = wcstok_s(file_contents, L"\n", &context);
                        while(token != NULL) 
                        {
                            if (wcscmp(L"Window", token) == 0) {
                                w = ReadInt(&context);
                                h = ReadInt(&context);
                                SetWindowPos(g_main_window_hwnd, NULL, CW_USEDEFAULT, CW_USEDEFAULT, w, h, SWP_SHOWWINDOW | SWP_NOMOVE);
                            }
                            else if (wcscmp(L"Pane", token) == 0) {
                                id = ReadInt(&context);
                                Pane *pane = FilePane_GetPaneById(id);
                                pane->id  = id;
                                pane->parent_id = ReadInt(&context);
                                pane->content_type = (PaneType)ReadInt(&context);
                                pane->rc.left = ReadInt(&context);
                                pane->rc.top = ReadInt(&context);
                                pane->rc.right = ReadInt(&context);
                                pane->rc.bottom = ReadInt(&context);
                                if (pane->content_type == PaneType::Container) {
                                    pane->content.container.split_type = (SplitType)ReadInt(&context);
                                    pane->content.container.split_direction = (SplitDirection)ReadInt(&context);
                                    pane->content.container.split = ReadFloat(&context);
                                    pane->content.container.lpane_id = ReadInt(&context);
                                    pane->content.container.rpane_id = ReadInt(&context);
                                }
                                else if (pane->content_type == PaneType::ExplorerBrowser) {
                                    text = wcstok_s(NULL, L"\n", &context);
                                    InitExplorerBrowserPaneUI(pane, text);
                                }
                                else if (pane->content_type == PaneType::FolderBrowser) {
                                    InitFolderBrowserPaneUI(pane);
                                }
                            }
                            token = wcstok_s(NULL, L"\n", &context);
                        }
                        load_default_state = 0;
                    }
                    free(file_contents);
                }
            }
            CloseHandle(hfile);
        }
    }

    if (load_default_state)
    {
        FilePane_LoadDefaultState();
    }

    ComputeLayout(g_main_window_hwnd);
}

/*
https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/winui/shell/appplatform/fileoperations/FileOperationSample.cpp
*/
HRESULT CreateAndInitializeFileOperation(REFIID riid, void **ppv)
{
    *ppv = NULL;
    IFileOperation *file_operation;
    HRESULT hr = CoCreateInstance(__uuidof(FileOperation), NULL, CLSCTX_ALL, IID_PPV_ARGS(&file_operation));
    if (SUCCEEDED(hr))
    {
        hr = file_operation->SetOperationFlags(FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR);//  FOF_NO_UI);
        if (SUCCEEDED(hr))
        {
            hr = file_operation->QueryInterface(riid, ppv);
        }
        file_operation->Release();
    }
    return hr;
}

#endif

