#include <xsearch/ResultTypes.h>

namespace xs {

// ===== FullPartialResult =====================================================
// _____________________________________________________________________________
void FullPartialResult::merge(FullPartialResult& other) {
  _count += other._count;
  _byte_offsets_line.insert(
      _byte_offsets_line.end(),
      std::move_iterator(other._byte_offsets_line.begin()),
      std::move_iterator(other._byte_offsets_line.end()));
  _byte_offsets_match.insert(
      _byte_offsets_match.end(),
      std::move_iterator(other._byte_offsets_match.begin()),
      std::move_iterator(other._byte_offsets_match.end()));
  _line_indices.insert(_line_indices.end(),
                       std::move_iterator(other._line_indices.begin()),
                       std::move_iterator(other._line_indices.end()));
  _lines.insert(_lines.end(), std::move_iterator(other._lines.begin()),
                std::move_iterator(other._lines.end()));
}

// ===== FullResult ============================================================
// _____________________________________________________________________________
void FullResult::addPartialResult(FullPartialResult partial_result) {
  std::unique_lock locker(*_mutex);
  this->_merged_result.push_back(std::move(partial_result));
}

// _____________________________________________________________________________
std::vector<FullPartialResult>& FullResult::getResult() {
  return this->_merged_result;
}

// _____________________________________________________________________________
ResultIterator<FullResult, FullPartialResult> FullResult::begin() {
  return {*this, 0};
}

// _____________________________________________________________________________
ResultIterator<FullResult, FullPartialResult> FullResult::end() {
  return {*this, _merged_result.size() - 1};
}

// _____________________________________________________________________________
FullPartialResult& FullResult::getEmpty() { return _empty; }

// ===== CountResult ===========================================================
// _____________________________________________________________________________
void CountResult::addPartialResult(uint64_t partial_result) {
  std::unique_lock locker(*_mutex);
  _sum_result += partial_result;
  _merged_result.push_back(partial_result);
  _cv->notify_one();
}

// _____________________________________________________________________________
std::vector<uint64_t>& CountResult::getResult() {
  std::unique_lock locker(*_mutex);
  return _merged_result;
}

// _____________________________________________________________________________
uint64_t CountResult::getCount() {
  std::unique_lock locker(*_mutex);
  return _sum_result;
}

// _____________________________________________________________________________
ResultIterator<CountResult, uint64_t> CountResult::begin() {
  return {*this, 0};
}

// _____________________________________________________________________________
ResultIterator<CountResult, uint64_t> CountResult::end() {
  return {*this, std::numeric_limits<size_t>::max()};
}

// _____________________________________________________________________________
uint64_t& CountResult::getEmpty() { return _empty; }

}  // namespace xs