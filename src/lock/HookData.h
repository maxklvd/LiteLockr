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

#ifndef HOOK_DATA_H
#define HOOK_DATA_H

#include <atomic>

#include "lock/WindowValidator.h"

namespace litelockr {

class HookData {
public:
    static std::atomic<bool> hotkeyPressedFlag;
    static std::atomic<bool> ctrlAltDelPressedFlag;
    static std::atomic<bool> sessionActive;

    struct FilterFlags {
        std::atomic<bool> sessionReconnected{false};
    };
    static FilterFlags keyboard;
    static FilterFlags mouse;

    static void setSessionActive(bool isActive);
    static bool isSessionActive();

    static WindowValidator& windowValidator() {
        static WindowValidator windowValidator_;
        return windowValidator_;
    }
};

} // namespace litelockr

#endif // HOOK_DATA_H
