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

#include "UnlockGuardView.h"

#include <cmath>

#include "app/guard/UnlockGuardPainter.h"
#include "ini/SettingsData.h"
#include "sys/KeyFrames.h"

namespace litelockr {

void UnlockGuardShowAnimation::create(AnimationFrameSet& result, Bitmap& buffer, const RECT& workArea,
                                      const std::wstring& line0, const std::wstring& line1, bool hide) {
    const int numFrames = hide ? 8 : 15;
    result.recreate(buffer.width(), buffer.height(), numFrames);

    auto&& wa = workArea;

    auto pinEnabled = SettingsData::instance().pinEnabled();

    if (hide) {
        UnlockGuardPainter painter(pinEnabled);
        painter.initialize(wa, buffer);
        painter.drawTopText(line0);
        painter.drawBottomText(line1);

        Bitmap image;
        image.createAs(buffer);
        image.copyFrom(painter.buffer(), wa.left, wa.top);

#pragma omp parallel for
        for (int i = 0; i < numFrames; i++) {
            Bitmap tmp;
            tmp.clone(buffer);

            int alpha = std::lround(255 * EASY_IN(1.0f - i / static_cast<float>(numFrames - 1)));
            tmp.blendFrom(image, 0, 0, 0, 0, static_cast<std::uint8_t>(alpha));

            result.addFrame(i, tmp);
        }
    } else {
#pragma omp parallel for
        for (int i = 0; i < numFrames; i++) {
            float val = EASY_IN(i / (float) (numFrames - 1));
            float invVal = EASY_OUT(1.0f - i / (float) (numFrames - 1));

            UnlockGuardPainter painter(pinEnabled);
            painter.initialize(wa, buffer);
            painter.offsetText0Y = painter.offsetText1Y = -8.0f * invVal;
            painter.backgroundBlurRadius = std::lround(UnlockGuardPainter::BACKGROUND_BLUR_RADIUS * val);
            painter.textOpacity = static_cast<std::uint8_t>(std::lround(UnlockGuardPainter::TEXT_OPACITY * val));
            painter.drawTopText(line0);
            painter.drawBottomText(line1);

            Bitmap tmp;
            tmp.createAs(buffer);
            tmp.copyFrom(painter.buffer(), wa.left, wa.top);

            result.addFrame(i, tmp);
        }
    }

    result.begin();
}

void UnlockGuardWrongAnimation::create(AnimationFrameSet& result, Bitmap& buffer, const RECT& workArea,
                                       const std::wstring& line0, const std::wstring& line1) {
    const int numFrames = 21;
    result.recreate(buffer.width(), buffer.height(), numFrames);

    Bitmap buf0;
    buf0.createAs(buffer);
    buf0.blitFrom(buffer);
    auto& wa = workArea;

    KeyFrames kf;
    kf.addStopPoint(0, 0);
    kf.addStopPoint(0.2f, 0.2f);
    kf.addStopPoint(0.4f, -0.3f);
    kf.addStopPoint(0.45f, 0.5f);
    kf.addStopPoint(0.5f, -0.8f);
    kf.addStopPoint(0.55f, 1.0f);
    kf.addStopPoint(0.6f, -0.8f);
    kf.addStopPoint(0.65f, 0.3f);
    kf.addStopPoint(0.8f, -0.2f);
    kf.addStopPoint(1, 0);

    auto pinEnabled = SettingsData::instance().pinEnabled();

    UnlockGuardPainter painter(pinEnabled);
    painter.initialize(wa, buffer);

    for (int i = 0; i < numFrames; i++) {
        float val = static_cast<float>(i) / static_cast<float>(numFrames - 1);
        float v = kf.interpolate(val);
        float dx = 10 * v;

        painter.offsetText1X = dx;
        painter.drawTopText(line0, true); // cache
        painter.drawBottomText(line1);

        buf0.copyFrom(painter.buffer(), wa.left, wa.top);
        result.addFrame(i, buf0);

    }

    result.begin();
}

void UnlockGuardView::draw(Bitmap& buffer) {
    RECT wa = model_.workArea();

    auto pinEnabled = SettingsData::instance().pinEnabled();

    UnlockGuardPainter painter(pinEnabled);
    painter.initialize(wa, buffer);
    painter.drawTopText(model_.getTopText());
    painter.drawBottomText(model_.getBottomText());

    buffer.copyFrom(painter.buffer(), wa.left, wa.top);
    bgBuffer_.clone(buffer);
}

AnimationFrameSet& UnlockGuardView::createShowAnimation(Bitmap& buf) {
    UnlockGuardShowAnimation::create(showAnimation_, buf, model_.workArea(),
                                     model_.getTopText(), model_.getBottomText(), false);
    return showAnimation_;
}

AnimationFrameSet& UnlockGuardView::createHideAnimation(Bitmap& buf) {
    UnlockGuardShowAnimation::create(hideAnimation_, buf, model_.workArea(),
                                     model_.getTopText(), model_.getBottomText(), true);
    return hideAnimation_;
}

AnimationFrameSet& UnlockGuardView::createWrongAnimation(Bitmap& buf) {
    UnlockGuardWrongAnimation::create(wrongAnimation_, buf, model_.workArea(),
                                      model_.getTopText(), model_.getBottomText());
    return wrongAnimation_;
}

} // namespace litelockr
