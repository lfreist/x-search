// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/simd_search.h>

namespace xs::map::bytes {

// _____________________________________________________________________________
std::vector<uint64_t> to_line_indices(
    const xs::DataChunk* data,
    const std::vector<uint64_t>& match_global_byte_offsets) {
  const auto& mapping_data = data->getNewLineIndices();
  auto first = mapping_data.begin();
  auto last = mapping_data.end();
  std::vector<uint64_t> result;
  result.reserve(match_global_byte_offsets.size());
  for (auto& bo : match_global_byte_offsets) {
    int increase_by_one = -1;
    if (bo > first->globalByteOffset) {
      first = std::lower_bound(
          first, last, bo,
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
      if (current_global_byte_offset == bo) {
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
std::string to_line(xs::DataChunk* data, uint64_t match_global_byte_offset) {
  uint64_t match_local_byte_offset =
      match_global_byte_offset - data->getOffset();
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