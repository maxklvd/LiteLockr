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

#ifndef PROCESS_H
#define PROCESS_H

#include <string>

#include <windows.h>

namespace litelockr {

class Process {
public:
    constexpr static auto APP_MUTEX = L"F4D67825-90D9-4A5E-A6D3-126F8EAF37E0";

    static void initialize();

    static HINSTANCE moduleInstance();
    static void setMainWindow(HWND hWnd);
    static HWND mainWindow();

    static DWORD mainThreadId();
    static DWORD processId();

    static const std::wstring& modulePath() {
        return modulePath_;
    }

    static void setHookThreadId(DWORD hookThreadId) {
        hookThreadId_ = hookThreadId;
    }

    static DWORD hookThreadId() {
        return hookThreadId_;
    }

    static void setWorkerThreadId(DWORD workerThreadId) {
        workerThreadId_ = workerThreadId;
    }

    static DWORD workerThreadId() {
        return workerThreadId_;
    }

    static void setWinEventThreadId(DWORD winEventThreadId) {
        winEventThreadId_ = winEventThreadId;
    }

    static DWORD winEventThreadId() {
        return winEventThreadId_;
    }

    static bool isRemoteSession() {
        return remoteSession_;
    }

    static void updateRemoteSession();

    static bool isSettingsDialogOpen() {
        return isSettingsDialogOpen_;
    }

    static void setSettingsDialogOpen(bool state) {
        isSettingsDialogOpen_ = state;
    }

    static void setActiveWindow(HWND hWnd) {
        activeWindow_ = hWnd;
    }

    static HWND getActiveWindow() {
        return activeWindow_;
    }

    static void setCurrentAppAutomationId(std::wstring_view id) {
        currentAppAutomationId_ = id;
    }

    static const std::wstring& currentAppAutomationId() {
        return currentAppAutomationId_;
    }

    static bool isLightMode() {
        return lightMode_;
    }

    static void setLightMode(bool lightMode) {
        lightMode_ = lightMode;
    }

private:
    static HINSTANCE hModuleInstance_;
    static HWND hWndRoot_;
    static DWORD mainThreadId_;
    static DWORD hookThreadId_;
    static DWORD workerThreadId_;
    static DWORD winEventThreadId_;
    static DWORD processId_;
    static std::wstring modulePath_;
    static bool remoteSession_;
    static bool isSettingsDialogOpen_;
    static HWND activeWindow_;
    static std::wstring currentAppAutomationId_;
    static bool lightMode_;
};

} // namespace litelockr

#endif // PROCESS_H
