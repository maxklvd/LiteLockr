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

#include "MeshObject.h"

namespace litelockr {

BaseGeometry& MeshObject::geometry() {
    assert(!geometries_.empty());
    return geometries_[0];
}

const BaseGeometry& MeshObject::geometry() const {
    assert(!geometries_.empty());
    return geometries_[0];
}

void MeshObject::create(BaseGeometry& geo) {
    assert(geometries_.empty());
    geometries_.push_back(geo);
}

void MeshObject::create(BaseGeometry& geo, Material& mat) {
    create(geo, {std::ref(mat)});
}

void MeshObject::create(BaseGeometry& geo, std::initializer_list<std::reference_wrapper<Material>> list) {
    create(geo);
    materials_.clear();
    materials_.insert(materials_.end(), list.begin(), list.end());
}

const Material& MeshObject::material(unsigned idx) const {
    assert(idx < materials_.size());
    return materials_[idx];
}

Material& MeshObject::material(unsigned idx) {
    assert(idx < materials_.size());
    return materials_[idx];
}

void MeshObject::setParent(MeshObject& obj) {
    parent = &obj;
}

void MeshObject::setPosition(float x, float y, float z) {
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;

    gmtl::setTrans(ml, Vector3(x, y, z));
}

void MeshObject::setPosition0(float x, float y, float z /*= 0*/) {
    Point3& v0 = geometry().vertices[0];
    setPosition(x - v0[0], y - v0[1], z - v0[2]);
}

[[maybe_unused]] void MeshObject::setRotXYZ(float angleX, float angleY, float angleZ) {
    rotation_[0] = angleX;
    rotation_[1] = angleY;
    rotation_[2] = angleZ;
    gmtl::setRot(ml, gmtl::EulerAngleXYZf(
            gmtl::Math::deg2Rad(angleX),
            gmtl::Math::deg2Rad(angleY),
            gmtl::Math::deg2Rad(angleZ)));
}

[[maybe_unused]] void MeshObject::setRotX(float angle) {
    rotation_[0] = angle;
    setRotXYZ(rotation_[0], rotation_[1], rotation_[2]);
}

[[maybe_unused]] void MeshObject::setRotY(float angle) {
    rotation_[1] = angle;
    setRotXYZ(rotation_[0], rotation_[1], rotation_[2]);
}

[[maybe_unused]] void MeshObject::setRotZ(float angle) {
    rotation_[2] = angle;
    setRotXYZ(rotation_[0], rotation_[1], rotation_[2]);
}

const gmtl::Matrix44f& MeshObject::getUpMg() const {
    if (parent) {
        return parent->getMg();
    } else {
        return gmtl::MAT_IDENTITY44F;
    }
}

const gmtl::Matrix44f& MeshObject::getMg() const {
    return mg;
}

void MeshObject::updateMg() {
    const gmtl::Matrix44f& up = getUpMg();
    mg.set(up.getData());
    mg *= ml;
}

} // namespace litelockr
