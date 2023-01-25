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

#ifndef PANEL_COMPONENT_H
#define PANEL_COMPONENT_H

namespace litelockr {

class PanelComponent {
public:
    virtual ~PanelComponent() = default;

    int id = -1;
    int x = 0;
    int y = 0;
    unsigned w = 0;
    unsigned h = 0;
    bool visible = true;
    bool pressed = false;
    bool hover = false;
    bool checked = false;
    int r = 0;
    bool useView3 = false;

    [[nodiscard]] bool contains(int _x, int _y, bool isView3 = false) const;

    [[nodiscard]] virtual bool isButton() const { return false; }

    struct PositionOffset {
        float x = 0;
        float y = 0;
        float z = 0;
    } offset3;

    struct View3 {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;

        [[nodiscard]] bool contains(int _x, int _y) const {
            return _x >= x && _x < (x + w)
                   && _y >= y && _y < (y + h);
        }
    } view3;
};

class PanelButton: public PanelComponent {
public:
    [[nodiscard]] bool isButton() const override { return true; }
};

} // namespace litelockr

#endif // PANEL_COMPONENT_H
