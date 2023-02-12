// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/simd_search.h>
#include <xsearch/tasks/Processor.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>

namespace xs::tasks {

// _____________________________________________________________________________
void LZ4Decompressor::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("decompression");
  // get and update meta data
  auto cmd = std::move(data->getMetaData());
  cmd.actual_size = cmd.original_size;
  cmd.actual_offset = cmd.original_offset;
  // create new chunk
  DataChunk chunk(std::move(cmd));
  // decompress into new chunk
  xs::utils::compression::LZ4::decompressToBuffer(data->data(), data->size(),
                                                  chunk.data(), chunk.size());
  *data = std::move(chunk);
  INLINE_BENCHMARK_WALL_STOP("decompression");
}

// _____________________________________________________________________________
void ZSTDDecompressor::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("decompression");
  // get and update meta data
  auto cmd = std::move(data->getMetaData());
  cmd.actual_size = cmd.original_size;
  cmd.actual_offset = cmd.original_offset;
  // create new chunk
  DataChunk chunk(std::move(cmd));
  // decompress into new chunk
  xs::utils::compression::ZSTD::decompressToBuffer(data->data(), data->size(),
                                                   chunk.data(), chunk.size());
  *data = std::move(chunk);
  INLINE_BENCHMARK_WALL_STOP("decompression");
}

// _____________________________________________________________________________
NewLineSearcher::NewLineSearcher(uint64_t distance) : _distance(distance) {}

// _____________________________________________________________________________
void NewLineSearcher::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("adding line index data");
  std::vector<ByteToNewLineMappingInfo> mapping_data;
  int64_t shift = 0;
  uint64_t current_distance = 0;
  while (true) {
    // By searching the next new line char, we assure that at most one
    //  mapping data pair is added, even if the _distance is reached before.
    shift = search::simd::findNextNewLine(data->data(), data->size(), shift);
    if (shift == -1) {
      break;
    }
    current_distance += shift;
    // only add the data, if the _distance is reached
    if (current_distance >= _distance) {
      mapping_data.push_back(
          {static_cast<uint64_t>(shift) + data->getMetaData().original_offset,
           _line_index});
    }
    _line_index++;
    shift++;
  }
  data->getMetaData().line_mapping_data = std::move(mapping_data);
  INLINE_BENCHMARK_WALL_STOP("adding line index data");
}

// _____________________________________________________________________________
void ToLower::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("transforming to lower case");
  std::transform(data->getData().begin(), data->getData().end(),
                 data->getData().begin(),
                 [](int c) { return std::tolower(c); });
  INLINE_BENCHMARK_WALL_STOP("transforming to lower case");
}

}  // namespace xs::tasks