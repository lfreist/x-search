#include <xsearch/ResultTypes.h>

namespace xs {

// ===== FullPartialResult =====================================================
// _____________________________________________________________________________
void FullPartialResult::merge(FullPartialResult& other) {
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

// ===== ByteOffsetsPartialResult ==============================================
void IndexPartialResult::merge(IndexPartialResult other) {
  indices.resize(indices.size() + other.indices.size());
  indices.insert(indices.end(),
                      std::make_move_iterator(other.indices.begin()),
                      std::make_move_iterator(other.indices.end()));
}

// ===== MatchByteOffsetsResult ================================================
// _____________________________________________________________________________
void MatchByteOffsetsResult::addPartialResult(
    IndexPartialResult partial_result) {
  _merged_result.push_back(std::move(partial_result));
}

// _____________________________________________________________________________
ResultIterator<MatchByteOffsetsResult, IndexPartialResult> MatchByteOffsetsResult::begin() {
  return {*this, 0};
}

// _____________________________________________________________________________
ResultIterator<MatchByteOffsetsResult, IndexPartialResult> MatchByteOffsetsResult::end() {
  return {*this, std::numeric_limits<size_t>::max()};
}

// _____________________________________________________________________________
IndexPartialResult& MatchByteOffsetsResult::getEmpty() {
  return _empty;
}

// ===== MatchByteOffsetsResult ================================================
// _____________________________________________________________________________
void LinesResult::addPartialResult(LinesPartialResult partial_result) {
  _merged_result.push_back(std::move(partial_result));
}

// _____________________________________________________________________________
ResultIterator<LinesResult, LinesPartialResult> LinesResult::begin() {
  return {*this, 0};
}

// _____________________________________________________________________________
ResultIterator<LinesResult, LinesPartialResult> LinesResult::end() {
  return {*this, std::numeric_limits<size_t>::max()};
}

// _____________________________________________________________________________
LinesPartialResult& LinesResult::getEmpty() {
  return _empty;
}

}  // namespace xs