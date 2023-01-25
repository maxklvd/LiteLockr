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

#include "BinaryResource.h"

#include "sys/Process.h"

namespace litelockr {

unsigned char *Bin::data(int resourceId) {
    auto hInstance = Process::moduleInstance();
    auto hrSrc = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (hrSrc) {
        auto hGlob = LoadResource(hInstance, hrSrc);
        if (hGlob) {
            return static_cast<unsigned char *>(LockResource(hGlob));
        }
    }
    return nullptr;
}

unsigned long Bin::size(int resourceId) {
    auto hInstance = Process::moduleInstance();
    auto hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (hResource) {
        return SizeofResource(hInstance, hResource);
    }
    return 0;
}

} // namespace litelockr
