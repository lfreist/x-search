// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/tasks/Searcher.h>
#include <xsearch/utils/utils.h>

namespace xs::tasks {

// ===== MatchCounter ==========================================================
// _____________________________________________________________________________
void MatchCounter::search(const std::string& pattern, DataChunk* data,
                          PartialResult* result) const {
  result->_count = search::count(data, pattern, false);
  result->_index = data->getIndex();
}

// _____________________________________________________________________________
void MatchCounter::search(re2::RE2* pattern, DataChunk* data,
                          PartialResult* result) const {
  result->_count = search::regex::count(data, *pattern, false);
  result->_index = data->getIndex();
}

// -----------------------------------------------------------------------------

// ===== LineCounter ==========================================================
// _____________________________________________________________________________
void LineCounter::search(const std::string& pattern, DataChunk* data,
                         PartialResult* result) const {
  result->_count = search::count(data, pattern, true);
  result->_index = data->getIndex();
}

// _____________________________________________________________________________
void LineCounter::search(re2::RE2* pattern, DataChunk* data,
                         PartialResult* result) const {
  result->_count = search::regex::count(data, *pattern, true);
  result->_index = data->getIndex();
}

// -----------------------------------------------------------------------------

// ===== MatchBytePositionSearcher =============================================
// _____________________________________________________________________________
void MatchBytePositionSearcher::search(const std::string& pattern,
                                       DataChunk* data,
                                       PartialResult* result) const {
  result->_byte_offsets =
      search::global_byte_offsets_match(data, pattern, false);
  result->_index = data->getIndex();
}

// _____________________________________________________________________________
void MatchBytePositionSearcher::search(re2::RE2* pattern, DataChunk* data,
                                       PartialResult* result) const {
  result->_byte_offsets =
      search::regex::global_byte_offsets_match(data, *pattern, false);
  result->_index = data->getIndex();
}

// -----------------------------------------------------------------------------

// ===== LineBytePositionSearcher ==============================================
// _____________________________________________________________________________
void LineBytePositionSearcher::search(const std::string& pattern,
                                      DataChunk* data,
                                      PartialResult* result) const {
  result->_byte_offsets = search::global_byte_offsets_line(data, pattern);
  result->_index = data->getIndex();
}

// _____________________________________________________________________________
void LineBytePositionSearcher::search(re2::RE2* pattern, DataChunk* data,
                                      PartialResult* result) const {
  result->_byte_offsets =
      search::regex::global_byte_offsets_line(data, *pattern);
  result->_index = data->getIndex();
}

// -----------------------------------------------------------------------------

// ===== LineIndexSearcher =====================================================
// _____________________________________________________________________________
void LineIndexSearcher::search(const std::string& pattern, DataChunk* data,
                               PartialResult* result) const {
  result->_line_indices =
      search::line_indices(data, result->_byte_offsets, pattern);
}

// _____________________________________________________________________________
void LineIndexSearcher::search(re2::RE2* pattern, DataChunk* data,
                               PartialResult* result) const {
  result->_line_indices =
      search::regex::line_indices(data, result->_byte_offsets, *pattern);
}

// -----------------------------------------------------------------------------

// ===== LinesSearcher =========================================================
// _____________________________________________________________________________
void LinesSearcher::search(const std::string& pattern, DataChunk* data,
                           PartialResult* result) const {
  result->_lines.reserve(result->_byte_offsets.size());
  for (auto bo : result->_byte_offsets) {
    result->_lines.push_back(xs::map::byte::to_line(data, bo));
  }
}

// _____________________________________________________________________________
void LinesSearcher::search(re2::RE2* pattern, DataChunk* data,
                           PartialResult* result) const {
  result->_lines.reserve(result->_byte_offsets.size());
  for (auto bo : result->_byte_offsets) {
    result->_lines.push_back(xs::map::byte::to_line(data, bo));
  }
}

// -----------------------------------------------------------------------------

}  // namespace xs::tasks