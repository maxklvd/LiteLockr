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

#ifndef HOOK_OPTIONS_H
#define HOOK_OPTIONS_H

namespace litelockr {

struct HookOptions {
    bool lockKeyboard = false;
    bool lockMouse = false;
    bool unlockOnCtrlAltDel = false;
    bool processCaptionButtons = false;

    struct {
        unsigned char vkCode = 0;
        bool ctrl = false;
        bool alt = false;
        bool shift = false;
    } hotkey;
};

} // namespace litelockr

#endif // HOOK_OPTIONS_H
