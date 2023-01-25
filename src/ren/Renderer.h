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

#ifndef RENDERER_H
#define RENDERER_H

#include "gfx/Bitmap.h"
#include "ren/Camera.h"
#include "ren/MeshObject.h"

namespace litelockr {

class SceneObjects {
public:
    MeshObjectVec objects;
    Camera camera;

    void add(MeshObject& obj);

private:
    bool checkParent(MeshObject& obj) const;
};

class Renderer {
public:
    explicit Renderer(Bitmap& buffer) : buffer_(buffer) {}

    void render(SceneObjects& scene);

private:
    void calculateCameraVertices(SceneObjects& scene) const;

    Bitmap& buffer_;
};

} // namespace litelockr

#endif // RENDERER_H
