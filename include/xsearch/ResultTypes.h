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

struct BasePartialResult {
  // index of the data chunk from which the results were collected
  size_t _index = 0;

  bool operator>(const BasePartialResult& other) const {
    return _index > other._index;
  }
  bool operator<(const BasePartialResult& other) const {
    return _index < other._index;
  }
  bool operator==(const BasePartialResult& other) const {
    return _index == other._index;
  }
  bool operator!=(const BasePartialResult& other) const {
    return _index != other._index;
  }
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

  // TODO: merged result name is ambitious since the result is not really merged
  //  but just a vector of part res
  virtual std::vector<PartResT>& getMergedResult() {
    std::unique_lock lock(*_res_vec_mutex);
    return _merged_result;
  }

  PartResT& operator[](size_t index) {
    std::unique_lock lock(*_res_vec_mutex);
    return _merged_result[index];
  }

  [[nodiscard]] size_t size() const {
    std::unique_lock lock(*_res_vec_mutex);
    return _merged_result.size();
  }

 protected:
  std::unique_ptr<std::mutex> _res_vec_mutex = std::make_unique<std::mutex>();
  std::vector<PartResT> _merged_result;
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

 public:
  FullResult() = default;

  void addPartialResult(FullPartialResult partial_result) override;

 private:
  FullPartialResult _empty{};
};

// ----- count matches ---------------------------------------------------------
class CountResult : public BaseResult<uint64_t> {

 public:
  CountResult() = default;

  void addPartialResult(uint64_t partial_result) override;
  uint64_t getCount();

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

 public:
  MatchByteOffsetsResult() = default;

  void addPartialResult(IndexPartialResult partial_result) override;

 protected:
  IndexPartialResult _empty{};
};

// NOTE: LineByteOffsetsResult is actually the same as MatchByteOffsetsResult.
//  However, we need them to be different types to activate different
//  extern_search() templates...
class LineByteOffsetsResult : public MatchByteOffsetsResult {};

class LineIndexResult : public MatchByteOffsetsResult {};

// ===== searching lines that contain a match ==================================
struct LinesPartialResult : BasePartialResult {
  std::vector<std::string> lines;
};

class LinesResult : public BaseResult<LinesPartialResult> {

 public:
  LinesResult() = default;

  void addPartialResult(LinesPartialResult partial_result) override;

 protected:
  LinesPartialResult _empty{};
};

}  // namespace xs