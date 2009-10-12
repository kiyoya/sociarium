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
#include "../graph/util/traverser.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  struct ElementFlag {
    enum _ {
      VISIBLE            = 0x01,
      CAPTURED           = 0x02,
      MARKED             = 0x04,
      TEMPORARY_MARKED   = 0x08,
      TEMPORARY_UNMARKED = 0x10,
      HIGHLIGHT          = 0x20,
      MASK               = 0x3f,
      // Other.
      TEXTURE_IS_SPECIFIED = 0x40
    };
  };

  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    bool is_active(T const& p, int flag) {
      return (p.get_flag()&flag)!=0;
    }

    template <typename T>
    bool is_visible(T const& p) {
      return (p.get_flag()&ElementFlag::VISIBLE)!=0;
    }

    template <typename T>
    bool is_hidden(T const& p) {
      return (p.get_flag()&ElementFlag::VISIBLE)==0;
    }

    template <typename T>
    bool is_marked(T const& p) {
      return (p.get_flag()&ElementFlag::MARKED)!=0;
    }

    template <typename T>
    bool is_temporary_marked(T const& p) {
      return (p.get_flag()&ElementFlag::TEMPORARY_MARKED)!=0;
    }

    template <typename T>
    bool is_temporary_unmarked(T const& p) {
      return (p.get_flag()&ElementFlag::TEMPORARY_UNMARKED)!=0;
    }

    /*
     * Following operations should be performed only on "visible" elements.
     */

    ////////////////////////////////////////////////////////////////////////////////
    struct ActivateFlag {
      template <typename T>
      void operator()(T* p, unsigned int flag) const {
        if (is_visible(*p)) p->set_flag(p->get_flag()|flag);
      }

      template <typename T>
      void operator()(T& p, unsigned int flag) const {
        typename T::second_type& value = p.second;
        if (is_visible(value)) value.set_flag(value.get_flag()|flag);
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct DeactivateFlag {
      template <typename T>
      void operator()(T* p, unsigned int flag) const {
        if (is_visible(*p)) p->set_flag(p->get_flag()&~flag);
      }

      template <typename T>
      void operator()(T& p, unsigned int flag) const {
        typename T::second_type& value = p.second;
        if (is_visible(value)) value.set_flag(value.get_flag()&~flag);
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct ActivateFlagIfMarked {
      template <typename T>
      void operator()(T& p, unsigned int flag) const {
        typename T::second_type& value = p.second;
        if (is_visible(value) && value.get_flag()&(ElementFlag::MARKED|ElementFlag::HIGHLIGHT))
          value.set_flag(value.get_flag()|flag);
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    struct ToggleFlag {
      template <typename T>
      void operator()(T& p, unsigned int flag) const {
        typename T::second_type& value = p.second;
        if (is_visible(value))
          value.set_flag((value.get_flag()^flag)&ElementFlag::MASK);
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    // Traversing condition.
    struct CircumventHiddenElements : ConditionalPass {
      typedef SociariumGraph::node_property_iterator node_property_iterator;
      typedef SociariumGraph::edge_property_iterator edge_property_iterator;

      CircumventHiddenElements(std::tr1::shared_ptr<SociariumGraph const> g) :
      nflag_(g->nsize(), PASS), eflag_(g->esize(), PASS) {

        {
          // Set closed nodes.
          node_property_iterator i   = g->node_property_begin();
          node_property_iterator end = g->node_property_end();

          for (; i!=end; ++i)
            if (is_hidden(i->second)) nflag_[i->first->index()] = CLOSED;
        }{
          // Set closed edges.
          edge_property_iterator i   = g->edge_property_begin();
          edge_property_iterator end = g->edge_property_end();

          for (; i!=end; ++i)
            if (is_hidden(i->second)) eflag_[i->first->index()] = CLOSED;
        }
      }

      int operator()(Edge const* e, Node const* n) const {
        return (nflag_[n->index()]==PASS && eflag_[e->index()]==PASS)?PASS:CLOSED;
      }

      std::vector<int> nflag_;
      std::vector<int> eflag_;
    };
  } // The end of the anonymous namespace
}

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_FLAG_OPERATION_H
