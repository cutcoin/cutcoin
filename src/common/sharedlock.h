// Copyright (c) 2019-2022, CUT coin
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef CUTCOIN_SHAREDLOCK_H
#define CUTCOIN_SHAREDLOCK_H

#include <condition_variable>
#include <memory>
#include <mutex>

namespace tools {

class SharedLock {

public:
  // Manipulators

  void lock();
  void unlock();
  void notify_one();
  void notify_all();
  std::unique_lock<std::mutex> get_lock();

  template< typename Rep, typename Period >
  void wait_for(std::unique_lock<std::mutex> &lock, const std::chrono::duration<Rep, Period> &rel_time);

private:
  std::mutex                    d_mutex;     // mutex used in the shared lock
  std::condition_variable       d_cond_var;  // condition variable used to wake up threads
};

template< typename Rep, typename Period >
void SharedLock::wait_for(std::unique_lock<std::mutex> &lock, const std::chrono::duration<Rep, Period> &rel_time)
{
  d_cond_var.wait_for(lock, rel_time);
}

}

#endif //CUTCOIN_SHAREDLOCK_H
