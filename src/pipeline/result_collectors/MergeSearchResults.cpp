// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/pipeline/result_collectors/MergeSearchResults.h>

namespace xs {

// ===== restype::count implementation =========================================
template <>
void YieldResult<restype::count>::addPartialResult(DataChunk* data) {
  _result += data->results._match_count;
}

// _____________________________________________________________________________
template <>
void YieldResult<restype::count>::markAsDone() {
  std::unique_lock locker(*_mutex);
  _done = true;
}

// ===== restype::count_lines implementation
// =========================================
template <>
void YieldResult<restype::count_lines>::addPartialResult(DataChunk* data) {
  _result += data->results._match_count;
}

// _____________________________________________________________________________
template <>
void YieldResult<restype::count_lines>::markAsDone() {
  std::unique_lock locker(*_mutex);
  _done = true;
}

// ===== restype::byte_positions implementation ================================
// _____________________________________________________________________________
template <>
void YieldResult<restype::byte_positions>::addPartialResult(DataChunk* data) {
  if (!data->results._local_byte_offsets.has_value()) {
    throw std::runtime_error("ERROR: No byte offset result data provided.");
  }
  auto& data_bo = data->results._local_byte_offsets.value();
  std::transform(data_bo.begin(), data_bo.end(), data_bo.begin(),
                 [](auto v) { return v; });
  _result.reserve(_result.size() + data_bo.size());
  _result.insert(_result.end(), std::make_move_iterator(data_bo.begin()),
                 std::make_move_iterator(data_bo.end()));
}

// _____________________________________________________________________________
template <>
void YieldResult<restype::byte_positions>::markAsDone() {
  std::unique_lock locker(*_mutex);
  _done = true;
}

// ===== restype::line_numbers implementation ================================
// _____________________________________________________________________________
template <>
void YieldResult<restype::line_numbers>::addPartialResult(DataChunk* data) {
  if (!data->results._global_line_indices.has_value()) {
    throw std::runtime_error("ERROR: No line indices data provided.");
  }
  auto& data_bo = data->results._global_line_indices.value();
  std::transform(data_bo.begin(), data_bo.end(), data_bo.begin(),
                 [](auto v) { return v + 1; });
  _result.reserve(_result.size() + data_bo.size());
  _result.insert(_result.end(), std::make_move_iterator(data_bo.begin()),
                 std::make_move_iterator(data_bo.end()));
}

// _____________________________________________________________________________
template <>
void YieldResult<restype::line_numbers>::markAsDone() {
  std::unique_lock locker(*_mutex);
  _done = true;
}

// ===== restype::line_indices implementation ================================
// _____________________________________________________________________________
template <>
void YieldResult<restype::line_indices>::addPartialResult(DataChunk* data) {
  if (!data->results._global_line_indices.has_value()) {
    throw std::runtime_error("ERROR: No line indices data provided.");
  }
  auto& data_bo = data->results._global_line_indices.value();
  _result.reserve(_result.size() + data_bo.size());
  _result.insert(_result.end(), std::make_move_iterator(data_bo.begin()),
                 std::make_move_iterator(data_bo.end()));
}

// _____________________________________________________________________________
template <>
void YieldResult<restype::line_indices>::markAsDone() {
  std::unique_lock locker(*_mutex);
  _done = true;
}

// ===== restype::lines implementation =========================================
// _____________________________________________________________________________
template <>
void YieldResult<restype::lines>::addPartialResult(DataChunk* data) {
  if (!data->results._matching_lines.has_value()) {
    throw std::runtime_error("ERROR: No lines data provided.");
  }
  auto& data_bo = data->results._matching_lines.value();
  _result.reserve(_result.size() + data_bo.size());
  _result.insert(_result.end(), std::make_move_iterator(data_bo.begin()),
                 std::make_move_iterator(data_bo.end()));
}

// _____________________________________________________________________________
template <>
void YieldResult<restype::lines>::markAsDone() {
  std::unique_lock locker(*_mutex);
  _done = true;
}

// ===== restype::full implementation ==========================================
// _____________________________________________________________________________
template <>
void YieldResult<restype::full>::addPartialResult(DataChunk* data) {
  _result.push_back(std::move(data->results));
}

// _____________________________________________________________________________
template <>
void YieldResult<restype::full>::markAsDone() {
  std::unique_lock locker(*_mutex);
  _done = true;
}

}  // namespace xs