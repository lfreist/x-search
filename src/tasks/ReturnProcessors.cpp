// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/tasks/ReturnProcessors.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/string_manipulation.h>

namespace xs::tasks {

// ===== MatchCounter ==========================================================
// _____________________________________________________________________________
MatchCounter::MatchCounter(std::string pattern, bool regex,
                           bool case_insensitive)
    : BaseSearcher<DataChunk, uint64_t>(std::move(pattern), regex,
                                        case_insensitive) {}

// _____________________________________________________________________________
uint64_t MatchCounter::process(DataChunk* data) const {
  if (_case_insensitive) {
    DataChunk tmp_chunk(*data);
    INLINE_BENCHMARK_WALL_START(to_lower, "transform to lower case");
    utils::simd::toLower(tmp_chunk.data(), tmp_chunk.size());
    INLINE_BENCHMARK_WALL_STOP("transform to lower case");
    INLINE_BENCHMARK_WALL_START(total, "searching count");
    return _regex ? search::regex::count(&tmp_chunk, *_re_pattern, false)
                  : search::count(&tmp_chunk, _pattern, false);
  }
  INLINE_BENCHMARK_WALL_START(total, "searching count");
  return _regex ? search::regex::count(data, *_re_pattern, false)
                : search::count(data, _pattern, false);
}

// ===== LineCounter ===========================================================
// _____________________________________________________________________________
LineCounter::LineCounter(std::string pattern, bool regex, bool case_insensitive)
    : BaseSearcher<DataChunk, uint64_t>(std::move(pattern), regex,
                                        case_insensitive) {}

// _____________________________________________________________________________
uint64_t LineCounter::process(DataChunk* data) const {
  if (_case_insensitive) {
    DataChunk tmp_chunk(*data);
    INLINE_BENCHMARK_WALL_START(to_lower, "transform to lower case");
    utils::simd::toLower(tmp_chunk.data(), tmp_chunk.size());
    INLINE_BENCHMARK_WALL_STOP("transform to lower case");
    INLINE_BENCHMARK_WALL_START(total, "searching count");
    return _regex ? search::regex::count(&tmp_chunk, *_re_pattern, true)
                  : search::count(&tmp_chunk, _pattern, true);
  }
  INLINE_BENCHMARK_WALL_START(total, "searching count");
  return _regex ? search::regex::count(data, *_re_pattern, true)
                : search::count(data, _pattern, true);
}
// ===== MatchBytePositionSearcher =============================================
// _____________________________________________________________________________
MatchBytePositionSearcher::MatchBytePositionSearcher(std::string pattern,
                                                     bool regex,
                                                     bool case_insensitive)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern), regex,
                                                     case_insensitive) {}

// _____________________________________________________________________________
std::vector<uint64_t> MatchBytePositionSearcher::process(
    DataChunk* data) const {
  if (_case_insensitive) {
    DataChunk tmp_chunk(*data);
    INLINE_BENCHMARK_WALL_START(to_lower, "transform to lower case");
    utils::simd::toLower(tmp_chunk.data(), tmp_chunk.size());
    INLINE_BENCHMARK_WALL_STOP("transform to lower case");
    INLINE_BENCHMARK_WALL_START(total, "searching byte offsets");
    return _regex
               ? search::regex::global_byte_offsets_match(&tmp_chunk,
                                                          *_re_pattern, false)
               : search::global_byte_offsets_match(&tmp_chunk, _pattern, false);
  }
  INLINE_BENCHMARK_WALL_START(total, "searching byte offsets");
  return _regex ? search::regex::global_byte_offsets_match(data, *_re_pattern,
                                                           false)
                : search::global_byte_offsets_match(data, _pattern, false);
}

// ===== LineBytePositionSearcher ==============================================
// _____________________________________________________________________________
LineBytePositionSearcher::LineBytePositionSearcher(std::string pattern,
                                                   bool regex,
                                                   bool case_insensitive)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern), regex,
                                                     case_insensitive) {}

