// s.o.c.i.a.r.i.u.m: timeline.cpp
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
#include "timeline.h"

namespace hashimoto_ut {

  namespace sociarium_project_timeline {

    namespace {

      // The unit is msec.
      UINT latency[AutoRun::NUMBER_OF_OPTIONS]
        = {
        0,    // not used.
        1000, // FORWARD_1
        500,  // FORWARD_2
        200,  // FORWARD_3
        100,  // FORWARD_4
        1000, // BACKWARD_1
        500,  // BACKWARD_2
        200,  // BACKWARD_3
        100,  // BACKWARD_4
      };

      int auto_run_id = AutoRun::STOP;
    }


    ////////////////////////////////////////////////////////////////////////////////
    UINT get_latency(void) {
      assert(0<=auto_run_id && auto_run_id<AutoRun::NUMBER_OF_OPTIONS);
      return latency[auto_run_id];
    }


    ////////////////////////////////////////////////////////////////////////////////
    int get_auto_run_id(void) {
      return auto_run_id;
    }

    void set_auto_run_id(int id) {
      assert(0<=id && id<AutoRun::NUMBER_OF_OPTIONS);
      auto_run_id = id;
    }

  } // The end of the namespace "sociarium_project_timeline"

} // The end of the namespace "hashimoto_ut"
