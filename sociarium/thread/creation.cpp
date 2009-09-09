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

#include <vector>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include "creation.h"
#include "layout.h"
#include "../module/creation.h"
#include "../common.h"
#include "../thread.h"
#include "../message.h"
#include "../draw.h"
#include "../layout.h"
#include "../texture.h"
#include "../sociarium_graph_time_series.h"
#include "../../shared/win32api.h"
#include "../../shared/msgbox.h"
#include "../../shared/predefined_color.h"

namespace hashimoto_ut {

  using std::vector;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::ifstream;
  using std::getline;
  using std::tr1::shared_ptr;
  using std::tr1::unordered_map;
  using std::tr1::unordered_set;

  namespace {
    HWND hwnd = 0;
    HDC dc = 0;
    HGLRC rc_trg = 0;
    HGLRC rc_src = 0;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  CreationThread::CreationThread(
    boost::mutex& m,
    boost::condition& c,
    bool& state,
    HGLRC rc,
    shared_ptr<SociariumGraphTimeSeries> const & time_series,
    wchar_t const* filename)
       : mutex_(m), condition_(c), state_(state), time_series_(time_series), filename_(filename) {
         rc_src = rc;
       }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void CreationThread::terminate(void) const {

    if (wglMakeCurrent(0, 0)==FALSE)
      sociarium_project_common::show_last_error(L"CreationThread::terminate/wglMakeCurrent(0)");
    if (ReleaseDC(hwnd, dc)==0)
      sociarium_project_common::show_last_error(L"CreationThread::terminate/ReleaseDC");
    if (wglDeleteContext(rc_trg)==FALSE)
      sociarium_project_common::show_last_error(L"CreationThread::terminate/wglDeleteContext");

    hwnd = 0;
    dc = 0;
    rc_src = 0;
    rc_trg = 0;

    sociarium_project_message::get()->first = L"";
    sociarium_project_message::get()->second = L"";
    time_series_->read_unlock();

    if (sociarium_project_layout::get_auto_layout()) {
      shared_ptr<Thread> th(new LayoutThread(time_series_));
      sociarium_project_thread::set_current_graph_layout_thread(th);
      boost::thread(boost::ref(*th));
    }

    sociarium_project_thread::set_current_graph_creation_thread(shared_ptr<Thread>());
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void CreationThread::operator()(void) {

    time_series_->read_lock();

    if ((hwnd=sociarium_project_common::get_window_handle())==0)
      sociarium_project_common::show_last_error(L"CreationThread::operator()/get_window_handle");
    if ((dc=GetDC(hwnd))==0)
      sociarium_project_common::show_last_error(L"CreationThread::operator()/GetDC");
    if ((rc_trg=wglCreateContext(dc))==0)
      sociarium_project_common::show_last_error(L"CreationThread::operator()/wglCreateContext");
    if (wglShareLists(rc_src, rc_trg)==FALSE)
      sociarium_project_common::show_last_error(L"CreationThread::operator()/wglShareLists");
    if (wglMakeCurrent(dc, rc_trg)==FALSE)
      sociarium_project_common::show_last_error(L"CreationThread::operator()/wglMakeCurrent(dc)");

    {
      boost::mutex::scoped_lock lock(mutex_);
      state_ = false;
      condition_.notify_one();
    }

    // ----------------------------------------------------------------------------------------------------
    // グラフ作成モジュールのロード
    sociarium_project_message::get()->first = (boost::wformat(L"%s")%sociarium_project_message::LOADING_MODULE).str();
    string const filename_mb = wcs2mbcs(filename_.c_str(), filename_.size());
    ifstream ifs(filename_mb.c_str());
    string line;
    string const prep_module = "#module=";
    int data_format = sociarium_project_module_creation::UNSUPPORTED;
    wstring module_file_name;
    while (getline(ifs, line)) {
      if (line.find("#format=AdjacencyMatrix")==0)    { data_format = sociarium_project_module_creation::ADJACENCY_MATRIX; break; }
      else if (line.find("#format=AdjacencyList")==0) { data_format = sociarium_project_module_creation::ADJACENCY_LIST; break; }
      else if (line.find("#format=EdgeList")==0)      { data_format = sociarium_project_module_creation::EDGE_LIST; break; }
      else if (line.find(prep_module)==0) {
        string const module_file_name_mb = line.substr(prep_module.size());
        module_file_name = mbcs2wcs(module_file_name_mb.c_str(), module_file_name_mb.size());
        data_format = sociarium_project_module_creation::USE_OTHER_MODULE;
        break;
      }
    }

    if (data_format==sociarium_project_module_creation::UNSUPPORTED) {
      MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE, L"%s [%s]",
             sociarium_project_message::ERROR_UNSUPPORTED_DATA_FORMAT, filename_.c_str());
      return terminate();
    }

    FuncCreateGraphTimeSeries create_graph_time_series = sociarium_project_module_creation::get(data_format, module_file_name.c_str());
    if (create_graph_time_series==0) return terminate();

    // ----------------------------------------------------------------------------------------------------
    // グラフ作成モジュールを実行
    sociarium_project_message::get()->first = (boost::wformat(L"%s")%sociarium_project_message::MODULE_IS_RUNNING).str();
    vector<shared_ptr<Graph> > graph_base;
    vector<vector<sociarium_project_user_defined_property::NodeProperty> > node_property;
    vector<vector<sociarium_project_user_defined_property::EdgeProperty> > edge_property;
    vector<wstring> time_label;
    create_graph_time_series(this, &sociarium_project_message::get()->first, graph_base, node_property, edge_property, time_label, filename_mb.c_str());
    if (graph_base.empty()) return terminate();

    // ----------------------------------------------------------------------------------------------------
    // SociariumGraphに変換
    size_t const number_of_layers = graph_base.size();
    vector<shared_ptr<SociariumGraph> > graph(number_of_layers);
    SociariumGraphTimeSeries::StaticNodePropertyContainer static_node_property;
    SociariumGraphTimeSeries::StaticEdgePropertyContainer static_edge_property;
    unordered_map<wstring, shared_ptr<StaticNodeProperty> > name2snp;
    unordered_map<wstring, shared_ptr<StaticEdgeProperty> > name2sep;

    for (size_t layer=0; layer<number_of_layers; ++layer) {

      if (number_of_layers>1) sociarium_project_message::get()->first
        = (boost::wformat(L"%s: %d%%")
           %sociarium_project_message::MAKING_GRAPH_TIME_SERIES_PROPERTIES
           %int(100.0*(layer+1)/number_of_layers)).str();

      // ----------------------------------------------------------------------------------------------------
      if (cancel_check()) return terminate();
      // ----------------------------------------------------------------------------------------------------

      shared_ptr<SociariumGraph>& g = graph[layer] = SociariumGraph::create();
      g->import_raw_graph(graph_base[layer]);

      unordered_set<wstring> node_name_check;
      unordered_set<wstring> edge_name_check;

      size_t const nsz = g->nsize();
      size_t const esz = g->esize();

      double const deg = M_2PI/nsz;
      double const rad = sociarium_project_draw::get_layout_frame_size();

      // ノードの属性
      for (size_t i=0; i<nsz; ++i) {

        (number_of_layers==1?sociarium_project_message::get()->first:sociarium_project_message::get()->second)
          = (boost::wformat(L"%s: %d%%")
             %sociarium_project_message::MAKING_NODE_PROPERTIES
             %int(100.0*(i+0.5)/nsz)).str();

        // ----------------------------------------------------------------------------------------------------
        if (cancel_check()) return terminate();
        // ----------------------------------------------------------------------------------------------------

        const sociarium_project_user_defined_property::NodeProperty& np = node_property[layer][i];

        if (node_name_check.find(np.name)!=node_name_check.end()) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"%s [%s:%s]", sociarium_project_message::ERROR_NODE_NAME_DUPLICATION, filename_.c_str(), np.name.c_str());
          return terminate();
        }

        node_name_check.insert(np.name);

        shared_ptr<StaticNodeProperty> snp;
        Vector2<float> pos_tmp;
        if (name2snp.find(np.name)==name2snp.end()) {
          // 静的属性
          snp.reset(new StaticNodeProperty(static_node_property.size()));
          static_node_property.insert(snp);
          snp->set_name(np.name);
          snp->set_position(Vector2<float>(float(rad*cos(i*deg)), float(rad*sin(i*deg))));
          snp->set_texture(sociarium_project_texture::get_node_texture(np.name));
          name2snp.insert(make_pair(snp->get_name(), snp));
        } else {
          snp = name2snp[np.name];
        }
        assert(snp!=0);
        // 動的属性
        Node* n = g->node(i);
        shared_ptr<DynamicNodeProperty> const& dnp = g->property(n);
        dnp->set_graph_element(n);
        dnp->set_static_property(snp);
        dnp->set_flag(ElementFlag::VISIBLE|ElementFlag::MARKED); // レイアウトのため
        dnp->set_color_id(PredefinedColor::WHITE);
        dnp->set_text(np.text);
        dnp->set_weight(np.weight);
        dnp->set_size(0.0f);
        // 静的属性に動的属性を登録
        snp->add_dynamic_property(dnp.get(), layer);
        assert(dnp->is_valid(snp));

        (number_of_layers==1?sociarium_project_message::get()->first:sociarium_project_message::get()->second)
          = (boost::wformat(L"%s: %d%%")
             %sociarium_project_message::MAKING_NODE_PROPERTIES
             %int(100.0*(i+1.0)/nsz)).str();
      }

