// s.o.c.i.a.r.i.u.m: thread/force_direction.cpp
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
#ifdef _MSC_VER
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include "force_direction.h"
#include "../algorithm_selector.h"
#include "../layout.h"
#include "../flag_operation.h"
#include "../thread.h"
#include "../sociarium_graph_time_series.h"
#include "../../shared/fps.h"
#include "../../shared/math.h"
#include "../../shared/mtrand.h"
#include "../../graph/util/traverser.h"

namespace hashimoto_ut {

  using std::vector;
  using std::pair;
  using std::tr1::shared_ptr;
  using std::tr1::weak_ptr;
  using std::tr1::dynamic_pointer_cast;
  using std::tr1::unordered_map;

  typedef SociariumGraph::node_property_iterator node_property_iterator;
  typedef SociariumGraph::edge_property_iterator edge_property_iterator;


  namespace {

    ////////////////////////////////////////////////////////////////////////////////
    FPSKeeper fps_keeper(200);
    bool update = true;
    bool thread_is_active = false;

    struct Momentum {
      Vector2<float> momentum;
      float mass;
    };

    unordered_map<DynamicNodeProperty const*, Momentum> momentum_list;

    float force_scale = 0.3f;
    float default_velocity_decay_rate = 0.9f;


    ////////////////////////////////////////////////////////////////////////////////
    float gauss_df(float t, float u) {
      double ran;
      do ran = mt::rand();
      while (ran==0.0);
      return float(sqrt(-2.0*t*log(ran))*sin(M_2PI*mt::rand())+u);
    }

    float gamma_df(float t) {
      double ran;
      do ran = mt::rand();
      while(ran==0.0);
      return float(sqrt(-2.0*t*log(ran)));
    }


    ////////////////////////////////////////////////////////////////////////////////
    namespace KamadaKawai {

      float force_NN = 0.0005f;
      float force_CC = 0.005f;

      struct Element {
        DynamicNodeProperty const* dnp0;
        DynamicNodeProperty const* dnp1;
        float distance;
        Momentum* p0;
        Momentum* p1;
      };

      vector<shared_ptr<Element> > element;

      // --------------------------------------------------------------------------------
      void update_force(float cf, float scale_of_distance) {

        // Scale the base length.
        scale_of_distance *= force_scale*sociarium_project_layout::get_layout_frame_size();

        // Limit the max force.
        float const force_max = 300.0f;

        vector<shared_ptr<Element> >::const_iterator i   = element.begin();
        vector<shared_ptr<Element> >::const_iterator end = element.end();

        for (; i!=end; ++i) {

          DynamicNodeProperty const* dnp0 = (*i)->dnp0;
          DynamicNodeProperty const* dnp1 = (*i)->dnp1;

          Vector2<float> const& pos0 = dnp0->get_static_property()->get_position();
          Vector2<float> const& pos1 = dnp1->get_static_property()->get_position();

          RadiusVector2<float> rv(pos0, pos1);

          // Convert the geodesic distance to the Euclidean equilibrium distance.
          float const dst0 = scale_of_distance*(*i)->distance;

          // The actual Euclidean distance.
          float const dst1 = float(rv.norm)-0.5f*(dnp0->get_size()+dnp1->get_size());

          // Difference from the equilibrium distance.
          float const diff = dst1-dst0;
          float const diff2 = diff*diff;

          // Force.
          float const dd = diff2/((*i)->distance*(*i)->distance);

          Vector2<float> const force = cf*(dd>force_max?force_max:dd)*rv.u;
          if (diff>0.0f) {
            (*i)->p0->momentum += force;
            (*i)->p1->momentum -= force;
          } else {
            (*i)->p0->momentum -= force;
            (*i)->p1->momentum += force;
          }
        }
      }

      // --------------------------------------------------------------------------------
      void initialize(shared_ptr<SociariumGraph> g, bool community_separation) {
        element.clear();
        momentum_list.clear();
        shared_ptr<CircumventHiddenElements> cond(new CircumventHiddenElements(g));
        vector<double> edge_weight(g->esize(), 0.0);
        shared_ptr<BFSTraverser> t;

        if (community_separation) {

          edge_property_iterator i   = g->edge_property_begin();
          edge_property_iterator end = g->edge_property_end();

          for (; i!=end; ++i) {
            Edge const* e = i->first;
            DynamicEdgeProperty const& dep = i->second;
            if (dep.number_of_upper_nodes()==0)
              edge_weight[e->index()] = 1.5;
            else
              edge_weight[e->index()] = 0.75;
          }

          t = DijkstraTraverser::create<bidirectional_tag>(g, edge_weight);
        } else
          t = BFSTraverser::create<bidirectional_tag>(g);

        t->set_condition(cond);

        node_property_iterator i   = g->node_property_begin();
        node_property_iterator end = g->node_property_end();

        for (; i!=end; ++i) {

          Node const* n = i->first;
          DynamicNodeProperty const& dnp = i->second;

          if (is_hidden(dnp))
            continue;

          float& m = momentum_list[&dnp].mass = 1.0f;

          for (adjacency_list_iterator j=n->begin(); j!=n->end(); ++j)
            if (is_visible(g->property(*j)))
              ++m;

          for (t->reset(), t->start(n, 0.0); !t->end(); t->advance()) {

            if (t->node()<=n) continue;

            shared_ptr<Element> fde(new Element);
            fde->dnp0 = &dnp;
            fde->dnp1 = &g->property(t->node());
            fde->distance
              = float(t->distance())-(n->degree()==1||t->node()->degree()==1?0.7f:0.0f);
            fde->p0 = &momentum_list[fde->dnp0];
            fde->p1 = &momentum_list[fde->dnp1];

            try {
              element.push_back(fde);
            } catch (...) {
              element.clear();
              return;
            }
          }
        }
      }

    } // The end of the namespace "KamadaKawai"


