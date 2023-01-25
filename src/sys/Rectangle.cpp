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

#include "Rectangle.h"

#include <cassert>
#include <sstream>

namespace litelockr {

RECT Rectangle::create(int x, int y, int width, int height) {
    assert(width >= 0);
    assert(height >= 0);

    return {x, y, x + width, y + height};
}

std::wstring Rectangle::toString(const RECT& rc) {
    //
    // {LT(0, 0) RB(10, 10)  [10 x 10]}
    //
    std::wstringstream buf;
    buf << L"{LT(";
    buf << rc.left;
    buf << L", ";
    buf << rc.top;
    buf << L") RB(";
    buf << rc.right;
    buf << L", ";
    buf << rc.bottom;
    buf << L")  [";
    buf << width(rc);
    buf << L" x ";
    buf << height(rc);
    buf << L"]}";
    return buf.str();
}

} // namespace litelockr
