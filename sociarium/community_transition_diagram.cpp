// s.o.c.i.a.r.i.u.m: community_transition_diagram.cpp
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

#include <map>
#include <unordered_map>
#include "community_transition_diagram.h"
#include "sociarium_graph_time_series.h"
#include "../shared/mutex.h"
#include "../shared/predefined_color.h"

namespace hashimoto_ut {

  using std::vector;
  using std::map;
  using std::pair;
  using std::make_pair;
  using std::tr1::unordered_map;
  using std::tr1::shared_ptr;

  typedef SociariumGraph::node_property_iterator node_property_iterator;

  namespace {
    shared_ptr<CommunityTransitionDiagram> diagram;

    size_t diagram_resolution = 10; // The number of interpolated points between layers.
    pair<int, int> diagram_scope(0, 8*diagram_resolution);
    int diagram_mode = 0; // 0:community, 1:node, 2:node_in_community

    Vector2<double> default_frame_position(0.05, 0.05);
    Vector2<double> default_frame_size(0.9, 0.4);
    Vector2<double> frame_position(default_frame_position);
    Vector2<double> frame_size(default_frame_size);

    ////////////////////////////////////////////////////////////////////////////////
    vector<float> calc_spline_curve(
      vector<float> const& data_point, size_t resolution) {

      int const number_of_points = int(data_point.size())-1;
      vector<float> alpha(number_of_points, 0.0f);
      vector<float> mu(number_of_points, 0.0f);
      vector<float> nu(number_of_points, 0.0f);
      vector<float> a(number_of_points+1, 0.0f);
      vector<float> b(number_of_points, 0.0f);
      vector<float> c(number_of_points+1, 0.0f);
      vector<float> d(number_of_points, 0.0f);

      for (int i=0; i<=number_of_points; ++i)
        a[i] = data_point[i];

      for (int i=1; i<number_of_points; ++i)
        alpha[i] = 3.0f*((a[i+1])-2.0f*a[i]+a[i-1]);

      mu[1] = 1.0f/4.0f;
      nu[1] = alpha[1]/4.0f;

      for (int i=2; i<number_of_points; ++i) {
        float const el = 4.0f-mu[i-1];
        mu[i] = 1.0f/el;
        nu[i] = (alpha[i]-nu[i-1])/el;
      }

      c[number_of_points] = nu[number_of_points-1];

      for (int i=number_of_points-1; i>0; --i)
        c[i] = nu[i]-mu[i]*c[i+1];

      for (int i=0; i<number_of_points; ++i) {
        b[i] = (a[i+1]-a[i])-(c[i+1]+2.0f*c[i])/3.0f;
        d[i] = (c[i+1]-c[i])/3.0f;
      }

      size_t const number_of_interpolated_points
        = resolution*number_of_points+1;

      vector<float> retval(number_of_interpolated_points, 0.0f);

      for (size_t i=0; i<number_of_interpolated_points; ++i) {
        float const jf = float(i)/resolution;
        size_t const j = size_t(jf);

        if (j==number_of_points) {
          retval[i] = a[number_of_points];
          break;
        }

        float const diff = jf-float(j);
        retval[i] = a[j]+diff*(b[j]+diff*(c[j]+diff*d[j]));
      }

      return retval;
    }

  } // The end of the anonymous namespace


  namespace sociarium_project_community_transition_diagram {

    ////////////////////////////////////////////////////////////////////////////////
    class TrajectoryImpl : public Trajectory {
    public:
      TrajectoryImpl(StaticNodeProperty* snp)
           : snp_(snp), duration_(0, 0) {}

      ~TrajectoryImpl() {}

      StaticNodeProperty* get_snp(void) const {
        return snp_;
      }

      pair<size_t, size_t> const& get_duration(void) const {
        return duration_;
      }

