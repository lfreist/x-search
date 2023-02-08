#include <xsearch/ResultTypes.h>

namespace xs {

// ===== FullResult ============================================================
// _____________________________________________________________________________
void FullResult::addPartialResult(FullPartialResult partial_result) {
  _merged_result.wlock()->push_back(std::move(partial_result));
}

// ===== CountResult ===========================================================
// _____________________________________________________________________________
void CountResult::addPartialResult(uint64_t partial_result) {
  _sum_result.fetch_add(partial_result);
}

// _____________________________________________________________________________
uint64_t CountResult::getCount() {
  return _sum_result.load();
}

// ===== MatchByteOffsetsResult ================================================
// _____________________________________________________________________________
void MatchByteOffsetsResult::addPartialResult(
    IndexPartialResult partial_result) {
  _merged_result.wlock()->push_back(std::move(partial_result));
}

// ===== LinesResult ================================================
// _____________________________________________________________________________
void LinesResult::addPartialResult(LinesPartialResult partial_result) {
  _merged_result.wlock()->push_back(std::move(partial_result));
}

}  // namespace xs