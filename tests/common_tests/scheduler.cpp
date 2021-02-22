// Copyright (c) 2018-2021, CUT coin
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

#include "common/scheduler.h"

#include "common/lightthreadpool.h"

#include <chrono>
#include <random>

#include "gtest/gtest.h"

TEST(scheduler, breathing)
{
  // Concerns: object creation and destruction.

  tools::LightThreadPool threadPool(5);
  tools::Scheduler s(threadPool);
}

TEST(scheduler, delayed_jobs)
{
  // Concerns: tasks scheduling itself, riming and order correctness.
  // TODO: Add custom timer to the scheduler to be independent on system timer (for testing purposes only).
  {
    using namespace std::chrono;

    size_t NUM_TESTS = 30;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1000);

    system_clock::duration epsilon = milliseconds(500);

    tools::LightThreadPool threadPool(NUM_TESTS);
    tools::Scheduler s(threadPool);
    s.start();

    size_t executionCounter{ 0 };
    std::mutex counterMutex;
    std::condition_variable cv;

    for (size_t i = 0; i < NUM_TESTS; i++) {
      auto time_now = system_clock::now();
      auto time_to_start = time_now + milliseconds(50 + dist(gen));
      tools::Scheduler::SharedTask task = s.schedule_at([&, time_to_start, epsilon]() {
                                                          auto duration = duration_cast<microseconds>(system_clock::now() - time_to_start);
                                                          EXPECT_LE(duration, epsilon);
                                                          std::lock_guard<std::mutex> lock(counterMutex);
                                                          ++executionCounter;
                                                          cv.notify_all();
                                                          return;
                                                        },
                                                        time_to_start);
    }

    std::unique_lock<std::mutex> lock(counterMutex);
    while (executionCounter < NUM_TESTS) {
      cv.wait(lock);
    }
  }
}
//
//TEST(scheduler, recurrent_jobs)
//{
//  tools::Scheduler<tools::threadpool> s(tools::threadpool::getInstance());
//}
//
//TEST(scheduler, recursive_scheduling)
//{
//  //TODO
//}
//
//TEST(scheduler, highload)
//{
//  //TODO
//}
