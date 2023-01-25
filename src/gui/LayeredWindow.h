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

#ifndef LAYERED_WINDOW_H
#define LAYERED_WINDOW_H

#include "gfx/Bitmap.h"
#include "gui/Window.h"

namespace litelockr {

class LayeredWindow: public Window {
public:
    LayeredWindow() {
        onMessage(WM_SETCURSOR, []() { SetCursor(LoadCursor(nullptr, IDC_ARROW)); });
    }

    void setPosition(POINT position) {
        position_ = position;
    }

    void createWindow(HWND hWndParent);

protected:
    void updateWindow();

    virtual void onCreate() {}

    static void updateLayeredWindow(HWND hWnd, POINT position, HBITMAP hBitmap, int width, int height);

    Bitmap buffer_;
    Bitmap bufferPre_;
    POINT position_{};
    DWORD style_ = WS_POPUP | WS_VISIBLE;
    DWORD exStyle_ = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    const wchar_t *className_ = DEFAULT_CLASS;
    const wchar_t *windowName_ = L"";
};

} // namespace litelockr

#endif // LAYERED_WINDOW_H
