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

#ifndef FLYOUT_LAYERS_H
#define FLYOUT_LAYERS_H

namespace litelockr {

struct FlyoutLayers {
    bool planes = true;
    bool circle = true;
    bool progressBackground = true;
    bool progress = true;
    bool lock = true;
    bool keyboard = true;
    bool mouse = true;
    bool lockUpdateMaterial = true;

    void set(bool val) {
        planes =
        circle =
        progressBackground =
        progress =
        lock =
        keyboard =
        mouse = val;
    }

    static FlyoutLayers all() {
        FlyoutLayers instance;
        return instance;
    }

    static FlyoutLayers none() {
        FlyoutLayers instance;
        instance.set(false);
        return instance;
    }

    static FlyoutLayers background() {
        FlyoutLayers instance;
        instance.set(true);
        instance.progress = instance.lock = instance.keyboard = instance.mouse = false;
        return instance;
    }
};

} // namespace litelockr

#endif // FLYOUT_LAYERS_H
