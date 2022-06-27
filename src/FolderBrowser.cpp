
#include "FilePane_Common.h"

void SetExplorerBrowserPath(LPWSTR path, IExplorerBrowser *browser)
{
    LPITEMIDLIST pidl;
    SHParseDisplayName(path, NULL, &pidl, NULL, NULL);
    browser->BrowseToIDList(pidl, SBSP_ABSOLUTE);
    CoTaskMemFree(pidl);
}

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

float GetWidth(PointF *points, int count)
{
    float max = 0;
    for(int i = 0; i < count; i++) {
        if (points[i].X > max) {
            max = points[i].X;
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

LRESULT 
FolderBrowser_OnCustomDraw(FolderBrowserTree *tree, LPNMTVCUSTOMDRAW pnmtvcd)
{
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
            FolderBrowser_OnItemPaint(tree, pnmtvcd);
            return CDRF_SKIPDEFAULT;
            break;

        // tree view item default painting complete, draw addition stuff on top.
        case CDDS_ITEMPOSTPAINT:
            //OnPaintTreeViewItem(pnmtvcd->nmcd.hdc, pnmtvcd->nmcd.rc);
            break;
    }
    return 0;
}

void 
FolderBrowser_OnItemPaint(FolderBrowserTree *tree, LPNMTVCUSTOMDRAW nmtvcd)
{
    RECT rc = nmtvcd->nmcd.rc;
    // not sure what these 0 height items are, but we don't need to draw them.
    if (rc.bottom == 0 && rc.top == 0) return;

    HDC hdc = nmtvcd->nmcd.hdc;
    Graphics g(hdc);
    Font hdcfont(hdc);
    Gdiplus::FontFamily family;
    hdcfont.GetFamily(&family);
    Font font(&family, hdcfont.GetSize(), 0, hdcfont.GetUnit());
    SolidBrush arrow_brush(Color(50,50,50));
    SolidBrush bk_highlight_brush(Color(229, 243, 255));
    SolidBrush bk_selected_brush(Color(205, 232, 255));
    SolidBrush bk_inactive_select_brush(Color(217,217,217));
    SolidBrush bk_white_brush(Color(255, 255, 255));
    SolidBrush text_brush(Color(0,0,0));

    HTREEITEM hitem = (HTREEITEM)nmtvcd->nmcd.dwItemSpec;

    TVITEMW item;
    WCHAR buffer[260] = {};
    item.hItem = hitem;
    item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_PARAM;
    item.pszText = buffer;
    item.cchTextMax = ARRAYSIZE(buffer);
    SendMessage(tree->hwnd, TVM_GETITEMW, 0, LPARAM(&item));

    FolderItemData *data = (FolderItemData*)item.lParam;
    int indent = data->level * 15;
    bool show_arrow = data->items_checked == 0 || data->has_items;

    if (item.state & TVIS_SELECTED) {
        if (tree->focused || (hdcfont.GetStyle() & Gdiplus::FontStyleUnderline)) {
            g.FillRectangle(&bk_selected_brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
        } 
        else {
            g.FillRectangle(&bk_inactive_select_brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
        }
    }
    else if (hdcfont.GetStyle() & Gdiplus::FontStyleUnderline) {
        g.FillRectangle(&bk_highlight_brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    }
    else {
        g.FillRectangle(&bk_white_brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    }  

    PointF local_points[6];
    memcpy(local_points, g_right_arrow_points, sizeof(g_right_arrow_points));
    int h = GetHeight(local_points, ARRAYSIZE(local_points));
    int w = GetWidth(local_points, ARRAYSIZE(local_points));
    indent += 5;
    if (show_arrow)
    {
        Translate(local_points, ARRAYSIZE(local_points), rc.left + indent, rc.top + ((rc.bottom - rc.top) - h)/2);
        g.FillPolygon(&arrow_brush, local_points, ARRAYSIZE(local_points));
    }

    int cx,cy;
    ImageList_GetIconSize(tree->image_list, &cx, &cy);
    indent += w + 10;
    ImageList_Draw(tree->image_list, item.iImage, hdc, rc.left + indent, rc.top + (((rc.bottom - rc.top) - cy)/2), ILD_TRANSPARENT);

    RectF layout_rect(0.0, 0.0, 100.0, 50.0);
    RectF bounding_box;
    g.MeasureString(L"Wg", 2, &font, layout_rect, &bounding_box);
    indent += cx + 2;
    PointF text_point;
    text_point.X = rc.left + indent;
    text_point.Y = rc.top + ((((rc.bottom - rc.top) - (bounding_box.Height)))/2); 
    g.DrawString((LPCWSTR)item.pszText, wcslen((LPCWSTR)item.pszText), &font, text_point, &text_brush);
}

BOOL 
FolderBrowser_HitTest(HWND hwnd, POINTS pts, TVITEMW *item, LPWSTR buffer, int buffer_size)
{
    TVHITTESTINFO hit_test;
    POINTSTOPOINT(hit_test.pt, pts);
    TreeView_HitTest(hwnd, &hit_test);
    if (hit_test.hItem != NULL)
    {
        item->hItem = hit_test.hItem;
        item->mask = TVIF_TEXT | TVIF_STATE | TVIF_CHILDREN;
        item->pszText = buffer;
        item->cchTextMax = buffer_size;
        SendMessageW(hwnd, TVM_GETITEMW, 0, (LPARAM)item);
        return TRUE;
    }
    return FALSE;
}

LRESULT 
FolderBrowser_SubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch(msg)
    {
        case WM_ERASEBKGND: {
            if (g_dragging_split_handle) return 0;
        } break;
        case WM_LBUTTONDBLCLK: {
            // prevent default behavior when double clicking
            // which causes the treeview to try expanding
            return 1; 
        } break;
        case WM_LBUTTONDOWN: {
            POINTS pts = MAKEPOINTS(lParam);
            TVITEMW item;
            WCHAR buffer[260] = {};
            if (FolderBrowser_HitTest(hwnd, pts, &item, buffer, ARRAYSIZE(buffer)))
            {
                Pane *explorer_pane = FilePane_GetActiveExplorerPane();
                FolderItemData *data = (FolderItemData*)item.lParam;
                int chevron_start = data->level * 15;
                int chevron_end = chevron_start + 15;
                if ((pts.x > chevron_start && pts.x < chevron_end) // first check for the arrow position
                    && (data->items_checked == 0 || data->has_items)) // do expand logic we haven't items_checked before OR it has items
                {
                    if (data->expanded)
                    {
                        BOOL ret = TreeView_Expand(hwnd, item.hItem, TVE_COLLAPSE);
                        data->expanded = 0;
                        //Alert(L"ret: %d", ret);
                    }
                    else
                    {
                        FolderBrowserPane *folder_pane = &FilePane_GetFolderBrowserPane()->content.folder;
                        FolderBrowser_FillItem(folder_pane->tree, &item);
                        if (data->has_items)
                        {
                            TreeView_Expand(hwnd, item.hItem, TVE_EXPAND);
                            data->expanded = 1;
                        }
                        else
                        {
                            SetExplorerBrowserPath(data->path, explorer_pane->content.explorer.browser);
                        }
                    }
                }
                else
                {
                    SetExplorerBrowserPath(data->path, explorer_pane->content.explorer.browser);
                }
                // prevent default behavior by returning  here
                //return 1;
            }
            else
            {
                //Alert(L"No Item Found");
            }
        } break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

void 
FolderBrowser_Create(FolderBrowserTree *tree, HWND parent, HINSTANCE hInstance, RECT *rc)
{
    RECT rcClient;

    GetClientRect(parent, &rcClient);
    HWND hwnd_tree = CreateWindowExW(
        NULL // dwExStyle
        , L"SysTreeView32" // lpClassName
        , L"Navigation Tree" //lpWindowName
        ,(WS_VISIBLE | WS_CHILD | TVS_FULLROWSELECT | TVS_DISABLEDRAGDROP | TVS_TRACKSELECT | TVS_NOHSCROLL) //| WS_BORDER | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT// dwStyle
        , rc->left, rc->top // x, y
        , rc->right - rc->left, rc->bottom - rc->top // width, height
        , parent // hwndParent
        , NULL //(HMENU)ID_TREEVIEW // hMenu
        , hInstance // hInstance
        , NULL // lpParam
    );

    SetWindowSubclass(hwnd_tree, FolderBrowser_SubClassProc, FPC_NAVIGATION_TREE, NULL);

    tree->font = (HFONT)SendMessage(tree->hwnd, WM_GETFONT, NULL, NULL);
    
    // Create and Associate the image list with the tree-view control. 
    SHGetImageList(SHIL_SMALL, IID_IImageList, (void **)&tree->image_list);
    TreeView_SetImageList(hwnd_tree, tree->image_list, TVSIL_NORMAL); 

    // set top and bottom padding to 3px by adding 6 to height
    int item_height = TreeView_GetItemHeight(hwnd_tree);
    item_height += 6;
    TreeView_SetItemHeight(hwnd_tree, item_height);
    
    tree->hwnd = hwnd_tree;
}

HTREEITEM 
FolderBrowser_InsertItem(FolderBrowserTree *tree, SHFILEINFOW *item, HTREEITEM parent, HTREEITEM prev, FolderItemData *data)
{
    TVITEMW tvi;
    TVINSERTSTRUCTW tvins;
    HTREEITEM hti;
    ICONINFO iconinfo;

    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM | TVIF_CHILDREN | TVIF_STATE;

    tvi.pszText = item->szDisplayName;
    tvi.cchTextMax = ARRAYSIZE(item->szDisplayName);
    tvi.iImage = item->iIcon;
    tvi.lParam = LPARAM(data);
    tvi.cChildren = 1;
    tvi.state = 0;
    tvins.item = tvi;
    tvins.hParent = parent;
    tvins.hInsertAfter = prev;

    hti = (HTREEITEM)SendMessage(tree->hwnd, TVM_INSERTITEMW, 0, (LPARAM)(LPTVINSERTSTRUCTW)&tvins);

    return hti;
}

void 
FolderBrowser_FillRoot(FolderBrowserTree *tree)
{
    HRESULT hr;
    LPITEMIDLIST folder_pidl, item_pidl, abs_pidl;
    IShellFolder *desktop_shell_folder;
    IShellFolder *drives_shell_folder;
    IEnumIDList *enum_id_list;
    SHFILEINFOW shell_file_info;
    HTREEITEM prev = NULL;

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

        FolderItemData *data = (FolderItemData*)calloc(1, sizeof(FolderItemData));
        data->level = 0;
        SHGetPathFromIDListW(abs_pidl, data->path);
        prev = FolderBrowser_InsertItem(tree, &shell_file_info, TVI_ROOT, prev, data);

        CoTaskMemFree(abs_pidl);
        CoTaskMemFree(item_pidl);
    }

    desktop_shell_folder->Release();
    drives_shell_folder->Release();
    enum_id_list->Release();
    CoTaskMemFree(folder_pidl);
}

void 
FolderBrowser_FillItem(FolderBrowserTree *tree, TVITEMW *parent)
{
    HRESULT hr;
    LPITEMIDLIST folder_pidl, item_pidl, abs_pidl;
    IShellFolder *desktop_shell_folder;
    IShellFolder *drives_shell_folder;
    IEnumIDList *enum_id_list;
    SHFILEINFOW shell_file_info;
    FolderItemData *parent_data = (FolderItemData*)parent->lParam;
    HTREEITEM prev = parent->hItem;
    if (parent_data->items_checked) return;
    parent_data->items_checked = 1;

    hr = SHGetDesktopFolder(&desktop_shell_folder);
    if FAILED(hr) { MessageBoxW(NULL, L"SHGetDesktopFolder failed", L"Error", MB_OK); return; }

    hr = SHParseDisplayName(parent_data->path, NULL, &folder_pidl, NULL, NULL);
    if FAILED(hr) { MessageBoxW(NULL, L"SHParseDisplayName failed", L"Error", MB_OK); return; }

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

        FolderItemData *data = (FolderItemData*)calloc(1, sizeof(FolderItemData));
        data->level = parent_data->level + 1;
        SHGetPathFromIDListW(abs_pidl, data->path);
        prev = FolderBrowser_InsertItem(tree, &shell_file_info, parent->hItem, TVI_LAST, data);
        parent_data->has_items = 1;

        CoTaskMemFree(abs_pidl);
        CoTaskMemFree(item_pidl);
    }

    desktop_shell_folder->Release();
    drives_shell_folder->Release();
    enum_id_list->Release();
    CoTaskMemFree(folder_pidl);
}

