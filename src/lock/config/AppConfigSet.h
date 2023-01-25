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

#ifndef APP_CONFIG_SET_H
#define APP_CONFIG_SET_H

#include <memory>
#include <string>
#include <unordered_map>

#include "AppCfg.h"

namespace litelockr {

class AppConfigSet {
public:
    AppConfigSet();
    AppConfigSet(AppConfigSet&) = delete;
    AppConfigSet(AppConfigSet&&) = delete;

    bool shouldSkipApp(const std::wstring& executable) const;
    bool shouldSkipApp(HWND hWnd) const;
    HWND findRootWindow(HWND hWnd) const;

private:
    std::unordered_map<std::wstring, std::unique_ptr<AppCfg>> map_;

    template<typename T>
    requires std::derived_from<T, AppCfg>
    void add() {
        auto cfg = std::make_unique<T>();
        map_.insert({cfg->executable(), std::move(cfg)});
    }
};

} // namespace litelockr

#endif // APP_CONFIG_SET_H
