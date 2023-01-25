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

#ifndef TASKBAR_BUTTON_DETECTOR_H
#define TASKBAR_BUTTON_DETECTOR_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>

#include "ini/ApplicationRecord.h"
#include "lock/uia/UIAutomationHelperTypes.h"

namespace litelockr {

class TaskbarButtonDetector {
public:
    using StringSet = std::unordered_set<std::wstring>;

    static std::wstring detectAutomationId(const ApplicationRecord& app);
    static ButtonProperties findByAutomationId(const std::wstring& id);
    static ButtonProperties findByExecutableInAutomationId(const std::wstring& executable);
    static ButtonProperties findByName(const StringSet& names);
};

} // namespace litelockr

#endif // TASKBAR_BUTTON_DETECTOR_H
