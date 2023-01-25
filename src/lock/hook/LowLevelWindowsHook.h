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

#ifndef LOW_LEVEL_WINDOWS_HOOK_H
#define LOW_LEVEL_WINDOWS_HOOK_H

#include <windows.h>
#include "lock/hook/AbstractHook.h"

namespace litelockr {

class LowLevelWindowsHook: public AbstractHook {
public:
    void initialize(const HookOptions& options) override;
    void dispose() override;

    bool canBeUsed() override { return true; }

    void start() override;
    void stop() override;

private:
    static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);

    static HHOOK hKeyboardHook_;
    static HHOOK hMouseHook_;
};

} // namespace litelockr

#endif // LOW_LEVEL_WINDOWS_HOOK_H
