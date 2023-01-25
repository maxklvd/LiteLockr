/*
 * Copyright (C) 2020, 2022 Max Kolesnikov <maxklvd@gmail.com>
 *
 * This file is part of LiteLockr.
 *
 * LiteLockr is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LiteLockr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LiteLockr.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Executable.h"

#include <cassert>

#ifndef __MINGW32__
#include <Propkey.h>
#include <ShlObj.h>
#endif

#include <shlwapi.h>
#include <tlhelp32.h>
#include "log/Logger.h"

namespace litelockr {

std::wstring Executable::getFileName(std::wstring_view path) {
    return std::wstring(path.substr(path.find_last_of(L"/\\") + 1));
}

#ifndef __MINGW32__
std::wstring getShellPropStringFromPath(const std::wstring& path, PROPERTYKEY const& key) {
    IShellItem2 *pItem;
    HRESULT hr = SHCreateItemFromParsingName(path.c_str(), nullptr, IID_IShellItem2, (void**)&pItem);
    if (FAILED(hr)) {
        return L"";
    }

    wchar_t* pValue = nullptr;
    hr = pItem->GetString(key, &pValue);
    if (FAILED(hr)) {
        safeRelease(pItem);
        return L"";
    }

    std::wstring result;
    if (pValue) {
        result = pValue;
    }
    CoTaskMemFree(pValue);

    safeRelease(pItem);
    return result;
}
#endif

std::wstring Executable::getFileDescription(const std::wstring& path) {
#ifndef __MINGW32__
    return getShellPropStringFromPath(path, PKEY_FileDescription);
#else
    return L"";
#endif
}

std::wstring Executable::getInternalName(const std::wstring& path) {
#ifndef __MINGW32__
    return getShellPropStringFromPath(path, PKEY_InternalName);
#else
    return L"";
#endif
}

std::wstring Executable::getProductName(const std::wstring& path) {
#ifndef __MINGW32__
    return getShellPropStringFromPath(path, PKEY_Software_ProductName);
#else
    return L"";
#endif
}

std::wstring Executable::getFriendlyName(const std::wstring& path) {
    wchar_t buf[MAX_PATH] = {0};
    DWORD bufSize = MAX_PATH;
    HRESULT hr = AssocQueryString(ASSOCF_INIT_BYEXENAME, ASSOCSTR_FRIENDLYAPPNAME,
                                  path.c_str(), nullptr, buf, &bufSize);
    if (FAILED(hr)) {
        return L"";
    }
    return buf;
}

std::wstring Executable::getExecutableName(const std::wstring& path) {
    auto str = getFileName(path);
    auto pos = str.find_last_of(L'.');
    if (pos == std::wstring::npos) {
        return L"";
    }
    return str.substr(0, pos);
}

std::wstring Executable::getExecutableFileByPID(DWORD processId) {
    //
    // QueryFullProcessImageName
    //
    auto file = getExecutablePathByPID(processId);
    if (!file.empty()) {
        return getFileName(file);
    }

    //
    // CreateToolhelp32Snapshot
    //
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    assert(snapshot);
    if (!snapshot) {
        return L"";
    }

    const wchar_t *exeFile = L"";

    PROCESSENTRY32 process = {sizeof(process)};
    if (Process32First(snapshot, &process)) {
        do {
            if (processId == process.th32ProcessID) {
                exeFile = process.szExeFile;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }

    CloseHandle(snapshot);
    return exeFile;
}

std::wstring Executable::getExecutablePathByPID(DWORD processId) {
    wchar_t buf[MAX_PATH] = {0};

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess) {
        DWORD bufSize = MAX_PATH;
        QueryFullProcessImageName(hProcess, 0, buf, &bufSize);
        CloseHandle(hProcess);
    }
    return buf;
}


} // namespace litelockr