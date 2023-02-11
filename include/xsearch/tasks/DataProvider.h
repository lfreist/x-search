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
class ExternBlockMetaReader : public BaseDataProvider<DataChunk> {
 public:
  ExternBlockMetaReader(std::string file_path,
                        const std::string& meta_file_path);

  std::optional<std::pair<DataChunk, uint64_t>> getNextData() override;

 private:
  std::string _file_path;
  MetaFile _meta_file;
};

// _____________________________________________________________________________
class ExternBlockReader : public BaseDataProvider<DataChunk> {
 public:
  explicit ExternBlockReader(std::string file_path, size_t min_size = 16777216,
                             size_t max_oversize = 16384);

  std::optional<std::pair<DataChunk, uint64_t>> getNextData() override;

 private:
  const std::string _file_path;
  const size_t _min_size = 16777216;
  const size_t _max_oversize = 16384;
  uint64_t _current_index = 0;
  uint64_t _current_offset = 0;
  std::ifstream _file_stream;
  std::unique_ptr<std::mutex> _stream_mutex = std::make_unique<std::mutex>();
};

}  // namespace xs::tasks