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

#ifndef CUTCOIN_LIGHTTHREADPOOL_H
#define CUTCOIN_LIGHTTHREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <iostream>

namespace tools {

class LightThreadPool {
public:
  // Types
  using Function = std::function<void(void)>;

public:
  explicit LightThreadPool(size_t threads_n = std::thread::hardware_concurrency());
  void enqueue(const Function &work_item);
  ~LightThreadPool();

private:
  // need to keep track of threads so we can join them
  std::vector< std::thread > workers;
  // the task queue
  std::queue< std::function<void()> > tasks;

  // synchronization
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};

// the constructor just launches some amount of workers
inline
LightThreadPool::LightThreadPool(size_t threads_n)
: stop(false)
{
  for(size_t i = 0; i<threads_n; ++i)
    workers.emplace_back(
        [this] {
          for(;;) {
            Function task;

            {
              std::unique_lock<std::mutex> lock(this->queue_mutex);
              this->condition.wait(lock,
                                   [this]{ return this->stop || !this->tasks.empty(); });
              if(this->stop && this->tasks.empty())
                return;
              task = this->tasks.front();
              this->tasks.pop();
            }

            task();
          }
        }
    );
}

// add new work item to the pool
inline
void LightThreadPool::enqueue(const Function &work_item)
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.push(work_item);
  }
  condition.notify_one();
}

// the destructor joins all threads
inline LightThreadPool::~LightThreadPool()
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }

  condition.notify_all();
  for(std::thread &worker: workers) {
    worker.join();
  }
}

} // namespace tools

#endif //CUTCOIN_LIGHTTHREADPOOL_H
