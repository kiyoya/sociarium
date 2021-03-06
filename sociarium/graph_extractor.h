﻿// s.o.c.i.a.r.i.u.m: graph_extractor.h
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

#include <string>
#ifdef _MSC_VER
#include <memory>
#include <unordered_map>
#else
#include <tr1/memory>
#include <tr1/unordered_map>
#endif
#include "sociarium_graph.h"

namespace hashimoto_ut {

  class Thread;

  namespace sociarium_project_graph_extractor {

    ////////////////////////////////////////////////////////////////////////////////
    // Generate a new graph that consists of elements matching @flag in @graph.
    // If the first return value is false, it means the thread was cancelled.
    std::pair<bool, std::tr1::shared_ptr<Graph> > get(
      Thread* th, std::wstring* status,
      std::tr1::shared_ptr<SociariumGraph const> graph, // Original graph.
      std::tr1::unordered_map<Node*, Node const*>& node2node, // Map a new node to the original node.
      std::tr1::unordered_map<Edge*, Edge const*>& edge2edge, // Map a new edge to the original edge.
      unsigned int flag);

  }; // The end of the namespace "sociarium_project_graph_extractor"

} // The end of the namespace "hashimoto_ut"
