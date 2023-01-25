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

#include "FlyoutWindow.h"

#include <commctrl.h>
#include <windowsx.h>
#include <wtsapi32.h>
#include "app/AppMessages.h"
#include "app/AppParameters.h"
#include "app/HotkeyHandler.h"
#include "app/NotificationArea.h"
#include "app/Sounds.h"
#include "gui/WindowUtils.h"
#include "lock/DisplayMonitors.h"
#include "lock/HookData.h"
#include "lock/MouseFilter.h"
#include "lock/hook/AbstractHook.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

namespace litelockr {

FlyoutWindow::FlyoutWindow() : view_(buffer_, model_) {
    onCommand(IDM_EXIT, []() { AppEvents::send(AppEvent::EXIT); });
    onCommand(IDM_LOCK, []() { AppEvents::send(AppEvent::LOCK); });
    onCommand(IDM_UNLOCK, []() { AppEvents::send(AppEvent::UNLOCK); });
    onCommand(IDM_SETTINGS, []() { AppEvents::send(AppEvent::SETTINGS); });
    onCommand(IDM_DOCUMENTATION, []() { AppEvents::send(AppEvent::HELP); });
    onCommand(IDM_WEBSITE, []() { AppEvents::send(AppEvent::WEBSITE); });
    onCommand(IDM_ABOUT, []() { AppEvents::send(AppEvent::ABOUT); });

    onMessage(WM_LBUTTONUP, &FlyoutWindow::onLButtonUp);
    onMessage(WM_LBUTTONDOWN, &FlyoutWindow::onLButtonDown);
    onMessage(WM_LBUTTONDBLCLK, &FlyoutWindow::onLButtonDblClk);
    onMessage(WM_RBUTTONUP, &FlyoutWindow::onRMouseUp);
    onMessage(WM_MOUSEMOVE, &FlyoutWindow::onMouseMove);
    onMessage(WM_KEYUP, &FlyoutWindow::onKeyUp);
    onMessage(WM_HOTKEY, [this](WPARAM wParam, LPARAM) {
        if (wParam == HotkeyHandler::instance().hotkeyId()) {
            onHotkeyPressed();
        }
    });
    onMessage(WM_CLOSE, []() { AppEvents::send(AppEvent::EXIT); });
    onMessage(WM_ACTIVATE, [this](WPARAM wParam, LPARAM) {
        if (LOWORD(wParam) == WA_INACTIVE) {
            hideWindowTimer_.setEnabled(true);
        }
    });
    onMessage(WM_WTSSESSION_CHANGE, &FlyoutWindow::onWTSessionChange);

    onMessage(WMU_SHOW_YOURSELF, &FlyoutWindow::showYourself);
    onMessage(WMU_LOCK_COMMAND, []() { AppEvents::send(AppEvent(AppEvent::LOCK_COMMAND)); });
    onMessage(WMU_LOCK_NOW_COMMAND, []() { AppEvents::send(AppEvent(AppEvent::LOCK_NOW_COMMAND)); });
    onMessage(WMU_UNLOCK_COMMAND, []() { AppEvents::send(AppEvent(AppEvent::UNLOCK_COMMAND)); });
    onMessage(WMU_HIDE_COMMAND, []() { AppEvents::send(AppEvent(AppEvent::HIDE_COMMAND)); });
    onMessage(WMU_SHOW_COMMAND, []() { AppEvents::send(AppEvent(AppEvent::SHOW_COMMAND)); });
    onMessage(WMU_EXIT_COMMAND, []() { AppEvents::send(AppEvent(AppEvent::EXIT_COMMAND)); });
    onMessage(WMU_TRAY_ICON_NOTIFY, &FlyoutWindow::onTrayIconNotify);
    onMessage(WMU_TRAY_ICON_UPDATE, [this]() { NotificationArea::updateIcon(model_.lockIcon()); });

    onMessage(WM_SETTINGCHANGE, [this]() { onSettingChange(); });
    onMessage(WM_DISPLAYCHANGE, [this]() { displayChangeDelay_.setEnabled(true); });
}

void FlyoutWindow::create(HWND hWndParent /*= nullptr*/) {
    HICON hAppIcon = nullptr;
    LoadIconMetric(Process::moduleInstance(), MAKEINTRESOURCE(IDI_APP_ICON), LIM_SMALL, &hAppIcon);
    WindowUtils::registerClass(WINDOW_CLASS, FlyoutWindow::windowProcStatic, CS_DBLCLKS, hAppIcon);

    className_ = WINDOW_CLASS;
    windowName_ = WINDOW_TITLE;
    createWindow(hWndParent);

    assert(hWnd);
    WTSRegisterSessionNotification(hWnd, NOTIFY_FOR_THIS_SESSION);
}

void FlyoutWindow::dispose() {
    assert(hWnd);
    WTSUnRegisterSessionNotification(hWnd);
}

void FlyoutWindow::tick() {
    inputLocker_.tick();

    if (hideWindowTimer_.timeout()) {
        hideWindowTimer_.setEnabled(false);
        AppEvents::send(AppEvent::HIDE);
    }
    if (hideTrayIconTimer_.timeout()) {
        hideTrayIconTimer_.setEnabled(false);
        NotificationArea::deleteIcon();
    }
    if (displayChangeDelay_.timeout()) {
        displayChangeDelay_.setEnabled(false);
        onDisplayChange();
    }
    if (model_.locked() && inactivityTimer_.timeout()) {
        inactivityTimer_.setEnabled(false);
        AppEvents::send(AppEvent::HIDE);
    }

    //
    // Updates the systray icon
    //
    static LockIcon prevLockIcon = LockIcon::Unlocked;
    auto lockIcon = model_.lockIcon();
    if (lockIcon != prevLockIcon) {
        prevLockIcon = lockIcon;
        NotificationArea::updateIcon(lockIcon);
    }

    if (view_.needsUpdate()) {
        markedForUpdate_ = true;
    }
    if (view_.drawAnimation()) {
        markedForUpdate_ = true;
    }
    if (view_.isGuardVisible() && model_.guard.isExpired()) {
        markedForUpdate_ = true;
        view_.createHideGuardAnimation();
    }
    if (view_.updateButtonOpacity()) {
        markedForUpdate_ = true;
    }

    checkMouseOut();
    update();
    doAfterWindowUpdate();
}

void FlyoutWindow::update() {
    if (!view_.hasInitialized()) {
        return;
    }

    if (markedForUpdate_) {
        markedForUpdate_ = false;
        updateWindow();
    }
}

void FlyoutWindow::show(bool visible, const ShowOptions& options) {
    if (model_.visible() != visible) {
        model_.setVisible(visible);

        if (FlyoutView::isProtected()) {
            view_.checkAction(options.actionBefore);
        }

        if (options.animation) {
            if (visible) {
                view_.createShowFlyoutAnimation(options.actionBefore, options.actionAfter);
            } else {
                view_.createHideFlyoutAnimation(options.actionBefore, options.actionAfter);
            }
        } else {
            assert(options.actionBefore == AnimationAction::NONE);
            assert(options.actionAfter == AnimationAction::NONE);
            if (!visible) {
                view_.resetGuard();
            }
            redraw();
        }
    }

    inactivityTimer_.setEnabled(visible);
}

void FlyoutWindow::showYourself() {
    if (!NotificationArea::iconExists()) {
        NotificationArea::addIcon(model_.lockIcon());
        hideTrayIconTimer_.setEnabled(true);
    }

    if (model_.visible()) {
        view_.createShakeAnimation();
    } else {
        show(true, {.actionAfter = AnimationAction::SHAKE, .animation = true});
    }
}

void FlyoutWindow::startup() {
    setWindowPosition();
    model_.setVisible(true);
    if (AppParameters::autoRun || AppParameters::hide) {
        // skip the startup animation
        show(false, {.animation = false});
        doAfterAnimation(view_.resources().startupAnimation, false);
    } else {
        view_.runStartupAnimation();
    }
}

void FlyoutWindow::redraw() {
    view_.draw();
    markedForUpdate_ = true;
}


void FlyoutWindow::onCreate() {
    Process::setMainWindow(hWnd);
    DisplayMonitors::update();
    updateLightMode();

    model_.initialize();
    view_.initialize(hWnd);
    view_.setBeforeAnimationCallback([this](const AnimationFrameSet& prop, bool isAbort) {
        doBeforeAnimation(prop, isAbort);
    });
    view_.setAfterAnimationCallback([this](const AnimationFrameSet& prop, bool isAbort) {
        doAfterAnimation(prop, isAbort);
    });

    model_.pack();
}

void FlyoutWindow::setWindowPosition() {
    POINT position{};
    RECT rcIcon = NotificationArea::getIconRect();
    if (!Rectangle::empty(rcIcon)) {
        POINT anchor = Rectangle::center(rcIcon);

        RECT rcWindow = model_.workArea();
        SIZE sizeWindow = {Rectangle::width(rcWindow), Rectangle::height(rcWindow)};

        if (CalculatePopupWindowPosition(&anchor, &sizeWindow,
                                         TPM_VERTICAL | TPM_VCENTERALIGN | TPM_CENTERALIGN | TPM_WORKAREA,
                                         &rcIcon, &rcWindow)) {
            position.x = rcWindow.left - FlyoutModel::MARGIN;
            position.y = rcWindow.top - FlyoutModel::MARGIN;
        }
    }

    setPosition(position);
}

void FlyoutWindow::showContextMenu(POINT pt, ContextMenuId menuId) {
    HMENU hMenu = createContextMenu(menuId);
    if (hMenu) {
        SetForegroundWindow(Process::mainWindow());
        TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON,
                       pt.x, pt.y, 0, Process::mainWindow(), nullptr);

        DestroyMenu(hMenu);
    }
}

