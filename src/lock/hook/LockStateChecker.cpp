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

#include "LockStateChecker.h"

#include "app/User32.h"
#include "lock/HookData.h"
#include "lock/HookThread.h"
#include "lock/hook/AbstractHook.h"
#include "sys/Time.h"

namespace litelockr {

bool LockStateChecker::lockAvailable_ = true;
TimeCounter LockStateChecker::interval_{std::chrono::milliseconds(200)};
TimeCounter LockStateChecker::waitForHookLastInputTime_{std::chrono::milliseconds(300)};

int LockStateChecker::passCounter_ = 0;
int LockStateChecker::failCounter_ = 0;

unsigned long LockStateChecker::systemLastInputTime_ = 0;
bool LockStateChecker::compareTimes_ = false;

bool LockStateChecker::lockAvailable() {
    return lockAvailable_;
}

void LockStateChecker::reset() {
    lockAvailable_ = true;
    passCounter_ = 0;
    failCounter_ = 0;
    compareTimes_ = false;
}

bool LockStateChecker::update() {
    if (HookThread::isLocked() &&           // the lock is enabled
        HookData::isSessionActive() &&      // the user is logged on
        User32::getForegroundWindow()) {    // some foreground window is exist

        if (interval_.timeout()) {
            if (compareTimes_) {
                if (waitForHookLastInputTime_.timeout()) {
                    compareTimes_ = false;

                    auto difference = systemLastInputTime_ - AbstractHook::lastInputTime();
                    if (static_cast<long>(difference) > 2000) { // 2 sec.
                        failCounter_++;
                        passCounter_ = 0;
                    } else {
                        passCounter_++;
                        failCounter_ = 0;
                    }

                    if (passCounter_ >= NUMBER_OF_TESTS) {
                        reset();
                        lockAvailable_ = true;
                        return true;
                    } else if (failCounter_ >= NUMBER_OF_TESTS) {
                        reset();
                        lockAvailable_ = false;
                        return true;
                    }
                }
            } else {
                unsigned long lastInputTime;
                if (Time::getLastInputTime(lastInputTime)) {
                    if (systemLastInputTime_ != lastInputTime) {
                        systemLastInputTime_ = lastInputTime;
                        compareTimes_ = true;
                        waitForHookLastInputTime_.reset();
                    }
                }
            }
        }
    } else {
        reset();
    }
    return false;
}

} // namespace litelockr
