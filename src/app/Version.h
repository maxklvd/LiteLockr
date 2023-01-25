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

#ifndef VERSION_H
#define VERSION_H

#include "res/AppVersion.h"

namespace litelockr {

#define LTEXT2(x)       L ## x
#define LTEXT(x)        LTEXT2(x)

#if APP_VERSION_SUB2 > 0
// Version A.B.C.D
#define APP_VERSION LTEXT(TOSTRING(APP_VERSION_MAIN)) L"." \
                    LTEXT(TOSTRING(APP_VERSION_MAIN2)) L"." \
                    LTEXT(TOSTRING(APP_VERSION_SUB)) L"." \
                    LTEXT(TOSTRING(APP_VERSION_SUB2))
#else
// Version A.B.C
#define APP_VERSION LTEXT(TOSTRING(APP_VERSION_MAIN)) L"." \
                    LTEXT(TOSTRING(APP_VERSION_MAIN2)) L"." \
                    LTEXT(TOSTRING(APP_VERSION_SUB))
#endif

constexpr static auto APP_NAME = L"LiteLockr";

} // namespace litelockr

#endif //VERSION_H
