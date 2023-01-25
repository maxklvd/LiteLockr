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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <string>

#include <windows.h>

namespace litelockr {

class Rectangle {
public:
    static RECT create(int x, int y, int width, int height);

    static int width(const RECT& rc) {
        return rc.right - rc.left;
    }

    static int height(const RECT& rc) {
        return rc.bottom - rc.top;
    }

    static SIZE size(const RECT& r) {
        return {Rectangle::width(r), Rectangle::height(r)};
    }

    static void offset(RECT& r, int dx, int dy) {
        r.left += dx;
        r.right += dx;

        r.top += dy;
        r.bottom += dy;
    }

    static void offset(RECT& r, POINT pt) {
        r.left += pt.x;
        r.right += pt.x;

        r.top += pt.y;
        r.bottom += pt.y;
    }

    static bool contains(const RECT& rc, const POINT& pt) {
        return pt.x >= rc.left && pt.x < rc.right &&
               pt.y >= rc.top && pt.y < rc.bottom;
    }

    static bool contains(const RECT& outer, const RECT& inner) {
        return inner.left >= outer.left &&
               inner.right <= outer.right &&
               inner.top >= outer.top &&
               inner.bottom <= outer.bottom;
    }

    static bool empty(const RECT& rc) {
        return width(rc) <= 0 || height(rc) <= 0;
    }

    static bool equals(const RECT& a, const RECT& b) {
        return a.left == b.left &&
               a.top == b.top &&
               a.right == b.right &&
               a.bottom == b.bottom;
    }

    static void clamp(RECT& rc, int minLeft, int minTop, int maxRight, int maxBottom) {
        if (rc.left < minLeft) {
            rc.left = minLeft;
        }
        if (rc.top < minTop) {
            rc.top = minTop;
        }
        if (rc.right > maxRight) {
            rc.right = maxRight;
        }
        if (rc.bottom > maxBottom) {
            rc.bottom = maxBottom;
        }
    }

    static POINT center(const RECT& rc) {
        return {(rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2};
    }

    static void inflate(RECT& rc, SIZE sz) {
        rc.left -= sz.cx;
        rc.top -= sz.cy;
        rc.right += sz.cx;
        rc.bottom += sz.cy;
    }

    static std::wstring toString(const RECT& rc);
};

} // namespace litelockr

#endif // RECTANGLE_H
