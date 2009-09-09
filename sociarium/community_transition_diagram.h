// s.o.c.i.a.r.i.u.m: community_transition_diagram.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_COMMUNITY_TRANSITION_DIAGRAM_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_COMMUNITY_TRANSITION_DIAGRAM_H

#include <memory>
#include <vector>
#include <array>
#include "../shared/vector2.h"
#include "../shared/vector3.h"

namespace hashimoto_ut {

  class SociariumGraphTimeSeries;
  class StaticNodeProperty;

  ////////////////////////////////////////////////////////////////////////////////
  class Trajectory {
  public:
    virtual ~Trajectory() {}
    virtual StaticNodeProperty* get_snp(void) const = 0;
    virtual std::pair<size_t, size_t> const& get_duration(void) const = 0;
    virtual std::pair<float, float>
      get_base_position(size_t index_of_layer) const = 0;
    virtual std::pair<float, float>
      get_interpolated_position(size_t index_of_point) const = 0;
    virtual Vector3<float> const&
      get_base_color(size_t index_of_layer) const = 0;
    virtual Vector3<float> const&
      get_interpolated_color(size_t index_of_point) const = 0;
  };


  ////////////////////////////////////////////////////////////////////////////////
  class CommunityTransitionDiagram {
  public:
    virtual ~CommunityTransitionDiagram() {}

    virtual void read_lock(void) const = 0;
    virtual void read_unlock(void) const = 0;
    virtual void write_lock(void) const = 0;
    virtual void write_unlock(void) const = 0;

    virtual std::vector<std::tr1::shared_ptr<Trajectory> >::const_iterator
      trajectory_begin(void) const = 0;
    virtual std::vector<std::tr1::shared_ptr<Trajectory> >::const_iterator
      trajectory_end(void) const = 0;

    virtual void set_current_layer(size_t index_of_layer) = 0;
    virtual float get_max_bundle_size(void) const = 0;

    virtual void update(int mode) = 0;
    virtual void clear(void) = 0;
    virtual bool empty(void) const = 0;
  };


  namespace sociarium_project_community_transition_diagram {

    ////////////////////////////////////////////////////////////////////////////////
    void initialize(void);
    std::tr1::shared_ptr<CommunityTransitionDiagram> get(void);
    int get_diagram_mode(void);

    ////////////////////////////////////////////////////////////////////////////////
    Vector2<double> const& get_frame_position(void);
    void set_frame_position(Vector2<double> const& pos);
    Vector2<double> const& get_frame_size(void);
    void set_frame_size(Vector2<double> size);

    ////////////////////////////////////////////////////////////////////////////////
    Vector2<double> const& get_default_frame_position(void);
    void set_default_frame_position(Vector2<double> const& pos);
    Vector2<double> const& get_default_frame_size(void);
    void set_default_frame_size(Vector2<double> size);

    ////////////////////////////////////////////////////////////////////////////////
    size_t get_resolution(void);
    void set_resolution(size_t resolution);
    std::pair<int, int> const& get_scope(void);
    void set_scope(int first_point, int last_point);

  } // The end of the namespace "sociarium_project_community_transition_diagram"
  
} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_COMMUNITY_TRANSITION_DIAGRAM_H

