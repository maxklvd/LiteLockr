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

#include "FlyoutProgressBar.h"

#include <cmath>

#include "ini/SettingsData.h"
#include "sys/AppTimer.h"
#include "sys/Comparison.h"
#include "sys/KeyFrames.h"

namespace litelockr {

void FlyoutProgressBar::reset() {
    enabled_ = false;
    stopping_ = false;
    progress_ = 0;
    value_ = MAX_VALUE;
    timer_.reset();
}

void FlyoutProgressBar::start() {
    reset();

    int delay = SettingsData::instance().delayBeforeLocking.value() * 1000;
    assert(delay > 0);

    timer_.destroy();
    timer_.create(std::chrono::milliseconds(delay));
    enabled_ = true;

    //smooth anim
    AppTimer::setAnimationActiveFor(delay);
}

void FlyoutProgressBar::stop() {
    if (enabled_) {
        enabled_ = false;
        stopping_ = true;
        AppTimer::setAnimationActiveFor(1000);
    }
}

void FlyoutProgressBar::tick() {
    if (stopping_) {
        decrease();
        if (value_ == 0) {
            stopping_ = false;
        }
        return;
    }

    timer_.timeout();

    float value = (static_cast<float>(timer_.elapsed().count()) / static_cast<float>(timer_.period().count())) * 100.0f;
    progress_ = std::clamp(value, 0.0f, 100.0f);
}

bool FlyoutProgressBar::done() const {
    return isFloatEquals(progress_, 100.0f);
}

std::uint8_t FlyoutProgressBar::opacity() const {
    float f = KeyFrames::interpolate(Interpolator::EASY_IN, 0, MAX_VALUE, float(value_));
    int value = std::lround(f * 255.0f);
    return static_cast<std::uint8_t>(std::clamp(value, 0, 255));
}

void FlyoutProgressBar::decrease() {
    value_ = std::clamp(--value_, 0, MAX_VALUE);
}

} // namespace litelockr
