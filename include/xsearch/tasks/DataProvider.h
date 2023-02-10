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

class ExternBlockReader : public BaseDataProvider<DataChunk> {
 public:
  ExternBlockReader(std::string file_path, const std::string& meta_file_path);

  std::optional<std::pair<DataChunk, uint64_t>> getNextData() override;

 private:
  std::string _file_path;
  MetaFile _meta_file;
};

}  // namespace xs::tasks