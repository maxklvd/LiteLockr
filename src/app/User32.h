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

#ifndef USER32_H
#define USER32_H

#include <windows.h>

namespace litelockr {

class User32 {
public:
    static HHOOK setWindowsHookEx(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId);
    static BOOL unhookWindowsHookEx(HHOOK hhk);
    static LRESULT callNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam);
    static HWND getForegroundWindow();
    static HWINEVENTHOOK setWinEventHook(
            DWORD eventMin,
            DWORD eventMax,
            HMODULE hmodWinEventProc,
            WINEVENTPROC pfnWinEventProc,
            DWORD idProcess,
            DWORD idThread,
            DWORD dwFlags);
    static BOOL unhookWinEvent(HWINEVENTHOOK hWinEventHook);

private:
    static HINSTANCE getLibrary();
};

} // namespace litelockr

#endif // USER32_H
