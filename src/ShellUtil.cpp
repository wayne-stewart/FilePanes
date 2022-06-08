#include <windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>


bool GetItemIdListFromPath(LPCWSTR path, LPITEMIDLIST *items)
{
    LPSHELLFOLDER shell_folder;
    HRESULT hr;
    ULONG eaten;

    hr = SHGetDesktopFolder(&shell_folder);
    if (FAILED(hr)) return false;

    hr = shell_folder->ParseDisplayName(NULL, NULL, (LPWSTR)path, &eaten, items, NULL);

    shell_folder->Release();

    if (FAILED(hr))
    {
        *items = NULL;
        return false;
    }

    return true;
}

// bool GetItemIdListForDesktop(LPITEMIDLIST *items)
// {
//     Shget
// }