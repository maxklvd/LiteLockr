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

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <string>

#include <windows.h>

namespace litelockr {

class Tooltip {
public:
    void create(HWND hWndParent);
    bool addWindow(HWND hWndControl, const std::wstring& text);
    [[maybe_unused]] bool addWindow(int dlgCtrlId, const std::wstring& text);
    bool addRectangle(const RECT& rect, const std::wstring& text);
    bool addRectangle(int dlgCtrlId, const std::wstring& text);
    void updateTipText(const std::wstring& text);
    void activate(bool state);

private:
    HWND hWndTip_ = nullptr;
    HWND hWndParent_ = nullptr;
};

} // namespace litelockr

#endif // TOOLTIP_H