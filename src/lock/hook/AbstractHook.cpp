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

#include "AbstractHook.h"

#include "log/Logger.h"
#include "sys/Time.h"

namespace litelockr {

std::atomic<bool> AbstractHook::displayChanged_{false};
std::atomic<unsigned long> AbstractHook::lastInputTime_{0};

bool AbstractHook::checkDisplayChanged() {
    if (displayChanged_.load(std::memory_order_relaxed)) {
        displayChanged_.store(false, std::memory_order_relaxed);
        return true;
    }
    return false;
}

void AbstractHook::setDisplayChanged(bool changed) {
    displayChanged_.store(changed, std::memory_order_relaxed);
}

unsigned long AbstractHook::lastInputTime() {
    return lastInputTime_.load(std::memory_order_relaxed);
}

void AbstractHook::initializeLastInputTime() {
    unsigned long time = 0;
    Time::getLastInputTime(time);
    lastInputTime_.store(time, std::memory_order_relaxed);
    LOG_VERBOSE(L"[AbstractHook] The last input time initialized with value %d", time);

}

void AbstractHook::updateLastInputTime() {
    auto t = Time::getTime();
    lastInputTime_.store(t, std::memory_order_relaxed);
    LOG_VERBOSE(L"[AbstractHook] The last input time updated. New value is %d", t);
}

} // namespace litelockr
