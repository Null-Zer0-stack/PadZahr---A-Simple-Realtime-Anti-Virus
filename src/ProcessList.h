#pragma once
#ifndef PROCESS_LIST_H
#define PROCESS_LIST_H
#include <windows.h>
#include <tlhelp32.h>
#include <processthreadsapi.h>
#include<vector>
#include<string>

// Function to get a list of all running processes
std::vector<std::pair<std::wstring, DWORD>> getProcessList() {
    std::vector<std::pair<std::wstring, DWORD>> processes;
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) return processes;

    if (Process32FirstW(hProcessSnap, &pe32)) {
        do {
            processes.push_back({ pe32.szExeFile, pe32.th32ProcessID });
        } while (Process32NextW(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
    return processes;
}

#endif PROCESS_LIST_H