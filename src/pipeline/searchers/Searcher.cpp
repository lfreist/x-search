// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/pipeline/searchers/Searchers.h>
#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/utils/InlineBench.h>

namespace xs::tasks {

// _____________________________________________________________________________
Searcher::Searcher(std::string pattern, bool regex)
    : _regex_pattern("(" + pattern + ")") {  // TODO: is this move okay?
  _regex = regex;
  _pattern = std::move(pattern);
}

// _____________________________________________________________________________
void Searcher::count(DataChunk* data) {
  data->results.pattern = _pattern;
  data->results.regex = _regex;
  if (_regex) {
    data->results._match_count = xs::search::regex::count(data, _regex_pattern);
  } else {
    data->results._match_count = xs::search::count(data, _pattern);
  }
}

// _____________________________________________________________________________
void Searcher::byte_offsets_match(DataChunk* data, bool skip_line) {
  INLINE_BENCHMARK_WALL_START("searching byte offsets match");
  data->results.pattern = _pattern;
  data->results.regex = _regex;
  if (_regex) {
    data->results._local_byte_offsets =
        xs::search::regex::local_byte_offsets_match(data, _regex_pattern,
                                                    skip_line);
  } else {
    data->results._local_byte_offsets =
        xs::search::local_byte_offsets_match(data, _pattern, skip_line);
  }
  data->results._match_count = data->results._local_byte_offsets->size();
  INLINE_BENCHMARK_WALL_STOP("searching byte offsets match");
}

// _____________________________________________________________________________
void Searcher::byte_offsets_line(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("searching byte offsets line");
  data->results.pattern = _pattern;
  data->results.regex = _regex;
  if (_regex) {
    data->results._local_byte_offsets =
        xs::search::regex::local_byte_offsets_line(data, _regex_pattern);
  } else {
    data->results._local_byte_offsets =
        xs::search::local_byte_offsets_line(data, _pattern);
  }
  data->results._match_count = data->results._local_byte_offsets->size();
  INLINE_BENCHMARK_WALL_STOP("searching byte offsets line");
}

// _____________________________________________________________________________
void Searcher::line_indices(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("mapping to line indices");
  data->results.pattern = _pattern;
  data->results.regex = _regex;
  if (!data->results._local_byte_offsets.has_value()) {
    INLINE_BENCHMARK_WALL_STOP("mapping to line indices");
    byte_offsets_match(data);
    INLINE_BENCHMARK_WALL_START("mapping to line indices");
  }
  data->results._global_line_indices = xs::map::bytes::to_line_indices(
      data, data->results._local_byte_offsets.value());
  INLINE_BENCHMARK_WALL_STOP("mapping to line indices");
}

// _____________________________________________________________________________
void Searcher::line(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("mapping to line");
  data->results.pattern = _pattern;
  data->results.regex = _regex;
  if (!data->results._local_byte_offsets.has_value()) {
    INLINE_BENCHMARK_WALL_STOP("mapping to line");
    byte_offsets_match(data);
    INLINE_BENCHMARK_WALL_START("mapping to line");
  }
  data->results._matching_lines = std::vector<std::string>();
  data->results._matching_lines.value().reserve(
      data->results._local_byte_offsets->size());
  for (auto lbo : data->results._local_byte_offsets.value()) {
    std::string line = xs::map::byte::to_line(data, lbo);
    data->results._matching_lines.value().push_back(std::move(line));
  }
  INLINE_BENCHMARK_WALL_STOP("mapping to line");
}

}  // namespace xs::tasks