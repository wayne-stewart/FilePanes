
#include <windows.h>
#include <commctrl.h>
#include <wingdi.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <ShlObj_core.h>
#include <KnownFolders.h>
#include <commoncontrols.h>

#include "MainWindow.h"

int g_nOpen, g_nClosed, g_nDocument;
HIMAGELIST himagelist;

HWND CreateNavigationTree(HWND parent, HINSTANCE hInstance, RECT *rc)
{
    RECT rcClient;

    GetClientRect(parent, &rcClient);
    HWND hwnd_tree = CreateWindowExW(
        NULL // dwExStyle
        , L"SysTreeView32" // lpClassName
        , L"Navigation Tree" //lpWindowName
        ,WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASBUTTONS | TVS_FULLROWSELECT //| TVS_HASLINES | TVS_LINESATROOT// dwStyle
        , rc->left, rc->top // x, y
        , rc->right - rc->left, rc->bottom - rc->top // width, height
        , parent // hwndParent
        , NULL //(HMENU)ID_TREEVIEW // hMenu
        , hInstance // hInstance
        , NULL // lpParam
    );

    // Create and Associate the image list with the tree-view control. 
    SHGetImageList(SHIL_SMALL, IID_IImageList, (void **)&himagelist);
    TreeView_SetImageList(hwnd_tree, himagelist, TVSIL_NORMAL); 
    //TreeView_SetImageList(hwnd_tree, himagelist, TVSIL_STATE);

    // indent by 10 pixels
    TreeView_SetIndent(hwnd_tree, 10);

    // set top and bottom padding to 3px by adding 6 to height
    int item_height = TreeView_GetItemHeight(hwnd_tree);
    item_height += 6;
    TreeView_SetItemHeight(hwnd_tree, item_height);

    //TreeView_setstat
    //TreeView_SetLineColor(hwnd_tree, 0x00FFFFFF);
    
    return hwnd_tree;
}

HTREEITEM InsertNavigationItem(HWND hwnd_tree, SHFILEINFOW *item, HTREEITEM parent, int level)
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

    hti = (HTREEITEM)SendMessage(hwnd_tree, TVM_INSERTITEMW, 0, (LPARAM)(LPTVINSERTSTRUCTW)&tvins);

    //SendDlgItemMessageW()


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

void FillNavigationRootItems(HWND hwnd_tree_view)
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
            ,SHGFI_PIDL|SHGFI_DISPLAYNAME|SHGFI_ATTRIBUTES|SHGFI_SYSICONINDEX); // SHGFI_ICON | SHGFI_LARGEICON

        InsertNavigationItem(hwnd_tree_view, &shell_file_info, TVI_ROOT, 1);

        CoTaskMemFree(abs_pidl);
        CoTaskMemFree(item_pidl);
    }

    desktop_shell_folder->Release();
    drives_shell_folder->Release();
    enum_id_list->Release();
    CoTaskMemFree(folder_pidl);
}