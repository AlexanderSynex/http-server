#include "snx/thread_pool.hpp"
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>

using namespace snx;

thread_pool::thread_pool(std::size_t N) {
  workers.reserve(N);
  for (auto i = 0; i < N; ++i) {
    workers.emplace_back(
        std::bind(&thread_pool::process_loop, this, stop_trigger.get_token()));
  }
}

thread_pool::~thread_pool() {
  {
    std::lock_guard l(m);
    stop_trigger.request_stop();
  }
  job_trigger.notify_all();
}

void thread_pool::process_loop(std::stop_token stoken) {
  callback_type job;
  for (; not stoken.stop_requested();) {
    {
      std::unique_lock l(m);
      job_trigger.wait(l, [&jobs = jobs, &stoken]() -> bool {
        return not jobs.empty() or stoken.stop_requested();
      });
      if (stoken.stop_requested() and jobs.empty()) {
        return;
      }
      job = std::move(jobs.front());
      jobs.pop();
    }
    job();
  }
}

void thread_pool::reset() {
  for (auto &thread : workers) {
    thread = std::jthread(
        std::bind(&thread_pool::process_loop, this, stop_trigger.get_token()));
  }
}
