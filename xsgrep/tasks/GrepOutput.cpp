// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include "./GrepOutput.h"

// ===== GrepPartialResult =====================================================
// _____________________________________________________________________________
bool GrepPartialResult::operator<(const GrepPartialResult& other) const {
  return byte_offset_match < other.byte_offset_match;
}

// ===== GrepOutput ============================================================
// _____________________________________________________________________________
GrepOutput::GrepOutput(GrepOptions options)
    : _options(std::move(options)), _ostream(std::cout) {}

// _____________________________________________________________________________
GrepOutput::GrepOutput(GrepOptions options, std::ostream& ostream)
    : _options(std::move(options)), _ostream(ostream) {}

// _____________________________________________________________________________
void GrepOutput::add(std::vector<GrepPartialResult> partial_result,
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
size_t GrepOutput::size() const { return _lines_written; }

// _____________________________________________________________________________
void GrepOutput::add(std::vector<GrepPartialResult> partial_result) {
  for (auto& r : partial_result) {
    if (_options.line_number) {
      if (_options.color) {
        _ostream << GREEN << r.line_number << CYAN << ":" << COLOR_RESET;
      } else {
        _ostream << r.line_number << ":";
      }
    }
    if (_options.byte_offset) {
      if (_options.color) {
        _ostream << GREEN
                 << (_options.only_matching ? r.byte_offset_match
                                            : r.byte_offset_line)
                 << CYAN << ":" << COLOR_RESET;
      } else {
        _ostream << (_options.only_matching ? r.byte_offset_match
                                            : r.byte_offset_line)
                 << ":";
      }
    }
    if (_options.only_matching) {
      if (_options.color) {
        _ostream << RED << r.str << COLOR_RESET << '\n';
      } else {
        _ostream << r.str << '\n';
      }
    } else {
      if (_options.color) {
        // search for every occurrence of pattern within the string and
        // print it out colored while the rest is printed uncolored.
        size_t shift = 0;
        size_t match_size = _options.pattern.size();
        bool stop = false;
        while (true) {
          size_t match_pos;
          if (_options.regex || _options.no_ascii) {
            re2::RE2 re_pattern(
                '(' +
                std::string(_options.regex
                                ? _options.pattern
                                : xs::utils::str::escaped(_options.pattern)) +
                ')');
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
          _ostream << std::string(
                          r.str.begin() + static_cast<int64_t>(shift),
                          r.str.begin() + static_cast<int64_t>(match_pos))
                   << RED << std::string(r.str.data() + match_pos, match_size)
                   << COLOR_RESET;
          // start next search at new shift
          shift = match_pos + match_size;
          if (stop || shift >= r.str.size()) {
            break;
          }
        }
        // print rest of the string (eq. pythonic substr is str[shift:])
        _ostream << std::string(r.str.begin() + static_cast<int64_t>(shift),
                                r.str.end())
                 << '\n';
      } else {
        _ostream << r.str << '\n';
      }
    }
  }
}
