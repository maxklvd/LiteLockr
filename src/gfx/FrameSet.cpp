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

#include "FrameSet.h"

#include "sys/BinaryResource.h"
#include "sys/Rectangle.h"

namespace litelockr {

bool FrameSet::isNull() const {
    return bufferBitmap_.isNull();
}

void FrameSet::recreate(unsigned width, unsigned height, unsigned numFrames) {
    width_ = width;
    height_ = height;
    numFrames_ = numFrames;
    currentFrame_ = 0;
    ready_ = false;
    finished_ = false;
    bufferBitmap_.create(width, height * numFrames);
    assert(!bufferBitmap_.isNull());
}

void FrameSet::createFromBitmap(const Bitmap& buf, unsigned numFrames) {
    width_ = buf.width();
    height_ = buf.height() / numFrames;
    numFrames_ = numFrames;
    bufferBitmap_.clone(buf);
    begin();
}

bool FrameSet::loadPng(int resourceId, unsigned numFrames) {
    Bitmap buf;
    if (buf.loadPng(Bin::data(resourceId), Bin::size(resourceId))) {
        createFromBitmap(buf, numFrames);
        return true;
    }
    return false;
}

bool FrameSet::addFrame(unsigned frame, const Bitmap& image) noexcept {
    assert(!image.isNull());
    assert(!bufferBitmap_.isNull());

    if (frame < numFrames_) {
        int offset = static_cast<int>(frame * height_);
        bufferBitmap_.copyFrom(image, 0, offset, 0, 0, width_, height_);
        return true;
    }
    return false;
}

void FrameSet::drawFrame(Bitmap& buffer, int x, int y) {
    const int offset = static_cast<int>(currentFrame_ * height_);
    buffer.copyFrom(bufferBitmap_, x, y, 0, offset, width_, height_);

    if (isLastFrame()) {
        finished_ = true;
    }
}

void FrameSet::begin() {
    currentFrame_ = 0;
    ready_ = true;
    finished_ = false;
}

void FrameSet::end() {
    currentFrame_ = std::max(0u, numFrames_ - 1);
    ready_ = true;
    finished_ = false;
}

void FrameSet::nextFrame() {
    auto lastFrame = std::max(0u, numFrames_ - 1);
    currentFrame_ = std::min(currentFrame_ + 1, lastFrame);
}

bool FrameSet::active() const {
    return ready_ && !finished_ && numFrames_ > 0;
}

void FrameSet::clone(const FrameSet& another) {
    width_ = another.width_;
    height_ = another.height_;
    numFrames_ = another.numFrames_;
    currentFrame_ = 0;
    bufferBitmap_.clone(another.bufferBitmap_);
}

void FrameSet::createWithBackground(const Bitmap& frames, unsigned numFrames, const Bitmap& backgroundFrame) {
    recreate(backgroundFrame.width(), backgroundFrame.height(), numFrames);
    for (unsigned i = 0; i < numFrames; i++) {
        addFrame(i, backgroundFrame);
    }
    bufferBitmap_.blendFrom(frames, 0, 0);
}

} // namespace litelockr
