
#include <windows.h>
#include <commctrl.h>
#include <wingdi.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <ShlObj_core.h>
#include <KnownFolders.h>

int g_nOpen, g_nClosed, g_nDocument;

BOOL InitTreeViewImageLists(HWND hwnd) 
{ 
    HIMAGELIST himl;  // handle to image list 
    HBITMAP hbmp;     // handle to bitmap 

    // Create the image list. 
    if ((himl = ImageList_Create(32, 
                                 32,
                                 FALSE, 
                                 3, 0)) == NULL) 
        return FALSE; 


    HMODULE hMod = GetModuleHandleW(L"C:\\Windows\\system32\\shell32.dll");

    // Add the open file, closed file, and document bitmaps. 
    hbmp = LoadBitmap(hMod, MAKEINTRESOURCE(1)); 
    g_nOpen = ImageList_Add(himl, hbmp, (HBITMAP)NULL); 
    DeleteObject(hbmp); 
 
    hbmp = LoadBitmap(hMod, MAKEINTRESOURCE(2)); 
    g_nClosed = ImageList_Add(himl, hbmp, (HBITMAP)NULL); 
    DeleteObject(hbmp); 

    hbmp = LoadBitmap(hMod, MAKEINTRESOURCE(3)); 
    g_nDocument = ImageList_Add(himl, hbmp, (HBITMAP)NULL); 
    DeleteObject(hbmp); 

    // Fail if not all of the images were added. 
    if (ImageList_GetImageCount(himl) < 3) 
        return FALSE; 

    // Associate the image list with the tree-view control. 
    TreeView_SetImageList(hwnd, himl, TVSIL_NORMAL); 

    DeleteObject(hMod);

    return TRUE; 
}

HWND CreateNavigationTree(HWND parent, HINSTANCE hInstance, RECT *rc)
{
    RECT rcClient;

    GetClientRect(parent, &rcClient);
    HWND hwnd = CreateWindowExW(
        NULL // dwExStyle
        , L"SysTreeView32" // lpClassName
        , L"Navigation Tree" //lpWindowName
        ,WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT // dwStyle
        , rc->left, rc->top // x, y
        , rc->right - rc->left, rc->bottom - rc->top // width, height
        , parent // hwndParent
        , NULL //(HMENU)ID_TREEVIEW // hMenu
        , hInstance // hInstance
        , NULL // lpParam
    );

    //InitTreeViewImageLists(parent);
    
    return hwnd;
}

HTREEITEM InsertNavigationItem(HWND hwnd, SHFILEINFOW *item, HTREEITEM parent, int level)
{
    TVITEMW tvi;
    TVINSERTSTRUCTW tvins;
    HTREEITEM hti;

    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

    tvi.pszText = item->szDisplayName;
    tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]);

    tvi.iImage = g_nDocument;
    tvi.iSelectedImage = g_nDocument;

    tvi.lParam = (LPARAM)level;
    tvins.item = tvi;
    tvins.hParent = parent;


    hti = (HTREEITEM)SendMessage(hwnd, TVM_INSERTITEMW, 0, (LPARAM)(LPTVINSERTSTRUCTW)&tvins);

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
        // STRRET strret;
        // LPWSTR lpwstr;
        // hr = drives_shell_folder->GetDisplayNameOf(item_pidl, SHGDN_NORMAL, &strret);
        // if FAILED(hr) { MessageBoxW(NULL, L"drives_shell_folder->GetDisplayNameOf failed", L"Error", MB_OK); return; }
        // hr = StrRetToStrW(&strret, item_pidl, &lpwstr);
        // if FAILED(hr) { MessageBoxW(NULL, L"StrRetToStrW failed", L"Error", MB_OK); return; }
        // MessageBoxW(NULL, lpwstr, L"Display Name", MB_OK);

        abs_pidl = ILCombine(folder_pidl, item_pidl);
        SHGetFileInfoW((LPCWSTR)abs_pidl,
            0 // dwFileAttributes
            ,&shell_file_info
            ,sizeof(SHFILEINFOW)
            ,SHGFI_PIDL|SHGFI_DISPLAYNAME|SHGFI_ATTRIBUTES|SHGFI_SYSICONINDEX);

        InsertNavigationItem(hwnd_tree_view, &shell_file_info, TVI_ROOT, 1);

        //MessageBoxW(NULL, shell_file_info.szDisplayName, L"Succeeded", MB_OK);
        CoTaskMemFree(abs_pidl);
        CoTaskMemFree(item_pidl);
        //CoTaskMemFree(lpwstr);
    }

    desktop_shell_folder->Release();
    drives_shell_folder->Release();
    enum_id_list->Release();
    CoTaskMemFree(folder_pidl);
}