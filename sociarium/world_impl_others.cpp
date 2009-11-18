// s.o.c.i.a.r.i.u.m: world_impl_others.cpp
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

#include <vector>
#include <map>
#include <fstream>
#include <windows.h>
#include "common.h"
#include "menu_and_message.h"
#include "selection.h"
#include "sociarium_graph_time_series.h"
#include "thread.h"
#include "world_impl.h"
#include "../shared/msgbox.h"
#include "../shared/win32api.h"

namespace hashimoto_ut {

  using std::vector;
  using std::map;
  using std::make_pair;
  using std::string;
  using std::wstring;
  using std::ofstream;
  using std::endl;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_common;
  using namespace sociarium_project_thread;
  using namespace sociarium_project_menu_and_message;

  ////////////////////////////////////////////////////////////////////////////////
  HWND WorldImpl::get_window_handle(void) const {
    return hwnd_;
  }

  HDC WorldImpl::get_device_context(void) const {
    return dc_;
  }

  HGLRC WorldImpl::get_rendering_context(int thread_id) const {
    assert(0<=thread_id && thread_id<RenderingContext::NUMBER_OF_THREAD_CATEGORIES);
    return rc_[thread_id];
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::step_forward_layer(void) {
    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();
    ts->move_layer(ts->index_of_current_layer()+1);
  }

  void WorldImpl::step_backward_layer(void) {
    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();
    ts->move_layer(ts->index_of_current_layer()-1);
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::clear_community(void) const {

    bool another_thread_is_running = false;

    for (int i=0; i<NUMBER_OF_THREAD_CATEGORIES; ++i) {
      if (i!=FORCE_DIRECTION && joinable(i)) {
        another_thread_is_running = true;
        break;
      }
    }

    if (another_thread_is_running) {
      message_box(hwnd_, mb_notice, APPLICATION_TITLE,
                  L"%s", get_message(Message::ANOTHER_THREAD_IS_RUNNING));
      return;
    }

    if (message_box(hwnd_, mb_ok_cancel, APPLICATION_TITLE,
                    L"%s", get_message(Message::CLEAR_COMMUNITY))==IDCANCEL)
      return;

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    ts->clear_community();
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::output_degree_distribution(wchar_t const* filename) const {

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    string const filename_mb = wcs2mbcs(filename, wcslen(filename));
    ofstream ofs(filename_mb.c_str());

    TimeSeriesLock lock(ts, TimeSeriesLock::Read);

    for (size_t i=0; i<ts->number_of_layers(); ++i) {

      wstring const& layer_name =  ts->get_layer_name(i);
      ofs << "#" << i << " " << wcs2mbcs(layer_name.c_str(), layer_name.size()) << endl;

      shared_ptr<SociariumGraph> g = ts->get_graph(0, i);

      map<int, int> histogram;

      for (node_iterator i=g->nbegin(), end=g->nend(); i!=end; ++i)
        ++histogram[int((*i)->degree())];

      for (map<int, int>::const_iterator i=histogram.begin(); i!=histogram.end(); ++i)
        ofs << i->first << "\t" << double(i->second)/g->nsize() << endl;

      ofs << endl;
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  void WorldImpl::output_community_information(wchar_t const* filename) const {

    shared_ptr<SociariumGraphTimeSeries> ts
      = sociarium_project_graph_time_series::get();

    std::wofstream ofs(filename);

    TimeSeriesLock lock(ts, TimeSeriesLock::Read);

    typedef SociariumGraphTimeSeries::StaticNodePropertySet StaticNodePropertySet;

      StaticNodePropertySet::const_iterator i = ts->static_node_property_begin(0);
    StaticNodePropertySet::const_iterator end = ts->static_node_property_end(0);

    ofs << L"# Node Affiliation" << endl;
    ofs << L"#" << endl;
    ofs << L"# [Format]" << endl;
    ofs << L"# node_id\tnode_name" << endl;
    ofs << L"# layer_id\tlayer_name\tcommunity_id0\tcommunity_id1\t...\n" << endl;

    for (; i!=end; ++i) {

      ofs << i->get_id() << L'\t' << i->get_name() << endl;

      map<size_t, DynamicNodeProperty*> m;

      {
        StaticNodeProperty::DynamicPropertyMap::const_iterator j = i->dynamic_property_begin();
        StaticNodeProperty::DynamicPropertyMap::const_iterator jend = i->dynamic_property_end();
        for (; j!=jend; ++j)
          m.insert(make_pair(j->second, j->first));
      }

      map<size_t, DynamicNodeProperty*>::const_iterator j = m.begin();
      map<size_t, DynamicNodeProperty*>::const_iterator jend = m.end();

      for (; j!=jend; ++j) {

        ofs << j->first << L'\t' << ts->get_layer_name(j->first);

        DynamicNodeProperty* dnp = j->second;

        vector<DynamicNodeProperty*>::const_iterator k = dnp->upper_nbegin();
        vector<DynamicNodeProperty*>::const_iterator kend = dnp->upper_nend();

        for (; k!=kend; ++k)
          ofs << L'\t' << (*k)->get_static_property()->get_id();
        ofs << endl;
      }

      ofs << endl;
    }
  }

} // The end of the namespace "hashimoto_ut"
