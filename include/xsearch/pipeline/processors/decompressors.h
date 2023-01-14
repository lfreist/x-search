// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>

#include <functional>
#include <vector>

namespace xs::processors::decompress {

/**
 * decompress ZSTD compressed data->data() into itself.
 * @param data
 * @return
 */
bool using_zstd(DataChunk* data);

/**
 * decompress LZ4 compressed data->data() into itself.
 * @param data
 * @return
 */
bool using_lz4(DataChunk* data);

}  // namespace xs::processors::decompress
