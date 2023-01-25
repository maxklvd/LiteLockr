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

#ifndef ANIMATION_FRAME_SET_H
#define ANIMATION_FRAME_SET_H

#include "gfx/FrameSet.h"

namespace litelockr {

enum class AnimationId {
    NONE = 0,
    STARTUP,
    SHOW_MAIN_WINDOW,
    HIDE_MAIN_WINDOW,
    SHOW_MENU,
    HIDE_MENU,
    SHOW_GUARD,
    HIDE_GUARD,
    GUARD_WRONG,
    START_LOCKING,
    FINISH_LOCKING,
    CANNOT_UNLOCK,
    UNLOCK,
    SHAKE,
};

enum class AnimationAction {
    NONE = 0,
    LOCK,
    UNLOCK,
    SHAKE,
    EXIT,
};

class AnimationFrameSet: public FrameSet {
public:
    void setId(AnimationId id) { id_ = id; }

    [[nodiscard]] AnimationId id() const { return id_; }

    void setActionBefore(AnimationAction actionBefore) {
        actionBefore_ = actionBefore;
    }

    [[maybe_unused]] [[nodiscard]] AnimationAction actionBefore() const { return actionBefore_; }

    void setActionAfter(AnimationAction actionAfter) {
        actionAfter_ = actionAfter;
    }

    [[nodiscard]] AnimationAction actionAfter() const { return actionAfter_; }

    void setPosition(int x, int y) {
        position_.x = x;
        position_.y = y;
    }

    [[nodiscard]] POINT position() const { return position_; }

private:
    AnimationId id_ = AnimationId::NONE;
    AnimationAction actionBefore_ = AnimationAction::NONE;
    AnimationAction actionAfter_ = AnimationAction::NONE;
    POINT position_{};
};

} // namespace litelockr

#endif // ANIMATION_FRAME_SET_H