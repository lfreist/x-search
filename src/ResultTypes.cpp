#include <xsearch/ResultTypes.h>

#include <numeric>

namespace xs {

// ===== CountMatchesResult ====================================================
// _____________________________________________________________________________
void CountResult::add(uint64_t partial_result) {
  std::unique_lock lock(*_mutex);
  _sum_result += partial_result;
  _cv->notify_one();
}

// _____________________________________________________________________________
void CountResult::sortedAdd(uint64_t partial_result) { add(partial_result); }

// _____________________________________________________________________________
void CountResult::sortedAdd(uint64_t partial_result, uint64_t id) {
  add(partial_result);
}

// _____________________________________________________________________________
size_t CountResult::size() const {
  std::unique_lock lock(*_mutex);
  return _sum_result;
}

}  // namespace xs