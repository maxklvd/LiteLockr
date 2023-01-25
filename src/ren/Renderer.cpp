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

#include "Renderer.h"

#include <ranges>

#include "gfx/BitmapContext.h"

namespace litelockr {

void SceneObjects::add(MeshObject& obj) {
    assert(checkParent(obj));
    objects.push_back(obj);
}

bool SceneObjects::checkParent(MeshObject& obj) const {
    if (obj.parent) {
        for (MeshObject& object: objects) {
            if (obj.parent == &object) {
                return true;
            }
        }
        return false;
    }

    return true;
}

void Renderer::render(SceneObjects& scene) {
    for (MeshObject& object: scene.objects) {
        object.updateMg();
    }

    scene.camera.width = buffer_.width();
    scene.camera.height = buffer_.height();
    calculateCameraVertices(scene);

    //
    // Draws all polygons
    //
    BitmapContext ctx(buffer_);

    for (const auto& objectRef: scene.objects | std::views::reverse) {
        const auto& object = objectRef.get();

        if (object.visible) {
            const auto& geo = object.geometry();
            for (const auto& face: geo.faces) {
                if (face.visible) {
                    const auto& mat = object.material(face.materialIndex);
                    if (mat.alpha > 0) {
                        assert(!mat.bitmap.isNull());

                        BitmapContext::QuadDouble quad{
                                geo.camera[face.a][0],
                                geo.camera[face.a][1],
                                geo.camera[face.b][0],
                                geo.camera[face.b][1],
                                geo.camera[face.c][0],
                                geo.camera[face.c][1],
                                geo.camera[face.d][0],
                                geo.camera[face.d][1],
                        };
                        ctx.perspective(mat.bitmap, quad, mat.alpha);
                    }
                }
            }
        }
    }
}

void Renderer::calculateCameraVertices(SceneObjects& scene) const {
    MeshObjectVec& objects = scene.objects;
    for (MeshObject& object: objects) {
        for (BaseGeometry& geo: object.getAllGeometries()) {
            for (unsigned i = 0; i < geo.vertices.size(); i++) {
                Point3 p = geo.vertices[i];
                p *= object.mg; // to absolute
                geo.camera[i] = scene.camera.calculateVertex(p);
            }
        }
    }
}

} // namespace litelockr
