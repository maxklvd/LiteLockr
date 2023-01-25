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

#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <gmtl/gmtl.h>

namespace litelockr {

using Point2 = gmtl::Point2f;
using Point3 = gmtl::Point3f;
using Point2Vec = std::vector<Point2>;
using Point3Vec = std::vector<Point3>;
using Vector3 = gmtl::Vec3f;

} // namespace litelockr

#endif // DATA_TYPE_H