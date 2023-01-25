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

#ifndef LOCK_AREA_H
#define LOCK_AREA_H

#include <vector>

#include <windows.h>

namespace litelockr {

struct LockArea {
    enum LockAreaId {
        NONE = -1,
        DENY = 0,
        ALLOW = 1,
        TASKBAR_BUTTON = 2,
        NOTIFICATION_AREA_ICON = 3,
    };

    int id = NONE;
    bool allowed = true;
    bool workArea = false;
    RECT rc{};
};

using LockAreaVec = std::vector<LockArea>;

} // namespace litelockr

#endif // LOCK_AREA_H
