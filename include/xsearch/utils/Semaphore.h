// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace xs::utils {

template <typename T, typename = std::enable_if_t<std::is_invocable_v<T>>>
class Semaphore {
 public:
  explicit Semaphore(int max_workers)
      : _max_workers(max_workers < 1 ? 1 : max_workers) {}

  void access(T func) {
    std::unique_lock lock(_mutex);
    _cv.wait(lock, [&]() { return _current_workers < _max_workers; });
    _current_workers++;
    lock.unlock();

    func();

    lock.lock();
    _current_workers--;
    _cv.notify_all();
  }

 private:
  int _max_workers{1};
  int _current_workers{0};
  std::mutex _mutex;
  std::condition_variable _cv;
};

}  // namespace xs::utils