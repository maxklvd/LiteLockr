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

#include "UIAutomationHelper.h"

#include "UIAutomationHelperImpl.h"

namespace litelockr {

UIAutomationHelper::UIAutomationHelper() : pImpl{std::make_unique<UIAutomationHelperImpl>()} {
    pImpl->initialize();
}

UIAutomationHelper::~UIAutomationHelper() {
    pImpl->dispose();
}

std::wstring UIAutomationHelper::getAutomationIdOfActiveButton() const {
    return pImpl->getAutomationIdOfActiveButton();
}

ButtonProperties UIAutomationHelper::getTaskbarButton(int index) const {
    return pImpl->getTaskbarButton(index);
}

ButtonProperties UIAutomationHelper::findTaskbarButton(int propertyId,
                                                       const StringSet& searchValues,
                                                       int searchOptions) const {
    return pImpl->findTaskbarButton(propertyId, searchValues, searchOptions);
}

void UIAutomationHelper::findTaskbarButtons(const StringSet& appNames,
                                            const StringSet& exeNames,
                                            const StringSet& autoIds,
                                            ButtonPropertiesList& result) const {
    return pImpl->findTaskbarButtons(appNames, exeNames, autoIds, result);
}

} // namespace litelockr
