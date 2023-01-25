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

#include "FlyoutModel.h"

#include <cassert>

#include "ren/PlanesGeometry.h"
#include "sys/Rectangle.h"

namespace litelockr {

void FlyoutModel::initialize() {
    close.id = CLOSE;
    menu.id = MENU;
    lockButton.id = LOCK;
    settings.id = OPT_SETTINGS;
    help.id = OPT_HELP;
    about.id = OPT_ABOUT;
    keyboard.id = KEYBOARD;
    mouse.id = MOUSE;

    titleBar.w = 138;
    titleBar.h = 23;
    close.w = 22;
    close.h = 23;
    menu.w = 26;
    menu.h = 23;

    lockButton.w = 44;
    lockButton.h = 55;
    body.w = 138;
    body.h = 132;
    menuBar.visible = false;
    menuBar.w = 40;
    menuBar.h = body.h;

    settings.w = 40;
    settings.h = 38;
    help.w = 40;
    help.h = 38;
    about.w = 40;
    about.h = 37;

    circle.w = circle.h = 101;
    circle.offset3.x = 0.5f;
    circle.offset3.y = 0;
    circle.offset3.z = -3;
    circle.x = 39;
    circle.y = 49;

    progressBackground.w = progressBackground.h = 101;
    progressBackground.offset3.x = 0.5f;
    progressBackground.offset3.y = 0;
    progressBackground.offset3.z = -3;
    progressBackground.x = MARGIN + 19;
    progressBackground.y = MARGIN + 29;

    progress = progressBackground;

    lock.w = lock.h = 81;
    lock.offset3.x = 0.5f;
    lock.offset3.y = 0;
    lock.offset3.z = -2.6f;

    keyboard.w = keyboard.h = 26;
    mouse.w = mouse.h = 26;
    keyboard.offset3.x = mouse.offset3.x = 0.5f;
    keyboard.offset3.y = mouse.offset3.y = -0.3f;
    keyboard.offset3.z = mouse.offset3.z = -3;

    lockButton.r = 44;
    keyboard.r = mouse.r = 13;

    lockButton.useView3 = true;
    lockButton.view3.x = MARGIN + 88;
    lockButton.view3.y = MARGIN + 48;
    lockButton.view3.w = 37;
    lockButton.view3.h = 53;

    pack();
}

void FlyoutModel::pack() {
    unsigned w = body.w + MARGIN * 2;
    unsigned h = titleBar.h + body.h + MARGIN * 2 - PlanesGeometry::Y_OFFSET;

    size_.cx = w;
    size_.cy = h;

    titleBar.x = MARGIN;
    titleBar.y = MARGIN;
    close.x = titleBar.x + titleBar.w - close.w;
    close.y = titleBar.y;
    menu.x = titleBar.x;
    menu.y = titleBar.y;

    body.x = MARGIN;
    body.y = MARGIN + titleBar.h;
    menuBar.x = body.x;
    menuBar.y = body.y;
    lockButton.x = body.x + body.w / 2 - lockButton.w / 2;
    lockButton.y = body.y + body.h / 2 - lockButton.h / 2 - 13;

    settings.x = menuBar.x;
    help.x = menuBar.x;
    about.x = menuBar.x;

    settings.y = menuBar.y;
    help.y = settings.y + settings.h;
    about.y = help.y + help.h;

    keyboard.x = MARGIN + 6;
    mouse.x = MARGIN + 106;
    keyboard.y = mouse.y = MARGIN + 104;

    lock.x = MARGIN + 29;
    lock.y = MARGIN + 39;

    guard.setWorkArea(workArea());
}

RECT FlyoutModel::workArea() const {
    return Rectangle::create(MARGIN, MARGIN, body.w, titleBar.h + body.h - PlanesGeometry::Y_OFFSET);
}

int FlyoutModel::lockState() const {
    assert(lockState_ > 0);
    return lockState_;
}

} // namespace litelockr
