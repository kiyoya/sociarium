// s.o.c.i.a.r.i.u.m: module/graph_creation.cpp
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

#pragma warning(disable:4503) // about the name decoration.

#include <cassert>
#include <unordered_map>
#include "graph_creation.h"
#include "../common.h"
#include "../menu_and_message.h"

namespace hashimoto_ut {

  using std::string;
  using std::wstring;
  using std::pair;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_menu_and_message;

  namespace sociarium_project_module_graph_creation {

    namespace {

      ////////////////////////////////////////////////////////////////////////////////
      // The module function should have the following name.
      string const function_name = "create_graph_time_series";


      ////////////////////////////////////////////////////////////////////////////////
      class GraphCreationModuleManager {
      public:
        typedef unordered_map<wstring, pair<HMODULE, FuncCreateGraphTimeSeries> >
          ModuleMap;

        GraphCreationModuleManager(void) {}

        ~GraphCreationModuleManager() {
          for (ModuleMap::const_iterator i=module_.begin(); i!=module_.end(); ++i)
            if (i->second.first) FreeLibrary(i->second.first);
        }

        FuncCreateGraphTimeSeries get(int data_format, wchar_t const* module_filename) {
          HMODULE handle = 0;
          wstring path = L"dll\\";
          if (data_format==DataFormat::ADJACENCY_MATRIX)
            path += L"graph_creation_read_adjacency_matrix.dll";
          else if (data_format==DataFormat::ADJACENCY_LIST)
            path += L"graph_creation_read_adjacency_list.dll";
          else if (data_format==DataFormat::EDGE_LIST)
            path += L"graph_creation_read_edge_list.dll";
          else if (data_format==DataFormat::USER_DEFINED_MODULE)
            path += module_filename;
          else assert(0 && "never reach");

          // Check if the module has already loaded.
          if (module_.find(path)==module_.end()) {
            // Not yet loaded.
            if ((handle=LoadLibrary(path.c_str()))==0) {
              show_last_error(path.c_str());
              return 0;
            }

            module_[path].first = handle;
          }

          // Yes, already loaded.
          else return module_[path].second;

          // Find an appropriate function in DLL.
          pair<HMODULE, FuncCreateGraphTimeSeries>& p = module_[path];
          p.second = (FuncCreateGraphTimeSeries)GetProcAddress(p.first, function_name.c_str());

          if (p.second==0) {
            show_last_error(path.c_str());
            return 0;
          }

          return p.second;
        }

      private:
        ModuleMap module_;
      };

      GraphCreationModuleManager graph_creation_module_manager;

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    FuncCreateGraphTimeSeries get(int data_format, wchar_t const* filaname) {
      return graph_creation_module_manager.get(data_format, filaname);
    }

  } // The end of the namespace "sociarium_project_module_graph_creation"

} // The end of the namespace "hashimoto_ut"
