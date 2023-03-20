// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <re2/re2.h>
#include <xsearch/DataChunk.h>
#include <xsearch/tasks/base/ReturnProcessor.h>
#include <xsearch/utils/string_utils.h>

#include <memory>
#include <string>

namespace xs::task::searcher {

/**
 * BaseSearcher: Base class that can be inherited instead of directly inheriting
 * ReturnProcessor for implementing searchers.
 * @tparam T: data type
 * @tparam R: return type
 */
template <typename T, typename R>
class BaseSearcher : public base::ReturnProcessor<T, R> {
 public:
  BaseSearcher(std::string pattern, bool regex, bool ignore_case, bool utf8)
      : _pattern(std::move(pattern)),
        _regex(regex),
        _ignore_case(ignore_case),
        _utf8(utf8) {
    if (_regex) {
      re2::RE2::Options options;
      options.set_posix_syntax(true);
      options.set_case_sensitive(!_ignore_case);
      _re_pattern = std::make_unique<re2::RE2>('(' + _pattern + ')', options);
      return;
    }
    if (_utf8 && _ignore_case) {
      re2::RE2::Options options;
      options.set_case_sensitive(false);
      _re_pattern = std::make_unique<re2::RE2>(
          '(' + xs::utils::str::escaped(_pattern) + ')', options);
      return;
    }
    if (_ignore_case) {
      std::transform(_pattern.begin(), _pattern.end(), _pattern.begin(),
                     ::tolower);
    }
  };

  R process(const T* data) const override = 0;

 protected:
  virtual R process_re2(const T* data) const = 0;
  virtual R process_ascii(const T* data) const = 0;

  std::string _pattern;
  std::unique_ptr<re2::RE2> _re_pattern = nullptr;
  bool _regex = false;
  bool _ignore_case = false;
  bool _utf8 = false;
};

/**
 * MatchCounter: A searcher class that counts the occurrences of a pattern
 * within data.
 */
class MatchCounter : public BaseSearcher<DataChunk, uint64_t> {
 public:
  MatchCounter(std::string pattern, bool regex, bool case_insensitive = false,
               bool utf8 = false);

  uint64_t process(const DataChunk* data) const override;

 private:
  uint64_t process_re2(const DataChunk* data) const override;
  uint64_t process_ascii(const DataChunk* data) const override;
};

/**
 * LineCounter: A searcher that counts the number of matching lines (lines that
 * contain the pattern at least once)
 */
class LineCounter : public BaseSearcher<DataChunk, uint64_t> {
 public:
  LineCounter(std::string pattern, bool regex, bool case_insensitive = false,
              bool utf8 = false);

  uint64_t process(const DataChunk* data) const override;

 private:
  uint64_t process_re2(const DataChunk* data) const override;
  uint64_t process_ascii(const DataChunk* data) const override;
};

/**
 * MatchBytePositionSearcher: A searcher that collects the byte offsets of all
 * matches within the data.
 */
class MatchBytePositionSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  MatchBytePositionSearcher(std::string pattern, bool regex,
                            bool case_insensitive = false, bool utf8 = false);

  std::vector<uint64_t> process(const DataChunk* data) const override;

 private:
  std::vector<uint64_t> process_re2(const DataChunk* data) const override;
  std::vector<uint64_t> process_ascii(const DataChunk* data) const override;
};

/**
 * LineBytePositionSearcher: A searcher that collects the byte offsets of the
 * first byte of all matching lines (lines that contain the pattern at least
 * once).
 */
class LineBytePositionSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  LineBytePositionSearcher(std::string pattern, bool regex,
                           bool case_insensitive = false, bool utf8 = false);

  std::vector<uint64_t> process(const DataChunk* data) const override;

 private:
  std::vector<uint64_t> process_re2(const DataChunk* data) const override;
  std::vector<uint64_t> process_ascii(const DataChunk* data) const override;
};

/**
 * LineIndexSearcher: A searcher that collects all line indices of the matching
 * lines.
 */
class LineIndexSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  LineIndexSearcher(std::string pattern, bool regex,
                    bool case_insensitive = false, bool utf8 = false);

  std::vector<uint64_t> process(const DataChunk* data) const override;

 private:
  std::vector<uint64_t> process_re2(const DataChunk* data) const override;
  std::vector<uint64_t> process_ascii(const DataChunk* data) const override;
};

/**
 * LineSearcher: A searcher collecting all matching lines as strings
 */
class LineSearcher : public BaseSearcher<DataChunk, std::vector<std::string>> {
 public:
  LineSearcher(std::string pattern, bool regex, bool case_insensitive = false,
               bool utf8 = false);

  std::vector<std::string> process(const DataChunk* data) const override;

 private:
  std::vector<std::string> process_re2(const DataChunk* data) const override;
  std::vector<std::string> process_ascii(const DataChunk* data) const override;

  static std::vector<std::string> map(
      const DataChunk* data, const std::vector<uint64_t>& mapping_data);
};

}  // namespace xs::task::searcher