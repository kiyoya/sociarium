#include "property_graph.h"
#include "../shared/mtrand.h"
#include <string>
#include <sstream>
#include <iostream>
using namespace std;
using namespace hashimoto_ut;
using std::tr1::shared_ptr;

struct EdgeProperty {
  string value;
};

typedef PropertyGraph<int, EdgeProperty> TestGraph;

void dump_adjacency_list(shared_ptr<TestGraph> const& g, node_iterator first, node_iterator last) {
  for (; first!=last; ++first) {
    cout << *g->property(*first) << "[" << (*first)->index() << "]: ";
    for (adjacency_list_iterator i=(*first)->obegin(); i!=(*first)->oend(); ++i)
      cout << "->" << *g->property((*i)->target()) << "[" << (*i)->target()->index() << "] by " << g->property(*i)->value << "[" << (*i)->index() << "], ";
    for (adjacency_list_iterator i=(*first)->ibegin(); i!=(*first)->iend(); ++i)
      cout << "<-" << *g->property((*i)->source()) << "[" << (*i)->source()->index() << "] by " << g->property(*i)->value << "[" << (*i)->index() << "], ";
    cout << endl;
  }
}

void dump_edge_list(shared_ptr<TestGraph> const& g, edge_iterator first, edge_iterator last) {
  for (; first!=last; ++first) {
    cout << g->property(*first)->value << "[" << (*first)->index() << "]: " << *g->property((*first)->source()) << "->" << *g->property((*first)->target()) << endl;
  }
}

