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

#ifndef MESH_OBJECT_H
#define MESH_OBJECT_H

#include <functional>

#include "ren/Geometry.h"
#include "ren/Material.h"

namespace litelockr {

class MeshObject {
public:
    MeshObject *parent = nullptr;
    gmtl::Matrix44f ml, mg;
    bool visible = true;

    MeshObject() = default;

    BaseGeometry& geometry();
    [[nodiscard]] const BaseGeometry& geometry() const;

    std::vector<std::reference_wrapper<BaseGeometry>>& getAllGeometries() {
        return geometries_;
    }

    void create(BaseGeometry& geo);
    void create(BaseGeometry& geo, Material& mat);
    void create(BaseGeometry& geo, std::initializer_list<std::reference_wrapper<Material>> list);

    [[nodiscard]] const Material& material(unsigned idx) const;
    Material& material(unsigned idx);

    void setParent(MeshObject& obj);

    void setPosition(float x, float y, float z);
    void setPosition0(float x, float y, float z = 0);

    [[maybe_unused]] void setRotXYZ(float angleX, float angleY, float angleZ);
    [[maybe_unused]] void setRotX(float angle);
    [[maybe_unused]] void setRotY(float angle);
    [[maybe_unused]] void setRotZ(float angle);

    [[nodiscard]] const gmtl::Matrix44f& getUpMg() const;
    [[nodiscard]] const gmtl::Matrix44f& getMg() const;
    void updateMg();

protected:
    std::vector<std::reference_wrapper<BaseGeometry>> geometries_;

private:
    Point3 position_;
    Vector3 rotation_;

    std::vector<std::reference_wrapper<Material>> materials_;
};

using MeshObjectVec = std::vector<std::reference_wrapper<MeshObject>>;

class NullObject: public MeshObject {
public:
    NullObject() {
        geometries_.push_back(geo);
    }

private:
    NullGeometry geo;
};

} // namespace litelockr

#endif // MESH_OBJECT_H