      pair<float, float>
        get_base_position(size_t index_of_layer) const {
          assert(index_of_layer>=duration_.first);
          assert(index_of_layer<=duration_.second);
          index_of_layer -= duration_.first;
          assert(base_position_.first.size()==base_position_.second.size());
          assert(0<=index_of_layer && index_of_layer<base_position_.first.size());
          return make_pair(base_position_.first[index_of_layer],
                           base_position_.second[index_of_layer]);
        }

      pair<float, float>
        get_interpolated_position(size_t index_of_point) const {
          size_t const first_point = diagram_resolution*duration_.first;
          size_t const last_point = diagram_resolution*duration_.second;
          assert(index_of_point>=first_point);
          assert(index_of_point<=last_point);
          index_of_point -= first_point;
          assert(interpolated_position_.first.size()==interpolated_position_.second.size());
          assert(0<=index_of_point && index_of_point<interpolated_position_.first.size());
          return make_pair(interpolated_position_.first[index_of_point],
                           interpolated_position_.second[index_of_point]);
        }

      Vector3<float> const& get_base_color(size_t index_of_layer) const {
        assert(index_of_layer>=duration_.first);
        assert(index_of_layer<=duration_.second);
        index_of_layer -= duration_.first;
        assert(0<=index_of_layer && index_of_layer<base_color_.size());
        return base_color_[index_of_layer];
      }

      Vector3<float> const& get_interpolated_color(size_t index_of_point) const {
        size_t const first_point = diagram_resolution*duration_.first;
        size_t const last_point = diagram_resolution*duration_.second;
        assert(index_of_point>=first_point);
        assert(index_of_point<=last_point);
        index_of_point -= first_point;
        assert(0<=index_of_point && index_of_point<interpolated_color_.size());
        return interpolated_color_[index_of_point];
      }

      void correct_base_position(vector<float> const& base_line) {
        assert(base_position_.first.size()==base_position_.second.size());
        for (size_t i=0; i<base_position_.first.size(); ++i) {
          size_t const index_of_layer = duration_.first+i;
          assert(index_of_layer<base_line.size());
          base_position_.first[i] += base_line[index_of_layer];
          base_position_.second[i] += base_line[index_of_layer];
        }
      }

      void interpolate(void) {

        assert(base_position_.first.size()==base_position_.second.size());

        size_t const number_of_base_positions = base_position_.first.size();
        size_t const number_of_interpolated_points
          = diagram_resolution*(number_of_base_positions-1)+1;

        vector<float>(number_of_interpolated_points).swap(interpolated_position_.first);
        vector<float>(number_of_interpolated_points).swap(interpolated_position_.second);
        vector<Vector3<float> >(number_of_interpolated_points).swap(interpolated_color_);

        // Interpolate positions.
        if (number_of_base_positions<3) {
          // Use straight lines.
          for (size_t i=0; i<number_of_base_positions; ++i) {
            float ylb = base_position_.first[i];
            float ylt = base_position_.second[i];
            interpolated_position_.first[i*diagram_resolution] = ylb;
            interpolated_position_.second[i*diagram_resolution] = ylt;
            if (i==number_of_base_positions-1) break;
            float const yrb = base_position_.first[i+1];
            float const yrt = base_position_.second[i+1];
            float const db = (yrb-ylb)/diagram_resolution;
            float const dt = (yrt-ylt)/diagram_resolution;
            for (size_t j=1; j<diagram_resolution; ++j) {
              interpolated_position_.first[i*diagram_resolution+j] = ylb+=db;
              interpolated_position_.second[i*diagram_resolution+j] = ylt+=dt;
            }
          }

        } else {
          // Use spline curves.
          { // Bottom line.
            vector<float> pos = calc_spline_curve(base_position_.first, diagram_resolution);
            assert(pos.size()==number_of_interpolated_points);
            interpolated_position_.first.swap(pos);
          }
          { // Top line.
            vector<float> pos = calc_spline_curve(base_position_.second, diagram_resolution);
            assert(pos.size()==number_of_interpolated_points);
            interpolated_position_.second.swap(pos);
          }
        }

        // Interpolate colors.
        for (size_t i=0; i<number_of_base_positions-1; ++i) {
          for (size_t j=0; j<diagram_resolution; ++j) {
            size_t index = i*diagram_resolution+j;
            float const r = float(diagram_resolution-j)/diagram_resolution;
            Vector3<float> const& rgb0 = base_color_[i];
            Vector3<float> const& rgb1 = base_color_[i+1];
            interpolated_color_[index] = r*rgb0+(1.0f-r)*rgb1;
          }
        }

        interpolated_color_[interpolated_color_.size()-1]
          = base_color_[base_color_.size()-1];
      }

