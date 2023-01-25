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

#ifndef FLYOUT_WINDOW_H
#define FLYOUT_WINDOW_H

#include "app/FlyoutDialogs.h"
#include "app/FlyoutModel.h"
#include "app/FlyoutState.h"
#include "app/FlyoutView.h"
#include "app/Version.h"
#include "app/animation/AnimationFrameSet.h"
#include "dlg/AboutDlg.h"
#include "dlg/LicenseDlg.h"
#include "dlg/SettingsDlg.h"
#include "gui/LayeredWindow.h"
#include "lock/InputLocker.h"
#include "lock/ui/LockPreviewWnd.h"
#include "sys/AppClock.h"
#include "sys/AppTimer.h"

namespace litelockr {

class FlyoutWindow: public FlyoutState<FlyoutWindow>,
                    public FlyoutDialogs,
                    public LayeredWindow {
public:
    constexpr static auto WINDOW_CLASS = L"LLClass";
    constexpr static auto WINDOW_TITLE = APP_NAME;

    FlyoutWindow();

    void create(HWND hWndParent = nullptr);
    void dispose();
    void tick();
    void update();

    struct ShowOptions {
        AnimationAction actionBefore = AnimationAction::NONE;
        AnimationAction actionAfter = AnimationAction::NONE;
        bool animation = false;
    };
    void show(bool visible, const ShowOptions& options);

    void showYourself();
    void startup();

    void redraw();

private:
    enum class ContextMenuId {
        NOTIFICATION_AREA,
        MAIN_WINDOW,
    };

    void onCreate() override;
    void setWindowPosition();

    void showContextMenu(POINT pt, ContextMenuId menuId);
    HMENU createContextMenu(ContextMenuId menuId);

    void onClick(int buttonId, WPARAM wParam) const;
    void onHotkeyPressed();
    void onMouseOut();
    void checkMouseOut();

    void onLButtonUp(WPARAM wParam, LPARAM lParam);
    void onLButtonDown(WPARAM wParam, LPARAM lParam);
    void onLButtonDblClk(WPARAM wParam, LPARAM lParam);
    void onRMouseUp(WPARAM wParam, LPARAM lParam);
    void onMouseMove(WPARAM wParam, LPARAM lParam);
    void onKeyUp(WPARAM wParam, LPARAM lParam);
    void onWTSessionChange(WPARAM wParam, LPARAM lParam);
    void onTrayIconNotify(WPARAM wParam, LPARAM lParam);
    void onTrayIconContextMenu();
    void onSettingChange() const;
    void onDisplayChange() const;
    void updateLightMode() const;

    void doAfterWindowUpdate();

    void updateButtonHoverState(int x, int y);

    bool fireGuardWrongAnimation_ = false;
    bool fireGuardHideAnimation_ = false;
    bool fireGuardExitCommand_ = false;

    TimeCounter hideWindowTimer_{std::chrono::milliseconds(GetDoubleClickTime()), false};
    TimeCounter hideTrayIconTimer_{std::chrono::milliseconds(1500), false};
    TimeCounter displayChangeDelay_{std::chrono::milliseconds(1500), false};
    TimeCounter inactivityTimer_{std::chrono::seconds(15), false};

    bool markedForUpdate_ = false;
    bool mouseInside_ = false;

    InputLocker inputLocker_;

    FlyoutModel model_;
    FlyoutView view_;

    friend class BaseEventHandler;

    friend class FlyoutState;
};

class FlyoutAppTimerListener: public DefaultAppTimerListener<FlyoutWindow> {
};

} // namespace litelockr

#endif // FLYOUT_WINDOW_H
