// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace xs {

// ----- Iterator class for Results --------------------------------------------
template <class ResultT, class PartResT>
class ResultIterator {
 public:
  ResultIterator(ResultT& result, size_t index) : _result(result) {
    _index = index;
  }

  PartResT& operator*() {
    std::unique_lock locker(*_result._mutex);
    while (_index >= _result._merged_result.size()) {
      if (_result._done) {
        // TODO: this is pretty ugly. I just need to stop the iterator one step
        //  earlier. Need to figure out how and where to implement it.
        return _result.getEmpty();
      }
      _result._cv->wait(locker);
    }
    return _result[_index];
  }

  ResultIterator<ResultT, PartResT>& operator++() {
    _index++;
    return *this;
  }

  ResultIterator<ResultT, PartResT>& operator--() {
    _index--;
    return *this;
  }

  bool operator!=(const ResultIterator<ResultT, PartResT>& other) {
    if (_result.isDone()) {
      return _index <= _result._merged_result.size();
    }
    return true;
  }

 private:
  ResultT& _result;
  size_t _index;
};
// -----------------------------------------------------------------------------

struct BasePartialResult {
  // index of the data chunk from which the results were collected
  size_t _index = 0;
};

template <class PartResT>
class BaseResult {
 public:
  BaseResult() = default;
  virtual ~BaseResult() = default;
  /**
   * adds a PartResT res to the _merged_result.
   * @param partial_result
   */
  virtual void addPartialResult(PartResT partial_result) = 0;
  virtual std::vector<PartResT>& getResult() { return _merged_result; }

  PartResT& operator[](size_t index) { return _merged_result[index]; }

  void markAsDone() {
    std::unique_lock locker(*_mutex);
    _done = true;
    _cv->notify_all();
  }

  bool isDone() {
    std::unique_lock locker(*_mutex);
    return _done;
  }

  // TODO: this corresponds to the issue of the iterator. remove this method as
  //  soon as i have fixed the iterator issue.
  virtual PartResT& getEmpty() = 0;

 protected:
  std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();
  std::vector<PartResT> _merged_result;
  bool _done = false;
};

// ===== Useful result types ===================================================

struct FullPartialResult : BasePartialResult {
  std::vector<uint64_t> _byte_offsets_match;
  std::vector<uint64_t> _byte_offsets_line;
  std::vector<uint64_t> _line_indices;
  std::vector<std::string> _lines;

  void merge(FullPartialResult& other);
};

// ----- full result -----------------------------------------------------------
class FullResult : public BaseResult<FullPartialResult> {
  friend class ResultIterator<FullResult, FullPartialResult>;

 public:
  FullResult() = default;

  void addPartialResult(FullPartialResult partial_result) override;

  ResultIterator<FullResult, FullPartialResult> begin();
  ResultIterator<FullResult, FullPartialResult> end();

  FullPartialResult& getEmpty() override;

 private:
  FullPartialResult _empty{};
};

// ----- count matches ---------------------------------------------------------
class CountResult : public BaseResult<uint64_t> {
  friend class ResultIterator<CountResult, uint64_t>;

 public:
  CountResult() = default;

  void addPartialResult(uint64_t partial_result) override;
  uint64_t getCount();

  ResultIterator<CountResult, uint64_t> begin();
  ResultIterator<CountResult, uint64_t> end();

  uint64_t& getEmpty() override;

 protected:
  uint64_t _sum_result = 0;
  uint64_t _empty = 0;
};

// This is actually the same as CountResult, but we need a different type for
// template specialization...
class CountLinesResult : public CountResult {};

// ----- byte positions --------------------------------------------------------
struct IndexPartialResult : BasePartialResult {
  std::vector<size_t> indices;

  void merge(IndexPartialResult other);
};

class MatchByteOffsetsResult : public BaseResult<IndexPartialResult> {
  friend class ResultIterator<MatchByteOffsetsResult, IndexPartialResult>;

 public:
  MatchByteOffsetsResult() = default;

  void addPartialResult(IndexPartialResult partial_result) override;

  ResultIterator<MatchByteOffsetsResult, IndexPartialResult> begin();
  ResultIterator<MatchByteOffsetsResult, IndexPartialResult> end();

  IndexPartialResult& getEmpty() override;

 protected:
  IndexPartialResult _empty{};
};

// NOTE: LineByteOffsetsResult is actually the same as MatchByteOffsetsResult.
//  However, we need them to be different types to activate different
//  extern_search() templates...
class LineByteOffsetsResult : public MatchByteOffsetsResult {};

// ===== searching lines that contain a match ==================================
struct LinesPartialResult : BasePartialResult {
  std::vector<std::string> lines;
};

class LinesResult : public BaseResult<LinesPartialResult> {
  friend class ResultIterator<LinesResult, LinesPartialResult>;

 public:
  LinesResult() = default;

  void addPartialResult(LinesPartialResult partial_result) override;

  ResultIterator<LinesResult, LinesPartialResult> begin();
  ResultIterator<LinesResult, LinesPartialResult> end();

  LinesPartialResult& getEmpty() override;

 protected:
  LinesPartialResult _empty{};
};

}  // namespace xs