    ////////////////////////////////////////////////////////////////////////////////
    namespace Spring {

      float force_CN = 0.1f;
      float force_NN = 0.05f;

      float length_CN = 0.015f;
      float length_NN = 0.5f;

      struct Element {
        DynamicNodeProperty const* dnp0;
        DynamicNodeProperty const* dnp1;
        float length;
        float coefficient;
        Momentum* p0;
        Momentum* p1;
      };

      vector<shared_ptr<Element> > element;

      // --------------------------------------------------------------------------------
      void update_force(float scale_of_distance) {

        // Scale the base length.
        scale_of_distance *= force_scale*sociarium_project_layout::get_layout_frame_size();

        // Limit the max force.
        float const force_max = 300.0f;

        vector<shared_ptr<Element> >::const_iterator i   = element.begin();
        vector<shared_ptr<Element> >::const_iterator end = element.end();

        for (; i!=end; ++i) {
          DynamicNodeProperty const* dnp0 = (*i)->dnp0;
          DynamicNodeProperty const* dnp1 = (*i)->dnp1;

          Vector2<float> const& pos0 = dnp0->get_static_property()->get_position();
          Vector2<float> const& pos1 = dnp1->get_static_property()->get_position();

          RadiusVector2<float> rv(pos0, pos1);

          // Convert the geodesic distance to the Euclidean equilibrium distance.
          float const dst0 = scale_of_distance*(*i)->length;

          // The actual Euclidean distance.
          float const dst1 = float(rv.norm)-0.5f*(dnp0->get_size()+dnp1->get_size());

          // Difference from the equilibrium distance.
          float const diff = dst1-dst0;
          float const abs_diff = fabs(diff);

          // Force.
          Vector2<float> const force
            = (*i)->coefficient*(abs_diff>force_max?force_max:abs_diff)*rv.u;

          if (diff>0.0) {
            (*i)->p0->momentum += force;
            (*i)->p1->momentum -= force;
          } else {
            (*i)->p0->momentum -= force;
            (*i)->p1->momentum += force;
          }
        }
      }

    } // The end of the namespace "Spring"


    ////////////////////////////////////////////////////////////////////////////////
    namespace SpringAndRepulsiveForce {

      float const spring_force = 0.01f;
      float const repulsive_force = 0.05f;
      float const repulsive_distance_max = 10.0f;

      struct Element {
        DynamicNodeProperty const* dnp0;
        DynamicNodeProperty const* dnp1;
        float length;
        Momentum* p0;
        Momentum* p1;
      };

      vector<shared_ptr<Element> > element;

      // --------------------------------------------------------------------------------
      void update_force(void) {

        using namespace sociarium_project_layout;

        float const spring_length = get_layout_frame_size()/15.0f;
        float const repulsive_distance = get_layout_frame_size()/4.0f;
        float const repulsive_distance_max = get_layout_frame_size()/4.0f;
        float const diff_max = 5.0f*spring_length;

        vector<shared_ptr<Element> >::const_iterator i   = element.begin();
        vector<shared_ptr<Element> >::const_iterator end = element.end();

        for (; i!=end; ++i) {

          DynamicNodeProperty const* dnp0 = (*i)->dnp0;
          DynamicNodeProperty const* dnp1 = (*i)->dnp1;

          Vector2<float> const& pos0 = dnp0->get_static_property()->get_position();
          Vector2<float> const& pos1 = dnp1->get_static_property()->get_position();

          RadiusVector2<float> rv(pos0, pos1);

          // The Euclidean distance.
          float const dst = fabs(float(rv.norm)-0.5f*(dnp0->get_size()+dnp1->get_size()));

          if ((*i)->length!=0.0f) {
            // Spring force works.
            float const diff = dst-spring_length*(*i)->length;
            Vector2<float> const force = spring_force*(diff<diff_max?diff:diff_max)*rv.u;
            (*i)->p0->momentum += force;
            (*i)->p1->momentum -= force;
          } else {
            // Repulsive force works.
            if (dst>repulsive_distance_max) continue;
            float const d = expf(-dst/repulsive_distance);
            Vector2<float> const force
              = -repulsive_force*(d<repulsive_distance_max?d:repulsive_distance_max)*rv.u;
            (*i)->p0->momentum += force;
            (*i)->p1->momentum -= force;
          }
        }
      }

