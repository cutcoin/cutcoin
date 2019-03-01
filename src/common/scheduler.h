// Copyright (c) 2018-2019, CUT coin
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

// Provide lightweight job scheduler based on C++ 11 threading.
// The scheduler expects a thread (or executor) pool with the simple call interface as the template parameter:
//
// void submit(waiter *waiter, std::function<void()> f, bool leaf = false);
//


#ifndef CUTCOIN_SCHEDULER_H
#define CUTCOIN_SCHEDULER_H

#include "common/lightthreadpool.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <set>

namespace tools {

class Scheduler {
private:
  // Forward declarations
  class SharedTaskComparator;
  class Task;

public:
  // Class type definitions

  using Job =  std::function<void(void)>;
  // Represent callable object for scheduling

  using Clock = std::chrono::system_clock;
  using Timepoint = Clock::time_point;
  using Period = Clock::duration;

  using SharedTask = std::shared_ptr<Task>;
  using TaskContainer = std::multiset<SharedTask, SharedTaskComparator>;

private:
  class SharedTaskComparator {
  public:
    inline bool operator()(const SharedTask& l, const SharedTask& r) const {
      return l->start() < r->start();
    }
  };

  class Task {
  public:
    Job d_job;
    Timepoint d_start;
    bool d_is_recursive;
    Period d_period;

  public:
    // Creators
    explicit Task(const Job& job, const Timepoint& start);

    explicit Task(const Job& job, const Timepoint& start, bool is_recursive, const Period& period);

    Task(const Task& other);

  public:
    // Member functions
    inline bool is_recursive() const;

    inline Timepoint start() const;

    inline Period period() const;

    Job job();

  public:
    // Deleted members
    Task& operator=(const Task& other) = delete;

    Task& operator=(Task&& other) = delete;

    Task(Task&& other) = delete;
  };

private:
  std::atomic<bool> d_is_running;
  TaskContainer d_tasks;
  LightThreadPool& d_executor_pool;
  std::mutex d_state_mutex;
  std::condition_variable d_awake_condition;

public:
  // Creators
  explicit Scheduler(LightThreadPool& executor_pool);
  // Create this object.

  ~Scheduler();
  // Destroy this object.

public:
  // Member functions

  SharedTask schedule_now(Scheduler::Job job);
  // Schedule execution of the 'function' now.

  SharedTask schedule_at(Job job, const Timepoint &time_point);
  // Schedule execution of the 'function' at the specified 'time_point'.

  SharedTask schedule_every(Job job, const Timepoint &time_point, const Period &time_delta);
  // Schedule execution of the 'function' every 'time_delta'.

  bool remove(SharedTask task);
  // Remove the specified 'task' from the schedule and reschedule if needed.
  // Return 'true' if erased at least one element and 'false' otherwise.

  void start();
  // Start scheduling loop.

  void shutdown();
  // Clear scheduled tasks and stop the scheduling loop.

  void clear();
  // Clear scheduled tasks.


private:
  void add_task(SharedTask task);
  // Add the specified 'task' to the container and notify the scheduling loop about that.
};

inline
bool Scheduler::Task::is_recursive() const
{
  return this->d_is_recursive;
}

inline
Scheduler::Timepoint Scheduler::Task::start() const
{
  return this->d_start;
}

inline
Scheduler::Period Scheduler::Task::period() const
{
  return this->d_period;
}

} // namespace tools

#endif //CUTCOIN_SCHEDULER_H
