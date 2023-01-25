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

#ifndef UIAUTOMATION_HELPER_TYPES_H
#define UIAUTOMATION_HELPER_TYPES_H

#include <string>

#include <windows.h>

namespace litelockr {

enum ElementPropertyIds {
    PROP_ALL = 0xFF,
    PROP_NAME = 0x01,
    PROP_CLASS_NAME = 0x02,
    PROP_CONTROL_TYPE = 0x04,
    PROP_BOUNDING_RECTANGLE = 0x08,
    PROP_ACC_STATE = 0x10,
    PROP_AUTOMATION_ID = 0x20,
    PROP_PROCESS_ID = 0x40
};

struct ElementProperties {
    std::wstring name;
    std::wstring className;
    int controlType = 0;
    RECT boundingRectangle{};
    DWORD accState = 0;
    std::wstring automationId;
    DWORD processId = 0;
};

struct ButtonProperties: public ElementProperties {
    int buttonIndex = -1;
    int selPosition = -1;
    int selLength = 0;
};

enum SearchOptions {
    EXACT_MATCH = 0b0'0001,
    STARTS_WITH = 0b0'0010,
    ENDS_WITH = 0b0'0100,
};

} // namespace litelockr

#endif // UIAUTOMATION_HELPER_TYPES_H
