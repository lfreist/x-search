// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <re2/re2.h>
#include <xsearch/pipeline/result_collectors/grep.h>
#include <xsearch/utils/IOColor.h>
#include <xsearch/utils/InlineBench.h>

#include <cstring>

namespace xs::collector {

// _____________________________________________________________________________
std::string _get_regex_match(const char* data, size_t size,
                             const re2::RE2& pattern) {
  re2::StringPiece input(data, size);
  re2::StringPiece match;
  re2::RE2::PartialMatch(input, pattern, &match);
  return match.as_string();
}

// _____________________________________________________________________________
void output_helper(SearchResults& results, bool line_number, bool byte_offset,
                   bool only_matching, bool color) {
  for (size_t i = 0; i < results._match_count; ++i) {
    if (line_number) {
      if (color) {
        std::cout << GREEN << results._global_line_indices->at(i) + 1 << CYAN
                  << ":" << COLOR_RESET;
      } else {
        std::cout << results._global_line_indices->at(i) + 1 << ":";
      }
    }
    if (byte_offset) {
      if (color) {
        std::cout << GREEN
                  << results._local_byte_offsets->at(i) + results.offset << CYAN
                  << ":" << COLOR_RESET;
      } else {
        std::cout << results._local_byte_offsets->at(i) + results.offset << ":";
      }
    }
    const std::string& line = results._matching_lines->at(i);
    if (only_matching) {
      if (color) {
        std::cout << RED
                  << (results.regex ? _get_regex_match(
                                          line.data(), line.size(),
                                          re2::RE2("(" + results.pattern + ")"))
                                    : results.pattern)
                  << COLOR_RESET << "\n";
      } else {
        std::cout << (results.regex ? _get_regex_match(
                                          line.data(), line.size(),
                                          re2::RE2("(" + results.pattern + ")"))
                                    : results.pattern)
                  << "\n";
      }
    } else {
      if (color) {
        std::string match =
            results.regex
                ? _get_regex_match(line.data(), line.size(),
                                   re2::RE2("(" + results.pattern + ")"))
                : results.pattern;
        // we need to find the offset within the line. Damn. Fix this another
        // day.
        const char* match_ = std::strstr(line.data(), match.data());
        if (match_ == nullptr) {
          std::cout << line;
          return;
        }
        size_t match_offset = match_ - line.data();
        std::cout << line.substr(0, match_offset) << RED << match << COLOR_RESET
                  << line.substr(match_offset + match.size());
      } else {
        std::cout << line;
      }
    }
  }
}

// ===== GrepOutput ============================================================
// _____________________________________________________________________________
GrepOutput::GrepOutput(bool byte_offset, bool line_number, bool only_matching,
                       bool count, bool color) {
  _byte_offset = byte_offset;
  _line_number = line_number;
  _only_matching = only_matching;
  _count = count;
  _color = color;
}

void GrepOutput::print(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("printing");
  if (_count) {
    _counter += data->results._match_count;
    INLINE_BENCHMARK_WALL_STOP("printing");
    return;
  }
  if (data->getOffset() == _offset) {
    output_helper(data->results, _line_number, _byte_offset, _only_matching,
                  _color);
    _offset += data->size();
    while (true) {
      auto search = _buffer.find(_offset);
      if (search == _buffer.end()) {
        break;
      }
      output_helper(search->second.first, _line_number, _byte_offset,
                    _only_matching, _color);
      _buffer.erase(_offset);
      _offset += search->second.second;
    }
  } else {
    _buffer.insert(
        {data->getOffset(), {std::move(data->results), data->size()}});
  }
  INLINE_BENCHMARK_WALL_STOP("printing");
}

uint64_t GrepOutput::getCount() const {
  INLINE_BENCHMARK_WALL_START("printing");
  if (_count) {
    std::cout << _counter << std::endl;
  }
  INLINE_BENCHMARK_WALL_STOP("printing");
  return _counter;
}

}  // namespace xs::collector