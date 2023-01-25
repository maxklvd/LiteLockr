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

#include "AppSelection.h"

#include <cassert>
#include <cmath>

#include "gfx/BitmapContext.h"
#include "gui/WindowUtils.h"
#include "sys/AppTimer.h"
#include "sys/KeyFrames.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

namespace litelockr {

void AppSelection::drawCorners() {
    assert(!Rectangle::empty(currentAppRect_));

    int thickness = getLineThickness();
    int w = thickness * 8; // width
    int h = w; // height

    cornerLT_.buffer().create(w, h);
    cornerRT_.buffer().create(w, h);
    cornerRB_.buffer().create(w, h);
    cornerLB_.buffer().create(w, h);

    cornerLT_.setPosition({currentAppRect_.left, currentAppRect_.top});
    cornerRT_.setPosition({currentAppRect_.right - w, currentAppRect_.top});
    cornerRB_.setPosition({currentAppRect_.right - w, currentAppRect_.bottom - h});
    cornerLB_.setPosition({currentAppRect_.left, currentAppRect_.bottom - h});

    const int numFrames = 15;
    cornerLT_.frames_.recreate(w, h, numFrames);
    cornerRT_.frames_.recreate(w, h, numFrames);
    cornerRB_.frames_.recreate(w, h, numFrames);
    cornerLB_.frames_.recreate(w, h, numFrames);

    KeyFrames kf;
    kf.addStopPoint(0, 0.6f);
    kf.addStopPoint(0.7f, 1);
    kf.addStopPoint(0.85f, 0.4f);
    kf.addStopPoint(1, 0.9f);

    Color color(7, 77, 255);
    Bitmap buf;
    buf.create(w, h);
    for (int i = 0; i < numFrames; i++) {
        float val = static_cast<float>(i) / static_cast<float>(numFrames - 1);
        float v = kf.interpolate(val);
        auto size = std::lround(v * static_cast<float>(w));

        drawCorner(buf, 0, size, thickness, color);
        cornerLT_.frames_.addFrame(i, buf);

        drawCorner(buf, 90, size, thickness, color);
        cornerRT_.frames_.addFrame(i, buf);

        drawCorner(buf, 180, size, thickness, color);
        cornerRB_.frames_.addFrame(i, buf);

        drawCorner(buf, 270, size, thickness, color);
        cornerLB_.frames_.addFrame(i, buf);
    }

    cornerLT_.frames_.begin();
    cornerRT_.frames_.begin();
    cornerRB_.frames_.begin();
    cornerLB_.frames_.begin();
}

int AppSelection::getLineThickness() {
    return WindowUtils::getDpiScale() > 1 ? 6 : 4; // in pixels
}

void AppSelection::drawCorner(Bitmap& buf, int rotateAngle, int size, int thickness, Color color) {
    buf.clear({0, 0, 0, 0});
    buf.fillRect(0, 0, size, thickness, color);
    buf.fillRect(0, 0, thickness, size, color);

    int cx = static_cast<int>(buf.width()) / 2;
    int cy = static_cast<int>(buf.height()) / 2;

    BitmapContext ctx(buf);
    ctx.rotate(rotateAngle);
    ctx.translate(cx, cy);

    int offset = thickness / 2;
    int ox = cx - offset;
    int oy = cy - offset;
    ctx.moveTo(size - ox, -oy);
    ctx.lineTo(-ox, -oy);
    ctx.lineTo(-ox, size - oy);

    ctx.strokeColor = color;
    ctx.lineWidth = static_cast<float>(thickness);

    ctx.clear({0, 0, 0, 0});
    ctx.stroke();
}

void AppSelection::show(HWND currentWnd) {
    if (currentWnd && cornerLT_.isNull()) {
        if (!WindowUtils::getWindowRect(currentWnd, currentAppRect_)) {
            return;
        }
        if (Rectangle::empty(currentAppRect_)) {
            return;
        }

        currentAppRect_.left -= 2;
        currentAppRect_.top -= 2;
        currentAppRect_.right += 2;
        currentAppRect_.bottom += 2;

        drawCorners();

        DWORD style = WS_POPUP | WS_DISABLED;
        cornerLT_.style_ = style;
        cornerRT_.style_ = style;
        cornerRB_.style_ = style;
        cornerLB_.style_ = style;
        cornerLT_.createWindow(Process::mainWindow());
        cornerRT_.createWindow(Process::mainWindow());
        cornerRB_.createWindow(Process::mainWindow());
        cornerLB_.createWindow(Process::mainWindow());
        ShowWindow(cornerLT_.hWnd, SW_SHOWNA);
        ShowWindow(cornerRT_.hWnd, SW_SHOWNA);
        ShowWindow(cornerRB_.hWnd, SW_SHOWNA);
        ShowWindow(cornerLB_.hWnd, SW_SHOWNA);

        delay_.reset();
        AppTimer::setAnimationActiveFor(1000);
    }
}

void AppSelection::tick() {
    if (cornerLT_.hWnd) {
        cornerLT_.draw();
        cornerRT_.draw();
        cornerRB_.draw();
        cornerLB_.draw();

        cornerLT_.update();
        cornerRT_.update();
        cornerRB_.update();
        cornerLB_.update();

        if (delay_.timeout()) {
            cornerLT_.destroy();
            cornerRT_.destroy();
            cornerRB_.destroy();
            cornerLB_.destroy();
        }
    }
}

} // namespace litelockr
