// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/tasks/base/InplaceProcessor.h>

#include <condition_variable>
#include <memory>

namespace xs::task::processor {

/**
 * LZ4Decompressor: Predefined processor that decompresses LZ4 compressed data
 */
class LZ4Decompressor : public base::InplaceProcessor<DataChunk> {
 public:
  LZ4Decompressor() = default;

  void process(DataChunk* data) override;
};

/**
 * ZSTDDecompressor: A predefined processor that decompresses ZStandard
 *  compressed data
 */
class ZSTDDecompressor : public base::InplaceProcessor<DataChunk> {
 public:
  ZSTDDecompressor() = default;

  void process(DataChunk* data) override;
};

/**
 * LZ4Compressor: Predefined processor that decompresses LZ4 compressed data
 */
class LZ4Compressor : public base::InplaceProcessor<DataChunk> {
 public:
  explicit LZ4Compressor(bool hc = false, int lvl = 1);

  void process(DataChunk* data) override;

 private:
  const int _compression_level;
  const bool _hc;

  uint64_t _offset = 0;
  uint64_t _next_chunk_id = 0;
  std::unique_ptr<std::mutex> _offset_mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _offset_cv =
      std::make_unique<std::condition_variable>();
};

/**
 * ZSTDDecompressor: A predefined processor that decompresses ZStandard
 *  compressed data
 */
class ZSTDCompressor : public base::InplaceProcessor<DataChunk> {
 public:
  explicit ZSTDCompressor(int lvl = 3);

  void process(DataChunk* data) override;

 private:
  const int _compression_level;

  uint64_t _offset = 0;
  uint64_t _next_chunk_id = 0;
  std::unique_ptr<std::mutex> _offset_mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _offset_cv =
      std::make_unique<std::condition_variable>();
};

/**
 * NewLineSearcher: A predefined processor searching new line indices and adds
 *  them to the mapping data of a xs::DataChunk.
 */
class NewLineSearcher : public base::InplaceProcessor<DataChunk> {
 public:
  NewLineSearcher() = default;
  explicit NewLineSearcher(uint64_t distance);

  void process(DataChunk* data) override;

 private:
  uint64_t _line_index = 0;
  uint64_t _next_chunk_id = 0;
  /// all _distance bytes, the mapping data are added.
  ///  At most one mapping data pair per line is added.
  const uint64_t _distance = 500;

  std::unique_ptr<std::mutex> _line_index_mutex =
      std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _line_index_cv =
      std::make_unique<std::condition_variable>();
};

}  // namespace xs::task::processor