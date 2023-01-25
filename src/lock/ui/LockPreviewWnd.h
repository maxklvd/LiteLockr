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

#ifndef LOCK_PREVIEW_WND_H
#define LOCK_PREVIEW_WND_H

#include <string>

#include "gui/LayeredWindow.h"
#include "lock/LockAreaMap.h"

namespace litelockr {

class LockPreviewWnd: public LayeredWindow {
public:
    LockPreviewWnd() {
        onMessage(WM_LBUTTONUP, &LockPreviewWnd::destroy);
        onMessage(WM_RBUTTONUP, &LockPreviewWnd::destroy);
        onMessage(WM_TIMER, &LockPreviewWnd::destroy);
    }

    virtual ~LockPreviewWnd() = default;

    [[nodiscard]] bool active() const { return IsWindow(hWnd); }

    void show(int previewMode);

protected:
    void onCreate() override;

    void visualizeMap(Bitmap& buffer, const LockAreaMap& map) const;
    void drawApplications(Bitmap& buffer, const LockAreaMap& map) const;
    void drawTaskbarButtons(Bitmap& buffer, const LockAreaMap& map) const;

    void fillRect(Bitmap& buffer, RECT rc, Color color, unsigned char cover) const;
    void drawRectBorder(Bitmap& buffer, RECT rc, Color color, int borderWidth, const std::wstring& label) const;
    void drawLockedBackground(Bitmap& buffer) const;
    void findAllTopLevelWindows(std::function<bool(HWND)> func) const;

    void destroy();

    constexpr static int DURATION = 3000; // shows this window for 3 secs
    constexpr static int TIMER_ID = 1;

private:
    int previewMode_ = 0;
};

} // namespace litelockr

#endif // LOCK_PREVIEW_WND_H
