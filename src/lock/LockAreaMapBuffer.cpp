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

#include "LockAreaMapBuffer.h"

#include <cassert>

#include "sys/Process.h"

namespace litelockr {

// read
LockArea LockAreaMapBuffer::getLockArea(int cursorX, int cursorY) const {
    std::scoped_lock<std::mutex> lock{readMtx_};
    return currentPtr_->getLockArea(cursorX, cursorY);
}

// read
const LockAreaMap& LockAreaMapBuffer::currentMap() const {
    std::scoped_lock<std::mutex> lock{readMtx_};

    assert(!currentPtr_->data().empty());
    return *currentPtr_;
}

// read
int LockAreaMapBuffer::width() const {
    std::scoped_lock<std::mutex> lock{readMtx_};
    return currentPtr_->width();
}

// read
int LockAreaMapBuffer::height() const {
    std::scoped_lock<std::mutex> lock{readMtx_};
    return currentPtr_->height();
}

// read
int LockAreaMapBuffer::offsetX() const {
    std::scoped_lock<std::mutex> lock{readMtx_};
    return currentPtr_->offsetX();
}

// read
int LockAreaMapBuffer::offsetY() const {
    std::scoped_lock<std::mutex> lock{readMtx_};
    return currentPtr_->offsetY();
}

// write
void LockAreaMapBuffer::update(const UpdateMapFunction& updateFn) {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    std::scoped_lock<std::mutex> lock{writeMtx_};
    auto& map = *nextPtr_;
    updateFn(map);
    bufferUpdated_ = true;
}

// swap
void LockAreaMapBuffer::swapUpdate() noexcept {
    int result = std::try_lock(readMtx_, writeMtx_);
    if (result == -1) {
        if (bufferUpdated_) {
            std::swap(currentPtr_, nextPtr_);
            bufferUpdated_ = false;
        }
        readMtx_.unlock();
        writeMtx_.unlock();
    }
}

} // namespace litelockr
