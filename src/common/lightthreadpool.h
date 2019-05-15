//
// Created by User on 21.01.2019.
//

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
