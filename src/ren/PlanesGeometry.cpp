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

#include "PlanesGeometry.h"

#include "sys/Comparison.h"

namespace litelockr {

void PlanesGeometry::create(float w0, float w1, float h, float val) {
    width0_ = w0;
    width1_ = w1;
    height_ = h;

    Point3 p;
    vertices = {p, p, p, p,  // 9 vertices
                p, p, p, p,
                p};
    setSize(w0, w1, h, val);

    camera.resize(vertices.size());

    faces.clear();
    faces.emplace_back(0, 1, 5, 4);
    faces.emplace_back(1, 2, 6, 5);
    faces.emplace_back(2, 3, 7, 6);

    updateFaceVisibility(val);
}

void PlanesGeometry::update(float val) {
    assert(width0_ > 0);
    assert(width1_ > 0);
    assert(height_ > 0);

    setSize(width0_, width1_, height_, val);
    updateFaceVisibility(val);
}

void PlanesGeometry::setSize(float width0, float width1, float h, float val) {
    h -= PlanesGeometry::Y_OFFSET;

    float w2 = width1 / 2.0f;
    float h2 = h / 2.0f;
    float x = -w2, y0 = -h2, y1 = h2, z = 0;

    float w0 = width0 * val;
    float len0 = width0 / 2;
    float len2 = width1;

    float x0 = x + 0;
    float x2 = x + w0;

    float ang1 = -90 * (1.0f - val);
    float angle = 31.885f;
    float ang2 = -angle * val;

    gmtl::Matrix44f rot1, rot2;
    gmtl::setRot(rot1, gmtl::AxisAnglef(gmtl::Math::deg2Rad(ang1), gmtl::Vec3f(0.0f, 1.0f, 0.0f)));
    gmtl::setRot(rot2, gmtl::AxisAnglef(gmtl::Math::deg2Rad(ang2), gmtl::Vec3f(0.0f, 1.0f, 0.0f)));

    Point3 pp1(len0, 0, 0);
    pp1 *= rot1;

    Point3 pp3(len2, 0, 0);
    pp3 *= rot2;

    Point3 pc(len2 / 2, 0, 0);
    pc *= rot2;

    center_[0] = x2 + pc[0];
    center_[1] = 0;
    center_[2] = pc[2];
    angle_ = ang2;

    y0 -= PlanesGeometry::Y_OFFSET;

    Point3 p0(x0, y0, z);
    Point3 p1(x0 + pp1[0], y0, z + pp1[2]);
    Point3 p2(x2, y0, z);
    Point3 p3(x2 + pp3[0], y0, z + pp3[2]);

    Point3 p4(x0, y1, z);
    Point3 p5(x0 + pp1[0], y1, z + pp1[2]);
    Point3 p6(x2, y1, z);
    Point3 p7(x2 + pp3[0], y1, z + pp3[2]);

    vertices[0] = p0;
    vertices[1] = p1;
    vertices[2] = p2;
    vertices[3] = p3;
    vertices[4] = p4;
    vertices[5] = p5;
    vertices[6] = p6;
    vertices[7] = p7;

    vertices[8] = center_;
}

void PlanesGeometry::updateFaceVisibility(float val) {
    if (faces.size() >= 2) {
        bool visible = !isFloatEquals(val, 0.0f);
        faces[0].visible = visible;
        faces[1].visible = visible;
    }
}

} // namespace litelockr
