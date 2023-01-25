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

#include "UnlockGuardPainter.h"

#include <cassert>

#include "gfx/BitmapContext.h"
#include "gfx/BitmapUtils.h"
#include "sys/Rectangle.h"

#include "res/Resources.h"

namespace litelockr {

void UnlockGuardPainter::initialize(const RECT& workArea, const Bitmap& bgBuffer) {
    workArea_ = workArea;
    unsigned w = Rectangle::width(workArea);
    unsigned h = Rectangle::height(workArea);
    buffer_.create(w, h);

    gradientBackground_.loadPng(IDB_GRADIENT_DARK);
    assert(!gradientBackground_.isNull());
    assert(gradientBackground_.width() == w);
    assert(gradientBackground_.height() == h);

    gradientText_.loadPng(IDB_GRADIENT_TEXT);
    assert(!gradientText_.isNull());
    assert(gradientText_.width() == w);
    assert(gradientText_.height() == h);

    bgBuffer_.createAs(bgBuffer);
    bgBuffer_.clear({55, 55, 55, 255}); // main background color
    bgBuffer_.blendFrom(bgBuffer, 0, 0);
}

void UnlockGuardPainter::drawTopText(const std::wstring& text, bool useCache /*= false*/) {
    if (useCache) {
        if (!cacheBuffer_.isNull()) {
            buffer_.blitFrom(cacheBuffer_);
            return;
        }
    } else {
        cacheBuffer_.free();
    }

    Bitmap mask;
    mask.createAs(buffer_);

    const TextSettings& ts = pinEnabled_ ? pinTopTextSettings_ : mathTopTextSettings_;

    BitmapContext ctx(mask);
    ctx.textColor = Color(255, 255, 255);
    ctx.fontFace = ts.fontFace;
    ctx.fontHeight = ts.fontHeight;
    ctx.fontLetterSpacing = ts.fontLetterSpacing;
    ctx.fontWeight = FW_BOLD;
    unsigned w = buffer_.width();
    unsigned h = buffer_.height();
    float tw = static_cast<float>(ctx.measureText(text.c_str()).cx) - ts.fontLetterSpacing;
    float tx = static_cast<float>(w) / 2.0f - tw / 2.0f;

    ctx.clear({0, 0, 0, 255});
    ctx.drawText(text.c_str(), tx, static_cast<float>(ts.positionY) + offsetText0Y);

    Bitmap textBuf;
    textBuf.clone(gradientText_);
    BitmapUtils::applyMask(textBuf, mask);

    if (backgroundBlurRadius) {
        BitmapContext ctxBlur(bgBuffer_);
        ctxBlur.blur(backgroundBlurRadius);
    }

    auto&& wa = workArea_;
    buffer_.copyFrom(bgBuffer_, 0, 0, wa.left, wa.top, w, h);

    buffer_.blendFrom(gradientBackground_, 0, 0);
    buffer_.blendFrom(textBuf, 0, 0, 0, 0, textOpacity);

    if (useCache && cacheBuffer_.isNull()) {
        cacheBuffer_.clone(buffer_);
    }
}

void UnlockGuardPainter::drawBottomText(const std::wstring& text) {
    if (text.empty()) {
        return;
    }

    Bitmap mask;
    mask.createAs(buffer_);

    const TextSettings& ts = pinEnabled_ ? pinBottomTextSettings_ : mathBottomTextSettings_;

    BitmapContext ctx(mask);
    ctx.textColor = Color(255, 255, 255);
    ctx.fontFace = ts.fontFace;
    ctx.fontHeight = ts.fontHeight;
    ctx.fontLetterSpacing = ts.fontLetterSpacing;
    ctx.fontWeight = FW_BOLD;
    unsigned w = buffer_.width();
    float tw = static_cast<float>(ctx.measureText(text.c_str()).cx) - ts.fontLetterSpacing;
    float tx = static_cast<float>(w) / 2.0f - tw / 2.0f;

    ctx.clear({0, 0, 0, 255});
    ctx.drawText(text.c_str(), tx + offsetText1X, static_cast<float>(ts.positionY) + offsetText1Y);

    Bitmap textBuf;
    textBuf.clone(gradientText_);
    BitmapUtils::applyMask(textBuf, mask);
    buffer_.blendFrom(textBuf, 0, 0, 0, 0, textOpacity);
}

} // namespace litelockr

