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

#ifndef NOTIFICATION_AREA_H
#define NOTIFICATION_AREA_H

#include <string>

#include <windows.h>
#include "app/LockState.h"

namespace litelockr {

class NotificationArea {
public:
    static bool addIcon(LockIcon lockIcon);
    static bool deleteIcon();
    static bool updateIcon(LockIcon lockIcon);
    static bool showBalloon(bool isLocked, const std::wstring& text);
    static RECT getIconRect();
    static bool iconExists();

private:
    static HICON getIcon(LockIcon lockIcon);
    static std::wstring getTooltip(LockIcon lockIcon);
};

} // namespace litelockr

#endif // NOTIFICATION_AREA_H
