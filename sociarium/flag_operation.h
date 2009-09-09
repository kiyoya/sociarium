// s.o.c.i.a.r.i.u.m: flag_operation.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_FLAG_OPERATION_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_FLAG_OPERATION_H

#include "sociarium_graph.h"

namespace hashimoto_ut {

  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    struct GetDynamicProperty {
      DynamicNodeProperty&
        operator()(Node* n, std::tr1::shared_ptr<SociariumGraph> const& g) const {
          return g->property(n);
        }

      DynamicEdgeProperty&
        operator()(Edge* e, std::tr1::shared_ptr<SociariumGraph> const& g) const {
          return g->property(e);
        }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct GetStaticProperty {
      StaticNodeProperty*
        operator()(std::pair<Node*, DynamicNodeProperty> const& pp) const {
          return pp.second.get_static_property();
        }

      StaticEdgeProperty*
        operator()(std::pair<Edge*, DynamicEdgeProperty> const& pp) const {
          return pp.second.get_static_property();
        }

      StaticNodeProperty*
        operator()(DynamicNodeProperty* dp) const {
          return dp->get_static_property();
        }

      StaticEdgeProperty*
        operator()(DynamicEdgeProperty* dp) const {
          return dp->get_static_property();
        }
    };


    /*
     * Following operations should be performed only on "visible" elements.
     */


    ////////////////////////////////////////////////////////////////////////////////
    struct ActivateFlag {
      template <typename T>
      void operator()(T* p, unsigned int flag) const {
        if (is_visible(*p))
          p->set_flag(p->get_flag()|flag);
      }

      template <typename T>
      void operator()(T& pp, unsigned int flag) const {
        typename T::second_type& p = pp.second;
        if (is_visible(p))
          p.set_flag(p.get_flag()|flag);
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct ActivateDynamicFlag {
      template <typename T>
      void operator()(
        T const* p, unsigned int flag,
        std::tr1::shared_ptr<SociariumGraph const> const& g) const {
          typename DynamicProperty<T>::type& dp = g->property(p);
          if (is_visible(dp))
            dp.set_flag(dp.get_flag()|flag);
        }

      void operator()(
        Edge const* e, unsigned int flag,
        std::tr1::shared_ptr<SociariumGraph const> const& g, int dir) const {

          DynamicEdgeProperty& dep = g->property(e);
          DynamicNodeProperty& dnp = g->property(dir>0?e->target():e->source());

          if (is_visible(dep)) {
            dep.set_flag(dep.get_flag()|flag);
            if (is_visible(dnp))
              dnp.set_flag(dnp.get_flag()|flag);
          }
        }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct DeactivateFlag {
      template <typename T>
      void operator()(T* p, unsigned int flag) const {
        if (is_visible(*p))
          p->set_flag(p->get_flag()&~flag);
      }

      template <typename T>
      void operator()(T& pp, unsigned int flag) const {
        typename T::second_type& p = pp.second;
        if (is_visible(p))
          p.set_flag(p.get_flag()&~flag);
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct ToggleFlag {
      template <typename T>
      void operator()(T& pp, unsigned int flag) const {
        typename T::second_type& p = pp.second;
        if (is_visible(p))
          p.set_flag((p.get_flag()^flag)&ElementFlag::MASK);
      }
    };

  } // The end of the anonymous namespace
}

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_FLAG_OPERATION_H
