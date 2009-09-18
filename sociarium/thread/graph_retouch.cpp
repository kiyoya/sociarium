// s.o.c.i.a.r.i.u.m: thread/graph_retouch.cpp
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
#include <deque>
#include <string>
#include <fstream>
#ifdef _MSC_VER
#include <unordered_map>
#include <unordered_set>
#else
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include "graph_retouch.h"
#include "node_size_update.h"
#include "edge_width_update.h"
#include "detail/read_file.h"
#include "../module/graph_creation.h"
#include "../common.h"
#include "../language.h"
#include "../color.h"
#include "../thread.h"
#include "../layout.h"
#include "../texture.h"
#include "../algorithm_selector.h"
#include "../sociarium_graph_time_series.h"
#include "../update_predefined_parameters.h"
#include "../../shared/general.h"
#include "../../shared/win32api.h"
#include "../../shared/msgbox.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::string;
  using std::wstring;
  using std::pair;
  using std::make_pair;
  using std::ifstream;
  using std::getline;
  using std::tr1::shared_ptr;
  using std::tr1::weak_ptr;
  using std::tr1::unordered_map;
  using std::tr1::unordered_set;

  using namespace sociarium_project_thread;
  using namespace sociarium_project_module_graph_creation;
  using namespace sociarium_project_common;
  using namespace sociarium_project_texture;
  using namespace sociarium_project_thread_detail_read_file;
  using namespace sociarium_project_language;

  class GraphRetouchThreadImpl : public GraphRetouchThread {
  public:
    ////////////////////////////////////////////////////////////////////////////////
    GraphRetouchThreadImpl(wchar_t const* filename)
         : filename_(filename),
           is_completed_(false) {}


    ////////////////////////////////////////////////////////////////////////////////
    ~GraphRetouchThreadImpl(){}


    ////////////////////////////////////////////////////////////////////////////////
    void terminate(void) {

#ifdef __APPLE__
      if (CGLSetCurrentContext(NULL) != kCGLNoError)
#elif _MSC_VER
      if (wglMakeCurrent(0, 0)==FALSE)
#endif
        show_last_error(L"GraphRetouchThread::terminate/wglMakeCurrent");

      // Clear the progress message.
      deque<wstring>& status = get_status(GRAPH_RETOUCH);
      deque<wstring>(status.size()).swap(status);

      if (is_completed_) {
        // Update node sizes and edge widths.
        invoke(NODE_SIZE_UPDATE, NodeSizeUpdateThread::create());
        invoke(EDGE_WIDTH_UPDATE, EdgeWidthUpdateThread::create());
      }

      detach(GRAPH_RETOUCH);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void operator()(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      ts->read_lock();
      /*
       * Don't forget to call read_unlock().
       */

      deque<wstring>& status = get_status(GRAPH_RETOUCH);

      // --------------------------------------------------------------------------------
      // Read and parse the file.

      unordered_map<string, pair<string, int> > params;
      vector<pair<string, int> > data;
      string const filename_mb = wcs2mbcs(filename_.c_str(), filename_.size());

      if (read_file(this, filename_mb.c_str(), params, data)==false) {
        ts->read_unlock();
        return terminate();
      }

      if (params.empty() && data.empty()) {
        ts->read_unlock();
        return terminate();
      }

      // --------------------------------------------------------------------------------
      // Load the graph creation module.

      unordered_map<string, pair<string, int> >::const_iterator pos_format
        = params.find("format");
      unordered_map<string, pair<string, int> >::const_iterator pos_module
        = params.find("module");

      int data_format = DataFormat::UNSUPPORTED;
      wstring module_filename;

      if (pos_format==params.end()) {
        if (pos_module==params.end()) {
          update_predefined_parameters(params);
          ts->read_unlock();
          return terminate();
        } else {
          string const s = pos_module->second.first;
          module_filename = mbcs2wcs(s.c_str(), s.size());
          data_format = DataFormat::USER_DEFINED_MODULE;
          params.erase(pos_module);
        }
      } else {

        if (pos_format->second.first=="AdjacencyMatrix")
          data_format = DataFormat::ADJACENCY_MATRIX;
        else if (pos_format->second.first=="AdjacencyList")
          data_format = DataFormat::ADJACENCY_LIST;
        else if (pos_format->second.first=="EdgeList")
          data_format = DataFormat::EDGE_LIST;
        else {
          message_box(
            get_window_handle(),
            MessageType::CRITICAL,
            APPLICATION_TITLE,
            L"%s: %s",
            get_message(Message::UNSUPPORTED_DATA_FORMAT),
            filename_.c_str());
          return terminate();
        }

        params.erase(pos_format);
      }

      FuncCreateGraphTimeSeries create_graph_time_series
        = get(data_format, module_filename.c_str());

      if (create_graph_time_series==0) {
        ts->read_unlock();
        return terminate();
      }

#ifdef __APPLE__
      CGLContextObj context = get_rendering_context(RenderingContext::LOAD_TEXTURES);
      
      if (CGLSetCurrentContext(context) != kCGLNoError)
#elif _MSC_VER
      HDC dc = get_device_context();
      HGLRC rc = get_rendering_context(RenderingContext::LOAD_TEXTURES);

      if (wglMakeCurrent(dc, rc)==FALSE)
#endif
        show_last_error(L"GraphRetouchThread::operator()/wglMakeCurrent");
      
      // --------------------------------------------------------------------------------
      // Execute the module function.

      vector<shared_ptr<Graph> > graph_base;
      vector<vector<NodeProperty> > node_property;
      vector<vector<EdgeProperty> > edge_property;
      vector<wstring> layer_name;

      create_graph_time_series(
        this,
        status,
        get_message_object(),
        graph_base,
        node_property,
        edge_property,
        layer_name,
        params, data,
        filename_);

      update_predefined_parameters(params);

      if (graph_base.empty()) {
        ts->read_unlock();
        return terminate();
      }

      //       // ----------------------------------------------------------------------------------------------------
      //       // Merge an old and a new graph.
      //
      //       size_t const number_of_layers_old = time_series_->number_of_layers();
      //       size_t const number_of_layers_new = graph_base.size(); assert(number_of_layers_new==layer_name.size());
      //
      //       // 新たに生成される静的属性を格納
      //       typedef SociariumGraph::StaticNodePropertySet StaticNodePropertySet;
      //       typedef SociariumGraph::StaticEdgePropertySet StaticEdgePropertySet;
      //       StaticNodePropertySet static_node_property_new;
      //       StaticEdgePropertySet static_edge_property_new;
      //
      //       // 名前から静的属性を取得
      //       unordered_map<wstring, StaticNodeProperty* > name2snp;
      //       unordered_map<wstring, StaticEdgeProperty* > name2sep;
      //       for (StaticNodePropertySet::const_iterator i=time_series_->static_node_property_begin(0);
      //            i!=time_series_->static_node_property_end(); ++i) name2snp[(*i)->get_name()] = *i;
      //       for (StaticEdgePropertySet::const_iterator i=time_series_->static_edge_property_begin(0);
      //            i!=time_series_->static_edge_property_end(); ++i) name2sep[(*i)->get_name()] = *i;
      //
      //        // レイヤー名からレイヤーインデクスを取得
      //       unordered_map<wstring, size_t> name2layer_old;
      //       unordered_map<wstring, size_t> name2layer_new;
      //       for (size_t layer=0; layer<number_of_layers_old; ++layer)
      //         name2layer_old.insert(make_pair(time_series_->get_layer_name(layer), layer));
      //       for (size_t layer=0; layer<number_of_layers_new; ++layer)
      //         name2layer_new.insert(make_pair(layer_name[layer], layer));
      //
      //
      //       for (size_t layer=0; layer<number_of_layers; ++layer) {
      //
      //         if (number_of_layers>1) sociarium_project_language::get()->first
      //           = (boost::wformat(L"%s: %d%%")
      //              %sociarium_project_language::MAKING_GRAPH_ATTRIBUTES
      //              %int(100.0*(layer+1)/number_of_layers)).str();
      //
      //         // ----------------------------------------------------------------------------------------------------
      //         if (cancel_check()) return terminate();
      //         // ----------------------------------------------------------------------------------------------------
      //
      //         shared_ptr<SociariumGraph> g = graph[layer] = SociariumGraph::create();
      //         g->import_raw_graph(graph_base[layer]);
      //
      //         size_t const nsz = g->nsize();
      //         size_t const esz = g->esize();
      //
      //         double const deg = M_2PI/nsz;
      //         double const rad = sociarium_project_draw::get_layout_frame_size();
      //         Vector2<double> const& center = sociarium_project_draw::get_layout_frame_position();
      //
      //         unordered_set<wstring> check_node_name_duplication;
      //         unordered_set<wstring> check_edge_name_duplication;
      //
      //         // ノードの属性
      //         for (size_t i=0; i<nsz; ++i) {
      //
      //           // ----------------------------------------------------------------------------------------------------
      //           if (cancel_check()) return terminate();
      //           // ----------------------------------------------------------------------------------------------------
      //
      //           sociarium_project_user_defined_property::NodeProperty const& np = node_property[layer][i];
      //
      //           if (check_node_name_duplication.find(np.name)!=check_node_name_duplication.end()) {
      //             MsgBox(get_window_handle(), APPLICATION_TITLE,
      //                    L"%s [%s:%s]", sociarium_project_language::ERROR_NODE_LABEL_DUPLICATION, filename_.c_str(), np.name.c_str());
      //             return terminate();
      //           }
      //
      //           check_node_name_duplication.insert(np.name);
      //
      //           // 静的属性の作成
      //           StaticNodeProperty* snp;
      //           if (name2snp.find(np.name)==name2snp.end()) {
      //             snp.reset(new StaticNodeProperty(static_node_property.size()));
      //             static_node_property.insert(snp);
      //             snp->set_name(np.name);
      //             snp->set_position(Vector2<float>(float(center.x+rad*cos(i*deg)), float(center.y+rad*sin(i*deg))));
      //             snp->set_texture(sociarium_project_texture::get_node_texture(np.name));
      //             name2snp.insert(make_pair(snp->get_name(), snp));
      //           } else {
      //             snp = name2snp[np.name];
      //           }
      //           // 動的属性の作成と静的属性とのリンク
      //           assert(snp!=0);
      //           Node* n = g->node(i);
      //           DynamicNodeProperty* dnp = link_dynamic_property_and_graph_element<DynamicNodeProperty>(g, n);
      //           link_dynamic_and_static_properties(layer, dnp, snp);
      //           dnp->set_flag(ElementFlag::VISIBLE);
      //           dnp->set_color_id(PredefinedColor::WHITE);
      //           dnp->set_weight(np.weight);
      //           dnp->set_size(sociarium_project_draw::get_default_node_size());
      //         }
      //
      //         // エッジの属性
      //         for (size_t i=0; i<esz; ++i) {
      //
      //           // ----------------------------------------------------------------------------------------------------
      //           if (cancel_check()) return terminate();
      //           // ----------------------------------------------------------------------------------------------------
      //
      //           Edge* e = g->edge(i);
      //           sociarium_project_user_defined_property::EdgeProperty const& ep = edge_property[layer][i];
      //           wstring const& name0 = g->property(e->source())->get_static_property()->get_name();
      //           wstring const& name1 = g->property(e->target())->get_static_property()->get_name();
      //           wstring const name = name0+L"~"+name1;
      //
      //           if (check_edge_name_duplication.find(name)!=check_edge_name_duplication.end()) {
      //             MsgBox(get_window_handle(), APPLICATION_TITLE,
      //                    L"%s [%s:%s]", sociarium_project_language::ERROR_EDGE_LABEL_DUPLICATION, filename_.c_str(), name.c_str());
      //             return terminate();
      //           }
      //
      //           check_edge_name_duplication.insert(name);
      //
      //           // 静的属性の作成
      //           StaticEdgeProperty* sep;
      //           if (name2sep.find(name)==name2sep.end()) {
      //             sep.reset(new StaticEdgeProperty(static_edge_property.size()));
      //             static_edge_property.insert(sep);
      //             sep->set_name(name);
      //             sep->set_texture(sociarium_project_texture::get_default_edge_texture());
      //             name2sep.insert(make_pair(sep->get_name(), sep));
      //           } else {
      //             sep = name2sep[name];
      //           }
      //           // 動的属性の作成と静的属性とのリンク
      //           assert(sep!=0);
      //           DynamicEdgeProperty* dep = link_dynamic_property_and_graph_element<DynamicEdgeProperty>(g, e);
      //           link_dynamic_and_static_properties(layer, dep, sep);
      //           dep->set_flag(ElementFlag::VISIBLE);
      //           dep->set_color_id(PredefinedColor::WHITE);
      //           dep->set_weight(ep.weight);
      //         }
      //       }
      //
      //       ts->update(graph, static_node_property, static_edge_property, layer_name);

      ts->read_unlock();
      is_completed_ = true;
      terminate();
    }

  private:
    wstring const filename_;
    bool is_completed_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of GraphRetouchThread.
  shared_ptr<GraphRetouchThread> GraphRetouchThread::create(wchar_t const* filename) {
    return shared_ptr<GraphRetouchThread>(new GraphRetouchThreadImpl(filename));
  }

} // The end of the namespace "hashimoto_ut"
