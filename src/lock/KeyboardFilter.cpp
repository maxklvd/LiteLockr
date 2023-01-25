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

#include "KeyboardFilter.h"

#include <algorithm>
#include <cassert>

#include "app/User32.h"
#include "lock/HookData.h"
#include "log/Logger.h"
#include "sys/Process.h"

namespace litelockr {

std::optional<std::reference_wrapper<const HookOptions>> KeyboardFilter::options_{};
bool KeyboardFilter::keyPressed_[KEY_PRESSED_SIZE] = {};

ModifierKeys KeyboardFilter::modKey_;

void KeyboardFilter::install(const HookOptions& options) {
    options_ = options;
    std::ranges::fill(keyPressed_, false);
}

void KeyboardFilter::uninstall() {
    options_ = {};
}

bool KeyboardFilter::processKeyStroke(const KeyStroke& stroke) {
    if (!HookData::sessionActive.load(std::memory_order_relaxed)) {
        // no active session, nothing to block
        return true;
    }

    if (HookData::keyboard.sessionReconnected.load(std::memory_order_relaxed)) {
        HookData::keyboard.sessionReconnected.store(false, std::memory_order_relaxed);
        std::ranges::fill(keyPressed_, false);
    }

    bool isPressed = stroke.state == KeyStroke::KEY_DOWN;
    updateModKeys(stroke.code, isPressed);
    if (!isPressed && !keyPressed_[stroke.code]) {
        return true; // releases the key that was pressed before locking
    }
    keyPressed_[stroke.code] = isPressed;

    assert(options_.has_value());
    const HookOptions& options = options_.value();

    //
    // Ctrl + Alt + Del
    //
    if (options.unlockOnCtrlAltDel &&
        modKey_.ctrl && modKey_.alt && stroke.code == VK_DELETE) {
        HookData::ctrlAltDelPressedFlag.store(true, std::memory_order_relaxed);
        LOG_DEBUG(L"Ctrl+Alt+Del pressed");
    }

    if (options.lockKeyboard) {
        //
        // Windows key
        //
        if (stroke.code == VK_LWIN || stroke.code == VK_RWIN) {
            LOG_DEBUG(L"[BLOCK] Left/Right Windows key");
            return false;
        }

        //
        // Applications key
        //
        if (stroke.code == VK_APPS) {
            LOG_DEBUG(L"[BLOCK] Applications key");
            return false;
        }

        //
        // Alt + Tab
        //
        if (modKey_.alt && stroke.code == VK_TAB) {
            LOG_DEBUG(L"[BLOCK] Alt+Tab");
            return false;
        }

        //
        // Ctrl + Esc
        // Ctrl + Shift + Esc
        // Alt + Esc
        //
        if ((modKey_.ctrl || modKey_.alt) && stroke.code == VK_ESCAPE) {
            LOG_DEBUG(L"[BLOCK] Ctrl+Esc or Alt+Esc");
            return false;
        }
    } else {
        //
        // Keyboard lock is disabled
        //
        return true;
    }

    HWND hWnd = User32::getForegroundWindow();
    if (hWnd && HookData::windowValidator().isAllowed(hWnd)) {
        return true;
    }

    if (stroke.state == KeyStroke::KEY_DOWN &&
        options.hotkey.vkCode == stroke.code &&
        options.hotkey.ctrl == modKey_.ctrl &&
        options.hotkey.alt == modKey_.alt &&
        options.hotkey.shift == modKey_.shift) {
        HookData::hotkeyPressedFlag.store(true, std::memory_order_relaxed);
    }
    return false;
}

void KeyboardFilter::updateModKeys(unsigned key, bool isKeyDown) {
    assert(GetCurrentThreadId() == Process::hookThreadId());

    switch (key) {
        case VK_LCONTROL:
        case VK_RCONTROL:
            modKey_.ctrl = isKeyDown;
            break;
        case VK_LMENU:
        case VK_RMENU:
            modKey_.alt = isKeyDown;
            break;
        case VK_LSHIFT:
        case VK_RSHIFT:
            modKey_.shift = isKeyDown;
            break;
        default:
            // skip
            break;
    }
}

} // namespace litelockr
