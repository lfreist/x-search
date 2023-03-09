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

  R process(T* data) const override = 0;

 protected:
  virtual R process_re2(T* data) const = 0;
  virtual R process_ascii(T* data) const = 0;

  std::string _pattern;
  std::unique_ptr<re2::RE2> _re_pattern = nullptr;
  bool _regex = false;
  bool _ignore_case = false;
  bool _utf8 = false;
};

class MatchCounter : public BaseSearcher<DataChunk, uint64_t> {
 public:
  MatchCounter(std::string pattern, bool regex, bool case_insensitive = false,
               bool utf8 = false);

  uint64_t process(DataChunk* data) const override;

 private:
  uint64_t process_re2(DataChunk* data) const override;
  uint64_t process_ascii(DataChunk* data) const override;
};

class LineCounter : public BaseSearcher<DataChunk, uint64_t> {
 public:
  LineCounter(std::string pattern, bool regex, bool case_insensitive = false,
              bool utf8 = false);

  uint64_t process(DataChunk* data) const override;

 private:
  uint64_t process_re2(DataChunk* data) const override;
  uint64_t process_ascii(DataChunk* data) const override;
};

class MatchBytePositionSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  MatchBytePositionSearcher(std::string pattern, bool regex,
                            bool case_insensitive = false, bool utf8 = false);

  std::vector<uint64_t> process(DataChunk* data) const override;

 private:
  std::vector<uint64_t> process_re2(DataChunk* data) const override;
  std::vector<uint64_t> process_ascii(DataChunk* data) const override;
};

class LineBytePositionSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  LineBytePositionSearcher(std::string pattern, bool regex,
                           bool case_insensitive = false, bool utf8 = false);

  std::vector<uint64_t> process(DataChunk* data) const override;

 private:
  std::vector<uint64_t> process_re2(DataChunk* data) const override;
  std::vector<uint64_t> process_ascii(DataChunk* data) const override;
};

class LineIndexSearcher
    : public BaseSearcher<DataChunk, std::vector<uint64_t>> {
 public:
  LineIndexSearcher(std::string pattern, bool regex,
                    bool case_insensitive = false, bool utf8 = false);

  std::vector<uint64_t> process(DataChunk* data) const override;

 private:
  std::vector<uint64_t> process_re2(DataChunk* data) const override;
  std::vector<uint64_t> process_ascii(DataChunk* data) const override;
};

class LineSearcher : public BaseSearcher<DataChunk, std::vector<std::string>> {
 public:
  LineSearcher(std::string pattern, bool regex, bool case_insensitive = false,
               bool utf8 = false);

  std::vector<std::string> process(DataChunk* data) const override;

 private:
  std::vector<std::string> process_re2(DataChunk* data) const override;
  std::vector<std::string> process_ascii(DataChunk* data) const override;

  static std::vector<std::string> map(
      DataChunk* data, const std::vector<uint64_t>& mapping_data);
};

}  // namespace xs::task::searcher