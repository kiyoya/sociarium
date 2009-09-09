// s.o.c.i.a.r.i.u.m: update_predefined_parameters.cpp
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

#include <windows.h>
#include <boost/lexical_cast.hpp>
#include <gl/glew.h>
#include "draw.h"
#include "layout.h"
#include "texture.h"
//#include "cvframe.h"
#include "update_predefined_parameters.h"
#include "thread/force_direction.h"
#include "../shared/win32api.h"

namespace hashimoto_ut {

  using std::string;
  using std::wstring;
  using std::pair;
  using std::tr1::unordered_map;

  namespace {

    wchar_t const* INITIAL_NODE_TEXTURE_FILE
      = L"___system_node.png";
    wchar_t const* INITIAL_EDGE_TEXTURE_FILE
      = L"___system_node.png"; // Not used.
    wchar_t const* INITIAL_COMMUNITY_TEXTURE_FILE
      = L"___system_community.png";
    wchar_t const* INITIAL_COMMUNITY_EDGE_TEXTURE_FILE
      = L"___system_community.png"; // Not used.

    string const pre_texture_folder   = "texture_folder";
    string const pre_texture_wrap_s   = "texture_wrap_s";
    string const pre_texture_wrap_t   = "texture_wrap_t";
    string const pre_node_texture     = "node_texture";
    string const pre_edge_texture     = "edge_texture";
    string const pre_community_texture      = "community_texture";
    string const pre_community_edge_texture = "community_edge_texture";

    string const pre_edge_width       = "edge_width";
    string const pre_node_size        = "node_size";
    string const pre_coordinates_size = "coordinates_size";
    string const pre_grid_interval    = "grid_interval";

    string const pre_force_scale      = "force_scale";
    string const pre_kk_force_NN      = "kk_force_NN";
    string const pre_kk_force_CC      = "kk_force_CC";
    string const pre_spring_force_CN  = "spring_force_CN";
    string const pre_spring_force_NN  = "spring_force_NN";
    string const pre_spring_length_CN = "spring_length_CN";
    string const pre_spring_length_NN = "spring_length_NN";

    string const pre_cvparam = "cvparam";

  } // The end of the anonymous namespace


