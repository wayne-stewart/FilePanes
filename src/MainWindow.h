#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HWND CreateMainWindow(HINSTANCE hInstance);
void RunMainWindowLoop();
void RunMainWindowLoopWhileMessagesExist();

#endif