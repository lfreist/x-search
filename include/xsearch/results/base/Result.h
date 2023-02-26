// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/utils/ad_utility/Synchronized.h>

#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace xs::result::base {

template <typename T>
class Result {
 public:
  Result() = default;
  virtual ~Result() = default;

  virtual void add(T partial_result) = 0;
  virtual void add(T partial_result, uint64_t id) = 0;

  [[nodiscard]] virtual size_t size() const = 0;

  void done() {
    std::unique_lock lock(*_mutex);
    _done = true;
    _cv->notify_all();
  }

 protected:
  std::shared_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  std::shared_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();
  bool _done = false;
};

template <typename T>
class ContainerResult : public Result<std::vector<T>> {
 public:
  template <typename R>
  class iterator {
   public:
    iterator(R& result, size_t index) : _result(result), _index(index) {}

    T operator*() { return _result[_index]; }

    iterator& operator++() {
      _index++;
      return *this;
    }

    bool operator!=(const iterator& other) {
      std::unique_lock lock(*_result._mutex);
      while (_index >= _result.size()) {
        if (_result._done) {
          return false;
        }
        _result._cv->wait(lock);
      }
      if (_result._done) {
        return _index <= _result.size();
      }
      return true;
    }

   private:
    R& _result;
    size_t _index;
  };

  using Iterator = iterator<ContainerResult<T>>;

 public:
  ContainerResult() = default;

  /**
   * adds a PartResT res to the _merged_result.
   * @param partial_result
   */
  void add(std::vector<T> partial_result) override {
    std::unique_lock lock(*this->_mutex);
    _data.withWriteLock([&](auto& data) {
      data.insert(data.end(), std::make_move_iterator(partial_result.begin()),
                  std::make_move_iterator(partial_result.end()));
    });
    this->_cv->notify_one();
  }

  void add(std::vector<T> partial_result, uint64_t id) override {
    add(std::move(partial_result));
  }

  std::vector<T> copyResultSafe() {
    std::unique_lock lock(*this->_mutex);
    std::vector<T> tmp(_data.rlock()->size());
    _data.withWriteLock([&](auto& mr) { tmp.assign(mr.begin(), mr.end()); });
    return tmp;
  }

  auto* getSynchronizedResultPtr() { return &_data; }

  auto getLockedResult() { return _data.wlock(); }

  T operator[](size_t index) { return _data.rlock()->at(index); }

  [[nodiscard]] size_t size() const override { return _data.rlock()->size(); }

  virtual Iterator begin() { return Iterator(*this, 0); }

  virtual Iterator end() { return Iterator(*this, 0); }

 protected:
  ad_utility::Synchronized<std::vector<T>> _data;
};

/**
 *  We define custom types for different results in order to be able to start
 *   corresponding searches (line indices, byte offsets, lines, ...) template
 *   argument specific.
 */

// ----- count results ---------------------------------------------------------
// For the CountResult, we also override the methods to sum up the values
//  instead of collecting them in a std::vector.
class CountResult : public Result<uint64_t> {
 public:
  class iterator {
   public:
    iterator(CountResult& result, size_t index)
        : _result(result), _index(index) {}

    uint64_t operator*() {
      std::unique_lock lock(*_result._mutex);
      return _result._sum_result;
    }

    iterator& operator++() {
      _index++;
      return *this;
    }

    bool operator!=(const iterator& other) {
      std::unique_lock lock(*_result._mutex);
      _result._cv->wait(lock);
      if (_result._done) {
        return false;
      }
      return true;
    }

   private:
    CountResult& _result;
    size_t _index;
  };

 public:
  CountResult() = default;

  void add(uint64_t partial_result) override;
  void add(uint64_t partial_result, uint64_t id) override;

  [[nodiscard]] size_t size() const override;

  iterator begin() { return {*this, 0}; }

  iterator end() { return {*this, 0}; }

 protected:
  uint64_t _sum_result = 0;
};

}  // namespace xs::result::base