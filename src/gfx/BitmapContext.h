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

#ifndef BITMAP_CONTEXT_H
#define BITMAP_CONTEXT_H

#include <array>
#include <cmath>
#include <numbers>
#include <string>

#include "gfx/Bitmap.h"
#include "gfx/BitmapUtils.h"
#include "sys/KeyFrames.h"

namespace litelockr {

enum class WaveDirection {
    MoveLeft = 0,
    MoveUp = 1,
    MoveRight = 2,
    MoveDown = 3,
};

template<class Source>
class SpanConvAlpha;

template<WaveDirection direction>
class TransWave;

class BitmapContext {
public:
    using QuadDouble = std::array<double, 8>;

    Bitmap& buffer;
    Color color{0, 0, 0};
    Color fillColor{0, 0, 0};
    Color strokeColor{0, 0, 0};
    Color textColor{0, 0, 0};
    float lineWidth = 1;
    HFONT font = nullptr;
    unsigned char cover = 255;

    //
    // True type font properties
    //
    const char *fontFace = "Arial";
    bool fontItalic = false;
    int fontWeight = FW_REGULAR;
    float fontHeight = 16;
    float fontLetterSpacing = 0;

    explicit BitmapContext(Bitmap& buffer) : buffer(buffer) {}

    void beginPath();
    void closePath();

    template<typename T>
    void moveTo(T x, T y) {
        aggObjects.path.move_to(double(x), double(y));
    }

    template<typename T>
    void lineTo(T x, T y) {
        aggObjects.path.line_to(double(x), double(y));
    }

    void addRoundedRect(double x0, double y0, double x1, double y1, double r);
    void addRect(double x0, double y0, double x1, double y1);

    template<typename T>
    void scale(T scaleXY) {
        aggObjects.transAffine *= agg::trans_affine_scaling(double(scaleXY));
    }

    template<typename T>
    void scale(T scaleX, T scaleY) {
        aggObjects.transAffine *= agg::trans_affine_scaling(double(scaleX), double(scaleY));
    }

    template<typename T>
    void rotate(T angle) {
        aggObjects.transAffine *= agg::trans_affine_rotation(angle * std::numbers::pi / 180.0);
    }

    template<typename T>
    void translate(T x, T y) {
        aggObjects.transAffine *= agg::trans_affine_translation(x, y);
    }

    void stroke();
    void fill();

    void textOut(const wchar_t *text, int x, int y);

    void textOut(const std::wstring& text, int x, int y) {
        textOut(text.c_str(), x, y);
    }

    SIZE textSize(const wchar_t *text) const;

    [[nodiscard]] SIZE textSize(const std::wstring& text) const {
        return textSize(text.c_str());
    }

    static SIZE textSize(const wchar_t *text, HFONT hFont, HWND hWnd);

    //
    // Primitives
    //
    void clear(Color c) {
        buffer.clear(c);
    }

    void rect(int x, int y, int width, int height);
    void rect(RECT rc);

    //
    // True type drawTopText text
    //
    void drawText(const wchar_t *text, float x, float y);

    SIZE measureText(const wchar_t *text);

    void blur(unsigned radius);

    void perspective(const Bitmap& image, const QuadDouble& quad, std::uint8_t alpha);

    template<WaveDirection direction>
    void wave(const Bitmap& image, float value, std::uint8_t alpha);

    void drawTTF(const wchar_t *text, float tx, float ty, SIZE& textSize, bool draw);

    struct {
        using BaseRenderer = agg::renderer_base<Bitmap::PixFmtType>;
        using SolidRenderer = agg::renderer_scanline_aa_solid<BaseRenderer>;

        bool hasInitialized = false;

        BaseRenderer baseRenderer;
        SolidRenderer solidRenderer;
        agg::rasterizer_scanline_aa<> rasterizer;
        agg::scanline_p8 scanline;
        agg::path_storage path;
        agg::trans_affine transAffine;

