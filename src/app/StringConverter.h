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

#ifndef STRING_CONVERTER_H
#define STRING_CONVERTER_H

#include <string>

namespace litelockr {

class StringConverter {
public:
    static std::wstring toBase64(const std::wstring& str);
    static std::wstring fromBase64(const std::wstring& str);
    static std::wstring toHex(const std::wstring& str);
    static std::wstring fromHex(const std::wstring& str);

    static std::wstring encodePin(const std::wstring& str);
    static std::wstring decodePin(const std::wstring& str);
};

} // namespace litelockr

#endif // STRING_CONVERTER_H
