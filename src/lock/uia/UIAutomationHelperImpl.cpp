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

#include "UIAutomationHelperImpl.h"

#ifndef __MINGW32__

#include <cassert>

#include <oleauto.h>
#include "gui/WindowUtils.h"
#include "log/Logger.h"
#include "sys/Executable.h"
#include "sys/StringUtils.h"

namespace litelockr {

void UIAutomationHelperImpl::initialize() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    assert(SUCCEEDED(hr));

    assert(uiaPtr_ == nullptr);
    hr = CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation),
                          (void **) &uiaPtr_);
    if (SUCCEEDED(hr)) {
        assert(uiaPtr_);
    } else {
        assert(false);
    }
}

void UIAutomationHelperImpl::dispose() {
    assert(uiaPtr_);

    if (uiaPtr_) {
        uiaPtr_->Release();
        uiaPtr_ = nullptr;
    }

    CoUninitialize();
}

std::wstring UIAutomationHelperImpl::getAutomationIdOfActiveButton() {
    std::wstring result;

    //
    // Process the running app buttons on the taskbar
    //
    auto toolbars = findRunningApplicationsToolBars();
    assert(!toolbars.empty());

    for (auto hWnd: toolbars) {
        IUIAutomationElement *pElement = nullptr;
        IUIAutomationTreeWalker *pWalk = nullptr;

        HRESULT hr = uiaPtr_->ElementFromHandle(hWnd, &pElement);
        if (SUCCEEDED(hr) && pElement) {
            uiaPtr_->get_ControlViewWalker(&pWalk);
            if (SUCCEEDED(hr) && pWalk) {
                findActiveButton(pWalk, pElement, result);
            } else {
                assert(false);
            }
        } else {
            assert(false);
        }

        safeRelease(pWalk);
        safeRelease(pElement);
    }
    return result;
}

void UIAutomationHelperImpl::findActiveButton(IUIAutomationTreeWalker *pWalk, IUIAutomationElement *pParent,
                                              std::wstring& result) {
    assert(pWalk);
    assert(pParent);
    if (pWalk == nullptr || pParent == nullptr) {
        return;
    }

    int loadProp = PROP_CONTROL_TYPE | PROP_AUTOMATION_ID | PROP_ACC_STATE;

    iterateChildren(pWalk, pParent, [this, pWalk, loadProp, &result](IUIAutomationElement *pNode) -> bool {
        ElementProperties prop;
        getElementProperties(pNode, prop, loadProp);
        if (prop.controlType == UIA_PaneControlTypeId || prop.controlType == UIA_ToolBarControlTypeId) {
            findActiveButton(pWalk, pNode, result); // NOTE: recursion here
        } else if (prop.controlType == UIA_ButtonControlTypeId || prop.controlType == UIA_MenuItemControlTypeId) {
            if (prop.accState & STATE_SYSTEM_PRESSED) {
                result = prop.automationId;

                // stop
                pNode->Release();
                return true;
            }
        }
        return false; //continue
    });
}

//
ButtonProperties UIAutomationHelperImpl::getTaskbarButton(int index) {
    ButtonProperties result;

    //
    // Process the running app buttons on the taskbar
    //
    auto toolbars = findRunningApplicationsToolBars();
    assert(!toolbars.empty());

    for (auto hWnd: toolbars) {
        IUIAutomationElement *pElement = nullptr;
        IUIAutomationTreeWalker *pWalk = nullptr;

        HRESULT hr = uiaPtr_->ElementFromHandle(hWnd, &pElement);
        if (SUCCEEDED(hr) && pElement) {
            uiaPtr_->get_ControlViewWalker(&pWalk);
            if (SUCCEEDED(hr) && pWalk) {
                int buttonIdx = 0;
                findButtons(pWalk, pElement,
                            [this, index, &buttonIdx, &result](const ElementProperties& prop) {
                                result.buttonIndex = buttonIdx;
                                result.automationId = prop.automationId;
                                result.name = prop.name;
                                result.boundingRectangle = prop.boundingRectangle;

                                buttonIdx++;

                                if (result.buttonIndex == index) {
                                    return true; // stop
                                }
                                return false; // continue
                            });
            } else {
                assert(false);
            }
        } else {
            assert(false);
        }

        safeRelease(pWalk);
        safeRelease(pElement);
    }

    return result;
}

