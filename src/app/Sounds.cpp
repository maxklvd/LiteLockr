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

#include "Sounds.h"

#include <thread>

#include "ini/SettingsData.h"
#include "sys/BinaryResource.h"
#include "sys/Process.h"

namespace litelockr {

void Sounds::play(int resourceId) {
    if (!SettingsData::instance().playSounds.value()) {
        // All sounds are disabled
        return;
    }

    //
    // .WAV data pointer
    //
    auto soundData = reinterpret_cast<LPCWSTR>(Bin::data(resourceId));

    //
    // Play the sound on a separate independent thread
    //
    std::thread thd([soundData]() {
        PlaySound(soundData, Process::moduleInstance(), SND_MEMORY);
    });
    thd.detach();
}

} // namespace litelockr
