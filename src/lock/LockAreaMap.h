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

#ifndef LOCK_AREA_MAP_H
#define LOCK_AREA_MAP_H

#include <vector>

#include "LockArea.h"

namespace litelockr {

class LockAreaMap {
public:
    using ValueType = std::byte;
    constexpr static ValueType IDX_DENY{0};
    constexpr static ValueType IDX_ALLOW{1};

    LockAreaMap() = default;

    void recreate();

    [[nodiscard]] int width() const { return width_; }

    [[nodiscard]] int height() const { return height_; }

    [[nodiscard]] int offsetX() const { return offsetX_; }

    [[nodiscard]] int offsetY() const { return offsetY_; }

    [[nodiscard]] const std::vector<ValueType>& data() const {
        return data_;
    };

    [[nodiscard]] LockArea getLockArea(int cursorX, int cursorY) const;

    [[nodiscard]] const LockAreaVec& getLockAreas() const { return lockAreas_; }

    void addRects(const std::vector<RECT>& rects, int lockAreaId, bool allowed = true, bool workArea = true);

    void fill(ValueType value);
    void fillRect(RECT rc, ValueType value);

private:
    std::vector<ValueType> data_;
    LockAreaVec lockAreas_;

    int width_ = 0;
    int height_ = 0;
    int offsetX_ = 0;
    int offsetY_ = 0;

    constexpr static int MAX_INDEX = sizeof(ValueType) << 8;
    constexpr static int WRONG_INDEX = -1;
    int addLockArea(const LockArea& area);
};

} // namespace litelockr

#endif // LOCK_AREA_MAP_H
