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

#ifndef APP_MESSAGES_H
#define APP_MESSAGES_H

#include <windows.h>

namespace litelockr {

constexpr UINT WMU_SHOW_YOURSELF = WM_APP + 1;
constexpr UINT WMU_LOCK_COMMAND = WM_APP + 2;
constexpr UINT WMU_LOCK_NOW_COMMAND = WM_APP + 3;
constexpr UINT WMU_UNLOCK_COMMAND = WM_APP + 4;
constexpr UINT WMU_HIDE_COMMAND = WM_APP + 5;
constexpr UINT WMU_SHOW_COMMAND = WM_APP + 6;
constexpr UINT WMU_EXIT_COMMAND = WM_APP + 7;
constexpr UINT WMU_TRAY_ICON_NOTIFY = WM_APP + 8;
constexpr UINT WMU_TRAY_ICON_UPDATE = WM_APP + 9;
constexpr UINT WMU_LOCALIZE_DIALOG = WM_APP + 11;

} // namespace litelockr

#endif // APP_MESSAGES_H