  ////////////////////////////////////////////////////////////////////////////////
  // TEXTURE
  void update_texture_parameters(
    unordered_map<string, pair<string, int> > const& params) {

    unordered_map<string, pair<string, int> >::const_iterator i;

    if ((i=params.find(pre_texture_folder))!=params.end()) {
      string const path = i->second.first;
      wstring path_mb = mbcs2wcs(path.c_str(), path.size());
      if (!path_mb.empty() && path_mb[path_mb.size()-1]!=L'\\')
        path_mb += L'\\';
      sociarium_project_texture::set_texture_folder_path(path_mb);
    } else {
      sociarium_project_texture::set_texture_folder_path(L"");
    }

    GLint wrap_s = GL_CLAMP_TO_EDGE_EXT;
    GLint wrap_t = GL_CLAMP_TO_EDGE_EXT;
    /* GL_REPEAT/GL_CLAMP/GL_CLAMP_TO_BORDER_EXT/GL_CLAMP_TO_EDGE_EXT
     */

    if ((i=params.find(pre_texture_wrap_s))!=params.end()) {
      string const s = i->second.first;
      if (s=="GL_CLAMP")
        wrap_s = GL_CLAMP;
      else if (s=="GL_REPEAT")
        wrap_s = GL_REPEAT;
      else if (s=="GL_CLAMP_TO_EDGE_EXT")
        wrap_s = GL_CLAMP_TO_EDGE_EXT;
      sociarium_project_texture::set_texture_parameter_wrap_s(wrap_s);
    }

    if ((i=params.find(pre_texture_wrap_t))!=params.end()) {
      string const s = i->second.first;
      if (s=="GL_CLAMP")
        wrap_t = GL_CLAMP;
      else if (s=="GL_REPEAT")
        wrap_t = GL_REPEAT;
      else if (s=="GL_CLAMP_TO_EDGE_EXT")
        wrap_t = GL_CLAMP_TO_EDGE_EXT;
      sociarium_project_texture::set_texture_parameter_wrap_t(wrap_t);
    }

    if ((i=params.find(pre_node_texture))!=params.end()) {
      // Change default node texture.
      string const s = i->second.first;
      wstring const filename = mbcs2wcs(s.c_str(), s.size());
      sociarium_project_texture::set_default_node_texture_tmp(filename);
    } else {
      wstring const path_tmp
        = sociarium_project_texture::get_texture_folder_path();
      sociarium_project_texture::set_texture_folder_path(L"");
      sociarium_project_texture::set_default_node_texture_tmp(
        INITIAL_NODE_TEXTURE_FILE);
      sociarium_project_texture::set_texture_folder_path(path_tmp);
    }

    if ((i=params.find(pre_edge_texture))!=params.end()) {
      // Change default edge texture.
      string const s = i->second.first;
      wstring const filename = mbcs2wcs(s.c_str(), s.size());
      sociarium_project_texture::set_default_edge_texture_tmp(filename);
    } else {
      wstring const path_tmp
        = sociarium_project_texture::get_texture_folder_path();
      sociarium_project_texture::set_texture_folder_path(L"");
      sociarium_project_texture::set_default_edge_texture_tmp(
        INITIAL_EDGE_TEXTURE_FILE);
      sociarium_project_texture::set_texture_folder_path(path_tmp);
    }

    if ((i=params.find(pre_community_texture))!=params.end()) {
      // Change default community texture.
      string const s = i->second.first;
      wstring const filename = mbcs2wcs(s.c_str(), s.size());
      sociarium_project_texture::set_default_community_texture_tmp(filename);
    } else {
      wstring const path_tmp
        = sociarium_project_texture::get_texture_folder_path();
      sociarium_project_texture::set_texture_folder_path(L"");
      sociarium_project_texture::set_default_community_texture_tmp(
        INITIAL_COMMUNITY_TEXTURE_FILE);
      sociarium_project_texture::set_texture_folder_path(path_tmp);
    }

    if ((i=params.find(pre_community_edge_texture))!=params.end()) {
      // Change default community edge texture.
      string const s = i->second.first;
      wstring const filename = mbcs2wcs(s.c_str(), s.size());
      sociarium_project_texture::set_default_community_edge_texture_tmp(filename);
    } else {
      wstring const path_tmp
        = sociarium_project_texture::get_texture_folder_path();
      sociarium_project_texture::set_texture_folder_path(L"");
      sociarium_project_texture::set_default_community_edge_texture_tmp(
        INITIAL_COMMUNITY_EDGE_TEXTURE_FILE);
      sociarium_project_texture::set_texture_folder_path(path_tmp);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  // OTHERS
  void update_predefined_parameters(
    unordered_map<string, pair<string, int> > const& params) {

    unordered_map<string, pair<string, int> >::const_iterator i;


    ////////////////////////////////////////////////////////////////////////////////
    // SIZE

    if ((i=params.find(pre_edge_width))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_draw::set_edge_width(v);
      } catch (...) {}
    } else {
      sociarium_project_draw::set_edge_width(
        sociarium_project_draw::get_default_edge_width());
    }

    if ((i=params.find(pre_node_size))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_draw::set_node_size(v);
      } catch (...) {}
    } else {
      sociarium_project_draw::set_node_size(
        sociarium_project_draw::get_default_node_size());
    }

    if ((i=params.find(pre_coordinates_size))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_draw::set_coordinates_size(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_grid_interval))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_layout::set_grid_interval(v);
      } catch (...) {}
    } 


    ////////////////////////////////////////////////////////////////////////////////
    // FORCE DIRECTION

    if ((i=params.find(pre_force_scale))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_force_scale(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_kk_force_NN))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_kk_force_CC(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_kk_force_CC))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_kk_force_CC(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_spring_force_CN))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_spring_force_CN(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_spring_force_NN))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_spring_force_NN(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_spring_length_CN))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_spring_length_CN(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_spring_length_NN))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_spring_length_NN(v);
      } catch (...) {}
    }

    sociarium_project_force_direction::should_be_updated();


#ifdef SOCIAIRUM_PROJECT_USES_OPENCV
    ////////////////////////////////////////////////////////////////////////////////
    // CVFRAME

    if ((i=params.find(pre_cvparam))!=params.end()) {
      string const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        sociarium_project_cvframe::set_param(v);
      } catch (...) {}
    }
#endif

  }

} // The end of the namespace "hashimoto_ut"
