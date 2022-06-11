
#include <windows.h>
#include <commctrl.h>
#include <wingdi.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <ShlObj_core.h>
#include <KnownFolders.h>
#include <commoncontrols.h>
#include <Gdiplus.h>
//#include <gdiplusinit.h>

#include "MainWindow.h"

using Graphics = Gdiplus::Graphics;
using SolidBrush = Gdiplus::SolidBrush;
using Color = Gdiplus::Color;
using PointF = Gdiplus::PointF;
using Font = Gdiplus::Font;

typedef struct
{
    HWND hwnd;
    HIMAGELIST image_list;
    HFONT font;
} NavigationTree;
HWND _parent;

void NavigationTree_OnItemPaint(NavigationTree *tree, LPNMTVCUSTOMDRAW nmtvcd);
LRESULT NavigationTree_OnNotify(NavigationTree *tree, LPNMHDR nmhdr, WPARAM wParam);

PointF points[6] = {
     {1,0}
    ,{4,3}
    ,{1,6}
    ,{0,5}
    ,{2,3}
    ,{0,1}
};

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



LRESULT NavigationTree_OnNotify(NavigationTree *tree, LPNMHDR nmhdr, WPARAM wParam)
{
    switch (nmhdr->code)
    {
        case NM_CLICK:
            //Alert(L"%d", nmhdr)
            break;
        case NM_CUSTOMDRAW:
            LPNMTVCUSTOMDRAW pnmtvcd = (LPNMTVCUSTOMDRAW)nmhdr;
            switch (pnmtvcd->nmcd.dwDrawStage)
            {
                // start of the paint cycle
                case CDDS_PREPAINT:
                    // tell the tree view control I want to custom paint items.
                    return CDRF_NOTIFYITEMDRAW;
                    break;
                
                // before tree view items are painted to customize default painting
                // return CDRF_NOTIFYPOSTPAINT to get the message when default painting is done.
                case CDDS_ITEMPREPAINT:
                    //pnmtvcd->clrTextBk = 0x0000FF00;
                    //OnPaintTreeViewItem(pnmtvcd->nmcd.hdc, pnmtvcd->nmcd.rc);
                    //return CDRF_DODEFAULT;
                    //return CDRF_NOTIFYPOSTPAINT;
                    NavigationTree_OnItemPaint(tree, pnmtvcd);
                    return CDRF_SKIPDEFAULT;
                    break;

                // tree view item default painting complete, draw addition stuff on top.
                case CDDS_ITEMPOSTPAINT:
                    //OnPaintTreeViewItem(pnmtvcd->nmcd.hdc, pnmtvcd->nmcd.rc);
                    break;
            }
            break;
    }
}

