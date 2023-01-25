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

#include "NotificationArea.h"

#include <cassert>

#include <strsafe.h>
#include <shellapi.h>
#include <CommCtrl.h>
#include "app/AppMessages.h"
#include "app/Version.h"
#include "lang/Messages.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

#include "res/Resources.h"

namespace litelockr {

bool NotificationArea::addIcon(LockIcon lockIcon) {
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.hWnd = Process::mainWindow();
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
    nid.dwInfoFlags = NIIF_NONE;
    nid.uID = 1;
    nid.uCallbackMessage = WMU_TRAY_ICON_NOTIFY;
    nid.hIcon = getIcon(lockIcon);

    auto tip = getTooltip(lockIcon);
    StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), tip.c_str());

    Shell_NotifyIcon(NIM_ADD, &nid);

    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

bool NotificationArea::deleteIcon() {
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.hWnd = Process::mainWindow();
    nid.uID = 1;
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

bool NotificationArea::updateIcon(LockIcon lockIcon) {
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP;
    nid.hWnd = Process::mainWindow();
    nid.uID = 1;
    nid.hIcon = getIcon(lockIcon);

    auto tip = getTooltip(lockIcon);
    StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), tip.c_str());

    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

bool NotificationArea::iconExists() {
    return !Rectangle::empty(getIconRect());
}

bool NotificationArea::showBalloon(bool isLocked, const std::wstring& text) {
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_INFO | NIF_ICON | NIF_REALTIME;
    nid.hWnd = Process::mainWindow();
    nid.uID = 1;
    nid.dwInfoFlags = NIIF_NOSOUND;
    nid.hIcon = getIcon(isLocked ? LockIcon::Locked : LockIcon::Unlocked);

    StringCchCopy(nid.szInfo, ARRAYSIZE(nid.szInfo), text.c_str());
    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

HICON NotificationArea::getIcon(LockIcon lockIcon) {
    bool light = Process::isLightMode();
    int id;

    switch (lockIcon) {
        case LockIcon::Locked:
            id = light ? IDI_LOCKED__LIGHT_MODE : IDI_LOCKED__DARK_MODE;
            break;
        case LockIcon::LockNotAvailable:
            id = light ? IDI_LOCKED_WARNING__LIGHT_MODE : IDI_LOCKED_WARNING__DARK_MODE;
            break;
        default:
            id = light ? IDI_UNLOCKED__LIGHT_MODE : IDI_UNLOCKED__DARK_MODE;
            break;
    }

    HICON hIcon = nullptr;
    LoadIconMetric(Process::moduleInstance(), MAKEINTRESOURCE(id), LIM_SMALL, &hIcon);

    assert(hIcon);
    return hIcon;
}

std::wstring NotificationArea::getTooltip(LockIcon lockIcon) {
    std::wstring tooltip(APP_NAME);
    tooltip += L" - ";

    switch (lockIcon) {
        case LockIcon::Locked:
            tooltip += Messages::get(MessageId::Locked);
            break;
        case LockIcon::LockNotAvailable:
            tooltip += Messages::get(MessageId::LockNotAvailable);
            break;
        default:
            tooltip += Messages::get(MessageId::Unlocked);
            break;
    }

    return tooltip;
}

RECT NotificationArea::getIconRect() {
    NOTIFYICONIDENTIFIER nii = {sizeof(nii)};
    nii.hWnd = Process::mainWindow();
    nii.uID = 1;
    RECT rc{};
    Shell_NotifyIconGetRect(&nii, &rc);
    return rc;
}

} // namespace litelockr
