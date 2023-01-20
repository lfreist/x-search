// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <string>
#include <utility>
#include <vector>

namespace xs {

struct PartialResult {
  size_t _index;
  uint64_t _count;
  std::vector<uint64_t> _byte_offsets;
  std::vector<uint64_t> _line_indices;
  std::vector<std::string> _lines;

  void merge(PartialResult& other) {
    _count += other._count;
    _byte_offsets.insert(_byte_offsets.end(), other._byte_offsets.begin(),
                         other._byte_offsets.end());
    _line_indices.insert(_line_indices.end(), other._line_indices.begin(),
                         other._line_indices.end());
    _lines.insert(_lines.end(), other._lines.begin(), other._lines.end());
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
  virtual void addPartialResult(PartResT& partial_result) = 0;
  virtual PartResT& getResult() = 0;

 protected:
  PartResT _merged_result;
};

template <class PartResT = PartialResult>
class Result : public BaseResult<PartResT> {
 public:
  Result() = default;
  explicit Result(std::string pattern, bool regex)
      : _pattern(std::move(pattern)) {
    _regex = regex;
  }

  void addPartialResult(PartResT& partial_result) override {
    this->_merged_result.merge(partial_result);
  }

  PartResT& getResult() override { return this->_merged_result; }

 private:
  std::string _pattern;
  bool _regex = false;
};

typedef Result<PartialResult> DefaultResult;

}  // namespace xs