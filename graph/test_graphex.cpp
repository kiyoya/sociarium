#include "graphex.h"
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

  g->add_node();
  g->add_node();
  g->add_node();
  g->add_edge(g->node(0), g->node(1));
  g->add_edge(g->node(1), g->node(2));
  g->add_edge(g->node(2), g->node(0));

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // copy
  shared_ptr<Graph> g2 = copy(g);
  cout << "\n######################################################################" << endl;
  cout << "# copyのテスト" << endl;
  dump_adjacency_list(g2->nbegin(), g2->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // copy
  shared_ptr<Graph> g3(Graph::create());
  copy(g, g3);
  cout << "\n######################################################################" << endl;
  cout << "# copyのテスト" << endl;
  dump_adjacency_list(g3->nbegin(), g3->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // remove_loop_edges
  g->add_edge(g->node(0), g->node(0));
  g->add_edge(g->node(1), g->node(1));
  g->add_edge(g->node(2), g->node(2));
  remove_loop_edges(g, g->node(2));
  cout << "\n######################################################################" << endl;
  cout << "# remove_loop_edgesのテスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // remove_loop_edges
  remove_loop_edges(g);
  cout << "\n######################################################################" << endl;
  cout << "# remove_loop_edgesのテスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // remove_parallel_edges
  Edge* e = g->add_edge(g->node(0), g->node(1));
  g->add_edge(g->node(0), g->node(1));
  g->add_edge(g->node(1), g->node(0));
  remove_parallel_edges(g, e);
  cout << "\n######################################################################" << endl;
  cout << "# remove_parallel_edgesのテスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // contract
  g2 = copy(g);
  contract(g2, g2->node(1), g2->node(2));
  cout << "\n######################################################################" << endl;
  cout << "# contractのテスト" << endl;
  dump_adjacency_list(g2->nbegin(), g2->nend());

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // contract
  contract(g, g->edge(0));
  cout << "\n######################################################################" << endl;
  cout << "# contractのテスト" << endl;
  dump_adjacency_list(g->nbegin(), g->nend());

  return 0;
}
