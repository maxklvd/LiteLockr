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

#ifndef MOUSE_POSITION_VALIDATOR_H
#define MOUSE_POSITION_VALIDATOR_H

#include "ini/ApplicationRecord.h"
#include "lock/LockAreaMap.h"
#include "lock/LockAreaMapBuffer.h"
#include "lock/uia/UIAutomationHelper.h"
#include "sys/AppClock.h"

namespace litelockr {

class MousePositionValidator {
public:
    MousePositionValidator() = default;
    MousePositionValidator(const MousePositionValidator&) = delete;
    MousePositionValidator& operator=(const MousePositionValidator&) = delete;

    int width() const { return mapBuffer_.width(); }

    int height() const { return mapBuffer_.height(); }

    LockArea getLockArea(int cursorX, int cursorY) const {
        return mapBuffer_.getLockArea(cursorX, cursorY);
    }

    const LockAreaMap& currentMap() const {
        return mapBuffer_.currentMap();
    }

    void recreate();
    static void fillSearchSets(const ApplicationRecord& app,
                               UIAutomationHelper::StringSet& buttonNames,
                               UIAutomationHelper::StringSet& exeNames,
                               UIAutomationHelper::StringSet& autoIds);

    void swapUpdate() {
        mapBuffer_.swapUpdate();
    }

    enum PreviewMode {
        NONE = 0,
        CHECK_ALL,
        CHECK_APP,
    };

    void setPreviewMode(int previewMode) { previewMode_ = previewMode; }

private:
    LockAreaMapBuffer mapBuffer_;
    int previewMode_ = PreviewMode::NONE;
};

class MousePositionValidatorTimer {
public:
    enum {
        UPDATE_IMMEDIATELY = 0,
        DELAY_BEFORE_UPDATE = 500,
        DELAY_BETWEEN_UPDATES = 2000,
    };

    MousePositionValidatorTimer() {
        delayBetweenUpdates_.timeout(); // initialize it
    }

    MousePositionValidatorTimer(const MousePositionValidatorTimer&) = delete;
    MousePositionValidatorTimer& operator=(const MousePositionValidatorTimer&) = delete;

    void markNeedsUpdate(DWORD delayBefore = DELAY_BEFORE_UPDATE);
    bool ready();

private:
    bool needsUpdate_ = false;
    OneShotTimeCounter delayBeforeUpdate_{std::chrono::milliseconds(DELAY_BEFORE_UPDATE)};
    TimeCounter delayBetweenUpdates_{std::chrono::milliseconds(DELAY_BETWEEN_UPDATES)};
};

} // namespace litelockr

#endif // MOUSE_POSITION_VALIDATOR_H
