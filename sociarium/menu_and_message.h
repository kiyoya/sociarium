// s.o.c.i.a.r.i.u.m: language.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_MENU_AND_MESSAGE_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_MENU_AND_MESSAGE_H

#include <cassert>
#include <vector>
#include <string>
#include <memory>

namespace hashimoto_ut {

  class Message {
  public:

    enum {
      /////////////////////////////////////////////////////////////////////////////
      QUIT = 0,

      /////////////////////////////////////////////////////////////////////////////
      CALCULATING_CLOSENESS_CENTRALITY,
      CALCULATING_MEAN_SHORTEST_PATH_LENGTH,
      CALCULATING_CONNECTED_COMPONENTS,
      CALCULATING_STRONGLY_CONNECTED_COMPONENTS,
      CALCULATING_BETWEENNESS_CENTRALITY,
      CALCULATING_PAGERANK,
      CALCULATING_MODULARITY,
      CALCULATING_LARGEST_CLIQUES,
      CALCULATING_3_CLIQUE_COMMUNITIES,
      CALCULATING_DEGREE_CENTRALITY,

      /////////////////////////////////////////////////////////////////////////////;
      GRAPH_EXTRACTOR_EXTRACTING_NODES,
      GRAPH_EXTRACTOR_EXTRACTING_EDGES,

      /////////////////////////////////////////////////////////////////////////////;
      GLEW_FAILED_TO_INITIALIZE,
      GLEW_MSAA_8,
      GLEW_MSAA_4,
      GLEW_MSAA_2,
      GLEW_FAILED_TO_ENABLE_MSAA,

      /////////////////////////////////////////////////////////////////////////////;
      FAILED_TO_CREATE_TEXTURE,

      /////////////////////////////////////////////////////////////////////////////;
      REMOVE_ELEMENTS,

      /////////////////////////////////////////////////////////////////////////////;
      CLEAR_COMMUNITY,

      /////////////////////////////////////////////////////////////////////////////;
      GRAPH_IS_LOCKED,
      ANOTHER_THREAD_IS_RUNNING,
      CANCEL_RUNNING_THREAD,

      /////////////////////////////////////////////////////////////////////////////;
      READING_DATA_FILE,
      CHECKING_TEXT_ENCODING,
      CONVERTING_INTO_UTF16_ENCODING,
      UNKNOWN_CHARACTER_ENCODING,
      PARSING_DATA,
      MAKING_GRAPH_TIME_SERIES,
      MAKING_GRAPH_SNAPSHOT,
      MAKING_GRAPH_ATTRIBUTES,
      MAKING_NODE_PROPERTIES,
      MAKING_EDGE_PROPERTIES,
      UNSUPPORTED_DATA_FORMAT,
      FAILED_TO_READ_DATA,
      UNCERTAIN_DELIMITER,
      INVALID_NUMBER_OF_ITEMS,
      NO_VALID_DATA,
      NODE_IDENTIFIER_DUPLICATION,
      EDGE_IDENTIFIER_DUPLICATION,

      /////////////////////////////////////////////////////////////////////////////;
      LAYOUTING,
      KAMADA_KAWAI_METHOD,
      KAMADA_KAWAI_METHOD_CALCULATING_SPRING_STRENGTH,
      HDE,
      HDE_CALCULATING_GRAPH_DISTANCE,
      HDE_CALCULATING_MATRIX,
      HDE_CALCULATING_PRINCIPAL_COMPONENTS,
      HDE_CALCULATING_POSITION,
      CARTOGRAMS,

      /////////////////////////////////////////////////////////////////////////////;
      DETECTING_COMMUNITIES,
      MAKING_COMMUNITY_TIME_SERIES,
      MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD,
      MODULARITY_MAXIMIZATION_USING_TEO_METHOD,
      BETWEENNESS_CENTRALITY_SEPARATION,
      INFORMATION_FLOW_MAPPING,

      /////////////////////////////////////////////////////////////////////////////;
      UPDATING_NODE_SIZE,
      UPDATING_EDGE_WIDTH,

      /////////////////////////////////////////////////////////////////////////////
      FTGL_ERROR_CHARMAP,
      FTGL_ERROR_FACESIZE,
      FTGL_ERROR_CREATE,

      /////////////////////////////////////////////////////////////////////////////
      FAILED_TO_CREATE_CAMERA_CAPTURE,
      FAILED_TO_CREATE_FILE_CAPTURE,
      FAILED_TO_LOAD_MASKING_IMAGE,

      /////////////////////////////////////////////////////////////////////////////
      NUMBER_OF_MESSAGES
    };

    Message(void) : message_(NUMBER_OF_MESSAGES) {}

    wchar_t const* get(int message_id) const {
      assert(0<=message_id && message_id<NUMBER_OF_MESSAGES);
      return message_[message_id].c_str();
    }

    void set(std::vector<std::wstring> const& message) {
      assert(message.size()==NUMBER_OF_MESSAGES);
      message_ = message;
    }

  private:
    std::vector<std::wstring> message_;
  };


  namespace sociarium_project_menu_and_message {

    void initialize(wchar_t const* filename);
    Message const& get_message_object(void);
    wchar_t const* get_message(int message_id);

  } // The end of the namespace "sociarium_project_menu_and_message"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_MENU_AND_MESSAGE_H
