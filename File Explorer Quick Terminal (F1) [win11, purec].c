#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

#define HOTKEY_ID 1
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

// This code was compiled under Win11 using Mingw 15.2.0
// Example: 
// gcc "File Explorer Quick Terminal (F1) [win11, purec].c" -o "File Explorer Quick Terminal (F1) [win11, purec].exe" -lcomctl32 -lshell32 -mwindows
// Or with icon:
// gcc "File Explorer Quick Terminal (F1) [win11, purec].c" FEQT.o -o "File Explorer Quick Terminal (F1) [win11, purec].exe" -lcomctl32 -lshell32 -mwindows
// Icon resource, FEQT.rc, contents:
// 1 ICON "FEQT.ico"
// Icon resource gen:
// windres FEQT.rc -O coff -o FEQT.o

// Globals
NOTIFYICONDATA nid = {0};
LPSTR g_lpCmdLine;
int SleepTime = 33; // Default sleep time

char* save_clipboard_content();
void restore_clipboard_content(const char* buffer);
char* save_clipboard_content() {
    if (!OpenClipboard(NULL)) {
        return NULL;
    }

    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL) {
        CloseClipboard();
        return NULL;
    }

    char* text = (char*)GlobalLock(hData);
    if (text == NULL) {
        CloseClipboard();
        return NULL;
    }

    char* buffer = _strdup(text);

    GlobalUnlock(hData);
    CloseClipboard();

    return buffer;
}

void restore_clipboard_content(const char* buffer) {
    if (buffer == NULL) {
        return;
    }

    if (!OpenClipboard(NULL)) {
        return;
    }

    EmptyClipboard();

    size_t len = strlen(buffer) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hMem == NULL) {
        CloseClipboard();
        return;
    }

    memcpy(GlobalLock(hMem), buffer, len);
    GlobalUnlock(hMem);

    SetClipboardData(CF_TEXT, hMem);

    CloseClipboard();
}
void escape_backslashes(const char* input, char* output) {
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\\') {
            output[j++] = '\\';
            output[j++] = '\\';
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_HOTKEY:
            if (wParam == HOTKEY_ID) {
                HWND fgWindow = GetForegroundWindow();
                char className[256];
                GetClassName(fgWindow, className, sizeof(className));
                if (strcmp(className, "CabinetWClass") == 0) {
                    char* original_clipboard = save_clipboard_content();

                    // Simulate F4
                    keybd_event(VK_F4, 0, 0, 0);
                    keybd_event(VK_F4, 0, KEYEVENTF_KEYUP, 0);
                    Sleep(SleepTime);

                    // Simulate Ctrl+C
                    keybd_event(VK_CONTROL, 0, 0, 0);
                    keybd_event('C', 0, 0, 0);
                    keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
                    keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                    Sleep(SleepTime);

                    if (OpenClipboard(NULL)) {
                        HANDLE hData = GetClipboardData(CF_TEXT);
                        if (hData) {
                            char* clipboardText = (char*)GlobalLock(hData);
                            if (clipboardText) {
                                char escaped_path[512];
                                escape_backslashes(clipboardText, escaped_path);
                                char command[1024];
                                snprintf(command, sizeof(command), "wt.exe -d \"%s\"", escaped_path);
                                GlobalUnlock(hData);
                                
                                STARTUPINFO si = {0};
                                PROCESS_INFORMATION pi = {0};
                                si.cb = sizeof(si);
                                CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
                                CloseHandle(pi.hProcess);
                                CloseHandle(pi.hThread);
                            }
                        }
                        CloseClipboard();
                    }

                    restore_clipboard_content(original_clipboard);
                    if (original_clipboard) {
                        free(original_clipboard);
                    }
                }
            }
            break;

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                POINT curPoint;
                GetCursorPos(&curPoint);
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Quit");
                SetForegroundWindow(hwnd); // Necessary for menu to disappear correctly
                TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, curPoint.x, curPoint.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_TRAY_EXIT) {
                DestroyWindow(hwnd);
            }
            break;

        case WM_DESTROY:
            if (strstr(g_lpCmdLine, "--notrayicon") == NULL) {
                Shell_NotifyIcon(NIM_DELETE, &nid);
            }
            UnregisterHotKey(hwnd, HOTKEY_ID);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HANDLE hMutex = CreateMutex(NULL, TRUE, "FileExplorerQuickTerminalMutex");
    if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, "File Explorer Quick Terminal is already running, please use either trayicon or taskmanager to close it as needed", "Information", MB_OK | MB_ICONINFORMATION);
        if (hMutex) CloseHandle(hMutex);
        return 1;
    }

    g_lpCmdLine = lpCmdLine;
    const char CLASS_NAME[] = "ExplorerWT Hotkey Window";
    int fkey_vk = VK_F1; // Default hotkey
    int fkey_num = 1;

    char* timing_arg = strstr(lpCmdLine, "--timing");
    if (timing_arg) {
        int timing_val = atoi(timing_arg + strlen("--timing"));
        if (timing_val >= 1 && timing_val <= 99) {
            SleepTime = timing_val;
        }
    }

    char* fhotkey_arg = strstr(lpCmdLine, "--fhotkey");
    if (fhotkey_arg) {
        int fkey_scan = atoi(fhotkey_arg + strlen("--fhotkey"));
        if (fkey_scan >= 1 && fkey_scan <= 12) {
            fkey_vk = VK_F1 + (fkey_scan - 1);
            fkey_num = fkey_scan;
        }
    }

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "ExplorerWT", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    if (hwnd == NULL) {
        return 1;
    }

    // Register Hotkey
    if (!RegisterHotKey(hwnd, HOTKEY_ID, 0, fkey_vk)) {
        MessageBox(NULL, "Failed to register hotkey.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    if (strstr(lpCmdLine, "--notrayicon") == NULL) {
        // Setup Tray Icon
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
        snprintf(nid.szTip, sizeof(nid.szTip), "File Explorer Quick Terminal (F%d) [win11, purec]", fkey_num);

        Shell_NotifyIcon(NIM_ADD, &nid);
    }

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hMutex) CloseHandle(hMutex);
    return 0;
}