        void initialize(Bitmap& buf) {
            if (!hasInitialized) {
                hasInitialized = true;

                baseRenderer.attach(buf.pixFmt());
                solidRenderer.attach(baseRenderer);
            }
        }
    } aggObjects;
};

template<class Source>
class SpanConvAlpha {
public:
    using SourceType = Source;
    using ColorType = typename SourceType::color_type;

    explicit SpanConvAlpha(const std::uint8_t alpha) : alpha_(alpha) {}

    void prepare() const { /* Nothing to do. This method is required by span_converter. */ }

    void generate(ColorType *span, int, int, unsigned len) const;

private:
    const std::uint8_t alpha_;
};

template<WaveDirection direction>
class TransWave {
public:
    using AxisWarp = std::vector<std::pair<float, float>>;

    TransWave(unsigned width, unsigned height, float value, float shift) {
        initializeXAxisWarp(width, height, value, shift);
        initializeYAxisWarp(width, height, value, shift);
    }

    void transform(double *px, double *py) const;

private:
    void initializeXAxisWarp(unsigned width, unsigned height, float value, float shift);
    void initializeYAxisWarp(unsigned width, unsigned height, float value, float shift);

    [[nodiscard]] constexpr float applyTransform(float value, const AxisWarp::value_type& warp) const {
        float scale = warp.first;
        float offset = warp.second;
        return value * scale - offset + 0.5f;
    }

    [[nodiscard]] constexpr float toStart(unsigned /*size*/, float /*scale*/) const {
        return 0;
    }

    [[nodiscard]] constexpr float toCenter(unsigned size, float scale) const {
        return toEnd(size, scale) / 2.0f;
    }

    [[nodiscard]] constexpr float toEnd(unsigned size, float scale) const {
        float scaledSize = static_cast<float>(size) / scale;
        return (static_cast<float>(size) - scaledSize) * scale;
    }

    [[nodiscard]] float interpolateCurveU(int size, int x, float val) const;
    [[nodiscard]] float interpolateCurveS(int size, int x, float val) const;

