// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <cstdint>
#include <optional>
#include <utility>

namespace xs {

typedef uint64_t chunk_index;

}

namespace xs::task::base {

/**
 * BaseDataProvider: Vase class that must be inherited by all primary data
 *  producing tasks.
 *  E.g.: tasks that read data from a file
 * @tparam DataT
 */
template <typename T>
class DataProvider {
 public:
  DataProvider() = default;
  virtual ~DataProvider() = default;

  virtual std::optional<std::pair<T, chunk_index>> getNextData() = 0;
};

}  // namespace xs::task::base