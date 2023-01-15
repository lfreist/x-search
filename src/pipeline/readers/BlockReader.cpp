// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/pipeline/reader/BlockReader.h>
#include <xsearch/utils/InlineBench.h>

namespace xs::reader {

BlockReader::BlockReader(std::string file_path, std::string meta_file_path,
                         int chunks_per_read)
    : _file_path(std::move(file_path)),
      _meta_file_path(std::move(meta_file_path)),
      _meta_file(_meta_file_path, std::ios::in) {
  _chunks_per_read = chunks_per_read;
}

std::vector<DataChunk> BlockReader::read() {
  INLINE_BENCHMARK_WALL_START("stream");
  std::ifstream stream(_file_path);
  INLINE_BENCHMARK_WALL_STOP("stream");
  INLINE_BENCHMARK_WALL_START("reading");
  std::vector<ChunkMetaData> cmds =
      _meta_file.nextChunkMetaData(_chunks_per_read);
  if (cmds.empty()) {
    INLINE_BENCHMARK_WALL_STOP("reading");
    return {};
  }
  std::vector<DataChunk> dcs;
  dcs.reserve(cmds.size());
  for (auto& cmd : cmds) {
    stream.seekg(static_cast<int64_t>(cmd.actual_offset), std::ios::beg);
    xs::DataChunk chunk(cmd.actual_size, cmd.original_size, cmd.original_offset,
                        std::move(cmd.line_mapping_data));
    stream.read(chunk.data(), static_cast<int64_t>(cmd.actual_size));
    dcs.push_back(std::move(chunk));
  }
  INLINE_BENCHMARK_WALL_STOP("reading");
  return dcs;
}

}  // namespace xs::reader