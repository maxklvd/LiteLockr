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

#include "LayeredWindow.h"

#include "sys/Process.h"

namespace litelockr {

void LayeredWindow::updateWindow() {
    const unsigned int w = buffer_.width();
    const unsigned int h = buffer_.height();
    bufferPre_.create(w, h);
    bufferPre_.blitFrom(buffer_);
    bufferPre_.pixFmt().premultiply();
    updateLayeredWindow(hWnd, position_, bufferPre_.operator HBITMAP(), w, h);
}

void LayeredWindow::createWindow(HWND hWndParent) {
    hWnd = CreateWindowEx(exStyle_, className_, windowName_, style_,
                          0, 0, 0, 0, hWndParent, nullptr, Process::moduleInstance(), nullptr);
    assert(hWnd);
    attach(hWnd);
    onCreate();
}

void LayeredWindow::updateLayeredWindow(HWND hWnd, POINT position, HBITMAP hBitmap, int width, int height) {
    HDC hWndDC = GetDC(hWnd);
    HDC hMemDC = CreateCompatibleDC(hWndDC);
    if (hMemDC) {
        HGDIOBJ oldObj = SelectObject(hMemDC, hBitmap);

        BLENDFUNCTION blendFunc = {
                .BlendOp = AC_SRC_OVER,
                .BlendFlags = 0,
                .SourceConstantAlpha = 255,
                .AlphaFormat = AC_SRC_ALPHA,
        };
        SIZE size{width, height};
        POINT src{};

        if (!UpdateLayeredWindow(hWnd, nullptr, &position, &size, hMemDC, &src, 0, &blendFunc, ULW_ALPHA)) {
            assert(false);
        }

        SelectObject(hMemDC, oldObj);
        DeleteDC(hMemDC);
    }
    ReleaseDC(hWnd, hWndDC);
}

} // namespace litelockr
