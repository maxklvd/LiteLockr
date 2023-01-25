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

#ifndef CONTROL_ACCESSOR_H
#define CONTROL_ACCESSOR_H

#include "ini/Property.h"
#include "gui/Window.h"

namespace litelockr {

template<typename T>
concept PropertyT = (std::derived_from<T, Property>);

class ControlAccessor {
public:
    explicit ControlAccessor(const HWND& hDlg) : hDlg_(hDlg) {}

    template<typename T>
    requires std::integral<T>
    void readValue(unsigned controlId, T& value) const {
        assert(IsWindow(hDlg_));

        if constexpr (std::same_as<T, bool>) {
            auto val = SendDlgItemMessage(hDlg_, controlId, BM_GETCHECK, 0, 0L);
            value = val; // 0 = NO, 1 = YES, 2 = unsupported (YES)
        } else {
            BOOL isTranslated = TRUE;
            BOOL isSigned = std::is_signed<T>::value;
            auto val = GetDlgItemInt(hDlg_, controlId, &isTranslated, isSigned);
            if (isTranslated) {
                value = val;
            }
        }
    }

    template<PropertyT T>
    requires std::integral<typename T::ValueType>
    void readValue(unsigned controlId, T& property) const {
        auto value = property.defaultValue();
        readValue(controlId, value);
        property.setValue(value);
    }

    template<typename T>
    requires std::integral<T>
    void writeValue(unsigned controlId, const T& value) const {
        assert(IsWindow(hDlg_));

        if constexpr (std::same_as<T, bool>) {
            SendDlgItemMessage(hDlg_, controlId, BM_SETCHECK, static_cast<WPARAM>(value), 0L);
        } else {
            BOOL isSigned = std::is_signed<T>::value;
            SetDlgItemInt(hDlg_, controlId, static_cast<UINT>(value), isSigned);
        }
    }

    template<PropertyT T>
    requires std::integral<typename T::ValueType>
    void writeValue(unsigned controlId, const T& property) const {
        auto value = property.value();
        writeValue(controlId, value);
    }

private:
    const HWND& hDlg_;
};

} // namespace litelockr

#endif // CONTROL_ACCESSOR_H
