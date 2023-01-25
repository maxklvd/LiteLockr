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

#ifndef APPLICATION_FRAME_HOST_CFG_H
#define APPLICATION_FRAME_HOST_CFG_H

#include "lock/config/AppCfg.h"

namespace litelockr {

class ApplicationFrameHostCfg: public AppCfg {
public:
    std::wstring executable() const override { return ApplicationFrameHostExe; }

    bool shouldSkipApp() const override { return true; }

    bool findRootWindow(HWND& hWnd) const override;

    constexpr static auto ApplicationFrameHostExe = L"APPLICATIONFRAMEHOST.EXE";
    constexpr static auto ApplicationFrameWindowClass = L"ApplicationFrameWindow";
    constexpr static auto CoreWindowClass = L"Windows.UI.Core.CoreWindow";

private:
    static bool isApplicationFrameWindow(HWND hWnd);
    static bool isCoreWindow(HWND hWnd);
};

} // namespace litelockr

#endif // APPLICATION_FRAME_HOST_CFG_H
