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

#include "AppEventHandler.h"

namespace litelockr {

void AppEventHandler::initialize() {
    setEventHandler(AppEvent::EXIT, &AppEventHandler::exit);
    setEventHandler(AppEvent::EXIT_COMMAND, &AppEventHandler::exitCommand);
    setEventHandler(AppEvent::MENU, &AppEventHandler::menuBar);
    setEventHandler(AppEvent::SETTINGS, &AppEventHandler::settings);
    setEventHandler(AppEvent::CLOSE_SETTINGS, &AppEventHandler::closeSettings);
    setEventHandler(AppEvent::HELP, &AppEventHandler::help);
    setEventHandler(AppEvent::ABOUT, &AppEventHandler::about);
    setEventHandler(AppEvent::WEBSITE, &AppEventHandler::website);
    setEventHandler(AppEvent::WEBSITE_GITHUB, &AppEventHandler::websiteGithub);
    setEventHandler(AppEvent::LICENSE_INFORMATION, &AppEventHandler::licenseInformation);
    setEventHandler(AppEvent::KEYBOARD, &AppEventHandler::keyboard);
    setEventHandler(AppEvent::MOUSE, &AppEventHandler::mouse);
    setEventHandler(AppEvent::SHOW, &AppEventHandler::show);
    setEventHandler(AppEvent::SHOW_COMMAND, &AppEventHandler::show);
    setEventHandler(AppEvent::HIDE, &AppEventHandler::hide);
    setEventHandler(AppEvent::HIDE_COMMAND, &AppEventHandler::hide);
    setEventHandler(AppEvent::SHOW_LOCK_INFORMATION, &AppEventHandler::showLockInformation);
    setEventHandler(AppEvent::LOCK_INFORMATION_ENDED, &AppEventHandler::lockInformationEnded);

    setEventHandler(AppEvent::LOCK, &AppEventHandler::lock);
    setEventHandler(AppEvent::LOCK_COMMAND, &AppEventHandler::lockCommand);
    setEventHandler(AppEvent::LOCK_NOW, &AppEventHandler::lockNow);
    setEventHandler(AppEvent::LOCK_NOW_COMMAND, &AppEventHandler::lockNowCommand);
    setEventHandler(AppEvent::CANCEL_LOCKING, &AppEventHandler::cancelLocking);
    setEventHandler(AppEvent::UNLOCK, &AppEventHandler::unlock);
    setEventHandler(AppEvent::UNLOCK_COMMAND, &AppEventHandler::unlockCommand);
}

void AppEventHandler::exit(const AppEvent& event) {
    if (lockState() == LockState::COUNTDOWN) {
        stopLocking();
        appHide();
    }

    if (hidden() && isCheckRequired(AppEvent::EXIT)) {
        window().show(true, {.actionBefore = AnimationAction::EXIT, .animation = true});
        return;
    }

    if (checkAccess(event)) {
        appExit();
    }
}

void AppEventHandler::exitCommand(const AppEvent& /*event*/) {
    closeApplication();
}

void AppEventHandler::menuBar(const AppEvent&/*event*/) {
    if (view().isMenuBarVisible()) {
        view().createHideMenuAnimation();
    } else {
        view().createShowMenuAnimation();
    }
}

//
// Opens the settings dialog
//
void AppEventHandler::settings(const AppEvent& event) {
    if (lockState() == LockState::COUNTDOWN) {
        stopLocking();
    }

    if (checkAccess(event)) {
        window().settingsDlg().show();
        appHide();
        AppEvents::send(AppEvent::UNLOCK_COMMAND);
    }
}

//
// Closes the settings dialog when locked
//
void AppEventHandler::closeSettings(const AppEvent& /*event*/) {
    auto& dlg = window().settingsDlg();
    if (!dlg.isNull()) {
        window().settingsDlg().destroy();
    }
}

//
// Opens the documentation page
//
void AppEventHandler::help(const AppEvent& /*event*/) {
    if (lockState() == LockState::COUNTDOWN) {
        stopLocking();
    }
    clickHelp();
}

//
// Shows the About window
//
void AppEventHandler::about(const AppEvent& /*event*/) {
    if (lockState() == LockState::COUNTDOWN) {
        stopLocking();
    }

    window().aboutDlg().show();
    appHide();
}

void AppEventHandler::website(const AppEvent& /*event*/) {
    ShellExecute(nullptr, L"open", L"https://litelockr.org", nullptr, nullptr, SW_SHOW);
}

void AppEventHandler::websiteGithub(const AppEvent& /*event*/) {
    ShellExecute(nullptr, L"open", L"https://github.com/maxklvd/LiteLockr", nullptr, nullptr, SW_SHOW);
}

//
// Shows the License window
//
void AppEventHandler::licenseInformation(const AppEvent& /*event*/) {
    window().licenseDlg().show();
}

//
// Toggles the keyboard lock on and off
//
void AppEventHandler::keyboard(const AppEvent& /*event*/) {
    clickKeyboard();
}

//
// Toggles the mouse lock on and off
//
void AppEventHandler::mouse(const AppEvent& /*event*/) {
    clickMouse();
}

//
// Shows the main window
//
void AppEventHandler::show(const AppEvent& event) {
    bool animation = event.data;
    appShow(animation);
}

//
// Hides the main window
//
void AppEventHandler::hide(const AppEvent& event) {
    bool animation = event.data;
    appHide(animation);
}

//
// Shows the fullscreen preview window with locking information
//
void AppEventHandler::showLockInformation(const AppEvent& event) {
    auto& lockPreviewWnd = window().lockPreviewWnd();
    if (!lockPreviewWnd.active()) {
        lockPreviewWnd.show(event.data);
    }
}

void AppEventHandler::lockInformationEnded(const AppEvent& /*event*/) {
    window().settingsDlg().onLockPreviewEnded();
}

void AppEventHandler::lock(const AppEvent& /*event*/) {
    if (hidden()) {
        if (lockState() != LockState::READY) {
            return;
        }
        if (appSettings().delayBeforeLocking.value() > 0) {
            window().show(true, {.actionAfter = AnimationAction::LOCK, .animation = true});
        } else {
            window().changeLockState(LockState::LOCKED);
        }
        return;
    }

    if (model().guard.visible()) {
        return;
    }
    clickLock(false);
}

void AppEventHandler::lockCommand(const AppEvent& event) {
    if (hidden()) {
        if (lockState() != LockState::READY) {
            return;
        }
        view().resetGuard();
        lock(event);
        return;
    }

    if (view().getProgressBar().enabled()) {
        return; // locking is on progress
    }
    view().resetGuard();
    startLocking(false);
}

void AppEventHandler::lockNow(const AppEvent& event) {
    if (hidden()) {
        if (lockState() != LockState::READY) {
            return;
        }
        window().changeLockState(LockState::LOCKED);
        return;
    }

    if (event.source != AppEvent::SRC_HOTKEY && model().guard.visible()) {
        // guard enabled
        return;
    }
    clickLock(true);
}

void AppEventHandler::lockNowCommand(const AppEvent& event) {
    if (hidden()) {
        if (lockState() != LockState::READY) {
            return;
        }
        view().resetGuard();
        lockNow(event);
        return;
    }

    if (lockState() == LockState::COUNTDOWN) {
        stopLocking();
    }
    if (lockState() != LockState::READY) {
        return;
    }
    view().resetGuard();
    startLocking(true);
}

void AppEventHandler::cancelLocking(const AppEvent& /*event*/) {
    if (lockState() != LockState::COUNTDOWN) {
        return;
    }
    stopLocking();
}

void AppEventHandler::unlock(const AppEvent& event) {
    if (hidden()) {
        if (lockState() != LockState::LOCKED) {
            return;
        } else if (event.source == AppEvent::SRC_HOTKEY) {
            window().changeLockState(LockState::READY);
        } else if (FlyoutView::isProtected()) {
            // protected
            window().show(true, {.actionBefore = AnimationAction::UNLOCK, .animation = true});
        } else {
            window().changeLockState(LockState::READY);
        }
        return;
    }

    if (lockState() != LockState::LOCKED) {
        return;
    }
    if (event.source == AppEvent::SRC_HOTKEY) {
        view().resetGuard();
        clickUnlock();
    } else if (checkAccess(event)) {
        clickUnlock();
    }
}

void AppEventHandler::unlockCommand(const AppEvent& /*event*/) {
    if (hidden()) {
        if (lockState() != LockState::LOCKED) {
            return;
        }
        view().resetGuard();
        window().changeLockState(LockState::READY);
        return;
    }

    if (lockState() == LockState::COUNTDOWN) {
        stopLocking(false);
        return;
    }
    if (lockState() != LockState::LOCKED) {
        return;
    }
    view().resetGuard();
    clickUnlock();
}


void AppEventHandler::handleEvent(const AppEvent& event) {
    if (!canHandleEvent(event)) {
        return;
    }

    auto it = eventHandlers_.find(event.id);
    if (it != eventHandlers_.end()) {
        (this->*it->second)(event);
    } else {
        assert(false);
    }

}

void AppEventHandler::setEventHandler(int eventId, AppEventHandler::EventHandlerPtr handler) {
    eventHandlers_[eventId] = handler;
}

} // namespace litelockr
