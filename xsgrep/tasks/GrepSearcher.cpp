// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include "./GrepSearcher.h"

// ----- Helper function -------------------------------------------------------
// _____________________________________________________________________________
std::string get_regex_match_(const char* data, size_t size,
                             const re2::RE2& pattern) {
  re2::StringPiece input(data, size);
  re2::StringPiece match;
  re2::RE2::PartialMatch(input, pattern, &match);
  return match.as_string();
}

// ===== GrepSearcher ==========================================================
// _____________________________________________________________________________
GrepSearcher::GrepSearcher(GrepOptions options) : _options(std::move(options)) {
  if (_options.regex) {
    re2::RE2::Options re2_options;
    re2_options.set_posix_syntax(true);
    re2_options.set_case_sensitive(!_options.ignore_case);
    _re_pattern =
        std::make_unique<re2::RE2>('(' + _options.pattern + ')', re2_options);
  } else if (_options.ignore_case && _options.no_ascii) {
    // not regex, but ignore case and not ascii: use re2 and implicitly assume
    // pattern to be UTF-8
    re2::RE2::Options re2_options;
    re2_options.set_case_sensitive(false);
    auto escaped_pattern = xs::utils::str::escaped(_options.pattern);
    _re_pattern =
        std::make_unique<re2::RE2>('(' + escaped_pattern + ')', re2_options);
  }
}

// _____________________________________________________________________________
std::vector<GrepPartialResult> GrepSearcher::process(
    xs::DataChunk* data) const {
  if (_options.regex || (_options.ignore_case && _options.no_ascii)) {
    return process_regex(data);
  }
  return process_plain(data);
}

// _____________________________________________________________________________
std::vector<GrepPartialResult> GrepSearcher::process_regex(
    xs::DataChunk* data) const {
  std::vector<uint64_t> byte_offsets =
      _options.only_matching
          ? xs::search::regex::global_byte_offsets_match(data, *_re_pattern,
                                                         false)
          : xs::search::regex::global_byte_offsets_line(data, *_re_pattern);
  std::vector<uint64_t> line_numbers;
  if (_options.line_number) {
    line_numbers = xs::map::bytes::to_line_indices(data, byte_offsets);
    std::transform(line_numbers.begin(), line_numbers.end(),
                   line_numbers.begin(), [](uint64_t li) { return li + 1; });
  }
  std::vector<std::string> lines;
  if (_options.only_matching) {
    lines.resize(byte_offsets.size());
    std::transform(byte_offsets.begin(), byte_offsets.end(), lines.begin(),
                   [&](uint64_t index) {
                     size_t local_byte_offset =
                         index - data->getMetaData().original_offset;
                     return get_regex_match_(data->data() + local_byte_offset,
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
    res[i].line_number = _options.line_number ? line_numbers[i] : 0;
    res[i].byte_offset_match = byte_offsets[i];
    res[i].byte_offset_line = byte_offsets[i];
    res[i].str = lines[i];
  }
  return res;
}

// _____________________________________________________________________________
std::vector<GrepPartialResult> GrepSearcher::process_plain(
    xs::DataChunk* data) const {
  xs::DataChunk tmp_chunk;
  xs::DataChunk* op_chunk = data;
  if (_options.ignore_case) {
    tmp_chunk = xs::DataChunk(*data);
    xs::utils::str::simd::toLower(tmp_chunk.data(), tmp_chunk.size());
    op_chunk = &tmp_chunk;
  }
  std::vector<uint64_t> byte_offsets_match =
      xs::search::global_byte_offsets_match(op_chunk, _options.pattern,
                                            !_options.only_matching);
  std::vector<uint64_t> byte_offsets_line;
  std::vector<uint64_t> line_numbers;
  if (_options.line_number) {
    line_numbers =
        xs::map::bytes::to_line_indices(op_chunk, byte_offsets_match);
    std::transform(line_numbers.begin(), line_numbers.end(),
                   line_numbers.begin(), [](uint64_t li) { return li + 1; });
  }
  std::vector<std::string> lines;
  lines.reserve(byte_offsets_match.size());
  if (_options.only_matching) {
    for (auto bo : byte_offsets_match) {
      lines.emplace_back(data->data() + bo - data->getMetaData().actual_offset,
                         _options.pattern.size());
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
    res[i].line_number = _options.line_number ? line_numbers[i] : 0;
    res[i].byte_offset_line = _options.only_matching ? 0 : byte_offsets_line[i];
    res[i].byte_offset_match = byte_offsets_match[i];
    res[i].str = lines[i];
  }
  return res;
}

// _____________________________________________________________________________
std::vector<GrepPartialResult> GrepSearcher::process_utf8(
    xs::DataChunk* data) const {
  return {};
}