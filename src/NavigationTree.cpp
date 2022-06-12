
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
using RectF = Gdiplus::RectF;

typedef struct
{
    HWND hwnd;
    HIMAGELIST image_list;
    HFONT font;
} NavigationTree;

HWND _parent;
UINT _flags[1024];
const int DELETED = 1;
const int SELECTED = 1 << 1;
const int HIGHLIGHTED = 1 << 1;

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

float GetHeight(PointF *points, int count)
{
    float max = 0;
    for(int i = 0; i < count; i++) {
        if (points[i].Y > max) {
            max = points[i].Y;
        }
    }
    return max;
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
    SolidBrush arrow_brush(Color(50,50,50));
    SolidBrush bk_highlight_brush(Color(229, 243, 255));
    SolidBrush bk_selected_brush(Color(205, 232, 255));
    SolidBrush bk_inactive_select_brush(Color(217,217,217));
    SolidBrush text_brush(Color(0,0,0));

    COLORREF color_arrow = RGB(50,50,50);
    COLORREF color_bk_highlight = RGB(229, 243, 255);
    COLORREF color_bk_selecteded = RGB(205, 232, 255);
    COLORREF color_bk_inactive_select = RGB(217,217,217);
    COLORREF color_text = RGB(0,0,0);

    HBRUSH brush_arrow = CreateSolidBrush(color_arrow);
    HBRUSH brush_bk_highlight = CreateSolidBrush(color_bk_highlight);
    HBRUSH brush_bk_selected = CreateSolidBrush(color_bk_selecteded);
    HBRUSH brush_bk_inactive_selected = CreateSolidBrush(color_bk_inactive_select);
    HBRUSH brush_text = CreateSolidBrush(color_text);


    HTREEITEM hitem = (HTREEITEM)nmtvcd->nmcd.dwItemSpec;

    TVITEMW item;
    WCHAR buffer[260] = {};
    item.hItem = hitem;
    item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE;
    item.pszText = buffer;
    item.cchTextMax = ARRAYSIZE(buffer);
    //TreeView_GetItem(tree->hwnd, &item);
    SendMessage(tree->hwnd, TVM_GETITEMW, 0, LPARAM(&item));

    //Alert(L"%d %d", item.state, item.stateMask);

    if (item.state & TVIS_SELECTED) {
        FillRect(hdc, &rc, brush_bk_selected);
        SetBkColor(hdc, color_bk_selecteded);
    }
        //g.FillRectangle(&bk_selected_brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    // else
    //     g.FillRectangle(&bk_highlight_brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

    PointF local_points[6];
    memcpy(local_points, points, sizeof(points));
    int h = GetHeight(local_points, ARRAYSIZE(local_points));
    Translate(local_points, ARRAYSIZE(local_points), rc.left + 5, rc.top + ((rc.bottom - rc.top) - h)/2);
    g.FillPolygon(&arrow_brush, local_points, ARRAYSIZE(local_points));

    int cx,cy;
    ImageList_GetIconSize(tree->image_list, &cx, &cy);
    ImageList_Draw(tree->image_list, item.iImage, hdc, rc.left+18, rc.top + (((rc.bottom - rc.top) - cy)/2), ILD_NORMAL);
    //rc.left += 50;
    //DrawTextW(hdc, item.pszText, wcslen((LPCWSTR)item.pszText), &rc, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    RectF layout_rect(0.0, 0.0, 100.0, 50.0);
    RectF bounding_box;
    g.MeasureString(L"Wg", 2, &font, layout_rect, &bounding_box);
    PointF text_point;
    text_point.X = rc.left + 35;
    text_point.Y = rc.top + ((((rc.bottom - rc.top) - (bounding_box.Height)))/2);
    g.DrawString((LPCWSTR)item.pszText, wcslen((LPCWSTR)item.pszText), &font, text_point, &text_brush);
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
        case WM_LBUTTONDOWN: {
            POINTS pts = MAKEPOINTS(lParam);
            TVHITTESTINFO hit_test;
            POINTSTOPOINT(hit_test.pt, pts);
            TreeView_HitTest(hwnd, &hit_test);
            if (hit_test.hItem != NULL)
            {
                TVITEMW item;
                WCHAR buffer[260] = {};
                item.hItem = hit_test.hItem;
                item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE;
                item.pszText = buffer;
                item.cchTextMax = ARRAYSIZE(buffer);
                TreeView_GetItem(hwnd, &item);
                SendMessageW(hwnd, TVM_GETITEMW, 0, (LPARAM)&item);
                // WCHAR buffer[1024] = {};
                // wsprintfW(buffer, L"%s", item.pszText);
                // SendMessageW(_parent, WM_SETTEXT, 0, LPARAM(item.pszText));
                Alert(L"State: %d, Image: %d", item.state, item.iImage);

                TreeView_SelectItem(hwnd, hit_test.hItem);
                //InvalidateRect(hwnd, NULL, 0);
            }
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
        ,WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_FULLROWSELECT | TVS_DISABLEDRAGDROP // | TVS_TRACKSELECT //| TVS_HASLINES | TVS_LINESATROOT// dwStyle
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

HTREEITEM InsertNavigationItem(NavigationTree *tree, SHFILEINFOW *item, HTREEITEM parent, HTREEITEM prev, int level, int option)
{
    TVITEMW tvi;
    TVINSERTSTRUCTW tvins;
    HTREEITEM hti;
    ICONINFO iconinfo;

    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM | TVIF_CHILDREN | TVIF_STATE;
    //tvi.stateMask = TVIF_S

    tvi.pszText = item->szDisplayName;
    tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]);
    tvi.iImage = item->iIcon;
    tvi.lParam = (LPARAM)level;
    tvi.cChildren = 1;
    tvi.state = option == 1 ? TVIS_SELECTED : 0;
    tvins.item = tvi;
    tvins.hParent = parent;
    tvins.hInsertAfter = prev;

    hti = (HTREEITEM)SendMessage(tree->hwnd, TVM_INSERTITEMW, 0, (LPARAM)(LPTVINSERTSTRUCTW)&tvins);

    return hti;
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

    int option = 0;
    HTREEITEM prev = NULL;
    while (S_OK == enum_id_list->Next(1, &item_pidl, NULL))
    {
        RunMainWindowLoopWhileMessagesExist();

        abs_pidl = ILCombine(folder_pidl, item_pidl);
        SHGetFileInfoW((LPCWSTR)abs_pidl,
            0 // dwFileAttributes
            ,&shell_file_info
            ,sizeof(SHFILEINFOW)
            ,SHGFI_PIDL|SHGFI_DISPLAYNAME|SHGFI_ATTRIBUTES|SHGFI_SYSICONINDEX|SHGFI_ICON); //  | SHGFI_LARGEICON

        if (option == 0) option = 1;
        else if (option == 1) option = 0;
        prev = InsertNavigationItem(tree, &shell_file_info, TVI_ROOT, prev, 0, option);

        CoTaskMemFree(abs_pidl);
        CoTaskMemFree(item_pidl);
    }

    desktop_shell_folder->Release();
    drives_shell_folder->Release();
    enum_id_list->Release();
    CoTaskMemFree(folder_pidl);
}