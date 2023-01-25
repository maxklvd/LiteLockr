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

#ifndef UIAUTOMATION_HELPER_IMPL_H
#define UIAUTOMATION_HELPER_IMPL_H

#include <vector>
#include <unordered_set>

#include "UIAutomationHelperTypes.h"

#ifdef __MINGW32__

namespace litelockr {

class UIAutomationHelperImpl {
public:
    using StringSet = std::unordered_set<std::wstring>;
    using RectList = std::vector<RECT>;
    using ButtonPropertiesList = std::vector<ButtonProperties>;

    void initialize() {}

    void dispose() {}

    std::wstring getAutomationIdOfActiveButton() { return L""; }

    ButtonProperties getTaskbarButton(int /*index*/) {
        return {};
    }

    ButtonProperties findTaskbarButton(int /*propertyId*/, const StringSet& /*searchValues*/, int /*searchOptions*/) {
        return {};
    }

    void findTaskbarButtons(const StringSet& appNames,
                            const StringSet& exeNames,
                            const StringSet& autoIds,
                            ButtonPropertiesList& result) {}
};
} // namespace litelockr

#else

#include <functional>

#include <UIAutomation.h>

namespace litelockr {

class UIAutomationHelperImpl {
public:
    using StringSet = std::unordered_set<std::wstring>;
    using RectList = std::vector<RECT>;
    using ButtonPropertiesList = std::vector<ButtonProperties>;

    void initialize();
    void dispose();
    std::wstring getAutomationIdOfActiveButton();
    ButtonProperties getTaskbarButton(int index);
    ButtonProperties findTaskbarButton(int propertyId, const StringSet& searchValues, int searchOptions);
    void findTaskbarButtons(const StringSet& appNames,
                            const StringSet& exeNames,
                            const StringSet& autoIds,
                            ButtonPropertiesList& result);

protected:
    IUIAutomation *uiaPtr_ = nullptr;

    void iterateChildren(IUIAutomationTreeWalker *pWalk,
                         IUIAutomationElement *pParent,
                         std::function<bool(IUIAutomationElement * )> func);

    void getElementProperties(IUIAutomationElement *pElement,
                              ElementProperties& prop,
                              unsigned int loadProp = PROP_ALL);

    using FindTaskbarButtonsCallback = std::function<bool(const ElementProperties&)>;

    void
    findButtons(IUIAutomationTreeWalker *pWalk, IUIAutomationElement *pParent, FindTaskbarButtonsCallback callbackFn);
    bool isButtonAllowed(const ElementProperties& prop,
                         const StringSet& appNames,
                         const StringSet& exeNames,
                         const StringSet& autoIds);
    bool startsWith(const std::wstring& value, const StringSet& searchStrings) const;
    bool endsWith(const std::wstring& value, const StringSet& searchStrings) const;
    bool endsWithDotExe(const std::wstring& value) const;

    void findActiveButton(IUIAutomationTreeWalker *pWalk, IUIAutomationElement *pParent, std::wstring& result);

    std::vector<HWND> findRunningApplicationsToolBars() const;
};

} // namespace litelockr

#endif // __MINGW32__

#endif // UIAUTOMATION_HELPER_IMPL_H
