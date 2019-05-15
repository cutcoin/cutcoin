// Copyright (c) 2019, CUT coin
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

#include "sharedlock.h"

#include <mutex>

namespace tools
{

void SharedLock::lock()
{
  d_mutex.lock();
}

void SharedLock::unlock()
{
  d_mutex.unlock();
}

std::unique_lock<std::mutex> SharedLock::get_lock()
{
  return std::unique_lock<std::mutex>(d_mutex);
}

void SharedLock::notify_one()
{
  d_mutex.lock();
  d_cond_var.notify_one();
}

void SharedLock::notify_all()
{
  d_mutex.lock();
  d_cond_var.notify_all();
}

//SharedLock::ScopeLock SharedLock::get_scope_lock()
//{
//  return ScopeLock(*this, false);
//}
//
//SharedLock::ScopeLock SharedLock::get_scope_lock_with_notification()
//{
//  return ScopeLock(*this, true);
//}
//
//SharedLock::ScopeLock::ScopeLock(ScopeLock &&other)
//: d_shared_lock(other.d_shared_lock)
//, d_lock(std::move(other.d_lock))
//, d_notify_all(other.d_notify_all)
//{
//
//}
//
//SharedLock::ScopeLock::ScopeLock(SharedLock &shared_lock, bool notify_all)
//: d_shared_lock(shared_lock)
//, d_lock(std::unique_lock<std::mutex>(shared_lock.d_mutex))
//, d_notify_all(notify_all)
//{
//}
//
//SharedLock::ScopeLock::~ScopeLock()
//{
//  if (d_notify_all) {
//    d_shared_lock.d_cond_var.notify_all();
//  }
//}

}
