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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_MODULE_CREATION_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_MODULE_CREATION_H

#include <vector>
#include <string>
#include <utility>
#ifdef _MSC_VER
#include <memory>
#include <windows.h>
#else
#include <tr1/memory>
#endif

namespace hashimoto_ut {

  class Thread;
  class Graph;

  // データ入力モジュール内で与えるグラフ要素の動的属性
  namespace sociarium_project_user_defined_property {

    struct NodeProperty {
      std::wstring name;
      std::wstring text;
      float weight;
    };

    struct EdgeProperty {
      std::wstring text;
      float weight;
    };

  } // The end of the namespace "sociarium_project_user_defined_property"

#ifdef _MSC_VER
  typedef __declspec(dllimport)
    void (__cdecl* FuncCreateGraphTimeSeries)(
#else
  typedef void (*FuncCreateGraphTimeSeries)(
#endif
      Thread* parent,
      std::wstring* message,
      std::vector<std::tr1::shared_ptr<Graph> >& graph,
      std::vector<std::vector<sociarium_project_user_defined_property::NodeProperty> >& node_property,
      std::vector<std::vector<sociarium_project_user_defined_property::EdgeProperty> >& edge_property,
      std::vector<std::wstring>& time_label,
      char const* filename
      );

  namespace sociarium_project_module_creation {

    enum { ADJACENCY_MATRIX, ADJACENCY_LIST, EDGE_LIST, USE_OTHER_MODULE, UNSUPPORTED };

    void create(void);
    FuncCreateGraphTimeSeries get(int data_format, wchar_t const* filename);

  } // The end of the namespace "sociarium_project_module_creation"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_MODULE_CREATION_H
