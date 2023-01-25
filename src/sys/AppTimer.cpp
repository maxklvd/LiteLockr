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

#include "AppTimer.h"

#include <cassert>

namespace litelockr {

bool AppTimer::isAnimationActive_{false};
AppClock::TimePoint AppTimer::animationExpiryTime_;

void AppTimer::fireTimeout() const {
    for (auto& pListener: listeners_) {
        assert(pListener);
        pListener->timeout();
    }

    //
    // Reset isAnimationActive flag
    //
    if (isAnimationActive_ && animationExpiryTime_ < AppClock::now()) {
        isAnimationActive_ = false;
    }
}

void AppTimer::addListener(AppTimerListener& listener) {
    // does it already exist?
    if (std::find(listeners_.begin(), listeners_.end(), &listener) != listeners_.end()) {
        assert(false);
        return;
    }

    listeners_.push_back(&listener);
}

} // namespace litelockr
