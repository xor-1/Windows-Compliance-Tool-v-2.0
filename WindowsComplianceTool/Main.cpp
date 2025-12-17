#include <windows.h>
#include "MainForm.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    MainForm form;
    if (!form.Create(hInstance, nCmdShow)) {
        DWORD error = GetLastError();
        char errorMsg[256];
        sprintf_s(errorMsg, "Failed to create window. Error code: %lu", error);
        MessageBoxA(nullptr, errorMsg, "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    int result = form.MessageLoop();

    // Debug output to see the exit code produced by MessageLoop()
    char buf[64];
    sprintf_s(buf, "MessageLoop returned %d\n", result);
    OutputDebugStringA(buf);

    CoUninitialize();
    return result;
}

