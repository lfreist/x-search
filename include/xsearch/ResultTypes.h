/**
* Copyright 2023, Leon Freist (https://github.com/lfreist)
* Author: Leon Freist <freist.leon@gmail.com>
*
* This file is part of x-search.
*/

#pragma once

#include <xsearch/utils/Synchronized.h>

#include <vector>
#include <memory>
#include <mutex>
#include <tuple>

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
  Result() = default;
  ~Result() = default;

  /// not copyable
  Result(const Result&) = delete;
  Result& operator=(const Result&) = delete;

  /// movable
  Result(Result&&) noexcept = default;
  Result& operator=(Result&&) noexcept = default;

  bool add(PartResT pr) {
    if (_closed) {
      return false;
    }
    auto data = _data.wlock();
    data->push_back(std::move(pr));
    return true;
  }

  const std::vector<PartResT>& get_unsafe() const { return _data.get_unsafe(); }

  const std::vector<PartResT>& get() const { return get_unsafe(); }

  const PartResT& operator[](size_t index) const {
    auto data = _data.rlock();
    return data->operator[](index);
  }

  const PartResT& at(size_t index) const {
    auto data = _data.rlock();
    return data->at(index);
  }

  size_t size() const {
    auto data = _data.rlock();
    return data->size();
  }

  bool empty() const {
    auto data = _data.rlock();
    return data->empty();
  }

  bool is_closed() const { return _closed.load(); }

  void close() { _closed.store(true); }

 private:
  Synchronized<std::vector<PartResT>> _data;
  std::atomic<bool> _closed{false};
};

}