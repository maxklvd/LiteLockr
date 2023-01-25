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

#include "InputLocker.h"

#include "app/User32.h"
#include "app/event/AppEvent.h"
#include "gui/WindowUtils.h"
#include "ini/SettingsData.h"
#include "lock/HookData.h"
#include "lock/HookThread.h"
#include "lock/MouseFilter.h"
#include "lock/hook/LockStateChecker.h"
#include "log/Logger.h"
#include "sys/Process.h"

namespace litelockr {

void InputLocker::tick() {
    if (HookThread::isLocked()) {
        MouseFilter::tick();

        if (HookData::hotkeyPressedFlag.load(std::memory_order_relaxed)) {
            HookData::hotkeyPressedFlag.store(false, std::memory_order_relaxed);

            if (!isHotkeyRepeated()) {
                updateHotkeyTime();

                AppEvent event(AppEvent::UNLOCK, AppEvent::SRC_HOTKEY);
                AppEvents::send(event);
            }
        } else if (HookData::ctrlAltDelPressedFlag.load(std::memory_order_relaxed)) {
            HookData::ctrlAltDelPressedFlag.store(false, std::memory_order_relaxed);

            LOG_DEBUG(L"Ctrl+Alt+Del pressed. Sending UNLOCK event.");
            AppEvent event(AppEvent::UNLOCK, AppEvent::SRC_HOTKEY);
            AppEvents::send(event);
        }
    }
}

void InputLocker::lock() {
    SettingsData& settings = SettingsData::instance();

    Process::setActiveWindow(getActiveWindow());
    if (Log::enabled(Log::Severity::Warning) && WindowUtils::isAutoHideTaskbar()) {
        LOG_WARNING(L"The taskbar with auto-hide option enabled is not supported");
    }

    if (settings.lockKeyboard.value() || settings.lockMouse.value()) {
        settings.save(nullptr);
        HookThread::startLocking();
    }

    LockStateChecker::reset();
}

void InputLocker::unlock() {
    HookThread::stopLocking();
    Process::setActiveWindow(nullptr);
}

bool InputLocker::isHotkeyRepeated() const {
    return (AppClock::now() - lastHotkeyPressed_) < std::chrono::microseconds(HOTKEY_REPEAT_TIME);
}

void InputLocker::updateHotkeyTime() {
    lastHotkeyPressed_ = AppClock::now();
}

HWND InputLocker::getActiveWindow() {
    if (SettingsData::instance().dontLockCurrentApp.value()) {
        return User32::getForegroundWindow();
    }
    return nullptr;
}

} // namespace litelockr
