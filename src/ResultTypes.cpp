#include <xsearch/ResultTypes.h>

#include <numeric>

namespace xs {

// ===== CountMatchesResult ====================================================
// _____________________________________________________________________________
void CountMatchesResult::add(std::vector<uint64_t> partial_result) {
  uint64_t sum = std::accumulate(partial_result.begin(), partial_result.end(),
                                 static_cast<uint64_t>(0));
  std::unique_lock lock(*_mutex);
  _sum_result += sum;
  _cv->notify_one();
}

// _____________________________________________________________________________
void CountMatchesResult::add(uint64_t partial_result) {
  std::unique_lock lock(*_mutex);
  _sum_result += partial_result;
  _cv->notify_one();
}

// _____________________________________________________________________________
void CountMatchesResult::sortedAdd(std::vector<uint64_t> partial_result) {
  add(std::move(partial_result));
}

// _____________________________________________________________________________
uint64_t CountMatchesResult::getCount() {
  std::unique_lock lock(*_mutex);
  return _sum_result;
}

}  // namespace xs