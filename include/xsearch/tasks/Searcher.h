// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <re2/re2.h>
#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/utils/TSQueue.h>

namespace xs::tasks {

using count = uint64_t;
using byte_positions = std::vector<uint64_t>;
using line_numbers = std::vector<uint64_t>;
using line_indices = std::vector<uint64_t>;

template <class DataT, class ResultT>
class BaseSearcher {
 public:
  BaseSearcher() = default;
  virtual ~BaseSearcher() = default;

  virtual void search(const std::string& pattern, DataT* data,
                      ResultT* result) const = 0;
  virtual void search(re2::RE2* pattern, DataT* data,
                      ResultT* result) const = 0;
};

class MatchCounter : public BaseSearcher<DataChunk, PartialResult> {
 public:
  MatchCounter() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartialResult* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartialResult* result) const override;
};

class LineCounter : public BaseSearcher<DataChunk, PartialResult> {
 public:
  LineCounter() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartialResult* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartialResult* result) const override;
};

class MatchBytePositionSearcher
    : public BaseSearcher<DataChunk, PartialResult> {
 public:
  MatchBytePositionSearcher() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartialResult* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartialResult* result) const override;
};

class LineBytePositionSearcher : public BaseSearcher<DataChunk, PartialResult> {
 public:
  LineBytePositionSearcher() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartialResult* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartialResult* result) const override;
};

class LineIndexSearcher : public BaseSearcher<DataChunk, PartialResult> {
 public:
  LineIndexSearcher() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartialResult* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartialResult* result) const override;
};

class LinesSearcher : public BaseSearcher<DataChunk, PartialResult> {
 public:
  LinesSearcher() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartialResult* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartialResult* result) const override;
};

}  // namespace xs::tasks