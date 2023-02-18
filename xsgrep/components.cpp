// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include "./components.h"

#include <xsearch/utils/string_manipulation.h>

// ----- Helper function -------------------------------------------------------
// _____________________________________________________________________________
std::string _get_regex_match_(const char* data, size_t size,
                              const re2::RE2& pattern) {
  re2::StringPiece input(data, size);
  re2::StringPiece match;
  re2::RE2::PartialMatch(input, pattern, &match);
  return match.as_string();
}
// -----------------------------------------------------------------------------
// ----- implementations for xsgrep/components.h -------------------------------
// _____________________________________________________________________________
bool GrepPartialResult::operator<(const GrepPartialResult& other) const {
  return byte_offset_match < other.byte_offset_match;
}

// _____________________________________________________________________________
GrepResult::GrepResult(std::string pattern) : _pattern(std::move(pattern)) {}

// _____________________________________________________________________________
GrepResult::GrepResult(std::string pattern, bool regex, bool byte_offset,
                       bool line_number, bool only_matching, bool color)
    : _pattern(std::move(pattern)),
      _regex(regex),
      _byte_offset(byte_offset),
      _line_number(line_number),
      _only_matching(only_matching),
      _color(color) {}

// _____________________________________________________________________________
void GrepResult::add(std::vector<GrepPartialResult> partial_result,
                     uint64_t id) {
  std::unique_lock lock(*this->_mutex);
  if (_current_index == id) {
    add(std::move(partial_result));
    _current_index++;
    // check if buffered results can be added now
    while (true) {
      auto search = _buffer.find(_current_index);
      if (search == _buffer.end()) {
        break;
      }
      add(std::move(search->second));
      _buffer.erase(_current_index);
      _current_index++;
    }
    // at least one partial_result was added -> notify
    this->_cv->notify_one();
  } else {
    // buffer the partial result
    _buffer.insert({id, std::move(partial_result)});
  }
}

// _____________________________________________________________________________
constexpr size_t GrepResult::size() const { return 0; }

// _____________________________________________________________________________
void GrepResult::add(std::vector<GrepPartialResult> partial_result) {
  INLINE_BENCHMARK_WALL_START(_, "formatting and printing");
  for (auto& r : partial_result) {
    if (_line_number) {
      if (_color) {
        std::cout << GREEN << r.line_number << CYAN << ":" << COLOR_RESET;
      } else {
        std::cout << r.line_number << ":";
      }
    }
    if (_byte_offset) {
      if (_color) {
        std::cout << GREEN
                  << (_only_matching ? r.byte_offset_match : r.byte_offset_line)
                  << CYAN << ":" << COLOR_RESET;
      } else {
        std::cout << (_only_matching ? r.byte_offset_match : r.byte_offset_line)
                  << ":";
      }
    }
    if (_only_matching) {
      if (_color) {
        std::cout << RED << r.str << COLOR_RESET << '\n';
      } else {
        std::cout << r.str << '\n';
      }
    } else {
      if (_color) {
        // search for every occurrence of pattern within the string and
        // print it out colored while the rest is printed uncolored.
        size_t shift = 0;
        size_t match_size = _pattern.size();
        bool stop = false;
        while (true) {
          size_t match_pos;
          if (_regex) {
            re2::RE2 re_pattern(_pattern);
            re2::StringPiece input(r.str.data() + shift, r.str.size() - shift);
            re2::StringPiece re_match;
            auto tmp = re2::RE2::PartialMatch(input, re_pattern, &re_match);
            if (tmp) {
              match_pos = re_match.data() - input.data() + shift;
              match_size = re_match.size();
            } else {
              break;
            }
          } else {
            match_pos = r.byte_offset_match - r.byte_offset_line + shift;
            stop = true;
          }
          // print string part uncolored (eq. pythonic substr is
          //  str[shift:match_pos]) and pattern in RED
          std::cout << std::string(
                           r.str.begin() + static_cast<int64_t>(shift),
                           r.str.begin() + static_cast<int64_t>(match_pos))
                    << RED << std::string(r.str.data() + match_pos, match_size)
                    << COLOR_RESET;
          // start next search at new shift
          shift = match_pos + _pattern.size();
          if (stop) {
            break;
          }
        }
        // print rest of the string (eq. pythonic substr is str[shift:])
        std::cout << std::string(r.str.begin() + static_cast<int64_t>(shift),
                                 r.str.end())
                  << '\n';
      } else {
        std::cout << r.str << '\n';
      }
    }
  }
}

