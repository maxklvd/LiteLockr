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

#ifndef LOCK_STATE_CHECKER_H
#define LOCK_STATE_CHECKER_H

#include "sys/AppClock.h"

namespace litelockr {

class LockStateChecker {
public:
    static bool lockAvailable();
    static void reset();
    static bool update();

private:
    static bool lockAvailable_;

    static TimeCounter interval_;
    static TimeCounter waitForHookLastInputTime_;

    constexpr static int NUMBER_OF_TESTS = 5;
    static int passCounter_;
    static int failCounter_;

    static unsigned long systemLastInputTime_;
    static bool compareTimes_;
};

} // namespace litelockr

#endif //LOCK_STATE_CHECKER_H
