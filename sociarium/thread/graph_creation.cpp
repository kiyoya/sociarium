// s.o.c.i.a.r.i.u.m: thread/graph_creation.cpp
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
#include <unordered_map>
#include <unordered_set>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include "graph_creation.h"
#include "node_size_update.h"
#include "edge_width_update.h"
#include "detail/read_file.h"
#include "../algorithm_selector.h"
#include "../color.h"
#include "../common.h"
#include "../flag_operation.h"
#include "../menu_and_message.h"
#include "../layout.h"
#include "../sociarium_graph_time_series.h"
#include "../texture.h"
#include "../thread.h"
#include "../update_predefined_parameters.h"
#include "../module/graph_creation.h"
#include "../../shared/msgbox.h"
#include "../../shared/predefined_color.h"
#include "../../shared/util.h"
#include "../../shared/win32api.h"

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

  using namespace sociarium_project_common;
  using namespace sociarium_project_menu_and_message;
  using namespace sociarium_project_module_graph_creation;
  using namespace sociarium_project_texture;
  using namespace sociarium_project_thread;
  using namespace sociarium_project_thread_detail_read_file;

  typedef SociariumGraphTimeSeries::StaticNodePropertySet StaticNodePropertySet;
  typedef SociariumGraphTimeSeries::StaticEdgePropertySet StaticEdgePropertySet;

  class GraphCreationThreadImpl : public GraphCreationThread {
  public:
    ////////////////////////////////////////////////////////////////////////////////
    GraphCreationThreadImpl(wchar_t const* filename)
         : filename_(filename),
           is_completed_(false) {}


    ////////////////////////////////////////////////////////////////////////////////
    ~GraphCreationThreadImpl(){}


    ////////////////////////////////////////////////////////////////////////////////
    void terminate(void) {

      initialize_texture_folder_path();

      wglMakeCurrent(0, 0);

      // Clear the progress message.
      deque<wstring>& status = get_status(GRAPH_CREATION);
      deque<wstring>(status.size()).swap(status);

      if (is_completed_) {
        // Update node sizes and edge widths.
        invoke(NODE_SIZE_UPDATE, NodeSizeUpdateThread::create());
        invoke(EDGE_WIDTH_UPDATE, EdgeWidthUpdateThread::create());
      }

      detach(GRAPH_CREATION);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void operator()(void) {

      shared_ptr<SociariumGraphTimeSeries> ts
        = sociarium_project_graph_time_series::get();

      ts->read_lock();
      /*
       * Don't forget to call read_unlock().
       */

      deque<wstring>& status = get_status(GRAPH_CREATION);

      // --------------------------------------------------------------------------------
      // Read and parse a given file.

      unordered_map<wstring, pair<wstring, int> > params;
      vector<pair<wstring, int> > data;

      if (read_file(this, filename_.c_str(), params, data)==false) {
        ts->read_unlock();
        return terminate();
      }

      if (params.empty() && data.empty()) {
        ts->read_unlock();
        return terminate();
      }

      // --------------------------------------------------------------------------------
      // Load a graph creation module.

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos_format
        = params.find(L"format");

      unordered_map<wstring, pair<wstring, int> >::const_iterator pos_module
        = params.find(L"module");

      int data_format = DataFormat::UNSUPPORTED;
      wstring module_filename;

      if (pos_format==params.end()) {
        if (pos_module==params.end()) {
          update_predefined_parameters(params);
          ts->read_unlock();
          return terminate();
        } else {
          module_filename = pos_module->second.first;
          data_format = DataFormat::USER_DEFINED_MODULE;
          params.erase(pos_module);
        }
      } else {

        if (pos_format->second.first==L"AdjacencyMatrix")
          data_format = DataFormat::ADJACENCY_MATRIX;
        else if (pos_format->second.first==L"AdjacencyList")
          data_format = DataFormat::ADJACENCY_LIST;
        else if (pos_format->second.first==L"EdgeList")
          data_format = DataFormat::EDGE_LIST;
        else {
          message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                      L"%s: %s", get_message(Message::UNSUPPORTED_DATA_FORMAT),
                      filename_.c_str());
          ts->read_lock();
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

      HDC dc = get_device_context();
      HGLRC rc = get_rendering_context(RenderingContext::LOAD_TEXTURES);

      if (wglMakeCurrent(dc, rc)==FALSE)
        show_last_error(L"GraphCreationThread::operator()/wglMakeCurrent");

      // --------------------------------------------------------------------------------
      // Execute the module.

      vector<shared_ptr<Graph> > graph_base;

      vector<vector<NodeProperty> > node_property;
      vector<vector<EdgeProperty> > edge_property;

      vector<wstring> layer_name;

      create_graph_time_series(
        *this,
        status,
        get_message_object(),
        graph_base,
        node_property,
        edge_property,
        layer_name,
        params, data,
        filename_);

      if (graph_base.empty()) {
        ts->read_unlock();
        return terminate();
      }

      update_texture_parameters(params);

      // --------------------------------------------------------------------------------
      // Create a sociarium graph.

      size_t const number_of_layers = graph_base.size();

      vector<shared_ptr<SociariumGraph> > graph(number_of_layers);

      SociariumGraphTimeSeries::StaticNodePropertySet static_node_property;
      SociariumGraphTimeSeries::StaticEdgePropertySet static_edge_property;

      unordered_map<wstring, StaticNodeProperty*> identifier2snp;
      unordered_map<wstring, StaticEdgeProperty*> identifier2sep;

      for (size_t layer=0; layer<number_of_layers; ++layer) {

        // **********  Catch a termination signal  **********
        if (cancel_check()) {
          ts->read_unlock();
          return terminate();
        }

        if (number_of_layers>1) {
          status[0]
            = (boost::wformat(L"%s: %d%%")
               %get_message(Message::MAKING_GRAPH_ATTRIBUTES)
               %int(100.0*(layer+1)/number_of_layers)).str();
        } else {
          status[0]
            = (boost::wformat(L"%s")
               %get_message(Message::MAKING_GRAPH_ATTRIBUTES)).str();
        }

        shared_ptr<SociariumGraph> g = graph[layer] = SociariumGraph::create();
        g->import_raw_graph(graph_base[layer]);

        size_t const nsz = g->nsize();
        size_t const esz = g->esize();

        float const deg = float(M_2PI)/nsz;
        float const rad = sociarium_project_layout::get_layout_frame_size();

        Vector2<float> const& center
          = sociarium_project_layout::get_layout_frame_position();

        unordered_set<wstring> check_node_id_duplication;
        unordered_set<wstring> check_edge_id_duplication;

        // Make a node property.
        for (size_t i=0; i<nsz; ++i) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[1]
            = (boost::wformat(L"%s: %d%%")
               %get_message(Message::MAKING_NODE_PROPERTIES)
               %int(100.0*(i+1.0)/nsz)).str();

          NodeProperty const& np = node_property[layer][i];

          if (check_node_id_duplication.find(np.identifier)
              !=check_node_id_duplication.end()) {
            message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                        L"%s: %s [%s]",
                        get_message(Message::NODE_IDENTIFIER_DUPLICATION),
                        filename_.c_str(), np.identifier.c_str());
            ts->read_unlock();
            return terminate();
          }

          check_node_id_duplication.insert(np.name);

          // Associate a node with a static property.
          StaticNodeProperty* snp = 0;
          unordered_map<wstring, StaticNodeProperty*>::iterator id2snp
            = identifier2snp.find(np.identifier);

          if (id2snp==identifier2snp.end()) {
            // Make a new static property for a newly appeared node.
            pair<StaticNodePropertySet::iterator, bool>
              p = static_node_property.insert(
                StaticNodeProperty(static_node_property.size()));
            assert(p.second==true);
            snp = &*p.first;

            {
              // Set identifier and name.
              snp->set_identifier(np.identifier);
              snp->set_name(np.name);
            }{
              // Set texture.
              if (np.texture_file_name.empty()) {
                Texture const* texture = get_texture_by_name(np.name);
                if (texture) {
                  snp->set_texture(texture);
                  snp->set_flag(snp->get_flag()|ElementFlag::TEXTURE_IS_SPECIFIED);
                } else
                  snp->set_texture(get_default_node_texture_tmp());
              } else {
                Texture const* texture = get_texture(np.texture_file_name);
                if (texture) snp->set_texture(texture);
                else         snp->set_texture(get_default_node_texture_tmp());
              }
            }{
              // Set position.
              if (np.position!=0)
                snp->set_position(*np.position);
              else
                snp->set_position(Vector2<float>(
                  center.x+rad*cosf(i*deg), center.y+rad*sinf(i*deg)));
              if (np.position_is_fixed)
                snp->set_flag(ElementFlag::CAPTURED);
            }

            // Store.
            identifier2snp.insert(make_pair(snp->get_identifier(), snp));

          } else snp = id2snp->second;

          assert(snp!=0);

          // Make a dynamic property, and associate it with a graph element and
          // a static property.
          Node* n = g->node(i);

          DynamicNodeProperty& dnp
            = link_dynamic_property_and_graph_element<DynamicNodeProperty>(g, n);

          link_dynamic_and_static_properties(layer, &dnp, snp);

          dnp.set_flag(ElementFlag::VISIBLE);

          if (is_active(*snp, ElementFlag::TEXTURE_IS_SPECIFIED))
            dnp.set_color_id(PredefinedColor::WHITE);
          else
            dnp.set_color_id(sociarium_project_color::get_default_node_color_id());

          dnp.set_weight(np.weight);
          dnp.set_size(1.0f);
        }

        // Make an edge property.
        for (size_t i=0; i<esz; ++i) {

          // **********  Catch a termination signal  **********
          if (cancel_check()) {
            ts->read_unlock();
            return terminate();
          }

          status[1]
            = (boost::wformat(L"%s: %d%%")
               %get_message(Message::MAKING_EDGE_PROPERTIES)
               %int(100.0*(i+1.0)/esz)).str();

          EdgeProperty const& ep = edge_property[layer][i];

          if (check_edge_id_duplication.find(ep.identifier)
              !=check_edge_id_duplication.end()) {
            message_box(get_window_handle(), mb_error, APPLICATION_TITLE,
                        L"%s: %s [%s]",
                        get_message(Message::EDGE_IDENTIFIER_DUPLICATION),
                        filename_.c_str(), ep.identifier.c_str());
            ts->read_unlock();
            return terminate();
          }

          check_edge_id_duplication.insert(ep.identifier);

          // Associate an edge with a static property.
          StaticEdgeProperty* sep = 0;
          unordered_map<wstring, StaticEdgeProperty*>::iterator id2sep
            = identifier2sep.find(ep.identifier);

          if (id2sep==identifier2sep.end()) {
            // Make a new static property for a newly appeared edge.
            pair<StaticEdgePropertySet::iterator, bool>
              p = static_edge_property.insert(
                StaticEdgeProperty(static_edge_property.size()));
            assert(p.second==true);
            sep = &*p.first;

            sep->set_identifier(ep.identifier);
            sep->set_name(ep.name);
            sep->set_texture(sociarium_project_texture::get_default_edge_texture_tmp());

            // Store.
            identifier2sep.insert(make_pair(sep->get_identifier(), sep));

          } else sep = id2sep->second;

          assert(sep!=0);

          // Make a dynamic property, and associate it with a graph element and
          // a static property.
          Edge* e = g->edge(i);

          DynamicEdgeProperty& dep
            = link_dynamic_property_and_graph_element<DynamicEdgeProperty>(g, e);

          link_dynamic_and_static_properties(layer, &dep, sep);

          dep.set_flag(ElementFlag::VISIBLE);
          dep.set_color_id(sociarium_project_color::get_default_edge_color_id());
          dep.set_weight(ep.weight);
          dep.set_width(1.0f);
        }
      }

      ts->update(graph, static_node_property, static_edge_property, layer_name);

      update_predefined_parameters(params);
      sociarium_project_texture::update_default_textures();

      ts->read_unlock();
      is_completed_ = true;
      terminate();
    }

  private:
    wstring const filename_;
    bool is_completed_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of GraphCreationThread.
  shared_ptr<GraphCreationThread> GraphCreationThread::create(wchar_t const* filename) {
    return shared_ptr<GraphCreationThread>(new GraphCreationThreadImpl(filename));
  }

} // The end of the namespace "hashimoto_ut"
