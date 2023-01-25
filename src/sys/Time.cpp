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

#include "Time.h"

#include "sys/Process.h"

namespace litelockr {

unsigned long Time::getTime() {
    return timeGetTime();
}

bool Time::getLastInputTime(unsigned long& time) {
    if (Process::isRemoteSession()) {
        // the program does not designed to work with RDP
        return false;
    }

    LASTINPUTINFO info = {sizeof(info)};
    if (GetLastInputInfo(&info)) {
        time = info.dwTime;
        return true;
    }
    return false;
}

} // namespace litelockr
