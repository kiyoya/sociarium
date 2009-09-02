#include "graph.h"
#include "../shared/mtrand.h"
#include <iostream>
using namespace std;
using namespace hashimoto_ut;
using std::tr1::shared_ptr;

bool recursive_find(vector<Edge*>& parallel, Node const* n0, Node const* n1, adjacency_list_iterator first, adjacency_list_iterator last) {
  adjacency_list_iterator i = n0->find(first, last, n1);
  cerr << "check: " << n0->index() << endl;
  if (i!=n0->end()) {
    parallel.push_back(*i);
    cout << (*i)->index() << ": " << (*i)->source()->index() << "<-->" << (*i)->target()->index() << endl;
    recursive_find(parallel, n0, n1, i+1, last);
    return true;
  }
  return false;
}

void dump_adjacency_list(node_iterator first, node_iterator last) {
  for (; first!=last; ++first) {
    cout << (*first)->index() << ": ";
    for (adjacency_list_iterator i=(*first)->obegin(); i!=(*first)->oend(); ++i)
      cout << "->" << (*i)->target()->index() << "[" << (*i)->index() << "], ";
    for (adjacency_list_iterator i=(*first)->ibegin(); i!=(*first)->iend(); ++i)
      cout << "<-" << (*i)->source()->index() << "[" << (*i)->index() << "], ";
    cout << endl;
  }
}

void dump_edge_list(edge_iterator first, edge_iterator last) {
  for (; first!=last; ++first) {
    cout << (*first)->index() << ": " << (*first)->source()->index() << "->" << (*first)->target()->index() << endl;
  }
}


