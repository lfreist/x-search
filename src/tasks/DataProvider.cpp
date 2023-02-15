// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/tasks/DataProvider.h>
#include <xsearch/utils/InlineBench.h>

namespace xs::tasks {

// ----- ExternBlockMetaReader -------------------------------------------------
// _____________________________________________________________________________
ExternBlockMetaReader::ExternBlockMetaReader(std::string file_path,
                                             const std::string& meta_file_path)
    : _file_path(std::move(file_path)),
      _meta_file(meta_file_path, std::ios::in) {}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, uint64_t>>
ExternBlockMetaReader::getNextData() {
  INLINE_BENCHMARK_WALL_START(read, "reading");
  INLINE_BENCHMARK_WALL_START_GLOBAL("construct stream");
  std::ifstream stream(_file_path);
  INLINE_BENCHMARK_WALL_STOP("construct stream");
  auto optCmd = _meta_file.nextChunkMetaData();
  if (!optCmd.has_value()) {
    return {};
  }
  auto& cmd = optCmd.value();
  INLINE_BENCHMARK_WALL_START_GLOBAL("seeking file position");
  stream.seekg(static_cast<int64_t>(cmd.actual_offset), std::ios::beg);
  INLINE_BENCHMARK_WALL_STOP("seeking file position");
  xs::DataChunk chunk(std::move(cmd));
  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  stream.read(chunk.data(), static_cast<int64_t>(chunk.size()));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// ----- ExternBlockReader -----------------------------------------------------
// _____________________________________________________________________________
ExternBlockReader::ExternBlockReader(std::string file_path, size_t min_size,
                                     size_t max_oversize)
    : _file_path(std::move(file_path)),
      _min_size(min_size),
      _max_oversize(max_oversize) {
  _file_stream.open(_file_path);
  if (!_file_stream.is_open()) {
    throw std::runtime_error("ERROR: Could not open file '" + _file_path +
                             "'.");
  }
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, uint64_t>> ExternBlockReader::getNextData() {
  std::unique_lock lock(*_stream_mutex);
  INLINE_BENCHMARK_WALL_START(read, "reading");
  ChunkMetaData cmd{_current_index,
                    _current_offset,
                    _current_offset,
                    _min_size + _max_oversize,
                    _min_size + _max_oversize,
                    {}};
  DataChunk chunk(std::move(cmd));
  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  _file_stream.read(chunk.data(), static_cast<int64_t>(_min_size));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  auto num_bytes_read = _file_stream.gcount();

  if (num_bytes_read > 0 && chunk.data()[num_bytes_read - 1] != '\n' &&
      !_file_stream.eof()) {
    int64_t additional_bytes_read = 0;
    INLINE_BENCHMARK_WALL_START_GLOBAL("read until next new line");
    while (true) {
      if (static_cast<size_t>(additional_bytes_read) > _max_oversize) {
        throw std::runtime_error(
            "ERROR: maximum size exceeded while reading data: " +
            std::to_string(additional_bytes_read));
      }
      _file_stream.read(chunk.data() + num_bytes_read + additional_bytes_read,
                        1);
      if (chunk.data()[num_bytes_read + additional_bytes_read] == '\n' ||
          _file_stream.eof()) {
        additional_bytes_read++;
        break;
      }
      additional_bytes_read++;
    }
    INLINE_BENCHMARK_WALL_STOP("read until next new line");
    num_bytes_read += additional_bytes_read;
  }
  if (num_bytes_read > 0) {
    chunk.resize(num_bytes_read);
    chunk.getMetaData().actual_size = num_bytes_read;
    chunk.getMetaData().original_size = num_bytes_read;
    _current_offset += num_bytes_read;
    return std::make_pair(std::move(chunk), _current_index++);
  }
  return {};
}

}  // namespace xs::tasks