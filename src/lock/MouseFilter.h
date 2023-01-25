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

#ifndef MOUSE_FILTER_H
#define MOUSE_FILTER_H

#include <algorithm>
#include <functional>
#include <optional>

#include "lock/DblClickRecognizer.h"
#include "lock/HookOptions.h"
#include "lock/MousePositionValidator.h"
#include "lock/MouseStroke.h"
#include "sys/AppClock.h"

namespace litelockr {

class MouseFilter {
public:
    static void install(const HookOptions& options);
    static void uninstall();
    static bool processMouseStroke(MouseStroke& stroke);
    static void tick();
    static void updatePositionValidator(DWORD delayBefore);

private:
    MouseFilter() = default;

    static bool isPositionAllowed(const POINT& cursorPosition, LockArea& area);
    static BOOL clipCursor(const RECT *rect);
    static BOOL getClipCursor(RECT *rect);

    static std::optional<std::reference_wrapper<const HookOptions>> options_;

    static MousePositionValidator positionValidator_;
    static MousePositionValidatorTimer positionValidatorTimer_;
    static RECT previousClipCursor_;
    static DblClickRecognizer dblClickDetector_;

    static TimeCounter checkTrayIconPeriod_;
    static RECT trayIconRect_;

    constexpr static int BUTTON_PRESSED_SIZE = 16;
    static bool buttonPressed_[BUTTON_PRESSED_SIZE];
};

} // namespace litelockr

#endif // MOUSE_FILTER_H
