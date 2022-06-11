#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND CreateMainWindow(HINSTANCE hInstance);
void RunMainWindowLoop();
void RunMainWindowLoopWhileMessagesExist();

void Alert(LPCWSTR format, ...)
{
    va_list args;
    va_start(args, format);
    WCHAR buffer[1024] = {};
    vswprintf(buffer, format, args);
    va_end(args);
    MessageBoxW(NULL, buffer, L"Alert", MB_OK | MB_ICONEXCLAMATION);
}

#endif