// _____________________________________________________________________________
std::vector<uint64_t> LineBytePositionSearcher::process(DataChunk* data) const {
  if (_case_insensitive) {
    DataChunk tmp_chunk(*data);
    INLINE_BENCHMARK_WALL_START(to_lower, "transform to lower case");
    utils::simd::toLower(tmp_chunk.data(), tmp_chunk.size());
    INLINE_BENCHMARK_WALL_STOP("transform to lower case");
    INLINE_BENCHMARK_WALL_START(total, "searching byte offsets");
    return _regex ? search::regex::global_byte_offsets_line(&tmp_chunk,
                                                            *_re_pattern)
                  : search::global_byte_offsets_line(&tmp_chunk, _pattern);
  }
  INLINE_BENCHMARK_WALL_START(total, "searching byte offsets");
  return _regex ? search::regex::global_byte_offsets_line(data, *_re_pattern)
                : search::global_byte_offsets_line(data, _pattern);
}

// ===== LineIndexSearcher =====================================================
// _____________________________________________________________________________
LineIndexSearcher::LineIndexSearcher(std::string pattern, bool regex,
                                     bool case_insensitive)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern), regex,
                                                     case_insensitive) {}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::process(DataChunk* data) const {
  std::vector<uint64_t> mapping_data;
  if (_case_insensitive) {
    DataChunk tmp_chunk(*data);
    INLINE_BENCHMARK_WALL_START(to_lower, "transform to lower case");
    utils::simd::toLower(tmp_chunk.data(), tmp_chunk.size());
    INLINE_BENCHMARK_WALL_STOP("transform to lower case");
    INLINE_BENCHMARK_WALL_START(total, "searching line indices");
    mapping_data =
        _regex
            ? search::regex::global_byte_offsets_line(&tmp_chunk, *_re_pattern)
            : search::global_byte_offsets_line(&tmp_chunk, _pattern);
  }
  INLINE_BENCHMARK_WALL_START(total, "searching line indices");
  mapping_data =
      _regex ? search::regex::global_byte_offsets_line(data, *_re_pattern)
             : search::global_byte_offsets_line(data, _pattern);
  return map(data, mapping_data);
}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::map(
    DataChunk* data, const std::vector<uint64_t>& mapping_data) {
  INLINE_BENCHMARK_WALL_START(_, "mapping to line indices");
  auto tmp = map::bytes::to_line_indices(data, mapping_data);
  return tmp;
}

// ===== LineSearcher ==========================================================
// _____________________________________________________________________________
LineSearcher::LineSearcher(std::string pattern, bool regex,
                           bool case_insensitive)
    : BaseSearcher<DataChunk, std::vector<std::string>>(
          std::move(pattern), regex, case_insensitive) {}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::process(DataChunk* data) const {
  std::vector<uint64_t> mapping_data;
  if (_case_insensitive) {
    DataChunk tmp_chunk(*data);
    INLINE_BENCHMARK_WALL_START(to_lower, "transform to lower case");
    utils::simd::toLower(tmp_chunk.data(), tmp_chunk.size());
    INLINE_BENCHMARK_WALL_STOP("transform to lower case");
    INLINE_BENCHMARK_WALL_START(total, "searching byte offsets");
    mapping_data =
        _regex
            ? search::regex::global_byte_offsets_line(&tmp_chunk, *_re_pattern)
            : search::global_byte_offsets_line(&tmp_chunk, _pattern);
  }
  INLINE_BENCHMARK_WALL_START(_, "searching byte position");
  mapping_data =
      _regex ? search::regex::global_byte_offsets_line(data, *_re_pattern)
             : search::global_byte_offsets_line(data, _pattern);
  return map(data, mapping_data);
}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::map(
    DataChunk* data, const std::vector<uint64_t>& mapping_data) {
  INLINE_BENCHMARK_WALL_START(_, "mapping to lines");
  std::vector<std::string> tmp;
  tmp.reserve(mapping_data.size());
  for (const auto bo : mapping_data) {
    tmp.push_back(map::byte::to_line(data, bo));
  }
  return tmp;
}

}  // namespace xs::tasks