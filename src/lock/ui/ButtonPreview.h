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

#ifndef BUTTON_PREVIEW_H
#define BUTTON_PREVIEW_H

#include <string>

#include "lock/uia/UIAutomationHelperTypes.h"
#include "gui/LayeredWindow.h"
#include "sys/AppClock.h"

namespace litelockr {

class ButtonPreview {
public:
    class LayeredWnd: public LayeredWindow {
    public:
        virtual ~LayeredWnd() = default;

        friend class ButtonPreview;
    };

    void show(const ButtonProperties& buttonProp, int propertyId);
    void tick();
    void _destroy();

protected:
    void createButtonBorder(const RECT& rc);
    void createButtonText(const RECT& rc, const std::wstring& text, int selPosition = -1, int selLength = 0);

    TimeCounter delay_{std::chrono::seconds(2)};
    ButtonProperties properties;

    LayeredWnd buttonWnd_;
    LayeredWnd textWnd_;
};

} // namespace litelockr

#endif // BUTTON_PREVIEW_H
