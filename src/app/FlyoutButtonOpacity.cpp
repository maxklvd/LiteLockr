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

#include "FlyoutButtonOpacity.h"

#include <algorithm>
#include <cmath>

#include "sys/KeyFrames.h"

namespace litelockr {

std::uint8_t FlyoutButtonOpacity::opacity() const {
    int val = std::lround(opacityVal() * 255.0f);
    return static_cast<std::uint8_t>(std::clamp(val, 0, 255));
}

float FlyoutButtonOpacity::opacityVal() const {
    return KeyFrames::interpolate(Interpolator::EASY_IN, 0, float(maxValue_), float(value_));
}

bool FlyoutButtonOpacity::update(bool hover, bool disabled) {
    int old = value_;

    if (disabled || (!hover && hide_.timeout())) {
        decrease();
        return old != value_;
    }

    if (hover) {
        hide_.reset();
        increase();
    } else if (value_ > 0) {
        increase();
    }

    return old != value_;
}

void FlyoutButtonOpacity::increase() {
    value_ = std::clamp(++value_, 0, maxValue_);
}

void FlyoutButtonOpacity::decrease() {
    value_ = std::clamp(--value_, 0, maxValue_);
}

} // namespace litelockr