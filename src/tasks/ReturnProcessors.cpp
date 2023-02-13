// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/tasks/ReturnProcessors.h>
#include <xsearch/utils/InlineBench.h>

namespace xs::tasks {

// ===== MatchCounter ==========================================================
// _____________________________________________________________________________
MatchCounter::MatchCounter(std::string pattern, bool regex)
    : BaseSearcher<DataChunk, uint64_t>(std::move(pattern), regex) {}

// _____________________________________________________________________________
uint64_t MatchCounter::process(DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  uint64_t count = _regex ? search::regex::count(data, *_re_pattern, false)
                          : search::count(data, _pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching count");
  return count;
}

// ===== LineCounter ===========================================================
// _____________________________________________________________________________
LineCounter::LineCounter(std::string pattern, bool regex)
    : BaseSearcher<DataChunk, uint64_t>(std::move(pattern), regex) {}

// _____________________________________________________________________________
uint64_t LineCounter::process(DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching count");
  auto count = _regex ? search::regex::count(data, *_re_pattern, true)
                      : search::count(data, _pattern, true);
  INLINE_BENCHMARK_WALL_STOP("searching count");
  return count;
}
// ===== MatchBytePositionSearcher =============================================
// _____________________________________________________________________________
MatchBytePositionSearcher::MatchBytePositionSearcher(std::string pattern,
                                                     bool regex)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern),
                                                     regex) {}

// _____________________________________________________________________________
std::vector<uint64_t> MatchBytePositionSearcher::process(
    DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto tmp = _regex ? search::regex::global_byte_offsets_match(
                          data, *_re_pattern, false)
                    : search::global_byte_offsets_match(data, _pattern, false);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return tmp;
}

// ===== LineBytePositionSearcher ==============================================
// _____________________________________________________________________________
LineBytePositionSearcher::LineBytePositionSearcher(std::string pattern,
                                                   bool regex)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern),
                                                     regex) {}

// _____________________________________________________________________________
std::vector<uint64_t> LineBytePositionSearcher::process(DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto tmp = _regex
                 ? search::regex::global_byte_offsets_line(data, *_re_pattern)
                 : search::global_byte_offsets_line(data, _pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return tmp;
}

// ===== LineIndexSearcher =====================================================
// _____________________________________________________________________________
LineIndexSearcher::LineIndexSearcher(std::string pattern, bool regex)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern),
                                                     regex) {}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::process(DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto mapping_data =
      _regex ? search::regex::global_byte_offsets_line(data, *_re_pattern)
             : search::global_byte_offsets_line(data, _pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return map(data, mapping_data);
}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::map(
    DataChunk* data, const std::vector<uint64_t>& mapping_data) {
  INLINE_BENCHMARK_WALL_START("mapping line index");
  auto tmp = map::bytes::to_line_indices(data, mapping_data);
  INLINE_BENCHMARK_WALL_STOP("mapping line index");
  return tmp;
}

// ===== LineSearcher ==========================================================
// _____________________________________________________________________________
LineSearcher::LineSearcher(std::string pattern, bool regex)
    : BaseSearcher<DataChunk, std::vector<std::string>>(std::move(pattern),
                                                        regex) {}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::process(DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START("searching byte position");
  auto mapping_data =
      _regex ? search::regex::global_byte_offsets_line(data, *_re_pattern)
             : search::global_byte_offsets_line(data, _pattern);
  INLINE_BENCHMARK_WALL_STOP("searching byte position");
  return map(data, mapping_data);
}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::map(
    DataChunk* data, const std::vector<uint64_t>& mapping_data) {
  std::vector<std::string> tmp;
  tmp.reserve(mapping_data.size());
  for (const auto bo : mapping_data) {
    tmp.push_back(map::byte::to_line(data, bo));
  }
  return tmp;
}

}  // namespace xs::tasks