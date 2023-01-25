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

#ifndef LOCK_AREA_MAP_BUFFER_H
#define LOCK_AREA_MAP_BUFFER_H

#include <functional>
#include <mutex>

#include "lock/LockAreaMap.h"

namespace litelockr {

class LockAreaMapBuffer {
public:
    // read from the current
    LockArea getLockArea(int cursorX, int cursorY) const;
    const LockAreaMap& currentMap() const;
    int width() const;
    int height() const;
    [[maybe_unused]] int offsetX() const;
    [[maybe_unused]] int offsetY() const;

    // write
    using UpdateMapFunction = std::function<void(LockAreaMap&)>;
    void update(const UpdateMapFunction& updateFn);

    // swap current & next
    void swapUpdate() noexcept;

private:
    LockAreaMap map0_;
    LockAreaMap map1_;
    LockAreaMap *currentPtr_{&map0_};
    LockAreaMap *nextPtr_{&map1_};

    bool bufferUpdated_ = false;

    mutable std::mutex readMtx_;
    mutable std::mutex writeMtx_;
};

} // namespace litelockr

#endif // LOCK_AREA_MAP_BUFFER_H
