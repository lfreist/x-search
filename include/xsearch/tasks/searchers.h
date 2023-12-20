/**
* Copyright 2023, Leon Freist (https://github.com/lfreist)
* Author: Leon Freist <freist.leon@gmail.com>
*
* This file is part of x-search.
*/

#pragma once

#include <xsearch/concepts.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/string_search/simd_search.h>

#include <string>
#include <optional>

namespace xs {

template <DefaultDataC T = strtype>
class IndexSearcher {
 public:
  explicit IndexSearcher(std::string pattern) : _pattern(std::move(pattern)) {}
  ~IndexSearcher() = default;

  std::optional<PartRes1<size_t>> search(T* data) {
    if (data == nullptr) {
      return {};
    }
    PartRes1<size_t> pr;
    size_t shift = 0;
    while (true) {
      int64_t match = xs::search::simd::findNext(
          _pattern.data(), _pattern.size(), data->data(), data->size(), shift);
      if (match < 0) {
        break;
      }
      pr.push_back(match);
      shift += match;
      shift += _pattern.size();
    }
    if (pr.empty()) {
      return {};
    }
    return {pr};
  }

 private:
  std::string _pattern;
};

}