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

#ifndef FLYOUT_STATE_H
#define FLYOUT_STATE_H

#include "app/animation/AnimationFrameSet.h"
#include "app/LockState.h"
#include "app/event/AppEvent.h"
#include "app/NotificationArea.h"
#include "app/guard/UnlockGuardModel.h"
#include "app/AppParameters.h"
#include "app/FlyoutView.h"
#include "ini/SettingsData.h"
#include "lang/Messages.h"
#include "sys/Process.h"

namespace litelockr {

template<typename Window>
class FlyoutState {
public:
    FlyoutState() = default;

    Window& window() {
        return static_cast<Window&>(*this);
    }

    void changeLockState(int newLockState) {
        auto& model = window().model_;
        auto& view = window().view_;

        switch (newLockState) {
            case LockState::PRE_LOCKED:
                assert(model.lockState() == LockState::COUNTDOWN || model.lockState() == LockState::READY);
                model.setLockState(LockState::PRE_LOCKED);
                view.runLocking(false);
                view.draw();
                break;
            case LockState::LOCKED:
                assert(model.lockState() == LockState::PRE_LOCKED || model.lockState() == LockState::READY);
                if (model.lockState() == LockState::READY) {
                    model.setLockState(LockState::PRE_LOCKED);
                }
                model.setLockState(LockState::LOCKED);
                view.setScenePlaneLockVisible(true);
                view.draw();
                window().inputLocker_.lock();

                AppEvents::send(AppEvent::HIDE);
                AppEvents::send(AppEvent::CLOSE_SETTINGS);

                if (SettingsData::instance().showNotifications.value()) {
                    showBalloon(true);
                }
                if (SettingsData::instance().hideTrayIconWhenLocked.value()) {
                    window().hideTrayIconTimer_.setEnabled(true);
                }
                break;
            case LockState::PRE_READY:
                assert(model.lockState() == LockState::LOCKED);
                model.setLockState(LockState::PRE_READY);
                break;
            case LockState::READY:
                assert(model.lockState() == LockState::PRE_READY || model.lockState() == LockState::LOCKED);
                if (model.lockState() == LockState::LOCKED) {
                    model.setLockState(LockState::PRE_READY);
                }
                model.setLockState(LockState::READY);
                view.setScenePlaneLockVisible(true);
                view.draw();
                window().inputLocker_.unlock();

                window().hideTrayIconTimer_.setEnabled(false);
                if (!NotificationArea::iconExists()) {
                    NotificationArea::addIcon(LockIcon::Unlocked);
                }
                if (SettingsData::instance().showNotifications.value()) {
                    showBalloon(false);
                    AppEvents::send(AppEvent::HIDE);
                }
                break;
            case LockState::PRE_COUNTDOWN:
                assert(model.lockState() == LockState::READY);
                model.setLockState(LockState::PRE_COUNTDOWN);
                view.setScenePlaneLockVisible(true);
                model.lockCancel = true;
                break;
            case LockState::COUNTDOWN:
                assert(model.lockState() == LockState::READY || model.lockState() == LockState::PRE_COUNTDOWN);
                model.setLockState(LockState::COUNTDOWN);
                view.draw();
                view.runLocking(true);
                break;
            default:
                break;
        }
    }

    void doBeforeAnimation(const AnimationFrameSet& animation, bool /*isAbort*/) {
        auto& view = window().view_;

        switch (animation.id()) {
            case AnimationId::START_LOCKING:
                changeLockState(LockState::PRE_COUNTDOWN);
                break;
            case AnimationId::FINISH_LOCKING:
                changeLockState(LockState::PRE_LOCKED);
                break;
            case AnimationId::CANNOT_UNLOCK:
                assert(window().model_.lockState() == LockState::LOCKED);
                break;
            case AnimationId::UNLOCK:
                changeLockState(LockState::PRE_READY);
                break;
            case AnimationId::SHOW_GUARD:
                view.setGuardActiveState(UnlockGuardActiveState::PRE_ACTIVE);
                break;
            case AnimationId::HIDE_GUARD:
                view.setGuardActiveState(UnlockGuardActiveState::PRE_INACTIVE);
                break;
            default:
                break;
        }

        AppTimer::setAnimationActiveFor(static_cast<int>(animation.size()) * 32);
    }

    void doAfterAnimation(const AnimationFrameSet& animation, bool isAbort) {
        auto& model = window().model_;
        auto& view = window().view_;
        const auto& settings = SettingsData::instance();

        switch (animation.id()) {
            case AnimationId::STARTUP:
                if (AppParameters::lock) {
                    AppEvents::send(AppEvent::LOCK);
                } else if (AppParameters::lockNow || settings.lockOnStartup.value()) {
                    AppEvents::send(AppEvent::LOCK_NOW);
                } else if (AppParameters::hide) {
                    AppEvents::send(AppEvent::HIDE);
                }
                break;

            case AnimationId::SHOW_MENU:
                model.menuBar.visible = true;
                break;

            case AnimationId::HIDE_MENU:
                model.menuBar.visible = false;
                break;

            case AnimationId::HIDE_MAIN_WINDOW:
                view.resetGuard();
                break;

            case AnimationId::SHOW_MAIN_WINDOW:
                switch (animation.actionAfter()) {
                    case AnimationAction::SHAKE:
                        if (isAbort) {
                            return;
                        }
                        view.createShakeAnimation();
                        break;
                    case AnimationAction::LOCK:
                        changeLockState(LockState::COUNTDOWN);
                        break;
                    default:
                        // nothing to do
                        break;
                }
                break;

            case AnimationId::SHOW_GUARD:
                view.setGuardActiveState(UnlockGuardActiveState::ACTIVE);
                FlyoutView::setForeground(Process::mainWindow());
                break;
            case AnimationId::HIDE_GUARD:
                view.setGuardActiveState(UnlockGuardActiveState::INACTIVE);
                if (model.guard.inputState() == UnlockGuardModel::CORRECT) {
                    AppEvents::send(model.guard.appEvent);
                }
                break;

            case AnimationId::GUARD_WRONG:
                model.guard.resetWrongState();
                break;
            case AnimationId::START_LOCKING:
                changeLockState(LockState::COUNTDOWN);
                break;
            case AnimationId::FINISH_LOCKING:
                changeLockState(LockState::LOCKED);
                break;
            case AnimationId::CANNOT_UNLOCK:
                assert(model.lockState() == LockState::LOCKED);
                if (isAbort) {
                    return;
                }
                view.createShowGuardAnimation();
                break;
            case AnimationId::UNLOCK:
                changeLockState(LockState::READY);
                break;
            default:
                break;
        }
    }

    static void showBalloon(bool isLocked) {
        const auto& settings = SettingsData::instance();
        bool keyboard = settings.lockKeyboard.value();
        bool mouse = settings.lockMouse.value();

        MessageId id = MessageId::Null;

        if (keyboard && mouse) {
            id = isLocked ? MessageId::KeyboardAndMouseLocked : MessageId::KeyboardAndMouseUnlocked;
        } else if (keyboard) {
            id = isLocked ? MessageId::KeyboardLocked : MessageId::KeyboardUnlocked;
        } else if (mouse) {
            id = isLocked ? MessageId::MouseLocked : MessageId::MouseUnlocked;
        }

        if (id != MessageId::Null) {
            NotificationArea::showBalloon(isLocked, Messages::get(id));
        }
    }
};

} // namespace litelockr

#endif // FLYOUT_STATE_H
