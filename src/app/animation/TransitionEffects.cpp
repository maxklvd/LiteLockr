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

#include "TransitionEffects.h"

#include <cmath>
#include <omp.h>
#include <gfx/BitmapContext.h>

#include "gfx/BitmapUtils.h"
#include "gui/WindowUtils.h"
#include "sys/Rectangle.h"
#include "sys/KeyFrames.h"

namespace litelockr {

void ShowWindowAnimation::create(AnimationFrameSet& result, const Bitmap& img, RECT workArea, bool inverted) {
    const int numFrames = inverted ? 18 : 16;
    const int width = static_cast<int>(img.width());
    const int height = static_cast<int>(img.height());
    result.recreate(width, height, numFrames);

    RECT rcMargin;
    int taskbarPosition = WindowUtils::getTaskbarPosition();
    switch (taskbarPosition) {
        case ABE_TOP:
            rcMargin = {0, 0, width, workArea.top}; // left, top, right, bottom
            break;
        case ABE_LEFT:
            rcMargin = {0, 0, workArea.left, height};
            break;
        case ABE_RIGHT:
            rcMargin = {workArea.right, 0, width, height};
            break;
        default:
            // BOTTOM
            rcMargin = {0, workArea.bottom, width, height};
            break;
    }

#pragma omp parallel
    {
        Bitmap frame;
        frame.create(width, height);

#pragma omp for ordered schedule(dynamic)
        for (int i = 0; i < numFrames; i++) {
            frame.clear({0, 0, 0, 0});

            int frameIdx = inverted ? numFrames - i - 1 : i;
            createFrame(frame, img, taskbarPosition, frameIdx, numFrames);
            frame.fillRect(rcMargin, {0, 0, 0, 0}); // the margin area should remain transparent

#pragma omp ordered
            result.addFrame(i, frame);
        }
    }

    result.begin();
}

void
ShowWindowAnimation::createFrame(Bitmap& dst, const Bitmap& src, int taskbarPosition, int index, int len) noexcept {
    float v = static_cast<float>(index) / static_cast<float>(len - 1);
    float value = EASY_IN(v);
    const int N = 2;
    float vOpacity = index < N ? 0 : static_cast<float>(index - N) / static_cast<float>(len - 1 - N);
    auto opacity = static_cast<std::uint8_t>(255.0f * (1.0f - EASY_IN(vOpacity)));

    BitmapContext ctx(dst);

    switch (taskbarPosition) {
        case ABE_LEFT:
            ctx.wave<WaveDirection::MoveLeft>(src, value, opacity);
            break;
        case ABE_TOP:
            ctx.wave<WaveDirection::MoveUp>(src, value, opacity);
            break;
        case ABE_RIGHT:
            ctx.wave<WaveDirection::MoveRight>(src, value, opacity);
            break;
        case ABE_BOTTOM:
            ctx.wave<WaveDirection::MoveDown>(src, value, opacity);
            break;
        default:
            break;
    }
}

void ShakeAnimation::create(AnimationFrameSet& result, const Bitmap& img, RECT workArea) {
    const int numFrames = 30;
    const int maxOffset = 7; // in pixels

    //
    // moves the image back and forth
    //
    KeyFrames kf;
    kf.addStopPoint(0, -0.5f);
    kf.addStopPoint(0.3f, 0.5f);
    kf.addStopPoint(0.45f, -0.6f);
    kf.addStopPoint(0.5f, 0.5f);
    kf.addStopPoint(0.55f, -0.6f);
    kf.addStopPoint(0.6f, 0.6f);
    kf.addStopPoint(0.65f, -0.6f);
    kf.addStopPoint(0.7f, 0.6f);
    kf.addStopPoint(1, 0);

    const unsigned width = img.width();
    const unsigned height = img.height();
    result.recreate(width, height, numFrames);

    Bitmap frame;
    frame.create(width, height);

    for (int i = 0; i < numFrames; i++) {
        float val = static_cast<float>(i) / static_cast<float>(numFrames - 1);
        int offsetX = std::lround(kf.interpolate(val) * maxOffset);

        RECT rc = workArea;
        Rectangle::offset(rc, offsetX, 0);
        assert(rc.left >= 0);

        frame.clear({0, 0, 0, 0});
        frame.copyFrom(img, rc.left, rc.top, workArea.left, workArea.top,
                       Rectangle::width(rc), Rectangle::height(rc));

        result.addFrame(i, frame);
    }

    result.begin();
}

} // namespace litelockr

