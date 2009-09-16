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
#ifdef _MSC_VER
#include <array>
#else
#include <tr1/array>
#endif
#include <string>
#include "thread.h"
#include "language.h"
#include "../shared/thread.h"

namespace hashimoto_ut {

  using std::deque;
  using std::wstring;
  using std::tr1::array;
  using std::tr1::shared_ptr;

  using namespace sociarium_project_language;

  namespace sociarium_project_thread {

    namespace {

      int const NUMBER_OF_THREAD_MESSAGE_HIERARCHIES = 2;

      array<deque<wstring>, NUMBER_OF_THREAD_CATEGORIES> status;
      array<boost::thread, NUMBER_OF_THREAD_CATEGORIES> thread_manager;
      array<shared_ptr<Thread>, NUMBER_OF_THREAD_CATEGORIES> thread_function;

    } // The end of the anonymous namespace


    ////////////////////////////////////////////////////////////////////////////////
    void initialize(void) {
      for (int i=0; i<NUMBER_OF_THREAD_CATEGORIES; ++i) {
        status[i].resize(NUMBER_OF_THREAD_MESSAGE_HIERARCHIES);
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void finalize(void) {
      for (int i=0; i<NUMBER_OF_THREAD_CATEGORIES; ++i) {

        boost::thread& tm = thread_manager[i];
        shared_ptr<Thread> tf = thread_function[i];

        if (tf) tf->cancel();
        if (tm.joinable()) tm.join();
      }
    }


    ////////////////////////////////////////////////////////////////////////////////
    void invoke(int thread_id, shared_ptr<Thread> tf_new) {

      assert(0<=thread_id && thread_id<NUMBER_OF_THREAD_CATEGORIES);

      boost::thread& tm = thread_manager[thread_id];
      shared_ptr<Thread>& tf = thread_function[thread_id];

      if (tf) tf->cancel();
      if (tm.joinable()) tm.join();

      tf = tf_new;
      boost::thread(boost::ref(*tf)).swap(tm);
    }


    ////////////////////////////////////////////////////////////////////////////////
    void join(int thread_id) {
      assert(0<=thread_id && thread_id<NUMBER_OF_THREAD_CATEGORIES);
      return thread_manager[thread_id].join();
    }


    ////////////////////////////////////////////////////////////////////////////////
    bool joinable(int thread_id) {
      assert(0<=thread_id && thread_id<NUMBER_OF_THREAD_CATEGORIES);
      return thread_manager[thread_id].joinable();
    }


    ////////////////////////////////////////////////////////////////////////////////
    void detach(int thread_id) {
      assert(0<=thread_id && thread_id<NUMBER_OF_THREAD_CATEGORIES);
      return thread_manager[thread_id].detach();
    }


    ////////////////////////////////////////////////////////////////////////////////
    shared_ptr<Thread> get_thread_function(int thread_id) {
      assert(0<=thread_id && thread_id<NUMBER_OF_THREAD_CATEGORIES);
      return thread_function[thread_id];
    }


    ////////////////////////////////////////////////////////////////////////////////
    deque<wstring>& get_status(int thread_id) {
      assert(0<=thread_id && thread_id<NUMBER_OF_THREAD_CATEGORIES);
      return status[thread_id];
    }

  } // The end of the namespace "sociarium_project_thread"

} // The end of the namespace "hashimoto_ut"
