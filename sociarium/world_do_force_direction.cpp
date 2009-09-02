// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/11

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   - Neither the name of the University of Tokyo nor the names of its contributors
 *     may be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "world.h"
#include "sociarium_graph.h"
#include "sociarium_graph_time_series.h"
#include "force_direction.h"
#include "draw.h"

namespace hashimoto_ut {

  using std::vector;
  using std::pair;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  void World::do_force_direction(void) const {

    using namespace sociarium_project_force_direction;
    
    // --------------------------------------------------------------------------------
    time_series_->read_lock();
    // --------------------------------------------------------------------------------

    size_t const index_of_current_layer = time_series_->index_of_current_layer();
    shared_ptr<SociariumGraph> const& g0 = time_series_->get_graph(0, index_of_current_layer);
    shared_ptr<SociariumGraph> const& g1 = time_series_->get_graph(1, index_of_current_layer);

    if (sociarium_project_force_direction::fd_element_should_be_updated())
      sociarium_project_force_direction::set_fd_element(g0);

    vector<shared_ptr<FDElement> > const& fd_element = get_fd_element();

    // Calculate the force acting between every nodes
    for (vector<shared_ptr<FDElement> >::const_iterator i=fd_element.begin(); i!=fd_element.end(); ++i) {
      DynamicNodeProperty const* dnp0 = (*i)->dnp0;
      DynamicNodeProperty const* dnp1 = (*i)->dnp1;
      Vector2<float> const& pos0 = dnp0->get_static_property()->get_position();
      Vector2<float> const& pos1 = dnp1->get_static_property()->get_position();

      RadiusVector2<float> rv(pos0, pos1);
      double const dst = rv.norm-0.5*(dnp0->get_size()+dnp1->get_size());
      double const scale = float(0.3*sociarium_project_draw::get_layout_frame_size());
      //double const diff = rv.norm-scale*(*i)->distance;
      double const diff = dst-scale*(*i)->distance;
      double const cf = float(5e-4/((*i)->distance*(*i)->distance));
      float const f = float(cf*(diff>0.0?diff*diff:-diff*diff));
      float const force = float(f<5e-1?f:5e-1);
      (*i)->force0->first += force*rv.u;
      (*i)->force1->first -= force*rv.u;
    }

    // Layout nodes
    for (unordered_map<DynamicNodeProperty*, pair<Vector2<float>, float> >::iterator i=force_begin(); i!=force_end(); ++i) {
      DynamicNodeProperty* dnp = i->first;
      if (dnp->get_flag()&ElementFlag::CAPTURED || i->second.second==0.0f)
        i->second.first.set(0.0f, 0.0f);
      else {
        dnp->get_static_property()->set_position(dnp->get_static_property()->get_position()+i->second.first/i->second.second);
        i->second.first *= 0.90f;
      }
    }

    // Layout communities
    for (SociariumGraph::node_property_iterator i=g1->node_property_begin(); i!=g1->node_property_end(); ++i)
      i->second->move_to_center_of_lower_nodes_position();

    // --------------------------------------------------------------------------------
    time_series_->read_unlock();
    // --------------------------------------------------------------------------------
  }
} // The end of the namespace "hashimoto_ut"
