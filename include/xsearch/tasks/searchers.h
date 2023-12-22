/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of x-search.
 */

#pragma once

#include <xsearch/ResultTypes.h>
#include <xsearch/concepts.h>
#include <xsearch/string_search/search_wrappers.h>

#include <functional>
#include <optional>
#include <string>

namespace xs {

/**
 *
 * @tparam R
 * @tparam T
 */
template <typename R, DefaultDataC T = strtype>
class Searcher_I {
 public:
  Searcher_I() = default;
  virtual ~Searcher_I() = default;

  virtual std::optional<R> operator()(const T&) const = 0;
};

/**
 *
 * @tparam T
 */
template <DefaultDataC T = strtype>
class IndexSearcher : Searcher_I<PartRes1<size_t>, T> {
 public:
  explicit IndexSearcher(std::string pattern) : _pattern(std::move(pattern)) {}
  ~IndexSearcher() = default;

  /**
   *
   * @param data
   * @return
   */
  std::optional<PartRes1<uint64_t>> operator()(const T& data) const override {
    PartRes1<uint64_t> match_indices = xs::search::byte_offsets_match(data, _pattern, false);
    if (match_indices.empty()) {
      return {};
    }
    return std::make_optional(std::move(match_indices));
  }

 private:
  std::string _pattern;
};

template <DefaultDataC T = strtype>
class LineIndexSearcher : Searcher_I<PartRes1<uint64_t>, T> {
 public:
  explicit LineIndexSearcher(std::string pattern) : _pattern(std::move(pattern)) {}

  std::optional<PartRes1<uint64_t>> operator()(const T& data) const override {
    PartRes1<uint64_t> match_indices = xs::search::byte_offsets_line(data, _pattern);
    if (match_indices.empty()) {
      return {};
    }
    return std::make_optional(std::move(match_indices));
  }

 private:
  std::string _pattern;
};

template <DefaultDataC T = strtype>
class LineSearcher : Searcher_I<PartRes1<std::string>, T> {
 public:
  explicit LineSearcher(std::string pattern) : _pattern(std::move(pattern)) {}

  std::optional<PartRes1<std::string>> operator()(const T& data) const override {
    PartRes1<std::string> lines = xs::search::line(data, _pattern);
    if (lines.empty()) {
      return {};
    }
    return lines;
  }

 private:
  std::string _pattern;
};

}  // namespace xs