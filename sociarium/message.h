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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_MESSAGE_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_MESSAGE_H

#include <memory>
#include <string>
#include <utility>

namespace hashimoto_ut {

  namespace sociarium_project_message {

    const std::tr1::shared_ptr<std::pair<std::wstring, std::wstring> >& get(void);
    void create(void);

    namespace {
#define SOCIARIUM_USES_JAPANESE_DIALOGUE
#ifdef SOCIARIUM_USES_JAPANESE_DIALOGUE
      // システム関連
      wchar_t const* WELCOME = L"こんにちわ，世界!";
      wchar_t const* DO_YOU_QUIT = L"終了しますか?";
      // フォント関連
      wchar_t const* FTGL_ERROR_CHARMAP = L"FTGLエラー: CharMapのセットに失敗しました";
      wchar_t const* FTGL_ERROR_FACESIZE = L"FTGLエラー: FaceSizeのセットに失敗しました";
      wchar_t const* FTGL_ERROR_CREATE = L"FTGLエラー: フォントの生成に失敗しました";
      // テクスチャ関連
      wchar_t const* TEXTURE_ERROR_CREATE = L"エラー: テクスチャの生成に失敗しました";
      // スレッド関連
      wchar_t const* ANOTHER_THREAD_IS_RUNNING = L"別スレッドが起動しています. 終了まで待つかESCキーでキャンセル後にリトライして下さい";
      wchar_t const* DO_YOU_CANCEL_THE_RUNNING_THREAD = L"起動中のスレッドを終了しますか?";
      // DLL関連
      wchar_t const* DLL_ERROR_FUNCTION_NOT_FOUND = L"DLLエラー: モジュール内に適切な関数名を見つけられませんでした";
      wchar_t const* LOADING_MODULE = L"モジュールをロードしています";
      wchar_t const* MODULE_IS_RUNNING = L"モジュールの実行中";
      // ファイル読み込み関連
      wchar_t const* ERROR_FAILED_TO_READ_DATA = L"データの読み込みに失敗しました";
      wchar_t const* ERROR_UNSUPPORTED_DATA_FORMAT = L"エラー: サポートされていないデータ形式です";
      wchar_t const* ERROR_UNCERTAIN_DELIMITER = L"エラー: データファイルに区切り文字が指定されていません";
      wchar_t const* ERROR_NO_LINE = L"エラー: データファイルに行がありません";
      wchar_t const* ERROR_INVALID_NUMBER_OF_LINES = L"エラー: 行の数が正しくありません";
      wchar_t const* ERROR_INVALID_NUMBER_OF_ITEMS = L"エラー: 行の項目数が正しくありません";
      wchar_t const* ERROR_NO_VALID_DATA = L"エラー: 有効なデータが存在しません";
      wchar_t const* ERROR_NODE_NAME_DUPLICATION = L"エラー: 同一レイヤーに名前の重複するノードがあります";
      wchar_t const* ERROR_EDGE_NAME_DUPLICATION = L"エラー: 同一レイヤーに名前の重複するエッジがあります";
      // グラフ構築関連
      wchar_t const* READING_GRAPH_TIME_SERIES = L"グラフ時系列データを読み込んでいます";
      wchar_t const* MAKING_GRAPH_TIME_SERIES = L"グラフ時系列を作成しています";
      wchar_t const* MAKING_GRAPH_TIME_SERIES_PROPERTIES = L"グラフ時系列の属性を作成しています";
      wchar_t const* MAKING_GRAPH_SNAPSHOT = L"グラフを作成しています";
      wchar_t const* MAKING_NODE_PROPERTIES = L"ノード属性を作成しています";
      wchar_t const* MAKING_EDGE_PROPERTIES = L"エッジ属性を作成しています";
      wchar_t const* UPDATING_TIME_SERIES = L"グラフ時系列の作成が完了しました．レイヤーを更新しています";
      // レイアウト関連全般
      wchar_t const* EXTRACTING_NODES = L"ノードを抽出しています";
      wchar_t const* EXTRACTING_EDGES = L"エッジを抽出しています";
      wchar_t const* LAYOUT_ADJUSTING = L"位置をフレーム内に調整しています";
      // レイアウト関連：Kamada-Kawai Method
      wchar_t const* LAYOUT_WITH_KAMADA_KAWAI_METHOD = L"Kamada-Kawai法";
      wchar_t const* KAMADA_KAWAI_CALCULATING_SPRING_STRENGTH = L"グラフ距離からバネ係数を計算しています";
      wchar_t const* KAMADA_KAWAI_ITERATING = L"イテレーション実行中";
      wchar_t const* GRAPH_IS_UNCONNECTED = L"グラフが非連結であるためレイアウトできませんでした";
      // レイアウト関連：HDE
      wchar_t const* LAYOUT_WITH_HDE = L"High Dimensional Embedding";
      wchar_t const* HDE_CALCULATING_GRAPH_DISTANCE = L"基準点からのグラフ距離を計算しています";
      wchar_t const* HDE_CALCULATING_VARIANCE_MATRIX = L"基準点からのグラフ距離に対する分散・共分散行列を計算しています";
      wchar_t const* HDE_CALCULATING_EIGEN_VECTOR = L"分散・共分散行列の主成分を計算しています";
      wchar_t const* HDE_CALCULATING_POSITION = L"分散・共分散行列の主成分から座標を計算しています";
      // グラフ統計量関連
      wchar_t const* STATISTICS_CLOSENESS_CENTRALITY = L"距離中心性を計算しています";
      wchar_t const* STATISTICS_BETWEENNESS_CENTRALITY = L"媒介中心性を計算しています";
      wchar_t const* STATISTICS_PAGERANK = L"ページランクを計算しています";
      wchar_t const* STATISTICS_CONNECTED_COMPONENT = L"連結成分を計算しています";
      wchar_t const* STATISTICS_MEAN_SHORTEST_PATH_LENGTH = L"平均最短経路長を計算しています";
      // コミュニティ検出関連
      wchar_t const* DETECTING_COMMUNITIES = L"コミュニティ検出を行っています";
      wchar_t const* IDENTIFYING_COMMUNITIES = L"コミュニティ時系列を作成しています";
      wchar_t const* CONNECTED_COMPONENTS = L"連結成分分解";
      wchar_t const* STRONGLY_CONNECTED_COMPONENTS = L"強連結成分分解";
      wchar_t const* MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD = L"モジュラリティ最大化 [欲張り法]";
      wchar_t const* MODULARITY_MAXIMIZATION_USING_TEO_METHOD = L"モジュラリティ最大化 [τ-EO法]";
      wchar_t const* CLIQUE_PERCOLATION = L"クリークパーコレーション";
      wchar_t const* BETWEENNESS_CENTRALITY_CLUSTERING = L"媒介中心性クラスタリング";
      // グラフ編集関連
      wchar_t const* GRAPH_TIME_SERIES_IS_LOCKED = L"グラフ時系列は他の処理によってロックされています";
      wchar_t const* DO_YOU_REALLY_REMOVE_ELEMENTS = L"選択された要素を削除します。元には戻せませんがよろしいですか?";
#else
      // System
      wchar_t const* WELCOME = L"Hello, world!";
      wchar_t const* DO_YOU_QUIT = L"Do you really quit?";
      // Font
      wchar_t const* FTGL_ERROR_CHARMAP = L"FTGL error: Failed to set CharMap";
      wchar_t const* FTGL_ERROR_FACESIZE = L"FTGL error: Failed to set FaceSize";
      wchar_t const* FTGL_ERROR_CREATE = L"FTGL error: Failed to create the font";
      // Texture
      wchar_t const* TEXTURE_ERROR_CREATE = L" Error: Failed to create the texture";
      // Thread
      wchar_t const* ANOTHER_THREAD_IS_RUNNING = L"";
      wchar_t const* DO_YOU_CANCEL_THE_RUNNING_THREAD = L"Do you terminate the running thread?";
      // DLL
      wchar_t const* DLL_ERROR_FUNCTION_NOT_FOUND = L"DLL error: Couldn't find the function";
      wchar_t const* LOADING_MODULE = L"Loading the module";
      wchar_t const* MODULE_IS_RUNNING = L"Executing the module";
      // File I/O
      wchar_t const* ERROR_FAILED_TO_READ_DATA = L"Failed to read data";
      wchar_t const* ERROR_UNSUPPORTED_DATA_FORMAT = L" Error: Unsupported data format";
      wchar_t const* ERROR_UNCERTAIN_DELIMITER = L"Error: No delimiter specified in data";
      wchar_t const* ERROR_NO_LINE = L" Error: No line";
      wchar_t const* ERROR_INVALID_NUMBER_OF_ITEMS = L" Error: Invalid number of items in the line";
      wchar_t const* ERROR_NO_VALID_DATA = L" Error: No available data";
      // Creating graph
      wchar_t const* READING_GRAPH_TIME_SERIES = L"Reading data of graph time-series";
      wchar_t const* MAKING_GRAPH_TIME_SERIES = L"Constructing graph time-series";
      wchar_t const* MAKING_GRAPH_TIME_SERIES_PROPERTIES = L"Appending attributes to graph time-series";
      wchar_t const* MAKING_GRAPH_SNAPSHOT = L"Constructing graph";
      wchar_t const* MAKING_NODE_PROPERTIES = L"Creating node attributes";
      wchar_t const* MAKING_EDGE_PROPERTIES = L"Creating edge attributes";
      // Layout
      wchar_t const* EXTRACTING_NODES = L"Extracting marked nodes";
      wchar_t const* EXTRACTING_EDGES = L"Extracting marked edges";
      wchar_t const* LAYOUT_ADJUSTING = L"Centering positions";
      wchar_t const* CALCULATING_CONNECTED_COMPONENT = L"Calculating connected compoents";
      // Layout: Kamada-Kawai method
      wchar_t const* LAYOUT_WITH_KAMADA_KAWAI_METHOD = L"Kamada-Kawai method";
      wchar_t const* KAMADA_KAWAI_CALCULATING_SPRING_STRENGTH = L"";
      wchar_t const* KAMADA_KAWAI_ITERATING = L"";
      wchar_t const* GRAPH_IS_UNCONNECTED = L"";
      // Layout: HDE
      wchar_t const* LAYOUT_WITH_HDE = L"";
      wchar_t const* HDE_CALCULATING_GRAPH_DISTANCE = L"";
      wchar_t const* HDE_CALCULATING_VARIANCE_MATRIX = L"";
      wchar_t const* HDE_CALCULATING_EIGEN_VECTOR = L"";
      wchar_t const* HDE_CALCULATING_POSITION = L"";
      // Graph statistics
      wchar_t const* STATISTICS_CLOSENESS_CENTRALITY = L"";
      wchar_t const* STATISTICS_BETWEENNESS_CENTRALITY = L"";
      wchar_t const* STATISTICS_PAGERANK = L"";
      // Community detection
      wchar_t const* DETECTING_COMMUNITIES = L"";
      wchar_t const* IDENTIFYING_COMMUNITIES = L"";
      wchar_t const* CONNECTED_COMPONENTS = L"";
      wchar_t const* STRONGLY_CONNECTED_COMPONENTS = L"";
      wchar_t const* MODULARITY_MAXIMIZATION_USING_GREEDY_METHOD = L"";
      wchar_t const* MODULARITY_MAXIMIZATION_USING_TEO_METHOD = L"";
      wchar_t const* CLIQUE_PERCOLATION = L"";
      wchar_t const* BETWEENNESS_CENTRALITY_CLUSTERING = L"";
      // Graph operation
      wchar_t const* GRAPH_TIME_SERIES_IS_LOCKED = L"";
      wchar_t const* DO_YOU_REALLY_REMOVE_ELEMENTS = L"";
#endif
    } // The end of the anonymous namespace

  } // The end of the namespace "sociarium_project_message"

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_MESSAGE_H
