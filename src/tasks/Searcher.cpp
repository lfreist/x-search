// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/tasks/Searcher.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/utils.h>

namespace xs::tasks {

// ===== MatchCounter ==========================================================
// _____________________________________________________________________________
template <>
void MatchCounter<uint64_t>::search(const std::string& pattern, DataChunk* data,
                                    uint64_t* result) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  *result = search::count(data, pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching count");
}

// _____________________________________________________________________________
template <>
void MatchCounter<uint64_t>::search(re2::RE2* pattern, DataChunk* data,
                                    uint64_t* result) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  *result = search::regex::count(data, *pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching count");
}

// -----------------------------------------------------------------------------

// ===== LineCounter ===========================================================
// _____________________________________________________________________________
template <>
void LineCounter<uint64_t>::search(const std::string& pattern, DataChunk* data,
                                   uint64_t* result) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  *result = search::count(data, pattern, true);
  INLINE_BENCHMARK_WALL_STOP("searching count");
}

// _____________________________________________________________________________
template <>
void LineCounter<uint64_t>::search(re2::RE2* pattern, DataChunk* data,
                                   uint64_t* result) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  *result = search::regex::count(data, *pattern, true);
  INLINE_BENCHMARK_WALL_STOP("searching count");
}

// -----------------------------------------------------------------------------

