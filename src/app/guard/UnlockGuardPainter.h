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

#ifndef UNLOCK_GUARD_PAINTER_H
#define UNLOCK_GUARD_PAINTER_H

#include <string>

#include "app/guard/UnlockGuardModel.h"
#include "gfx/Bitmap.h"

namespace litelockr {

class UnlockGuardPainter {
public:
    constexpr static int BACKGROUND_BLUR_RADIUS = 4;
    constexpr static int TEXT_OPACITY = 255;

    int backgroundBlurRadius = BACKGROUND_BLUR_RADIUS;
    std::uint8_t textOpacity = TEXT_OPACITY;

    float offsetText0Y = 0;
    float offsetText1X = 0;
    float offsetText1Y = 0;

    explicit UnlockGuardPainter(bool usePinCode) : pinEnabled_(usePinCode) {}

    void initialize(const RECT& workArea, const Bitmap& bgBuffer);

    void drawTopText(const std::wstring& text, bool useCache = false);
    void drawBottomText(const std::wstring& text);

    [[nodiscard]] const Bitmap& buffer() const {
        return buffer_;
    }

private:
    Bitmap buffer_;
    Bitmap bgBuffer_;
    RECT workArea_{};
    Bitmap cacheBuffer_;

    Bitmap gradientBackground_;
    Bitmap gradientText_;

    bool pinEnabled_ = false;

    struct TextSettings {
        const char *fontFace;
        float fontHeight;
        float fontLetterSpacing;
        int positionY;
    };

    TextSettings mathTopTextSettings_{"Lato", 45, 2, 59};
    TextSettings mathBottomTextSettings_{"Lato", 45, 2, 110};

    TextSettings pinTopTextSettings_{"fontello", 25, 0, 47};
    TextSettings pinBottomTextSettings_{"fontello", 19, 12, 87};
};

} // namespace litelockr

#endif // UNLOCK_GUARD_PAINTER_H
