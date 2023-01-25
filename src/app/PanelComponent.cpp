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

#include "PanelComponent.h"

#include <utility>

namespace litelockr {

bool PanelComponent::contains(int _x, int _y, bool isView3) const {
    if (useView3 && isView3) {
        return view3.contains(_x, _y);
    }
    return _x >= x && std::cmp_less(_x, x + w)
           && _y >= y && std::cmp_less(_y, y + h);
}

} // namespace litelockr
