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

#ifndef KEYBOARD_FILTER_H
#define KEYBOARD_FILTER_H

#include <functional>
#include <optional>

#include "lock/HookOptions.h"
#include "lock/KeyStroke.h"
#include "lock/ModifierKeys.h"

namespace litelockr {

class KeyboardFilter {
public:
    static void install(const HookOptions& options);
    static void uninstall();
    static bool processKeyStroke(const KeyStroke& stroke);

    static const ModifierKeys& modifierKeys() { return modKey_; }

private:
    KeyboardFilter() = default;

    static void updateModKeys(unsigned key, bool isKeyDown);

    static std::optional<std::reference_wrapper<const HookOptions>> options_;
    static ModifierKeys modKey_;

    constexpr static int KEY_PRESSED_SIZE = 256;
    static bool keyPressed_[KEY_PRESSED_SIZE];
};

} // namespace litelockr

#endif // KEYBOARD_FILTER_H