      // --------------------------------------------------------------------------------
      void initialize(shared_ptr<SociariumGraph> g) {
        element.clear();
        momentum_list.clear();

        node_property_iterator i   = g->node_property_begin();
        node_property_iterator end = g->node_property_end();

        for (; i!=end; ++i) {

          Node const* n0 = i->first;
          DynamicNodeProperty const& dnp0 = i->second;

          if (is_hidden(dnp0)) continue;

          float& m = momentum_list[&dnp0].mass = 1.0f;

          for (adjacency_list_iterator j=n0->begin(); j!=n0->end(); ++j)
            if (is_visible(g->property(*j)))
              ++m;

          node_property_iterator j = i;

          for (++j; j!=g->node_property_end(); ++j) {

            Node const* n1 = j->first;
            DynamicNodeProperty const& dnp1 = j->second;

            if (is_hidden(dnp1)) continue;

            adjacency_list_iterator k = n0->find(n0->begin(), n0->end(), n1);

            shared_ptr<Element> fde(new Element);
            fde->dnp0 = &dnp0;
            fde->dnp1 = &dnp1;
            fde->p0 = &momentum_list[fde->dnp0];
            fde->p1 = &momentum_list[fde->dnp1];

            if (k!=n0->end() && is_visible(g->property(*k)))
              fde->length = (n0->degree()==1||n1->degree()==1)?0.5f:2.0f;
            else
              fde->length = 0.0f;

            try {
              element.push_back(fde);
            } catch (...) {
              element.clear();
              return;
            }
          }
        }
      }

    } // The end of the namespace "SpringAndRepulsiveForce"


    ////////////////////////////////////////////////////////////////////////////////
    namespace LatticeGasMethod {

      float const temperature = 0.1f;
      size_t number_of_colors = 1;
      int number_of_cell_x = 0;
      int number_of_cell_y = 0;
      Vector2<float> position_min;
      Vector2<float> position_max;
      float grid_interval;

      struct Cell;

      // --------------------------------------------------------------------------------
      struct Particle {
        DynamicNodeProperty const* dnp;
        Momentum p;
        Cell* cell;
      };

      // --------------------------------------------------------------------------------
      struct Cell {
        vector<Particle*> particles;
        vector<Cell*> neighbors;
        Momentum p;
        vector<int> number_of_nodes;
        float rot[2];

        void update_local_parameters(void) {
          p.mass = 0.0f;
          p.momentum.set(0.0f, 0.0f);
          vector<int>(number_of_colors, 0).swap(number_of_nodes);

          // Update the mean velocity of particles in the cell.
          if (particles.empty()) return;

          {
            vector<Particle*>::const_iterator i   = particles.begin();
            vector<Particle*>::const_iterator end = particles.end();

            for (; i!=end; ++i) {
              p.mass += (*i)->p.mass;
              p.momentum += (*i)->p.momentum;
            }
          }

          {
            // Update the number of particles in the cell.
            vector<Particle*>::const_iterator i   = particles.begin();
            vector<Particle*>::const_iterator end = particles.end();

            for (; i!=end ; ++i) {

              vector<DynamicNodeProperty*>::const_iterator j    = (*i)->dnp->upper_nbegin();
              vector<DynamicNodeProperty*>::const_iterator jend = (*i)->dnp->upper_nend();;

              for (; j!=jend; ++j) {
                size_t const community_index = (*j)->get_graph_element()->index();
                assert(community_index<number_of_colors);
                ++number_of_nodes[community_index];
              }
            }
          }
        }

