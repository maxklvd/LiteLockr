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

#include "HookData.h"

#include <cassert>

#include "sys/Process.h"

#include "res/Resources.h"

namespace litelockr {

std::atomic<bool> HookData::hotkeyPressedFlag{false};
std::atomic<bool> HookData::ctrlAltDelPressedFlag{false};
std::atomic<bool> HookData::sessionActive{true};
HookData::FilterFlags HookData::mouse{};
HookData::FilterFlags HookData::keyboard{};

void HookData::setSessionActive(bool isActive) {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    bool state = sessionActive.load(std::memory_order_relaxed);
    if (state != isActive) {
        sessionActive.store(isActive, std::memory_order_relaxed);

        if (isActive) {
            keyboard.sessionReconnected.store(true, std::memory_order_relaxed);
            mouse.sessionReconnected.store(true, std::memory_order_relaxed);
        }
    }
}

bool HookData::isSessionActive() {
    return sessionActive.load(std::memory_order_relaxed);
}

} // namespace litelockr
