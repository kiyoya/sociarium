﻿// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/03/06

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

#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

#include <cassert>
#ifdef _MSC_VER
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#include "layout.h"
#include "../common.h"
#include "../algorithm_selector.h"
#include "../../shared/msgbox.h"

namespace hashimoto_ut {

  using std::wstring;
  using std::pair;
  using std::tr1::unordered_map;
  using std::tr1::shared_ptr;

  namespace sociarium_project_module_layout {

    namespace {
      // グラフレイアウトモジュール
      class LayoutModuleManager {
      public:
        LayoutModuleManager(void) {}

        ~LayoutModuleManager() {
#ifdef __APPLE__
          for (unordered_map<CFURLRef, pair<CFBundleRef, FuncLayoutGraph> >::iterator i=module_.begin(); i!=module_.end(); ++i)
          {
            if (i->first) CFRelease(i->first);
            if (i->second.first) CFRelease(i->second.first);
          }
#elif _MSC_VER
          for (unordered_map<wstring, pair<HMODULE, FuncLayoutGraph> >::iterator i=module_.begin(); i!=module_.end(); ++i)
            if (i->second.first) FreeLibrary(i->second.first);
#endif
        }

        FuncLayoutGraph get(int method) {
#ifdef __APPLE__
          CFBundleRef handle = NULL;
          CFBundleRef mainBundle = CFBundleGetMainBundle();
          CFURLRef pluginURL = CFBundleCopyBuiltInPlugInsURL(mainBundle);
          CFURLRef path;
          if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::KAMADA_KAWAI_METHOD)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("LayoutKamadaKawaiMethod.plugin"), FALSE);
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::HDE)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("LayoutHighDimensionalEmbedding.plugin"), FALSE);
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("LayoutCircle.plugin"), FALSE);
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("LayoutCircleInSizeOrder.plugin"), FALSE);
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::RANDOM)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("LayoutRandom.plugin"), FALSE);
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::ARRAY)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("LayoutArray.plugin"), FALSE);
          else assert(0 && "never");
          CFRelease(pluginURL);
#elif _MSC_VER
          HMODULE handle = 0;
          wstring path = L"dll\\";
          if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::KAMADA_KAWAI_METHOD)
            path += L"layout_kamada_kawai_method.dll";
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::HDE)
            path += L"layout_high_dimensional_embedding.dll";
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE)
            path += L"layout_circle.dll";
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::CIRCLE_IN_SIZE_ORDER)
            path += L"layout_circle_in_size_order.dll";
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::RANDOM)
            path += L"layout_random.dll";
          else if (method==sociarium_project_algorithm_selector::LayoutAlgorithm::ARRAY)
            path += L"layout_array.dll";
          else assert(0 && "never");
#endif
          
          // モジュールが既にロード済みか判定
          if (module_.find(path)==module_.end()) {
#ifdef __APPLE__
            if ((handle=CFBundleCreate(kCFAllocatorSystemDefault, path))==NULL) {
              // [TODO]
            }
#elif _MSC_VER
            if ((handle=LoadLibrary(path.c_str()))==0) {
              // 新規ロードに失敗した場合
              LPVOID buf;
              FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL, GetLastError(),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR) &buf, 0, NULL);
              MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE, L"%s [%s]", (wchar_t*)buf, path.c_str());
              LocalFree(buf);
              return 0;
            }
#endif
            module_[path].first = handle;
          } else {
            // ロード済みの場合
            return module_[path].second;
          }

#ifdef __APPLE__
          if ((module_[path].second=(FuncLayoutGraph)CFBundleGetFunctionPointerForName(module_[path].first, CFSTR("layout_graph")))==NULL) {
            // [TODO]
          }
#elif _MSC_VER
          if ((module_[path].second=(FuncLayoutGraph)GetProcAddress(module_[path].first, "layout_graph"))==0) {
            LPVOID buf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL, GetLastError(),
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR) &buf, 0, NULL);
            MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE, L"%s [%s]", (wchar_t*)buf, path.c_str());
            LocalFree(buf);
            return 0;
          }
#endif
          return module_[path].second;
        }

      private:
#ifdef __APPLE__
        unordered_map<CFURLRef, pair<CFBundleRef, FuncLayoutGraph> > module_;
#elif _MSC_VER
        unordered_map<wstring, pair<HMODULE, FuncLayoutGraph> > module_;
#endif
      };

      shared_ptr<LayoutModuleManager> graph_layout_module_manager;

    } // The end of the anonymous namespace

    void create(void) {
      graph_layout_module_manager = shared_ptr<LayoutModuleManager>(new LayoutModuleManager());
    }

    FuncLayoutGraph get(int method) {
      return graph_layout_module_manager->get(method);
    }

  } // The end of the namespace "sociarium_project_module_layout"

} // The end of the namespace "hashimoto_ut"