// ===== MatchBytePositionSearcher =============================================
// _____________________________________________________________________________
template <>
void MatchBytePositionSearcher<IndexPartialResult>::search(
    const std::string& pattern, DataChunk* data,
    IndexPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->indices = search::global_byte_offsets_match(data, pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// _____________________________________________________________________________
template <>
void MatchBytePositionSearcher<FullPartialResult>::search(
    const std::string& pattern, DataChunk* data,
    xs::FullPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->_byte_offsets_match =
      search::global_byte_offsets_match(data, pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// _____________________________________________________________________________
template <>
void MatchBytePositionSearcher<FullPartialResult>::search(
    re2::RE2* pattern, DataChunk* data, xs::FullPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->_byte_offsets_match =
      search::regex::global_byte_offsets_match(data, *pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// _____________________________________________________________________________
template <>
void MatchBytePositionSearcher<IndexPartialResult>::search(
    re2::RE2* pattern, DataChunk* data, IndexPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->indices =
      search::regex::global_byte_offsets_match(data, *pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// -----------------------------------------------------------------------------

// ===== LineBytePositionSearcher ==============================================
// _____________________________________________________________________________
template <>
void LineBytePositionSearcher<IndexPartialResult>::search(
    const std::string& pattern, DataChunk* data,
    IndexPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->indices = search::global_byte_offsets_line(data, pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// _____________________________________________________________________________
template <>
void LineBytePositionSearcher<FullPartialResult>::search(
    const std::string& pattern, DataChunk* data,
    FullPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte position");
  if (!result->_byte_offsets_match.empty()) {
    result->_byte_offsets_line.reserve(result->_byte_offsets_match.size());
    for (size_t bo : result->_byte_offsets_match) {
      // byte offset of line is byte offset of match (bo) - func + 1
      result->_byte_offsets_line.push_back(
          bo - search::previous_new_line_offset_relative_to_match(data, bo) +
          1);
    }
  } else {
    result->_byte_offsets_line =
        search::global_byte_offsets_line(data, pattern);
  }
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// _____________________________________________________________________________
template <>
void LineBytePositionSearcher<IndexPartialResult>::search(
    re2::RE2* pattern, DataChunk* data, IndexPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->indices = search::regex::global_byte_offsets_line(data, *pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// _____________________________________________________________________________
template <>
void LineBytePositionSearcher<FullPartialResult>::search(
    re2::RE2* pattern, DataChunk* data, FullPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->_byte_offsets_line =
      search::regex::global_byte_offsets_line(data, *pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// -----------------------------------------------------------------------------

// ===== LineIndexSearcher =====================================================
// _____________________________________________________________________________
template <>
void LineIndexSearcher<IndexPartialResult>::search(
    const std::string& pattern, DataChunk* data,
    IndexPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte offsets for mapping");
  auto byte_offsets = search::global_byte_offsets_line(data, pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte offsets for mapping");
  INLINE_BENCHMARK_WALL_START("mapping line index");
  result->indices = search::line_indices(data, byte_offsets, pattern);
  INLINE_BENCHMARK_WALL_STOP("mapping line index");
}

// _____________________________________________________________________________
template <>
void LineIndexSearcher<FullPartialResult>::search(
    const std::string& pattern, DataChunk* data,
    FullPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("mapping line index");
  result->_line_indices =
      search::line_indices(data, result->_byte_offsets_line, pattern);
  INLINE_BENCHMARK_WALL_STOP("mapping line index");
}

// _____________________________________________________________________________
template <>
void LineIndexSearcher<IndexPartialResult>::search(
    re2::RE2* pattern, DataChunk* data, IndexPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte offsets for mapping");
  auto byte_offsets = search::regex::global_byte_offsets_line(data, *pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte offsets for mapping");
  INLINE_BENCHMARK_WALL_START("mapping line index");
  result->indices = search::regex::line_indices(data, byte_offsets, *pattern);
  INLINE_BENCHMARK_WALL_STOP("mapping line index");
}

// _____________________________________________________________________________
template <>
void LineIndexSearcher<FullPartialResult>::search(
    re2::RE2* pattern, DataChunk* data, FullPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("mapping line index");
  result->_line_indices =
      search::regex::line_indices(data, result->_byte_offsets_line, *pattern);
  INLINE_BENCHMARK_WALL_STOP("mapping line index");
}
// -----------------------------------------------------------------------------

// ===== LinesSearcher =========================================================
// _____________________________________________________________________________
template <>
void LineSearcher<LinesPartialResult>::search(
    const std::string& pattern, DataChunk* data,
    LinesPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte offsets for mapping");
  auto byte_offsets = search::global_byte_offsets_line(data, pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte offsets for mapping");
  INLINE_BENCHMARK_WALL_START("mapping line");
  result->lines.reserve(byte_offsets.size());
  for (auto bo : byte_offsets) {
    result->lines.push_back(xs::map::byte::to_line(data, bo));
  }
  INLINE_BENCHMARK_WALL_STOP("mapping line");
}

// _____________________________________________________________________________
template <>
void LineSearcher<FullPartialResult>::search(const std::string& pattern,
                                             DataChunk* data,
                                             FullPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("mapping line");
  result->_lines.reserve(result->_byte_offsets_line.size());
  for (auto bo : result->_byte_offsets_line) {
    result->_lines.push_back(xs::map::byte::to_line(data, bo));
  }
  INLINE_BENCHMARK_WALL_STOP("mapping line");
}

// _____________________________________________________________________________
template <>
void LineSearcher<LinesPartialResult>::search(
    re2::RE2* pattern, DataChunk* data, LinesPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("searching byte offsets for mapping");
  auto byte_offsets = search::regex::global_byte_offsets_line(data, *pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte offsets for mapping");
  INLINE_BENCHMARK_WALL_START("mapping line");
  result->lines.reserve(byte_offsets.size());
  for (auto bo : byte_offsets) {
    result->lines.push_back(xs::map::byte::to_line(data, bo));
  }
  INLINE_BENCHMARK_WALL_STOP("mapping line");
}

// _____________________________________________________________________________
template <>
void LineSearcher<FullPartialResult>::search(re2::RE2* pattern, DataChunk* data,
                                             FullPartialResult* result) const {
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_START("mapping line");
  result->_lines.reserve(result->_byte_offsets_line.size());
  for (auto bo : result->_byte_offsets_line) {
    result->_lines.push_back(xs::map::byte::to_line(data, bo));
  }
  INLINE_BENCHMARK_WALL_STOP("mapping line");
}

// -----------------------------------------------------------------------------

}  // namespace xs::tasks