HMENU FlyoutWindow::createContextMenu(ContextMenuId menuId) {
    if (menuId != ContextMenuId::NOTIFICATION_AREA && view_.isAnimationActive()) {
        return nullptr;
    }

    HMENU hMenu = CreatePopupMenu();

    auto hotkey = HotkeyHandler::instance().hotkeyFormatted();

    if (model_.lockState() == LockState::LOCKED) {
        auto s = Messages::get(MessageId::MenuUnlock);
        s += L"\t";
        s += hotkey;

        AppendMenu(hMenu, MF_STRING, IDM_UNLOCK, s.c_str());
    } else {
        auto s = Messages::get(MessageId::MenuLock);
        s += L"\t";
        s += hotkey;

        AppendMenu(hMenu, MF_STRING, IDM_LOCK, s.c_str());
    }
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

    if (menuId == ContextMenuId::NOTIFICATION_AREA) {
        AppendMenu(hMenu, MF_STRING, IDM_SETTINGS, Messages::get(MessageId::MenuSettings).c_str());

        HMENU hHelpMenu = CreatePopupMenu();
        AppendMenu(hHelpMenu, MF_STRING, IDM_DOCUMENTATION, Messages::get(MessageId::MenuOnlineDocumentation).c_str());
        AppendMenu(hHelpMenu, MF_STRING, IDM_WEBSITE, Messages::get(MessageId::MenuWebSite).c_str());
        AppendMenu(hHelpMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenu(hHelpMenu, MF_STRING, IDM_ABOUT, Messages::get(MessageId::MenuAbout).c_str());

        AppendMenu(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hHelpMenu), Messages::get(MessageId::MenuHelp).c_str());
        AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
        if (model_.lockState() == LockState::LOCKED) {
            EnableMenuItem(hMenu, reinterpret_cast<UINT_PTR>(hHelpMenu), MF_DISABLED | MF_GRAYED);
        }
    }

    AppendMenu(hMenu, MF_STRING, IDM_EXIT, Messages::get(MessageId::MenuExit).c_str());

    if (model_.lockState() == LockState::DISABLED || Process::isSettingsDialogOpen()) {
        EnableMenuItem(hMenu, IDM_LOCK, MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_UNLOCK, MF_DISABLED | MF_GRAYED);
    }
    if (model_.lockState() == LockState::LOCKED) {
        EnableMenuItem(hMenu, IDM_SETTINGS, MF_DISABLED | MF_GRAYED);
    }
    if (model_.lockCancel) {
        EnableMenuItem(hMenu, IDM_LOCK, MF_DISABLED | MF_GRAYED);
    }
    if (model_.guard.visible()) {
        EnableMenuItem(hMenu, IDM_LOCK, MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_UNLOCK, MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_EXIT, MF_DISABLED | MF_GRAYED);
    }
    return hMenu;
}