    private:
      friend class CommunityTransitionDiagramImpl;
      StaticNodeProperty* snp_;
      pair<size_t, size_t> duration_;
      pair<vector<float>, vector<float> > base_position_; // first:bottom, second:top
      pair<vector<float>, vector<float> > interpolated_position_;
      vector<Vector3<float> > base_color_;
      vector<Vector3<float> > interpolated_color_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    class CommunityTransitionDiagramImpl :
      public CommunityTransitionDiagram,
      public Mutex {

    public:
      CommunityTransitionDiagramImpl(void) : max_bundle_size_(0.0f) {}

      ~CommunityTransitionDiagramImpl() {}

      ////////////////////////////////////////////////////////////////////////////////
      void read_lock(void) const {
        Mutex::read_lock();
      }

      void read_unlock(void) const {
        Mutex::read_unlock();
      }

      void write_lock(void) const {
        Mutex::write_lock();
      }

      void write_unlock(void) const {
        Mutex::write_unlock();
      }

      ////////////////////////////////////////////////////////////////////////////////
      vector<shared_ptr<Trajectory> >::const_iterator
        trajectory_begin(void) const {
          return trajectory_.begin();
        }

      vector<shared_ptr<Trajectory> >::const_iterator
        trajectory_end(void) const {
          return trajectory_.end();
        }

      ////////////////////////////////////////////////////////////////////////////////
      void set_current_layer(size_t index_of_layer) {
        pair<int, int> const& scope = get_scope();
        int const range = (scope.second-scope.first)/2;
        assert(range>=int(diagram_resolution));
        int const center = int(index_of_layer*diagram_resolution);
        set_scope(center-range, center+range);
      }

      ////////////////////////////////////////////////////////////////////////////////
      float get_max_bundle_size(void) const {
        return max_bundle_size_;
      }

      ////////////////////////////////////////////////////////////////////////////////
      // Before and after calling update(), don't forget to call write_lock/unlock().
        void update(int mode) {

          typedef SociariumGraphTimeSeries::StaticNodePropertySet StaticNodePropertySet;
          typedef SociariumGraphTimeSeries::DynamicNodePropertyMap DynamicNodePropertyMap;

          struct TrajectoryCore {
            size_t first_layer;
            pair<vector<float>, vector<float> > base_position;
            vector<Vector3<float> > color;
          };

          diagram_mode = mode;

          shared_ptr<SociariumGraphTimeSeries> ts = sociarium_project_graph_time_series::get();
          TimeSeriesLock lock(ts, TimeSeriesLock::Read);

          trajectory_.clear();

          size_t const number_of_layers = ts->number_of_layers();

          vector<float> base_line(number_of_layers, 0);

          typedef unordered_map<StaticNodeProperty const*, TrajectoryCore>
            SNP2TrajectoryCore;
          SNP2TrajectoryCore ccc_map;

          {
            StaticNodePropertySet::const_iterator i
              = ts->static_node_property_begin(mode!=0?0:1);
            StaticNodePropertySet::const_iterator end
              = ts->static_node_property_end(mode!=0?0:1);

            for (; i!=end; ++i) {
              StaticNodeProperty const& snp = *i;
              assert(snp.number_of_dynamic_properties()>0);

              // Sort dynamic nodes in time order.

              map<size_t, DynamicNodeProperty*> m;

              {
                DynamicNodePropertyMap::const_iterator j
                  = snp.dynamic_property_begin();
                DynamicNodePropertyMap::const_iterator jend
                  = snp.dynamic_property_end();

                for (; j!=jend; ++j)
                  m.insert(make_pair(j->second, j->first));
              }

              assert(!m.empty());

              size_t const first_layer = m.begin()->first;
              size_t const last_layer = m.rbegin()->first;
              size_t const duration = last_layer-first_layer+1;

              // Layout community continuums on base line 0.

              TrajectoryCore ccc;

              if (first_layer==0) {
                ccc.first_layer = first_layer;
                if (last_layer==number_of_layers-1) {
                  vector<float>(duration, 0.0f).swap(ccc.base_position.first);
                  vector<float>(duration, 0.0f).swap(ccc.base_position.second);
                  vector<Vector3<float> >(duration).swap(ccc.color);
                } else {
                  vector<float>(duration+1, 0.0f).swap(ccc.base_position.first);
                  vector<float>(duration+1, 0.0f).swap(ccc.base_position.second);
                  vector<Vector3<float> >(duration+1).swap(ccc.color);
                }
              } else {
                ccc.first_layer = first_layer-1;
                if (last_layer==number_of_layers-1) {
                  vector<float>(duration+1, 0.0f).swap(ccc.base_position.first);
                  vector<float>(duration+1, 0.0f).swap(ccc.base_position.second);
                  vector<Vector3<float> >(duration+1).swap(ccc.color);
                } else {
                  vector<float>(duration+2, 0.0f).swap(ccc.base_position.first);
                  vector<float>(duration+2, 0.0f).swap(ccc.base_position.second);
                  vector<Vector3<float> >(duration+2).swap(ccc.color);
                }
              }

              {
                map<size_t, DynamicNodeProperty*>::const_iterator j = m.begin();
                size_t lay = j->first;
                size_t color_id = j->second->get_color_id();

                if (first_layer==0) {

                  for (size_t k=0; k<duration; ++k, ++lay) {

                    if (j!=m.end() && j->first==lay) {
                      float& pos = base_line[lay];
                      ccc.base_position.first[k] = pos;
                      if (mode==0) pos += j->second->number_of_lower_nodes();
                      else pos += 2;
                      ccc.base_position.second[k] = pos;
                      ccc.color[k] = predefined_color[j->second->get_color_id()];
                      ++pos;
                      ++j;
                    } else {
                      float& pos = base_line[lay];
                      ccc.base_position.first[k] = pos;
                      ccc.base_position.second[k] = pos;
                      ccc.color[k] = predefined_color[color_id];
                      ++pos;
                    }
                  }

                  if (last_layer!=number_of_layers-1) {
                    float& pos = base_line[last_layer+1];
                    ccc.base_position.first[duration] = pos;
                    ccc.base_position.second[duration] = pos;
                    ccc.color[duration] = predefined_color[color_id];
                    ++pos;
                  }
                }

                else {

                  float& pos = base_line[first_layer-1];
                  ccc.base_position.first[0] = pos;
                  ccc.base_position.second[0] = pos;
                  ccc.color[0] = predefined_color[color_id];
                  ++pos;

                  for (size_t k=0; k<duration; ++k, ++lay) {

                    if (j!=m.end() && j->first==lay) {
                      float& pos = base_line[lay];
                      ccc.base_position.first[k+1] = pos;
                      if (mode==0) pos += j->second->number_of_lower_nodes();
                      else pos += 2;
                      ccc.base_position.second[k+1] = pos;
                      ccc.color[k+1] = predefined_color[j->second->get_color_id()];
                      ++pos;
                      ++j;
                    } else {
                      float& pos = base_line[lay];
                      ccc.base_position.first[k+1] = pos;
                      ccc.base_position.second[k+1] = pos;
                      ccc.color[k+1] = predefined_color[color_id];
                      ++pos;
                    }
                  }

                  if (last_layer!=number_of_layers-1) {
                    float& pos = base_line[last_layer+1];
                    ccc.base_position.first[duration+1] = pos;
                    ccc.base_position.second[duration+1] = pos;
                    ccc.color[duration+1] = predefined_color[color_id];
                    ++pos;
                  }
                }
              }

              ccc_map.insert(make_pair(&snp, ccc));
            }
          }

          max_bundle_size_
            = *std::max_element(base_line.begin(), base_line.end());

          for (size_t i=0; i<number_of_layers; ++i)
            base_line[i] *= -0.5f;

          // Layout community continuums considering modified base line.

          StaticNodePropertySet::const_iterator i
            = ts->static_node_property_begin(mode!=0?0:1);
          StaticNodePropertySet::const_iterator end
            = ts->static_node_property_end(mode!=0?0:1);

          for (; i!=end; ++i) {
            SNP2TrajectoryCore::iterator c = ccc_map.equal_range(&*i).first;
            if (c==ccc_map.end()) continue;

            TrajectoryCore& ccc = c->second;

            TrajectoryImpl* cc_impl
              = new TrajectoryImpl(const_cast<StaticNodeProperty*>(c->first));
            cc_impl->duration_.first = ccc.first_layer;
            cc_impl->duration_.second = ccc.first_layer+ccc.base_position.first.size()-1;
            cc_impl->base_position_.first.swap(ccc.base_position.first);
            cc_impl->base_position_.second.swap(ccc.base_position.second);
            cc_impl->correct_base_position(base_line);
            cc_impl->base_color_.swap(ccc.color);
            cc_impl->interpolate();
            trajectory_.push_back(shared_ptr<Trajectory>(cc_impl));
          }

          set_current_layer(ts->index_of_current_layer());
        }

      void clear(void) {
        trajectory_.clear();
        max_bundle_size_ = 0.0f;
      }

      bool empty(void) const {
        return trajectory_.empty();
      }

    private:
      vector<shared_ptr<Trajectory> > trajectory_;
      //vector<shared_ptr<Trajectory> > branch_;
      float max_bundle_size_;
    };


