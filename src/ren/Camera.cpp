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

#include "Camera.h"

namespace litelockr {

gmtl::Point3f multiplyPoint(gmtl::Point3f& p, gmtl::Matrix44f mat) {
    float x = p[0];
    float y = p[1];
    float z = p[2];

    gmtl::Point3f pt(
            x * mat[0][0] + y * mat[1][0] + z * mat[2][0] + mat[3][0],
            x * mat[0][1] + y * mat[1][1] + z * mat[2][1] + mat[3][1],
            x * mat[0][2] + y * mat[1][2] + z * mat[2][2] + mat[3][2]);

    float w = pt[2];
    if (w != 0) {
        pt[0] /= w;
        pt[1] /= w;
        pt[2] /= w;
    }

    return pt;
}

gmtl::Point3f projectVertex(gmtl::Matrix44f& proj, gmtl::Point3f& v, unsigned width, unsigned height) {
    gmtl::Vec3f p = multiplyPoint(v, proj);

    width++;
    height++;

    float px = static_cast<float>(width) * (p[0] + 1.0f) / 2.0f;
    float py = static_cast<float>(height) * (1.0f - ((p[1] + 1.0f) / 2.0f));

    p[0] = px;
    p[1] = py;
    p[2] = 0;
    return p;
}

Point2 Camera::calculateVertex(Point3 point) const {
    float cx = static_cast<float>(width) / 2.0f;
    float cy = static_cast<float>(height) / 2.0f;
    // o(0,0) = point(-cx, cy)
    point[0] = -cx + point[0];
    point[1] = cy - point[1];
    point[2] += static_cast<float>(width) * 1.391f;

    gmtl::Matrix44f rot;
    gmtl::setRot(rot, gmtl::AxisAnglef(gmtl::Math::deg2Rad(180.0f),
                                       gmtl::Vec3f(0.0f, 0.0f, 1.0f)));
    point *= rot;

    constexpr float FOV = 40.0f;
    constexpr float NEAR_PLANE = 0.00001f;
    constexpr float FAR_PLANE = 1000.0f;
    gmtl::Matrix44f proj;
    gmtl::setPerspective(proj, FOV, 1.0f, NEAR_PLANE, FAR_PLANE);

    point = projectVertex(proj, point, width, height);

    return {point[0] - 0.5f, point[1] - 0.5f};
}

} // namespace litelockr