void FlyoutWindow::onClick(int buttonId, WPARAM wParam) const {
    if (buttonId == FlyoutModel::CLOSE) {
        AppEvents::send(AppEvent(AppEvent::EXIT));
    } else if (buttonId == FlyoutModel::MENU) {
        AppEvents::send(AppEvent(AppEvent::MENU));
    } else if (buttonId == FlyoutModel::LOCK) {
        if (model_.lockState() == LockState::READY) {
            if (wParam & MK_CONTROL) { // ctrl is down
                AppEvents::send(AppEvent(AppEvent::LOCK_NOW));
            } else {
                AppEvents::send(AppEvent(AppEvent::LOCK));
            }
        } else if (model_.lockState() == LockState::COUNTDOWN) {
            AppEvents::send(AppEvent(AppEvent::CANCEL_LOCKING));
        } else if (model_.lockState() == LockState::LOCKED) {
            AppEvents::send(AppEvent(AppEvent::UNLOCK));
        }
    } else if (buttonId == FlyoutModel::KEYBOARD) {
        AppEvents::send(AppEvent(AppEvent::KEYBOARD));
    } else if (buttonId == FlyoutModel::MOUSE) {
        AppEvents::send(AppEvent(AppEvent::MOUSE));
    } else if (buttonId == FlyoutModel::OPT_SETTINGS) {
        AppEvents::send(AppEvent(AppEvent::SETTINGS));
    } else if (buttonId == FlyoutModel::OPT_HELP) {
        AppEvents::send(AppEvent(AppEvent::HELP));
    } else if (buttonId == FlyoutModel::OPT_ABOUT) {
        AppEvents::send(AppEvent(AppEvent::ABOUT));
    }
}

