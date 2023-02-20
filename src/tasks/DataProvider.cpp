// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xsearch/tasks/DataProvider.h>
#include <xsearch/utils/InlineBench.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace xs::tasks {

// ----- FileBlockMetaReader -------------------------------------------------
// _____________________________________________________________________________
FileBlockMetaReader::FileBlockMetaReader(std::string file_path,
                                         const std::string& meta_file_path)
    : _file_path(std::move(file_path)),
      _meta_file(meta_file_path, std::ios::in) {}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, uint64_t>>
FileBlockMetaReader::getNextData() {
  INLINE_BENCHMARK_WALL_START(read, "reading");

  auto optCmd = _meta_file.nextChunkMetaData();
  if (!optCmd.has_value()) {
    return {};
  }
  std::ifstream stream(_file_path);
  auto& cmd = optCmd.value();
  stream.seekg(static_cast<int64_t>(cmd.actual_offset), std::ios::beg);
  xs::DataChunk chunk(std::move(cmd));
  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  stream.read(chunk.data(), static_cast<int64_t>(chunk.size()));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// ----- FileBlockMetaReaderMMAP ---------------------------------------------
// _____________________________________________________________________________
FileBlockMetaReaderMMAP::FileBlockMetaReaderMMAP(
    std::string file_path, const std::string& meta_file_path)
    : _file_path(std::move(file_path)),
      _meta_file(meta_file_path, std::ios::in) {}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, uint64_t>>
FileBlockMetaReaderMMAP::getNextData() {
  INLINE_BENCHMARK_WALL_START(read, "reading");
  auto optCmd = _meta_file.nextChunkMetaData();
  if (!optCmd.has_value()) {
    return {};
  }
  auto cmd = std::move(optCmd.value());
  if (cmd.actual_size < static_cast<uint64_t>(1 << 20)) {
    return FileBlockMetaReaderMMAP::_read_no_mmap(_file_path, std::move(cmd));
  }
  size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t page_offset = cmd.actual_offset % page_size;
  int fd = open(_file_path.c_str(), O_RDONLY);
  if (fd < 0) {
    exit(1);
  }
  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  void* mapped =
      mmap(nullptr, cmd.actual_size + page_offset, PROT_READ, MAP_PRIVATE, fd,
           static_cast<int64_t>(cmd.actual_offset - page_offset));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  close(fd);
  if (mapped == MAP_FAILED) {
    // read data using std::ifstream...
    return FileBlockMetaReaderMMAP::_read_no_mmap(_file_path, std::move(cmd));
  }
  xs::DataChunk chunk;
  chunk.assign_mmap_data(static_cast<char*>(mapped), cmd.actual_size,
                         page_offset);
  chunk.getMetaData() = std::move(cmd);
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, uint64_t>>
FileBlockMetaReaderMMAP::_read_no_mmap(const std::string& file_path,
                                       ChunkMetaData cmd) {
  std::ifstream stream(file_path);
  stream.seekg(static_cast<int64_t>(cmd.actual_offset), std::ios::beg);
  xs::DataChunk chunk(std::move(cmd));
  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  stream.read(chunk.data(), static_cast<int64_t>(chunk.size()));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// ----- FileBlockReader -----------------------------------------------------
// _____________________________________________________________________________
FileBlockReader::FileBlockReader(std::string file_path, size_t min_size,
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
std::optional<std::pair<DataChunk, uint64_t>> FileBlockReader::getNextData() {
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
    while (true) {
      if (static_cast<size_t>(additional_bytes_read) > _max_oversize) {
        throw std::runtime_error(
            "ERROR: maximum size exceeded while reading data: " +
            std::to_string(additional_bytes_read + num_bytes_read));
      }
      _file_stream.read(chunk.data() + num_bytes_read + additional_bytes_read,
                        1);
      additional_bytes_read++;
      if (chunk.data()[num_bytes_read + additional_bytes_read - 1] == '\n' ||
          _file_stream.eof()) {
        break;
      }
    }
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

// _____________________________________________________________________________
FileBlockReaderMMAP::FileBlockReaderMMAP(std::string file_path, size_t min_size,
                                         size_t max_oversize)
    : _file_path(std::move(file_path)),
      _min_size(min_size),
      _max_oversize(max_oversize),
      // rounding _max size to a factorial of page size
      _mmap_read_size(sysconf(_SC_PAGE_SIZE) *
                      ceil(static_cast<double>(min_size + max_oversize) /
                           static_cast<double>(sysconf(_SC_PAGE_SIZE)))) {
  int fd = open(_file_path.c_str(), O_RDONLY);
  _file_size = lseek(fd, 0, SEEK_END);
  close(fd);
}

std::optional<std::pair<DataChunk, uint64_t>>
FileBlockReaderMMAP::getNextData() {
  std::unique_lock lock(*_stream_mutex);
  INLINE_BENCHMARK_WALL_START(read, "reading");
  if (_current_offset >= _file_size) {
    return {};
  }
  int fd = open(_file_path.c_str(), O_RDONLY);
  if (fd < 0) {
    throw std::runtime_error("ERROR: Could not open file '" + _file_path + "'");
  }
  size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t page_offset = _current_offset % page_size;
  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  char* buffer;
  void* mapped =
      mmap(nullptr, _mmap_read_size + page_size, PROT_READ, MAP_PRIVATE, fd,
           static_cast<int64_t>(_current_offset - page_offset));
  close(fd);
  if (mapped == MAP_FAILED) {
    // read data using std::ifstream
    return FileBlockReaderMMAP::_read_no_mmap();
  }
  buffer = static_cast<char*>(mapped);
  INLINE_BENCHMARK_WALL_STOP("actual read");
  // search new line char
  size_t size = std::min<size_t>(_file_size - _current_offset, _min_size);
  // get last char of data
  size_t offset = page_offset + size;
  if (size == _min_size) {
    while (true) {
      if (buffer[offset] == '\n' ||
          offset - page_offset + _current_offset >= _file_size) {
        break;
      }
      offset++;
      if (offset - page_offset - size > _max_oversize) {
        throw std::runtime_error("ERROR: failed to find new line char.");
      }
    }
  }
  size_t actual_size = offset - page_offset;
  xs::DataChunk chunk;
  chunk.assign_mmap_data(buffer, actual_size, page_offset);
  chunk.getMetaData() = {_current_index, _current_offset, _current_offset,
                         actual_size,    actual_size,     {}};
  _current_offset += actual_size;
  return std::make_pair(std::move(chunk), _current_index++);
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, uint64_t>>
FileBlockReaderMMAP::_read_no_mmap() {
  ChunkMetaData cmd{_current_index,
                    _current_offset,
                    _current_offset,
                    _min_size + _max_oversize,
                    _min_size + _max_oversize,
                    {}};
  DataChunk chunk(std::move(cmd));
  std::ifstream stream(_file_path);
  stream.seekg(static_cast<int64_t>(_current_offset), std::ios::beg);
  INLINE_BENCHMARK_WALL_START_GLOBAL("actual read");
  stream.read(chunk.data(), static_cast<int64_t>(_min_size));
  INLINE_BENCHMARK_WALL_STOP("actual read");
  auto num_bytes_read = stream.gcount();

  if (num_bytes_read > 0 && chunk.data()[num_bytes_read - 1] != '\n' &&
      !stream.eof()) {
    int64_t additional_bytes_read = 0;
    while (true) {
      if (static_cast<size_t>(additional_bytes_read) > _max_oversize) {
        throw std::runtime_error(
            "ERROR: maximum size exceeded while reading data: " +
            std::to_string(additional_bytes_read + num_bytes_read));
      }
      stream.read(chunk.data() + num_bytes_read + additional_bytes_read, 1);
      additional_bytes_read++;
      if (chunk.data()[num_bytes_read + additional_bytes_read - 1] == '\n' ||
          stream.eof()) {
        break;
      }
    }
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