
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

NavigationTree nav_tree = {};
IExplorerBrowser *_peb1;
IExplorerBrowser *_peb2;

#endif