        void update_particle_velocity(void) {

          // Update the collision matrix.
          if (number_of_colors<2 || neighbors[0]==this) {
            if (mt::rand()>0.5) {
              rot[0] = 0.0f;
              rot[1] = 1.0f;
            } else {
              rot[0] = 0.0f;
              rot[1] = -1.0f;
            }
          } else {
            vector<Vector2<float> > flux(number_of_colors);
            vector<Vector2<float> > field(number_of_colors);

            { // Update the color flux in the cell.
              vector<Particle*>::const_iterator i   = particles.begin();
              vector<Particle*>::const_iterator end = particles.end();

              for (; i!=end; ++i) {

                vector<DynamicNodeProperty*>::const_iterator j    = (*i)->dnp->upper_nbegin();
                vector<DynamicNodeProperty*>::const_iterator jend = (*i)->dnp->upper_nend();

                for (; j!=jend; ++j) {
                  size_t const community_index = (*j)->get_graph_element()->index();
                  assert(community_index<number_of_colors);
                  flux[community_index] += (*i)->p.momentum/(*i)->p.mass;
                }
              }
            }

            { // Update the color field around the cell.
              for (size_t i=0; i<number_of_colors; ++i) {
                field[i].x
                  = float(neighbors[1]->number_of_nodes[i]
                          -neighbors[2]->number_of_nodes[i])
                    + 0.5f*(neighbors[5]->number_of_nodes[i]
                            -neighbors[7]->number_of_nodes[i]
                            +neighbors[6]->number_of_nodes[i]
                            -neighbors[8]->number_of_nodes[i]);
                field[i].y
                  = float(neighbors[3]->number_of_nodes[i]
                          -neighbors[4]->number_of_nodes[i])
                    + 0.5f*(neighbors[5]->number_of_nodes[i]
                            -neighbors[6]->number_of_nodes[i]
                            +neighbors[7]->number_of_nodes[i]
                            -neighbors[8]->number_of_nodes[i]);
              }
            }

            double A = 0.0;
            double B = 0.0;

            // Calculate the collision angle that conforms the color fluxt to the color field.
            //   w = A*cos(theta)+B*sin(theta)
            //     = sqrt(A*A+B*B)*sin(theta+arctan(A/B))
            //   theta = (3/2)pi-arctan(A/B) : w==min(w)

            for (size_t i=0; i<number_of_colors; ++i) {

              Vector2<float> const& fdi = field[i];
              Vector2<float> const& fxi = flux[i];

              for (size_t j=i+1; j<number_of_colors; ++j) {
                Vector2<float> const& fdj = field[j];
                Vector2<float> const& fxj = flux[j];
                A += fxi*fdj+fxj*fdi;
                B += fxi%fdj+fxj%fdi;
              }
            }

            if (A==0.0 && B==0.0) {
              if (mt::rand()>0.5) {
                rot[0] = 0.0f;
                rot[1] = 1.0f;
              } else {
                rot[0] = 0.0f;
                rot[1] = -1.0f;
              }
            } else {
              float const theta = float(M_3PI2)-atan2f(float(A), float(B));
              rot[0] = cosf(theta);
              rot[1] = sinf(theta);
            }
          }

          for (vector<Particle*>::iterator i=particles.begin(); i!=particles.end(); ++i) {
            Cell const* c = (*i)->cell;
            Vector2<float> const particle_velocity = (*i)->p.momentum/(*i)->p.mass;
            Vector2<float> const cell_velocity = c->p.momentum/c->p.mass;
            Vector2<float> const diff(particle_velocity-cell_velocity);
            Vector2<float> const diff_rot(diff.x*c->rot[0]-diff.y*c->rot[1],
                                          diff.x*c->rot[1]+diff.y*c->rot[0]);
            (*i)->p.momentum = (*i)->p.mass*(cell_velocity+diff_rot);
          }
        }
      };

      vector<shared_ptr<Particle> > particles;
      vector<vector<shared_ptr<Cell> > > cells;

