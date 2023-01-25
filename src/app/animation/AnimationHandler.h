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

#ifndef ANIMATION_HANDLER_H
#define ANIMATION_HANDLER_H

#include <functional>
#include <optional>

#include "AnimationFrameSet.h"
#include "sys/AppClock.h"

namespace litelockr {

class AnimationHandler {
public:
    using OptRefAnimation = std::optional<std::reference_wrapper<AnimationFrameSet>>;

    void runAnimation(AnimationFrameSet& animation, DWORD delayBeforeRun = 0);

    bool ready();
    bool active();

    bool draw(Bitmap& buffer);
    void drawLastFrame(Bitmap& buffer);

    [[nodiscard]] bool empty() const {
        return !animation_.has_value();
    }

    void free();

    [[nodiscard]] const AnimationFrameSet& animation() const {
        assert(animation_.has_value());
        return std::cref(animation_.value());
    }

private:
    AnimationFrameSet& getAnimation() {
        assert(animation_.has_value());
        return animation_.value();
    }

    OptRefAnimation animation_{};
    OneShotTimeCounter delay_{std::chrono::seconds(0)};
};

} // namespace litelockr

#endif // ANIMATION_HANDLER_H