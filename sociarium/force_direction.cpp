// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

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

#include "force_direction.h"
#include "sociarium_graph.h"

namespace hashimoto_ut {

  using std::vector;
  using std::pair;
  using std::tr1::unordered_map;
  using std::tr1::shared_ptr;

  namespace sociarium_project_force_direction {

    namespace {
      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // ローカル変数
      vector<shared_ptr<FDElement> > fd_element;
      unordered_map<DynamicNodeProperty*, pair<Vector2<float>, float> > fd_force;
      bool update = true;
    }

    bool fd_element_should_be_updated(void) {
      return update;
    }

    void fd_element_should_be_updated(bool b) {
      update = b;
    }

    vector<shared_ptr<FDElement> > const& get_fd_element(void) {
      return fd_element;
    }

    unordered_map<DynamicNodeProperty*, pair<Vector2<float>, float> >::iterator force_begin(void) {
      return fd_force.begin();
    }

    unordered_map<DynamicNodeProperty*, pair<Vector2<float>, float> >::iterator force_end(void) {
      return fd_force.end();
    }

    void set_fd_element(shared_ptr<SociariumGraph const> const& g) {

      fd_element.clear();
      fd_force.clear();
      shared_ptr<AvoidHiddenElements> cond(new AvoidHiddenElements(g));
      shared_ptr<BFSTraverser> t = BFSTraverser::create<bidirectional_tag>(g);
      t->set_condition(cond);

      for (node_iterator i=g->nbegin(); i!=g->nend(); ++i) {

        float& w = fd_force[g->property(*i).get()].second;
        w = 0.0f;
        for (adjacency_list_iterator j=(*i)->begin(); j!=(*i)->end(); ++j)
          if (is_visible(g->property(*j))) ++w;

        for (t->reset(), t->start(*i, 0.0); !t->end(); t->advance()) {
          if (t->node()<=*i) continue;
          shared_ptr<FDElement> fde(new FDElement);
          fde->dnp0 = g->property(*i).get();
          fde->dnp1 = g->property(t->node()).get();
          fde->distance = float(t->distance()-(t->node()->degree()==1?0.7:0.0));
          fde->force0 = &fd_force[fde->dnp0];
          fde->force1 = &fd_force[fde->dnp1];
          try {
            fd_element.push_back(fde);
          } catch (...) {
            fd_element.clear();
            return;
          }
        }
      }

      update = false;
    }

  }
} // The end of the namespace "hashimoto_ut"