int main(void) {

  shared_ptr<Graph> g = Graph::create();
  vector<Node*> n;
  vector<Edge*> e;
  vector<Edge*> loop;
  vector<Edge*> uparallel;
  vector<Edge*> dparallel;


  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // ノードの作成テスト
  for (size_t i=0; i<10000; ++i) n.push_back(g->add_node());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // エッジの作成テスト
  for (size_t i=0; i<100000; ++i) e.push_back(g->add_edge(g->node(size_t(g->nsize()*mt::rand())), g->node(size_t(g->nsize()*mt::rand()))));

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 隣接リストの出力テスト
  cout << "\n######################################################################" << endl;
  cout << "# 隣接リスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // エッジリストの出力テスト
  cout << "\n######################################################################" << endl;
  cout << "# エッジリスト" << endl;
  dump_edge_list(g->ebegin(), g->eend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // ループ検索のテスト
  cout << "\n######################################################################" << endl;
  cout << "# ループ検索" << endl;
  for (edge_iterator i=g->ebegin(); i!=g->eend(); ++i) {
    if ((*i)->source()==(*i)->target()) {
      loop.push_back(*i);
      cout << (*i)->index() << ": " << (*i)->source()->index() << "<-->" << (*i)->target()->index() << endl;
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // パラレル検索のテスト（find関数）
  cout << "\n######################################################################" << endl;
  cout << "# パラレル検索（有向エッジ）" << endl;
  for (node_iterator i=g->nbegin(); i!=g->nend(); ++i) {
    for (node_iterator j=g->nbegin(); j!=g->nend(); ++j) {
      adjacency_list_iterator k = (*i)->find((*i)->obegin(), (*i)->oend(), *j);
      if (k!=(*i)->oend()) {
        Edge* e = *k;
        if (recursive_find(dparallel, *i, *j, k+1, (*i)->oend())) {
          dparallel.push_back(e);
          cout << e->index() << ": " << e->source()->index() << "<-->" << e->target()->index() << endl;
        }
      }
    }
  }

  cout << "\n######################################################################" << endl;
  cout << "# パラレル検索（無向エッジ）" << endl;
  for (node_iterator i=g->nbegin(); i!=g->nend(); ++i) {
    for (node_iterator j=i+1; j!=g->nend(); ++j) {
      adjacency_list_iterator k = (*i)->find((*i)->begin(), (*i)->end(), *j);
      if (k!=(*i)->end()) {
        Edge* e = *k;
        if (recursive_find(dparallel, *i, *j, k+1, (*i)->oend())) {
          dparallel.push_back(e);
          cout << e->index() << ": " << e->source()->index() << "<-->" << e->target()->index() << endl;
        }
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // エッジ削除のテスト
  for (size_t i=0; i<g->esize(); ++i) swap(e[i], e[size_t(mt::rand()*(e.size()-i))+i]);
  for (size_t i=0, sz=size_t(0.5*g->esize()); i<sz; ++i) g->remove_edge(e[i]);
  cout << "\n######################################################################" << endl;
  cout << "# エッジ削除後のエッジリスト" << endl;
  dump_edge_list(g->ebegin(), g->eend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // ノード削除のテスト
  for (size_t i=0; i<g->nsize(); ++i) swap(n[i], n[size_t(mt::rand()*(n.size()-i))+i]);
  for (size_t i=0, sz=size_t(0.5*g->nsize()); i<sz; ++i) g->remove_node(n[i]);
  cout << "\n######################################################################" << endl;
  cout << "# ノード削除後の隣接リスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // エッジ繋ぎ換えのテスト
  for (size_t i=0, sz=size_t(0.1*g->esize()); i<sz; ++i) g->change_source(g->edge(size_t(mt::rand()*g->esize())), g->node(size_t(mt::rand()*g->nsize())));
  for (size_t i=0, sz=size_t(0.1*g->esize()); i<sz; ++i) g->change_target(g->edge(size_t(mt::rand()*g->esize())), g->node(size_t(mt::rand()*g->nsize())));
  cout << "\n######################################################################" << endl;
  cout << "# エッジ繋ぎ換え後の隣接リスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 複数のエッジ削除のテスト
  vector<Edge*> removed_edges(g->ebegin(), g->ebegin()+size_t(0.5*g->esize()));
  g->remove_edges(removed_edges.begin(), removed_edges.end());
  cout << "\n######################################################################" << endl;
  cout << "# 複数エッジ削除後の隣接リストとエッジリスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());
  cout << endl;
  dump_edge_list(g->ebegin(), g->eend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 複数のノード削除のテスト
  vector<Node*> removed_nodes(g->nbegin(), g->nbegin()+size_t(0.5*g->nsize()));
  g->remove_nodes(removed_nodes.begin(), removed_nodes.end());
  cout << "\n######################################################################" << endl;
  cout << "# 複数ノード削除後の隣接リストとエッジリスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());
  cout << endl;
  dump_edge_list(g->ebegin(), g->eend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // mergeのテスト
  shared_ptr<Graph> g2 = Graph::create();
  Node* n0 = g2->add_node();
  Node* n1 = g2->add_node();
  g2->add_edge(n0, n1);
  g2->add_edge(g2->add_node(), g2->add_node());
  g2->add_edge(g2->add_node(), g2->add_node());
  g->merge(g2);
  assert(g2->empty());
  cout << "\n######################################################################" << endl;
  cout << "# マージ後の隣接リストとエッジリスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());
  cout << endl;
  dump_edge_list(g->ebegin(), g->eend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // split_induced_subgraphのテスト
  vector<Node*> splitted_nodes(g->nbegin(), g->nbegin()+size_t(0.5*g->nsize()));
  g2 = g->split_induced_subgraph(splitted_nodes.begin(), splitted_nodes.end());
  cout << "\n######################################################################" << endl;
  cout << "# 分割後の隣接リストとエッジリスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());
  cout << endl;
  dump_edge_list(g->ebegin(), g->eend());
  cout << endl;
  dump_adjacency_list(g2->nbegin(), g2->nend());
  cout << endl;
  dump_edge_list(g2->ebegin(), g2->eend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // copy_induced_subgraphのテスト
  vector<Node*> copied_nodes(g->nbegin(), g->nbegin()+size_t(0.5*g->nsize()));
  g2 = g->copy_induced_subgraph(copied_nodes.begin(), copied_nodes.end());
  cout << "\n######################################################################" << endl;
  cout << "# コピー後の隣接リストとエッジリスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());
  cout << endl;
  dump_edge_list(g->ebegin(), g->eend());
  cout << endl;
  dump_adjacency_list(g2->nbegin(), g2->nend());
  cout << endl;
  dump_edge_list(g2->ebegin(), g2->eend());

  return 0;
}