void FlyoutWindow::onHotkeyPressed() {
    if (inputLocker_.isHotkeyRepeated()) {
        return;
    }
    inputLocker_.updateHotkeyTime();

    if (!Process::isSettingsDialogOpen()) {
        if (model_.lockState() == LockState::READY) {
            AppEvents::remove(AppEvent(AppEvent::LOCK));
            AppEvents::send(AppEvent(AppEvent::LOCK_NOW, AppEvent::SRC_HOTKEY));
        } else if (model_.lockState() == LockState::COUNTDOWN) {
            AppEvents::send(AppEvent(AppEvent::CANCEL_LOCKING, AppEvent::SRC_HOTKEY));
        } else if (model_.lockState() == LockState::LOCKED) {
            AppEvents::send(AppEvent(AppEvent::UNLOCK, AppEvent::SRC_HOTKEY));
        } else {
            Sounds::play(IDB_SND_WRONG);
        }
    }
}

void FlyoutWindow::onMouseOut() {
    mouseInside_ = false;

    for (PanelComponent& c: model_.components) {
        c.hover = false;
    }
    model_.buttonHover = false;

    if (!view_.isAnimationActive()) {
        view_.draw();
        markedForUpdate_ = true;
    }
}

void FlyoutWindow::checkMouseOut() {
    static int prevX = INT_MIN;
    static int prevY = INT_MIN;

    if (mouseInside_) {
        POINT pos;
        GetCursorPos(&pos);
        RECT wa = model_.workArea();
        Rectangle::offset(wa, position_);
        BOOL in = PtInRect(&wa, pos);

        if (!in && (pos.x != prevX || pos.y != prevY)) {
            prevX = pos.x;
            prevY = pos.y;
            onMouseOut();
        }
    }
}

void FlyoutWindow::onLButtonUp(WPARAM wParam, LPARAM lParam) {
    inactivityTimer_.reset();

    auto x = GET_X_LPARAM(lParam);
    auto y = GET_Y_LPARAM(lParam);

    mouseInside_ = true;

    bool needUpdate = false;

    if (!model_.guard.visible() && model_.buttonPressed.has_value()) {
        const auto& button = model_.buttonPressed.value().get();
        if (button.visible) {
            if (button.contains(x, y, view_.isMenuBarVisible())) {
                onClick(button.id, wParam);
            }
            needUpdate = true;
        }
    }
    model_.buttonPressed = {};

    for (PanelComponent& c: model_.components) {
        c.pressed = false;
    }

    if (needUpdate) {
        redraw();
    }
}

void FlyoutWindow::onLButtonDown(WPARAM /*wParam*/, LPARAM lParam) {
    inactivityTimer_.reset();

    auto x = GET_X_LPARAM(lParam);
    auto y = GET_Y_LPARAM(lParam);

    mouseInside_ = true;

    if (view_.isAnimationActive()) {
        return;
    }

    bool needUpdate = false;

    for (PanelComponent& c: model_.components) {
        c.pressed = false;
    }

    model_.buttonPressed = {};
    for (PanelComponent& c: model_.components) {
        if (c.visible && c.contains(x, y, view_.isMenuBarVisible())) {
            c.pressed = true;
            if (c.isButton()) {
                model_.buttonPressed = static_cast<PanelButton&>(c);
            }
            needUpdate = true;
            break;
        }
    }

    if (needUpdate) {
        redraw();
    }
}

