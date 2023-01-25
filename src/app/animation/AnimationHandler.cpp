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

#include "AnimationHandler.h"

#include "sys/AppTimer.h"

namespace litelockr {

void AnimationHandler::runAnimation(AnimationFrameSet& animation, DWORD delayBeforeRun /*= 0*/) {
    assert(!animation.isNull());

    animation_ = animation;
    delay_.destroy();
    delay_.create(std::chrono::microseconds(delayBeforeRun));
}

bool AnimationHandler::ready() {
    return active() && delay_.timeout();
}

bool AnimationHandler::active() {
    if (!delay_.timeout()) {
        //not active yet
        return false;
    }
    return !empty() && getAnimation().active();
}

bool AnimationHandler::draw(Bitmap& buffer) {
    auto&& anim = getAnimation();
    anim.drawFrame(buffer, anim.position().x, anim.position().y);
    if (anim.isLastFrame()) {
        return true;
    }

    anim.nextFrame();
    return false;
}

void AnimationHandler::drawLastFrame(Bitmap& buffer) {
    getAnimation().end();
    getAnimation().drawFrame(buffer, getAnimation().position().x, getAnimation().position().y);
}

void AnimationHandler::free() {
    if (animation_.has_value()) {
        animation_ = {};
    }
}

} // namespace litelockr
