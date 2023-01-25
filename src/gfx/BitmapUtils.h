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

#ifndef BITMAP_UTILS_H
#define BITMAP_UTILS_H

#include <cstdint>
#include "gfx/Bitmap.h"

namespace litelockr {

class BitmapUtils {
public:
    static void applyMask(Bitmap& image, const Bitmap& mask);
    static void fillAlpha(Bitmap& image, int x, int y, unsigned width, unsigned height, std::uint8_t alpha);
};

} // namespace litelockr

#endif // BITMAP_UTILS_H