void FlyoutWindow::onLButtonDblClk(WPARAM /*wParam*/, LPARAM lParam) {
    if (view_.isAnimationActive()) {
        return;
    }

    auto x = GET_X_LPARAM(lParam);
    auto y = GET_Y_LPARAM(lParam);

    mouseInside_ = true;

    if (model_.visible()) {
        if (model_.guard.visible()) {
            AppEvent event(AppEvent::HIDE);
            event.data = true; // animation parameter
            AppEvents::send(event);
            return;
        }

        bool buttonFound = false;

        for (const PanelComponent& c: model_.components) {
            if (c.visible &&
                c.contains(x, y, view_.isMenuBarVisible()) &&
                c.isButton()) {
                buttonFound = true;
                break;
            }
        }

        if (!buttonFound) {
            AppEvent event(AppEvent::HIDE);
            event.data = true; // animation parameter
            AppEvents::send(event);
        }
    } else {
        AppEvent event(AppEvent::SHOW);
        event.data = true; // animation parameter
        AppEvents::send(event);
    }
}

void FlyoutWindow::onRMouseUp(WPARAM /*wParam*/, LPARAM lParam) {
    inactivityTimer_.reset();

    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ClientToScreen(Process::mainWindow(), &pt);
    showContextMenu(pt, ContextMenuId::MAIN_WINDOW);
}

void FlyoutWindow::onMouseMove(WPARAM /*wParam*/, LPARAM lParam) {
    inactivityTimer_.reset();

    auto x = GET_X_LPARAM(lParam);
    auto y = GET_Y_LPARAM(lParam);

    mouseInside_ = true;

    if (view_.isAnimationActive()) {
        return;
    }
    if (model_.guard.visible()) {
        return;
    }

    if (!model_.buttonPressed.has_value()) {
        int prevId = INT_MIN;
        bool needUpdate = false;

        for (PanelComponent& c: model_.components) {
            if (c.hover) {
                prevId = c.id;
                needUpdate = true;
            }
            c.hover = false;
        }
        for (PanelComponent& c: model_.components) {
            if (c.visible && c.contains(x, y, view_.isMenuBarVisible())) {
                needUpdate = (c.id != prevId);
                c.hover = true;
                break;
            }
        }

        if (needUpdate) {
            redraw();
        }
    }

    updateButtonHoverState(x, y);

    if (markedForUpdate_) {
        markedForUpdate_ = false;
        updateWindow();
    }
}

void FlyoutWindow::onKeyUp(WPARAM wParam, LPARAM /*lParam*/) {
    inactivityTimer_.reset();

    if (!view_.isAnimationActive() && view_.isGuardVisible()) {
        if (wParam == VK_ESCAPE) {
            AppEvents::send(AppEvent::HIDE);
            return;
        }
        model_.guard.onKeyUp(wParam);
        view_.draw();
        markedForUpdate_ = true;
        update();

        if (model_.guard.inputState() == UnlockGuardModel::WRONG) {
            fireGuardWrongAnimation_ = true;
        } else if (model_.guard.inputState() == UnlockGuardModel::CORRECT
                   || model_.guard.inputState() == UnlockGuardModel::CANCEL) {
            if (model_.guard.appEvent.id == AppEvent::EXIT) {
                fireGuardExitCommand_ = true; // means to exit immediately, no 'hide' animation needed
            } else {
                fireGuardHideAnimation_ = true;
            }
        }
    } else {
        switch (wParam) {
            case VK_ESCAPE:
                if (model_.lockState() == LockState::COUNTDOWN) {
                    AppEvents::send(AppEvent::CANCEL_LOCKING);
                } else {
                    AppEvents::send(AppEvent::HIDE);
                }
                break;
            case VK_F1:
                AppEvents::send(AppEvent::HELP);
                break;
            default:
                break;
        }
    }
}

