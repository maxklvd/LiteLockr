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

#ifndef KEY_STROKE_H
#define KEY_STROKE_H

namespace litelockr {

struct KeyStroke {
    enum State {
        KEY_DOWN = 1,
        KEY_UP = 2,
    };

    unsigned int code = 0;   // Virtual Key
    unsigned int state = 0;  // State
};

} // namespace litelockr

#endif // KEY_STROKE_H
