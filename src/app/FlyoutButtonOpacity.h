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

#ifndef FLYOUT_BUTTON_OPACITY_H
#define FLYOUT_BUTTON_OPACITY_H

#include "sys/AppClock.h"

namespace litelockr {

class FlyoutButtonOpacity {
public:
    FlyoutButtonOpacity() = default;

    [[nodiscard]] bool visible() const {
        return value_ > 0;
    }

    [[nodiscard]] std::uint8_t opacity() const;

    bool update(bool hover, bool disabled);

protected:
    void increase();

    void decrease();

    void reset() {
        value_ = 0;
    }

    [[nodiscard]] float opacityVal() const;

private:
    int value_ = 0;
    int maxValue_ = 15;

    OneShotTimeCounter hide_{std::chrono::seconds(8)};
};

} // namespace litelockr

#endif // FLYOUT_BUTTON_OPACITY_H
