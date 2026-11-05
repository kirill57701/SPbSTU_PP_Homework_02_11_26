#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>
#include <iostream>
#include <syncstream>
#include <chrono>

struct task_t { void operator()(); };

struct th_pool_t {
  using f_t = std::function< void(void) >;
  explicit th_pool_t(unsigned int k);
  ~th_pool_t();
  void submit(f_t f);
  void wait();
  void cancel();
private:
  void execute();
  std::queue< f_t > tasks_;
  std::mutex mx_;
  std::condition_variable cd_;
  std::condition_variable idle_;
  bool stop_{false};
  size_t busy_{0};
  std::vector< std::thread > ths_;
};

void th_pool_t::submit(f_t f) {
  mx_.lock(); tasks_.push(std::move(f)); mx_.unlock();
  cd_.notify_one();
}

void th_pool_t::wait() {
  std::unique_lock lck(mx_);
  idle_.wait(lck, [&]() { return tasks_.empty() && !busy_; });
}
