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

#include "TaskbarButtonDetector.h"

#include <algorithm>
#include <future>

#include "lock/MousePositionValidator.h"
#include "sys/StringUtils.h"

namespace litelockr {

using namespace std::literals::string_literals;

std::wstring TaskbarButtonDetector::detectAutomationId(const ApplicationRecord& app) {
    auto appCopy = app;
    appCopy.findByAutomationId_ = true;
    appCopy.searchExecutableInAutomationId_ = true;
    appCopy.findByName_ = true;

    UIAutomationHelper::StringSet buttonNames, exeNames, _;
    MousePositionValidator::fillSearchSets(appCopy, buttonNames, exeNames, _);

    auto future = std::async(std::launch::async, [buttonNames, exeNames]() {
        UIAutomationHelper uia;
        UIAutomationHelper::ButtonPropertiesList propList;
        UIAutomationHelper::StringSet autoIds;
        uia.findTaskbarButtons(buttonNames, exeNames, autoIds, propList);

        for (const auto& prop: propList) {
            autoIds.insert(prop.automationId);
        }
        return autoIds.size() == 1 ? *std::begin(autoIds) : L""s;
    });
    return future.get();
}


ButtonProperties TaskbarButtonDetector::findByAutomationId(const std::wstring& id) {
    auto future = std::async(std::launch::async, [&id]() {
        UIAutomationHelper uia;
        return uia.findTaskbarButton(PROP_AUTOMATION_ID, {StringUtils::prepareSearchString(id)}, EXACT_MATCH);
    });

    auto result = future.get();
    result.selPosition = -1; // no need the text selection
    return result;
}

ButtonProperties TaskbarButtonDetector::findByExecutableInAutomationId(const std::wstring& executable) {
    auto future = std::async(std::launch::async, [&executable]() {
        std::wstring name = L"\\"s + StringUtils::prepareSearchString(executable);

        UIAutomationHelper uia;
        return uia.findTaskbarButton(PROP_AUTOMATION_ID, {name}, ENDS_WITH);
    });

    auto result = future.get();

    if (result.selPosition >= 0) {
        result.selPosition++; // skip the added '\' character
    }
    return result;
}

ButtonProperties TaskbarButtonDetector::findByName(const StringSet& names) {
    auto future = std::async(std::launch::async, [&names]() {
        UIAutomationHelper uia;
        return uia.findTaskbarButton(PROP_NAME, names, STARTS_WITH | ENDS_WITH);
    });

    return future.get();
}


} // namespace litelockr
