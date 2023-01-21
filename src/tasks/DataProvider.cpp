// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/tasks/DataProvider.h>
#include <xsearch/utils/InlineBench.h>

namespace xs::tasks {

ExternBlockReader::ExternBlockReader(std::string file_path,
                                     const std::string& meta_file_path)
    : _file_path(std::move(file_path)),
      _meta_file(meta_file_path, std::ios::in) {}

std::vector<DataChunk> ExternBlockReader::getNextData(int num) {
  INLINE_BENCHMARK_WALL_START("construct stream");
  std::ifstream stream(_file_path);
  INLINE_BENCHMARK_WALL_STOP("construct stream");
  std::vector<ChunkMetaData> cmds = _meta_file.nextChunkMetaData(num);
  if (cmds.empty()) {
    return {};
  }
  INLINE_BENCHMARK_WALL_START("reading");
  std::vector<DataChunk> dcs;
  dcs.reserve(cmds.size());
  for (auto& cmd : cmds) {
    INLINE_BENCHMARK_WALL_START("seeking file position");
    stream.seekg(static_cast<int64_t>(cmd.actual_offset), std::ios::beg);
    INLINE_BENCHMARK_WALL_STOP("seeking file position");
    xs::DataChunk chunk(cmd.actual_size, cmd.original_size, cmd.original_offset,
                        std::move(cmd.line_mapping_data), cmd.chunk_index);
    INLINE_BENCHMARK_WALL_START("actual read");
    stream.read(chunk.data(), static_cast<int64_t>(cmd.actual_size));
    INLINE_BENCHMARK_WALL_STOP("actual read");
    dcs.push_back(std::move(chunk));
  }
  INLINE_BENCHMARK_WALL_STOP("reading");
  return dcs;
}
}  // namespace xs::tasks