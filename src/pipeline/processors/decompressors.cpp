// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/pipeline/processors/decompressors.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>

namespace xs::processors::decompress {

/**
 * decompress LZ4 compressed data->data() and store the result in the same
 * DataChunk.
 * @param data
 * @return
 */
bool using_zstd(DataChunk* data) {
  DataChunk chunk(data->getOriginalSize(), data->getOriginalSize(),
                  data->getOffset(), data->moveNewLineIndices());
  INLINE_BENCHMARK_WALL_START("decompression");
  xs::utils::compression::ZSTD::decompressToBuffer(data->data(), data->size(),
                                                   chunk.data(), chunk.size());
  *data = std::move(chunk);
  INLINE_BENCHMARK_WALL_STOP("decompression");
  return true;
}

/**
 * decompress LZ4 compressed data->data() and store the result in the same
 * DataChunk.
 * @param data
 * @return
 */
bool using_lz4(DataChunk* data) {
  DataChunk chunk(data->getOriginalSize(), data->getOriginalSize(),
                  data->getOffset(), data->moveNewLineIndices());
  INLINE_BENCHMARK_WALL_START("decompression");
  xs::utils::compression::LZ4::decompressToBuffer(data->data(), data->size(),
                                                  chunk.data(), chunk.size());
  *data = std::move(chunk);
  INLINE_BENCHMARK_WALL_STOP("decompression");
  return true;
}

}  // namespace xs::processors::decompress