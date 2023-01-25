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

#include "LockState.h"

#include <cassert>

namespace litelockr {

bool LockState::isNewStateCorrect(int oldState, int newState) {
    assert(newState == DISABLED
           || newState == PRE_READY
           || newState == READY
           || newState == PRE_COUNTDOWN
           || newState == COUNTDOWN
           || newState == PRE_LOCKED
           || newState == LOCKED);

    if (oldState == newState) {
        return true;
    }

    bool correct = false;
    switch (newState) {
        case DISABLED:
            correct = (oldState == READY
                       || oldState == COUNTDOWN
                       || oldState == NONE);
            assert(correct);
            break;
        case PRE_READY:
            correct = (oldState == LOCKED);
            assert(correct);
            break;
        case READY:
            correct = (oldState == PRE_READY
                       || oldState == DISABLED
                       || oldState == COUNTDOWN
                       || oldState == NONE);
            assert(correct);
            break;
        case PRE_COUNTDOWN:
            correct = (oldState == READY);
            assert(correct);
            break;
        case COUNTDOWN:
            correct = (oldState == PRE_COUNTDOWN
                       || oldState == READY);
            assert(correct);
            break;
        case PRE_LOCKED:
            correct = (oldState == READY
                       || oldState == COUNTDOWN);
            assert(correct);
            break;
        case LOCKED:
            correct = (oldState == PRE_LOCKED);
            assert(correct);
            break;
        default:
            assert(false);
            break;
    }
    return correct;
}

} // namespace litelockr
