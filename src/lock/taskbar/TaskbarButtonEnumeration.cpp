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

#include "TaskbarButtonEnumeration.h"

#include <future>

#include "lock/uia/UIAutomationHelper.h"

namespace litelockr {

TaskbarButtonEnumeration::OptRefButtonProperties TaskbarButtonEnumeration::prev() {
    if (index_ <= 0) {
        index_ = -1;
        return {};
    }

    auto future = std::async(std::launch::async, [idx = index_]() {
        UIAutomationHelper uia;
        return uia.getTaskbarButton(idx - 1);
    });
    auto result = future.get();

    if (result.buttonIndex >= 0) {
        index_ = result.buttonIndex;
        buttonProperties_ = result;
        return std::cref(buttonProperties_);
    }
    return {};
}

TaskbarButtonEnumeration::OptRefButtonProperties TaskbarButtonEnumeration::next() {
    auto future = std::async(std::launch::async, [idx = index_]() {
        UIAutomationHelper uia;
        return uia.getTaskbarButton(idx + 1);
    });
    auto result = future.get();

    if (result.buttonIndex >= 0) {
        index_ = result.buttonIndex;
        buttonProperties_ = result;
        return std::cref(buttonProperties_);
    }
    return {};
}

} // namespace litelockr
