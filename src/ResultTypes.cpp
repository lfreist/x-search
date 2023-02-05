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
  std::unique_lock locker(*_res_vec_mutex);
  _merged_result.push_back(std::move(partial_result));
}

// ===== CountResult ===========================================================
// _____________________________________________________________________________
void CountResult::addPartialResult(uint64_t partial_result) {
  std::unique_lock locker(*_res_vec_mutex);
  _sum_result += partial_result;
  _merged_result.push_back(partial_result);
}

// _____________________________________________________________________________
uint64_t CountResult::getCount() {
  std::unique_lock locker(*_res_vec_mutex);
  return _sum_result;
}

// ===== ByteOffsetsPartialResult ==============================================
void IndexPartialResult::merge(IndexPartialResult other) {
  indices.resize(indices.size() + other.indices.size());
  indices.insert(indices.end(), std::make_move_iterator(other.indices.begin()),
                 std::make_move_iterator(other.indices.end()));
}

// ===== MatchByteOffsetsResult ================================================
// _____________________________________________________________________________
void MatchByteOffsetsResult::addPartialResult(
    IndexPartialResult partial_result) {
  _merged_result.push_back(std::move(partial_result));
}

// ===== MatchByteOffsetsResult ================================================
// _____________________________________________________________________________
void LinesResult::addPartialResult(LinesPartialResult partial_result) {
  _merged_result.push_back(std::move(partial_result));
}

}  // namespace xs