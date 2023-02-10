// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/tasks/DataProvider.h>
#include <xsearch/utils/InlineBench.h>

namespace xs::tasks {

ExternBlockReader::ExternBlockReader(std::string file_path,
                                     const std::string& meta_file_path)
    : _file_path(std::move(file_path)),
      _meta_file(meta_file_path, std::ios::in) {}

std::optional<std::pair<DataChunk, uint64_t>> ExternBlockReader::getNextData() {
  INLINE_BENCHMARK_WALL_START("reading");
  INLINE_BENCHMARK_WALL_START("construct stream");
  std::ifstream stream(_file_path);
  INLINE_BENCHMARK_WALL_STOP("construct stream");
  auto optCmd = _meta_file.nextChunkMetaData();
  if (!optCmd.has_value()) {
    INLINE_BENCHMARK_WALL_STOP("reading");
    return {};
  }
  auto& cmd = optCmd.value();
  INLINE_BENCHMARK_WALL_START("seeking file position");
  stream.seekg(static_cast<int64_t>(cmd.actual_offset), std::ios::beg);
  INLINE_BENCHMARK_WALL_STOP("seeking file position");
  xs::DataChunk chunk(cmd.actual_size, cmd.original_size, cmd.original_offset,
                      std::move(cmd.line_mapping_data), cmd.chunk_index);
  INLINE_BENCHMARK_WALL_START("actual read");
  stream.read(chunk.data(), static_cast<int64_t>(cmd.actual_size));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  INLINE_BENCHMARK_WALL_STOP("reading");
  return std::make_pair(std::move(chunk), cmd.chunk_index);
}

}  // namespace xs::tasks