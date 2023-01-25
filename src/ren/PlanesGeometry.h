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

#ifndef PLANES_GEOMETRY_H
#define PLANES_GEOMETRY_H

#include "ren/Geometry.h"

namespace litelockr {

class PlanesGeometry: public BaseGeometry {
public:
    constexpr static int Y_OFFSET = 19;

    PlanesGeometry() = default;

    [[nodiscard]] Point3 center() const { return center_; }

    [[nodiscard]] float angle() const { return angle_; }

    void create(float w0, float w1, float h, float val);
    void update(float val);
    void setSize(float width0, float width1, float h, float val);

private:
    void updateFaceVisibility(float val);

    float width0_ = -1;
    float width1_ = -1;
    float height_ = -1;
    Point3 center_;
    float angle_ = 0;
};

} // namespace litelockr

#endif // PLANES_GEOMETRY_H
