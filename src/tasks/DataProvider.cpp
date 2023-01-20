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
  INLINE_BENCHMARK_WALL_START("stream");
  std::ifstream stream(_file_path);
  INLINE_BENCHMARK_WALL_STOP("stream");
  INLINE_BENCHMARK_WALL_START("reading");
  std::vector<ChunkMetaData> cmds = _meta_file.nextChunkMetaData(num);
  if (cmds.empty()) {
    INLINE_BENCHMARK_WALL_STOP("reading");
    return {};
  }
  std::vector<DataChunk> dcs;
  dcs.reserve(cmds.size());
  for (auto& cmd : cmds) {
    stream.seekg(static_cast<int64_t>(cmd.actual_offset), std::ios::beg);
    xs::DataChunk chunk(cmd.actual_size, cmd.original_size, cmd.original_offset,
                        std::move(cmd.line_mapping_data), cmd.chunk_index);
    stream.read(chunk.data(), static_cast<int64_t>(cmd.actual_size));
    dcs.push_back(std::move(chunk));
  }
  INLINE_BENCHMARK_WALL_STOP("reading");
  return dcs;
}
}  // namespace xs::tasks