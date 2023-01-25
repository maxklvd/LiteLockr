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

#ifndef FRAME_SET_H
#define FRAME_SET_H

#include "gfx/Bitmap.h"

namespace litelockr {

class FrameSet {
public:
    FrameSet() = default;
    FrameSet(const FrameSet&) = delete;
    FrameSet& operator=(const FrameSet&) = delete;

    void clone(const FrameSet& another);
    bool isNull() const;
    void recreate(unsigned width, unsigned height, unsigned numFrames);
    void createFromBitmap(const Bitmap& buf, unsigned numFrames);
    bool loadPng(int resourceId, unsigned numFrames);
    bool addFrame(unsigned frame, const Bitmap& image) noexcept;
    void drawFrame(Bitmap& buffer, int x, int y);
    void begin();
    void end();
    void nextFrame();

    [[nodiscard]] bool isFirstFrame() const { return currentFrame_ == 0; }
    [[nodiscard]] bool isLastFrame() const { return currentFrame_ == numFrames_ - 1; }

    [[nodiscard]] bool active() const;

    [[nodiscard]] unsigned width() const { return width_; }
    [[nodiscard]] unsigned height() const { return height_; }
    [[nodiscard]] unsigned size() const { return numFrames_; };

    Bitmap& bitmap() { return bufferBitmap_; }

    void createWithBackground(const Bitmap& frames, unsigned numFrames, const Bitmap& backgroundFrame);

private:
    Bitmap bufferBitmap_;

    unsigned width_ = 0;
    unsigned height_ = 0;
    unsigned numFrames_ = 0;
    unsigned currentFrame_ = 0;

    bool ready_ = false;
    bool finished_ = false;
};

} // namespace litelockr

#endif // FRAME_SET_H
