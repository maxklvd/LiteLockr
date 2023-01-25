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

#ifndef BITMAP_H
#define BITMAP_H

#include <cassert>
#include <cstring>

#include <windows.h>
#include "gfx/Color.h"

namespace litelockr {

class Bitmap {
public:
    using PixFmtType = agg::pixfmt_alpha_blend_rgba<agg::blender_rgba_plain<agg::rgba8, agg::order_bgra>,
            agg::rendering_buffer>;

    Bitmap() : pixFmt_(renderingBuffer_) {}

    Bitmap(const Bitmap&) = delete;
    const Bitmap& operator=(const Bitmap&) = delete;

    ~Bitmap() {
        free();
    }

    [[nodiscard]] bool isNull() const { return hBitmap_ == nullptr; }

    [[nodiscard]] PixFmtType& pixFmt() { return pixFmt_; }
    [[nodiscard]] const PixFmtType& pixFmt() const { return pixFmt_; }

    explicit operator HBITMAP() const { return hBitmap_; }
    [[nodiscard]] const BITMAP& bitmap() const { return bitmap_; }
    [[nodiscard]] BITMAP& bitmap() { return bitmap_; }

    [[nodiscard]] unsigned width() const { return bitmap_.bmWidth; }
    [[nodiscard]] unsigned height() const { return bitmap_.bmHeight; }
    [[nodiscard]] SIZE size() const {
        return {bitmap_.bmWidth, bitmap_.bmHeight};
    }

    void free();
    bool create(unsigned width, unsigned height);
    bool createAs(const Bitmap& another);
    bool loadPng(const uint8_t *data, size_t size);
    bool loadPng(int resourceId);
    bool clone(const Bitmap& another);

    //
    // Some convenient methods
    //
    void clear(Color color);
    void copyFrom(const Bitmap& src, int dstX, int dstY, int srcX = 0, int srcY = 0);
    void copyFrom(const Bitmap& src, int dstX, int dstY, int srcX, int srcY, unsigned width, unsigned height);
    void blendFrom(const Bitmap& src, int dstX, int dstY, int srcX = 0, int srcY = 0, std::uint8_t cover = 255);
    void blendFrom(const Bitmap& src, int dstX, int dstY, int srcX, int srcY, unsigned width, unsigned height,
                   std::uint8_t cover = 255);
    void blitFrom(const Bitmap& from);
    void fillRect(const RECT& rect, Color color, uint8_t cover = 255);
    void fillRect(int x, int y, unsigned width, unsigned height, Color color, uint8_t cover = 255);

private:
    bool attach(HBITMAP hBitmap);

    BITMAP bitmap_{};
    HBITMAP hBitmap_ = nullptr;
    agg::rendering_buffer renderingBuffer_;
    PixFmtType pixFmt_;
};

} // namespace litelockr

#endif // BITMAP_H
