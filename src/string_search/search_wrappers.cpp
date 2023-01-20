// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <re2/re2.h>
#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/string_search/simd_search.h>
#include <xsearch/utils/IOColor.h>

namespace xs::search {

// ===== helpers ===============================================================
// _____________________________________________________________________________
std::vector<uint64_t> _byte_offsets(
    xs::DataChunk* data, const std::string& pattern, bool skip_to_nl = true,
    const std::function<int64_t(uint64_t)>& func = [](uint64_t x) {
      return x;
    }) {
  std::vector<uint64_t> results;
  size_t data_size = data->size();
  char* data_c = data->data();
  const char* pattern_c = pattern.c_str();
  size_t shift = 0;
  while (shift < data->size()) {
    int64_t match =
        simd::findNext(pattern_c, pattern.size(), data_c, data_size, shift);
    if (match == -1) {
      break;
    }
    results.push_back(func(match));
    shift = match + pattern.size();
    if (skip_to_nl) {
      match = simd::findNextNewLine(data_c, data_size, shift);
      if (match == -1) {
        break;
      }
      shift = match + 1;
    }
  }
  return results;
}

// _____________________________________________________________________________
std::vector<uint64_t> _regex_byte_offsets(
    const xs::DataChunk* data, const re2::RE2& pattern, bool skip_to_nl = true,
    const std::function<int64_t(uint64_t)>& func = [](uint64_t x) {
      return x;
    }) {
  std::vector<uint64_t> results;
  re2::StringPiece input(data->data(), data->size());
  re2::StringPiece match;
  size_t total_shift = 0;
  while (re2::RE2::PartialMatch(input, pattern, &match)) {
    size_t shift = match.data() - input.data();
    results.push_back(func(shift + total_shift));
    shift += match.size();
    input.remove_prefix(shift);
    if (skip_to_nl) {
      size_t next_nl = input.find('\n');
      if (next_nl == re2::StringPiece::npos) {
        break;
      }
      next_nl++;
      shift += next_nl;
      input.remove_prefix(next_nl);
    }
    total_shift += shift;
  }
  return results;
}

// _____________________________________________________________________________
uint64_t _previous_new_line_offset_relative_to_match(
    const xs::DataChunk* data, uint64_t match_local_byte_offset) {
  uint64_t relative_offset = 0;
  while (true) {
    if (data->data()[match_local_byte_offset - relative_offset] == '\n') {
      return relative_offset - 1;
    }
    relative_offset++;
    if (relative_offset >= match_local_byte_offset) {
      return match_local_byte_offset;
    }
  }
}

std::string _get_regex_match(const char* line, size_t size,
                             const re2::RE2& pattern) {
  re2::StringPiece input(line, size);
  re2::StringPiece match;
  re2::RE2::PartialMatch(input, pattern, &match);
  return match.as_string();
}
// =============================================================================

// _____________________________________________________________________________
std::vector<uint64_t> local_byte_offsets_match(xs::DataChunk* data,
                                               const std::string& pattern,
                                               bool skip_to_nl) {
  return _byte_offsets(data, pattern, skip_to_nl);
}

// _____________________________________________________________________________
std::vector<uint64_t> global_byte_offsets_match(xs::DataChunk* data,
                                                const std::string& pattern,
                                                bool skip_to_nl) {
  return _byte_offsets(data, pattern, skip_to_nl,
                       [data](uint64_t v) { return v + data->getOffset(); });
}

// _____________________________________________________________________________
std::vector<uint64_t> local_byte_offsets_line(xs::DataChunk* data,
                                              const std::string& pattern) {
  return _byte_offsets(data, pattern, true, [data](uint64_t v) {
    return v - _previous_new_line_offset_relative_to_match(data, v);
  });
}

// _____________________________________________________________________________
std::vector<uint64_t> global_byte_offsets_line(xs::DataChunk* data,
                                               const std::string& pattern) {
  return _byte_offsets(data, pattern, true, [data](uint64_t v) {
    return v - _previous_new_line_offset_relative_to_match(data, v) +
           data->getOffset();
  });
}

// _____________________________________________________________________________
std::vector<uint64_t> line_indices(xs::DataChunk* data,
                                   const std::string& pattern) {
  return map::bytes::to_line_indices(data, _byte_offsets(data, pattern));
}

// _____________________________________________________________________________
std::vector<std::string> lines(xs::DataChunk* data,
                                   const std::string& pattern) {
  // TODO: implement
  return {};
}

// _____________________________________________________________________________
uint64_t count(xs::DataChunk* data, const std::string& pattern,
               bool skip_to_nl) {
  uint64_t result = 0;
  char* data_c = data->data();
  const char* pattern_c = pattern.c_str();
  size_t shift = 0;
  while (shift < data->size()) {
    int64_t match = xs::search::simd::findNext(pattern_c, pattern.size(),
                                               data_c, data->size(), shift);
    if (match == -1) {
      break;
    }
    result++;
    shift = match + pattern.size();
    if (skip_to_nl) {
      match = simd::findNextNewLine(data_c, data->size(), shift);
      if (match == -1) {
        break;
      }
      shift = match + 1;
    }
  }
  return result;
}

// _____________________________________________________________________________
std::vector<uint64_t> regex::local_byte_offsets_match(const xs::DataChunk* data,
                                                      const re2::RE2& pattern,
                                                      bool skip_to_nl) {
  return _regex_byte_offsets(data, pattern, skip_to_nl);
}

// _____________________________________________________________________________
std::vector<uint64_t> regex::global_byte_offsets_match(
    const xs::DataChunk* data, const re2::RE2& pattern, bool skip_to_nl) {
  return _regex_byte_offsets(data, pattern, skip_to_nl, [data](uint64_t v) {
    return v + data->getOffset();
  });
}

// _____________________________________________________________________________
std::vector<uint64_t> regex::local_byte_offsets_line(const xs::DataChunk* data,
                                                     const re2::RE2& pattern) {
  return _regex_byte_offsets(data, pattern, true, [data](uint64_t v) {
    return v - _previous_new_line_offset_relative_to_match(data, v);
  });
}

// _____________________________________________________________________________
std::vector<uint64_t> regex::global_byte_offsets_line(const xs::DataChunk* data,
                                                      const re2::RE2& pattern) {
  return _regex_byte_offsets(data, pattern, true, [data](uint64_t v) {
    return v - _previous_new_line_offset_relative_to_match(data, v) +
           data->getOffset();
  });
}

// _____________________________________________________________________________
std::vector<uint64_t> regex::line_indices(xs::DataChunk* data,
                                          const re2::RE2& pattern) {
  return map::bytes::to_line_indices(data, _regex_byte_offsets(data, pattern));
}

// _____________________________________________________________________________
std::vector<std::string> lines(xs::DataChunk* data,
                               const re2::RE2& pattern) {
  // TODO: implement
  return {};
}

// _____________________________________________________________________________
uint64_t regex::count(xs::DataChunk* data, const re2::RE2& pattern,
                      bool skip_to_nl) {
  uint64_t counter = 0;
  re2::StringPiece input(data->data(), data->size());
  re2::StringPiece match;
  while (re2::RE2::PartialMatch(input, pattern, &match)) {
    size_t shift = match.data() - input.data();
    counter++;
    shift += match.size();
    input.remove_prefix(shift);
    if (skip_to_nl) {
      size_t next_nl = input.find('\n');
      if (next_nl == re2::StringPiece::npos) {
        break;
      }
      next_nl++;
      input.remove_prefix(next_nl);
    }
  }
  return counter;
}

}  // namespace xs::search