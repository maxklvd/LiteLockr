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

#ifndef MOUSE_STROKE_H
#define MOUSE_STROKE_H

#include <windows.h>

namespace litelockr {

struct MouseStroke {
    enum State {
        LEFT_BUTTON_DOWN = 1,
        LEFT_BUTTON_UP,
        RIGHT_BUTTON_DOWN,
        RIGHT_BUTTON_UP,
        MOUSE_MOVE,
    };

    POINT pt{};              // Mouse cursor's screen coordinates
    unsigned int state = 0;  // State
};

} // namespace litelockr

#endif // MOUSE_STROKE_H