ButtonProperties UIAutomationHelperImpl::findTaskbarButton(int propertyId,
                                                           const StringSet& searchValues, int searchOptions) {
    ButtonProperties result;

    //
    // Process the running app buttons on the taskbar
    //
    auto toolbars = findRunningApplicationsToolBars();
    assert(!toolbars.empty());

    for (auto hWnd: toolbars) {
        IUIAutomationElement *pElement = nullptr;
        IUIAutomationTreeWalker *pWalk = nullptr;

        HRESULT hr = uiaPtr_->ElementFromHandle(hWnd, &pElement);
        if (SUCCEEDED(hr) && pElement) {
            uiaPtr_->get_ControlViewWalker(&pWalk);
            if (SUCCEEDED(hr) && pWalk) {
                int buttonIdx = 0;
                findButtons(pWalk, pElement,
                            [this, propertyId, &searchValues, searchOptions, &buttonIdx, &result](
                                    const ElementProperties& prop) {
                                std::wstring value;

                                switch (propertyId) {
                                    case PROP_NAME:
                                        value = StringUtils::prepareSearchString(prop.name);
                                        break;
                                    case PROP_AUTOMATION_ID:
                                        value = StringUtils::prepareSearchString(prop.automationId);
                                        break;
                                }

                                if (value.empty()) {
                                    return false; // continue
                                }

                                bool found = false;
                                bool exactMatch = (searchOptions & EXACT_MATCH);
                                bool startsWith = (searchOptions & STARTS_WITH);
                                bool endsWith = (searchOptions & ENDS_WITH);

                                for (const auto& searchValue: searchValues) {
                                    if (searchValue.empty()) {
                                        continue;
                                    }
                                    if (exactMatch && value == searchValue) {
                                        found = true;
                                        result.selPosition = 0;
                                        result.selLength = value.size();
                                        break;
                                    }
                                    if (startsWith && value.starts_with(searchValue)) {
                                        found = true;
                                        result.selPosition = 0;
                                        result.selLength = searchValue.size();
                                        break;
                                    }
                                    if (endsWith && value.ends_with(searchValue)) {
                                        found = true;
                                        result.selPosition = value.size() - searchValue.size();
                                        result.selLength = searchValue.size();
                                        break;
                                    }
                                }

                                if (found) {
                                    result.buttonIndex = buttonIdx;
                                    result.automationId = prop.automationId;
                                    result.name = prop.name;
                                    result.boundingRectangle = prop.boundingRectangle;
                                    return true; // stop
                                }

                                buttonIdx++;
                                return false; // continue
                            });
            } else {
                assert(false);
            }
        } else {
            assert(false);
        }

        safeRelease(pWalk);
        safeRelease(pElement);
    }

    return result;
}

void UIAutomationHelperImpl::iterateChildren(IUIAutomationTreeWalker *pWalk,
                                             IUIAutomationElement *pParent,
                                             std::function<bool(IUIAutomationElement * )> func) {
    assert(pWalk);
    assert(pParent);

    IUIAutomationElement *pNode = nullptr;
    HRESULT hr = pWalk->GetFirstChildElement(pParent, &pNode);
    if (SUCCEEDED(hr) && pNode) {
        do {
            if (func(pNode)) {
                break;
            }

            IUIAutomationElement *pNext = nullptr;
            hr = pWalk->GetNextSiblingElement(pNode, &pNext);
            pNode->Release();
            pNode = pNext;
        } while (SUCCEEDED(hr) && pNode);
    }
}

