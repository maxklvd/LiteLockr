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

#ifndef UIAUTOMATION_HELPER_H
#define UIAUTOMATION_HELPER_H

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "UIAutomationHelperTypes.h"

namespace litelockr {

class UIAutomationHelperImpl;

class UIAutomationHelper {
public:
    using StringSet = std::unordered_set<std::wstring>;
    using RectList = std::vector<RECT>;
    using ButtonPropertiesList = std::vector<ButtonProperties>;

    UIAutomationHelper();
    UIAutomationHelper(const UIAutomationHelper&) = delete;
    UIAutomationHelper& operator=(const UIAutomationHelper&) = delete;
    ~UIAutomationHelper();

    std::wstring getAutomationIdOfActiveButton() const;
    ButtonProperties getTaskbarButton(int index) const;
    ButtonProperties findTaskbarButton(int propertyId, const StringSet& searchValues, int searchOptions) const;

    void findTaskbarButtons(const StringSet& appNames,
                            const StringSet& exeNames,
                            const StringSet& autoIds,
                            ButtonPropertiesList& result) const;

private:
    std::unique_ptr<UIAutomationHelperImpl> pImpl;
};

} // namespace litelockr

#endif // UIAUTOMATION_HELPER_H
