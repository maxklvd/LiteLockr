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

#include "WindowUtils.h"

#include <cassert>

#include <dwmapi.h>
#include "gui/Window.h"
#include "log/Logger.h"
#include "lock/DisplayMonitors.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

namespace litelockr {

bool WindowUtils::getWindowRect(HWND hWnd, RECT& rc) {
    HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rc, sizeof(RECT));
    if (hr == S_OK) {
        return true;
    }
    return GetWindowRect(hWnd, &rc) != 0;
}

std::wstring WindowUtils::getWindowText(HWND hWnd) {
    constexpr int BUF_SIZE = 256;
    wchar_t buf[BUF_SIZE] = {0};
    GetWindowText(hWnd, buf, BUF_SIZE);
    return buf;
}

std::wstring WindowUtils::getClassName(HWND hWnd) {
    constexpr int BUF_SIZE = 256;
    wchar_t buf[BUF_SIZE] = {0};
    GetClassName(hWnd, buf, BUF_SIZE);
    return buf;
}

std::vector<HWND> WindowUtils::findShellTrayWindows() {
    if (getNumMonitors() == 1) {
        HWND hWnd = FindWindow(Shell_TrayWnd, nullptr);
        if (hWnd) {
            return {hWnd};
        }
        return {};
    } else {
        std::vector<HWND> list;
        EnumWindows(shellTrayWindowEnum, reinterpret_cast<LPARAM>(&list));
        return list;
    }
}

BOOL CALLBACK WindowUtils::shellTrayWindowEnum(HWND hWnd, LPARAM lparam) {
    auto className = getClassName(hWnd);
    if (className == Shell_TrayWnd || className == Shell_SecondaryTrayWnd) {
        auto& list = *reinterpret_cast<std::vector<HWND> *>(lparam);
        list.push_back(hWnd);
    }
    return TRUE;
}

bool WindowUtils::isRunningApplications(HWND hWnd) {
    return getClassName(hWnd) == MSTaskListWClass;
}

bool WindowUtils::isTopLevelWindow(HWND hWnd) {
    if (!IsWindowVisible(hWnd)) {
        return false;
    }

    LONG style = GetWindowLong(hWnd, GWL_EXSTYLE);
    if (style & WS_EX_TOOLWINDOW) {
        return false;
    }

    if (!isAltTabWindow(hWnd)) {
        return false;
    }

    int cloaked = 0;
    HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (hr == S_OK && cloaked) {
        return false;
    }

    return true;
}

// See http://blogs.msdn.com/b/oldnewthing/archive/2007/10/08/5351207.aspx
BOOL WindowUtils::isAltTabWindow(HWND hWnd) {
    // Start at the root owner
    HWND hwndWalk = GetAncestor(hWnd, GA_ROOTOWNER);

    // See if we are the last active visible popup
    HWND hwndTry;
    while ((hwndTry = GetLastActivePopup(hwndWalk)) != hwndTry) {
        if (IsWindowVisible(hwndTry)) break;
        hwndWalk = hwndTry;
    }
    return hwndWalk == hWnd;
}

int WindowUtils::getTaskbarPosition() {
    int position = ABE_BOTTOM;
    APPBARDATA abd = {0};
    abd.cbSize = sizeof(abd);
    if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd)) {
        position = static_cast<int>(abd.uEdge);
    }
    return position;
}

bool WindowUtils::isAutoHideTaskbar() {
    APPBARDATA abd = {sizeof(APPBARDATA)};
    auto state = SHAppBarMessage(ABM_GETSTATE, &abd);
    return state & ABS_AUTOHIDE;
}

float WindowUtils::getDpiScale() {
    HDC screen = GetDC(nullptr);
    float sx = static_cast<float>(GetDeviceCaps(screen, LOGPIXELSX)) / 96.0f;
    ReleaseDC(nullptr, screen);
    return sx;
}

bool WindowUtils::registerClass(const wchar_t *className,
                                WNDPROC wndProc,
                                UINT style /*= 0*/,
                                HICON hIcon /*= nullptr*/,
                                HCURSOR hCursor /*= nullptr*/,
                                HBRUSH hbrBackground /*= nullptr*/) {
    WNDCLASSEX wc = {sizeof(wc)};
    wc.style = style;
    wc.lpfnWndProc = wndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = Process::moduleInstance();
    wc.hIcon = hIcon;
    wc.hCursor = hCursor;
    wc.hbrBackground = hbrBackground;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = className;
    return RegisterClassEx(&wc);
}

void WindowUtils::registerDefaultClass() {
    registerClass(Window::DEFAULT_CLASS, Window::windowProcStatic, CS_DBLCLKS, nullptr);
}

int WindowUtils::getNumMonitors() {
    return DisplayMonitors::numMonitors();
}

bool WindowUtils::isFullScreenWindow(HWND hWnd) {
    MonitorInfo mi{};
    DisplayMonitors::get(mi);

    HWND hWndRoot = GetAncestor(hWnd, GA_ROOTOWNER);
    assert(hWndRoot);

    RECT rc = {0};
    GetWindowRect(hWndRoot, &rc);

    HMONITOR hMon = MonitorFromWindow(hWndRoot, MONITOR_DEFAULTTONEAREST);
    if (mi.hMonitors.contains(hMon)) {
        const auto& mon = mi.monitors[mi.hMonitors[hMon]];
        return Rectangle::equals(mon, rc);
    }
    return false;
}

} // namespace litelockr

