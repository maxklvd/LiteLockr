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

#ifndef UNLOCK_GUARD_MODEL_H
#define UNLOCK_GUARD_MODEL_H

#include <string>
#include <vector>

#include <windows.h>
#include "app/event/AppEvent.h"
#include "sys/AppClock.h"

namespace litelockr {

enum class UnlockGuardActiveState {
    PRE_INACTIVE,
    INACTIVE,
    PRE_ACTIVE,
    ACTIVE,
};

class UnlockGuardModel {
public:
    AppEvent appEvent{AppEvent::NONE};

    enum InputState {
        WAIT = 0,
        CORRECT = 1,
        WRONG = 2,
        CANCEL = 3,
    };

    void initialize();

    [[nodiscard]] std::wstring getTopText() const;
    std::wstring getBottomText();

    [[nodiscard]] int inputState() const { return inputState_; }

    bool isExpired();
    void onKeyUp(WPARAM wParam);

    [[nodiscard]] RECT workArea() const { return workArea_; }

    void setWorkArea(RECT wa) {
        workArea_ = wa;
    }

    void resetWrongState();
    void cancel();
    void reset();

    void setActiveState(UnlockGuardActiveState newState);

    [[nodiscard]] bool visible() const { return activeState_ != UnlockGuardActiveState::INACTIVE; }

private:
    [[nodiscard]] bool isCorrect() const {
        return enteredAnswer_ == correctAnswer_; // compare two vectors
    }

    int getRandomFrom2to9();
    void generate(std::vector<int>& result);

    UnlockGuardActiveState activeState_ = UnlockGuardActiveState::INACTIVE;

    std::vector<int> enteredAnswer_;
    std::vector<int> correctAnswer_;
    bool isPin_ = false;

    int inputState_ = WAIT;
    int value0_ = 0;
    int value1_ = 0;

    RECT workArea_{};
    int last_ = -1;
    TimeCounter inactivityTimer_{std::chrono::seconds(10)};

    std::vector<int> numbersFrom2To9_{2, 3, 4, 5, 6, 7, 8, 9};
};

} // namespace litelockr

#endif // UNLOCK_GUARD_MODEL_H
