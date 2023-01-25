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

#include "KeyFrames.h"

#include <cassert>

#include "sys/Comparison.h"

namespace litelockr {

void KeyFrames::addStopPoint(float offset, float value, Interpolator interpolator) {
    if (!stopPoints_.empty()) {
        assert(offset > stopPoints_.back().offset);
    }
    stopPoints_.push_back({offset, value, interpolator});
}

void KeyFrames::clear() {
    stopPoints_.clear();
}

float KeyFrames::interpolate(float value) const {
    assert(stopPoints_.size() >= 2);
    assert(isFloatEquals(stopPoints_.front().offset, 0.0f));
    assert(isFloatEquals(stopPoints_.back().offset, 1.0f));

    for (unsigned i = 0; i < stopPoints_.size() - 1; i++) {
        auto& p0 = stopPoints_[i];
        auto& p1 = stopPoints_[i + 1];

        if (value < p1.offset) {
            float len = p1.value - p0.value;
            return p0.value + interpolate(p0.interpolator, 0, p1.offset - p0.offset, value - p0.offset) * len;
        }
    }

    auto& last = stopPoints_.back();
    if (isFloatEquals(value, last.offset)) {
        return last.value;
    }
    return 0;
}

} // namespace litelockr
