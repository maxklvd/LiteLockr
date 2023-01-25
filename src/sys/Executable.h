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

#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include <string>

#include <windows.h>

namespace litelockr {

template<class T>
void safeRelease(T *& pT) {
    if (pT) {
        pT->Release();
        pT = nullptr;
    }
}

class Executable {
public:
    static std::wstring getFileName(std::wstring_view path);

    static std::wstring getFileDescription(const std::wstring& path);
    static std::wstring getInternalName(const std::wstring& path);
    static std::wstring getProductName(const std::wstring& path);
    static std::wstring getFriendlyName(const std::wstring& path);
    static std::wstring getExecutableName(const std::wstring& path);

    static std::wstring getExecutableFileByPID(DWORD processId);
    static std::wstring getExecutablePathByPID(DWORD processId);
};


} // namespace litelockr

#endif // EXECUTABLE_H
