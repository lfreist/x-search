// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/simd_search.h>

namespace xs::map::bytes {

// _____________________________________________________________________________
std::vector<uint64_t> to_line_indices(
    const xs::DataChunk* data,
    const std::vector<uint64_t>& match_global_byte_offsets) {
  const auto& mapping_data = data->getMetaData().line_mapping_data;
  // we store the iterators outside the for loop, because we assume that the
  //  match_global_byte_offsets vector is sorted. Thus, we do not need to
  //  perform the full binary search for every single byte offset but rather can
  //  continue at the previous iterators.
  auto first = mapping_data.begin();
  auto last = mapping_data.end();

  std::vector<uint64_t> result;
  result.reserve(match_global_byte_offsets.size());

  // perform binary search to the mapping data that is closest to the byte
  //  offset that we work for
  for (const auto bo : match_global_byte_offsets) {
    // indicates whether we need to search new line indices forwards or
    //  backwards later (depends on the relative position of the closest mapping
    //  data...)
    int continue_by_one = -1;
    if (bo > first->globalByteOffset) {
      first = std::lower_bound(
          first, last, bo,
          [](const ByteToNewLineMappingInfo& element, uint64_t value) -> bool {
            return element.globalByteOffset < value;
          });
      if (first == mapping_data.end()) {
        continue_by_one = 1;
        // set to last element of vector
        first -= 1;
      }
    }
    uint64_t line_index = first->globalLineIndex;
    uint64_t current_global_byte_offset = first->globalByteOffset;

    // Search new lines starting from the result of the binary search above.
    while (true) {
      if (current_global_byte_offset == bo) {
        // if we perform backwards search and bo is the byte offset of a new
        //  line character, we need to consider it for the line_index value.
        if (continue_by_one == -1 &&
            data->data()[current_global_byte_offset -
                         data->getMetaData().actual_offset] == '\n') {
          line_index--;
        }
        // line index found
        result.push_back(line_index);
        break;
      }
      // This is placed after the above if statement because, if forward search
      //  is performed and bo is the offset of a new line char, we do not want
      //  to consider it for the line_index value...
      if (data->data()[current_global_byte_offset -
                       data->getMetaData().actual_offset] == '\n') {
        line_index += continue_by_one;
      }
      current_global_byte_offset += continue_by_one;
    }
  }
  return result;
}

}  // namespace xs::map::bytes

namespace xs::map::byte {

// _____________________________________________________________________________
std::string to_line(xs::DataChunk* data, uint64_t match_global_byte_offset) {
  uint64_t match_local_byte_offset =
      match_global_byte_offset - data->getMetaData().actual_offset;
  std::string result;
  if (match_local_byte_offset >= data->size()) {
    // we should never reach this point!
    throw std::runtime_error("ERROR: out of scope.");
  }
  char* match = data->data() + match_local_byte_offset;
  // we search the position of the previous new line char starting at the byte
  //  position of the match...
  char* prev_nl = (*match == '\n') ? match - 1 : match;
  while (prev_nl > data->data() && *prev_nl != '\n') {
    prev_nl--;
  }
  // ... we then search the next new line char...
  char* next_nl = xs::search::simd::strchr(
      match, data->size() - (match - data->data()), '\n');
  // ... and construct a std::string from prev_nl to next_nl
  if (next_nl == nullptr) {
    // if the match occurs in the last line of the file, we might not fight a
    //  next_nl if the file does not end with a new line char.
    auto res = std::string(prev_nl + (*prev_nl == '\n' ? 1 : 0),
                           data->data() + data->size() - (prev_nl + 1));
    return res;
  } else {
    return {prev_nl + (*prev_nl == '\n' ? 1 : 0), next_nl};
  }
}

}  // namespace xs::map::byte