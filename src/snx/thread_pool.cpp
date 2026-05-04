#include "snx/thread_pool.hpp"
#include "snx/logging/logger.hpp"
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
  auto job = std::function<void()>{};
  for (; not stoken.stop_requested();) {
    {
      std::unique_lock l(m);
      job_trigger.wait(l, stoken,
                       [&jobs = jobs]() -> bool { return not jobs.empty(); });
      if (stoken.stop_requested() and jobs.empty()) {
        return;
      }

      job = jobs.front();
      jobs.pop();
    }
    job();
  }
}