void UIAutomationHelperImpl::getElementProperties(IUIAutomationElement *pElement,
                                                  ElementProperties& prop,
                                                  unsigned int loadProp) {
    BSTR str;
    prop = {};

    assert(pElement);
    if (!pElement) {
        return;
    }

    HRESULT hr;
    if (loadProp & PROP_NAME) {
        //
        // Current Name
        //
        hr = pElement->get_CurrentName(&str);
        if (SUCCEEDED(hr) && SysStringLen(str) > 0) {
            prop.name = std::wstring(str, SysStringLen(str));
            SysFreeString(str);
        }
    }

    if (loadProp & PROP_CLASS_NAME) {
        //
        // Current Class Name
        //
        hr = pElement->get_CurrentClassName(&str);
        if (SUCCEEDED(hr) && SysStringLen(str) > 0) {
            prop.className = std::wstring(str, SysStringLen(str));
            SysFreeString(str);
        }
    }

    if (loadProp & PROP_CONTROL_TYPE) {
        //
        // Current Control Type
        //
        CONTROLTYPEID typeId;
        hr = pElement->get_CurrentControlType(&typeId);
        if (SUCCEEDED(hr)) {
            prop.controlType = typeId;
        }
    }

    if (loadProp & PROP_BOUNDING_RECTANGLE) {
        //
        // Current Bounding Rectangle
        //
        RECT rc = {0};
        hr = pElement->get_CurrentBoundingRectangle(&rc);
        if (SUCCEEDED(hr)) {
            prop.boundingRectangle = rc;
        }
    }

    if (loadProp & PROP_ACC_STATE) {
        //
        // Current LegacyIAccessibleState
        //
        VARIANT val;
        hr = pElement->GetCurrentPropertyValue(UIA_LegacyIAccessibleStatePropertyId, &val);
        if (SUCCEEDED(hr) && val.vt == VT_I4) {
            prop.accState = val.lVal;
        }
    }

    if (loadProp & PROP_AUTOMATION_ID) {
        //
        // Current AutomationId
        //
        hr = pElement->get_CurrentAutomationId(&str);
        if (SUCCEEDED(hr) && SysStringLen(str) > 0) {
            prop.automationId = std::wstring(str, SysStringLen(str));
            SysFreeString(str);
        }
    }

    if (loadProp & PROP_PROCESS_ID) {
        //
        // Current ProcessId
        //
        int val;
        hr = pElement->get_CurrentProcessId(&val);
        if (SUCCEEDED(hr)) {
            prop.processId = val;
        }
    }
}

void UIAutomationHelperImpl::findTaskbarButtons(const StringSet& appNames,
                                                const StringSet& exeNames,
                                                const StringSet& autoIds,
                                                ButtonPropertiesList& result) {
    //
    // Process the running app buttons on the taskbar
    //
    auto toolbars = findRunningApplicationsToolBars();
    assert(!toolbars.empty());

    for (auto hWnd: toolbars) {
        IUIAutomationElement *pElement = nullptr;
        IUIAutomationTreeWalker *pWalk = nullptr;

        HRESULT hr = uiaPtr_->ElementFromHandle(hWnd, &pElement);
        if (SUCCEEDED(hr) && pElement) {
            uiaPtr_->get_ControlViewWalker(&pWalk);
            if (SUCCEEDED(hr) && pWalk) {
                int buttonIdx = 0;
                findButtons(pWalk, pElement,
                            [this, &buttonIdx, &appNames, &exeNames, &autoIds, &result](const ElementProperties& prop) {
                                if (isButtonAllowed(prop, appNames, exeNames, autoIds)) {
                                    // adds a rectangle
                                    ButtonProperties buttonProp(prop);
                                    buttonProp.buttonIndex = buttonIdx;
                                    result.push_back(buttonProp);
                                }

                                buttonIdx++;

                                return false; // continue
                            });
            } else {
                assert(false);
            }
        } else {
            assert(false);
        }

        safeRelease(pWalk);
        safeRelease(pElement);
    }
}

