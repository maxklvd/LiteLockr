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

#include "LowLevelWindowsHook.h"

#include <cassert>

#include "app/User32.h"
#include "lock/KeyboardFilter.h"
#include "lock/MouseFilter.h"
#include "log/Logger.h"
#include "sys/Process.h"

#include "res/Resources.h"

namespace litelockr {

HHOOK LowLevelWindowsHook::hKeyboardHook_{nullptr};
HHOOK LowLevelWindowsHook::hMouseHook_{nullptr};

void LowLevelWindowsHook::initialize(const HookOptions& options) {
    //
    // install hooks for keyboard and mouse
    //
    KeyboardFilter::install(options);

    assert(hKeyboardHook_ == nullptr);
    hKeyboardHook_ = User32::setWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProc, GetModuleHandle(nullptr), 0);
    assert(hKeyboardHook_);
    LOG_DEBUG(L"[KEYBOARD_LL] The keyboard hook is set: %d", hKeyboardHook_);
    LOG_VERBOSE(L"[KEYBOARD_LL] Hotkey: %s%s%svkCode=0x%x",
                options.hotkey.ctrl ? L"Ctrl " : L"",
                options.hotkey.alt ? L"Alt " : L"",
                options.hotkey.shift ? L"Shift " : L"",
                options.hotkey.vkCode);

    MouseFilter::install(options);

    assert(hMouseHook_ == nullptr);
    hMouseHook_ = User32::setWindowsHookEx(WH_MOUSE_LL, mouseHookProc, GetModuleHandle(nullptr), 0);
    assert(hMouseHook_);
    LOG_DEBUG(L"[MOUSE_LL] The mouse hook is set: %d", hMouseHook_);

    initializeLastInputTime();
}

void LowLevelWindowsHook::dispose() {
    //
    // uninstall the hooks
    //
    if (hKeyboardHook_) {
        User32::unhookWindowsHookEx(hKeyboardHook_);
        hKeyboardHook_ = nullptr;
    }

    if (hMouseHook_) {
        User32::unhookWindowsHookEx(hMouseHook_);
        hMouseHook_ = nullptr;
    }

    KeyboardFilter::uninstall();
    MouseFilter::uninstall();
}

void LowLevelWindowsHook::start() {
    //
    // message loop
    //
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void LowLevelWindowsHook::stop() {
    assert(Process::hookThreadId());
    PostThreadMessage(Process::hookThreadId(), WM_QUIT, 0, 0);
}

LRESULT CALLBACK LowLevelWindowsHook::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    assert(GetCurrentThreadId() == Process::hookThreadId());

    if (nCode != HC_ACTION) {
        return User32::callNextHookEx(hKeyboardHook_, nCode, wParam, lParam);
    }

    updateLastInputTime();

    const auto data = reinterpret_cast<const KBDLLHOOKSTRUCT *>(lParam);

    KeyStroke stroke{.code = data->vkCode};

    switch (wParam) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            stroke.state = KeyStroke::KEY_DOWN;
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            stroke.state = KeyStroke::KEY_UP;
            break;
        default:
            stroke.state = 0;
            break;
    }

    LOG_VERBOSE(L"[KEYBOARD_LL] key event, state: %s (0x%x), flags: 0x%x",
                (stroke.state == KeyStroke::KEY_DOWN ? L"KeyDown" : (stroke.state == KeyStroke::KEY_UP ? L"KeyUp"
                                                                                                       : L"Other")),
                wParam, data->flags);

    if (KeyboardFilter::processKeyStroke(stroke)) {
        return User32::callNextHookEx(hKeyboardHook_, nCode, wParam, lParam);
    } else {
        return 1; // block the keystroke
    }
}

LRESULT CALLBACK LowLevelWindowsHook::mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    assert(GetCurrentThreadId() == Process::hookThreadId());

    if (nCode != HC_ACTION) {
        return User32::callNextHookEx(hMouseHook_, nCode, wParam, lParam);
    }

    updateLastInputTime();

    const auto data = reinterpret_cast<const MSLLHOOKSTRUCT *>(lParam);

    MouseStroke stroke{.pt=data->pt};
    LOG_VERBOSE(L"[MOUSE_LL] X,Y = {%d, %d}, state: 0x%x, flags: 0x%x", stroke.pt.x, stroke.pt.y, wParam, data->flags);

    switch (wParam) {
        case WM_LBUTTONDOWN:
            stroke.state = MouseStroke::LEFT_BUTTON_DOWN;
            break;
        case WM_LBUTTONUP:
            stroke.state = MouseStroke::LEFT_BUTTON_UP;
            break;
        case WM_RBUTTONDOWN:
            stroke.state = MouseStroke::RIGHT_BUTTON_DOWN;
            break;
        case WM_RBUTTONUP:
            stroke.state = MouseStroke::RIGHT_BUTTON_UP;
            break;
        case WM_MOUSEMOVE:
            stroke.state = MouseStroke::MOUSE_MOVE;
            break;
        default:
            stroke.state = 0;
            break;
    }

    if (MouseFilter::processMouseStroke(stroke)) {
        return User32::callNextHookEx(hMouseHook_, nCode, wParam, lParam);
    }
    return 1; // block the mouse event
}

} // namespace litelockr
