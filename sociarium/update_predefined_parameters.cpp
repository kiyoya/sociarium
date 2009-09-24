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
#include "flag_operation.h"
#include "layout.h"
#include "texture.h"
#include "thread/force_direction.h"
#include "update_predefined_parameters.h"
#include "../shared/win32api.h"

namespace hashimoto_ut {

  using std::string;
  using std::wstring;
  using std::pair;
  using std::tr1::unordered_map;

  namespace {

    wchar_t const* DEFAULT_NODE_TEXTURE_FILE
      = L"___system_node.png";
    wchar_t const* DEFAULT_EDGE_TEXTURE_FILE
      = L"___system_node.png"; // Not used.
    wchar_t const* DEFAULT_COMMUNITY_TEXTURE_FILE
      = L"___system_community.png";
    wchar_t const* DEFAULT_COMMUNITY_EDGE_TEXTURE_FILE
      = L"___system_community.png"; // Not used.

    wstring const pre_texture_folder   = L"texture_folder";
    wstring const pre_texture_wrap_s   = L"texture_wrap_s";
    wstring const pre_texture_wrap_t   = L"texture_wrap_t";
    wstring const pre_node_texture     = L"node_texture";
    wstring const pre_edge_texture     = L"edge_texture";
    wstring const pre_community_texture      = L"community_texture";
    wstring const pre_community_edge_texture = L"community_edge_texture";

    wstring const pre_edge_width       = L"edge_width";
    wstring const pre_node_size        = L"node_size";
    wstring const pre_coordinates_size = L"coordinates_size";
    wstring const pre_grid_interval    = L"grid_interval";

    wstring const pre_force_scale      = L"force_scale";
    wstring const pre_kk_force_NN      = L"kk_force_NN";
    wstring const pre_kk_force_CC      = L"kk_force_CC";
    wstring const pre_spring_force_CN  = L"spring_force_CN";
    wstring const pre_spring_force_NN  = L"spring_force_NN";
    wstring const pre_spring_length_CN = L"spring_length_CN";
    wstring const pre_spring_length_NN = L"spring_length_NN";

  } // The end of the anonymous namespace


