// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/tasks/Processor.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>

namespace xs::tasks {

void LZ4Decompressor::process(DataChunk* data) {
  DataChunk chunk(data->getOriginalSize(), data->getOriginalSize(),
                  data->getOffset(), data->moveNewLineIndices(),
                  data->getIndex());
  INLINE_BENCHMARK_WALL_START("decompression");
  xs::utils::compression::LZ4::decompressToBuffer(data->data(), data->size(),
                                                  chunk.data(), chunk.size());
  *data = std::move(chunk);
  INLINE_BENCHMARK_WALL_STOP("decompression");
}

void ZSTDDecompressor::process(DataChunk* data) {
  DataChunk chunk(data->getOriginalSize(), data->getOriginalSize(),
                  data->getOffset(), data->moveNewLineIndices(),
                  data->getIndex());
  INLINE_BENCHMARK_WALL_START("decompression");
  xs::utils::compression::ZSTD::decompressToBuffer(data->data(), data->size(),
                                                   chunk.data(), chunk.size());
  *data = std::move(chunk);
  INLINE_BENCHMARK_WALL_STOP("decompression");
}

}  // namespace xs::tasks