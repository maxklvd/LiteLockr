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

#include "Tooltip.h"

#include <cassert>

#include <CommCtrl.h>

namespace litelockr {

void Tooltip::create(HWND hWndParent) {
    assert(hWndParent);
    hWndParent_ = hWndParent;

    hWndTip_ = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
                              WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 0, 0, 0, 0, hWndParent, nullptr, nullptr,
                              nullptr);

    SetWindowPos(hWndTip_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    assert(hWndTip_);
}

bool Tooltip::addWindow(HWND hWndControl, const std::wstring& text) {
    assert(hWndTip_);

    TTTOOLINFO ti{sizeof ti};
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.hwnd = hWndParent_;
    ti.uId = reinterpret_cast<UINT_PTR>(hWndControl);
    ti.lpszText = const_cast<LPTSTR>(text.c_str());
    return SendMessage(hWndTip_, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
}

[[maybe_unused]] bool Tooltip::addWindow(int dlgCtrlId, const std::wstring& text) {
    HWND hWndControl = GetDlgItem(hWndParent_, dlgCtrlId);
    return addWindow(hWndControl, text);
}

bool Tooltip::addRectangle(const RECT& rect, const std::wstring& text) {
    assert(hWndTip_);

    TTTOOLINFO ti{sizeof ti};
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hWndParent_;
    ti.lpszText = const_cast<LPTSTR>(text.c_str());
    ti.rect = rect;
    return SendMessage(hWndTip_, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
}

bool Tooltip::addRectangle(int dlgCtrlId, const std::wstring& text) {
    RECT rect;
    HWND hWndControl = GetDlgItem(hWndParent_, dlgCtrlId);
    GetClientRect(hWndControl, &rect);
    MapWindowPoints(hWndControl, hWndParent_, reinterpret_cast<LPPOINT>(&rect), 2);
    return addRectangle(rect, text);
}

void Tooltip::updateTipText(const std::wstring& text) {
    assert(hWndTip_);

    TTTOOLINFO ti{sizeof ti};
    ti.hwnd = hWndParent_;
    ti.lpszText = const_cast<LPTSTR>(text.c_str());
    SendMessage(hWndTip_, TTM_UPDATETIPTEXT, 0, reinterpret_cast<LPARAM>(&ti));
}

void Tooltip::activate(bool state) {
    SendMessage(hWndTip_, TTM_ACTIVATE, state, 0L);
}

} // namespace litelockr
