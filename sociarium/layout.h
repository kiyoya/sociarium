﻿// s.o.c.i.a.r.i.u.m: layout.h
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of the University of Tokyo nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_LAYOUT_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_LAYOUT_H

#include <memory>
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class DynamicNodeProperty;

  namespace sociarium_project_layout {

    ////////////////////////////////////////////////////////////////////////////////
    float get_layout_frame_default_size(void);
    void set_layout_frame_default_size(float size);

    ////////////////////////////////////////////////////////////////////////////////
    float get_layout_frame_size(void);
    void set_layout_frame_size(float size);

    ////////////////////////////////////////////////////////////////////////////////
    float get_layout_frame_previous_size(void);
    void set_layout_frame_previous_size(float size);

    ////////////////////////////////////////////////////////////////////////////////
    Vector2<float> const& get_layout_frame_position(void);
    void set_layout_frame_position(Vector2<float> const& position);

    ////////////////////////////////////////////////////////////////////////////////
    Vector2<float> const& get_layout_frame_previous_position(void);
    void set_layout_frame_previous_position(Vector2<float> const& position);

    ////////////////////////////////////////////////////////////////////////////////
    bool get_update_layout_frame(void);
    void set_update_layout_frame(bool b);

    ////////////////////////////////////////////////////////////////////////////////
    float get_grid_interval(void);
    void set_grid_interval(float interval);

    ////////////////////////////////////////////////////////////////////////////////
    void reset_position(DynamicNodeProperty& dnp);

  } // The end of the namespace "sociarium_project_layout"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_LAYOUT_H
