// Copyright (c) 2018-2020, CUT coin
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

#include "scheduler.h"

#include "common/lightthreadpool.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <utility>

namespace tools {

Scheduler::Task::Task(const Scheduler::Job& job, const Scheduler::Timepoint& start)
: d_job{job}
, d_start{start}
, d_is_recursive{false}
, d_period{Scheduler::Period::zero()}
{
}

Scheduler::Task::Task(const Scheduler::Job& job,
                      const Scheduler::Timepoint& start,
                      bool is_recursive,
                      const Scheduler::Period& period)
: d_job{job}
, d_start{start}
, d_is_recursive{is_recursive}
, d_period{period}
{
}

Scheduler::Task::Task(const Scheduler::Task& other)
: d_job(other.d_job)
, d_start(other.d_start)
, d_is_recursive(other.d_is_recursive)
, d_period(other.d_is_recursive)
{
}

Scheduler::Job Scheduler::Task::job()
{
  return this->d_job;
}

Scheduler::Scheduler(LightThreadPool& executor_pool)
    : d_is_running{false}
    , d_executor_pool(executor_pool)
{
}

Scheduler::~Scheduler()
{
  shutdown();
}

Scheduler::SharedTask Scheduler::schedule_now(Scheduler::Job job)
{
  SharedTask task = std::make_shared<Scheduler::Task>(job, Clock::now());
  add_task(task);
  return task;
}

Scheduler::SharedTask Scheduler::schedule_at(Scheduler::Job job, const Scheduler::Timepoint &time_point)
{
  SharedTask task = std::make_shared<Scheduler::Task>(job, time_point);
  add_task(task);
  return task;
}

Scheduler::SharedTask Scheduler::schedule_every(Scheduler::Job job,
                                                const Scheduler::Timepoint &time_point,
                                                const Scheduler::Period &time_delta)
{
  SharedTask task = std::make_shared<Scheduler::Task>(job, time_point, true, time_delta);
  add_task(task);
  return task;
}

bool Scheduler::remove(SharedTask task)
{
  std::lock_guard<std::mutex> lock(d_state_mutex);
  return d_tasks.erase(task) > 0;
}

void Scheduler::start()
{
  d_executor_pool.enqueue([this] {
    d_is_running = true;
    for(;;) {
      SharedTask task(nullptr);
      {
        std::unique_lock<std::mutex> lock(d_state_mutex);
        if (d_tasks.empty()) {
          this->d_awake_condition.wait(lock,
              [this]{ return !d_is_running || !this->d_tasks.empty(); });
        } else {
          auto start_time = (*d_tasks.begin())->start();
          this->d_awake_condition.wait_until(lock, start_time,
              [this, start_time]{ return !d_is_running || start_time <= Clock::now(); });
        }

        if (!d_is_running) {
          return;
        }

        if (!this->d_tasks.empty() && (*d_tasks.begin())->start() <= Clock::now()) {
          task = *d_tasks.begin();
          d_tasks.erase(d_tasks.begin());

          if (task->is_recursive()) {
            d_tasks.insert(std::make_shared<Task>(
                task->job(),
                task->start() + task->period(),
                task->is_recursive(),
                task->period()));
          }
        }
      }
      if (task) {
        d_executor_pool.enqueue(task->job());
      }
    }
  });
}

void Scheduler::shutdown()
{
  d_is_running = false;
  d_tasks.clear();
  d_awake_condition.notify_one();

}

void Scheduler::clear()
{
  d_tasks.clear();
}

void Scheduler::add_task(SharedTask task)
{
  std::lock_guard<std::mutex> lock(d_state_mutex);
  d_tasks.insert(task);
  d_awake_condition.notify_one();
}

} // namespace tools