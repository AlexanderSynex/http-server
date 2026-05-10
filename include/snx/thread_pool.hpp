#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

namespace snx {
class thread_pool {
 public:
  using callback_type = std::move_only_function<void()>;

  explicit thread_pool(std::size_t N);
  thread_pool(const thread_pool &) = delete;
  thread_pool(thread_pool &&) = delete;
  thread_pool &operator=(const thread_pool &) = delete;
  thread_pool &operator=(thread_pool &&) = delete;
  ~thread_pool();

  template <typename Func, typename... Args>
  void attach(Func &&, Args &&...args);

  void reset();

 protected:
  void process_loop(std::stop_token);

 private:
  std::mutex m;
  std::condition_variable_any job_trigger;
  std::queue<callback_type> jobs = {};
  std::stop_source stop_trigger;
  std::vector<std::jthread> workers;
};
}  // namespace snx

template <typename Func, typename... Args>
void snx::thread_pool::attach(Func &&job, Args &&...args) {
  {
    std::lock_guard l(m);
    jobs.push([functor = std::forward<Func>(job),
               ... args = std::forward<Args>(args)]() {
      functor(std::forward<Args>(args)...);
    });
  }
  job_trigger.notify_one();
}