void NavigationTree_OnItemPaint(NavigationTree *tree, LPNMTVCUSTOMDRAW nmtvcd)
{
    RECT rc = nmtvcd->nmcd.rc;
    // not sure what these 0 height items are, but we don't need to draw them.
    if (rc.bottom == 0 && rc.top == 0) return;

    HDC hdc = nmtvcd->nmcd.hdc;
    Graphics g(hdc);
    Font font(hdc, tree->font);
    //Gdiplus::Pen pen(Gdiplus::Color(255,0,0,255));
    SolidBrush arrow_brush(Color(255,255,0,0));
    SolidBrush bk_highlight_brush(Color(229, 243, 255));
    SolidBrush bk_selected_brush(Color(205, 232, 255));
    SolidBrush bk_inactive_select_brush(Color(217,217,217));
    PointF text_point;
    text_point.X = rc.left + 35;
    text_point.Y = rc.top;

    HTREEITEM hitem = (HTREEITEM)nmtvcd->nmcd.dwItemSpec;

    TVITEMW item;
    WCHAR buffer[260] = {};
    item.hItem = hitem;
    item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE;
    item.pszText = buffer;
    item.cchTextMax = ARRAYSIZE(buffer);
    TreeView_GetItem(tree->hwnd, &item);
    SendMessage(tree->hwnd, TVM_GETITEMW, 0, LPARAM(&item));

    HTREEITEM hit_test = (HTREEITEM)SendMessage(tree->hwnd, TVM_HITTEST, 0, 0);

    //Alert(L"%d %d", item.state, item.stateMask);

    PointF local_points[6];
    memcpy(local_points, points, sizeof(points));
    Translate(local_points, ARRAYSIZE(local_points), rc.left, rc.top);

    if (item.state & TVIS_SELECTED)
        g.FillRectangle(&bk_selected_brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    else
        g.FillRectangle(&bk_highlight_brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

    g.FillPolygon(&arrow_brush, local_points, ARRAYSIZE(local_points));
    g.DrawString((LPCWSTR)item.pszText, wcslen((LPCWSTR)item.pszText), &font, text_point, &arrow_brush);
}


LRESULT NavigationTree_SubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch(msg)
    {
        // case WM_NOTIFY: {
        //     LPNMHDR nmhdr = (LPNMHDR)lParam;
        //     if (nmhdr->hwndFrom == nav_tree.hwnd)
        //     {
        //         return NavigationTree_OnNotify(&nav_tree, nmhdr, wParam);
        //     }
        // } break;
        case WM_MOUSEMOVE: {
            POINTS pts = MAKEPOINTS(lParam);
            WCHAR buffer[1024] = {};
            wsprintfW(buffer, L"%d, %d", pts.x, pts.y);
            SendMessageW(_parent, WM_SETTEXT, 0, LPARAM(buffer));
        } break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}


void CreateNavigationTree(NavigationTree *tree, HWND parent, HINSTANCE hInstance, RECT *rc)
{
    _parent = parent;
    RECT rcClient;

    GetClientRect(parent, &rcClient);
    HWND hwnd_tree = CreateWindowExW(
        NULL // dwExStyle
        , L"SysTreeView32" // lpClassName
        , L"Navigation Tree" //lpWindowName
        ,WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_FULLROWSELECT // | TVS_TRACKSELECT //| TVS_HASLINES | TVS_LINESATROOT// dwStyle
        , rc->left, rc->top // x, y
        , rc->right - rc->left, rc->bottom - rc->top // width, height
        , parent // hwndParent
        , NULL //(HMENU)ID_TREEVIEW // hMenu
        , hInstance // hInstance
        , NULL // lpParam
    );

    SetWindowSubclass(hwnd_tree, NavigationTree_SubClassProc, 1, NULL);

    tree->font = (HFONT)SendMessage(tree->hwnd, WM_GETFONT, NULL, NULL);
    

    // Create and Associate the image list with the tree-view control. 
    SHGetImageList(SHIL_SMALL, IID_IImageList, (void **)&tree->image_list);
    TreeView_SetImageList(hwnd_tree, tree->image_list, TVSIL_NORMAL); 
    //TreeView_SetImageList(hwnd_tree, himagelist, TVSIL_STATE);

    // indent by 10 pixels
    //TreeView_SetIndent(hwnd_tree, 20);

    //Alert(L"Indent Width: %d", TreeView_GetIndent(hwnd_tree));

    // set top and bottom padding to 3px by adding 6 to height
    int item_height = TreeView_GetItemHeight(hwnd_tree);
    item_height += 6;
    TreeView_SetItemHeight(hwnd_tree, item_height);

    //TreeView_setstat
    //TreeView_SetLineColor(hwnd_tree, 0x00FFFFFF);
    
    tree->hwnd = hwnd_tree;
}

HTREEITEM InsertNavigationItem(NavigationTree *tree, SHFILEINFOW *item, HTREEITEM parent, int level)
{
    TVITEMW tvi;
    TVINSERTSTRUCTW tvins;
    HTREEITEM hti;
    ICONINFO iconinfo;

    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_CHILDREN;

    tvi.pszText = item->szDisplayName;
    tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]);
    tvi.iImage = item->iIcon;
    tvi.iSelectedImage = tvi.iImage;
    tvi.lParam = (LPARAM)level;
    tvi.cChildren = 1;
    //tvi.state = 0;
    tvins.item = tvi;
    tvins.hParent = parent;

    hti = (HTREEITEM)SendMessage(tree->hwnd, TVM_INSERTITEMW, 0, (LPARAM)(LPTVINSERTSTRUCTW)&tvins);

    return hti;
}

bool Test()
{
    IKnownFolderManager *manager;
    HRESULT hr;
    UINT count;
    KNOWNFOLDERID *kfid;
    IKnownFolder *known_folder;
    IShellItem *shell_item;

    hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&manager));
    if FAILED(hr) return false;

    hr = manager->GetFolder(FOLDERID_Downloads, &known_folder);
    if FAILED(hr) goto CLEANUP;

    hr = known_folder->GetShellItem(NULL, IID_PPV_ARGS(&shell_item));
    if SUCCEEDED(hr) {

        known_folder->Release();
    }

    CLEANUP:
    manager->Release();

    return false;
}

void FillNavigationRootItems(NavigationTree *tree)
{
    HRESULT hr;
    LPITEMIDLIST folder_pidl, item_pidl, abs_pidl;
    IShellFolder *desktop_shell_folder;
    IShellFolder *drives_shell_folder;
    IEnumIDList *enum_id_list;
    SHFILEINFOW shell_file_info;

    hr = SHGetDesktopFolder(&desktop_shell_folder);
    if FAILED(hr) { MessageBoxW(NULL, L"SHGetDesktopFolder failed", L"Error", MB_OK); return; }

    hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &folder_pidl);

    desktop_shell_folder->BindToObject(folder_pidl, NULL, IID_IShellFolder, (void **)&drives_shell_folder);

    hr = drives_shell_folder->EnumObjects(NULL, SHCONTF_FOLDERS, &enum_id_list);
    if FAILED(hr) { MessageBoxW(NULL, L"desktop_shell_folder->EnumObjects failed", L"Error", MB_OK); return; }

    while (S_OK == enum_id_list->Next(1, &item_pidl, NULL))
    {
        RunMainWindowLoopWhileMessagesExist();

        abs_pidl = ILCombine(folder_pidl, item_pidl);
        SHGetFileInfoW((LPCWSTR)abs_pidl,
            0 // dwFileAttributes
            ,&shell_file_info
            ,sizeof(SHFILEINFOW)
            ,SHGFI_PIDL|SHGFI_DISPLAYNAME|SHGFI_ATTRIBUTES|SHGFI_SYSICONINDEX|SHGFI_ICON); //  | SHGFI_LARGEICON

        InsertNavigationItem(tree, &shell_file_info, TVI_ROOT, 1);

        CoTaskMemFree(abs_pidl);
        CoTaskMemFree(item_pidl);
    }

    desktop_shell_folder->Release();
    drives_shell_folder->Release();
    enum_id_list->Release();
    CoTaskMemFree(folder_pidl);
}