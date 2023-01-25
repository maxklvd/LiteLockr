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

#include "DblClickRecognizer.h"

#include <cstdlib>

namespace litelockr {

void DblClickRecognizer::initialize() {
    cxDoubleClk_ = GetSystemMetrics(SM_CXDOUBLECLK);
    cyDoubleClk_ = GetSystemMetrics(SM_CYDOUBLECLK);
    doubleClickTime_ = std::chrono::milliseconds(GetDoubleClickTime());
    lastClickTime_ = AppClock::now();
    lastClickPos_ = {};
}

bool DblClickRecognizer::isDoubleClick(POINT pt) {
    auto time = AppClock::now();

    bool dblClick = false;
    if ((time - lastClickTime_ < doubleClickTime_)
        && std::abs(lastClickPos_.x - pt.x) < cxDoubleClk_
        && std::abs(lastClickPos_.y - pt.y) < cyDoubleClk_) {
        dblClick = true;
    } else {
        lastClickTime_ = time;
        lastClickPos_ = pt;
    }
    return dblClick;
}

} // namespace litelockr