// _____________________________________________________________________________
GrepSearcher::GrepSearcher(std::string pattern, bool regex,
                           bool case_insensitive, bool line_number,
                           bool only_matching)
    : BaseSearcher(std::move(pattern), regex, case_insensitive),
      _line_number(line_number),
      _only_matching(only_matching) {}

// _____________________________________________________________________________
std::vector<GrepPartialResult> GrepSearcher::process(
    xs::DataChunk* data) const {
  INLINE_BENCHMARK_WALL_START(_, "searching");
  return _regex ? process_regex(data) : process_plain(data);
}

// _____________________________________________________________________________
std::vector<GrepPartialResult> GrepSearcher::process_plain(
    xs::DataChunk* data) const {
  xs::DataChunk tmp_chunk;
  xs::DataChunk* op_chunk = data;
  if (_case_insensitive) {
    tmp_chunk = xs::DataChunk(*data);
    xs::utils::simd::toLower(tmp_chunk.data(), tmp_chunk.size());
    op_chunk = &tmp_chunk;
  }
  std::vector<uint64_t> byte_offsets_match =
      xs::search::global_byte_offsets_match(op_chunk, _pattern,
                                            !_only_matching);
  std::vector<uint64_t> byte_offsets_line;
  std::vector<uint64_t> line_numbers;
  if (_line_number) {
    line_numbers =
        xs::map::bytes::to_line_indices(op_chunk, byte_offsets_match);
    std::transform(line_numbers.begin(), line_numbers.end(),
                   line_numbers.begin(), [](uint64_t li) { return li + 1; });
  }
  std::vector<std::string> lines;
  lines.reserve(byte_offsets_match.size());
  if (_only_matching) {
    for (auto bo : byte_offsets_match) {
      lines.emplace_back(data->data() + bo - data->getMetaData().actual_offset,
                         _pattern.size());
    }
  } else {
    byte_offsets_line.resize(byte_offsets_match.size());
    std::transform(
        byte_offsets_match.begin(), byte_offsets_match.end(),
        byte_offsets_line.begin(), [data](uint64_t index) {
          return index - xs::search::previous_new_line_offset_relative_to_match(
                             data, index - data->getMetaData().actual_offset);
        });
    lines.resize(byte_offsets_match.size());
    std::transform(
        byte_offsets_line.begin(), byte_offsets_line.end(), lines.begin(),
        [data](uint64_t index) { return xs::map::byte::to_line(data, index); });
  }

  std::vector<GrepPartialResult> res(byte_offsets_match.size());
  for (uint64_t i = 0; i < byte_offsets_match.size(); ++i) {
    res[i].line_number = _line_number ? line_numbers[i] : 0;
    res[i].byte_offset_line = _only_matching ? 0 : byte_offsets_line[i];
    res[i].byte_offset_match = byte_offsets_match[i];
    res[i].str = lines[i];
  }
  return res;
}

// _____________________________________________________________________________
std::vector<GrepPartialResult> GrepSearcher::process_regex(
    xs::DataChunk* data) const {
  std::vector<uint64_t> byte_offsets =
      _only_matching
          ? xs::search::regex::global_byte_offsets_match(data, *_re_pattern,
                                                         false)
          : xs::search::regex::global_byte_offsets_line(data, *_re_pattern);
  std::vector<uint64_t> line_numbers;
  if (_line_number) {
    line_numbers = xs::map::bytes::to_line_indices(data, byte_offsets);
    std::transform(line_numbers.begin(), line_numbers.end(),
                   line_numbers.begin(), [](uint64_t li) { return li + 1; });
  }
  std::vector<std::string> lines;
  if (_only_matching) {
    lines.resize(byte_offsets.size());
    std::transform(byte_offsets.begin(), byte_offsets.end(), lines.begin(),
                   [&](uint64_t index) {
                     size_t local_byte_offset =
                         index - data->getMetaData().original_offset;
                     return _get_regex_match_(data->data() + local_byte_offset,
                                              data->size() - local_byte_offset,
                                              *_re_pattern);
                   });
  } else {
    lines.resize(byte_offsets.size());
    std::transform(
        byte_offsets.begin(), byte_offsets.end(), lines.begin(),
        [data](uint64_t index) { return xs::map::byte::to_line(data, index); });
  }

  std::vector<GrepPartialResult> res(byte_offsets.size());
  for (uint64_t i = 0; i < byte_offsets.size(); ++i) {
    res[i].line_number = _line_number ? line_numbers[i] : 0;
    res[i].byte_offset_match = byte_offsets[i];
    res[i].byte_offset_line = byte_offsets[i];
    res[i].str = lines[i];
  }
  return res;
}