int main(void) {

  shared_ptr<TestGraph> g = TestGraph::create();

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // ノードの作成テスト
  for (size_t i=0; i<1000; ++i) {
    Node* n = g->add_node();
    *g->property(n) = int(i);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // エッジの作成テスト
  for (size_t i=0; i<10000; ++i) {
    Node* n0 = g->node(size_t(g->nsize()*mt::rand()));
    Node* n1 = g->node(size_t(g->nsize()*mt::rand()));
    Edge* e = g->add_edge(n0, n1);
    stringstream s;
    s << n0->index() << "<->" << n1->index();
    g->property(e)->value = s.str();
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 隣接リストの出力テスト
  cout << "\n######################################################################" << endl;
  cout << "# 隣接リスト" << endl;
  dump_adjacency_list(g, g->nbegin(), g->nend());
  cout << "\n######################################################################" << endl;
  cout << "# エッジリスト" << endl;
  dump_edge_list(g, g->ebegin(), g->eend());
  
//   ////////////////////////////////////////////////////////////////////////////////////////////////////
//   // エッジ削除のテスト
//   for (size_t i=0; i<g->esize(); ++i) swap(e[i], e[size_t(mt::rand()*(e.size()-i))+i]);
//   for (size_t i=0, sz=size_t(0.5*g->esize()); i<sz; ++i) g->remove_edge(e[i]);
//   cout << "\n######################################################################" << endl;
//   cout << "# エッジ削除後のエッジリスト" << endl;
//   dump_edge_list(g->ebegin(), g->eend());
//
//   ////////////////////////////////////////////////////////////////////////////////////////////////////
//   // ノード削除のテスト
//   for (size_t i=0; i<g->nsize(); ++i) swap(n[i], n[size_t(mt::rand()*(n.size()-i))+i]);
//   for (size_t i=0, sz=size_t(0.5*g->nsize()); i<sz; ++i) g->remove_node(n[i]);
//   cout << "\n######################################################################" << endl;
//   cout << "# ノード削除後の隣接リスト" << endl;
//   dump_adjacency_list(g->nbegin(), g->nend());
//
//   ////////////////////////////////////////////////////////////////////////////////////////////////////
//   // エッジ繋ぎ換えのテスト
//   for (size_t i=0, sz=size_t(0.1*g->esize()); i<sz; ++i) g->change_source(g->edge(size_t(mt::rand()*g->esize())), g->node(size_t(mt::rand()*g->nsize())));
//   for (size_t i=0, sz=size_t(0.1*g->esize()); i<sz; ++i) g->change_target(g->edge(size_t(mt::rand()*g->esize())), g->node(size_t(mt::rand()*g->nsize())));
//   cout << "\n######################################################################" << endl;
//   cout << "# エッジ繋ぎ換え後の隣接リスト" << endl;
//   dump_adjacency_list(g->nbegin(), g->nend());
//
//   ////////////////////////////////////////////////////////////////////////////////////////////////////
//   // 複数のエッジ削除のテスト
//   vector<Edge*> removed_edges(g->ebegin(), g->ebegin()+size_t(0.5*g->esize()));
//   g->remove_edges(removed_edges.begin(), removed_edges.end());
//   cout << "\n######################################################################" << endl;
//   cout << "# 複数エッジ削除後の隣接リストとエッジリスト" << endl;
//   dump_adjacency_list(g->nbegin(), g->nend());
//   cout << endl;
//   dump_edge_list(g->ebegin(), g->eend());
//
//   ////////////////////////////////////////////////////////////////////////////////////////////////////
//   // 複数のノード削除のテスト
//   vector<Node*> removed_nodes(g->nbegin(), g->nbegin()+size_t(0.5*g->nsize()));
//   g->remove_nodes(removed_nodes.begin(), removed_nodes.end());
//   cout << "\n######################################################################" << endl;
//   cout << "# 複数ノード削除後の隣接リストとエッジリスト" << endl;
//   dump_adjacency_list(g->nbegin(), g->nend());
//   cout << endl;
//   dump_edge_list(g->ebegin(), g->eend());
//
//   ////////////////////////////////////////////////////////////////////////////////////////////////////
//   // mergeのテスト
//   shared_ptr<Graph> g2 = Graph::create();
//   Node* n0 = g2->add_node();
//   Node* n1 = g2->add_node();
//   g2->add_edge(n0, n1);
//   g2->add_edge(g2->add_node(), g2->add_node());
//   g2->add_edge(g2->add_node(), g2->add_node());
//   g->merge(g2);
//   assert(g2->empty());
//   cout << "\n######################################################################" << endl;
//   cout << "# マージ後の隣接リストとエッジリスト" << endl;
//   dump_adjacency_list(g->nbegin(), g->nend());
//   cout << endl;
//   dump_edge_list(g->ebegin(), g->eend());
//
//   ////////////////////////////////////////////////////////////////////////////////////////////////////
//   // split_induced_subgraphのテスト
//   vector<Node*> splitted_nodes(g->nbegin(), g->nbegin()+size_t(0.5*g->nsize()));
//   g2 = g->split_induced_subgraph(splitted_nodes.begin(), splitted_nodes.end());
//   cout << "\n######################################################################" << endl;
//   cout << "# 分割後の隣接リストとエッジリスト" << endl;
//   dump_adjacency_list(g->nbegin(), g->nend());
//   cout << endl;
//   dump_edge_list(g->ebegin(), g->eend());
//   cout << endl;
//   dump_adjacency_list(g2->nbegin(), g2->nend());
//   cout << endl;
//   dump_edge_list(g2->ebegin(), g2->eend());
//
//   ////////////////////////////////////////////////////////////////////////////////////////////////////
//   // copy_induced_subgraphのテスト
//   vector<Node*> copied_nodes(g->nbegin(), g->nbegin()+size_t(0.5*g->nsize()));
//   g2 = g->copy_induced_subgraph(copied_nodes.begin(), copied_nodes.end());
//   cout << "\n######################################################################" << endl;
//   cout << "# コピー後の隣接リストとエッジリスト" << endl;
//   dump_adjacency_list(g->nbegin(), g->nend());
//   cout << endl;
//   dump_edge_list(g->ebegin(), g->eend());
//   cout << endl;
//   dump_adjacency_list(g2->nbegin(), g2->nend());
//   cout << endl;
//   dump_edge_list(g2->ebegin(), g2->eend());

  return 0;
}
