// #include <windows.h>
// #include <Shlwapi.h>
// #include <ShlObj.h>


// bool GetItemIdListFromPath(LPCWSTR path, LPITEMIDLIST *items)
// {
//     LPSHELLFOLDER shell_folder;
//     HRESULT hr;
//     ULONG eaten;

//     hr = SHGetDesktopFolder(&shell_folder);
//     if (FAILED(hr)) return false;

//     hr = shell_folder->ParseDisplayName(NULL, NULL, (LPWSTR)path, &eaten, items, NULL);

//     shell_folder->Release();

//     if (FAILED(hr))
//     {
//         *items = NULL;
//         return false;
//     }

//     return true;
// }

// // bool GetItemIdListForDesktop(LPITEMIDLIST *items)
// // {
// //     Shget
// // }

// bool ShellUtil_Test()
// {
//     IKnownFolderManager *manager;
//     HRESULT hr;
//     UINT count;
//     KNOWNFOLDERID *kfid;
//     IKnownFolder *known_folder;
//     IShellItem *shell_item;

//     hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&manager));
//     if FAILED(hr) return false;

//     hr = manager->GetFolder(FOLDERID_Downloads, &known_folder);
//     if FAILED(hr) goto CLEANUP;

//     hr = known_folder->GetShellItem(NULL, IID_PPV_ARGS(&shell_item));
//     if SUCCEEDED(hr) {

//         known_folder->Release();
//     }

//     CLEANUP:
//     manager->Release();

//     return false;
// }