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

#include "User32.h"

namespace litelockr {

using FnSetWindowsHookExW = decltype(SetWindowsHookExW) *;
using FnUnhookWindowsHookEx = decltype(UnhookWindowsHookEx) *;
using FnCallNextHookEx = decltype(CallNextHookEx) *;
using FnGetForegroundWindow = decltype(GetForegroundWindow) *;
using FnSetWinEventHook = decltype(SetWinEventHook) *;
using FnUnhookWinEvent = decltype(UnhookWinEvent) *;

HHOOK User32::setWindowsHookEx(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId) {
    static FnSetWindowsHookExW procAddressPtr = nullptr;

    if (!procAddressPtr) {
        procAddressPtr = reinterpret_cast<FnSetWindowsHookExW>(GetProcAddress(getLibrary(), "SetWindowsHookExW"));
    }
    return procAddressPtr(idHook, lpfn, hmod, dwThreadId);
}

BOOL User32::unhookWindowsHookEx(HHOOK hhk) {
    static FnUnhookWindowsHookEx procAddressPtr = nullptr;

    if (!procAddressPtr) {
        procAddressPtr = reinterpret_cast<FnUnhookWindowsHookEx>(GetProcAddress(getLibrary(), "UnhookWindowsHookEx"));
    }
    return procAddressPtr(hhk);
}

LRESULT User32::callNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam) {
    static FnCallNextHookEx procAddressPtr = nullptr;

    if (!procAddressPtr) {
        procAddressPtr = reinterpret_cast<FnCallNextHookEx>(GetProcAddress(getLibrary(), "CallNextHookEx"));
    }
    return procAddressPtr(hhk, nCode, wParam, lParam);
}

HWND User32::getForegroundWindow() {
    static FnGetForegroundWindow procAddressPtr = nullptr;

    if (!procAddressPtr) {
        procAddressPtr = reinterpret_cast<FnGetForegroundWindow>(GetProcAddress(getLibrary(), "GetForegroundWindow"));
    }
    return procAddressPtr();
}

HWINEVENTHOOK
User32::setWinEventHook(DWORD eventMin, DWORD eventMax, HMODULE hmodWinEventProc, WINEVENTPROC pfnWinEventProc,
                        DWORD idProcess, DWORD idThread, DWORD dwFlags) {
    static FnSetWinEventHook procAddressPtr = nullptr;

    if (!procAddressPtr) {
        procAddressPtr = reinterpret_cast<FnSetWinEventHook>(GetProcAddress(getLibrary(), "SetWinEventHook"));
    }
    return procAddressPtr(
            eventMin,
            eventMax,
            hmodWinEventProc,
            pfnWinEventProc,
            idProcess,
            idThread,
            dwFlags);
}

BOOL User32::unhookWinEvent(HWINEVENTHOOK hWinEventHook) {
    static FnUnhookWinEvent procAddressPtr = nullptr;

    if (!procAddressPtr) {
        procAddressPtr = reinterpret_cast<FnUnhookWinEvent>(GetProcAddress(getLibrary(), "UnhookWinEvent"));
    }
    return procAddressPtr(hWinEventHook);
}

HINSTANCE User32::getLibrary() {
    static HINSTANCE hInstance = nullptr;

    if (!hInstance) {
        hInstance = GetModuleHandle(L"USER32.DLL");
    }
    return hInstance;
}

} // namespace litelockr

