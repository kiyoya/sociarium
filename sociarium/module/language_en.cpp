// s.o.c.i.a.r.i.u.m: module/language_en.cpp
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

#include <cassert>
#include <string>
#include <vector>
#ifdef _MSC_VER
#include <unordered_map>
#include <windows.h>
#else
#include <tr1/unordered_map>
#endif
#include "../resource.h"
#include "../menu_and_message.h"

#ifdef _MSC_VER
BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}
#endif

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::tr1::unordered_map;

#ifdef _MSC_VER
  extern "C" __declspec(dllexport)
    void __cdecl load_menu(unordered_map<int, wstring>& menu) {
#else
  extern "C" void load_menu(unordered_map<int, wstring>& menu) {
#endif

      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_FILE]
        = L"&File";
      menu[IDM_FILE_CANCEL]
        = L"&Cancel\tESC";
      menu[IDM_FILE_QUIT]
        = L"&Quit\tESC";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_VIEW]
        = L"&View";
      menu[IDM_VIEW_SHOW_NODE]
        = L"Show node(&1)\t1";
      menu[IDM_VIEW_SHOW_EDGE]
        = L"Show edge(&2)\t2";
      menu[IDM_VIEW_SHOW_COMMUNITY]
        = L"Show community(&3)\t3";
      menu[IDM_VIEW_SHOW_COMMUNITY_EDGE]
        = L"Show community edge(&4)\t4";

      menu[IDM_VIEW_NODE_SIZE]
        = L"Node size(&N)";
      menu[IDM_VIEW_NODE_SIZE_UPDATE]
        = L"&Update";
      menu[IDM_VIEW_NODE_SIZE_CANCEL]
        = L"&Cancel";
      menu[IDM_VIEW_NODE_SIZE_UNIFORM]
        = L"Uniform(&1)";
      menu[IDM_VIEW_NODE_SIZE_WEIGHT]
        = L"Weight(&2)";
      menu[IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY]
        = L"Degree Centrality(&3)";
      menu[IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY]
        = L"Closeness Centrality(&4)";
      menu[IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY]
        = L"Betweenness Centrality(&5)";
      menu[IDM_VIEW_NODE_SIZE_PAGERANK]
        = L"PageRank(&6)";
      menu[IDM_VIEW_NODE_SIZE_POINT]
        = L"Point(&7)";

      menu[IDM_VIEW_EDGE_WIDTH]
        = L"Edge width(&E)";
      menu[IDM_VIEW_EDGE_WIDTH_UPDATE]
        = L"&Update";
      menu[IDM_VIEW_EDGE_WIDTH_CANCEL]
        = L"&Cancel";
      menu[IDM_VIEW_EDGE_WIDTH_UNIFORM]
        = L"Uniform(&1)";
      menu[IDM_VIEW_EDGE_WIDTH_WEIGHT]
        = L"Weight(&2)";
      menu[IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY]
        = L"Betweenness Centrality(&3)";

      menu[IDM_VIEW_NODE_STYLE]
        = L"Node style(&5)\tShift+Ctrl+1";
      menu[IDM_VIEW_NODE_STYLE_POLYGON]
        = L"Polygon(&1)";
      menu[IDM_VIEW_NODE_STYLE_TEXTURE]
        = L"Texture(&2)";
      menu[IDM_VIEW_EDGE_STYLE]
        = L"Edge style(&6)\tShift+Ctrl+2";
      menu[IDM_VIEW_EDGE_STYLE_LINE]
        = L"Line(&1)";
      menu[IDM_VIEW_EDGE_STYLE_POLYGON]
        = L"Polygon(&2)";
      menu[IDM_VIEW_COMMUNITY_STYLE]
        = L"Community style(&7)\tShift+Ctrl+3";
      menu[IDM_VIEW_COMMUNITY_STYLE_POLYGON_CIRCLE]
        = L"Polygon(&1)";
      menu[IDM_VIEW_COMMUNITY_STYLE_TEXTURE]
        = L"Texture(&2)";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE]
        = L"Community edge style(&8)\tShift+Ctrl+4";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE_LINE]
        = L"Line(&1)";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE_POLYGON]
        = L"Polygon(&2)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_STRING]
        = L"&String";
      menu[IDM_STRING_SHOW_NODE_NAME]
        = L"Show node name(&1)\tCtrl+1";
      menu[IDM_STRING_SHOW_EDGE_NAME]
        = L"Show edge name(&2)\tCtrl+2";
      menu[IDM_STRING_SHOW_COMMUNITY_NAME]
        = L"Show community name(&3)\tCtrl+3";
      menu[IDM_STRING_SHOW_COMMUNITY_EDGE_NAME]
        = L"Show community edge name(&4)\tCtrl+4";
      menu[IDM_STRING_NODE_NAME_SIZE]
        = L"Size of node name(&5)";
      menu[IDM_STRING_NODE_NAME_SIZE_VARIABLE]
        = L"Proportional to node size(&0)";
      menu[IDM_STRING_NODE_NAME_SIZE_0]
        = L"Tiny(&1)";
      menu[IDM_STRING_NODE_NAME_SIZE_1]
        = L"Small(&2)";
      menu[IDM_STRING_NODE_NAME_SIZE_2]
        = L"Medium(&3)";
      menu[IDM_STRING_NODE_NAME_SIZE_3]
        = L"Large(&4)";
      menu[IDM_STRING_NODE_NAME_SIZE_4]
        = L"Huge(&5)";
      menu[IDM_STRING_EDGE_NAME_SIZE]
        = L"Size of edge name(&6)";
      menu[IDM_STRING_EDGE_NAME_SIZE_VARIABLE]
        = L"Proportional to edge width(&0)";
      menu[IDM_STRING_EDGE_NAME_SIZE_0]
        = L"Tiny(&1)";
      menu[IDM_STRING_EDGE_NAME_SIZE_1]
        = L"Small(&2)";
      menu[IDM_STRING_EDGE_NAME_SIZE_2]
        = L"Medium(&3)";
      menu[IDM_STRING_EDGE_NAME_SIZE_3]
        = L"Large(&4)";
      menu[IDM_STRING_EDGE_NAME_SIZE_4]
        = L"Huge(&5)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE]
        = L"Size of community name(&7)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_VARIABLE]
        = L"Proportional to community size(&0)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_0]
        = L"Tiny(&1)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_1]
        = L"Small(&2)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_2]
        = L"Medium(&3)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_3]
        = L"Large(&4)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_4]
        = L"Huge(&5)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE]
        = L"Size of community edge name(&8)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_VARIABLE]
        = L"Proportional to community edge width(&0)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_0]
        = L"Tiny(&1)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_1]
        = L"Small(&2)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_2]
        = L"Medium(&3)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_3]
        = L"Large(&4)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_4]
        = L"Huge(&5)";
      menu[IDM_STRING_FONT_TYPE]
        = L"Font type(&T)";
      menu[IDM_STRING_FONT_TYPE_POLYGON]
        = L"Polygon(&1)";
      menu[IDM_STRING_FONT_TYPE_TEXTURE]
        = L"Texture(&2)";
      menu[IDM_STRING_SHOW_FPS]
        = L"Show FPS(&P)";
      menu[IDM_STRING_SHOW_LAYER_NAME]
        = L"Show &layer name\tL";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_EDIT]
        = L"&Edit";
      menu[IDM_EDIT_MARK_ON_CURRENT_LAYER]
        = L"Select on the current layer(&1)";
      menu[IDM_EDIT_MARK_ALL_NODES_ON_CURRENT_LAYER]
        = L"Select all nodes(&1)";
      menu[IDM_EDIT_MARK_ALL_EDGES_ON_CURRENT_LAYER]
        = L"Select all edges(&2)";
      menu[IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"Select nodes inside communities(&3)";
      menu[IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"Select edges inside communities(&4)";
      menu[IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"Select nodes+edges inside communities(&5)";
      menu[IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"Select nodes outside communities(&6)";
      menu[IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"Select edges outside communities(&7)";
      menu[IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"Select nodes+edges outside communities(&8)";
      menu[IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER]
        = L"Select nodes in selected communities(&9)";
      menu[IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER]
        = L"Select edges in selected communities(&9)";
      menu[IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_CURRENT_LAYER]
        = L"Select elements in selected communities(&9)";
      menu[IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER]
        = L"Select all elements(&A)\tCtrl+A";
      menu[IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER]
        = L"Select inverse(&I)\tCtrl+I";

      menu[IDM_EDIT_MARK_ON_EACH_LAYER]
        = L"Select on each layer(&2)";
      menu[IDM_EDIT_MARK_ALL_NODES_ON_EACH_LAYER]
        = L"Select all nodes(&1)";
      menu[IDM_EDIT_MARK_ALL_EDGES_ON_EACH_LAYER]
        = L"Select all edges(&2)";
      menu[IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"Select nodes inside communities(&3)";
      menu[IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"Select edges inside communities(&4)";
      menu[IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"Select nodes+edges inside communities(&5)";
      menu[IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"Select nodes outside communities(&6)";
      menu[IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"Select edges outside communities(&7)";
      menu[IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"Select nodes+edges outside communities(&8)";
      menu[IDM_EDIT_MARK_NODES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER]
        = L"Select nodes in selected community-continuums(&9)";
      menu[IDM_EDIT_MARK_EDGES_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER]
        = L"Select edges in selected community-continuums(&9)";
      menu[IDM_EDIT_MARK_ELEMENTS_IN_SELECTED_COMMUNITY_CONTINUUMS_ON_EACH_LAYER]
        = L"Select elements in selected community-continuums(&9)";
      menu[IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER]
        = L"Select all elements(&A)\tShift+Ctrl+A";
      menu[IDM_EDIT_INVERT_MARK_ON_EACH_LAYER]
        = L"Select inverse(&I)\tShift+Ctrl+I";

      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS]
        = L"Hide selected elements(&3)",
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER]
        = L"Hide selected nodes on the current layer(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER]
        = L"Hide selected edges on the current layer(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER]
        = L"Hide selected nodes+edges on the current layer(&A)";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER]
        = L"Hide selected nodes on each layer(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER]
        = L"Hide selected edges on each layer(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER]
        = L"Hide selected nodes+edges on each layer(&A)";

      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ALL]
        = L"Hide elements identical to selected elements(&4)";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER_ALL]
        = L"Hide elements identical to selected nodes on the current layer(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER_ALL]
        = L"Hide elements identical to selected edges on the current layer(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL]
        = L"Hide elements identical to selected nodes+edges on the current layer(&E)";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER_ALL]
        = L"Hide elements identical to selected nodes on each layer(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER_ALL]
        = L"Hide elements identical to selected edges on each layer(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL]
        = L"Hide elements identical to selected nodes+edges on each layer(&E)";

      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS]
        = L"Show hidden elements(&5)";
      menu[IDM_EDIT_SHOW_HIDDEN_NODES_ON_CURRENT_LAYER]
        = L"Show hidden nodes on the current layer(&N)";
      menu[IDM_EDIT_SHOW_HIDDEN_EDGES_ON_CURRENT_LAYER]
        = L"Show hidden edges on the current layer(&E)";
      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER]
        = L"Show hidden nodes+edges on the current layer(&A)\tCtrl+U";
      menu[IDM_EDIT_SHOW_HIDDEN_NODES_ON_EACH_LAYER]
        = L"Show hidden nodes on each layer(&N)";
      menu[IDM_EDIT_SHOW_HIDDEN_EDGES_ON_EACH_LAYER]
        = L"Show hidden edges on each layer(&E)";
      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER]
        = L"Show hidden nodes+edges on each layer(&A)\tShift+Ctrl+U";

      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS]
        = L"Remove selected elements(&6)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER]
        = L"Remove selected nodes on the current layer(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER]
        = L"Remove selected edges on the current layer(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER]
        = L"Remove selected nodes+edges on the current layer(&A)\tDel";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER]
        = L"Remove selected nodes on each layer(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER]
        = L"Remove selected edges on each layer(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER]
        = L"Remove selected nodes+edges on each layer(&A)\tCtrl+Del";

      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ALL]
        = L"Remove elements identical to selected elements(&7)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER_ALL]
        = L"Remove elements identical to selected nodes on the current layer(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER_ALL]
        = L"Remove elements identical to selected edges on the current layer(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL]
        = L"Remove elements identical to selected nodes+edges on the current layer(&A)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER_ALL]
        = L"Remove elements identical to selected nodes on each layer(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER_ALL]
        = L"Remove elements identical to selected edges on each layer(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL]
        = L"Remove elements identical to selected nodes+edges on each layer(&A)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_LAYOUT]
        = L"&Layout";
      menu[IDM_LAYOUT_UPDATE]
        = L"&Update\tSpace";
      menu[IDM_LAYOUT_CANCEL]
        = L"&Cancel(&C)";
      menu[IDM_LAYOUT_KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai method(&1)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING]
        = L"Hide Dimensional Embedding(&2)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2]
        = L"Use 1st-2nd principal components(&1)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3]
        = L"Use 1st-3rd principal components(&2)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3]
        = L"Use 2nd-3rd principal components(&3)";
      menu[IDM_LAYOUT_CIRCLE]
        = L"Circle: Initial arrangement(&3)";
      menu[IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER]
        = L"Circle: In size order(&4)";
      menu[IDM_LAYOUT_LATTICE]
        = L"Lattice(&5)";
      menu[IDM_LAYOUT_RANDOM]
        = L"Random(&6)";
      menu[IDM_LAYOUT_CARTOGRAMS]
        = L"Cartograms(&7)";
      menu[IDM_LAYOUT_FORCE_DIRECTION]
        = L"Real-time force direction(&0)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_RUN]
        = L"Run(&0)\tCtrl+Space";
      menu[IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai method(&1)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION]
        = L"Kamada-Kawai method with community separation(&2)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_COMMUNITY_ORIENTED]
        = L"Community oriented(&3)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_SPRING_AND_REPULSIVE_FORCE]
        = L"Spring and repulsion(&4)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_LATTICE_GAS_METHOD]
        = L"Lattice Gas method(&5)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_DESIGNTIDE]
        = L"DESIGNTIDE(&6)";
      menu[IDM_LAYOUT_SHOW_LAYOUT_FRAME]
        = L"Show layout &frame\tF";
      menu[IDM_LAYOUT_INITIALIZE_LAYOUT_FRAME]
        = L"&Initialize layout frame\tCtrl+Home";
      menu[IDM_LAYOUT_SHOW_GRID]
        = L"Show &grid\tG";
      menu[IDM_LAYOUT_SHOW_CENTER]
        = L"Show center and &reference point";
      menu[IDM_LAYOUT_INITIALIZE_EYEPOINT]
        = L"&Initialize eyepoint\tHome";
      menu[IDM_LAYOUT_ZOOM_IN]
        = L"Zoom in(&Z)\tPage Down";
      menu[IDM_LAYOUT_ZOOM_OUT]
        = L"Zoom out(&X)\tPage Up";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_COMMUNITY_DETECTION]
        = L"&Community";
      menu[IDM_COMMUNITY_DETECTION_UPDATE]
        = L"&Update\tEnter";
      menu[IDM_COMMUNITY_DETECTION_CANCEL]
        = L"&Cancel";
      menu[IDM_COMMUNITY_DETECTION_CONNECTED_COMPONENTS]
        = L"Connected components(&1)";
      menu[IDM_COMMUNITY_DETECTION_WEAKLY_CONNECTED_COMPONENTS]
        = L"Weakly connected components(&1)";
      menu[IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS]
        = L"Strongly connected components(&2)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION]
        = L"Modularity Maximization(&2)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD]
        = L"Greedy method(&1)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD]
        = L"tau-EO method(&2)";
      menu[IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY]
        = L"Use weighted modularity(&W)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION]
        = L"Clique percolation(&3)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3]
        = L"3-clique percolation(&1)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4]
        = L"4-clique percolation(&2)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5]
        = L"5-clique percolation(&3)";
      menu[IDM_COMMUNITY_DETECTION_OTHERS]
        = L"Others(&4)";
      menu[IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_SEPARATION]
        = L"Betweenness Centrality Separation(&1)";
      menu[IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING]
        = L"Information Flow Mapping(&2)";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM]
        = L"Show Community Transition &Diagram\tD";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_WIDER]
        = L"Wider range(&X)\tCtrl+Left";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER]
        = L"Narrower range(&Z)\tCtrl+Right";
      menu[IDM_COMMUNITY_DETECTION_CLEAR]
        = L"Clear(&0)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_TIMELINE]
        = L"&Timeline";
      menu[IDM_TIMELINE_NEXT]
        = L"&Next\tUp";
      menu[IDM_TIMELINE_PREV]
        = L"&Prev\tDown";
      menu[IDM_TIMELINE_AUTO_RUN]
        = L"&Auto-run";
      menu[IDM_TIMELINE_STOP]
        = L"Stop(&0)";
      menu[IDM_TIMELINE_FORWARD_1]
        = L"1 step/sec(&1)";
      menu[IDM_TIMELINE_FORWARD_2]
        = L"2 step/sec(&2)";
      menu[IDM_TIMELINE_FORWARD_3]
        = L"5 step/sec(&3)";
      menu[IDM_TIMELINE_FORWARD_4]
        = L"10 step/sec(&4)";
      menu[IDM_TIMELINE_BACKWARD_1]
        = L"-1 step/sec(&5)";
      menu[IDM_TIMELINE_BACKWARD_2]
        = L"-2 step/sec(&6)";
      menu[IDM_TIMELINE_BACKWARD_3]
        = L"-5 step/sec(&7)";
      menu[IDM_TIMELINE_BACKWARD_4]
        = L"-10 step/sec(&8)";
      menu[IDM_TIMELINE_SHOW_SLIDER]
        = L"&Show time slider\tS";
    }


