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

#include "HookFactory.h"

#include "lock/hook/EventInterception.h"
#include "lock/hook/InterceptionHook.h"
#include "lock/hook/LowLevelWindowsHook.h"
#include "lock/hook/NullHook.h"

namespace litelockr {

std::unique_ptr<AbstractHook> HookFactory::create(int eventInterception) {
    switch (eventInterception) {
        case EventInterception::NULL_HOOK:
            return std::make_unique<NullHook>();
        case EventInterception::INTERCEPTION_DRIVER:
            return std::make_unique<InterceptionHook>();
        default:
            return std::make_unique<LowLevelWindowsHook>();
    }
}

} // namespace litelockr
