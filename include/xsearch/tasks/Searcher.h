// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

/**
 * MARK: We have considered implementing different searchers template specific
 *  (similar to the result types) but decided to use inheritance instead.
 *  The reason for this is, that we want developers to be able to add their
 *  own searchers and this is more intuitive by inheriting a BaseSearcher class
 *  rather than adding template specialized implementations of searcher methods.
 */

#pragma once

#include <re2/re2.h>
#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/utils/TSQueue.h>

namespace xs::tasks {

template <class DataT, typename ResT>
class BaseSearcher {
 public:
  BaseSearcher() = default;
  virtual ~BaseSearcher() = default;

  virtual ResT search(const std::string& pattern, DataT* data) const = 0;
  virtual ResT search(re2::RE2* pattern, DataT* data) const = 0;

 protected:
  virtual ResT search(DataT* data,
                      const std::vector<uint64_t>& mapping_data) const = 0;
};

class MatchCounter : public BaseSearcher<DataChunk, uint64_t> {
 public:
  MatchCounter() = default;

  uint64_t search(const std::string& pattern, DataChunk* data) const override;
  uint64_t search(re2::RE2* pattern, DataChunk* data) const override;

 private:
  // private because we do not want anyone to use this method...
  uint64_t search(DataChunk* data, const std::vector<uint64_t>& mapping_data) const override { return 0; }
};

class LineCounter : public BaseSearcher<DataChunk, uint64_t> {
 public:
  LineCounter() = default;

  uint64_t search(const std::string& pattern, DataChunk* data) const override;
  uint64_t search(re2::RE2* pattern, DataChunk* data) const override;

 private:
  // private because we do not want anyone to use this method...
  uint64_t search(DataChunk* data, const std::vector<uint64_t>& mapping_data) const override { return 0; }
};

class MatchBytePositionSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  MatchBytePositionSearcher() = default;

  std::vector<uint64_t> search(const std::string& pattern,
                               DataChunk* data) const override;
  std::vector<uint64_t> search(re2::RE2* pattern,
                               DataChunk* data) const override;

 private:
  // private because we do not want anyone to use this method...
  std::vector<uint64_t> search(DataChunk* data, const std::vector<uint64_t>& mapping_data) const override { return {}; }
};

class LineBytePositionSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  LineBytePositionSearcher() = default;

  std::vector<uint64_t> search(const std::string& pattern,
                               DataChunk* data) const override;
  std::vector<uint64_t> search(re2::RE2* pattern,
                               DataChunk* data) const override;

 private:
  // private because we do not want anyone to use this method...
  std::vector<uint64_t> search(DataChunk* data, const std::vector<uint64_t>& mapping_data) const override { return {}; }
};

class LineIndexSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  LineIndexSearcher() = default;

  std::vector<uint64_t> search(const std::string& pattern,
                               DataChunk* data) const override;
  std::vector<uint64_t> search(re2::RE2* pattern,
                               DataChunk* data) const override;
  std::vector<uint64_t> search(
      DataChunk* data,
      const std::vector<uint64_t>& mapping_data) const override;
};

class LineSearcher : public BaseSearcher<DataChunk, std::vector<std::string>> {
 public:
  LineSearcher() = default;

  std::vector<std::string> search(const std::string& pattern,
                                  DataChunk* data) const override;
  std::vector<std::string> search(re2::RE2* pattern,
                                  DataChunk* data) const override;
  std::vector<std::string> search(
      DataChunk* data,
      const std::vector<uint64_t>& mapping_data) const override;
};

}  // namespace xs::tasks