      // --------------------------------------------------------------------------------
      void initialize(shared_ptr<SociariumGraph> g0, shared_ptr<SociariumGraph> g1) {
        particles.clear();
        cells.clear();
        number_of_colors = g1->nsize();

        using namespace sociarium_project_layout;
        float const frame_size = get_layout_frame_size();
        position_min = get_layout_frame_position()-Vector2<float>(frame_size, frame_size);
        position_max = get_layout_frame_position()+Vector2<float>(frame_size, frame_size);
        grid_interval = get_grid_interval();

        int const num = int(2.0f*frame_size/grid_interval);
        assert(frame_size==0.5f*num*grid_interval);
        number_of_cell_x = num;
        number_of_cell_y = num;
        vector<vector<shared_ptr<Cell> > >(
          number_of_cell_x, vector<shared_ptr<Cell> >(number_of_cell_y)).swap(cells);

        // Make the cell space.
        for (int i=0; i<number_of_cell_x; ++i) {
          for (int j=0; j<number_of_cell_y; ++j) {
            shared_ptr<Cell>& c = cells[i][j];
            c.reset(new Cell);
          }
        }

        for (int i=0; i<number_of_cell_x; ++i) {

          int const im1 = (number_of_cell_x+i-1)%number_of_cell_x;
          int const ip1 = (i+1)%number_of_cell_x;

          for (int j=0; j<number_of_cell_y; ++j) {

            int const jm1 = (number_of_cell_y+j-1)%number_of_cell_y;
            int const jp1 = (j+1)%number_of_cell_y;

            shared_ptr<Cell>& c = cells[i][j];
            c->neighbors.resize(9);

            if (i==0 || i==number_of_cell_x-1 || j==0 || j==number_of_cell_y-1)
              c->neighbors[0] = c.get();
            else
              c->neighbors[0] = 0;

            c->neighbors[1] = cells[ip1][j].get();
            c->neighbors[2] = cells[im1][j].get();
            c->neighbors[3] = cells[i][jp1].get();
            c->neighbors[4] = cells[i][jm1].get();
            c->neighbors[5] = cells[ip1][jp1].get();
            c->neighbors[6] = cells[ip1][jm1].get();
            c->neighbors[7] = cells[im1][jp1].get();
            c->neighbors[8] = cells[im1][jm1].get();
          }
        }

        { // Make the particle list.
          node_property_iterator i   = g0->node_property_begin();
          node_property_iterator end = g0->node_property_end();

          for (; i!=end; ++i) {
            DynamicNodeProperty const& dnp = i->second;
            StaticNodeProperty* snp = dnp.get_static_property();
            Vector2<float> const& pos = snp->get_position();

            if (is_visible(dnp) &&
                pos.x>=position_min.x && pos.x<position_max.x &&
                pos.y>=position_min.y && pos.y<position_max.y) {
              shared_ptr<Particle> particle(new Particle);
              particle->dnp = &dnp;

              int x = int((pos.x-position_min.x)/grid_interval);
              int y = int((pos.y-position_min.y)/grid_interval);

              if (x==number_of_cell_x) x = number_of_cell_x-1;
              if (y==number_of_cell_y) y = number_of_cell_y-1;

              assert(0<=x && x<int(number_of_cell_x));
              assert(0<=y && y<int(number_of_cell_y));

              particle->cell = cells[x][y].get();
              cells[x][y]->particles.push_back(particle.get());
              particle->p.mass = 0.5f;

              adjacency_list_iterator j    = i->first->begin();
              adjacency_list_iterator jend = i->first->end();

              for (; j!=jend; ++j)
                if (is_visible(g0->property(*j)))
                  particle->p.mass += 0.2f;

              particle->p.momentum.set(gauss_df(temperature, 0.0f)/particle->p.mass,
                                       gauss_df(temperature, 0.0f)/particle->p.mass);

              particles.push_back(particle);
            }
          }
        }
      }

    } // The end of the namespace "LatticeGasMethod"


