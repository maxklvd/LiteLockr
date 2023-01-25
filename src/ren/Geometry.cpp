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

#include "Geometry.h"

namespace litelockr {

void BaseGeometry::offset(float dx, float dy, float dz /*= 0*/) {
    for (auto& v: vertices) {
        v[0] += dx;
        v[1] += dy;
        v[2] += dz;
    }
}

void PlaneGeometry::setSize(float w, float h) {
    float w2 = w / 2.0f;
    float h2 = h / 2.0f;

    float x = 0, y = 0, z = 0;
    Point3 p0(x - w2, y - h2, z);
    Point3 p1(x + w2, y - h2, z);
    Point3 p2(x + w2, y + h2, z);
    Point3 p3(x - w2, y + h2, z);

    vertices[0] = p0;
    vertices[1] = p1;
    vertices[2] = p2;
    vertices[3] = p3;
}

void PlaneGeometry::create(float w, float h) {
    Point3 p;
    vertices = {p, p, p, p}; // 4 vertices
    setSize(w, h);

    camera.resize(vertices.size());

    faces.clear();
    faces.emplace_back(0, 1, 2, 3);
}

} // namespace litelockr
