// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/tasks/Searcher.h>
#include <xsearch/utils/utils.h>

namespace xs::tasks {

// ===== MatchCounter ==========================================================
// _____________________________________________________________________________
void MatchCounter::search(const std::string& pattern,
                                         DataChunk* data,
                                         PartialResult* result) const {
  result->_count = search::count(data, pattern, false);
}

// _____________________________________________________________________________
void MatchCounter::search(re2::RE2* pattern,
                                         DataChunk* data,
                                         PartialResult* result) const {
  result->_count = search::regex::count(data, *pattern, false);
}

// -----------------------------------------------------------------------------

// ===== MatchBytePositionSearcher =============================================
// _____________________________________________________________________________
void MatchBytePositionSearcher::search(
    const std::string& pattern, DataChunk* data, PartialResult* result) const {
  result->_byte_offsets =
      search::global_byte_offsets_match(data, pattern, false);
}

// _____________________________________________________________________________
void MatchBytePositionSearcher::search(re2::RE2* pattern,
                                                      DataChunk* data,
                                                      PartialResult* result) const {
  result->_byte_offsets =
      search::regex::global_byte_offsets_match(data, *pattern, false);
}

// -----------------------------------------------------------------------------

// ===== LineBytePositionSearcher ==============================================
// _____________________________________________________________________________
void LineBytePositionSearcher::search(
    const std::string& pattern, DataChunk* data, PartialResult* result) const {
  result->_byte_offsets =
      search::global_byte_offsets_line(data, pattern);
}

// _____________________________________________________________________________
void LineBytePositionSearcher::search(re2::RE2* pattern,
                                       DataChunk* data,
                                       PartialResult* result) const {
  result->_byte_offsets =
      search::regex::global_byte_offsets_line(data, *pattern);
}

// -----------------------------------------------------------------------------

// ===== LineIndexSearcher =====================================================
// _____________________________________________________________________________
void LineIndexSearcher::search(
    const std::string& pattern, DataChunk* data, PartialResult* result) const {
  result->_line_indices =
      search::line_indices(data, pattern);
}

// _____________________________________________________________________________
void LineIndexSearcher::search(re2::RE2* pattern,
                                       DataChunk* data,
                                       PartialResult* result) const {
  result->_line_indices =
      search::regex::line_indices(data, *pattern);
}

// -----------------------------------------------------------------------------

/*
// ===== LinesSearcher =========================================================
// _____________________________________________________________________________
void LinesSearcher::search(
    const std::string& pattern, DataChunk* data, PartialResult* result) const {
  result->_lines =
      search::line_indices(data, pattern);
}

// _____________________________________________________________________________
void LinesSearcher::search(re2::RE2* pattern,
                               DataChunk* data,
                               PartialResult* result) const {
  result->_lines =
      search::regex::line_indices(data, *pattern);
}

// -----------------------------------------------------------------------------
*/
}  // namespace xs::tasks