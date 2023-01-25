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

#ifndef APP_SELECTION_H
#define APP_SELECTION_H

#include "gui/LayeredWindow.h"
#include "lock/ui/FrameSetWnd.h"
#include "sys/AppClock.h"

namespace litelockr {

class AppSelection {
public:
    AppSelection() = default;
    AppSelection(const AppSelection&) = delete;
    AppSelection& operator=(const AppSelection&) = delete;

    void show(HWND currentWnd);

    void tick();

private:
    TimeCounter delay_{std::chrono::seconds(1)};
    RECT currentAppRect_{};

    FrameSetWnd cornerLT_;
    FrameSetWnd cornerRT_;
    FrameSetWnd cornerRB_;
    FrameSetWnd cornerLB_;

    static void drawCorner(Bitmap& buf, int rotateAngle, int size, int thickness, Color color);
    void drawCorners();
    static int getLineThickness();
};

} // namespace litelockr

#endif // APP_SELECTION_H
