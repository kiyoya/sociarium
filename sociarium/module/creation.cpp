// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/31

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

#pragma warning(disable:4503)

#include <cassert>
#include <unordered_map>
#include "creation.h"
#include "../common.h"
#include "../message.h"
#include "../../shared/msgbox.h"

namespace hashimoto_ut {

  using std::wstring;
  using std::pair;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;

  namespace sociarium_project_module_creation {

    namespace {

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      class CreationModuleManager {
      public:
        CreationModuleManager(void) {}

        ~CreationModuleManager() {
          for (unordered_map<wstring, pair<HMODULE, FuncCreateGraphTimeSeries> >::iterator i=module_.begin(); i!=module_.end(); ++i)
            if (i->second.first) FreeLibrary(i->second.first);
        }

        FuncCreateGraphTimeSeries get(int data_format, wchar_t const* filename) {
          HMODULE handle = 0;
          wstring path = L"dll\\";
          if (data_format==ADJACENCY_MATRIX)      path += L"creation_read_adjacency_matrix.dll";
          else if (data_format==ADJACENCY_LIST)   path += L"creation_read_adjacency_list.dll";
          else if (data_format==EDGE_LIST)        path += L"creation_read_edge_list.dll";
          else if (data_format==USE_OTHER_MODULE) path += filename;
          else assert(0 && "never reach");

          // モジュールが既にロード済みか判定
          if (module_.find(path)==module_.end()) {
            if ((handle=LoadLibrary(path.c_str()))==0) {
              // 新規ロードに失敗した場合
              LPVOID buf;
              FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL, GetLastError(),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR) &buf, 0, NULL);
              MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                     L"%s [%s]", (wchar_t*)buf, path.c_str());
              LocalFree(buf);
              return 0;
            }
            module_[path].first = handle;
          } else {
            // ロード済みの場合
            return module_[path].second;
          }

          if ((module_[path].second=(FuncCreateGraphTimeSeries)GetProcAddress(module_[path].first, "create_graph_time_series"))==0) {
            LPVOID buf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL, GetLastError(),
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR) &buf, 0, NULL);
            MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                   L"%s [%s]", (wchar_t*)buf, path.c_str());
            LocalFree(buf);
            return 0;
          }
          return module_[path].second;
        }

      private:
        unordered_map<wstring, pair<HMODULE, FuncCreateGraphTimeSeries> > module_;
      };

      shared_ptr<CreationModuleManager> graph_creation_module_manager;

    } // The end of the anonymous namespace

    void create(void) {
      graph_creation_module_manager = shared_ptr<CreationModuleManager>(new CreationModuleManager());
    }

    FuncCreateGraphTimeSeries get(int data_format, wchar_t const* filaname) {
      return graph_creation_module_manager->get(data_format, filaname);
    }

  } // The end of the namespace "sociarium_project_module_creation"

} // The end of the namespace "hashimoto_ut"