#ifdef _MSC_VER
  extern "C" __declspec(dllexport)
    void __cdecl load_message(vector<wstring>& message) {
#else
  extern "C" void load_message(vector<wstring>& message) {
#endif

      using namespace sociarium_project_menu_and_message;

      assert(message.size()==Message::NUMBER_OF_MESSAGES);

      /////////////////////////////////////////////////////////////////////////////
      message[Message::QUIT]
        = L"Do you really quit?";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::CALCULATING_CLOSENESS_CENTRALITY]
        = L"Calculating Closeness Centrality";
      message[Message::CALCULATING_MEAN_SHORTEST_PATH_LENGTH]
        = L"Calculating Mean Shortest Path Length";
      message[Message::CALCULATING_CONNECTED_COMPONENTS]
        = L"Calculating Weakly Connected Components";
      message[Message::CALCULATING_STRONGLY_CONNECTED_COMPONENTS]
        = L"Calculating Strongly Connected Components";
      message[Message::CALCULATING_BETWEENNESS_CENTRALITY]
        = L"Calculating Betweenness Centrality";
      message[Message::CALCULATING_PAGERANK]
        = L"Calculating PageRank";
      message[Message::CALCULATING_MODULARITY]
        = L"Calculating Modularity";
      message[Message::CALCULATING_LARGEST_CLIQUES]
        = L"Calculating Largest Cliques";
      message[Message::CALCULATING_3_CLIQUE_COMMUNITIES]
        = L"Calculating 3-Clique Communities";
      message[Message::CALCULATING_DEGREE_CENTRALITY]
        = L"Calculating Degree Centrality";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GRAPH_EXTRACTOR_EXTRACTING_NODES]
        = L"Extracting nodes";
      message[Message::GRAPH_EXTRACTOR_EXTRACTING_EDGES]
        = L"Extracting edges";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GLEW_FAILED_TO_INITIALIZE]
        = L"Failed to initialize the GLEW environment";
      message[Message::GLEW_MSAA_8]
        = L"Multi-Sampling Anti-Aliasing (x8) is enabled.";
      message[Message::GLEW_MSAA_4]
        = L"Multi-Sampling Anti-Aliasing (x4) is enabled.";
      message[Message::GLEW_MSAA_2]
        = L"Multi-Sampling Anti-Aliasing (x2) is enabled.";
      message[Message::GLEW_FAILED_TO_ENABLE_MSAA]
        = L"Multi-Sampling Anti-Aliasing is unavailable";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::FAILED_TO_CREATE_TEXTURE]
        = L"Failed to create a texture";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::REMOVE_ELEMENTS]
        = L"You are trying to remove marked elements. This operation is irreversible. Are you sure?";
      message[Message::CLEAR_COMMUNITY]
        = L"You are trying to clear community information. This operation is irreversible. Are you sure?";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GRAPH_IS_LOCKED]
        = L"The graph time-series is locked by another thread.";
      message[Message::ANOTHER_THREAD_IS_RUNNING]
        = L"Another thread is running. Please wait until the running thread will finish, or retry after cancelling the running thread.";
      message[Message::CANCEL_RUNNING_THREAD]
        = L"Do you cancel the running thread?";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::READING_DATA_FILE]
        =  L"Reading data";
      message[Message::CHECKING_TEXT_ENCODING]
        = L"Checking text encoing";
      message[Message::CONVERTING_INTO_UTF16_ENCODING]
        = L"Converting text into UTF-16";
      message[Message::UNKNOWN_CHARACTER_ENCODING]
        = L"Unknown text encoding";
      message[Message::PARSING_DATA]
        = L"Parsing data";
      message[Message::MAKING_GRAPH_TIME_SERIES]
        = L"Making a graph time-series";
      message[Message::MAKING_GRAPH_SNAPSHOT]
        = L"Making a graph snapshot";
      message[Message::MAKING_GRAPH_ATTRIBUTES]
        = L"Making graph attributes";
      message[Message::MAKING_NODE_PROPERTIES]
        = L"Making node attributes";
      message[Message::MAKING_EDGE_PROPERTIES]
        = L"Making edge attributes";
      message[Message::UNSUPPORTED_DATA_FORMAT]
        = L"Unsupported file format";
      message[Message::FAILED_TO_READ_DATA]
        = L"Failed to read data";
      message[Message::UNCERTAIN_DELIMITER]
        = L"Delimiter is not specified";
      message[Message::INVALID_NUMBER_OF_ITEMS]
        = L"Invalid number of items in the line";
      message[Message::NO_VALID_DATA]
        = L"There is no valid data";
      message[Message::NODE_IDENTIFIER_DUPLICATION]
        = L"Identifier duplication detected with nodes";
      message[Message::EDGE_IDENTIFIER_DUPLICATION]
        = L"Identifier duplication detected with edges";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::LAYOUTING]
        = L"Layouting selected elements";
      message[Message::KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai Method";
      message[Message::KAMADA_KAWAI_METHOD_CALCULATING_SPRING_STRENGTH]
        = L"Kamada-Kawai Method [Calculating spring coefficients]";
      message[Message::HDE]
        = L"High Dimensional Embedding";
      message[Message::HDE_CALCULATING_GRAPH_DISTANCE]
        = L"High Dimensional Embedding [Calculating base distances]";
      message[Message::HDE_CALCULATING_MATRIX]
        = L"High Dimensional Embedding [Calculating variance-covariance matrix]";
      message[Message::HDE_CALCULATING_PRINCIPAL_COMPONENTS]
        = L"High Dimensional Embedding [Calculating the principal components]";
      message[Message::HDE_CALCULATING_POSITION]
        = L"High Dimensional Embedding [Calculating positions]";
      message[Message::CARTOGRAMS]
        = L"Cartograms";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::DETECTING_COMMUNITIES]
        = L"Detecting communities";
      message[Message::MAKING_COMMUNITY_TIME_SERIES]
        = L"Making community time-series";
      message[Message::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD]
        = L"Modularity Maximization [Greedy Method]";
      message[Message::MODULARITY_MAXIMIZATION_USING_TEO_METHOD]
        = L"Modularity Maximization [tau-EO Method]";
      message[Message::BETWEENNESS_CENTRALITY_SEPARATION]
        = L"Betweenness Centrality Clustering";
      message[Message::INFORMATION_FLOW_MAPPING]
        = L"Information Flow Mapping";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::UPDATING_NODE_SIZE]
        = L"Updating node sizes";
      message[Message::UPDATING_EDGE_WIDTH]
        = L"Updating edge widths";


      ////////////////////////////////////////////////////////////////////////////////
      message[Message::FTGL_ERROR_CHARMAP]
        = L"Failed in FTfont::CharMap";
      message[Message::FTGL_ERROR_FACESIZE]
        = L"Failed in FTfont::FaceSize";
      message[Message::FTGL_ERROR_CREATE]
        = L"Failed to create a font object";
    }

} // The end of the namespace "hashimoto_ut"
