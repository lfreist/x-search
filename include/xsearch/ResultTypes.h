/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of x-search.
 */

#pragma once

#include <xsearch/utils/Synchronized.h>

#include <memory>
#include <mutex>
#include <tuple>
#include <vector>

namespace xs {

template <typename T>
using PartRes1 = std::vector<T>;

template <typename T0, typename T1>
using PartRes2 = std::vector<std::tuple<T0, T1>>;

template <typename T0, typename T1, typename T2>
using PartRes3 = std::vector<std::tuple<T0, T1, T2>>;

template <typename T0, typename T1, typename T2, typename T3>
using PartRes4 = std::vector<std::tuple<T0, T1, T2, T3>>;

template <typename PartResT>
class Result {
 public:
  template <typename R>
  class iterator {
   public:
    explicit iterator(R& result) : _result(result), _current_index(0) {}

    iterator& operator++() {
      _current_index++;
      return *this;
    }

    PartResT& operator*() {
      return _result[_current_index];
    }

    bool operator!=(const iterator& other) {
      std::unique_lock lock(*_result._m);
      while (_current_index >= _result.get_unsafe().size()) {
        if (_result.is_closed()) {
          return false;
        }
        _result._cv->wait(lock);
      }
      if (_result.is_closed()) {
        return _current_index <= _result.get_unsafe().size();
      }
      return true;
    }

   private:
    size_t _current_index;
    R& _result;
  };

  using Iterator = iterator<Result<PartResT>>;

 public:
  Result() = default;
  ~Result() = default;

  /// not copyable
  Result(const Result&) = delete;
  Result& operator=(const Result&) = delete;

  /// movable
  Result(Result&& other) noexcept = default;
  Result& operator=(Result&&) noexcept = default;

  bool add(PartResT pr) {
    if (is_closed()) {
      return false;
    }
    std::unique_lock lock(*_m);
    _data.push_back(std::move(pr));
    _cv->notify_one();
    return true;
  }

  const std::vector<PartResT>& get_unsafe() const { return _data; }

  std::vector<PartResT> get() const { return _data; }

  const PartResT& operator[](size_t index) const {
    std::unique_lock lock(*_m);
    return _data[index];
  }

  const PartResT& at(size_t index) const {
    std::unique_lock lock(*_m);
    return _data.at(index);
  }

  size_t size() const {
    std::unique_lock lock(*_m);
    return _data.size();
  }

  bool empty() const {
    std::unique_lock lock(*_m);
    return _data.empty();
  }

  bool is_closed() const { return _closed.load(); }

  void close() {
    _closed.store(true);
    _cv->notify_all();
  }

  Iterator begin() { return Iterator(*this); }
  Iterator end() { return Iterator(*this); }

 private:
  std::vector<PartResT> _data;
  std::atomic<bool> _closed{false};
  std::unique_ptr<std::mutex> _m;
  std::unique_ptr<std::condition_variable> _cv;
};

}  // namespace xs