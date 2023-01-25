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

#include "MouseFilter.h"

#include "app/AppMessages.h"
#include "app/NotificationArea.h"
#include "lock/HookData.h"
#include "lock/KeyboardFilter.h"
#include "lock/WinEventThread.h"
#include "lock/WorkerThread.h"
#include "log/Logger.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

#include "res/Resources.h"

namespace litelockr {

using namespace std::chrono_literals;

std::optional<std::reference_wrapper<const HookOptions>> MouseFilter::options_{};

MousePositionValidator MouseFilter::positionValidator_;
MousePositionValidatorTimer MouseFilter::positionValidatorTimer_;
RECT MouseFilter::previousClipCursor_ = {};
DblClickRecognizer MouseFilter::dblClickDetector_;

TimeCounter MouseFilter::checkTrayIconPeriod_{3s};
RECT MouseFilter::trayIconRect_ = {};

bool MouseFilter::buttonPressed_[BUTTON_PRESSED_SIZE] = {};

void MouseFilter::install(const HookOptions& options) {
    options_ = options;

    dblClickDetector_.initialize();
    previousClipCursor_ = {};
    trayIconRect_ = NotificationArea::getIconRect();
    std::ranges::fill(buttonPressed_, false);
}

void MouseFilter::uninstall() {
    options_ = {};
    ClipCursor(nullptr);
}

bool MouseFilter::processMouseStroke(MouseStroke& stroke) {
    assert(options_.has_value());
    const HookOptions& options = options_.value();

    if (!options.lockMouse) {
        return true;
    }

    if (!HookData::isSessionActive()) {
        // no active session, no blocking
        return true;
    }

    if (HookData::mouse.sessionReconnected.load(std::memory_order_relaxed)) {
        HookData::mouse.sessionReconnected.store(false, std::memory_order_relaxed);
        std::ranges::fill(buttonPressed_, false);
    }

    const int LEFT_BUTTON = 0;
    const int RIGHT_BUTTON = 1;
    switch (stroke.state) {
        case MouseStroke::LEFT_BUTTON_DOWN:
            buttonPressed_[LEFT_BUTTON] = true;
            break;
        case MouseStroke::RIGHT_BUTTON_DOWN:
            buttonPressed_[RIGHT_BUTTON] = true;
            break;
        case MouseStroke::LEFT_BUTTON_UP:
            if (!buttonPressed_[LEFT_BUTTON]) {
                return true; // releases the button
            }
            buttonPressed_[LEFT_BUTTON] = false;
            break;
        case MouseStroke::RIGHT_BUTTON_UP:
            if (!buttonPressed_[RIGHT_BUTTON]) {
                return true; // releases the button
            }
            buttonPressed_[RIGHT_BUTTON] = false;
            break;
        default:
            // nothing to do
            break;
    }

    HWND hWnd = WindowFromPoint(stroke.pt);
    bool appAllowed = HookData::windowValidator().isAllowed(hWnd);
    if (appAllowed && HookData::windowValidator().isFullScreenWindow(hWnd)) {
        clipCursor(nullptr);
        getClipCursor(&previousClipCursor_);
        LOG_DEBUG(L"[PASS] Allowed fullscreen window");
        return true;
    }

    LockArea area;
    bool positionAllowed = isPositionAllowed(stroke.pt, area);

    if (stroke.state == MouseStroke::MOUSE_MOVE) {
        RECT clipCursorRect{};
        if (getClipCursor(&clipCursorRect)) {
            if (positionAllowed) {
                // mouse move is allowed
                if (Rectangle::empty(area.rc)) {
                    if (!Rectangle::equals(clipCursorRect, previousClipCursor_)) {
                        clipCursor(nullptr);
                        getClipCursor(&previousClipCursor_);
                    }
                } else if (!Rectangle::equals(clipCursorRect, area.rc)) {
                    clipCursor(&area.rc);
                    previousClipCursor_ = area.rc;
                }
            } else {
                // mouse move is prohibited
                if (!Rectangle::equals(clipCursorRect, previousClipCursor_) &&
                    !Rectangle::empty(previousClipCursor_)) {
                    clipCursor(&previousClipCursor_);
                }

                if (!Rectangle::empty(previousClipCursor_)) {
                    stroke.pt.x = std::clamp(stroke.pt.x, previousClipCursor_.left, previousClipCursor_.right - 1);
                    stroke.pt.y = std::clamp(stroke.pt.y, previousClipCursor_.top, previousClipCursor_.bottom - 1);
                    LOG_VERBOSE(L"STROKE UPDATED: new %d, %d clip=%s", stroke.pt.x, stroke.pt.y,
                                Rectangle::toString(previousClipCursor_).c_str());
                }
            }
        }
        return true;
    }

    if (!positionAllowed) {
        LOG_DEBUG(L"[BLOCK] The position is not allowed");
        return false; //blocks all mouse events which are outside the work areas/allowRects
    }

    if (area.id == LockArea::TASKBAR_BUTTON) {
        LOG_DEBUG(L"[PASS] TASKBAR_BUTTON");
        return true;
    }
    if (area.id == LockArea::NOTIFICATION_AREA_ICON) {
        LOG_DEBUG(L"[PASS] NOTIFICATION_AREA_ICON");
        return true;
    }
    if (appAllowed) {
        LOG_DEBUG(L"[PASS] Allowed application");
        return true;
    }

    if (options.processCaptionButtons && stroke.state == MouseStroke::LEFT_BUTTON_UP) {
        WPARAM param = 0;
        if (dblClickDetector_.isDoubleClick(stroke.pt) && !HookData::windowValidator().isExplorer(hWnd)) {
            param |= WorkerThread::PCB_DBL_CLICK;
        }
        if (KeyboardFilter::modifierKeys().ctrl) {
            param |= WorkerThread::PCB_CTRL_PRESSED;
        }
        PostThreadMessage(Process::workerThreadId(), WMU_WT_PROCESS_CAPTION_BUTTON,
                          param, MAKELPARAM(stroke.pt.x, stroke.pt.y));
    }

    if (stroke.state == MouseStroke::LEFT_BUTTON_UP || stroke.state == MouseStroke::RIGHT_BUTTON_UP) {
        WPARAM wParam = stroke.state == MouseStroke::LEFT_BUTTON_UP ? WM_LBUTTONUP : WM_RBUTTONUP;
        auto lParam = MAKELPARAM(stroke.pt.x, stroke.pt.y);

        if (HookData::windowValidator().isExplorer(hWnd)) {
            PostThreadMessage(Process::workerThreadId(), WMU_WT_PROCESS_SYSTRAY_ICON, wParam, lParam);
        }
        PostMessage(Process::mainWindow(), WMU_HIDE_COMMAND, wParam, lParam);
    }

    return false;
}

void MouseFilter::tick() {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    if (WinEventThread::isTaskbarChanged()) {
        WinEventThread::resetTaskbarChanged();
        positionValidatorTimer_.markNeedsUpdate();
    }

    if (checkTrayIconPeriod_.timeout() &&
        !Rectangle::empty(trayIconRect_)) {

        RECT rc = NotificationArea::getIconRect();
        if (!Rectangle::equals(rc, trayIconRect_)) {
            trayIconRect_ = rc;
            positionValidatorTimer_.markNeedsUpdate();
        }
    }

    if (positionValidatorTimer_.ready()) {
        positionValidator_.recreate();
    }
}

void MouseFilter::updatePositionValidator(DWORD delayBefore) {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    if (delayBefore == 0) {
        // update immediately
        positionValidator_.recreate();
    } else {
        // update with a delay timer
        positionValidatorTimer_.markNeedsUpdate(delayBefore);
    }
}

bool MouseFilter::isPositionAllowed(const POINT& cursorPosition, LockArea& area) {
    positionValidator_.swapUpdate();
    area = positionValidator_.getLockArea(cursorPosition.x, cursorPosition.y);
    return area.allowed;
}

BOOL MouseFilter::clipCursor(const RECT *rect) {
    if (!Process::isRemoteSession()) {
        return ClipCursor(rect);
    }
    return FALSE;
}

BOOL MouseFilter::getClipCursor(RECT *rect) {
    assert(rect);

    if (!Process::isRemoteSession()) {
        return GetClipCursor(rect);
    }
    rect->left = 0;
    rect->top = 0;
    rect->right = 0;
    rect->bottom = 0;
    return FALSE;
}

} // namespace litelockr
