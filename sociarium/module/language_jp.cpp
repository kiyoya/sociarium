// s.o.c.i.a.r.i.u.m: module/language_jp.cpp
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
#include <unordered_map>
#include <windows.h>
#include "../resource.h"
#include "../language.h"

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved) {
  return TRUE;
}

namespace hashimoto_ut {

  using std::vector;
  using std::wstring;
  using std::tr1::unordered_map;

  extern "C" __declspec(dllexport)
    void __cdecl load_menu(unordered_map<int, wstring>& menu) {

      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_FILE]
        = L"ファイル(&F)";
      menu[IDM_FILE_CANCEL]
        = L"キャンセル(&C)\tESC";
      menu[IDM_FILE_QUIT]
        = L"終了(&X)\tESC";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_VIEW]
        = L"描画(&V)";
      menu[IDM_VIEW_SHOW_NODE]
        = L"ノードを表示(&1)\t1";
      menu[IDM_VIEW_SHOW_EDGE]
        = L"エッジを表示(&2)\t2";
      menu[IDM_VIEW_SHOW_COMMUNITY]
        = L"コミュニティを表示(&3)\t3";
      menu[IDM_VIEW_SHOW_COMMUNITY_EDGE]
        = L"コミュニティエッジを表示(&4)\t4";

      menu[IDM_VIEW_NODE_SIZE]
        = L"ノードの大きさ(&N)";
      menu[IDM_VIEW_NODE_SIZE_UPDATE]
        = L"再計算(&0)";
      menu[IDM_VIEW_NODE_SIZE_CANCEL]
        = L"キャンセル(&C)";
      menu[IDM_VIEW_NODE_SIZE_UNIFORM]
        = L"一様(&1)";
      menu[IDM_VIEW_NODE_SIZE_WEIGHT]
        = L"重み(&2)";
      menu[IDM_VIEW_NODE_SIZE_DEGREE_CENTRALITY]
        = L"次数中心性(&3)";
      menu[IDM_VIEW_NODE_SIZE_CLOSENESS_CENTRALITY]
        = L"距離中心性(&4)";
      menu[IDM_VIEW_NODE_SIZE_BETWEENNESS_CENTRALITY]
        = L"媒介中心性(&5)";
      menu[IDM_VIEW_NODE_SIZE_PAGERANK]
        = L"ページランク(&6)";
      menu[IDM_VIEW_NODE_SIZE_POINT]
        = L"点(&7)";

      menu[IDM_VIEW_EDGE_WIDTH]
        = L"エッジの太さ(&E)";
      menu[IDM_VIEW_EDGE_WIDTH_UPDATE]
        = L"再計算(&0)";
      menu[IDM_VIEW_EDGE_WIDTH_CANCEL]
        = L"キャンセル(&C)";
      menu[IDM_VIEW_EDGE_WIDTH_UNIFORM]
        = L"一様(&1)";
      menu[IDM_VIEW_EDGE_WIDTH_WEIGHT]
        = L"重み(&2)";
      menu[IDM_VIEW_EDGE_WIDTH_BETWEENNESS_CENTRALITY]
        = L"媒介中心性(&3)";

      menu[IDM_VIEW_NODE_STYLE]
        = L"ノードの描画方法(&5)\tShift+Ctrl+1";
      menu[IDM_VIEW_NODE_STYLE_POLYGON]
        = L"ポリゴン(&1)";
      menu[IDM_VIEW_NODE_STYLE_TEXTURE]
        = L"テクスチャ(&2)";
      menu[IDM_VIEW_EDGE_STYLE]
        = L"エッジの描画方法(&6)\tShift+Ctrl+2";
      menu[IDM_VIEW_EDGE_STYLE_LINE]
        = L"ライン(&1)";
      menu[IDM_VIEW_EDGE_STYLE_POLYGON]
        = L"ポリゴン(&2)";
      menu[IDM_VIEW_COMMUNITY_STYLE]
        = L"コミュニティの描画方法(&7)\tShift+Ctrl+3";
      menu[IDM_VIEW_COMMUNITY_STYLE_POLYGON_CIRCLE]
        = L"ポリゴン(&1)";
      menu[IDM_VIEW_COMMUNITY_STYLE_TEXTURE]
        = L"テクスチャ(&2)";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE]
        = L"コミュニティエッジの描画方法(&8)\tShift+Ctrl+4";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE_LINE]
        = L"ライン(&1)";
      menu[IDM_VIEW_COMMUNITY_EDGE_STYLE_POLYGON]
        = L"ポリゴン(&2)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_STRING]
        = L"文字列(&S)";
      menu[IDM_STRING_SHOW_NODE_NAME]
        = L"ノードの名前を表示(&1)\tCtrl+1";
      menu[IDM_STRING_SHOW_EDGE_NAME]
        = L"エッジの名前を表示(&2)\tCtrl+2";
      menu[IDM_STRING_SHOW_COMMUNITY_NAME]
        = L"コミュニティの名前を表示(&3)\tCtrl+3";
      menu[IDM_STRING_SHOW_COMMUNITY_EDGE_NAME]
        = L"コミュニティエッジの名前を表示(&4)\tCtrl+4";
      menu[IDM_STRING_NODE_NAME_SIZE]
        = L"ノードの名前の大きさ(&5)";
      menu[IDM_STRING_NODE_NAME_SIZE_VARIABLE]
        = L"ノードの大きさに連動(&0)";
      menu[IDM_STRING_NODE_NAME_SIZE_0]
        = L"最小(&1)";
      menu[IDM_STRING_NODE_NAME_SIZE_1]
        = L"小(&2)";
      menu[IDM_STRING_NODE_NAME_SIZE_2]
        = L"標準(&3)";
      menu[IDM_STRING_NODE_NAME_SIZE_3]
        = L"大(&4)";
      menu[IDM_STRING_NODE_NAME_SIZE_4]
        = L"最大(&5)";
      menu[IDM_STRING_EDGE_NAME_SIZE]
        = L"エッジの名前の大きさ(&6)";
      menu[IDM_STRING_EDGE_NAME_SIZE_VARIABLE]
        = L"エッジの太さに連動(&0)";
      menu[IDM_STRING_EDGE_NAME_SIZE_0]
        = L"最小(&1)";
      menu[IDM_STRING_EDGE_NAME_SIZE_1]
        = L"小(&2)";
      menu[IDM_STRING_EDGE_NAME_SIZE_2]
        = L"標準(&3)";
      menu[IDM_STRING_EDGE_NAME_SIZE_3]
        = L"大(&4)";
      menu[IDM_STRING_EDGE_NAME_SIZE_4]
        = L"最大(&5)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE]
        = L"コミュニティの名前の大きさ(&7)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_VARIABLE]
        = L"コミュニティの大きさに連動(&0)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_0]
        = L"最小(&1)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_1]
        = L"小(&2)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_2]
        = L"標準(&3)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_3]
        = L"大(&4)";
      menu[IDM_STRING_COMMUNITY_NAME_SIZE_4]
        = L"最大(&5)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE]
        = L"コミュニティエッジの名前の大きさ(&8)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_VARIABLE]
        = L"コミュニティエッジの太さに連動(&0)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_0]
        = L"最小(&1)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_1]
        = L"小(&2)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_2]
        = L"標準(&3)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_3]
        = L"大(&4)";
      menu[IDM_STRING_COMMUNITY_EDGE_NAME_SIZE_4]
        = L"最大(&5)";
      menu[IDM_STRING_FONT_TYPE]
        = L"文字の描画方法(&R)";
      menu[IDM_STRING_FONT_TYPE_POLYGON]
        = L"ポリゴン(&1)";
      menu[IDM_STRING_FONT_TYPE_TEXTURE]
        = L"テクスチャ(&2)";
      menu[IDM_STRING_SHOW_FPS]
        = L"FPSを表示(&F)\tCtrl+F";
      menu[IDM_STRING_SHOW_LAYER_NAME]
        = L"レイヤーの名前を表示(&L)\tL";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_EDIT]
        = L"編集(&E)";
      menu[IDM_EDIT_MARK_ON_CURRENT_LAYER]
        = L"現在のレイヤーから選択(&1)";
      menu[IDM_EDIT_MARK_NODES_ON_CURRENT_LAYER]
        = L"ノードを選択(&1)";
      menu[IDM_EDIT_MARK_EDGES_ON_CURRENT_LAYER]
        = L"エッジを選択(&2)";
      menu[IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"コミュニティに所属するノードを選択(&3)";
      menu[IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"コミュニティに所属するエッジを選択(&4)";
      menu[IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"コミュニティに所属するノードとエッジを選択(&5)";
      menu[IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"コミュニティに所属しないノードを選択(&6)";
      menu[IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"コミュニティに所属しないエッジを選択(&7)";
      menu[IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_CURRENT_LAYER]
        = L"コミュニティに所属しないノードとエッジを選択(&8)";
      menu[IDM_EDIT_MARK_ALL_ELEMENTS_ON_CURRENT_LAYER]
        = L"全てを選択(&A)\tCtrl+A";
      menu[IDM_EDIT_INVERT_MARK_ON_CURRENT_LAYER]
        = L"選択を反転(&I)\tCtrl+I";

      menu[IDM_EDIT_MARK_ON_EACH_LAYER]
        = L"全てのレイヤーから選択(&2)";
      menu[IDM_EDIT_MARK_NODES_ON_EACH_LAYER]
        = L"ノードを選択(&1)";
      menu[IDM_EDIT_MARK_EDGES_ON_EACH_LAYER]
        = L"エッジを選択(&2)";
      menu[IDM_EDIT_MARK_NODES_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"コミュニティに所属するノードを選択(&3)";
      menu[IDM_EDIT_MARK_EDGES_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"コミュニティに所属するエッジを選択(&4)";
      menu[IDM_EDIT_MARK_ELEMENTS_INSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"コミュニティに所属するノードとエッジを選択(&5)";
      menu[IDM_EDIT_MARK_NODES_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"コミュニティに所属しないノードを選択(&6)";
      menu[IDM_EDIT_MARK_EDGES_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"コミュニティに所属しないエッジを選択(&7)";
      menu[IDM_EDIT_MARK_ELEMENTS_OUTSIDE_COMMUNITY_ON_EACH_LAYER]
        = L"コミュニティに所属しないノードとエッジを選択(&8)";
      menu[IDM_EDIT_MARK_ALL_ELEMENTS_ON_EACH_LAYER]
        = L"全てを選択(&A)\tShift+Ctrl+A";
      menu[IDM_EDIT_INVERT_MARK_ON_EACH_LAYER]
        = L"選択を反転(&I)\tShift+Ctrl+I";

      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS]
        = L"選択された要素を隠す(&3)",
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER]
        = L"現在のレイヤーの選択されたノードを対象(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER]
        = L"現在のレイヤーの選択されたエッジを対象(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER]
        = L"現在のレイヤーの選択されたノードとエッジを対象(&A)\tCtrl+H";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER]
        = L"全てのレイヤーの選択されたノードを対象(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER]
        = L"全てのレイヤーの選択されたエッジを対象(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER]
        = L"全てのレイヤーの選択されたノードとエッジを対象(&A)\tShift+Ctrl+H";

      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ALL]
        = L"選択された要素の同一要素を全てのレイヤーから隠す(&4)";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_CURRENT_LAYER_ALL]
        = L"現在のレイヤーの選択されたノードを対象(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_CURRENT_LAYER_ALL]
        = L"現在のレイヤーの選択されたエッジを対象(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL]
        = L"現在のレイヤーの選択されたノードとエッジを対象(&A)";
      menu[IDM_EDIT_HIDE_MARKED_NODES_ON_EACH_LAYER_ALL]
        = L"全てのレイヤーの選択されたノードを対象(&N)";
      menu[IDM_EDIT_HIDE_MARKED_EDGES_ON_EACH_LAYER_ALL]
        = L"全てのレイヤーの選択されたエッジを対象(&E)";
      menu[IDM_EDIT_HIDE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL]
        = L"全てのレイヤーの選択されたノードとエッジを対象(&A)";

      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS]
        = L"隠れた要素を表示(&5)";
      menu[IDM_EDIT_SHOW_HIDDEN_NODES_ON_CURRENT_LAYER]
        = L"現在のレイヤーの隠れたノードを対象(&N)";
      menu[IDM_EDIT_SHOW_HIDDEN_EDGES_ON_CURRENT_LAYER]
        = L"現在のレイヤーの隠れたエッジを対象(&E)";
      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_CURRENT_LAYER]
        = L"現在のレイヤーの隠れたノードとエッジを対象(&A)\tCtrl+U";
      menu[IDM_EDIT_SHOW_HIDDEN_NODES_ON_EACH_LAYER]
        = L"全てのレイヤーの隠れたノードを対象(&N)";
      menu[IDM_EDIT_SHOW_HIDDEN_EDGES_ON_EACH_LAYER]
        = L"全てのレイヤーの隠れたエッジを対象(&E)";
      menu[IDM_EDIT_SHOW_HIDDEN_ELEMENTS_ON_EACH_LAYER]
        = L"全てのレイヤーの隠れたノードとエッジを対象(&A)\tShift+Ctrl+U";

      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS]
        = L"選択された要素を削除(&6)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER]
        = L"現在のレイヤーの選択されたノードを対象(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER]
        = L"現在のレイヤーの選択されたエッジを対象(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER]
        = L"現在のレイヤーの選択されたノードとエッジを対象(&A)\tDel";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER]
        = L"全てのレイヤーの選択されたノードを対象(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER]
        = L"全てのレイヤーの選択されたエッジを対象(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER]
        = L"全てのレイヤーの選択されたノードとエッジを対象(&A)\tCtrl+Del";

      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ALL]
        = L"選択された要素の同一要素を全てのレイヤーから削除(&7)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_CURRENT_LAYER_ALL]
        = L"現在のレイヤーの選択されたノードを対象(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_CURRENT_LAYER_ALL]
        = L"現在のレイヤーの選択されたエッジを対象(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_CURRENT_LAYER_ALL]
        = L"現在のレイヤーの選択されたノードとエッジを対象(&A)";
      menu[IDM_EDIT_REMOVE_MARKED_NODES_ON_EACH_LAYER_ALL]
        = L"全てのレイヤーの選択されたノードを対象(&N)";
      menu[IDM_EDIT_REMOVE_MARKED_EDGES_ON_EACH_LAYER_ALL]
        = L"全てのレイヤーの選択されたエッジを対象(&E)";
      menu[IDM_EDIT_REMOVE_MARKED_ELEMENTS_ON_EACH_LAYER_ALL]
        = L"全てのレイヤーの選択されたノードとエッジを対象(&A)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_LAYOUT]
        = L"レイアウト(&L)";
      menu[IDM_LAYOUT_UPDATE]
        = L"再計算\tSpace";
      menu[IDM_LAYOUT_CANCEL]
        = L"キャンセル(&C)";
      menu[IDM_LAYOUT_KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai法(&1)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING]
        = L"High Dimensional Embedding(&2)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_2]
        = L"第1-2固有成分(&1)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_1_3]
        = L"第1-3固有成分(&2)";
      menu[IDM_LAYOUT_HIGH_DIMENSIONAL_EMBEDDING_2_3]
        = L"第2-3固有成分(&3)";
      menu[IDM_LAYOUT_CIRCLE]
        = L"円：初期状態(&3)";
      menu[IDM_LAYOUT_CIRCLE_IN_SIZE_ORDER]
        = L"円：大きさ順(&4)";
      menu[IDM_LAYOUT_LATTICE]
        = L"格子(&5)";
      menu[IDM_LAYOUT_RANDOM]
        = L"ランダム(&6)";
      menu[IDM_LAYOUT_FORCE_DIRECTION]
        = L"リアルタイム力学計算(&0)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_RUN]
        = L"実行(&0)\tCtrl+Space";
      menu[IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai法(&1)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION]
        = L"Kamada-Kawai法＋コミュニティ間斥力(&2)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_COMMUNITY_ORIENTED]
        = L"コミュニティ指向(&3)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_SPRING_AND_REPULSIVE_FORCE]
        = L"ばね＋斥力(&4)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_LATTICE_GAS_METHOD]
        = L"格子ガス法(&5)";
      menu[IDM_LAYOUT_FORCE_DIRECTION_DESIGNTIDE]
        = L"DESIGNTIDE(&6)";
      menu[IDM_LAYOUT_SHOW_LAYOUT_FRAME]
        = L"レイアウトフレームの表示(&F)\tF";
      menu[IDM_LAYOUT_INITIALIZE_LAYOUT_FRAME]
        = L"レイアウトフレームの初期化(&I)\tCtrl+Home";
      menu[IDM_LAYOUT_SHOW_GRID]
        = L"グリッドの表示(&G)\tG";
      menu[IDM_LAYOUT_SHOW_CENTER]
        = L"中心点と注視点の表示(&R)";
      menu[IDM_LAYOUT_INITIALIZE_EYEPOINT]
        = L"視点の初期化(&I)\tHome";
      menu[IDM_LAYOUT_ZOOM_IN]
        = L"拡大(&Z)\tPage Down";
      menu[IDM_LAYOUT_ZOOM_OUT]
        = L"縮小(&X)\tPage Up";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_COMMUNITY_DETECTION]
        = L"コミュニティ(&C)";
      menu[IDM_COMMUNITY_DETECTION_UPDATE]
        = L"再計算\tEnter";
      menu[IDM_COMMUNITY_DETECTION_CANCEL]
        = L"キャンセル(&C)";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM]
        = L"遷移図の表示(&D)\tD";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_WIDER]
        = L"より広いエリア(&X)\tCtrl+←";
      menu[IDM_COMMUNITY_TRANSITION_DIAGRAM_SCOPE_NARROWER]
        = L"より狭いエリア(&Z)\tCtrl+→";
      menu[IDM_COMMUNITY_DETECTION_CONNECTED_COMPONENTS]
        = L"連結成分分解(&1)";
      menu[IDM_COMMUNITY_DETECTION_WEAKLY_CONNECTED_COMPONENTS]
        = L"弱連結成分分解(&1)";
      menu[IDM_COMMUNITY_DETECTION_STRONGLY_CONNECTED_COMPONENTS]
        = L"強連結成分分解(&2)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION]
        = L"モジュラリティ最大化(&2)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD]
        = L"欲張り法(&1)";
      menu[IDM_COMMUNITY_DETECTION_MODULARITY_MAXIMIZATION_USING_TEO_METHOD]
        = L"τ-EO法(&2)";
      menu[IDM_COMMUNITY_DETECTION_USE_WEIGHTED_MODULARITY]
        = L"重み付きモジュラリティ(&W)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION]
        = L"クリークパーコレーション(&3)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_3]
        = L"3-クリークパーコレーション(&1)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_4]
        = L"4-クリークパーコレーション(&2)";
      menu[IDM_COMMUNITY_DETECTION_CLIQUE_PERCOLATION_5]
        = L"5-クリークパーコレーション(&3)";
      menu[IDM_COMMUNITY_DETECTION_OTHERS]
        = L"その他(&4)";
      menu[IDM_COMMUNITY_DETECTION_BETWEENNESS_CENTRALITY_CLUSTERING]
        = L"媒介中心性分割(&1)";
      menu[IDM_COMMUNITY_DETECTION_INFORMATION_FLOW_MAPPING]
        = L"Information Flow Mapping(&2)";
      menu[IDM_COMMUNITY_DETECTION_CLEAR]
        = L"初期化(&0)";


      /////////////////////////////////////////////////////////////////////////////
      menu[IDM_TIMELINE]
        = L"時系列(&T)";
      menu[IDM_TIMELINE_NEXT]
        = L"次の時刻(&1)\t↑";
      menu[IDM_TIMELINE_PREV]
        = L"前の時刻(&2)\t↓";
      menu[IDM_TIMELINE_AUTO_RUN]
        = L"自動(&0)";
      menu[IDM_TIMELINE_STOP]
        = L"停止(&0)";
      menu[IDM_TIMELINE_FORWARD_1]
        = L"1ステップ/秒(&1)";
      menu[IDM_TIMELINE_FORWARD_2]
        = L"2ステップ/秒(&2)";
      menu[IDM_TIMELINE_FORWARD_3]
        = L"5ステップ/秒(&3)";
      menu[IDM_TIMELINE_FORWARD_4]
        = L"10ステップ/秒(&4)";
      menu[IDM_TIMELINE_BACKWARD_1]
        = L"-1ステップ/秒(&5)";
      menu[IDM_TIMELINE_BACKWARD_2]
        = L"-2ステップ/秒(&6)";
      menu[IDM_TIMELINE_BACKWARD_3]
        = L"-5ステップ/秒(&7)";
      menu[IDM_TIMELINE_BACKWARD_4]
        = L"-10ステップ/秒(&8)";
      menu[IDM_TIMELINE_SHOW_SLIDER]
        = L"レイヤー切り替えスライダーの表示(&S)\tS";
    }


  extern "C" __declspec(dllexport)
    void __cdecl load_message(vector<wstring>& message) {

      using namespace sociarium_project_language;

      assert(message.size()==Message::NUMBER_OF_MESSAGES);

      /////////////////////////////////////////////////////////////////////////////
      message[Message::QUIT]
        = L"終了しますか?";
      message[Message::IS_NOT_TEXTFILE]
        = L"テキストファイルではありません";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::CALCULATING_CLOSENESS_CENTRALITY]
        = L"距離中心性を計算しています";
      message[Message::CALCULATING_MEAN_SHORTEST_PATH_LENGTH]
        = L"平均最短経路長を計算しています";
      message[Message::CALCULATING_CONNECTED_COMPONENTS]
        = L"弱連結成分を計算しています";
      message[Message::CALCULATING_STRONGLY_CONNECTED_COMPONENTS]
        = L"強連結成分を計算しています";
      message[Message::CALCULATING_BETWEENNESS_CENTRALITY]
        = L"媒介中心性を計算しています";
      message[Message::CALCULATING_PAGERANK]
        = L"ページランクを計算しています";
      message[Message::CALCULATING_MODULARITY]
        = L"モジュラリティを計算しています";
      message[Message::CALCULATING_LARGEST_CLIQUES]
        = L"最大クリークを計算しています";
      message[Message::CALCULATING_3_CLIQUE_COMMUNITIES]
        = L"3-クリークコミュニティを計算しています";
      message[Message::CALCULATING_DEGREE_CENTRALITY]
        = L"次数中心性を計算しています";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GRAPH_EXTRACTOR_EXTRACTING_NODES]
        = L"ノードを抽出しています";
      message[Message::GRAPH_EXTRACTOR_EXTRACTING_EDGES]
        = L"エッジを抽出しています";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GLEW_FAILED_TO_INITIALIZE]
        = L"GLEWの初期化に失敗しました";
      message[Message::GLEW_MSAA_8]
        = L"マルチサンプリングアンチエリアシング(x8)が有効になりました";
      message[Message::GLEW_MSAA_4]
        = L"マルチサンプリングアンチエリアシング(x4)が有効になりました";
      message[Message::GLEW_MSAA_2]
        = L"マルチサンプリングアンチエリアシング(x2)が有効になりました";
      message[Message::GLEW_FAILED_TO_ENABLE_MSAA]
        = L"マルチサンプリングアンチエリアシングが利用できません";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::FAILED_TO_CREATE_TEXTURE]
        = L"テクスチャの生成に失敗しました";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::REMOVE_ELEMENTS]
        = L"選択された要素を削除します。元には戻せませんがよろしいですか?";
      message[Message::CLEAR_COMMUNITY]
        =  L"コミュニティ情報を削除します。元には戻せませんがよろしいですか?";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::GRAPH_IS_LOCKED]
        = L"グラフ時系列は他の処理によってロックされています";
      message[Message::ANOTHER_THREAD_IS_RUNNING]
        = L"別スレッドが起動しています。終了まで待つかキャンセル後にリトライして下さい";
      message[Message::CANCEL_RUNNING_THREAD]
        = L"起動中のスレッドを終了しますか?";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::READING_DATA_FILE]
        = L"データを読み込んでいます";
      message[Message::CHECKING_TEXT_ENCODING]
        = L"文字コードを判別しています";
      message[Message::CONVERTING_INTO_SJIS_ENCODING]
        = L"文字コードをShift_JISに変換しています";
      message[Message::UNKNOWN_CHARACTER_ENCODING]
        = L"不明なテキストエンコーディングです";
      message[Message::PARSING_DATA]
        = L"データを解析しています";
      message[Message::MAKING_GRAPH_TIME_SERIES]
        = L"グラフ時系列を作成しています";
      message[Message::MAKING_GRAPH_SNAPSHOT]
        = L"グラフを作成しています";
      message[Message::MAKING_GRAPH_ATTRIBUTES]
        = L"グラフ要素の属性を作成しています";
      message[Message::MAKING_NODE_PROPERTIES]
        = L"ノードの属性を作成しています";
      message[Message::MAKING_EDGE_PROPERTIES]
        = L"エッジの属性を作成しています";
      message[Message::UNSUPPORTED_DATA_FORMAT]
        = L"サポートされていないデータ形式です";
      message[Message::FAILED_TO_READ_DATA]
        = L"データの読み込みに失敗しました";
      message[Message::UNCERTAIN_DELIMITER]
        = L"データファイルに区切り文字が指定されていません";
      message[Message::INVALID_NUMBER_OF_ITEMS]
        = L"行の項目数が正しくありません";
      message[Message::NO_VALID_DATA]
        = L"有効なデータが存在しません";
      message[Message::NODE_IDENTIFIER_DUPLICATION]
        = L"同一レイヤーに識別子の重複するノードが存在します";
      message[Message::EDGE_IDENTIFIER_DUPLICATION]
        = L"同一レイヤーに識別子の重複するエッジが存在します";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::KAMADA_KAWAI_METHOD]
        = L"Kamada-Kawai法";
      message[Message::KAMADA_KAWAI_METHOD_CALCULATING_SPRING_STRENGTH]
        = L"グラフ距離からバネ係数を計算しています";
      message[Message::KAMADA_KAWAI_METHOD_ITERATING]
        = L"イテレーション実行中";
      message[Message::HDE]
        = L"High Dimensional Embedding";
      message[Message::HDE_CALCULATING_GRAPH_DISTANCE]
        = L"基準点からのグラフ距離を計算しています";
      message[Message::HDE_CALCULATING_MATRIX]
        = L"基準点からのグラフ距離に対する分散・共分散行列を計算しています";
      message[Message::HDE_CALCULATING_PRINCIPAL_COMPONENTS]
        = L"分散・共分散行列の主成分を計算しています";
      message[Message::HDE_CALCULATING_POSITION]
        = L"分散・共分散行列の主成分から座標を計算しています";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::DETECTING_COMMUNITIES]
        = L"コミュニティ検出を行っています";
      message[Message::MAKING_COMMUNITY_TIME_SERIES]
        = L"コミュニティ時系列を作成しています";
      message[Message::MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD]
        = L"モジュラリティ最大化 [欲張り法]";
      message[Message::MODULARITY_MAXIMIZATION_USING_TEO_METHOD]
        = L"モジュラリティ最大化 [τ-EO法]";
      message[Message::BETWEENNESS_CENTRALITY_CLUSTERING]
        = L"媒介中心性クラスタリング";
      message[Message::INFORMATION_FLOW_MAPPING]
        = L"Information Flow Mapping";


      /////////////////////////////////////////////////////////////////////////////
      message[Message::UPDATING_NODE_SIZE]
        = L"ノードの大きさを更新しています";
      message[Message::UPDATING_EDGE_WIDTH]
        = L"エッジの太さを更新しています";


      ////////////////////////////////////////////////////////////////////////////////
      message[Message::FTGL_ERROR_CHARMAP]
        = L"FTFont::CharMapの呼び出しに失敗しました";
      message[Message::FTGL_ERROR_FACESIZE]
        = L"FTFont::FaceSizeの呼び出しに失敗しました";
      message[Message::FTGL_ERROR_CREATE]
        = L"Failed to create a font object";
    }

} // The end of the namespace "hashimoto_ut"
