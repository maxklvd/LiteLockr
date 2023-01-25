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

#ifndef FRAME_SET_WND_H
#define FRAME_SET_WND_H

#include "gfx/FrameSet.h"
#include "gui/LayeredWindow.h"

namespace litelockr {

class FrameSetWnd: public LayeredWindow {
public:
    virtual ~FrameSetWnd() = default;

    Bitmap& buffer() { return buffer_; }

    void draw();

    void update() {
        updateWindow();
    }

    void destroy();

private:
    FrameSet frames_;

    friend class AppSelection;
};

} // namespace litelockr

#endif // FRAME_SET_WND_H
