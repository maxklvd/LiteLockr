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

#ifndef AGG_ALL_H
#define AGG_ALL_H

#ifdef _MSC_VER
#pragma warning(push)
#endif

#ifdef __MINGW32__
#pragma GCC diagnostic push
#endif

#include <agg_config.h>

#include <agg_pixfmt_rgba.h>
#include <agg_pixfmt_rgb.h>
#include <agg_renderer_base.h>
#include <agg_path_storage.h>
#include <agg_font_cache_manager.h>
#include <agg_gradient_lut.h>
#include <agg_renderer_scanline.h>
#include <agg_span_gradient.h>
#include <agg_span_interpolator_linear.h>
#include <agg_span_allocator.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_scanline_p.h>
#include <agg_conv_stroke.h>
#include <agg_font_win32_tt.h>
#include <agg_blur.h>
#include <agg_image_accessors.h>
#include <agg_trans_perspective.h>
#include <agg_span_interpolator_trans.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_span_converter.h>
#include <agg_rounded_rect.h>

#ifdef __MINGW32__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // AGG_ALL_H
