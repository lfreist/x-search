// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>

namespace xs::map::bytes {

/**
 * Map byte offsets from match_local_byte offsets to line indices
 *
 * !! MARK:
 *     - match_local_byte_offsets must be sorted!
 *     - data._byte_to_nl_mapping_data must not be empty!
 * !!
 *
 * @param data
 * @param match_local_byte_offsets
 * @return std::vector<uint64_t>: line indices
 */
std::vector<uint64_t> to_line_indices(
    const xs::DataChunk* data,
    const std::vector<uint64_t>& match_local_byte_offsets);

}  // namespace xs::map::bytes

namespace xs::map::byte {

/**
 * Returns the line containing the byte offset.
 *
 * @param data
 * @param match_local_byte_offset
 * @return std::string: full line containing byte offset
 */
std::string to_line(xs::DataChunk* data, uint64_t match_global_byte_offset);

}  // namespace xs::map::byte