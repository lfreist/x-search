// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/simd_search.h>
#include <xsearch/tasks/InplaceProcessors.h>
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
LZ4Compressor::LZ4Compressor(bool hc, int lvl)
    : _compression_level(lvl), _hc(hc) {}

// _____________________________________________________________________________
void LZ4Compressor::process(DataChunk* data) {
  assert(data->size() <= std::numeric_limits<int>::max());
  INLINE_BENCHMARK_WALL_START("compression");
  auto compressed = xs::utils::compression::LZ4::compress(
      data->data(), static_cast<int>(data->size()), _hc, _compression_level);
  auto cmd = std::move(data->getMetaData());
  cmd.actual_size = compressed.size();
  // we need the new actual offset, which depends on previous chunks -> wait for
  // them to be processed.
  std::unique_lock lock(*_offset_mutex);
  _offset_cv->wait(lock, [&]() { return cmd.chunk_index == _next_chunk_id; });
  cmd.actual_offset = _offset;
  _offset += compressed.size();
  _next_chunk_id++;
  *data = DataChunk({std::make_move_iterator(compressed.begin()),
                     std::make_move_iterator(compressed.end())},
                    std::move(cmd));
  _offset_cv->notify_all();
}

// _____________________________________________________________________________
ZSTDCompressor::ZSTDCompressor(int lvl) : _compression_level(lvl) {}

// _____________________________________________________________________________
void ZSTDCompressor::process(DataChunk* data) {
  assert(data->size() <= std::numeric_limits<int>::max());
  INLINE_BENCHMARK_WALL_START("compression");
  auto compressed = xs::utils::compression::ZSTD::compress(
      data->data(), static_cast<int>(data->size()), _compression_level);
  auto cmd = std::move(data->getMetaData());
  cmd.actual_size = compressed.size();
  // we need the new actual offset, which depends on previous chunks -> wait for
  // them to be processed.
  std::unique_lock lock(*_offset_mutex);
  _offset_cv->wait(lock, [&]() { return cmd.chunk_index == _next_chunk_id; });
  cmd.actual_offset = _offset;
  _offset += compressed.size();
  _next_chunk_id++;
  *data = DataChunk({std::make_move_iterator(compressed.begin()),
                     std::make_move_iterator(compressed.end())},
                    std::move(cmd));
  _offset_cv->notify_all();
}

// _____________________________________________________________________________
NewLineSearcher::NewLineSearcher(uint64_t distance) : _distance(distance) {}

// _____________________________________________________________________________
void NewLineSearcher::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START("adding line index data");
  std::vector<ByteToNewLineMappingInfo> mapping_data;
  int64_t shift = 0;
  uint64_t current_distance = 0;
  uint64_t local_line_index = 0;
  uint64_t prev_shift = 0;
  while (true) {
    // By searching the next new line char, we assure that at most one
    //  mapping data pair is added, even if the _distance is reached before.
    shift = search::simd::findNextNewLine(data->data(), data->size(), shift);
    if (shift == -1) {
      break;
    }
    current_distance += shift - prev_shift;
    prev_shift = shift;
    // only add the data, if the _distance is reached
    if (current_distance >= _distance) {
      mapping_data.push_back(
          {static_cast<uint64_t>(shift) + data->getMetaData().original_offset,
           local_line_index});
      current_distance = 0;
    }
    local_line_index++;
    shift++;
  }
  // We have searched all local line indices and now want to transform them to
  //  global line indices. Therefor, we need to now the line indices of the
  //  precedent chunks.
  // We lock the processing thread until all precedent chunks have been
  //  processed and their line indices were added to global line index
  //  (_line_index).
  std::unique_lock lock(*_line_index_mutex);
  _line_index_cv->wait(lock, [&]() {
    return data->getMetaData().chunk_index == _next_chunk_id;
  });
  std::for_each(
      mapping_data.begin(), mapping_data.end(),
      [&](ByteToNewLineMappingInfo& mi) { mi.globalLineIndex += _line_index; });
  _line_index += local_line_index;
  data->getMetaData().line_mapping_data = std::move(mapping_data);
  _next_chunk_id++;
  _line_index_cv->notify_all();
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