// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

namespace xs::task::base {

/**
 * Abstract base class for return processors. Return processors are processors,
 * that do not alter the original data but create new data out of them and
 * return them (e.g. searchers return search results).
 * @tparam T: data type
 * @tparam R: return type
 */
template <typename T, typename R>
class ReturnProcessor {
 public:
  ReturnProcessor() = default;
  virtual ~ReturnProcessor() = default;

  /**
   * This function is called by the Executor.
   * @param data
   * @return
   */
  virtual R process(const T* data) const = 0;
};

}  // namespace xs::task::base