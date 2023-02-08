// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/utils/ad_utility/Synchronized.h>

#include <condition_variable>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

namespace xs {

template <typename T>
class BaseResult {
 public:
  BaseResult() = default;
  virtual ~BaseResult() = default;

  /**
   * adds a PartResT res to the _merged_result.
   * @param partial_result
   */
  virtual void add(std::vector<T> partial_result) {
    std::unique_lock lock(*_mutex);
    _data.withWriteLock([&](auto& data) {
           data.insert(data.end(),
                       std::make_move_iterator(partial_result.begin()),
                       std::make_move_iterator(partial_result.end()));
         });
    _cv->notify_one();
  }

  virtual void add(T partial_result) {
    std::unique_lock lock(*_mutex);
    _data.wlock()->push_back(std::move(partial_result));
    _cv->notify_one();
  }

  virtual void sortedAdd(std::vector<T> partial_result) {
    if (partial_result.empty()) {
      return;
    }
    std::unique_lock lock(*_mutex);
    _data.wlock()->insert(
        std::upper_bound(_data.rlock()->begin(), _data.rlock()->end(),
                         partial_result.front()),
        std::make_move_iterator(partial_result.begin()),
        std::make_move_iterator(partial_result.end()));
    _cv->notify_one();
  }

  virtual void sortedAdd(std::vector<T> partial_result, uint64_t result_index) {
    std::unique_lock lock(*_mutex);
    if (_current_index == result_index) {
      add(std::move(partial_result));
      _current_index++;
      // check if buffered results can be added now
      while (true) {
        auto search = _buffer.find(_current_index);
        if (search == _buffer.end()) {
          break;
        }
        add(std::move(search->second));
        _buffer.erase(_current_index++);
      }
      // at least one partial_result was added -> notify
      _cv->notify_one();
    } else {
      // buffer the partial result
      _buffer.insert({result_index, std::move(partial_result)});
    }
  }

  std::vector<T> copyResultSafe() {
    std::unique_lock lock(*_mutex);
    std::vector<T> tmp(_data.rlock()->size());
    _data.withWriteLock([&](auto& mr) { tmp.assign(mr.begin(), mr.end()); });
    return tmp;
  }

  auto* getSynchronizedResultPtr() { return &_data; }

  auto getLockedResult() { return _data.wlock(); }

  T operator[](size_t index) { return _data.rlock()->at(index); }

  [[nodiscard]] size_t size() const { return _data.rlock()->size(); }

  void done() {
    std::unique_lock lock(*_mutex);
    _done = true;
    _cv->notify_all();
  }

 protected:
  std::shared_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  std::shared_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();

  ad_utility::Synchronized<std::vector<T>> _data;
  std::unordered_map<uint64_t, std::vector<T>> _buffer;
  uint64_t _current_index = 0;
  bool _done = false;
};

/**
 *  We define custom types for different results in order to be able to start
 *   corresponding searches (line indices, byte offsets, lines, ...) template
 *   argument specific.
 */

// ----- count matches ---------------------------------------------------------
// For the CountResult, we also override the methods to sum up the values
//  instead of collecting them in a std::vector.
class CountMatchesResult : public BaseResult<uint64_t> {
 public:
  CountMatchesResult() = default;

  void add(std::vector<uint64_t> partial_result) override;
  void add(uint64_t partial_result) override;
  void sortedAdd(std::vector<uint64_t> partial_result) override;

  uint64_t getCount();

 protected:
  uint64_t _sum_result = 0;
};

// ----- count matching lines --------------------------------------------------
// we straight up inherit CountMatchesResult because all we need is a different
//  type, while functionalities and methods remain the same...
class CountLinesResult : public CountMatchesResult {};

// ----- search match byte offsets ---------------------------------------------
class MatchByteOffsetsResult : public BaseResult<uint64_t> {};

// ----- search line byte offsets ----------------------------------------------
class LineByteOffsetsResult : public BaseResult<uint64_t> {};

// ----- search indices of matching lines --------------------------------------
class LineIndicesResult : public BaseResult<uint64_t> {};

// ----- search matching lines -------------------------------------------------
class LinesResult : public BaseResult<std::string> {};

}  // namespace xs