      // エッジの属性
      for (size_t i=0; i<esz; ++i) {

        (number_of_layers==1?sociarium_project_message::get()->first:sociarium_project_message::get()->second)
          = (boost::wformat(L"%s: %d%%")
             %sociarium_project_message::MAKING_EDGE_PROPERTIES
             %int(100.0*(i+0.5)/esz)).str();

        // ----------------------------------------------------------------------------------------------------
        if (cancel_check()) return terminate();
        // ----------------------------------------------------------------------------------------------------

        Edge* e = g->edge(i);
        const sociarium_project_user_defined_property::EdgeProperty& ep = edge_property[layer][i];
        wstring const& name0 = g->property(e->source())->get_static_property()->get_name();
        wstring const& name1 = g->property(e->target())->get_static_property()->get_name();
        wstring const name = name0+L"~"+name1;

        if (edge_name_check.find(name)!=edge_name_check.end()) {
          MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                 L"%s [%s:%s]", sociarium_project_message::ERROR_EDGE_NAME_DUPLICATION, filename_.c_str(), name.c_str());
          return terminate();
        }

        edge_name_check.insert(name);

        shared_ptr<StaticEdgeProperty> sep;

        if (name2sep.find(name)==name2sep.end()) {
          // 静的属性
          sep.reset(new StaticEdgeProperty(static_edge_property.size()));
          static_edge_property.insert(sep);
          sep->set_name(name);
          sep->set_texture(sociarium_project_texture::get_default_edge_texture());
          name2sep.insert(make_pair(sep->get_name(), sep));
        } else {
          sep = name2sep[name];
        }
        assert(sep!=0);
        // 動的属性
        shared_ptr<DynamicEdgeProperty> const& dep = g->property(e);
        dep->set_graph_element(e);
        dep->set_static_property(sep);
        dep->set_flag(ElementFlag::VISIBLE|ElementFlag::MARKED); // レイアウトのため
        dep->set_color_id(PredefinedColor::WHITE);
        dep->set_text(ep.text);
        dep->set_weight(ep.weight);
        dep->set_length(1.0f);
        // 静的属性に動的属性を登録
        sep->add_dynamic_property(dep.get(), layer);
        assert(dep->is_valid(sep));

        (number_of_layers==1?sociarium_project_message::get()->first:sociarium_project_message::get()->second)
          = (boost::wformat(L"%s: %d%%")
             %sociarium_project_message::MAKING_EDGE_PROPERTIES
             %int(100.0*(i+1.0)/esz)).str();
      }
    }

    // アップデート
    sociarium_project_message::get()->first = (boost::wformat(L"%s")%sociarium_project_message::UPDATING_TIME_SERIES).str();
    sociarium_project_message::get()->second = L"";
    time_series_->update(graph, static_node_property, static_edge_property, time_label);
    terminate();
  }

} // The end of the namespace "hashimoto_ut"
