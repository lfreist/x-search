// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/tasks/defaults/searchers.h>
#include <xsearch/utils/InlineBench.h>

namespace xs::task::searcher {

// ===== MatchCounter ==========================================================
// _____________________________________________________________________________
MatchCounter::MatchCounter(std::string pattern, bool regex,
                           bool case_insensitive, bool utf8)
    : BaseSearcher<DataChunk, uint64_t>(std::move(pattern), regex,
                                        case_insensitive, utf8) {}

// _____________________________________________________________________________
uint64_t MatchCounter::process(DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START(_, "search");
  return _re_pattern == nullptr ? process_ascii(data) : process_re2(data);
}

// _____________________________________________________________________________
uint64_t MatchCounter::process_ascii(DataChunk* data) const {
  if (_ignore_case) {
    DataChunk chunk(*data);
    utils::str::simd::toLower(chunk.data(), chunk.size());
    return search::count(&chunk, _pattern, false);
  }
  return search::count(data, _pattern, false);
}

// _____________________________________________________________________________
uint64_t MatchCounter::process_re2(DataChunk* data) const {
  return search::regex::count(data, *_re_pattern, false);
}

// ===== LineCounter ===========================================================
// _____________________________________________________________________________
LineCounter::LineCounter(std::string pattern, bool regex, bool case_insensitive,
                         bool utf8)
    : BaseSearcher<DataChunk, uint64_t>(std::move(pattern), regex,
                                        case_insensitive, utf8) {}

// _____________________________________________________________________________
uint64_t LineCounter::process(DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START(_, "search");
  return _re_pattern == nullptr ? process_ascii(data) : process_re2(data);
}

// _____________________________________________________________________________
uint64_t LineCounter::process_ascii(DataChunk* data) const {
  if (_ignore_case) {
    DataChunk chunk(*data);
    utils::str::simd::toLower(chunk.data(), chunk.size());
    return search::count(&chunk, _pattern, true);
  }
  return search::count(data, _pattern, true);
}

// _____________________________________________________________________________
uint64_t LineCounter::process_re2(DataChunk* data) const {
  return search::regex::count(data, *_re_pattern, true);
}

// ===== MatchBytePositionSearcher =============================================
// _____________________________________________________________________________
MatchBytePositionSearcher::MatchBytePositionSearcher(std::string pattern,
                                                     bool regex,
                                                     bool case_insensitive,
                                                     bool utf8)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern), regex,
                                                     case_insensitive, utf8) {}

// _____________________________________________________________________________
std::vector<uint64_t> MatchBytePositionSearcher::process(
    DataChunk* data) const {
  return _re_pattern == nullptr ? process_ascii(data) : process_re2(data);
}

// _____________________________________________________________________________
std::vector<uint64_t> MatchBytePositionSearcher::process_ascii(
    DataChunk* data) const {
  if (_ignore_case) {
    DataChunk chunk(*data);
    utils::str::simd::toLower(chunk.data(), chunk.size());
    return search::global_byte_offsets_match(&chunk, _pattern, false);
  }
  return search::global_byte_offsets_match(data, _pattern, false);
}

// _____________________________________________________________________________
std::vector<uint64_t> MatchBytePositionSearcher::process_re2(
    DataChunk* data) const {
  return search::regex::global_byte_offsets_match(data, *_re_pattern, false);
}

// ===== LineBytePositionSearcher ==============================================
// _____________________________________________________________________________
LineBytePositionSearcher::LineBytePositionSearcher(std::string pattern,
                                                   bool regex,
                                                   bool case_insensitive,
                                                   bool utf8)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern), regex,
                                                     case_insensitive, utf8) {}

// _____________________________________________________________________________
std::vector<uint64_t> LineBytePositionSearcher::process(DataChunk* data) const {
  return _re_pattern == nullptr ? process_ascii(data) : process_re2(data);
}

// _____________________________________________________________________________
std::vector<uint64_t> LineBytePositionSearcher::process_ascii(
    DataChunk* data) const {
  if (_ignore_case) {
    DataChunk chunk(*data);
    utils::str::simd::toLower(chunk.data(), chunk.size());
    return search::global_byte_offsets_line(&chunk, _pattern);
  }
  return search::global_byte_offsets_line(data, _pattern);
}

// _____________________________________________________________________________
std::vector<uint64_t> LineBytePositionSearcher::process_re2(
    DataChunk* data) const {
  return search::regex::global_byte_offsets_line(data, *_re_pattern);
}

// ===== LineIndexSearcher =====================================================
// _____________________________________________________________________________
LineIndexSearcher::LineIndexSearcher(std::string pattern, bool regex,
                                     bool ignore_case, bool utf8)
    : BaseSearcher<DataChunk, std::vector<uint64_t>>(std::move(pattern), regex,
                                                     ignore_case, utf8) {}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::process(DataChunk* data) const {
  return _re_pattern == nullptr
             ? map::bytes::to_line_indices(data, process_ascii(data))
             : map::bytes::to_line_indices(data, process_re2(data));
}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::process_ascii(DataChunk* data) const {
  if (_ignore_case) {
    DataChunk chunk(*data);
    utils::str::simd::toLower(chunk.data(), chunk.size());
    return search::global_byte_offsets_line(&chunk, _pattern);
  }
  return search::global_byte_offsets_line(data, _pattern);
}

// _____________________________________________________________________________
std::vector<uint64_t> LineIndexSearcher::process_re2(DataChunk* data) const {
  return search::regex::global_byte_offsets_line(data, *_re_pattern);
}

// ===== LineSearcher ==========================================================
// _____________________________________________________________________________
LineSearcher::LineSearcher(std::string pattern, bool regex,
                           bool case_insensitive, bool utf8)
    : BaseSearcher<DataChunk, std::vector<std::string>>(
          std::move(pattern), regex, case_insensitive, utf8) {}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::process(DataChunk* data) const {
  return _re_pattern == nullptr ? process_ascii(data) : process_re2(data);
}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::process_ascii(DataChunk* data) const {
  if (_ignore_case) {
    DataChunk chunk(*data);
    utils::str::simd::toLower(chunk.data(), chunk.size());
    return map(data, search::global_byte_offsets_line(&chunk, _pattern));
  }
  return map(data, search::global_byte_offsets_line(data, _pattern));
}

// _____________________________________________________________________________
std::vector<std::string> LineSearcher::process_re2(DataChunk* data) const {
  return map(data, search::regex::global_byte_offsets_line(data, *_re_pattern));
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

}  // namespace xs::task::searcher