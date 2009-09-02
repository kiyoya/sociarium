#include "graph.h"
#include "util/traverser.h"
#include "../shared/mtrand.h"
#include <iostream>
using namespace std;
using namespace hashimoto_ut;
using std::tr1::shared_ptr;

void dump_path_BFS(shared_ptr<BFSTraverser> const& t) {
  for (; !t->end(); t->advance())
    cout << "n" << t->node()->index() << "[" << t->distance() << "]" << endl;
}

void dump_path_BFSRec(shared_ptr<BFSRecordingTraverser> const& t) {
  for (; !t->end(); t->advance()) {
    cout << t->node()->index() << "[" << t->distance() << "]: ";
     for (vector<pair<Edge const*, Node const*> >::const_iterator i=t->pbegin(); i!=t->pend(); ++i)
       cout << "n" << i->second->index() << "[" << t->distance(i->second) << "] by " << "e" << i->first->index() << ", ";
    cout << endl;
  }
}

void dump_path_DFS(shared_ptr<DFSTraverser> const& t) {
  for (; !t->end(); t->advance())
    cout << "n" << t->node()->index() << "[" << t->order() << "]" << endl;
}

void dump_path_DFSRec(shared_ptr<DFSRecordingTraverser> const& t) {
  for (; !t->end(); t->advance()) {
    cout << "n" << t->node()->index() << "[" << t->order() << "]: ";
    if (t->order()>0) cout << "n" << t->predecessor().second->index() << "[" << t->order(t->predecessor().second) << "] by " << "e" << t->predecessor().first->index();
    cout << endl;
  }
}


