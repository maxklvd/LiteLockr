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

#include "Window.h"

namespace litelockr {

LRESULT Window::windowProcStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window *wndPtr = getInstance(hWnd);
    if (wndPtr) {
        LRESULT lResult = 0;
        BOOL bRet = wndPtr->windowProc(hWnd, uMsg, wParam, lParam, lResult);
        // do the default processing if message was not handled
        if (!bRet) {
            lResult = DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        return lResult;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::attach(HWND hwnd) {
    assert(hwnd);
    this->hWnd = hwnd;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

HWND Window::detach() {
    HWND hwnd = this->hWnd;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    this->hWnd = nullptr;
    return hwnd;
}

void Window::destroyWindow() {
    assert(hWnd);
    DestroyWindow(detach());
}

Window *Window::getInstance(HWND hWnd) {
    return reinterpret_cast<Window *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
}

BOOL Window::windowProc(HWND /*hWnd*/, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult) {
    auto iter = messageHandlers_.callbacks.find(uMsg);
    if (iter != messageHandlers_.callbacks.end()) {
        BOOL bHandled = TRUE;

        auto& handler = iter->second;
        lResult = handler(uMsg, wParam, lParam, bHandled);
        if (bHandled) { // the value can be changed by the handler
            return TRUE;
        }
    }

    if (uMsg == WM_COMMAND) {
        WORD id = LOWORD(wParam);

        auto it = commandHandlers_.callbacks.find(id);
        if (it != commandHandlers_.callbacks.end()) {
            BOOL bHandled = TRUE;
            auto& handler = it->second;
            lResult = handler(HIWORD(wParam), id, (HWND) lParam, bHandled);
            if (bHandled) { // the value can be changed by the handler
                return TRUE;
            }
        }
    } else if (uMsg == WM_NOTIFY) {
        auto pNMHDR = reinterpret_cast<LPNMHDR>(lParam);

        auto it = notifyHandlers_.callbacks.find(pNMHDR->code);
        if (it != notifyHandlers_.callbacks.end()) {
            BOOL bHandled = TRUE;
            auto& handler = it->second;
            lResult = handler(static_cast<int>(wParam), pNMHDR, bHandled);
            if (bHandled) { // the value can be changed by the handler
                return TRUE;
            }
        }
    }
    return FALSE;
}

} // namespace litelockr
