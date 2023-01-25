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

#ifndef EVENT_INTERCEPTION_H
#define EVENT_INTERCEPTION_H

namespace litelockr {

struct EventInterception {
    enum {
        NULL_HOOK = -1,
        GLOBAL_WINDOWS_HOOK = 0,
        INTERCEPTION_DRIVER = 1,
    };

    constexpr static auto NullHook = L"Null Hook";
    constexpr static auto GlobalWindowsHook = L"Global Windows Hook";
    constexpr static auto InterceptionDriver = L"Interception Driver";

    constexpr static auto getString(int value) {
        switch (value) {
            case NULL_HOOK:
                return NullHook;
            case GLOBAL_WINDOWS_HOOK:
                return GlobalWindowsHook;
            case INTERCEPTION_DRIVER:
                return InterceptionDriver;
            default:
                return L"";
        }
    }
};

} // namespace litelockr

#endif // EVENT_INTERCEPTION_H
