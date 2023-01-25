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

#ifndef FLYOUT_MODEL_H
#define FLYOUT_MODEL_H

#include <functional>
#include <optional>
#include <vector>

#include <windows.h>
#include "app/LockState.h"
#include "app/guard/UnlockGuardModel.h"
#include "app/PanelComponent.h"
#include "gfx/Color.h"
#include "lock/hook/LockStateChecker.h"

namespace litelockr {

class FlyoutModel {
public:
    constexpr static int MARGIN = 8; // needs some extra space for Shake animation

    enum ButtonId {
        CLOSE = 1,
        MENU,
        LOCK,
        OPT_SETTINGS,
        OPT_HELP,
        OPT_ABOUT,
        KEYBOARD,
        MOUSE,
    };

    void initialize();
    void pack();

    [[nodiscard]] RECT workArea() const;

    void setLockState(int newState) {
        if (LockState::isNewStateCorrect(lockState_, newState)) {
            lockState_ = newState;
        }
    }

    [[nodiscard]] int lockState() const;

    [[nodiscard]] bool locked() const { return locked_; }

    void setLocked(bool locked) {
        locked_ = locked;
    }

    [[nodiscard]] LockIcon lockIcon() const {
        if (locked_) {
            return LockStateChecker::lockAvailable() ? LockIcon::Locked : LockIcon::LockNotAvailable;
        }
        return LockIcon::Unlocked;
    }

    [[nodiscard]] bool visible() const { return visible_; }

    void setVisible(bool visible) {
        visible_ = visible;
    }

    [[nodiscard]] SIZE size() const {
        return size_;
    }

    UnlockGuardModel guard;

    std::optional<std::reference_wrapper<PanelButton>> buttonPressed{};

    using ComponentVec = std::vector<std::reference_wrapper<PanelComponent>>;
    ComponentVec components{
            menu,
            close,
            settings,
            help,
            about,
            lockButton,
            keyboard,
            mouse,
            body,
    };

    PanelButton close;
    PanelButton menu;
    PanelButton lockButton;
    PanelComponent titleBar;
    PanelComponent body;

    PanelComponent menuBar;
    PanelButton settings;
    PanelButton help;
    PanelButton about;

    Color menuBarBackground{35, 35, 38, 245};

    struct BaseComponent {
        int x = 0;
        int y = 0;
        unsigned w = 0;
        unsigned h = 0;

        struct PositionOffset {
            float x = 0;
            float y = 0;
            float z = 0;
        } offset3;
    };

    struct LockComponent: public BaseComponent {
        bool open = true;
        bool enabled = true;
    };

    BaseComponent circle;
    BaseComponent progressBackground;
    BaseComponent progress;
    LockComponent lock;
    PanelButton keyboard;
    PanelButton mouse;

    bool buttonHover = false;
    bool lockCancel = false;

private:
    bool visible_ = false;
    bool locked_ = false;
    int lockState_ = LockState::NONE;
    SIZE size_{};
};

} // namespace litelockr

#endif // FLYOUT_MODEL_H
