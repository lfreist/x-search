// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xsearch/tasks/DataProvider.h>
#include <xsearch/utils/InlineBench.h>

#include <cstdlib>

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

// ----- ExternBlockMetaReaderMMAP ---------------------------------------------
// _____________________________________________________________________________
ExternBlockMetaReaderMMAP::ExternBlockMetaReaderMMAP(
    std::string file_path, const std::string& meta_file_path)
    : _file_path(std::move(file_path)),
      _meta_file(meta_file_path, std::ios::in) {}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, uint64_t>>
ExternBlockMetaReaderMMAP::getNextData() {
  INLINE_BENCHMARK_WALL_START(read, "reading");
  auto optCmd = _meta_file.nextChunkMetaData();
  if (!optCmd.has_value()) {
    return {};
  }
  auto cmd = std::move(optCmd.value());
  INLINE_BENCHMARK_WALL_START_GLOBAL("construct stream");
  int fd = open(_file_path.c_str(), O_RDONLY);
  INLINE_BENCHMARK_WALL_STOP("construct stream");
  if (fd < 0) {
    exit(1);
  }
  xs::DataChunk chunk;
  size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t page_offset = cmd.actual_offset % page_size;
  if (page_size > cmd.actual_offset && page_offset != 0) {
    return ExternBlockMetaReaderMMAP::getData(_file_path, std::move(cmd));
  }

  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  chunk._data = static_cast<char*>(
      mmap(nullptr, cmd.actual_size + page_offset, PROT_READ, MAP_PRIVATE, fd,
           static_cast<int64_t>(cmd.actual_offset - page_offset)));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  chunk.set_size(cmd.actual_size);
  chunk.set_mmap_offset(page_offset);
  chunk.set_mmap();
  chunk.getMetaData() = std::move(cmd);
  close(fd);
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, uint64_t>>
ExternBlockMetaReaderMMAP::getData(const std::string& file_path,
                                   ChunkMetaData cmd) {
  std::ifstream stream(file_path);
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

ExternBlockReaderMMAP::ExternBlockReaderMMAP(std::string file_path,
                                             size_t max_size)
    : _file_path(std::move(file_path)),
      _max_size(max_size),
      _mmap_read_size(max_size - max_size % sysconf(_SC_PAGE_SIZE) +
                      sysconf(_SC_PAGE_SIZE)) {
  int fd = open(_file_path.c_str(), O_RDONLY);
  _file_size = lseek(fd, 0, SEEK_END);
  close(fd);
}

std::optional<std::pair<DataChunk, uint64_t>>
ExternBlockReaderMMAP::getNextData() {
  std::unique_lock lock(*_stream_mutex);
  if (_current_offset >= _file_size) {
    return {};
  }
  INLINE_BENCHMARK_WALL_START(read, "reading");
  INLINE_BENCHMARK_WALL_START_GLOBAL("construct stream");
  int fd = open(_file_path.c_str(), O_RDONLY);
  INLINE_BENCHMARK_WALL_STOP("construct stream");
  if (fd < 0) {
    throw std::runtime_error("ERROR: Could not open file '" + _file_path + "'");
  }
  xs::DataChunk chunk;
  size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t page_offset = _current_offset % page_size;
  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  chunk._data = static_cast<char*>(
      mmap(nullptr, _mmap_read_size + page_size, PROT_READ, MAP_PRIVATE, fd,
           static_cast<int64_t>(_current_offset - page_offset)));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  // search new line char
  size_t size = std::min<size_t>(_file_size - _current_offset, _max_size);
  INLINE_BENCHMARK_WALL_START(_, "searching previous new line");
  size_t offset = page_offset + size;
  if (size == _max_size) {
    while (true) {
      if (chunk._data[offset] == '\n') {
        break;
      }
      offset--;
      if (offset <= 0) {
        throw std::runtime_error("ERROR: failed to find new line char.");
      }
    }
  } else {
    int i = 0;
  }
  INLINE_BENCHMARK_WALL_STOP("searching previous new line");
  size_t actual_size = offset - page_offset;
  chunk.set_size(actual_size);
  chunk.set_mmap_offset(page_offset);
  chunk.set_mmap();
  chunk.getMetaData() = {_current_index, _current_offset, _current_offset,
                         actual_size,    actual_size,     {}};
  close(fd);
  _current_offset += actual_size;
  _current_index++;
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

}  // namespace xs::tasks