    ////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    void update_position(T first, T last, float velocity_decay_rate) {
      for (; first!=last; ++first) {
        DynamicNodeProperty const* dnp = first->first;
        StaticNodeProperty* snp = dnp->get_static_property();
        Momentum& p = first->second;

        if (dnp->get_flag()&ElementFlag::CAPTURED
            || snp->get_flag()&ElementFlag::CAPTURED)
          p.momentum.set(0.0f, 0.0f);
        else {
          assert(p.mass>0.0f);
          Vector2<float> velocity = p.momentum/p.mass;
          snp->set_position(snp->get_position()+velocity);
          p.momentum *= velocity_decay_rate;
        }
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void force_direction_kamada_kawai_method(
      shared_ptr<SociariumGraphTimeSeries> time_series) {

      size_t const index_of_current_layer = time_series->index_of_current_layer();

      shared_ptr<SociariumGraph> g0
        = time_series->get_graph(0, index_of_current_layer);

      shared_ptr<SociariumGraph> g1
        = time_series->get_graph(1, index_of_current_layer);

      if (update) {
        update = false;
        KamadaKawai::initialize(g0, false);
      }

      KamadaKawai::update_force(KamadaKawai::force_NN, 1.0f);
      update_position(momentum_list.begin(), momentum_list.end(),
                      default_velocity_decay_rate);

      node_property_iterator i   = g1->node_property_begin();
      node_property_iterator end = g1->node_property_end();

      for (; i!=end; ++i)
        sociarium_project_layout::reset_position(i->second);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void force_direction_kamada_kawai_method_with_community_separation(
      shared_ptr<SociariumGraphTimeSeries> time_series) {

      size_t const index_of_current_layer = time_series->index_of_current_layer();

      shared_ptr<SociariumGraph> g0
        = time_series->get_graph(0, index_of_current_layer);

      shared_ptr<SociariumGraph> g1
        = time_series->get_graph(1, index_of_current_layer);

      if (update) {
        update = false;
        KamadaKawai::initialize(g0, true);
      }

      KamadaKawai::update_force(KamadaKawai::force_NN, 1.0f);
      update_position(momentum_list.begin(), momentum_list.end(),
                      default_velocity_decay_rate);

      node_property_iterator i   = g1->node_property_begin();
      node_property_iterator end = g1->node_property_end();

      for (; i!=end; ++i)
        sociarium_project_layout::reset_position(i->second);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void force_direction_community_oriented(
      shared_ptr<SociariumGraphTimeSeries> time_series) {

      size_t const index_of_current_layer = time_series->index_of_current_layer();

      shared_ptr<SociariumGraph> g0
        = time_series->get_graph(0, index_of_current_layer);

      shared_ptr<SociariumGraph> g1
        = time_series->get_graph(1, index_of_current_layer);

      if (update) {
        update = false;
        KamadaKawai::element.clear();
        Spring::element.clear();
        momentum_list.clear();

        // Calculate the force between communities.
        {
          node_property_iterator i   = g1->node_property_begin();
          node_property_iterator end = g1->node_property_end();

          for (; i!=end; ++i) {

            DynamicNodeProperty const& dnp0 = i->second;

            if (is_hidden(dnp0)) continue;

            float& m = momentum_list[&dnp0].mass = 1.0f;

            vector<DynamicNodeProperty*>::const_iterator j    = dnp0.lower_nbegin();
            vector<DynamicNodeProperty*>::const_iterator jend = dnp0.lower_nend();

            for (; j!=jend; ++j)
              if (is_visible(**j)) ++m;
          }
        }{
          shared_ptr<BFSTraverser> t = BFSTraverser::create<bidirectional_tag>(g1);

          node_property_iterator i   = g1->node_property_begin();
          node_property_iterator end = g1->node_property_end();

          for (; i!=end; ++i) {
            Node const* n = i->first;
            DynamicNodeProperty& dnp0 = i->second;

            if (is_hidden(dnp0)) continue;

            for (t->reset(), t->start(n); !t->end(); t->advance()) {
              if (t->node()<=n) continue;

              DynamicNodeProperty const& dnp1 = g1->property(t->node());
              shared_ptr<KamadaKawai::Element> kke(new KamadaKawai::Element);
              kke->dnp0 = &dnp0;
              kke->dnp1 = &dnp1;
              kke->distance = float(t->distance());
              kke->p0 = &momentum_list[kke->dnp0];
              kke->p1 = &momentum_list[kke->dnp1];

              try {
                KamadaKawai::element.push_back(kke);
              } catch (...) {
                KamadaKawai::element.clear();
                return;
              }
            }
          }
        }

        {
          node_property_iterator i   = g1->node_property_begin();
          node_property_iterator end = g1->node_property_end();

          for (; i!=end; ++i) {

            DynamicNodeProperty const& dnp = i->second;

            float const r = sqrt(momentum_list[&dnp].mass);
            float const len0 = Spring::length_CN*r;
            float const len1 = Spring::length_NN*sqrt(r);
            float const cf = Spring::force_NN/(r*r);

            vector<DynamicNodeProperty*>::const_iterator j    = dnp.lower_nbegin();
            vector<DynamicNodeProperty*>::const_iterator jend = dnp.lower_nend();

            for (; j!=jend; ++j) {

              if (is_hidden(**j)) continue;

              // Calculate the force between the center of the community and
              // its members.
              momentum_list[*j].mass = 1.0f;
              shared_ptr<Spring::Element> se(new Spring::Element);
              se->dnp0 = &dnp;
              se->dnp1 = *j;
              se->length = len0;
              se->coefficient = Spring::force_CN;
              se->p0 = &momentum_list[se->dnp0];
              se->p1 = &momentum_list[se->dnp1];

              try {
                Spring::element.push_back(se);
              } catch (...) {
                Spring::element.clear();
                return;
              }

              // Calculate the force between the members of the community.
              Node const* n = (*j)->get_graph_element();

              vector<DynamicNodeProperty*>::const_iterator k = j+1;

              for (; k!=jend; ++k) {

                if (is_hidden(**k)) continue;

                momentum_list[*j].mass = 1.0f;
                shared_ptr<Spring::Element> se(new Spring::Element);
                se->dnp0 = *j;
                se->dnp1 = *k;
                se->length = len1;
                se->coefficient = cf;
                se->p0 = &momentum_list[se->dnp0];
                se->p1 = &momentum_list[se->dnp1];

                try {
                  Spring::element.push_back(se);
                } catch (...) {
                  Spring::element.clear();
                  return;
                }
              }
            }
          }
        }
      }

      KamadaKawai::update_force(KamadaKawai::force_CC, 2.0);

      Spring::update_force(1.0);

      update_position(momentum_list.begin(), momentum_list.end(),
                      default_velocity_decay_rate);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void force_direction_spring_and_repulsive_force(
      shared_ptr<SociariumGraphTimeSeries> time_series) {

      size_t const index_of_current_layer = time_series->index_of_current_layer();

      shared_ptr<SociariumGraph> g0
        = time_series->get_graph(0, index_of_current_layer);

      shared_ptr<SociariumGraph> g1
        = time_series->get_graph(1, index_of_current_layer);

      if (update) {
        update = false;
        SpringAndRepulsiveForce::initialize(g0);
      }

      SpringAndRepulsiveForce::update_force();
      update_position(momentum_list.begin(), momentum_list.end(),
                      default_velocity_decay_rate);

      node_property_iterator i   = g1->node_property_begin();
      node_property_iterator end = g1->node_property_end();

      for (; i!=end; ++i)
        sociarium_project_layout::reset_position(i->second);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void force_direction_spring_and_drift(
      shared_ptr<SociariumGraphTimeSeries> time_series) {

      size_t const index_of_current_layer = time_series->index_of_current_layer();

      shared_ptr<SociariumGraph> g0
        = time_series->get_graph(0, index_of_current_layer);

      shared_ptr<SociariumGraph> g1
        = time_series->get_graph(1, index_of_current_layer);

      if (update) {
        update = false;
        Spring::element.clear();
        momentum_list.clear();

        for (edge_property_iterator i=g0->edge_property_begin(); i!=g0->edge_property_end(); ++i) {
          Edge const* e = i->first;
          DynamicEdgeProperty const& dep = i->second;

          if (is_hidden(dep)) continue;

          DynamicNodeProperty const& dnp0 = g0->property(e->source());
          DynamicNodeProperty const& dnp1 = g0->property(e->target());

          shared_ptr<Spring::Element> se(new Spring::Element);
          se->dnp0 = &dnp0;
          se->dnp1 = &dnp1;

          if (dep.get_weight()<0.001f) continue;

          se->length = 0.05f*(1.0f/dep.get_weight()+5.0f);

          se->coefficient = float(1e-2);
          se->p0 = &momentum_list[se->dnp0];
          se->p1 = &momentum_list[se->dnp1];
          se->p0->mass = 10.0f;
          se->p1->mass = 10.0f;

          try {
            Spring::element.push_back(se);
          } catch (...) {
            Spring::element.clear();
            return;
          }
        }
      }

      Spring::update_force(1.0);

      {
        unordered_map<DynamicNodeProperty const*, Momentum>::iterator i   = momentum_list.begin();
        unordered_map<DynamicNodeProperty const*, Momentum>::iterator end = momentum_list.end();

        for (; i!=end; ++i) {
          Momentum& p = i->second;
          p.momentum += 0.1f*Vector2<float>(float(mt::rand()-0.5), float(mt::rand()-0.5));
        }
      }

      update_position(momentum_list.begin(), momentum_list.end(), default_velocity_decay_rate);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void force_direction_lattice_gas_method(
      shared_ptr<SociariumGraphTimeSeries> time_series) {

      using namespace LatticeGasMethod;

      size_t const index_of_current_layer = time_series->index_of_current_layer();

      shared_ptr<SociariumGraph> g0
        = time_series->get_graph(0, index_of_current_layer);

      shared_ptr<SociariumGraph> g1
        = time_series->get_graph(1, index_of_current_layer);

      vector<pair<Particle*, Particle*> > edge_list;

      if (update) {
        update = false;
        initialize(g0, g1);
      }

      for (int i=0; i<number_of_cell_x; ++i) {
        for (int j=0; j<number_of_cell_y; ++j) {
          cells[i][j]->update_local_parameters();
        }
      }

      for (int i=0; i<number_of_cell_x; ++i) {
        for (int j=0; j<number_of_cell_y; ++j) {
          cells[i][j]->update_particle_velocity();
        }
      }

      for (int i=0; i<number_of_cell_x; ++i) {
        for (int j=0; j<number_of_cell_y; ++j) {
          cells[i][j]->particles.clear();
        }
      }

      for (vector<shared_ptr<Particle> >::iterator i=particles.begin(); i!=particles.end(); ++i) {
        Momentum& p = (*i)->p;
        DynamicNodeProperty const* dnp = (*i)->dnp;
        StaticNodeProperty* snp = dnp->get_static_property();

        if (dnp->get_flag()&ElementFlag::CAPTURED || snp->get_flag()&ElementFlag::CAPTURED)
          p.momentum.set(0.0f, 0.0f);
        else {
          assert(p.mass>0.0f);
          Vector2<float> velocity = p.momentum/p.mass;
          Vector2<float> pos = snp->get_position()+velocity;
          int const x = int((pos.x-position_min.x)/grid_interval);
          int const y = int((pos.y-position_min.y)/grid_interval);
          Vector2<float> pos_old = snp->get_position();
          int const x_old = int((pos_old.x-position_min.x)/grid_interval);
          int const y_old = int((pos_old.y-position_min.y)/grid_interval);
          if (pos.x<position_min.x) {
            p.momentum.set(gamma_df(temperature)/p.mass, gauss_df(temperature, 0.0f)/p.mass);
            if (x_old>=0 && x_old<number_of_cell_x && y_old>=0 && y_old<number_of_cell_y) {
              (*i)->cell = cells[x_old][y_old].get();
              cells[x_old][y_old]->particles.push_back(i->get());
            }
          }
          else if (pos.x>=position_max.x) {
            p.momentum.set(-gamma_df(temperature)/p.mass, gauss_df(temperature, 0.0f)/p.mass);
            if (x_old>=0 && x_old<number_of_cell_x && y_old>=0 && y_old<number_of_cell_y) {
              (*i)->cell = cells[x_old][y_old].get();
              cells[x_old][y_old]->particles.push_back(i->get());
            }
          }
          else if (pos.y<position_min.y) {
            p.momentum.set(gauss_df(temperature, 0.0f)/p.mass, gamma_df(temperature)/p.mass);
            if (x_old>=0 && x_old<number_of_cell_x && y_old>=0 && y_old<number_of_cell_y) {
              (*i)->cell = cells[x_old][y_old].get();
              cells[x_old][y_old]->particles.push_back(i->get());
            }
          }
          else if (pos.y>=position_max.y) {
            p.momentum.set(gauss_df(temperature, 0.0f)/p.mass, -gamma_df(temperature)/p.mass);
            if (x_old>=0 && x_old<number_of_cell_x && y_old>=0 && y_old<number_of_cell_y) {
              (*i)->cell = cells[x_old][y_old].get();
              cells[x_old][y_old]->particles.push_back(i->get());
            }
          }
          else {
            snp->set_position(pos);
            (*i)->cell = cells[x][y].get();
            cells[x][y]->particles.push_back(i->get());
          }
        }
      }

      for (node_property_iterator i=g1->node_property_begin(); i!=g1->node_property_end(); ++i)
        sociarium_project_layout::reset_position(i->second);
    }

  } // The end of the anonymous namespace


  namespace sociarium_project_force_direction {

    ////////////////////////////////////////////////////////////////////////////////
    void toggle_execution(void) {

      using namespace sociarium_project_thread;

      shared_ptr<Thread> tf = get_thread_function(FORCE_DIRECTION);

      if (thread_is_active) {
        tf->suspend();
        thread_is_active = false;
      } else {
        tf->resume();
        thread_is_active = true;
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool is_active(void) {
      return thread_is_active;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void should_be_updated(void) {
      update = true;
    }


    ////////////////////////////////////////////////////////////////////////////////
    void set_force_scale(float scale) {
      force_scale = scale;
    }

    void set_kk_force_NN(float value) {
      KamadaKawai::force_NN = value;
    }

    void set_kk_force_CC(float value) {
      KamadaKawai::force_CC = value;
    }

    void set_spring_force_CN(float value) {
      Spring::force_CN = value;
    }

    void set_spring_length_CN(float value) {
      Spring::length_CN = value;
    }

    void set_spring_force_NN(float value) {
      Spring::force_NN = value;
    }

    void set_spring_length_NN(float value) {
      Spring::length_NN = value;
    }

  } // The end of the namespace "sociarium_project_force_direction"


  using namespace sociarium_project_thread;
  using namespace sociarium_project_force_direction;
  using namespace sociarium_project_algorithm_selector;
  using namespace RealTimeForceDirectionAlgorithm;


  ////////////////////////////////////////////////////////////////////////////////
  class ForceDirectionThreadImpl : public ForceDirectionThread {
  public:
    ForceDirectionThreadImpl(void) {}

    ~ForceDirectionThreadImpl() {}

    void operator()(void) {

      for (;;) {

        if (cancel_check()) break;

        shared_ptr<SociariumGraphTimeSeries> ts
          = sociarium_project_graph_time_series::get();

        if (ts==0) break;

        ts->read_lock();
        /*
         * Don't forget to call read_unlock().
         */

        switch (get_force_direction_algorithm()) {

        case KAMADA_KAWAI_METHOD:
          force_direction_kamada_kawai_method(ts);
          break;

        case KAMADA_KAWAI_METHOD_WITH_COMMUNITY_SEPARATION:
          force_direction_kamada_kawai_method_with_community_separation(ts);
          break;

        case COMMUNITY_ORIENTED:
          force_direction_community_oriented(ts);
          break;

        case SPRING_AND_REPULSIVE_FORCE:
          force_direction_spring_and_repulsive_force(ts);
          break;

        case LATTICE_GAS_METHOD:
          force_direction_lattice_gas_method(ts);
          break;

        case DESIGNTIDE:
          force_direction_spring_and_drift(ts);
          break;

        default: assert(0 && "never reach");
        }

        ts->read_unlock();

        fps_keeper.wait();
      }

      detach(FORCE_DIRECTION);
    }

  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of ForceDirectionThread.
  shared_ptr<ForceDirectionThread> ForceDirectionThread::create(void) {
    return shared_ptr<ForceDirectionThread>(new ForceDirectionThreadImpl);
  }

} // The end of the namespace "hashimoto_ut"