    AxisWarp xAxisWarp_;
    AxisWarp yAxisWarp_;
};

template<WaveDirection direction>
inline
void TransWave<direction>::transform(double *px, double *py) const {
    auto x = static_cast<int>(*px);
    auto y = static_cast<int>(*py);
    x = std::clamp(x, 0, static_cast<int>(yAxisWarp_.size()) - 1);
    y = std::clamp(y, 0, static_cast<int>(xAxisWarp_.size()) - 1);
    *px = applyTransform(x, xAxisWarp_[y]);
    *py = applyTransform(y, yAxisWarp_[x]);
}

template<WaveDirection direction>
inline
void TransWave<direction>::initializeXAxisWarp(unsigned int width, unsigned int height, float value, float shift) {
    xAxisWarp_.resize(height);

    for (unsigned y = 0; y < height; y++) {
        if constexpr (direction == WaveDirection::MoveDown) {
            float scale = interpolateCurveS(height, y, value);
            xAxisWarp_[y] = std::make_pair(scale, toCenter(width, scale));
        } else if constexpr (direction == WaveDirection::MoveUp) {
            float scale = interpolateCurveS(height, height - y, value);
            xAxisWarp_[y] = std::make_pair(scale, toCenter(width, scale));
        } else if constexpr (direction == WaveDirection::MoveLeft) {
            float scale = interpolateCurveU(height, height - y, value);
            xAxisWarp_[y] = std::make_pair(scale, toStart(width, scale) - shift);
        } else if constexpr (direction == WaveDirection::MoveRight) {
            float scale = interpolateCurveU(height, y, value);
            xAxisWarp_[y] = std::make_pair(scale, toEnd(width, scale) + shift);
        }
    }
}

template<WaveDirection direction>
inline
void TransWave<direction>::initializeYAxisWarp(unsigned int width, unsigned int height, float value, float shift) {
    yAxisWarp_.resize(width);

    for (unsigned x = 0; x < width; x++) {
        if constexpr (direction == WaveDirection::MoveDown) {
            float scale = interpolateCurveU(width, x, value);
            yAxisWarp_[x] = std::make_pair(scale, toEnd(height, scale) + shift);
        } else if constexpr (direction == WaveDirection::MoveUp) {
            float scale = interpolateCurveU(width, x, value);
            yAxisWarp_[x] = std::make_pair(scale, toStart(height, scale) - shift);
        } else if constexpr (direction == WaveDirection::MoveLeft) {
            float scale = interpolateCurveS(width, width - x, value);
            yAxisWarp_[x] = std::make_pair(scale, toCenter(height, scale));
        } else if constexpr (direction == WaveDirection::MoveRight) {
            float scale = interpolateCurveS(width, x, value);
            yAxisWarp_[x] = std::make_pair(scale, toCenter(height, scale));
        }
    }
}

template<WaveDirection direction>
inline
float TransWave<direction>::interpolateCurveU(int size, int x, float val) const {
    KeyFrames kf;
    kf.addStopPoint(0, 1, Interpolator::EASY_OUT);
    kf.addStopPoint(0.5f, 1 + 0.25f * val, Interpolator::EASY_IN);
    kf.addStopPoint(1, 1);
    return kf.interpolate(static_cast<float>(x) / static_cast<float>(size));
}

template<WaveDirection direction>
inline
float TransWave<direction>::interpolateCurveS(int size, int x, float val) const {
    KeyFrames kf;
    kf.addStopPoint(0, 1 + 0.3875f * val, Interpolator::EASY_OUT);
    kf.addStopPoint(0.1f, 1 + 0.35f * val, Interpolator::EASY_BOTH);
    kf.addStopPoint(1, 1 + 3 * val);
    return kf.interpolate(static_cast<float>(x) / static_cast<float>(size));
}

template<class Source>
inline
void SpanConvAlpha<Source>::generate(ColorType *span, int, int, unsigned len) const {
    do {
        span->a = span->a * alpha_ >> 8;
        ++span;
    } while (--len);
}

template<WaveDirection direction>
inline
void BitmapContext::wave(const Bitmap& image, float value, std::uint8_t alpha) {
    using PixFmt = Bitmap::PixFmtType;
    using ImgAccessorType = agg::image_accessor_clone<PixFmt>;
    ImgAccessorType ia(image.pixFmt());

    auto& ao = aggObjects;
    ao.initialize(buffer);

    unsigned width = image.width();
    unsigned height = image.height();

    ao.rasterizer.reset();
    ao.rasterizer.move_to_d(0, 0);
    ao.rasterizer.line_to_d(width, 0);
    ao.rasterizer.line_to_d(width, height);
    ao.rasterizer.line_to_d(0, height);

    agg::span_allocator<PixFmt::color_type> sa;
    agg::image_filter_bilinear filterKernel;
    agg::image_filter_lut filter(filterKernel, false);

    const float shift = static_cast<float>(width) * value * 0.7f;
    TransWave<direction> tr(width, height, value, shift);

    using InterpolatorType = agg::span_interpolator_trans<TransWave<direction>>;
    InterpolatorType interpolator(tr);

    using SpanGenType = agg::span_image_filter_rgba_bilinear<ImgAccessorType, InterpolatorType>;
    SpanGenType sg(ia, interpolator);

    using SpanConv = agg::span_converter<SpanGenType, SpanConvAlpha<ImgAccessorType>>;

    SpanConvAlpha<ImgAccessorType> scAlpha(alpha);
    SpanConv sc(sg, scAlpha);
    agg::render_scanlines_aa(ao.rasterizer, ao.scanline, ao.baseRenderer, sa, sc);
}

} // namespace litelockr

#endif // BITMAP_CONTEXT_H
