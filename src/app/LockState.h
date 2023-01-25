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

#ifndef LOCK_STATE_H
#define LOCK_STATE_H

namespace litelockr {

struct LockState {
    enum LockStateId {
        NONE = 0,           // uninitialized state, zero at start up
        DISABLED = 1,
        PRE_READY = 2,      // unlock effect is in progress
        READY = 3,
        PRE_COUNTDOWN = 4,  // start locking effect is in progress
        COUNTDOWN = 5,
        PRE_LOCKED = 6,     // finish lock effect is in progress
        LOCKED = 7
    };

    static bool isNewStateCorrect(int oldState, int newState);
};

enum class LockIcon {
    Unlocked,
    Locked,
    LockNotAvailable,
};

} // namespace litelockr

#endif // LOCK_STATE_H
