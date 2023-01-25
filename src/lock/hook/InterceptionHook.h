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

#ifndef INTERCEPTION_HOOK_H
#define INTERCEPTION_HOOK_H

#include <atomic>

#include <interception/interception.h>
#include "lock/DisplayMonitors.h"
#include "lock/KeyboardFilter.h"
#include "lock/MouseFilter.h"
#include "lock/hook/AbstractHook.h"

namespace litelockr {

class InterceptionHook: public AbstractHook {
public:
    void initialize(const HookOptions& options) override;
    void dispose() override;
    void start() override;
    void stop() override;

    bool canBeUsed() override {
        return isDriverInstalled();
    };
    static bool isDriverInstalled();

private:
    MouseStroke getMouseStroke(const InterceptionMouseStroke& mstroke) const;
    KeyStroke getKeyStroke(const InterceptionKeyStroke& kstroke) const;
    [[nodiscard]] unsigned short getVirtualKey(unsigned short scanCode, bool e0) const;

    struct KeyCode {
        unsigned short vkCode = 0;
        unsigned short scanCode = 0;
        bool e0 = false;
    };
    KeyCode hotkeyCode_{};

    static std::atomic<bool> interceptionAlive_;
    mutable POINT position_{};
    MonitorInfo monitorInfo_;
};

} // namespace litelockr

#endif // INTERCEPTION_HOOK_H
