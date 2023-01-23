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
void MatchCounter::search(const std::string& pattern, DataChunk* data,
                          FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  result->_count = search::count(data, pattern, false);
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_STOP("searching count");
}

// _____________________________________________________________________________
void MatchCounter::search(re2::RE2* pattern, DataChunk* data,
                          FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  result->_count = search::regex::count(data, *pattern, false);
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_STOP("searching count");
}

// -----------------------------------------------------------------------------

// ===== LineCounter ==========================================================
// _____________________________________________________________________________
void LineCounter::search(const std::string& pattern, DataChunk* data,
                         uint64_t* result) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  *result = search::count(data, pattern, true);
  INLINE_BENCHMARK_WALL_STOP("searching count");
}

// _____________________________________________________________________________
void LineCounter::search(re2::RE2* pattern, DataChunk* data,
                         uint64_t* result) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  *result = search::regex::count(data, *pattern, true);
  INLINE_BENCHMARK_WALL_STOP("searching count");
}

// -----------------------------------------------------------------------------

// ===== MatchBytePositionSearcher =============================================
// _____________________________________________________________________________
void MatchBytePositionSearcher::search(const std::string& pattern,
                                       DataChunk* data,
                                       FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->_byte_offsets_line =
      search::global_byte_offsets_match(data, pattern, false);
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// _____________________________________________________________________________
void MatchBytePositionSearcher::search(re2::RE2* pattern, DataChunk* data,
                                       FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->_byte_offsets_line =
      search::regex::global_byte_offsets_match(data, *pattern, false);
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// -----------------------------------------------------------------------------

// ===== LineBytePositionSearcher ==============================================
// _____________________________________________________________________________
void LineBytePositionSearcher::search(const std::string& pattern,
                                      DataChunk* data,
                                      FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->_byte_offsets_line = search::global_byte_offsets_line(data, pattern);
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// _____________________________________________________________________________
void LineBytePositionSearcher::search(re2::RE2* pattern, DataChunk* data,
                                      FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  result->_byte_offsets_line =
      search::regex::global_byte_offsets_line(data, *pattern);
  result->_index = data->getIndex();
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
}

// -----------------------------------------------------------------------------

// ===== LineIndexSearcher =====================================================
// _____________________________________________________________________________
void LineIndexSearcher::search(const std::string& pattern, DataChunk* data,
                               FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("mapping line index");
  result->_line_indices =
      search::line_indices(data, result->_byte_offsets_line, pattern);
  INLINE_BENCHMARK_WALL_STOP("mapping line index");
}

// _____________________________________________________________________________
void LineIndexSearcher::search(re2::RE2* pattern, DataChunk* data,
                               FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("mapping line index");
  result->_line_indices =
      search::regex::line_indices(data, result->_byte_offsets_line, *pattern);
  INLINE_BENCHMARK_WALL_STOP("mapping line index");
}

// -----------------------------------------------------------------------------

// ===== LinesSearcher =========================================================
// _____________________________________________________________________________
void LinesSearcher::search(const std::string& pattern, DataChunk* data,
                           FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("mapping line");
  result->_lines.reserve(result->_byte_offsets_line.size());
  for (auto bo : result->_byte_offsets_line) {
    result->_lines.push_back(xs::map::byte::to_line(data, bo));
  }
  INLINE_BENCHMARK_WALL_STOP("mapping line");
}

// _____________________________________________________________________________
void LinesSearcher::search(re2::RE2* pattern, DataChunk* data,
                           FullPartialResult* result) const {
  INLINE_BENCHMARK_WALL_START("mapping line");
  result->_lines.reserve(result->_byte_offsets_line.size());
  for (auto bo : result->_byte_offsets_line) {
    result->_lines.push_back(xs::map::byte::to_line(data, bo));
  }
  INLINE_BENCHMARK_WALL_STOP("mapping line");
}

// -----------------------------------------------------------------------------

}  // namespace xs::tasks