// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

namespace xs::utils {

/**
 * @brief thread safe queue using std::queue
 *
 */
template <class T>
class TSQueue {
 public:
  [[maybe_unused]] explicit TSQueue(unsigned maxSize = 100) {
    _maxSize = maxSize;
  }

  /**
   * @brief push an element of type T to queue
   *
   * @param element
   */
  void push(T element) {
    if (isClosed()) {
      throw std::runtime_error("Trying to push to a closed queue.");
    }
    std::unique_lock lockQueue(_queueMutex);
    while (_queue.size() >= _maxSize) {
      _pushCondVar.wait(lockQueue);
    }
    _queue.push(std::move(element));
    _popCondVar.notify_one();
  }

  /**
   * Push element to queue. If the queue is at minimum size of maxSize / 2,
   * push_warn_flag is set to true. If queue is at maxSize and allow_oversize
   * is true, data are pushed anyways!
   * @param element
   * @param push_warn_flag
   * @param allow_oversize
   */
  void push(T element, bool* push_warn_flag, bool allow_oversize = false) {
    if (isClosed()) {
      throw std::runtime_error("Trying to push to a closed queue.");
    }
    std::unique_lock lockQueue(_queueMutex);
    if (!allow_oversize) {
      while (_queue.size() >= _maxSize) {
        _pushCondVar.wait(lockQueue);
      }
    }
    _queue.push(std::move(element));
    *push_warn_flag = (_queue.size() >= (_maxSize / 2));
    _popCondVar.notify_one();
  }

  /**
   * @brief pop an element from queue. Note that in contrast to std::queue, this
   * pop returns the first element. If the queue is empty, pop() waits for a
   * push() call. If the queue is empty and the queue was closed, pop() returns
   *  empty optional.
   *
   * @return T
   */
  std::optional<T> pop() {
    std::unique_lock lockQueue(_queueMutex);
    while (_queue.empty()) {
      if (isClosed()) {
        return {};
      }
      _popCondVar.wait(lockQueue);
    }
    T element = std::move(_queue.front());
    _queue.pop();
    _pushCondVar.notify_one();
    return element;
  }

  /**
   * Pop element from queue. If queue is empty but not closed, pop_failed_flag
   * is set to true.
   * @param pop_failed_flag
   * @return
   */
  std::optional<T> pop(bool* pop_failed_flag) {
    std::unique_lock lockQueue(_queueMutex);
    if (_queue.empty()) {
      if (isClosed()) {
        *pop_failed_flag = false;
        return {};
      }
      *pop_failed_flag = true;
      return {};
    }
    T element = std::move(_queue.front());
    _queue.pop();
    _pushCondVar.notify_one();
    *pop_failed_flag = false;
    return element;
  }

  /**
   * @brief returns, whether the Queue is empty or not
   *
   * @return bool
   */
  bool empty() const {
    std::lock_guard lockQueue(_queueMutex);
    return _queue.empty();
  }

  /**
   * @brief returns number of elements in the queue
   *
   * @return size_t
   */
  size_t size() const {
    std::lock_guard lockQueue(_queueMutex);
    return _queue.size();
  }

  /**
   * @brief closes the queue -> no pushes possible anymore
   *
   */
  void close() {
    if (isClosed()) {
      return;
    }
    std::unique_lock lockClosed(_closedMutex);
    _closed = true;
    _popCondVar.notify_all();
  }

  bool isClosed() {
    std::unique_lock<std::mutex> lockClosed(_closedMutex);
    return _closed;
  }

  void reset() {
    std::unique_lock lockClosed(_closedMutex);
    _closed = false;
    _queue = std::queue<T>();
  }

  bool isFull() { return size() >= _maxSize; }

 private:
  unsigned _maxSize;
  bool _closed = false;

  std::queue<T> _queue;

  mutable std::mutex _queueMutex;
  mutable std::mutex _closedMutex;

  std::condition_variable _popCondVar;
  std::condition_variable _pushCondVar;
};

}  // namespace xs::utils