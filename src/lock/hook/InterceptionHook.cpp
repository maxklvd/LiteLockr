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

#include "InterceptionHook.h"

#include "log/Logger.h"

namespace litelockr {

std::atomic<bool> InterceptionHook::interceptionAlive_{false};

void InterceptionHook::initialize(const HookOptions& options) {
    KeyboardFilter::install(options);
    MouseFilter::install(options);

    UINT code = MapVirtualKey(options.hotkey.vkCode, MAPVK_VK_TO_VSC_EX);
    hotkeyCode_.vkCode = options.hotkey.vkCode;
    hotkeyCode_.scanCode = code & 0xffu;
    hotkeyCode_.e0 = code & 0xff00u;
    LOG_VERBOSE(L"[Interception] hotkey: %s%s%svkCode=0x%x -> code=0x%x -> scanCode=0x%x e0=%d",
                options.hotkey.ctrl ? L"Ctrl " : L"",
                options.hotkey.alt ? L"Alt " : L"",
                options.hotkey.shift ? L"Shift " : L"",
                options.hotkey.vkCode, code, hotkeyCode_.scanCode, hotkeyCode_.e0);

    DisplayMonitors::get(monitorInfo_);

    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    initializeLastInputTime();
}

void InterceptionHook::dispose() {
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

    KeyboardFilter::uninstall();
    MouseFilter::uninstall();
}

bool InterceptionHook::isDriverInstalled() {
    static int installed = -1;
    if (installed < 0) {
        InterceptionContext context = interception_create_context();
        interception_destroy_context(context);
        installed = context ? 1 : 0;
    }
    return installed > 0;
}

void InterceptionHook::start() {
    GetCursorPos(&position_);

    LOG_DEBUG(L"[Interception] Current cursor position: X,Y = {%d, %d}", position_.x, position_.y);

    class InterceptionContextWrapper {
    public:
        InterceptionContextWrapper() {
            context = interception_create_context();
            LOG_DEBUG(L"[Interception] Interception context has created");
        }

        ~InterceptionContextWrapper() {
            interception_destroy_context(context);
            context = nullptr;
            LOG_DEBUG(L"[Interception] Interception context has destroyed");
        }

        InterceptionContext context = nullptr;
    } wrapper;

    interception_set_filter(wrapper.context, interception_is_keyboard,
                            INTERCEPTION_FILTER_KEY_ALL);
    interception_set_filter(wrapper.context, interception_is_mouse,
                            INTERCEPTION_FILTER_MOUSE_ALL);

    interceptionAlive_.store(true);

    while (interceptionAlive_.load(std::memory_order_relaxed)) {
        InterceptionDevice device = interception_wait_with_timeout(wrapper.context, 500); // 500ms

        InterceptionStroke stroke;
        if (interception_receive(wrapper.context, device, &stroke, 1) > 0) {
            if (interception_is_mouse(device)) {
                updateLastInputTime();

                auto& msStroke = reinterpret_cast<InterceptionMouseStroke&>(stroke);

                MouseStroke ms = getMouseStroke(msStroke);
                LOG_VERBOSE(L"[Interception] X,Y = {%d, %d}, state: 0x%x, flags: 0x%x, information: 0x%x",
                            ms.pt.x, ms.pt.y, msStroke.state, msStroke.flags, msStroke.information);

                if (MouseFilter::processMouseStroke(ms)) {
                    position_ = ms.pt;
                    msStroke.flags = INTERCEPTION_MOUSE_MOVE_ABSOLUTE;
                    int width = monitorInfo_.primaryMonitor.cx;
                    int height = monitorInfo_.primaryMonitor.cy;
                    msStroke.x = static_cast<int>((0xFFFF * position_.x) / width);
                    msStroke.y = static_cast<int>((0xFFFF * position_.y) / height);
                    interception_send(wrapper.context, device, &stroke, 1);
                }
            }
            if (interception_is_keyboard(device)) {
                updateLastInputTime();

                auto const& kbdStroke = reinterpret_cast<InterceptionKeyStroke&>(stroke);

                KeyStroke ks = getKeyStroke(kbdStroke);
                LOG_VERBOSE(L"[Interception] key event, state: %s (0x%x), information: 0x%x",
                            (ks.state == KeyStroke::KEY_DOWN ? L"KeyDown" : (ks.state == KeyStroke::KEY_UP ? L"KeyUp"
                                                                                                           : L"Other")),
                            kbdStroke.state, kbdStroke.information);

                if (KeyboardFilter::processKeyStroke(ks)) {
                    interception_send(wrapper.context, device, &stroke, 1);
                }
            }
        }

        if (checkDisplayChanged()) {
            LOG_DEBUG(L"[Interception] The display resolution was changed");
            DisplayMonitors::get(monitorInfo_);
        }
    }
}

void InterceptionHook::stop() {
    interceptionAlive_.store(false, std::memory_order_relaxed);
}

MouseStroke InterceptionHook::getMouseStroke(const InterceptionMouseStroke& mstroke) const {
    POINT pos{mstroke.x, mstroke.y};
    MouseStroke ms{};

    if (mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE) {
        int width = monitorInfo_.primaryMonitor.cx;
        int height = monitorInfo_.primaryMonitor.cy;
        ms.pt.x = static_cast<int>((pos.x * width) / 0xFFFF);
        ms.pt.y = static_cast<int>((pos.y * height) / 0xFFFF);
        LOG_VERBOSE(L"[Interception] [ABSOLUTE] X,Y = {%d, %d}", ms.pt.x, ms.pt.y);
    } else {
        position_.x += pos.x;
        position_.y += pos.y;
        ms.pt = position_;
        LOG_VERBOSE(L"[Interception] [RELATIVE] X,Y = {%d, %d}  dx,dy = {%d, %d}", position_.x, position_.y, pos.x,
                    pos.y);
    }

    switch (mstroke.state) {
        case INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN:
            ms.state = MouseStroke::LEFT_BUTTON_DOWN;
            break;
        case INTERCEPTION_MOUSE_LEFT_BUTTON_UP:
            ms.state = MouseStroke::LEFT_BUTTON_UP;
            break;
        case INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN:
            ms.state = MouseStroke::RIGHT_BUTTON_DOWN;
            break;
        case INTERCEPTION_MOUSE_RIGHT_BUTTON_UP:
            ms.state = MouseStroke::RIGHT_BUTTON_UP;
            break;
        case 0:
            ms.state = MouseStroke::MOUSE_MOVE;
            break;
        default:
            break;
    }
    return ms;
}

namespace ScanCode {
enum {
    Esc = 0x01,
    Tab = 0x0f,
    Ctrl = 0x1d,
    Alt = 0x38,
    LShift = 0x2a,
    RShift = 0x36,
    LWin = 0x5b,
    RWin = 0x5c,
    Apps = 0x5d,
    Home = 0x47,
    Up = 0x48,
    PgUp = 0x49,
    Left = 0x4b,
    Right = 0x4d,
    End = 0x4f,
    Down = 0x50,
    PgDn = 0x51,
    Insert = 0x52,
    Delete = 0x53,
};
}

KeyStroke InterceptionHook::getKeyStroke(const InterceptionKeyStroke& kstroke) const {
    KeyStroke ks{};
    ks.state = kstroke.state & INTERCEPTION_KEY_UP ? KeyStroke::KEY_UP : KeyStroke::KEY_DOWN;
    ks.code = getVirtualKey(kstroke.code, kstroke.state & INTERCEPTION_KEY_E0);
    return ks;
}

unsigned short InterceptionHook::getVirtualKey(unsigned short scanCode, bool e0) const {
    if (e0) {
        switch (scanCode) {
            case ScanCode::Ctrl:
                return VK_RCONTROL; // e0 1d (RCtrl)
            case ScanCode::Alt:
                return VK_RMENU;    // e0 38 (RAlt)
            case ScanCode::LWin:
                return VK_LWIN;     // e0 5b (LeftWindow)
            case ScanCode::RWin:
                return VK_RWIN;     // e0 5c (RightWindow)
            case ScanCode::Apps:
                return VK_APPS;     // e0 5d (Menu)
            case ScanCode::Home:
                return VK_HOME;     // e0 47 (Grey Home)
            case ScanCode::Up:
                return VK_UP;       // e0 48 (Grey Up)
            case ScanCode::PgUp:
                return VK_PRIOR;    // e0 49 (Grey PgUp)
            case ScanCode::Left:
                return VK_LEFT;     // e0 4b (Grey Left)
            case ScanCode::Right:
                return VK_RIGHT;    // e0 4d (Grey Right)
            case ScanCode::End:
                return VK_END;      // e0 4f (Grey End)
            case ScanCode::Down:
                return VK_DOWN;     // e0 50 (Grey Down)
            case ScanCode::PgDn:
                return VK_NEXT;     // e0 51 (Grey PgDn)
            case ScanCode::Insert:
                return VK_INSERT;   // e0 52 (Grey Insert)
            case ScanCode::Delete:
                return VK_DELETE;   // e0 53 (Grey Delete)
            default:
                break;
        }
    } else {
        switch (scanCode) {
            case ScanCode::Esc:
                return VK_ESCAPE;   // 01 (Esc)
            case ScanCode::Tab:
                return VK_TAB;      // 0f (Tab)
            case ScanCode::Ctrl:
                return VK_LCONTROL; // 1d (LCtrl)
            case ScanCode::Alt:
                return VK_LMENU;    // 38 (LAlt)
            case ScanCode::LShift:
                return VK_LSHIFT;   // 2a (LShift)
            case ScanCode::RShift:
                return VK_RSHIFT;   // 36 (RShift)
            case ScanCode::Home:
                return VK_HOME;     // 47 (Keypad-7/Home)
            case ScanCode::Up:
                return VK_UP;       // 48 (Keypad-8/UpArrow)
            case ScanCode::PgUp:
                return VK_PRIOR;    // 49 (Keypad-9/PgUp)
            case ScanCode::Left:
                return VK_LEFT;     // 4b (Keypad-4/Left)
            case ScanCode::Right:
                return VK_RIGHT;    // 4d (Keypad-6/Right)
            case ScanCode::End:
                return VK_END;      // 4f (Keypad-1/End)
            case ScanCode::Down:
                return VK_DOWN;     // 50 (Keypad-2/DownArrow)
            case ScanCode::PgDn:
                return VK_NEXT;     // 51 (Keypad-3/PgDn)
            case ScanCode::Insert:
                return VK_INSERT;   // 52 (Keypad-0/Ins)
            case ScanCode::Delete:
                return VK_DELETE;   // 53 (Keypad-./Del)
            default:
                break;
        }
    }

    if (hotkeyCode_.scanCode == scanCode && hotkeyCode_.e0 == e0) {
        return hotkeyCode_.vkCode;
    }
    return 0;
}

} // namespace litelockr