void UIAutomationHelperImpl::findButtons(IUIAutomationTreeWalker *pWalk, IUIAutomationElement *pParent,
                                         FindTaskbarButtonsCallback callbackFn) {
    assert(pWalk);
    assert(pParent);
    if (pWalk == nullptr || pParent == nullptr) {
        return;
    }

    unsigned int loadProp = PROP_CONTROL_TYPE | PROP_NAME | PROP_BOUNDING_RECTANGLE | PROP_AUTOMATION_ID;

    iterateChildren(pWalk, pParent, [this, pWalk, loadProp, &callbackFn](IUIAutomationElement *pNode) -> bool {
        ElementProperties prop;
        getElementProperties(pNode, prop, loadProp);
        if (prop.controlType == UIA_PaneControlTypeId || prop.controlType == UIA_ToolBarControlTypeId) {
            findButtons(pWalk, pNode, callbackFn); // NOTE: recursion here
        } else if (prop.controlType == UIA_ButtonControlTypeId || prop.controlType == UIA_MenuItemControlTypeId) {
            if (callbackFn(prop)) {
                pNode->Release();
                return true; // stop
            }
        }
        return false; //continue
    });
}

bool UIAutomationHelperImpl::isButtonAllowed(const ElementProperties& prop,
                                             const StringSet& appNames,
                                             const StringSet& exeNames,
                                             const StringSet& autoIds) {
    //
    // AutomationId
    //
    if (!prop.automationId.empty()) {
        //
        // 1) Search by AutomationId
        //
        if (autoIds.find(prop.automationId) != autoIds.end()) {
            LOG_DEBUG(L"[Allowed Button] AutomationId = %s", prop.automationId.c_str());
            return true;
        }

        //
        // 2) Search by application .exe name that may exists at the end of AutomationId
        //
        if (endsWithDotExe(prop.automationId)) {
            std::wstring exeName = StringUtils::toUpperCase(Executable::getFileName(prop.automationId));

            if (exeNames.find(exeName) != exeNames.end()) {
                LOG_DEBUG(L"[Allowed Button] .exe in automationId [%s] [%s]",
                          prop.automationId.c_str(), exeName.c_str());
                return true;
            }
        }
    }

    //
    // Button name
    //
    if (!prop.name.empty()) {
        std::wstring buttonName = StringUtils::prepareSearchString(prop.name);

        //
        // 3) Search by startsWith
        //
        if (startsWith(buttonName, appNames)) {
            LOG_DEBUG(L"[Allowed Button] starts with [%s]", prop.name.c_str());
            return true;
        }

        //
        // 4) Search by endsWith
        //
        if (endsWith(buttonName, appNames)) {
            LOG_DEBUG(L"[Allowed Button] ends with [%s]", prop.name.c_str());
            return true;
        }
    }

    return false;
}

bool UIAutomationHelperImpl::startsWith(const std::wstring& value, const StringSet& searchStrings) const {
    for (const auto& searchString: searchStrings) {
        if (value.starts_with(searchString)) {
            return true;
        }
    }
    return false;
}

bool UIAutomationHelperImpl::endsWith(const std::wstring& value, const StringSet& searchStrings) const {
    for (const auto& searchString: searchStrings) {
        if (value.ends_with(searchString)) {
            return true;
        }
    }
    return false;
}

bool UIAutomationHelperImpl::endsWithDotExe(const std::wstring& str) const {
    if (str.size() < 4) {
        return false;
    }

    int dot = str.size() - 4;
    auto d0 = str.at(dot + 0);
    auto e1 = str.at(dot + 1);
    auto x2 = str.at(dot + 2);
    auto e3 = str.at(dot + 3);

    return (d0 == L'.' &&
            (e1 == L'e' || e1 == L'E') &&
            (x2 == L'x' || x2 == L'X') &&
            (e3 == L'e' || e3 == L'E'));
}

std::vector<HWND> UIAutomationHelperImpl::findRunningApplicationsToolBars() const {
    std::vector<HWND> wnds;

    WindowUtils::findRunningApplicationsToolBars([&wnds](HWND hWnd) {
        wnds.push_back(hWnd);
    });
    return wnds;
}

} // namespace litelockr

#endif // __MINGW32__
