#include "ComplianceCheck.h"
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#include <sstream>
#include <fstream>

#pragma comment(lib, "wbemuuid.lib")

// FIXED: No more CMD popup windows
std::string ComplianceCheck::executePowerShell(const std::string& command) {
    std::string result;
    std::string psCommand = "powershell.exe -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass -Command \"" + command + "\"";

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hReadPipe, hWritePipe;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        return "";
    }

    STARTUPINFOA si = {};
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;  // CRITICAL: Hide the window

    PROCESS_INFORMATION pi = {};

    if (CreateProcessA(NULL, (LPSTR)psCommand.c_str(), NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hWritePipe);

        char buffer[4096];
        DWORD bytesRead;

        while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            result += buffer;
        }

        WaitForSingleObject(pi.hProcess, 30000); // 30 second timeout
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        CloseHandle(hWritePipe);
    }

    CloseHandle(hReadPipe);

    // Remove trailing newlines
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }

    return result;
}

std::string ComplianceCheck::readRegistryValue(const std::string& keyPath, const std::string& valueName) {
    HKEY hKey;
    std::string result;

    size_t backslashPos = keyPath.find('\\');
    if (backslashPos == std::string::npos) return result;

    std::string rootKey = keyPath.substr(0, backslashPos);
    std::string subKey = keyPath.substr(backslashPos + 1);

    HKEY hRootKey = HKEY_LOCAL_MACHINE;
    if (rootKey == "HKEY_CURRENT_USER") hRootKey = HKEY_CURRENT_USER;
    else if (rootKey == "HKEY_CLASSES_ROOT") hRootKey = HKEY_CLASSES_ROOT;
    else if (rootKey == "HKEY_USERS") hRootKey = HKEY_USERS;

    LONG lResult = RegOpenKeyExA(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS) {
        DWORD dwType;
        DWORD dwSize = 0;

        RegQueryValueExA(hKey, valueName.c_str(), nullptr, &dwType, nullptr, &dwSize);

        if (dwSize > 0) {
            char* buffer = new char[dwSize];
            if (RegQueryValueExA(hKey, valueName.c_str(), nullptr, &dwType, (LPBYTE)buffer, &dwSize) == ERROR_SUCCESS) {
                if (dwType == REG_SZ || dwType == REG_EXPAND_SZ) {
                    result = std::string(buffer);
                }
                else if (dwType == REG_DWORD) {
                    DWORD value = *(DWORD*)buffer;
                    result = std::to_string(value);
                }
            }
            delete[] buffer;
        }

        RegCloseKey(hKey);
    }

    return result;
}

bool ComplianceCheck::registryValueExists(const std::string& keyPath, const std::string& valueName) {
    HKEY hKey;

    size_t backslashPos = keyPath.find('\\');
    if (backslashPos == std::string::npos) return false;

    std::string rootKey = keyPath.substr(0, backslashPos);
    std::string subKey = keyPath.substr(backslashPos + 1);

    HKEY hRootKey = HKEY_LOCAL_MACHINE;
    if (rootKey == "HKEY_CURRENT_USER") hRootKey = HKEY_CURRENT_USER;
    else if (rootKey == "HKEY_CLASSES_ROOT") hRootKey = HKEY_CLASSES_ROOT;
    else if (rootKey == "HKEY_USERS") hRootKey = HKEY_USERS;

    LONG lResult = RegOpenKeyExA(hRootKey, subKey.c_str(), 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS) {
        DWORD dwType;
        DWORD dwSize = 0;
        lResult = RegQueryValueExA(hKey, valueName.c_str(), nullptr, &dwType, nullptr, &dwSize);
        RegCloseKey(hKey);
        return (lResult == ERROR_SUCCESS);
    }

    return false;
}