// s.o.c.i.a.r.i.u.m: thread_manager.h
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

#ifndef INCLUDE_GUARD_SOCIARIUM_PROJECT_THREAD_MANAGER_H
#define INCLUDE_GUARD_SOCIARIUM_PROJECT_THREAD_MANAGER_H

#include <memory>
#include <deque>
#include <string>

namespace hashimoto_ut {

  class Thread;

  ////////////////////////////////////////////////////////////////////////////////
  class ThreadManager {
  public:
    virtual ~ThreadManager() {}
    virtual std::tr1::shared_ptr<Thread> get(void) const = 0;
    virtual void set(std::tr1::shared_ptr<Thread> th) = 0;
    virtual void terminate(void) = 0;
    virtual bool is_running(void) const = 0;

    static std::tr1::shared_ptr<ThreadManager> create(void);
  };

  namespace sociarium_project_thread_manager {

    enum {
      ////////////////////////////////////////////////////////////////////////////////
      // List of threads invoked by the application.
      GRAPH_CREATION = 0,
      GRAPH_RETOUCH,
      LAYOUT,
      COMMUNITY_DETECTION,
      NODE_SIZE_UPDATE,
      EDGE_WIDTH_UPDATE,
      FORCE_DIRECTION,
      NUMBER_OF_THREAD_CATEGORIES
    };

    ////////////////////////////////////////////////////////////////////////////////
    // Generate all thread managers listed above.
    // This should be called when the world is created.
    void initialize(void);

    ////////////////////////////////////////////////////////////////////////////////
    // Terminate all threads safely.
    // This should be called when the world is destructed.
    void finalize(void);

    ////////////////////////////////////////////////////////////////////////////////
    // Return each thread manager specified by @thread_id.
    std::tr1::shared_ptr<ThreadManager> get(int thread_id);

    ////////////////////////////////////////////////////////////////////////////////
    // Return the message list that informs the progress of thread calculation.
    std::deque<std::wstring>& get_status(int thread_id);
  }

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_PROJECT_THREAD_MANAGER_H
