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

#ifndef FLYOUT_PROGRESS_BAR_H
#define FLYOUT_PROGRESS_BAR_H

#include "sys/AppClock.h"

namespace litelockr {

class FlyoutProgressBar {
public:
    FlyoutProgressBar() {
        reset();
    }

    [[nodiscard]] bool enabled() const { return enabled_; }
    [[nodiscard]] bool stopping() const { return stopping_; }
    [[nodiscard]] float progress() const { return progress_; }

    void reset();
    void start();
    void stop();
    void tick();
    [[nodiscard]] bool done() const;
    [[nodiscard]] std::uint8_t opacity() const;

private:
    bool enabled_ = false;
    bool stopping_ = false;
    float progress_ = 0;
    TimeCounter timer_;

    constexpr static int MAX_VALUE = 15;
    int value_ = MAX_VALUE;

    void decrease();
};

} // namespace litelockr

#endif // FLYOUT_PROGRESS_BAR_H
