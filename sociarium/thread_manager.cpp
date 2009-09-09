// s.o.c.i.a.r.i.u.m: thread_manager.cpp
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
#include <deque>
#include <string>
#include "thread_manager.h"
#include "language.h"
#include "../shared/thread.h"

namespace hashimoto_ut {

  using std::vector;
  using std::deque;
  using std::wstring;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_language;

  namespace sociarium_project_thread_manager {

    namespace {

      int const NUMBER_OF_THREAD_MESSAGE_HIERARCHIES = 2;

      vector<deque<wstring> >
        status(NUMBER_OF_THREAD_CATEGORIES,
               deque<wstring>(NUMBER_OF_THREAD_MESSAGE_HIERARCHIES));

      vector<std::tr1::shared_ptr<ThreadManager> > thread_manager;

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    void initialize(void) {
      for (int i=0; i<NUMBER_OF_THREAD_CATEGORIES; ++i)
        thread_manager.push_back(ThreadManager::create());
    }


    ////////////////////////////////////////////////////////////////////////////////
    void finalize(void) {
      for (int i=0; i<NUMBER_OF_THREAD_CATEGORIES; ++i)
        thread_manager[i]->terminate();
    }


    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<ThreadManager> get(int thread_id) {
      assert(0<=thread_id && thread_id<NUMBER_OF_THREAD_CATEGORIES);
      return thread_manager[thread_id];
    }


    ////////////////////////////////////////////////////////////////////////////////
    deque<wstring>& get_status(int thread_id) {
      assert(0<=thread_id && thread_id<NUMBER_OF_THREAD_CATEGORIES);
      return status[thread_id];
    }

  } // The end of the namespace "sociarium_project_thread_manager"


  ////////////////////////////////////////////////////////////////////////////////
  class ThreadManagerImpl : public ThreadManager {
  public:
    ThreadManagerImpl(void) {}

    ~ThreadManagerImpl() {}

    shared_ptr<Thread> get(void) const {
      return thread_;
    }

    void set(shared_ptr<Thread> th) {
      thread_ = th;
      if (thread_)
        base_.reset(new boost::thread(boost::ref(*thread_)));
    }

    void terminate(void) {
      shared_ptr<Thread> th = thread_;

      if (th) {
        th->cancel(); // Send a termination signal to the thread.
        base_->join(); // Wait until the thread finishes.
      }

      base_.reset();
      /* Temporary object @th is required for completing the termination process
       * safely, because 'th->cancel()' subsequently calls 'this->set(0)'
       * (see 'terminate()' function in each thread implementation) and
       * that deletes @thread_ itself. So, if you wrote 'thread_->cancel()',
       * it can't work safely.
       */
    }

    bool is_running(void) const {
      return thread_!=0;
    }

  private:
    shared_ptr<Thread> thread_;
    shared_ptr<boost::thread> base_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // Factory function of ThreadManager.
  shared_ptr<ThreadManager> ThreadManager::create(void) {
    return shared_ptr<ThreadManager>(new ThreadManagerImpl);
  }

} // The end of the namespace "hashimoto_ut"
