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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "ren/Polygon.h"

namespace litelockr {

class BaseGeometry {
public:
    int width = 0;
    int height = 0;
    Point3Vec vertices;
    std::vector<Polygon> faces;

    Point2Vec camera;

    BaseGeometry() = default;
    virtual ~BaseGeometry() = default;

    virtual void update() {}

    void offset(float dx, float dy, float dz = 0);

    void setMaterialIndex(unsigned faceIdx, unsigned materialIdx) {
        if (faceIdx < faces.size()) {
            faces[faceIdx].materialIndex = materialIdx;
        } else {
            assert(false);
        }
    }
};

class PlaneGeometry: public BaseGeometry {
public:
    PlaneGeometry() = default;

    void setSize(float w, float h);

    void setSize(unsigned w, unsigned h) {
        setSize(static_cast<float>(w), static_cast<float>(h));
    }

    void create(float w, float h);

    void create(unsigned w, unsigned h) {
        create(static_cast<float>(w), static_cast<float>(h));
    }
};

class NullGeometry: public BaseGeometry {
public:
    NullGeometry() {
        vertices.emplace_back(0.0f, 0.0f, 0.0f);
        camera.resize(vertices.size());
    }
};

} // namespace litelockr

#endif // GEOMETRY_H
