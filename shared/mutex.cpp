// mutex.cpp
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

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include "mutex.h"

namespace hashimoto_ut {

  ////////////////////////////////////////////////////////////////////////////////
  // Mutex Implementations.
  class Mutex::MutexImpl {
  public:
    MutexImpl(void)
         : number_of_reading_blocks_(0),
           number_of_writing_blocks_(0) {}

    ~MutexImpl() {}

    void read_lock(void) const {
      boost::mutex::scoped_lock lock(mutex_);
      while (number_of_writing_blocks_>0)
        condition_.wait(lock);
      ++number_of_reading_blocks_;
    }

    void write_lock(void) const {
      boost::mutex::scoped_lock lock(mutex_);
      while (number_of_reading_blocks_>0 || number_of_writing_blocks_>0)
        condition_.wait(lock);
      ++number_of_writing_blocks_;
    }

    void read_to_write_lock(void) const {
      boost::mutex::scoped_lock lock(mutex_);
      while (number_of_writing_blocks_>0)
        condition_.wait(lock);
      ++number_of_writing_blocks_;
      while (number_of_reading_blocks_>1)
        condition_.wait(lock);
      --number_of_reading_blocks_;
    }

    void write_to_read_lock(void) const {
      boost::mutex::scoped_lock lock(mutex_);
      ++number_of_reading_blocks_;
      --number_of_writing_blocks_;
      condition_.notify_all();
    }

    void read_unlock(void) const {
      boost::mutex::scoped_lock lock(mutex_);
      --number_of_reading_blocks_;
      condition_.notify_all();
    }

    void write_unlock(void) const {
      boost::mutex::scoped_lock lock(mutex_);
      --number_of_writing_blocks_;
      condition_.notify_all();
    }

  private:
    mutable boost::mutex mutex_;
    mutable boost::condition condition_;
    mutable volatile int number_of_reading_blocks_;
    mutable volatile int number_of_writing_blocks_;
  };


  ////////////////////////////////////////////////////////////////////////////////
  // The class 'Mutex' delegates its methods to concrete implementations.
  Mutex::Mutex(void) : impl_(new MutexImpl()) {}
  Mutex::~Mutex() {}
  void Mutex::read_lock(void) const { impl_->read_lock(); }
  void Mutex::write_lock(void) const { impl_->write_lock(); }
  void Mutex::read_to_write_lock(void) const { impl_->read_to_write_lock(); }
  void Mutex::write_to_read_lock(void) const { impl_->write_to_read_lock(); }
  void Mutex::read_unlock(void) const { impl_->read_unlock(); }
  void Mutex::write_unlock(void) const { impl_->write_unlock(); }

} // The end of the namespace "hashimoto_ut"
