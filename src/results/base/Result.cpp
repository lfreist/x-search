// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/results/base/Result.h>

namespace xs::result::base {

// _____________________________________________________________________________
void CountResult::add(uint64_t partial_result) {
  std::unique_lock lock(*_mutex);
  _sum_result += partial_result;
  _cv->notify_one();
}

// _____________________________________________________________________________
void CountResult::add(uint64_t partial_result, uint64_t id) {
  add(partial_result);
}

// _____________________________________________________________________________
size_t CountResult::size() const {
  std::unique_lock lock(*_mutex);
  return _sum_result;
}

}  // namespace xs::result::base