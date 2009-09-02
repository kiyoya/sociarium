// s.o.c.i.a.r.i.u.m
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/05/10

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

#include <fstream>
#include "creation_detail.h"
#include "../common.h"
#include "../message.h"
#include "../../shared/msgbox.h"
#include "../../shared/win32api.h"
#include "../../shared/thread.h"
#include "../../graph/graph.h"

namespace hashimoto_ut {

  namespace sociarium_project_module_creation_detail {

    using std::vector;
    using std::string;
    using std::wstring;
    using std::ifstream;
    using std::getline;
    using std::tr1::shared_ptr;

    namespace {

      string const pre_delimiter = "#delimiter="; // char
      string const pre_title = "#title="; // string
      string const pre_layer_label= "#layer_label="; // "time_t" or not
      string const pre_node_label = "#node_label="; // string array delimited by the delimiter
      string const pre_directed = "#directed";
      string const pre_interval = "#interval="; // time_t
      string const pre_chartime = "#characteristic_time="; // time_t
      string const pre_threshold = "#threshold="; // double

    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    string get_body(string const& s) {
      size_t const pos0 = s.find("//");
      size_t const pos1 = s.find('\r');
      return s.substr(0, pos0<pos1?pos0:pos1);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    int get_items(Thread* parent, char const* filename, char& delimiter, wstring& title, string& nlabel, bool& directed) {

      wstring filename_w = mbcs2wcs(filename, strlen(filename));
      ifstream ifs(filename);
      string line;

      delimiter = '\0';
      title = L"";

      int number_of_lines = 0;
      while (getline(ifs, line)) {

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) return -1;
        // ----------------------------------------------------------------------------------------------------

        ++number_of_lines;
        string const body = get_body(line);
        
        if (body.find(pre_delimiter)==0 && delimiter=='\0') {
          if (body.size()<pre_delimiter.size()+1) continue;
          delimiter = body[pre_delimiter.size()];
        } else if (body.find(pre_title)==0 && title.empty()) {
          if (body.size()<pre_title.size()+1) continue;
          string const t = body.substr(pre_title.size());
          title = mbcs2wcs(t.c_str(), t.size());
        } else if (body.find(pre_node_label)==0 && nlabel.empty()) {
          if (body.size()<pre_node_label.size()+1) continue;
          nlabel = body.substr(pre_node_label.size());
        } else if (body.find(pre_directed)==0) {
          directed = true;
        }
      }

      if (delimiter=='\0') {
        MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
               L"%s [%s]", sociarium_project_message::ERROR_UNCERTAIN_DELIMITER, filename_w.c_str());
        return -1;
      }

      return number_of_lines;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    int get_items(Thread* parent, char const* filename, char& delimiter, wstring& title, string& tlabel, bool& directed,
                  time_t& interval, time_t& characteristic_time, double& threshold) {

      wstring filename_w = mbcs2wcs(filename, strlen(filename));
      ifstream ifs(filename);
      string line;

      delimiter = '\0';
      title = L"";
      tlabel = "";
      interval = -1;
      characteristic_time = 0;
      threshold = 0.0;

      int number_of_lines = 0;
      while (getline(ifs, line)) {

        // ----------------------------------------------------------------------------------------------------
        if (parent->cancel_check()) return -1;
        // ----------------------------------------------------------------------------------------------------

        ++number_of_lines;
        string const body = get_body(line);

        if (body.find(pre_delimiter)==0 && delimiter=='\0') {
          if (body.size()<pre_delimiter.size()+1) continue;
          delimiter = body[pre_delimiter.size()];
        } else if (body.find(pre_title)==0 && title.empty()) {
          if (body.size()<pre_title.size()+1) continue;
          string const t = body.substr(pre_title.size());
          title = mbcs2wcs(t.c_str(), t.size());
        } else if (body.find(pre_layer_label)==0 && tlabel.empty()) {
          if (body.size()<pre_layer_label.size()+1) continue;
          tlabel = body.substr(pre_layer_label.size());
        } else if (body.find(pre_directed)==0) {
          directed = true;
        } else if (body.find(pre_interval)==0 && interval==-1) {
          try {
            interval = boost::lexical_cast<time_t>(body.substr(pre_interval.size()));
          } catch (...) {
            MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                   L"bad interval: [%s, %d]", filename_w.c_str(), number_of_lines);
            interval = -1;
            continue;
          }
        } else if (body.find(pre_chartime)==0 && characteristic_time==0) {
          try {
            characteristic_time = boost::lexical_cast<time_t>(body.substr(pre_chartime.size()));
          } catch (...) {
            MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                   L"bad characteristic time: [%s, %d]", filename_w.c_str(), number_of_lines);
            characteristic_time = 0;
            continue;
          }
        } else if (body.find(pre_threshold)==0 && threshold==0.0) {
          try {
            threshold = boost::lexical_cast<double>(body.substr(pre_threshold.size()));
          } catch (...) {
            MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
                   L"bad threshold: [%s, %d]", filename_w.c_str(), number_of_lines);
            threshold = 0.0;
            continue;
          }
        }
      }

      if (number_of_lines==0) {
        MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
               L"%s [%s]", sociarium_project_message::ERROR_NO_LINE, filename_w.c_str());
        return -1;
      }

      if (delimiter=='\0') {
        MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
               L"%s [%s]", sociarium_project_message::ERROR_UNCERTAIN_DELIMITER, filename_w.c_str());
        return -1;
      }

      if (interval==-1) {
        MsgBox(sociarium_project_common::get_window_handle(), sociarium_project_common::APPLICATION_TITLE,
               L"%s [%s]", sociarium_project_message::ERROR_FAILED_TO_READ_DATA, filename_w.c_str());
        return -1;
      }

      return number_of_lines;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void set_properties_for_snapshot(shared_ptr<Graph> const& g,
                                     vector<vector<sociarium_project_user_defined_property::NodeProperty> >& node_property,
                                     vector<vector<sociarium_project_user_defined_property::EdgeProperty> >& edge_property,
                                     vector<string> const& node_name,
                                     vector<float> const& edge_weight) {

      size_t const nsz = g->nsize();
      size_t const esz = g->esize();

      assert(node_name.size()==nsz);
      assert(edge_weight.size()==esz);

      node_property.resize(1, vector<sociarium_project_user_defined_property::NodeProperty>(nsz));
      for (size_t i=0; i<nsz; ++i) {
        sociarium_project_user_defined_property::NodeProperty& np = node_property[0][i];
        np.name = mbcs2wcs(node_name[i].c_str(), node_name[i].size());
        np.text = L"";
        np.weight = 1.0f;
      }

      edge_property.resize(1, vector<sociarium_project_user_defined_property::EdgeProperty>(esz));
      for (size_t i=0; i<esz; ++i) {
        sociarium_project_user_defined_property::EdgeProperty& ep = edge_property[0][i];
        ep.text = L"";
        ep.weight = edge_weight[i];
      }
    }

  } // The end of the namespace "sociarium_project_module_creation_detail"
} // The end of the namespace "hashimoto_ut"
