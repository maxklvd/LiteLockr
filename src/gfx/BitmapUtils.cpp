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

#include "BitmapUtils.h"

#include <cstdlib>

namespace litelockr {

void BitmapUtils::applyMask(Bitmap& image, const Bitmap& mask) {
    const unsigned width = image.width();
    const unsigned height = image.height();
    assert(width == mask.width());
    assert(height == mask.height());

    for (unsigned i = 0; i < height; i++) {
        auto dst = image.pixFmt().row_ptr(i);
        auto src = mask.pixFmt().row_ptr(i);

        dst += 3; // shift to the alpha channel

        unsigned j = width;
        do {
            *dst = (((*dst) * (*src)) >> 8);
            dst += 4;
            src += 4;
        } while (--j);
    }
}

void BitmapUtils::fillAlpha(Bitmap& image, int x, int y, unsigned width, unsigned height, std::uint8_t alpha) {
    for (unsigned i = 0; i < height; i++) {
        auto ptr = image.pixFmt().pix_ptr(x, y + i);

        ptr += 3; // shift to the alpha channel

        unsigned j = width;
        do {
            *ptr = alpha;
            ptr += 4;
        } while (--j);
    }
}


} // namespace litelockr
