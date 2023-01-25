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

#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

#include "gfx/AggAll.h"

namespace litelockr {

using Color = agg::rgba8;
constexpr static auto ColorR = agg::order_bgra::R;
constexpr static auto ColorG = agg::order_bgra::G;
constexpr static auto ColorB = agg::order_bgra::B;
constexpr static auto ColorA = agg::order_bgra::A;

} // namespace litelockr

#endif // COLOR_H