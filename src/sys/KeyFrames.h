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

#ifndef KEY_FRAMES_H
#define KEY_FRAMES_H

#include <algorithm>
#include <vector>

namespace litelockr {

#define EASY_IN(x) (KeyFrames::interpolate(Interpolator::EASY_IN,0,1,x))
#define EASY_OUT(x) (KeyFrames::interpolate(Interpolator::EASY_OUT,0,1,x))
#define EASY_BOTH(x) (KeyFrames::interpolate(Interpolator::EASY_BOTH,0,1,x))

enum class Interpolator {
    LINEAR,
    EASY_IN,
    EASY_OUT,
    EASY_BOTH,
};

class KeyFrames {
public:
    constexpr static float interpolate(Interpolator interpolator, float startValue, float endValue, float value) {
        float val = std::clamp((value - startValue) / (endValue - startValue), 0.0f, 1.0f);

        switch (interpolator) {
            case Interpolator::EASY_BOTH:
                return smoothStep(val);
            case Interpolator::EASY_IN:
                return 2.0f * smoothStep(val / 2.0f);
            case Interpolator::EASY_OUT:
                return 2.0f * smoothStep((val + 1.0f) / 2.0f) - 1.0f;
            case Interpolator::LINEAR:
                [[fallthrough]];
            default:
                return val;
        }
    }

    void addStopPoint(float offset, float value, Interpolator interpolator = Interpolator::EASY_BOTH);
    void clear();
    [[nodiscard]] float interpolate(float value) const;

private:
    template<typename T>
    constexpr static T smoothStep(T t) {
        return t * t * (3 - 2 * t);
    }

    struct StopPoint {
        float offset = 0;
        float value = 0;
        Interpolator interpolator = Interpolator::LINEAR;
    };
    std::vector<StopPoint> stopPoints_;
};

} // namespace litelockr

#endif // KEY_FRAMES_H
