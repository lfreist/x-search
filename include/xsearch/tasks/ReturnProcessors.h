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
class BaseReturnProcessor {
 public:
  BaseReturnProcessor() = default;
  virtual ~BaseReturnProcessor() = default;

  virtual ResT process(DataT* data) const = 0;
};

template <class DataT, typename ResT>
class BaseSearcher : public BaseReturnProcessor<DataT, ResT> {
 public:
  BaseSearcher(std::string pattern, bool regex, bool case_insensitive)
      : _pattern(std::move(pattern)),
        _regex(regex),
        _case_insensitive(case_insensitive) {
    if (_regex) {
      _re_pattern = std::make_unique<re2::RE2>('(' + _pattern + ')');
    }
  };
  ResT process(DataT* data) const override = 0;

 protected:
  std::string _pattern;
  std::unique_ptr<re2::RE2> _re_pattern;
  bool _regex = false;
  bool _case_insensitive = false;
};

class MatchCounter : public BaseSearcher<DataChunk, uint64_t> {
 public:
  MatchCounter(std::string pattern, bool regex, bool case_insensitive);

  uint64_t process(DataChunk* data) const override;
};

class LineCounter : public BaseSearcher<DataChunk, uint64_t> {
 public:
  LineCounter(std::string pattern, bool regex, bool case_insensitive);

  uint64_t process(DataChunk* data) const override;
};

class MatchBytePositionSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  MatchBytePositionSearcher(std::string pattern, bool regex,
                            bool case_insensitive);

  std::vector<uint64_t> process(DataChunk* data) const override;
};

class LineBytePositionSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  LineBytePositionSearcher(std::string pattern, bool regex,
                           bool case_insensitive);

  std::vector<uint64_t> process(DataChunk* data) const override;
};

class LineIndexSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  LineIndexSearcher(std::string pattern, bool regex, bool case_insensitive);

  std::vector<uint64_t> process(DataChunk* data) const override;

 private:
  static std::vector<uint64_t> map(DataChunk* data,
                                   const std::vector<uint64_t>& mapping_data);
};

class LineSearcher : public BaseSearcher<DataChunk, std::vector<std::string>> {
 public:
  LineSearcher(std::string pattern, bool regex, bool case_insensitive);

  std::vector<std::string> process(DataChunk* data) const override;

 private:
  static std::vector<std::string> map(
      DataChunk* data, const std::vector<uint64_t>& mapping_data);
};

}  // namespace xs::tasks