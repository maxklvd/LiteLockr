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

#include "LockAreaMap.h"

#include <cassert>

#include "lock/DisplayMonitors.h"
#include "sys/Rectangle.h"

namespace litelockr {

void LockAreaMap::recreate() {
    MonitorInfo mi{};
    DisplayMonitors::get(mi);
    const auto& rc = mi.unionMonitor;
    width_ = Rectangle::width(rc);
    height_ = Rectangle::height(rc);
    assert(width_ > 0 && height_ > 0);

    offsetX_ = rc.left;
    offsetY_ = rc.top;
    data_.resize(width_ * height_);

    RECT rcAll{0, 0, width_, height_};
    lockAreas_.clear();

    LockArea deny{LockArea::DENY, false, false, rcAll};
    [[maybe_unused]] int idxDeny = addLockArea(deny);
    assert(idxDeny == static_cast<int>(LockAreaMap::IDX_DENY));

    LockArea allow{LockArea::ALLOW, true, false, rcAll};
    [[maybe_unused]] int idxAllow = addLockArea(allow);
    assert(idxAllow == static_cast<int>(LockAreaMap::IDX_ALLOW));
}

LockArea LockAreaMap::getLockArea(int cursorX, int cursorY) const {
    auto lockAreaIdx = IDX_DENY;

    int x = cursorX - offsetX_;
    int y = cursorY - offsetY_;

    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        unsigned dataIdx = width_ * y + x;

        assert(dataIdx < data_.size());
        if (dataIdx < data_.size()) {
            lockAreaIdx = data_[dataIdx];
            assert(static_cast<size_t>(lockAreaIdx) < lockAreas_.size());
        }
    }

    return lockAreas_[static_cast<int>(lockAreaIdx)];
}

int LockAreaMap::addLockArea(const LockArea& area) {
    auto idx = lockAreas_.size();
    if (idx < MAX_INDEX) {
        lockAreas_.push_back(area);
        return static_cast<int>(idx);
    }
    return WRONG_INDEX;
}

void LockAreaMap::addRects(const std::vector<RECT>& rects, int lockAreaId, bool allowed, bool workArea) {
    for (auto rc: rects) {
        LockArea lockArea{lockAreaId, allowed, workArea, rc};
        int idx = addLockArea(lockArea);
        if (idx > 0) {
            Rectangle::offset(rc, -offsetX_, -offsetY_);
            fillRect(rc, static_cast<ValueType>(idx));
        }
    }
}

void LockAreaMap::fill(ValueType value) {
    std::fill(data_.begin(), data_.end(), value);
}

void LockAreaMap::fillRect(RECT rc, ValueType value) {
    Rectangle::clamp(rc, 0, 0, width_, height_);

    for (int y = rc.top; y < rc.bottom; y++) {
        auto it = data_.begin() + width_ * y + rc.left;
        auto len = rc.right - rc.left;
        std::fill_n(it, len, value);
    }
}

} // namespace litelockr
