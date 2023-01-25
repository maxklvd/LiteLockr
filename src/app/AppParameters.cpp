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

#include "AppParameters.h"

#include <cassert>

#include <windows.h>
#include <shellapi.h>
#include "log/Logger.h"
#include "sys/StringUtils.h"

namespace litelockr {

bool AppParameters::hasInitialized_(false);
bool AppParameters::autoRun(false);
bool AppParameters::lock(false);
bool AppParameters::lockNow(false);
bool AppParameters::unlock(false);
bool AppParameters::hide(false);
bool AppParameters::show(false);
Log::Severity AppParameters::log{Log::Severity::None};
bool AppParameters::exit(false);

void AppParameters::initialize() {
    int numArgs;

    wchar_t **argList = CommandLineToArgvW(GetCommandLineW(), &numArgs);
    if (argList) {
        if (numArgs > 1) {
            for (int i = 0; i < numArgs; i++) {
                auto cmd = StringUtils::toUpperCase(argList[i]);

                if (cmd == L"/LOCK") {
                    AppParameters::lock = true;
                } else if (cmd == L"/LOCKNOW") {
                    AppParameters::lockNow = true;
                } else if (cmd == L"/UNLOCK") {
                    AppParameters::unlock = true;
                } else if (cmd == L"/HIDE") {
                    AppParameters::hide = true;
                } else if (cmd == L"/SHOW") {
                    AppParameters::show = true;
                } else if (cmd == L"/AUTORUN") {
                    AppParameters::autoRun = true;
                } else if (cmd == L"/LOG") {
                    AppParameters::log = Log::Severity::Info; // by default
                } else if (cmd.starts_with(L"/LOG=")) {
                    auto name = cmd.substr(5); // skip /LOG=
                    Log::SeverityName severityName;
                    AppParameters::log = severityName.getValue(name);
                } else if (cmd == L"/EXIT") {
                    AppParameters::exit = true;
                }
            }
        }
        LocalFree(argList);
    } else {
        assert(false);
    }

    //
    // parameter priorities
    //
    if (lockNow) {
        lock = false;
    }
    if (hide) {
        show = false;
    }

    hasInitialized_ = true;
}

} // namespace litelockr
                        