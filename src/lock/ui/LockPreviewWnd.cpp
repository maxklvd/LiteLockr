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

#include "LockPreviewWnd.h"

#include <array>

#include <dwmapi.h>
#include "app/event/AppEvent.h"
#include "gfx/BitmapContext.h"
#include "gui/WindowUtils.h"
#include "ini/SettingsData.h"
#include "lock/DisplayMonitors.h"
#include "lock/MousePositionValidator.h"
#include "lock/WindowValidator.h"
#include "lock/ui/PreviewStyle.h"
#include "log/Logger.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

#include "res/Resources.h"

namespace litelockr {

void LockPreviewWnd::onCreate() {
    MousePositionValidator positionValidator;
    positionValidator.setPreviewMode(previewMode_);
    positionValidator.recreate();
    positionValidator.swapUpdate();
    const auto& map = positionValidator.currentMap();
    visualizeMap(buffer_, map);

    MonitorInfo mi{};
    DisplayMonitors::get(mi);
    setPosition({mi.unionMonitor.left, mi.unionMonitor.top});

    drawApplications(buffer_, map);
    drawTaskbarButtons(buffer_, map);

    updateWindow();
}

void LockPreviewWnd::visualizeMap(Bitmap& buffer, const LockAreaMap& map) const {
    const int width = map.width();
    const int height = map.height();

    buffer.create(width, height);
    drawLockedBackground(buffer);

    uint32_t colorAllow = 0x80000000; // RGBA(0, 0, 0, 128)

    constexpr auto LEN = 256;
    std::array<bool, LEN> colorExists{};
    std::array<uint32_t, LEN> colors{};

    int i = 0;
    for (const auto& area: map.getLockAreas()) {
        switch (area.id) {
            case LockArea::ALLOW:
                colorExists[i] = true;
                colors[i] = colorAllow;
                break;
            case LockArea::TASKBAR_BUTTON:
            case LockArea::NOTIFICATION_AREA_ICON:
                colorExists[i] = true;
                colors[i] = 0;
                break;
            default:
                // skip all others
                break;
        }

        i++;
    }

    auto pixf = buffer.pixFmt();
    assert(pixf.pix_width == sizeof(uint32_t));

    const auto& data = map.data();

    for (int y = 0; y < height; y++) {
        int rowIdx = width * y;

        auto colPtr = reinterpret_cast<uint32_t *>(pixf.row_ptr(y));
        for (int x = 0; x < width; x++) {
            auto areaIdx = static_cast<unsigned>(data[rowIdx + x]);
            assert(areaIdx < map.getLockAreas().size());

            if (colorExists[areaIdx]) {
                *colPtr = colors[areaIdx];
            }
            colPtr++; // next pixel
        }
    }
}

void LockPreviewWnd::drawApplications(Bitmap& buffer, const LockAreaMap& map) const {
    //
    // Shows all allowed apps
    //
    const auto& settings = SettingsData::instance();
    WindowValidator validator;

    for (const auto& app: settings.getAllowedApps()) {
        if (app.enabled) {
            validator.addAllowedApp(app.path());
        }
    }

    int offsetX = map.offsetX();
    int offsetY = map.offsetY();

    using namespace PreviewStyle;

    findAllTopLevelWindows([this, &buffer, &validator, offsetX, offsetY](HWND hwnd) {
        assert(hwnd);
        std::wstring exeName;

        if (validator.isAllowedInfo(hwnd, exeName)) {
            RECT rc{};
            if (WindowUtils::getWindowRect(hwnd, rc)) {
                Rectangle::offset(rc, -offsetX, -offsetY);

                Rectangle::inflate(rc, {2, 2});

                this->fillRect(buffer, rc, ApplicationBackgroundColor, ApplicationBackgroundOpacity);
                this->drawRectBorder(buffer, rc, ApplicationBorderColor, ApplicationBorderWidth, exeName);
            }
        }
        return true;
    });
}

void LockPreviewWnd::drawTaskbarButtons(Bitmap& buffer, const LockAreaMap& map) const {
    using namespace PreviewStyle;

    //
    // Shows all allowed taskbar buttons
    //
    for (const auto& area: map.getLockAreas()) {
        if (area.id == LockArea::TASKBAR_BUTTON ||
            area.id == LockArea::NOTIFICATION_AREA_ICON) {
            RECT rc = area.rc;
            Rectangle::offset(rc, -map.offsetX(), -map.offsetY());

            drawRectBorder(buffer, rc, ButtonBorderColor, ButtonBorderWidth, L"");
        }
    }
}


BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    if (IsWindowVisible(hWnd) && !IsIconic(hWnd)) {
        auto f = reinterpret_cast<std::function<bool(HWND)> *>(lparam);
        if (f && !f->operator()(hWnd)) {
            return FALSE;
        }
    }
    return TRUE;
}

