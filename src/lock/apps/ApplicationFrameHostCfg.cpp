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

#include "ApplicationFrameHostCfg.h"

#include "gui/WindowUtils.h"

namespace litelockr {

bool ApplicationFrameHostCfg::findRootWindow(HWND& hWnd) const {
    HWND hWndRootOwner = GetAncestor(hWnd, GA_ROOTOWNER);

    if (isApplicationFrameWindow(hWndRootOwner)) {
        HWND hWndChild = GetWindow(hWndRootOwner, GW_CHILD);
        while (IsWindow(hWndChild)) {
            if (isCoreWindow(hWndChild)) {
                hWnd = hWndChild;
                return true;
            }
            hWndChild = GetWindow(hWndChild, GW_HWNDNEXT);
        }
    }
    return false;
}

bool ApplicationFrameHostCfg::isApplicationFrameWindow(HWND hWnd) {
    return WindowUtils::getClassName(hWnd) == ApplicationFrameWindowClass;
}

bool ApplicationFrameHostCfg::isCoreWindow(HWND hWnd) {
    return WindowUtils::getClassName(hWnd) == CoreWindowClass;
}


} // namespace litelockr