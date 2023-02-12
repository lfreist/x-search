// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include "./components.h"

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
  return index < other.index;
}

// _____________________________________________________________________________
void GrepResult::add(grep_result partial_result, uint64_t id) {
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
void GrepResult::add(grep_result partial_result) {
  auto& res = partial_result.first;
  auto& settings = partial_result.second;
  for (auto& r : res) {
    if (settings.index) {
      if (settings.color) {
        std::cout << GREEN << r.index << CYAN << ":" << COLOR_RESET;
      } else {
        std::cout << r.index << ":";
      }
    }
    if (settings.only_matching) {
      if (settings.color) {
        std::cout << RED << r.str << COLOR_RESET << '\n';
      } else {
        std::cout << r.str << '\n';
      }
    } else {
      if (settings.color) {
        // search for every occurrence of pattern within the string and
        // print it out colored while the rest is printed uncolored.
        size_t shift = 0;
        std::string match = settings.pattern;
        while (true) {
          size_t match_pos;
          if (settings.regex) {
            re2::RE2 re_pattern(settings.pattern);
            re2::StringPiece input(r.str.data() + shift, r.str.size() - shift);
            re2::StringPiece re_match;
            auto tmp = re2::RE2::PartialMatch(input, re_pattern, &re_match);
            if (tmp) {
              match_pos = re_match.data() - input.data() + shift;
              match = re_match.as_string();
            } else {
              break;
            }
          } else {
            match_pos = r.str.find(settings.pattern, shift);
          }
          if (match_pos == std::string::npos) {
            break;
          }
          // print string part uncolored (eq. pythonic substr is
          //  str[shift:match_pos]) and pattern in RED
          std::cout << std::string(
                           r.str.begin() + static_cast<int64_t>(shift),
                           r.str.begin() + static_cast<int64_t>(match_pos))
                    << RED << match << COLOR_RESET;
          // start next search at new shift
          shift = match_pos + match.size();
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
GrepSearcher::GrepSearcher(bool byte_offset, bool line_number,
                           bool only_matching, bool color)
    : _byte_offset(byte_offset),
      _line_number(line_number),
      _only_matching(only_matching),
      _color(color) {}

// _____________________________________________________________________________
grep_result GrepSearcher::search(const std::string& pattern,
                                 xs::DataChunk* data) const {
  std::vector<uint64_t> byte_offsets =
      _only_matching
          ? xs::search::global_byte_offsets_match(data, pattern, false)
          : xs::search::global_byte_offsets_line(data, pattern);
  std::vector<uint64_t> line_numbers;
  if (_line_number) {
    line_numbers = xs::map::bytes::to_line_indices(data, byte_offsets);
    std::transform(line_numbers.begin(), line_numbers.end(),
                   line_numbers.begin(), [](uint64_t li) { return li + 1; });
  }
  std::vector<std::string> lines;
  if (!_only_matching) {
    lines.resize(byte_offsets.size());
    std::transform(
        byte_offsets.begin(), byte_offsets.end(), lines.begin(),
        [data](uint64_t index) { return xs::map::byte::to_line(data, index); });
  }

  std::pair<std::vector<GrepPartialResult>, GrepResultSettings> res = {
      {},
      {false, _line_number || _byte_offset, _color, _only_matching, pattern}};
  res.first.resize(byte_offsets.size());
  for (uint64_t i = 0; i < byte_offsets.size(); ++i) {
    res.first[i].index = _line_number ? line_numbers[i] : byte_offsets[i];
    res.first[i].str = _only_matching ? pattern : lines[i];
  }
  return res;
}

// _____________________________________________________________________________
grep_result GrepSearcher::search(re2::RE2* pattern, xs::DataChunk* data) const {
  std::vector<uint64_t> byte_offsets =
      _only_matching
          ? xs::search::regex::global_byte_offsets_match(data, *pattern, false)
          : xs::search::regex::global_byte_offsets_line(data, *pattern);
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
                   [data, pattern](uint64_t index) {
                     size_t local_byte_offset =
                         index - data->getMetaData().original_offset;
                     return _get_regex_match_(data->data() + local_byte_offset,
                                              data->size() - local_byte_offset,
                                              *pattern);
                   });
  } else {
    lines.resize(byte_offsets.size());
    std::transform(
        byte_offsets.begin(), byte_offsets.end(), lines.begin(),
        [data](uint64_t index) { return xs::map::byte::to_line(data, index); });
  }

  std::pair<std::vector<GrepPartialResult>, GrepResultSettings> res = {
      {},
      {true, _line_number || _byte_offset, _color, _only_matching,
       pattern->pattern()}};
  res.first.resize(byte_offsets.size());
  for (uint64_t i = 0; i < byte_offsets.size(); ++i) {
    res.first[i].index = _line_number ? line_numbers[i] : byte_offsets[i];
    // we have placed either the line or the regex match in here above!
    res.first[i].str = lines[i];
  }
  return res;
}