void FlyoutWindow::onWTSessionChange(WPARAM wParam, LPARAM /*lParam*/) {
    if (SettingsData::instance().unlockOnSessionChange.value()) {
        LOG_DEBUG(L"[WTSSESSION_CHANGE] Sending UNLOCK event");
        AppEvent event(AppEvent::UNLOCK, AppEvent::SRC_HOTKEY);
        AppEvents::send(event);
    }

    Process::updateRemoteSession();

    switch (wParam) {
        case WTS_CONSOLE_CONNECT:// (0x1)
        case WTS_REMOTE_CONNECT:// (0x3)
        case WTS_SESSION_LOGON:// (0x5)
        case WTS_SESSION_UNLOCK:// (0x8)
            //connect, resume blocking
            HookData::setSessionActive(true);
            LOG_DEBUG(L"[WTSSESSION_CHANGE] Change the sessionActive value to true");
            MouseFilter::updatePositionValidator(1000);
            break;

        case WTS_CONSOLE_DISCONNECT:// (0x2)
        case WTS_REMOTE_DISCONNECT:// (0x4)
        case WTS_SESSION_LOGOFF:// (0x6)
        case WTS_SESSION_LOCK:// (0x7)
            //disconnect, no blocking
            HookData::setSessionActive(false);
            LOG_DEBUG(L"[WTSSESSION_CHANGE] Change the sessionActive value to false");
            // no need to update the map
            break;

        default:
            //just update the map
            MouseFilter::updatePositionValidator(1000);
            break;
    }
}

void FlyoutWindow::onTrayIconNotify(WPARAM /*wParam*/, LPARAM lParam) {
    switch (LOWORD(lParam)) {
        case NIN_SELECT:
            hideWindowTimer_.setEnabled(false);

            setWindowPosition();
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOMOVE);
            SetForegroundWindow(hWnd);

            if (model_.visible()) {
                AppEvent event(AppEvent::HIDE);
                event.data = true; // animation parameter
                AppEvents::send(event);
            } else {
                AppEvent event(AppEvent::SHOW);
                event.data = true; // animation parameter
                AppEvents::send(event);
            }
            break;
        case WM_CONTEXTMENU:
            onTrayIconContextMenu();
            break;
        default:
            break;
    }
}

void FlyoutWindow::onTrayIconContextMenu() {
    hideWindowTimer_.setEnabled(false);

    POINT pos;
    GetCursorPos(&pos);
    RECT rc = NotificationArea::getIconRect();
    if (!Rectangle::empty(rc) && !Rectangle::contains(rc, pos)) {
        pos = Rectangle::center(rc);
    }
    showContextMenu(pos, ContextMenuId::NOTIFICATION_AREA);
}

void FlyoutWindow::onSettingChange() const {
    bool prevValue = Process::isLightMode();
    updateLightMode();
    if (prevValue != Process::isLightMode()) {
        NotificationArea::updateIcon(model_.lockIcon());
    }
}

void FlyoutWindow::updateLightMode() const {
    const auto& settings = SettingsData::instance();

    switch (settings.lightMode.value()) {
        case SettingsData::LightMode::ON:
            Process::setLightMode(true);
            break;
        case SettingsData::LightMode::OFF:
            Process::setLightMode(false);
            break;
        default:
            bool isLightMode = false;
            DWORD value = 0;
            DWORD size = sizeof(DWORD);
            if (RegGetValue(
                    HKEY_CURRENT_USER,
                    L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                    L"SystemUsesLightTheme",
                    RRF_RT_REG_DWORD,
                    nullptr,
                    &value,
                    &size) == ERROR_SUCCESS) {
                isLightMode = (value == 1);
            }
            Process::setLightMode(isLightMode);
    }
}

void FlyoutWindow::onDisplayChange() const {
    DisplayMonitors::update();
    AbstractHook::setDisplayChanged(true);
}

void FlyoutWindow::doAfterWindowUpdate() {
    if (fireGuardWrongAnimation_) {
        fireGuardWrongAnimation_ = false;
        view_.createGuardWrongAnimation();
    }
    if (fireGuardHideAnimation_) {
        fireGuardHideAnimation_ = false;
        view_.createHideGuardAnimation();
    }
    if (fireGuardExitCommand_) {
        fireGuardExitCommand_ = false;
        Sleep(200); // pause to see the last digit entered by user
        view_.resetGuard();
        AppEvents::send(AppEvent::EXIT_COMMAND);
    }
}

void FlyoutWindow::updateButtonHoverState(int x, int y) {
    if (view_.isMenuBarVisible()) {
        return;
    }

    model_.buttonHover = false;
    for (const PanelComponent& c: model_.components) {
        if ((c.id == FlyoutModel::LOCK ||
             c.id == FlyoutModel::KEYBOARD ||
             c.id == FlyoutModel::MOUSE) &&
            c.contains(x, y, view_.isMenuBarVisible())) {
            model_.buttonHover = true;
            break;
        }
    }

    if (view_.updateButtonOpacity()) {
        markedForUpdate_ = true;
    }
}

} // namespace litelockr
