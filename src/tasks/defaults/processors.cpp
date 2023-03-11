// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/string_search/simd_search.h>
#include <xsearch/tasks/defaults/processors.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>
#include <xsearch/utils/string_utils.h>

#include <cassert>

namespace xs::task::processor {

// _____________________________________________________________________________
void LZ4Decompressor::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START(_, "decompression");
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
}

// _____________________________________________________________________________
void ZSTDDecompressor::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START(_, "decompression");
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
}

// _____________________________________________________________________________
LZ4Compressor::LZ4Compressor(bool hc, int lvl)
    : _compression_level(lvl), _hc(hc) {}

// _____________________________________________________________________________
void LZ4Compressor::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START(_, "compression");
  assert(data->size() <= std::numeric_limits<int>::max());
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
  *data = DataChunk(compressed.data(), compressed.size(), std::move(cmd));
  _offset_cv->notify_all();
}

// _____________________________________________________________________________
ZSTDCompressor::ZSTDCompressor(int lvl) : _compression_level(lvl) {}

// _____________________________________________________________________________
void ZSTDCompressor::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START(_, "compression");
  assert(data->size() <= std::numeric_limits<int>::max());
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
  *data = DataChunk(compressed.data(), compressed.size(), std::move(cmd));
  _offset_cv->notify_all();
}

// _____________________________________________________________________________
NewLineSearcher::NewLineSearcher(uint64_t distance) : _distance(distance) {}

// _____________________________________________________________________________
void NewLineSearcher::process(DataChunk* data) {
  INLINE_BENCHMARK_WALL_START(_, "adding line index data");
  std::vector<ByteToNewLineMappingInfo> mapping_data;
  int64_t shift = 0;
  uint64_t current_distance = 0;
  uint64_t local_line_index = 0;
  uint64_t prev_shift = 0;
  // Add first byte of data to mapping data.
  //  This assures that at least one mapping data pair is available for each
  //  chunk, even, if the mapping distance is larger than the chunk size.
  mapping_data.push_back({data->getMetaData().original_offset, 0});
  while (true) {
    // By searching the next new line char, we assure that at most one
    //  mapping data pair is added, even if the _distance is reached before.
    shift = search::simd::findNextNewLine(data->data(), data->size(), shift);
    if (shift == -1) {
      break;
    }
    current_distance += shift - prev_shift;
    prev_shift = shift;
    // We store the line index of the next line rather than that of the found
    //  line. This is because we do not want to map the byte offset of the
    //  new line char to a line index. Why? Simply because matches found
    //  within search processes most likely are NOT new line chars but
    //  byte offsets of the first char of a line.
    local_line_index++;
    shift++;
    if (static_cast<uint64_t>(shift) >= data->size()) {
      break;
    }
    // only add the data, if the _distance is reached
    if (current_distance >= _distance) {
      mapping_data.push_back(
          {static_cast<uint64_t>(shift) + data->getMetaData().original_offset,
           local_line_index});
      current_distance = 0;
    }
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
}

}  // namespace xs::task::processor