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

#include "UnlockGuardModel.h"

#include <random>
#include <sstream>

#include "ini/SettingsData.h"

namespace litelockr {

void UnlockGuardModel::generate(std::vector<int>& result) {
    int product;
    int counter = 0;
    do {
        value0_ = getRandomFrom2to9();
        value1_ = getRandomFrom2to9();
        product = value0_ * value1_;

        if (++counter > 100) {
            break;
        }
    } while (product < 10 || product == last_);

    last_ = product;

    int d1 = product / 10;
    int d0 = product - d1 * 10;
    result.push_back(d1);
    result.push_back(d0);
}

void UnlockGuardModel::initialize() {
    correctAnswer_.clear();
    isPin_ = false;

    if (SettingsData::instance().pinEnabled()) {
        isPin_ = true;
        for (auto c: SettingsData::instance().pin.value()) {
            assert(c >= L'0' && c <= L'9');
            auto digit = static_cast<int>(c - L'0');
            correctAnswer_.push_back(digit);
        }
    } else {
        generate(correctAnswer_);
    }

    enteredAnswer_.clear();
    inputState_ = WAIT;
    inactivityTimer_.reset();
}

std::wstring UnlockGuardModel::getTopText() const {
    std::wstringstream buf;
    if (isPin_) {
        buf << L'\uE802'; // 'lock' sign
    } else {
        buf << value0_ << L'\u00D7' << value1_ << L'='; // 2x2=
    }
    return buf.str();
}

std::wstring UnlockGuardModel::getBottomText() {
    unsigned num = 2;
    wchar_t bullet = L'\u2013'; // '-' sign
    if (isPin_) {
        num = 4;
        bullet = L'\uE800'; // 'o' sign
    }

    std::wstringstream buf;
    auto size = enteredAnswer_.size();

    for (unsigned i = 0; i < num; i++) {
        if (i < size) {
            if (isPin_) {
                buf << L'\uE801'; // 'o' filled sign
            } else {
                buf << enteredAnswer_[i]; // entered digit
            }
        } else {
            buf << bullet; // '-' or 'o'
        }
    }
    return buf.str();
}

bool UnlockGuardModel::isExpired() {
    if (inactivityTimer_.timeout()) {
        inactivityTimer_.reset();
        cancel();
        return true;
    }
    return false;
}

void UnlockGuardModel::onKeyUp(WPARAM wParam) {
    inactivityTimer_.reset();

    int code = static_cast<int>(wParam);
    int digit = -1;
    if (code >= '0' && code <= '9') {
        digit = code - '0';
    } else if (code >= VK_NUMPAD0 && code <= VK_NUMPAD9) {
        digit = code - VK_NUMPAD0;
    }

    if (digit >= 0) {
        auto numDigits = correctAnswer_.size();

        if (enteredAnswer_.size() < numDigits) {
            enteredAnswer_.push_back(digit);
        }

        if (enteredAnswer_.size() == numDigits) {
            if (isCorrect()) {
                inputState_ = CORRECT;
            } else {
                inputState_ = WRONG;
            }
        } else {
            inputState_ = WAIT;
        }
    } else if (code == VK_RETURN) { // enter
        inputState_ = WRONG;
    } else if (code == VK_BACK || code == VK_DELETE) { // back & delete
        if (!enteredAnswer_.empty()) {
            enteredAnswer_.pop_back();
            inputState_ = WAIT;
        }
    } else if (code == VK_ESCAPE) { // ESCAPE
        cancel();
    }
}

void UnlockGuardModel::resetWrongState() {
    if (inputState_ == WRONG) {
        enteredAnswer_.clear();
        inputState_ = WAIT;
    }
}

void UnlockGuardModel::cancel() {
    appEvent = AppEvent(AppEvent::NONE);
    enteredAnswer_.clear();
    inputState_ = CANCEL;
    inactivityTimer_.reset();
}

void UnlockGuardModel::reset() {
    appEvent = AppEvent(AppEvent::NONE);
    enteredAnswer_.clear();
    inputState_ = WAIT;
    inactivityTimer_.reset();
}

int UnlockGuardModel::getRandomFrom2to9() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::shuffle(numbersFrom2To9_.begin(), numbersFrom2To9_.end(), mt);
    return numbersFrom2To9_[0];
}

void UnlockGuardModel::setActiveState(UnlockGuardActiveState newState) {
    activeState_ = newState;
}

} // namespace litelockr
