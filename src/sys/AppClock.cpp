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

#include "AppClock.h"

#include <cassert>

using namespace std::chrono_literals;

namespace litelockr {

TimeCounter::TimeCounter(AppClock::Duration periodTime)
        : period_(periodTime),
          elapsed_(0ns),
          enabled_(true) {}

TimeCounter::TimeCounter(AppClock::Duration periodTime, bool enabled)
        : period_(periodTime),
          elapsed_(0ns),
          enabled_(enabled) {}

void TimeCounter::create(AppClock::Duration period) {
    assert(!hasInitialized_);
    period_ = period;
    elapsed_ = 0ns;
}

void TimeCounter::destroy() {
    period_ = 0ns;
    elapsed_ = 0ns;
    hasInitialized_ = false;
}

void TimeCounter::reset() {
    lastTime_ = AppClock::now();
    elapsed_ = 0ns;
}

bool TimeCounter::timeout() {
    if (!enabled_) {
        return false;
    }

    auto currentTime = AppClock::now();

    if (!hasInitialized_) {
        lastTime_ = currentTime;
        hasInitialized_ = true;
        return period_ == 0ns;
    }

    elapsed_ = currentTime - lastTime_;
    if (elapsed_ >= period_) {
        lastTime_ = currentTime;
        return true;
    }
    return false;
}

AppClock::Duration TimeCounter::period() const {
    return period_;
}

AppClock::Duration TimeCounter::elapsed() const {
    return elapsed_;
}

void TimeCounter::setEnabled(bool enabled) {
    enabled_ = enabled;
    reset();
}

void OneShotTimeCounter::create(AppClock::Duration waitTime) {
    timeCounter_.create(waitTime);
}

void OneShotTimeCounter::recreate(AppClock::Duration waitTime) {
    finished_ = false;
    timeCounter_.destroy();
    timeCounter_.create(waitTime);
}

void OneShotTimeCounter::reset() {
    finished_ = false;
    timeCounter_.reset();
}

void OneShotTimeCounter::destroy() {
    finished_ = false;
    timeCounter_.destroy();
}

bool OneShotTimeCounter::timeout() {
    if (finished_) {
        return true;
    }

    if (timeCounter_.timeout()) {
        finished_ = true;
    }
    return finished_;
}

} // namespace litelockr
