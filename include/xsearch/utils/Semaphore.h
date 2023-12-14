// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <future>

namespace xs::utils {

template <typename F>
concept VoidCallable = requires(F f) {
  { std::invoke(f) } -> std::same_as<void>;
};

template <typename F>
concept NonVoidCallable = requires(F f) {
  { std::invoke(f) } -> std::same_as<std::invoke_result<F>>;
};

class Semaphore {
 public:
  explicit Semaphore(int max_workers)
      : _max_workers(max_workers < 1 ? 1 : max_workers) {}

  template <typename F>
  requires std::invocable<F>
  auto access(F func) {
    std::unique_lock lock(_mutex);
    _cv.wait(lock, [&]() { return _current_workers < _max_workers; });
    _current_workers++;
    lock.unlock();

    auto task = std::async(std::launch::async, func);

    lock.lock();
    _current_workers--;
    _cv.notify_all();

    if constexpr (std::is_void_v<decltype(func())>) {
      task.get();
    } else {
      return task.get();
    }
  }

 private:
  int _max_workers{1};
  int _current_workers{0};
  std::mutex _mutex;
  std::condition_variable _cv;
};

}  // namespace xs::utils