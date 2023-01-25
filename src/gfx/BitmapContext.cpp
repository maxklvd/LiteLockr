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

#include "BitmapContext.h"

#include <cmath>

#include "sys/Rectangle.h"

namespace litelockr {

void BitmapContext::rect(RECT rc) {
    buffer.fillRect(rc.left, rc.top, Rectangle::width(rc), Rectangle::height(rc), color, cover);
}

void BitmapContext::stroke() {
    auto& ao = aggObjects;
    ao.initialize(buffer);

    agg::conv_transform trans(ao.path, ao.transAffine);
    agg::conv_curve curve(trans);
    agg::conv_stroke p(curve);
    p.width(lineWidth);
    ao.rasterizer.add_path(p);

    ao.solidRenderer.color(strokeColor);
    agg::render_scanlines(ao.rasterizer, ao.scanline, ao.solidRenderer);
}

void BitmapContext::fill() {
    auto& ao = aggObjects;
    ao.initialize(buffer);

    agg::conv_transform trans(ao.path, ao.transAffine);
    agg::conv_curve curve(trans);
    ao.rasterizer.add_path(curve);

    ao.solidRenderer.color(fillColor);
    agg::render_scanlines(ao.rasterizer, ao.scanline, ao.solidRenderer);
}

void BitmapContext::drawTTF(const wchar_t *text, float tx, float ty, SIZE& textSize, bool draw) {

    using FontEngineType = agg::font_engine_win32_tt_int16;
    using FontManagerType = agg::font_cache_manager<FontEngineType>;
    FontEngineType fontEngine(GetDC(nullptr));
    FontManagerType fontManager(fontEngine);

    auto& ao = aggObjects;
    ao.initialize(buffer);

    agg::trans_affine transAffine;
    transAffine *= agg::trans_affine_translation(tx, ty);

    agg::conv_transform trans(fontManager.path_adaptor(), transAffine);
    agg::conv_curve fcurves(trans);
    fcurves.approximation_scale(2.0);

    fontEngine.height(fontHeight);
    fontEngine.flip_y(true);
    fontEngine.weight(fontWeight);
    fontEngine.italic(fontItalic);

    if (!fontEngine.create_font(fontFace, agg::glyph_ren_outline)) {
        return;
    }

    double x = 0;
    double y = 0;
    const wchar_t *p = text;

    while (*p) {
        const agg::glyph_cache *glyph = fontManager.glyph(*p);
        if (glyph) {
            fontManager.add_kerning(&x, &y);
            fontManager.init_embedded_adaptors(glyph, x, y);

            if (draw && glyph->data_type == agg::glyph_data_outline) {
                ao.rasterizer.reset();
                ao.rasterizer.add_path(fcurves);
                ao.solidRenderer.color(textColor);
                agg::render_scanlines(ao.rasterizer, ao.scanline, ao.solidRenderer);
            }

            x += glyph->advance_x + fontLetterSpacing;
            y += glyph->advance_y;
        }
        ++p;
    }

    textSize.cx = std::lround(x);
    textSize.cy = std::lround(fontHeight);
}

void BitmapContext::beginPath() {
    auto& ao = aggObjects;
    ao.path.remove_all();
    ao.transAffine.reset();
}

void BitmapContext::closePath() {
    aggObjects.path.end_poly();
}

void BitmapContext::drawText(const wchar_t *text, float x, float y) {
    SIZE size{};
    drawTTF(text, x, y, size, true);
}

SIZE BitmapContext::measureText(const wchar_t *text) {
    SIZE size{};
    drawTTF(text, 0, 0, size, false);
    return size;
}

void BitmapContext::blur(unsigned int radius) {
    agg::stack_blur<agg::rgba8, agg::stack_blur_calc_rgba<>> blur;
    blur.blur(buffer.pixFmt(), radius);
}

void BitmapContext::perspective(const Bitmap& image, const BitmapContext::QuadDouble& quad, std::uint8_t alpha) {
    using PixFmt = Bitmap::PixFmtType;
    using ImgAccessorType = agg::image_accessor_clone<PixFmt>;
    ImgAccessorType ia(image.pixFmt());

    auto& ao = aggObjects;
    ao.initialize(buffer);

    ao.rasterizer.reset();
    ao.rasterizer.move_to_d(quad[0], quad[1]);
    ao.rasterizer.line_to_d(quad[2], quad[3]);
    ao.rasterizer.line_to_d(quad[4], quad[5]);
    ao.rasterizer.line_to_d(quad[6], quad[7]);

    agg::span_allocator<PixFmt::color_type> sa;
    agg::image_filter_bilinear filterKernel;
    agg::image_filter_lut filter(filterKernel, false);

    double quadArr[8]; // C-style array is required by agg::trans_perspective constructor
    std::copy(std::begin(quad), std::end(quad), std::begin(quadArr));

    agg::trans_perspective tr(quadArr, 0, 0, image.width(), image.height());
    if (tr.is_valid()) {
        using InterpolatorType = agg::span_interpolator_trans<agg::trans_perspective>;
        InterpolatorType interpolator(tr);

        using SpanGenType = agg::span_image_filter_rgba_bilinear<ImgAccessorType, InterpolatorType>;
        SpanGenType sg(ia, interpolator);

        using SpanConv = agg::span_converter<SpanGenType, SpanConvAlpha<ImgAccessorType>>;

        SpanConvAlpha<ImgAccessorType> scAlpha(alpha);
        SpanConv sc(sg, scAlpha);
        agg::render_scanlines_aa(ao.rasterizer, ao.scanline, ao.baseRenderer, sa, sc);
    }
}

void BitmapContext::textOut(const wchar_t *text, int x, int y) {
    SIZE size = textSize(text);
    if (size.cx == 0 || size.cy == 0) {
        return;
    }

    HDC hDC = CreateCompatibleDC(nullptr);
    if (hDC) {
        auto hOldBmp = static_cast<HBITMAP>(SelectObject(hDC, buffer.operator HBITMAP()));
        HGDIOBJ hOldFont = SelectObject(hDC, font);

        const int len = lstrlen(text);
        SetBkMode(hDC, TRANSPARENT);
        SetTextColor(hDC, RGB(textColor.r, textColor.g, textColor.b));
        ExtTextOut(hDC, x, y, 0, nullptr, text, len, nullptr);

        SelectObject(hDC, hOldFont);
        SelectObject(hDC, hOldBmp);
        DeleteDC(hDC);

        BitmapUtils::fillAlpha(buffer, x, y, size.cx, size.cy, textColor.a);
    }
}

SIZE BitmapContext::textSize(const wchar_t *text) const {
    return BitmapContext::textSize(text, font, nullptr);
}

SIZE BitmapContext::textSize(const wchar_t *text, HFONT hFont, HWND hWnd) {
    SIZE size{};
    if (text) {
        HDC hDC = GetDC(hWnd);

        HGDIOBJ oldFont = nullptr;
        if (hFont) {
            oldFont = SelectObject(hDC, hFont);
        }
        GetTextExtentPoint32(hDC, text, lstrlen(text), &size);
        if (hFont) {
            SelectObject(hDC, oldFont);
        }
        ReleaseDC(hWnd, hDC);
    }
    return size;
}

void BitmapContext::rect(int x, int y, int width, int height) {
    buffer.fillRect(x, y, width, height, color, cover);
}

void BitmapContext::addRoundedRect(double x0, double y0, double x1, double y1, double r) {
    agg::rounded_rect rr(x0, y0, x1, y1, r);
    rr.normalize_radius();
    aggObjects.path.concat_path(rr);
}

void BitmapContext::addRect(double x0, double y0, double x1, double y1) {
    addRoundedRect(x0, y0, x1, y1, 0);
}

} // namespace litelockr