int main(void) {

  shared_ptr<Graph> g = Graph::create();
  vector<Node*> n;
  vector<double> edge_weight;
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // テストグラフの作成
  for (size_t i=0; i<9; ++i) n.push_back(g->add_node());

  g->add_edge(n[0], n[1]); edge_weight.push_back(1.0);
  g->add_edge(n[1], n[2]); edge_weight.push_back(1.0);
  g->add_edge(n[2], n[3]); edge_weight.push_back(1.0);
  g->add_edge(n[3], n[4]); edge_weight.push_back(1.0);
  g->add_edge(n[4], n[0]); edge_weight.push_back(1.0);
  g->add_edge(n[2], n[4]); edge_weight.push_back(0.5);
  g->add_edge(n[2], n[5]); edge_weight.push_back(0.5);
  g->add_edge(n[5], n[6]); edge_weight.push_back(0.1);
  g->add_edge(n[6], n[7]); edge_weight.push_back(0.1);
  g->add_edge(n[7], n[8]); edge_weight.push_back(0.1);
  g->add_edge(n[8], n[4]); edge_weight.push_back(0.1);
  g->add_edge(n[4], n[8]); edge_weight.push_back(0.1);

  vector<Node*> closed_nodes;
  closed_nodes.push_back(n[2]);
  closed_nodes.push_back(n[6]);
  shared_ptr<ConditionalPass> cp(new AvoidSpecifiedNodes(g, closed_nodes.begin(), closed_nodes.end()));

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 幅優先探索のテスト
  shared_ptr<BFSTraverser> b0 = BFSTraverser::create<downward_tag>(g);
  shared_ptr<BFSTraverser> b1 = BFSTraverser::create<upward_tag>(g);
  shared_ptr<BFSTraverser> b2 = BFSTraverser::create<bidirectional_tag>(g);

  cout << "################################################################################" << endl;
  cout << "# 幅優先探索のテスト" << endl;
  cout << "\n# downward探索" << endl;
  b0->start(n[0], 0);
  dump_path_BFS(b0);
  cout << "\n# upward探索" << endl;
  b1->start(n[0], 0);
  dump_path_BFS(b1);
  cout << "\n# bidirectional探索" << endl;
  b2->set_condition(cp);
  b2->start(n[0], 0);
  dump_path_BFS(b2);

  shared_ptr<BFSRecordingTraverser> brec0 = BFSRecordingTraverser::create<downward_tag>(g);
  shared_ptr<BFSRecordingTraverser> brec1 = BFSRecordingTraverser::create<upward_tag>(g);
  shared_ptr<BFSRecordingTraverser> brec2 = BFSRecordingTraverser::create<bidirectional_tag>(g);

  cout << "\n################################################################################" << endl;
  cout << "# 幅優先探索（経路記録）のテスト" << endl;
  cout << "\n# downward探索" << endl;
  brec0->start(n[0], 0);
  dump_path_BFSRec(brec0);
  cout << "\n# upward探索" << endl;
  brec1->start(n[0], 0);
  dump_path_BFSRec(brec1);
  cout << "\n# bidirectional探索" << endl;
  brec2->set_condition(cp);
  brec2->start(n[0], 0);
  dump_path_BFSRec(brec2);

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // ダイクストラ幅優先探索のテスト
  shared_ptr<BFSTraverser> dk0 = DijkstraTraverser::create<downward_tag>(g, edge_weight);
  shared_ptr<BFSTraverser> dk1 = DijkstraTraverser::create<upward_tag>(g, edge_weight);
  shared_ptr<BFSTraverser> dk2 = DijkstraTraverser::create<bidirectional_tag>(g, edge_weight);

  cout << "\n################################################################################" << endl;
  cout << "# ダイクストラ幅優先探索のテスト" << endl;
  cout << "\n# downward探索" << endl;
  dk0->start(n[0], 0);
  dump_path_BFS(dk0);
  cout << "\n# upward探索" << endl;
  dk1->start(n[0], 0);
  dump_path_BFS(dk1);
  cout << "\n# bidirectional探索" << endl;
  dk2->start(n[0], 0);
  dump_path_BFS(dk2);

  shared_ptr<BFSRecordingTraverser> dkrec0 = DijkstraRecordingTraverser::create<downward_tag>(g, edge_weight);
  shared_ptr<BFSRecordingTraverser> dkrec1 = DijkstraRecordingTraverser::create<upward_tag>(g, edge_weight);
  shared_ptr<BFSRecordingTraverser> dkrec2 = DijkstraRecordingTraverser::create<bidirectional_tag>(g, edge_weight);

  cout << "\n################################################################################" << endl;
  cout << "# ダイクストラ幅優先探索（経路記録）のテスト" << endl;
  cout << "\n# downward探索" << endl;
  dkrec0->start(n[0], 0);
  dump_path_BFSRec(dkrec0);
  cout << "\n# upward探索" << endl;
  dkrec1->start(n[0], 0);
  dump_path_BFSRec(dkrec1);
  cout << "\n# bidirectional探索" << endl;
  dkrec2->start(n[0], 0);
  dump_path_BFSRec(dkrec2);

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  // 深さ優先探索のテスト
  shared_ptr<DFSTraverser> d0 = DFSTraverser::create<downward_tag>(g);
  shared_ptr<DFSTraverser> d1 = DFSTraverser::create<upward_tag>(g);
  shared_ptr<DFSTraverser> d2 = DFSTraverser::create<bidirectional_tag>(g);

  cout << "\n################################################################################" << endl;
  cout << "# 深さ優先探索のテスト" << endl;
  cout << "\n# downward探索" << endl;
  d0->start(n[0], 0);
  dump_path_DFS(d0);
  cout << "\n# upward探索" << endl;
  d1->start(n[0], 0);
  dump_path_DFS(d1);
  cout << "\n# bidirectional探索" << endl;
  d2->start(n[0], 0);
  dump_path_DFS(d2);

  shared_ptr<DFSRecordingTraverser> drec0 = DFSRecordingTraverser::create<downward_tag>(g);
  shared_ptr<DFSRecordingTraverser> drec1 = DFSRecordingTraverser::create<upward_tag>(g);
  shared_ptr<DFSRecordingTraverser> drec2 = DFSRecordingTraverser::create<bidirectional_tag>(g);

  cout << "\n################################################################################" << endl;
  cout << "# 深さ優先探索（経路記録）のテスト" << endl;
  cout << "\n# downward探索" << endl;
  drec0->start(n[0], 0);
  dump_path_DFSRec(drec0);
  cout << "\n# upward探索" << endl;
  drec1->start(n[0], 0);
  dump_path_DFSRec(drec1);
  cout << "\n# bidirectional探索" << endl;
  drec2->start(n[0], 0);
  dump_path_DFSRec(drec2);

  return 0;
}
