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

#include "ButtonPreview.h"

#include <cassert>

#include "gfx/BitmapContext.h"
#include "lock/ui/PreviewStyle.h"
#include "sys/Process.h"
#include "sys/Rectangle.h"

namespace litelockr {

void ButtonPreview::show(const ButtonProperties& buttonProp, int propertyId) {
    _destroy();

    if (buttonProp.buttonIndex < 0) {
        return;
    }

    std::wstring text;
    int selPosition = buttonProp.selPosition;
    int selLength = buttonProp.selLength;

    switch (propertyId) {
        case PROP_NAME:
            text = buttonProp.name;
            break;
        case PROP_AUTOMATION_ID:
            text = buttonProp.automationId;
            break;
        default:
            assert(false);
            break;
    }

    createButtonBorder(buttonProp.boundingRectangle);
    if (!text.empty()) {
        createButtonText(buttonProp.boundingRectangle, text, selPosition, selLength);
    }
    delay_.reset();
}

void ButtonPreview::tick() {
    if (delay_.timeout()) {
        _destroy();
    }
}

void ButtonPreview::_destroy() {
    if (buttonWnd_.hWnd) {
        buttonWnd_.destroyWindow();
    }
    if (textWnd_.hWnd) {
        textWnd_.destroyWindow();
    }
}

void ButtonPreview::createButtonBorder(const RECT& rc) {
    using namespace PreviewStyle;

    unsigned w = Rectangle::width(rc);
    unsigned h = Rectangle::height(rc);
    buttonWnd_.buffer_.create(w, h);
    BitmapContext ctx(buttonWnd_.buffer_);
    ctx.addRect(0.5, 0.5, w - 1, h - 1);
    ctx.strokeColor = ButtonBorderColor;
    ctx.lineWidth = ButtonBorderWidth;
    ctx.stroke();
    buttonWnd_.style_ = WS_POPUP | WS_DISABLED;
    buttonWnd_.createWindow(Process::mainWindow());
    buttonWnd_.setPosition({rc.left, rc.top});
    buttonWnd_.updateWindow();
    ShowWindow(buttonWnd_.hWnd, SW_SHOWNA);
}

void ButtonPreview::createButtonText(const RECT& rc, const std::wstring& text, int selPosition, int selLength) {
    using namespace PreviewStyle;

    if (text.empty()) {
        return;
    }

    NONCLIENTMETRICS metrics = {sizeof(metrics)};
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
    HFONT hFont = CreateFontIndirect(&metrics.lfCaptionFont);

    Bitmap textBuf;
    BitmapContext ctx(textBuf);
    ctx.font = hFont;
    SIZE textSize = ctx.textSize(text.c_str());
    if (textSize.cx == 0 || textSize.cy == 0) {
        return;
    }

    textBuf.create(textSize.cx, textSize.cy);
    textBuf.clear(ButtonHintBackgroundColor);

    ctx.textColor = ButtonHintTextColor;
    ctx.textOut(text.c_str(), 0, 0);

    const int paddingX = 14;
    const int paddingY = 10;
    const int width = textSize.cx + paddingX * 2;
    const int height = textSize.cy + paddingY * 2;
    textWnd_.buffer_.create(width, height);
    textWnd_.buffer_.clear(ButtonHintBackgroundColor);
    textWnd_.buffer_.copyFrom(textBuf, paddingX, paddingY);

    if (selPosition >= 0 && selLength > 0) {
        const auto selectedText = text.substr(selPosition, selLength);
        SIZE selSize = ctx.textSize(selectedText.c_str());
        textBuf.create(selSize.cx, selSize.cy);
        textBuf.clear(ButtonHintBackgroundColor);

        ctx.addRoundedRect(0, 0, selSize.cx, selSize.cy, 2);
        ctx.fillColor = ButtonHintSelectionColor;
        ctx.fill();

        ctx.textColor = ButtonHintTextSelColor;
        ctx.textOut(selectedText.c_str(), 0, 0);

        std::wstring beforeSelection = text.substr(0, selPosition);
        int offsetX = ctx.textSize(beforeSelection.c_str()).cx;
        textWnd_.buffer_.copyFrom(textBuf, paddingX + offsetX, paddingY);
    }

    textWnd_.style_ = WS_POPUP | WS_DISABLED;
    textWnd_.createWindow(Process::mainWindow());

    POINT winPosition;

    POINT ptAnchor = Rectangle::center(rc);
    SIZE sizeWindow{width, height};
    RECT rcResult = {0};
    auto rc_ = rc;
    if (CalculatePopupWindowPosition(&ptAnchor, &sizeWindow,
                                     TPM_VERTICAL | TPM_VCENTERALIGN | TPM_CENTERALIGN | TPM_WORKAREA,
                                     &rc_, &rcResult)) {
        winPosition.x = rcResult.left;
        winPosition.y = rcResult.top;

        textWnd_.setPosition(winPosition);

        textWnd_.updateWindow();
        ShowWindow(textWnd_.hWnd, SW_SHOWNA);
    }

    DeleteObject(hFont);
}

} // namespace litelockr
