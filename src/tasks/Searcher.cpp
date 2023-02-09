// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/tasks/Searcher.h>
#include <xsearch/utils/InlineBench.h>

namespace xs::tasks {

// ===== MatchCounter ==========================================================
// _____________________________________________________________________________
uint64_t MatchCounter::search(const std::string& pattern,
                              DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  auto tmp = search::count(data, pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching count");
  return tmp;
}

// _____________________________________________________________________________
uint64_t MatchCounter::search(re2::RE2* pattern, DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  auto tmp = search::regex::count(data, *pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching count");
  return tmp;
}

// -----------------------------------------------------------------------------

// ===== LineCounter ===========================================================
uint64_t LineCounter::search(const std::string& pattern,
                             DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  auto tmp = search::count(data, pattern, true);
  INLINE_BENCHMARK_WALL_STOP("searching count");
  return tmp;
}

// _____________________________________________________________________________
uint64_t LineCounter::search(re2::RE2* pattern, DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  auto tmp = search::regex::count(data, *pattern, true);
  INLINE_BENCHMARK_WALL_STOP("searching count");
  return tmp;
}

// ===== MatchBytePositionSearcher =============================================
// _____________________________________________________________________________
std::vector<uint64_t> MatchBytePositionSearcher::search(
    const std::string& pattern, DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto tmp = search::global_byte_offsets_match(data, pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return tmp;
}

// _____________________________________________________________________________
std::vector<uint64_t> MatchBytePositionSearcher::search(re2::RE2* pattern,
                                                        DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto tmp = search::regex::global_byte_offsets_match(data, *pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return tmp;
}

// ===== LineBytePositionSearcher ==============================================
// _____________________________________________________________________________
std::vector<uint64_t> LineBytePositionSearcher::search(
    const std::string& pattern, DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto tmp = search::global_byte_offsets_line(data, pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return tmp;
}

// _____________________________________________________________________________
std::vector<uint64_t> LineBytePositionSearcher::search(re2::RE2* pattern,
                                                       DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto tmp = search::regex::global_byte_offsets_line(data, *pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return tmp;
}

// ===== LineIndexSearcher =====================================================
// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::search(const std::string& pattern,
                                                DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto mapping_data = search::global_byte_offsets_line(data, pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return search(data, mapping_data);
}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::search(re2::RE2* pattern,
                                                DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto mapping_data = search::regex::global_byte_offsets_line(data, *pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return search(data, mapping_data);
}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::search(
    DataChunk* data, const std::vector<uint64_t>& mapping_data) const {
  INLINE_BENCHMARK_WALL_START("mapping line index");
  auto tmp = map::bytes::to_line_indices(data, mapping_data);
  INLINE_BENCHMARK_WALL_STOP("mapping line index");
  return tmp;
}

// ----- LineSearcher ----------------------------------------------------------
// _____________________________________________________________________________
std::vector<std::string> LineSearcher::search(const std::string& pattern,
                                              DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto mapping_data = search::global_byte_offsets_line(data, pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return search(data, mapping_data);
}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::search(re2::RE2* pattern,
                                              DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto mapping_data = search::regex::global_byte_offsets_line(data, *pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return search(data, mapping_data);
}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::search(
    DataChunk* data, const std::vector<uint64_t>& mapping_data) const {
  std::vector<std::string> tmp;
  tmp.reserve(mapping_data.size());
  for (const auto bo : mapping_data) {
    tmp.push_back(map::byte::to_line(data, bo));
  }
  return tmp;
}

}  // namespace xs::tasks