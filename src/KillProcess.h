#pragma once
#ifndef KILL_PROCESS_H
#define KILL_PROCESS_H
#include <Windows.h>
#include < processthreadsapi.h >

// Function to terminate a process by PID
bool killProcess(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (hProcess == NULL) return false;

    bool result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result;
}
#endif 
