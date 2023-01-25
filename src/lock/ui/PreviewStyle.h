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

#ifndef PREVIEW_STYLE_H
#define PREVIEW_STYLE_H

#include "gfx/Color.h"

namespace litelockr::PreviewStyle {
constexpr static int ButtonBorderWidth = 2; // px
constexpr static int ApplicationBorderWidth = 2; // px
constexpr static int ApplicationBackgroundOpacity = 30; // 0..255
const static inline Color ButtonBorderColor = {255, 255, 255};
const static inline Color ButtonHintTextColor = {255, 255, 255};
const static inline Color ButtonHintTextSelColor = {0, 0, 0};
const static inline Color ButtonHintSelectionColor = {255, 201, 14};
const static inline Color ButtonHintBackgroundColor = {35, 35, 35};
const static inline Color ApplicationBorderColor = {26, 159, 221};
const static inline Color ApplicationBackgroundColor = {26, 159, 221};
const static inline Color ApplicationTextColor = {0, 0, 0};
} // namespace litelockr::PreviewStyle

#endif // PREVIEW_STYLE_H
