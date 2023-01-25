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

#include "BaseEventHandler.h"

#include <cassert>
#include <string>

#include "app/HotkeyHandler.h"
#include "app/NotificationArea.h"
#include "app/Sounds.h"
#include "lock/HookThread.h"
#include "log/Logger.h"
#include "sys/Process.h"
#include "sys/Time.h"

namespace litelockr {

void BaseEventHandler::appExit() const {
    std::wstring title = Messages::get(MessageId::ConfirmExit);
    title += L" - ";
    title += APP_NAME;

    int answer = MessageBox(Process::mainWindow(),
                            Messages::get(MessageId::ConfirmExitMessage, APP_NAME).c_str(),
                            title.c_str(),
                            MB_YESNO | MB_ICONQUESTION);

    if (answer == IDYES) {
        closeApplication();
    }
}

void BaseEventHandler::closeApplication() const {
    appSettings().save(nullptr);
    HotkeyHandler::instance().free();
    NotificationArea::deleteIcon();
    window().dispose();
    LOG_DEBUG(L"Closing the application");
    PostQuitMessage(0);
}


bool BaseEventHandler::isIdle() {
    DWORD lastInputTime;
    if (Time::getLastInputTime(lastInputTime)) {
        DWORD now = Time::getTime();

        if (lastInputTime <= now) {
            DWORD lockWhenIdle = appSettings().lockWhenIdle.value() * 60 * 1000; // to milliseconds
            DWORD duration = now - lastInputTime;
            return lockWhenIdle > 0 && duration > lockWhenIdle;
        }
    }
    return false;
}

void BaseEventHandler::timeout() {
    HookThread::currentAppSelection().tick();

    if (checkInterval_.timeout()) {
        //
        // LockWhenIdle option
        //
        if (appSettings().lockWhenIdle.value() > 0 && isIdle()) {
            if (!lockWhenIdleFired_) {
                lockWhenIdleFired_ = true;
                AppEvents::send(AppEvent::LOCK_NOW_COMMAND);
            }
        } else {
            lockWhenIdleFired_ = false;
        }
    }

    if (LockStateChecker::update()) {
        static bool prevAvailable = true;
        bool available = LockStateChecker::lockAvailable();
        if (prevAvailable != available) {
            prevAvailable = available;
            NotificationArea::updateIcon(model().lockIcon());
        }
    }

    if (model().guard.visible()) {
        return;
    }

    auto& locking = view().getProgressBar();
    if (locking.enabled() || locking.stopping()) {
        locking.tick();
        window().redraw();

        if (locking.done()) {
            locking.reset();
            view().runFinishLockingAnimation(false);
        }
    }
}

bool BaseEventHandler::isCheckRequired(int appEventId) const {
    bool checkRequired = appSettings().preventUnlockingInput.value();
    if (checkRequired && (appEventId == AppEvent::EXIT || appEventId == AppEvent::SETTINGS)) {
        return model().locked();
    }
    return checkRequired;
}

bool BaseEventHandler::checkAccess(const AppEvent& event) {
    if (isCheckRequired(event.id)) {
        auto& mlock = model().guard;

        if (mlock.appEvent.uid == event.uid) {
            // continue
            mlock.appEvent = AppEvent(AppEvent::NONE);
            return true;
        } else if (mlock.appEvent.id == AppEvent::NONE) {
            // run
            mlock.appEvent = event;
            mlock.initialize();

            if (event.id == AppEvent::UNLOCK) {
                view().runCannotUnlockAnimation();
            } else {
                view().createShowGuardAnimation();
            }
        }
        return false;
    }

    return true;
}

bool BaseEventHandler::canHandleEvent(const AppEvent& event) const {
    auto lockState = model().lockState();

    switch (event.id) {
        case AppEvent::LOCK:
        case AppEvent::LOCK_NOW:
        case AppEvent::LOCK_COMMAND:
        case AppEvent::LOCK_NOW_COMMAND:
            if (lockState != LockState::READY) {
                return false;
            }
            break;

        case AppEvent::CANCEL_LOCKING:
            if (lockState != LockState::COUNTDOWN) {
                return false;
            }
            break;

        case AppEvent::UNLOCK:
        case AppEvent::UNLOCK_COMMAND:
            if (lockState != LockState::LOCKED) {
                return false;
            }
            break;

        default:
            // nothing to do
            break;
    }
    return true;
}

void BaseEventHandler::clickKeyboard() {
    view().stopCurrentAnimation();
    if (model().lockState() != LockState::DISABLED
        && model().lockState() != LockState::READY
        && model().lockState() != LockState::COUNTDOWN) {
        return;
    }

    auto& m = model();
    m.keyboard.checked = !m.keyboard.checked; // inverse
    bool enabled = !(m.keyboard.checked && m.mouse.checked);
    m.setLockState(enabled ? LockState::READY : LockState::DISABLED);
    m.lockCancel = false;
    view().updateButtonMaterial();
    view().getProgressBar().stop();
    if (m.lockState() == LockState::DISABLED) {
        Sounds::play(IDB_SND_WRONG);
    }

    appSettings().lockKeyboard.setValue(!m.keyboard.checked);
    window().redraw();
}

void BaseEventHandler::clickMouse() {
    view().stopCurrentAnimation();
    if (model().lockState() != LockState::DISABLED
        && model().lockState() != LockState::READY
        && model().lockState() != LockState::COUNTDOWN) {
        return;
    }

    auto& m = model();
    m.mouse.checked = !m.mouse.checked;
    bool enabled = !(m.keyboard.checked && m.mouse.checked);
    m.setLockState(enabled ? LockState::READY : LockState::DISABLED);
    m.lockCancel = false;
    view().updateButtonMaterial();
    view().getProgressBar().stop();
    if (m.lockState() == LockState::DISABLED) {
        Sounds::play(IDB_SND_WRONG);
    }

    appSettings().lockMouse.setValue(!m.mouse.checked);
    window().redraw();
}

void BaseEventHandler::stopLocking(bool needRedraw/* = true*/) {
    assert(model().lockState() == LockState::COUNTDOWN);

    auto& m = model();
    auto& locking = view().getProgressBar();
    if (locking.enabled()) {
        //stop locking
        m.setLockState(LockState::READY);

        Sounds::play(IDB_SND_CANCEL);
        m.lockCancel = false;
        locking.stop();
        if (needRedraw) {
            window().redraw();
        }
    }
}

void BaseEventHandler::startLocking(bool lockNow) {
    assert(model().lockState() == LockState::READY);

    if (appSettings().delayBeforeLocking.value() == 0) {
        lockNow = true;
    }

    if (lockNow) {
        // lock immediately
        view().runFinishLockingAnimation(lockNow);
    } else {
        view().runStartLockingAnimation();
    }
}

void BaseEventHandler::clickLock(bool lockNow) {
    if (model().lockState() != LockState::READY) {
        return;
    }

    startLocking(lockNow);
}

void BaseEventHandler::clickUnlock() {
    if (model().lockState() != LockState::LOCKED) {
        return;
    }

    view().runUnlockAnimation();
}

void BaseEventHandler::clickHelp() {
    using namespace std::literals::string_literals;
    std::wstring helpLink = L"https://litelockr.org/help/?lang="s + appSettings().language.value();
    ShellExecute(nullptr, L"open", helpLink.c_str(), nullptr, nullptr, SW_SHOW);
}

void BaseEventHandler::appHide(bool animation) {
    auto& m = model();
    if (!m.lockCancel) { // don't allow to hide while the locking is in progress
        window().show(false, {.animation = animation});
    }
}

void BaseEventHandler::appShow(bool animation) {
    window().show(true, {.animation = animation});
}

FlyoutWindow& BaseEventHandler::window() const {
    return window_;
}

FlyoutModel& BaseEventHandler::model() {
    return window_.model_;
}

const FlyoutModel& BaseEventHandler::model() const {
    return window_.model_;
}

FlyoutView& BaseEventHandler::view() const {
    return window_.view_;
}

} // namespace litelockr