  ////////////////////////////////////////////////////////////////////////////////
  // TEXTURE
  void update_texture_parameters(
    unordered_map<wstring, pair<wstring, int> > const& params) {

    unordered_map<wstring, pair<wstring, int> >::const_iterator i;

    if ((i=params.find(pre_texture_folder))!=params.end()) {
      wstring path = i->second.first;
      if (!path.empty() && path[path.size()-1]!=L'\\') path += L'\\';
      sociarium_project_texture::set_texture_folder_path(path);
    } else
      sociarium_project_texture::set_texture_folder_path(L"");

    GLint wrap_s = GL_CLAMP_TO_EDGE_EXT;
    GLint wrap_t = GL_CLAMP_TO_EDGE_EXT;
    /* GL_REPEAT
     * GL_CLAMP
     * GL_CLAMP_TO_BORDER_EXT (unavailable??)
     * GL_CLAMP_TO_EDGE_EXT
     */

    if ((i=params.find(pre_texture_wrap_s))!=params.end()) {

      wstring const s = i->second.first;

      if (s==L"GL_CLAMP")
        wrap_s = GL_CLAMP;
      else if (s==L"GL_REPEAT")
        wrap_s = GL_REPEAT;
      else if (s==L"GL_CLAMP_TO_EDGE_EXT")
        wrap_s = GL_CLAMP_TO_EDGE_EXT;

      sociarium_project_texture::set_texture_parameter_wrap_s(wrap_s);
    }

    if ((i=params.find(pre_texture_wrap_t))!=params.end()) {

      wstring const s = i->second.first;

      if (s==L"GL_CLAMP")
        wrap_t = GL_CLAMP;
      else if (s==L"GL_REPEAT")
        wrap_t = GL_REPEAT;
      else if (s==L"GL_CLAMP_TO_EDGE_EXT")
        wrap_t = GL_CLAMP_TO_EDGE_EXT;

      sociarium_project_texture::set_texture_parameter_wrap_t(wrap_t);
    }

    if ((i=params.find(pre_node_texture))!=params.end()) {
      // Change default node texture.
      wstring const filename = i->second.first;
      sociarium_project_texture::set_default_node_texture_tmp(filename);
    } else {
      wstring const path_tmp
        = sociarium_project_texture::get_texture_folder_path();
      sociarium_project_texture::set_texture_folder_path(L"");
      sociarium_project_texture::set_default_node_texture_tmp(
        DEFAULT_NODE_TEXTURE_FILE);
      sociarium_project_texture::set_texture_folder_path(path_tmp);
    }

    if ((i=params.find(pre_edge_texture))!=params.end()) {
      // Change default edge texture.
      wstring const filename = i->second.first;
      sociarium_project_texture::set_default_edge_texture_tmp(filename);
    } else {
      wstring const path_tmp
        = sociarium_project_texture::get_texture_folder_path();
      sociarium_project_texture::set_texture_folder_path(L"");
      sociarium_project_texture::set_default_edge_texture_tmp(
        DEFAULT_EDGE_TEXTURE_FILE);
      sociarium_project_texture::set_texture_folder_path(path_tmp);
    }

    if ((i=params.find(pre_community_texture))!=params.end()) {
      // Change default community texture.
      wstring const filename = i->second.first;
      sociarium_project_texture::set_default_community_texture_tmp(filename);
    } else {
      wstring const path_tmp
        = sociarium_project_texture::get_texture_folder_path();
      sociarium_project_texture::set_texture_folder_path(L"");
      sociarium_project_texture::set_default_community_texture_tmp(
        DEFAULT_COMMUNITY_TEXTURE_FILE);
      sociarium_project_texture::set_texture_folder_path(path_tmp);
    }

    if ((i=params.find(pre_community_edge_texture))!=params.end()) {
      // Change default community edge texture.
      wstring const filename = i->second.first;
      sociarium_project_texture::set_default_community_edge_texture_tmp(filename);
    } else {
      wstring const path_tmp
        = sociarium_project_texture::get_texture_folder_path();
      sociarium_project_texture::set_texture_folder_path(L"");
      sociarium_project_texture::set_default_community_edge_texture_tmp(
        DEFAULT_COMMUNITY_EDGE_TEXTURE_FILE);
      sociarium_project_texture::set_texture_folder_path(path_tmp);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  // OTHERS
  void update_predefined_parameters(
    unordered_map<wstring, pair<wstring, int> > const& params) {

    unordered_map<wstring, pair<wstring, int> >::const_iterator i;


    ////////////////////////////////////////////////////////////////////////////////
    // SIZE

    if ((i=params.find(pre_edge_width))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_draw::set_edge_width(v);
      } catch (...) {}
    } else {
      sociarium_project_draw::set_edge_width(
        sociarium_project_draw::get_default_edge_width());
    }

    if ((i=params.find(pre_node_size))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_draw::set_node_size(v);
      } catch (...) {}
    } else {
      sociarium_project_draw::set_node_size(
        sociarium_project_draw::get_default_node_size());
    }

    if ((i=params.find(pre_coordinates_size))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_draw::set_coordinates_size(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_grid_interval))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_layout::set_grid_interval(v);
      } catch (...) {}
    } 


    ////////////////////////////////////////////////////////////////////////////////
    // FORCE DIRECTION

    if ((i=params.find(pre_force_scale))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_force_scale(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_kk_force_NN))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_kk_force_CC(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_kk_force_CC))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_kk_force_CC(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_spring_force_CN))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_spring_force_CN(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_spring_force_NN))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_spring_force_NN(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_spring_length_CN))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_spring_length_CN(v);
      } catch (...) {}
    } 

    if ((i=params.find(pre_spring_length_NN))!=params.end()) {
      wstring const& s = i->second.first;
      try {
        float const v = boost::lexical_cast<float>(s);
        if (v>0.0f) sociarium_project_force_direction::set_spring_length_NN(v);
      } catch (...) {}
    }

    sociarium_project_force_direction::should_be_updated();

  }

} // The end of the namespace "hashimoto_ut"
