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

#include "StringUtils.h"

#include <locale>

namespace litelockr {

void StringUtils::toUpperCase(std::wstring& str) {
    for (auto& c: str) {
        c = std::toupper(c, std::locale());
    }
}

std::wstring StringUtils::toUpperCase(const std::wstring& str) {
    std::wstring buf(str);
    toUpperCase(buf);
    return buf;
}

std::wstring StringUtils::prepareSearchString(std::wstring str) {
    for (auto&& c: str) {
        if (c == L'\u00A0') { // 'NO-BREAK SPACE' character
            c = L' ';
        }
        c = std::toupper(c, std::locale());
    }
    return str;
}

std::wstring StringUtils::truncate(std::wstring str, size_t width) {
    if (str.length() > width) {
        return str.substr(0, width) + L"...";
    }
    return str;
}

} // namespace litelockr
