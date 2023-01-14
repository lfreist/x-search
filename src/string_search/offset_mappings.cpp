// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/simd_search.h>

namespace xs::map::bytes {

// _____________________________________________________________________________
std::vector<uint64_t> to_line_indices(
    const xs::DataChunk* data,
    const std::vector<uint64_t>& match_local_byte_offsets) {
  const auto& mapping_data = data->getNewLineIndices();
  auto first = mapping_data.begin();
  auto last = mapping_data.end();
  std::vector<uint64_t> result;
  result.reserve(match_local_byte_offsets.size());
  for (auto& bo : match_local_byte_offsets) {
    int increase_by_one = -1;
    if (bo + data->getOffset() > first->globalByteOffset) {
      first = std::lower_bound(
          first, last, bo + data->getOffset(),
          [](const ByteToNewLineMappingInfo& element, uint64_t value) -> bool {
            return element.globalByteOffset < value;
          });
      if (first == mapping_data.end()) {
        increase_by_one = 1;
        // set to last element of vector
        first -= 1;
      }
    }
    uint64_t line_index = first->globalLineIndex;
    uint64_t current_global_byte_offset = first->globalByteOffset;
    while (true) {
      if (data->str()[current_global_byte_offset - data->getOffset()] == '\n') {
        line_index += increase_by_one;
      }
      if (current_global_byte_offset == bo + data->getOffset()) {
        result.push_back(line_index);
        break;
      }
      current_global_byte_offset += increase_by_one;
    }
  }
  return result;
}

}  // namespace xs::map::bytes

namespace xs::map::byte {

// _____________________________________________________________________________
uint64_t to_line_index(const xs::DataChunk* data,
                       const uint64_t match_local_byte_offset) {
  if (match_local_byte_offset > data->size()) {
    throw std::runtime_error("ERROR: byte offset out of range.");
  }
  const auto& mapping_data = data->getNewLineIndices();
  if (mapping_data.empty()) {
    // mapping data obviously are required for mappings...
    throw std::runtime_error(
        "ERROR: meta data containing new line indices must be provided for "
        "line index searching");
  }
  // get the mapping data that is closest to the byte offset of the match
  const auto lower = std::lower_bound(
      mapping_data.begin(), mapping_data.end(),
      match_local_byte_offset + data->getOffset(),
      [](const ByteToNewLineMappingInfo& element, uint64_t value) -> bool {
        return element.globalByteOffset < value;
      });
  int increase_by_one;
  uint64_t line_index;
  uint64_t current_global_byte_offset;
  // Check if the line number we are looking for is closer when we count line
  //  numbers forwards or backwards. This way we achieve to manually count the
  //  minimum number of new line chars possible.
  if (lower == mapping_data.end()) {
    // count line numbers and byte offsets forwards
    increase_by_one = 1;
    line_index = mapping_data.back().globalLineIndex;
    current_global_byte_offset = mapping_data.back().globalByteOffset;
  } else {
    // count line numbers and byte offsets backwards
    increase_by_one = -1;
    line_index = lower->globalLineIndex;
    current_global_byte_offset = lower->globalByteOffset;
  }
  while (true) {
    if (current_global_byte_offset - data->getOffset() > data->size()) {
      // TODO: check if this is correct!
      //  We previously threw a runtime error here but I think it should be fine
      //  this way...
      break;
    }
    if (data->str()[current_global_byte_offset - data->getOffset()] == '\n') {
      line_index += increase_by_one;
    }
    if (current_global_byte_offset ==
        match_local_byte_offset + data->getOffset()) {
      break;
    }
    current_global_byte_offset += increase_by_one;
  }
  return line_index;
}

// _____________________________________________________________________________
std::string to_line(xs::DataChunk* data, uint64_t match_local_byte_offset) {
  std::string result;
  char* data_p = data->data();
  if (match_local_byte_offset > data->size()) {
    // TODO: check if this error occurs where it shouldn't
    // match_local_byte_offset = data->size();
    throw std::runtime_error("ERROR: out of scope.");
  }
  char* match = data_p + match_local_byte_offset;
  char* prev_nl = (*match == '\n') ? match - 1 : match;
  while (prev_nl > data_p && *prev_nl != '\n') {
    prev_nl--;
  }
  char* next_nl =
      xs::search::simd::strchr(match, data->size() - (match - data_p), '\n');
  if (next_nl == nullptr) {
    auto res = std::string(prev_nl + (*prev_nl == '\n' ? 1 : 0),
                           data->data() + data->size() - (prev_nl + 1));
    res.push_back('\n');
    return res;
  } else {
    return {prev_nl + (*prev_nl == '\n' ? 1 : 0), next_nl + 1};
  }
}

}  // namespace xs::map::byte