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

#ifndef UNLOCK_GUARD_VIEW_H
#define UNLOCK_GUARD_VIEW_H

#include <string>

#include "app/animation/AnimationFrameSet.h"
#include "app/guard/UnlockGuardModel.h"
#include "gfx/Bitmap.h"
#include "gfx/BitmapContext.h"

namespace litelockr {

class UnlockGuardShowAnimation {
public:
    static void create(AnimationFrameSet& result, Bitmap& buffer, const RECT& workArea, const std::wstring& line0,
                       const std::wstring& line1, bool hide);
};

class UnlockGuardWrongAnimation {
public:
    static void create(AnimationFrameSet& result, Bitmap& buffer, const RECT& workArea,
                       const std::wstring& line0, const std::wstring& line1);
};

class UnlockGuardView {
public:
    UnlockGuardView(Bitmap& buf, UnlockGuardModel& _m)
            : buffer_(buf),
              model_(_m),
              ctx_(buf) {}

    void initialize() {
        model_.initialize();
    }

    void draw() {
        draw(buffer_);
    }

    void draw(Bitmap& buffer);

    AnimationFrameSet& createShowAnimation(Bitmap& buf);
    AnimationFrameSet& createHideAnimation(Bitmap& buf);
    AnimationFrameSet& createWrongAnimation(Bitmap& buf);

private:
    Bitmap& buffer_;
    UnlockGuardModel& model_;
    BitmapContext ctx_;

    AnimationFrameSet showAnimation_;
    AnimationFrameSet hideAnimation_;
    AnimationFrameSet wrongAnimation_;

    Bitmap bgBuffer_;
};

} // namespace litelockr

#endif // UNLOCK_GUARD_VIEW_H
