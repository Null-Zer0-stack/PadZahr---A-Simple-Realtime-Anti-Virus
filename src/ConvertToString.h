#pragma once
#ifndef CONVERT_TO_STRING_H
#define CONVERT_TO_STRING_H

#include <string>

// Helper function to convert a wide-character string (from Win32)
// to a UTF-8 string that Ultralight and JavaScript can use.
std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}


#endif