    ////////////////////////////////////////////////////////////////////////////////
    void initialize(void) {
      diagram.reset(new CommunityTransitionDiagramImpl);
    }

    shared_ptr<CommunityTransitionDiagram> get(void) {
      return diagram;
    }

    int get_diagram_mode(void) {
      return diagram_mode;
    }


    ////////////////////////////////////////////////////////////////////////////////
    Vector2<double> const& get_frame_position(void) {
      return frame_position;
    }

    void set_frame_position(Vector2<double> const& pos) {
      frame_position = pos;
    }

    Vector2<double> const& get_frame_size(void) {
      return frame_size;
    }

    void set_frame_size(Vector2<double> size) {
      if (size.x<0.1) size.x = 0.1;
      if (size.y<0.1) size.y = 0.1;
      frame_size = size;
    }


    ////////////////////////////////////////////////////////////////////////////////
    Vector2<double> const& get_default_frame_position(void) {
      return default_frame_position;
    }

    void set_default_frame_position(Vector2<double> const& pos) {
      default_frame_position = pos;
    }

    Vector2<double> const& get_default_frame_size(void) {
      return default_frame_size;
    }

    void set_default_frame_size(Vector2<double> size) {
      if (size.x<0.1) size.x = 0.1;
      if (size.y<0.1) size.y = 0.1;
      default_frame_size = size;
    }


    ////////////////////////////////////////////////////////////////////////////////
    size_t get_resolution(void) {
      return diagram_resolution;
    }

    void set_resolution(size_t resolution) {
      diagram_resolution = resolution;
    }

    pair<int, int> const& get_scope(void) {
      return diagram_scope;
    }

    void set_scope(int first_point, int last_point) {
      if (last_point-first_point<int(diagram_resolution))
        return;
      int const center = (last_point+first_point)/2;
      int const resolution = int(diagram_resolution);
      int range = (last_point-first_point)/2;
      diagram_scope.first = center-range;
      diagram_scope.second = center+range;
    }

  } // The end of the namespace "sociarium_project_community_transition_diagram"

} // The end of the namespace "hashimoto_ut"
