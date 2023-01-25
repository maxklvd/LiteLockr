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

#include "Bitmap.h"

#include <lodepng/lodepng.h>
#include "log/Logger.h"
#include "sys/BinaryResource.h"

namespace litelockr {

void Bitmap::free() {
    if (hBitmap_) {
        DeleteObject(hBitmap_);
        hBitmap_ = nullptr;
    }
}

bool Bitmap::attach(HBITMAP hBitmap) {
    assert(hBitmap);

    if (GetObject(hBitmap, sizeof(bitmap_), &bitmap_)) {
        assert(bitmap_.bmBitsPixel == PixFmtType::pix_width << 3);

        hBitmap_ = hBitmap;
        renderingBuffer_.attach(reinterpret_cast<unsigned char *>(bitmap_.bmBits), bitmap_.bmWidth,
                                bitmap_.bmHeight, -bitmap_.bmWidthBytes);
        pixFmt_.attach(renderingBuffer_);
        return true;
    }
    return false;
}

bool Bitmap::create(unsigned width, unsigned height) {
    assert(width > 0);
    assert(height > 0);

    if (hBitmap_) {
        if (this->width() == width && this->height() == height) {
            return true;
        }
    }

    HDC hMemDC = CreateCompatibleDC(nullptr);
    if (hMemDC) {
        BITMAPINFOHEADER bih{sizeof(bih)};
        bih.biWidth = static_cast<long>(width);
        bih.biHeight = static_cast<long>(height);
        bih.biPlanes = 1;
        bih.biBitCount = PixFmtType::pix_width << 3;
        bih.biCompression = BI_RGB;

        void *bits;
        HBITMAP hDib = CreateDIBSection(hMemDC, reinterpret_cast<BITMAPINFO *>(&bih),
                                        DIB_RGB_COLORS, &bits, nullptr, 0);
        DeleteDC(hMemDC);
        if (hDib) {
            return attach(hDib);
        }
    }

    LOG_ERROR(L"Could not create an image (size=%dx%d)", width, height);
    return false;
}

bool Bitmap::createAs(const Bitmap& another) {
    assert(!another.isNull());
    return create(another.width(), another.height());
}

bool Bitmap::loadPng(const uint8_t *data, size_t size) {
    std::vector<unsigned char> image;
    unsigned width;
    unsigned height;
    unsigned error = lodepng::decode(image, width, height, data, size);
    if (error) {
        assert(false);
        return false;
    }

    if (!create(width, height)) {
        assert(false);
        return false;
    }

    auto src = image.data();
    auto dst = reinterpret_cast<uint8_t *>(bitmap().bmBits) + bitmap().bmWidthBytes * (height - 1);

    const unsigned widthBytes = width * PixFmtType::pix_width;

    for (unsigned y = 0; y < height; y++) {
        for (unsigned i = 0; i < widthBytes; i += PixFmtType::pix_width) {
            dst[i + 0] = src[i + ColorR];
            dst[i + 1] = src[i + ColorG];
            dst[i + 2] = src[i + ColorB];
            dst[i + 3] = src[i + ColorA];
        }

        src += widthBytes;
        dst -= bitmap().bmWidthBytes;
    }
    return true;
}

bool Bitmap::loadPng(int resourceId) {
    bool result = loadPng(Bin::data(resourceId), Bin::size(resourceId));
    if (!result) {
        LOG_ERROR(L"Could not load an image from resource (id=0x%04x)", resourceId);
    }
    return result;
}

bool Bitmap::clone(const Bitmap& another) {
    if (createAs(another)) {
        blitFrom(another);
        return true;
    }
    return false;
}

void Bitmap::clear(Color color) {
    agg::renderer_base rb(pixFmt());
    rb.clear(color);
}

void Bitmap::copyFrom(const Bitmap& src, int dstX, int dstY, int srcX, int srcY) {
    copyFrom(src, dstX, dstY, srcX, srcY, src.width(), src.height());
}

void Bitmap::copyFrom(const Bitmap& src, int dstX, int dstY, int srcX, int srcY, unsigned width, unsigned height) {
    for (unsigned i = 0; i < height; i++) {
        pixFmt().copy_from(src.pixFmt(), dstX, dstY + i, srcX, srcY + i, width);
    }
}

void Bitmap::blendFrom(const Bitmap& src, int dstX, int dstY, int srcX, int srcY, std::uint8_t cover) {
    blendFrom(src, dstX, dstY, srcX, srcY, src.width(), src.height(), cover);
}

void Bitmap::blendFrom(const Bitmap& src, int dstX, int dstY, int srcX, int srcY,
                       unsigned width, unsigned height, std::uint8_t cover) {
    for (unsigned i = 0; i < height; i++) {
        pixFmt().blend_from(src.pixFmt(), dstX, dstY + i, srcX, srcY + i, width, cover);
    }
}

void Bitmap::blitFrom(const Bitmap& from) {
    assert(bitmap_.bmBitsPixel == from.bitmap_.bmBitsPixel);
    assert(bitmap_.bmWidthBytes == from.bitmap_.bmWidthBytes);
    assert(bitmap_.bmHeight == from.bitmap_.bmHeight);

    const auto src = from.bitmap_.bmBits;
    auto dst = bitmap_.bmBits;
    size_t size = bitmap_.bmWidthBytes * bitmap_.bmHeight;

    if (dst && src) {
        std::memmove(dst, src, size);
    }
}

void Bitmap::fillRect(const RECT& rect, Color color, uint8_t cover) {
    fillRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, color, cover);
}

void Bitmap::fillRect(int x, int y, unsigned width, unsigned height, Color color, uint8_t cover) {
    if (cover == 255) {
        for (unsigned i = 0; i < height; i++) {
            pixFmt().copy_hline(x, y + i, width, color);
        }
    } else {
        for (unsigned i = 0; i < height; i++) {
            pixFmt().blend_hline(x, y + i, width, color, cover);
        }
    }
}

} // namespace litelockr
