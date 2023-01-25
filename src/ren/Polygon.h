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

#ifndef POLYGON_H
#define POLYGON_H

#include <vector>

#include "gfx/Bitmap.h"
#include "ren/DataType.h"

namespace litelockr {

class Polygon {
public:
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    unsigned materialIndex = 0;
    bool visible = true;

    Polygon() = default;

    Polygon(int _a, int _b, int _c, int _d)
            : a(_a), b(_b), c(_c), d(_d) {}
};

} // namespace litelockr

#endif // POLYGON_H
