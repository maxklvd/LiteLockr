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

#include "MousePositionValidator.h"

#include <algorithm>
#include <cassert>
#include <future>

#include "app/AppParameters.h"
#include "app/NotificationArea.h"
#include "ini/SettingsData.h"
#include "lock/DisplayMonitors.h"
#include "lock/uia/UIAutomationHelper.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"
#include "sys/StringUtils.h"

namespace litelockr {

void MousePositionValidator::fillSearchSets(const ApplicationRecord& app,
                                            UIAutomationHelper::StringSet& buttonNames,
                                            UIAutomationHelper::StringSet& exeNames,
                                            UIAutomationHelper::StringSet& autoIds) {

    if (app.enabled) {
        if (app.findByAutomationId()) {
            if (!app.buttonAutomationId().empty()) {
                autoIds.insert(app.buttonAutomationId());
            }
            if (app.searchExecutableInAutomationId()) {
                exeNames.insert(StringUtils::toUpperCase(app.exeName()));
            }
        }
        if (app.findByName()) {
            for (const auto& name: app.getButtonNames()) {
                buttonNames.insert(StringUtils::prepareSearchString(name));
            }
        }
    }
}

void MousePositionValidator::recreate() {
    assert(GetCurrentThreadId() == Process::mainThreadId());

    MonitorInfo mi{};
    DisplayMonitors::update();
    DisplayMonitors::get(mi);

    const auto& settings = SettingsData::instance();
    //
    // button rects
    //
    UIAutomationHelper::StringSet buttonNames, exeNames, autoIds;
    for (const auto& app: settings.getAllowedApps()) {
        fillSearchSets(app, buttonNames, exeNames, autoIds);
    }
    if (previewMode_ == 0 && !Process::currentAppAutomationId().empty()) {
        autoIds.insert(Process::currentAppAutomationId());
    }

    auto buttonRectsFuture = std::async(std::launch::async, [buttonNames, exeNames, autoIds]() {
        UIAutomationHelper uia;
        UIAutomationHelper::ButtonPropertiesList propList;
        uia.findTaskbarButtons(buttonNames, exeNames, autoIds, propList);

        UIAutomationHelper::RectList rects;
        std::ranges::transform(propList, std::back_inserter(rects),
                               [](ButtonProperties& prop) -> RECT { return prop.boundingRectangle; });
        return rects;
    });
    UIAutomationHelper::RectList butRects = buttonRectsFuture.get();

    UIAutomationHelper::RectList trayIconRects;
    if (!settings.hideTrayIconWhenLocked.value() && previewMode_ != PreviewMode::CHECK_APP) {
        RECT iconRect = NotificationArea::getIconRect();
        if (!Rectangle::empty(iconRect)) {
            trayIconRects.push_back(iconRect);
        }
    }


    mapBuffer_.update([&settings, &mi, &butRects, &trayIconRects](LockAreaMap& map) {
        map.recreate();
        if (settings.lockMouse.value()) {
            //
            // Mouse Lock is enabled
            //
            if (settings.allowMouseMovement.value()) {
                // allowMouseMovement is on
                map.fill(LockAreaMap::IDX_DENY);          // everything allowed
                map.addRects(mi.workAreas, LockArea::ALLOW, true, true);
                map.addRects(butRects, LockArea::TASKBAR_BUTTON, true);
                map.addRects(trayIconRects, LockArea::NOTIFICATION_AREA_ICON, true);
            } else {
                // allowMouseMovement is off
                map.fill(LockAreaMap::IDX_DENY);            // nothing allowed
            }
        } else {
            //
            // Mouse Lock is disabled, nothing to lock
            //
            map.fill(LockAreaMap::IDX_ALLOW);
        }
    });
}

void MousePositionValidatorTimer::markNeedsUpdate(DWORD delayBefore /*= DELAY_BEFORE_UPDATE*/) {
    delayBeforeUpdate_.recreate(std::chrono::milliseconds(delayBefore));
    delayBeforeUpdate_.reset();
    needsUpdate_ = true;
}

bool MousePositionValidatorTimer::ready() {
    if (needsUpdate_ && delayBeforeUpdate_.timeout()) {
        if (delayBetweenUpdates_.timeout()) {
            delayBeforeUpdate_.reset();
            delayBetweenUpdates_.reset();
            needsUpdate_ = false;
            return true;
        }
    }
    return false;
}

} // namespace litelockr
