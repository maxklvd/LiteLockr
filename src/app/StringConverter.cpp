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

#include "StringConverter.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace litelockr {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127)
#endif

std::wstring StringConverter::toBase64(const std::wstring& str) {
    using namespace boost::archive::iterators;
    using it = base64_from_binary<transform_width<std::wstring::const_iterator, 6, 8>>;
    return {it(std::begin(str)), it(std::end(str))};
}

std::wstring StringConverter::fromBase64(const std::wstring& str) {
    using namespace boost::archive::iterators;
    using it = transform_width<binary_from_base64<std::wstring::const_iterator>, 8, 6>;
    try {
        return {it(std::begin(str)), it(std::end(str))};
    } catch (...) {
        assert(false);
    }
    return L"";
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

std::wstring StringConverter::toHex(const std::wstring& str) {
    std::wstringstream buf;

    for (wchar_t c: str) {
        auto ord = static_cast<unsigned int>(c);
        buf << std::setfill(L'0') << std::setw(2) << std::hex << ord;
    }

    return buf.str();
}

std::wstring StringConverter::fromHex(const std::wstring& str) {
    std::wstring buf;

    auto len = str.size();
    assert(len % 2 == 0);
    if (len % 2 == 0) {
        for (unsigned i = 0; i < len; i += 2) {
            std::wstring num;
            num += str[i];
            num += str[i + 1];
            int ord = std::stoi(num, nullptr, 16);

            buf += static_cast<wchar_t>(ord);
        }
    }

    return buf;
}

//
// Hides PIN from basic viewing
//
std::wstring StringConverter::encodePin(const std::wstring& pin) {
    if (pin.empty()) {
        return L"";
    }
    //
    // PIN > BASE64 > HEX STRING
    //
    return StringConverter::toHex(StringConverter::toBase64(pin));
}

//
// the opposite function of encodePin
//
std::wstring StringConverter::decodePin(const std::wstring& encodedPin) {
    if (encodedPin.empty()) {
        return L"";
    }
    return StringConverter::fromBase64(StringConverter::fromHex(encodedPin));
}

} // namespace litelockr

