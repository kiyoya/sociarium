// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2009/02/27

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   - Neither the name of the University of Tokyo nor the names of its contributors
 *     may be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INCLUDE_GUARD_SHARED_THREAD_H
#define INCLUDE_GUARD_SHARED_THREAD_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace hashimoto_ut {

  class Thread {
  public:
    Thread(void) :
    cancel_(false),
    suspend_(false)
    {}

    virtual ~Thread() {}

    void suspend(void) {
      boost::mutex::scoped_lock lock(mutex_);
      suspend_ = true;
    }

    void resume(void) {
      boost::mutex::scoped_lock lock(mutex_);
      suspend_ = false;
      condition_.notify_one();
    }

    void cancel(void) {
      cancel_ = true;
      suspend_ = false;
      condition_.notify_one();
    }

    // To be inserted into, for example, a while loop in operator()
    bool cancel_check(void) const {
      boost::mutex::scoped_lock lock(mutex_);
      if (suspend_) condition_.wait(lock);
      return cancel_;
    }
    
    virtual void operator()(void) = 0;

  private:
    bool cancel_;
    bool suspend_;
    mutable boost::mutex mutex_;
    mutable boost::condition condition_;
  };

} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SOCIARIUM_THREAD_H
