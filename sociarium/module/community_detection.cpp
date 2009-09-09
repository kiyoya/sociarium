// s.o.c.i.a.r.i.u.m: module/community_detection.cpp
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

#ifdef _MSC_VER
#pragma warning(disable:4503) // about the name decoration.
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
#include "community_detection.h"
#include "../common.h"
#include "../language.h"
#include "../algorithm_selector.h"

namespace hashimoto_ut {

  using std::string;
  using std::wstring;
  using std::pair;
  using std::tr1::unordered_map;

  using namespace sociarium_project_common;
  using namespace sociarium_project_language;

  namespace sociarium_project_module_community_detection {

    namespace {

      ////////////////////////////////////////////////////////////////////////////////
      // The module function should have the following name.
      string const function_name = "detect_community";


      ////////////////////////////////////////////////////////////////////////////////
      class CommunityDetectionModuleManager {
      public:
        typedef unordered_map<wstring, pair<HMODULE, FuncDetectCommunity> > ModuleMap;

        CommunityDetectionModuleManager(void) {}

        ~CommunityDetectionModuleManager() {
          for (ModuleMap::const_iterator i=module_.begin(); i!=module_.end(); ++i)
          {
#ifdef __APPLE__
            if (i->first) CFRelease(i->first);
            if (i->second.first) CFRelease(i->second.first);
#elif _MSC_VER
            if (i->second.first) FreeLibrary(i->second.first);
#endif
          }
        }

        FuncDetectCommunity get(int method) {

          using namespace sociarium_project_algorithm_selector;

#ifdef __APPLE__
          CFBundleRef handle = NULL;
          CFBundleRef mainBundle = CFBundleGetMainBundle();
          CFURLRef pluginURL = CFBundleCopyBuiltInPlugInsURL(mainBundle);
          CFURLRef path;
          if (method==CommunityDetectionAlgorithm::CONNECTED_COMPONENTS)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionConnectedComponents.plugin"), FALSE);
          else if (method==CommunityDetectionAlgorithm::STRONGLY_CONNECTED_COMPONENTS)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionStronglyConnectedComponents.plugin"), FALSE);
          else if (method==CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionModularityMaximizationUsingGreedyMethod.plugin"), FALSE);
          else if (method==CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_TEO_METHOD)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionModularityMaximizationUsingT-EoMethod.plugin"), FALSE);
          else if (method==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_3)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionCliquePercolation3.plugin"), FALSE);
          else if (method==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_4)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionCliquePercolation4.plugin"), FALSE);
          else if (method==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_5)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionCliquePercolation5.plugin"), FALSE);
          else if (method==CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_SEPARATION)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionBetweennessCentralitySeparation.plugin"), FALSE);
          else if (method==CommunityDetectionAlgorithm::INFORMATION_FLOW_MAPPING)
            path = CFURLCreateCopyAppendingPathComponent(NULL, pluginURL, CFSTR("CommunityDetectionInformationFlowMapping.plugin"), FALSE);
          else assert(0 && "never");
          CFRelease(pluginURL);
#elif _MSC_VER
          HMODULE handle = 0;
          wstring path = L"dll\\";
          if (method==CommunityDetectionAlgorithm::CONNECTED_COMPONENTS)
            path += L"community_detection_connected_components.dll";
          else if (method==CommunityDetectionAlgorithm::STRONGLY_CONNECTED_COMPONENTS)
            path += L"community_detection_strongly_connected_components.dll";
          else if (method==CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD)
            path += L"community_detection_modularity_maximization_using_greedy_method.dll";
          else if (method==CommunityDetectionAlgorithm::MODULARITY_MAXIMIZATION_USING_TEO_METHOD)
            path += L"community_detection_modularity_maximization_using_t-eo_method.dll";
          else if (method==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_3)
            path += L"community_detection_clique_percolation_3.dll";
          else if (method==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_4)
            path += L"community_detection_clique_percolation_4.dll";
          else if (method==CommunityDetectionAlgorithm::CLIQUE_PERCOLATION_5)
            path += L"community_detection_clique_percolation_5.dll";
          else if (method==CommunityDetectionAlgorithm::BETWEENNESS_CENTRALITY_SEPARATION)
            path += L"community_detection_betweenness_centrality_separation.dll";
          else if (method==CommunityDetectionAlgorithm::INFORMATION_FLOW_MAPPING)
            path += L"community_detection_information_flow_mapping.dll";
          else assert(0 && "never reach");
#endif

          // Check if the module has already loaded.
          if (module_.find(path)==module_.end()) {
            // Not yet loaded.
#ifdef __APPLE__
            if ((handle=CFBundleCreate(kCFAllocatorSystemDefault, path))==NULL) {
#elif _MSC_VER
            if ((handle=LoadLibrary(path.c_str()))==0) {
#endif
              show_last_error(path.c_str());
              return 0;
            }
            module_[path].first = handle;
          }

          // Yes, already loaded.
          else return module_[path].second;

          // Find an appropriate function in DLL.
#ifdef __APPLE__
          pair<CFBundleRef, FuncDetectCommunity>& p = module_[path];
          p.second = (FuncDetectCommunity)CFBundleGetFunctionPointerForName(p.first, CFSTR(function_name.c_str()));
#elif _MSC_VER
          pair<HMODULE, FuncDetectCommunity>& p = module_[path];
          p.second = (FuncDetectCommunity)GetProcAddress(p.first, function_name.c_str());
#endif

          if (p.second==0) {
            show_last_error(path.c_str());
            return 0;
          }

          return p.second;
        }

      private:
        ModuleMap module_;
      };


      ////////////////////////////////////////////////////////////////////////////////
      CommunityDetectionModuleManager community_detection_module_manager;

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    FuncDetectCommunity get(int method) {
      return community_detection_module_manager.get(method);
    }

  } // The end of the "sociarium_project_module_community_detection"

} // The end of the namespace "hashimoto_ut"
