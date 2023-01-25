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

#ifndef DBL_CLICK_RECOGNIZER_H
#define DBL_CLICK_RECOGNIZER_H

#include <windows.h>
#include "sys/AppClock.h"

namespace litelockr {

class DblClickRecognizer {
public:
    DblClickRecognizer() = default;
    DblClickRecognizer(const DblClickRecognizer&) = delete;
    DblClickRecognizer& operator=(const DblClickRecognizer&) = delete;

    void initialize();
    bool isDoubleClick(POINT pt);

private:
    int cxDoubleClk_ = 0;
    int cyDoubleClk_ = 0;
    AppClock::Duration doubleClickTime_{};
    AppClock::TimePoint lastClickTime_{};
    POINT lastClickPos_{};
};

} // namespace litelockr

#endif // DBL_CLICK_RECOGNIZER_H
