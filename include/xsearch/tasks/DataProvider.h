// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/MetaFile.h>
#include <xsearch/utils/TSQueue.h>

namespace xs::tasks {

// ===== Base Class ============================================================
template <class DataT>
class BaseDataProvider {
 public:
  BaseDataProvider() = default;
  virtual ~BaseDataProvider() = default;

  virtual std::optional<std::pair<DataT, uint64_t>> getNextData() = 0;
};
// -----------------------------------------------------------------------------

// _____________________________________________________________________________
class FileBlockMetaReader : public BaseDataProvider<DataChunk> {
 public:
  FileBlockMetaReader(std::string file_path, const std::string& meta_file_path);

  std::optional<std::pair<DataChunk, uint64_t>> getNextData() override;

 private:
  std::string _file_path;
  MetaFile _meta_file;
};

// _____________________________________________________________________________
class FileBlockMetaReaderMMAP : public BaseDataProvider<DataChunk> {
 public:
  FileBlockMetaReaderMMAP(std::string file_path,
                          const std::string& meta_file_path);

  std::optional<std::pair<DataChunk, uint64_t>> getNextData() override;

 private:
  static std::optional<std::pair<DataChunk, uint64_t>> _read_no_mmap(
      const std::string& file_path, ChunkMetaData cmd);
  std::string _file_path;
  MetaFile _meta_file;
};

// _____________________________________________________________________________
class FileBlockReader : public BaseDataProvider<DataChunk> {
 public:
  explicit FileBlockReader(std::string file_path, size_t min_size = 16777216,
                           size_t max_oversize = 65536);

  std::optional<std::pair<DataChunk, uint64_t>> getNextData() override;

 private:
  const std::string _file_path;
  const size_t _min_size;
  const size_t _max_oversize;
  uint64_t _current_index = 0;
  uint64_t _current_offset = 0;
  std::ifstream _file_stream;
  std::unique_ptr<std::mutex> _stream_mutex = std::make_unique<std::mutex>();
};

// _____________________________________________________________________________
class FileBlockReaderMMAP : public BaseDataProvider<DataChunk> {
 public:
  explicit FileBlockReaderMMAP(std::string file_path,
                               size_t min_size = 16777216,
                               size_t max_oversize = 65536);

  std::optional<std::pair<DataChunk, uint64_t>> getNextData() override;

 private:
  std::optional<std::pair<DataChunk, uint64_t>> _read_no_mmap();

  const std::string _file_path;
  const size_t _min_size;
  const size_t _max_oversize;
  const size_t _mmap_read_size;
  size_t _file_size;
  uint64_t _current_index = 0;
  uint64_t _current_offset = 0;
  std::unique_ptr<std::mutex> _stream_mutex = std::make_unique<std::mutex>();
};

}  // namespace xs::tasks