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

#ifndef WINDOW_UTILS_H
#define WINDOW_UTILS_H

#include <functional>
#include <string>
#include <vector>
#include <utility>

#include <windows.h>

namespace litelockr {

class WindowUtils {
public:
    static bool getWindowRect(HWND hWnd, RECT& rc);
    static std::wstring getWindowText(HWND hWnd);
    static std::wstring getClassName(HWND hWnd);
    static std::vector<HWND> findShellTrayWindows();

    template<typename F>
    requires std::invocable<F, HWND>
    static void findRunningApplicationsToolBars(F callback) {
        auto shellTrays = findShellTrayWindows();
        for (auto& hWnd: shellTrays) {
            EnumChildWindows(hWnd, runningApplicationsEnum < F > , reinterpret_cast<LPARAM>(&callback));
        }
    }

    template<typename F>
    requires std::invocable<F, HWND>
    static BOOL CALLBACK runningApplicationsEnum(HWND hWnd, LPARAM lParam) {
        if (isRunningApplications(hWnd)) {
            auto& callback = *reinterpret_cast<F *>(lParam);
            std::invoke(callback, hWnd);

            if (getNumMonitors() == 1) {
                return FALSE; // stop
            }
        }
        return TRUE; // continue
    }

    static bool isRunningApplications(HWND hWnd);

    template<typename F>
    requires std::invocable<F, HWND>
    static void findTopLevelWindows(F callback) {
        EnumWindows(topLevelWindowsEnum < F > , reinterpret_cast<LPARAM>(&callback));
    }

    template<typename F>
    requires std::invocable<F, HWND>
    static BOOL CALLBACK topLevelWindowsEnum(HWND hWnd, LPARAM lParam) {
        if (isTopLevelWindow(hWnd)) {
            auto& callback = *reinterpret_cast<F *>(lParam);
            std::invoke(callback, hWnd);
        }
        return TRUE;
    }

    static int getTaskbarPosition();
    static bool isAutoHideTaskbar();

    static float getDpiScale();

    static bool registerClass(const wchar_t *class_name, WNDPROC wndProc, UINT style = 0, HICON hIcon = nullptr,
                              HCURSOR hCursor = nullptr, HBRUSH hbrBackground = nullptr);


    static void registerDefaultClass();

    static bool isFullScreenWindow(HWND hWnd);

private:
    static BOOL isAltTabWindow(HWND hWnd);
    static bool isTopLevelWindow(HWND hWnd);
    static int getNumMonitors();

    static BOOL CALLBACK shellTrayWindowEnum(HWND hWnd, LPARAM lparam);

    constexpr static auto MSTaskListWClass = L"MSTaskListWClass";
    constexpr static auto Shell_TrayWnd = L"Shell_TrayWnd";
    constexpr static auto Shell_SecondaryTrayWnd = L"Shell_SecondaryTrayWnd";
};

} // namespace litelockr

#endif // WINDOW_UTILS_H