void LockPreviewWnd::findAllTopLevelWindows(std::function<bool(HWND)> func) const {
    EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(&func));
}

void LockPreviewWnd::drawLockedBackground(Bitmap& buffer) const {
    Bitmap locked;
    locked.loadPng(IDB_LOCKED_AREA);
    assert(!locked.isNull());

    Bitmap row;
    row.create(buffer.width(), locked.height());

    unsigned x = 0;
    do {
        auto w = locked.width();
        if (x + w > row.width()) {
            w = row.width() - x;
        }
        auto h = locked.height();

        row.copyFrom(locked, x, 0, 0, 0, w, h);
        x += locked.width();
    } while (x < row.width());

    unsigned y = 0;
    do {
        auto w = row.width();
        auto h = row.height();
        if (y + h > buffer.height()) {
            h = buffer.height() - y;
        }
        buffer.copyFrom(row, 0, y, 0, 0, w, h);
        y += row.height();
    } while (y < buffer.height());
}

void LockPreviewWnd::fillRect(Bitmap& buffer, RECT rc, Color color, unsigned char cover) const {
    Rectangle::clamp(rc, 0, 0, buffer.width(), buffer.height());
    if (!Rectangle::empty(rc)) {
        buffer.fillRect(rc, color, cover);
    }
}

void LockPreviewWnd::drawRectBorder(Bitmap& buffer, RECT rc, Color color, int borderWidth,
                                    const std::wstring& label) const {
    using namespace PreviewStyle;

    Rectangle::clamp(rc, 0, 0, buffer_.width(), buffer_.height());
    Rectangle::inflate(rc, {-borderWidth / 2, -borderWidth / 2});
    if (!Rectangle::empty(rc)) {
        const int width = Rectangle::width(rc);
        const int height = Rectangle::height(rc);

        BitmapContext ctx(buffer);
        ctx.addRect(rc.left, rc.top, rc.right, rc.bottom);
        ctx.strokeColor = color;
        ctx.lineWidth = static_cast<float>(borderWidth);
        ctx.stroke();

        if (!label.empty()) {
            NONCLIENTMETRICS metrics = {sizeof(metrics)};
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
            HFONT hFont = CreateFontIndirect(&metrics.lfCaptionFont);

            ctx.font = hFont;
            SIZE labelSize = ctx.textSize(label);
            if (labelSize.cx > 0 && labelSize.cy > 0) {
                Bitmap textBuf;
                BitmapContext textCtx(textBuf);
                const int bufWidth = labelSize.cx + 5;
                const int bufHeight = labelSize.cy;
                textBuf.create(bufWidth, bufHeight);
                textBuf.clear(color);

                textCtx.font = hFont;
                textCtx.textColor = ApplicationTextColor;
                textCtx.textOut(label, 2, 0);

                int w = std::min(bufWidth, width);
                int h = std::min(bufHeight, height);
                buffer.copyFrom(textBuf, rc.left, rc.top, 0, 0, w, h);
            }
            DeleteObject(hFont);
        }
    }
}

void LockPreviewWnd::show(int previewMode) {
    if (hWnd) {
        assert(false);
        return;
    }
    previewMode_ = previewMode;

    //
    // Create the window
    //
    style_ = WS_OVERLAPPEDWINDOW; // fullscreen, но не скрывать taskbar
    createWindow(Process::mainWindow());
    if (hWnd) {
        ShowWindow(hWnd, SW_SHOWNA);
        SetTimer(hWnd, TIMER_ID, DURATION, nullptr);
    }
}

void LockPreviewWnd::destroy() {
    KillTimer(hWnd, TIMER_ID);
    destroyWindow();
    AppEvents::send(AppEvent(AppEvent::LOCK_INFORMATION_ENDED));
}

} // namespace litelockr
