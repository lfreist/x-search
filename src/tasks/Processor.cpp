// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/tasks/Processor.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>
#include <xsearch/string_search/simd_search.h>

namespace xs::tasks {

// _____________________________________________________________________________
void LZ4Decompressor::process(DataChunk* data) {
  DataChunk chunk(data->getOriginalSize(), data->getOriginalSize(),
                  data->getOffset(), data->moveMappingData(), data->getIndex());
  INLINE_BENCHMARK_WALL_START("decompression");
  xs::utils::compression::LZ4::decompressToBuffer(data->data(), data->size(),
                                                  chunk.data(), chunk.size());
  *data = std::move(chunk);
  INLINE_BENCHMARK_WALL_STOP("decompression");
}

// _____________________________________________________________________________
void ZSTDDecompressor::process(DataChunk* data) {
  DataChunk chunk(data->getOriginalSize(), data->getOriginalSize(),
                  data->getOffset(), data->moveMappingData(), data->getIndex());
  INLINE_BENCHMARK_WALL_START("decompression");
  xs::utils::compression::ZSTD::decompressToBuffer(data->data(), data->size(),
                                                   chunk.data(), chunk.size());
  *data = std::move(chunk);
  INLINE_BENCHMARK_WALL_STOP("decompression");
}

// _____________________________________________________________________________
void NewLineSearcher::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("adding line index data");
  std::vector<ByteToNewLineMappingInfo> mapping_data;
  int64_t shift = 0;
  while (true) {
    shift = search::simd::findNextNewLine(data->data(), data->size(), shift);
    if (shift == -1) {
      break;
    }
    mapping_data.push_back({static_cast<uint64_t>(shift) + data->getOffset(), _line_index});
    _line_index++;
    shift++;
  }
  data->setMappingData(std::move(mapping_data));
  INLINE_BENCHMARK_WALL_STOP("adding line index data");
}

}  // namespace xs::tasks